#include <stdio.h>
#include <string.h>
#include "agostats.h"
#include "game.h"
#include "matching.h"
#include "players.h"
#include "stathandling.h"
#include "flagtracking.h"
#include "tools.h"
#include "highscores.h"
#include "events.h"


// Functions

void initialize_frag_matrix( void )
{
	int i, j, k;

	for ( i = 0; i < MAX_PLAYERS  ; i++ ) {
		for ( j = 0; j < MAX_PLAYERS ; j++ ) {
			frag_matrix[ i ][ j ].frags = 0;
			frag_matrix[ i ][ j ].fragged_sentry = 0;
			frag_matrix[ i ][ j ].fragged_supplystation = 0;

			if ( i != j ) {
				frag_matrix[ i ][ j ].same_team = TEAM_RELATION_UNKNOWN;
			}
			else {
				// players relation to himself
				frag_matrix[ i ][ j ].same_team = PLAYERS_SAME_TEAM; // todo:
				frag_matrix[ i ][ j ].same_team = TEAM_RELATION_UNKNOWN;
			}
			for ( k = 0; k < W_NR_WEAPONS; k++ ) {
				frag_matrix[ i ][ j ].frags_per_weapon[ k ] = 0;
				frag_matrix[ i ][ j ].frags_on_flagcarrier_per_weapon [ k ] = 0;
			}
		}
	}
}

void clear_player_table( void )
{
	int i;

	for ( i = 0; i < MAX_PLAYERS ; i ++ ) {
		init_player( i );	
	}

	player_count = 0;
}

void reset_statistics( void )
{
	current_player	     = -1;
	current_team	     = -1;
	current_flag         = -1;
	conflicting_teaminfo = FALSE;
	first_blood_awarded  = FALSE;
	first_blood_killer   = NO_PLAYER;
	first_blood_victim   = NO_PLAYER;

	set_gametype( GT_UNKNOWN );
	//map[ 0 ] = '\0';

	init_match_totals();
	initialize_frag_matrix();
	clear_player_table();
	clear_team_table();
	clear_flag_table();
	clear_events();

}

void move_player( int player_to, int player_from )
{
	int p;

	for ( p = 0; p < player_count ; p++ ) {
		if ( p != player_to && p != player_from ) {
			// adjust row in fragmatrix
			memcpy( &frag_matrix[ p ][ player_to ], &frag_matrix[ p ][ player_from ], sizeof( relation ) );

			// adjust column in fragmatrix
			memcpy( &frag_matrix[ player_to ][ p ], &frag_matrix[ player_from ][ p ], sizeof( relation ) );
		}
	}
	
	// copy suicide stats
	memcpy( &frag_matrix[ player_to ][ player_to ], &frag_matrix[ player_from ][ player_from ], sizeof( relation ) );
	
	memcpy( &PLAYERS[ player_to ], &PLAYERS[ player_from ], sizeof( player_stats ) );
	
	// capture history
	for ( p = 0; p < teamscore_event_count; p++ ) {
		if ( teamscore_history[ p ].scoring_player == player_from ) {
			teamscore_history[ p ].scoring_player = player_to;
		}
	}

	// first blood
	if ( first_blood_awarded ) {
		if ( first_blood_killer == player_from ) {
			first_blood_killer = player_to;
		}
		if ( first_blood_victim == player_from ) {
			first_blood_victim = player_to;
		}
	}
	
}

void remove_player( int player )
{
// remove_player( 3 )
// before:				after:
//	player1				player1
//	player2				player2
//	player3				player4
//	player4				player5
//  player5
	int p;

	for ( p = player; p < player_count - 1; p++ ) {
		move_player( p, p + 1 );
	}
	player_count--;
}


// pre: only merge players that are assumed to be in the same team
void merge_players( int player1, int player2 )
{
// merge player( 3, 5 )
// before:				after:
//	player1				player1
//	player2				player2
//	player3				player3 + 5
//	player4				player4
//  player5
	int p1, p2, px;
	int w, t;

	if ( player1 == player2 ) {
		return;
	}

	if ( player1 < player2 ){
		p1 = player1;
		p2 = player2;
	}
	else {
		p1 = player2;
		p2 = player1;
	}
	
	// consolidate scores with other players
	for ( px = 0; px < player_count; px ++ ) {
		
		if ( px != p1 && px != p2 ) {
			// update row in fragmatrix
			frag_matrix[ px ][ p1 ].fragged_sentry         += frag_matrix[ px ][ p2 ].fragged_sentry;
			frag_matrix[ px ][ p1 ].fragged_supplystation  += frag_matrix[ px ][ p2 ].fragged_supplystation;
			frag_matrix[ px ][ p1 ].frags                  += frag_matrix[ px ][ p2 ].frags;
			for ( w = 0; w < W_NR_WEAPONS; w++ ) {
				frag_matrix[ px ][ p1 ].frags_per_weapon[ w ] += frag_matrix[ px ][ p2 ].frags_per_weapon[ w ];
			}

			// update column in fragmatrix
			frag_matrix[ p1 ][ px ].fragged_sentry        += frag_matrix[ p2 ][ px ].fragged_sentry;
			frag_matrix[ p1 ][ px ].fragged_supplystation += frag_matrix[ p2 ][ px ].fragged_supplystation;
			frag_matrix[ p1 ][ px ].frags                 += frag_matrix[ p2 ][ px ].frags;
			for ( w = 0; w < W_NR_WEAPONS; w++ ) {
				frag_matrix[ p1 ][ px ].frags_per_weapon[ w ] += frag_matrix[ p2 ][ px ].frags_per_weapon[ w ];
			}

			if ( frag_matrix[ px ][ p1 ].same_team == TEAM_RELATION_UNKNOWN && 
				 frag_matrix[ px ][ p2 ].same_team != TEAM_RELATION_UNKNOWN ) {

				frag_matrix[ px ][ p1 ].same_team = frag_matrix[ px ][ p2 ].same_team;
				frag_matrix[ p1 ][ px ].same_team = frag_matrix[ px ][ p2 ].same_team;
			}
		}
	}

	// handle suicidal stuff from the fragmatrix, almost forgot this
	frag_matrix[ p1 ][ p1 ].fragged_sentry         += frag_matrix[ p2 ][ p2 ].fragged_sentry;
	frag_matrix[ p1 ][ p1 ].fragged_supplystation  += frag_matrix[ p2 ][ p2 ].fragged_supplystation;
	frag_matrix[ p1 ][ p1 ].frags                  += frag_matrix[ p2 ][ p2 ].frags;
	for ( w = 0; w < W_NR_WEAPONS; w++ ) {
		frag_matrix[ p1 ][ p1 ].frags_per_weapon[ w ] += frag_matrix[ p2 ][ p2 ].frags_per_weapon[ w ];
	}

	// add values
	PLAYERS[ p1 ].flags_captured            += PLAYERS[ p2 ].flags_captured;
	PLAYERS[ p1 ].coast_to_coast            += PLAYERS[ p2 ].coast_to_coast ;
	PLAYERS[ p1 ].deaths                    += PLAYERS[ p2 ].deaths;
	PLAYERS[ p1 ].def_points                += PLAYERS[ p2 ].def_points;
	PLAYERS[ p1 ].def_points_indirect       += PLAYERS[ p2 ].def_points_indirect;
	PLAYERS[ p1 ].defend_flag_at_base       += PLAYERS[ p2 ].defend_flag_at_base;
	PLAYERS[ p1 ].defend_flag_in_field      += PLAYERS[ p2 ].defend_flag_in_field;
	PLAYERS[ p1 ].defend_flagcarrier        += PLAYERS[ p2 ].defend_flagcarrier;
	PLAYERS[ p1 ].destroyed_sentry          += PLAYERS[ p2 ].destroyed_sentry;
	PLAYERS[ p1 ].destroyed_supply          += PLAYERS[ p2 ].destroyed_supply;
	PLAYERS[ p1 ].first_touches             += PLAYERS[ p2 ].first_touches;
	PLAYERS[ p1 ].flagtouches               += PLAYERS[ p2 ].flagtouches;
	PLAYERS[ p1 ].frags                     += PLAYERS[ p2 ].frags; 
	PLAYERS[ p1 ].killed_flagcarrier        += PLAYERS[ p2 ].killed_flagcarrier;
	PLAYERS[ p1 ].killed_near_flag_in_base  += PLAYERS[ p2 ].killed_near_flag_in_base;
	PLAYERS[ p1 ].killed_near_flag_in_field += PLAYERS[ p2 ].killed_near_flag_in_field;
	PLAYERS[ p1 ].linecount                 += PLAYERS[ p2 ].linecount;
	PLAYERS[ p1 ].lost_sentry               += PLAYERS[ p2 ].lost_sentry ;
	PLAYERS[ p1 ].lost_supply               += PLAYERS[ p2 ].lost_supply;
	PLAYERS[ p1 ].max_fragstreak             = my_max( PLAYERS[ p1 ].max_fragstreak, PLAYERS[ p2 ].max_fragstreak );
	PLAYERS[ p1 ].off_points                += PLAYERS[ p2 ].off_points;
	PLAYERS[ p1 ].off_points_indirect       += PLAYERS[ p2 ].off_points_indirect;
	PLAYERS[ p1 ].sit_on_grenade            += PLAYERS[ p2 ].sit_on_grenade;
	PLAYERS[ p1 ].suicides                  += PLAYERS[ p2 ].suicides ;
	PLAYERS[ p1 ].teamdeaths                += PLAYERS[ p2 ].teamdeaths;
	PLAYERS[ p1 ].teamkilled_flagcarrier    += PLAYERS[ p2 ].teamkilled_flagcarrier;
	PLAYERS[ p1 ].teamkilled_sentry         += PLAYERS[ p2 ].teamkilled_sentry;
	PLAYERS[ p1 ].teamkills                 += PLAYERS[ p2 ].teamkills;
	
	// special stuff
	for ( w = 0; w < W_NR_WEAPONS; w++ ) {
		PLAYERS[ p1 ].frags_per_weapon [ w ]               += PLAYERS[ p2 ].frags_per_weapon  [ w ];
		PLAYERS[ p1 ].deaths_per_weapon[ w ]               += PLAYERS[ p2 ].deaths_per_weapon [ w ];
		PLAYERS[ p1 ].fragged_flagcarrier_per_weapon [ w ] += PLAYERS[ p2 ].fragged_flagcarrier_per_weapon [ w ];
	}
	// update teamscore history
	for ( t = 0; t < teamscore_event_count; t++ ) {
		if ( teamscore_history[ t ].scoring_player == p2 ) {
			teamscore_history[ t ].scoring_player = p1;
		}
	}

	// copy teaminfo
	for ( t = 0; t < MAX_TEAMS ; t++ ) {
		if ( PLAYERS[ p1 ].in_team[ t ] == UNKNOWN && PLAYERS[ p2 ].in_team[ t ] != UNKNOWN ) {
			PLAYERS[ p1 ].in_team[ t ] = PLAYERS[ p2 ].in_team[ t ];
		}
	}
	if ( PLAYERS[ p1 ].team == TEAM_UNKNOWN && PLAYERS[ p2 ].in_team[ t ] != TEAM_UNKNOWN ) {
		set_team( p1, PLAYERS[ p2 ].team );
	}
	
	// first line and last line
	if ( PLAYERS[ p2 ].first_line < PLAYERS[ p1 ].first_line ) {
		PLAYERS[ p1 ].first_line = PLAYERS[ p2 ].first_line;
	}
	if ( PLAYERS[ p2 ].last_line > PLAYERS[ p1 ].last_line ) {
		PLAYERS[ p1 ].last_line = PLAYERS[ p2 ].last_line;
	}

	// first blood
	if ( first_blood_awarded ) {
		if ( first_blood_killer == p2 ) {
			first_blood_killer = p1;
		}
		if ( first_blood_victim == p2 ) {
			first_blood_victim = p1;
		}
	}
	
	PLAYERS[ p1 ].enabled = TRUE;

	remove_player( p2 );
}

// identical players have the same cleanname and are potentially in the same team
int players_identical( int p1, int p2 )
{
	
	if ( p1 == p2 ) {
		// trivial
		return TRUE;
	}

	if ( 0 != strcmp( PLAYERS[ p1 ].nameclean, PLAYERS[ p2 ].nameclean ) ) {
		// their names do not match
		return FALSE;
	}

	if ( PLAYERS[ p1 ].team != UNKNOWN && PLAYERS[ p2 ].team != UNKNOWN && PLAYERS[ p1 ].team != PLAYERS[ p2 ].team ) {
		// both players teams are known, and they are not the same
		return FALSE;
	}

	// identical players cannot be in the game at the same time, so they cannot frag each other
	if ( frag_matrix[ p1 ][ p2 ].frags != 0                 || 
		 frag_matrix[ p2 ][ p1 ].frags != 0                 ||
		 frag_matrix[ p1 ][ p2 ].fragged_sentry != 0        ||
		 frag_matrix[ p2 ][ p1 ].fragged_sentry != 0        ||
		 frag_matrix[ p1 ][ p2 ].fragged_supplystation != 0 ||
		 frag_matrix[ p2 ][ p2 ].fragged_supplystation != 0 ) {
			 
		return FALSE;
	}

    // todo: compare relation to other players
	

	return TRUE;
}

int find_identical_players( int *p1, int *p2 )
{
	int pa, pb;

	for ( pa = 0; pa < player_count; pa++ ) {
		for ( pb = pa + 1; pb < player_count; pb++ ) {
			if ( players_identical( pa, pb ) ) {
				*p1 = pa;
				*p2 = pb;
				return TRUE;
			}
		}
	}
	return FALSE;
}

int find_inactive_player( int *player ){
	int p;

	for ( p = 0; p < player_count; p++ ) {
		if ( player_participated( p ) == FALSE ) {
			*player = p;
			return TRUE;
		}
	}

	return FALSE;
}

void remove_inactive_players( void )
{
	int p;

	while ( find_inactive_player( &p ) ) {
		remove_player( p );
	}
	
}

void merge_identical_players( void )
{
	int p1,p2;
 
	while ( find_identical_players( &p1, &p2 ) ) {
		merge_players( p1, p2 );
	}
}

void cleanup_stats( void )
{
	remove_inactive_players();
	merge_identical_players();
}
