#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "matching.h"
#include "agostats.h"
#include "flagtracking.h"
#include "maps.h"
#include "game.h"
#include "players.h"
#include "weapons.h"
#include "tools.h"

char linebuffer		[ LINE_BUFFER_SIZE ];
char patternbuffer	[ LINE_BUFFER_SIZE ];

char player		[ PLAYER_NAME_SIZE ]; // %p
char newname	[ PLAYER_NAME_SIZE ]; // %n
char killer		[ PLAYER_NAME_SIZE ]; // %k
char victim		[ PLAYER_NAME_SIZE ]; // %v
char location	[ LOCATION_SIZE    ]; // %l
char debug		[ DEBUG_SIZE       ]; // %e
char chat		[ CHAT_SIZE        ]; // %c
char team		[ TEAM_SIZE        ]; // %t
char flag		[ FLAG_SIZE        ]; // %f
char ammo		[ AMMO_SIZE        ]; // %a
char weapon		[ WEAPON_SIZE      ]; // %w
char action		[ ACTION_SIZE      ]; // %x
char building	[ BUILDING_SIZE    ]; // %b
char array		[ VAR_ARRAY_SIZE ][ VAR_ARRAY_ELEMENT_SIZE ]; // %r /* array to store multiple variables from a line"
int iar;                              // index that indicates element number in array
char map		[ MAP_NAME_SIZE ];
char color;
char optional   [ MAX_PATTERN_SIZE ];    // variable to store optional strings
long max_storage_length;  // depends on the storage variable: player, team, etc

int chatline;
int teamchatline;
int teamkill;
int suicide ;
int	team_nr;

int flag_nr;
int	player_nr;
int	killer_nr;
int	victim_nr;
int squad_nr;
int old_line_type;
int old_player_nr;
int	old_flag_nr;
int old_line_info1;
int	line_type;




t_pattern LOG_PATTERNS[]= {
#if !defined(SERVER_STATS)
	// pattern																// linetype	// Type of frag		// weapon type				// unused		// unused

	// agostats control patterns
	"as.date \\[%r-%r-%r\\]%i"														, LT_AGOSTATS, INFO_DATE, NO_INFO					, NO_INFO		, NO_INFO,
	"as.time \\[%r:%r\\]%i"															, LT_AGOSTATS, INFO_TIME, NO_INFO					, NO_INFO		, NO_INFO,

	// frags
	// todo: in debug mode print largest string length and number of items
	// assault rifle
	"%v received [^sally^7 ]%k's bullet spray.\x0a"								, LT_FRAG, T_KILL	, W_ASSAULT_RIFLE			, NO_INFO		, NO_INFO,	
	"%v was killed by [^sally^7 ]%k's resourceful auto rifle.\x0a"				, LT_FRAG, T_KILL	, W_ASSAULT_RIFLE			, NO_INFO		, NO_INFO,	
	"%v got filled with holes from [^sally^7 ]%k's auto rifle.\x0a"				, LT_FRAG, T_KILL	, W_ASSAULT_RIFLE			, NO_INFO		, NO_INFO,	
	// q3f
	"%v received [ally ]%k's bullet spray\x0a"									, LT_FRAG, T_KILL	, W_ASSAULT_RIFLE			, NO_INFO		, NO_INFO,	
	// q3f

	// autosentry
	"%v was caught with {his|her} pants down by [^sally^7 ]%k's autosentry.\x0a", LT_FRAG, T_KILL	, W_AUTOSENTRY_BULLET		, NO_INFO		, NO_INFO,
	"%v was caught with her skirt up by [^sally^7 ]%k's autosentry.\x0a"		, LT_FRAG, T_KILL	, W_AUTOSENTRY_BULLET		, NO_INFO		, NO_INFO,
	"%v felt the wrath of [^sally^7 ]%k's autosentry.\x0a"						, LT_FRAG, T_KILL	, W_AUTOSENTRY_BULLET		, NO_INFO		, NO_INFO,
	"%v was exploded along with [^sally^7 ]%k's autosentry.\x0a"				, LT_FRAG, T_KILL	, W_AUTOSENTRY_EXPLOSION	, NO_INFO		, NO_INFO,
	"%v was wrecked in [^sally^7 ]%k's autosentry explosion.\x0a"				, LT_FRAG, T_KILL	, W_AUTOSENTRY_EXPLOSION	, NO_INFO		, NO_INFO,
	"%v was a casualty statistic in [^sally^7 ]%k's autosentry mishap.\x0a"		, LT_FRAG, T_KILL	, W_AUTOSENTRY_EXPLOSION	, NO_INFO		, NO_INFO,
	"%v stood too close to [^sally^7 ]%k's autosentry when it blew up.\x0a"		, LT_FRAG, T_KILL	, W_AUTOSENTRY_EXPLOSION	, NO_INFO		, NO_INFO,
	"%v was blown up by [^sally^7 ]%k's autosentry rocket.\x0a"					, LT_FRAG, T_KILL	, W_AUTOSENTRY_ROCKET		, NO_INFO		, NO_INFO,
	"%v swallowed a rocket from [^sally^7 ]%k's autosentry.\x0a"				, LT_FRAG, T_KILL	, W_AUTOSENTRY_ROCKET		, NO_INFO		, NO_INFO,
	// q3f
	"%v swallowed a rocket from [ally ]%k's autosentry\x0a"						, LT_FRAG, T_KILL	, W_AUTOSENTRY_ROCKET		, NO_INFO		, NO_INFO,
	"%v felt the wrath of [ally ]%k's autosentry\x0a"							, LT_FRAG, T_KILL	, W_AUTOSENTRY_BULLET		, NO_INFO		, NO_INFO,
	"%v exploded along with [ally ]%k's autosentry\x0a"							, LT_FRAG, T_KILL	, W_AUTOSENTRY_EXPLOSION	, NO_INFO		, NO_INFO,
	// q3f

	// battleaxe
	"%v was chopped to bits by [^sally^7 ]%k.\x0a"								, LT_FRAG, T_KILL	, W_BATTLEAXE				, NO_INFO		, NO_INFO,
	"%v couldn't handle [^sally^7 ]%k's elite hacking skills.\x0a"				, LT_FRAG, T_KILL	, W_BATTLEAXE				, NO_INFO		, NO_INFO,
	"%v was neatly murdered by [^sally^7 ]%k.\x0a"								, LT_FRAG, T_KILL	, W_BATTLEAXE				, NO_INFO		, NO_INFO,
	"%v was bludgeoned by [^sally^7 ]%k's hatchet.\x0a"							, LT_FRAG, T_KILL	, W_BATTLEAXE				, NO_INFO		, NO_INFO,
	// q3f, in q3f it is called gauntlet, a straight copy of the quake3 gauntlet
	"%v was pummeled by [ally ]%k\x0a"											, LT_FRAG, T_KILL	, W_BATTLEAXE				, NO_INFO		, NO_INFO,
	// q3f

	// HE charge
	"%v crumbled into dust after being exposed to [^sally^7 ]%k's HE charge.\x0a", LT_FRAG, T_KILL	, W_HE_CHARGE				, NO_INFO		, NO_INFO,
	"%v was vaporized by [^sally^7 ]%k's HE charge explosion.\x0a"				, LT_FRAG, T_KILL	, W_HE_CHARGE				, NO_INFO		, NO_INFO,
	"%v learned the true power of [^sally^7 ]%k's HE charge.\x0a"				, LT_FRAG, T_KILL	, W_HE_CHARGE				, NO_INFO		, NO_INFO,
	// q3f
	"%v learned the true power of [ally ]%k's HE charge\x0a"					, LT_FRAG, T_KILL	, W_HE_CHARGE				, NO_INFO		, NO_INFO,
	// q3f

	// Cluster bomb
	"%v found {him|her}self on the wrong end of [^sally^7 ]%k's cluster bomb spam.\x0a", LT_FRAG, T_KILL	, W_CLUSTER_GRENADE	, NO_INFO		, NO_INFO,
	"%v couldn't hide from [^sally^7 ]%k's cluster bomb.\x0a"					, LT_FRAG, T_KILL	, W_CLUSTER_GRENADE			, NO_INFO		, NO_INFO,
	"%v was spammed by [^sally^7 ]%k's cluster bomb.\x0a"						, LT_FRAG, T_KILL	, W_CLUSTER_GRENADE			, NO_INFO		, NO_INFO,
	"%v ate [^sally^7 ]%k's cluster bomb shrapnel.\x0a"							, LT_FRAG, T_KILL	, W_CLUSTER_GRENADE			, NO_INFO		, NO_INFO,
	// q3f
	"%v found {him|her}self on the wrong end of [ally ]%k's cluster-bomb spam\x0a", LT_FRAG, T_KILL	, W_CLUSTER_GRENADE			, NO_INFO		, NO_INFO,
	// q3f
	
	// single shotgun
	"%v was able to avoid everything except [^sally^7 ]%k's single-barreled shotgun.\x0a", LT_FRAG, T_KILL, W_SINGLE_SHOTGUN	, NO_INFO		, NO_INFO,
	"%v was dishonored by [^sally^7 ]%k's single-barreled shotgun.\x0a"			, LT_FRAG, T_KILL	, W_SINGLE_SHOTGUN			, NO_INFO		, NO_INFO,
	"%v is picking [^sally^7 ]%k's buckshot out of {his|her} body.\x0a"			, LT_FRAG, T_KILL	, W_SINGLE_SHOTGUN			, NO_INFO		, NO_INFO,
	"%v was gunned down by [^sally^7 ]%k.\x0a"									, LT_FRAG, T_KILL	, W_SINGLE_SHOTGUN			, NO_INFO		, NO_INFO,

	// double shotgun
	"%v caught a mouthful of lead from [^sally^7 ]%k's shotgun.\x0a"			, LT_FRAG, T_KILL	, W_DOUBLE_SHOTGUN			, NO_INFO		, NO_INFO,
	"%v was shelled by [^sally^7 ]%k's shotgun.\x0a"							, LT_FRAG, T_KILL	, W_DOUBLE_SHOTGUN			, NO_INFO		, NO_INFO,
	"%v ate [^sally^7 ]%k's buckshot ball.\x0a"									, LT_FRAG, T_KILL	, W_DOUBLE_SHOTGUN			, NO_INFO		, NO_INFO,
	"%v 's torso is feeling drafty due to [^sally^7 ]%k's super shotgun.\x0a"	, LT_FRAG, T_KILL	, W_DOUBLE_SHOTGUN			, NO_INFO		, NO_INFO,

	// q3f shotgun, double or single is not specified
	"%v was gunned down by [ally ]%k\x0a"										, LT_FRAG, T_KILL	, W_SHOTGUN					, NO_INFO		, NO_INFO,
	// q3f
	
	
	// tranquilizer gun
	"%v is put to sleep by [^sally^7 ]%k.\x0a"									, LT_FRAG, T_KILL	, W_DARTGUN					, NO_INFO		, NO_INFO,
	"%v had {his|her} insomnia cured by [^sally^7 ]%k's tranquilizer dart.\x0a"	, LT_FRAG, T_KILL	, W_DARTGUN					, NO_INFO		, NO_INFO,
	"%v was knocked out by [^sally^7 ]%k.\x0a"									, LT_FRAG, T_KILL	, W_DARTGUN					, NO_INFO		, NO_INFO,

	// syringe
	"%v couldn't find the cure for [^sally^7 ]%k's mysterious illness.\x0a"		, LT_FRAG, T_KILL	, W_DISEASE					, NO_INFO		, NO_INFO,
	"%v succumbed to [^sally^7 ]%k's infection.\x0a"							, LT_FRAG, T_KILL	, W_DISEASE					, NO_INFO		, NO_INFO,
	"%v rotted to death due to [^sally^7 ]%k's virus.\x0a"						, LT_FRAG, T_KILL	, W_DISEASE					, NO_INFO		, NO_INFO,
	"%v decomposed rapidly as a result of [^sally^7 ]%k's disease.\x0a"			, LT_FRAG, T_KILL	, W_DISEASE					, NO_INFO		, NO_INFO,
	"%v didn't survive [^sally^7 ]%k's operation.\x0a"							, LT_FRAG, T_KILL	, W_SYRINGE					, NO_INFO		, NO_INFO,
	"%v was too weak to live through [^sally^7 ]%k's needle prick.\x0a"			, LT_FRAG, T_KILL	, W_SYRINGE					, NO_INFO		, NO_INFO,
	// q3f
	"%v succumbed to [ally ]%k's infection\x0a"									, LT_FRAG, T_KILL	, W_DISEASE					, NO_INFO		, NO_INFO,
	"%v didn't survive [ally ]%k's operation\x0a"								, LT_FRAG, T_KILL	, W_SYRINGE					, NO_INFO		, NO_INFO,
	// q3f

	// flamethrower
	"%v was burnt to a crisp by [^sally^7 ]%k.\x0a"								, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	"%v was caramelized by [^sally^7 ]%k's flame.\x0a"							, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	"%v was cooked to 'well done' by [^sally^7 ]%k's flame.\x0a"				, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	"%v was deep fried without batter by [^sally^7 ]%k's flame.\x0a"			, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	"%v was set ablaze by [^sally^7 ]%k's flame.\x0a"							, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	"%v stood too close to [^sally^7 ]%k's campfire.\x0a"						, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	"%v was roasted alive by [^sally^7 ]%k's flame.\x0a"						, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	"%v went out in a blaze of glory thanks to [^sally^7 ]%k.\x0a"				, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,
	// q3f
	"%v was burnt to a crisp by [ally ]%k\x0a"									, LT_FRAG, T_KILL	, W_FLAMETHROWER			, NO_INFO		, NO_INFO,	
	// q3f
	
	// flash grenade
	"%v was scorched by [^sally^7 ]%k's flash grenade.\x0a"						, LT_FRAG, T_KILL	, W_FLASH_GRENADE			, NO_INFO		, NO_INFO,
	"%v forgot to close {his|her} eyes when [^sally^7 ]%k's flash grenade exploded.\x0a", LT_FRAG, T_KILL, W_FLASH_GRENADE		, NO_INFO		, NO_INFO,
	"%v had {his|her} eyeballs melted by [^sally^7 ]%k's flashbang grenade.\x0a", LT_FRAG, T_KILL	, W_FLASH_GRENADE			, NO_INFO		, NO_INFO,
	// q3f
	"%v had {his|her} eyeballs melted [ally ]%k's flashbang grenade\x0a"		, LT_FRAG, T_KILL	, W_FLASH_GRENADE			, NO_INFO		, NO_INFO,
	// q3f

	// gas explosion
	"%v joined [^sally^7 ]%k's barbeque.\x0a"									, LT_FRAG, T_KILL	, W_GAS_EXPLOSION			, NO_INFO		, NO_INFO,
	"%v was transformed into a tasty morsel by [^sally^7 ]%k's barbeque.\x0a"	, LT_FRAG, T_KILL	, W_GAS_EXPLOSION			, NO_INFO		, NO_INFO,
	"%v took part in [^sally^7 ]%k's combustion experiment.\x0a"				, LT_FRAG, T_KILL	, W_GAS_EXPLOSION			, NO_INFO		, NO_INFO,
	// q3f	
	"%v joined [ally ]%k's barbecue\x0a"										, LT_FRAG, T_KILL	, W_GAS_EXPLOSION			, NO_INFO		, NO_INFO,
	// q3f

	// gas
	"%v liked [^sally^7 ]%k's pretty colors.\x0a"								, LT_FRAG, T_KILL	, W_GAS_GRENADE				, NO_INFO		, NO_INFO,
	"%v asphyxiated on [^sally^7 ]%k's gas.\x0a"								, LT_FRAG, T_KILL	, W_GAS_GRENADE				, NO_INFO		, NO_INFO,
	"%v had visions right before dying from [^sally^7 ]%k's hallucinogenic gas.\x0a", LT_FRAG, T_KILL, W_GAS_GRENADE			, NO_INFO		, NO_INFO,
	"%v suffocated on [^sally^7 ]%k's noxious gases.\x0a"						, LT_FRAG, T_KILL	, W_GAS_GRENADE				, NO_INFO		, NO_INFO,
	"%v expired from an overdose of [^sally^7 ]%k's hallucinogenic gas.\x0a"	, LT_FRAG, T_KILL	, W_GAS_GRENADE				, NO_INFO		, NO_INFO,
	// q3f
	"%v expired from an overdose of [ally ]%k's hallucinogenic gas\x0a"			, LT_FRAG, T_KILL	, W_GAS_GRENADE				, NO_INFO		, NO_INFO,
	// q3f

	// grenade launcher
	"%v ate [^sally^7 ]%k's grenade.\x0a"										, LT_FRAG, T_KILL	, W_RED_PIPE_DIRECT			, NO_INFO		, NO_INFO,
	"%v played with [^sally^7 ]%k's pineapple.\x0a"								, LT_FRAG, T_KILL	, W_RED_PIPE_DIRECT			, NO_INFO		, NO_INFO,
	"%v 's head was exploded by [^sally^7 ]%k's grenade.\x0a"					, LT_FRAG, T_KILL	, W_RED_PIPE_DIRECT			, NO_INFO		, NO_INFO,
	"%v received a pineapple enema from [^sally^7 ]%k.\x0a"						, LT_FRAG, T_KILL	, W_RED_PIPE_SPLASH			, NO_INFO		, NO_INFO,
	"%v didn't see [^sally^7 ]%k's grenade on the ground.\x0a"					, LT_FRAG, T_KILL	, W_RED_PIPE_SPLASH			, NO_INFO		, NO_INFO,
	"%v failed to avoid [^sally^7 ]%k's grenade.\x0a"							, LT_FRAG, T_KILL	, W_RED_PIPE_SPLASH			, NO_INFO		, NO_INFO,
	// q3f
	"%v ate [ally ]%k's grenade\x0a"											, LT_FRAG, T_KILL	, W_RED_PIPE_DIRECT			, NO_INFO		, NO_INFO,
	"%v failed to avoid [ally ]%k's grenade\x0a"								, LT_FRAG, T_KILL	, W_RED_PIPE_SPLASH			, NO_INFO		, NO_INFO,
	// q3f

	// handgrenade
	"%v mistook [^sally^7 ]%k's grenade for a pineapple.\x0a"					, LT_FRAG, T_KILL	, W_HAND_GRENADE			, NO_INFO		, NO_INFO,
	"%v caught [^sally^7 ]%k's grenade.\x0a"									, LT_FRAG, T_KILL	, W_HAND_GRENADE			, NO_INFO		, NO_INFO,
	"%v jumped onto [^sally^7 ]%k's grenade.\x0a"								, LT_FRAG, T_KILL	, W_HAND_GRENADE			, NO_INFO		, NO_INFO,
	// q3f
	"%v mistook [ally ]%k's grenade for a pineapple\x0a"						, LT_FRAG, T_KILL	, W_HAND_GRENADE			, NO_INFO		, NO_INFO,
	// q3f

	// knife
	"%v is knifed by [^sally^7 ]%k.\x0a"										, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	"%v was slashed and gutted by [^sally^7 ]%k's knife.\x0a"					, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	"%v was mugged by [^sally^7 ]%k.\x0a"										, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	"%v was carved open by [^sally^7 ]%k's knife.\x0a"							, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	"%v was skewered by [^sally^7 ]%k's knife.\x0a"								, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	"%v died from [^sally^7 ]%k's knife incision.\x0a"							, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	"%v had {his|her} organs fall out from [^sally^7 ]%k's knife incision.\x0a"	, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	"%v was stabbed to death by [^sally^7 ]%k.\x0a"								, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	// q3f
	"%v is knifed by [ally ]%k\x0a"												, LT_FRAG, T_KILL	, W_KNIFE					, NO_INFO		, NO_INFO,
	// q3f

	// minigun
	"%v was turned into humanslaw by [^sally^7 ]%k's minigun.\x0a"				, LT_FRAG, T_KILL	, W_MINIGUN					, NO_INFO		, NO_INFO,
	"%v was grated by [^sally^7 ]%k's minigun.\x0a"								, LT_FRAG, T_KILL	, W_MINIGUN					, NO_INFO		, NO_INFO,
	"%v was reduced to chunks by [^sally^7 ]%k's minigun.\x0a"					, LT_FRAG, T_KILL	, W_MINIGUN					, NO_INFO		, NO_INFO,
	"%v was transformed into a bloody pulp by [^sally^7 ]%k's minigun.\x0a"		, LT_FRAG, T_KILL	, W_MINIGUN					, NO_INFO		, NO_INFO,
	"%v was floored by a hail of lead from [^sally^7 ]%k's minigun.\x0a"		, LT_FRAG, T_KILL	, W_MINIGUN					, NO_INFO		, NO_INFO,
	"%v was cut down in {his|her} prime with [^sally^7 ]%k's minigun.\x0a"		, LT_FRAG, T_KILL	, W_MINIGUN					, NO_INFO		, NO_INFO,
	// q3f
	"%v was floored by a hail of lead from [ally ]%k's minigun\x0a"				, LT_FRAG, T_KILL	, W_MINIGUN					, NO_INFO		, NO_INFO,
	// q3f

	// nail grenade
	"%v was shredded by [^sally^7 ]%k's nail bomb.\x0a"							, LT_FRAG, T_KILL	, W_NAIL_GRENADE			, NO_INFO		, NO_INFO,
	"%v was thrashed by [^sally^7 ]%k's nail grenade.\x0a"						, LT_FRAG, T_KILL	, W_NAIL_GRENADE			, NO_INFO		, NO_INFO,
	// q3f
	"%v was shredded by [ally ]%k's nail bomb\x0a"								, LT_FRAG, T_KILL	, W_NAIL_GRENADE			, NO_INFO		, NO_INFO,
	// q3f

	// nailgun
	"%v didn't think [^sally^7 ]%k's nails would ever do enough damage to kill him.\x0a", LT_FRAG, T_KILL	, W_SMALL_NAILGUN	, NO_INFO		, NO_INFO,
	"%v was transfixed by [^sally^7 ]%k's nails.\x0a"							, LT_FRAG, T_KILL	, W_SMALL_NAILGUN			, NO_INFO		, NO_INFO,
	"%v was finished off by [^sally^7 ]%k's nailgun.\x0a"						, LT_FRAG, T_KILL	, W_SMALL_NAILGUN			, NO_INFO		, NO_INFO,
	"%v was nailed into place by [^sally^7 ]%k.\x0a"							, LT_FRAG, T_KILL	, W_SMALL_NAILGUN			, NO_INFO		, NO_INFO,
	"%v was nailed by [^sally^7 ]%k.\x0a"										, LT_FRAG, T_KILL	, W_SMALL_NAILGUN			, NO_INFO		, NO_INFO,
	// q3f nailgun, supernailgun or small nailgun is not specified
	"%v was nailed by [ally ]%k\x0a"											, LT_FRAG, T_KILL	, W_SMALL_NAILGUN			, NO_INFO		, NO_INFO,
	// q3f
	
	// napalm gren
	"%v found [^sally^7 ]%k's napalm grenade too hot to handle.\x0a"			, LT_FRAG, T_KILL	, W_NAPALM_GRENADE			, NO_INFO		, NO_INFO,
	"%v was immolated by [^sally^7 ]%k's napalm.\x0a"							, LT_FRAG, T_KILL	, W_NAPALM_GRENADE			, NO_INFO		, NO_INFO,
	"%v was burned alive by [^sally^7 ]%k's napalm.\x0a"						, LT_FRAG, T_KILL	, W_NAPALM_GRENADE			, NO_INFO		, NO_INFO,
	// q3f
	"%v found [ally ]%k's napalm grenade too hot to handle\x0a"					, LT_FRAG, T_KILL	, W_NAPALM_GRENADE			, NO_INFO		, NO_INFO,	
	// q3f

	// pulsegren
	"%v was pulsated by [^sally^7 ]%k's pulse grenade.\x0a"						, LT_FRAG, T_KILL	, W_PULSE_GRENADE			, NO_INFO		, NO_INFO,
	"%v was vibrated to death by [^sally^7 ]%k's pulse grenade.\x0a"			, LT_FRAG, T_KILL	, W_PULSE_GRENADE			, NO_INFO		, NO_INFO,
	"%v was crisped by [^sally^7 ]%k's pulse grenade.\x0a"						, LT_FRAG, T_KILL	, W_PULSE_GRENADE			, NO_INFO		, NO_INFO,
	"%v felt [^sally^7 ]%k's pulse.\x0a"										, LT_FRAG, T_KILL	, W_PULSE_GRENADE			, NO_INFO		, NO_INFO,
	// q3f
	"%v was crisped by [ally ]%k's pulse grenade\x0a"							, LT_FRAG, T_KILL	, W_PULSE_GRENADE			, NO_INFO		, NO_INFO,
	// q3f

	// pipe launcher
	"%v thought it was safe to cross [^sally^7 ]%k's pipe trap.\x0a"			, LT_FRAG, T_KILL	, W_YELLOW_PIPE				, NO_INFO		, NO_INFO,
	"%v tried to eat [^sally^7 ]%k's banana.\x0a"								, LT_FRAG, T_KILL	, W_YELLOW_PIPE				, NO_INFO		, NO_INFO,
	"%v took a pipebomb suppository from [^sally^7 ]%k.\x0a"					, LT_FRAG, T_KILL	, W_YELLOW_PIPE				, NO_INFO		, NO_INFO,
	"%v swallowed [^sally^7 ]%k's pipe.\x0a"									, LT_FRAG, T_KILL	, W_YELLOW_PIPE				, NO_INFO		, NO_INFO,
	// q3f
	"%v swallowed [ally ]%k's pipe\x0a"											, LT_FRAG, T_KILL	, W_YELLOW_PIPE				, NO_INFO		, NO_INFO,
	// q3f

	// railgun
	"%v was railed by [^sally^7 ]%k.\x0a"										, LT_FRAG, T_KILL	, W_RAILGUN					, NO_INFO		, NO_INFO,
	"%v was impaled by [^sally^7 ]%k's rail.\x0a"								, LT_FRAG, T_KILL	, W_RAILGUN					, NO_INFO		, NO_INFO,
	"%v was perforated by [^sally^7 ]%k.\x0a"									, LT_FRAG, T_KILL	, W_RAILGUN					, NO_INFO		, NO_INFO,
	// q3f
	"%v was railed by [ally ]%k\x0a"											, LT_FRAG, T_KILL	, W_RAILGUN					, NO_INFO		, NO_INFO,
	// q3f

	// sniper rifle body
	"%v is sniped by [^sally^7 ]%k.\x0a"										, LT_FRAG, T_KILL	, W_SNIPERRIFLE_BODY		, NO_INFO		, NO_INFO,
	"%v was given an extra orifice by [^sally^7 ]%k's sniper rifle.\x0a"		, LT_FRAG, T_KILL	, W_SNIPERRIFLE_BODY		, NO_INFO		, NO_INFO,
	"%v was shot in the liver by [^sally^7 ]%k.\x0a"							, LT_FRAG, T_KILL	, W_SNIPERRIFLE_BODY		, NO_INFO		, NO_INFO,
	"%v was picked off by [^sally^7 ]%k.\x0a"									, LT_FRAG, T_KILL	, W_SNIPERRIFLE_BODY		, NO_INFO		, NO_INFO,
	"%v had {his|her} organs shot out by [^sally^7 ]%k.\x0a"					, LT_FRAG, T_KILL	, W_SNIPERRIFLE_BODY		, NO_INFO		, NO_INFO,
	// q3f
	"%v is sniped by [ally ]%k\x0a"												, LT_FRAG, T_KILL	, W_SNIPERRIFLE_BODY		, NO_INFO		, NO_INFO,
	// q3f
	
	// sniper rifle head
	"%v gets a bullet between the eyes from [^sally^7 ]%k.\x0a"					, LT_FRAG, T_KILL	, W_SNIPERRIFLE_HEAD		, NO_INFO		, NO_INFO,
	"%v didn't see [^sally^7 ]%k's large laser spot on {his|her} forehead.\x0a"	, LT_FRAG, T_KILL	, W_SNIPERRIFLE_HEAD		, NO_INFO		, NO_INFO,
	"%v had {his|her} block knocked off by [^sally^7 ]%k's sniper rifle.\x0a"	, LT_FRAG, T_KILL	, W_SNIPERRIFLE_HEAD		, NO_INFO		, NO_INFO,
	"%v lost {his|her} head in [^sally^7 ]%k's crosshair.\x0a"					, LT_FRAG, T_KILL	, W_SNIPERRIFLE_HEAD		, NO_INFO		, NO_INFO,
	"%v had {his|her} head taken off by [^sally^7 ]%k.\x0a"						, LT_FRAG, T_KILL	, W_SNIPERRIFLE_HEAD		, NO_INFO		, NO_INFO,
	// q3f
	"%v gets a bullet between the eyes from [ally ]%k\x0a"						, LT_FRAG, T_KILL	, W_SNIPERRIFLE_HEAD		, NO_INFO		, NO_INFO,
	// q3f

	// sniper rifle legs
	"%v gets {his|her} legs blown off by [^sally^7 ]%k.\x0a"					, LT_FRAG, T_KILL	, W_SNIPERRIFLE_LEGS		, NO_INFO		, NO_INFO,
	"%v had {his|her} legs amputated by [^sally^7 ]%k's sniper round.\x0a"		, LT_FRAG, T_KILL	, W_SNIPERRIFLE_LEGS		, NO_INFO		, NO_INFO,
	"%v was kneecapped by [^sally^7 ]%k.\x0a"									, LT_FRAG, T_KILL	, W_SNIPERRIFLE_LEGS		, NO_INFO		, NO_INFO,
	// q3f
	"%v gets {his|her} legs blown off by [ally ]%k\x0a"							, LT_FRAG, T_KILL	, W_SNIPERRIFLE_LEGS		, NO_INFO		, NO_INFO,
	// q3f

	// rocket launcher direct
	"%v ate [^sally^7 ]%k's rocket.\x0a"										, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_DIRECT	, NO_INFO		, NO_INFO,
	"%v straddled [^sally^7 ]%k's rocket.\x0a"									, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_DIRECT	, NO_INFO		, NO_INFO,
	"%v was pulverized by [^sally^7 ]%k's rocket.\x0a"							, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_DIRECT	, NO_INFO		, NO_INFO,
	"%v tried to get a better look at [^sally^7 ]%k's rocket.\x0a"				, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_DIRECT	, NO_INFO		, NO_INFO,
	"%v was reamed by [^sally^7 ]%k's rocket.\x0a"								, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_DIRECT	, NO_INFO		, NO_INFO,
	// q3f
	"%v ate [ally ]%k's rocket\x0a"												, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_DIRECT	, NO_INFO		, NO_INFO,
	// q3f
	// rocket launcher blast
	"%v almost dodged [^sally^7 ]%k's rocket.\x0a"								, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_SPLASH 	, NO_INFO		, NO_INFO,
	"%v couldn't escape [^sally^7 ]%k's rocket.\x0a"							, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_SPLASH 	, NO_INFO		, NO_INFO,
	// q3f
	"%v almost dodged [ally ]%k's rocket\x0a"									, LT_FRAG, T_KILL	, W_ROCKET_LAUNCHER_SPLASH 	, NO_INFO		, NO_INFO,
	// q3f 

	// super nailgun
	"%v had {his|her} body penetrated by [^sally^7 ]%k's super nailgun.\x0a"	, LT_FRAG, T_KILL	, W_SUPER_NAILGUN			, NO_INFO		, NO_INFO,
	"%v became saturated with nails from [^sally^7 ]%k's super nailgun.\x0a"	, LT_FRAG, T_KILL	, W_SUPER_NAILGUN			, NO_INFO		, NO_INFO,
	"%v had a hole punched through {his|her} heart by [^sally^7 ]%k's super nailgun.\x0a", LT_FRAG, T_KILL, W_SUPER_NAILGUN		, NO_INFO		, NO_INFO,
	"%v got nailed hard by [^sally^7 ]%k.\x0a"									, LT_FRAG, T_KILL	, W_SUPER_NAILGUN			, NO_INFO		, NO_INFO,
	"%v was impaled on [^sally^7 ]%k's nails.\x0a"								, LT_FRAG, T_KILL	, W_SUPER_NAILGUN			, NO_INFO		, NO_INFO,

	// supply station explosion
	"%v didn't know that [^sally^7 ]%k's supply station didn't accept pennies.\x0a", LT_FRAG, T_KILL, W_SUPPLY_STATION_EXPLOSION, NO_INFO		, NO_INFO,
	"%v had their credit card declined by [^sally^7 ]%k's supply station.\x0a"	, LT_FRAG, T_KILL	, W_SUPPLY_STATION_EXPLOSION, NO_INFO		, NO_INFO,
	"%v lost an argument with [^sally^7 ]%k's supply station.\x0a"				, LT_FRAG, T_KILL	, W_SUPPLY_STATION_EXPLOSION, NO_INFO		, NO_INFO,
	// q3f
	"%v lost an argument with [ally ]%k's supply station\x0a"					, LT_FRAG, T_KILL	, W_SUPPLY_STATION_EXPLOSION, NO_INFO		, NO_INFO,
	// q3f

	// telefrag
	"%v tried to invade [^sally^7 ]%k's personal space.\x0a"					, LT_FRAG, T_KILL	, W_TELEFRAG				, NO_INFO		, NO_INFO,

	// wrench
	"%v got a wrench shaped dent from [^sally^7 ]%k.\x0a"						, LT_FRAG, T_KILL	, W_WRENCH					, NO_INFO		, NO_INFO,
	"%v had his nuts tightened by [^sally^7 ]%k's wrench.\x0a"					, LT_FRAG, T_KILL	, W_WRENCH					, NO_INFO		, NO_INFO,
	"%v had her nipples twisted off by [^sally^7 ]%k's wrench.\x0a"				, LT_FRAG, T_KILL	, W_WRENCH					, NO_INFO		, NO_INFO,

	// this can also be a teamkill
	"[print ]%v's supply station has been destroyed by %k^7!\x0a"				, LT_FRAG, T_KILLED_SUPPLYSTATION, W_UNKNOWN	, NO_INFO		, NO_INFO,
	// this can also be a teamkill
	"[print ]%v's autosentry has been destroyed by %k^7!\x0a"					, LT_FRAG, T_KILLED_AUTOSENTRY, W_UNKNOWN		, NO_INFO		, NO_INFO,
	
	// SUICIDES

	// autosentry bullet
	"%v looked down the barrel of {his|her} own autosentry.\x0a"			, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_BULLET			, NO_INFO		, NO_INFO,
	"%v stood on the wrong side of {his|her} autosentry.\x0a"				, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_BULLET			, NO_INFO		, NO_INFO,
	"%v picked a bad time to polish the barrel of {his|her} autosentry.\x0a", LT_FRAG, T_SUICIDE, W_AUTOSENTRY_BULLET			, NO_INFO		, NO_INFO,
	
	// autosentry explosion
	"%v couldn't repair {his|her} autosentry in time.\x0a"					, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_EXPLOSION		, NO_INFO		, NO_INFO,
	"%v forgot to wear {his|her} safety mask.\x0a"							, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_EXPLOSION		, NO_INFO		, NO_INFO,
	// todo: test
	"%v learned just how heavy {his|her} autosentry really is.\x0a"			, LT_FRAG, T_SUICIDE, W_CRUSHED_BY_SENTRY			, NO_INFO		, NO_INFO,
	"%v exploded along with {his|her} autosentry.\x0a"						, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_EXPLOSION		, NO_INFO		, NO_INFO,

	// autosentry rocket
	"%v tried to argue with {his|her} autosentry's rocket.\x0a"				, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_ROCKET			, NO_INFO		, NO_INFO,
	"%v was blown up by {his|her} own sentry gun.\x0a"						, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_ROCKET			, NO_INFO		, NO_INFO,
	"%v got in the way of {his|her} autosentry's rocket.\x0a"				, LT_FRAG, T_SUICIDE, W_AUTOSENTRY_ROCKET			, NO_INFO		, NO_INFO,

	// heavy explosive charge
	"%v disintegrated {him|her}self with {his|her} own HE charge.\x0a"		, LT_FRAG, T_SUICIDE, W_HE_CHARGE					, NO_INFO		, NO_INFO,
	"%v wanted to see what happened when his HE charge went off, and learned a valuable lesson.\x0a", LT_FRAG, T_SUICIDE, W_HE_CHARGE, NO_INFO	, NO_INFO,
	"%v paints the town red.\x0a"											, LT_FRAG, T_SUICIDE, W_HE_CHARGE					, NO_INFO		, NO_INFO,
	// q3f
	"%v underestimated the power of the HE charge.\x0a"						, LT_FRAG, T_SUICIDE, W_HE_CHARGE					, NO_INFO		, NO_INFO,
	// q3f

	// cluster bomb
	"%v spammed {him|her}self with {his|her} cluster bomb.\x0a"				, LT_FRAG, T_SUICIDE, W_CLUSTER_GRENADE				, NO_INFO		, NO_INFO,

	// flamethrower
	// todo: all these messages are not working, always says killed himself
	"%v killed {him|her}self.\x0a"											, LT_FRAG, T_SUICIDE, W_FLAMETHROWER				, NO_INFO		, NO_INFO,
	"%v burned {him|her}self.\x0a"											, LT_FRAG, T_SUICIDE, W_FLAMETHROWER				, NO_INFO		, NO_INFO,
	"%v incinerated {him|her}self.\x0a"										, LT_FRAG, T_SUICIDE, W_FLAMETHROWER				, NO_INFO		, NO_INFO,
	"%v couldn't take the heat.\x0a"										, LT_FRAG, T_SUICIDE, W_FLAMETHROWER				, NO_INFO		, NO_INFO,
	"%v burned through {his|her} own flame.\x0a"							, LT_FRAG, T_SUICIDE, W_FLAMETHROWER				, NO_INFO		, NO_INFO,

	// flashbang
	"%v burned out {his|her} own eye sockets.\x0a"							, LT_FRAG, T_SUICIDE, W_FLASH_GRENADE				, NO_INFO		, NO_INFO,
	"%v was dazzled by {his|her} own grenade.\x0a"							, LT_FRAG, T_SUICIDE, W_FLASH_GRENADE				, NO_INFO		, NO_INFO,
	"%v was blinded by {his|her} own light.\x0a"							, LT_FRAG, T_SUICIDE, W_FLASH_GRENADE				, NO_INFO		, NO_INFO,

	// gas explosion
	"%v got trapped in {his|her} own inferno.\x0a"							, LT_FRAG, T_SUICIDE, W_GAS_EXPLOSION				, NO_INFO		, NO_INFO,

	// gas grenade
	"%v discombobulated {his|her} senses with a lethal intake of his own gas.\x0a", LT_FRAG, T_SUICIDE, W_GAS_GRENADE			, NO_INFO		, NO_INFO,
	"%v succumbed to {his|her} own gas grenade.\x0a"						, LT_FRAG, T_SUICIDE, W_GAS_GRENADE					, NO_INFO		, NO_INFO,

	// grenade launcher splash
	"%v launched {him|her}self a present.\x0a"								, LT_FRAG, T_SUICIDE, W_RED_PIPE_SPLASH				, NO_INFO		, NO_INFO,
	"%v fed {him|her}self some pineapples.\x0a"								, LT_FRAG, T_SUICIDE, W_RED_PIPE_SPLASH				, NO_INFO		, NO_INFO,
	"%v stepped on {his|her} own spam.\x0a"									, LT_FRAG, T_SUICIDE, W_RED_PIPE_SPLASH				, NO_INFO		, NO_INFO,
	"%v tripped on {his|her} own grenade.\x0a"								, LT_FRAG, T_SUICIDE, W_RED_PIPE_SPLASH				, NO_INFO		, NO_INFO,

	// handgrenade
	"%v and {his|her} grenade had a violent divorce.\x0a"					, LT_FRAG, T_SUICIDE, W_HAND_GRENADE				, NO_INFO		, NO_INFO,
	"%v needed to see {his|her} grenade one last time.\x0a"					, LT_FRAG, T_SUICIDE, W_HAND_GRENADE				, NO_INFO		, NO_INFO,
	"%v couldn't avoid {his|her} own grenade.\x0a"							, LT_FRAG, T_SUICIDE, W_HAND_GRENADE				, NO_INFO		, NO_INFO,
	"%v miscalculated {his|her} grenade toss just a little bit.\x0a"		, LT_FRAG, T_SUICIDE, W_HAND_GRENADE				, NO_INFO		, NO_INFO,
	"%v swallowed {his|her} own grenade.\x0a"								, LT_FRAG, T_SUICIDE, W_HAND_GRENADE				, NO_INFO		, NO_INFO,
	"%v played hide & seek with {his|her} own grenade, and lost.\x0a"		, LT_FRAG, T_SUICIDE, W_HAND_GRENADE				, NO_INFO		, NO_INFO,
	"%v threw the pin.\x0a"													, LT_FRAG, T_SUICIDE, W_HAND_GRENADE				, NO_INFO		, NO_INFO,

	// nail bomb
	"%v learned how {his|her} nail bomb worked.\x0a"						, LT_FRAG, T_SUICIDE, W_NAIL_GRENADE				, NO_INFO		, NO_INFO,
	"%v couldn't avoid {his|her} own nail bomb.\x0a"						, LT_FRAG, T_SUICIDE, W_NAIL_GRENADE				, NO_INFO		, NO_INFO,
	"%v didn't take notice of {his|her} rotating disc of death.\x0a"		, LT_FRAG, T_SUICIDE, W_NAIL_GRENADE				, NO_INFO		, NO_INFO,
	"%v was hypnotized by {his|her} spinning nail bomb.\x0a"				, LT_FRAG, T_SUICIDE, W_NAIL_GRENADE				, NO_INFO		, NO_INFO,

	// napalm grenade
	"%v toasted {him|her}self with {his|her} napalm grenade.\x0a"			, LT_FRAG, T_SUICIDE, W_NAPALM_GRENADE				, NO_INFO		, NO_INFO,
	"%v walked into {his|her} own napalm spray.\x0a"						, LT_FRAG, T_SUICIDE, W_NAPALM_GRENADE				, NO_INFO		, NO_INFO,
	"%v started {his|her} day with a large serving of napalm.\x0a"			, LT_FRAG, T_SUICIDE, W_NAPALM_GRENADE				, NO_INFO		, NO_INFO,

	// pipe grenade
	"%v tripped over {his|her} own pipe trap.\x0a"							, LT_FRAG, T_SUICIDE, W_YELLOW_PIPE					, NO_INFO		, NO_INFO,
	"%v caught {him|her}self with {his|her} own pipe trap.\x0a"				, LT_FRAG, T_SUICIDE, W_YELLOW_PIPE					, NO_INFO		, NO_INFO,
	"%v detonated {him|her}self.\x0a"										, LT_FRAG, T_SUICIDE, W_YELLOW_PIPE					, NO_INFO		, NO_INFO,
	"%v mistook {him|her}self for an enemy.\x0a"							, LT_FRAG, T_SUICIDE, W_YELLOW_PIPE					, NO_INFO		, NO_INFO,

	// pulse grenade
	"%v was shocked by {his|her} pulse grenade.\x0a"						, LT_FRAG, T_SUICIDE, W_PULSE_GRENADE				, NO_INFO		, NO_INFO,
	"%v rattles {his|her} bones.\x0a"										, LT_FRAG, T_SUICIDE, W_PULSE_GRENADE				, NO_INFO		, NO_INFO,
	"%v fulgurated {him|her}self with {his|her} own pulse grenade.\x0a"		, LT_FRAG, T_SUICIDE, W_PULSE_GRENADE				, NO_INFO		, NO_INFO,
	"%v exploded {his|her} own ammunition.\x0a"								, LT_FRAG, T_SUICIDE, W_PULSE_GRENADE				, NO_INFO		, NO_INFO,
	
	// rocket launcher splash
	"%v missed with {his|her} rocket.\x0a"									, LT_FRAG, T_SUICIDE, W_ROCKET_LAUNCHER_SPLASH 		, NO_INFO		, NO_INFO,
	"%v blew {him|her}self up.\x0a"											, LT_FRAG, T_SUICIDE, W_ROCKET_LAUNCHER_SPLASH 		, NO_INFO		, NO_INFO,
	"%v became disgusted with {his|her} own rocket aim.\x0a"				, LT_FRAG, T_SUICIDE, W_ROCKET_LAUNCHER_SPLASH 		, NO_INFO		, NO_INFO,

	// supply station
	"%v used {his|her} supply station as a suicide machine.\x0a"			, LT_FRAG, T_SUICIDE, W_SUPPLY_STATION_EXPLOSION	, NO_INFO		, NO_INFO,
	"%v fell out with {his|her} supply station.\x0a"						, LT_FRAG, T_SUICIDE, W_SUPPLY_STATION_EXPLOSION	, NO_INFO		, NO_INFO,

	// SUICIDES by environment
	// console
	"%v suicides.\x0a"														, LT_FRAG, T_SUICIDE, W_CONSOLE						, NO_INFO		, NO_INFO,
	"%v quits life.\x0a"													, LT_FRAG, T_SUICIDE, W_CONSOLE						, NO_INFO		, NO_INFO,
	
	// lava
	"%v did a back flip into the lava.\x0a"									, LT_FRAG, T_SUICIDE, W_LAVA						, NO_INFO		, NO_INFO,
	"%v found out molten rock is hot.\x0a"									, LT_FRAG, T_SUICIDE, W_LAVA						, NO_INFO		, NO_INFO,
	"%v sacrificed {him|her}self to the lava god.\x0a"						, LT_FRAG, T_SUICIDE, W_LAVA						, NO_INFO		, NO_INFO,
	"%v did a cannonball into the lava.\x0a"								, LT_FRAG, T_SUICIDE, W_LAVA						, NO_INFO		, NO_INFO,
	// q3f
	"%v does a back flip into the lava.\x0a"								, LT_FRAG, T_SUICIDE, W_LAVA						, NO_INFO		, NO_INFO,
	// q3f
	
	// Slime
	// q3f
	"%v melted.\x0a"														, LT_FRAG, T_SUICIDE, W_SLIME						, NO_INFO		, NO_INFO,
	// a3f

	// falling
	"%v cratered.\x0a"														, LT_FRAG, T_SUICIDE, W_FALLING						, NO_INFO		, NO_INFO,
	"%v broke {his|her} legs.\x0a"											, LT_FRAG, T_SUICIDE, W_FALLING						, NO_INFO		, NO_INFO,
	"%v felt the wrath of gravity.\x0a"										, LT_FRAG, T_SUICIDE, W_FALLING						, NO_INFO		, NO_INFO,
	"%v found out Newton was right.\x0a"									, LT_FRAG, T_SUICIDE, W_FALLING						, NO_INFO		, NO_INFO,
	"%v tried to fly.\x0a"													, LT_FRAG, T_SUICIDE, W_FALLING						, NO_INFO		, NO_INFO,
	
	// water
	"%v thought {he|she} could evolve gills.\x0a"							, LT_FRAG, T_SUICIDE, W_WATER						, NO_INFO		, NO_INFO,
	"%v sank like a rock.\x0a"												, LT_FRAG, T_SUICIDE, W_WATER						, NO_INFO		, NO_INFO,
	"%v couldn't find the sunken treasure.\x0a"								, LT_FRAG, T_SUICIDE, W_WATER						, NO_INFO		, NO_INFO,
	"%v sleeps in Davy Jones' locker. Yarrrrrr.\x0a"						, LT_FRAG, T_SUICIDE, W_WATER						, NO_INFO		, NO_INFO,
	"%v tried to find Nemo.\x0a"											, LT_FRAG, T_SUICIDE, W_WATER						, NO_INFO		, NO_INFO,
	
	// crushed
	"%v was piledriven.\x0a"												, LT_FRAG, T_SUICIDE, W_CRUSHED						, NO_INFO		, NO_INFO,
	"%v forgot to wear {his|her} helmet.\x0a"								, LT_FRAG, T_SUICIDE, W_CRUSHED						, NO_INFO		, NO_INFO,
	"%v couldn't hold as much as Atlas.\x0a"								, LT_FRAG, T_SUICIDE, W_CRUSHED						, NO_INFO		, NO_INFO,
	"%v was squished.\x0a"													, LT_FRAG, T_SUICIDE, W_CRUSHED						, NO_INFO		, NO_INFO,
	"%v was pressed flat.\x0a"												, LT_FRAG, T_SUICIDE, W_CRUSHED						, NO_INFO		, NO_INFO,

	// wrong place
	"%v died.\x0a"															, LT_FRAG, T_SUICIDE, W_WRONG_PLACE					, NO_INFO		, NO_INFO,
	"%v was in the wrong place.\x0a"										, LT_FRAG, T_SUICIDE, W_WRONG_PLACE					, NO_INFO		, NO_INFO,

	// MATCHLOG Patterns
	"kill MOD_%w \"%v\" [^sally^7 ]\"%k\"\x0a"								, LT_FRAG		, T_KILL			, NO_INFO	, INFO_MATCHLOG	, NO_INFO,
	"suicide MOD_%w %v\x0a"													, LT_FRAG		, T_SUICIDE			, NO_INFO	, INFO_MATCHLOG	, NO_INFO,
	"chat %p^7\x19: %c\x0a"													, LT_CHAT		, NO_INFO			, NO_INFO	, NO_INFO		, NO_INFO,
	"matchlog start\x0a"													, LT_STATUS_CHANGE,	GS_PRE_MATCH	, NO_INFO	, NO_INFO		, NO_INFO,
	"info map %r %r\x0a"													, LT_MAPINFO	, INFO_MAPNAME		, NO_INFO	, NO_INFO		, NO_INFO,
	"info gameindex %s\x0a"													, LT_INFO		, INFO_GAMEINDEX	, NO_INFO	, NO_INFO		, NO_INFO,
	"info description %s\x0a"												, LT_INFO		, INFO_DESCRIPTION	, NO_INFO	, NO_INFO		, NO_INFO,
	"info date %s\x0a"														, LT_INFO		, INFO_DATE			, NO_INFO	, NO_INFO		, NO_INFO,
	"info time %s\x0a"														, LT_INFO		, INFO_TIME			, NO_INFO	, NO_INFO		, NO_INFO,
	//


	// OTHER stats
	// todo:
	"[print ]%v ^7has destroyed {his|her} supply station.\x0a"					, LT_IGNORE		, T_DESTROYED_SUPPLYSTATION	, W_UNKNOWN		, NO_INFO	, NO_INFO,
	"[print ]%v ^7has destroyed {his|her} autosentry.\x0a"						, LT_IGNORE		, T_DESTROYED_AUTOSENTRY	, W_UNKNOWN		, NO_INFO	, NO_INFO,
	// sit on grenade
	"{print |^7}No, %p, you're supposed to THROW the grenade!\x0a"				, LT_THROW_PIN	, NO_INFO					, NO_INFO		, NO_INFO	, NO_INFO,
	"{print |^7}Oh snap, %p, you forgot to throw your grenade!\x0a"				, LT_THROW_PIN	, NO_INFO					, NO_INFO		, NO_INFO	, NO_INFO,
	"[print ]%p, perhaps you should throw that grenade.\x0a"					, LT_THROW_PIN	, NO_INFO					, NO_INFO		, NO_INFO	, NO_INFO,
	// q3f
	"No, %p, you're supposed to THROW the grenade!\x0a"							, LT_THROW_PIN	, NO_INFO					, NO_INFO		, NO_INFO	, NO_INFO,
	// q3f

	// Pre-match mode - reset statistics to filter out pre-match crap

	// todo: check print for matchlog
	"Server has been running for %i hours.\x0a"									,LT_STATUS_CHANGE,	GS_PRE_MATCH			, NO_INFO		, NO_INFO	, NO_INFO,
	"[print ]%i^7, please ready up...\x0a"										,LT_STATUS_CHANGE,	GS_PRE_MATCH			, NO_INFO		, NO_INFO	, NO_INFO,
	"^7Waiting for %i to ready up...\x0a"										,LT_STATUS_CHANGE,	GS_PRE_MATCH			, NO_INFO		, NO_INFO	, NO_INFO,
	"^7You are ready.\x0a"														,LT_STATUS_CHANGE,	GS_PRE_MATCH			, NO_INFO		, NO_INFO	, NO_INFO,
	"^7Ceasefire off\x0a"	/* todo: ceasefire halverwege game? */				,LT_STATUS_CHANGE,	GS_PRE_MATCH			, NO_INFO		, NO_INFO	, NO_INFO,

	// 1 FLAG CTF objective patterns
	// there is no message for flagdrop
	"{print |^7}The flag has returned.\x0a"				, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED		, NO_INFO			, GT_CAPTURE_THE_FLAG + GTM_ONE_FLAG, NO_INFO,
	"[print ]%p has captured the flag in the %t base!\x0a", LT_OBJECTIVE	, O_CTF_FLAGCAPTURE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ONE_FLAG, NO_INFO,
	"[print ]%p has taken the flag\x0a"					, LT_OBJECTIVE		, O_CTF_FLAGTAKE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ONE_FLAG, NO_INFO,
	"[print ]%p of the %t team has taken the flag\x0a"	, LT_OBJECTIVE		, O_CTF_FLAGTAKE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ONE_FLAG, NO_INFO,
	"[print ]%p of the %t team has nabbed Yorick!\x0a"	, LT_OBJECTIVE		, O_CTF_FLAGTAKE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ONE_FLAG, NO_INFO,
	"[print ]%p of the %t team has taken the flag\x0a"	, LT_OBJECTIVE		, O_CTF_FLAGTAKE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ONE_FLAG, NO_INFO,
	"[print ]%p took the flag\x0a"						, LT_OBJECTIVE		, O_CTF_FLAGTAKE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ONE_FLAG, NO_INFO,

	// 2 FLAG CTF
	"[print ]%p has TAKEN the %f flag!\x0a"				, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
	"[print ]%p took the %f flag!\x0a"					, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
	"[print ]%p nabbed the %f flag!\x0a"				, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG				, NO_INFO,
	"[print ]%p nabbed the %f ^7flag!\x0a"				, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG				, NO_INFO,
	"[print ]%p grabbed the %f flag!\x0a"				, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG				, NO_INFO,
	"[print ]%p took the %f Cross\x0a"					, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG				, NO_INFO,
	// q3f		
	"%p has taken the %f ^7flag!\x0a"										, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG				, NO_INFO,	
	// q3f

	// Captures
	"[print ]%p has CAPTURED the %f flag!\x0a"			, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"[print ]%p CAPTURED the %f flag!\x0a"				, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"[print ]%p CAPTURED the %f ^7flag!\x0a"			, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG			, NO_INFO,
	// q3f
	"%p captured the %f ^7flag!\x0a"					, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG			, NO_INFO,	
	// q3f
		
	// Drops
	"[print ]%p has DROPPED the %f flag!\x0a"			, LT_OBJECTIVE		, O_CTF_FLAGDROP   , EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,

	// Returns
	"{print |^7}The %f flag has returned.\x0a"			, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"{print |^7}the %f flag has returned\x0a"			, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"{print |^7}The %f ^7flag returned to base!\x0a"	, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	// q3f
	"the %f flag has returned !\x0a"					, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"the %f flag has returned\x0a"						, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"The %f flag has returned.\x0a"						, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"The %f flag returned to base!\x0a"					, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"The %f ^7flag returned to base!\x0a"				, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,
	"The %f ^7flag returned home.\x0a"					, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED, NO_INFO		, GT_CAPTURE_THE_FLAG			, NO_INFO,	
	// q3f

//	// MULTIPLE CTF Objective patterns // todo: some of these patterns may occur in more gametypes
	"%p scores 1 bonus point for fragging an enemy while capturing the flag!\x0a"	, LT_OBJECTIVE		, O_ACTF_FRAG_CAP_1				, NO_INFO		, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
	"%p scores %e bonus points for fragging enemies while capturing the flag!\x0a"	, LT_OBJECTIVE		, O_ACTF_FRAG_CAP_MULTI			, NO_INFO		, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
    "%p killed the %t team's flag carrier!\x0a"/* todo:wrong team in 1flag ??*/	, LT_OBJECTIVE		, O_ACTF_FRAG_ENEMY_CARRIER		, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
	"%p teamkilled the %t team's flag carrier!\x0a"								, LT_OBJECTIVE		, O_ACTF_FRAG_FRIENDLY_CARRIER	, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
//	"%p has TAKEN the %f flag!\x0a"														, LT_OBJECTIVE		, O_CTF_FLAGTAKE				, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
//	"%p has DROPPED the %f flag!\x0a"													, LT_OBJECTIVE		, O_CTF_FLAGDROP				, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
//	
//	//^0=^1T^0= ^7GamisH^7 of the ^2Green Team^7 has TAKEN the flag!
	"[print ]%p defends the %f flag carrier!\x0a"											, LT_OBJECTIVE		, O_ACTF_CARRIER_DEFEND		, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
//	"%p protects the %f Flag carrier!\x0a"												, LT_OBJECTIVE		, O_ACTF_CARRIER_DEFEND		, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
//	// todo: test this one	
	"[print ]%k killed the %t team's flag carrier while carrying the enemy flag {him|her}self!\x0a", LT_OBJECTIVE	, O_ACTF_FRAG_ENEMY_CARRIER	, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
//	
//	// ADVANCED (2 or more flag)CTF Objective patterns // todo: test if these patterns also occur in other gametypes
//	"^7The %f flag has returned.\x0a"												, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED		, NO_INFO		, GT_CAPTURE_THE_FLAG, NO_INFO,
//	"%p has CAPTURED the %f flag!\x0a"												, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG, NO_INFO,
//	// IMPORTANT!!!: this defends dropped flag has to be first, otherwise dropped is interpreted as team
	"[print ]%p defends the %f flag at its base!\x0a"		, LT_OBJECTIVE		, O_ACTF_FLAG_DEFEND_BASE	, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ADVANCED	, NO_INFO,
	"[print ]%p defends the %f flag in the field!\x0a"		, LT_OBJECTIVE		, O_ACTF_FLAG_DEFEND_FIELD	, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_ADVANCED	, NO_INFO,
//	"^7The %f flag returned to base!\x0a"											, LT_OBJECTIVE		, O_CTF_FLAG_RETURNED		, NO_INFO		, GT_CAPTURE_THE_FLAG					, NO_INFO,
//	
//	// REVERSE CTF objective patterns
//	// means he's in the %f team himself 
//	"%p has CAPTURED the %f flag in the %t base!\x0a"								, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG + GTM_REVERSE, NO_INFO,
//
//	// Map specific patterns
//

	// etf_Alps
	"[print ]%p captured the flag\x0a"										, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE, EX_NO_FLAG_INFO, GT_CAPTURE_THE_FLAG			, NO_INFO,

	// etf_stag
	// in the matchlog interpret this as a frag
	"print %v fell under the hypnotic gaze of the king cobra.\x0a"			, LT_FRAG		, T_SUICIDE, W_WRONG_PLACE, NO_INFO	, NO_INFO,
	// in the console.log ignore it because it is followed by a "%v died"
	//"%v fell under the hypnotic gaze of the king cobra.\x0a"			    , LT_IGNORE		, NO_INFO  , NO_INFO	  , NO_INFO	, NO_INFO,

	// etf_rock
	//^7[pnp]donaldo^7 grabbed the ^4Blue^7 key!
	"[print ]%p grabbed the %f^7 key!\x0a"									, LT_OBJECTIVE		, O_CTF_FLAGTAKE			, EX_DEDUCT_TEAM	, GT_CAPTURE_THE_FLAG, NO_INFO,
	"%v sucked in too much gas.\x0a"										, LT_FRAG, T_SUICIDE, W_WRONG_PLACE				, NO_INFO	, NO_INFO,
	
	// Rock has weird capture messages
	// Red team captured blue flag
	"[print ]^1Now Those Bastards Will Pay!^7\x0a"		, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_CAPTURE_BY_RED, GT_CAPTURE_THE_FLAG			, NO_INFO,
	// Blue team captured red flag
	"[print ]^4Now Those Bastards Will Pay!^7\x0a"		, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_CAPTURE_BY_BLUE, GT_CAPTURE_THE_FLAG		, NO_INFO,

	// etf_dissect 
	"[print ]%p^7^7 has the %f ^7flag.\x0a"									, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM + EX_PLAYER_IN_TEAM, GT_CAPTURE_THE_FLAG + GTM_REVERSE, NO_INFO,
	//^1|^4|^7Hitman^7^7^7 captured ^1RED ^7CP^14^7: Circles.
	"[print ]%p^7 captured %f ^7CP%i^7:%i\x0a"								, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_DEDUCT_TEAM + EX_PLAYER_NOT_IN_TEAM, GT_CAPTURE_THE_FLAG + GTM_REVERSE , NO_INFO,

//	// Canalzone, Orbital
	"[print ]%p^7\\[%t Team^7^7\\] has claimed the %l\x0a"							, LT_OBJECTIVE, O_CAH_LOCATION_CLAIM, EX_DEDUCT_TEAM, GT_CAPTURE_AND_HOLD, NO_INFO,

//	"%t controls all of the command points.\x0a"						, LT_OBJECTIVE, O_CAH_ALL_CLAIMED	, NO_INFO		, NO_INFO			 , NO_INFO,
//	"^3STORM MODE!^7\x0a"												, LT_OBJECTIVE, NO_INFO				, NO_INFO		, NO_INFO			 , NO_INFO,
//	"%t base is open to attack!\x0a"									, LT_OBJECTIVE, NO_INFO				, NO_INFO		, NO_INFO			 , NO_INFO,
//	"%t Team Wins!!\x0a"												, LT_OBJECTIVE, O_CAH_VICTORY		, NO_INFO		, NO_INFO			 , NO_INFO,

	// q3f
	// q3f_well	
	"^1Red^7 is no match for the mighty ^4Blue^7!\x0a"						, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_CAPTURE_BY_BLUE, GT_CAPTURE_THE_FLAG, NO_INFO,
	"^4Blue^7 is no match for the mighty ^1Red^7!\x0a"						, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_CAPTURE_BY_RED, GT_CAPTURE_THE_FLAG, NO_INFO,
	// q3f_lastresort
	"The ^1Red^7 flag has been captured, Nice job Blue Team!\x0a"			, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_CAPTURE_BY_BLUE, GT_CAPTURE_THE_FLAG, NO_INFO,
	"The ^4blue^7 flag has been captured, Nice job Red Team!\x0a"			, LT_OBJECTIVE		, O_CTF_FLAGCAPTURE, EX_CAPTURE_BY_RED, GT_CAPTURE_THE_FLAG, NO_INFO,	
	// q3f

//	// local client stuff
	"^1ERROR: %i\x0a"														, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"^3WARNING: %i\x0a"														, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
//	
// Other gameflow stuff
//	"%p was kicked\x0a"    /*"^3.^2standard^7 was kicked" */		, LT_KICK			, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
//	"^7...loaded %r F2R Scripts, %r Spirit Systems\x0a"				, LT_IGNORE			, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"Parsing menu file:%i\x0a"										, LT_STATUS_CHANGE	, GS_PRE_MATCH	, NO_INFO	, NO_INFO	, NO_INFO,
	"...loaded %r faces, %r meshes, %r trisurfs, %r flares %r foliage\x0a"	, LT_MAPINFO, INFO_MAPSIGNATURE, NO_INFO, NO_INFO	, NO_INFO,

	// q3f
	"...loaded %r faces, %r meshes, %r trisurfs, %r flares\x0a"		, LT_MAPINFO, INFO_MAPSIGNATURE		, NO_INFO, NO_INFO	, NO_INFO,
	// q3f

//	"^7...loaded %i locations, %i flares, %i sunflares\x0a"			, LT_STATUS_CHANGE	, GS_PRE_MATCH	, NO_INFO	, NO_INFO	, NO_INFO,
//	"----- CL_Shutdown -----\x0a"									, LT_TIMELIMIT		, GS_POST_MATCH	, NO_INFO	, NO_INFO	, NO_INFO,
	"{print |^7}Timelimit hit.\x0a"									, LT_TIMELIMIT		, GS_POST_MATCH	, NO_INFO	, NO_INFO	, NO_INFO,
	"[print ]%p^7 renamed to %n\x0a"								, LT_RENAME			, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"[print ]%p joined the %t Team^7.\x0a"							, LT_JOIN			, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"[print ]%p connected\x0a"										, LT_CONNECT		, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"[print ]%p ^1has disconnected.\x0a"							, LT_DISCONNECT		, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"[print ]%p is spectating the %t team.\x0a"						, LT_SPECTATE		, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"^7server: %i\x0a"												, LT_IGNORE			, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	// q3f
	"Timelimit hit.\x0a"											, LT_TIMELIMIT		, GS_POST_MATCH	, NO_INFO	, NO_INFO	, NO_INFO,
	"%p joined the %t Team.\x0a"									, LT_JOIN			, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"%p disconnected\x0a"											, LT_DISCONNECT		, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	// q3f

	// obsolete (worked for etf). now skip debug lines = anything that does not start with a '^'
	// beta1 contains non-debug messages that don't start with a colorcode, so basically agostats is not good for beta1
	// fix this by listing all debug messages that don't start with a '^' ' todo: also check for etf and q3f2
	
	//"~^%i.pk3 (%d files)\x0a"	/* d:\games\quake3\baseq3\pak8.pk3 (9 files)*/, LT_IGNORE		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
/*
	"Unknown command \"%i\"\x0a"									, LT_IGNORE		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"-----%i\x0a"													, LT_IGNORE		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"...%i\x0a"														, LT_IGNORE		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"Now listening on: #%i.\x0a"									, LT_IGNORE		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
*/	
	// q3f

//"~^%i\x0a"														, LT_IGNORE			, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,

//	// the rest can be interpreted as chat i guess
//
//	// put chat patterns at the end, when nothing else is found
//	// TEAMCHAT
	"(%p^7): %c\x0a"											, LT_TEAMCHAT	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
//	// CHAT
	"^5PunkBuster Client: %c\x0a"								, LT_IGNORE		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"%p^7: %c\x0a"												, LT_CHAT		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,


	//"print %i\x0a"				, LT_PRINT, NO_INFO			, NO_INFO	, NO_INFO	, NO_INFO,
	// to stop non-game messages (debug messages) to be interpreted
	//"chat %p\x19: %c\x0a"													, LT_CHAT		, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,

#else

	//2246:463 changed name. new = ^0P^Yaranoid ^0A^Yndroid^0[^7MPK^0], ct = 134800800, t = 134806550
	//ClientUserinfoChanged (setteam): 17 n\^zFeral^7Sacred\t\1\cls\0\g\0
	//"%p has TAKEN the %f flag!\x0a"				, LT_OBJECTIVE		, O_CTF_FLAGTAKE, EX_DEDUCT_TEAM, GT_CAPTURE_THE_FLAG + GTM_ADVANCED, NO_INFO,
	//2414:44------------------------------------------------------------
	//1628:17Destroy: autosentry 1189731840 14: Gm^db^7H^d*^7Brama^7destroyed ^gkuslig^7's autosentry.
	//1639:04Destroy: supplystation 1189731124 14: ^7Gm^db^7H^d*^7Sock^7 destroyed ^gkuslig^7's supplystation.	
	//1640:06etfbroadcast: _messageall "FoLke" FoLke^7 defends the blue flag at its base!

	//2290:17InitGame: \sv_numbots\0\g_unlaggedVersion\2.0\players_blue\0 \score_green\0\score_yellow\0\score_blue\50\score_red\20\g_etfVersion\ETF 1.1.0\g_gameindex\1\gamename\etf\g_gametype\0\g_voteFlags\0\g_minGameClients\8\g_needpass\1\sv_allowAnonymous\0\sv_privateClients\2\mapname\etf_silverfort\protocol\83\sv_keywords\etf, fortress\version\ET 2.56 linux-i386 Sep 10 2003\sv_minRate\2500\sv_minSnaps\10\dmflags\0\fraglimit\0\timelimit\44\sv_hostname\#spontanetf Gather Server 3 - by clan^2-^7JM^7\sv_maxclients\20\sv_punkbuster\1\sv_minguidage\0\sv_maxRate\20000\sv_minPing\0\sv_maxPing\0\sv_floodProtect\0\g_friendlyFire\Full\g_maxlives\1\g_antilag\1\g_banRules\3\capturelimit\0\g_maxGameClients\0\g_agentHitBeep\0\g_unlagged\1\sv_pure\1
	// this can also be a teamkill

	"%d:%d InitGame: %i\x0a"																, LTS_INITGAME	, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d Exit: Timelimit hit.\x0a"														, LT_TIMELIMIT	, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	//"%d:%d ShutdownGame:\x0a"																, LT_TIMELIMIT	, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,

	"%d:%d ClientConnect: %p\x0a"															, LT_CONNECT		, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,
	"%d:%d ClientDisconnect: %p\x0a"														, LT_DISCONNECT		, NO_INFO		, NO_INFO	, NO_INFO	, NO_INFO,

	"%d:%d Destroy: supplystation %i %i: %k^7 destroyed %v^7's supplystation.\x0a"			, LTS_FRAG		, T_KILLED_SUPPLYSTATION	, W_UNKNOWN			, NO_INFO								, NO_INFO,
	"%d:%d Destroy: autosentry %i %i: %k^7destroyed %v^7's autosentry.\x0a"					, LTS_FRAG		, T_KILLED_AUTOSENTRY		, W_UNKNOWN			, NO_INFO								, NO_INFO,

	"%d:%d ------------------------------------------------------------\x0a"				, LT_IGNORE		, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	//1619:16ClientUserinfoChanged (setteam): 14 n\^gkuslig\t\1\cls\0\g\0
	"%d:%d ClientUserinfoChanged (setteam): %p n\\\\%n\\\\t\\\\%t\\\\cls\\\\%i\x0a"			, LTS_SETTEAM	, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	// apparently during matchmode the "(setteam)" is never done, so use "(spawn with new class)" to determine team
	"%d:%d ClientUserinfoChanged (spawn with new class): %p n\\\\%n\\\\t\\\\%t\\\\cls\\\\%i\x0a", LTS_SETTEAM	, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	
	"%d:%d %p changed name. new = %n, ct = %i, t = %i\x0a"									, LTS_RENAME	, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d Item: %i backpack\x0a"															, LT_CLIENT_ONLY, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d Item: %i ammobox_%i\x0a"															, LT_CLIENT_ONLY, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d etfbroadcast: single_message \"%i\x0a"											, LT_CLIENT_ONLY, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d Kill: %k %v 44: %i\x0a"															, LTS_MOD_CUSTOM, NO_INFO					, PLAYER_BY_NUMBER	, NO_INFO								, NO_INFO,
	"%d:%d Kill: %k %v %w: %i\x0a"															, LTS_FRAG		, NO_INFO					, PLAYER_BY_NUMBER	, NO_INFO								, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 has CAPTURED the %f flag!\x0a"				, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE			, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 CAPTURED the %f flag!\x0a"					, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE			, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" The %f^7 flag has returned.\x0a"				, LT_OBJECTIVE	, O_CTF_FLAG_RETURNED	, NO_INFO			, GT_CAPTURE_THE_FLAG					, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 has TAKEN the %f flag!\x0a"				, LT_OBJECTIVE	, O_CTF_FLAGTAKE		, NO_INFO			, NO_INFO								, NO_INFO,
	// map etf_excel
	"%d:%d etfbroadcast: nonteam_message \"%i\" %p^7 has TAKEN the %f flag!\x0a"			, LT_OBJECTIVE	, O_CTF_FLAGTAKE		, NO_INFO			, NO_INFO								, NO_INFO,
	// map etf_mini_noheros
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 nabbed the %f ^7flag!\x0a"					, LT_OBJECTIVE	, O_CTF_FLAGTAKE		, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 CAPTURED the %f ^7flag!\x0a"				, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE		, NO_INFO			, NO_INFO								, NO_INFO,
	
	// map etf_rock
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 grabbed the %f key!\x0a"					, LT_OBJECTIVE	, O_CTF_FLAGTAKE		, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" ^1Now Those Bastards Will Pay!^7\x0a"			, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE		, EX_CAPTURE_BY_RED	, NO_INFO								, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" ^4Now Those Bastards Will Pay!^7\x0a"			, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE		, EX_CAPTURE_BY_BLUE, NO_INFO								, NO_INFO,

	// map etf_alps
	"%d:%d etfbroadcast: _messageall \"%i\" %p captured the flag\x0a"						, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE		, EX_NO_FLAG_INFO, NO_INFO								, NO_INFO,
	//"[print ]%p captured the flag\x0a"										, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE, EX_NO_FLAG_INFO, GT_CAPTURE_THE_FLAG			, NO_INFO,


	// map etf_spazball
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7^7^7 (%i Team^7^7) ^5has the SPAZ BALL!\x0a", LT_OBJECTIVE	, O_CTF_FLAGTAKE			, NO_INFO			, NO_INFO								, NO_INFO,
	//825:48 etfbroadcast: _messageall "cs^44^7u^3!^7B^va^7rb^3i^7e" cs^44^7u^3!^7B^va^7rb^3i^7e^7^7^7 (^1Red Team^7^7) ^5has the SPAZ BALL!
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 scores against the %f team!\x0a"				, LT_OBJECTIVE	, O_CTF_FLAGCAPTURE			, NO_INFO			, NO_INFO								, NO_INFO,
	//828:33 etfbroadcast: _messageall "cs^44^7u^3!^7B^va^7rb^3i^7e" cs^44^7u^3!^7B^va^7rb^3i^7e^7^7 scores against the ^2Green^7 team!

	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 defends the %f flag at its base!\x0a"		, LT_OBJECTIVE	, O_ACTF_FLAG_DEFEND_BASE	, NO_INFO			, GT_CAPTURE_THE_FLAG + GTM_ADVANCED	, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 defends the %f flag in the field!\x0a"		, LT_OBJECTIVE	, O_ACTF_FLAG_DEFEND_FIELD	, NO_INFO			, GT_CAPTURE_THE_FLAG + GTM_ADVANCED	, NO_INFO,
	"%d:%d etfbroadcast: _messageall \"%i\" %p^7 defends the blue flag carrier!\x0a"		, LT_OBJECTIVE	, O_ACTF_CARRIER_DEFEND		, NO_INFO			, GT_CAPTURE_THE_FLAG + GTM_ADVANCED	, NO_INFO,
	// 1614:16Mapname: etf_bases, gameindex: 1
	"%d:%d Mapname: %r, gameindex: %r\x0a"													, LTS_MAPINFO	, INFO_MAPNAME				, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d sayteam: %p: %c\x0a"																, LT_TEAMCHAT	, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
	"%d:%d say: %p: %c\x0a"																	, LT_CHAT		, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
#if defined(_DEBUG)
	// remove some clutter from parsed.txt when debugging
	"%d:%d ClientUserinfoChanged (%i): %p n\\\\%n\\\\t\\\\%t\\\\cls\\\\%i\x0a"				, LT_IGNORE		, NO_INFO					, NO_INFO			, NO_INFO								, NO_INFO,
#endif
	// 2735:05yellow:2151  2735:05green:2478  2735:05
	// for map orbital
	//"%i:{0|1|2|3|4|5}{0|1|2|3|4|5|6|7|8|9}yellow:%a %i:{0|1|2|3|4|5}{0|1|2|3|4|5|6|7|8|9}green:%a %i\x0a"			, LTS_ENDSCORE	, MAP_ORBITAL		, NO_INFO			, NO_INFO	, NO_INFO,
#endif

#if defined(_DEBUG)
	// Add strings here to remove clutter from the parsed.txt file that displays
	// unmatched strings
	"^7Sorry, you have no ammo to drop.\x0a"					, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"^7%i primed, four seconds.\x0a"							, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"^7You have no Hand grenades left.\x0a"						, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"^7You picked up %i\x0a"									, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"...%i\x0a"													, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
	"~^%i.pk3 (%d files)\x0a"									, LT_CLIENT_ONLY	, NO_INFO	, NO_INFO	, NO_INFO	, NO_INFO,
#endif

	"", 0, 0, 0, 0, 0 // empty to indicate end of patternlist


};

void clear_optional_string( void )
{
	optional[ 0 ] = '\0';
}

void store_optional_string( char *found )
{
	strcpy( optional, found );
}

// todo: enumerate storage variables
char *select_storage_variable( char v )
{
	max_storage_length = 0;

	switch (v) {
		case 'a':
			max_storage_length = sizeof( ammo ) - 1;
			return ammo;

		case 'b':
			max_storage_length = sizeof( building ) - 1;
			return building;

		case 'c' :
			max_storage_length = sizeof( chat ) - 1;
			return chat;

		//case 'd' : 

		case 'e':
			max_storage_length = sizeof( debug ) - 1;
			return debug;

		case 'f' :
			max_storage_length = sizeof( flag ) - 1;
			return flag;

		//case 'g' :

		case 'i': // ignore
			return NULL;

		case 'k' : 
			max_storage_length = sizeof( killer ) - 1;
			return killer;

		case 'l' :
			max_storage_length = sizeof( location ) - 1;
			return location;

		case 'n' :
			max_storage_length = sizeof( newname ) - 1;
			return newname;
	
		case 'p' :
			max_storage_length = sizeof( player ) - 1;
			return player;

		case 'r' :
			max_storage_length = VAR_ARRAY_ELEMENT_SIZE - 1;
			if ( iar < VAR_ARRAY_SIZE ) {
				return array[ iar++ ];
			}
			else {
				return array[ VAR_ARRAY_SIZE ];
			}

		case 't' :
			max_storage_length = sizeof( team ) - 1;
			return team;
	
		case 'v' :
			max_storage_length = sizeof( victim ) - 1;
			return victim;

		case 'w':
			max_storage_length = sizeof( weapon ) - 1;
			return weapon;

		case 'x':
			max_storage_length = sizeof( action ) - 1;
			return action;

		default :
			return NULL;
			break;
		
	}
}

int is_token( char *ctest ){
	int result = FALSE;

#if defined(SERVER_STATS)
	// todo: check the construction below for normal logfiles
#else
	if ( *(ctest - 1 ) == TK_ESCAPE ) {
		return result;
	}
#endif

	switch (*ctest) {

		case TK_CHOICE_START :
			result = TRUE;
			break;

		case TK_SEPARATOR :
			result = TRUE;
			break;

		case TK_CHOICE_END :
			result = TRUE;
			break;

		case TK_OPTIONAL_START :
			result = TRUE;
			break;

		case TK_OPTIONAL_END :
			result = TRUE;
			break;

		case TK_STORE:
			result = TRUE;
			break;

		case TK_END_OF_LINE :
			result = TRUE;
			break;

		case TK_UNMATCH :
			result = TRUE;
			break;
	}

	return result;
}

void clear_line_info( void )
{
	player	[ 0 ] = '\0';
	newname	[ 0 ] = '\0';
	killer	[ 0 ] = '\0';
	victim	[ 0 ] = '\0';
	location[ 0 ] = '\0';
	chat	[ 0 ] = '\0';
	team	[ 0 ] = '\0';
	flag	[ 0 ] = '\0';
	ammo	[ 0 ] = '\0';
	debug	[ 0 ] = '\0';
	weapon	[ 0 ] = '\0';
	action	[ 0 ] = '\0';
	building[ 0 ] = '\0';

	chatline	= FALSE;
	teamchatline= FALSE;
	teamkill    = FALSE;
	suicide		= FALSE;

	player_nr = 0;
	killer_nr = 0;
	victim_nr = 0;
	flag_nr   = 0;

	iar       = 0; // array index
	
}

void build_line_info( char *store, char *begin, char *end )
{
	unsigned int i;

	if ( store != NULL && begin != NULL && end != NULL ) {

		strncopy( store, begin, my_min( (end - begin), max_storage_length ) );
		store[ end - begin ] = '\0';

		if ( store == team || store == flag ) {
			//  convert to plaintext and uppercase
			for ( i = 0; i < strlen( store ) ; i++ ) {
				store[ i ] = toupper( store[ i ] );
			}
		}
	}
}

int match_whitespace( char **pattern )
{
	int i = 0;
	int result = TRUE;
	char *pattern_pointer;

	pattern_pointer = *pattern;

	if ( pattern_pointer[ i ] != ' ' ) {
		result = FALSE;
	}
	else {
		while ( pattern_pointer[ i ] == ' ' ) {
			i++;
		}
	}
	if ( result == TRUE ) {
		*pattern = pattern_pointer + i;
	}
	return result;
}

int match_character( char **pattern )
{
	// match whitespace too
	int i = 0;
	int result = FALSE;
	char *pattern_pointer;

	pattern_pointer = *pattern;
	if ( pattern_pointer[ i ] != TK_END_OF_LINE ) {
		pattern_pointer++;
		*pattern = pattern_pointer + i;
		result = TRUE;
	}

	return result;
}
int match_number( char **pattern )
{
	// match whitespace too
	int i = 0;
	int result = TRUE;
	char *pattern_pointer;

	match_whitespace( pattern );
	pattern_pointer = *pattern;
	if ( !is_digit( pattern_pointer[ 0 ] ) ) {
		result = FALSE;
	}
	else {
		while ( is_digit( pattern_pointer[ i ] ) && 
				pattern_pointer[ i ] != TK_END_OF_LINE ) {
			i++;
		}
	}

	if ( result == TRUE ) {
		*pattern = pattern_pointer + i;
	}

	return result;
}

// skip characters in the pattern until ctoken is found
char *find_token( char *pattern, char ctoken ) {
	int i = 0;

	while ( pattern[ i ] != ctoken && pattern[ i ] != TK_END_OF_LINE ) {

		if ( pattern[ i ] == TK_ESCAPE ) {
			i+=2;
		}
		else {
			i++;
		}
		
	}

	return pattern + i;
}
//
// verbatim reads until the first token ('%', '{', '\0' ) character and
// copies the characters to a temporary string
// returns pointer to first character of rest of pattern (where to continue)
char *fill_patternbuffer( char *string )
{
	int is = 0;
	int ip = 0;

	while ( !is_token( &(string[ is ]) ) ) {
		if ( string[ is ] != TK_ESCAPE ) {
			patternbuffer[ ip ] = string[ is ];
			ip++;
		}
		else {
			is++;
			// literally take the next character;
			patternbuffer[ ip ] = string[ is ];
			ip++;
		}
		is++;
	}

	patternbuffer[ ip ] = '\0';

	return string + is;
}

int my_compare( char *s1, char *s2 ){
	// string compare upto max length of smallest string
	int i = 0;
	int equal = TRUE;

	while ( equal && s1[ i ] != '\0' && s2[ i ] != '\0' ){
		equal = s1[ i ] == s2[ i ];
		i++;
	}

	return equal;
}

// reject line if storage variables contain weird values
int validate_line()
{
	int result = TRUE;

	if ( strlen( player ) > 40 ) {
		result = FALSE;
	}
	if ( strlen( team ) > 30 ) {
		result = FALSE;
	}
	if ( strlen( killer ) > 40 ) {
		result = FALSE;
	}
	if ( strlen( victim ) > 40 ) {
		result = FALSE;
	}
	if ( strlen( flag ) > 30 ) {
		result = FALSE;
	}
	if ( strlen( ammo ) > 30 ) {
		result = FALSE;
	}
	if ( strlen( weapon ) > 30 ) {
		result = FALSE;
	}
	if ( strlen( building ) > 30 ) {
		result = FALSE;
	}
#if defined (_DEBUG)
	if ( result == FALSE ) {
		// put a breakpoint here
		result = FALSE;
	}
#endif
	return result;
}

// check if pattern occurs in line and fill variables if it does
// line is line from qconsole.log
// pattern is patternstring from PATTERNS
int match( char *line, char *pattern )
{
	int result		= TRUE;
	int finished	= FALSE;
	int choice_matched;
	int optional_matched;
    char *line_pointer = line;
	char *pattern_pointer = pattern;
	char *end_of_line;
	char *found		= NULL;
	char *store		= NULL;
	// todo: optional "[ally]"
	// choice   "{a|b|c}"

	iar = 0;
	clear_optional_string();
	end_of_line = line + strlen(line);

    while ( result && !finished ) {

		switch (*pattern_pointer) {
			case '\0':
				// do nothing, loop will terminate
				if (line_pointer < end_of_line ) {
					build_line_info( store, line_pointer, end_of_line );
					line_pointer = end_of_line;
				}
				break;

			case TK_OPTIONAL_START : 
				optional_matched = FALSE;
				pattern_pointer++;
				while (*pattern_pointer != TK_OPTIONAL_END && 
					   *pattern_pointer != TK_END_OF_LINE ){
				
					pattern_pointer = fill_patternbuffer( pattern_pointer );

					if ( my_compare( line_pointer, patternbuffer ) ) {
						// we got a match
						store_optional_string( patternbuffer );
						// move line pointer forward
						line_pointer = line_pointer + strlen( patternbuffer );
						// move pattern pointer forward			
						pattern_pointer = find_token( pattern_pointer, TK_OPTIONAL_END );
						optional_matched = TRUE;
					}
				}
				if ( *pattern_pointer == TK_OPTIONAL_END ) {
					pattern_pointer++;
				}
				result = TRUE;
				break;

			case TK_CHOICE_START : // '{'
				
				choice_matched = FALSE;
				pattern_pointer++;
				while (*pattern_pointer != TK_CHOICE_END && 
					   *pattern_pointer != TK_END_OF_LINE ){
				
					pattern_pointer = fill_patternbuffer( pattern_pointer );

					if ( my_compare( line_pointer, patternbuffer ) ) {
						// we got a match
						// todo: store found string in some variable
						// move line pointer forward
						line_pointer = line_pointer + strlen( patternbuffer );
						// move pattern pointer forward			
						pattern_pointer = find_token( pattern_pointer, TK_CHOICE_END );
						choice_matched = TRUE;
					}
					else {
						if ( *pattern_pointer == TK_SEPARATOR ) {
							pattern_pointer++;
						}
					}
				}
				if ( *pattern_pointer == TK_CHOICE_END ) {
					pattern_pointer++;
				}
				if ( !choice_matched ) {
					result = FALSE;
				}
				break;

			case TK_STORE : // '%'
				pattern_pointer++;

				// todo: match numbers ( match special stuff)
				switch( *pattern_pointer ) {
					case 'd':
						result = match_number( &line_pointer );
						if ( result == FALSE ) {
							break;
						}
						else {
							pattern_pointer++;
						}
						break;

					case 'g':
						result = match_character( &line_pointer );
						if ( result == FALSE ) {
							break;
						}
						else {
							pattern_pointer++;
						}
						break;

					default:
						store = select_storage_variable( *pattern_pointer );
					
						if ( *pattern_pointer != '\0' ) {
							pattern_pointer++;
						}
						break;
				}
				break;

			case TK_UNMATCH : // '~7'  -> means the next character must be unequal to 7
				pattern_pointer++;
				if  ( *pattern_pointer == *line_pointer ) {
					result = FALSE;
				}
				else {
					line_pointer++;
					pattern_pointer++;
				}
				break;

			default :
				pattern_pointer = fill_patternbuffer( pattern_pointer );

				found = strstr( line_pointer, patternbuffer );
				if ( found != NULL ) {
					// cool we found something
					// the string inbetween line_pointer and found should be put in a variable;
					build_line_info( store, line_pointer, found );
					store = NULL; // make sure the variable is not overwritten

					line_pointer = found + strlen( patternbuffer );
				}
				else {
					// it didn't match
					result = FALSE;
				}
				break;

		}

		finished = ( line_pointer >= end_of_line );
	}

	if ( result == TRUE ) {
		result = validate_line();
	}

	return result;
}
