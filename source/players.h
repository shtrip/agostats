// players module
#if !defined(PLAYERS_H)
#define PLAYERS_H

#include "globals.h"
#include "teams.h"
#include "weapons.h"

// defines
#define NO_PLAYER				-1
#define PLAYER_WORLD            -2
#define PLAYER_WORLD_ID			1022
#define PLAYER_NAME_SIZE		256
#define Q3A_MAX_PLAYERNAMESIZE	29 // max is 29

// playerscores offence
#define PS_FIRST_TOUCH						40
#define	PS_FLAG_TOUCH						 3
#define PS_FLAG_CAPTURE						 3
#define PS_DIED_NEAR_FLAG_AT_BASE			40
#define PS_DIED_NEAR_FLAG_IN_FIELD			 4
#define PS_COAST_TO_COAST					15 // this player also gets the first touch points
#define PS_DEFEND_FLAGCARRIER				 2
#define PS_TEAMKILL_FLAGCARRIER              2
#define PS_DESTROY_SENTRY                    3
#define PS_DESTROY_SUPPLYSTATION             3

// playerscores defence
#define PS_DEFEND_FLAG_AT_BASE				40
#define PS_DEFEND_FLAG_IN_FIELD				 4
#define PS_KILL_FLAGCARRIER					 4
#define PS_TEAMKILL_SENTRY					 3
#define PS_TEAMKILL_SUPPLYSTATION			 3

typedef enum player_events {

	// offence
	PE_FIRST_TOUCH,
	PE_FLAG_TOUCH,
	PE_FLAG_CAPTURE,
	PE_DIED_NEAR_FLAG_AT_BASE,
	PE_DIED_NEAR_FLAG_IN_FIELD,
	PE_COAST_TO_COAST,
	PE_DEFEND_FLAGCARRIER,    // ACTF
	PE_TEAMKILL_FLAGCARRIER,
	PE_DESTROY_SENTRY,
	PE_DESTROY_SUPPLYSTATION,

	// defence
	PE_DEFEND_FLAG_AT_BASE,   // ACTF
	PE_DEFEND_FLAG_IN_FIELD,  // ACTF
	PE_KILL_FLAGCARRIER,
	PE_TEAMKILL_SENTRY,
	PE_TEAMKILL_SUPPLYSTATION,

	PE_MAX_EVENTS // dummy to indicate number of events

} t_player_events;

// types
enum {
	ENV_UNKNOWN = 0,

	ENV_CRATERED,
	ENV_DROWNED,
	ENV_LAVA,
	ENV_SQUISHED,
	ENV_MELTED,
	ENV_TELEFRAGGED,
	ENV_WRONG_PLACE,

	ENV_NR_ENVIRONMENTS
};

typedef struct _player_stats{

	int  initial_id; // to keep track of this player in the frag_matrix after sorting
	char name             [ 50 ];
	char nameclean        [ 50 ]; // without any ^7 etc.
	char nameclean_rev    [ 50 ]; // reversed nameclean, use for finding postfixed clantags
	char nameclean_html   [ 200 ]; // plaintext name with html tags properly converted
	char name_html		  [ 2048 ]; // todo: figure out max

	long server_id;		   // number assigned by server

	long first_line;       // line number of first line in logfile where player appears (not chat, but real game action)
	long last_line;        // line number of last line in logfile where player appears
	long linecount;        // number of lines where this players occurs (in action)

	int  team; // contains team number when team is found; 
	// in_team is a list of teams of which this player is not a part.
	char in_team [ MAX_TEAMS ]; // relation that is known to each team { yes, no, unknown }
	//long events  [ PE_MAX_EVENTS ]; // todo: use this
	int  squad;          // in which squad is this player

	long frags;			// a frag is a kill on enemy
	long deaths;		// a death is caused by enemy
	long teamkills;		// a teamkill is a kill on a teammate
	long teamdeaths;	// a teamdeath is caused by teammate
	long suicides;
	
	long destroyed_sentry;  // only count enemy sentries 
	long teamkilled_sentry; // killed a teamsentry
	long destroyed_supply;  // only count enemy suplly stations
	long lost_sentry;	    // number of sentries this player built and were destroyed
	long lost_supply;	    // number of supplystations this player built and were destroyed
	long flagtouches;
	long first_touches;     // number of times he took the flag from base (starting position)
	long flags_captured;
	long coast_to_coast;	// capture without dropping it
	long frags_per_weapon               [ W_NR_WEAPONS ];
	long fragged_flagcarrier_per_weapon [ W_NR_WEAPONS ];
	long deaths_per_weapon              [ W_NR_WEAPONS ];
	//long suicides_per_environment       [ ENV_NR_ENVIRONMENTS ];  // todo: remove this, will be done with a weapon
	long sit_on_grenade;
	long current_fragstreak;
	long max_fragstreak;

	long off_points;         // offence points
	long def_points;         // defence points
	long off_points_indirect;// calculated from enemy scores
	long def_points_indirect;// calculated from enemy scores

	long killed_near_flag_in_base;
	long killed_near_flag_in_field;

	long sort_value1; // todo: use for new sorting procedure
	long sort_value2;

	// objective stuff
	long defend_flag_at_base;
	long defend_flag_in_field;
	long defend_flagcarrier;
	long killed_flagcarrier;
	long teamkilled_flagcarrier;

	// other
	int  enabled;                     // use this for team switching players
	int  acquaintances_squad_known;   // from how many players this player fragged/was fragged by is the squad known
	int  sentry_up;                   // set to true if the sentry is fragging people, false when destroyed
	
	// squad
	double role_factor; // avg (frags+deaths) per encountered enemy, higher value means player played his role "cleaner"
	long enemy_factor;  // sum of frags + deaths against an enemy squad, use for sorting

	// todo: add last killed player, to use for checking killed near flag
} player_stats;

typedef struct _relation { // this structure describes the relation between 2 players from "player 1 to player 2"
	long frags;		       // player 1 killed player 2 x times
	char same_team;        // Same Different Unknown
	long fragged_sentry;
	long fragged_supplystation;
	long frags_per_weapon[ W_NR_WEAPONS ];
	long frags_on_flagcarrier_per_weapon[ W_NR_WEAPONS ];
} relation;



// public variables
extern player_stats PLAYERS[]; // player info storage
extern int          current_player;
extern int          player_count;
extern relation		frag_matrix [ MAX_PLAYERS ][ MAX_PLAYERS ]; // contains entire who fragged who table
extern int			player_index[ MAX_PLAYERS  ];

// public functions
extern void init_player					( int player_nr );
extern int  select_player				( char *playername );
extern int  player_exists				( char *playername );
extern int  find_player_nameclean		( char *nameclean );
extern void rename_player				( void );
extern int  player_participated			( int p );
extern void update_player_score			( t_player_events event, int player, int multiplier );
extern int  worst_enemy					( int player );
extern long close_range_frags			( int p );
extern char* environmental_death_name	( int environment_nr );
extern void mark_player_line			( int player, long linenumber );
extern player_stats *sorted_player		( int p );
#if defined(SERVER_STATS)
int select_player_by_number				( char *player_number );
void set_playername						( int player, char *player_name );
#endif


#endif

