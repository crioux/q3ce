/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
/*
=======================================================================

MULTIPLAYER MENU (SERVER BROWSER)

=======================================================================
*/


#include"ui_pch.h"


#define MAX_GLOBALSERVERS		128
#define MAX_PINGREQUESTS		32
#define MAX_ADDRESSLENGTH		64
#define MAX_HOSTNAMELENGTH		22
#define MAX_MAPNAMELENGTH		16
#define MAX_LISTBOXITEMS		128
#define MAX_LOCALSERVERS		128
#define MAX_STATUSLENGTH		64
#define MAX_LEAGUELENGTH		28
#define MAX_LISTBOXWIDTH		68

#define ART_BACK0				"menu/art/back_0"
#define ART_BACK1				"menu/art/back_1"
#define ART_CREATE0				"menu/art/create_0"
#define ART_CREATE1				"menu/art/create_1"
#define ART_SPECIFY0			"menu/art/specify_0"
#define ART_SPECIFY1			"menu/art/specify_1"
#define ART_REFRESH0			"menu/art/refresh_0"
#define ART_REFRESH1			"menu/art/refresh_1"
#define ART_CONNECT0			"menu/art/fight_0"
#define ART_CONNECT1			"menu/art/fight_1"
#define ART_ARROWS0				"menu/art/arrows_vert_0"
#define ART_ARROWS_UP			"menu/art/arrows_vert_top"
#define ART_ARROWS_DOWN			"menu/art/arrows_vert_bot"
#define ART_UNKNOWNMAP			"menu/art/unknownmap"
#define ART_REMOVE0				"menu/art/delete_0"
#define ART_REMOVE1				"menu/art/delete_1"
#define ART_PUNKBUSTER		"menu/art/pblogo"

#define ID_MASTER			10
#define ID_GAMETYPE			11
#define ID_SORTKEY			12
#define ID_SHOW_FULL		13
#define ID_SHOW_EMPTY		14
#define ID_LIST				15
#define ID_SCROLL_UP		16
#define ID_SCROLL_DOWN		17
#define ID_BACK				18
#define ID_REFRESH			19
#define ID_SPECIFY			20
#define ID_CREATE			21
#define ID_CONNECT			22
#define ID_REMOVE			23
#define ID_PUNKBUSTER 24

#define GR_LOGO				30
#define GR_LETTERS			31

#define AS_LOCAL			0
#define AS_MPLAYER			1
#define AS_GLOBAL			2
#define AS_FAVORITES		3

#define SORT_HOST			0
#define SORT_MAP			1
#define SORT_CLIENTS		2
#define SORT_GAME			3
#define SORT_PING			4

#define GAMES_ALL			0
#define GAMES_FFA			1
#define GAMES_TEAMPLAY		2
#define GAMES_TOURNEY		3
#define GAMES_CTF			4

static const char *master_items[] = {
	"Local",
	"Internet",
	"Favorites",
	0
};

static const char *servertype_items[] = {
	"All",
	"Free For All",
	"Team Deathmatch",
	"Tournament",
	"Capture the Flag",
	0
};

static const char *sortkey_items[] = {
	"Server Name",
	"Map Name",
	"Open Player Spots",
	"Game Type",
	"Ping Time",
	0
};

static const char* gamenames[] = {
	"DM ",	// deathmatch
	"1v1",	// tournament
	"SP ",	// single player
	"Team DM",	// team deathmatch
	"CTF",	// capture the flag
	"One Flag CTF",		// one flag ctf
	"OverLoad",				// Overload
	"Harvester",			// Harvester
	"Rocket Arena 3",	// Rocket Arena 3
	"Q3F",						// Q3F
	"Urban Terror",		// Urban Terror
	"OSP",						// Orange Smoothie Productions
	"???",			// unknown
	0
};

static const char* netnames[] = {
	"???",
	"UDP",
	"IPX",
	NULL
};

static char quake3worldMessage[] = "Visit www.quake3world.com - News, Community, Events, Files";

const char* punkbuster_items[] = {
	"Disabled",
	"Enabled",
	NULL
};

const char* punkbuster_msg[] = {
	"PunkBuster will be",
	"disabled the next time",
	"Quake III Arena",
	"is started.",
	NULL
};

typedef struct {
	char	adrstr[MAX_ADDRESSLENGTH];
	int		start;
} pinglist_t;

typedef struct servernode_s {
	char	adrstr[MAX_ADDRESSLENGTH];
	char	hostname[MAX_HOSTNAMELENGTH+3];
	char	mapname[MAX_MAPNAMELENGTH];
	int		numclients;
	int		maxclients;
	int		pingtime;
	int		gametype;
	char	gamename[12];
	int		nettype;
	int		minPing;
	int		maxPing;
	qboolean bPB;

} servernode_t; 

typedef struct {
	char			buff[MAX_LISTBOXWIDTH];
	servernode_t*	servernode;
} table_t;

typedef struct {
	menuframework_s		menu;

	menutext_s			banner;

	menulist_s			master;
	menulist_s			gametype;
	menulist_s			sortkey;
	menuradiobutton_s	showfull;
	menuradiobutton_s	showempty;

	menulist_s			list;
	menubitmap_s		mappic;
	menubitmap_s		arrows;
	menubitmap_s		up;
	menubitmap_s		down;
	menutext_s			status;
	menutext_s			statusbar;

	menubitmap_s		remove;
	menubitmap_s		back;
	menubitmap_s		refresh;
	menubitmap_s		specify;
	menubitmap_s		create;
	menubitmap_s		go;

	pinglist_t			pinglist[MAX_PINGREQUESTS];
	table_t				table[MAX_LISTBOXITEMS];
	const char*				items[MAX_LISTBOXITEMS];
	int					numqueriedservers;
	int					*numservers;
	servernode_t		*serverlist;	
	int					currentping;
	qboolean			refreshservers;
	int					nextpingtime;
	int					maxservers;
	int					refreshtime;
	char				favoriteaddresses[MAX_FAVORITESERVERS][MAX_ADDRESSLENGTH];
	int					numfavoriteaddresses;

	menulist_s		punkbuster;
	menubitmap_s	pblogo;
} arenaservers_t;

static arenaservers_t	s_arenaservers;


static servernode_t		s_globalserverlist[MAX_GLOBALSERVERS];
static int				s_numglobalservers;
static servernode_t		s_localserverlist[MAX_LOCALSERVERS];
static int				s_numlocalservers;
static servernode_t		s_favoriteserverlist[MAX_FAVORITESERVERS];
static int				s_numfavoriteservers;
static servernode_t		s_mplayerserverlist[MAX_GLOBALSERVERS];
static int				s_nummplayerservers;
static int				s_servertype;
static int				s_gametype;
static int				s_sortkey;
static int				s_emptyservers;
static int				s_fullservers;


/*
=================
ArenaServers_MaxPing
=================
*/
static int ArenaServers_MaxPing( void ) {
	int		maxPing;

	maxPing = FIXED_TO_INT(_UI_trap_Cvar_VariableValue( "cl_maxPing" ));
	if( maxPing < 100 ) {
		maxPing = 100;
	}
	return maxPing;
}


/*
=================
ArenaServers_Compare
=================
*/
static int QDECL ArenaServers_Compare( const void *arg1, const void *arg2 ) {
	gfixed			f1;
	gfixed			f2;
	servernode_t*	t1;
	servernode_t*	t2;

	t1 = (servernode_t *)arg1;
	t2 = (servernode_t *)arg2;

	switch( s_sortkey ) {
	case SORT_HOST:
		return Q_stricmp( t1->hostname, t2->hostname );

	case SORT_MAP:
		return Q_stricmp( t1->mapname, t2->mapname );

	case SORT_CLIENTS:
		f1 = MAKE_GFIXED(t1->maxclients - t1->numclients);
		if( f1 < GFIXED_0 ) {
			f1 = GFIXED_0;
		}

		f2 = MAKE_GFIXED(t2->maxclients - t2->numclients);
		if( f2 < GFIXED_0 ) {
			f2 = GFIXED_0;
		}

		if( f1 < f2 ) {
			return 1;
		}
		if( f1 == f2 ) {
			return 0;
		}
		return -1;

	case SORT_GAME:
		if( t1->gametype < t2->gametype ) {
			return -1;
		}
		if( t1->gametype == t2->gametype ) {
			return 0;
		}
		return 1;

	case SORT_PING:
		if( t1->pingtime < t2->pingtime ) {
			return -1;
		}
		if( t1->pingtime > t2->pingtime ) {
			return 1;
		}
		return Q_stricmp( t1->hostname, t2->hostname );
	}

	return 0;
}


/*
=================
ArenaServers_Go
=================
*/
static void ArenaServers_Go( void ) {
	servernode_t*	servernode;

	servernode = s_arenaservers.table[s_arenaservers.list.curvalue].servernode;
	if( servernode ) {
		_UI_trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", servernode->adrstr ) );
	}
}


/*
=================
ArenaServers_UpdatePicture
=================
*/
static void ArenaServers_UpdatePicture( void ) {
	static char		picname[64];
	servernode_t*	servernodeptr;

	if( !s_arenaservers.list.numitems ) {
		s_arenaservers.mappic.generic.name = NULL;
	}
	else {
		servernodeptr = s_arenaservers.table[s_arenaservers.list.curvalue].servernode;
		Com_sprintf( picname, sizeof(picname), "levelshots/%s.tga", servernodeptr->mapname );
		s_arenaservers.mappic.generic.name = picname;
	
	}

	// force shader update during draw
	s_arenaservers.mappic.shader = 0;
}


/*
=================
ArenaServers_UpdateMenu
=================
*/
static void ArenaServers_UpdateMenu( void ) {
	int				i;
	int				j;
	int				count;
	char*			buff;
	servernode_t*	servernodeptr;
	table_t*		tableptr;
	const char			*pingColor;

	if( s_arenaservers.numqueriedservers > 0 ) {
		// servers found
		if( s_arenaservers.refreshservers && ( s_arenaservers.currentping <= s_arenaservers.numqueriedservers ) ) {
			// show progress
			Com_sprintf( s_arenaservers.status.string, MAX_STATUSLENGTH, "%d of %d Arena Servers.", s_arenaservers.currentping, s_arenaservers.numqueriedservers);
			s_arenaservers.statusbar.string  = strdup("Press SPACE to stop");
			qsort( s_arenaservers.serverlist, *s_arenaservers.numservers, sizeof( servernode_t ), ArenaServers_Compare);
		}
		else {
			// all servers pinged - enable controls
			s_arenaservers.master.generic.flags		&= ~QMF_GRAYED;
			s_arenaservers.gametype.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.sortkey.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.showempty.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.showfull.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.list.generic.flags		&= ~QMF_GRAYED;
			s_arenaservers.refresh.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.go.generic.flags			&= ~QMF_GRAYED;
			s_arenaservers.punkbuster.generic.flags &= ~QMF_GRAYED;

			// update status bar
			if( s_servertype == AS_GLOBAL || s_servertype == AS_MPLAYER ) {
				s_arenaservers.statusbar.string = quake3worldMessage;
			}
			else {
				s_arenaservers.statusbar.string = strdup("");
			}

		}
	}
	else {
		// no servers found
		if( s_arenaservers.refreshservers ) {
			strcpy( s_arenaservers.status.string,"Scanning For Servers." );
			s_arenaservers.statusbar.string = strdup("Press SPACE to stop");

			// disable controls during refresh
			s_arenaservers.master.generic.flags		|= QMF_GRAYED;
			s_arenaservers.gametype.generic.flags	|= QMF_GRAYED;
			s_arenaservers.sortkey.generic.flags	|= QMF_GRAYED;
			s_arenaservers.showempty.generic.flags	|= QMF_GRAYED;
			s_arenaservers.showfull.generic.flags	|= QMF_GRAYED;
			s_arenaservers.list.generic.flags		|= QMF_GRAYED;
			s_arenaservers.refresh.generic.flags	|= QMF_GRAYED;
			s_arenaservers.go.generic.flags			|= QMF_GRAYED;
			s_arenaservers.punkbuster.generic.flags |= QMF_GRAYED;
		}
		else {
			if( s_arenaservers.numqueriedservers < 0 ) {
				strcpy(s_arenaservers.status.string,"No Response From Master Server." );
			}
			else {
				strcpy(s_arenaservers.status.string,"No Servers Found." );
			}

			// update status bar
			if( s_servertype == AS_GLOBAL || s_servertype == AS_MPLAYER ) {
				s_arenaservers.statusbar.string = quake3worldMessage;
			}
			else {
				s_arenaservers.statusbar.string = strdup("");
			}

			// end of refresh - set control state
			s_arenaservers.master.generic.flags		&= ~QMF_GRAYED;
			s_arenaservers.gametype.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.sortkey.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.showempty.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.showfull.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.list.generic.flags		|= QMF_GRAYED;
			s_arenaservers.refresh.generic.flags	&= ~QMF_GRAYED;
			s_arenaservers.go.generic.flags			|= QMF_GRAYED;
			s_arenaservers.punkbuster.generic.flags &= ~QMF_GRAYED;
		}

		// zero out list box
		s_arenaservers.list.numitems = 0;
		s_arenaservers.list.curvalue = 0;
		s_arenaservers.list.top      = 0;

		// update picture
		ArenaServers_UpdatePicture();
		return;
	}

	// build list box strings - apply culling filters
	servernodeptr = s_arenaservers.serverlist;
	count         = *s_arenaservers.numservers;
	for( i = 0, j = 0; i < count; i++, servernodeptr++ ) {
		tableptr = &s_arenaservers.table[j];
		tableptr->servernode = servernodeptr;
		buff = tableptr->buff;

		// can only cull valid results
		if( !s_emptyservers && !servernodeptr->numclients ) {
			continue;
		}

		if( !s_fullservers && ( servernodeptr->numclients == servernodeptr->maxclients ) ) {
			continue;
		}

		switch( s_gametype ) {
		case GAMES_ALL:
			break;

		case GAMES_FFA:
			if( servernodeptr->gametype != GT_FFA ) {
				continue;
			}
			break;

		case GAMES_TEAMPLAY:
			if( servernodeptr->gametype != GT_TEAM ) {
				continue;
			}
			break;

		case GAMES_TOURNEY:
			if( servernodeptr->gametype != GT_TOURNAMENT ) {
				continue;
			}
			break;

		case GAMES_CTF:
			if( servernodeptr->gametype != GT_CTF ) {
				continue;
			}
			break;
		}

		if( servernodeptr->pingtime < servernodeptr->minPing ) {
			pingColor = S_COLOR_BLUE;
		}
		else if( servernodeptr->maxPing && servernodeptr->pingtime > servernodeptr->maxPing ) {
			pingColor = S_COLOR_BLUE;
		}
		else if( servernodeptr->pingtime < 200 ) {
			pingColor = S_COLOR_GREEN;
		}
		else if( servernodeptr->pingtime < 400 ) {
			pingColor = S_COLOR_YELLOW;
		}
		else {
			pingColor = S_COLOR_RED;
		}

		Com_sprintf( buff, MAX_LISTBOXWIDTH, "%-20.20s %-12.12s %2d/%2d %-8.8s %3s %s%3d " S_COLOR_YELLOW "%s", 
			servernodeptr->hostname, servernodeptr->mapname, servernodeptr->numclients,
 			servernodeptr->maxclients, servernodeptr->gamename,
			netnames[servernodeptr->nettype], pingColor, servernodeptr->pingtime, servernodeptr->bPB ? "Yes" : "No" );
		j++;
	}

	s_arenaservers.list.numitems = j;
	s_arenaservers.list.curvalue = 0;
	s_arenaservers.list.top      = 0;

	// update picture
	ArenaServers_UpdatePicture();
}


/*
=================
ArenaServers_Remove
=================
*/
static void ArenaServers_Remove( void )
{
	int				i;
	servernode_t*	servernodeptr;
	table_t*		tableptr;

	if (!s_arenaservers.list.numitems)
		return;

	// remove selected item from display list
	// items are in scattered order due to sort and cull
	// perform delete on list box contents, resync all lists

	tableptr      = &s_arenaservers.table[s_arenaservers.list.curvalue];
	servernodeptr = tableptr->servernode;

	// find address in master list
	for (i=0; i<s_arenaservers.numfavoriteaddresses; i++)
		if (!Q_stricmp(s_arenaservers.favoriteaddresses[i],servernodeptr->adrstr))
				break;

	// delete address from master list
	if (i <= s_arenaservers.numfavoriteaddresses-1)
	{
		if (i < s_arenaservers.numfavoriteaddresses-1)
		{
			// shift items up
			memcpy( &s_arenaservers.favoriteaddresses[i], &s_arenaservers.favoriteaddresses[i+1], (s_arenaservers.numfavoriteaddresses - i - 1)*sizeof(MAX_ADDRESSLENGTH));
		}
		s_arenaservers.numfavoriteaddresses--;
	}	

	// find address in server list
	for (i=0; i<s_numfavoriteservers; i++)
		if (&s_favoriteserverlist[i] == servernodeptr)
				break;

	// delete address from server list
	if (i <= s_numfavoriteservers-1)
	{
		if (i < s_numfavoriteservers-1)
		{
			// shift items up
			memcpy( &s_favoriteserverlist[i], &s_favoriteserverlist[i+1], (s_numfavoriteservers - i - 1)*sizeof(servernode_t));
		}
		s_numfavoriteservers--;
	}	

	s_arenaservers.numqueriedservers = s_arenaservers.numfavoriteaddresses;
	s_arenaservers.currentping       = s_arenaservers.numfavoriteaddresses;
}


/*
=================
ArenaServers_Insert
=================
*/
static void ArenaServers_Insert( char* adrstr, char* info, int pingtime )
{
	servernode_t*	servernodeptr;
	const char*			s;
	int				i;


	if ((pingtime >= ArenaServers_MaxPing()) && (s_servertype != AS_FAVORITES))
	{
		// slow global or local servers do not get entered
		return;
	}

	if (*s_arenaservers.numservers >= s_arenaservers.maxservers) {
		// list full;
		servernodeptr = s_arenaservers.serverlist+(*s_arenaservers.numservers)-1;
	} else {
		// next slot
		servernodeptr = s_arenaservers.serverlist+(*s_arenaservers.numservers);
		(*s_arenaservers.numservers)++;
	}

	Q_strncpyz( servernodeptr->adrstr, adrstr, MAX_ADDRESSLENGTH );

	Q_strncpyz( servernodeptr->hostname, Info_ValueForKey( info, "hostname"), MAX_HOSTNAMELENGTH );
	Q_CleanStr( servernodeptr->hostname );
	Q_strupr( servernodeptr->hostname );

	Q_strncpyz( servernodeptr->mapname, Info_ValueForKey( info, "mapname"), MAX_MAPNAMELENGTH );
	Q_CleanStr( servernodeptr->mapname );
	Q_strupr( servernodeptr->mapname );

	servernodeptr->numclients = atoi( Info_ValueForKey( info, "clients") );
	servernodeptr->maxclients = atoi( Info_ValueForKey( info, "sv_maxclients") );
	servernodeptr->pingtime   = pingtime;
	servernodeptr->minPing    = atoi( Info_ValueForKey( info, "minPing") );
	servernodeptr->maxPing    = atoi( Info_ValueForKey( info, "maxPing") );
	servernodeptr->bPB = atoi( Info_ValueForKey( info, "punkbuster") );

	/*
	s = Info_ValueForKey( info, "nettype" );
	for (i=0; ;i++)
	{
		if (!netnames[i])
		{
			servernodeptr->nettype = 0;
			break;
		}
		else if (!Q_stricmp( netnames[i], s ))
		{
			servernodeptr->nettype = i;
			break;
		}
	}
	*/
	servernodeptr->nettype = atoi(Info_ValueForKey(info, "nettype"));

	s = Info_ValueForKey( info, "game");
	i = atoi( Info_ValueForKey( info, "gametype") );
	if( i < 0 ) {
		i = 0;
	}
	else if( i > 11 ) {
		i = 12;
	}
	if( *s ) {
		servernodeptr->gametype = i;//-1;
		Q_strncpyz( servernodeptr->gamename, s, sizeof(servernodeptr->gamename) );
	}
	else {
		servernodeptr->gametype = i;
		Q_strncpyz( servernodeptr->gamename, gamenames[i], sizeof(servernodeptr->gamename) );
	}
}


/*
=================
ArenaServers_InsertFavorites

Insert nonresponsive address book entries into display lists.
=================
*/
void ArenaServers_InsertFavorites( void )
{
	int		i;
	int		j;
	char	info[MAX_INFO_STRING];

	// resync existing results with new or deleted cvars
	info[0] = '\0';
	Info_SetValueForKey( info, "hostname", "No Response" );
	for (i=0; i<s_arenaservers.numfavoriteaddresses; i++)
	{
		// find favorite address in refresh list
		for (j=0; j<s_numfavoriteservers; j++)
			if (!Q_stricmp(s_arenaservers.favoriteaddresses[i],s_favoriteserverlist[j].adrstr))
				break;

		if ( j >= s_numfavoriteservers)
		{
			// not in list, add it
			ArenaServers_Insert( s_arenaservers.favoriteaddresses[i], info, ArenaServers_MaxPing() );
		}
	}
}


/*
=================
ArenaServers_LoadFavorites

Load cvar address book entries into local lists.
=================
*/
void ArenaServers_LoadFavorites( void )
{
	int				i;
	int				j;
	int				numtempitems;
	char			emptyinfo[MAX_INFO_STRING];
	char			adrstr[MAX_ADDRESSLENGTH];
	servernode_t	templist[MAX_FAVORITESERVERS];
	qboolean		found;

	found        = qfalse;
	emptyinfo[0] = '\0';

	// copy the old
	memcpy( templist, s_favoriteserverlist, sizeof(servernode_t)*MAX_FAVORITESERVERS );
	numtempitems = s_numfavoriteservers;

	// clear the current for sync
	memset( s_favoriteserverlist, 0, sizeof(servernode_t)*MAX_FAVORITESERVERS );
	s_numfavoriteservers = 0;

	// resync existing results with new or deleted cvars
	for (i=0; i<MAX_FAVORITESERVERS; i++)
	{
		_UI_trap_Cvar_VariableStringBuffer( va("server%d",i+1), adrstr, MAX_ADDRESSLENGTH );
		if (!adrstr[0])
			continue;

		// quick sanity check to avoid slow domain name resolving
		// first character must be numeric
		if (adrstr[0] < '0' || adrstr[0] > '9')
			continue;

		// favorite server addresses must be maintained outside refresh list
		// this mimics local and global netadr's stored in client
		// these can be fetched to fill ping list
		strcpy( s_arenaservers.favoriteaddresses[s_numfavoriteservers], adrstr );

		// find this server in the old list
		for (j=0; j<numtempitems; j++)
			if (!Q_stricmp( templist[j].adrstr, adrstr ))
				break;

		if (j < numtempitems)
		{
			// found server - add exisiting results
			memcpy( &s_favoriteserverlist[s_numfavoriteservers], &templist[j], sizeof(servernode_t) );
			found = qtrue;
		}
		else
		{
			// add new server
			Q_strncpyz( s_favoriteserverlist[s_numfavoriteservers].adrstr, adrstr, MAX_ADDRESSLENGTH );
			s_favoriteserverlist[s_numfavoriteservers].pingtime = ArenaServers_MaxPing();
		}

		s_numfavoriteservers++;
	}

	s_arenaservers.numfavoriteaddresses = s_numfavoriteservers;

	if (!found)
	{
		// no results were found, reset server list
		// list will be automatically refreshed when selected
		s_numfavoriteservers = 0;
	}
}


/*
=================
ArenaServers_StopRefresh
=================
*/
static void ArenaServers_StopRefresh( void )
{
	if (!s_arenaservers.refreshservers)
		// not currently refreshing
		return;

	s_arenaservers.refreshservers = qfalse;

	if (s_servertype == AS_FAVORITES)
	{
		// nonresponsive favorites must be shown
		ArenaServers_InsertFavorites();
	}

	// final tally
	if (s_arenaservers.numqueriedservers >= 0)
	{
		s_arenaservers.currentping       = *s_arenaservers.numservers;
		s_arenaservers.numqueriedservers = *s_arenaservers.numservers; 
	}
	
	// sort
	qsort( s_arenaservers.serverlist, *s_arenaservers.numservers, sizeof( servernode_t ), ArenaServers_Compare);

	ArenaServers_UpdateMenu();
}


/*
=================
ArenaServers_DoRefresh
=================
*/
static void ArenaServers_DoRefresh( void )
{
	int		i;
	int		j;
	int		time;
	int		maxPing;
	char	adrstr[MAX_ADDRESSLENGTH];
	char	info[MAX_INFO_STRING];

	if (uis.realtime < s_arenaservers.refreshtime)
	{
	  if (s_servertype != AS_FAVORITES) {
			if (s_servertype == AS_LOCAL) {
				if (!_UI_trap_LAN_GetServerCount(s_servertype)) {
					return;
				}
			}
			if (_UI_trap_LAN_GetServerCount(s_servertype) < 0) {
			  // still waiting for response
			  return;
			}
	  }
	}

	if (uis.realtime < s_arenaservers.nextpingtime)
	{
		// wait for time trigger
		return;
	}

	// trigger at 10Hz intervals
	s_arenaservers.nextpingtime = uis.realtime + 10;

	// process ping results
	maxPing = ArenaServers_MaxPing();
	for (i=0; i<MAX_PINGREQUESTS; i++)
	{
		_UI_trap_LAN_GetPing( i, adrstr, MAX_ADDRESSLENGTH, &time );
		if (!adrstr[0])
		{
			// ignore empty or pending pings
			continue;
		}

		// find ping result in our local list
		for (j=0; j<MAX_PINGREQUESTS; j++)
			if (!Q_stricmp( adrstr, s_arenaservers.pinglist[j].adrstr ))
				break;

		if (j < MAX_PINGREQUESTS)
		{
			// found it
			if (!time)
			{
				time = uis.realtime - s_arenaservers.pinglist[j].start;
				if (time < maxPing)
				{
					// still waiting
					continue;
				}
			}

			if (time > maxPing)
			{
				// stale it out
				info[0] = '\0';
				time    = maxPing;
			}
			else
			{
				_UI_trap_LAN_GetPingInfo( i, info, MAX_INFO_STRING );
			}

			// insert ping results
			ArenaServers_Insert( adrstr, info, time );

			// clear this query from internal list
			s_arenaservers.pinglist[j].adrstr[0] = '\0';
   		}

		// clear this query from external list
		_UI_trap_LAN_ClearPing( i );
	}

	// get results of servers query
	// counts can increase as servers respond
	if (s_servertype == AS_FAVORITES) {
	  s_arenaservers.numqueriedservers = s_arenaservers.numfavoriteaddresses;
	} else {
	  s_arenaservers.numqueriedservers = _UI_trap_LAN_GetServerCount(s_servertype);
	}

//	if (s_arenaservers.numqueriedservers > s_arenaservers.maxservers)
//		s_arenaservers.numqueriedservers = s_arenaservers.maxservers;

	// send ping requests in reasonable bursts
	// iterate ping through all found servers
	for (i=0; i<MAX_PINGREQUESTS && s_arenaservers.currentping < s_arenaservers.numqueriedservers; i++)
	{
		if (_UI_trap_LAN_GetPingQueueCount() >= MAX_PINGREQUESTS)
		{
			// ping queue is full
			break;
		}

		// find empty slot
		for (j=0; j<MAX_PINGREQUESTS; j++)
			if (!s_arenaservers.pinglist[j].adrstr[0])
				break;

		if (j >= MAX_PINGREQUESTS)
			// no empty slots available yet - wait for timeout
			break;

		// get an address to ping

		if (s_servertype == AS_FAVORITES) {
		  strcpy( adrstr, s_arenaservers.favoriteaddresses[s_arenaservers.currentping] ); 		
		} else {
		  _UI_trap_LAN_GetServerAddressString(s_servertype, s_arenaservers.currentping, adrstr, MAX_ADDRESSLENGTH );
		}

		strcpy( s_arenaservers.pinglist[j].adrstr, adrstr );
		s_arenaservers.pinglist[j].start = uis.realtime;

		_UI_trap_Cmd_ExecuteText( EXEC_NOW, va( "ping %s\n", adrstr )  );
		
		// advance to next server
		s_arenaservers.currentping++;
	}

	if (!_UI_trap_LAN_GetPingQueueCount())
	{
		// all pings completed
		ArenaServers_StopRefresh();
		return;
	}

	// update the user interface with ping status
	ArenaServers_UpdateMenu();
}


/*
=================
ArenaServers_StartRefresh
=================
*/
static void ArenaServers_StartRefresh( void )
{
	int		i;
	char	myargs[32], protocol[32];

	memset( s_arenaservers.serverlist, 0, s_arenaservers.maxservers*sizeof(table_t) );

	for (i=0; i<MAX_PINGREQUESTS; i++)
	{
		s_arenaservers.pinglist[i].adrstr[0] = '\0';
		_UI_trap_LAN_ClearPing( i );
	}

	s_arenaservers.refreshservers    = qtrue;
	s_arenaservers.currentping       = 0;
	s_arenaservers.nextpingtime      = 0;
	*s_arenaservers.numservers       = 0;
	s_arenaservers.numqueriedservers = 0;

	// allow max 5 seconds for responses
	s_arenaservers.refreshtime = uis.realtime + 5000;

	// place menu in zeroed state
	ArenaServers_UpdateMenu();

	if( s_servertype == AS_LOCAL ) {
		_UI_trap_Cmd_ExecuteText( EXEC_APPEND, "localservers\n" );
		return;
	}

	if( s_servertype == AS_GLOBAL || s_servertype == AS_MPLAYER ) {
		if( s_servertype == AS_GLOBAL ) {
			i = 0;
		}
		else {
			i = 1;
		}

		switch( s_arenaservers.gametype.curvalue ) {
		default:
		case GAMES_ALL:
			myargs[0] = 0;
			break;

		case GAMES_FFA:
			strcpy( myargs, " ffa" );
			break;

		case GAMES_TEAMPLAY:
			strcpy( myargs, " team" );
			break;

		case GAMES_TOURNEY:
			strcpy( myargs, " tourney" );
			break;

		case GAMES_CTF:
			strcpy( myargs, " ctf" );
			break;
		}


		if (s_emptyservers) {
			strcat(myargs, " empty");
		}

		if (s_fullservers) {
			strcat(myargs, " full");
		}

		protocol[0] = '\0';
		_UI_trap_Cvar_VariableStringBuffer( "debus_protocol", protocol, sizeof(protocol) );
		if (strlen(protocol)) {
			_UI_trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %s%s\n", i, protocol, myargs ));
		}
		else {
			_UI_trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %d%s\n", i, FIXED_TO_INT(_UI_trap_Cvar_VariableValue( "protocol" )), myargs ) );
		}
	}
}


/*
=================
ArenaServers_SaveChanges
=================
*/
void ArenaServers_SaveChanges( void )
{
	int	i;

	for (i=0; i<s_arenaservers.numfavoriteaddresses; i++)
		_UI_trap_Cvar_Set( va("server%d",i+1), s_arenaservers.favoriteaddresses[i] );

	for (; i<MAX_FAVORITESERVERS; i++)
		_UI_trap_Cvar_Set( va("server%d",i+1), "" );
}


/*
=================
ArenaServers_Sort
=================
*/
void ArenaServers_Sort( int type ) {
	if( s_sortkey == type ) {
		return;
	}

	s_sortkey = type;
	qsort( s_arenaservers.serverlist, *s_arenaservers.numservers, sizeof( servernode_t ), ArenaServers_Compare);
}


/*
=================
ArenaServers_SetType
=================
*/
void ArenaServers_SetType( int type )
{
	if (s_servertype == type)
		return;

	s_servertype = type;

	switch( type ) {
	default:
	case AS_LOCAL:
		s_arenaservers.remove.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_arenaservers.serverlist = s_localserverlist;
		s_arenaservers.numservers = &s_numlocalservers;
		s_arenaservers.maxservers = MAX_LOCALSERVERS;
		break;

	case AS_GLOBAL:
		s_arenaservers.remove.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_arenaservers.serverlist = s_globalserverlist;
		s_arenaservers.numservers = &s_numglobalservers;
		s_arenaservers.maxservers = MAX_GLOBALSERVERS;
		break;

	case AS_FAVORITES:
		s_arenaservers.remove.generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
		s_arenaservers.serverlist = s_favoriteserverlist;
		s_arenaservers.numservers = &s_numfavoriteservers;
		s_arenaservers.maxservers = MAX_FAVORITESERVERS;
		break;

	case AS_MPLAYER:
		s_arenaservers.remove.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_arenaservers.serverlist = s_mplayerserverlist;
		s_arenaservers.numservers = &s_nummplayerservers;
		s_arenaservers.maxservers = MAX_GLOBALSERVERS;
		break;
		
	}

	if( !*s_arenaservers.numservers ) {
		ArenaServers_StartRefresh();
	}
	else {
		// avoid slow operation, use existing results
		s_arenaservers.currentping       = *s_arenaservers.numservers;
		s_arenaservers.numqueriedservers = *s_arenaservers.numservers; 
		ArenaServers_UpdateMenu();
	}
	strcpy(s_arenaservers.status.string,"hit refresh to update");
}

/*
=================
PunkBuster_Confirm
=================
*/
static void Punkbuster_ConfirmEnable( qboolean result ) {
	if (result)
	{		
		_UI_trap_SetPbClStatus(1);
	}
	s_arenaservers.punkbuster.curvalue = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED_1, MAKE_GFIXED(_UI_trap_Cvar_VariableValue( "cl_punkbuster" )) ));
}

static void Punkbuster_ConfirmDisable( qboolean result ) {
	if (result)
	{
		_UI_trap_SetPbClStatus(0);
		UI_Message( punkbuster_msg );
	}
	s_arenaservers.punkbuster.curvalue = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED_1, MAKE_GFIXED(_UI_trap_Cvar_VariableValue( "cl_punkbuster" )) ));
}

/*
=================
ArenaServers_Event
=================
*/
static void ArenaServers_Event( void* ptr, int event ) {
	int		id;
	int value;

	id = ((menucommon_s*)ptr)->id;

	if( event != QM_ACTIVATED && id != ID_LIST ) {
		return;
	}

	switch( id ) {
	case ID_MASTER:
		value = s_arenaservers.master.curvalue;
		if (value >= 1)
		{
			value++;
		}
		_UI_trap_Cvar_SetValue( "ui_browserMaster", MAKE_LFIXED(value) );
		ArenaServers_SetType( value );
		break;

	case ID_GAMETYPE:
		_UI_trap_Cvar_SetValue( "ui_browserGameType", MAKE_LFIXED(s_arenaservers.gametype.curvalue) );
		s_gametype = s_arenaservers.gametype.curvalue;
		ArenaServers_UpdateMenu();
		break;

	case ID_SORTKEY:
		_UI_trap_Cvar_SetValue( "ui_browserSortKey", MAKE_LFIXED(s_arenaservers.sortkey.curvalue) );
		ArenaServers_Sort( s_arenaservers.sortkey.curvalue );
		ArenaServers_UpdateMenu();
		break;

	case ID_SHOW_FULL:
		_UI_trap_Cvar_SetValue( "ui_browserShowFull", MAKE_LFIXED(s_arenaservers.showfull.curvalue) );
		s_fullservers = s_arenaservers.showfull.curvalue;
		ArenaServers_UpdateMenu();
		break;

	case ID_SHOW_EMPTY:
		_UI_trap_Cvar_SetValue( "ui_browserShowEmpty", MAKE_LFIXED(s_arenaservers.showempty.curvalue) );
		s_emptyservers = s_arenaservers.showempty.curvalue;
		ArenaServers_UpdateMenu();
		break;

	case ID_LIST:
		if( event == QM_GOTFOCUS ) {
			ArenaServers_UpdatePicture();
		}
		break;

	case ID_SCROLL_UP:
		ScrollList_Key( &s_arenaservers.list, K_UPARROW );
		break;

	case ID_SCROLL_DOWN:
		ScrollList_Key( &s_arenaservers.list, K_DOWNARROW );
		break;

	case ID_BACK:
		ArenaServers_StopRefresh();
		ArenaServers_SaveChanges();
		UI_PopMenu();
		break;

	case ID_REFRESH:
		ArenaServers_StartRefresh();
		break;

	case ID_SPECIFY:
		UI_SpecifyServerMenu();
		break;

	case ID_CREATE:
		UI_StartServerMenu( qtrue );
		break;

	case ID_CONNECT:
		ArenaServers_Go();
		break;

	case ID_REMOVE:
		ArenaServers_Remove();
		ArenaServers_UpdateMenu();
		break;
	
	case ID_PUNKBUSTER:
		if (s_arenaservers.punkbuster.curvalue)			
		{
			UI_ConfirmMenu_Style( "Enable Punkbuster?",  UI_CENTER|UI_INVERSE|UI_SMALLFONT, (voidfunc_f)NULL, Punkbuster_ConfirmEnable );
		}
		else
		{
			UI_ConfirmMenu_Style( "Disable Punkbuster?", UI_CENTER|UI_INVERSE|UI_SMALLFONT, (voidfunc_f)NULL, Punkbuster_ConfirmDisable );
		}
		break;
	}
}


/*
=================
ArenaServers_MenuDraw
=================
*/
static void ArenaServers_MenuDraw( void )
{
	if (s_arenaservers.refreshservers)
		ArenaServers_DoRefresh();

	Menu_Draw( &s_arenaservers.menu );
}


/*
=================
ArenaServers_MenuKey
=================
*/
static sfxHandle_t ArenaServers_MenuKey( int key ) {
	if( key == K_SPACE  && s_arenaservers.refreshservers ) {
		ArenaServers_StopRefresh();	
		return menu_move_sound;
	}

	if( ( key == K_DEL || key == K_KP_DEL ) && ( s_servertype == AS_FAVORITES ) &&
		( Menu_ItemAtCursor( &s_arenaservers.menu) == &s_arenaservers.list ) ) {
		ArenaServers_Remove();
		ArenaServers_UpdateMenu();
		return menu_move_sound;
	}

	if( key == K_MOUSE2 || key == K_ESCAPE ) {
		ArenaServers_StopRefresh();
		ArenaServers_SaveChanges();
	}


	return Menu_DefaultKey( &s_arenaservers.menu, key );
}


/*
=================
ArenaServers_MenuInit
=================
*/
static void ArenaServers_MenuInit( void ) {
	int			i;
	int			type;
	int			y;
	int			value;
	static char	statusbuffer[MAX_STATUSLENGTH];

	// zero set all our globals
	memset( &s_arenaservers, 0 ,sizeof(arenaservers_t) );

	ArenaServers_Cache();

	s_arenaservers.menu.fullscreen = qtrue;
	s_arenaservers.menu.wrapAround = qtrue;
	s_arenaservers.menu.draw       = ArenaServers_MenuDraw;
	s_arenaservers.menu.key        = ArenaServers_MenuKey;

	s_arenaservers.banner.generic.type  = MTYPE_BTEXT;
	s_arenaservers.banner.generic.flags = QMF_CENTER_JUSTIFY;
	s_arenaservers.banner.generic.x	    = 320;
	s_arenaservers.banner.generic.y	    = 16;
	s_arenaservers.banner.string  		= strdup("ARENA SERVERS");
	s_arenaservers.banner.style  	    = UI_CENTER;
	s_arenaservers.banner.color  	    = color_white;

	y = 80;
	s_arenaservers.master.generic.type			= MTYPE_SPINCONTROL;
	s_arenaservers.master.generic.name			= "Servers:";
	s_arenaservers.master.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_arenaservers.master.generic.callback		= ArenaServers_Event;
	s_arenaservers.master.generic.id			= ID_MASTER;
	s_arenaservers.master.generic.x				= 320;
	s_arenaservers.master.generic.y				= y;
	s_arenaservers.master.itemnames				= master_items;

	y += SMALLCHAR_HEIGHT;
	s_arenaservers.gametype.generic.type		= MTYPE_SPINCONTROL;
	s_arenaservers.gametype.generic.name		= "Game Type:";
	s_arenaservers.gametype.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_arenaservers.gametype.generic.callback	= ArenaServers_Event;
	s_arenaservers.gametype.generic.id			= ID_GAMETYPE;
	s_arenaservers.gametype.generic.x			= 320;
	s_arenaservers.gametype.generic.y			= y;
	s_arenaservers.gametype.itemnames			= servertype_items;

	y += SMALLCHAR_HEIGHT;
	s_arenaservers.sortkey.generic.type			= MTYPE_SPINCONTROL;
	s_arenaservers.sortkey.generic.name			= "Sort By:";
	s_arenaservers.sortkey.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_arenaservers.sortkey.generic.callback		= ArenaServers_Event;
	s_arenaservers.sortkey.generic.id			= ID_SORTKEY;
	s_arenaservers.sortkey.generic.x			= 320;
	s_arenaservers.sortkey.generic.y			= y;
	s_arenaservers.sortkey.itemnames			= sortkey_items;

	y += SMALLCHAR_HEIGHT;
	s_arenaservers.showfull.generic.type		= MTYPE_RADIOBUTTON;
	s_arenaservers.showfull.generic.name		= "Show Full:";
	s_arenaservers.showfull.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_arenaservers.showfull.generic.callback	= ArenaServers_Event;
	s_arenaservers.showfull.generic.id			= ID_SHOW_FULL;
	s_arenaservers.showfull.generic.x			= 320;
	s_arenaservers.showfull.generic.y			= y;

	y += SMALLCHAR_HEIGHT;
	s_arenaservers.showempty.generic.type		= MTYPE_RADIOBUTTON;
	s_arenaservers.showempty.generic.name		= "Show Empty:";
	s_arenaservers.showempty.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_arenaservers.showempty.generic.callback	= ArenaServers_Event;
	s_arenaservers.showempty.generic.id			= ID_SHOW_EMPTY;
	s_arenaservers.showempty.generic.x			= 320;
	s_arenaservers.showempty.generic.y			= y;

	y += 3 * SMALLCHAR_HEIGHT;
	s_arenaservers.list.generic.type			= MTYPE_SCROLLLIST;
	s_arenaservers.list.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_arenaservers.list.generic.id				= ID_LIST;
	s_arenaservers.list.generic.callback		= ArenaServers_Event;
	s_arenaservers.list.generic.x				= 72;
	s_arenaservers.list.generic.y				= y;
	s_arenaservers.list.width					= MAX_LISTBOXWIDTH;
	s_arenaservers.list.height					= 11;
	s_arenaservers.list.itemnames				= (const char **)s_arenaservers.items;
	for( i = 0; i < MAX_LISTBOXITEMS; i++ ) {
		s_arenaservers.items[i] = s_arenaservers.table[i].buff;
	}

	s_arenaservers.mappic.generic.type			= MTYPE_BITMAP;
	s_arenaservers.mappic.generic.flags			= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_arenaservers.mappic.generic.x				= 72;
	s_arenaservers.mappic.generic.y				= 80;
	s_arenaservers.mappic.width					= 128;
	s_arenaservers.mappic.height				= 96;
	s_arenaservers.mappic.errorpic				= ART_UNKNOWNMAP;

	s_arenaservers.arrows.generic.type			= MTYPE_BITMAP;
	s_arenaservers.arrows.generic.name			= ART_ARROWS0;
	s_arenaservers.arrows.generic.flags			= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_arenaservers.arrows.generic.callback		= ArenaServers_Event;
	s_arenaservers.arrows.generic.x				= 512+48;
	s_arenaservers.arrows.generic.y				= 240-64+16;
	s_arenaservers.arrows.width					= 64;
	s_arenaservers.arrows.height				= 128;

	s_arenaservers.up.generic.type				= MTYPE_BITMAP;
	s_arenaservers.up.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_arenaservers.up.generic.callback			= ArenaServers_Event;
	s_arenaservers.up.generic.id				= ID_SCROLL_UP;
	s_arenaservers.up.generic.x					= 512+48;
	s_arenaservers.up.generic.y					= 240-64+16;
	s_arenaservers.up.width						= 64;
	s_arenaservers.up.height					= 64;
	s_arenaservers.up.focuspic					= ART_ARROWS_UP;

	s_arenaservers.down.generic.type			= MTYPE_BITMAP;
	s_arenaservers.down.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_arenaservers.down.generic.callback		= ArenaServers_Event;
	s_arenaservers.down.generic.id				= ID_SCROLL_DOWN;
	s_arenaservers.down.generic.x				= 512+48;
	s_arenaservers.down.generic.y				= 240+16;
	s_arenaservers.down.width					= 64;
	s_arenaservers.down.height					= 64;
	s_arenaservers.down.focuspic				= ART_ARROWS_DOWN;

	y = 376;
	s_arenaservers.status.generic.type		= MTYPE_TEXT;
	s_arenaservers.status.generic.x			= 320;
	s_arenaservers.status.generic.y			= y;
	s_arenaservers.status.string			= statusbuffer;
	s_arenaservers.status.style				= UI_CENTER|UI_SMALLFONT;
	s_arenaservers.status.color				= menu_text_color;

	y += SMALLCHAR_HEIGHT;
	s_arenaservers.statusbar.generic.type   = MTYPE_TEXT;
	s_arenaservers.statusbar.generic.x	    = 320;
	s_arenaservers.statusbar.generic.y	    = y;
	s_arenaservers.statusbar.string	        = strdup("");
	s_arenaservers.statusbar.style	        = UI_CENTER|UI_SMALLFONT;
	s_arenaservers.statusbar.color	        = text_color_normal;

	s_arenaservers.remove.generic.type		= MTYPE_BITMAP;
	s_arenaservers.remove.generic.name		= ART_REMOVE0;
	s_arenaservers.remove.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arenaservers.remove.generic.callback	= ArenaServers_Event;
	s_arenaservers.remove.generic.id		= ID_REMOVE;
	s_arenaservers.remove.generic.x			= 450;
	s_arenaservers.remove.generic.y			= 86;
	s_arenaservers.remove.width				= 96;
	s_arenaservers.remove.height			= 48;
	s_arenaservers.remove.focuspic			= ART_REMOVE1;

	s_arenaservers.back.generic.type		= MTYPE_BITMAP;
	s_arenaservers.back.generic.name		= ART_BACK0;
	s_arenaservers.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arenaservers.back.generic.callback	= ArenaServers_Event;
	s_arenaservers.back.generic.id			= ID_BACK;
	s_arenaservers.back.generic.x			= 0;
	s_arenaservers.back.generic.y			= 480-64;
	s_arenaservers.back.width				= 128;
	s_arenaservers.back.height				= 64;
	s_arenaservers.back.focuspic			= ART_BACK1;

	s_arenaservers.specify.generic.type	    = MTYPE_BITMAP;
	s_arenaservers.specify.generic.name		= ART_SPECIFY0;
	s_arenaservers.specify.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arenaservers.specify.generic.callback = ArenaServers_Event;
	s_arenaservers.specify.generic.id	    = ID_SPECIFY;
	s_arenaservers.specify.generic.x		= 128;
	s_arenaservers.specify.generic.y		= 480-64;
	s_arenaservers.specify.width  		    = 128;
	s_arenaservers.specify.height  		    = 64;
	s_arenaservers.specify.focuspic         = ART_SPECIFY1;

	s_arenaservers.refresh.generic.type		= MTYPE_BITMAP;
	s_arenaservers.refresh.generic.name		= ART_REFRESH0;
	s_arenaservers.refresh.generic.flags	= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arenaservers.refresh.generic.callback	= ArenaServers_Event;
	s_arenaservers.refresh.generic.id		= ID_REFRESH;
	s_arenaservers.refresh.generic.x		= 256;
	s_arenaservers.refresh.generic.y		= 480-64;
	s_arenaservers.refresh.width			= 128;
	s_arenaservers.refresh.height			= 64;
	s_arenaservers.refresh.focuspic			= ART_REFRESH1;

	s_arenaservers.create.generic.type		= MTYPE_BITMAP;
	s_arenaservers.create.generic.name		= ART_CREATE0;
	s_arenaservers.create.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arenaservers.create.generic.callback	= ArenaServers_Event;
	s_arenaservers.create.generic.id		= ID_CREATE;
	s_arenaservers.create.generic.x			= 384;
	s_arenaservers.create.generic.y			= 480-64;
	s_arenaservers.create.width				= 128;
	s_arenaservers.create.height			= 64;
	s_arenaservers.create.focuspic			= ART_CREATE1;

	s_arenaservers.go.generic.type			= MTYPE_BITMAP;
	s_arenaservers.go.generic.name			= ART_CONNECT0;
	s_arenaservers.go.generic.flags			= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arenaservers.go.generic.callback		= ArenaServers_Event;
	s_arenaservers.go.generic.id			= ID_CONNECT;
	s_arenaservers.go.generic.x				= 640;
	s_arenaservers.go.generic.y				= 480-64;
	s_arenaservers.go.width					= 128;
	s_arenaservers.go.height				= 64;
	s_arenaservers.go.focuspic				= ART_CONNECT1;

	s_arenaservers.punkbuster.generic.type			= MTYPE_SPINCONTROL;
	s_arenaservers.punkbuster.generic.name			= "Punkbuster:";
	s_arenaservers.punkbuster.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_arenaservers.punkbuster.generic.callback		= ArenaServers_Event;
	s_arenaservers.punkbuster.generic.id			= ID_PUNKBUSTER;
	s_arenaservers.punkbuster.generic.x				= 480+32;
	s_arenaservers.punkbuster.generic.y				= 144;
	s_arenaservers.punkbuster.itemnames				= punkbuster_items;
	
	s_arenaservers.pblogo.generic.type			= MTYPE_BITMAP;
	s_arenaservers.pblogo.generic.name			= ART_PUNKBUSTER;
	s_arenaservers.pblogo.generic.flags			= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_arenaservers.pblogo.generic.x				= 526;
	s_arenaservers.pblogo.generic.y				= 176;
	s_arenaservers.pblogo.width					= 32;
	s_arenaservers.pblogo.height				= 16;
	s_arenaservers.pblogo.errorpic				= ART_UNKNOWNMAP;
	
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.banner );

	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.master );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.gametype );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.sortkey );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.showfull);
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.showempty );

	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.mappic );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.list );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.status );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.statusbar );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.arrows );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.up );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.down );

	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.remove );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.back );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.specify );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.refresh );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.create );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.go );

	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.punkbuster );
	Menu_AddItem( &s_arenaservers.menu, (void*) &s_arenaservers.pblogo );
	
	ArenaServers_LoadFavorites();

	s_servertype = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED(3,0), MAKE_GFIXED(ui_browserMaster.integer) ));
	// hack to get rid of MPlayer stuff
	value = s_servertype;
	if (value >= 1)
		value--;
	s_arenaservers.master.curvalue = value;

	s_gametype = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED(4,0), MAKE_GFIXED(ui_browserGameType.integer) ));
	s_arenaservers.gametype.curvalue = s_gametype;

	s_sortkey = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED(4,0), MAKE_GFIXED(ui_browserSortKey.integer) ));
	s_arenaservers.sortkey.curvalue = s_sortkey;

	s_fullservers = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED_1, MAKE_GFIXED(ui_browserShowFull.integer) ));
	s_arenaservers.showfull.curvalue = s_fullservers;

	s_emptyservers = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED_1, MAKE_GFIXED(ui_browserShowEmpty.integer) ));
	s_arenaservers.showempty.curvalue = s_emptyservers;
	
	s_arenaservers.punkbuster.curvalue = FIXED_TO_INT(Com_Clamp( GFIXED_0, GFIXED_1, MAKE_GFIXED(_UI_trap_Cvar_VariableValue( "cl_punkbuster" )) ));

	// force to initial state and refresh
	type = s_servertype;
	s_servertype = -1;
	ArenaServers_SetType( type );

	_UI_trap_Cvar_Register(NULL, "debus_protocol", "", 0 );
}


/*
=================
ArenaServers_Cache
=================
*/
void ArenaServers_Cache( void ) {
	_UI_trap_R_RegisterShaderNoMip( ART_BACK0 );
	_UI_trap_R_RegisterShaderNoMip( ART_BACK1 );
	_UI_trap_R_RegisterShaderNoMip( ART_CREATE0 );
	_UI_trap_R_RegisterShaderNoMip( ART_CREATE1 );
	_UI_trap_R_RegisterShaderNoMip( ART_SPECIFY0 );
	_UI_trap_R_RegisterShaderNoMip( ART_SPECIFY1 );
	_UI_trap_R_RegisterShaderNoMip( ART_REFRESH0 );
	_UI_trap_R_RegisterShaderNoMip( ART_REFRESH1 );
	_UI_trap_R_RegisterShaderNoMip( ART_CONNECT0 );
	_UI_trap_R_RegisterShaderNoMip( ART_CONNECT1 );
	_UI_trap_R_RegisterShaderNoMip( ART_ARROWS0  );
	_UI_trap_R_RegisterShaderNoMip( ART_ARROWS_UP );
	_UI_trap_R_RegisterShaderNoMip( ART_ARROWS_DOWN );
	_UI_trap_R_RegisterShaderNoMip( ART_UNKNOWNMAP );
	_UI_trap_R_RegisterShaderNoMip( ART_PUNKBUSTER );
}


/*
=================
UI_ArenaServersMenu
=================
*/
void UI_ArenaServersMenu( void ) {
	ArenaServers_MenuInit();
	UI_PushMenu( &s_arenaservers.menu );
}						  
