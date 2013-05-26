// matching
#if !defined( MATCHING_H )
#define MATCHING_H


// defines
#define LINE_BUFFER_SIZE			4096
#define MAX_PATTERN_SIZE			128 // should give a warning when too small
#define VAR_ARRAY_ELEMENT_SIZE		255
#define VAR_ARRAY_SIZE				10 // means 10 %r variables in a line can be used

// line type , todo: convert everything to enum
typedef enum {
	LT_UNKNOWN,
	LT_AGOSTATS,
	LT_IGNORE,
	LT_OBJECTIVE,
	LT_CLIENT_ONLY,
	LT_FRAG,
	LT_CHAT,
	LT_TEAMCHAT,
	LT_MAPINFO,
	LT_INFO,
	LT_PRINT,
	LT_STATUS_CHANGE,
	LT_DEBUG,
	LT_RENAME,
	LT_CONNECT,
	LT_SPECTATE,
	LT_DISCONNECT,
	LT_JOIN,
	LT_TIMELIMIT,
	LT_THROW_PIN,
	LT_KICK,
	// Matchlog
	LT_MATCHLOG_FRAG,
	LT_MATCHLOG_SUICIDE,
#if defined(SERVER_STATS)
	LTS_FRAG,
	LTS_INITGAME,
	LTS_MAPINFO,
	LTS_MOD_CUSTOM,
	LTS_RENAME,
	LTS_SETTEAM,
#endif

	LT_END
} _line_type;

typedef enum {
	NO_INFO,
	//INFO_MAP,
	INFO_GAMEINDEX,
	INFO_DATE,
	INFO_TIME,
	INFO_DESCRIPTION,
	INFO_MAPNAME,
	INFO_MAPSIGNATURE,
#if defined(SERVER_STATS)
	PLAYER_BY_NUMBER,
	PLAYER_BY_NAME,
#endif
	INFO_MATCHLOG,
	INFO_END
} _line_type_modifiers;


// tokens
#define TK_STORE			'%'
#define TK_CHOICE_START		'{'
#define TK_SEPARATOR		'|'
#define TK_CHOICE_END		'}'
#define TK_OPTIONAL_START	'['
#define TK_OPTIONAL_END		']'
#define TK_END_OF_LINE		'\0'
#define TK_UNMATCH			'~'
#define TK_ESCAPE           '\\'

// types

typedef struct _pattern {
	char pat[ MAX_PATTERN_SIZE ];
	long line_type;
	long info1;
	long info2;
	long info3;
	long info4;

} t_pattern;


// public variables

// variables that contain the info extracted from the line
extern int chatline;
extern int teamchatline;
extern int teamkill;
extern int suicide ;
extern int	team_nr;
extern int flag_nr;
extern int player_nr;
extern int killer_nr;
extern int victim_nr;
extern int squad_nr;
extern int old_line_type;
extern int old_player_nr;
extern int old_flag_nr;
extern int old_line_info1;
extern int line_type;

extern char linebuffer[];
extern char newname[];
extern char killer[];
extern char victim[];
extern char location[];
extern char debug[];
extern char chat[];
extern char team[];
extern char flag[];
extern char player[];
extern char ammo[];
extern char weapon[];
extern char action[];
extern char building[];
extern char optional[];
extern char array[ VAR_ARRAY_SIZE ][ VAR_ARRAY_ELEMENT_SIZE ];
extern int  iar; // index, belongs to array, set to 0 before parsing each line
extern char map[];

extern t_pattern LOG_PATTERNS[];

// public functions
extern int	match( char *line, char *pattern );
extern void clear_line_info();

#endif

