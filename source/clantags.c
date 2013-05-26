#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "clantags.h"
#include "output.h"
#include "players.h"
#include "sorting.h"
#include "tools.h"


#define MIN_TAGLENGTH		2

// Private Functions
// return the length of the found prefix in x
int common_prefix( int y1, int y2, int *x, int prefix )
{
	int y;
	int xpos;
	int result;
	int same_prefix;
	int finished;
	char c1, c2;
#if defined (_DEBUG)
	char test[ 100 ];
#endif

	result = FALSE;
	xpos = 0;

	same_prefix = TRUE;
	finished = FALSE;
	while ( same_prefix && !finished ) {
	
		// try players from y1 to y2, and check if they have the same prefix
		if ( prefix ) {
			c1 = sorted_player( y1 )->nameclean[ xpos ];
		}
		else { // postfix
			c1 = sorted_player( y1 )->nameclean_rev[ xpos ];
		}
		for ( y = y1 + 1 ; y <= y2 && same_prefix ; y ++ ) {
			if ( prefix ) {
				c2 = sorted_player( y )->nameclean[ xpos ];
			}
			else {
				c2 = sorted_player( y )->nameclean_rev[ xpos ];
			}

			if ( c1 == '\0' || c2 == '\0' ) {
				finished = TRUE; // we're finished when we reach the end of a name
			}
			else {
				if ( tolower( c1 ) != tolower( c2 ) ) { // compare the characters lowercase
					same_prefix = FALSE;
				}
			}
		}

		if ( !finished && same_prefix ) {
			xpos++;
		}
	}

	*x = xpos;

	if ( xpos > 0 ) {
		result = TRUE;
#if defined (_DEBUG)
		if ( prefix ) {
			strncopy( test, sorted_player( y )->nameclean, xpos );
		}
		else {
			strncopy( test, sorted_player( y )->nameclean_rev, xpos );
		}
#endif
	}

	return result;

}

// calculate how many players need to have the same prefix in order to
// consider it a clantag
int calculate_pivot_point( int players )
{
	static int result;

	if ( players < 0 ) {
		result = -1;
	}
	else {

		switch( players ) {
			case 0 :
				result = -1;
				break;
			case 1:
				result = -1;
				break;
			default:
				result = 1+ ( players / 2 );
				break;
		}
	}

	return result;
}			

// stupid function only to make the functionality easier understandable in the "determine_clantags" function
int nr_players_in_range( int p1, int p2) {

	return abs( p2 - p1 ) + 1;
}

void build_clantag( char *clantag, char *rawtag )
{
	char plaintag[ 50 ];
	
	strcpy( plaintag, rawtag );
	remove_et_color_codes( plaintag );
	sprintf( outbuf, HTML_TEAM_TAG, plaintag );
	sprintf( clantag, "<a href=\"%s\">%s</a>", outbuf, et_2_html( rawtag ) );
}

// we need to find out where the tag ends in the raw q3string playername.
// then we can convert it to html
void create_clantag_html( int pl, int reverse )
{
	char rawtag[ 50 ];
	
	int icol;
	int itag = 0; // pointer in tag string
	int itag_end = strlen( TEAMS[ PLAYERS[ pl ].team ].clantag ) - 1;
	char x,y;
	int iraw = 0; // pointer in playername string
	int iraw_end = strlen( PLAYERS[ pl ].name ) - 1;
	
	if ( reverse ) { // ^7 s^3i^7gge   ^5plan^7-^3B , match with plan-b
		// this seems to be working, tested in qconsole.log.Hardcore3 
		itag = itag_end; // start at the end ( bla plan-B )
		iraw = iraw_end;
		strcpy( TEAMS[ PLAYERS[ pl ].team ].clantag_html, TEAMS[ PLAYERS[ pl ].team ].clantag );
		while ( itag >= 0  &&
				iraw >= 0     ) {

			x = TEAMS[ PLAYERS[ pl ].team ].clantag[ itag ];
			y = PLAYERS[ pl ].name[ iraw ];
			
			if ( tolower( x ) == tolower( y ) ) { // x: the current character in the plaintext clantag
				itag--;
			}
			if ( itag >= 0 ) {
				// so it won't be done the last time
				iraw--;	
			}
		}
		// ok now we got "  ^5plan^7-^3B"
		strcpy( rawtag, PLAYERS[ pl ].name + iraw );
		lstrip( rawtag ); 
		// now we got "^5plan^7-^3B"
		// test if rawtag starts with a colorcode, if not then
		// look for previous colorcode so we know what color the tag starts with
		if ( rawtag[ 0 ] == '^' ) {
			// ok
		}
		else {

			icol = iraw;
			while ( icol > 0 && PLAYERS[ pl ].name[ icol - 1 ] != '^' ) {
				icol--;
			}
			if ( PLAYERS[ pl ].name[ icol - 1 ] == '^' ) {
				strncopy( rawtag, &(PLAYERS[ pl ].name[ icol - 1 ]), 2 );
			}
			else { // use default colorcode ^7 (white)
				strcpy( rawtag, "^7" );
			}
			strcat( rawtag, PLAYERS[ pl ].name + iraw );
		}
		
		build_clantag( TEAMS[ PLAYERS[ pl ].team ].clantag_html, rawtag );
		//strcpy( TEAMS[ PLAYERS[ pl ].team ].clantag_html, et_2_html( rawtag ) );
		return;
	}
	else {
		while (  ( ( x = TEAMS[ PLAYERS[ pl ].team ].clantag[ itag ] ) != '\0' ) && 
				( itag <= itag_end ) && 
				( iraw <= iraw_end ) ) {

			y = PLAYERS[ pl ].name[ iraw ]; // y: the current character in the raw quake3 name
			// compare the lowercase characters because the NAMECLEAN is converted to lowercase
			if ( tolower( x ) == tolower( y ) ) { // x: the current character in the plaintext clantag
				itag++;
				iraw++;
			}
			else {
				if ( y == '^' ) {
					iraw = iraw + 2;
				}
				else {
					// actually this should not happen, but just in case, prevent infinite loop
					iraw = iraw + 1;
				}
			}
		}
		// iraw gives the length of the raw string
		strncopy( rawtag, PLAYERS[ pl ].name, iraw );
		// todo: add google search link
		build_clantag( TEAMS[ PLAYERS[ pl ].team ].clantag_html, rawtag );
		//strcpy( TEAMS[ PLAYERS[ pl ].team ].clantag_html, et_2_html( rawtag ) );
	}
}

// precondition: players should be sorted by team by nameclean or nameclean reverse
void determine_clantags( int prefix )
{
	// if prefix is true , search for prefix, else search for postfix

	// players should be sorted by team by nameclean
	// determine clantags from teams
	// we need to extract the [dbB+ ] part
	// go for the biggest common prefix (for at least half the team)
/*	Let's hear it for the BLUE team :
    PluTo               |
    Spammage            |
    dbB+ BLADE          y1
    dbB+ CROW           |
    dbB+ DEMOMAN        |
    dbB+ DeG            | 
    dbB+ SNK            y2
    duB+ twist          |
	----x--------

	the nr of players from y1 to y2 should be >= pivot
	'x' is the length of the tag
*/
	//int MIN_TAGLENGTH  = 2; // this is the minimum size to be considered a tag
							// btw this fixes a bug too.

	int y1, y2;				// variables to indicate a playerrange : players y1..y2
	int p1, p2;				// indicate the first and last player in a team
	int t;					// variable used in for loop to walk the teams
	int tag_length;			// 
	int max_length;			// longest tag found
	int max_y1, max_y2;		// playerrange of players that have this longest tag in common
	int found_tag;
	int finished;			// tagsearch finished for this team
	int pivot;				// nr of players needed to make it a clantag
	
	// do all the teams
	p1 = 0; p2 = -1;
	for ( t = 0 ; t < team_count ; t++ ) {
		
		p1 = p2 + 1;
		p2 = p1;
		while ( p2 < player_count - 1 && ( sorted_player( p1 )->team == sorted_player( p2 )->team ) ) {
		// look for the last player in this team
			p2++;
		}

		// in case loop ended because of p2 == player_count
		if ( sorted_player( p1 )->team != sorted_player( p2 )->team ) {
			p2--;
		}
		// ok, now we got p1 and p2 being the first and last player of the same team
		pivot = calculate_pivot_point( nr_players_in_range( p1, p2 ) );

		if ( pivot > 0 ) {

			y1 = p1;
			y2 = p1 + 1;
			found_tag = FALSE;
			max_length = 0;
			finished = ( nr_players_in_range( y1, p2 ) < pivot );
			while ( !found_tag && !finished ) {
				
				y2 = y1 + 1;

				//tag_length = 0;
				while ( y2 <= p2 && common_prefix( y1, y2, &tag_length, prefix ) ){
					// we got a playerrange with a common prefix of length "tag_length"

					if ( nr_players_in_range( y1, y2 ) >= pivot ) {
						// we got the number of players needed to make it a clantag
						if ( tag_length > max_length ) {
							// cool, it's even bigger than a previously found tag
							max_length = tag_length;
							max_y1 = y1; // remember first player
							max_y2 = y2; // remember last player

							// because the tag won't get bigger when we increase y2, we
							// may as well speed up the search and break out of this while loop
							break;
						}
					}

					y2++;
				}

				y1++;

				finished = ( nr_players_in_range( y1, p2 ) < pivot );
				// this means there are not enough players in the range y1..p2 to find 
				// a possible clantag, so we might as well stop searching this range.
			}

			if ( max_length >= MIN_TAGLENGTH ) {
				// ok we found it
				found_tag = TRUE;
				if ( prefix ) {
					// normal clantag
					if ( max_length > (signed)strlen( TEAMS[ sorted_player( max_y1 )->team ].clantag ) ) {
						// only copy if it's bigger than the current clantag
						strncopy( TEAMS[ sorted_player( max_y1 )->team ].clantag , sorted_player( max_y1 )->nameclean, max_length );
						create_clantag_html( player_index[ max_y1 ], FALSE );

					}
				}
				else {
					// reverse clantag
					if ( max_length > (signed)strlen( TEAMS[ sorted_player( max_y1 )->team ].clantag ) )  {
						// only copy if it's bigger than the current clantag
						strncopy( TEAMS[ sorted_player( max_y1 )->team ].clantag , 
								  sorted_player( max_y1 )->nameclean + strlen( sorted_player( max_y1 )->nameclean ) - max_length, 
								  max_length );
						create_clantag_html( player_index[ max_y1 ], TRUE );
					}
				}
				rstrip( TEAMS[ sorted_player( max_y1 )->team ].clantag );
			}
		}
		else {
			// don't bother, teamsize is 0 or 1, no use to try to get the clantag
		}
	}
	
}


// Public Functions
void determine_clantags_v2( void )
{
	sort_players( BY_TEAM_BY_NAMECLEAN );
	determine_clantags( TRUE );

	sort_players( BY_TEAM_BY_NAMECLEAN_REV );
	determine_clantags( FALSE );

}
