#if !defined(OUTPUT_H_INCLUDED)
#define OUTPUT_H_INCLUDED

#include "tools.h"
#include "game.h"

#define STATSFILE			"stats"
#define EXT_HTML			"htm"

// foreground colors
#define CSS_COLOR_TEAM_UNKNOWN      "COLOR_TEAM_UNKNOWN"
#define CSS_COLOR_TEAM_RED			"COLOR_TEAM_RED"
#define CSS_COLOR_TEAM_BLUE			"COLOR_TEAM_BLUE"
#define CSS_COLOR_TEAM_GREEN		"COLOR_TEAM_GREEN"
#define CSS_COLOR_TEAM_YELLOW		"COLOR_TEAM_YELLOW"

// background colors
#define CSS_BACKGROUND_TEAM_RED		"BACKGROUND_TEAM_RED"
#define CSS_BACKGROUND_TEAM_BLUE	"BACKGROUND_TEAM_BLUE"
#define CSS_BACKGROUND_TEAM_GREEN	"BACKGROUND_TEAM_GREEN"
#define CSS_BACKGROUND_TEAM_YELLOW	"BACKGROUND_TEAM_YELLOW"

// HTML definitions
#define HTML_DOCTYPE_XHTML10_TRANSITIONAL "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
#define HTML_DOCTYPE_XHTML10_STRICT       "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
#define HTML_DOCTYPE_XHTML11              "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">"

#define HTML_SET_COLOR_STRING	"<span class=\"%s\">%s</span>" // (color,string)

#define HTML_PLAYER_CLICKABLE   "<a href=\"#SUPERGREG_NUMBA_%d\">%s</a>" // takes playernumber and htmlname as params
#define HTML_PLAYER_LOCATION    "<a id=\"SUPERGREG_NUMBA_%d\">%s</a>" // takes playernumber and htmlname as params

// new constants
#define HTML_PARENT_TABLE_INIT  "<table summary=\"This is the top level table containing all the other tables\" class=\"MAIN\"><tr><td>\n"
#define HTML_PARENT_TABLE_TERM  "</td></tr></table>\n"

// MM1 log
// AGO Pook                      Hello
#define HTML_EVENTLOG_INIT      "<table summary=\"This table lists the events and chats\" class=\"EVENTS\">\n"
#define HTML_EVENTLOG_FORMAT    " <tr><td class=\"EVENTS_COLUMN1\">%s</td><td class=\"EVENTS_COLUMN2\">%s</td></tr>\n"
#define HTML_EVENTLOG_TERM      "</table>\n"

// The Result
#define HTML_RESULT_INIT        "<div class=\"RESULT\">\n"
#define HTML_RESULT_LINE		"<span class=\"RESULT\">%s</span>\n"
#define HTML_TEAM_TAG			"http://www.google.com/search?hl=en&amp;q=etf+clan+%s&amp;btnG=Google+Search"
#define HTML_RESULT_TERM        "</div>\n"

// The Game
#define HTML_GAME_INIT          "<table class=\"THE_GAME\"><caption class=\"THE_GAME\">The Game</caption>\n"
#define HTML_GAME_LINE          " <tr><td class=\"THE_GAME\">%s</td></tr>\n"
#define HTML_GAME_TERM          "</table>\n"

// The Teams
#define HTML_TEAM_INIT			"<table summary=\"This table lists the teams\" class=\"THE_TEAMS\"><caption class=\"THE_TEAMS\">The Teams</caption>\n"
#define HTML_TEAM_ROW_INIT		" <tr>"
#define HTML_TEAM_ROW_TERM		"</tr>\n"
											// Empty                      // Red			
#define HTML_TEAM_HEAD			"<td class=\"THE_TEAMS_HEADER\" colspan=\"3\">%s</td>"
//#define HTML_TEAM_PLAYER		"<td class=\"THE_TEAMS_ROLE %s\">%s</td><td class=\"THE_TEAMS_PLAYER\" colspan=\"2\">%s</td>"
#define HTML_TEAM_PLAYER		"<td class=\"THE_TEAMS_ROLE %s\">%s</td><td class=\"THE_TEAMS_PLAYER\">%s</td><td></td>"

#define HTML_TEAM_TERM			"</table>\n"
#define HTML_TEAM_HEADER_OBJ    "<td class=\"THE_TEAMS_HEADER_OBJ\" colspan=\"3\">Objectives</td>"
#define HTML_TEAM_HEADER_DEF    "<td class=\"THE_TEAMS_HEADER_DEF\" colspan=\"3\">Defence squad</td>"
#define HTML_TEAM_HEADER_OFF    "<td class=\"THE_TEAMS_HEADER_OFF\" colspan=\"3\">Offence squad</td>"
#define HTML_TEAM_HEADER_FND    "<td class=\"THE_TEAMS_HEADER_FND\" colspan=\"3\">Frags 'n deaths</td>"

//#define HTML_TEAM_LINE_OBJ	    "<td class=\"THE_TEAMS_LINE_OBJ\" colspan=\"3\">%s</td>"
#define HTML_TEAM_LINE_OBJ_LONG "<td class=\"THE_TEAMS_LINE_OBJ\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_OBJ_NUMBER\">%ld</td>"
#define HTML_TEAM_LINE_OBJ_DOUBLE "<td class=\"THE_TEAMS_LINE_OBJ\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_OBJ_NUMBER\">%*.*lf</td>"
#define HTML_TEAM_LINE_OBJ_STRING "<td class=\"THE_TEAMS_LINE_OBJ\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_OBJ_NUMBER\">%s</td>"

//#define HTML_TEAM_LINE_DEF	    "<td class=\"THE_TEAMS_LINE_DEF\" colspan=\"3\">%s</td>"
#define HTML_TEAM_LINE_DEF_LONG	  "<td class=\"THE_TEAMS_LINE_DEF\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_DEF_NUMBER\">%ld</td>"
#define HTML_TEAM_LINE_DEF_DOUBLE "<td class=\"THE_TEAMS_LINE_DEF\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_DEF_NUMBER\">%*.*lf</td>"
#define HTML_TEAM_LINE_DEF_STRING "<td class=\"THE_TEAMS_LINE_DEF\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_DEF_NUMBER\">%s</td>"

//#define HTML_TEAM_LINE_OFF	    "<td class=\"THE_TEAMS_LINE_OFF\" colspan=\"3\">%s</td>"
#define HTML_TEAM_LINE_OFF_LONG	  "<td class=\"THE_TEAMS_LINE_OFF\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_OFF_NUMBER\">%ld</td>"
#define HTML_TEAM_LINE_OFF_DOUBLE "<td class=\"THE_TEAMS_LINE_OFF\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_OFF_NUMBER\">%*.*lf</td>"
#define HTML_TEAM_LINE_OFF_STRING "<td class=\"THE_TEAMS_LINE_OFF\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_OFF_NUMBER\">%s</td>"

//#define HTML_TEAM_LINE_FND	    "<td class=\"THE_TEAMS_LINE_FND\" colspan=\"3\">%s</td>"
#define HTML_TEAM_LINE_FND_LONG	  "<td class=\"THE_TEAMS_LINE_FND\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_FND_NUMBER\">%ld</td>"
#define HTML_TEAM_LINE_FND_DOUBLE "<td class=\"THE_TEAMS_LINE_FND\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_FND_NUMBER\">%*.*lf</td>"
#define HTML_TEAM_LINE_FND_STRING "<td class=\"THE_TEAMS_LINE_FND\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_LINE_FND_NUMBER\">%s</td>"
#define HTML_TEAM_LINE_EMPTY      "<td class=\"THE_TEAMS_LINE_EMPTY\" colspan=\"3\">&#160;</td>"

#define HTML_TEAM_FRAGS_BY_WEAPON_HEADER	"<td class=\"THE_TEAMS_FRAGS_BY_WEAPON_HEADER\" colspan=\"3\">Frags by weapon</td>"
//#define HTML_TEAM_FRAGS_BY_WEAPON_LINE		"<td></td><td class=\"THE_TEAMS_FRAGS_BY_WEAPON_NAME\">%s</td><td class=\"THE_TEAMS_FRAGS_BY_WEAPON\">%s</td>"
#define HTML_TEAM_FRAGS_BY_WEAPON_LINE		"<td class=\"THE_TEAMS_FRAGS_BY_WEAPON_NAME\" colspan=\"2\">%s</td><td class=\"THE_TEAMS_FRAGS_BY_WEAPON\">%s</td>"

// Teamscore (capture) History
#define HTML_TEAMSCOREHISTORY_INIT	     "<table summary=\"This table contains the teamscore history\" class=\"CAPTURES\"><caption class=\"CAPTURES\">Captures</caption>\n"
//#define HTML_TEAMSCOREHISTORY_HEADER  "<th class=\"CAPTURES\">%s</th>"
#define HTML_TH_TEAMSCOREHISTORY_TEAM    "<th class=\"CAPS_TEAMNAME\">%s</th>"
#define HTML_TH_TEAMSCOREHISTORY_PLAYER  "<th class=\"CAPS_PLAYER\">%s</th>"
#define HTML_TH_TEAMSCOREHISTORY_TOUCHES "<th class=\"CAPS_TOUCHES\" colspan=\"2\">%s</th>"
#define HTML_TD_TEAMSCOREHISTORY_SCORE   "<td class=\"CAPS_SCORE\">%d</td>"
#define HTML_TD_TEAMSCOREHISTORY_PLAYER  "<td class=\"CAPS_PLAYER\">%s</td>"
#define HTML_TD_TEAMSCOREHISTORY_TOUCHES "<td class=\"CAPS_TOUCHES\">%ld</td>"
#define HTML_TD_TEAMSCOREHISTORY_C2C     "<td class=\"CAPS_COAST2COAST\">%s</td>"

#define HTML_TEAMSCOREHISTORY_TERM	 "</table>\n"

// The players
// Role Player                       Frags     Deaths  Teamkills  TK deaths   Suicides    Touches   Captures
// ---------------------------------------------------------------------------------------------------------
// DEF  AGO Pook                        25         21          1          8          6          0          0
#define HTML_PLAYERLIST_INIT     "<table summary=\"This table lists some values grouped by player\" class=\"THE_PLAYERS\" ><caption class=\"THE_PLAYERS\">The Players</caption>\n"
#define HTML_PLAYERLIST_HEADER   " <tr><td class=\"THE_PLAYERS_HEADER_ROLE\">%s</td><td class=\"THE_PLAYERS_HEADER_NAME\">%s</td><td class=\"THE_PLAYERS_HEADER_FRAGS\">%s</td><td class=\"THE_PLAYERS_HEADER_DEATHS\">%s</td><td class=\"THE_PLAYERS_HEADER_TEAMKILLS\">%s</td><td class=\"THE_PLAYERS_HEADER_TEAMDEATHS\">%s</td><td class=\"THE_PLAYERS_HEADER_SUICIDES\">%s</td><td class=\"THE_PLAYERS_HEADER_FLAGTOUCHES\">%s</td><td class=\"THE_PLAYERS_HEADER_CAPTURES\">%s</td></tr>\n"
#define HTML_PLAYERLIST_LINE     " <tr><td class=\"THE_PLAYERS_ROLE %s\">%s</td><td class=\"THE_PLAYERS_NAME\">%s</td><td class=\"THE_PLAYERS_FRAGS\">%ld</td><td class=\"THE_PLAYERS_DEATHS\">%ld</td><td class=\"THE_PLAYERS_TEAMKILLS\">%ld</td><td class=\"THE_PLAYERS_TEAMDEATHS\">%ld</td><td class=\"THE_PLAYERS_SUICIDES\">%ld</td><td class=\"THE_PLAYERS_FLAGTOUCHES\">%ld</td><td class=\"THE_PLAYERS_CAPTURES\">%ld</td></tr>\n"
#define HTML_PLAYERLIST_LINE_OFF " <tr><td class=\"THE_PLAYERS_OFF_ROLE %s\">%s</td><td class=\"THE_PLAYERS_OFF_NAME\">%s</td><td class=\"THE_PLAYERS_OFF_FRAGS\">%ld</td><td class=\"THE_PLAYERS_OFF_DEATHS\">%ld</td><td class=\"THE_PLAYERS_OFF_TEAMKILLS\">%ld</td><td class=\"THE_PLAYERS_OFF_TEAMDEATHS\">%ld</td><td class=\"THE_PLAYERS_OFF_SUICIDES\">%ld</td><td class=\"THE_PLAYERS_OFF_FLAGTOUCHES\">%ld</td><td class=\"THE_PLAYERS_OFF_CAPTURES\">%ld</td></tr>\n"
#define HTML_PLAYERLIST_TERM     "</table>\n"

// Player vs Player (fragmatrix)
#define HTML_FRAGMAP_GROUP_INIT             "<table summary=\"This table is container for the Player versus player and legend tables\" class=\"PVP_GROUP\"><tr><td>\n"
#define HTML_FRAGMAP_GROUP_TERM             "</td></tr></table>\n"

#define HTML_FRAGMAP_INIT					"<table summary=\"Player versus player\" class=\"PLAYER_VS_PLAYER\" ><caption class=\"PLAYER_VS_PLAYER\">Player vs Player</caption>\n"
#define HTML_FRAGMAP_ROW_INIT				" <tr>"
#define HTML_FRAGMAP_ROW_TERM				"</tr>\n"
#define HTML_FRAGMAP_CELL_TOPLEFT			"<td class=\"PVP_TOPLEFT\"></td><td class=\"PVP_HEADER_ROLE\">Role</td><td class=\"PVP_HEADER_PLAYER\">Player</td>"
#define HTML_FRAGMAP_CELL_PLAYER			"<td class=\"PVP_PLAYER_NUMBER\">%ld</td><td class=\"PVP_PLAYER_ROLE %s\">%s</td><td class=\"PVP_PLAYER_NAME\">%s</td>"
#define HTML_FRAGMAP_CELL_LONG				"<td class=\"PVP_NUMBER\">%ld</td>"

#define HTML_FRAGMAP_CELL_LONG_SUICIDE_INIT	"<td class=\"PVP_SUICIDE POPUP\"><a href=\"#\"><span class=\"SUICIDE\">%ld</span>"
#define HTML_FRAGMAP_CELL_LONG_SUICIDE_TERM	"</a></td>"

#define HTML_FRAGMAP_CELL_POPUP_INIT        "<span class=\"popup\">"
#define HTML_FRAGMAP_CELL_POPUP_TERM        "</span>"

// The <a href=""> is only needed to make popups work for internet explorer, just <a> works for the other browsers
#define HTML_FRAGMAP_CELL_LONG_TK_INIT		"<td class=\"PVP_TEAMKILL POPUP\"><a href=\"#\"><span class=\"TEAMKILL\">%ld</span>"
#define HTML_FRAGMAP_CELL_LONG_TK_TERM		"</a></td>"

#define HTML_FRAGMAP_CELL_LONG_FRAG_INIT	"<td class=\"PVP_FRAG POPUP\"><a href=\"#\"><span class=\"FRAG\">%ld</span>"
#define HTML_FRAGMAP_CELL_LONG_FRAG_TERM	"</a></td>"

#define HTML_FRAGMAP_CELL_LONG_CHASE_INIT	"<td class=\"PVP_CHASE POPUP\"><a href=\"#\"><span class=\"CHASE\">%ld</span>"
#define HTML_FRAGMAP_CELL_LONG_CHASE_TERM	"</a></td>"

#define HTML_FRAGMAP_CELL_LONG_DEF_ATTACK_INIT	"<td class=\"PVP_DEF_ATTACK POPUP\"><a href=\"#\"><span class=\"DEF_ATTACK\">%ld</span>"
#define HTML_FRAGMAP_CELL_LONG_DEF_ATTACK_TERM	"</a></td>"

#define HTML_FRAGMAP_CELL_STRING			"<td class=\"PVP_STRING\">%s</td>"
#define HTML_FRAGMAP_HINT                   "<span class=\"PVP_HINT\">Hint: hover your mousepointer over the numbers, click on playernames for detailed info</span>\n"
#define HTML_FRAGMAP_TERM					"</table>\n"

// PVP Color Legend
#define HTML_PVP_LEGEND_INIT				"<table summary=\"Legend for Player versus player table\" class=\"PVP_LEGEND\"><caption class=\"PVP_LEGEND\">Color legend</caption>\n"
#define HTML_PVP_LEGEND_LINE				" <tr><td class=\"PVP_LEGEND_NUMBER\"><span class=\"%s\">%ld</span></td><td class=\"PVP_LEGEND_EXPLANATION\">%s</td></tr>\n"
#define HTML_PVP_LEGEND_TERM				"</table>\n"

// The Awards
#define HTML_AWARD_INIT           "<table summary=\"This table contains the awards\" class=\"THE_AWARDS\"><caption class=\"THE_AWARDS\">The Awards</caption>\n"
#define HTML_AWARD_HIGHSCORE_INIT "<table summary=\"This table contains the highscores\" class=\"THE_AWARDS\"><caption class=\"THE_AWARDS\">The Highscores</caption>\n"
#define HTML_AWARD_ROW_INIT       " <tr>"
#define HTML_AWARD_GROUPNAME      " <tr><td class=\"THE_AWARDS_GROUPNAME\">%s</td></tr>\n"
#define HTML_AWARD_NAME			  "<td class=\"THE_AWARDS_AWARDNAME\">%s</td>"
#define HTML_AWARD_NAME_HIGHSCORE "<td class=\"THE_AWARDS_AWARDNAME\">%s<span class=\"HIGHSCORE_INDICATOR\">*</span></td>"
#define HTLM_HIGHSCORE_DATE		  "<td class=\"HIGHSCORE_DATE\">%s</td>" 
#define HTML_AWARD_WINNER         "%s"
#define HTML_AWARD_ROW_TERM       "</tr>\n"
#define HTML_AWARD_TERM           "</table>\n"

// Detailed player info
#define HTML_PLAYER_INIT		 "<table summary=\"This table contains detailed player info\" class=\"DETAILED_PLAYER_INFO\"><caption class=\"DETAILED_PLAYER_INFO\">Detailed player info</caption>\n"
#define HTML_PLAYER_NAME		 " <tr><td class=\"DPI_PLAYER_NAME\" colspan=\"4\">%s</td></tr>\n"
#define HTML_TEAM_COLOR_BAR		 " <tr><td class=\"DPI_TEAMCOLOR %s\" colspan=\"4\"></td></tr>\n"
#define HTML_PLAYER_DEBUG        " <tr><td class=\"DPI_PLAYER_DEBUG\" colspan=\"4\">Debug</td></tr>\n"
#define HTML_PLAYER_DEF          " <tr><td class=\"DPI_PLAYER_DEF\" colspan=\"4\">Defence</td></tr>\n"
#define HTML_PLAYER_OFF          " <tr><td class=\"DPI_PLAYER_OFF\" colspan=\"4\">Offence</td></tr>\n"
#define HTML_PLAYER_FND          " <tr><td class=\"DPI_PLAYER_FND\" colspan=\"4\">Frags \'n deaths</td></tr>\n"
//#define HTML_PLAYER_INFO_INIT	 " <tr><td class=\"DPI_PLAYER_INFO_BEGIN\" colspan=\"4\"> "
//#define HTML_PLAYER_INFO_PROCESS "%s<br /> "
#define HTML_PLAYER_INFO_PROCESS_LONG   " <tr><td class=\"DPI_PLAYER_INFO_DESCRIPTION\" colspan=\"2\">%s</td><td class=\"DPI_PLAYER_INFO_NUMBER\" colspan=\"2\">%*ld</td></tr>\n"
#define HTML_PLAYER_INFO_PROCESS_DOUBLE " <tr><td class=\"DPI_PLAYER_INFO_DESCRIPTION\" colspan=\"2\">%s</td><td class=\"DPI_PLAYER_INFO_NUMBER\" colspan=\"2\">%*.*lf</td></tr>\n"
#define HTML_PLAYER_INFO_PROCESS_STRING " <tr><td class=\"DPI_PLAYER_INFO_DESCRIPTION\" colspan=\"2\">%s</td><td class=\"DPI_PLAYER_INFO_NUMBER\" colspan=\"2\">%s</td></tr>\n"
//#define HTML_PLAYER_INFO_TERM	 "</td></tr>\n"
// try to have some empty space between players
// IE does this differently from firefox, if the &#160;
// is left out, ie will leave no space between the previous
// and playerinfos, firefox does.
#define HTML_PLAYER_VOID        " <tr><td class=\"DPI_EMPTY_SPACE\" colspan=\"4\">&#160;</td></tr>\n"
#define HTML_PLAYER_TERM		"</table>\n<br />\n<br />\n"

// Detailed player info - frags and deaths per weapon
#define HTML_FD_BY_WEAPON_INIT    "<table summary=\"This table contains frags and deaths per weapon for a player\" class=\"DPI_FRAGS_DEATHS\">\n"
#define HTML_FD_BY_WEAPON_HEADER  " <tr><td class=\"DPI_FRAGS_DEATHS_HEADER\" colspan=\"2\">%s</td><td class=\"DPI_FRAGS_DEATHS_HEADER\" colspan=\"2\">%s</td></tr>\n"
#define HTML_FD_BY_WEAPON_LINE    " <tr><td class=\"DPI_FRAGWEAPON\">%s</td><td class=\"DPI_FRAGWEAPON_FRAGS\">%s</td><td class=\"DPI_DEATHWEAPON\">%s</td><td class=\"DPI_DEATHWEAPON_DEATHS\">%s</td></tr>\n"
#define HTML_FD_BY_WEAPON_TERM    "</table>\n"

#define HTML_PROGRAM_INFO_INIT    "<hr />\n<p class=\"PROGRAM_INFO\">\n"
#define HTML_PROGRAM_INFO_LINE    ""
#define HTML_PROGRAM_INFO_TERM    "</p>\n"

// touches and captures table
#define HTML_TOUCHES_CAPS_INIT    "<table summary=\"This table contains flagtouches and captures grouped by player\" class=\"FLAG_ACTION\"><caption class=\"FLAG_ACTION\"><a class=\"FLAG_ACTION\" href=\"http://en.wikipedia.org/wiki/Prima_donna\">The Prima Donnas</a></caption>\n"
#define HTML_TOUCHES_CAPS_HEADER  " <tr><td class=\"FLAG_ACTION_HEADER\" colspan=\"2\">%s</td><td class=\"FLAG_ACTION_HEADER\" colspan=\"2\">%s</td></tr>\n"
#define HTML_TOUCHES_CAPS_LINE    " <tr><td class=\"FLAG_ACTION_PLAYER\">%s</td><td class=\"FLAG_ACTION_SCORE\">%s</td><td class=\"FLAG_ACTION_PLAYER\">%s</td><td class=\"FLAG_ACTION_SCORE\">%s</td></tr>\n"
#define HTML_TOUCHES_CAPS_TERM	  "</table>\n"

#define FILENAME_FORBIDDEN			"\\/:*?\"<>|" // characters that are forbidden in filename (in windows for now)

// Statfile output constants
enum {
	SF_INIT,
	SF_PRINT,
	SF_TERM
}; // SF_MESSAGES;


// enumerate Output Elements

typedef enum {
	OE_INIT,
	OE_LINE,
	OE_PLAIN, // already encoded as txt or html, send straight to output
	OE_TERM
} OUTPUT_ELEMENT;

typedef enum { // output types
	OT_TXT,
	OT_HTML
} _OUTPUT_TYPE;

typedef struct _game_color_to_css {
	char color_code;        // the character '0' in "^0"
	char css_color  [ 20 ]; // the name of the CSS class "ET_C0"
	char rgb_value  [ 10 ]; // the rgb number of the color "#123456"
} t_game_color_to_css;

extern _OUTPUT_TYPE OUTPUT_TYPE; // txt or html
extern int			USE_STDOUT ;
extern int			CUSTOM_OUTPUTFILE;
extern int			CUSTOMIZE_FILENAME;
extern char			custom_outputfile[];
extern char			filename_options[];
extern char			outbuf[];
extern char			htmlbuf[];
extern int			OUTPUT_EVENTS ;  // captures, disconnected, renamed etc. default off

// prototypes
extern char* et_2_html_v2( char *et_string );
extern void  remove_et_color_codes( char *et_string );
extern char* et_2_html( char *et_string );
extern char* color_string( char *source, char *color );
extern void  set_output_type_obsolete( _OUTPUT_TYPE ot );
extern int   get_output_type_obsolete( void );
extern void  statfile( int action, char *message ); // todo: make this one private
extern void  AS_OUT( OUTPUT_ELEMENT e, void* info );
extern char* html_player_clickable( int p );
extern char* html_player_location( int p );
//extern void  rename_output_file( void );
extern char* css_team_background_color( int team );
extern void  convert_plaintext_to_html( char *plaintext );
extern void  delete_statfile( void );
#endif // !defined(OUTPUT_H_INCLUDED)
