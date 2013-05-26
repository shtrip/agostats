#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "events.h"

#include "output.h"
#include "skiplist.h"

table game_events;
long id;

int event_id_compare( void *a, void *b )
{
	int result;
	long id1, id2;
	event_type *event1, *event2;

	if ( a != NULL && b != NULL ) {
		event1 = a;
		event2 = b;

		id1 = event1->id;
		id2 = event2->id;

		if ( id1 == id2 ) {
			result = 0;
		}
		else {
			if ( id1 > id2 ) {
				result = 1;
			}
			else {
				result = -1;
			}
		}
	} 
	else {
		// a == NULL or b == NULL or both
		if ( a == NULL && b == NULL ) {
			result = 0;
		}
		else {
			// either a == NULL or b == NULL
			if ( a == NULL ) {
				result = 1;
			}
			else {
				result = -1;
			}
		}
	}
	return result;
}

void event_delete( void *event )
{
	event_type *temp;

	temp = event;
	if ( temp->info1 != NULL ) {
		free(temp->info1);
	}
	if ( temp->info2 != NULL ) {
		free(temp->info2);
	}
}

void events_init()
{
	id = 0;
	create_table( &game_events, event_id_compare, event_delete, sizeof( event_type ) );
}

void events_add( EVENT_TYPE et, char *info1, char *info2 )
{
	event_type new_event;
	
	id = id + 1;
	new_event.id    = id;
	new_event.type  = et;
	new_event.info1 = NULL;
	new_event.info2 = NULL;

	if ( info1 != NULL ) {
		new_event.info1 = malloc( strlen( info1 ) + 1 );
		strcpy( new_event.info1, info1 );
		//printf( "event_info1[%s]\n", info1 );
	}
	
	if ( info2 != NULL ) {
		new_event.info2 = malloc( strlen( info2 ) + 1 );
		strcpy( new_event.info2, info2 );
		//printf( "event_info2[%s]\n", info1 );
	}

	insert_record( &game_events, &new_event );

}

void output_events( void )
{
	event_type *temp;
			
	// process events
	select_first_record( &game_events );
	if ( game_events.current == NULL ) {
		return;
	}	

	// init table
	sprintf( eventbuf, HTML_EVENTLOG_INIT );
	AS_OUT( OE_PLAIN, eventbuf );

	do {
		temp = game_events.current->record;
			
		switch( temp->type ) {

			case ET_CHAT :
				sprintf( eventbuf, HTML_EVENTLOG_FORMAT, et_2_html( temp->info1 ), et_2_html( temp->info2 ) );
				AS_OUT( OE_PLAIN, eventbuf );
				break;

			case ET_GAME :
				sprintf( eventbuf, HTML_EVENTLOG_FORMAT, temp->info1, temp->info2 );
				AS_OUT( OE_PLAIN, eventbuf );
				break;

			case ET_OBJECTIVE:
				sprintf( eventbuf, HTML_EVENTLOG_FORMAT, temp->info1, temp->info2 );
				AS_OUT( OE_PLAIN, eventbuf );
				break;
			default:
				// add row and 2 cells but put entire string in first cell
				if ( temp->info1 != NULL ) {
					sprintf( eventbuf, HTML_EVENTLOG_FORMAT, temp->info1, temp->info2 );
					AS_OUT( OE_PLAIN, eventbuf );
				}
				break;
		}

	} while ( select_next_record( &game_events ) );	
	
	// terminate table
	sprintf( eventbuf, HTML_EVENTLOG_TERM );
	AS_OUT( OE_PLAIN, eventbuf );
			
}

void clear_events()
{
	clear_table( &game_events );
}
