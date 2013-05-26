#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "flagtracking.h"
#include "game.h"
#include "teams.h"
#include "players.h"
#include "sorting.h"
#include "output.h"

// prototypes;
#if defined(SERVER_STATS)
void create_new_team_from_name( char *team );
#endif

// public variables
team_stats				TEAMS		[ MAX_TEAMS ]; // team array
int						team_index	[ MAX_TEAMS ]; // unique index, each element points to a team in TEAMS[]
int						team_count;
int						current_team;
int						conflicting_teaminfo;
teamscore_type			teamscore_history[ MAX_TEAMSCORING_EVENTS ];
int						teamscore_event_count;
int						active_teams;

// private variables

// private functions
void add_player_to_team( int p )
{
	int count;
	int t;

	if ( p >= 0 && p < MAX_PLAYERS ) {

		t = PLAYERS[ p ].team;
		count = TEAMS[ t ].player_count;

		if ( t >= 0 && t < MAX_TEAMS ) {
			TEAMS[ t ].playerlist[ count ] = p;
			TEAMS[ t ].player_count++;
		}
	}
}

void set_team( int player, int team )
{
	if ( player >= 0 && player < MAX_PLAYERS &&  (( team >= 0 && team <= MAX_TEAMS ) || team == TEAM_UNKNOWN) ) {
		PLAYERS[ player ].team = team;
	}
}

// players must be sorted before this is called
void attach_playerlist_to_teams( void )
{
	int i;

	for ( i = 0; i < player_count; i++ ){
		add_player_to_team( player_index[ i ] );
	}
}

void create_extra_team( void )
{
	if ( 0 == strcmp( TEAMS[ 0 ].name, "RED" ) ) {
		select_team( "BLUE");
	}
	else {
		select_team( "RED");
	}
}

void reset_player_relations_info( int player )
{
	int t;

	for ( t = 0; t < player_count; t++ ) {

		if ( t != player ) {
			frag_matrix[ player ][ t ].same_team = TEAM_RELATION_UNKNOWN;
			frag_matrix[ t ][ player ].same_team = TEAM_RELATION_UNKNOWN;
		}
	}
}

void mark_relation( int player1, int relation1, int player2, int relation2 )
{
#if defined(SERVER_STATS)
	// nothing to do
#else
	int i;
	// mark player1, buddies, player2, friend -> make player2 the friend of all player1's buddies

	for ( i = 0; i < player_count; i++ ) {

		if ( ( frag_matrix[ player1 ][ i ].same_team == relation1 ) ) {

			switch( relation2 ){

				case PLAYERS_DIFFERENT_TEAM :
					if ( PLAYERS_SAME_TEAM == frag_matrix[ player2][ i ].same_team ) {
						reset_player_relations_info( player2 );
						reset_player_relations_info( i );
					}
					break;

				case PLAYERS_SAME_TEAM :
					if ( PLAYERS_DIFFERENT_TEAM == frag_matrix[ player2][ i ].same_team  ) {
						reset_player_relations_info( player2 );
						reset_player_relations_info( i );
					}
					break;
			}

			frag_matrix[ player2][ i ].same_team = relation2;
			frag_matrix[ i][ player2 ].same_team = relation2;
		}

	}
#endif
}

void mark_known_player_team( int player, int team ) {
	
#if defined(SERVER_STATS)
	// nothing to do
#else
	int i;

	for ( i = 0; i < player_count; i++ ) {
		//if ( ( frag_matrix[ player ][ i ].same_team == YES ) ) {
		if ( ( frag_matrix[ player ][ i ].same_team == PLAYERS_SAME_TEAM) ) {
			set_team( player, team );
		}
		//if ( ( frag_matrix[ player ][ i ].same_team == NO ) ) {
		if ( ( frag_matrix[ player ][ i ].same_team == PLAYERS_DIFFERENT_TEAM ) ) {
			PLAYERS[ i ].in_team[ team ] = NO;
		}
	}
#endif
}

void reset_player_team_info( int player )
{
	int t;

	set_team( player, TEAM_UNKNOWN );

	for ( t = 0; t < MAX_TEAMS ; t++ ) {
		PLAYERS[ player ].in_team[ t ] = UNKNOWN;
	}

	for ( t = 0; t < player_count; t++ ) {

		if ( t != player ) {
			frag_matrix[ player ][ t ].same_team = TEAM_RELATION_UNKNOWN;
			frag_matrix[ t ][ player ].same_team = TEAM_RELATION_UNKNOWN;
		}
	}
}

void set_player_team_relation( int player, int team, char pt_relation ) 
{

#if defined(SERVER_STATS)
	// nothing to do
#else

	switch ( pt_relation ) {
		case YES :
			if ( PLAYERS[ player ].team == TEAM_UNKNOWN ) {
				// if the team was unknown we can set it now
				set_team( player, team );
				PLAYERS[ player ].in_team[ team ] = YES;
				mark_known_player_team( player, team );
			}
			else {
				// the team was known, check if the new info is not conflicting with the old info
				if ( PLAYERS[ player ].team != team ) {
					conflicting_teaminfo = TRUE;
					// probably he switched teams
					//reset_player_team_info( player );
					conflicting_teaminfo = FALSE;
				}	
			}
			break;

		case NO :
			if ( PLAYERS[ player ].team == team ) {
				conflicting_teaminfo = TRUE;
				reset_player_team_info( player );
				conflicting_teaminfo = FALSE;
			}
			else {
				PLAYERS[ player ].in_team[ team ] = NO;
			}
			
			break;

		default:
			PLAYERS[ player ].in_team[ team ] = UNKNOWN;
			break;
	}
#endif
}

int deduct_team_reverse_logic( int *team_not )
{
	static int team;
	int t;
	int unknown_count = 0;

	team = -1;

	for ( t = 0 ; t < team_count ; t++ ) {
		switch ( team_not[ t ] ) {
		case YES:
			team = t;
			return team;
			break;
		case NO:
			break;
		case UNKNOWN:
			// these are the candidates for a team
			// if there is only one unknown then that's it:) w00t
			unknown_count++;
			team = t;
			break;

		}
	}

	if ( team != -1 && unknown_count == 1 ) {
		// yeah we found it
		return team;
	}

	return -1;
}

void complete_teamscore_history( void )
{
	// set the team of the scoring player
	int t;
	int p; // player

	for ( t = 0; t < teamscore_event_count; t++ ) {
		p = teamscore_history[ t ].scoring_player;
		if ( p >= 0 && p < MAX_PLAYERS ) {
			teamscore_history[ t ].scoring_team = PLAYERS[ p ].team;
		}
	}
}

void add_teamscore_event( int scoring_team, int yielding_team, int scoring_player, long flagtouches )
{
	// scoring team = -1 means team is unknown because the team of the player is unknown
	teamscore_history[ teamscore_event_count ].scoring_player = scoring_player;
	teamscore_history[ teamscore_event_count ].scoring_team   = scoring_team;
	teamscore_history[ teamscore_event_count ].yielding_team  = yielding_team;
	teamscore_history[ teamscore_event_count ].flagtouches    = flagtouches;
	teamscore_event_count++;
}

void init_teamscore_history( void )
{
	int t;

	for ( t=0; t < MAX_TEAMSCORING_EVENTS; t++ ) {
		teamscore_history[ t ].scoring_player = NO_PLAYER;
		teamscore_history[ t ].scoring_team   = TEAM_UNKNOWN;
		teamscore_history[ t ].yielding_team  = TEAM_UNKNOWN;
		teamscore_history[ t ].flagtouches    = 0;
	}

	teamscore_event_count = 0;
}

void init_team( int i )
{
	int j,w;

	TEAMS[ i ].flags_lost = 0;
	TEAMS[ i ].flags_captured = 0;
	strcpy( TEAMS[ i ].name, "" );
	strcpy( TEAMS[ i ].name_html, "" );
	strcpy( TEAMS[ i ].clantag, "" );
	strcpy( TEAMS[ i ].clantag_html, "" );
	strcpy( TEAMS[ i ].filename, "" );

	TEAMS[ i ].deaths = 0;
	TEAMS[ i ].defend_flag_at_base = 0;
	TEAMS[ i ].defend_flag_in_field = 0;
	TEAMS[ i ].defend_flagcarrier = 0;
	TEAMS[ i ].destroyed_sentry = 0;
	TEAMS[ i ].destroyed_supply = 0;
	TEAMS[ i ].flagtouches = 0;
	TEAMS[ i ].first_touches = 0;
	TEAMS[ i ].frags = 0;
	TEAMS[ i ].killed_flagcarrier = 0;
	TEAMS[ i ].killed_near_flag_in_base = 0;
	TEAMS[ i ].lost_sentry = 0;
	TEAMS[ i ].lost_supply = 0;
	TEAMS[ i ].player_count = 0;
	TEAMS[ i ].score = -99999;
	TEAMS[ i ].suicides = 0;
	TEAMS[ i ].teamdeaths = 0;
	TEAMS[ i ].teamkills = 0;
	TEAMS[ i ].teamkilled_flagcarrier = 0;

	for ( j = 0; j < W_NR_WEAPONS; j++ ) {
		TEAMS[ i ].frags_per_weapon[ j ] = 0;
		TEAMS[ i ].deaths_per_weapon[ j ] = 0;
	}
	
	for ( j = 0; j < MAX_SQUADS; j++ ) {
		TEAMS[ i ].squad[ j ].avg_rolefactor = 0;
		TEAMS[ i ].squad[ j ].avg_worst_enemy_factor = 0;
		TEAMS[ i ].squad[ j ].captures = 0;
		TEAMS[ i ].squad[ j ].deaths = 0;
		TEAMS[ i ].squad[ j ].deaths_with_flag_moved = 0;
		TEAMS[ i ].squad[ j ].frags = 0;
		TEAMS[ i ].squad[ j ].frags_with_flag_moved = 0;
		TEAMS[ i ].squad[ j ].internal_tks = 0;
		TEAMS[ i ].squad[ j ].killed_flagcarrier = 0;
		TEAMS[ i ].squad[ j ].suicides = 0;		
		for ( w = 0; w < W_NR_WEAPONS; w++ ) {
			TEAMS[ i ].squad[ j ].frags_per_weapon[ w ] = 0;
		}
	}

	TEAMS[ i ].sit_on_grenade = 0;
}

// determine the teamname to be used in a filename
void determine_team_filename( int team_nr )
{
	char* filename = TEAMS[ team_nr ].filename;
	// first go by clantag AGO etc.
	// then by name "RED" etc.

	if ( strlen( TEAMS[ team_nr ].clantag  ) > 0 ) {
		strcpy( filename, TEAMS[ team_nr ].clantag );
		filter( filename, FILENAME_FORBIDDEN );
	}

	if ( strlen( filename ) <= 0 ) {
		strcpy( filename, TEAMS[ team_nr ].name );
	}
}


void output_teams_html_element( TEAM_MATRIX msg, int p )
{

	switch( msg ) {
		case TM_UNDEFINED	:
			break;
		case TM_INIT		:
			sprintf( outbuf, "<br />\n" );
			AS_OUT( OE_PLAIN, outbuf );

			sprintf( outbuf, HTML_TEAM_INIT );
			AS_OUT( OE_PLAIN, outbuf );
			break;
		case TM_ROW_INIT	:
			sprintf( outbuf, HTML_TEAM_ROW_INIT );
			AS_OUT( OE_PLAIN, outbuf );
			break;
		case TM_ROW_TERM	:
			sprintf( outbuf, HTML_TEAM_ROW_TERM );
			AS_OUT( OE_PLAIN, outbuf );
			break;
		case TM_HEADER  	:
			sprintf( outbuf, HTML_TEAM_HEAD, TEAMS[ p ].name_html );
			AS_OUT( OE_PLAIN, outbuf );
			break;
		case TM_PLAYER		:
			if ( p >= 0 ) {
				sprintf( outbuf, HTML_TEAM_PLAYER, 
					css_team_background_color( PLAYERS[ p ].team ),
					//TEAMS[ PLAYERS[ p ].team ].name, 
					query_squadname( p ), 
					html_player_clickable( p ) );
			}
			else {
				// use default background color
				sprintf( outbuf, HTML_TEAM_PLAYER, "", "" , "" );
			}
			AS_OUT( OE_PLAIN, outbuf );
			break;
		/*case TM_LINE_OBJ :
			sprintf( htmlbuf, HTML_TEAM_LINE_OBJ, outbuf );
			AS_OUT( OE_PLAIN, htmlbuf );
			break;*/
		/*case TM_LINE_DEF :
			sprintf( htmlbuf, HTML_TEAM_LINE_DEF, outbuf );
			AS_OUT( OE_PLAIN, htmlbuf );
			break;*/
		/*case TM_LINE_OFF :
			sprintf( htmlbuf, HTML_TEAM_LINE_OFF, outbuf );
			AS_OUT( OE_PLAIN, htmlbuf );
			break;*/
		/*case TM_LINE_FND :
			sprintf( htmlbuf, HTML_TEAM_LINE_FND, outbuf );
			AS_OUT( OE_PLAIN, htmlbuf );
			break;*/
		case TM_LINE_EMPTY :
			sprintf( htmlbuf, HTML_TEAM_LINE_EMPTY );
			AS_OUT( OE_PLAIN, htmlbuf );
			break;
		case TM_TERM		:
			sprintf( outbuf, HTML_TEAM_TERM );
			AS_OUT( OE_PLAIN, outbuf );
			break;
	}
}

void output_squad_top_weapons( int squad_nr )
{
	int  t;
	int  st;
	int  top;
	int  weapon_index[ MAX_TEAMS ][ W_NR_WEAPONS ];
	char number[ 20 ];
	int  nr_weapons = 0;
	int  weapon_used;
	int  print_squad_weapons;
	long frags;
	

	print_squad_weapons = FALSE; 
	// first determine if frags were scored at all by these squads
	// if there were no frags we don't print anything
	
	for ( t = 0; t < team_count; t++ ) { 
		st = sorted_team_nr( t );
		if ( team_participated( st ) ) {

			sort_weapons( &(TEAMS[ st ].squad[ squad_nr ].frags_per_weapon[ 0 ]) , &( weapon_index[ st ][ 0 ] ) );

			frags = TEAMS[ st ].squad[ squad_nr ].frags_per_weapon[ weapon_index[ st ][ 0 ] ];
			if ( frags > 0 ) { // check the frags of the best-scoring weapon
				print_squad_weapons = TRUE;
			}			
		}
	}

	if ( print_squad_weapons ) {
		// print header
		output_teams_html_element( TM_ROW_INIT, 0 );

		for ( t = 0; t < team_count; t++ ) { 
			st = sorted_team_nr( t );
			
			if ( team_participated( st ) ) {
				sprintf( outbuf, HTML_TEAM_FRAGS_BY_WEAPON_HEADER );
				AS_OUT( OE_PLAIN, outbuf );
			}
		}

		output_teams_html_element( TM_ROW_TERM, 0 );

		// now determine how many lines to print 
		for ( top = 0; top < W_NR_WEAPONS; top++ ) {
			weapon_used = FALSE;
			
			for ( t = 0; t < team_count; t++ ) { 
				st = sorted_team_nr( t );
				if ( team_participated( st ) ) {
					frags = TEAMS[ st ].squad[ squad_nr ].frags_per_weapon[ weapon_index[ st ][ top ] ];
					if ( frags > 0 ) {
						weapon_used = TRUE;
					}
				}
			}
			
			if ( weapon_used ) {
				nr_weapons = top + 1;
			}
		}
		
		// now print the actual weapons and frags
		for ( top = 0; top < nr_weapons; top++ ) {
			output_teams_html_element( TM_ROW_INIT, 0 );

			for ( t = 0; t < team_count; t++ ) { 
				st = sorted_team_nr( t );
				if ( team_participated( st ) ) {
					frags = TEAMS[ st ].squad[ squad_nr ].frags_per_weapon[ weapon_index[ st ][ top ] ];
					if ( frags > 0 ) {
						sprintf( number, "%ld", frags );
						sprintf( outbuf, HTML_TEAM_FRAGS_BY_WEAPON_LINE, weapon_name( weapon_index[ st ][ top ] ) , number );
						//sprintf( outbuf, "<td>%s</td><td>%ld</td>", weapon_name( weapon_index[ st ][ top ] ), TEAMS[ st ].squad[ squad_nr ].frags_per_weapon[ weapon_index[ st ][ top ] ] );
					}
					else {
						sprintf( outbuf, HTML_TEAM_FRAGS_BY_WEAPON_LINE, "", "" );
					}
					
					AS_OUT( OE_PLAIN, outbuf );
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );
		} // end for
	} // end if
}

void output_squad_def_statistics_html( void )
{
	int t;

	// output header
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count; t++ ) { 
		if ( team_participated( sorted_team_nr( t ) ) ) {
			sprintf( outbuf, HTML_TEAM_HEADER_DEF );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Captured %ld flags",  sorted_team( t )->squad[ SQUAD_DEF ].captures );
			//output_teams_html_element( TM_LINE_DEF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_DEF_LONG, "Flags captured", sorted_team( t )->squad[ SQUAD_DEF ].captures );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	// todo: defended at base etc.
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld frags",  sorted_team( t )->squad[ SQUAD_DEF ].frags );
			//output_teams_html_element( TM_LINE_DEF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_DEF_LONG, "Frags", sorted_team( t )->squad[ SQUAD_DEF ].frags );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld deaths",  sorted_team( t )->squad[ SQUAD_DEF ].deaths );
			//output_teams_html_element( TM_LINE_DEF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_DEF_LONG, "Deaths", sorted_team( t )->squad[ SQUAD_DEF ].deaths );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );
	
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			if ( 0 != sorted_team( t )->squad[ SQUAD_DEF ].deaths ) {
				//sprintf( outbuf, "%.2lf frag/death ratio", (double)sorted_team( t )->squad[ SQUAD_DEF ].frags / (double)sorted_team( t )->squad[ SQUAD_DEF ].deaths );
				sprintf( outbuf, HTML_TEAM_LINE_DEF_DOUBLE, "Frag/death ratio", 4,2,(double)sorted_team( t )->squad[ SQUAD_DEF ].frags / (double)sorted_team( t )->squad[ SQUAD_DEF ].deaths );
			}
			else {
				//sprintf( outbuf, "No frag/death ratio" );
				sprintf( outbuf, HTML_TEAM_LINE_DEF_STRING, "No frag/death ratio", "" );
			}

			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld suicides",  sorted_team( t )->squad[ SQUAD_DEF ].suicides );
			//output_teams_html_element( TM_LINE_DEF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_DEF_LONG, "Suicides", sorted_team( t )->squad[ SQUAD_DEF ].suicides );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld teamkills (within the squad)", sorted_team( t )->squad[ SQUAD_DEF ].internal_tks );
			//output_teams_html_element( TM_LINE_DEF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_DEF_LONG, "Teamkills within squad", sorted_team( t )->squad[ SQUAD_DEF ].internal_tks );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Killed the flagcarrier %ld times",  sorted_team( t )->squad[ SQUAD_DEF ].killed_flagcarrier  );
			//output_teams_html_element( TM_LINE_DEF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_DEF_LONG, "Kills on flagcarrier", sorted_team( t )->squad[ SQUAD_DEF ].killed_flagcarrier );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_squad_top_weapons( SQUAD_DEF );
}

void output_squad_off_statistics_html( void )
{
	int t;

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count; t++ ) { 
		if ( team_participated( sorted_team_nr( t ) ) ) {
			sprintf( outbuf, HTML_TEAM_HEADER_OFF );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );
	
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Captured %ld flags",  sorted_team( t )->squad[ SQUAD_OFF ].captures );
			//output_teams_html_element( TM_LINE_OFF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_OFF_LONG, "Flags captured", sorted_team( t )->squad[ SQUAD_OFF ].captures );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld frags",  sorted_team( t )->squad[ SQUAD_OFF ].frags );
			//output_teams_html_element( TM_LINE_OFF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_OFF_LONG, "Frags", sorted_team( t )->squad[ SQUAD_OFF ].frags );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld deaths",  sorted_team( t )->squad[ SQUAD_OFF ].deaths );
			//output_teams_html_element( TM_LINE_OFF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_OFF_LONG, "Deaths", sorted_team( t )->squad[ SQUAD_OFF ].deaths );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );
	
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			if ( 0 != sorted_team( t )->squad[ SQUAD_OFF ].deaths ) {
				//sprintf( outbuf, "%.2lf frag/death ratio", (double)sorted_team( t )->squad[ SQUAD_OFF ].frags / (double)sorted_team( t )->squad[ SQUAD_OFF ].deaths );
				sprintf( outbuf, HTML_TEAM_LINE_OFF_DOUBLE, "Frag/death ratio", 4,2, (double)sorted_team( t )->squad[ SQUAD_OFF ].frags / (double)sorted_team( t )->squad[ SQUAD_OFF ].deaths );
			}
			else {
				//sprintf( outbuf, "No frag/death ratio" );
				sprintf( outbuf, HTML_TEAM_LINE_OFF_STRING, "No frag/death ratio", "" );
			}
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld suicides",  sorted_team( t )->squad[ SQUAD_OFF ].suicides );
			//output_teams_html_element( TM_LINE_OFF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_OFF_LONG, "Suicides", sorted_team( t )->squad[ SQUAD_OFF ].suicides );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld teamkills (within the squad)", sorted_team( t )->squad[ SQUAD_OFF ].internal_tks );
			//output_teams_html_element( TM_LINE_OFF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_OFF_LONG, "Teamkills within squad", sorted_team( t )->squad[ SQUAD_OFF ].internal_tks );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Killed the flagcarrier %ld times",  sorted_team( t )->squad[ SQUAD_OFF ].killed_flagcarrier  );
			//output_teams_html_element( TM_LINE_OFF, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_OFF_LONG, "Kills on flagcarrier", sorted_team( t )->squad[ SQUAD_OFF ].killed_flagcarrier );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_squad_top_weapons( SQUAD_OFF );
}

void output_teams_fnd_statistics_html( void )
{
	int t;
	double ratio;

	// header
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count; t++ ) { 
		if ( team_participated( sorted_team_nr( t ) ) ) {
			sprintf( outbuf, HTML_TEAM_HEADER_FND );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );
	
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld frags", sorted_team( t )->frags );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Frags", sorted_team( t )->frags );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld deaths", sorted_team( t )->deaths );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Deaths", sorted_team( t )->deaths );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			if ( sorted_team( t )->deaths > 0 ) {
				ratio = (double)sorted_team( t )->frags / (double)sorted_team( t )->deaths;
				//sprintf( outbuf, "%5.2lf frag/death ratio", ratio );
				sprintf( outbuf, HTML_TEAM_LINE_FND_DOUBLE, "Frag/death ratio",4,2, ratio );
			}
			else {
				//sprintf( outbuf, "no deahts" );
				sprintf( outbuf, HTML_TEAM_LINE_FND_STRING, "No Frag/death ratio", "" );
			}
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Destroyed %ld sentries", sorted_team( t )->destroyed_sentry );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Sentries destroyed", sorted_team( t )->destroyed_sentry );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Destroyed %ld supply stations", sorted_team( t )->destroyed_supply  );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Supply stations destroyed", sorted_team( t )->destroyed_supply );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Lost %ld sentries", sorted_team( t )->lost_sentry );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Sentries lost", sorted_team( t )->lost_sentry );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "Lost %ld supply stations", sorted_team( t )->lost_supply );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Supply stations lost", sorted_team( t )->lost_supply );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld teamkills", sorted_team( t )->teamdeaths );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Teamkills", sorted_team( t )->teamdeaths );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count ; t++ ) {
		if ( team_participated( sorted_team_nr( t ) ) ) {
			//sprintf( outbuf, "%ld suicides", sorted_team( t )->suicides );
			//output_teams_html_element( TM_LINE_FND, 0 );
			sprintf( outbuf, HTML_TEAM_LINE_FND_LONG, "Suicides", sorted_team( t )->suicides );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

}

// pre: squad statistics have to be calculated
void output_squad_statistics( int team, int squadnr ) 
{
	long frags, deaths, suicides, teamkills;
	double ratio;
	squad_stats *sq;

	sq = &TEAMS[ team ].squad[ squadnr ];

	frags = sq->frags;
	deaths = sq->deaths;
	suicides = sq->suicides;
	teamkills = sq->internal_tks;
	ratio = 0.0;
	if ( deaths != 0.0 ) {
		ratio = (double)frags / (double)deaths;
	}
	sprintf( outbuf, "  frags:%ld deaths:%ld ratio:%.2lf tk's:%ld suicides:%ld\n", frags, deaths, ratio, teamkills, suicides );
	AS_OUT( OE_LINE, outbuf );

}

// public functions
void clear_team_table( void )
{
	int i;

	for ( i = 0; i < MAX_TEAMS ; i ++ ) {
		init_team( i );
	}
	
	team_count = 0;
	active_teams = 0;

	init_teamscore_history();
}

void calculate_teamscores( void )
{	
	int i,t;

	for ( i = 0; i < player_count ; i++ ) {
		if ( PLAYERS[ i ].team != -1 ) {
			TEAMS[ PLAYERS[ i ].team ].flags_captured += PLAYERS[ i ].flags_captured;
		}
		else {
			i = i;
		}
	}
	
	for ( t = 0; t < team_count; t++ ) {
		if ( team_participated( t ) ) {
			TEAMS[ t ].score = TEAMS[ t ].flags_captured - TEAMS[ t ].flags_lost;
		}
	}

}

void output_teams_objectives_html( void )
{
	int t;
	int flag_nr;
	int taken_from_base;
	//long times_taken; // nr of times it was taken from starting position
	//double perc_capped; // percentage of 1st touches that resulted into cap

	// output header
	output_teams_html_element( TM_ROW_INIT, 0 );
	for ( t = 0; t < team_count; t++ ) { 
		if ( team_participated( sorted_team_nr( t ) ) ) {
			sprintf( outbuf, HTML_TEAM_HEADER_OBJ );
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	output_teams_html_element( TM_ROW_TERM, 0 );

	switch( query_gametype() ) {

		case GT_CAPTURE_THE_FLAG :
			
			// Score calculated by flags
			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Final score", sorted_team( t )->score );
					AS_OUT( OE_PLAIN, outbuf );
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			// Captures
			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					//sprintf( outbuf, "Captured %ld flags",  sorted_team( t )->flags_captured );
					//output_teams_html_element( TM_LINE_OBJ, 0 );

					sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Flags captured", sorted_team( t )->flags_captured );
					AS_OUT( OE_PLAIN, outbuf );
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			// Flagtouches
			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					//sprintf( outbuf, "%ld flagtouches",  sorted_team( t )->flagtouches );
					//output_teams_html_element( TM_LINE_OBJ, 0 );
					sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Flagtouches", sorted_team( t )->flagtouches );
					AS_OUT( OE_PLAIN, outbuf );
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			// Touches/Capture
			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					if ( 0 != (double)sorted_team( t )->flags_captured ) {
						sprintf( outbuf, HTML_TEAM_LINE_OBJ_DOUBLE, "Average touches/capture", 4,2, (double)sorted_team( t )->flagtouches / (double)sorted_team( t )->flags_captured );
						AS_OUT( OE_PLAIN, outbuf );
					}
					else {
						sprintf( outbuf, HTML_TEAM_LINE_OBJ_STRING, "No Average touches/capture", "" );
						AS_OUT( OE_PLAIN, outbuf );
					}
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					flag_nr = select_flag( sorted_team( t )->name );
					taken_from_base = FLAGS[ flag_nr ].capped + FLAGS[ flag_nr ].saved;
					if ( FLAGS[ flag_nr ].current_alive_touches > 0 ) {
						taken_from_base++;
					}
											
					sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Flags taken from base", taken_from_base );
					AS_OUT( OE_PLAIN, outbuf );					
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					flag_nr = select_flag( sorted_team( t )->name );
					taken_from_base = FLAGS[ flag_nr ].capped + FLAGS[ flag_nr ].saved;
					if ( FLAGS[ flag_nr ].current_alive_touches > 0 ) {
						taken_from_base++;
					}
					
					sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Flags saved", FLAGS[ flag_nr ].saved );
					AS_OUT( OE_PLAIN, outbuf );					
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					flag_nr = select_flag( sorted_team( t )->name );
					taken_from_base = FLAGS[ flag_nr ].capped + FLAGS[ flag_nr ].saved;
					if ( FLAGS[ flag_nr ].current_alive_touches > 0 ) {
						taken_from_base++;
					}
					
					sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Flags lost", FLAGS[ flag_nr ].capped );
					AS_OUT( OE_PLAIN, outbuf );					
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					flag_nr = select_flag( sorted_team( t )->name );
					if ( gtm_contains( GTM_REVERSE ) ) {
						// todo:
					}
					else {
						//sprintf( outbuf, "Biggest streak of touches saved: %ld",  FLAGS[ flag_nr ].max_touches_saved );
						//output_teams_html_element( TM_LINE_OBJ, 0 );
						sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Biggest streak of touches saved", FLAGS[ flag_nr ].max_touches_saved );
						AS_OUT( OE_PLAIN, outbuf );
					}
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					flag_nr = select_flag( sorted_team( t )->name );
					if ( gtm_contains( GTM_REVERSE ) ) {
						// todo:
					}
					else {
						//sprintf( outbuf, "Biggest streak of touches lost: %ld",  FLAGS[ flag_nr ].max_touches_capped );
						//output_teams_html_element( TM_LINE_OBJ, 0 );
						sprintf( outbuf, HTML_TEAM_LINE_OBJ_LONG, "Biggest streak of touches lost", FLAGS[ flag_nr ].max_touches_capped );
						AS_OUT( OE_PLAIN, outbuf );
					}
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );
			break;
		
		
		case GT_CAPTURE_AND_HOLD :
			
			/*output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count ; t++ ) {
				if ( team_participated( sorted_team_nr( t ) ) ) {
					sprintf( outbuf, "Future stuff" );
					output_teams_html_element( TM_LINE_OBJ, 0 );
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );*/
			break;
	}

	

}

void calculate_team_statistics( void )
{
	int p;
	int t;
	int w;

	for ( p = 0; p < player_count; p++ ) {
		t= PLAYERS[ p ].team;
		if ( t != -1 ) {
			TEAMS[ t ].deaths					+= PLAYERS[ p ].deaths;
			for ( w = 0; w < W_NR_WEAPONS; w++ ){
				TEAMS[ t ].deaths_per_weapon[ w ] += PLAYERS[ p ].deaths_per_weapon[ w ];
				TEAMS[ t ].frags_per_weapon [ w ] += PLAYERS[ p ].frags_per_weapon [ w ];
			}
			TEAMS[ t ].defend_flag_at_base		+= PLAYERS[ p ].defend_flag_at_base;
			TEAMS[ t ].defend_flag_in_field		+= PLAYERS[ p ].defend_flag_in_field;
			TEAMS[ t ].defend_flagcarrier		+= PLAYERS[ p ].defend_flagcarrier;
			TEAMS[ t ].destroyed_sentry			+= PLAYERS[ p ].destroyed_sentry;
			TEAMS[ t ].destroyed_supply			+= PLAYERS[ p ].destroyed_supply;
			TEAMS[ t ].flagtouches				+= PLAYERS[ p ].flagtouches;
			TEAMS[ t ].first_touches			+= PLAYERS[ p ].first_touches;
			TEAMS[ t ].frags					+= PLAYERS[ p ].frags;
			TEAMS[ t ].killed_flagcarrier		+= PLAYERS[ p ].killed_flagcarrier;
			TEAMS[ t ].lost_sentry  			+= PLAYERS[ p ].lost_sentry;
			TEAMS[ t ].lost_supply   			+= PLAYERS[ p ].lost_supply;
			TEAMS[ t ].sit_on_grenade			+= PLAYERS[ p ].sit_on_grenade;
			TEAMS[ t ].suicides					+= PLAYERS[ p ].suicides;
			TEAMS[ t ].teamdeaths				+= PLAYERS[ p ].teamdeaths;
			TEAMS[ t ].teamkilled_flagcarrier	+= PLAYERS[ p ].teamkilled_flagcarrier;
			TEAMS[ t ].teamkills				+= PLAYERS[ p ].teamkills;
			TEAMS[ t ].killed_near_flag_in_base += PLAYERS[ p ].killed_near_flag_in_base;
			
		}
	}

}



void create_new_team( char *team )
{

	init_team( team_count );

	strcpy( TEAMS[ team_count].name, team );

	// use black as default
	sprintf( TEAMS[ team_count].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_UNKNOWN, team  );

	if ( 0 == strcmp( team, "RED" ) ) {
		//strcpy( TEAMS[ team_count].name_html, color_string( team, CSS_COLOR_TEAM_RED ) );
		sprintf( TEAMS[ team_count].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_RED, team  );
	}
	if ( 0 == strcmp( team, "BLUE" ) ) {
		sprintf( TEAMS[ team_count].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_BLUE, team  );
	}
	if ( 0 == strcmp( team, "GREEN" ) ) {
		sprintf( TEAMS[ team_count].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_GREEN, team  );
	}
	if ( 0 == strcmp( team, "YELLOW" ) ) {
		sprintf( TEAMS[ team_count].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_YELLOW, team  );
	}
	
	TEAMS[ team_count ].flags_captured = 0;
		
	team_count++;
}

int select_team( char *team )
{
	static int result;
	int i = 0;
	int found = FALSE;
	char *p;

	//p = strupr( plain_text( team ) );
	p = plain_text( team );
	string_to_upper( p );

	while ( !found && i < MAX_TEAMS ) {
		if ( 0 == strcmp( p, TEAMS[ i ].name ) ){
			found = TRUE;
			result = i;
		}
		i++;
	}

	if ( !found ) {
#if defined (SERVER_STATS)
		create_new_team_from_name( p );
#else
		create_new_team( p );
		result = team_count - 1;
#endif
	}

	return result;
}

#if defined(SERVER_STATS)
void create_new_team_from_number( int team_number )
{
	char team[ 50 ];
	switch( team_number ) {
		case 1:
			strcpy( team, "RED" );
			break;
		case 2:
			strcpy( team, "BLUE" );
			break;
		case 3:
			strcpy( team, "GREEN" );
			break;
		case 4:
			strcpy( team, "YELLOW" );
			break;
		case 5:
			// spectator
			return;
			break;
	}
	init_team( team_number );
	strcpy( TEAMS[ team_number ].name, team );

	// use black as default
	sprintf( TEAMS[ team_number ].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_UNKNOWN, team  );

	if ( 0 == strcmp( team, "RED" ) ) {
		strcpy( TEAMS[ team_number ].name_html, color_string( team, CSS_COLOR_TEAM_RED ) );
		sprintf( TEAMS[ team_number ].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_RED, team  );
	}
	if ( 0 == strcmp( team, "BLUE" ) ) {
		sprintf( TEAMS[ team_number ].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_BLUE, team  );
	}
	if ( 0 == strcmp( team, "GREEN" ) ) {
		sprintf( TEAMS[ team_number ].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_GREEN, team  );
	}
	if ( 0 == strcmp( team, "YELLOW" ) ) {
		sprintf( TEAMS[ team_number ].name_html,"<span class=\"%s\">%s</span>", CSS_COLOR_TEAM_YELLOW, team  );
	}
	
	if ( team_number + 1 > team_count ) {
		team_count = team_number + 1;
	}
}

void create_new_team_from_name( char *team )
{
	int team_number;

	if ( 0 == strcmp( team, "RED" ) ) {
		team_number = 1;
	}
	if ( 0 == strcmp( team, "BLUE" ) ) {
		team_number = 2;
	}
	if ( 0 == strcmp( team, "GREEN" ) ) {
		team_number = 3;
	}
	if ( 0 == strcmp( team, "YELLOW" ) ) {
		team_number = 4;
	}
	
	create_new_team_from_number( team_number );
	
}

int select_team_by_number( char *team_number )
{
	static int result=-1;
	
	if ( strlen( team_number ) > 0 ) {
		result = atoi( team_number );

		if ( 0 == strlen( TEAMS[ result ].name ) ) {
			create_new_team_from_number( result );
			
		}
		//if ( result >= team_count ) {
		//	team_count = result + 1;
		//}		
		
	}

	return result;

}
#endif

team_stats *sorted_team( int t )
{
	return &(TEAMS [ team_index [ t ] ]);
}

// has to be called after determining clantags
// todo: call it
void post_process_teams( void )
{
	int t;

	for ( t = 0 ; t < team_count ; t++ ) {
		if ( team_participated( t ) ) {
			active_teams++;
			determine_team_filename( t );
		}
	}
}

void determine_teams( void )
{
	int i,j,k,t;
	int team_not[ MAX_TEAMS ];
#if defined(SERVER_STATS)
	return; // teams are already known
#endif

	if ( team_count == 1 ) {
		// if there was only 1 flagtouch for instance, only 1 team(color) is identified
		// in ACTF or Old CTF
		create_extra_team(); 
	}

	// a player's buddies and enemies are marked during processing,

	// for each player data is gathered :
	// in which teams he's not
	// who are his enemies ( by kills )
	// who are his teammates ( by teamkills )
	// and possibly which team he's in.
	for ( i = 0; i < player_count ; i++ ) {
		for ( j = 0; j < player_count ; j ++ ) {
			
			if ( frag_matrix[ i ][ j ].same_team == PLAYERS_SAME_TEAM ) {
				mark_relation( i, PLAYERS_SAME_TEAM, j, PLAYERS_SAME_TEAM );
			}

			if ( frag_matrix[ i ][ j ].same_team == PLAYERS_DIFFERENT_TEAM ) {
				mark_relation( i, PLAYERS_SAME_TEAM, j, PLAYERS_DIFFERENT_TEAM );
			}
		}
	}
	
	// at this point there can still be unknown player relations, this may be caused
	// by off and def squads not tk'ing each other :)). From both off and def squads 
	// the team may be known so the players can hopefully still be sorted.

	// for each player check if his team is known, 
	// if yes, mark all his buddies in same team
	for ( i = 0; i < player_count ; i++ ) {
		for ( j = 0; j < player_count ; j ++ ) {
			if ( PLAYERS[ i ].team != -1 ) {
				if (PLAYERS[ i ].team == PLAYERS[ j ].team) {
					mark_relation( i, PLAYERS_SAME_TEAM, j, PLAYERS_SAME_TEAM );	
				}
			}
			else { // team unknown, check relation
				if ( frag_matrix[ i ][ j ].same_team == PLAYERS_SAME_TEAM ) {
					if ( PLAYERS[ j ].team != -1 ) {
						set_player_team_relation( i, PLAYERS[ j ].team, YES );
					}
				}
			}
		}
	}

	// mark enemies
	for ( i = 0; i < player_count ; i++ ) {
		for ( j = 0; j < player_count ; j ++ ) {
			
			if ( frag_matrix[ i ][ j ].same_team == PLAYERS_DIFFERENT_TEAM ) {
				mark_relation( i, PLAYERS_SAME_TEAM, j, PLAYERS_DIFFERENT_TEAM );
				mark_relation( j, PLAYERS_SAME_TEAM, i, PLAYERS_DIFFERENT_TEAM );
			}	

		}
	}

	// use reverse logic and check what teams a player may possibly be in.
	// if there are players left without a team, then gather for each these players all 
	// the teams that his buddies are not in. Also determine all the teams that have no players.
	// With a bit of guessing things could be worked out.

	// determining the team for a player depends on knowing the team for other players
	// for players that cap a flag we know which team they are not in
	// this tells us which team his teammates are not in and will likely result
	// into having only one team left to put them in, which means we know which team
	// they ARE in.
	// This helps in being able to determine the team for the other teams' players
	
	// this logic is most needed for the old ctf maps

	// todo: remove for-loop now that we use mark_known_player_team() on a found team
	for ( t = 0; t <  (team_count > 2 ? team_count + 1 : 3)  ; t++ ) {
		// run the loop below n times where n is the number of teams found and n >= 2

		for ( i = 0; i < player_count ; i++ ) { 
			if ( PLAYERS[ i ].team == -1 ) { // try each player who's team is unknown

				for ( k = 0; k < team_count ; k++ ) { // initialize team_not array
					team_not[ k ] = PLAYERS[ i ].in_team[ k ];
				}

				for ( j = 0; j < player_count ; j ++ ) { // test relation to every other player

					if ( frag_matrix[ i ][ j ].same_team == PLAYERS_SAME_TEAM ) {
						// these 2 players are in the same team
						// get teams that are marked 'enemy' for the teammate
						for ( k = 0; k < team_count ; k++ ) {
							if ( PLAYERS[ j ].in_team[ k ] == NO ) {
								team_not[ k ] = PLAYERS[ j ].in_team[ k ];
							}
						}
					}

					if ( frag_matrix[ i ][ j ].same_team == PLAYERS_DIFFERENT_TEAM ) {
						// these 2 players are in a different team
						// get team that is marked
						// if the team of the enemy is known then mark this
						if ( PLAYERS[ j ].team != -1 ) {
							team_not[ PLAYERS[ j ].team ] = NO;
						}
					}
				}
				// now we hopefully have a conclusive list of teams this player is not a part of.
				//PLAYERS[ i ].team =deduct_team_reverse_logic( team_not );
				set_team( i, deduct_team_reverse_logic( team_not ) );
				if ( PLAYERS[ i ].team != TEAM_UNKNOWN ) {
					mark_known_player_team( i, PLAYERS[ i ].team );
				}
			}	
		}
	}

	//debug_output_player_relations(); // for debugging puproses
}
int team_participated( int t )
{
	int result = FALSE;

	result = TEAMS[ t ].frags > 0			||
			 TEAMS[ t ].deaths > 0			||
			 TEAMS[ t ].flagtouches > 0		||
			 TEAMS[ t ].teamkills > 0		||
			 TEAMS[ t ].flags_lost > 0		||
			 strlen( TEAMS[ t ].name ) > 0;
	return result;
}

void output_teams_html( void )
{
	int p,t,x;
	int row_init;
	int print_row; // boolean to indicate to add another row to the table

	if ( query_gametype() == GT_DUELL ) {
		return;
	}

	if ( !conflicting_teaminfo ) {

		if ( team_count > 0 ) {

			output_teams_html_element( TM_INIT, 0 );
			
			// output RED and BLUE, todo: more
			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( t = 0; t < team_count; t++ ) { 
				if ( team_participated( sorted_team_nr( t ) ) ) {
					output_teams_html_element( TM_HEADER, sorted_team_nr( t ) );
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );					

			print_row = TRUE;
			// for each player
			for ( p = 0; p < player_count && print_row ; p++ ) {
				// for each team
			
				row_init = FALSE;
				print_row = FALSE; // hm kinda tricky
				// first determine if there's anything left to print
				for ( t = 0; t < team_count; t++ ) {
					if ( team_participated( sorted_team_nr( t ) ) ) {
						if ( sorted_team( t )->player_count > p ) {
							print_row = TRUE;
							break; // exit the for-loop
						}
					}
				}

				if ( print_row ) {
					for ( t = 0; t < team_count; t++ ) {
						
						// todo: test if team was active
						if ( team_participated( sorted_team_nr( t ) ) ) {
							if ( row_init == FALSE ) {
								output_teams_html_element( TM_ROW_INIT, 0 );
								row_init = TRUE;
							}
							if ( sorted_team( t )->player_count > p ) {
								output_teams_html_element( TM_PLAYER, sorted_team( t )->playerlist[ p ] );
							}
							else {
								// print empty line
								output_teams_html_element( TM_PLAYER,  -1 );
							}
						}
						
					}
				}

				if ( row_init ) {
					// terminate the row
					output_teams_html_element( TM_ROW_TERM, 0 );
				}
			
			}

			// create an empty line under the teams
			output_teams_html_element( TM_ROW_INIT, 0 );
			for ( x = 0; x < team_count ; x++ ) {
				if ( team_participated( x ) ) {
					output_teams_html_element( TM_LINE_EMPTY, 0 );
				}
			}
			output_teams_html_element( TM_ROW_TERM, 0 );

			// output touches, caps etc.
			output_teams_objectives_html();
			
			if ( query_gametype() == GT_CAPTURE_THE_FLAG ) {
				// output DEF squad stats		
				output_squad_def_statistics_html();
		
				// output OFF squad stats
				output_squad_off_statistics_html();
			}

			// output FRAGS 'N DEATHS stats
			output_teams_fnd_statistics_html();

			output_teams_html_element( TM_TERM, 0);
		}
	}
}

// todo: use this function
/*
void process_player_team_info( DEDUCT_PLAYER_TEAM info )
{
	switch( info ) {
		case PLAYER_IN_TEAM	:
			break;
		case PLAYER_NOT_IN_TEAM :
			break;
		case PLAYER_IN_TEAM_FLAG :
			break;
		case PLAYER_NOT_IN_TEAM_FLAG :
			break;
	}
}
*/

/*
void output_teaminfo_detailed( int i )
{

// todo: put the info below back in somehow

	team_stats *t;
	int w;
	long times_taken; // nr of times it was taken from starting position
	double perc_capped; // percentage of 1st touches that resulted into cap
	double ratio;

	t = &TEAMS[ i ];

	sort_weapons( t->frags_per_weapon );
	sprintf( outbuf, "\nFrags per weapon :\n" );
	AS_OUT( OE_LINE, outbuf );
	
	w = W_UNKNOWN + 1;
	while ( t->frags_per_weapon[ weapon_index[ w ] ] > 0 && w < W_NR_WEAPONS ) {
		output_weaponfrags_html( OBJ_PROCESS, weapon_name( weapon_index[ w ] ), t->frags_per_weapon[ weapon_index[ w ] ] );
		w++;
	}

	output_weaponfrags_html( OBJ_TERM, NULL, 0 );

	sprintf( outbuf, "\n\n" );
	AS_OUT( OE_LINE, outbuf );
}
*/
