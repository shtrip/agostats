// skiplist module
#if !defined(SKIPLIST_H)
#define SKIPLIST_H

#define MAX_LEVEL 10

typedef struct _node {
	//int 		  height; // todo: remove
	struct _node* forward[ MAX_LEVEL ];
	void*		  record;
} node;

typedef struct _table {
	int 		  height;					   // current height of the skip list
	node*		  list;						   // the header of the list
	node*         previous[ MAX_LEVEL ];       // this array is filled during a search
	node*         current;					   // points to current record
	
	int			  record_size;				   // size of the structure that holds the record
	long		  record_count;
	int           (*compare) ( void*, void* ); // compare function to compare the keys of 2 records
	void          (*free_record) (void *);      // delete function to free memory allocated by record
	// todo: keep more statistics
	
} table;

extern void test_skiplist( void );
void create_table( table* t, int (*compare_function) ( void*, void* ), void (*delete_function) (void*), int record_size );
extern void insert_record( table* t, void* record );
extern void delete_record( table* t, void* record );
extern void print_table( table t );
extern void clear_table( table* t );
extern int  select_first_record( table *t );
extern int  select_next_record( table *t );
extern void save_table( table* t, char *filename );
extern void load_table( table* t, char *filename );

#endif

