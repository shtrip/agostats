// flagtracking module
#if !defined(FLAGTRACKING_H)
#define FLAGTRACKING_H

#include "globals.h"
#include "players.h"

// defines

// Extra FlagTake Teamonly, only teaminfo in the flagtake message, not which flag was taken
#define EXTRA_FT_TEAMONLY			0x00000001 // bla of the RED team took the flag type of message (dewwars)


// types
typedef struct _flag_stats {
	char name[ 50 ];
	long currently;              // status: ( home, field, carried, captured )
	long previously;             // status: ( home, field, carried, captured )
	long current_alive_touches;
	long max_touches_saved;
	long max_touches_capped;
	long saved ;                // means it returned to base from "alive" status
	long capped;                // number of times it was capped
	long carrier;               // player that is currently carrying this flag
	
	// capped + saved + current_status tells us how many times flag was taken
	// from original position

} flag_stats;



enum {
	// Flag Status
	FS_UNKNOWN = 0,
	FS_BASE,
	FS_CARRIED,
	FS_FIELD,
	FS_CAPTURED
} ;

// Public variables
extern flag_stats		FLAGS  []; // flags array
extern int				flag_count;
extern int				current_flag;

// Public functions
extern int  player_carrying_flag ( int player, int *flag );
extern int  select_flag          ( char *flag );
extern void set_flagstatus       ( int flagstatus, int flag, int flagcarrier );
extern void create_new_flag      ( char *flag );
extern void clear_flag_table     ( void );


#endif

