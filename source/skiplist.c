#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "skiplist.h"
#include "globals.h"

#define SKIPLIST_BUFFER_SIZE 4096
char buffer[ SKIPLIST_BUFFER_SIZE ];

// helper functions
void init_previous( table* t )
{
	int i;
	for ( i = 0; i < MAX_LEVEL; i++ ) {
		t->previous[ i ] = t->list;
	}
}

int randomheight( void )
{
	int result = 1;
	
	while ( rand() & 3 && result < MAX_LEVEL ) {
		result++;
	}
	
	return result;
}

node* make_node( void* record, int record_size )
{
	node* new_node;
	void* new_record;
	int i;

	new_node   = (node*) malloc( sizeof( node ) );

	for ( i = 0 ; i < MAX_LEVEL; i++ ) {
		new_node->forward[ i ] = NULL;
	}
	
	if ( record != NULL ) {
		new_record = malloc( record_size );
		memcpy( new_record, record, record_size );
		new_node->record = new_record;
	}
	else {
		new_node->record = NULL;
	}

	return new_node;
}

void delete_node( table *t, node* byebye )
{
	if ( byebye != NULL ) {
		if ( byebye->record != NULL ) {
			t->free_record( byebye->record );
			free( byebye->record );
		}
		free( byebye );
	}
}

void delete_list( table *t )
{
	node *temp = t->list;
	node *behind;

	while ( temp != NULL ) {
		behind = temp;
		temp = temp->forward[ 0 ];
		delete_node( t, behind );
	}
}

// public functions

void create_table( table* t, int (*compare_function) ( void*, void* ), void (*delete_function) (void*), int record_size )
{
	t->compare      = compare_function;
	t->free_record  = delete_function;
	t->record_size  = record_size;
	t->list		    = make_node( NULL, record_size );
	t->record_count = 0L;
	t->height       = 1;
}

void clear_table( table* t )
{
	delete_list( t );
	t->current = NULL;
	t->height  = 1;
	//t->list	   = NULL;
	t->list	   = make_node( NULL, t->record_size  );
	t->record_count = 0;
}

// select first record
int select_first_record( table *t )
{
	int result = FALSE;
	node *temp;
	
	if ( t->list != NULL ) {
		temp = t->list->forward[ 0 ];
		if ( temp != NULL ) {
			t->current = temp;
			result = TRUE;
		}
	}
	return result;
}

// return FALSE when no valid next record is found
int select_next_record( table *t )
{
	int result = FALSE;
	node *temp;

	if ( t->list != NULL ) {
		temp = t->current->forward[ 0 ];
		t->current = temp;
		result = temp != NULL;
	}

	return result;
}

void print_table( table t )
{
	node *temp;
	
	printf( "print table begin\n" );
	if ( t.list != NULL ) {
		temp = t.list->forward[ 0 ];

		while ( temp != NULL ) {
			// todo: use variable printfunc
			printf( "%s\n", temp->record );
			temp = temp->forward[ 0 ];
		}
		
	}
	printf( "print table end\n" );
}

int search_record( table *t, void *record )
{
	int i;
	int finished;
	int result   = FALSE;
	node* x;

	init_previous( t );

	if ( t != NULL ) {
		x = t->list;
		if ( x != NULL ) {

			for ( i = t->height - 1; i >= 0 ; i-- ) {

				finished = FALSE;

				while ( !finished ) {
					if ( x->forward[ i ] == NULL ) {
						// go one level lower
						finished = TRUE;
					}
					else {

						if ( t->compare( x->forward[ i ]->record, record ) >= 0 ) {
							finished = TRUE;
						}
						else {
							x = x->forward[ i ];
						}
					}
				}
				t->previous[ i ] = x;
			}
			
			x = x->forward[ 0 ];
			if ( x != NULL ) {
				if ( 0 == t->compare( x->record, record ) ) {
					t->current = x;
					result = TRUE;
				}
			}
		}
	}

	return result;
}

void insert_record( table* t, void* record )
{
	int i;
	int height;
	node* x;

	if ( search_record( t, record ) ) {
		// already exists
	}
	else {
		// record not found, insert it
		height = randomheight();
		if ( height > t->height ) {
			
			for ( i = t->height - 1; i < height; i++ ) {
				t->previous[ i ] = t->list;
			}
			t->height = height;
		}
		x = make_node( record, t->record_size );

		for ( i = 0; i < height; i++ ) {
			if ( t->previous[ i ] != NULL ) {
				x->forward[ i ] = t->previous[ i ]->forward[ i ];
				t->previous[ i ]->forward[ i ] = x;
			}
			//t->previous[ i ]->forward[ i ] = x;
		}
		t->record_count++;
		t->current = x;

	//	printf( "inserted record: [%s]\n", t->current->record );
	}
}

void delete_record( table* t, void* record )
{
	int i;
	node *next;

	if ( search_record( t, record ) ) {
		next = t->current->forward[ 0 ];
		if ( next == NULL ) {
			t->previous[ 0 ]; // todo: controleren
		}
		// it exists
		for ( i = 0; i < t->height; i++ ) {
			// todo: iets wat ik niet snap in die pdf
			t->previous[ i ]->forward[ i ] = t->current->forward[ i ];
		}
		
		delete_node( t, t->current );

		while ( t->height > 1 && t->list->forward[ t->height ] == NULL ) {
			t->height = t->height - 1;
		}

		t->record_count--;
		t->current = next;
		printf( "deleted record: [%s]\n", record );
	}
}

void save_table( table* t, char *filename )
{
	FILE *fsave;
	long buffer_pointer = 0;
	
	fsave = fopen( filename, "wb" );
	
	if ( fsave == NULL ) {
		printf( "error opening file : %s\n", filename );
		return;
	}

	if ( select_first_record( t ) ) {
		do {
			if ( buffer_pointer + t->record_size < SKIPLIST_BUFFER_SIZE ) {
				memcpy(	buffer + buffer_pointer, t->current->record, t->record_size );
				buffer_pointer = buffer_pointer + t->record_size;
			}
			else {
				fwrite( buffer, buffer_pointer, 1, fsave );
				buffer_pointer = 0;
				memcpy(	buffer + buffer_pointer, t->current->record, t->record_size );
				buffer_pointer = buffer_pointer + t->record_size;
			}
		}
		
		while ( select_next_record( t ) );
		if ( buffer_pointer > 0 ) {
			// save remainder
			fwrite( buffer, buffer_pointer, 1, fsave );
		}
	}
	fclose( fsave );

}
void load_table( table* t, char *filename )
{
	FILE *fload;
	size_t count, index, max_records;
	void *new_record;

	fload = fopen( filename, "rb" );
	if ( fload == NULL ) {
		printf( "error opening file : %s\n", filename );
		return;
	}
	clear_table( t );

	new_record = malloc( t->record_size );
	max_records = SKIPLIST_BUFFER_SIZE / t->record_size;

	do {
		count = fread( buffer, t->record_size, max_records, fload );
		for ( index = 0; index < count; index++ ) {
			memcpy( new_record, buffer + index * t->record_size, t->record_size );
			insert_record( t, new_record );			
		}
	} while ( count > 0 );

	free( new_record );
}

void get_table_statistics( void )
{
	// todo: check height distribution
}

void test_delete_func( void *a )
{
	
}

int test_compare_func( void* a, void *b )
{
	int result;
	if ( a != NULL && b != NULL ) {
		result = strcmp( a, b );
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

void test_skiplist( void )
{
	char element[ 20 ];
	int i;
	table pietje;

	create_table( &pietje, test_compare_func, test_delete_func, sizeof( element ) );

	strcpy( element, "tweede element" );
	insert_record( &pietje, element );

	printf( "record count:%ld\n", pietje.record_count );

	for ( i = 1000; i > 0; i-- ) {
		sprintf( element, "element %d", i );
		insert_record( &pietje, element );
	}

	strcpy( element, "element 5" );
	delete_record( &pietje, element );

	print_table( pietje );
	printf( "record count:%ld\n", pietje.record_count );
	
	save_table( &pietje, "testsave" );
	clear_table( &pietje );
	load_table( &pietje, "testsave" );

	print_table( pietje );
	printf( "record count:%ld\n", pietje.record_count );

	/*
	for ( i = 10000; i > 0; i-- ) {
		sprintf( element, "element %d", i );
		insert_record( &pietje, element );
	}
	print_table( pietje );
	clear_table( &pietje );
	print_table( pietje );
	printf( "record count:%ld\n", pietje.record_count );
	*/
}

