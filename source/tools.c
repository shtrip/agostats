#include <string.h>
#include <ctype.h>
#include "tools.h"


// public functions
void copy_reverse_string( char* dest, char *source )
{
	int idest, isource;

	idest = 0;
	for ( isource = strlen( source ) - 1 ; isource >= 0 ; isource-- ) {
		dest[ idest ] = source[ isource ];
		idest++;
	}
	
	dest[ idest ] = '\0';
}

void strncopy( char *s1, char *s2, long count )
{
	// s1 has to have room for count + 1 characters
	memcpy( (void*)s1, (void*)s2, count ) ;
	s1[ count ] = '\0';
}

// remove leading spaces
void lstrip( char *s )
{
	char *t = s;
 
	while( *s != '\0' && isspace( *s ) ) {
		s++;
	}

	if ( s != t ) {
		memmove( t, s , strlen( s ) + 1 );
	}
}

// strip trailing spaces
char *rstrip( char *s )
{
	int i = strlen( s ) - 1;

	while ( i > 0 && s[ i ]== ' ' ) {
		s[ i ] = '\0';
		i--;
	}

	return s;
}

void string_to_lower( char *s )
{
	char *p = s;
	while ( *p != '\0' ) {
		*p = tolower( *p );
		p++;
	}
}

void string_to_upper( char *s )
{
	char *p = s;
	while ( *p != '\0' ) {
		*p = toupper( *p );
		p++;
	}
}

long my_max ( long v1, long v2 )
{
	if ( v1 > v2 ) {
		return v1;
	}
	else {
		return v2;
	}
}

long my_min( long v1, long v2 )
{
	if ( v1 < v2 ) {
		return v1;
	}
	else {
		return v2;
	}
}


// main: abcdefabc
// find: a
// replace: bb
// main: bbbcdefbbc
void str_replace( char* main, char* find, char* replace )
{
	char* source = main;
	char* dest;
	char* found;
	long length;
	int  len_dif;
	
	int lfind = strlen( find );		  // length of string to find
	int lreplace = strlen( replace ); // length of string to replace
	int mainlen = strlen(main);		  // length of the main string

	if ( lfind == 0 ) {
		return;
	}

	len_dif = lreplace - lfind;

	// while loop to find each occurrence
	while ( (found = strstr( source, find )) != NULL ) {
		source = found;
		if ( lfind != lreplace ) {
			// we have to make room or lose room for the new string
			if ( len_dif < 0 ) {
				source = found + lfind;
				dest   = found + lreplace;
				length = mainlen - (source - main) + 1;
				memmove( dest, source, length );
			}
			else {
				source = found + lfind;
				dest   = source + len_dif;
				length = mainlen - (source - main) + 1;
				memmove( dest, source, length );				
			}
			mainlen += len_dif; // calculate the new length of the main string
		}
		memcpy( found, replace, lreplace );
		source = found + lreplace;
	}
}

//void replace_debuginfo( char *main, char *find, char *replace )
//{
//	char a[ 100 ];
//	char b[ 100 ];
//	char c[ 100 ];
//
//	strcpy( a, main );
//	strcpy( b, find );
//	strcpy( c, replace );
//
//	str_replace( a, b, c );
//	sprintf( outbuf, "replace [%s]->[%s] with [%s] = [%s]\n", main, find, replace, a );
//	printf( outbuf );
//}
//
//void replace_testcases( void )
//{
//	char forbidden[] = "2";
//
//	replace_debuginfo( "ABC123", "1" , "XYZ" );
//	replace_debuginfo( "ABC123", "12" , "1" );
//	replace_debuginfo( "ABC123", "1" , "" );
//
//	replace_debuginfo( "ABC123", "ABC123" , "XYZ" );
//	replace_debuginfo( "ABC123", "ABC123" , "ABCD1234" );
//	replace_debuginfo( "ABC123", "ABC123" , "" );
//
//	replace_debuginfo( "ABABAB", "B" , "" );
//	replace_debuginfo( "ABABAB", "B" , "XYZ" );
//	replace_debuginfo( "ABABAB", "A" , "B" );
//	replace_debuginfo( "AAAAAA", "A" , "B" );
//	replace_debuginfo( "AAAAAA", "B" , "C" );
//	replace_debuginfo( "AAAAAA", "" , "" );
//	
//}

// remove the characters in string forbidden from string source
void filter( char *source, char *forbidden )
{
	int remove = strlen( forbidden ) - 1;
	char temp[ 2 ];

	temp[ 1 ] = '\0';

	while ( remove >= 0 ) {
		temp[ 0 ] = forbidden[ remove ];
		str_replace( source, temp, "" );
		remove --;
	}
}
