// Handle output to html or txt
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "matching.h"
#include "maps.h"
#include "output.h"
#include "agostats.h"
#include "game.h"
#include "tools.h"
#include "players.h"
#include "teams.h"
#include "stathandling.h"
#include "highscores.h"
#include "sorting.h"
#include "versionhistory.h"

#define HTML_BUF_COUNT				4
#define DEFAULT_FILENAME_OPTIONS    "t-n-r" // timestamp mapname result

const char red_background[] = CSS_BACKGROUND_TEAM_RED;

// public variables
int OUTPUT_EVENTS ;  // captures, disconnected, renamed etc. default off

char outbuf						[ LINE_BUFFER_SIZE ];
char htmlbuf					[ LINE_BUFFER_SIZE ];

// private variables
char output_filename			[ 256 ];
char highscore_filename			[ 256 ];
char filename_options			[ 256 ];

static int buffer_number = 0;

char HTML_SOURCE_BUF [ 4096 ];
char HTML_BUF [ HTML_BUF_COUNT ][ 4096 ];

char custom_outputfile [ 255 ];

int				USE_STDOUT;
int				CUSTOM_OUTPUTFILE;   // complete name enter by using -o option
int				CUSTOMIZE_FILENAME; // name customized using options -f...
_OUTPUT_TYPE	OUTPUT_TYPE ; // Default to text for now
char			OUTPUT_EXTENSION[ 10 ]; // txt, html


t_game_color_to_css css_colors[ ]= {

	'0', "ET_C0", "#C0C0C0",
	'1', "ET_C1", "#FF0000",
	'2', "ET_C2", "#00FF00",
	'3', "ET_C3", "#FFFF00",
	'4', "ET_C4", "#0000FF",
	'5', "ET_C5", "#00FFFF",
	'6', "ET_C6", "#800080",
	'7', "ET_C7", "#FFFFFF",
	'8', "ET_C8", "#FF8000",
	'9', "ET_C9", "#666867",
	'A', "ET_CA", "#ff991B",
	'B', "ET_CB", "#008080",
	'C', "ET_CC", "#800080",
	'D', "ET_CD", "#0272ED",
	'E', "ET_CE", "#7F00FF",
	'F', "ET_CF", "#2D84AF",
	'G', "ET_CG", "#CDFFCA",
	'H', "ET_CH", "#006636",
	'I', "ET_CI", "#FE0034",
	'J', "ET_CJ", "#791A18",
	'K', "ET_CK", "#993400",
	'L', "ET_CL", "#AF882D",
	'M', "ET_CM", "#6E7124",
	'N', "ET_CN", "#D9DCA3",
	'O', "ET_CO", "#FEFF7F",
	'P', "ET_CP", "#7F7F7F", // zwart, nu grijs
	'Q', "ET_CQ", "#D80200",
	'R', "ET_CR", "#00BB02",
	'S', "ET_CS", "#FFFF01",
	'T', "ET_CT", "#0001FC",
	'U', "ET_CU", "#01EBEA",
	'V', "ET_CV", "#FC01FF",
	'W', "ET_CW", "#919B92",
	'X', "ET_CX", "#ED7600",
	'Y', "ET_CY", "#7F7F7F",
	'Z', "ET_CZ", "#B2B2B2",

	// different naming to create valid CSS
	'/', "ET_C_SLASH"   , "#CBCB69",
	'*', "ET_C_STAR"    , "#BBBBBB",
	'-', "ET_C_MINUS"   , "#757329",
	'+', "ET_C_PLUS"    , "#9B3100",
	'?', "ET_C_QUESTION", "#6F0205",
	'@', "ET_C_AT"      , "#6D3804",

	 0 , ""     , "" // the end of it
};

static void output_css_colors( void )
{	
	int i = 0;

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definition of the Enemy Territory colors ^0 ^1 etc.          */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );

	while ( css_colors[ i ].color_code != 0 ) {
		sprintf( HTML_BUF[ 0 ], "    .%s  { color : %s; }\n", css_colors[ i ].css_color, css_colors[ i ].rgb_value );
		statfile( SF_PRINT, HTML_BUF[ 0 ] );
		i++;
	}

	statfile( SF_PRINT, "\n" );
}

void next_html_buffer( void )
{
	buffer_number++ ;
	if ( buffer_number == HTML_BUF_COUNT ) {
		buffer_number = 0;
	}
}

int get_output_type_obsolete( void )
{
	return OUTPUT_TYPE;
}



void insert_gamenumber( char *filename )
{
	int pos;
	int len;
	int found = FALSE;
	char extension[ 255 ];
	char game_info[ 255 ];

	len = strlen( filename );
	pos = len - 1;
	// find dot '.'
	while ( !found && pos > 0 ) {
		found = ( filename[ pos ] == '.' );
		pos--;
	}

	if ( found ) {
		strncopy( extension, filename + pos + 1, sizeof( extension ) - 1 );
		sprintf( game_info, "-game-%ld", game_count );
		strcpy( filename + pos + 1, game_info );
		strcat( filename, extension );
	}

}

void append_timestamp( char *filename )
{
	struct tm	*time_now;
	time_t		secs_now;

	time(&secs_now);
	time_now = localtime(&secs_now);

	sprintf( outbuf, "%4d%02d%02d-%02d%02d", 
		1900 +  time_now->tm_year,1+ time_now->tm_mon,time_now->tm_mday, 
		time_now->tm_hour, time_now->tm_min );

	strcat( filename, outbuf );
}

void append_mapname( char *filename )
{
	if ( map_id != -1 ) {
		sprintf( outbuf, "%s", MAP_INFOS[ map_id ].mapname );
		strcat( filename, outbuf );
	}
	else {
		if ( strlen( map ) > 0 ) {
			strcat( filename, map );
		}
		else {
			strcat( filename, "map_unknown" );
		}
	}
}

void append_result( char *filename )
{
	char score [ 100 ];
	int t, st;
	int count = 0;
	for ( t = 0; t < team_count ; t++ ) {
		st = team_index[ t ];
		if ( team_participated( st ) ) {
			
			count++;
			sprintf( score, "%s-%ld", TEAMS[ st ].filename, TEAMS[ st ].flags_captured );
			strcat( filename, score );
			if ( count < active_teams ) {
				strcat( filename, "-" );
			}
			
		}
	}
}
void append_custom_name( char *filename )
{
	strcat( filename, custom_outputfile );
}
void customize_filename( char *filename )
{
	char dummy[ 2 ];
	int  sequence_index = 0;
	int  len;

	dummy[ 1 ] = '\0';
	
	len = strlen( filename_options );
	while ( sequence_index < len ) {
		switch( filename_options[ sequence_index ] ) {

			case 't': // timestamp
				append_timestamp( filename );
				break;

			case 'n': // mapname
				append_mapname( filename );
				break;

			case 'r': // result (scores)
				append_result( filename );
				break;
			
			case 'c': // custom name (using -o option)
				append_custom_name( filename );
				break;

			default :
				// append single character
				dummy[ 0 ] = filename_options[ sequence_index ];
				strcat( filename, dummy );
				break;
		}
		sequence_index++;
	}
	strcat( filename, ".htm" );
}

char *create_filename( void )
{
	// yyyymmdd-hhmm-stats.txt
	char *filename;

	if ( writing_highscores ) {
		if ( writing_highscore_player_id > 0 ) {
			filename = output_filename;
			sprintf( filename, "%s/p%ld.htm", PERSONAL_DIRECTORY, writing_highscore_player_id );
		}
		else {
			if ( writing_linkpage ) {
				filename = output_filename;
				sprintf( filename, "%s/%s", PERSONAL_DIRECTORY, PERSONAL_LINKPAGE );
			}
			else {
				filename = highscore_filename;
				strcpy( filename, HIGHSCORE_OUTPUT );
			}
		}
	}
	else {
		filename = output_filename;
		
		filename[ 0 ] = '\0'; // initialize filename

		if ( ! CUSTOMIZE_FILENAME ) {	
			strcpy( filename_options, DEFAULT_FILENAME_OPTIONS );
		}
		if ( CUSTOM_OUTPUTFILE && !CUSTOMIZE_FILENAME ) {
			strcpy( filename, custom_outputfile );
		}
		else {
			customize_filename( filename );
		}

		if ( game_count > 1L && !ONLY_LAST_GAME && !writing_highscores ) {
			insert_gamenumber( filename );
		}		
	}

	return filename;
}

void delete_statfile( void )
{
	if ( output_filename[ 0 ] != '\0' ) {
		remove( output_filename );
	}
}

// statfile output operations
void statfile( int action, char *message )
{
	static FILE *f;

	switch ( action ) {

		case SF_INIT :
			
			if ( USE_STDOUT ) {
				f = stdout;
			}
			else { 
				f = fopen( create_filename(), "w" );
			}
			break;

		case SF_PRINT :
			//fprintf( f, "%s", message );
			fprintf( f, message );
			break;

		case SF_TERM :
			if ( !USE_STDOUT ) {
				fclose( f );
				//if ( !writing_highscores ) {
				//	rename_output_file();
				//}
			}
			break;
	}
}

int find_colorcode( char *et_string )
{
	int i = 0;
	
	while ( et_string[ i ] != '\0' ) {
		if ( et_string[ i ] == '^' ) {
			return i;
		}
		i++;
	}

	// if no colorcode found
	return -1;
}

char *left_string( char *s, int count )
{
	static char result[ 255 ];

	strncopy( result, s, count );

	return result;
	
}

char *html_player_clickable( int p )
{
	next_html_buffer();
	sprintf( HTML_BUF[ buffer_number ], HTML_PLAYER_CLICKABLE, p, PLAYERS[ p ].name_html );

	return HTML_BUF[ buffer_number ];
}

char *html_player_location( int p )
{
	next_html_buffer();
	sprintf( HTML_BUF[ buffer_number ], HTML_PLAYER_LOCATION, p, PLAYERS[ p ].name_html );
	return HTML_BUF[ buffer_number ];
}

// used to color "RED" red, "BLUE" blue etc.
char *color_string( char *source, char *color )
{
	next_html_buffer();

	sprintf( HTML_BUF[ buffer_number ], "<span class=\"%s\">%s</span>", color, source );

	return HTML_BUF[ buffer_number ];
}

char *get_color_class( char *et_color )
{
	int i = 0;
	char *result = css_colors[ 7 ].css_color; // default white
	int found = FALSE;

	while ( css_colors[ i ].color_code != 0 && !found && ( et_color != NULL ) ) {
		if ( toupper( css_colors[ i ].color_code ) == toupper(*et_color) ) {
			found = TRUE;
			result = css_colors[ i ].css_color;
		}
		i++;
	}

	return result;
	/*
	if ( !found ) {
		strcpy( result, CSS_ET_COLOR_7 );
		return result;
	}

	color_code = ( *et_color );

	switch (color_code) {
		case '0':
			strcpy( result, CSS_ET_COLOR_0 );
			break;

		case '1':
			strcpy( result, CSS_ET_COLOR_1 );
			break;

		case '2':
			strcpy( result, CSS_ET_COLOR_2 );
			break;

		case 3:
			strcpy( result, CSS_ET_COLOR_3 );
			break;

		case 4:
			strcpy( result, CSS_ET_COLOR_4 );
			break;

		case 5:
			strcpy( result, CSS_ET_COLOR_5 );
			break;

		case 6:
			strcpy( result, CSS_ET_COLOR_6 );
			break;

		case 7:
			strcpy( result, CSS_ET_COLOR_7 );
			break;

		default: // black or something, should not happen
			strcpy( result, CSS_ET_COLOR_7 );
			break;
	}

	return result;
	*/
}
void convert_plaintext_to_html( char *plaintext )
{
	// use escape characters for html codes
	str_replace( plaintext,"&" , "&#38;" );
	str_replace( plaintext,"<" , "&#60;" );
	str_replace( plaintext,">" , "&#62;" );
	str_replace( plaintext,"\"" , "&#34;" );
}

int only_spaces( char *source )
{
	int index = 0;

	while ( source[index] != '\0' ) {
		if ( !isspace( source[ index ] ) ) {
			return FALSE;
		}
		index++;
	}
	return TRUE;
}

void append_string_to_html_buf( char* source, char *dest, char* color )
{
	// use escape characters for html codes
	//str_replace( source,"<" , "&#60;" );
	//str_replace( source,">" , "&#62;" );
	convert_plaintext_to_html( source );
	if ( only_spaces( source ) ) {
		strcat( dest, source );
	}
	else {
		sprintf( dest, HTML_SET_COLOR_STRING, get_color_class( color ), source ); // substring that is colored
	}
}

void remove_et_color_codes( char *et_string )
{
	int head, newhead, len;
	
	head = 0;
	newhead = 0;
	len = strlen( et_string );
	while ( head < len ) {
		if ( et_string[ head ] == '^' ) {
			head = head + 2;
		}
		else {
			et_string[ newhead ] = et_string[ head ];
			head++;
			newhead++;
		}
	}
	et_string[ newhead ] = '\0';
	rstrip(et_string);
	lstrip(et_string);
}

char *et_2_html( char *et_string )
{
	int p1    = 0;
	int len	  = strlen( et_string );
	char *color = NULL; // causes default color to be used initially
	int html_pointer = 0;

	//^2AGO^7 Pook -> <font class="col_green">AGO</font><font class="col_white"> Pook</font>

// color codes (modulo 8):
// 0 Black
// 1 Red
// 2 Green
// 3 Yellow
// 4 Blue
// 5 Cyan?
// 6 Purple
// 7 White

	next_html_buffer();
	HTML_BUF[ buffer_number ][ 0 ] = '\0';

	while ( p1 < len ) {
		
		if ( et_string [ p1 ] == '^' ) {
			if ( p1 >= len - 2 ) break; // if the string ends in a colorcode then stop
			if ( p1 > 0 ) {
				// convert the sourcebuffer to html and append it
				html_pointer = strlen( HTML_BUF[ buffer_number ] );
				append_string_to_html_buf( HTML_SOURCE_BUF,                           // source
										   HTML_BUF[ buffer_number ] + html_pointer,  // destination
										   color  );
				HTML_SOURCE_BUF[ 0 ] = '\0';
			}
			p1++;
			if ( p1 < len ) {
				color = &et_string[ p1 ];
				p1++;
			}
		}
		else {
			if ( et_string[ p1 ] != 0x0a ) { //  because of 'x renamed to y#0a'lines
				strncat( HTML_SOURCE_BUF, &et_string[ p1 ], 1 );
			}
			p1++;
		}

	}

	if ( p1 > 0 ) {
		// convert the sourcebuffer to html and append it
		html_pointer = strlen( HTML_BUF[ buffer_number ] );
		append_string_to_html_buf( HTML_SOURCE_BUF,                            // source
									HTML_BUF[ buffer_number ] + html_pointer,  // destination
									color  );
		HTML_SOURCE_BUF[ 0 ] = '\0';
	}

	return HTML_BUF[ buffer_number ];
}

void output_write_stylesheet()
{
	// Begin of default style
	
	statfile( SF_PRINT, "\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Global document settings (color, font, text alignment, etc)  */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    BODY         { background-color: #000000;\n" );
	statfile( SF_PRINT, "                   color: #FFFFFF;\n"            );
	statfile( SF_PRINT, "                   font-family: verdana;\n"      );
	statfile( SF_PRINT, "                   font-size: 12px ;\n"          );
#if defined (_DEBUG)
	//statfile( SF_PRINT, "                   background-image: url(pictures/jolie.jpg);\n" );
	//statfile( SF_PRINT, "                   background-repeat: no-repeat;\n" );
	//statfile( SF_PRINT, "                   background-attachment: fixed;\n" );
	//statfile( SF_PRINT, "                   background-position : top right;\n" );
#endif
	statfile( SF_PRINT, "                 }\n"                            );
	statfile( SF_PRINT, "\n" );
	

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for the Main table                               */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.MAIN { background-color: transparent;\n"   );
	statfile( SF_PRINT, "                 border: 0px solid #C0C0C0;\n"       );
	statfile( SF_PRINT, "                 border-collapse: collapse;\n"       );
	statfile( SF_PRINT, "                 color: #FFFFFF;\n"                  );
	statfile( SF_PRINT, "                 margin: 0px 0px;\n"                 );
	statfile( SF_PRINT, "               }\n"                                  );
	statfile( SF_PRINT, "\n" );

if ( !writing_highscores ) {
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for EVENTS (MM1 etc)                             */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.EVENTS { background-color: transparent;\n" );
	statfile( SF_PRINT, "                   border: 0px solid #C0C0C0;\n" );
	statfile( SF_PRINT, "                   border-collapse: collapse;\n" );
	statfile( SF_PRINT, "                   margin: 0px auto;\n"          );
	statfile( SF_PRINT, "                   text-align: left;\n"          );
	statfile( SF_PRINT, "                 }\n"                            );

	statfile( SF_PRINT, "    TD.EVENTS_COLUMN1 { border: 0px; padding: 0px 4px; }\n" );
	statfile( SF_PRINT, "    TD.EVENTS_COLUMN2 { border: 0px; padding: 0px 4px; }\n" );
	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Result                                   */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    DIV.RESULT     { background-color: transparent;\n" );
	statfile( SF_PRINT, "                     border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                     margin: 0px auto;\n"              );
	statfile( SF_PRINT, "                     text-align: center;\n"            );
	statfile( SF_PRINT, "                   }\n"                                );
	statfile( SF_PRINT, "    SPAN.RESULT      { font-weight: bold;\n"           );
	statfile( SF_PRINT, "                       font-size: x-large;\n"         );
	statfile( SF_PRINT, "                       margin : 0px auto;\n"           );
	statfile( SF_PRINT, "                     }\n"                              );
	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for Captures                                     */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.CAPTURES   { background-color: transparent;\n" );
	statfile( SF_PRINT, "                       border: 0px solid;\n"             );
	statfile( SF_PRINT, "                       border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                       margin: 0px auto;\n"              );
	statfile( SF_PRINT, "                       padding: 0px 0px;\n"              );
	statfile( SF_PRINT, "                     }\n"                                );
	statfile( SF_PRINT, "    CAPTION.CAPTURES { font-weight: bold;\n"          );
	statfile( SF_PRINT, "                       margin : 0px auto;\n"          );
	statfile( SF_PRINT, "                       padding: 5px 5px;\n"           );
	statfile( SF_PRINT, "                       }\n"                           );
	statfile( SF_PRINT, "    TH.CAPTURES       { border: 1px solid #606060; font-weight: bold; padding: 5px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TH.CAPS_TEAMNAME  { border: 1px solid #606060; font-weight: bold; padding: 5px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TH.CAPS_PLAYER    { border: 1px solid #606060; font-weight: bold; padding: 0px 10px; text-align: left; }\n" );
	statfile( SF_PRINT, "    TH.CAPS_TOUCHES   { border: 1px solid #606060; font-weight: bold; padding: 0px 5px; text-align: left; }\n" );
	statfile( SF_PRINT, "    TD.CAPS_SCORE     { border: 0px solid; font-weight: bold; padding: 0px 7px; text-align: center;}\n" );
	statfile( SF_PRINT, "    TD.CAPS_PLAYER    { border: 0px solid; padding: 0px 10px; text-align: left; }\n" );
	statfile( SF_PRINT, "    TD.CAPS_TOUCHES   { border: 0px solid; padding: 0px 10px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.CAPS_COAST2COAST { border: 0px solid; padding: 0px 0px; text-align: left; }\n" );
	statfile( SF_PRINT, "\n" );


	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Prima Donnas                             */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.FLAG_ACTION { background-color: transparent;\n"				);
	statfile( SF_PRINT, "                       border: 0px solid;\n"							);
	statfile( SF_PRINT, "                       border-collapse: collapse;\n"					);
	statfile( SF_PRINT, "                       margin: 0px auto;\n"							);
	statfile( SF_PRINT, "                       padding: 0px 0px;\n"							);
	statfile( SF_PRINT, "                     }\n"												);
	statfile( SF_PRINT, "    CAPTION.FLAG_ACTION { font-weight: bold;\n"						);
	statfile( SF_PRINT, "                       margin : 0px auto;\n"							);
	statfile( SF_PRINT, "                       padding: 5px 5px;\n"							);
	statfile( SF_PRINT, "                       }\n"											);
	statfile( SF_PRINT, "    TD.FLAG_ACTION_HEADER  { border: 1px solid #606060; font-weight: bold; padding: 5px 5px; text-align: center;}\n" );
	statfile( SF_PRINT, "    TD.FLAG_ACTION_PLAYER  { border: 0px solid; padding: 0px 10px; text-align: left; }\n" );
	statfile( SF_PRINT, "    TD.FLAG_ACTION_SCORE   { border: 0px solid; padding: 0px 10px; text-align: right; }\n" );
	statfile( SF_PRINT, "    A.FLAG_ACTION:link     { color: #FFFFFF; }\n"                      );
	statfile( SF_PRINT, "    A.FLAG_ACTION:visited  { color: #FFFFFF; }\n"                      );
	statfile( SF_PRINT, "    A.FLAG_ACTION:hover    { color: #FFFFFF; }\n"                      );
	statfile( SF_PRINT, "    A.FLAG_ACTION:active   { color: #FFFFFF; }\n"                      );
	statfile( SF_PRINT, "\n" );


//	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
//	statfile( SF_PRINT, "/*  Definitions for The Game                                     */\n" );
//	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
//	statfile( SF_PRINT, "    TABLE.THE_GAME { background-color: transparent;\n" );
//	statfile( SF_PRINT, "                     border: 1px solid #C0C0C0;\n"     );
//	statfile( SF_PRINT, "                     border-collapse: collapse;\n"     );
//	statfile( SF_PRINT, "                     margin: 0px auto;\n"              );
//	statfile( SF_PRINT, "                     text-align: left;\n"              );
//	statfile( SF_PRINT, "                   }\n"                                );
//	statfile( SF_PRINT, "    CAPTION.THE_GAME { font-weight: bold;\n"           );
//	statfile( SF_PRINT, "                       margin : 0px auto;\n"           );
//	statfile( SF_PRINT, "                     }\n"                              );
//	statfile( SF_PRINT, "    TD.THE_GAME      { padding: 0px 2px; text-align: left; }\n" );
//	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Teams                                    */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.THE_TEAMS { background-color: transparent;\n" );
	statfile( SF_PRINT, "                      border: 0px solid #606060;\n"     );
	statfile( SF_PRINT, "                      border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                      margin: 0px auto;\n"              );
	statfile( SF_PRINT, "                      text-align: left;\n"              );
	statfile( SF_PRINT, "                    }\n"                                );
	statfile( SF_PRINT, "    CAPTION.THE_TEAMS { font-weight: bold;\n"           );
	statfile( SF_PRINT, "                        margin : 0px auto;\n"           );
	statfile( SF_PRINT, "                        padding: 5px 5px;\n"            );
	statfile( SF_PRINT, "                      }\n"                              );
	statfile( SF_PRINT, "    TD.THE_TEAMS_HEADER     { border: 1px solid #606060; padding: 2px 5px; text-align: center; font-size: 16px; }\n"               );
	statfile( SF_PRINT, "    TD.THE_TEAMS_ROLE       { border: 0px solid #606060; padding: 2px 5px; text-align: center; font-weight: bold; }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_PLAYER     { border: 0px solid #606060; padding: 2px 5px; }\n"   );
	statfile( SF_PRINT, "    TD.THE_TEAMS_HEADER_OBJ { border: 1px solid #606060; text-align: center; padding: 5px; font-weight: bold;}\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_OBJ   { text-align: left; padding-left: 5px; padding-right: 5px }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_OBJ_NUMBER { text-align: right; padding-right: 5px }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_HEADER_DEF { border: 1px solid #606060; text-align: center; padding: 5px; font-weight: bold; }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_DEF   { text-align: left; padding-left: 5px; padding-right: 5px}\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_DEF_NUMBER { text-align: right; padding-right: 5px }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_HEADER_OFF { border: 1px solid #606060; text-align: center; padding: 5px; font-weight: bold; }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_OFF   { text-align: left; padding-left: 5px; padding-right: 5px }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_OFF_NUMBER { text-align: right; padding-right: 5px }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_HEADER_FND { border: 1px solid #606060; text-align: center; padding: 5px; font-weight: bold; }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_FND   { text-align: left; padding-left: 5px; padding-right: 5px }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_FND_NUMBER { text-align: right; padding-right: 5px }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_LINE_EMPTY { text-align: left; }\n"                              );
	statfile( SF_PRINT, "    TD.THE_TEAMS_FRAGS_BY_WEAPON_HEADER { text-align: center; padding: 5px; font-weight: bold; }\n"  );
	statfile( SF_PRINT, "    TD.THE_TEAMS_FRAGS_BY_WEAPON_NAME   { text-align: left; padding-left: 5px;  }\n" );
	statfile( SF_PRINT, "    TD.THE_TEAMS_FRAGS_BY_WEAPON        { text-align: right; padding-right: 5px }\n" );
	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Players                                  */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.THE_PLAYERS { background-color: transparent;\n" );
	statfile( SF_PRINT, "                        border: 0px;\n"                   );
	statfile( SF_PRINT, "                        border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                        margin: 0px auto;\n"              );
	statfile( SF_PRINT, "                      }\n" );
	statfile( SF_PRINT, "    CAPTION.THE_PLAYERS { font-weight: bold;\n"           );
	statfile( SF_PRINT, "                          margin : 0px auto;\n"           );
	statfile( SF_PRINT, "                          padding: 5px 5px;\n"            );
	statfile( SF_PRINT, "                        }\n"                              );
	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Players ( headerline )                   */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_ROLE        { border: 1px solid #606060; padding: 2px 5px; text-align: left; font-weight: bold; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_NAME        { border: 1px solid #606060; padding: 2px 5px; text-align: left; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_FRAGS       { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_DEATHS      { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_TEAMKILLS   { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_TEAMDEATHS  { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_SUICIDES    { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_FLAGTOUCHES { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_HEADER_CAPTURES    { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Players ( non-OFFENCE players )          */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_ROLE        { border: 0px solid #606060; padding: 2px 5px; text-align: center; font-weight: bold;}\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_NAME        { border: 1px solid #606060; padding: 2px 5px; text-align: left; }\n"   );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_FRAGS       { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_DEATHS      { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_TEAMKILLS   { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_TEAMDEATHS  { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_SUICIDES    { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_FLAGTOUCHES { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_CAPTURES    { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Players ( OFFENCE players )              */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_ROLE        { border: 0px solid #606060; padding: 2px 5px; text-align: center; font-weight: bold; }\n"                   );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_NAME        { border: 1px solid #606060; padding: 2px 5px; text-align: left; }\n"                     );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_FRAGS       { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"                    );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_DEATHS      { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"                    );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_TEAMKILLS   { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"                    );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_TEAMDEATHS  { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"                    );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_SUICIDES    { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"                    );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_FLAGTOUCHES { border: 1px solid #606060; padding: 2px 5px; text-align: right; font-weight: bold; }\n" );
	statfile( SF_PRINT, "    TD.THE_PLAYERS_OFF_CAPTURES    { border: 1px solid #606060; padding: 2px 5px; text-align: right; font-weight: bold; }\n" );
	statfile( SF_PRINT, "\n" );
	
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for Player vs Player (PVP)                       */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.PVP_GROUP           { background-color: transparent;\n" );
	statfile( SF_PRINT, "                                 border: 0px solid #606060;\n"    );
	statfile( SF_PRINT, "                                 border-collapse: collapse;\n"    );
	statfile( SF_PRINT, "                                 margin: 0px auto;\n"             );
	statfile( SF_PRINT, "                                 padding: 0px;\n"                 );
	statfile( SF_PRINT, "                              }\n"                                );
	statfile( SF_PRINT, "    TABLE.PLAYER_VS_PLAYER    { background-color: transparent;\n" );
	statfile( SF_PRINT, "                                border: 1px solid #606060;\n"     );
	statfile( SF_PRINT, "                                border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                                margin: 0px auto;\n"              );
	statfile( SF_PRINT, "                                padding: 0px;\n"                  );
	statfile( SF_PRINT, "                              }\n"                                );
	statfile( SF_PRINT, "    CAPTION.PLAYER_VS_PLAYER  { font-weight: bold;\n"             );
	statfile( SF_PRINT, "                                margin : 0px auto;\n"             );
	statfile( SF_PRINT, "                                padding: 5px 5px;\n"              );
	statfile( SF_PRINT, "                              }\n"                                );
	statfile( SF_PRINT, "    SPAN.TEAMKILL              { color: #FF0000; }\n" );
	statfile( SF_PRINT, "    SPAN.SUICIDE               { color: #FF0000; }\n" );
	statfile( SF_PRINT, "    SPAN.FRAG                  { color: #90EE90; }\n" );
	statfile( SF_PRINT, "    SPAN.CHASE                 { color: #FF99CC; }\n" );
	statfile( SF_PRINT, "    SPAN.DEF_ATTACK            { color: #FFFFFF; }\n" );
	statfile( SF_PRINT, "    SPAN.PVP_HINT              { color: #C0C0C0; font-size: 10px; }\n" );
	statfile( SF_PRINT, "    TD.TOPLEFT                 { border: 1px solid #606060; }\n" );
	statfile( SF_PRINT, "    TD.PVP_HEADER_ROLE         { border: 1px solid #606060; padding: 2px 5px; text-align: left; }\n"   );
	statfile( SF_PRINT, "    TD.PVP_HEADER_PLAYER       { border: 1px solid #606060; padding: 2px 5px; text-align: left; }\n"   );
	statfile( SF_PRINT, "    TD.PVP_PLAYER_NUMBER       { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.PVP_PLAYER_ROLE         { border: 0px solid #606060; padding: 2px 5px; text-align: center; font-weight: bold;}\n" );
	statfile( SF_PRINT, "    TD.PVP_PLAYER_NAME         { border: 1px solid #606060; padding: 2px 5px; text-align: left; }\n"   );
	statfile( SF_PRINT, "    TD.PVP_TEAMKILL            { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.PVP_SUICIDE             { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.PVP_FRAG                { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.PVP_CHASE               { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.PVP_DEF_ATTACK          { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "    TD.PVP_STRING              { border: 1px solid #606060; }\n"                                       );
	statfile( SF_PRINT, "    TD.PVP_NUMBER              { border: 1px solid #606060; padding: 2px 5px; text-align: right; }\n"  );
	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for Explanation of the colors                    */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.PVP_LEGEND { background-color: transparent;\n" );
	statfile( SF_PRINT, "                       border: 1px solid #C0C0C0;\n"     );
	statfile( SF_PRINT, "                       border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                       padding: 5px;\n"                  );
	statfile( SF_PRINT, "                     }\n"                                );
	statfile( SF_PRINT, "    CAPTION.PVP_LEGEND        { font-weight: bold; }\n"  );
	
	statfile( SF_PRINT, "    TD.PVP_LEGEND_NUMBER      { font-weight: bold; padding: 0px 5px; text-align: right; }\n" );
	statfile( SF_PRINT, "    TD.PVP_LEGEND_EXPLANATION { padding: 0px 5px; text-align:left; }\n"                      );
	statfile( SF_PRINT, "\n" );
}	
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for The Awards                                   */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.THE_AWARDS { background-color: transparent;\n" );
	statfile( SF_PRINT, "                       border: 0px;\n"                   );
	statfile( SF_PRINT, "                       border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                       margin: 0px auto;\n"              );
	statfile( SF_PRINT, "                       padding: 0px 0px;\n"              );
	statfile( SF_PRINT, "                     }\n"                                );
	statfile( SF_PRINT, "    CAPTION.THE_AWARDS { font-weight: bold;\n"          );
	statfile( SF_PRINT, "                         margin : 0px auto;\n"          );
	statfile( SF_PRINT, "                       }\n"                             );
	statfile( SF_PRINT, "    TD.THE_AWARDS_GROUPNAME    { border: 0px; font-weight: bold; padding: 0px 0px; }\n" );
	statfile( SF_PRINT, "    TD.THE_AWARDS_AWARDNAME    { border: 0px; padding: 0px 20px; }\n" );
	statfile( SF_PRINT, "    TD.HIGHSCORE_DATE          { color : #C0C0C0; }\n"   );
	statfile( SF_PRINT, "    SPAN.HIGHSCORE_INDICATOR   { color : #FFFF00; }\n"   );
	statfile( SF_PRINT, "\n" );

if ( !writing_highscores ) {
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definitions for Detailed Player Info (DPI)                   */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TABLE.DETAILED_PLAYER_INFO   { background-color: transparent;\n" );
	statfile( SF_PRINT, "                                   border: 0px solid #606060;\n"     );
	statfile( SF_PRINT, "                                   border-collapse: collapse;\n"     );
	statfile( SF_PRINT, "                                   margin: 0px auto; \n"             );
	statfile( SF_PRINT, "                                 }\n"                                );
	statfile( SF_PRINT, "    CAPTION.DETAILED_PLAYER_INFO { font-weight: bold;\n"   );
	statfile( SF_PRINT, "                                   margin : 0px auto;\n"   );
	statfile( SF_PRINT, "                                   padding-bottom: 20px ;\n"   );
	statfile( SF_PRINT, "                                 }\n"                      );
	statfile( SF_PRINT, "    TD.DPI_PLAYER_NAME           { border: 0px solid #606060;font-size: 16px; text-align: center; padding-top: 10px; padding-bottom: 10px }\n" );
	statfile( SF_PRINT, "    TD.DPI_TEAMCOLOR			  { border: 0px; height:5px;}\n" );
#if defined(_DEBUG)
	statfile( SF_PRINT, "    TD.DPI_PLAYER_DEBUG          { border: 1px solid #606060; font-weight: bold; text-align: center; }\n" );
#endif
	statfile( SF_PRINT, "    TD.DPI_PLAYER_DEF            { border: 1px solid #606060; font-weight: bold; text-align: center; }\n" );
	statfile( SF_PRINT, "    TD.DPI_PLAYER_OFF            { border: 1px solid #606060; font-weight: bold; text-align: center; }\n" );
	statfile( SF_PRINT, "    TD.DPI_PLAYER_FND            { border: 1px solid #606060; font-weight: bold; text-align: center; }\n" );
	statfile( SF_PRINT, "    TD.DPI_PLAYER_INFO_DESCRIPTION{border: 0px solid #606060; border-right: 0px; padding: 2px 5px; text-align: left }\n" );
	statfile( SF_PRINT, "    TD.DPI_PLAYER_INFO_NUMBER    { border: 0px solid #606060; border-left: 0px; padding: 2px 5px; text-align: right }\n" );
	statfile( SF_PRINT, "    TABLE.DPI_FRAGS_DEATHS       { background-color: transparent;\n"   );
	statfile( SF_PRINT, "                                   border: 1px solid #606060;\n"       );
	statfile( SF_PRINT, "                                   border-collapse: collapse;\n"       );
	statfile( SF_PRINT, "                                   padding: 2px 5px; margin: 10px\n"                );
	statfile( SF_PRINT, "                                 }\n"                                  );
	statfile( SF_PRINT, "    TD.DPI_FRAGS_DEATHS_HEADER   { border: 1px solid #606060; padding: 2px 5px; text-align: center; font-weight:bold }\n" );
	statfile( SF_PRINT, "    TD.DPI_FRAGWEAPON            { border-left : 1px solid #606060; padding: 2px 5px; }\n" );
	statfile( SF_PRINT, "    TD.DPI_FRAGWEAPON_FRAGS      { border-right: 1px solid #606060; padding: 2px 5px; text-align:right; }\n" );	
	statfile( SF_PRINT, "    TD.DPI_DEATHWEAPON           { border-left : 1px solid #606060; padding: 2px 5px; }\n" );
	statfile( SF_PRINT, "    TD.DPI_DEATHWEAPON_DEATHS    { border-right: 1px solid #606060; padding: 2px 5px; text-align:right; }\n" );
	statfile( SF_PRINT, "    TD.DPI_EMPTY_SPACE           { border-top  : 1px solid #606060; padding: 10px 0px; }\n" );
	statfile( SF_PRINT, "\n" );
}
	output_css_colors();

if ( !writing_highscores ) {
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Definition of the ETF teamcolors ( red, blue, etc.)          */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    .COLOR_TEAM_RED         { color : #FF0000; }\n" );
	statfile( SF_PRINT, "    .COLOR_TEAM_BLUE        { color : #0000FF; }\n" );
	statfile( SF_PRINT, "    .COLOR_TEAM_GREEN       { color : #00C600; }\n" );
	statfile( SF_PRINT, "    .COLOR_TEAM_YELLOW      { color : #D7D700; }\n" );
	statfile( SF_PRINT, "    .BACKGROUND_TEAM_RED    { background : #FF0000; }\n" );
	statfile( SF_PRINT, "    .BACKGROUND_TEAM_BLUE   { background : #0000FF; }\n" );
	statfile( SF_PRINT, "    .BACKGROUND_TEAM_GREEN  { background : #00C600; }\n" );
	statfile( SF_PRINT, "    .BACKGROUND_TEAM_YELLOW { background : #D7D700; }\n" );
	statfile( SF_PRINT, "    .COLOR_TEAM_UNKNOWN     { color : #000000; }\n" );
	statfile( SF_PRINT, "\n" );
}
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Style settings for links (clickable players)                 */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    A:link    { text-decoration: none; }\n" );
	statfile( SF_PRINT, "    A:visited { text-decoration: none; }\n" );
	statfile( SF_PRINT, "    A:active  { text-decoration: none; }\n" );
	statfile( SF_PRINT, "    A:hover   { text-decoration: none; }\n" );
	statfile( SF_PRINT, "\n" );
	
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "/*  Style settings for popups                                    */\n" );
	statfile( SF_PRINT, "/*---------------------------------------------------------------*/\n" );
	statfile( SF_PRINT, "    TD.POPUP a:hover            { border: 1px; }\n" );
	statfile( SF_PRINT, "    TD.POPUP a span.popup       { display: none; }\n" );
	statfile( SF_PRINT, "    TD.POPUP a:hover span.popup { display: block; position: absolute; color: #AAA; background: black; border: 1px solid #FFFFFF; padding: 2px; text-align:left; white-space:pre; font-family:monospace}\n" );

}

void output_html_metatags( void )
{
	int t;

	sprintf( outbuf, "<meta name=\"agostats_version\"   content=\"%s\" />\n", VERSION);
	statfile( SF_PRINT, outbuf );

	if ( !writing_highscores ) {
		sprintf( htmlbuf, "%s", map );
		convert_plaintext_to_html( htmlbuf );
		sprintf( outbuf, "<meta name=\"etf_map\"            content=\"%s\" />\n", htmlbuf );
		statfile( SF_PRINT, outbuf );

		/*sprintf( htmlbuf, "%s", map_description );
		convert_plaintext_to_html( htmlbuf );
		sprintf( outbuf, "<meta name=\"etf_mapdesc\"        content=\"%s\" />\n", htmlbuf );
		statfile( SF_PRINT, outbuf );*/
	}

	sprintf( htmlbuf, "" );
	convert_plaintext_to_html( htmlbuf );
	sprintf( outbuf, "<meta name=\"etf_date\"           content=\"%s\" />\n", current_date );
	statfile( SF_PRINT, outbuf );

	if ( !writing_highscores ) {

		// teams should be sorted winner first
		for ( t = 0 ; t < team_count; t++ ) {
			if ( team_participated( sorted_team_nr( t ) ) ) {
				sprintf( htmlbuf, "%s", sorted_team( t )->name_html );
				convert_plaintext_to_html( htmlbuf );
				sprintf( outbuf, "<meta name=\"etf_team%d\"          content=\"%s\" />\n", t+1, htmlbuf  );
				statfile( SF_PRINT, outbuf );

				sprintf( outbuf, "<meta name=\"etf_score%d\"         content=\"%ld\" />\n", t+1, sorted_team( t )->flags_captured );
				statfile( SF_PRINT, outbuf );
			}
		}
	}
}

void output_html_init()
{
	statfile( SF_INIT, NULL );
	statfile( SF_PRINT, HTML_DOCTYPE_XHTML11 );
	statfile( SF_PRINT, "\n" );
	statfile( SF_PRINT, "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" );
if ( writing_highscores ) {
	statfile( SF_PRINT, "<head><title>Enemy Territory Fortress Highscores</title>\n" );
}
else {
	statfile( SF_PRINT, "<head><title>Enemy Territory Fortress match statistics</title>\n" );
}
	statfile( SF_PRINT, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />\n" );
if ( writing_highscores ) {
	statfile( SF_PRINT, "<meta name=\"keywords\"           content=\"agostats,q3f,etf,stats,statistics,enemy territory,player,team,fortress,highscores\" />\n" );
	statfile( SF_PRINT, "<meta name=\"description\"        content=\"Enemy Territory Fortress Highscores\" />\n" );
}
else {
	statfile( SF_PRINT, "<meta name=\"keywords\"           content=\"agostats,q3f,etf,stats,statistics,enemy territory,player,team,fortress\" />\n" );
	statfile( SF_PRINT, "<meta name=\"description\"        content=\"Enemy Territory Fortress Match Statistics\" />\n" );
}
	statfile( SF_PRINT, "<meta name=\"author\"             content=\"AGO Pook\" />\n" );
	statfile( SF_PRINT, "<meta name=\"copyright\"          content=\"the information in this document is free\" />\n" );
	statfile( SF_PRINT, "<meta name=\"resource-type\"      content=\"document\" />\n" );
	statfile( SF_PRINT, "<meta name=\"language\"           content=\"en-us\" />\n" );

	output_html_metatags();

	statfile( SF_PRINT, "\n" );

	statfile( SF_PRINT, "<style type=\"text/css\">\n" );
	statfile( SF_PRINT, "/*<![CDATA[*/\n" );

	// link style for program info
	statfile( SF_PRINT, "    P.PROGRAM_INFO { color : #C0C0C0; font-size: 10px; }\n" );
	statfile( SF_PRINT, "    .PROGRAM_INFO  A:link    { text-decoration: underline; color: #C0C0C0; }\n" );
	statfile( SF_PRINT, "    .PROGRAM_INFO  A:visited { text-decoration: underline; color: #C0C0C0; }\n" );
	statfile( SF_PRINT, "    .PROGRAM_INFO  A:active  { text-decoration: underline; color: #C0C0C0; }\n" );
	statfile( SF_PRINT, "    .PROGRAM_INFO  A:hover   { text-decoration: underline; color: #C0C0C0; }\n" );

	// the idea is to always have a viewable document in itself
	// but let the user make changes as he desires
	// the external stylesheet will overwrite the internal one
	output_write_stylesheet();

	statfile( SF_PRINT, "/*]]>*/\n");
	statfile( SF_PRINT, "</style>\n" );

		// link external stylesheet
	statfile( SF_PRINT, "<link rel=\"stylesheet\" type=\"text/css\" href=\"agostats.css\" />\n" );

	statfile( SF_PRINT, "\n" );
	statfile( SF_PRINT, "</head>\n" );
	statfile( SF_PRINT, "\n" );
	statfile( SF_PRINT, "<body>\n" );

}


void output_html_line( char *info)
{
	// replace "\n" with "<br />\n" in string
	str_replace( info, "\n", "<br />\n" );
	
	if ( info != NULL ) {
		statfile( SF_PRINT, info );
	}
}

void output_plain( char *info)
{
	if ( info != NULL ) {
		statfile( SF_PRINT, info );
	}
}

void output_html_term()
{	// EXPERIMENTAL, the if condition is not correct yet, program info is not output in normal stats
//	if ( writing_highscores && !(writing_highscore_player_id > 0) && !writing_linkpage ) {
		output_program_info();
//	}
	statfile( SF_PRINT, "</body>\n" );
	statfile( SF_PRINT, "</html>\n" );
	statfile( SF_TERM, "" );
}

void AS_OUT( OUTPUT_ELEMENT e, void *info )
{
	static int initialized = FALSE;

	switch ( e ) {

		case OE_INIT:
			if ( initialized == FALSE ) {
				output_html_init();
			}
			initialized = TRUE;
			break;

		case OE_LINE:
			if ( initialized == FALSE ) {
				AS_OUT( OE_INIT, NULL );
			}
			output_html_line( (char*)info );
			break;
		
		case OE_PLAIN:
			if ( initialized == FALSE ) {
				AS_OUT( OE_INIT, NULL );
			}
			output_plain( (char*)info );
			break;

		case OE_TERM:
			if ( initialized == TRUE ) {
				output_html_term();
			}
			initialized = FALSE;
			break;

		default:

		break;
	}

}	

char* css_team_background_color( int team )
{
	static char result[ 50 ];
	
	result[ 0 ] = '\0';

	if ( 0 == strcmp( TEAMS[ team ].name, "RED" )) {
		strncpy( result, CSS_BACKGROUND_TEAM_RED, 50 );
	}
	if ( 0 == strcmp( TEAMS[ team ].name, "BLUE" )) {
		strncpy( result, CSS_BACKGROUND_TEAM_BLUE, 50 );
	}
	if ( 0 == strcmp( TEAMS[ team ].name, "GREEN" )) {
		strncpy( result, CSS_BACKGROUND_TEAM_GREEN, 50 );
	}
	if ( 0 == strcmp( TEAMS[ team ].name, "YELLOW" )) {
		strncpy( result, CSS_BACKGROUND_TEAM_YELLOW, 50 );
	}

	return result;
}
