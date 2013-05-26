// game module
#if !defined(GAME_H)
#define GAME_H

// game types
#define GT_UNKNOWN					0x00000000
#define GT_CAPTURE_THE_FLAG			0x00000001 // capture the flag, can have x flags and y teams
#define GT_CAPTURE_AND_HOLD			0x00000002 // capture and hold, can have x locations and y teams
#define GT_DUELL					0x00000003 // 1 on 1
#define GT_KEEPAWAY					0x00000004 // todo:map etf_bringit : one flag keep away

// game type modifiers
#define GTM_ONE_FLAG				0x00010000 // take flag from middle to caplocation
#define GTM_REVERSE					0x00020000 // take the flag from your base to other base
#define GTM_ADVANCED				0x00040000 // contains advanced ctf messages

#define MASK_BASIC_GAMETYPE			0x0000FFFF
#define MASK_GAMETYPE_MODIFIER		0xFFFF0000
// examples:
// forts advanced ctf = GT_CTF + GTM_ADVANCED
// forts reverse = GT_CTF + GTM_REVERSE + GTM_ADVANCED
// spazzball = GT_CTF + GTM_ONE_FLAG + GTM_REVERSE (4 teams 1 flag)
// castles   = GT_CTF (4 teams 4 flags)
// sorrow    = GT_CAPTURE_HOLD + GTM_ONE_FLAG
// canalzone = GT_CAPTURE_HOLD

// game objectives
typedef enum {
	O_NO_OBJECTIVE = 0,

	// CTF
	O_CTF_FLAGTAKE,
	O_CTF_FLAGDROP,
	O_CTF_FLAGCAPTURE,
	O_CTF_FLAG_RETURNED,
	
	// Advanced CTF
	O_ACTF_FRAG_ENEMY_CARRIER,		// Advanced CTF stuff
	O_ACTF_FRAG_FRIENDLY_CARRIER,
	O_ACTF_FRAG_CAP_1,				// bonuspoints for fragging 1 enemy while capping the flag
	O_ACTF_FRAG_CAP_MULTI,			// bonuspoints for fragging multiple enemies while capping the flag
	O_ACTF_FLAG_DEFEND_BASE,		
	O_ACTF_FLAG_DEFEND_FIELD,
	O_ACTF_CARRIER_DEFEND,
	
	// capture and hold
	O_CAH_ITEM_TAKE,				// for instance on sorrow where there is one item in the game for capping
	O_CAH_ITEM_DROP,				// for instance on sorrow where there is one item in the game for capping
	O_CAH_LOCATION_CLAIM,			// when a player captures a location, for instance sorrow, canalzone
	O_CAH_ALL_CLAIMED,				// when a team has claimed every location
	O_CAH_VICTORY					// when also the other base is capped in canalzone

} GAME_OBJECTIVES;

// extra options
#define EX_DEDUCT_TEAM			0x00000001 // deduct, determine which player is in which team
#define EX_PLAYER_IN_TEAM		0x00000002
#define EX_PLAYER_NOT_IN_TEAM	0x00000004
#define EX_CAPTURE_BY_RED		0x00000008 // for Rock
#define EX_CAPTURE_BY_BLUE		0x00000010 // for Rock
#define EX_NO_FLAG_INFO			0x00000020 // for Alps, the capture message doesn't say which flag

// todo: use
/*
typedef enum {
	PLAYER_IN_TEAM,
	PLAYER_NOT_IN_TEAM,
	PLAYER_IN_TEAM_FLAG,
	PLAYER_NOT_IN_TEAM_FLAG
} DEDUCT_PLAYER_TEAM;
*/

// game status
#define GS_UNKNOWN					1
#define GS_PRE_MATCH				2
#define GS_LIVE						3
#define GS_POST_MATCH				4 // after "timelimit hit"


// types
typedef struct _match_stats {
	long players;
	long teams;
	long captures;
	long destroyed_sentry; // only count enemy sentries
	long destroyed_supply;
	long flagtouches;
	long frags;
	long suicides;
	long teamkills;

	// action lines
	long first_line;
	long last_line;
	long actionlinecount;
} match_stats;

// EVENT TYPE
typedef enum {
	ET_UNDEFINED,      
	ET_CHAT,       // messagemode 1 chat
	ET_OBJECTIVE,  // player captures
	ET_GAME,       // begins / ends
	ET_OTHER       // for instance, connect disconnect rename
} EVENT_TYPE;

// public variables
extern int          game_type;
extern int	        game_status;
extern long			game_count;
extern match_stats  MATCH;
extern char			eventbuf[];
extern int			OUTPUT_MM1;

// public functions
extern void  calculate_match_statistics	( void );
extern void  process_status_change		( long new_status );
extern char* plain_text					( char *name );
extern int   is_colorcode				( char *et_string );
extern void  set_gametype				( long gt );
extern long  query_gametype				( void );
extern long  query_gametype_modifier	( void );
extern char* query_gametype_string		( void );
extern long  determine_gametype			( long line_game_type );
extern void  determine_gametype_final	( void );
extern int   empty_gametype_modifier	( void );
extern int   gtm_contains				( long gametype_modifier );
extern int   query_output_defence		( void );
extern int   query_output_offence		( void );
extern int   query_output_frags_and_deaths( void );
extern void  mark_match_line            ( long linenumber );
extern void  init_match_totals          ( void );
#endif

