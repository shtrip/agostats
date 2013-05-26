// teams module
#if !defined(TEAMS_H)
#define TEAMS_H


#include "globals.h"
#include "agostats.h"
#include "players.h"
#include "squads.h"
#include "weapons.h"
#include "game.h"

// defines
#define PLAYERS_DIFFERENT_TEAM	'-'
#define PLAYERS_SAME_TEAM		'+'
#define TEAM_RELATION_UNKNOWN	'?'
#define TEAM_UNKNOWN            -1

#if defined(SERVER_STATS)
#define TEAM_SPECTATOR			5
#endif

#define sorted_team_nr(i)	team_index [ i ]
//#define sorted_team(i)		((team_stats)(TEAMS[ team_index [ i ]]))

// types
// todo: maybe add a color property for "RED" "BLUE", 
// name should then be either color or clantag
typedef struct _team_stats {
	char name          [ 50 ];
	char name_html     [ 50 ];
	int  playerlist	   [ MAX_PLAYERS ];
	int  player_count;
	char clantag       [ 50 ];  // trying to extract clantags
	char clantag_html  [ 512 ]; // trying to extract clantags
	char filename	   [ 50 ];  // either name or clantag without forbidden filename characters 
	squad_stats squad  [ MAX_SQUADS ]; // unknown, def, off
	long flags_captured  ; // how many times this team captured an enemy flag
	long flags_lost		 ; // how many times this flag was captured.

	// gather from playerstats
	long frags;			// a frag is a kill on enemy
	long deaths;		// a death is caused by enemy
	long teamkills;		// a teamkill is a kill on a teammate
	long teamdeaths;	// a teamdeath is caused by teammate
	long suicides;
	long destroyed_sentry; // only count enemy sentries
	long destroyed_supply; // only count enemy supply stations
	long lost_sentry     ; // count how many teamsentries were lost
	long lost_supply     ; // count how many team supply stations were lost
	long first_touches;
	long flagtouches;
	long frags_per_weapon [ W_NR_WEAPONS ];
	long deaths_per_weapon[ W_NR_WEAPONS ];
	long sit_on_grenade;

	// objective stuff
	long defend_flag_at_base;
	long defend_flag_in_field;
	long defend_flagcarrier;
	long killed_flagcarrier;
	long teamkilled_flagcarrier;
	long killed_near_flag_in_base;
	
	// other
	long score;              // calculate something like ( captured flags - lost flags )
	
} team_stats;

typedef struct _teamscore_type {
	int  scoring_team;      // this team captures
	int  yielding_team;     // against this team
	int  scoring_player;    // this is the player that captures
	long flagtouches;       // number of flagtouches before the flag was captured
	
} teamscore_type;

// Team matrix
typedef enum {
	TM_UNDEFINED,
	TM_INIT,
	TM_ROW_INIT,
	TM_ROW_TERM,
	TM_HEADER,
	TM_PLAYER,
	//TM_LINE_OBJ,
	//TM_LINE_DEF,
	//TM_LINE_OFF,
	//TM_LINE_FND,
	TM_LINE_EMPTY,
	TM_TERM
} TEAM_MATRIX;


// public variables
extern team_stats		TEAMS[];	  // team array
extern int				team_index[]; // unique index, each element points to a team in TEAMS[]
extern int				team_count;
extern int				current_team;
extern int				conflicting_teaminfo;
extern teamscore_type	teamscore_history[];
extern int				teamscore_event_count;
extern int				active_teams;

// public functions
extern void create_new_team           ( char *team );
extern int  select_team               ( char *team );
extern int  team_participated		  ( int t );
extern void calculate_team_statistics ( void );
extern void calculate_teamscores      ( void );
extern void clear_team_table          ( void );
extern void determine_teams			  ( void );
extern void set_player_team_relation  ( int player, int team, char pt_relation );
extern void output_teams_html		  ( void );
extern void attach_playerlist_to_teams( void );
extern void add_teamscore_event       ( int scoring_team, int yielding_team, int scoring_player, long flagtouches );
extern void complete_teamscore_history( void );
extern team_stats *sorted_team( int t );
extern void set_team				  ( int player, int team );
extern void post_process_teams		  ( void );

#if defined(SERVER_STATS)
int select_team_by_number( char *team_number );
#else

#endif


#endif

