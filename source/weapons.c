#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "weapons.h"
#include "globals.h"

weapon_name_table WEAPON_NAME_TABLE[] = {
	// AGOStats weapon ID		,	AGOStats weapon name
	W_AUTOSENTRY_BULLET			,	"Autosentry (bullet)",
	W_AUTOSENTRY_EXPLOSION		,	"Autosentry (explosion)",
	W_AUTOSENTRY_ROCKET			,	"Autosentry (rocket)",
	W_BATTLEAXE					,	"Battle axe",
	W_BEAM						,	"Beam",
	W_HE_CHARGE					,	"Heavy explosive charge",
	W_CLUSTER_GRENADE			,	"Cluster grenade",
	W_CRUSHED					,	"Crushed",
	W_CRUSHED_BY_SENTRY			,	"Crushed by sentry",
	W_CRUSHED_BY_SUPPLYSTATION	,	"Crushed by supply station",
	W_KILL_BEFORE_KICK			,	"Killed by server",
	W_DARTGUN					,	"Tranquilizer gun",
	W_DISEASE                   ,	"Disease",
	W_SYRINGE					,	"Syringe",
	W_FALLING					,	"Cratering",
	W_FLAMETHROWER			    ,	"Flamethrower",
	W_FLAME_SPLASH				,	"Flame splash",
	W_FLASH_GRENADE			    ,	"Flash grenade",
	W_GAS_EXPLOSION			    ,	"Gas explosion",
	W_GAS_GRENADE				,	"Gas grenade",
	W_RED_PIPE_DIRECT			,	"Red pipe (direct)",
	W_RED_PIPE_SPLASH	        ,	"Red pipe (splash)",
	W_HAND_GRENADE				,	"Hand grenade",
	W_KNIFE					    ,	"Knife",
	W_LAVA					    ,	"Lava",
	W_MAPSENTRY                 ,	"Mapsentry",
	W_MAPSENTRY_BULLET			,	"Mapsentry (bullet)",
	W_MAPSENTRY_ROCKET          ,	"Mapsentry (rocket)",
	W_MINIGUN					,	"Minigun",
	W_MIRROR                    ,	"Mirror",
	W_NAIL_GRENADE				,	"Nail grenade",
	W_SMALL_NAILGUN				,	"Nailgun",
	W_NAPALM_GRENADE            ,	"Napalm grenade",
	W_NODROP		            ,	"Nodrop",
	W_YELLOW_PIPE               ,	"Yellow pipe",
	W_PULSE_GRENADE             ,	"Pulse grenade",
	W_RAILGUN					,	"Railgun",
	W_ASSAULT_RIFLE			    ,	"Assault rifle",
	W_ROCKET_LAUNCHER_DIRECT    ,	"Rocket (direct)",
	W_ROCKET_LAUNCHER_SPLASH    ,	"Rocket (splash)",
	W_DOUBLE_SHOTGUN			,	"Double shotgun",
	W_SINGLE_SHOTGUN			,	"Single shotgun",
	// q3f
	W_SHOTGUN					,	"Shotgun", // it is unknown if it's the single or double shotgun
	// q3f
	W_SLIME					    ,	"Slime",
	W_SNIPERRIFLE_BODY			,	"Sniper rifle (body)",
	W_SNIPERRIFLE_LEGS			,	"Sniper rifle (legs)",
	W_SNIPERRIFLE_HEAD          ,	"Sniper rifle (head)",
	W_CONSOLE              		,	"Console",
	W_SUPER_NAILGUN			    ,	"Super nailgun",
	W_SUPPLY_STATION_EXPLOSION  ,	"Supply station explosion",
	W_TARGET_LASER              ,	"Laser",
	W_TELEFRAG					,	"Telefrag",
	W_WRONG_PLACE				,	"Wrong place",
	W_UNKNOWN					,	"Unknown",
	W_WATER						,	"Drowning",
	W_WRENCH					,	"Wrench",

	-1, ""
};

serverlog_weapon_table SERVERLOG_WEAPON_TABLE[] = {
	// Serverlog weapon ID, AGOStats weapon ID
	  0,		W_UNKNOWN, // just as initializer for 0

	  1,		W_DOUBLE_SHOTGUN,
	  2,		W_BATTLEAXE,
	  3,		W_SMALL_NAILGUN,
	  4,		W_RED_PIPE_DIRECT,
	  5,		W_RED_PIPE_SPLASH,
	  6,		W_YELLOW_PIPE,
	  7,		W_ROCKET_LAUNCHER_DIRECT,
	  8,		W_ROCKET_LAUNCHER_SPLASH,
	  9,		W_FLAMETHROWER,
	 10,		W_UNKNOWN,
	 11,		W_RAILGUN,
	 12,		W_WATER,
	 13,		W_UNKNOWN,
	 14,		W_LAVA,
	 15,		W_CRUSHED,
	 16,		W_TELEFRAG,
	 17,		W_FALLING,
	 18,		W_CONSOLE,
	 19,		W_UNKNOWN,
	 20,		W_WRONG_PLACE,
	 21,		W_SNIPERRIFLE_BODY,
	 22,		W_SNIPERRIFLE_HEAD,
	 23,		W_SNIPERRIFLE_LEGS,
	 24,		W_ASSAULT_RIFLE,
	 25,		W_DARTGUN,
	 26,		W_KNIFE,
	 27,		W_DISEASE,
	 28,		W_SYRINGE,
	 29,		W_WRENCH,
	 30,		W_HAND_GRENADE,
	 31,		W_FLASH_GRENADE,
	 32,		W_NAIL_GRENADE,
	 33,		W_CLUSTER_GRENADE,
	 34,		W_NAPALM_GRENADE,
	 35,		W_GAS_GRENADE,
	 36,		W_PULSE_GRENADE,
	 37,		W_HE_CHARGE,
	 38,		W_AUTOSENTRY_BULLET,
	 39,		W_AUTOSENTRY_ROCKET,
	 40,		W_AUTOSENTRY_EXPLOSION,
	 41,		W_SUPPLY_STATION_EXPLOSION,
	 42,		W_SINGLE_SHOTGUN,
	 43,		W_MINIGUN,
	 44,		W_UNKNOWN,
	 45,		W_UNKNOWN,
	 46,		W_UNKNOWN,
	 47,		W_UNKNOWN,
	 48,		W_SINGLE_SHOTGUN,
	 49,		W_CRUSHED_BY_SENTRY,
	 50,		W_UNKNOWN,
	 51,		W_UNKNOWN,
	 52,		W_UNKNOWN,
	 53,		W_SUPER_NAILGUN,
	 54,		W_UNKNOWN,
	 55,		W_UNKNOWN,
	 56,		W_UNKNOWN,
	 57,		W_UNKNOWN,
	 58,		W_UNKNOWN,
	 59,		W_UNKNOWN,
	 60,		W_UNKNOWN,

	 -1,	-1
};

// todo: create function for this one
matchlog_weapon_table MATCHLOG_WEAPON_TABLE[] = {
#if defined(SERVER_STATS)
#else
	// Matchlog name			, AGOStats ID
	"AUTOSENTRY_BULLET"			, W_AUTOSENTRY_BULLET			,
	"AUTOSENTRY_EXPLODE"		, W_AUTOSENTRY_EXPLOSION		,
	"AUTOSENTRY_ROCKET"			, W_AUTOSENTRY_ROCKET			,
	"AXE"						, W_BATTLEAXE					,
	"BEAM"						, W_BEAM						,
	"CHARGE"					, W_HE_CHARGE					,
	"CLUSTERGREN"				, W_CLUSTER_GRENADE				,
	"CRUSH"						, W_CRUSHED						,
	"CRUSHEDBYSENTRY"			, W_CRUSHED_BY_SENTRY			,
	"CRUSHEDBYSUPPLYSTATION"	, W_CRUSHED_BY_SUPPLYSTATION	,
	"CUSTOM"					, W_KILL_BEFORE_KICK			,
	"DARTGUN"					, W_DARTGUN					    ,
	"DISEASE"					, W_DISEASE                     ,
	"FAILED_OPERATION"			, W_SYRINGE					    ,
	"FALLING"					, W_FALLING					    ,
	"FLAME"						, W_FLAMETHROWER			    ,
	"FLAME_SPLASH"				, W_FLAME_SPLASH				,
	"FLASHGREN"					, W_FLASH_GRENADE			    ,
	"GASEXPLOSION"				, W_GAS_EXPLOSION			    ,
	"GASGREN"					, W_GAS_GRENADE					,
	"GRENADE"					, W_RED_PIPE_DIRECT				,
	"GRENADE_SPLASH"			, W_RED_PIPE_SPLASH	            ,
	"HANDGREN"					, W_HAND_GRENADE				,
	"KNIFE"						, W_KNIFE					    ,
	"LAVA"						, W_LAVA					    ,
	"MAPSENTRY"					, W_MAPSENTRY                   ,
	"MAPSENTRY_BULLET"			, W_MAPSENTRY_BULLET			,
	"MAPSENTRY_ROCKET"			, W_MAPSENTRY_ROCKET            ,
	"MINIGUN"					, W_MINIGUN					    ,
	"MIRROR"					, W_MIRROR                      ,
	"NAILGREN"					, W_NAIL_GRENADE				,
	"NAILGUN"					, W_SMALL_NAILGUN				,
	"NAPALMGREN"				, W_NAPALM_GRENADE              ,
	"NODROP"					, W_NODROP		                ,
	"PIPE"						, W_YELLOW_PIPE                 ,
	"PULSEGREN"					, W_PULSE_GRENADE               ,
	"RAILGUN"					, W_RAILGUN					    ,
	"RIFLE_ASSAULT"				, W_ASSAULT_RIFLE			    ,
	"ROCKET"					, W_ROCKET_LAUNCHER_DIRECT      ,
	"ROCKET_SPLASH"				, W_ROCKET_LAUNCHER_SPLASH      ,
	"SHOTGUN"					, W_DOUBLE_SHOTGUN				,
	"SINGLESHOTGUN"				, W_SINGLE_SHOTGUN			    ,
	"SLIME"						, W_SLIME					    ,
	"SNIPER_RIFLE"				, W_SNIPERRIFLE_BODY			,
	"SNIPER_RIFLE_FOOT"			, W_SNIPERRIFLE_LEGS			,
	"SNIPER_RIFLE_HEAD"			, W_SNIPERRIFLE_HEAD            ,
	"SUICIDE"					, W_CONSOLE						, // todo: check
	"SUPERNAILGUN"				, W_SUPER_NAILGUN			    ,
	"SUPPLYSTATION_EXPLODE"		, W_SUPPLY_STATION_EXPLOSION    ,
	"TARGET_LASER"				, W_TARGET_LASER                ,
	"TELEFRAG"					, W_TELEFRAG					,
	"TRIGGER_HURT"				, W_WRONG_PLACE					,
	"UNKNOWN"					, W_UNKNOWN						,
	"WATER"						, W_WATER						,
	"WRENCH"					, W_WRENCH						,
#endif
	// end of list
	"", 0, 0,
};

// public variables
int weapon_index1[ W_NR_WEAPONS ];
int weapon_index2[ W_NR_WEAPONS ];

// private variables


// private functions

// public functions
void sort_weapons( long *frags_per_weapon, int *weapon_index )
{
	int i, j;
	long temp;

		// init index;
	for ( i = 0; i < W_NR_WEAPONS; i++ ) {
		weapon_index[ i ] = i;
	}

	for ( i = 0; i < W_NR_WEAPONS ; i++ ) {
		for ( j= i + 1; j < W_NR_WEAPONS ; j++ ) {
			if ( frags_per_weapon[ weapon_index[ j ] ] > frags_per_weapon[ weapon_index[ i ] ] ) {
				temp = weapon_index[ j ];
				weapon_index[ j ] = weapon_index[ i ];
				weapon_index[ i ] = temp;
			}
		}
	}
}

int get_matchlog_weapon( char* weapon )
{
	int start, i;
	int found = FALSE;
	int res;
	int result = W_UNKNOWN;
	
	start = W_NR_WEAPONS / 2;

	res = strcmp( weapon, MATCHLOG_WEAPON_TABLE[ start ].name );

	if ( res > 0 ) {
		for ( i = start + 1; i < W_NR_WEAPONS - 1; i++ ) {
			if ( 0 == strcmp( weapon, MATCHLOG_WEAPON_TABLE[ i ].name ) ) {
				found = TRUE;
				result = MATCHLOG_WEAPON_TABLE[ i ].id;
				break;
			}	
		}
	}
	else {
		if ( res < 0 ) {
			for ( i = start - 1; i >= 0; i-- ) {
				if ( 0 == strcmp( weapon, MATCHLOG_WEAPON_TABLE[ i ].name ) ) {
					found = TRUE;
					result = MATCHLOG_WEAPON_TABLE[ i ].id;
					break;
				}
			}
		}
		else {
			found = TRUE;
			result = MATCHLOG_WEAPON_TABLE[ start ].id;
		}
	}

	return result;
}

char *weapon_name( int weapon_nr )
{
	char *result;
	if ( weapon_nr >= 0 && weapon_nr <= W_NR_WEAPONS ) {
		result = WEAPON_NAME_TABLE[ weapon_nr ].agostats_weapon_name;
	}
	else {
		result = WEAPON_NAME_TABLE[ W_UNKNOWN ].agostats_weapon_name;
	}
	return result;
}
#if defined(_DEBUG) 
void print_sorted_weapons( int *weapon_index, long* frags_per_weapon )
{
	int w;

	for ( w = 0; w < W_NR_WEAPONS; w++ ) {
		printf( "%s - %ld\n", weapon_name( weapon_index[ w ] ), frags_per_weapon[ weapon_index[ w ] ] );		
	}
}
#endif

#if defined(SERVER_STATS)

// conver weapon number
int get_server_weapon( char *weapon )
{
	int weapon_nr;
	int result = W_UNKNOWN;
	//[16:39] (AGO|Pook): Hitman, can you find out what MOD_CUSTOM in the games.log means, it's a means of suiciding
	//[16:41] (Hitman--): looks like a fall threw..let me ask cana
	//[16:45] (AGO|Pook): I'm creating stats for server logs and I have no clue what it means
	//[16:45] (Hitman--): in all of etf code
	//[16:45] (Hitman--): it is in 1 place
	//[16:45] (Hitman--): 	if( mod == MOD_CUSTOM )
	//[16:45] (Hitman--): 		return;			// Golliwog: Server handled this obit
	//[17:04] (Hitman--): [09:44 AM] <Hitman--> hey
	//[17:04] (Hitman--): [09:45 AM] <Hitman--> what does MOD_CUSTOM mean
	//[17:04] (Hitman--): [10:07 AM] <CaNaBiS-PH> hmm usually used when the engine kills someone before a kick
	//[17:04] (Hitman--): [10:07 AM] <Hitman--> ahhh ok

	if ( strlen( weapon ) > 0 ) {
		weapon_nr = atoi( weapon );
		if ( weapon_nr >= 0 && weapon_nr <= W_NR_WEAPONS ) {
			result = SERVERLOG_WEAPON_TABLE[ weapon_nr ].agostats_weapon_id;
		}
	}

	return result;
}
#endif

