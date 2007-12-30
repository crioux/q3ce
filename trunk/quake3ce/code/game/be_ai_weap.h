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

/*****************************************************************************
 * name:		be_ai_weap.h
 *
 * desc:		weapon AI
 *
 * $Archive: /source/code/botlib/be_ai_weap.h $
 *
 *****************************************************************************/

//projectile flags
#define PFL_WINDOWDAMAGE			1		//projectile damages through window
#define PFL_RETURN					2		//set when projectile returns to owner
//weapon flags
#define WFL_FIRERELEASED			1		//set when projectile is fired with key-up event
//damage types
#define DAMAGETYPE_IMPACT			1		//damage on impact
#define DAMAGETYPE_RADIAL			2		//radial damage
#define DAMAGETYPE_VISIBLE			4		//damage to all entities visible to the projectile

typedef struct projectileinfo_s
{
	char name[MAX_STRINGFIELD];
	char model[MAX_STRINGFIELD];
	int flags;
	bfixed gravity;
	int damage;
	bfixed radius;
	int visdamage;
	int damagetype;
	int healthinc;
	gfixed push;
	gfixed detonation;
	gfixed bounce;
	gfixed bouncefric;
	gfixed bouncestop;
} projectileinfo_t;

typedef struct weaponinfo_s
{
	int valid;					//true if the weapon info is valid
	int number;									//number of the weapon
	char name[MAX_STRINGFIELD];
	char model[MAX_STRINGFIELD];
	int level;
	int weaponindex;
	int flags;
	char projectile[MAX_STRINGFIELD];
	int numprojectiles;
	gfixed hspread;
	gfixed vspread;
	gfixed speed;
	gfixed acceleration;
	bvec3_t recoil;
	bvec3_t offset;
	avec3_t angleoffset;
	gfixed extrazvelocity;
	int ammoamount;
	int ammoindex;
	gfixed activate;
	gfixed reload;
	gfixed spinup;
	gfixed spindown;
	projectileinfo_t proj;						//pointer to the used projectile
} weaponinfo_t;

//setup the weapon AI
int BotSetupWeaponAI(void);
//shut down the weapon AI
void BotShutdownWeaponAI(void);
//returns the best weapon to fight with
int BotChooseBestFightWeapon(int weaponstate, int *inventory);
//returns the information of the current weapon
void BotGetWeaponInfo(int weaponstate, int weapon, weaponinfo_t *weaponinfo);
//loads the weapon weights
int BotLoadWeaponWeights(int weaponstate, char *filename);
//returns a handle to a newly allocated weapon state
int BotAllocWeaponState(void);
//frees the weapon state
void BotFreeWeaponState(int weaponstate);
//resets the whole weapon state
void BotResetWeaponState(int weaponstate);
