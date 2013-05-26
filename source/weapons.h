// weapons module
#if !defined(WEAPONS_H)
#define WEAPONS_H

#define MAX_WEAPON_STRING 32

// types
typedef struct _weapon_name_table{
	long agostats_weapon_id;
	char agostats_weapon_name[ MAX_WEAPON_STRING ];
} weapon_name_table;

typedef struct _serverlog_weapon_table{
	long server_weapon_id;
	long agostats_weapon_id;
} serverlog_weapon_table;

typedef struct _matchlog_weapon_table {
	char name [ MAX_WEAPON_STRING ];
	long id;
} matchlog_weapon_table;
// new weapon table

enum {
	W_AUTOSENTRY_BULLET			,
	W_AUTOSENTRY_EXPLOSION		,
	W_AUTOSENTRY_ROCKET			,
	W_BATTLEAXE					,
	W_BEAM						,
	W_HE_CHARGE					,
	W_CLUSTER_GRENADE			,
	W_CRUSHED					,
	W_CRUSHED_BY_SENTRY			,
	W_CRUSHED_BY_SUPPLYSTATION	,
	W_KILL_BEFORE_KICK			,
	W_DARTGUN					,
	W_DISEASE                   ,
	W_SYRINGE					,
	W_FALLING					,
	W_FLAMETHROWER			    ,
	W_FLAME_SPLASH				,
	W_FLASH_GRENADE			    ,
	W_GAS_EXPLOSION			    ,
	W_GAS_GRENADE				,
	W_RED_PIPE_DIRECT			,
	W_RED_PIPE_SPLASH	        ,
	W_HAND_GRENADE				,
	W_KNIFE					    ,
	W_LAVA					    ,
	W_MAPSENTRY                 ,
	W_MAPSENTRY_BULLET			,
	W_MAPSENTRY_ROCKET          ,
	W_MINIGUN					,
	W_MIRROR                    ,
	W_NAIL_GRENADE				,
	W_SMALL_NAILGUN				,
	W_NAPALM_GRENADE            ,
	W_NODROP		            ,
	W_YELLOW_PIPE               ,
	W_PULSE_GRENADE             ,
	W_RAILGUN					,
	W_ASSAULT_RIFLE			    ,
	W_ROCKET_LAUNCHER_DIRECT    ,
	W_ROCKET_LAUNCHER_SPLASH    ,
	W_DOUBLE_SHOTGUN			,
	W_SINGLE_SHOTGUN			,
	// q3f
	W_SHOTGUN					,
	// q3f
	W_SLIME					    ,
	W_SNIPERRIFLE_BODY			,
	W_SNIPERRIFLE_LEGS			,
	W_SNIPERRIFLE_HEAD          ,
	W_CONSOLE              		,
	W_SUPER_NAILGUN			    ,
	W_SUPPLY_STATION_EXPLOSION  ,
	W_TARGET_LASER              ,
	W_TELEFRAG					,
	W_WRONG_PLACE				,
	W_UNKNOWN					,
	W_WATER						,
	W_WRENCH					,


	W_NR_WEAPONS,
};


// weapons 
//enum {
//	//W_UNKNOWN = -1,
//	W_UNKNOWN = 0,
//
//	W_ASSAULT_RIFLE,
//	W_BATTLEAXE,
//	W_DARTGUN,
//	W_DOUBLE_SHOTGUN,
//	W_FLAMETHROWER,
//	W_RED_PIPE_SPLASH,
//	W_RED_PIPE_DIRECT,
//	W_KNIFE,
//	W_MINIGUN,
//	W_SMALL_NAILGUN,
//	W_NAPALM_GRENADE_LAUNCHER,
//	W_YELLOW_PIPE,
//	W_RAILGUN,
//	W_SNIPERRIFLE_BODY,					
//	W_SNIPERRIFLE_HEAD,
//	W_SNIPERRIFLE_LEGS,
//	W_ROCKET_LAUNCHER_SPLASH ,
//	W_ROCKET_LAUNCHER_DIRECT,
//	W_SINGLE_SHOTGUN,
//	W_SYRINGE,
//	W_SUPER_NAILGUN,
//	W_WRENCH,					
//							
//	// grenade types
//	W_CLUSTER_GRENADE,
//	W_FLASH_GRENADE,
//	W_GAS_GRENADE,
//	W_HAND_GRENADE,
//	W_NAIL_GRENADE,
//	W_NAPALM_GRENADE,
//	W_PULSE_GRENADE,
//	W_STUN,
//	
//	// Special weapons
//	W_AUTOSENTRY_BULLET,
//	W_CRUSHED_BY_SENTRY,
//	W_AUTOSENTRY_EXPLOSION,	
//	W_AUTOSENTRY_ROCKET,	
//	W_HE_CHARGE,
//	W_SUPPLY_STATION_EXPLOSION,
//	W_GAS_EXPLOSION, // gas ignited by flamethrower
//	W_TELEFRAG, // through teleporter
//
//	// Environmental suicides
//	//W_ENVIRONMENTAL_SUICIDES_BEGIN, // marker
//	W_CRUSH,
//	W_FALLING,
//	W_SLIME,
//	W_WATER,
//	W_WRONG_PLACE,
//	W_LAVA,
//	//W_ENVIRONMENTAL_SUICIDES_END, // marker
//	// 
//	
//	W_CONSOLE, // typing "kill" in the console
//
//	W_NR_WEAPONS, // dummy, just mark the end of the enumeration
//};
// public variables
extern int weapon_index1[];
extern int weapon_index2[];

// public functions
extern char *weapon_name( int weapon_nr );
extern void sort_weapons( long *frags_per_weapon, int *weapon_index );
extern int  get_matchlog_weapon( char* weapon );

#if defined(_DEBUG) 
extern void print_sorted_weapons( int *weapon_index, long* frags_per_weapon );
#endif
#if defined(SERVER_STATS)

int get_server_weapon( char *weapon );
#endif

#endif

