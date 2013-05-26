#include <string.h>
#include "agostats.h"
#include "game.h"
#include "flagtracking.h"
#include "players.h"
#include "tools.h"

// public variables
flag_stats FLAGS [ MAX_FLAGS ];
int        flag_count;
int        current_flag;
// private variables

// private functions
void init_flag( int i )
{
	FLAGS[ i ].capped = 0;
	FLAGS[ i ].carrier = NO_PLAYER;
	FLAGS[ i ].current_alive_touches = 0;
	FLAGS[ i ].currently  = FS_BASE; // current flagstatus
	FLAGS[ i ].max_touches_capped = 0;
	FLAGS[ i ].max_touches_saved = 0;
	FLAGS[ i ].name[ 0 ] = '\0';
	FLAGS[ i ].previously = FS_UNKNOWN; // previous flagstatus
	FLAGS[ i ].saved = 0;
}

// public functions
void clear_flag_table( void )
{
	int i;

	for ( i = 0; i < MAX_FLAGS ; i++ ) {
		init_flag( i );
	}

	flag_count = 0;
}

void create_new_flag( char *flag ) 
{	
	init_flag( flag_count );
	strcpy( FLAGS[ flag_count].name, flag );
	
	flag_count++;
}

int select_flag( char *flag )
{
	static int result;
	int i = 0;
	int found = FALSE;
	char *p;

	//p = strupr( plain_text( flag ) );
	p = plain_text( flag );
	string_to_upper( p );

	while ( !found && i < flag_count ) {
		if ( 0 == strcmp( p, FLAGS[ i ].name ) ){
			found = TRUE;
			result = i;
		}
		i++;
	}

	if ( !found ) {
		create_new_flag( p );
		
		result = flag_count - 1;
	}

	return result;
}

// the flagstatus is determined by a flag, a status and possibly 2 players (carrier and killer)
void set_flagstatus( int flagstatus, int flag_nr, int flagcarrier )
{

/*
**      -----Base------<------
**     |      |               |
**     |     \|/              |
**     |      |               | 
**     |    Carried--->---Captured
**     |    |    |
**     |   \|/  /|\
**     |    |    |
**      -<--Field
**
*/
	int prev;
	
	// process queue like
	FLAGS[ flag_nr ].previously = FLAGS[ flag_nr ].currently;
	FLAGS[ flag_nr ].currently = flagstatus;

	prev = FLAGS[ flag_nr ].previously;

	switch ( flagstatus ) {

		case FS_CARRIED  : // this is considered a flagtake
			FLAGS[ flag_nr ].carrier = flagcarrier;
			FLAGS[ flag_nr ].current_alive_touches++;

			// update playerscore
			if ( FLAGS[ flag_nr ].current_alive_touches == 1 ) {
				update_player_score( PE_FIRST_TOUCH, flagcarrier, 1 );
			}
			else {
				update_player_score( PE_FLAG_TOUCH, flagcarrier, 1 );
			}
			break;
	
		case FS_FIELD    :
			FLAGS[ flag_nr ].carrier = NO_PLAYER;
			break;

		case FS_CAPTURED :

			if ( FLAGS[ flag_nr ].current_alive_touches == 1 ) {
				// coast to coast capture
				//PLAYERS[ flagcarrier ].coast_to_coast++;
				update_player_score( PE_COAST_TO_COAST, flagcarrier, 1 );
			}
			else {
				update_player_score( PE_FLAG_CAPTURE, flagcarrier, 1 );
			}

			if ( FLAGS[ flag_nr ].current_alive_touches > FLAGS[ flag_nr ].max_touches_capped ) {
				FLAGS[ flag_nr ].max_touches_capped = FLAGS[ flag_nr ].current_alive_touches;
			}
			FLAGS[ flag_nr ].capped++;
			// this should be taken care of in FS_BASE, but maybe the "flag returned to base" message
			// won't come if something is wrong in a map
			FLAGS[ flag_nr ].carrier = NO_PLAYER;
			FLAGS[ flag_nr ].current_alive_touches = 0;
			break;

		case FS_BASE :
			if ( prev == FS_FIELD ) { // saved
				if ( FLAGS[ flag_nr ].current_alive_touches > FLAGS[ flag_nr ].max_touches_saved ) {
					FLAGS[ flag_nr ].max_touches_saved = FLAGS[ flag_nr ].current_alive_touches;
				}
				FLAGS[ flag_nr ].saved++; // update saved counter
			}
			FLAGS[ flag_nr ].carrier = NO_PLAYER;
			FLAGS[ flag_nr ].current_alive_touches = 0; // reset
			break;
	}
}

// return if player is carrying a flag and which one he's carrying
// if it is ever needed to return he's carrying more than 1 flag i will make another function for that
// return true if player is carrying
int player_carrying_flag( int player_nr, int *flag )
{
	int result = FALSE;
	int f;
	
	for ( f = 0; f < MAX_FLAGS; f++ ) {
		if ( FLAGS[ f ].carrier == player_nr ) {
			result = TRUE;
			*flag = f;
			break;
		}
	}

	return result;
}

