#include <string.h>
#include <stdio.h>
#include "matching.h"
#include "output.h"
#include "game.h"
#include "players.h"
#include "teams.h"
#include "stathandling.h"

// public variables
int				gametype; // todo: make private ?
int				game_status;
match_stats		MATCH;
char eventbuf	[ LINE_BUFFER_SIZE ];
int  OUTPUT_MM1;  // messagemode1 messages, default off
long game_count;

// private variables

// private functions

// public functions
int is_colorcode( char *et_string )
{
	return ( et_string[0] == '^'  ) && ( et_string[1] != '^' ) ;
}

// this function adjusts the gametype by setting the basic gametype or ORring a type modifier
void set_gametype( long gt )
{
	if ( gt == GT_UNKNOWN ) {
		// set game type to unknown
		gametype = GT_UNKNOWN;
	}
	else {
		if ( GT_UNKNOWN == ( gt & MASK_BASIC_GAMETYPE ) ) {
			// OR the type modifier, see below
		}
		else {
			// set the basic game type, keep the existing type modifier
			gametype = ( gametype & MASK_GAMETYPE_MODIFIER ) | ( gt & MASK_BASIC_GAMETYPE );
		}

		gametype = gametype | ( gt & MASK_GAMETYPE_MODIFIER );			
	}
}

void set_status( long new_status )
{
	game_status = new_status;

#if defined(_DEBUG)
	switch( new_status ) {
		case GS_UNKNOWN:
			printf("set game_status UNKNOWN\n");
			break;
		case GS_PRE_MATCH:
			printf("set game_status PRE MATCH\n");
			break;
		case GS_LIVE:
			printf("set game_status LIVE\n");
			break;
		case GS_POST_MATCH:
			printf("set game_status POST MATCH\n");
			break;
		default:
			printf("set game_status UNDEFINED\n");
			break;
	}
#endif
}

void process_status_change( long new_status )
{
	
	if ( new_status == game_status ) {
		return;
	}

	switch ( new_status ) {

		case GS_UNKNOWN :
			set_status( new_status );
			break;

		case GS_PRE_MATCH:
			if ( game_status == GS_POST_MATCH || game_status == GS_LIVE ) {
				process_game_end();
			}
			if ( searching_last_game ) {
				file_position_game_begin = file_position;
			}
			reset_statistics();
			set_status( new_status );
			process_game_begin();
			break;

		case GS_LIVE:			
			set_status( new_status );
			break;

		case GS_POST_MATCH:			
			set_status( new_status );	
			break;

		default:
			set_status( new_status );	
			break;

	}
}

char *plain_text( char *name )
{
	static char temp[ 1024 ]; // todo: fix this buffer
	int i = 0;
	int j = 0;
	int len = strlen( name );

	// remove color codes ^7 etc.
	temp[ 0 ] = '\0';

	while ( i < len ) {
		if ( name[ i ] != '^' ) {
			temp[ j ] = name [ i ];
			//temp[ j ] = tolower( name [ i ] );
			i++;
			j++;
		}
		else {
			i+=2;
		}
	}
	temp[ j ] = '\0';

	return temp;
}

void mark_match_line( long linenumber )
{
	if ( MATCH.first_line == NO_LINE_NUMBER ) {
		MATCH.first_line = linenumber;
	}
	
	if ( MATCH.last_line < linenumber ) {
		MATCH.last_line = linenumber;
	}

	MATCH.actionlinecount++;
}

void init_match_totals( void )
{
	MATCH.actionlinecount   = 0;
	MATCH.captures          = 0;
	MATCH.destroyed_sentry  = 0;
	MATCH.destroyed_supply  = 0;
	MATCH.first_line        = NO_LINE_NUMBER;
	MATCH.flagtouches       = 0;
	MATCH.frags             = 0;
	MATCH.last_line         = NO_LINE_NUMBER;
	MATCH.players           = 0;
	MATCH.teamkills         = 0;
	MATCH.suicides          = 0;
	MATCH.teams             = 0;
}

void calculate_match_statistics( void )
{
	int p;
	
	MATCH.players = player_count;
	MATCH.teams = team_count;
	
	for ( p = 0; p < player_count; p++ ){
		MATCH.captures			+= PLAYERS[ p ].flags_captured;
		MATCH.destroyed_sentry  += PLAYERS[ p ].destroyed_sentry;
		MATCH.destroyed_supply  += PLAYERS[ p ].destroyed_supply;
		MATCH.flagtouches       += PLAYERS[ p ].flagtouches;

		MATCH.frags             += PLAYERS[ p ].frags;
		//MATCH.frags             += PLAYERS[ p ].destroyed_sentry ;

		MATCH.suicides          += PLAYERS[ p ].suicides;
		MATCH.teamkills         += PLAYERS[ p ].teamkills;
	}
}

long query_gametype( void )
{
	// returns the basic gametype
	// todo: change this gametype crap
	// don't use it for determining anything
	long result;

	result = gametype & MASK_BASIC_GAMETYPE;
	
	return result;
}

long query_gametype_modifier( void )
{
	long result;

	result = gametype & MASK_GAMETYPE_MODIFIER;

	return result;
}

// todo: create a query for type modifier too, or include in this one
char* query_gametype_string( void )
{
	// returns the globally determined gametype
	static char result[ 64 ];
	strcpy( result, "Gametype unknown" );

	switch ( gametype & MASK_BASIC_GAMETYPE ){

		case GT_CAPTURE_THE_FLAG :
			strcpy( result, "Capture the Flag" );
			break;

		case GT_CAPTURE_AND_HOLD :
			strcpy( result, "Capture & Hold" );
			break;

		case GT_DUELL :
			strcpy( result, "Duell" );
			break;
	}

	return result;
}

// this is called after all the stats are gathered in a final attempt
// to determine the gametype
void determine_gametype_final( void )
{
	if ( query_gametype() == GT_UNKNOWN ) {
		// ok, this might have been a duell
		if ( player_count == 2 ) {
			set_gametype( GT_DUELL );
		}
	}
}

long determine_gametype( long line_game_type )
{
	long result;
	
	if ( line_game_type != GT_UNKNOWN ) {
		set_gametype( line_game_type );
	}

	result = query_gametype();

	return result;
}

int empty_gametype_modifier( void )
{
	int result;

	result = ( 0x00000000 == query_gametype_modifier() );

	return result;
}

int gtm_contains( long gametype_modifier )
{
	int result;

	result = ( ( gametype_modifier & query_gametype_modifier() ) > 0 );

	return result;
}

int query_output_defence( void ) 
{
	int result = FALSE;
	
	switch( query_gametype() ) {
		case GT_CAPTURE_THE_FLAG :
			result = TRUE;
			break;
	}
	return result;
}

int query_output_offence( void ) 
{
	int result = FALSE;
	
	switch( query_gametype() ) {
		case GT_CAPTURE_THE_FLAG :
			result = TRUE;
			break;
	}
	return result;
}

int query_output_frags_and_deaths( void )
{
	int result = FALSE;
	
	switch( query_gametype() ) {
		case GT_CAPTURE_THE_FLAG :
			result = TRUE;
			break;

		case GT_CAPTURE_AND_HOLD :
			result = TRUE;
			break;

		case GT_DUELL :
			result = TRUE;
			break;

		default: 
			result = TRUE;
			break;
	}
	return result;
}
