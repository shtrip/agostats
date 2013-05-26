#if !defined(AGOSTATS_H_INCLUDED)
#define AGOSTATS_H_INCLUDED

#define UNKNOWN			'?'
#define YES				'Y'
#define NO				'N'

#define DEBUG_SIZE			2048
#define LOCATION_SIZE		 256
#define CHAT_SIZE			1024
#define TEAM_SIZE			 512
#define FLAG_SIZE			 256
#define BUILDING_SIZE		 256
#define AMMO_SIZE			 256
#define WEAPON_SIZE			 256
#define ACTION_SIZE			 256


//#define NO_INFO						0x00000000


// deaths and causes
#define T_NO_DEATH					0x00000000
#define T_KILL						0x00000001
#define T_TEAMKILL					0x00000002
#define T_SUICIDE					0x00000004
//#define T_SUICIDE_ENVIRONMENT		0x00000008 // caused by lava, cratered, squished
#define T_DESTROYED_AUTOSENTRY		0x00000010 // engineer destroying his autosentry
#define T_DESTROYED_SUPPLYSTATION	0x00000020 // engineer destroying his supplystation
#define T_KILLED_AUTOSENTRY			0x00000040
#define T_KILLED_SUPPLYSTATION		0x00000080


#define D_CONNECTED					0x00000400
#define D_DISCONNECTED				0x00000800
#define D_SERVER_OVERFLOW			0x00001000

#define P_CLIENT_SHUTDOWN			"----- CL_Shutdown -----\x0a"
#define P_SERVER_SHUTDOWN			"----- Server Shutdown -----\x0a"
#define P_MAP_END					"Current search path:\x0a"


// linetype frag, use this mask to access the info
typedef struct _frag_mask { // make this mask point to pattern.info1

	unsigned long death_type;
	unsigned long weapon;
	unsigned long extra_info;
	unsigned long hit_location;

} frag_mask;


// Public variables
extern int SET_NO_DATE;
extern int FILE_OFFSET;
extern int ONLY_LAST_GAME;
extern int stop_processing;
extern int remove_statfile;
extern char current_date[];
extern int first_blood_awarded;
extern int first_blood_killer ;
extern int first_blood_victim ;

extern int searching_last_game;
extern long file_position;
extern long file_position_game_begin;


#if defined (_DEBUG)
extern void debugmsg		( char *msg );
#endif
extern void output_program_info	( void );
extern void process_game_end	( void );
extern void process_game_begin	( void );
#endif
