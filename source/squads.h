// squads module
#if !defined(SQUADS_H)
#define SQUADS_H
#include "weapons.h"
//#include "players.h"

// defines

// types

enum {
	SQUAD_UNKNOWN = 0,
	SQUAD_DEF,
	SQUAD_OFF,
	
	MAX_SQUADS // dummy, to indicate the number of squads
}; // Squads

typedef struct _squad_stats{

	long frags				   ; // total frags by this squad
	long deaths                ; // total deaths by this squad
	long suicides			   ; // total suicides by this squad
	long internal_tks          ; // total teamkills by squadmembers on squadmembers
	long frags_with_flag_moved ;
	long deaths_with_flag_moved;
	long captures              ;
	long killed_flagcarrier    ;
	long frags_per_weapon [ W_NR_WEAPONS ];

	// todo: point to enemy squads ?
	double avg_rolefactor;
	double avg_worst_enemy_factor;

} squad_stats;

/*
typedef struct _squad_stats_v2{

	int    team;                   // a squad belongs to a team
	int    players[ MAX_PLAYERS ]; // a squad consists of players
	int    role;                   // a squad has a role: offence or defence or something else

	// calculated properties:
	double avg_rolefactor;
	double avg_worst_enemy_factor;
	long   frags				   ; // total frags by this squad
	long   deaths                ; // total deaths by this squad
	long   suicides			   ; // total suicides by this squad
	long   internal_tks          ; // total teamkills by squadmembers on squadmembers
	
	long   frags_with_flag_moved ;
	long   deaths_with_flag_moved;
	long   captures              ;
	long   killed_flagcarrier    ; 
	// todo: point to enemy squads ?
	
} squad_stats_v2;
*/

// public variables
extern squad_stats	SQUADS []; 

// public functions
extern char*	query_squadname( int p );
extern void		fill_squad_info( void );
extern void		fill_squad_info_dm_based( void );

#endif

