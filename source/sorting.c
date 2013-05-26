#include <stdlib.h>
#include <string.h>
#include "sorting.h"
#include "players.h"
#include "weapons.h"


// public variables



// private functions

long sort_compare( void *val1, void *val2, int type )
{
	long result;
	double dresult;

	// return -x, 0 , x
	switch ( type ) {
		case T_INTEGER :
			result = *(int*)val2 - *(int*)val1;
			break;
		case T_LONG :
			result = *(long*)val2 - *(long*)val1;
			break;
		case T_DOUBLE :
			dresult = (double)*((double*)val2) -(double)*((double*)val1);
			result = 0;
			if ( dresult > 0.0 ) result = 1;
			if ( dresult < 0.0 ) result = -1;
			break;
		case T_STRING:
			result = strcmp( (char*)val1, (char*)val2 );
			break;
	}

	return result;
}

void sort_swap_teams( int t1, int t2 )
{
	int temp;

	temp = team_index[ t1 ];
	team_index[ t1 ] = team_index[ t2 ];
	team_index[ t2 ] = temp;
}


int sort_2_teams( int team1, int team2, SORT_ORDER order )
{
	int result;
	team_stats *t1, *t2;

	result = 0;

	t1 = &TEAMS[ team_index [ team1 ] ];
	t2 = &TEAMS[ team_index [ team2 ] ];

	switch( order ) {
		case BY_SCORE:
			result = sort_compare( &t1->score, &t2->score, T_LONG );
			if ( 0 < result ) {
				sort_swap_teams( team1, team2 );
			}
			break;
	}

	return result;
}


void sort_teams( SORT_ORDER order )
{
	int t1, t2;

	// init index;
	for ( t1 = 0; t1 < MAX_TEAMS; t1++) {
		team_index[ t1 ] = t1;
	}

	for ( t1 = 0; t1 < MAX_TEAMS; t1++ ) {
		for ( t2= t1 + 1; t2 < team_count; t2++ ) {
			sort_2_teams( t1, t2, order );
		}
	}	
}

void sort_swap( int i, int j)
{
	int temp;

	temp = player_index[ i ];
	player_index[ i ] = player_index[ j ];
	player_index[ j ] = temp;
}

// public functions
void* push_player_index( void )
{
	void *temp_storage;

	temp_storage = malloc( sizeof( player_index ) );
	memcpy( temp_storage, player_index, sizeof( player_index ) );

	return temp_storage;
}

void pop_player_index( void *temp_storage )
{
	memcpy( player_index, temp_storage, sizeof( player_index ) );
	free( temp_storage );
}

long sort_2_players( int i, int j, int order )
{
	// sort 2 players by order
	int result;
	player_stats *p1, *p2;

	result = 0;

	p1 = &(PLAYERS[ player_index[ i ]]);
	p2 = &(PLAYERS[ player_index[ j ]]);

	switch ( order ) {
		case BY_ACQUAINTANCES_SQUAD_KNOWN :
			// players with unknown squad first
			// and put players with higher number first
			if ( p2->squad == SQUAD_UNKNOWN && p1->squad == SQUAD_UNKNOWN ) {
				result = ( p2->acquaintances_squad_known - p1->acquaintances_squad_known  );
				if ( 0 < result ) {
					sort_swap( i, j );
				}
			}
			
			else  {
				if ( p2->squad == SQUAD_UNKNOWN ) {
					sort_swap( i, j );
				}
			}
			break;
		case BY_CAPTURES:
			result = sort_compare( &p1->flags_captured , &p2->flags_captured ,T_LONG );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_DUELL_SCORE :
			// duell score is frags - suicides
			result = ( &p2->frags - &p2->suicides ) - ( &p1->frags - &p1->suicides );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_ENEMY_FACTOR:
			result = sort_compare( &p1->enemy_factor, &p2->enemy_factor, T_LONG );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_FLAGTOUCHES:
			result = sort_compare( &p1->flagtouches, &p2->flagtouches, T_LONG );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_NAMECLEAN:
			result = sort_compare( &p1->nameclean, &p2->nameclean, T_STRING );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_NAMECLEAN_REV:
			result = sort_compare( &p1->nameclean_rev, &p2->nameclean_rev, T_STRING );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_ROLE_FACTOR:
			result = sort_compare( &p1->role_factor, &p2->role_factor, T_DOUBLE );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_SQUAD:
			result = sort_compare( &p2->squad, &p1->squad, T_INTEGER );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_TEAM:
			result = sort_compare( &p1->team, &p2->team, T_INTEGER );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
		case BY_TEAMSCORE_DESC :
			result = sort_compare( &TEAMS[ p1->team ].score, &TEAMS[ p2->team ].score, T_LONG );
			if ( 0 < result ) {
				sort_swap( i, j );
			}
			break;
	}

	return result;
}

void sort_players( SORT_ORDER order )
{
	int i,j;

	// init index;
	for (i=0; i< player_count; i++) {
		player_index[ i ] = i;
	}

	switch ( order ) {
		case BY_TEAM_BY_NAMECLEAN:
			for (i=0; i< player_count; i++) {
				for ( j= i + 1; j< player_count; j++ ) {
					if ( 0 == sort_2_players( i, j, BY_TEAM ) ) {
						sort_2_players( i, j, BY_NAMECLEAN );	
					}
				}
			}
			break;

		case BY_TEAM_BY_NAMECLEAN_REV:
			for (i=0; i< player_count; i++) {
				for ( j= i + 1; j< player_count; j++ ) {
					if ( 0 == sort_2_players( i, j, BY_TEAM ) ) {
						sort_2_players( i, j, BY_NAMECLEAN_REV );	
					}
				}
			}
			break;

		case BY_TEAM_BY_SQUAD :
			for (i=0; i< player_count; i++) {
				for ( j= i + 1; j< player_count; j++ ) {
					if ( 0 == sort_2_players( i, j, BY_TEAM ) ) {
						sort_2_players( i, j, BY_SQUAD );	
					}
				}
			}
			break;

		case BY_TOUCHES_BY_CAPTURES :
			for (i=0; i< player_count; i++) {
				for ( j= i + 1; j< player_count; j++ ) {
					if ( 0 == sort_2_players( i, j, BY_FLAGTOUCHES ) ) {
						sort_2_players( i, j, BY_CAPTURES );	
					}
				}
			}
			break;

		case BY_CAPTURES_BY_TOUCHES :
			for (i=0; i< player_count; i++) {
				for ( j= i + 1; j< player_count; j++ ) {
					if ( 0 == sort_2_players( i, j, BY_CAPTURES ) ) {
						sort_2_players( i, j, BY_FLAGTOUCHES );	
					}
				}
			}
			break;

		case BY_TEAM_BY_SQUAD_BY_NAMECLEAN :
			for (i=0; i< player_count; i++) {
				for ( j= i + 1; j< player_count; j++ ) {
					if ( 0 == sort_2_players( i, j, BY_TEAM ) ) {
						if ( 0 == sort_2_players( i, j, BY_SQUAD ) ) {
							sort_2_players( i, j, BY_NAMECLEAN );
						}
					}
				}
			}
			break;

		case BY_TEAMSCORE_DESC_BY_TEAM_BY_SQUAD_BY_FLAGTOUCHES_BY_CAPTURES_BY_NAMECLEAN :
			for (i=0; i< player_count; i++) {
				for ( j= i + 1; j< player_count; j++ ) {
					if ( 0 == sort_2_players( i, j, BY_TEAMSCORE_DESC ) ) {
						if ( 0 == sort_2_players( i, j, BY_TEAM ) ) {
							if ( 0 == sort_2_players( i, j, BY_SQUAD ) ) {
								if ( 0 == sort_2_players( i, j, BY_FLAGTOUCHES ) ) {
									if ( 0 == sort_2_players( i, j, BY_CAPTURES ) ) {
										sort_2_players( i, j, BY_NAMECLEAN );
									}
								}
							}
						}
					}
				}
			}
			break;

		default : // do some bubblesorting
			for ( i = 0; i < player_count; i++ ) {
				for ( j= i + 1; j < player_count; j++ ) {
					sort_2_players( i, j, order );
				}
			}
			break;
	}
}
