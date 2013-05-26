// awards module
#if !defined(AWARDS_H)
#define AWARDS_H

// AWARD GROUPS
// todo: add new groups here
typedef enum {
	AG_UNDEFINED,
	AG_DEATH_AND_DESTRUCTION,
	AG_FLAG_ACTION,
	AG_STUPID,
	AG_IN_YOUR_FACE,
	AG_SPECIALIST,
	AG_LUCKY_BITCH
} _award_groups;

// todo: add new awards here (at the end)
typedef enum {
	AW_UNDEFINED,
	// Death and destruction
	AW_TERMINATOR,
	AW_SERIAL_KILLER,
	AW_FRAGMEAT,
	AW_RESUP_DEFENDER,
	AW_GODZILLA,
	AW_ACTION_MAN,

	// Flag action
	AW_PENETRATOR,
	AW_ESCAPE_ARTIST,
	AW_WANDERING_HANDS,
	AW_SAVING_PRIVATE_RYAN,
	AW_GOALKEEPER,
	AW_FLYING_GOALKEEPER,
	AW_TERRIER,

	// Stoopiiiid!!!!!!
	AW_CAPTAIN_LEMMING,
	AW_SUCKZILLA,
	AW_TEAMKILLER,
	AW_JACKASS,
	AW_SUCKER,
	AW_METEORITE,
	
	// In Your Face
	AW_ROCKETLAUNCHER,
	AW_GRENADELAUNCHER,
	AW_SNIPERRIFLE,
	AW_FLASH_GRENADE,    //"blink blink","The eyes have it","For your eyes only","dazzling performance"

	// Specialist
	AW_SHOTGUN,
	AW_FLAMETHROWER,
	AW_NAILGUN,
	AW_MINIGUN,
	AW_SENTRY,
	AW_STALKER,
	AW_TOSSER,
	AW_SANDMAN,

	// Lucky bitch
	AW_HE_CHARGE,
	AW_CLUSTER,
	AW_NAILGRENADE,
	AW_LAVAGRENADE,

	// add new awards here to not mess up the highscores
	AW_MOST_TEAMKILLED,
	AW_FIRST_BLOOD,

	// no award, just counter
	AW_NR_OF_AWARDS
} _award_identifiers;


typedef struct _meta_award {

	_award_identifiers awardID			; // value taken from _award_list
	_award_groups      group			; // Award group Stoopiiiiid
	char			   name[ 50 ]		; // Award name Terminator
	long (*sort_func)( int, int, int )	; // pointer to function that compares 2 players
	int sort_order  					; // sorting order
	int keep_highscore					; // keep highscore for this award
	
} meta_award;

typedef struct __score {
	long score1;
	long score2;
	long score3;
	long score4;
} _score;

extern meta_award AWARD_LIST[];
extern void output_awards_v2( void );
extern void output_highscore_awards( void *hs );
extern void calculate_award_score( int award, int p, _score* result );
extern int  select_award( int award_id );
extern long score_compare( _score s1, _score s2 );
extern int  awardscore_bigger_than_null( _score s );
#endif
