// stathandling
#if !defined(STATHANDLING_H)
#define STATHANDLING_H

#include "agostats.h"
#include "players.h"

extern void reset_statistics   ( void );
extern void cleanup_stats      ( void );

extern player_stats PLAYERS[];

extern int current_player;
extern int current_team;
extern int current_flag;

extern int player_count;
extern int team_count;
extern int flag_count;

extern int conflicting_teaminfo;

#endif

