#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "maps.h"
#include "matching.h"
#include "tools.h"

//  strings compiled from :"...loaded 11737 faces, 717 meshes, 263 trisurfs, 0 flares 0 foliage"

char map_description [ MAP_NAME_SIZE ];
int  map_id; // location in MAP_INFOS array

mapinfo MAP_INFOS[] = {
	// mapname					, map description					, map signature
	
	// q3f
	"chill"						, "Castle Hill"						, " 8678  267  105  246",
	"duel321"					, "Duel321"							, "  101    0    4    0",
	"q3f_2machse"				, "Mach"							, " 7043  294    0  253",
	"q3f_2fort5"				, "2Fort5"							, " 2153    0    0    0",
	"q3f_2night3"				, "The Morning After (29/03/00)"	, "20292  162   96  293",
	"q3f_2stag2"				, "Stag"							, " 8921    0    0 1058",
	"q3f_32smooth"				, "32 Smooth"						, " 4939    4   12    0",
	"q3f_anything"				, "Q3F Anything"					, "18954  230   66    0",
	"q3f_bam5"					, "Bam 5"							, " 4947  186    1   44",
	"q3f_castle"				, "King of the Castle"				, " 4796    0  256    0",
	"q3f_cath1"					, "The Cathedrals"					, "10732  536  176  246",
	"q3f_causeway"				, "The Causeway"					, "21236  342  123    0",
	"q3f_cbases"				, "Q3F cBases - (30/03/2000-b)"		, " 7173  464    0  246",
	"q3f_chaos3"				, "Chaos 3 (30/03/00)"				, " 2190    2    0   40",
	"q3f_dbb+bases"				, "DBB+Bases"						, " 6828   46   82    4",
	"q3f_duckbowl"				, "Duckbowl"						, "  386   22   50    0",
	"q3f_dungeon"				, "Riptor'z Dungeon"				, "12728   28  246    0",
	"q3f_engbat"				, "Engineer Battle"					, "  849    0   12   16",
	"q3f_flib30d"				, "Mr. Flibbles Bases"				, " 1490    4    0  126",
	"q3f_forts"					, "Forts"							, "10216  600  288    0",
	"q3f_gotduck"				, "Got Duck?"						, " 1639    0    0    0",
	"q3f_h4rdcore"				, "Hardcore"						, " 2745  140  542    0",
	"q3f_liberty5r"				, "Liberty5"						, " 8478   25    0   65",
	"q3f_lastresort"			, "The Last Resort"					, "12178  152  382   69",
	"q3f_openfire2a"			, "Openfire 2a"						, " 5070  102  102   22",
	"q3f_orbital"				, "Orbital Abstraction"				, "15214  100   27    0",
	"q3f_outdoor"				, "Condorman's Great Outdoors"		, " 6746    0   48    0",
	"q3f_smartb"				, "Smart Bases"						, "15555   86    0  942",
	"q3f_smartbases"			, "Smart Bases"						, "18562  134    0    0",
	"q3f_sorrow"				, "Vault of Sorrows"				, "12420  109  133    0",
	"q3f_spamtech"				, "Spamtech"						, "11967  196    2    0",
	"q3f_tf2k"					, "The Final 2 Kingdoms"			, " 7288   32   20    0",
	"q3f_thepit"				, "The Pit"							, " 2138    0    0    0",
	"q3f_well"					, "oh Well"							, "10474   32    0    0",
	"waste"						, "Waste"							, " 2854  170    0   42",
	// q3f

	// q3f2

	// q3f2

	// etf
	"2legofort2"				, "Lego Forts"						, " 1324    0 6122    0    0",
	"etf_2night3"				, "2Night3: The Morning After"		, "20292  162   96  293    0",
	"etf_2stoned"				, "Too Stoned!"						, " 3796  900    0    0    3",
	"etf_2stoned2"				, "Too Stoned!"						, " 4574 1993    0    0    3",
	"etf_2xgenic"				, "2XGenic"							, "11044  148    0    0    0",
	"etf_4you"					, "4 YOU!"							, "  543   37    2    0    0",
	"etf_5_heroes"				, "5 Heroes"						, " 1427    2    2    0    0",
	"etf_allduel"				, "Allduel"							, " 3957  184    0    0    0",
	"etf_alps"					, "The Alps"						, " 4331  330   42    0    0",
	"etf_assaultforts"			, "Assault: Forts"					, " 6798  403  108    0    0",
	"etf_bam5"					, "Bam 5"							, " 4947  186    1   44    0",
	"etf_bases"					, "Bases"							, " 8354  458    5    0    0",
	"etf_border"				, "Cross the Border"				, " 1193   70   16    0    0",
	"etf_bringit"				, "Bring iT"						, " 2977  178  240    0   60", 
	"etf_canalzone"				, "Canalzone"						, "11513  513   14    0    0",
	"etf_castles"				, "King of the Castles"				, " 3247    0  261    0    0",
	"etf_cathedrals"			, "Cathedrals"						, "10771  710  419    0    0",
	"etf_chaos"					, "Chaos"							, " 4576  338   90    0    0",
	"etf_civsaw"				, "Civsaw"							, "  438   91    0    0    0",
	"etf_crossfire"				, "Crossfire"						, " 2347  125  212    0    0",
	"etf_crossfire2"			, "Crossfire 2"						, " 2321  211  172    0    0",
	"etf_cube"					, "Cube Runners"					, "  887    0   51    0    0",
	"etf_dam"					, "DAM" 							, " 3707    0    0    0    0",
	"etf_dissect"				, "Dissect"							, "17551  192    0    0    0",
	"etf_dmbox"					, "The Death Match Box"				, "  281    0    0    0    0",
	"etf_duel"					, "Duel ported to ETF"				, "   82    0    0    0    0",
	"etf_dungeonz"				, "Dungeonz"						, " 7881   14  316    0    0",
	"etf_engbat"				, "Engineer Battle"					, " 1028    0    0    0    0",
	"etf_engybat"				, "Engineer Battle"					, "  849    0   12   16    0",
	"etf_excel"					, "Excelsior"						, " 9294   72    0    0    0",
	"etf_forts"					, "Forts"							, "11534  805  212    0    0",
	"etf_frontline"				, "Frontline OvD"					, " 1873    0  142    0    0",
	"etf_gc2"					, "ginc's Castles"					, " 4640  468    0    0    0",
	"etf_genders"				, "Genders"							, " 2968    0    1    0    0",
	"etf_gotduck"				, "Got Duck?"						, " 1194    0    0    0    0",
	"etf_hardcore"				, "Hardcore"						, "18049  260    0    0    0",
	"etf_ìmpact"				, "impact"							, " 4962   36   32    0    0", 
	"etf_ìnert"					, "Inert Anti-Chase CTF"			, " 3442    0    0    0    0", 
	"etf_japanc"				, "Japanese castles"				, " 9224  214  248    0    0",
	"etf_lastresort"			, "Last Resort"						, "11695   96    0    0    0",
	"etf_mach"					, "Mach"							, " 4278  278    0    0    0",
	"etf_maxforts"				, "Maximum Forts"					, " 2300   24    0    0    0",
	"etf_mini32s"				, "Mini 32smooth!"					, " 1455    0    0    0    0",
	"etf_mini_heros"			, "Mini Heros"						, " 1788    0    2    0    0",
	"etf_mini_noheros"			, "Mini NoHeros"					, " 1087    2   18    0    0",
	"etf_monkey_s"				, "Monkey"							, " 4943    0    0    0    0",
	"etf_monkey_tech_preview"	, "Monkey (tech preview)"			, " 4107   32    0    0    0",
	"etf_muon"					, "Muon"							, "12590  318    0    0    0",
	"etf_murderduck_beta02"		, "Murderduck Beta0.2"				, " 2131    0    0    0    0",
	"etf_nevermore2a"			, "Nevermore2"						, "12525   62  428    0    0",
	"etf_nocturne"				, "Nocturne"						, " 7997  620   22    0    0",
	"etf_openfire"				, "Openfire"						, "13121  242    0    0    0",
	"etf_orbital"				, "Orbital"							, "18283  358   36    0    0",
	"etf_rock"					, "Rock"							, "20825  572    6    0    0",
	"etf_shi"					, "Shi"							    , " 5850    0    0    0    0",
	"etf_shoop"					, "Shoop"							, " 5538  168    0    0    0",
	"etf_silverfort"			, "Silver Fort"						, "32290 1846   56    0    0",
	
	// looks like there are 2 smartbases versions
	"etf_smartbases2"			, "Smart Bases 2"					, " 9738  134    0    0    0",
	"etf_smartbases2"			, "Smart Bases 2"					, " 9741  134    0    0    0",
	
	"etf_smooth"				, "Smooth"							, " 7499  181    0    0    0",
	"etf_sniperg"				, "Snipergrounds"					, " 1697   12   72    0    0",
	"etf_softcore"				, "The Softcore Hideout"			, " 9027   69   14    0    0",
	"etf_spamtech"				, "The Technology Of Spam"			, "11967  196    2    0    0", 
	"etf_spazball"				, "Spazball"						, " 5682    0   36    0    0",
	"etf_spiderx"				, "Spider Crossings"				, " 4939  443   78    0    0",
	"etf_spring_final"			, "Spring"							, " 6376  148   30    0    0",
	"etf_stag"					, "Stag"							, " 9163    0   82    0    0",
	"etf_stq_zigzag_b5"			, "ZigZag by StatusQ beta5"			, "  923  156   40    0    0",
	"etf_tbase"					, "Teleporter Base"					, " 4771  147   20    0    0",
	"etf_tf2k"					, "The Final 2 Kingdoms"			, " 7288   32   20    0    0",
	"etf_ultima"				, "Ultima"							, " 3982    2   56    0    0",
	"etf_well"					, "Oh Well"							, " 9658   98    0    0    0",
	"etf_well2k7"				, "Well2k7"							, " 2076    0    0    0    0",
	"etf_xpress"				, "Express"							, "  613    0    0    0    0",
	"etf_zigzag"				, "Zig Zag"							, "  737    0  234    0    0",
	"etf_zpillars_beta2"		, "ZPillars"						, " 2456  164    0    0    0",
	

	""							, ""						, ""
};

// public functions

void process_mapinfo( int signature )
{
	char temp[ VAR_ARRAY_ELEMENT_SIZE ];
	char temp2[ 10 ]; // should contain the value of an array element
	int i;
	int found;
	
	map_id = -1;
	map_description[ 0 ] = '\0';

	if ( signature ) {
		
		map[ 0 ]  = '\0';
		temp[ 0 ] = '\0';
		// compile id-string
		for ( i = 0; i < iar; i++ ) {
			sprintf( temp2, "%5.5s", array[ i ] ); 
			strcat( temp, temp2 );
		}
		
		// find id string
		i = 0;
		found = FALSE;
		while ( !found && MAP_INFOS[ i ].id[ 0 ] != '\0' ) {
			if ( 0 == strcmp( MAP_INFOS[ i ].id, temp ) ) {
				found = TRUE;
				map_id = i;
				strcpy( map, MAP_INFOS[ i ].mapname );
				strcpy( map_description, MAP_INFOS[ i ].mapdesc );
			}
			else {
				i++;
			}
		}
	}
	else {
		// mapname is known, get description
		//strcpy( map, array[ 0 ] );
		i = 0;
		found = FALSE;
		while ( !found && MAP_INFOS[ i ].id[ 0 ] != '\0' ) {
			if ( 0 == strcmp( MAP_INFOS[ i ].mapname, map ) ) {
				found = TRUE;
				map_id = i;
				strcpy( map_description, MAP_INFOS[ i ].mapdesc );
			}
			else {
				i++;
			}
		}
		if ( !found ) {
			if ( 0 == strncmp( map, "etf_", 4 ) && strlen( map ) > 4 ) {
				strcpy( map_description, map + 4 );
				map_description[ 0 ] = toupper( map_description[ 0 ] );
			}
			else {
				strcpy( map_description, map );
			}
		}	
	}
}
