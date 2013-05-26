/*
** Date       Version What
** ----------------------------------------------------------------------------------
** 20-10-2007 0.65    support for q3f1
**                    added serverlog support for etf_spamtech
**                    updated url to agostats homepage
**                    fixed -l parameter causing wrong filename for serverstats
**                    fixed -l parameter to base highscores on last game only
**                    show timelimit, and mm1 after timelimit
**                    added award first blood
**                    improved how events are displayed
**                    experimental: individual player highscores (option -a)
**                                  files are created in agostats_data directory
**                    experimental: in etf console type echo as.date [2007-05-15]
** 18-06-2006 0.64    added support for some maps
**                    fixed empty mapdescription for serverlog                  
**                    fixed incorrect html for events table
**                    added most teamdeaths award
**                    small improvements to default style and layout
** 11-03-2006 0.63    fixed in matchmode no team detection in serverlog 
**                    added mapname in filename, requested by mechano, 
**                          filename is now much more configurable like tables 
**                          -ftnsc (Timestamp, mapName, Scores, Customname) 
**                          combine with customname from -o option
**					        related commandline options -n,-s,-d are removed
**                    fixed incorrect teamscores in filename
**                    removed obsolete filename options
**                    added frags by weapon for squads
**                    fixed align numbers in popups
**                    fixed some buffer overflow
**                    fixed custom table sequence, table nr.2 displayed wrong table                   
** 08-09-2005 0.62    popup info 0 frags but sentries destroyed in pvp table
**                    mm1/events table is available again
**                    missing players from serverstats
**                    captures/touches table requested by Hitman
** 04-07-2005 0.61    fixed wrong date in highscores
**                    fixed wrong number in godzilla award/highscore
**                    fixed "gunned down" pattern
**                    added player vs player: sentries destroyed
**                    added tranquilizer award
**                    added crushed by sentry
**                    added support for some third party maps
**                    added meta tags for teams/maps/scores
**                    added map recognition for etconsole.log
**                    added google link to clantags in the gameresult
**                    disabled events/mm1 until I got the database thingy working
** 03-04-2005 0.60    highscores for awards are kept
**                    new option -t for table selection
**                    new option -l to only do stats for last game in logfile
**                    support for map etf_dissect	
** 19-03-2005 0.59    Fixed many serverstats bugs
**                    Fixed some clientstats bugs
**                    get multiple games from logfiles
** 01-03-2005 0.58    added initial support for 2 team capture and hold
**                    added matchlog support
**                    added games.log support (logfile on server)
** 15-02-2005 0.57    Support for ETF1.1
**                    fixed a bunch of patterns
**                    added optional strings to patternmatching
**                    filter out pre-match action
** 13-02-2005 0.56    fixed some patterns
** 08-02-2005 0.55    Ported AGOStats to ETF based on etconsole.log
** 08-12-2004 0.54    fixed missing last character of clan tag, often a space so nobody missed it
**                    added OS to version info W=Windows, L=Linux
**                    fixed popup numbers in player vs player table, some were doubled in value
** 06-11-2004 0.53    output is compliant to the xhtml1.1 standard.
**                    added capture history with the number of touches needed for each capture
**                    Removed "The Game", it looked crappy
**                    added (JMr|DivinatioN): "teamkilled blabla (while carrying enemy flag) 1"
** 07-08-2004 0.51    fixed table caption alignment for mozillah firefox
**                    removed C++ constructions, sourcecode is straight C now
**                    addded frags on flagcarrier by weapon to popups
** 08-05-2004 0.50    Added popups that give player vs player frags by weapon info,
**                    also detailed suicide info
**                    Added option -v to display programname and version
** 08-04-2004 0.49    Every piece of presentation done by stylesheet allowing users
**                    to fully customize the look.
**            0.48    Output compliant to the XHTML 1.0 Transitional standard
** 09-02-2004 0.47    crater award, flashkill award, linux support
** 16-01-2004 0.46    ignoring actf player defends flag in field message because it's too 
**                    buggy
**                    removed a bunch of unneeded sorting stuff now that awards have their 
**                    own sorting
**                    added missing pattern for wrench kills
**                    fixed: clan eca tag was not found
**                    frags/deaths by weapon are now sorted by frags descending
**                    handling sentry death by environment (doors/lifts)
** 04-01-2004 0.45    Initial support for Capture & Hold and Duelling, mainly layout tweaks
**                    added flagpatterns for q3f_castle
**                    if a sentry is not killed its frag/deathratio is not printed in the 
**                    stats
**                    oops, added missing pattern for death by drowning
**                    changed awards layout, also some internal changes
** 27-11-2003 0.44    addded missing pattern for infection teamkill
**                    unknown squad is printed as "" instead of "?"
**                    sorting players by teamscore in players and fragmatrix tables, so the 
**                    winning team comes first
**                    extra info in team objectives section
**                    now all q3f 2.3 maps are recognized (nothing done yet for special 
**                    maps like canalzone/chaos/gotduck)
**                    ignore "bla defended the flag in the field" messages in japanese 
**                    castles because they appear incorrectly (a lot!)
** 25-10-2003 0.43    flagtracking no longer based on actf messages as most mappers don't 
**                    seem to bother using them
**                    base game totals on players, not on teams
**                    support for new maps aztec, struggle
**                    fixed buffer overflow, shame on me
**                    new commandline parameter -s to show teamscores in filename, 
**                    "stats-BLUE-14-RED-11.htm"
** 19-10-2003 0.42    Before calculating everything the cleanup procedure is called to
**                    remove inactive players and merge duplicate players, fixed some
**                    minor other stuff
** 05-10-2003 0.41    Fixed bug with converting q3string to html
** 04-10-2003 0.40    Bunch of fixes, some layout improvement
** 11-09-2003 0.39.4  Added messages for Dew Wars, fixed a bug in html clantag
** 08-08-2003         Layout improvements, disabled player_joins_team section because it 
**                    contains a bug somewhere. Teams sorted by score
** 05-08-2003         Clickable players
** 03-08-2003 0.39    Improved squad detection algorithm, added option -o<outputfile>
**                    Teams now displayed side by side, fragratio for sentries
** 22-07-2003         Players changing team or re-joining are handled a bit better now.
** 18-07-2003 0.38    Adjusted method for determining which squad a player is in,
**                    added some info to html fragmatrix
** 17-07-2003         HTML clantag now extracted for normal clantags. Reverse clantags 
**                    (plan-B)-like are just plaintext
** 16-07-2003         Fixed a bug that would display a weird mapname, added info for chaos 
**                    and opposition.
** 15-07-2003 0.37    Popup info in fragmatrix, html links underlined now, explanation of
**                    colored numbers in fragmatrix.
** 13-07-2003 0.36    HTML validated
** 12-07-2003 0.35    Done the the bulk part for the HTML output
** 22-04-2003 0.34    Added some strings for new maps openfire2b, Ice, Ultima, Impact.
**                    Created workaround for Ice because it doesn't have the 
**                    "bla killed the blue flagcarrier" message.
** 17-04-2003 0.33    Sorting players BY_TEAM_BY_SQUAD for easier comparing
** 08-03-2003 0.32    Renamed program to agostats
** 02-03-2003 0.31    Added flag patterns for smartbases2
** 22-02-2003 0.30    Added squad statistics
** 15-02-2003 0.29    Support for use of stdin/stdout so redirection can be used on the 
**                    commandline. Added extra options for showing mm1 and events.
** 08-02-2003 0.28    Better team detection
** 02-02-2003 0.27    Added Maprecognition and tracking of fragging the flagcarrier in
**                    OLD ctf.
** 31-01-2003 0.26    Added long playernames, no disconnects after timelimit shown
**                    Added map recognition
** 28-01-2003 0.25    Fixed a bug where variables would be cleared before stats were
**                    printed
** 19-01-2003 0.24    Added output for kicked players
** 08-12-2002 0.23    Added more player variables and off/def squads
** 25-11-2002 0.22    Comparing also plaintext playernames to select player
**                    Filter out punkbuster client messages
** 12-11-2002 0.21    Frag/Death ratio in playerstats
** 22-10-2002 0.20    Fixed "team unknown AGO Zealot" bug, added some team stats
** 17-10-2002 0.19    Added optional matches {his|her} to detect gender dependent messages
**                    support for spazball
** 11-10-2002 0.18    Fixed month in timestamp, added some commandline params
**                    Added flagtracking and counter for coast to coast captures
** 06-10-2002 0.17    Added timestamp to filename, better name correction
** 06-08-2002 0.16    Added support for map q3f_h4rdcore, has some different messages
**
** April 2002 Initial version
*/

#define VERSION			"0.65"
#if defined(SERVER_STATS)
	#define PROGRAM_NAME	"AGOStats Server"
#else
	#define PROGRAM_NAME	"AGOStats Client"
#endif
#define FOR_GAME        "ETF (+Q3F1)"
#if defined(WIN32)
	#define OS              "W"
#else
	#if defined(LINUX)
		#define OS          "L"
	#else 
		#if defined(LINUX_STATIC)
			#define OS      "LS"
		#else
			#define OS "X"
		#endif
	#endif
#endif

// ===== todo/ideas:
// take into account for awards when player was in multiple teams, example: qconsole.log.changeteam
// teamkleuren in the fragmatrix top balk waar de playernummers staan
// try roles : inner/outer for canalzone maybe chaser capper deffer
// flagcap matrix for more than 3 teams ctf
// create more popups, bijv top 3 popup at awards
// calculate an off/def % for role use ?
// calculate the percentage by counting frags against off/def players ?
// determine dm based squads
// award for strongest attacker (off player with highest frag ratio)
// store events in player structure, and calculate points afterwards, also based on gametype(gtm_reverse, etc.)
// directs > splashkills ! some sort of indication
// add aliases that players renamed to in the detailed player info

// player uses echo command to give extra info for instance:
// as.league:eetfl5
// as.type:(war|internal|scrimm)
// as.date [2007-5-21] (yyyy-m-d)
// as.directory:mywars
// also add this manually to logfiles afterwards

// ===== problems:
// multiple players for the same highscores
// for spiderx there is no flagreturn message in the games.log, looks like this happens for other maps as well

// ===== in progress:

// ===== fixed:
