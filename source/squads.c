#include <stdio.h>
#include "sorting.h"
#include "squads.h"
#include "teams.h"


/*****************************************************************************
**                                                                          **
**  Public variables                                                        **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
*****************************************************************************/
squad_stats  SQUADS [ MAX_TEAMS * 2 ];

/*****************************************************************************
**                                                                          **
**  Private variables                                                       **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**                                                                          **
**  Private Functions                                                       **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
*****************************************************************************/

#if defined(_DEBUG)
void debug_output_players( int i )
{
	int p;
	int sp;
	switch( i ) {
		case 0: 
			printf( "Role factor\n" );
			break;
		case 1:
			printf( "Enemy factor\n" );
			break;
	}

	for ( p = 0; p < player_count ; p++ ) {
		sp = player_index[ p ];
		switch( i ) {
			case 0: 
				printf( "%30.30s:%5.2lf\n", PLAYERS[ sp ].nameclean, PLAYERS[ sp ].role_factor );
				break;
			case 1:
				printf( "%30.30s: %5ld\n", PLAYERS[ sp ].nameclean, PLAYERS[ sp ].enemy_factor );
				break;

		}
		
	}
}
#endif
// pre: the squads need to have players in them before this is called
void calculate_squad_statistics( void )
{
	int squad, team, player, player2;
	int player_squad, player_team;
	int w;
	squad_stats *sq;

	for ( team = 0; team < team_count; team++ ) {
		for ( squad = 0; squad < MAX_SQUADS; squad++ ) {

			sq = &TEAMS[ team ].squad[ squad ];
			for ( player = 0; player < player_count; player++ ) {

				player_squad = PLAYERS[ player ].squad;
				player_team  = PLAYERS[ player ].team;

				if ( player_squad == squad && player_team == team ) {
					sq->deaths   += PLAYERS[ player ].deaths;
					sq->frags    += PLAYERS[ player ].frags;
					sq->suicides += PLAYERS[ player ].suicides;
					sq->captures += PLAYERS[ player ].flags_captured;
					sq->killed_flagcarrier  += PLAYERS[ player ].killed_flagcarrier;
					
					for ( w = 0; w < W_NR_WEAPONS; w++ ) {
						sq->frags_per_weapon[ w ] += PLAYERS[ player ].frags_per_weapon[ w ];
					}

					for ( player2 = 0; player2 < player_count; player2++ ) {
						if ( player != player2 &&
							PLAYERS[ player2 ].team == player_team && 
							PLAYERS[ player2 ].squad == player_squad ) {

							sq->internal_tks += frag_matrix[ player ][ player2 ].frags;
						}
					}
				}
			}
		}
	}
}

// in this function the obvious def and off players are detected
int player_in_squad_fase1( int p )
{
	int result = SQUAD_UNKNOWN;

	// first sort out the obvious off and def players
	if ( ( PLAYERS[ p ].def_points > PLAYERS[ p ].off_points ) &&
		( PLAYERS[ p ].def_points_indirect > PLAYERS[ p ].off_points_indirect ) ) {
		result = SQUAD_DEF;
	}
	if ( ( PLAYERS[ p ].off_points > PLAYERS[ p ].def_points ) &&
		( PLAYERS[ p ].off_points_indirect > PLAYERS[ p ].def_points_indirect ) ) {
		result = SQUAD_OFF;
	}

	return result;
}

// check if a player has significantly more off than def points
int player_in_squad_fase2( int p )
{
	int result = SQUAD_UNKNOWN;

	// sometimes there seems to be a bit much indirect def points inflation,
	// this should look beyond that
	if ( PLAYERS[ p ].off_points > 0 && PLAYERS[ p ].def_points > 0 ) {
		if ( ( PLAYERS[ p ].off_points / PLAYERS[ p ].def_points ) > 6 ) {
			// this means if a player has 6 times more off points than def points we say he's off
			result = SQUAD_OFF;
		}
	}
	else {
		// todo: ?
	}

	return result;
}

int player_in_squad_fase3( int p )
{
	int result = SQUAD_UNKNOWN;
	long def = 0;
	long off = 0;
	int i;
	
	// todo : for each player count the number of enemies
	// the players with small amount of different enemies have played their role properly
	// only do this when there are only a few players with unknown squad
	// also count the number of different off and def events and maybe decide from that
	//return result; // todo: test, remove

	for ( i = 0; i < player_count; i++ ) {
		if ( PLAYERS[ p ].team != PLAYERS[ i ].team ){
			if ( PLAYERS[ i ].squad == SQUAD_DEF ) {
				off = off + frag_matrix[ p ][ i ].frags;
				off = off + frag_matrix[ i ][ p ].frags;
			}

			if ( PLAYERS[ i ].squad == SQUAD_OFF ) {
				def = def + frag_matrix[ p ][ i ].frags;
				def = def + frag_matrix[ i ][ p ].frags;
			}
		}
		else { // check teamkills too
			if ( PLAYERS[ i ].squad == SQUAD_DEF ) {
				def = def + frag_matrix[ p ][ i ].frags;
				def = def + frag_matrix[ i ][ p ].frags;
			}

			if ( PLAYERS[ i ].squad == SQUAD_OFF ) {
				off = off + frag_matrix[ p ][ i ].frags;
				off = off + frag_matrix[ i ][ p ].frags;
			}

		}
	}

	if ( off > def ) {
		result = SQUAD_OFF;
	}
	if ( def > off ) {
		result = SQUAD_DEF;
	}

	return result;
}

// before this function squads fase1 has to be done
void calculate_acquaintances_squad_known( void )
{
	int p, q;

	for ( p = 0; p < player_count; p++ ) {
		PLAYERS[ p ].acquaintances_squad_known = 0;
		for ( q = 0; q < player_count; q++ ) {
			if ( p != q ) {
				if ( frag_matrix[ p ][ q ].frags > 0 || frag_matrix[ q ][ p ].frags > 0 ) {
					if ( PLAYERS[ q ].squad != SQUAD_UNKNOWN ) {
						// we found an acquantance whos squad is known
						PLAYERS[ p ].acquaintances_squad_known++;
					}
				}
			}
		}
	}

}


/*****************************************************************************
**                                                                          **
**  Public Functions                                                        **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
**                                                                          **
*****************************************************************************/


// return name of squad for player p
// has to be determined before this function is called

char *query_squadname( int p ) {

	switch( PLAYERS[ p ].squad  ) {

		case SQUAD_DEF: return "DEF";

		case SQUAD_OFF: return "OFF";
	};

	return "---";
}

void fill_squad_info( void )
{
	int j,s ; 

	// fase1, decide squad by direct and indirect points
	for ( j = 0; j < player_count; j++ ) {
		PLAYERS[ j ].squad = player_in_squad_fase1( j );
	}

	// fase2, decide squad by direct and indirect points
	for ( j = 0; j < player_count; j++ ) {
		if ( PLAYERS[ j ].squad == SQUAD_UNKNOWN ) {
			PLAYERS[ j ].squad = player_in_squad_fase2( j );
		}
	}
	
	// fase3
	// handle players that have squad unknown after fase1
	// order players by nr of enemies that have squad-known
	for ( j = 0; j < player_count; j++ ) {
		calculate_acquaintances_squad_known();
		sort_players( BY_ACQUAINTANCES_SQUAD_KNOWN );
		s = player_index [ 0 ]; // yes always take the first player after sorting
		if ( PLAYERS[ s ].squad == SQUAD_UNKNOWN ) {
			PLAYERS[ s ].squad = player_in_squad_fase3( s );
		}
		else {
			break; // leave the for-loop, all playersquads are known
		}
	}
	
	calculate_squad_statistics();	
}



// experimental stuff below

// parameters: team, squad, player
// return frags + deaths of squad vs player
// pre: teams have to be known
long calculate_enemy_factor( int t, int s, int p )
{
	int x;
	long enemy_factor = 0;
	
	for ( x = 0; x < player_count; x++ ){
		if ( PLAYERS[ x ].team == t && PLAYERS[ x ].squad == s ) {
			if ( ( x != p ) &&  ( PLAYERS[ x ].team != PLAYERS[ p ].team ) ) {
				enemy_factor += frag_matrix[ x ][ p ].frags + frag_matrix[ p ][ x ].frags;
			}
		}
	}

	return enemy_factor;
}
void initialize_enemy_factor( int t, int s )
{
	int p;

	for ( p = 0; p < player_count; p++ ){
		if ( PLAYERS[ p ].squad == SQUAD_UNKNOWN ) {
			// we are only looking for players that have unknown squad
			PLAYERS[ p ].enemy_factor = calculate_enemy_factor( t, s, p );
		}
		else {
			PLAYERS[ p ].enemy_factor = 0;
		}
	}
}

// todo: return worst enemy of a squad
// t = team, s = squad
int squad_worst_squadless_enemy( int t, int s )
{
	int result = -1;

	initialize_enemy_factor( t, s );
	sort_players( BY_ENEMY_FACTOR );
#if defined(_DEBUG)
	debug_output_players( 1 );
#endif
	if ( sorted_player( 0 )->enemy_factor > 0 ) {
		result = player_index[ 0 ];
	}
	
	return result;
}

void initialize_role_factor( void )
{
	int p, q;
	long sum_encounters;
	long encounters;
	double factor;
	int enemies;

	for ( p = 0; p < player_count; p++ ) {

		sum_encounters = 0;
		enemies = 0;
		factor = 0.0;

		for ( q = 0; q < player_count; q++ ) {
			if ( PLAYERS[ p ].team != PLAYERS[ q ].team ) {
				encounters = frag_matrix[ p ][ q ].frags + frag_matrix[ q ][ p ].frags;
				if ( encounters > 0 ) {
					enemies++;
					sum_encounters += encounters;
				}
			}
		}

		if ( enemies > 0 ) {
			factor = (double)sum_encounters / enemies;
		}

		PLAYERS[ p ].role_factor = factor;
	}
}
// todo: determine squads without flagaction
// use function squad_worst_enemy()
void fill_squad_info_dm_based( void )
{
	int p;
	int enemy;
	//int team;
	//int squad;
	int player;

	for ( p = 0; p < player_count; p++ ) {
		PLAYERS[ p ].squad = SQUAD_UNKNOWN;
	}

	initialize_role_factor();
	sort_players( BY_ROLE_FACTOR );
#if defined(_DEBUG)
	debug_output_players( 0 );
#endif
	
	player = player_index[ 0 ];
	PLAYERS[ player ].squad = 2;

	for ( p = 0; p < player_count; p++ ) {
		enemy = squad_worst_squadless_enemy( PLAYERS[ player ].team, PLAYERS[ player ].squad );
		if ( enemy >= 0 ) {
			PLAYERS[ enemy ].squad = 2;
			player = enemy;
		}
		else {
			break;
		}
	}
	
}

