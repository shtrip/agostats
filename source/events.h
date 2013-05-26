// events module
#if !defined(EVENTS_H_INCLUDED)
#define EVENTS_H_INCLUDED
#include "game.h"    // contains event types


typedef struct _event{

	long       id;     // id used for ordering
	EVENT_TYPE type;
	char*      info1;  // for instance playername in case of mm1
	char*      info2;  // for instance mm1
} event_type;

extern void events_init( void );
extern void clear_events();
extern void events_add( EVENT_TYPE et, char *info1, char *info2 );
extern void output_events( void );
#endif

