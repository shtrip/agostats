// maps
#if !defined( MAPS_H )
#define MAPS_H

// defines
#define MAP_NAME_SIZE		 32	   // map name "etf_japanc"
#define MAP_DESC_SIZE		 128   // map description "Japanese castles"	
#define MAP_ID_SIZE			 32



// types

typedef struct _mapinfo {
	char mapname	[ MAP_NAME_SIZE ];
	char mapdesc	[ MAP_DESC_SIZE ];
	char id			[ MAP_ID_SIZE   ];
} mapinfo;

// public variables
extern char			map_description[];
extern int			map_id;
extern mapinfo		MAP_INFOS[];

// public functions
extern void process_mapinfo( int signature );

#endif

