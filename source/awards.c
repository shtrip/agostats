#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tools.h"
#include "sorting.h"
#include "output.h"
#include "players.h"
#include "weapons.h"
#include "highscores.h"
#include "awards.h"

/*
Death and destruction
	Terminator 			AGO Th for fragging 113 enemies.
	Serial killer 		AGO Th for a streak of 15 frags.
	Fragmeat 			AGO black for being fragged 78 times.
	Resupply defender 	AGO S3D|B for being fragged only 25 times.
	Godzilla 			JM|tinkywinkie for destroying 8 autosentries and 0 supplystations.
	Action Man          AGO Bla for 100 frags + deaths

Flag action
	Penetrator 			AGO f1y for 11 first touches.
	Escape artist 	    JM|tinkywinkie for 2 coast to coast captures.
	Wandering hands 	AGO black for 22 touches.
	Saving private ryan
	Goalkeeper
	Flying goalkeeper
	Terrier				GmbH*Foxz for killing the flagcarrier 22 times.  

Stoopiiiid!!!!!!
	Captain Lemming 	AGO dubtwist for committing suicide 11 times.
	SUCKZILLA!			for teamkilling sentries
	Do we still want you? 	AGO dubtwist for killing 8 teammates.
	Jackass 			AGO Pook for sitting on his grenade 4 times.
	Sucker				for teamkilling the flagcarrier
	Meteorite			for cratering x times

In Your Face
	Rocketlauncher		AGO Pook for 8 direct rocket kills.
	Grenadelauncher    	JM|King for 8 direct grenadelauncher kills.
	Sniperrifle			AGO Th for 54 headshot kills.
	wink wink    		for x frags by flashgrenade

Specialist
	Shotgun		     	AGO S3D|B for 10 shotgun kills.
	Flamethrower		JM|Timber for 24 flamethrower kills.
	Nailgun 			AGO black for 15 nailgun kills.
	Minigun 			JM|Scorpion for 39 minigun kills.
	Sentry  			AGO S3D|B for his sentry getting 32 kills
	Stalker 	        AGO f1y for 2 close range kills.
	Tosser 				JM|tinkywinkie for 32 handgrenade kills.

Lucky bitch
	Heavy explosive charge
	Cluster
	Nailgrenade
	Lavagrenade

	
*/

typedef enum {
	SM_DEFAULT = 0,
	SM_COMBINED,
	SM_OVERRULE,
	SM_SUM
} _sort_method;

typedef enum { // sort order
	SO_DESCENDING = 0, // highest value first
	SO_ASCENDING   // lowest value first
} _sort_order;


_award_groups current_group;

// todo: add award here
void calculate_award_score( int award, int p, _score* result )
{
	result->score1 = 0L;
	result->score2 = 0L;
	result->score3 = 0L;
	result->score4 = 0L;
	
	switch( award ) {
		case AW_TERMINATOR :
			result->score1  = PLAYERS[ p ].frags;
			break;
		case AW_SERIAL_KILLER :
			result->score1 = PLAYERS[ p ].max_fragstreak;
			break;
		case AW_FRAGMEAT :
			result->score1 = PLAYERS[ p ].deaths;
			break;
		case AW_RESUP_DEFENDER:
			result->score1 = PLAYERS[ p ].deaths;
			break;
		case AW_GODZILLA :
			result->score1 = PLAYERS[ p ].destroyed_sentry; 
			result->score2 = PLAYERS[ p ].destroyed_supply;
			break;
		case AW_ACTION_MAN :
			result->score1 = PLAYERS[ p ].frags + PLAYERS[ p ].deaths;
			break;
		case AW_PENETRATOR :
			result->score1 = PLAYERS[ p ].first_touches;
			break;
		case AW_ESCAPE_ARTIST :
			result->score1 = PLAYERS[ p ].coast_to_coast;
			break;
		case AW_WANDERING_HANDS :
			result->score1 = PLAYERS[ p ].flagtouches;
			break;
		case AW_SAVING_PRIVATE_RYAN :
			result->score1 = PLAYERS[ p ].defend_flagcarrier;
			break;
		case AW_GOALKEEPER :
			result->score1 = PLAYERS[ p ].defend_flag_at_base;
			break;
		case AW_FLYING_GOALKEEPER :
			result->score1 = PLAYERS[ p ].defend_flag_in_field;
			break;
		case AW_TERRIER :
			result->score1 = PLAYERS[ p ].killed_flagcarrier;
			break;
		case AW_CAPTAIN_LEMMING :
			result->score1 = PLAYERS[ p ].suicides;
			break;
		case AW_SUCKZILLA :
			result->score1 = PLAYERS[ p ].teamkilled_sentry;
			break;
		case AW_TEAMKILLER :
			result->score1 = PLAYERS[ p ].teamkills;
			break;
		case AW_JACKASS :
			result->score1 = PLAYERS[ p ].sit_on_grenade;
			break;
		case AW_SUCKER :
			result->score1 = PLAYERS[ p ].teamkilled_flagcarrier;
			break;
		case AW_METEORITE :
			result->score1 = frag_matrix[ p ][ p ].frags_per_weapon[ W_FALLING ];
			break;
		case AW_ROCKETLAUNCHER :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_ROCKET_LAUNCHER_DIRECT ];
			break;
		case AW_GRENADELAUNCHER :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_RED_PIPE_DIRECT ];
			break;
		case AW_SNIPERRIFLE :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_SNIPERRIFLE_HEAD ];
			break;
		case AW_FLASH_GRENADE :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_FLASH_GRENADE ];
			break;
		case AW_SHOTGUN :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_SINGLE_SHOTGUN ] + PLAYERS[ p ].frags_per_weapon[ W_DOUBLE_SHOTGUN ];
			break;
		case AW_FLAMETHROWER :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_FLAMETHROWER ];
			break;
		case AW_NAILGUN :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_SMALL_NAILGUN ] + PLAYERS[ p ].frags_per_weapon[ W_SUPER_NAILGUN ];
			break;
		case AW_MINIGUN :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_MINIGUN ];
			break;
		case AW_SENTRY :
			result->score1 = (	PLAYERS[ p ].frags_per_weapon[ W_AUTOSENTRY_BULLET ] + 
						PLAYERS[ p ].frags_per_weapon[ W_AUTOSENTRY_ROCKET ] + 
						PLAYERS[ p ].frags_per_weapon[ W_AUTOSENTRY_EXPLOSION ] );
			break;
		case AW_STALKER :
			result->score1 = (	PLAYERS[ p ].frags_per_weapon[ W_BATTLEAXE ]	+
						PLAYERS[ p ].frags_per_weapon[ W_KNIFE ]		+
						PLAYERS[ p ].frags_per_weapon[ W_SYRINGE ]		+
						PLAYERS[ p ].frags_per_weapon[ W_DISEASE ]		+
						PLAYERS[ p ].frags_per_weapon[ W_WRENCH ]			);
			break;
		case AW_TOSSER :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_HAND_GRENADE ];
			break;
		case AW_SANDMAN :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_DARTGUN ];
			break;
		case AW_HE_CHARGE:
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_HE_CHARGE ];
			break;
		case AW_CLUSTER :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_CLUSTER_GRENADE ];
			break;
		case AW_NAILGRENADE :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_NAIL_GRENADE ];
			break;
		case AW_LAVAGRENADE :
			result->score1 = PLAYERS[ p ].frags_per_weapon[ W_NAPALM_GRENADE ];
			break;

		case AW_MOST_TEAMKILLED:
			result->score1 = PLAYERS[ p ].teamdeaths;
			break;

		case AW_FIRST_BLOOD:			
			if ( first_blood_awarded && p == first_blood_killer ) {
				result->score1 = 10;
			}
			break;
	}
}
// BEGIN COMPARE FUNCTIONS for sorting out the winners

long score_compare( _score s1, _score s2 )
{
	long result;

	result = s1.score1 - s2.score1;
	if ( result == 0L ) {
		result = s1.score2 - s2.score2;
		if ( result == 0L ) {
			result = s1.score3 - s2.score3;
			if ( result == 0L ) {
				result = s1.score4 - s2.score4;
			}
		}
	}

	return result;
}
long default_compare( int award, int p1, int p2 )
{
	long result;
	_score score_p1, score_p2;

	calculate_award_score( award, p1, &score_p1 );
	calculate_award_score( award, p2, &score_p2 );

	result = score_compare( score_p1, score_p2 );

	return result;
}

// todo: add compare function for new awards here

// END COMPARE FUNCTIONS

// The award list
// This list determines in what order the awards/highscores are presented
// todo: add new award here
meta_award AWARD_LIST[] = {

	AW_FIRST_BLOOD,				AG_DEATH_AND_DESTRUCTION,			"First blood",				default_compare,	SO_DESCENDING,	FALSE,
	AW_TERMINATOR,				AG_DEATH_AND_DESTRUCTION,			"Terminator",				default_compare,	SO_DESCENDING,	TRUE,
	AW_SERIAL_KILLER,			AG_DEATH_AND_DESTRUCTION,			"Serial killer",			default_compare,	SO_DESCENDING,	TRUE,
	AW_FRAGMEAT,				AG_DEATH_AND_DESTRUCTION,			"Fragmeat",					default_compare,	SO_DESCENDING,	TRUE,
	AW_RESUP_DEFENDER,			AG_DEATH_AND_DESTRUCTION,			"Resupply defender",		default_compare,	SO_ASCENDING,	FALSE,
	AW_GODZILLA,				AG_DEATH_AND_DESTRUCTION,			"Godzilla",					default_compare,	SO_DESCENDING,	TRUE,
	AW_ACTION_MAN,				AG_DEATH_AND_DESTRUCTION,			"Action Man",				default_compare,	SO_DESCENDING,	TRUE,

	AW_PENETRATOR,				AG_FLAG_ACTION,						"Penetrator",				default_compare,	SO_DESCENDING,	TRUE,
	AW_ESCAPE_ARTIST,			AG_FLAG_ACTION,						"Escape Artist",			default_compare,	SO_DESCENDING,	TRUE,
	AW_WANDERING_HANDS,			AG_FLAG_ACTION,						"Wandering Hands",			default_compare,	SO_DESCENDING,	TRUE,
	AW_SAVING_PRIVATE_RYAN,		AG_FLAG_ACTION,						"Saving Private Ryan",		default_compare,	SO_DESCENDING,	TRUE,
	AW_GOALKEEPER,				AG_FLAG_ACTION,						"Goalkeeper",				default_compare,	SO_DESCENDING,	TRUE,
	AW_FLYING_GOALKEEPER,		AG_FLAG_ACTION,						"Flying Goalkeeper",		default_compare,	SO_DESCENDING,	TRUE,
	AW_TERRIER,					AG_FLAG_ACTION,						"Terrier",					default_compare,	SO_DESCENDING,	TRUE,

	AW_CAPTAIN_LEMMING,			AG_STUPID,							"Captain Lemming",			default_compare,	SO_DESCENDING,	TRUE,
	AW_SUCKZILLA,				AG_STUPID,							"SUCKZILLA!",				default_compare,	SO_DESCENDING,	TRUE,
	AW_TEAMKILLER,				AG_STUPID,							"Do we still want you?",	default_compare,	SO_DESCENDING,	TRUE,
	AW_JACKASS,					AG_STUPID,							"Jackass",					default_compare,	SO_DESCENDING,	TRUE,
	AW_SUCKER,					AG_STUPID,							"Sucker",					default_compare,	SO_DESCENDING,	TRUE,
	AW_METEORITE,				AG_STUPID,							"Meteorite",				default_compare,	SO_DESCENDING,	TRUE,
	AW_MOST_TEAMKILLED,			AG_STUPID,							"Still doesn't get it",		default_compare,	SO_DESCENDING,	TRUE,

	AW_ROCKETLAUNCHER,			AG_IN_YOUR_FACE,					"Rocketlauncher",			default_compare,	SO_DESCENDING,	TRUE,
	AW_GRENADELAUNCHER,			AG_IN_YOUR_FACE,					"Grenadelauncher",			default_compare,	SO_DESCENDING,	TRUE,
	AW_SNIPERRIFLE,				AG_IN_YOUR_FACE,					"Sniperrifle",				default_compare,	SO_DESCENDING,	TRUE,
	AW_FLASH_GRENADE,			AG_IN_YOUR_FACE,					"For your eyes only",		default_compare,	SO_DESCENDING,	TRUE,

	AW_SHOTGUN,					AG_SPECIALIST,						"Shotgun",					default_compare,	SO_DESCENDING,	TRUE,
	AW_FLAMETHROWER,			AG_SPECIALIST,						"Flamethrower",				default_compare,	SO_DESCENDING,	TRUE,
	AW_NAILGUN,					AG_SPECIALIST,						"Nailgun",					default_compare,	SO_DESCENDING,	TRUE,
	AW_MINIGUN,					AG_SPECIALIST,						"Minigun",					default_compare,	SO_DESCENDING,	TRUE,
	AW_SENTRY,					AG_SPECIALIST,						"Sentry",					default_compare,	SO_DESCENDING,	TRUE,
	AW_STALKER,					AG_SPECIALIST,						"Stalker",					default_compare,	SO_DESCENDING,	TRUE,
	AW_TOSSER,					AG_SPECIALIST,						"Tosser",					default_compare,	SO_DESCENDING,	TRUE,
	AW_SANDMAN,					AG_SPECIALIST,						"Enter sandman",			default_compare,	SO_DESCENDING,	TRUE,

	AW_HE_CHARGE,				AG_LUCKY_BITCH,						"Charge",					default_compare,	SO_DESCENDING,	TRUE,
	AW_CLUSTER,					AG_LUCKY_BITCH,						"Clusterbomb",				default_compare,	SO_DESCENDING,	TRUE,
	AW_NAILGRENADE,				AG_LUCKY_BITCH,						"Nail grenade",				default_compare,	SO_DESCENDING,	TRUE,
	AW_LAVAGRENADE,				AG_LUCKY_BITCH,						"Napalm grenade",			default_compare,	SO_DESCENDING,	TRUE,

	AW_UNDEFINED,				AG_UNDEFINED,						"",							NULL,				0			 ,  FALSE
};

int select_award( int award_id )
{
	int result = 0;
	int found = FALSE;
	do {
		if ( AWARD_LIST[ result ].awardID == award_id ) {
			found = TRUE;
			break;
		}
		else {
			result++;
		}
	} while ( AWARD_LIST[ result ].awardID != AW_UNDEFINED );
	
	if ( found == FALSE ) {
		result = 0;
	}
	return result;
}

void awards_sort_swap( int i, int j )
{
	int temp;

	temp = player_index[ i ];
	player_index[ i ] = player_index[ j ];
	player_index[ j ] = temp;
}

void awards_sort_players( int award )
{
	int sort_order = AWARD_LIST[ award ].sort_order;
	int i, j;
	int p1, p2;
	long (*sort_func)( int, int, int );
	long sort_compare;
	
	// init index;
	for ( i=0; i< player_count; i++ ) {
		player_index[ i ] = i;
	}

	sort_func = AWARD_LIST[ award ].sort_func;

	for ( i = 0 ; i < player_count ; i++ ) {
		for ( j = i + 1 ; j < player_count ; j++ ) {
			
			p1 = player_index[ i ];
			p2 = player_index[ j ];
			sort_compare = sort_func( AWARD_LIST[ award ].awardID,p1, p2 ); 

			switch( sort_order ) {
				case SO_ASCENDING :
					if ( sort_compare > 0 ) {
						awards_sort_swap( i, j );
					}
					break;
				case SO_DESCENDING :
					if ( sort_compare < 0 ) {
						awards_sort_swap( i, j );
					}
					break;
			}
		}
	}
}

char *query_award_groupname( _award_groups ag )
{
	switch( ag ) {

		case AG_DEATH_AND_DESTRUCTION	: return "Death and Destruction";
		case AG_FLAG_ACTION				: return "Flag Action";
		case AG_STUPID					: return "Stupiiid!";
		case AG_IN_YOUR_FACE			: return "In Your Face";
		case AG_SPECIALIST				: return "Specialist";
		case AG_LUCKY_BITCH				: return "Lucky Bitch";
	}

	return "";
}

void award_output_group( _award_groups ag )
{
	// todo: class
	sprintf( outbuf, HTML_AWARD_GROUPNAME, query_award_groupname( ag ) );
	AS_OUT( OE_PLAIN, outbuf );
}

void award_output_awardname( int award )
{
	int p = player_index[ 0 ];
	_score final_award_score;
	_score current_highscore;

	calculate_award_score( AWARD_LIST[ award ].awardID, p, &final_award_score );
	get_highscore( AWARD_LIST[ award ].awardID, &current_highscore, global_highscores );

	if ( AWARD_LIST[ award ].keep_highscore ) {
		if ( score_compare( final_award_score, current_highscore ) > 0L ) {
			// there is a new highscore
			update_highscores( AWARD_LIST[ award ].awardID, p, final_award_score, global_highscores );
			sprintf( outbuf, HTML_AWARD_NAME_HIGHSCORE, AWARD_LIST[ award ].name );
		}
		else {
			sprintf( outbuf, HTML_AWARD_NAME, AWARD_LIST[ award ].name );
		}
	}
	else {
		sprintf( outbuf, HTML_AWARD_NAME, AWARD_LIST[ award ].name );
	}
	AS_OUT( OE_PLAIN, outbuf );
}

void award_output_players( int award )
{
	int p;
	long (*compare)( int, int, int );
	int player_printed = FALSE;
	
	compare = AWARD_LIST[ award ].sort_func;
	p = 0;

	while ( p < player_count && ( 0 == compare( AWARD_LIST[ award ].awardID, player_index[ 0 ], player_index[ p ] ) ) ) {
		if ( player_printed ) {
			//print a comma before the next player
			sprintf( outbuf, ", " );
			AS_OUT( OE_PLAIN, outbuf );
		}
		sprintf( outbuf, "%s", html_player_clickable( player_index [ p ] ) );
		AS_OUT( OE_PLAIN, outbuf );

		player_printed = TRUE;
		p++;
	}
}

// todo: add new awards here
// this function tells if the award should be printed at all
int awardscore_bigger_than_null( _score s )
{
	return ( s.score1 > 0 || s.score2 > 0 || s.score3 > 0 || s.score4 > 0 );
}

int query_output_award( int award )
{
	int result = FALSE;
	int p = player_index[ 0 ];
	_score score;

	calculate_award_score( award, p, &score );

	switch( award ) {
		case AW_FIRST_BLOOD:
			result = ( first_blood_awarded == TRUE );
			break;
		default:
			
			result = ( score.score1 > 0 || score.score2 > 0 || score.score3 > 0 || score.score4 > 0 );
			break;
	}

	return result;
}
// todo: add award here
void award_output_description( int award, int for_highscore, _highscore_type *highscores )
{
	int  p = player_index[ 0 ];
	_score final_award_score;
	
	outbuf[ 0 ] = '\0';

	if ( for_highscore ) {
		get_highscore( AWARD_LIST[ award ].awardID, & final_award_score, highscores );
		//final_award_score = highscores[ AWARD_LIST[ award ].awardID ].score;
	}
	else {
		calculate_award_score( AWARD_LIST[ award ].awardID, p, &final_award_score );
	}

	switch( AWARD_LIST[ award ].awardID ) {
		case AW_TERMINATOR :
			sprintf( outbuf, " for fragging %ld enemies.", final_award_score.score1 );
			break;
		case AW_SERIAL_KILLER :
			sprintf( outbuf, " for a streak of %ld frags.", final_award_score.score1 );
			break;
		case AW_FRAGMEAT :
			sprintf( outbuf, " for being fragged %ld times.", final_award_score.score1 );
			break;
		case AW_RESUP_DEFENDER :
			sprintf( outbuf, " for being fragged only %ld times.", final_award_score.score1 );
			break;
		case AW_GODZILLA :
			// only count sentries for highscore
			sprintf( outbuf, " for destroying %ld autosentries and %ld supplystations.", final_award_score.score1 , final_award_score.score2 );
			break;
		case AW_ACTION_MAN :
			sprintf( outbuf, " for %ld frags + deaths.", final_award_score.score1 );
			break;
		case AW_PENETRATOR:
			sprintf( outbuf, " for taking the flag from base %ld times.", final_award_score.score1 );
			break;
		case AW_ESCAPE_ARTIST:
			sprintf( outbuf, " for %ld coast to coast captures.", final_award_score.score1 );
			break;
		case AW_WANDERING_HANDS:
			sprintf( outbuf, " for %ld flagtouches.", final_award_score.score1 );
			break;
		case AW_SAVING_PRIVATE_RYAN :
			sprintf( outbuf, " for defending the flagcarrier %ld times.", final_award_score.score1 );
			break;
		case AW_GOALKEEPER :
			sprintf( outbuf, " for defending the flag at base %ld times.", final_award_score.score1 );
			break;
		case AW_FLYING_GOALKEEPER :
			sprintf( outbuf, " for defending the flag in the field %ld times.", final_award_score.score1 );
			break;
		case AW_TERRIER :
			sprintf( outbuf, " for killing the flagcarrier %ld times.", final_award_score.score1 );
			break;
		case AW_CAPTAIN_LEMMING :
			sprintf( outbuf, " for committing suicide %ld times.", final_award_score.score1 );
			break;
		case AW_SUCKZILLA :
			sprintf( outbuf, " for teamkilling a sentry %ld times.", final_award_score.score1 );
			break;
		case AW_TEAMKILLER :
			sprintf( outbuf, " for killing %ld teammates.", final_award_score.score1 );
			break;
		case AW_JACKASS :
			sprintf( outbuf, " for sitting on his grenade %ld times.", final_award_score.score1 );
			break;
		case AW_SUCKER :
			sprintf( outbuf, " for teamkilling the flagcarrier %ld times.", final_award_score.score1 );
			break;
		case AW_METEORITE :
			sprintf( outbuf, " for cratering %ld times.", final_award_score.score1 );
			break;
		case AW_ROCKETLAUNCHER :
			sprintf( outbuf, " for %ld direct rocket kills.", final_award_score.score1 );
			break;
		case AW_GRENADELAUNCHER :
			sprintf( outbuf, " for %ld direct red pipe kills.", final_award_score.score1 );
			break;
		case AW_SNIPERRIFLE :
			sprintf( outbuf, " for %ld headshot kills.", final_award_score.score1 );
			break;
		case AW_FLASH_GRENADE :
			sprintf( outbuf, " for %ld flashbang kills.", final_award_score.score1 );
			break;
		case AW_SHOTGUN :
			sprintf( outbuf, " for %ld shotgun kills.", final_award_score.score1 );
			break;
		case AW_FLAMETHROWER :
			sprintf( outbuf, " for %ld flamethrower kills.", final_award_score.score1 );
			break;
		case AW_NAILGUN :
			sprintf( outbuf, " for %ld nailgun kills.", final_award_score.score1 );
			break;
		case AW_MINIGUN :
			sprintf( outbuf, " for %ld minigun kills.", final_award_score.score1 );
			break;
		case AW_SENTRY :
			sprintf( outbuf, " for letting his autosentry get %ld kills.", final_award_score	);
			break;
		case AW_STALKER :
			sprintf( outbuf, " for %ld close range kills.",final_award_score.score1 );
			break;
		case AW_TOSSER :
			sprintf( outbuf, " for %ld handgrenade kills.", final_award_score.score1 );
			break;
		case AW_SANDMAN:
			switch( final_award_score.score1) {
				case 1: sprintf( outbuf, " for putting 1 player to sleep using the dartgun.", final_award_score.score1 ); 
					break;
				default:sprintf( outbuf, " for putting %ld people to sleep using the dartgun.", final_award_score.score1 );
					break;
			}	
			
			break;
		case AW_HE_CHARGE :
			sprintf( outbuf, " for %ld kills with a heavy explosive charge.", final_award_score.score1 );
			break;
		case AW_CLUSTER :
			sprintf( outbuf, " for %ld clusterbomb kills.", final_award_score.score1 );
			break;
		case AW_NAILGRENADE :
			sprintf( outbuf, " for %ld nail grenade kills.", final_award_score.score1 );
			break;
		case AW_LAVAGRENADE :
			sprintf( outbuf, " for %ld napalm grenade kills.", final_award_score.score1 );
			break;

		case AW_MOST_TEAMKILLED :
			sprintf( outbuf, " for being teamkilled %ld times.", final_award_score.score1 );
			break;
		case AW_FIRST_BLOOD:
			sprintf( outbuf, " got the first kill. The victim was %s.", html_player_clickable( first_blood_victim ) );
			break;
	}

	if ( for_highscore == FALSE ) {
		if ( outbuf[ 0 ] != '\0' ) {
			AS_OUT( OE_PLAIN, outbuf );
		}
	}
	else {
		// this is just for highscores
	}
}

void output_award( int award )
{
	sprintf( outbuf, HTML_AWARD_ROW_INIT );
	AS_OUT( OE_PLAIN, outbuf );

	award_output_awardname( award );

	sprintf( outbuf, "<td> " );
	AS_OUT( OE_PLAIN, outbuf );

	award_output_players( award );

	award_output_description( award, FALSE, NULL );

	sprintf( outbuf, "</td>" );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, HTML_AWARD_ROW_TERM );
	AS_OUT( OE_PLAIN, outbuf );
}

void output_highscore_award( int award, _highscore_type *highscores )
{
	sprintf( outbuf, HTML_AWARD_ROW_INIT );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, HTML_AWARD_NAME, AWARD_LIST[ award ].name );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, "<td> " );
	AS_OUT( OE_PLAIN, outbuf );

	
	if ( writing_highscore_player_id > 0 ) {
		sprintf( outbuf, "%s", PLAYERS[ writing_highscore_player_number ].name_html );
	}
	else {
		sprintf( outbuf, "%s", et_2_html( global_highscores[ AWARD_LIST[ award ].awardID ].playername ) );
	}
	AS_OUT( OE_PLAIN, outbuf );

	if ( writing_highscore_player_id > 0 ) {
		award_output_description( award, TRUE, highscores );
	}
	else {
		award_output_description( award, TRUE, global_highscores );
	}
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, "</td>" );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, HTLM_HIGHSCORE_DATE, highscores[ AWARD_LIST[ award ].awardID ].date );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, HTML_AWARD_ROW_TERM );
	AS_OUT( OE_PLAIN, outbuf );
}

void output_highscore_awards( void *hs )
{
	int a = 0;
	
	_highscore_type *highscores = hs;
	current_group = AG_UNDEFINED;
	sprintf( outbuf, HTML_AWARD_HIGHSCORE_INIT );
	AS_OUT( OE_PLAIN, outbuf );

	while ( 0 != strcmp( AWARD_LIST[ a ].name, "" ) ) {

		if ( AWARD_LIST[ a ].keep_highscore && 
			awardscore_bigger_than_null( highscores[ AWARD_LIST[ a ].awardID ].score ) ) {

			if ( current_group != AWARD_LIST[ a ].group ) {
				current_group = AWARD_LIST[ a ].group;
				sprintf( outbuf, HTML_AWARD_GROUPNAME, query_award_groupname( current_group ) );
				AS_OUT( OE_PLAIN, outbuf );
			}
			output_highscore_award( a, highscores );
		}
		a++;
	}
	sprintf( outbuf, HTML_AWARD_TERM );
	AS_OUT( OE_PLAIN, outbuf );
}

void output_awards_v2( void )
{
	int a = 0;
	void *temp_storage;

	temp_storage = push_player_index();
	current_group = AG_UNDEFINED;

	// create some space between this and previous item
	sprintf( outbuf, "\n<br />\n<br />\n" );
	AS_OUT( OE_PLAIN, outbuf );

	sprintf( outbuf, HTML_AWARD_INIT );
	AS_OUT( OE_PLAIN, outbuf );

	while ( 0 != strcmp( AWARD_LIST[ a ].name, "" ) ) {
		awards_sort_players( a );
		if ( query_output_award( AWARD_LIST[ a ].awardID ) ) {
			if ( current_group != AWARD_LIST[ a ].group ) {
				current_group = AWARD_LIST[ a ].group;
				award_output_group( current_group );
			}
			output_award( a );
		}
		a++;
	}

	sprintf( outbuf, HTML_AWARD_TERM );
	AS_OUT( OE_PLAIN, outbuf );

	// restore original sorting for other tables
	pop_player_index( temp_storage );

}

