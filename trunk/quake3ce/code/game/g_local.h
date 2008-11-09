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
// g_local.h -- local definitions for game module

#include "q_shared.h"
#include "bg_public.h"
#include "g_public.h"

//==================================================================

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"baseq3"

#define BODY_QUEUE_SIZE		8

#ifdef INFINITE
#undef INFINITE
#endif

#define INFINITE			1000000

#define	FRAMETIME			100					// msec
#define	CARNAGE_REWARD_TIME	3000
#define REWARD_SPRITE_TIME	2000

#define	INTERMISSION_DELAY_TIME	1000
#define	SP_INTERMISSION_DELAY_TIME	5000

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots
#define FL_FORCE_GESTURE		0x00008000	// force gesture on client

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

#define SP_PODIUM_MODEL		"models/mapobjects/podium/podium4.md3"

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s	*client;			// NULL if not a client

	qboolean	inuse;

	const char		*classname;			// set in QuakeEd
	int			spawnflags;			// set in QuakeEd

	qboolean	neverFree;			// if true, FreeEntity will only unlink
									// bodyque uses this

	int			flags;				// FL_* variables

	const char		*model;
	const char		*model2;
	int			freetime;			// level.time when the object was freed
	
	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

	qboolean	physicsObject;		// if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects, 
	gfixed		physicsBounce;		// GFIXED_1 = continuous bounce, GFIXED_0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	// movers
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*parent;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	bvec3_t		pos1, pos2;

	char		*message;

	int			timestamp;		// body queue sinking, etc

	afixed		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*team;
	char		*targetShaderName;
	char		*targetShaderNewName;
	gentity_t	*target_ent;

	bfixed		speed;
	avec3_t		movedir;

	int			nextthink;
	void		(*think)(gentity_t *self);
	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
	void		(*blocked)(gentity_t *self, gentity_t *other);
	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage);
	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	int			pain_debounce_time;
	int			fly_sound_debounce_time;	// wind tunnel
	int			last_move_time;

	int			health;

	qboolean	takedamage;

	int			damage;
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

	int			count;

	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team

#ifdef MISSIONPACK
	int			kamikazeTime;
	int			kamikazeShockTime;
#endif

	int			watertype;
	int			waterlevel;

	int			noise_index;

	// timing variables
	gfixed		wait;
	gfixed		random;

	gitem_t		*item;			// for bonus items
};


typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef struct {
	playerTeamStateState_t	state;

	int			location;

	int			captures;
	int			basedefense;
	int			carrierdefense;
	int			flagrecovery;
	int			fragcarrier;
	int			assists;

	gfixed		lasthurtcarrier;
	gfixed		lastreturnedflag;
	gfixed		flagsince;
	gfixed		lastfraggedcarrier;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
	team_t		sessionTeam;
	int			spectatorTime;		// for determining next-in-line to play
	spectatorState_t	spectatorState;
	int			spectatorClient;	// for chasecam and follow mode
	int			wins, losses;		// tournament stats
	qboolean	teamLeader;			// true when this client is a team leader
} clientSession_t;

//
#define MAX_NETNAME			36
#define	MAX_VOTE_COUNT		3

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t	connected;	
	usercmd_t	cmd;				// we would lose angles if not persistant
	qboolean	localClient;		// true if "ip" info key is "localhost"
	qboolean	initialSpawn;		// the first spawn should be at a cool location
	qboolean	predictItemPickup;	// based on cg_predictItems userinfo
	qboolean	pmoveFixed;			//
	char		netname[MAX_NETNAME];
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
	playerTeamState_t teamState;	// status in teamplay games
	int			voteCount;			// to prevent people from constantly calling votes
	int			teamVoteCount;		// to prevent people from constantly calling votes
	qboolean	teamInfo;			// send team overlay updates?
} clientPersistant_t;


// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	qboolean	readyToExit;		// wishes to leave the intermission

	qboolean	noclip;

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	bvec3_t		oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	avec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			accurateCount;		// for "impressive" reward sound

	int			accuracy_shots;		// total number of shots
	int			accuracy_hits;		// total number of hits

	//
	int			lastkilled_client;	// last client that this client killed
	int			lasthurt_client;	// last client that damaged this client
	int			lasthurt_mod;		// type of damage the client did

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given
	int			rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int			airOutTime;

	int			lastKillTime;		// for multiple kill rewards

	qboolean	fireHeld;			// used for hook
	gentity_t	*hook;				// grapple hook if out

	int			switchTeamTime;		// time the player switched teams

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

#ifdef MISSIONPACK
	gentity_t	*persistantPowerup;
	int			portalID;
	int			ammoTimes[WP_NUM_WEAPONS];
	int			invulnerabilityTime;
#endif

	char		*areabits;
};


//
// this structure is cleared as each map is entered
//
#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	4096

typedef struct {
	struct gclient_s	*clients;		// [maxclients]

	struct gentity_s	*gentities;
	int			gentitySize;
	int			num_entities;		// current number, <= MAX_GENTITIES

	int			warmupTime;			// restart match at this time

	fileHandle_t	logFile;

	// store latched cvars here that we want to get at often
	int			maxclients;

	int			framenum;
	int			time;					// in msec
	int			previousTime;			// so movers can back up when blocked

	int			startTime;				// level.time the map was started

	int			teamScores[TEAM_NUM_TEAMS];
	int			lastTeamLocationTime;		// last time of client team location update

	qboolean	newSession;				// don't use any old session data, because
										// we changed gametype

	qboolean	restarted;				// waiting for a map_restart to fire

	int			numConnectedClients;
	int			numNonSpectatorClients;	// includes connecting clients
	int			numPlayingClients;		// connected, non-spectators
	int			sortedClients[MAX_CLIENTS];		// sorted by score
	int			follow1, follow2;		// clientNums for auto-follow spectators

	int			snd_fry;				// sound index for standing in lava

	int			warmupModificationCount;	// for detecting if g_warmup is changed

	// voting state
	char		voteString[MAX_STRING_CHARS];
	char		voteDisplayString[MAX_STRING_CHARS];
	int			voteTime;				// level.time vote was called
	int			voteExecuteTime;		// time the vote is executed
	int			voteYes;
	int			voteNo;
	int			numVotingClients;		// set by CalculateRanks

	// team voting state
	char		teamVoteString[2][MAX_STRING_CHARS];
	int			teamVoteTime[2];		// level.time vote was called
	int			teamVoteYes[2];
	int			teamVoteNo[2];
	int			numteamVotingClients[2];// set by CalculateRanks

	// spawn variables
	qboolean	spawning;				// the G_Spawn*() functions are valid
	int			numSpawnVars;
	char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int			numSpawnVarChars;
	char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// intermission state
	int			intermissionQueued;		// intermission was qualified, but
										// wait INTERMISSION_DELAY_TIME before
										// actually going there so the last
										// frag can be watched.  Disable future
										// kills during this delay
	int			intermissiontime;		// time the intermission was started
	char		*changemap;
	qboolean	readyToExit;			// at least one client wants to exit
	int			exitTime;
	bvec3_t		intermission_origin;	// also used for spectator spawns
	avec3_t		intermission_angle;

	qboolean	locationLinked;			// target_locations get linked
	gentity_t	*locationHead;			// head of the location list
	int			bodyQueIndex;			// dead bodies
	gentity_t	*bodyQue[BODY_QUEUE_SIZE];
#ifdef MISSIONPACK
	int			portalSequence;
#endif
} level_locals_t;


//
// g_spawn.c
//
qboolean	G_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
#ifndef FIXED_IS_FLOAT
qboolean	G_SpawnFloat( const char *key, const char *defaultString, afixed *out );
qboolean	G_SpawnFloat( const char *key, const char *defaultString, bfixed *out );
#endif
qboolean	G_SpawnFloat( const char *key, const char *defaultString, gfixed *out );
qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	G_SpawnVector( const char *key, const char *defaultString, gfixed *out );
void		G_SpawnEntitiesFromString( void );
char *G_NewString( const char *string );

//
// g_cmds.c
//
void Cmd_Score_f (gentity_t *ent);
void StopFollowing( gentity_t *ent );
void BroadcastTeamChange( gclient_t *client, int oldTeam );
void SetTeam( gentity_t *ent, const char *s );
void Cmd_FollowCycle_f( gentity_t *ent, int dir );

//
// g_items.c
//
void G_CheckTeamItems( void );
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );

void UseHoldableItem( gentity_t *ent );
void PrecacheItem (gitem_t *it);
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, afixed angle );
gentity_t *LaunchItem( gitem_t *item, bvec3_t origin, bvec3_t velocity );
void SetRespawn (gentity_t *ent, gfixed delay);
void G_SpawnItem (gentity_t *ent, gitem_t *item);
void FinishSpawningItem( gentity_t *ent );
void Think_Weapon (gentity_t *ent);
int ArmorIndex (gentity_t *ent);
void	Add_Ammo (gentity_t *ent, int weapon, int count);
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );

//
// g_utils.c
//
int G_ModelIndex( const char *name );
int		G_SoundIndex( const char *name );
void	G_TeamCommand( team_t team, const char *cmd );
void	G_KillBox (gentity_t *ent);
gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
gentity_t *G_PickTarget (const char *targetname);
void	G_UseTargets (gentity_t *ent, gentity_t *activator);
void	G_SetMovedir ( avec3_t angles, avec3_t movedir);

void	G_InitGentity( gentity_t *e );
gentity_t	*G_Spawn (void);
gentity_t *G_TempEntity( bvec3_t origin, int event );
void	G_Sound( gentity_t *ent, int channel, int soundIndex );
void	G_FreeEntity( gentity_t *e );
qboolean	G_EntitiesFree( void );

void	G_TouchTriggers (gentity_t *ent);
void	G_TouchSolids (gentity_t *ent);

char	*vtos( const bvec3_t v );

afixed vectoyaw( const bvec3_t vec );

void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, bvec3_t origin );
void AddRemap(const char *oldShader, const char *newShader, gfixed timeOffset);
const char *BuildShaderStateConfig();

//
// g_combat.c
//
qboolean CanDamage (gentity_t *targ, bvec3_t origin);
void G_Damage (gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, avec3_t dir, bvec3_t point, int damage, int dflags, int mod);
qboolean G_RadiusDamage (bvec3_t origin, gentity_t *attacker, gfixed damage, bfixed radius, gentity_t *ignore, int mod);
int G_InvulnerabilityEffect( gentity_t *targ, avec3_t dir, bvec3_t point, bvec3_t impactpoint, bvec3_t bouncedir );
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void TossClientItems( gentity_t *self );
#ifdef MISSIONPACK
void TossClientPersistantPowerups( gentity_t *self );
#endif
void TossClientCubes( gentity_t *self );

// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK			0x00000004	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION		0x00000008  // armor, shields, invulnerability, and godmode have no effect
#ifdef MISSIONPACK
#define DAMAGE_NO_TEAM_PROTECTION	0x00000010  // armor, shields, invulnerability, and godmode have no effect
#endif

//
// g_missile.c
//
void G_RunMissile( gentity_t *ent );

gentity_t *fire_blaster (gentity_t *self, bvec3_t start, avec3_t aimdir);
gentity_t *fire_plasma (gentity_t *self, bvec3_t start, avec3_t aimdir);
gentity_t *fire_grenade (gentity_t *self, bvec3_t start, avec3_t aimdir);
gentity_t *fire_rocket (gentity_t *self, bvec3_t start, avec3_t dir);
gentity_t *fire_bfg (gentity_t *self, bvec3_t start, avec3_t dir);
gentity_t *fire_grapple (gentity_t *self, bvec3_t start, avec3_t dir);
#ifdef MISSIONPACK
gentity_t *fire_nail( gentity_t *self, bvec3_t start, avec3_t forward, avec3_t right, avec3_t up );
gentity_t *fire_prox( gentity_t *self, bvec3_t start, avec3_t aimdir );
#endif


//
// g_mover.c
//
void G_RunMover( gentity_t *ent );
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace );

//
// g_trigger.c
//
void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace );


//
// g_misc.c
//
void TeleportPlayer( gentity_t *player, bvec3_t origin, avec3_t angles );
#ifdef MISSIONPACK
void DropPortalSource( gentity_t *ent );
void DropPortalDestination( gentity_t *ent );
#endif


//
// g_weapon.c
//
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker );
void CalcMuzzlePoint ( gentity_t *ent, avec3_t forward, avec3_t right, avec3_t up, bvec3_t muzzlePoint );
void SnapVectorTowards( bvec3_t v, bvec3_t to );
qboolean CheckGauntletAttack( gentity_t *ent );
void Weapon_HookFree (gentity_t *ent);
void Weapon_HookThink (gentity_t *ent);


//
// g_client.c
//
team_t TeamCount( int ignoreClientNum, int team );
int TeamLeader( int team );
team_t PickTeam( int ignoreClientNum );
void SetClientViewAngle( gentity_t *ent, avec3_t angle );
gentity_t *SelectSpawnPoint ( bvec3_t avoidPoint, bvec3_t origin, avec3_t angles );
void CopyToBodyQue( gentity_t *ent );
void respawn (gentity_t *ent);
void BeginIntermission (void);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientSpawn( gentity_t *ent );
void player_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void AddScore( gentity_t *ent, bvec3_t origin, int score );
void CalculateRanks( void );
qboolean SpotWouldTelefrag( gentity_t *spot );

//
// g_svcmds.c
//
qboolean	ConsoleCommand( void );
void G_ProcessIPBans(void);
qboolean G_FilterPacket (const char *from);

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent );
#ifdef MISSIONPACK
void G_StartKamikaze( gentity_t *ent );
#endif

//
// p_hud.c
//
void MoveClientToIntermission (gentity_t *client);
void G_SetStats (gentity_t *ent);
void DeathmatchScoreboardMessage (gentity_t *client);

//
// g_cmds.c
//

//
// g_pweapon.c
//


//
// g_main.c
//
void FindIntermissionPoint( void );
void SetLeader(int team, int client);
void CheckTeamLeader( int team );
void G_RunThink (gentity_t *ent);
void QDECL G_LogPrintf( const char *fmt, ... );
void SendScoreboardMessageToAllClients( void );
void QDECL G_Printf( const char *fmt, ... );
void QDECL G_Error( const char *fmt, ... );

//
// g_client.c
//
const char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
void Team_CheckDroppedItem( gentity_t *dropped );
qboolean CheckObeliskAttack( gentity_t *obelisk, gentity_t *attacker );

//
// g_mem.c
//
void *G_Alloc( int size );
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadSessionData( gclient_t *client );
void G_InitSessionData( gclient_t *client, char *userinfo );

void G_InitWorldSession( void );
void G_WriteSessionData( void );

//
// g_arenas.c
//
void UpdateTournamentInfo( void );
void SpawnModelsOnVictoryPads( void );
void Svcmd_AbortPodium_f( void );

//
// g_bot.c
//
void G_InitBots( qboolean restart );
const char *G_GetBotInfoByNumber( int num );
const char *G_GetBotInfoByName( const char *name );
void G_CheckBotSpawn( void );
void G_RemoveQueuedBotBegin( int clientNum );
qboolean G_BotConnect( int clientNum, qboolean restart );
void Svcmd_AddBot_f( void );
void Svcmd_BotList_f( void );
void BotInterbreedEndMatch( void );

// ai_main.c
#define MAX_FILEPATH			144

//bot settings
typedef struct bot_settings_s
{
	char characterfile[MAX_FILEPATH];
	gfixed skill;
	char team[MAX_FILEPATH];
} bot_settings_t;

int BotAISetup( int restart );
int BotAIShutdown( int restart );
int BotAILoadMap( int restart );
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
int BotAIShutdownClient( int client, qboolean restart );
int BotAIStartFrame( int time );
void BotTestAAS(bvec3_t origin);

#include "g_team.h" // teamplay specific stuff


extern	level_locals_t	level;
extern	gentity_t		g_entities[MAX_GENTITIES];

#define	FOFS(x) ((int)&(((gentity_t *)0)->x))

extern	vmCvar_t	g_gametype;
extern	vmCvar_t	g_dedicated;
extern	vmCvar_t	g_cheats;
extern	vmCvar_t	g_maxclients;			// allow this many total, including spectators
extern	vmCvar_t	g_maxGameClients;		// allow this many active
extern	vmCvar_t	g_restarted;

extern	vmCvar_t	g_dmflags;
extern	vmCvar_t	g_fraglimit;
extern	vmCvar_t	g_timelimit;
extern	vmCvar_t	g_capturelimit;
extern	vmCvar_t	g_friendlyFire;
extern	vmCvar_t	g_password;
extern	vmCvar_t	g_needpass;
extern	vmCvar_t	g_gravity;
extern	vmCvar_t	g_speed;
extern	vmCvar_t	g_knockback;
extern	vmCvar_t	g_quadfactor;
extern	vmCvar_t	g_forcerespawn;
extern	vmCvar_t	g_inactivity;
extern	vmCvar_t	g_debugMove;
extern	vmCvar_t	g_debugAlloc;
extern	vmCvar_t	g_debugDamage;
extern	vmCvar_t	g_weaponRespawn;
extern	vmCvar_t	g_weaponTeamRespawn;
extern	vmCvar_t	g_synchronousClients;
extern	vmCvar_t	g_motd;
extern	vmCvar_t	g_warmup;
extern	vmCvar_t	g_doWarmup;
extern	vmCvar_t	g_blood;
extern	vmCvar_t	g_allowVote;
extern	vmCvar_t	g_teamAutoJoin;
extern	vmCvar_t	g_teamForceBalance;
extern	vmCvar_t	g_banIPs;
extern	vmCvar_t	g_filterBan;
extern	vmCvar_t	g_obeliskHealth;
extern	vmCvar_t	g_obeliskRegenPeriod;
extern	vmCvar_t	g_obeliskRegenAmount;
extern	vmCvar_t	g_obeliskRespawnDelay;
extern	vmCvar_t	g_cubeTimeout;
extern	vmCvar_t	g_redteam;
extern	vmCvar_t	g_blueteam;
extern	vmCvar_t	g_smoothClients;
extern	vmCvar_t	pmove_fixed;
extern	vmCvar_t	pmove_msec;
extern	vmCvar_t	g_rankings;
extern	vmCvar_t	g_enableDust;
extern	vmCvar_t	g_enableBreath;
extern	vmCvar_t	g_singlePlayer;
extern	vmCvar_t	g_proxMineTimeout;

void	_G_trap_Printf( const char *fmt );
void	_G_trap_Error( const char *fmt );
int		_G_trap_Milliseconds( void );
int		_G_trap_Argc( void );
void	_G_trap_Argv( int n, char *buffer, int bufferLength );
void	_G_trap_Args( char *buffer, int bufferLength );
int		_G_trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	_G_trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	_G_trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void	_G_trap_FS_FCloseFile( fileHandle_t f );
int		_G_trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int		_G_trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t
void	_G_trap_SendConsoleCommand( int exec_when, const char *text );
void	_G_trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void	_G_trap_Cvar_Update( vmCvar_t *cvar );
void	_G_trap_Cvar_Set( const char *var_name, const char *value );
int		_G_trap_Cvar_VariableIntegerValue( const char *var_name );
gfixed	_G_trap_Cvar_VariableValue( const char *var_name );
void	_G_trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	_G_trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
void	_G_trap_DropClient( int clientNum, const char *reason );
void	_G_trap_SendServerCommand( int clientNum, const char *text );
void	_G_trap_SetConfigstring( int num, const char *string );
void	_G_trap_GetConfigstring( int num, char *buffer, int bufferSize );
void	_G_trap_GetUserinfo( int num, char *buffer, int bufferSize );
void	_G_trap_SetUserinfo( int num, const char *buffer );
void	_G_trap_GetServerinfo( char *buffer, int bufferSize );
void	_G_trap_SetBrushModel( gentity_t *ent, const char *name );
void	_G_trap_Trace( trace_t *results, const bvec3_t start, const bvec3_t mins, const bvec3_t maxs, const bvec3_t end, int passEntityNum, int contentmask );
int		_G_trap_PointContents( const bvec3_t point, int passEntityNum );
qboolean _G_trap_InPVS( const bvec3_t p1, const bvec3_t p2 );
qboolean _G_trap_InPVSIgnorePortals( const bvec3_t p1, const bvec3_t p2 );
void	_G_trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean _G_trap_AreasConnected( int area1, int area2 );
void	_G_trap_LinkEntity( gentity_t *ent );
void	_G_trap_UnlinkEntity( gentity_t *ent );
int		_G_trap_EntitiesInBox( const bvec3_t mins, const bvec3_t maxs, int *entityList, int maxcount );
qboolean _G_trap_EntityContact( const bvec3_t mins, const bvec3_t maxs, const gentity_t *ent );
int		_G_trap_BotAllocateClient( void );
void	_G_trap_BotFreeClient( int clientNum );
void	_G_trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean	_G_trap_GetEntityToken( char *buffer, int bufferSize );

int		_G_trap_DebugPolygonCreate(int color, int numPoints, bvec3_t *points);
void	_G_trap_DebugPolygonDelete(int id);

int		_G_trap_BotLibSetup( void );
int		_G_trap_BotLibShutdown( void );
int		_G_trap_BotLibVarSet(const char *var_name, const char *value);
int		_G_trap_BotLibVarGet(const char *var_name, char *value, int size);
int		_G_trap_BotLibDefine(const char *string);
int		_G_trap_BotLibStartFrame(gfixed time);
int		_G_trap_BotLibLoadMap(const char *mapname);
int		_G_trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int		_G_trap_BotLibTest(int parm0, char *parm1, bvec3_t parm2, avec3_t parm3);

int		_G_trap_BotGetSnapshotEntity( int clientNum, int sequence );
int		_G_trap_BotGetServerCommand(int clientNum, char *message, int size);
void	_G_trap_BotUserCommand(int client, usercmd_t *ucmd);

int		_G_trap_AAS_BBoxAreas(bvec3_t absmins, bvec3_t absmaxs, int *areas, int maxareas);
int		_G_trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info );
void	_G_trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);

int		_G_trap_AAS_Initialized(void);
void	_G_trap_AAS_PresenceTypeBoundingBox(int presencetype, bvec3_t mins, bvec3_t maxs);
gfixed	_G_trap_AAS_Time(void);

int		_G_trap_AAS_PointAreaNum(bvec3_t point);
int		_G_trap_AAS_PointReachabilityAreaIndex(bvec3_t point);
int		_G_trap_AAS_TraceAreas(bvec3_t start, bvec3_t end, int *areas, bvec3_t *points, int maxareas);

int		_G_trap_AAS_PointContents(bvec3_t point);
int		_G_trap_AAS_NextBSPEntity(int ent);
int		_G_trap_AAS_ValueForBSPEpairKey(int ent, const char *key, char *value, int size);
int		_G_trap_AAS_VectorForBSPEpairKey(int ent, const char *key, bvec3_t v);
int		_G_trap_AAS_FloatForBSPEpairKey(int ent, const char *key, gfixed *value);
int		_G_trap_AAS_IntForBSPEpairKey(int ent, const char *key, int *value);

int		_G_trap_AAS_AreaReachability(int areanum);

int		_G_trap_AAS_AreaTravelTimeToGoalArea(int areanum, bvec3_t origin, int goalareanum, int travelflags);
int		_G_trap_AAS_EnableRoutingArea( int areanum, int enable );
int		_G_trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, bvec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum);

int		_G_trap_AAS_AlternativeRouteGoals(bvec3_t start, int startareanum, bvec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type);
int		_G_trap_AAS_Swimming(bvec3_t origin);
int		_G_trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, bvec3_t origin, int presencetype, int onground, bvec3_t velocity, bvec3_t cmdmove, int cmdframes, int maxframes, gfixed frametime, int stopevent, int stopareanum, int visualize);


void	_G_trap_EA_Say(int client, const char *str);
void	_G_trap_EA_SayTeam(int client, const char *str);
void	_G_trap_EA_Command(int client, const char *command);

void	_G_trap_EA_Action(int client, int action);
void	_G_trap_EA_Gesture(int client);
void	_G_trap_EA_Talk(int client);
void	_G_trap_EA_Attack(int client);
void	_G_trap_EA_Use(int client);
void	_G_trap_EA_Respawn(int client);
void	_G_trap_EA_Crouch(int client);
void	_G_trap_EA_MoveUp(int client);
void	_G_trap_EA_MoveDown(int client);
void	_G_trap_EA_MoveForward(int client);
void	_G_trap_EA_MoveBack(int client);
void	_G_trap_EA_MoveLeft(int client);
void	_G_trap_EA_MoveRight(int client);
void	_G_trap_EA_SelectWeapon(int client, int weapon);
void	_G_trap_EA_Jump(int client);
void	_G_trap_EA_DelayedJump(int client);
void	_G_trap_EA_Move(int client, avec3_t dir, bfixed speed);
void	_G_trap_EA_View(int client, avec3_t viewangles);

void	_G_trap_EA_EndRegular(int client, gfixed thinktime);
void	_G_trap_EA_GetInput(int client, gfixed thinktime, void /* struct bot_input_s */ *input);
void	_G_trap_EA_ResetInput(int client);


int		_G_trap_BotLoadCharacter(const char *charfile, gfixed skill);
void	_G_trap_BotFreeCharacter(int character);
gfixed	_G_trap_Characteristic_Float(int character, int index);
gfixed	_G_trap_Characteristic_BFloat(int character, int index, gfixed min, gfixed max);
int		_G_trap_Characteristic_Integer(int character, int index);
int		_G_trap_Characteristic_BInteger(int character, int index, int min, int max);
void	_G_trap_Characteristic_String(int character, int index, char *buf, int size);

int		_G_trap_BotAllocChatState(void);
void	_G_trap_BotFreeChatState(int handle);
void	_G_trap_BotQueueConsoleMessage(int chatstate, int type, const char *message);
void	_G_trap_BotRemoveConsoleMessage(int chatstate, int handle);
int		_G_trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
int		_G_trap_BotNumConsoleMessages(int chatstate);
void	_G_trap_BotInitialChat(int chatstate, const char *type, int mcontext, const char *var0, const char *var1, const char *var2, const char *var3, const char *var4, const char *var5, const char *var6, const char *var7 );
int		_G_trap_BotNumInitialChats(int chatstate, const char *type);
int		_G_trap_BotReplyChat(int chatstate, const char *message, int mcontext, int vcontext, const char *var0, const char *var1, const char *var2, const char *var3, const char *var4, const char *var5, const char *var6, const char *var7 );
int		_G_trap_BotChatLength(int chatstate);
void	_G_trap_BotEnterChat(int chatstate, int client, int sendto);
void	_G_trap_BotGetChatMessage(int chatstate, char *buf, int size);
int		_G_trap_StringContains(const char *str1, const char *str2, int casesensitive);
int		_G_trap_BotFindMatch(const char *str, void /* struct bot_match_s */ *match, unsigned long int context);
void	_G_trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
void	_G_trap_UnifyWhiteSpaces(char *string);
void	_G_trap_BotReplaceSynonyms(char *string, unsigned long int context);
int		_G_trap_BotLoadChatFile(int chatstate, const char *chatfile, const char *chatname);
void	_G_trap_BotSetChatGender(int chatstate, int gender);
void	_G_trap_BotSetChatName(int chatstate, const char *name, int client);
void	_G_trap_BotResetGoalState(int goalstate);
void	_G_trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void	_G_trap_BotResetAvoidGoals(int goalstate);
void	_G_trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void	_G_trap_BotPopGoal(int goalstate);
void	_G_trap_BotEmptyGoalStack(int goalstate);
void	_G_trap_BotDumpAvoidGoals(int goalstate);
void	_G_trap_BotDumpGoalStack(int goalstate);
void	_G_trap_BotGoalName(int number, char *name, int size);
int		_G_trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		_G_trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		_G_trap_BotChooseLTGItem(int goalstate, bvec3_t origin, int *inventory, int travelflags);
int		_G_trap_BotChooseNBGItem(int goalstate, bvec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, gfixed maxtime);
int		_G_trap_BotTouchingGoal(bvec3_t origin, void /* struct bot_goal_s */ *goal);
int		_G_trap_BotItemGoalInVisButNotVisible(int viewer, bvec3_t eye, avec3_t viewangles, void /* struct bot_goal_s */ *goal);
int		_G_trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
int		_G_trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
int		_G_trap_BotGetLevelItemGoal(int index, const char *classname, void /* struct bot_goal_s */ *goal);
gfixed	_G_trap_BotAvoidGoalTime(int goalstate, int number);
void	_G_trap_BotSetAvoidGoalTime(int goalstate, int number, gfixed avoidtime);
void	_G_trap_BotInitLevelItems(void);
void	_G_trap_BotUpdateEntityItems(void);
int	_G_trap_BotLoadItemWeights(int goalstate, const char *filename);
void	_G_trap_BotFreeItemWeights(int goalstate);
void	_G_trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void	_G_trap_BotSaveGoalFuzzyLogic(int goalstate, const char *filename);
void	_G_trap_BotMutateGoalFuzzyLogic(int goalstate, gfixed range);
int		_G_trap_BotAllocGoalState(int state);
void	_G_trap_BotFreeGoalState(int handle);

void	_G_trap_BotResetMoveState(int movestate);
void	_G_trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags);
int		_G_trap_BotMoveInDirection(int movestate, avec3_t dir, bfixed speed, int type);
void	_G_trap_BotResetAvoidReach(int movestate);
void	_G_trap_BotResetLastAvoidReach(int movestate);
int		_G_trap_BotReachabilityArea(bvec3_t origin, int testground);
int		_G_trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, bfixed lookahead, bvec3_t target);
int		_G_trap_BotPredictVisiblePosition(bvec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, bvec3_t target);
int		_G_trap_BotAllocMoveState(void);
void	_G_trap_BotFreeMoveState(int handle);
void	_G_trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
void	_G_trap_BotAddAvoidSpot(int movestate, bvec3_t origin, bfixed radius, int type);

int		_G_trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void	_G_trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
int		_G_trap_BotLoadWeaponWeights(int weaponstate, const char *filename);
int		_G_trap_BotAllocWeaponState(void);
void	_G_trap_BotFreeWeaponState(int weaponstate);
void	_G_trap_BotResetWeaponState(int weaponstate);

int		_G_trap_GeneticParentsAndChildSelection(int numranks, gfixed *ranks, int *parent1, int *parent2, int *child);

