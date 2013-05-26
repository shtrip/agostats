#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "agostats.h"
#include "highscores.h"
#include "events.h"
#include "game.h"
#include "tools.h"
#include "matching.h"
#include "output.h"
#include "game.h"
#include "players.h"
#include "teams.h"
#include "weapons.h"

// public variables
int				current_player;
int				player_count  ;
player_stats	PLAYERS[ MAX_PLAYERS ]; // player info storage
relation		frag_matrix [ MAX_PLAYERS ][ MAX_PLAYERS ];
int				player_index[ MAX_PLAYERS  ];// unique index, each element points to a player in PLAYERS[]

// private variables

// private functions
// strip all colorcodes from left until a normal character is encountered
// also rightstrip colorcodes
// in some messages max name length = 35 characters
// in some messages max name length = 29 characters
// left$(name, 29) can end with a ^
// a line always starts with a colorcode (added by player or quake)
// a name is always terminated with ^7 (added by quake)
// rightstrip ^7
// if a name doesn't start with colorcode then prepend ^7 so it will always start with 
// a colorcode
char* correct_name_v2( char *name )
{
	static char corrected_name[ PLAYER_NAME_SIZE ];
	int len, index;

	corrected_name[ 0 ]= '\0';

	len = strlen( name );

	// rightstrip all color codes until len <= Q3A_MAX_PLAYERNAMESIZE
	while ( len > 2 && name[ len - 2 ] == '^' ) {
		name[ len - 2 ] = '\0';
		len = len - 2;
	}

	if ( name[ 0 ] != '^' ) { // if name doesn't start with colorcode then prepend it
		strcpy( corrected_name, "^7" );
	}

	strcat( corrected_name, name );
	
	// now replace ascii control characters with a dot '.'
	index = strlen( corrected_name ) - 1;
	while ( index >= 0 ) {
		if ( corrected_name[ index ] < 32 ) {
			corrected_name[ index ] = '.';
		}
		index--;
	}


	return corrected_name;
}

// public functions

long close_range_frags( int p )
{
	static long result;

	result = PLAYERS[ p ].frags_per_weapon[ W_BATTLEAXE ] +
			 PLAYERS[ p ].frags_per_weapon[ W_KNIFE		] +
			 PLAYERS[ p ].frags_per_weapon[ W_WRENCH	] +
			 PLAYERS[ p ].frags_per_weapon[ W_SYRINGE	] ;

	return result;

}

int worst_enemy( int player )
{
	// return the worst enemy for player
	int enemy;
	long max_amount = -1; // amount, frags + deaths
	long amount;
	int worst_enemy_sofar = -1; // is the worst enemy so far

	for ( enemy = 0; enemy < player_count; enemy++ ) {
		// only check players in different teams
		if ( frag_matrix[ player ][ enemy].same_team == PLAYERS_DIFFERENT_TEAM ) {
			amount = frag_matrix[ player ][ enemy].frags + frag_matrix[ enemy ][ player ].frags;

			// for testing
	//		sprintf( outbuf, "\n%-30.30s %5ld", PLAYERS[ enemy ].nameclean, amount );
	//		statfile( SF_PRINT, outbuf );

			if ( amount > max_amount ) {
				max_amount = amount;
				worst_enemy_sofar = enemy;
			}
		}
	}

	return worst_enemy_sofar;

}

//void calculate_role_factor()
//{
//	long sum;
//	double role_factor;
//	int p, q;
//	int nr_enemies;
//
//	for ( p = 0; p < player_count; p++ ) {
//		sum = 0;
//		nr_enemies = 0;
//		for ( q = 0; q < player_count; q++ ) {
//			if ( frag_matrix[ p ][ q ].same_team = NO ) {
//				sum = sum + frag_matrix[ p ][ q ].frags;
//				sum = sum + frag_matrix[ q ][ p ].frags;
//			}
//		}
//		role_factor = 0.0;
//		if ( nr_enemies > 0 ) {
//			role_factor = (double)(sum / nr_enemies);
//		}
//		PLAYERS[ p ].role_factor = role_factor;
//	}
//
//}

// todo:
//long event_points( t_player_events event )
//{
//	long result;
//
//	switch( event ) {
//		case PE_FIRST_TOUCH :
//			break;
//		case PE_FLAG_TOUCH :
//			break;
//		case PE_FLAG_CAPTURE :
//			break;
//		case PE_DIED_NEAR_FLAG_AT_BASE :
//			break;
//		case PE_DIED_NEAR_FLAG_IN_FIELD :
//			break;
//		case PE_COAST_TO_COAST :
//			break;
//		case PE_DEFEND_FLAGCARRIER :
//			break;
//		case PE_TEAMKILL_FLAGCARRIER :
//			break;
//		case PE_DESTROY_SENTRY :
//			break;
//		case PE_DESTROY_SUPPLYSTATION :
//			break;
//		case PE_DEFEND_FLAG_AT_BASE :
//			break;
//		case PE_DEFEND_FLAG_IN_FIELD:
//			break;
//		case PE_KILL_FLAGCARRIER :
//			break;
//		case PE_TEAMKILL_SENTRY :	
//			break;
//		case PE_TEAMKILL_SUPPLYSTATION :
//			break;
//	}
//	return result;
//}
//
//// todo:
//void process_event_scores( void )
//{
//	int p,e;
//
//	for ( p = 0; p < player_count; p++ ) {
//		for ( e = 0; e < PE_MAX_EVENTS; e++ ) {
//			
//		}
//	}
//}

void update_player_score( t_player_events event, int player, int multiplier )
{
	//if ( event > 0 && event < PE_MAX_EVENTS ) {
	//	PLAYERS[ player ].events[ event ]++;
	
	switch( event ) {
			// offence
		case PE_FIRST_TOUCH :
			if ( query_gametype_modifier() == GTM_REVERSE ) {
				PLAYERS[ player ].off_points += PS_FLAG_CAPTURE * multiplier;
			}
			else {
				PLAYERS[ player ].off_points += PS_FIRST_TOUCH * multiplier;
			}
			PLAYERS[ player ].first_touches ++;
			PLAYERS[ player ].flagtouches++;
			break;

		case PE_FLAG_TOUCH :
			PLAYERS[ player ].flagtouches++;
			PLAYERS[ player ].off_points += PS_FLAG_TOUCH * multiplier;
			break;

		case PE_FLAG_CAPTURE :
			PLAYERS[ player ].flags_captured++;
			if ( query_gametype_modifier() == GTM_REVERSE ) {
				PLAYERS[ player ].off_points += PS_FIRST_TOUCH * multiplier;
			}
			else {
				PLAYERS[ player ].off_points += PS_FLAG_CAPTURE * multiplier;
			}
			break;

		case PE_DIED_NEAR_FLAG_AT_BASE :
			PLAYERS[ player ].killed_near_flag_in_base++;
			PLAYERS[ player ].off_points += PS_DIED_NEAR_FLAG_AT_BASE * multiplier;
			break;

		case PE_DIED_NEAR_FLAG_IN_FIELD :
			PLAYERS[ player ].killed_near_flag_in_field++;
			PLAYERS[ player ].off_points += PS_DIED_NEAR_FLAG_IN_FIELD * multiplier;
			break;

		case PE_COAST_TO_COAST :
			PLAYERS[ player ].flags_captured++;
			PLAYERS[ player ].coast_to_coast++;
			PLAYERS[ player ].off_points += PS_COAST_TO_COAST * multiplier;
			break;
		case PE_DEFEND_FLAGCARRIER :
			PLAYERS[ player ].defend_flagcarrier++;
			PLAYERS[ player ].off_points += PS_DEFEND_FLAGCARRIER * multiplier;
			break;
		case PE_TEAMKILL_FLAGCARRIER :
			PLAYERS[ player ].teamkilled_flagcarrier++;
			PLAYERS[ player ].off_points += PS_TEAMKILL_FLAGCARRIER * multiplier;
			break;

		case PE_DESTROY_SENTRY :
			PLAYERS[ player ].destroyed_sentry += multiplier;
			PLAYERS[ player ].off_points += PS_DESTROY_SENTRY * multiplier;
			break;

		case PE_DESTROY_SUPPLYSTATION :
			PLAYERS[ player ].destroyed_supply += multiplier;
			PLAYERS[ player ].off_points += PS_DESTROY_SUPPLYSTATION * multiplier;
			break;

		// defence
		case PE_DEFEND_FLAG_AT_BASE :
			PLAYERS[ player ].defend_flag_at_base++;
			PLAYERS[ player ].def_points += PS_DEFEND_FLAG_AT_BASE * multiplier;
			break;

		case PE_DEFEND_FLAG_IN_FIELD:  // ACTF
			PLAYERS[ player ].defend_flag_in_field++;
			PLAYERS[ player ].def_points += PS_DEFEND_FLAG_IN_FIELD * multiplier;
			break;
		case PE_KILL_FLAGCARRIER :
			PLAYERS[ player ].killed_flagcarrier++;
			PLAYERS[ player ].def_points += PS_KILL_FLAGCARRIER * multiplier;
			break;
		case PE_TEAMKILL_SENTRY :
			PLAYERS[ player ].teamkilled_sentry+= multiplier;
			PLAYERS[ player ].def_points += PS_TEAMKILL_SENTRY * multiplier;
			break;
		case PE_TEAMKILL_SUPPLYSTATION :
			//PLAYERS[ player ].teamkilled_supply, todo:
			PLAYERS[ player ].def_points += PS_TEAMKILL_SUPPLYSTATION * multiplier;
			break;
		}
	//}
}

// keep track of the first and last line a player appears in 
void mark_player_line( int player, long linenumber )
{
	if ( player >= 0 && player < MAX_PLAYERS ) {
		PLAYERS[ player ].linecount++;
		if ( PLAYERS[ player ].first_line == NO_LINE_NUMBER ) {
			PLAYERS[ player ].first_line = linenumber;
		}

		if ( linenumber > PLAYERS[ player ].last_line ) {
			PLAYERS[ player ].last_line = linenumber;
		}
	}

	mark_match_line( linenumber );
}

// take name info from parsed line, %p = oldname, %n = newname
// todo: check first_line and last_line ?
int player_participated( int p )
{
	int result;

	result = FALSE;

	if ( p >= 0 && p < player_count ) {
		if ( 
				PLAYERS[ p ].frags > 0              ||
				PLAYERS[ p ].deaths > 0             ||
				PLAYERS[ p ].flags_captured > 0     ||
				PLAYERS[ p ].flagtouches > 0        ||
				PLAYERS[ p ].lost_sentry  > 0       ||
				PLAYERS[ p ].lost_supply > 0        ||
				PLAYERS[ p ].sit_on_grenade  > 0    ||
				PLAYERS[ p ].suicides   > 1         ||
				PLAYERS[ p ].teamdeaths  > 0        ||
				PLAYERS[ p ].teamkills  > 0         ||
				PLAYERS[ p ].teamkilled_sentry > 0
				) {
			result = TRUE;
		}
	}

	return result;
}

void set_playername( int player, char *player_name )
{
	strcpy( PLAYERS[ player ].name	   , correct_name_v2( player_name ) );
	strcpy( PLAYERS[ player ].name_html, et_2_html ( PLAYERS[ player ].name ) );
	strcpy( PLAYERS[ player ].nameclean, plain_text( PLAYERS[ player ].name ) );

	strcpy( PLAYERS[ player ].nameclean_html, PLAYERS[ player ].nameclean );
	convert_plaintext_to_html( PLAYERS[ player ].nameclean_html );

	copy_reverse_string( PLAYERS[ player ].nameclean_rev, PLAYERS[ player ].nameclean );
}

void rename_player( void )
{
	int i;
	char *oldname = player;

	// oldname renamed to newname
    // todo: something about unnamedplayers
	if ( player_exists( oldname ) >= 0 ) {
		if ( player_exists( newname ) >= 0 ) {
			// todo: This player renames from an existing name to an existing name.
			// todo: de-activate old player, activate new player
			i = select_player( oldname );
			PLAYERS[ i ].enabled = FALSE;
			i = select_player( newname );
			PLAYERS[ i ].enabled = TRUE;
		}
		else {
			// Old player exists, new player does not exist.
			// This should be the most occuring case.
			i = select_player( oldname );
			if ( i != -1 ) {
				set_playername( i, newname );
			}
		}
	}
	else {
		if ( player_exists( newname ) >= 0 ) {
			// Old player does not exist, new player does exist.
			// This can happen when a player reconnects as unnamedplayer.
			// In this case we don't do anything
		}
		else {
			// Both players have not been created. Don't bother creating them
		}
	}

	if ( OUTPUT_EVENTS ) {
		sprintf( outbuf, "[ renamed to %s ]", et_2_html(newname) );
		events_add( ET_OTHER, et_2_html(oldname), outbuf );		
	}
}

// return playernumber if found
// return -1 if not found
int player_exists( char *playername )
{
	static int result;
	static char p [ PLAYER_NAME_SIZE ];
	char *p2;
	int i;

	strcpy( p, correct_name_v2( playername ) );
	
	// first check on complete name
	result = -1;
	i = 0;
	while ( result == -1 && i < player_count ) {
		if ( 0 == strcmp( p, PLAYERS[ i ].name ) ) {
			if ( PLAYERS[ i ].enabled == TRUE ) {
				result = i;
				break;
			}
		}
		i++;
	}
	
	// This appears to be no longer a problem in ETF
	i = 0;
	while ( result == -1 && i < player_count ) {
		
		// names should match at least Q3A_MAX_PLAYERNAMESIZE - 2 characters
		// see this example:
		// ^y[^4k^y1^4ck^y]Haunter ^4>^yqwtf
		// ^y[^4k^y1^4ck^y]Haunter ^4>^y  : len == Q3A_MAX_PLAYERNAMESIZE 
		// after stripping the last colorcode (by correct_name_v2()) we will have:
		// ^y[^4k^y1^4ck^y]Haunter ^4>

		// todo: to make it perfect:
		// put ^7 colorcodes in all messages
		// adjust correct_name_v2 to only strip until name length <= Q3A_MAX_PLAYERNAMESIZE
		if ( 0 == strncmp( p, PLAYERS[ i ].name, Q3A_MAX_PLAYERNAMESIZE - 2 ) ) {
			
			if ( PLAYERS[ i ].enabled == TRUE ) {

				if ( strlen( p ) > strlen( PLAYERS[ i ].name ) ) {
					// we found a longer playername, put it in player structure
					set_playername( i, p );
				}
				result = i;
				break;
			}
		}
		i++;
	}
	if ( result == -1 ) {
		// check on plaintext name
		p2 = plain_text( p );
		i = 0;
		while ( result == -1 && i < player_count ) {
			if ( 0 == strcmp( p2, PLAYERS[ i ].nameclean )) {
				if ( PLAYERS[ i ].enabled == TRUE ) {
					result = i;
					break;
				}
			}
			i++;
		}
	}

	return result;
}

void init_player( int i )
{
	int j;

	PLAYERS[ i ].flags_captured = 0;
	PLAYERS[ i ].coast_to_coast = 0;
	PLAYERS[ i ].current_fragstreak = 0;
	PLAYERS[ i ].deaths = 0;
	PLAYERS[ i ].def_points = 0;
	PLAYERS[ i ].def_points_indirect = 0;
	PLAYERS[ i ].defend_flag_at_base = 0;
	PLAYERS[ i ].defend_flag_in_field = 0;
	PLAYERS[ i ].defend_flagcarrier = 0;
	PLAYERS[ i ].destroyed_sentry = 0;
	PLAYERS[ i ].destroyed_supply = 0;
	PLAYERS[ i ].flagtouches = 0;
	PLAYERS[ i ].first_line = NO_LINE_NUMBER;
	PLAYERS[ i ].first_touches = 0;
	PLAYERS[ i ].frags = 0;
	PLAYERS[ i ].initial_id = i;
	PLAYERS[ i ].killed_flagcarrier = 0;
	PLAYERS[ i ].killed_near_flag_in_base = 0;
	PLAYERS[ i ].killed_near_flag_in_field = 0;
	PLAYERS[ i ].last_line = NO_LINE_NUMBER;
	PLAYERS[ i ].linecount = 0;
	PLAYERS[ i ].lost_sentry = 0;
	PLAYERS[ i ].lost_supply = 0;
	PLAYERS[ i ].max_fragstreak = 0;
	PLAYERS[ i ].off_points = 0;	
	PLAYERS[ i ].off_points_indirect = 0;
	PLAYERS[ i ].sentry_up = FALSE;
	PLAYERS[ i ].sit_on_grenade = 0;
	PLAYERS[ i ].squad = SQUAD_UNKNOWN;
	PLAYERS[ i ].suicides = 0;
	set_team( i, TEAM_UNKNOWN );
	PLAYERS[ i ].teamdeaths = 0;
	PLAYERS[ i ].teamkills = 0;
	PLAYERS[ i ].teamkilled_flagcarrier = 0;
	PLAYERS[ i ].teamkilled_sentry = 0;

	strcpy( PLAYERS[ i ].name             , "" );
	strcpy( PLAYERS[ i ].nameclean        , "" );
	strcpy( PLAYERS[ i ].nameclean_rev    , "" );
	strcpy( PLAYERS[ i ].nameclean_html   , "" );
	strcpy( PLAYERS[ i ].name_html        , "" );
	

	for ( j = 0; j < MAX_TEAMS; j++ ) {
		PLAYERS[ i ].in_team[ j ] = UNKNOWN;
	}

	for ( j = 0; j < W_NR_WEAPONS; j++ ) {
		PLAYERS[ i ].frags_per_weapon[ j ] = 0;
		PLAYERS[ i ].deaths_per_weapon[ j ] = 0;
		PLAYERS[ i ].fragged_flagcarrier_per_weapon[ j ] = 0;
	}
	/*for ( j = 1; j < ENV_NR_ENVIRONMENTS; j++ ) {
		PLAYERS[ i ].suicides_per_environment[ j ] = 0;
	}*/
	
	PLAYERS[ i ].sort_value1 = 0;
	PLAYERS[ i ].sort_value2 = 0;
	PLAYERS[ i ].enabled = TRUE;  // set default to true, disable when a player changes team

	PLAYERS[ i ].role_factor  = 0.0;
	PLAYERS[ i ].enemy_factor = 0;
	
}
player_stats *sorted_player( int p )
{
	return &(PLAYERS [ player_index [ p ] ]);
}

int find_player_nameclean( char *nameclean )
{
	int result = -1;
	int p;

	for ( p = 0; p < player_count; p++ ) {
		if ( 0 == strcmp( nameclean, PLAYERS[ p ].nameclean ) ){
			result = p;
			break;
		}
	}

	return result;
}

int select_player( char *playername )
{
	int result;
	int i = 0;
	int found = FALSE;
	//char *p;
	// todo: figure out what is the best place to check for enabled
	// todo: build in check for disabled players [ x ].enabled

	result = player_exists( playername );

	if ( result >= 0 ) {
		found = TRUE;
	}
	else {
		found = FALSE;
	}

	if ( !found && player_count < MAX_PLAYERS ) {
		// create new player 
		init_player( player_count );
		set_playername( player_count, playername );

		if ( 0 == strlen( PLAYERS[ player_count].nameclean ) ) {
			// name is empty, don't create it 
			return NO_PLAYER;
		}	
		result = player_count;

		player_count++;
	}
	
	current_player = result;

	return result;
}

#if defined(SERVER_STATS)
int player_id_exists( long player_id )
{
	int result;
	int i;

	result = -1;

	i = 0;
	while ( result == -1 && i < player_count ) {
		if ( PLAYERS[ i ].server_id == player_id ) {
			if ( PLAYERS[ i ].enabled == TRUE ) {
				result = i;
				break;
			}
		}
		i++;
	}

	return result;
}

void set_player_id( int player, long id ) {
	PLAYERS[ player ].server_id = id;
}

int select_player_by_number( char *player_number )
{
	int result;
	int found;
	long player_id = NO_PLAYER;

	result = NO_PLAYER;

	if ( strlen( player_number ) > 0 ) {
		player_id = atol( player_number );
		
		if ( player_id == PLAYER_WORLD_ID ) {
			result = PLAYER_WORLD;
		}
		else {
			result = player_id_exists( player_id );
		}
	}

	if ( result >= 0 || result == PLAYER_WORLD ) {
		found = TRUE;
	}
	else {
		found = FALSE;
	}

	if ( !found && player_count < MAX_PLAYERS ) {
		// create new player 
		init_player( player_count );

		set_player_id( player_count, player_id );

		result = player_count;

		player_count++;
	}

	current_player = result;

	return result;
}
#endif

char *environmental_death_name( int environment_nr )
{
	switch ( environment_nr ) {
		case	ENV_UNKNOWN :		return "Environmental accident";
		case	ENV_CRATERED :		return "Cratered";
		case	ENV_DROWNED :		return "Drowned";
		case	ENV_LAVA :			return "Lava'd";
		case	ENV_SQUISHED :		return "Squished";
		case	ENV_MELTED :		return "Melted";
		case	ENV_TELEFRAGGED :	return "Telefragged";
		case	ENV_WRONG_PLACE :	return "Wrong place";
	}

	return "Unknown cause of death";
}

