#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
	#include <direct.h>
#endif

#include "highscores.h"
#include "awards.h"
#include "matching.h"
#include "players.h"
#include "skiplist.h"
#include "output.h"
#include "tools.h"


FILE *fh_out;
_highscore_type personal_highscores	[ AW_NR_OF_AWARDS ]; // contains player specific highscores
_highscore_type global_highscores   [ AW_NR_OF_AWARDS ];
_highscore_type highscore_file		[ AW_NR_OF_AWARDS ]; // highscores as they are in the highscore file

int				writing_highscores;
int				writing_linkpage;
unsigned long	writing_highscore_player_id = 0;
int				writing_highscore_player_number;
char			filename_buffer [ 256 ];

// todo: highscores v2
table hs_players;			// Player_ID, Player name
table hs_players_index;		// index on playername
table hs_highscores;		// Played_ID, Award_ID, scores

// highscore file format:
// awardID;date;etf playername;score
// separator: /x19

// forward declarations
void generate_htm_highscores( void );
void generate_htm_linkpage( void );

void clear_highscores( _highscore_type *highscores )
{
	int i;

	for ( i = 0; i < AW_NR_OF_AWARDS; i++ ) {
		highscores[ i ].award_ID = -1;
		highscores[ i ].playername[ 0 ] = '\0';
		highscores[ i ].date[ 0 ] = '\0';
		highscores[ i ].score.score1 = 0;
		highscores[ i ].score.score2 = 0;
		highscores[ i ].score.score3 = 0;
		highscores[ i ].score.score4 = 0;
	}
}

void load_record( FILE *filehandle, int *counter, _highscore_type *highscores )
{	
	int	award_id;
	if ( match( linebuffer, "%r\x19%r\x19%r\x19%r\x19%r\x19%r\x19%r\n" ) ) {

		if ( iar == 7 ) {
			counter++;
			award_id = atoi( array[ 0 ] );

			highscores[ award_id ].award_ID =      atoi( array[ 0 ] ); // to indicate it is filled
			strncopy( highscores[ award_id ].date,       array[ 1 ], 19 ); // todo: use constant
			strncopy( highscores[ award_id ].playername, array[ 2 ], 49 );
			highscores[ award_id ].score.score1 = atol(  array[ 3 ] );
			highscores[ award_id ].score.score2 = atol(  array[ 4 ] );
			highscores[ award_id ].score.score3 = atol(  array[ 5 ] );
			highscores[ award_id ].score.score4 = atol(  array[ 6 ] );
		}
	}
}

// read highscores from file in structure
void read_highscores( char *filename, _highscore_type *highscores )
{
	int		counter;
	FILE	*f_highscores;
	
	clear_highscores( highscores );
	counter = 0;
	
	f_highscores = fopen( filename, "r" );
	
	if ( f_highscores != NULL ) {
		while ( NULL != fgets( linebuffer, LINE_BUFFER_SIZE, f_highscores ) ){
			load_record( f_highscores, &counter, highscores );
		}
		fclose( f_highscores );
		memcpy( highscore_file, highscores, sizeof( highscore_file ) );
	}
}

void update_personal_highscores( int player_number )
{
	int a = 0;
	_score score;

	while ( 0 != strcmp( AWARD_LIST[ a ].name, "" ) ) {
		if ( AWARD_LIST[ a ].keep_highscore ) {
			calculate_award_score( AWARD_LIST[ a ].awardID, player_number, &score );
			update_highscores( AWARD_LIST[ a ].awardID, player_number, score, personal_highscores );
		}
		a++;
	}
}

// highscores v2 Begin -------------------------------------------------------------------------------------------------------------
int hs_players_index_compare( void* record1, void* record2 )
{
	int result = 0;
	char* player1 = NULL;
	char* player2 = NULL;
	
	_highscore_player_type* prec1, *prec2;
	
	prec1 = record1;
	prec2 = record2;
	
	if (prec1 !=NULL ) {
		player1 = prec1->playername;
	}
	if (prec2 != NULL ) {
		player2 = prec2->playername;
	}

	if ( player1 != NULL && player2 != NULL ) {
#if defined(_WIN32)			
		if ( stricmp( player1, player2 ) > 0 )  {
			result = 1;
		}
#else // linux
		if ( strcasecmp( player1, player2 ) > 0 )  {
			result = 1;
		}
#endif
		else { 
#if defined(_WIN32)			
			if ( stricmp( player2, player1 ) > 0 ) {
				result = -1;
			}
#else // linux
			if ( strcasecmp( player2, player1 ) > 0 ) {
				result = -1;
			}
#endif			
		}
	} 
	else {
		// a == NULL or b == NULL or both
		if ( player1 == NULL && player2 == NULL ) {
			result = 0;
		}
		else {
			// either ID1 == NULL or Name2 == NULL
			if ( player1 == NULL ) {
				result = -1;
			}
			else {
				result = 1;
			}
		}
	}
	return result;

}
int hs_players_compare( void* ID1, void* ID2 )
{
	int result = 0;
	unsigned long player1;
	unsigned long player2; 

	if ( ID1 != NULL && ID2 != NULL ) {
		player1 = *(unsigned long*)ID1;
		player2 = *(unsigned long*)ID2;
		if ( player1 > player2 ) {
			result = 1;
		}
		else { 
			if ( player2 > player1 ) {
				result = -1;
			}
		}
	} 
	else {
		// a == NULL or b == NULL or both
		if ( ID1 == NULL && ID2 == NULL ) {
			result = 0;
		}
		else {
			// either ID1 == NULL or ID2 == NULL
			if ( ID1 == NULL ) {
				result = -1;
			}
			else {
				result = 1;
			}
		}
	}
	return result;
}

void hs_players_delete( void* data )
{
	_highscore_player_type* record_data = data;
	free( record_data );
}

int hs_playerhighscores_compare( void* record1, void* record2 )
{
	int result = 0;
	_highscore_record_type *rec1, *rec2;

	// lets ignore NULL values
	if ( record1 != NULL && record2 != NULL ) {
		rec1 = record1;
		rec2 = record2;
		if ( rec1->player_ID > rec2->player_ID ) {
			result = 1;
		}
		else {
			if ( rec2->player_ID > rec1->player_ID ) {
				result = -1;
			}
			else {	// they are equal, compare award ID
				if ( rec1->award_ID > rec2->award_ID ) {
					result = 1;
				}
				else {
					if ( rec2->award_ID > rec1->award_ID ) {
						result = -1;
					}
				}
			}
		}
	}
	return result;
}

void hs_playerhighscores_delete( void* data )
{
	_highscore_record_type *record_data = data;
	free( record_data );
}

int find_player_record( unsigned long player_id )
{
	int result = FALSE;	
	unsigned long max_id = 0;
	_highscore_player_type *record;

	if ( select_first_record( &hs_players ) ) {
		do {
			record = hs_players.current->record;
			if ( record->player_ID == player_id ) {
				result = TRUE;
				break;
			}
		} while ( select_next_record( &hs_players ) );
	}
	
	return result;
}
#if defined (_DEBUG)
void print_players( void )
{
	_highscore_player_type *player_record;
	if ( select_first_record( &hs_players )) {
		do {
			player_record = hs_players.current->record;
			printf( "%ld %-30.30s\n", player_record->player_ID, player_record->playername );
		} while ( select_next_record( &hs_players ));
	}
}

void print_highscores( void )
{
	_highscore_player_type *player_record;
	_highscore_record_type *highscore_record;
	unsigned long player_id, award_id, current_player;

	current_player = 0;
	if ( select_first_record( &hs_highscores) ) {
		do {
			highscore_record = hs_highscores.current->record;
			player_id = highscore_record->player_ID;
			award_id = highscore_record->award_ID;
			
			if ( player_id != current_player ) {
				if ( find_player_record( player_id ) ) {
					current_player = player_id;
					player_record = hs_players.current->record;
				}
			}
			printf( "%-30.30s %-30.30s %4ld\n", player_record->playername, AWARD_LIST[ select_award(award_id) ].name, highscore_record->score.score1 );
		} while ( select_next_record( &hs_highscores ) );
	}
}
#endif

void load_individual_highscores( void )
{
	// todo:
	create_table( &hs_players, hs_players_compare, hs_players_delete , sizeof(_highscore_player_type) );
	// same table, sorted by playername
	create_table( &hs_players_index, hs_players_index_compare, hs_players_delete, sizeof(_highscore_player_type) );
	sprintf( filename_buffer, "%s/%s", PERSONAL_DIRECTORY, PERSONAL_PLAYERS );
	load_table( &hs_players, filename_buffer );

	create_table( &hs_highscores, hs_playerhighscores_compare, hs_playerhighscores_delete, sizeof(_highscore_record_type) );
	sprintf( filename_buffer, "%s/%s", PERSONAL_DIRECTORY, PERSONAL_HIGHSCORES );
	load_table( &hs_highscores, filename_buffer );
	
	//print_players(); // todo: remove
	//print_highscores(); // todo: remove

}

void save_individual_highscores( void )
{
	sprintf( filename_buffer, "%s/%s", PERSONAL_DIRECTORY, PERSONAL_PLAYERS );
	save_table( &hs_players		, filename_buffer );
	sprintf( filename_buffer, "%s/%s", PERSONAL_DIRECTORY, PERSONAL_HIGHSCORES );
	save_table( &hs_highscores	, filename_buffer );
	generate_htm_highscores();
}


unsigned long find_player_id( int player_number )
{
	unsigned long result = 0;
	int found = FALSE;
	unsigned long max_id = 0;
	_highscore_player_type *record;
	_highscore_player_type new_record;

	if ( select_first_record( &hs_players ) ) {
		do {
			record = hs_players.current->record;
			max_id = record->player_ID;
			if ( 0 == strcmp( PLAYERS[ player_number ].nameclean, record->playername ) ) {
				found = TRUE;
				result = record->player_ID;
			}
		} while ( select_next_record( &hs_players ) && !found );
	}
	if (!found) {
		// add new ID
		new_record.player_ID = max_id + 1;
		strcpy( new_record.playername, PLAYERS[ player_number ].nameclean );
		insert_record( &hs_players, &new_record );
		result = new_record.player_ID;
	}
	return result;
}

int find_first_playerhighscore_record( unsigned long player_id )
{
	int result = FALSE;
	_highscore_record_type *record;

	if ( select_first_record( &hs_highscores ) ) {
		do {
			record = hs_highscores.current->record;
			if ( record->player_ID == player_id ) {
				result = TRUE;
				break;
			}
		} while ( select_next_record( &hs_highscores ) ); 
	}

	return result;
}

int find_next_playerhighscore_record( unsigned long player_id )
{
	int result = FALSE;
	_highscore_record_type *record;

	if ( select_next_record( &hs_highscores ) ) {
		record = hs_highscores.current->record;
		if ( record->player_ID == player_id ) {
			result = TRUE;
		}
	}

	return result;
}

void load_highscores_from_table( unsigned long player_id, _highscore_type* highscores )
{
	_highscore_record_type *record;
	int award_id;
	int counter = 0;

	clear_highscores( highscores );
	if ( find_first_playerhighscore_record( player_id ) ) {
		do {
			record = hs_highscores.current->record;
			award_id = record->award_ID;
			
			highscores[ award_id ].award_ID = award_id;
			memcpy( &(highscores[ award_id ].score), &(record->score), sizeof( _score ) );
			counter++;

		} while ( find_next_playerhighscore_record( player_id ) );
	}
}

int find_playerhighscore_record( unsigned long player_id, unsigned long award_id )
{
	int result = FALSE;
	_highscore_record_type *record;
	_highscore_record_type new_record;

	
	if ( find_first_playerhighscore_record( player_id ) ) {
		do {
			record = hs_highscores.current->record;
			if ( player_id != record->player_ID ) {
				// we encountered the next player
				break;
			}
			else {
				if ( award_id == record->award_ID ) {
					result = TRUE;
					break;
				}
			}
		} while ( select_next_record( &hs_highscores ) );
	}

	if ( result == FALSE ) {
		new_record.player_ID = player_id;
		new_record.award_ID = award_id;
		new_record.score.score1 = 0;
		new_record.score.score2 = 0;
		new_record.score.score3 = 0;
		new_record.score.score4 = 0;
		insert_record( &hs_highscores, &new_record );
		result = TRUE;
	}

	return result;
}

void update_highscore_table( unsigned long player_id, _highscore_type* highscores )
{
	_highscore_record_type *record;
	int award_id, counter ;
	int award;

	counter = 0;

	while ( counter != AW_NR_OF_AWARDS ) {
		award_id = highscores[ counter ].award_ID;
		if ( award_id != -1 ) {
			award = select_award( award_id );
			if ( AWARD_LIST[ award ].keep_highscore ) {
				if ( awardscore_bigger_than_null( highscores[ counter ].score ) ) {
					if ( find_playerhighscore_record( player_id, award_id ) ) {
						record = hs_highscores.current->record;
						// only if new score bigger than old score ? // todo: test
						if ( score_compare( highscores[ counter ].score, record->score ) > 0 ) {
							memcpy( &(record->score), &(highscores[ counter ].score), sizeof( _score ));
						}
					}
				}
			}
		}
		counter++;
	}
}

int check_data_directory( void )
{
	int status;
#if defined(_WIN32)
	status = mkdir( PERSONAL_DIRECTORY );
#else // linux
	status = mkdir( PERSONAL_DIRECTORY, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif

	return 1==1;
}

void update_player_personal_highscores( void )
{
	unsigned long player_id;
	int player_number;

	if ( check_data_directory() ) {
		// update personal highscores for each player
		for ( player_number = 0; player_number < player_count; player_number++ ) {
			
			player_id = find_player_id( player_number );
			if ( player_id > 0 ) {
				load_highscores_from_table( player_id, personal_highscores );
				update_personal_highscores( player_number );
				update_highscore_table( player_id, personal_highscores );
			}
		}
		save_individual_highscores();
	}
	else {
		// todo: output
	}
}
// highscores v2 Begin -------------------------------------------------------------------------------------------------------------
void reset_highscores( _highscore_type *highscores )
{
	memcpy( highscores, highscore_file, sizeof( highscores ) );
}

void get_highscore( int award_id, _score* highscore, _highscore_type *highscores )
{
	if ( award_id >= 0 && award_id < AW_NR_OF_AWARDS ) {
		highscore->score1 = highscores[ award_id ].score.score1;
		highscore->score2 = highscores[ award_id ].score.score2;
		highscore->score3 = highscores[ award_id ].score.score3;
		highscore->score4 = highscores[ award_id ].score.score4;
	}
}

// update highscores in structure
void update_highscores( int award_id, int player, _score highscore, _highscore_type *highscores  )
{
	// todo: add player if multiple players have the same score
	highscores[ award_id ].award_ID = award_id;
	// todo: safe strcpy
	strcpy( highscores[ award_id ].date, current_date );
	strcpy( highscores[ award_id ].playername, PLAYERS[ player ].name );
	highscores[ award_id ].score.score1 = highscore.score1;
	highscores[ award_id ].score.score2 = highscore.score2;
	highscores[ award_id ].score.score3 = highscore.score3;
	highscores[ award_id ].score.score4 = highscore.score4;
//	strcpy( highscores[ award_id ].date, );
	strcpy( highscores[ award_id ].mapname, map_description );
}

// write highscores from structure to file
void store_highscores(  char *filename, _highscore_type *highscores )
{
	FILE *f_highscores;
	int  counter;

	f_highscores = fopen( filename, "wc" );
	if ( f_highscores != NULL ) {
		
		counter = 0;
		while ( counter != AW_NR_OF_AWARDS ) {
			if ( highscores[ counter ].award_ID != -1 ) {
				sprintf( linebuffer, "%d\x19%s\x19%s\x19%ld\x19%ld\x19%ld\x19%ld\n", 
									highscores[ counter ].award_ID,
									highscores[ counter ].date,
									highscores[ counter ].playername ,
									highscores[ counter ].score.score1,
									highscores[ counter ].score.score2,
									highscores[ counter ].score.score3,
									highscores[ counter ].score.score4 );

				fprintf( f_highscores, linebuffer );
			}
			counter++;
		}
		fclose( f_highscores );
	}

}

void output_highscores( void )
{
	writing_highscores = TRUE;
	AS_OUT( OE_INIT, NULL );
	
	output_highscore_awards( global_highscores );
	
	AS_OUT( OE_TERM, NULL );
	writing_highscores = FALSE;
}

void generate_htm_highscores( void )
{
	int player_number;

	writing_highscores = TRUE;
	
	//print_highscores();
	for ( player_number = 0; player_number < player_count; player_number++ ) {
		writing_highscore_player_number = player_number;
		writing_highscore_player_id = find_player_id( player_number );
		
		load_highscores_from_table( writing_highscore_player_id, personal_highscores );
		AS_OUT( OE_INIT, NULL );
		output_highscore_awards( personal_highscores );
		AS_OUT( OE_TERM, NULL );
	}
	writing_highscore_player_id = 0;

	// write linkpage
	writing_linkpage = TRUE;
	AS_OUT( OE_INIT, NULL );
	generate_htm_linkpage();
	AS_OUT( OE_TERM, NULL );
	writing_linkpage = FALSE;

	// write navigation page - todo:
	
	writing_highscores = FALSE;
}

void generate_htm_linkpage( void )
{
	// loop all players
	// todo: sort by name, case insensitive, create second list
	// todo: create directory agostats data

	_highscore_player_type *player_record;

	// create index on playername
	if ( select_first_record( &hs_players )) {
		do {
			player_record = hs_players.current->record;
			insert_record( &hs_players_index, player_record  );

		} while ( select_next_record( &hs_players ));
	}
	
	if ( select_first_record( &hs_players_index )) {
		do {
			player_record = hs_players_index.current->record;
			//printf( "%ld %-30.30s\n", player_record->player_ID, player_record->playername );
			sprintf( htmlbuf, "%s", player_record->playername );
			convert_plaintext_to_html( htmlbuf );
			sprintf( outbuf, "<a href=\"p%ld.htm\" target=\"scoreframe\">%s</a><br>\n", player_record->player_ID, htmlbuf );
			//sprintf( outbuf, "<a href=\"p%ld.htm\" target=\"scoreframe\">%s</a><br>\n", player_record->player_ID, player_record->playername );
			AS_OUT( OE_PLAIN, outbuf );

		} while ( select_next_record( &hs_players_index ));
	}
}
