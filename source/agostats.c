// agostats.c : Defines the entry point for the console application.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "versionhistory.h"
#include "globals.h"
#include "agostats.h"
#include "clantags.h"
#include "events.h"
#include "flagtracking.h"
#include "game.h"
#include "highscores.h"
#include "maps.h"
#include "matching.h"
#include "output.h"
#include "players.h"
#include "sorting.h"
#include "squads.h"
#include "stathandling.h"
#include "teams.h"
#include "tools.h"
#include "weapons.h"
#include "awards.h"
#include "skiplist.h"

#if defined(SERVER_STATS)
	#define DEFAULT_LOGFILE		"games.log"
	#define SECONDARY_LOGFILE	NULL
#else
	#define DEFAULT_LOGFILE		"etconsole.log"
	#define SECONDARY_LOGFILE	"qconsole.log"
#endif

#define RESULT_CTF_DRAW     "%s and %s drew with %ld caps each"
#define RESULT_CTF_WIN      "%s beat %s with %ld caps to %ld"

#if defined (_DEBUG)
	FILE *debugout;
#endif

FILE *input;
long file_position;
long file_position_game_begin;
int searching_last_game;

int USE_STDIN    ;
int FILE_OFFSET  ;  // start parsing logfile at offset
int ONLY_LAST_GAME;
int CUSTOM_TABLE_SEQUENCE;
int	KEEP_INDIVIDUAL_HIGHSCORES; // True/False : keep highscores for individual players
int stop_processing;
int first_blood_awarded;
int first_blood_killer;
int first_blood_victim;


long linecounter	= 1;
long logfile_offset = 0;

char current_date [ 20 ];


// TABLE ITEMS for fragmatrix
typedef enum {
	TI_UNDEFINED,
	TI_ROW_INIT,
	TI_ROW_TERM,
	TI_CELL_TOPLEFT,
	TI_CELL_EMPTY,
	TI_CELL_LONG,
	TI_CELL_LONG_TK,      // teamkills
	TI_CELL_LONG_SUICIDE, // teamkills
	TI_CELL_LONG_GREEN,   // off v def and def v off frags
	TI_CELL_LONG_YELLOW,  // suspected chase frags
	TI_CELL_LONG_WHITE,   // def v def frags
	TI_CELL_PLAYER,
	TI_CELL_STRING
} TABLE_ITEM;



// AWARD ITEM
typedef enum {
	AI_UNDEFINED,
	AI_INIT,
	AI_ROW_INIT,
	AI_NAME, // awardname
	AI_WINNER,
	AI_ROW_TERM,
	AI_TERM
} AWARD_ITEM;



// Global variables
static char inputfile		[ 255 ];
static char table_sequence  [ 255 ];
static int matched_line; // indicates which pattern is matched
int tracebug = FALSE;
static int  end_of_file   = FALSE;


// Functions
#if defined (_DEBUG)
void debugmsg( char *msg )
{
	const p1 = 3;
	const p2 = 2;
	fprintf( debugout, "%s", msg );
	//if ( player_count >= 7 ) {
	//fprintf( debugout, "%s vs %s : %c %c\n",PLAYERS[p1].nameclean,PLAYERS[p2].nameclean, frag_matrix[ p1 ][ p2 ].same_team , frag_matrix[ p2 ][ p1 ].same_team );
	//}
	//if ( 0 == strcmp( msg, "na if 1:3 7+ +" ) ) {
	/*if ( 0 == strncmp( msg, "na if 1:3 7",11 ) ) {
		int x=0;
	}*/
}
void test( void )
{
//WIN32_FIND_DATA findData;
}
#endif

void check_first_blood( int killer_nr, int victim_nr )
{
	if ( first_blood_awarded != TRUE ) {
		first_blood_killer = killer_nr;
		first_blood_victim = victim_nr;
		first_blood_awarded = TRUE;
	}
}

int killer_is_environment( void )
{
	// return true if the string in the killer variable implies the killer is the environment
	int result = FALSE;

	// used for "sentry is killed by somebody"
	// this happens when the sentry (or supplystation)is killed by closing doors or moving lifts for instance
	// if there is a player using the name "somebody", we will give him the benefit of the doubt :)

	if ( 0 == strcmp( killer, "somebody^7" ) ) {
		if ( player_exists( killer ) < 0 ) {
			result = TRUE;
		}
	}

	return result;
}

void process_throw_pin( void )
{
	int p;
	
	process_status_change( GS_LIVE );
	p = select_player( player );
	if ( p == -1 ) {
		return;
	}
	mark_player_line( p, linecounter );
	PLAYERS[ p ].sit_on_grenade++;
}

void process_unknown( void )
{

}

// sentry status is used at the end of the game
// to determine if the sentry is up or not
void set_sentry_status( int player, int status )
{
	PLAYERS[ player ].sentry_up = status;
}

// set the relation between 2 players
// create 2 new players if their current relation conflicts with that
void set_player_player_relation( int* p1, int* p2, char* p1_name, char* p2_name, int player_relation )
{
	switch ( player_relation ) {
		case PLAYERS_SAME_TEAM:
			if ( frag_matrix[ *p1 ][ *p2 ].same_team == PLAYERS_DIFFERENT_TEAM ) {
				// we got a slight problem, create 2 new players because we don't
				// know which one is wrong
				PLAYERS[ *p1 ].enabled = FALSE;
				PLAYERS[ *p2 ].enabled = FALSE;
				*p1 = select_player( p1_name );
				*p2 = select_player( p2_name );
			}
			frag_matrix[ *p1 ][ *p2 ].same_team = player_relation;
			frag_matrix[ *p2 ][ *p1 ].same_team = player_relation;
			break;

		case PLAYERS_DIFFERENT_TEAM:
			if ( frag_matrix[ *p1 ][ *p2 ].same_team == PLAYERS_SAME_TEAM ) {
				// we got a slight problem, create 2 new players because we don't
				// know which one is wrong
				PLAYERS[ *p1 ].enabled = FALSE;
				PLAYERS[ *p2 ].enabled = FALSE;
				*p1 = select_player( p1_name );
				*p2 = select_player( p2_name );
			}
			frag_matrix[ *p1 ][ *p2 ].same_team = player_relation;
			frag_matrix[ *p2 ][ *p1 ].same_team = player_relation;
			break;
	}
}

void process_teamkill( void )
{
	frag_mask *f;
	int temp_flag;
	int weapon_nr;

	f = (frag_mask*)&LOG_PATTERNS[ matched_line ].info1;

	process_status_change( GS_LIVE );
	killer_nr = select_player( killer );
	victim_nr = select_player( victim );

	if ( killer_nr == -1 || victim_nr == -1 ) {
		return;
	}

	if ( f->extra_info == INFO_MATCHLOG ) {
		weapon_nr = get_matchlog_weapon( weapon );
	}
	else {
		weapon_nr = f->weapon;
	}

	mark_player_line( killer_nr, linecounter );
	set_player_player_relation( &killer_nr, &victim_nr, killer, victim, PLAYERS_SAME_TEAM );

	PLAYERS[ victim_nr ].current_fragstreak = 0;

	// update fragmatrix
	frag_matrix[ killer_nr ][ victim_nr ].frags++;
	frag_matrix[ killer_nr ][ victim_nr ].frags_per_weapon[ weapon_nr ]++;

	if ( killer_nr != victim_nr ) {
		PLAYERS[ killer_nr ].teamkills++;			
		PLAYERS[ victim_nr ].teamdeaths++;

		frag_matrix[ killer_nr ][ victim_nr ].same_team = PLAYERS_SAME_TEAM;
		frag_matrix[ victim_nr ][ killer_nr ].same_team = PLAYERS_SAME_TEAM;
	}
	else { // killer_nr == victim_nr
		// sometimes there's a message AGO Pook was vaporized by ally AGO Pook
		// handle it like a suicide
		PLAYERS[ victim_nr ].suicides++;
	}

	// track flags involved 
	if ( player_carrying_flag( victim_nr, &temp_flag ) ) {
		set_flagstatus( FS_FIELD, temp_flag, victim_nr );
		update_player_score( PE_TEAMKILL_FLAGCARRIER, killer_nr, 1 );

		// count frags on flagcarrier per weapon
		frag_matrix[ killer_nr ][ victim_nr ].frags_on_flagcarrier_per_weapon[ weapon_nr ]++;
	}
	// flag tracked
}
#if defined(SERVER_STATS)
void process_server_teamkill( void )
{
	frag_mask *f;
	int temp_flag;
	int weapon_nr;

	f = (frag_mask*)&LOG_PATTERNS[ matched_line ].info1;
	weapon_nr = get_server_weapon( weapon );

	// track flags involved 
	if ( player_carrying_flag( victim_nr, &temp_flag ) ) {
		set_flagstatus( FS_FIELD, temp_flag, victim_nr );
		update_player_score( PE_TEAMKILL_FLAGCARRIER, killer_nr, 1 );

		// count frags on flagcarrier per weapon
		frag_matrix[ killer_nr ][ victim_nr ].frags_on_flagcarrier_per_weapon[ weapon_nr ]++;
	}
	// flag tracked

	PLAYERS[ victim_nr ].current_fragstreak = 0;
}

void process_server_suicide( void )
{
	frag_mask *f;
	int temp_flag;
	int weapon_nr;

	f = (frag_mask*)&LOG_PATTERNS[ matched_line ].info1;
	weapon_nr = get_server_weapon( weapon );

	// track flags involved 
	if ( player_carrying_flag( victim_nr, &temp_flag ) ) {
		set_flagstatus( FS_FIELD, temp_flag, victim_nr );

		// count frags on flagcarrier per weapon
		frag_matrix[ victim_nr ][ victim_nr ].frags_on_flagcarrier_per_weapon[ weapon_nr ]++;
	}
	// flag tracked

	PLAYERS[ victim_nr ].suicides++;
	PLAYERS[ victim_nr ].current_fragstreak = 0;

}

void process_server_building_destroyed( void )
{
	frag_mask *f;

	if ( game_status != GS_LIVE ) {
		// ignore it, these buildings get destroyed because players leave the game
		// after it has ended
		return;
	}

	killer_nr = select_player( killer );			
	victim_nr = select_player( victim );

	if ( killer_nr == -1 || victim_nr == -1 ) {
		return;
	}

	f = (frag_mask*)&LOG_PATTERNS[ matched_line ].info1;
	switch ( f->death_type ) {
		case T_KILLED_AUTOSENTRY:
		// because we don't know if this is a regular or teamkill, we mark it and decide later
		set_sentry_status( victim_nr, FALSE );	
		frag_matrix[ killer_nr ][ victim_nr ].fragged_sentry++;
		break;

	case T_KILLED_SUPPLYSTATION:
		// because we don't know if this is a regular or teamkill, we mark it and decide later
		frag_matrix[ killer_nr ][ victim_nr ].fragged_supplystation++;
		break;
	}

	// more stuff is processed in function process_buildings_destroyed();
}

void process_server_frag( void )
{
	// kills, teamkills and suicides
	// todo: handle sentry and supplystation kills
	frag_mask *f;
	int temp_flag;
	int weapon_nr;

	f = (frag_mask*)&LOG_PATTERNS[ matched_line ].info1;
	
	if ( f->death_type == T_KILLED_AUTOSENTRY || f->death_type == T_KILLED_SUPPLYSTATION ) {
		process_server_building_destroyed();
		return;
	}

	weapon_nr = get_server_weapon( weapon );
	//if ( weapon_nr == W_UNKNOWN ) {
	//	printf( "%s\x0d", linebuffer); // use in release version to find unknown weapons
	//}

	killer_nr = select_player_by_number( killer );			
	victim_nr = select_player_by_number( victim );

	if ( killer_nr == PLAYER_WORLD ) {
		killer_nr = victim_nr;
	}

	if ( killer_nr == NO_PLAYER || victim_nr == NO_PLAYER ) {
		return;
	}
	
	process_status_change( GS_LIVE );

	mark_player_line( killer_nr, linecounter );
	mark_player_line( victim_nr, linecounter );

	// update fragmatrix
	frag_matrix[ killer_nr ][ victim_nr ].frags++;
	frag_matrix[ killer_nr ][ victim_nr ].frags_per_weapon[ weapon_nr ]++;

	// update sentry status
	if ( f->weapon == W_AUTOSENTRY_BULLET   ||
		f->weapon == W_AUTOSENTRY_ROCKET    ) {

		set_sentry_status( killer_nr, TRUE );
	}
	if ( f->weapon == W_AUTOSENTRY_EXPLOSION ) {
		set_sentry_status( killer_nr, FALSE );
	}

	if ( PLAYERS[ killer_nr ].team == PLAYERS[ victim_nr ].team ) {
		// teamkill or suicide
		if ( killer_nr == victim_nr ) {
			// suicide
			process_server_suicide();
		}
		else {
			// teamkill
			process_server_teamkill();
		}
	} 
	else {
		// killed enemy
		check_first_blood( killer_nr, victim_nr );

		// track flags involved 
		if ( player_carrying_flag( victim_nr, &temp_flag ) ) {
			set_flagstatus( FS_FIELD, temp_flag, victim_nr );
			update_player_score( PE_KILL_FLAGCARRIER, killer_nr, 1 );
			
			// count frags on flagcarrier per weapon
			PLAYERS[ killer_nr ].fragged_flagcarrier_per_weapon[ weapon_nr ]++;
			frag_matrix[ killer_nr ][ victim_nr ].frags_on_flagcarrier_per_weapon[ weapon_nr ]++;
		}
		// flag tracked

		PLAYERS[ killer_nr ].frags++;
		PLAYERS[ killer_nr ].current_fragstreak++;
		PLAYERS[ killer_nr ].frags_per_weapon[ weapon_nr ]++;

		PLAYERS[ victim_nr ].deaths++;
		PLAYERS[ victim_nr ].deaths_per_weapon[ weapon_nr ]++;
		PLAYERS[ victim_nr ].current_fragstreak = 0;
		
		// update max fragstreak at each frag, otherwise a max streak at the end of the
		// game would be lost.
		if ( PLAYERS[ killer_nr ].current_fragstreak > PLAYERS[ killer_nr ].max_fragstreak ) {
			PLAYERS[ killer_nr ].max_fragstreak = PLAYERS[ killer_nr ].current_fragstreak;
		}

		// check if previous line was advanced_ctf message
		switch ( old_line_type ) {
			case LT_OBJECTIVE :

				switch ( old_line_info1 ) {
					case O_ACTF_FLAG_DEFEND_BASE :
						if ( killer_nr == old_player_nr ) { // double check
							// reward the victim with offence points
							// the killer is updated in the previous line
							update_player_score( PE_DIED_NEAR_FLAG_AT_BASE, victim_nr, 1 );
						}
						break;

					case O_ACTF_FLAG_DEFEND_FIELD :
						if ( killer_nr == old_player_nr ) { // double check
							// reward the victim with offence points
							update_player_score( PE_DIED_NEAR_FLAG_IN_FIELD, victim_nr, 1 );
						}
						break;
				
				}
				break;
		}
	}
}

void process_server_initgame( void )
{
	//remove_statfile = FALSE;
	process_status_change( GS_PRE_MATCH );
}

void process_server_mod_custom( void )
{
	// this is some sort of disconnect or kick from the server
	// do nothing:
}

void process_server_rename( void )
{
	player_nr = select_player_by_number( player );
	set_playername( player_nr, newname );
}

void process_server_set_team( void )
{
	team_nr = select_team_by_number( team );
	if ( team_nr != TEAM_SPECTATOR ) {
		
		player_nr = select_player_by_number( player );
		if ( PLAYERS[ player_nr ].team != team_nr ) {
			// only proceed if new team is different from current team

			if ( player_participated( player_nr ) ) {
				// existing player is changing team
				// disable current player

				PLAYERS[ player_nr ].enabled = FALSE;
				
				// create new player
				player_nr = select_player_by_number( player );
				set_team( player_nr, team_nr );
			}
			else {
				set_team( player_nr, team_nr );
			}
		}
	}
}

void post_process_server_stats( void )
{
	int i,j;

	for ( i = 0; i < player_count; i++ ) {
		for ( j = 0; j < player_count; j++ ) {
			if ( PLAYERS[ i ].team != PLAYERS[ j ].team ) {
				frag_matrix[ i ][ j ].same_team = PLAYERS_DIFFERENT_TEAM;
				frag_matrix[ j ][ i ].same_team = PLAYERS_DIFFERENT_TEAM;
			}
			else {
				if ( i != j ) {
					PLAYERS[ i ].teamkills  += frag_matrix[ i ][ j ].frags;
					PLAYERS[ j ].teamdeaths += frag_matrix[ i ][ j ].frags;
				}
				frag_matrix[ i ][ j ].same_team = PLAYERS_SAME_TEAM;
				frag_matrix[ j ][ i ].same_team = PLAYERS_SAME_TEAM;
			}
			if ( i == j ) {
				PLAYERS[ i ].suicides = frag_matrix[ i ][ j ].frags;
			}
		}
	}
}
void process_server_mapinfo( void )
{
	int game_index;

	if ( strlen( array[ 1 ] ) > 0 ) {
		game_index = atoi( array[ 1 ] );
		switch( game_index ){
			case 1 :
				set_gametype( GT_CAPTURE_THE_FLAG );
				break;
		}
	}

	strncopy( map, &array[ 0 ][ 0 ], VAR_ARRAY_ELEMENT_SIZE - 1 );
	process_mapinfo( FALSE ); // fill mapdescription
}
#endif

void process_matchlog_frag( void )
{
	int weapon_nr;

	weapon_nr = get_matchlog_weapon( weapon );
}


void process_frag( void )
{
	frag_mask *f;
	int temp_flag;
	int weapon_nr;

	f = (frag_mask*)&LOG_PATTERNS[ matched_line ].info1;

	switch ( f->death_type ) {
		case T_NO_DEATH :
			break;

		case T_KILL:
			if ( teamkill ) {
				process_teamkill();
				return;
			}

			process_status_change( GS_LIVE );
			killer_nr = select_player( killer );			
			victim_nr = select_player( victim );

			if ( killer_nr == -1 || victim_nr == -1 ) {
				return;
			}
			
			check_first_blood( killer_nr, victim_nr );

			if ( f->extra_info == INFO_MATCHLOG ) {
				weapon_nr = get_matchlog_weapon( weapon );
			}
			else {
				weapon_nr = f->weapon;
			}

			mark_player_line( killer_nr, linecounter );
			mark_player_line( victim_nr, linecounter );
			set_player_player_relation( &killer_nr, &victim_nr, killer, victim, PLAYERS_DIFFERENT_TEAM );
			
			// track flags involved 
			if ( player_carrying_flag( victim_nr, &temp_flag ) ) {
				set_flagstatus( FS_FIELD, temp_flag, victim_nr );
				update_player_score( PE_KILL_FLAGCARRIER, killer_nr, 1 );
				
				// count frags on flagcarrier per weapon
				PLAYERS[ killer_nr ].fragged_flagcarrier_per_weapon[ weapon_nr ]++;
				frag_matrix[ killer_nr ][ victim_nr ].frags_on_flagcarrier_per_weapon[ weapon_nr ]++;
			}
			// flag tracked
			
			PLAYERS[ killer_nr ].frags++;
			PLAYERS[ killer_nr ].current_fragstreak++;
			PLAYERS[ killer_nr ].frags_per_weapon[ weapon_nr ]++;

			PLAYERS[ victim_nr ].deaths++;
			PLAYERS[ victim_nr ].deaths_per_weapon[ weapon_nr ]++;
			PLAYERS[ victim_nr ].current_fragstreak = 0;
			
			// update max fragstreak at each frag, otherwise a max streak at the end of the
			// game would be lost.
			if ( PLAYERS[ killer_nr ].current_fragstreak > PLAYERS[ killer_nr ].max_fragstreak ) {
				PLAYERS[ killer_nr ].max_fragstreak = PLAYERS[ killer_nr ].current_fragstreak;
			}

			if ( weapon_nr == W_AUTOSENTRY_BULLET   ||
				weapon_nr == W_AUTOSENTRY_ROCKET    ) {

				set_sentry_status( killer_nr, TRUE );
			}
			if ( weapon_nr == W_AUTOSENTRY_EXPLOSION ) {
				set_sentry_status( killer_nr, FALSE );
			}
				
			frag_matrix[ killer_nr ][ victim_nr ].frags++;
			frag_matrix[ killer_nr ][ victim_nr ].frags_per_weapon[ weapon_nr ]++;
			frag_matrix[ killer_nr ][ victim_nr ].same_team = PLAYERS_DIFFERENT_TEAM;
			frag_matrix[ victim_nr ][ killer_nr ].same_team = PLAYERS_DIFFERENT_TEAM;
						
			// check if previous line was advanced_ctf message
			switch ( old_line_type ) {
				case LT_OBJECTIVE :

					switch ( old_line_info1 ) {
						case O_ACTF_FLAG_DEFEND_BASE :
							if ( killer_nr == old_player_nr ) { // double check
								// reward the victim with offence points
								// the killer is updated in the previous line
								update_player_score( PE_DIED_NEAR_FLAG_AT_BASE, victim_nr, 1 );
							}
							break;

						case O_ACTF_FLAG_DEFEND_FIELD :
							if ( killer_nr == old_player_nr ) { // double check
								// reward the victim with offence points
								update_player_score( PE_DIED_NEAR_FLAG_IN_FIELD, victim_nr, 1 );
							}
							break;
					
					}
					break;
			}

			break;

		case T_TEAMKILL:
			process_teamkill();
			break;

		case T_SUICIDE:
			// increase suicide counter
			// wrong victim number is returned
			victim_nr = select_player( victim );

			if ( victim_nr == -1 ) {
				return;
			}
			process_status_change( GS_LIVE );

			if ( f->extra_info == INFO_MATCHLOG ) {
				weapon_nr = get_matchlog_weapon( weapon );
			}
			else {
				weapon_nr = f->weapon;
			}

			mark_player_line( victim_nr, linecounter );
			// track flags involved 
			if ( player_carrying_flag( victim_nr, &temp_flag ) ) {
				set_flagstatus( FS_FIELD, temp_flag, victim_nr );

				// count frags on flagcarrier per weapon
				frag_matrix[ victim_nr ][ victim_nr ].frags_on_flagcarrier_per_weapon[ weapon_nr ]++;
			}
			// flag tracked

			PLAYERS[ victim_nr ].current_fragstreak = 0;
			PLAYERS[ victim_nr ].suicides++;

			frag_matrix[ victim_nr ][ victim_nr ].frags++;
			frag_matrix[ victim_nr ][ victim_nr ].frags_per_weapon[ weapon_nr ]++;
			break;

		/*case T_SUICIDE_ENVIRONMENT:
			// increase suicide counter
			victim_nr = select_player( victim );
			
			if ( victim_nr == -1 ) {
				return;
			}

			mark_player_line( victim_nr, linecounter );
			// track flags involved 
			if ( player_carrying_flag( victim_nr, &temp_flag ) ) {
				set_flagstatus( FS_FIELD, temp_flag, victim_nr );
			}
			// flag tracked

			PLAYERS[ victim_nr ].current_fragstreak = 0;
			PLAYERS[ victim_nr ].suicides++;
			//PLAYERS[ victim_nr ].suicides_per_environment[ f->weapon ]++; // count cratered, drowned etc.

			frag_matrix[ victim_nr ][ victim_nr ].frags++;
			break;
		*/
		case T_KILLED_AUTOSENTRY:
			// because we don't know if this is a regular or teamkill, we mark it and decide later

			victim_nr = select_player( victim );
			mark_player_line( victim_nr, linecounter );
			set_sentry_status( victim_nr, FALSE );

			if ( killer_is_environment() ) {
				PLAYERS[ victim_nr ].lost_sentry++;
			}
			else {
				process_status_change( GS_LIVE );
				killer_nr = select_player( killer );
				
				if ( killer_nr == -1 || victim_nr == -1 ) {
					return;
				}

				mark_player_line( killer_nr, linecounter );
				frag_matrix[ killer_nr ][ victim_nr ].fragged_sentry++;
			}
			break;

		case T_KILLED_SUPPLYSTATION:
			// because we don't know if this is a regular or teamkill, we mark it and decide later
			victim_nr = select_player( victim );
			mark_player_line( victim_nr, linecounter );
			if ( killer_is_environment() ) {
				PLAYERS[ victim_nr ].lost_supply++;
			}
			else {

				process_status_change( GS_LIVE );
				killer_nr = select_player( killer );
				
				if ( killer_nr == -1 || victim_nr == -1 ) {
					return;
				}
				mark_player_line( killer_nr, linecounter );
				frag_matrix[ killer_nr ][ victim_nr ].fragged_supplystation++;
			}
			break;

		default:
			break;
	}
}

void process_chat( void )
{
	// don't add player to playerlist, maybe he's only a spectator
	// or it is punkbuster
	if ( OUTPUT_MM1 ) {
		events_add( ET_CHAT, player, chat );
	}
}

void process_teamchat( void )
{
	// hehe, do nothing
}
void process_client_only( void )
{
	// hehe, do nothing
}

void process_ignore( void )
{
	// again, do nothing
}

void process_debug( void )
{

}

void process_nonstandard_flagtake( long line_extra_type )
{
	switch( line_extra_type ) {
		case EXTRA_FT_TEAMONLY :
			process_status_change( GS_LIVE );
			player_nr = select_player( player );
			
			if ( player_nr == -1 ) { // dunno why, just copied it
				return;
			}

			update_player_score( PE_FLAG_TOUCH, player_nr, 1 );

			// well it says which team a player is on (player of the RED team has taken the flag)
#if defined(SERVER_STATS)
			set_team( player_nr, team_nr );
#else
			set_player_team_relation( player_nr, team_nr, YES );
#endif
			break;

		default: break;
	}
}

void process_rock_capture( long line_info2 )
{
	switch( line_info2 ) {
		case EX_CAPTURE_BY_RED:
			flag_nr = select_flag( "BLUE" );
			team_nr = select_team( "BLUE" );
			break;
		case EX_CAPTURE_BY_BLUE:
			flag_nr = select_flag( "RED" );
			team_nr = select_team( "RED" );
			break;
	}
	player_nr =  FLAGS[ flag_nr ].carrier;
	mark_player_line( player_nr, linecounter );
	// handle the rest like 2 flag ctf

	// at this point we don't know the team of the player, so use -1
	add_teamscore_event( -1, team_nr, player_nr, FLAGS[ flag_nr ].current_alive_touches );

	set_flagstatus( FS_CAPTURED, flag_nr, player_nr );

	// if a player captures the 'red' flag we can deduct he's not in the 'red' team
	set_player_team_relation( player_nr, team_nr, NO );

	if ( OUTPUT_EVENTS ) {
		sprintf( outbuf, "[ captured the %s flag ]", TEAMS[ team_nr ].name_html );
		events_add( ET_OBJECTIVE, et_2_html( PLAYERS[ player_nr ].name ), outbuf );
	}
	if ( team_nr >= 0 && team_nr < MAX_TEAMS ) {
		TEAMS[ team_nr ].flags_lost++;
	}
}

// flag is not given in the logfile, so we have to find out which one it is
void process_alps_capture( void )
{
	int f;
	player_nr = select_player( player );
	flag_nr = -1;

	if ( player_nr == -1 ) {
		return;
	}

	mark_player_line( player_nr, linecounter );
	
	// find out which flag he's carrying
	for ( f = 0; f < flag_count ; f++ ) {
		if ( FLAGS[ f ].carrier == player_nr ) {
			flag_nr = f;
			break;
		}
	}

	if ( flag_nr != -1 ) {

		team_nr = select_team( FLAGS[ flag_nr ].name );
		// handle the rest like 2 flag ctf

		// at this point we don't know the team of the player, so use -1
		add_teamscore_event( -1, team_nr, player_nr, FLAGS[ flag_nr ].current_alive_touches );

		set_flagstatus( FS_CAPTURED, flag_nr, player_nr );

		// if a player captures the 'red' flag we can deduct he's not in the 'red' team
		set_player_team_relation( player_nr, team_nr, NO );

		if ( OUTPUT_EVENTS ) {			
			sprintf( outbuf, "[ captured the %s flag ]", TEAMS[ team_nr ].name_html );
			events_add( ET_OBJECTIVE, et_2_html( player ), outbuf );
		}
		if ( team_nr >= 0 && team_nr < MAX_TEAMS ) {
			TEAMS[ team_nr ].flags_lost++;
		}
	}
	


}

void process_objective( void )
{
	long line_deduction  = LOG_PATTERNS[ matched_line ].info1;
	long line_info2		 = LOG_PATTERNS[ matched_line ].info2;
	long line_game_type  = LOG_PATTERNS[ matched_line ].info3;
	long line_extra_type = LOG_PATTERNS[ matched_line ].info4;
	long game_type;
	long game_type_modifier;
	int  yielding_team;

	if ( game_status == GS_POST_MATCH ) {
		// this stuff can happen after game has ended, ignore it
		return;
	}

	game_type = determine_gametype( line_game_type );
	game_type_modifier = query_gametype_modifier();
   
	// if not then disable the player and create a new one
	switch( LOG_PATTERNS[ matched_line ].info1 ) {

		case O_NO_OBJECTIVE :
			break;

		case O_CTF_FLAGTAKE:
			// default type of message : bla has taken the RED flag     ( %f = RED )
			// other options : bla of the GREEN team has taken the flag ( %t = GREEN )
			if ( line_extra_type != NO_INFO ) { // bla of the GREEN team has taken the flag ( %t = GREEN )
			//	process_nonstandard_flagtake( line_extra_type ); // todo: for dewwars
//				break;
			}

			// ok, we got a standard type of flagtake
			process_status_change( GS_LIVE );
			player_nr = select_player( player );
			flag_nr = select_flag( flag );
			
			if ( player_nr == -1 ) {
				return;
			}

			set_flagstatus( FS_CARRIED, flag_nr, player_nr );
			mark_player_line( player_nr, linecounter );
		
			// check game type modifiers		
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
				team_nr = select_team( team );
				set_player_team_relation( player_nr, team_nr, YES );
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					if ( strlen( team ) > 0 ) {
						team_nr = select_team( team );
					}
					else {
						// todo: use EX_PLAYER_IN_TEAM
						team_nr = select_team( flag ); // for dissect for instance
					}
					set_player_team_relation( player_nr, team_nr, YES );
				}
				else {
					// default:
					team_nr = select_team( flag );
					// if a player takes the 'red' flag we can deduct he's not in the 'red' team
					set_player_team_relation( player_nr, team_nr, NO );
				}
			}
			break;

		case O_CTF_FLAGDROP:
			// TODOOOOOOOO!!!!!!!!!!!!!!!!!
			break;
			// basically ignore this, track flagdrops by fragging flagcarrier
			process_status_change( GS_LIVE );
			player_nr = select_player( player );

			if ( player_nr == -1 ) {
				return;
			}

			mark_player_line( player_nr, linecounter );

			// check game type modifiers
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
		
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					// "%p has DROPPED the %f flag!\x0a"
					// if a player drops the 'red' flag we can deduct he is in the 'red' team
					team_nr = select_team( flag );
					flag_nr = select_flag( flag );
					set_flagstatus( FS_FIELD, flag_nr, player_nr );
					set_player_team_relation( player_nr, team_nr, YES );
				}
				else {
					// default:
					// "%p has DROPPED the %f flag!\x0a"
					// if a player drops the 'red' flag we can deduct he's not in the 'red' team
					team_nr = select_team( flag );
					flag_nr = select_flag( flag );
					set_flagstatus( FS_FIELD, flag_nr, player_nr );
					set_player_team_relation( player_nr, team_nr, NO );
				}
			}		
			break;

		case O_CTF_FLAGCAPTURE	: // w00t, this is what it's all about

			process_status_change( GS_LIVE );
			
			// first check for alternative capture messages
			// etf_rock
			if ( 0 != ( line_info2 & ( EX_CAPTURE_BY_RED | EX_CAPTURE_BY_BLUE ) ) ) {
				process_rock_capture( line_info2 );
				return;
			}
			
			// etf_alps
			if ( 0 != ( line_info2 & EX_NO_FLAG_INFO ) ) {
				process_alps_capture();
				return;
			}

			player_nr = select_player( player );

			if ( player_nr == -1 ) {
				return;
			}

			mark_player_line( player_nr, linecounter );

			// check game type modifiers
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
				// "%p has captured the flag in the %t base!\x0a"
				team_nr = select_team( team );

				// if a player captures the flag in the 'red' base we can deduct he's not in the 'red' team
				set_player_team_relation( player_nr, team_nr, NO );
				
				if ( OUTPUT_EVENTS ) {					
					sprintf( outbuf, "[ captured the flag in the %s base ]", TEAMS[ team_nr ].name_html );
					events_add( ET_OBJECTIVE, et_2_html( player ), outbuf );
				}
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					// "%p has CAPTURED the %f flag in the %t base!\x0a"
					if ( line_deduction && EX_PLAYER_NOT_IN_TEAM ) {
						// flag color is other team color, for instance in etf_dissect
						// player has the blue flag, player captured red
						team_nr = PLAYERS[ player_nr ].team; // workaround, todo: make more solid
						yielding_team = select_team( flag );  // flagcolor is teamcolor
						add_teamscore_event( -1, yielding_team, player_nr, FLAGS[ flag_nr ].current_alive_touches );

						if ( team_nr >= 0 && team_nr < MAX_TEAMS ) {
							TEAMS[ yielding_team ].flags_lost++;
						}
					}
					else {
						team_nr = select_team( flag );  // flagcolor is teamcolor
						add_teamscore_event( -1, team_nr, player_nr, FLAGS[ flag_nr ].current_alive_touches );
						
					}

					set_flagstatus( FS_CAPTURED, team_nr, player_nr );
				}
				else {
					// default: assume gt_ctf
					flag_nr = select_flag( flag );
					team_nr = select_team( flag );

					// at this point we don't know the team of the player, so use -1
					add_teamscore_event( -1, team_nr, player_nr, FLAGS[ flag_nr ].current_alive_touches );

					set_flagstatus( FS_CAPTURED, flag_nr, player_nr );

					// if a player captures the 'red' flag we can deduct he's not in the 'red' team
					set_player_team_relation( player_nr, team_nr, NO );

					if ( OUTPUT_EVENTS ) {						
						sprintf( outbuf, "[ captured the %s flag ]", TEAMS[ team_nr ].name_html );
						events_add( ET_OBJECTIVE, et_2_html( player ), outbuf );
					}
					if ( team_nr >= 0 && team_nr < MAX_TEAMS ) {
						TEAMS[ team_nr ].flags_lost++;
					}
				}
			}
			//
			break;

		case O_ACTF_FLAG_DEFEND_BASE	:
			process_status_change( GS_LIVE );
			team_nr = select_team( flag );
			player_nr = select_player( player );
			flag_nr = select_flag( flag );

			set_flagstatus( FS_BASE, flag_nr, NO_PLAYER );

			if ( player_nr == -1 ) {
				return;
			}

			mark_player_line( player_nr, linecounter );
			update_player_score( PE_DEFEND_FLAG_AT_BASE, player_nr, 1 );
			
			// check game type modifiers
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
				// todo: probably doesn't exist
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					// todo: does this message exist ?
				}
				else {
					// default:
					// we know in which team this player belongs
					set_player_team_relation( player_nr, team_nr, YES );
				}
			}
			break;

		case O_ACTF_FLAG_DEFEND_FIELD :
			// fuck it, this actf message is bugged on multiple maps (japanc, forts)
			return;
			// disable this for japanc
			if ( 0 == strcmp( map, "Japanese Castles (2.3)" ) ) {
				return;
			}
			
			process_status_change( GS_LIVE );
			team_nr = select_team( flag );
			player_nr = select_player( player );
			flag_nr = select_flag( flag );

			if ( player_nr == -1 ) {
				return;
			}
			mark_player_line( player_nr, linecounter );
			set_flagstatus( FS_FIELD, flag_nr, player_nr );
			update_player_score( PE_DEFEND_FLAG_IN_FIELD, player_nr, 1 );

			// check game type modifiers
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
				// todo: exists?
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					// todo: exists?
				}
				else {
					// default:
					set_player_team_relation( player_nr, team_nr, YES );
				}
			}
			break;

		case O_CTF_FLAG_RETURNED :
			flag_nr = select_flag( flag );
			set_flagstatus( FS_BASE, flag_nr, NO_PLAYER );
			break;

		case O_ACTF_CARRIER_DEFEND :
			process_status_change( GS_LIVE );
			team_nr = select_team( flag );
			player_nr = select_player( player );

			if ( player_nr == -1 ) {
				return;
			}
			mark_player_line( player_nr, linecounter );
			update_player_score( PE_DEFEND_FLAGCARRIER, player_nr, 1 );


			// check game type modifiers
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
				// todo: make sure
				set_player_team_relation( player_nr, team_nr, YES );
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					set_player_team_relation( player_nr, team_nr, YES ); // todo: make sure
				}
				else {
					// default:
					set_player_team_relation( player_nr, team_nr, YES );
				}
			}
			break;

		case O_ACTF_FRAG_ENEMY_CARRIER:
			process_status_change( GS_LIVE );
			team_nr = select_team( team );
			player_nr = select_player( player );

			if ( player_nr == -1 ) {
				return;
			}
			mark_player_line( player_nr, linecounter );

			// check game type modifiers
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
				// "%p killed the %t team's flag carrier!\x0a"
				// todo: the q3f logfile is bugged at this point, the wrong team is printed
				//	set_player_team_relation( player_nr, team_nr, NO ); // todo: test
				//	PLAYERS[ player_nr ].killed_flagcarrier++;
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					set_player_team_relation( player_nr, team_nr, NO ); // todo:test
				}
				else {
					// default:
					// if a player kills the 'red' team flagcarrier we can deduct he's not in the 'red' team
					set_player_team_relation( player_nr, team_nr, NO );
				}
			}
			break;

		case O_ACTF_FRAG_FRIENDLY_CARRIER :
			process_status_change( GS_LIVE );	
			team_nr = select_team( team );
			player_nr = select_player( player );

			if ( player_nr == -1 ) {
				return;
			}

			mark_player_line( player_nr, linecounter );

			// check game type modifiers
			if ( gtm_contains( GTM_ONE_FLAG ) ) {
				// todo: deduct teaminfo
			}
			else {
				if ( gtm_contains( GTM_REVERSE ) ) {
					// todo: deduct teaminfo
				}
				else {
					// default:
					// player teamkills his team's carrier
					set_player_team_relation( player_nr, team_nr, YES );
				}
			}
			break;

		case O_ACTF_FRAG_CAP_1	:
			process_status_change( GS_LIVE );
			break;

		case O_ACTF_FRAG_CAP_MULTI :
			process_status_change( GS_LIVE );
			break;

		case O_CAH_LOCATION_CLAIM : // X of the Y team claimed Z
			team_nr = select_team( team );
			player_nr = select_player( player );

			if ( player_nr == -1 ) {
				return;
			}
			mark_player_line( player_nr, linecounter );
			set_player_team_relation( player_nr, team_nr, YES );
			set_gametype( GT_CAPTURE_AND_HOLD );
			break;
		case O_CAH_ALL_CLAIMED :
			// todo
			break;
		default :
			break;
	}
}

void process_timelimit( void )
{
	process_status_change( GS_POST_MATCH );
	if ( OUTPUT_EVENTS ) {
		events_add( ET_GAME, "", "[ Timelimit hit ]" );
	}
}



// return : the number of teams a player may fit into
static int teams_for_player( int p )
{
	static int result;
	int i, t;
	int tempres;

	result = 0;

	for ( t = 0; t < team_count ; t++ ) {
		// test the relation of all players in team with player p
		tempres = TRUE;
		for ( i = 0; i < player_count ; i++ ) {
			if ( i != p && PLAYERS[ i ].team == t ) {
				// only compare to players in team t

				if ( frag_matrix[ i ][ p ].same_team == PLAYERS_DIFFERENT_TEAM ) {
					// he doesn't fit the team because it contains at least one enemy
					tempres = FALSE;
					break; // we don't need to look any further for this team
				}
			}

		}
		if ( tempres ) {
			result++;
		}
		
	}


	return result;
}

void debug_output_player_relations( void )
{
	int i, is, p, ps;

	sort_players( BY_TEAM );

	for ( p = 0; p < player_count; p++ ) {
		ps = player_index[ p ];
		sprintf( outbuf, "\n%s's relations to other players\n", PLAYERS[ ps ].nameclean);
		AS_OUT( OE_LINE, outbuf );
		//statfile( SF_PRINT, outbuf );

		for ( i = 0; i < player_count; i++ ) {
			is = player_index[ i ];
			//sprintf( outbuf, "%2.2d %c %s %s\n", is, frag_matrix[ is ][ ps ].same_team, PLAYERS[ is ].nameclean, TEAMS[ PLAYERS[ is ].team ].name);
			sprintf( outbuf, "%2.2d %c %-20.20s %1.1d %-3.3s\n", is, frag_matrix[ is ][ ps ].same_team, PLAYERS[ is ].nameclean, PLAYERS[ is ].team, PLAYERS[ is ].in_team );
			AS_OUT( OE_LINE, outbuf );
			//statfile( SF_PRINT, outbuf );
		}
	}
}


// count destroyed sentries sup stations
// precondition: teamrelation between players should be known
void process_buildings_destroyed( void )
{
	int i, j;

	for ( i = 0; i < player_count ; i++ ) {
		for ( j = 0; j < player_count ; j ++ ) {
		
			switch ( frag_matrix[ i ][ j ].same_team ) {
				// autosentry and supplystation frags are not counted to begin with
				case PLAYERS_SAME_TEAM :
					// count as teamkill
					PLAYERS[ i ].teamkills         += frag_matrix[ i ][ j ].fragged_sentry;
					PLAYERS[ i ].teamkilled_sentry += frag_matrix[ i ][ j ].fragged_sentry;
					PLAYERS[ j ].lost_sentry       += frag_matrix[ i ][ j ].fragged_sentry;
					break;

				case PLAYERS_DIFFERENT_TEAM :
					// add autosentry kills to frags
					PLAYERS[ i ].frags += frag_matrix[ i ][ j ].fragged_sentry;
					
					// add off score
					update_player_score( PE_DESTROY_SENTRY, i, frag_matrix[ i ][ j ].fragged_sentry );
					update_player_score( PE_DESTROY_SUPPLYSTATION, i, frag_matrix[ i ][ j ].fragged_supplystation );

					PLAYERS[ j ].lost_sentry += frag_matrix[ i ][ j ].fragged_sentry;
					PLAYERS[ j ].lost_supply += frag_matrix[ i ][ j ].fragged_supplystation;
					break;

				default:
					break;
			}
		
		}
	}
}

void process_kick( void )
{
	if ( OUTPUT_EVENTS ) {
		player_nr = select_player( player );
		if ( player_nr == -1 ) {
			return;
		}
		mark_player_line( player_nr, linecounter );		
		events_add( ET_GAME, PLAYERS[ player_nr ].name_html, "[ was kicked ]" );

	}
}

// for each player calculate indirect off and def points by
// multiplying frags + deaths against each player with their off and def points
void calculate_indirect_points( void )
{
	int i;
	int j;
	long points;
	long frags;
	long deaths;
	
	for ( i = 0; i < player_count ; i++ ) {
		for ( j = 0; j < player_count ; j++ ) {
			// only include players not in the same team
			// calculate indirect def points from enemy off points and vice versa
			if ( PLAYERS[ i ].team != PLAYERS[ j ].team ) {
				// todo: test, count frags by player more than deaths
				frags  = frag_matrix[ j ][ i ].frags;
				deaths = frag_matrix[ i ][ j ].frags;
				points = PLAYERS[ j ].off_points * ( frags + deaths ); 

				// add indirect off points to the indirect def points
				PLAYERS[ i ].def_points_indirect += points;


				points = PLAYERS[ j ].def_points * ( frags + deaths );
				//points = PLAYERS[ j ].def_points * ( frag_matrix[ i ][ j ].frags + 2 * frag_matrix[ j ][ i ].frags );
				PLAYERS[ i ].off_points_indirect += points;
			}
		}
	}
}

int player_disabled_in_team( int t )
{
	int result = -1;
	int p = 0;

	for ( p = 0; p < player_count; p++ ) {
		if ( !PLAYERS[ p ].enabled && PLAYERS[ p ].team == t ) {
			result = p;
			break;
		}
	}
	
	return result;
}

void process_player_joined_team( void )
{
	int p, player_team, joined_team;

	if ( OUTPUT_EVENTS ) {
		sprintf( outbuf, "[ joined the %s team ]", et_2_html( team ) );
		events_add( ET_OTHER, et_2_html( player ), outbuf );
	}

	if ( game_status == GS_LIVE ) {
		// only process this stuff when the game is live, otherwise there will be unnameds causing noise
		joined_team = select_team( team );

		if ( player_exists( player ) ) {
			// This player is an existing enabled player, maybe he's reconnecting or so
			// todo: he joins during the match or changes team
			p = select_player( player );
			player_team = PLAYERS[ p ].team;

			// check if team is known
			if ( player_team == -1 ) {
				// his team is not known yet, set it to the team he just joined
				// todo: this can be wrong when the rejoins another team ( player was red, times out, joins blue )
				//set_player_team_relation( p, joined_team, YES );
				//player_team = joined_team;

				// try this:
				PLAYERS[ p ].enabled = FALSE;
				// then create new player
				p = select_player( player );
				set_player_team_relation( p, joined_team, YES );
			}
			else {
				// his team is already known
				player_team = select_team( TEAMS[ PLAYERS[ p ].team ].name );
			}
			
			if ( player_team == joined_team ) {
				// hmm, no worries
			}
			else {
				// player joined a team but he already exists in a different team
				// first disable him
				PLAYERS[ p ].enabled = FALSE;
				// then create new player
				p = select_player( player );
				set_player_team_relation( p, joined_team, YES );
			}
			
		}
		else {
			// maybe this player exists and is disabled, or maybe he doesn't exist at all
			p = player_disabled_in_team( joined_team );
			if ( p != -1 ) {
				// enable him
				PLAYERS[ p ].enabled = TRUE;
			}
			else {
				// if nothing found then create a new player and add him to the team
				select_player( player );
			}
		}
	}
}

void set_userdate( void ) {
	// initialized by as.date in logfile
	sprintf( current_date, "%s-%s-%s", array[ 0 ], array[ 1 ], array[ 2 ] );
}

void process_usertyped_info( void )
{
	switch( LOG_PATTERNS[ matched_line ].info1 ) {
		case INFO_DATE :
			set_userdate();
			break;
		default:
			break;
	}
}


// check for optional strings matched
void post_process( void )
{
	if ( LOG_PATTERNS[ matched_line ].line_type == LT_FRAG ) {
		if ( 0 == strcmp( optional, "^sally^7 " ) ) {
			// we are dealing with a teamkill
			teamkill = TRUE;
		}
		// q3f
		if ( 0 == strcmp( optional, "ally " ) ) {
			// we are dealing with a teamkill
			teamkill = TRUE;
		}
		// q3f
	}
}


// this function is called after each line is parsed
void update_statistics( void ) 
{
#if defined(SERVER_STATS)
	int player_id;
#endif

	line_type = LOG_PATTERNS[ matched_line ].line_type;
	switch ( line_type ) {

		case LT_UNKNOWN:
			process_unknown();
			break;

		case LT_AGOSTATS:
			process_usertyped_info();
			break;

		case LT_OBJECTIVE : 
			process_objective();
			break;

		case LT_FRAG :
			process_frag();
			break;

		/*case LT_MATCHLOG_FRAG:
			process_matchlog_frag();
			break;*/

		case LT_MATCHLOG_SUICIDE:
			process_matchlog_frag();
			break;

		case LT_CHAT :
			process_chat();
			break;

		case LT_IGNORE:
			process_ignore();
			break;

		case LT_STATUS_CHANGE:
			process_status_change( LOG_PATTERNS[ matched_line ].info1 );
			break;

		case LT_DEBUG:
			process_debug();
			break;

		case LT_RENAME:
			rename_player();
			break;

		case LT_CONNECT:
			if ( OUTPUT_EVENTS ) {
#if defined(SERVER_STATS)
				// todo: playername unknown at this point			
#else
				sprintf( outbuf, "[ connected ]" );
				events_add( ET_OTHER, et_2_html( player ), outbuf );
#endif
			}
#if defined(SERVER_STATS)
			// disable the player in case a new player is assigned the same player number
			player_id = select_player_by_number( player );
			PLAYERS[ player_id ].enabled = FALSE;
#endif
			break;

		case LT_DISCONNECT:
			if ( OUTPUT_EVENTS ) {
#if defined(SERVER_STATS)
				// todo: playername unknown at this point			
#else
				sprintf( outbuf, "[ disconnected ]" );
				events_add( ET_OTHER, et_2_html( player ), outbuf );
#endif
			}
			break;

		case LT_JOIN:
			process_player_joined_team();
			break;

		//case LT_OVERFLOW:
		//	if ( OUTPUT_EVENTS ) {
		//		sprintf( outbuf, "%s overflowed", et_2_html( player ) );
		//		output_event_html( OBJ_PROCESS, ET_UNDEFINED, outbuf );
		//	}
		//	break;

		case LT_THROW_PIN:
			process_throw_pin();
			break;

		//case LT_SPECTATE:
		//	break;

		case LT_TIMELIMIT:
			process_timelimit();
			break;

		case LT_KICK :
			process_kick();
			break;

		case LT_MAPINFO :
			process_mapinfo( ( LOG_PATTERNS[ matched_line ].info1 == INFO_MAPSIGNATURE ));
			break;
#if defined(SERVER_STATS)
		case LTS_FRAG:
			process_server_frag();
			break;

		case LTS_INITGAME:
			process_server_initgame();
			break;

		case LTS_MOD_CUSTOM:
			process_server_mod_custom();
			break;

		case LTS_RENAME:
			process_server_rename();
			break;

		case LTS_SETTEAM:
			process_server_set_team();
			break;

		case LTS_MAPINFO:
			process_server_mapinfo();
			break;
#endif
		default :
			break;
	}
	
}

void output_program_info( void )
{
	sprintf( outbuf, HTML_PROGRAM_INFO_INIT );
	AS_OUT( OE_PLAIN, outbuf );	

	sprintf( outbuf, "statistics generated by %s (%s %s)<br />\n", PROGRAM_NAME, VERSION, OS );
	AS_OUT( OE_PLAIN, outbuf );	
	sprintf( outbuf, "visit the <a class=\"PROGRAM_INFO\" href=\"http://home.planet.nl/~pwned/agostats\">%s homepage</a><br />\n", PROGRAM_NAME );
	AS_OUT( OE_PLAIN, outbuf );	
	sprintf( outbuf, "visit the <a class=\"PROGRAM_INFO\" href=\"http://www.armygeddon.ch\">The Armygeddon</a><br />\n" );
	AS_OUT( OE_PLAIN, outbuf );	

	sprintf( outbuf, HTML_PROGRAM_INFO_TERM );
	AS_OUT( OE_PLAIN, outbuf );	

	//sprintf( outbuf, "<center class=\"LINK1\"><a href=\"http://www.xs4all.nl/~pook/agostats\">%s</a> (%s) for Q3F2, created by %s (<a href=\"http://www.armygeddon.ch\">www.armygeddon.ch</a>)</center><br />\n\n", PROGRAM_NAME, VERSION, et_2_html( "^2AGO^7 Pook" ) );
	//AS_OUT( OE_PLAIN, outbuf );	
}

// return the teamname as it will be displayed in the stats
char *display_team_name( int team )
{
	char *result;

	result = TEAMS[ team ].name_html;
	if ( strlen( TEAMS[ team ].clantag_html ) > 0 ) {
		result = TEAMS[ team ].clantag_html;
	}
	
	return result;
}

void output_match_ctf( void )
{
	
	sprintf( outbuf, "%ld active players", MATCH.players );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld teams", MATCH.teams );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld flagtouches", MATCH.flagtouches );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld captures", MATCH.captures );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld frags", MATCH.frags );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld teamkills", MATCH.teamkills  );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld suicides", MATCH.suicides  );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld sentries destroyed", MATCH.destroyed_sentry   );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld supply stations destroyed", MATCH.destroyed_supply );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );
}

void output_match_cap_and_hold( void )
{
	sprintf( outbuf, "I don't know who won (yet...)" );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld active players", MATCH.players );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, "%ld teams", MATCH.teams );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );

}

void output_match_duell( void )
{
	long score1, score2;
	int p1, p2;

	sort_players( BY_DUELL_SCORE );

	p1 = player_index[ 0 ];
	p2 = player_index[ 1 ];
	
	score1 = PLAYERS[ p1 ].frags - PLAYERS[ p1 ].suicides;
	score2 = PLAYERS[ p2 ].frags - PLAYERS[ p2 ].suicides;

	sprintf( outbuf, "");
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );
	
	sprintf( outbuf, "%s <b>&#160; VS.&#160;</b> %s", PLAYERS[ p1 ].name_html, PLAYERS[ p2 ].name_html );
	sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
	AS_OUT( OE_PLAIN, htmlbuf );
}

void output_match_html( void )
{
	sprintf( htmlbuf, "<br />\n" );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( htmlbuf, HTML_GAME_INIT );
	AS_OUT( OE_PLAIN, htmlbuf );

	outbuf[ 0 ] = '\0';
	// output MAP and Gametype
	if ( strlen( map ) > 0 ) {	
		sprintf( outbuf, "%s ", map ); 
	}

	if ( query_gametype() != GT_UNKNOWN ) {
		strcat( outbuf, query_gametype_string() );
	}

	if ( strlen( outbuf ) > 0 ) {
		convert_plaintext_to_html( outbuf );
		sprintf( htmlbuf, HTML_GAME_LINE, outbuf );
		AS_OUT( OE_PLAIN, htmlbuf );
	}

	switch( query_gametype() ) {
		case GT_CAPTURE_THE_FLAG :
			output_match_ctf();
			break;
		case GT_CAPTURE_AND_HOLD :
			output_match_cap_and_hold();
			break;
		case GT_DUELL :
			output_match_duell();
			break;
	}

	sprintf( htmlbuf, HTML_GAME_TERM );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( htmlbuf, "<br />\n" );
	AS_OUT( OE_PLAIN, htmlbuf );

}

void output_result_ctf( void )
{
	char t1[ TEAM_SIZE ];
	char t2[ TEAM_SIZE ];

	if ( active_teams == 2 ) {
		strcpy( t1, display_team_name( sorted_team_nr( 0 ) ) );
		strcpy( t2, display_team_name( sorted_team_nr( 1 ) ) );
	
		if ( TEAMS[ sorted_team_nr( 0 ) ].flags_captured == TEAMS[ sorted_team_nr( 1 ) ].flags_captured ) {
			sprintf( outbuf, RESULT_CTF_DRAW, t1, t2, TEAMS[ sorted_team_nr( 0 ) ].flags_captured );
			//sprintf( htmlbuf, HTML_RESULT_LINE, outbuf );
		}
		else {
			if ( TEAMS[ sorted_team_nr( 0 ) ].flags_captured  > TEAMS[ sorted_team_nr( 1 ) ].flags_captured ) {
				sprintf( outbuf, RESULT_CTF_WIN, t1, t2, TEAMS[ sorted_team_nr( 0 ) ].flags_captured , TEAMS[ sorted_team_nr( 1 ) ].flags_captured );
				//sprintf( htmlbuf, HTML_RESULT_LINE, outbuf );
			}
			else {
				sprintf( outbuf, RESULT_CTF_WIN, t2, t1, TEAMS[ sorted_team_nr( 1 ) ].flags_captured , TEAMS[ sorted_team_nr( 0 ) ].flags_captured );
				//sprintf( htmlbuf, HTML_RESULT_LINE, outbuf );
			}
		}
		if ( strlen( map_description ) > 0 ) {
			strcat( outbuf, " on " );
			strcat( outbuf, map_description );
		}
		sprintf( htmlbuf, HTML_RESULT_LINE, outbuf );
		AS_OUT( OE_PLAIN, htmlbuf );
	}
}

void output_result_cap_and_hold( void )
{
	char t1[ TEAM_SIZE ];
	char t2[ TEAM_SIZE ];
	int t, active_teams;

	active_teams = 0;
	for ( t = 0; t < team_count; t++ ) {
		if ( team_participated( t ) ) {
			active_teams++;
		}
	}

	if ( active_teams == 2 ) {
		strcpy( t1, display_team_name( sorted_team_nr( 0 ) ) );
		strcpy( t2, display_team_name( sorted_team_nr( 1 ) ) );
		sprintf( outbuf, "%s and %s played Capture And Hold", t1, t2, TEAMS[ sorted_team_nr( 0 ) ].name_html  );

		if ( strlen( map ) > 0 ) {
			strcat( outbuf, " on " );
			strcat( outbuf, map );
		}
		sprintf( htmlbuf, HTML_RESULT_LINE, outbuf );
		AS_OUT( OE_PLAIN, htmlbuf );
	}

}

void output_result_duell( void )
{

}
void output_result( void )
{
	// output Clan A beat the crap out of Clan B

	sprintf( htmlbuf, HTML_RESULT_INIT );
	AS_OUT( OE_PLAIN, htmlbuf );
	sprintf( htmlbuf, "<br />\n" );
	AS_OUT( OE_PLAIN, htmlbuf );

	switch( query_gametype() ) {
		case GT_CAPTURE_THE_FLAG :
			output_result_ctf();
			break;
		case GT_CAPTURE_AND_HOLD :
			output_result_cap_and_hold();
			break;
		case GT_DUELL :
			output_result_duell();
			break;
	}
	sprintf( htmlbuf, HTML_RESULT_TERM );
	AS_OUT( OE_PLAIN, htmlbuf );
}

// the prima donnas 
void output_touches_and_captures( void )
{
	int p;
	void *player_index_org;
	void *player_index_caps;
	void *player_index_touches;
	int  output_this_table = FALSE;
	int  player_capture, player_touch; // player that captured, player that touched
	char output_captures[20], output_touches[20];
	long captures, touches;

	player_index_org = push_player_index();

	sort_players( BY_TOUCHES_BY_CAPTURES );
	if ( PLAYERS[ player_index[ 0 ] ].flagtouches > 0 ) {
		output_this_table = TRUE;
	}
	player_index_touches = push_player_index();

	sort_players( BY_CAPTURES_BY_TOUCHES );
	if ( PLAYERS[ player_index[ 0 ] ].flags_captured > 0 ) {
		output_this_table = TRUE;
	}
	player_index_caps = push_player_index();

	if ( output_this_table ) {
		AS_OUT( OE_PLAIN, "<br />\n" );
		sprintf( htmlbuf, HTML_TOUCHES_CAPS_INIT );
		AS_OUT( OE_PLAIN, htmlbuf );

		sprintf( htmlbuf, HTML_TOUCHES_CAPS_HEADER, "Flagtouches", "Captures" );
		AS_OUT( OE_PLAIN, htmlbuf );


		for ( p = 0; p < player_count; p++ ) {
			output_captures[ 0 ] = '\0';
			output_touches [ 0 ] = '\0';
		
			player_capture = ((int*)player_index_caps)[ p ];
			player_touch   = ((int*)player_index_touches)[ p ];

			captures = PLAYERS[ player_capture ].flags_captured;
			touches  = PLAYERS[ player_touch ].flagtouches;

			if ( touches > 0 ) {
				sprintf( output_touches, "%ld", touches );
			
				if ( captures > 0 ) { // both touches and captures
					sprintf( output_captures, "%ld", captures );

					sprintf( htmlbuf, HTML_TOUCHES_CAPS_LINE, 
					html_player_clickable( player_touch ), output_touches,
					html_player_clickable( player_capture ), output_captures );
					AS_OUT( OE_PLAIN, htmlbuf );
				}
				else { // touches but no captures
					sprintf( htmlbuf, HTML_TOUCHES_CAPS_LINE, 
					html_player_clickable( player_touch ), output_touches,
					"", "" );
					AS_OUT( OE_PLAIN, htmlbuf );
				}
			}
			else {
				if ( captures > 0 ) {
					// captures but no touches
					sprintf( output_captures, "%ld", captures );

					sprintf( htmlbuf, HTML_TOUCHES_CAPS_LINE, 
					"", "",
					html_player_clickable( player_capture ), output_captures );
					AS_OUT( OE_PLAIN, htmlbuf );
				}
				else {
					// we are done
					break;
				}
			}
		}
		sprintf( htmlbuf, HTML_TOUCHES_CAPS_TERM );
		AS_OUT( OE_PLAIN, htmlbuf );
	}

	pop_player_index( player_index_caps );
	pop_player_index( player_index_touches );
	pop_player_index( player_index_org );
}

void output_teamscore_history( void )
{
	// Capture: score team1..teamn, capping player
// something like this
//	 AGO    Chosen   Player scoring
//
//    0        0     AGO Pook
//    1        0     AGO dub
//    2        0     AGO black
//    2        1     Chosen bla

// number of columns needed is nr of teams + 1 for the scoring players
	int t, st;
	int ts_events; // teamscore events
	int teamscores[ MAX_TEAMS ];
	int scoring_team;

	if ( query_gametype() != GT_CAPTURE_THE_FLAG ) {
		return;
	}

	sprintf( htmlbuf, "<br />\n" );
	AS_OUT( OE_PLAIN, htmlbuf );

	sprintf( outbuf, HTML_TEAMSCOREHISTORY_INIT );
	AS_OUT( OE_PLAIN, outbuf );
	
	// column headers
	sprintf( outbuf, "<tr>" );
	AS_OUT( OE_PLAIN, outbuf );
	for ( t = 0; t < team_count; t++ ) {
		st = sorted_team_nr(t);
		//st = team_index [ t ];
		if ( team_participated( st ) ) {
			if ( strlen( TEAMS[ st ].clantag_html ) > 0 ) {
				sprintf( outbuf, HTML_TH_TEAMSCOREHISTORY_TEAM, TEAMS[ st ].clantag_html );
			}
			else {
				sprintf( outbuf, HTML_TH_TEAMSCOREHISTORY_TEAM, TEAMS[ st ].name_html );
			}
			AS_OUT( OE_PLAIN, outbuf );
			teamscores[ st ] = 0;
		}
	}
	sprintf( outbuf, HTML_TH_TEAMSCOREHISTORY_PLAYER, "Player" );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, HTML_TH_TEAMSCOREHISTORY_TOUCHES, "Flagtouches" );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, "</tr>\n" );
	AS_OUT( OE_PLAIN, outbuf );
	
	// output rows
	for ( ts_events = 0; ts_events < teamscore_event_count; ts_events++ ) {
		sprintf( outbuf, "<tr>" );
		AS_OUT( OE_PLAIN, outbuf );

		// output teamscore columns
		scoring_team = teamscore_history[ ts_events ].scoring_team;
		if ( scoring_team >= 0 && scoring_team < MAX_TEAMS ) {
			teamscores[ scoring_team ]++;
		}
		for ( t = 0; t < team_count; t++ ) {
			st = team_index [ t ];
			if ( team_participated( st ) ) {
				sprintf( outbuf, HTML_TD_TEAMSCOREHISTORY_SCORE, teamscores[ st ] );
				AS_OUT( OE_PLAIN, outbuf );
			}
		}

		// output scoring player
		sprintf( outbuf, HTML_TD_TEAMSCOREHISTORY_PLAYER, html_player_clickable( teamscore_history[ ts_events ].scoring_player ) );
		AS_OUT( OE_PLAIN, outbuf );
		
		sprintf( outbuf, HTML_TD_TEAMSCOREHISTORY_TOUCHES, teamscore_history[ ts_events ].flagtouches );
		AS_OUT( OE_PLAIN, outbuf );

		// output number of flagtouches
		if ( 1 == teamscore_history[ ts_events ].flagtouches ) {
			// coast to coast capture
			sprintf( outbuf, HTML_TD_TEAMSCOREHISTORY_C2C, "(Coast to coast)" );
			AS_OUT( OE_PLAIN, outbuf );
		}
		else {
			sprintf( outbuf, "" );
			AS_OUT( OE_PLAIN, outbuf );
		}
		
		sprintf( outbuf, "</tr>\n" );
		AS_OUT( OE_PLAIN, outbuf );
	}
	
	sprintf( outbuf, HTML_TEAMSCOREHISTORY_TERM );
	AS_OUT( OE_PLAIN, outbuf );
}

void output_flagaction( void )
{
 // todo:
}

void output_general_scores( void )
{
	output_result();
	//output_match_html();
	output_touches_and_captures();
	output_teamscore_history();
	output_teams_html();
	
}

void output_playerlist_html( OBJ_MESSAGE msg, int player_nr )
{
	static int initialized = FALSE;

	switch (msg) {
		case OBJ_INIT:
			if (!initialized) {
				sprintf( outbuf, "<br />\n" );
				AS_OUT( OE_PLAIN, outbuf );
				sprintf( outbuf, HTML_PLAYERLIST_INIT );
				AS_OUT( OE_PLAIN, outbuf );

				sprintf( outbuf, HTML_PLAYERLIST_HEADER, 
					"Role",
					"Player", 
					"Frags", 
					"Deaths", 
					"Teamkills", 
					"Teamdeaths",
					"Suicides", 
					"Flagtouches", 
					"Captures" );
				AS_OUT( OE_PLAIN, outbuf );

				initialized = TRUE;
			}
			break;

		case OBJ_PROCESS:
			if (!initialized) {
				output_playerlist_html( OBJ_INIT,0 );
			}
			// takes 8 string parameters
			if ( PLAYERS[ player_nr ].squad != SQUAD_OFF ) {
				sprintf( outbuf, HTML_PLAYERLIST_LINE, 
						//PLAYERS[ player_nr ].name_html, 
						
						css_team_background_color( PLAYERS[ player_nr ].team ),
						//TEAMS [ PLAYERS[ player_nr ].team ].name, 
						query_squadname( player_nr ),
						html_player_clickable( player_nr ),
						PLAYERS[ player_nr ].frags, 
						PLAYERS[ player_nr ].deaths, 
						PLAYERS[ player_nr ].teamkills, 
						PLAYERS[ player_nr ].teamdeaths,
						PLAYERS[ player_nr ].suicides, 
						PLAYERS[ player_nr ].flagtouches, 
						PLAYERS[ player_nr ].flags_captured  );
			}
			else {
				sprintf( outbuf, HTML_PLAYERLIST_LINE_OFF, 
						//PLAYERS[ player_nr ].name_html, 
							
						css_team_background_color( PLAYERS[ player_nr ].team ),
						//TEAMS [ PLAYERS[ player_nr ].team ].name, 
						query_squadname( player_nr ),
						html_player_clickable( player_nr ),
						PLAYERS[ player_nr ].frags, 
						PLAYERS[ player_nr ].deaths, 
						PLAYERS[ player_nr ].teamkills, 
						PLAYERS[ player_nr ].teamdeaths,
						PLAYERS[ player_nr ].suicides, 
						PLAYERS[ player_nr ].flagtouches, 
						PLAYERS[ player_nr ].flags_captured   );
			}

			AS_OUT( OE_PLAIN, outbuf );
			break;

		case OBJ_TERM:
			if (initialized) {
				sprintf( outbuf, HTML_PLAYERLIST_TERM );
				AS_OUT( OE_PLAIN, outbuf );
				initialized = FALSE;
			}
			break;
	}
}

void output_individual_scores( void )
{
	int i;
	
	//sort_players( BY_TEAMSCORE_DESC_BY_TEAM_BY_SQUAD_BY_FLAGTOUCHES_BY_CAPTURES_BY_NAMECLEAN );

	for (i = 0 ; i < player_count; i++ ) {
		output_playerlist_html( OBJ_PROCESS, player_index[ i ] );
	}
	output_playerlist_html( OBJ_TERM, 0 );
}

void output_fragmap_explanation( void )
{
	sprintf( outbuf, HTML_FRAGMAP_HINT );
	AS_OUT( OE_PLAIN, outbuf );

	switch( query_gametype() ) {
		
		// explanation of colored frags
		case GT_CAPTURE_THE_FLAG :
			sprintf( outbuf, HTML_PVP_LEGEND_INIT );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "TEAMKILL", 7, "teamkills and suicides" );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "FRAG", 16, "kills between offence and defence players" );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "CHASE", 2, "kills between offence players" );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "DEF_ATTACK", 3, "kills between defence players" );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_TERM );
			AS_OUT( OE_PLAIN, outbuf );
			break;

		case GT_CAPTURE_AND_HOLD :
			sprintf( outbuf, HTML_PVP_LEGEND_INIT );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "TEAMKILL", 7, "teamkills and suicides" );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "FRAG", 16, "kills between enemies" );
			AS_OUT( OE_PLAIN, outbuf );
			
			sprintf( outbuf, HTML_PVP_LEGEND_TERM );
			AS_OUT( OE_PLAIN, outbuf );
			break;

		case GT_DUELL :
			sprintf( outbuf, HTML_PVP_LEGEND_INIT );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "TEAMKILL", 7, "teamkills and suicides" );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_PVP_LEGEND_LINE, "FRAG", 16, "kills between enemies" );
			AS_OUT( OE_PLAIN, outbuf );
			
			sprintf( outbuf, HTML_PVP_LEGEND_TERM );
			AS_OUT( OE_PLAIN, outbuf );
			break;
	}
}

int get_number_length( long number )
{
	// calculate how many positions are required to print a number 
	// 100 = 3
	//  10 = 2 
	//   1 = 1
	int length = 1;
	while ( number >= 10 ) {
		number = number / 10;
		length = length + 1;
	}
	return length;

}
void output_pvp_frags_per_weapon( int killer, int victim, char *sinfo )
{
	int w;
	long weapon_frags;
	long weapon_frags_carrier;
	long *frags_per_weapon;
	long max_frags;
	int length, max_length, frags_length;
	
	frags_per_weapon = &(frag_matrix[ killer ][ victim ].frags_per_weapon[0]);
	sort_weapons( frags_per_weapon , weapon_index1 );

	sprintf( outbuf, HTML_FRAGMAP_CELL_POPUP_INIT );
	AS_OUT( OE_PLAIN, outbuf );
	sprintf( outbuf, "%s<br />", sinfo );
	AS_OUT( OE_PLAIN, outbuf );
	
	// determine how much space is required nicely print the weapons and frags
	// max_length for weaponname and frags_length for fragnumber
	max_frags = 0;
	max_length = 0;
	for ( w = 0; w < W_NR_WEAPONS; w++ ) {
		weapon_frags = frag_matrix[ killer ][ victim ].frags_per_weapon[ weapon_index1[ w ] ];
		if ( weapon_frags > 0 ) {
			length = strlen( weapon_name( weapon_index1[ w ] ) );
	
			if ( length > max_length ) {
				// weaponname
				max_length = length;
			}

			if ( weapon_frags > max_frags ) {
				// fragcount for weapon
				max_frags = weapon_frags;
			}
		}
	}
	frags_length = get_number_length( max_frags );

	for ( w = 0; w < W_NR_WEAPONS; w++ ) {
		weapon_frags = frag_matrix[ killer ][ victim ].frags_per_weapon[ weapon_index1[ w ] ];
		weapon_frags_carrier = frag_matrix[ killer ][ victim ].frags_on_flagcarrier_per_weapon [ weapon_index1[ w ] ];
		if ( weapon_frags > 0 ) {
			// 25 characters so far is the longest weapon name "crushed by supply station"
			sprintf( outbuf, "- %-*.*s : %*ld", max_length, max_length, weapon_name( weapon_index1[ w ] ), frags_length, weapon_frags );
			AS_OUT( OE_PLAIN, outbuf );
			switch( weapon_frags_carrier ) {
				case 0:
					break;
				case 1:
					sprintf( outbuf, " (carrying flag %ld time)", weapon_frags_carrier );
					AS_OUT( OE_PLAIN, outbuf );
					break;
				default:
					sprintf( outbuf, " (carrying flag %ld times)", weapon_frags_carrier );
					AS_OUT( OE_PLAIN, outbuf );
			}

			sprintf( outbuf, "<br />" ); // because of white-space:pre; we leave out the \n
			AS_OUT( OE_PLAIN, outbuf );
		}
	}

	sprintf( outbuf, HTML_FRAGMAP_CELL_POPUP_TERM );
	AS_OUT( OE_PLAIN, outbuf );
}

void output_fragmap_html( OBJ_MESSAGE msg, TABLE_ITEM item, char *sinfo, long linfo, long linfo2, int p1, int p2 )
{
	static int initialized = FALSE;

	switch (msg) {
		case OBJ_INIT:
			if (!initialized) {
				//sort_players( BY_TEAMSCORE_DESC_BY_TEAM_BY_SQUAD_BY_FLAGTOUCHES_BY_CAPTURES_BY_NAMECLEAN );
				sprintf( outbuf, HTML_FRAGMAP_GROUP_INIT );
				AS_OUT( OE_PLAIN, outbuf );
				sprintf( outbuf, HTML_FRAGMAP_INIT );
				AS_OUT( OE_PLAIN, outbuf );
				initialized = TRUE;
			}
			break;

		case OBJ_PROCESS:
			if (!initialized) {
				output_fragmap_html( OBJ_INIT, TI_UNDEFINED, NULL, 0, 0, 0, 0 );
			}
			switch( item ) {

				case TI_ROW_INIT:
					sprintf( outbuf, HTML_FRAGMAP_ROW_INIT );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_ROW_TERM:
					sprintf( outbuf, HTML_FRAGMAP_ROW_TERM );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_TOPLEFT:
					sprintf( outbuf, HTML_FRAGMAP_CELL_TOPLEFT, linfo, sinfo );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_PLAYER:
					// in params sinfo = empty, linfo = volgnr, linfo2 = player
					// volgnr, red/blue, squad, playername
					sprintf( outbuf, 
						HTML_FRAGMAP_CELL_PLAYER, linfo, 
						css_team_background_color( PLAYERS[ linfo2 ].team ),
						//TEAMS [PLAYERS[ linfo2 ].team ].name, 
						query_squadname( linfo2 ), 
						html_player_clickable( linfo2 ) );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_STRING:
					sprintf( outbuf, HTML_FRAGMAP_CELL_STRING, sinfo );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_LONG:
					if ( linfo != 0 ) {
						sprintf( outbuf, HTML_FRAGMAP_CELL_LONG, linfo );
					}
					else {
						// check in internet explorer, it doesn't draw empty cells
						sprintf( outbuf, HTML_FRAGMAP_CELL_STRING, "" );
					}
					AS_OUT( OE_PLAIN, outbuf );
					break;
					
				case TI_CELL_LONG_TK:
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_TK_INIT, linfo );
					AS_OUT( OE_PLAIN, outbuf );
					output_pvp_frags_per_weapon( p1, p2, sinfo );
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_TK_TERM );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_LONG_SUICIDE:
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_SUICIDE_INIT, linfo );
					AS_OUT( OE_PLAIN, outbuf );
					output_pvp_frags_per_weapon( p1, p2, sinfo );
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_SUICIDE_TERM );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_LONG_GREEN:
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_FRAG_INIT, linfo );
					AS_OUT( OE_PLAIN, outbuf );
					output_pvp_frags_per_weapon( p1, p2, sinfo );
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_FRAG_TERM );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_LONG_YELLOW:
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_CHASE_INIT, linfo );
					AS_OUT( OE_PLAIN, outbuf );
					output_pvp_frags_per_weapon( p1, p2, sinfo );
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_CHASE_TERM );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_LONG_WHITE:
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_DEF_ATTACK_INIT, linfo );
					AS_OUT( OE_PLAIN, outbuf );
					output_pvp_frags_per_weapon( p1, p2, sinfo );
					sprintf( outbuf, HTML_FRAGMAP_CELL_LONG_DEF_ATTACK_TERM );
					AS_OUT( OE_PLAIN, outbuf );
					break;

				case TI_CELL_EMPTY :
					sprintf( outbuf, HTML_FRAGMAP_CELL_STRING, "" );
					AS_OUT( OE_PLAIN, outbuf );
					break;
			}
			break;

		case OBJ_TERM:
			if (initialized) {
				sprintf( outbuf, HTML_FRAGMAP_TERM );
				AS_OUT( OE_PLAIN, outbuf );
				
				output_fragmap_explanation();

				sprintf( outbuf, HTML_FRAGMAP_GROUP_TERM );
				AS_OUT( OE_PLAIN, outbuf );
				initialized = FALSE;
			}
			break;
	}

}

void build_fragcell_popupinfo( char* titlebuf, long frags, long sentries_destroyed, long supplies_destroyed )
{
	if ( sentries_destroyed > 0 || supplies_destroyed > 0 ) {
		strcat( titlebuf, "<br />" );
	}
	if ( sentries_destroyed > 0 ) {
		sprintf( outbuf, "Destroyed his sentrygun %ld times.<br />", sentries_destroyed );
		strcat( titlebuf, outbuf );
	}
	if ( supplies_destroyed > 0 ) {
		sprintf( outbuf, "Destroyed his supplystation %ld times.<br />", supplies_destroyed );
		strcat( titlebuf, outbuf );
	}
	if ( frags > 0 ) {
		strcat( titlebuf, "<br />Frags by weapon:<br />" );
	}
}

// k = killer, v = victim
// this function outputs a cell containing a number (or empty), specialized for each gametype
void output_fragmap_cell( int k, int v, long gametype )
{
	char titlebuf[ 200 ];
	char relation;
	long frags;
	long sentries_destroyed;
	long supplies_destroyed;

	frags = frag_matrix[ k ][ v ].frags;
	sentries_destroyed = frag_matrix[ k ][ v ].fragged_sentry;
	supplies_destroyed = frag_matrix[ k ][ v ].fragged_supplystation;

	if ( k == v ) {
		// killer == victim means suicide
		sprintf( titlebuf, "%s suicided %ld times:", 
				PLAYERS[ k ].nameclean_html,
				frags );

		output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_SUICIDE, titlebuf, frags , 0, k , v );
	}
	else {

		relation = frag_matrix[ k ][ v ].same_team;
		if ( relation != PLAYERS_DIFFERENT_TEAM && relation != PLAYERS_SAME_TEAM ) {
			relation = TEAM_RELATION_UNKNOWN; // shouldn't really happen if 2 players fragged each other
		}
		switch( gametype ) {

			case GT_CAPTURE_THE_FLAG:
				
				if ( frags == 0 && sentries_destroyed == 0 && supplies_destroyed == 0 ) {
					// there were no frags, draw empty cell
					output_fragmap_html( OBJ_PROCESS, TI_CELL_EMPTY, "", 0, 0, k , v );	
				}
				else {
					// there were frags
					if ( relation == PLAYERS_SAME_TEAM ) {
						// the frags where teamkills
						sprintf( titlebuf, "%s teamkilled %s %ld times.<br />", 
							PLAYERS[ k ].nameclean_html,	PLAYERS[ v ].nameclean_html, frags );
						
						build_fragcell_popupinfo( titlebuf, frags, sentries_destroyed, supplies_destroyed );

						output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_TK, titlebuf, frags, 0, k, v );
					}
					else {
						// the frags were kills on enemy team
						sprintf( titlebuf, "%s fragged %s %ld times.<br />", 
							PLAYERS[ k ].nameclean_html, PLAYERS[ v ].nameclean_html, frags );

						build_fragcell_popupinfo( titlebuf, frags, sentries_destroyed, supplies_destroyed );

						// here comes the interesting part, decide which color to print the frags

						if ( PLAYERS[ k ].squad == SQUAD_OFF && PLAYERS[ v ].squad == SQUAD_OFF ) {
							// def vs def
							output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_YELLOW, titlebuf, frags, 0, k, v );
						}
						else {
							if ( PLAYERS[ k ].squad == SQUAD_DEF && PLAYERS[ v ].squad == SQUAD_DEF ) {
								// off vs off
								output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_WHITE, titlebuf, frags, 0, k, v );
							}
							else {
								// the rest
								output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_GREEN, titlebuf, frags, 0, k, v );
							}	
						}
					}

				}
				break;

			case GT_CAPTURE_AND_HOLD:
				
				// teamkills
				if ( frags == 0 && sentries_destroyed == 0 && supplies_destroyed == 0 ) {
					// draw empty cell
					output_fragmap_html( OBJ_PROCESS, TI_CELL_EMPTY, "", 0, 0, k, v );	
				}
				else {

					if ( relation == PLAYERS_SAME_TEAM ) {
						sprintf( titlebuf, "%s teamkilled %s %ld times:", 
							PLAYERS[ k ].nameclean_html, PLAYERS[ v ].nameclean_html, frags );
						
						build_fragcell_popupinfo( titlebuf, frags, sentries_destroyed, supplies_destroyed );

						output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_TK, titlebuf, frags, 0, k, v );	
					} 
					else {
						sprintf( titlebuf, "%s fragged %s %ld times:", 
							PLAYERS[ k ].nameclean_html, PLAYERS[ v ].nameclean_html, frags );

						build_fragcell_popupinfo( titlebuf, frags, sentries_destroyed, supplies_destroyed );

						output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_GREEN, titlebuf, frags, 0, k, v );
					}

				}
				break;

			case GT_DUELL :
				
				if ( relation == PLAYERS_SAME_TEAM ) {
					sprintf( titlebuf, "%s teamkilled %s %ld times:", 
							PLAYERS[ k ].nameclean_html, PLAYERS[ v ].nameclean_html, frags );

						output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_TK, titlebuf, frags, 0, k, v );	
				} 
				else {
					sprintf( titlebuf, "%s fragged %s %ld times:", 
						PLAYERS[ k ].nameclean_html, PLAYERS[ v ].nameclean_html, frags );

					output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_GREEN, titlebuf, frags, 0, k, v );
				}
				break;


			default:
				// if gametype is unknown
				if ( frag_matrix[ k ][ v ].frags == 0 ) {
					// draw empty cell
					output_fragmap_html( OBJ_PROCESS, TI_CELL_EMPTY, "", 0, 0, k, v );	
				}
				else {

					if ( relation == PLAYERS_SAME_TEAM ) {
						sprintf( titlebuf, "%s teamkilled %s %ld times:", 
							PLAYERS[ k ].nameclean_html, PLAYERS[ v ].nameclean_html, frags );

						output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_TK, titlebuf, frags, 0, k, v );	
					} 
					else {
						sprintf( titlebuf, "%s fragged %s %ld times:", 
							PLAYERS[ k ].nameclean_html, PLAYERS[ v ].nameclean_html, frags );

						output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG_GREEN, titlebuf, frags, 0, k, v );
					}

				}
		}
	}
}

void output_fragmap( void )
{
	int i, j;
	int k, v;
	int nr  ; // number in row and column
	long gt; // gametype

	gt = query_gametype();

	sprintf( outbuf, "<br />\n<br />\n" );
	AS_OUT( OE_PLAIN, outbuf );

	output_fragmap_html( OBJ_PROCESS, TI_ROW_INIT, NULL, 0, 0, 0, 0 );
	output_fragmap_html( OBJ_PROCESS, TI_CELL_TOPLEFT, "", 0, 0, 0, 0 );
	nr = 1;
	for ( i = 0; i < player_count; i++ ) {		
		output_fragmap_html( OBJ_PROCESS, TI_CELL_LONG, NULL, nr, 0, 0, 0 );
		nr ++;
	}
	output_fragmap_html( OBJ_PROCESS, TI_ROW_TERM, NULL, 0, 0, 0, 0 );
	
	nr = 0;
	// for each row
	for ( i = 0; i < player_count ; i++ ) {
		
		nr ++;
		output_fragmap_html( OBJ_PROCESS, TI_ROW_INIT, NULL, 0, 0, 0, 0 );
		output_fragmap_html( OBJ_PROCESS, TI_CELL_PLAYER, "" , nr , player_index [ i ], 0, 0 );

		// print row
		for ( j = 0; j < player_count; j++ ) {
			
			k = player_index[ i ]; // killer
			v = player_index[ j ]; // victim

			output_fragmap_cell( k, v, gt );

		} // end for

		output_fragmap_html( OBJ_PROCESS, TI_ROW_TERM, NULL, 0, 0, 0, 0 );

	} // end for

	output_fragmap_html( OBJ_TERM, TI_UNDEFINED, NULL, 0, 0, 0, 0 );
}

void output_frags_deaths_per_weapon_html( OBJ_MESSAGE msg, char *sinfo1, long linfo1, char *sinfo2, long linfo2 )
{
	static int initialized = FALSE;
	char ltemp1[ 20 ]; 
	char ltemp2[ 20 ]; 

	switch (msg) {
		case OBJ_INIT:
			if (!initialized) {
				sprintf( outbuf, HTML_FD_BY_WEAPON_HEADER, "Frags by weapon", "Deaths by weapon" );
				AS_OUT( OE_PLAIN, outbuf );

				initialized = TRUE;
			}
			break;

		case OBJ_PROCESS:
			if (!initialized) {
				output_frags_deaths_per_weapon_html( OBJ_INIT, NULL, 0, NULL, 0 );
			}
			sprintf( ltemp1, "%ld", linfo1 );
			sprintf( ltemp2, "%ld", linfo2 );
			
			if (linfo1 == 0 ){
				strcpy( ltemp1, "" );
			}
			if (linfo2 == 0 ){
				strcpy( ltemp2, "" );
			}

			sprintf( outbuf, HTML_FD_BY_WEAPON_LINE, sinfo1, ltemp1, sinfo2, ltemp2 );
			AS_OUT( OE_PLAIN, outbuf );
			break;

		case OBJ_TERM:
			if (initialized) {
				initialized = FALSE;
			}
			break;
	}
}

// for each section ( debug, defence, offence, ... ) there has to be an init and a term
void output_html_playerinfo( OBJ_MESSAGE msg, char* info )
{
	static int initialized = FALSE;

	switch( msg ) {
		case OBJ_INIT:
			if ( !initialized ) {
				//sprintf( outbuf, HTML_PLAYER_INFO_INIT );
				//AS_OUT( OE_PLAIN, outbuf );
				initialized = TRUE;
			}
			break;
		case OBJ_PROCESS:
			if ( !initialized ) {
				output_html_playerinfo( OBJ_INIT, NULL );
			}
			AS_OUT( OE_PLAIN, info );
			break;
		case OBJ_TERM:
			if ( initialized ) {
				//sprintf( outbuf, HTML_PLAYER_INFO_TERM );
				//AS_OUT( OE_PLAIN, outbuf );
				initialized = FALSE;
			}
			break;
	}
}

void output_playerinfo_detailed( void)
{
	int i, wf, wd;
	int enemy;
	player_stats *p;
	double ratio;
	long frags_by_sentry;

	// sort same order as fragmatrix
	//sort_players( BY_TEAMSCORE_DESC_BY_TEAM_BY_SQUAD_BY_FLAGTOUCHES_BY_CAPTURES_BY_NAMECLEAN );
	
	sprintf( outbuf, "<br />\n<br />\n" );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, HTML_PLAYER_INIT );
	AS_OUT( OE_PLAIN, outbuf );
	

	// output debug, def, off, frags 'n deaths
	for ( i = 0; i < player_count ; i++ ) {
		//p = &sorted_player( i );
		p = &PLAYERS[ player_index[ i ] ];
	
		sprintf( outbuf, HTML_PLAYER_NAME, html_player_location( player_index [ i ] ) );
		AS_OUT( OE_PLAIN, outbuf );

		sprintf( outbuf, HTML_TEAM_COLOR_BAR, css_team_background_color( p->team ) );
		AS_OUT( OE_PLAIN, outbuf );

//#if defined(_DEBUG)
//		sprintf( htmlbuf, HTML_PLAYER_DEBUG );
//		AS_OUT( OE_PLAIN, htmlbuf );
//
//		sprintf( outbuf, "off rating:%ld", p->off_points );
//		
//		sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
//		output_html_playerinfo( OBJ_PROCESS, htmlbuf );
//		
//		sprintf( outbuf, "def rating:%ld", p->def_points );	
//		sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
//		output_html_playerinfo( OBJ_PROCESS, htmlbuf );
//	
//		sprintf( outbuf, "indirect off rating:%ld", p->off_points_indirect );
//		sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
//		output_html_playerinfo( OBJ_PROCESS, htmlbuf );
//				
//		sprintf( outbuf, "indirect def rating:%ld", p->def_points_indirect );
//		sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
//		output_html_playerinfo( OBJ_PROCESS, htmlbuf );
//
//		output_html_playerinfo( OBJ_TERM, NULL );
//#endif
		if ( query_output_defence() ) {
			// DEFENCE STUFF
			
			//sprintf( htmlbuf, HTML_PLAYER_DEF );
			AS_OUT( OE_PLAIN, HTML_PLAYER_DEF );
	//			output_html_playerinfo( OBJ_INIT, NULL );

			// teamkilled sentry
			if ( p->teamkilled_sentry > 0 ) {
				//sprintf( outbuf, "Teamkilled %ld autosentries", p->teamkilled_sentry );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Teamkilled autosentries", 4, p->teamkilled_sentry) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			
			// defended flag at base
			if ( p->defend_flag_at_base > 0 ) {
				//sprintf( outbuf, "Defended the flag at base %ld times", p->defend_flag_at_base );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Defended the flag at base", 4, p->defend_flag_at_base) ;
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}
				
			// defended flag in field
			if ( p->defend_flag_in_field > 0 ) {
				//sprintf( outbuf, "Defended the flag in the field %ld times", p->defend_flag_in_field );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Defended the flag in the field", 4, p->defend_flag_in_field) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}
			

			if ( p->killed_flagcarrier > 0 ) {
				//sprintf( outbuf, "Killed the flagcarrier %ld times", p->killed_flagcarrier );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Killed the flagcarrier", 4, p->killed_flagcarrier) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			output_html_playerinfo( OBJ_TERM, NULL );
		}
		
		if ( query_output_offence() ) {
			// OFFENCE Stuff
			sprintf( htmlbuf, HTML_PLAYER_OFF );
			AS_OUT( OE_PLAIN, htmlbuf );

			if ( p->flagtouches > 0 ) {
				//sprintf( outbuf, "%ld flagtouches", p->flagtouches );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Flagtouches", 4, p->flagtouches) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->first_touches > 0 ) {
				//sprintf( outbuf, "%ld first touches", p->first_touches  );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "First touches", 4, p->first_touches) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->flags_captured > 0 ) {
				//sprintf( outbuf, "%ld captures", p->flags_captured );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Captures", 4, p->flags_captured) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->coast_to_coast > 0 ) {
				//sprintf( outbuf, "%ld coast to coast captures", p->coast_to_coast );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Coast to coast captures", 4, p->coast_to_coast) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}
			
			if ( p->destroyed_sentry > 0 ) {
				//sprintf( outbuf, "Destroyed %ld autosentries", p->destroyed_sentry );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Autosentries destroyed", 4, p->destroyed_sentry) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->destroyed_supply > 0 ) {
				//sprintf( outbuf, "Destroyed %ld supply stations", p->destroyed_supply );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Supply stations destroyed", 4, p->destroyed_supply) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->killed_near_flag_in_base > 0 ) {
				//sprintf( outbuf, "Died %ld times near the flag in enemy base", p->killed_near_flag_in_base );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Died near flag in enemy base", 4, p->killed_near_flag_in_base) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->killed_near_flag_in_field > 0 ) {
				//sprintf( outbuf, "Died %ld times near the flag in the field", p->killed_near_flag_in_field );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Died near flag in the field", 4, p->killed_near_flag_in_field) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}
			
			if ( p->defend_flagcarrier > 0 ) {
				//sprintf( outbuf, "Defended the flagcarrier %ld times", p->defend_flagcarrier );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Defended the flagcarrier", 4, p->defend_flagcarrier) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->teamkilled_flagcarrier > 0 ) {
				//sprintf( outbuf, "Teamkilled the flagcarrier %ld times", p->teamkilled_flagcarrier );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Teamkilled the flagcarrier", 4, p->teamkilled_flagcarrier) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

//			output_html_playerinfo( OBJ_TERM, NULL );
		}

		if ( query_output_frags_and_deaths() ) {
			// FRAGS 'N DEATHS
			AS_OUT( OE_PLAIN, (void*)HTML_PLAYER_FND );

			// worst enemy ( frags + deaths )
			enemy = worst_enemy( player_index [ i ] );
			if ( enemy > -1 ) {
				//sprintf( outbuf, "Worst enemy: %s", html_player_clickable( enemy ) );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_STRING, "Worst enemy", html_player_clickable( enemy ) );
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			//sprintf( outbuf, "Biggest fragstreak: %ld", p->max_fragstreak );
			//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
			sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Biggest fragstreak", 4, p->max_fragstreak) ;
			output_html_playerinfo( OBJ_PROCESS, htmlbuf );

			if ( p->deaths > 0.0 ) {
				//sprintf( outbuf, "Frag/Death ratio: %.2lf", (double)p->frags/(double)p->deaths );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_DOUBLE, "Frag/Death ratio", 4, 2, (double)p->frags/(double)p->deaths) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
					
			}
			else {
				// todo: come up with something better
				//sprintf( outbuf, "Frag/Death ratio: This sucker has not been fragged" );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_STRING, "Frag/Death ratio", "No deaths") ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			frags_by_sentry = p->frags_per_weapon [ W_AUTOSENTRY_BULLET    ] +
								p->frags_per_weapon [ W_AUTOSENTRY_EXPLOSION ] +
								p->frags_per_weapon [ W_AUTOSENTRY_ROCKET    ] ;
			/*
			sentry_lifes = p->lost_sentry;
			if ( p->sentry_up ) { // this indicates if the sentry is currently up
				// this is not 100% secure, but the closest we can get
				sentry_lifes++;
			}
			*/
			// if this player had a sentry
			if ( frags_by_sentry > 0L || p->lost_sentry > 0L ) {
				//sprintf( outbuf, "Frags by autosentry: %ld", frags_by_sentry );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Frags by autosentry", 4, frags_by_sentry) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );

				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Autosentries lost", 4, p->lost_sentry) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			
				if ( p->lost_sentry != 0L ) {
					ratio = ( (double)frags_by_sentry / (double)p->lost_sentry );
					//sprintf( outbuf, "Sentries lost: %ld (frag/death ratio: %.2lf)", p->lost_sentry, ratio );
					//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Sentries lost", 4, p->lost_sentry) ;
					sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_DOUBLE, "Autosentry frag/death ratio", 4, 2, ratio) ;
				}
				else {
					//sprintf( outbuf, "Sentries lost: 0 (frag/death ratio: Nice :)" );
					sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_STRING, "Autosentry frag/death ratio", "No autosentries lost") ;
				}
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			if ( p->lost_supply > 0 ) {
				//sprintf( outbuf, "Lost %ld supply stations", p->lost_supply );
				//sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS, outbuf );
				sprintf( htmlbuf, HTML_PLAYER_INFO_PROCESS_LONG, "Supply stations lost", 4, p->lost_supply) ;
				output_html_playerinfo( OBJ_PROCESS, htmlbuf );
			}

			output_html_playerinfo( OBJ_TERM, NULL );

			// fnd weapon
			output_frags_deaths_per_weapon_html( OBJ_INIT, NULL, 0 , NULL, 0 );

			wf = 0;
			wd = 0;

			sort_weapons( &(p->frags_per_weapon [ wf ]) , weapon_index1 );
			sort_weapons( &(p->deaths_per_weapon[ wd ]) , weapon_index2 );

			while ( wf < W_NR_WEAPONS || wd < W_NR_WEAPONS ) {
				if ( wf < W_NR_WEAPONS ) {
					if ( p->frags_per_weapon[ weapon_index1[ wf ] ] == 0 ) {
						wf++;
					}
				}
				
				if ( wd < W_NR_WEAPONS ) {
					if ( p->deaths_per_weapon[ weapon_index2[ wd ] ] == 0 ) { 
						wd++;
					}
				}

				if ( wf < W_NR_WEAPONS ) {
					if ( wd < W_NR_WEAPONS ) {

						if ( p->frags_per_weapon[ weapon_index1[ wf ] ] > 0 && 
							p->deaths_per_weapon[ weapon_index2[ wd ] ] > 0  ) {

							output_frags_deaths_per_weapon_html( OBJ_PROCESS, 
									weapon_name( weapon_index1[ wf ] ),
									p->frags_per_weapon[ weapon_index1[ wf ]  ],
									weapon_name( weapon_index2[ wd ] ), 
									p->deaths_per_weapon[ weapon_index2[ wd ] ] );
							
							wf++;
							wd++;
						}

					}
					else {
						if ( p->frags_per_weapon[ weapon_index1[ wf ]  ] > 0 ) {
							
							output_frags_deaths_per_weapon_html( OBJ_PROCESS, 
									weapon_name( weapon_index1[ wf ] ),
									p->frags_per_weapon[ weapon_index1[ wf ] ],
									"", 
									0 );
						}
						wf++;
					}

				}
				else {
					if ( wd < W_NR_WEAPONS ) {
						if ( p->deaths_per_weapon[ weapon_index2[ wd ] ] > 0 ) {
							
							output_frags_deaths_per_weapon_html( OBJ_PROCESS, 
									"",
									0,
									weapon_name( weapon_index2[ wd ] ),
									p->deaths_per_weapon[ weapon_index2[ wd ] ]
									);
						}
						wd++;
					}
					else {
						// nothing to do
					}
				}	
			} // end while
			output_frags_deaths_per_weapon_html( OBJ_TERM, NULL, 0, NULL, 0 );
		}

	sprintf( outbuf, HTML_PLAYER_VOID );
	AS_OUT( OE_PLAIN, outbuf );

	} // end for

	sprintf( outbuf, HTML_PLAYER_TERM );
	AS_OUT( OE_PLAIN, outbuf );
	
}
void output_parent_table_init( void )
{
	sprintf( outbuf, HTML_PARENT_TABLE_INIT );
	AS_OUT( OE_PLAIN, outbuf );
}
void output_parent_table_term( void )
{
	sprintf( outbuf, HTML_PARENT_TABLE_TERM );
	AS_OUT( OE_PLAIN, outbuf );
}

void output_custom_table_sequence( void )
{
	int sequence_index = 0;
	int len;

	len = strlen( table_sequence );
	while ( sequence_index < len ) {
		switch( table_sequence[ sequence_index ] ) {

			case '1': output_result();
				break;

			case '2': output_touches_and_captures();
				break;

			case '3': output_teamscore_history();
				break;

			case '4': output_teams_html();
				break;

			case '5': output_individual_scores();
				break;

			case '6': output_fragmap();
				break;

			case '7': output_awards_v2();
				break;

			case '8': output_playerinfo_detailed();
				break;

			case '9': output_events();
				break;
		}

		sequence_index++;
	}
}

void post_process_stats( void )
{
	determine_teams();

	cleanup_stats(); // want to call this after determine teams

	determine_clantags_v2();
	post_process_teams(); // has to be called after clantags

	process_buildings_destroyed();
	
	complete_teamscore_history();

	calculate_team_statistics();
	calculate_teamscores(); // call this after calculate_team_statistics()
	calculate_match_statistics();
	calculate_indirect_points();

	determine_gametype_final(); // figure out if this was maybe a duell

	// calculate team/squad statistics
	fill_squad_info();
	sort_players( BY_TEAMSCORE_DESC_BY_TEAM_BY_SQUAD_BY_FLAGTOUCHES_BY_CAPTURES_BY_NAMECLEAN );
	attach_playerlist_to_teams();
	sort_teams( BY_SCORE );
}

void output_statistics( void )
{
	// write to file
	AS_OUT( OE_INIT, NULL );
	output_parent_table_init(); 

	if ( CUSTOM_TABLE_SEQUENCE ) {
		output_custom_table_sequence();
	}
	else {
		output_general_scores();
		output_individual_scores();
		output_fragmap();
		output_awards_v2();
		output_playerinfo_detailed();
		output_events();
	}

	sprintf( outbuf, "\n\n" );
	AS_OUT( OE_LINE, outbuf );
	output_parent_table_term();

	// close the outputfile
	AS_OUT( OE_TERM, NULL );
}

void update_linehistory( void )
{
	// save line variables of the current line that we may need in the future
	old_line_type  = line_type;
	old_line_info1 = LOG_PATTERNS[ matched_line ].info1;
	old_player_nr  = player_nr;
	old_flag_nr    = flag_nr;
}

int process_line( void )
{
	// match the line against all patterns
	int matched = FALSE;
	int i;

	update_linehistory(); // copy some info of the current line to history

	i=0;
	while ( !matched && ( LOG_PATTERNS[ i ].pat[0] != '\0' ) ) {
		clear_line_info();
		matched = match( linebuffer, LOG_PATTERNS[ i ].pat );
		i++;
	}

	if ( matched ) {
		matched_line = i - 1;
		post_process();
		update_statistics();
	}

	return matched;	
}

void process_game_begin( void )
{
	reset_statistics(); // todo: test
	//printf("process game begin -> %s", linebuffer);
	if ( game_count > 1L && FILE_OFFSET == TRUE ) {
		// if a file offset was used, then only process 1 game
		stop_processing = TRUE;
	}
	//if ( ONLY_LAST_GAME ) { // 2007-07-10
	//	reset_highscores( global_highscores ); // only base highscores on current highscores in file and the awards of the last game
	//}
}

void process_game_end( void )
{
	if ( searching_last_game ) {
		return;
	}

#if defined(SERVER_STATS)
	post_process_server_stats();
#endif
	
	post_process_stats();

	if ( MATCH.frags > 0 || MATCH.flagtouches > 0 || MATCH.suicides > 0 ) {
		game_count++;
		output_statistics();
		
		// EXPERIMENTAL
		if ( KEEP_INDIVIDUAL_HIGHSCORES == TRUE ) {
			update_player_personal_highscores();
		}

		if ( FILE_OFFSET == TRUE ) {
			// if a file offset was used, then only process 1 game
			stop_processing = TRUE;
		}
	}
}

void find_last_game( void )
{
	if ( ONLY_LAST_GAME ) {
		searching_last_game = TRUE;
		// parse the whole logfile to find the last game
		file_position_game_begin = 0;

		file_position = 0; // contains fileposition before current line is read
		while ( !stop_processing && NULL != fgets( linebuffer, LINE_BUFFER_SIZE, input ) ){
			process_line();
			file_position = ftell( input );
		}

		logfile_offset = file_position_game_begin;
		FILE_OFFSET = TRUE;

		searching_last_game = FALSE;		
	}
}

void process_logfile( char *filename, char *filename2 )
{
	long foundcounter = 0;
	int res;

	player_count = 0;

	if ( USE_STDIN ) {
		input = stdin;
	}
	else {
		input = fopen( filename, "rb" );
		if ( input == NULL && filename2 != NULL ) {
			input = fopen( filename2, "rb" );
		}
	}

	if (input == NULL) {		
		printf( "File not found : %s\n", filename );
		return;
	}

#if defined (_DEBUG)
	debugout = fopen( "parsed.txt", "w" );
#endif
	events_init();
	
	find_last_game();

	// start at offset
	if ( logfile_offset > 0 ) {
		res = fseek( input, logfile_offset, SEEK_SET );
		if ( res != 0 ) {
			printf( "Failed to set offset %ld.", logfile_offset );
			return;
		}
	}

	end_of_file = FALSE;
	reset_statistics();
	
	read_highscores( HIGHSCORE_FILENAME, global_highscores );
	// EXPERIMENTAL
	if ( KEEP_INDIVIDUAL_HIGHSCORES == TRUE ) {
		load_individual_highscores(); // todo: test
	}
	game_status	= GS_UNKNOWN;
	
	file_position = 0; // contains fileposition before current line is read
	while ( !stop_processing && NULL != fgets( linebuffer, LINE_BUFFER_SIZE, input ) ){
		if ( process_line() ) {
			// line is recognized
			foundcounter++;
		}
		else {
#if defined(_DEBUG)
			fprintf( debugout, linebuffer );
#endif
		}
		linecounter++;
		file_position = ftell( input );
	}
	
	// end of file is reached
	end_of_file = TRUE;

	// finish up current game
	switch( game_status ) {

		case GS_LIVE:
			process_game_end();
			break;
		case GS_POST_MATCH:
			process_game_end();
			break;
	}

	store_highscores( HIGHSCORE_FILENAME, global_highscores );
	output_highscores();

	clear_events();

	if ( !USE_STDIN ) {
		fclose( input );
	}

#if defined (_DEBUG)
	fclose( debugout );
#endif
}


void output_helpscreen( void )
{
	printf( "usage: agostats [options]\n", PROGRAM_NAME );
	printf( "the following options are available:\n" );
	printf( "-h -?        : this info\n" );

	printf( "-a           : keep individual player highscores in agostats_data directory\n" );
	printf( "             : open the file agostats_data\\players.html\n" );
	printf( "-f<options>  : customize output filename\n" );
	printf( "             : special options t(timestamp),n(mapname),r(teamscores),\n" );
	printf( "             : c(filename passed with -o parameter)\n" );
	printf( "             : combine options for example -ftnr, see agostats.txt for examples\n" );
	printf( "-o<filename> : custom output filename, use in combination with -fc parameter\n" );
	
	//  obsolete options as of version 0.63
	//	printf( "-n    : show mapname in filename\n" );
	//	printf( "-s    : show teamscores in filename\n" );
	//	printf( "-d    : no datestamp in output filename\n" );

	printf( "-i<logfile>  : filename of logfile\n" );
	printf( "-l           : only do stats for last (complete) game in logfile\n" );
	
	printf( "-p<offset>   : start processing from fileposition <offset>\n" );
	printf( "-r           : take input from stdin, sent output to stdout\n" );

	printf( "-t<tablenumbers> : -t123456789 is the default table order.\n" );
	printf( "                 : Table 9 is only displayed if option -m or -e is used.\n" );
	printf( "-e           : show events (capture, connect, kick, etc)\n" );
	printf( "-m           : show global messages (mm1)\n" );

	printf( "-v           : display programname and version\n" );
	printf( "\nSee agostats.txt for more explanation and examples.\n");
	
	//printf( "-ft     : 20060204-2119.htm\n" );
	//printf( "-fn     : etf_hardcore.htm\n" );
	//printf( "-fr     : BLUE-6-RED-1.htm\n" );
	//printf( "-ft-n-r : 20060204-2119-eft_hardcore-BLUE-6-RED-1.htm\n" );
	//printf( "-ft+n+r : 20060204-2119+eft_hardcore+BLUE-6-RED-1.htm\n" );
	//printf( "-ostats -fct  : stats20060204-2119.htm\n" );
	//printf( "-ostats -fc-t : stats-20060204-2119.htm\n" );
	
}

int process_parameters( int argc, char *argv[] )
{
	int i;
	int option_recognized = TRUE;
	int process_logfile   = TRUE;
	char temp[ 20 ];

	for ( i = 1; i < argc ; i++ ) { 
		// skip the first argument because that's the complete pathname
		option_recognized = FALSE;

		if ( 0 == strcmp( argv[ i ], "-a" ) ) { // per player highscores
			option_recognized = TRUE;
			KEEP_INDIVIDUAL_HIGHSCORES = TRUE;
		}

		if ( 0 == strcmp( argv[ i ], "-e" ) ) { // output events
			option_recognized = TRUE;
			OUTPUT_EVENTS = TRUE;
		}
		
		if ( 0 == strncmp( argv[ i ], "-f", 2 )) { // customized filename
			if ( strlen( argv[ i ] ) > 2 ) {
				strcpy( filename_options, argv[ i ]+2 );
				CUSTOMIZE_FILENAME = TRUE;
			}
			option_recognized = TRUE;
		}

		if ( 0 == strcmp( argv[ i ], "-h" ) || // show helpscreen
			 0 == strcmp( argv[ i ], "-?" ) ) {
			output_helpscreen();
			process_logfile = FALSE;
			option_recognized = TRUE;
		}
		
		if ( 0 == strncmp( argv[ i ], "-i", 2 )) { // determine inputfilename -ispaz.log
			if ( strlen( argv[ i ] ) > 2 ) {
				strcpy( inputfile, argv[ i ]+2 );
			}
			option_recognized = TRUE;
		}

		if ( 0 == strcmp( argv[ i ], "-l" ) ) { // only output last game
			option_recognized = TRUE;
			ONLY_LAST_GAME = TRUE;
		}
		
		if ( 0 == strcmp( argv[ i ], "-m" ) ) { // output messagemode1
			option_recognized = TRUE;
			OUTPUT_MM1 = TRUE;
		}
		
		if ( 0 == strncmp( argv[ i ], "-p", 2 ) ) { // set file offset
			option_recognized = TRUE;
			if ( strlen( argv[ i ] ) > 2 ) {
				strncopy( temp, argv[ i ]+2, sizeof( temp ) - 1 );
				logfile_offset = atol( temp );
				FILE_OFFSET = TRUE;
			}
		}

		if ( 0 == strcmp( argv[ i ], "-r" ) ) { // use stdin to read input
			option_recognized = TRUE;
			USE_STDIN  = TRUE;
			USE_STDOUT = TRUE;
		}

		if ( 0 == strncmp( argv[ i ], "-t", 2 )) { // change table sequence
			if ( strlen( argv[ i ] ) > 2 ) {
				strcpy( table_sequence, argv[ i ]+2 );
				CUSTOM_TABLE_SEQUENCE = TRUE;
			}
			option_recognized = TRUE;
		}

		if ( 0 == strncmp( argv[ i ], "-o", 2 ) ) { // set outputfile
			option_recognized = TRUE;
			if ( strlen( argv[ i ] ) > 2 ) {
				strcpy( custom_outputfile, argv[ i ]+2 );
				CUSTOM_OUTPUTFILE = TRUE;
			}
		}

		if ( 0 == strncmp( argv[ i ], "-v", 2 ) ) { // output version
			//process_logfile = FALSE;
			option_recognized = TRUE;
			printf("%s %s\n", PROGRAM_NAME, VERSION );
		}

		if ( !option_recognized ) { // unrecognized option, display help
			output_helpscreen();
			process_logfile = FALSE;
		}

	}
	return process_logfile;
}

void init_date( void )
{
    time_t t = time(0);
    strftime( current_date, sizeof( current_date ),"%Y-%m-%d", localtime( &t ) );
}

void initialize_defaults()
{
	USE_STDIN					= FALSE;
	OUTPUT_MM1					= FALSE;  // messagemode1 messages, default off
    OUTPUT_EVENTS				= FALSE;  // captures, disconnected, renamed etc. default off
	USE_STDOUT					= FALSE;
	CUSTOM_OUTPUTFILE			= FALSE;
	FILE_OFFSET					= FALSE;
	ONLY_LAST_GAME				= FALSE;
	CUSTOM_TABLE_SEQUENCE		= FALSE;
	CUSTOMIZE_FILENAME			= FALSE;
	KEEP_INDIVIDUAL_HIGHSCORES	= FALSE;
	game_count					= 0L;
	stop_processing				= FALSE;
	writing_highscores			= FALSE;
	init_date();
}

int main( int argc, char* argv[] )
{
	initialize_defaults();

	if ( process_parameters( argc, argv ) ) {

#if defined (_DEBUG)
		if ( USE_STDIN ) {
			sprintf( outbuf, "Input from STDIN\n" );
		}
		else {
			sprintf( outbuf, "Input from file\n" );
		}
#endif	
		if ( 0 != strcmp( inputfile, "" ) ){
			process_logfile( inputfile, NULL );
		}
		else {
			process_logfile( DEFAULT_LOGFILE, SECONDARY_LOGFILE );
		}
		
	}

//	test_skiplist();

	return 0;
}
