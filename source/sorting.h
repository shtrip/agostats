// sorting
#if !defined( SORTING_H )
#define SORTING_H

// defines

// types
enum {
	T_INTEGER,
	T_LONG,
	T_DOUBLE,
	T_STRING,
	
};

typedef enum {
	
	BY_ACQUAINTANCES_SQUAD_KNOWN,
	BY_CAPTURES,
	BY_CAPTURES_BY_TOUCHES,
	BY_DUELL_SCORE,
	BY_ENEMY_FACTOR,
	BY_FLAGTOUCHES,
	BY_NAMECLEAN,
	BY_NAMECLEAN_REV,
	BY_ROLE_FACTOR,
	BY_SCORE,
	BY_SQUAD,
	BY_TEAM,
	BY_TEAM_BY_NAMECLEAN,
	BY_TEAM_BY_NAMECLEAN_REV,
	BY_TEAM_BY_SQUAD,
	BY_TEAM_BY_SQUAD_BY_NAMECLEAN,
	BY_TEAMSCORE_DESC,
	BY_TEAMSCORE_DESC_BY_TEAM_BY_SQUAD_BY_FLAGTOUCHES_BY_CAPTURES_BY_NAMECLEAN,
	BY_TOUCHES_BY_CAPTURES

} SORT_ORDER;

// public variables


// public functions
extern void* push_player_index( void );
extern void  pop_player_index ( void *temp_storage );

extern long  sort_2_players	  ( int i, int j, int order );
extern void  sort_teams		  ( SORT_ORDER order );
extern void  sort_players	  ( SORT_ORDER order );

#endif

