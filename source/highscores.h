#if !defined(HISCORES_H_INCLUDED)
#define HISCORES_H_INCLUDED

#include "awards.h"
#include "maps.h"

#define HIGHSCORE_OUTPUT	"highscores.htm"
#define HIGHSCORE_FILENAME	"highscores.txt"

#define PERSONAL_DIRECTORY	"agostats_data"
#define PERSONAL_HIGHSCORES "playerscores.txt"
#define PERSONAL_PLAYERS	"players.txt"
#define PERSONAL_LINKPAGE	"players.htm"


typedef struct {

	long	award_ID; 
	char	date	   [ 11 ];
	char	playername [ 50 ];
	char	mapname	   [ MAP_NAME_SIZE ];
	_score	score;

} _highscore_type;

typedef struct {
	unsigned long	player_ID;
	char			playername [ 50 ];
} _highscore_player_type;

typedef struct {
	unsigned long	player_ID;
	unsigned long	award_ID;
	_score			score;
} _highscore_record_type;


extern _highscore_type global_highscores[];
// variables to be used by output.c
extern int				writing_highscores;
extern int				writing_linkpage;
extern unsigned long	writing_highscore_player_id;
extern int				writing_highscore_player_number;

extern void read_highscores( char *filename, _highscore_type *highscores );
void reset_highscores( _highscore_type *some_highscores );
extern void update_highscores( int award_id, int player, _score highscore, _highscore_type *highscores  );
extern void update_player_personal_highscores( void );
extern void store_highscores(  char *filename, _highscore_type *highscores );
extern void get_highscore( int award_id, _score* highscore, _highscore_type *highscores );
extern void output_highscores ( void );
extern void output_highscore_line( char *line );
extern void load_individual_highscores( void );
#endif
