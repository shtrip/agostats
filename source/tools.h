// tools module
#if !defined(TOOLS_H_INCLUDED)
#define TOOLS_H_INCLUDED

#define  is_digit(x)    ('0'<=(x)  &&  (x)<='9')

// types
typedef enum {
	OBJ_UNDEFINED,
	OBJ_INIT,
	OBJ_PROCESS,
	OBJ_TERM,
} OBJ_MESSAGE;

// public functions
extern void  lstrip					( char *s );
extern char* rstrip					( char *s );
extern void  string_to_lower		( char *s );
void string_to_upper				( char *s );
extern void  strncopy				( char *s1, char *s2, long count );
extern long  my_max					( long v1, long v2 );
extern long  my_min					( long v1, long v2 );
extern void  copy_reverse_string	( char* dest, char *source );
extern void  str_replace			( char *main, char *find, char *replace );
extern void  filter					( char *source, char *forbidden );
#endif

