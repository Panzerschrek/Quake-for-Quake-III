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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//

#include "g_local.h"

level_locals_t	level;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
} cvarTable_t;

vmCvar_t	g_maxclients;
vmCvar_t	g_speed;
vmCvar_t	g_debugAlloc;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;

vmCvar_t	teamplay;
vmCvar_t	skill;
vmCvar_t	deathmatch;
vmCvar_t	coop;
vmCvar_t	fraglimit;
vmCvar_t	timelimit;

vmCvar_t	sv_maxvelocity;
vmCvar_t	sv_gravity;
vmCvar_t	sv_nostep;
vmCvar_t	sv_friction;
vmCvar_t	sv_edgefriction;
vmCvar_t	sv_stopspeed;
vmCvar_t	sv_maxspeed;
vmCvar_t	sv_accelerate;
vmCvar_t	sv_idealpitchscale;
vmCvar_t	sv_aim;

vmCvar_t	cl_rollspeed;
vmCvar_t	cl_rollangle;

vmCvar_t	g_registered;

// Pass save file in teporary cvar.
vmCvar_t g_server_start_save_file;

static cvarTable_t		gameCvarTable[] = {
	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "gamedate", PRODUCT_DATE , CVAR_ROM, 0, qfalse  },

	// latched vars
	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },

	{ &g_speed, "g_speed", "320", 0, 0 },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0 },

	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0 },
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0 },

	// TODO - set proper flags.
	{ &teamplay, "teamplay", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0 },
	{ &skill, "skill", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &deathmatch, "deathmatch", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0 },
	{ &coop, "coop", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0 },
	{ &fraglimit, "fraglimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },

	{ &sv_maxvelocity, "sv_maxvelocity", "2000", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_gravity, "sv_gravity", "800", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_nostep, "sv_nostep", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_friction, "sv_friction", "4", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_edgefriction, "sv_edgefriction", "2", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_stopspeed, "sv_stopspeed", "100", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_maxspeed, "sv_maxspeed", "320", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_accelerate, "sv_accelerate", "10", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_idealpitchscale, "sv_idealpitchscale", "0.8", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &sv_aim, "sv_aim", "0.93", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },

	{ &cl_rollspeed, "cl_rollspeed", "200", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },
	{ &cl_rollangle, "cl_rollangle", "2.0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0 },

	// PANZER TODO - know exact version (trial/registered). Or maybe just say "funck you" to Bethesda and treat the game as registered only?
	{ &g_registered, "registered", "1.0", CVAR_ARCHIVE, 0 },

	{ &g_server_start_save_file, "g_server_start_save_file", "", CVAR_TEMP, 0 },
};

static int gameCvarTableSize = ARRAY_LEN( gameCvarTable );

double host_frametime;

void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {
	switch ( command ) {
	case GAME_INIT:
		G_InitGame( arg0, arg1, arg2 );
		return 0;
	case GAME_SHUTDOWN:
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (intptr_t)ClientConnect( arg0, arg1, arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		ClientUserinfoChanged( arg0 );
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		return 0;
	case GAME_CLIENT_BEGIN:
		ClientBegin( arg0 );
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		return 0;
	case GAME_RUN_FRAME:
		G_RunFrame( arg0 );
		return 0;
	case GAME_CONSOLE_COMMAND:
		return ConsoleCommand();
	case BOTAI_START_FRAME:
		return 0;
	}

	return -1;
}


void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL G_DPrintf( const char *fmt, ... ) {
	// TODO - implement it
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Error( text );
}


/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;
	}
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"", 
						cv->cvarName, cv->vmCvar->string ) );
				}
			}
		}
	}
}

void G_SetResourcesConfig()
{
	int		i;
	for (i=0 ; i < MAX_MODELS; i++)
	{
		if (!sv.model_precache[i])
			continue;

		trap_SetConfigstring(CS_MODELS + i, sv.model_precache[i]);
	}

	for (i=0 ; i < MAX_SOUNDS; i++)
	{
		if (!sv.sound_precache[i])
			continue;

		trap_SetConfigstring(CS_SOUNDS + i, sv.sound_precache[i]);
	}

	{
		char musicIndex[16];
		Com_sprintf(musicIndex, sizeof(musicIndex), "%d", (int)sv.edicts->v.sounds);
		trap_SetConfigstring(CS_MUSIC, musicIndex);
	}
}

void SV_SpawnServer()
{
	edict_t		*ent;
	char		mapname[MAX_OSPATH];
	char		mapname_corrected[MAX_OSPATH];
	int			i;

	trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
	Com_sprintf(mapname_corrected, sizeof(mapname_corrected), "maps/%s.bsp", mapname);

	//
	// make cvars consistant
	//
	if (coop.value)
		trap_Cvar_Set ("deathmatch", "0");
	current_skill = (int)(skill.value + 0.5);
	if (current_skill < 0)
		current_skill = 0;
	if (current_skill > 3)
		current_skill = 3;

	{
		char skill_value_str[2];
		skill_value_str[0] = '0' + current_skill;
		skill_value_str[1]= 0;
		trap_Cvar_Set ("skill", skill_value_str);
	}

	memset (&sv, 0, sizeof(sv));

	strcpy(sv.name, mapname);

	// load progs to get entity field count
	PR_LoadProgs ();

	// allocate server memory
	sv.max_edicts = MAX_EDICTS;

	sv.edicts = G_Alloc (sv.max_edicts*pr_edict_size);

	sv.datagram.maxsize = sizeof(sv.datagram_buf);
	sv.datagram.cursize = 0;
	sv.datagram.data = sv.datagram_buf;

	sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
	sv.reliable_datagram.cursize = 0;
	sv.reliable_datagram.data = sv.reliable_datagram_buf;

	sv.signon.maxsize = sizeof(sv.signon_buf);
	sv.signon.cursize = 0;
	sv.signon.data = sv.signon_buf;

	sv.state = ss_loading;

	// leave slots at start for clients only
	sv.num_edicts = svs.maxclients+1;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;
		ent->s.number = i+1;
	}

	sv.time = 1.0;

	// let the server system know where the entites are
	trap_LocateGameData(
		sv.edicts, sv.max_edicts, pr_edict_size,
		&svs.clients[0].ps, sizeof( svs.clients[0] ) );

	sv.sound_precache[0] = pr_strings;
	sv.model_precache[0] = pr_strings;

	//
	// load the rest of the entities
	//
	ent = EDICT_NUM(0);
	memset (&ent->v, 0, progs->entityfields * 4);
	ent->v.model = ED_NewString (mapname_corrected) - pr_strings;
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop.value)
		pr_global_struct->coop = coop.value;
	else
		pr_global_struct->deathmatch = deathmatch.value;

	G_SpawnEntitiesFromString();

	sv.active = qtrue;

	// all setup is completed, any further precache statements are errors
	sv.state = ss_active;

	// run two frames to allow everything to settle
	host_frametime = 0.1;
	SV_Physics ();
	SV_Physics ();

	if (g_server_start_save_file.string[0] != 0)
	{
		trap_Cvar_Set("g_server_start_save_file", "");
		G_Printf("Detected game loading request, save name: %s\n", g_server_start_save_file.string);

		// Free spawned edicts to spawn them again during loading of saved game.
		for (i = svs.maxclients  + 1; i < sv.num_edicts; ++i)
			ED_Free(EDICT_NUM(i));

		G_LoadGame(g_server_start_save_file.string);
		sv.loadgame = qtrue;
	}
	else
		sv.loadgame = qfalse;

}

/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAMEVERSION);
	G_Printf ("gamedate: %s\n", PRODUCT_DATE);

	srand( randomSeed );

	G_InitMemory();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	// initialize all clients for this game
	svs.maxclientslimit = svs.maxclients = 8;
	svs.clients = G_Alloc (svs.maxclientslimit*sizeof(client_t));
	memset( svs.clients, 0, svs.maxclientslimit * sizeof(svs.clients[0]) );

	G_RegisterCvars();

	PR_Init();

	SV_SpawnServer();

	G_SetResourcesConfig();

	G_Printf ("-----------------------------------\n");
}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	G_Printf ("==== ShutdownGame ====\n");
}



//===================================================================

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	sec = ( level.time - level.startTime ) / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	Q_vsnprintf(string + 7, sizeof(string) - 7, fmt, argptr);
	va_end( argptr );
}

/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int			i, j;
	edict_t		*edict;
	edict_t		*other;
	edict_t		*event;
	client_t	*client;

	host_frametime = ( levelTime - level.time ) / 1000.0;

	// don't allow really long or short frames
	if (host_frametime > 0.1)
		host_frametime = 0.1;
	if (host_frametime < 0.001)
		host_frametime = 0.001;

	pr_global_struct->frametime = host_frametime;
	level.time = levelTime;

	// get any cvar changes
	G_UpdateCvars();

	// Run Quake1 physics (for all entities)
	SV_Physics();

	// Process messages generated via "MSG_Write*" functions, convert these messages into event entities.
	SV_ProcessMessages();

	// Free old events
	for (i=0 ; i< sv.num_edicts; i++) {
		edict = EDICT_NUM(i);
		if ( edict->free ) {
			continue;
		}
		if( edict->s.event != 0 && level.time - edict->eventTime > EVENT_VALID_MSEC )
		{
			edict->s.event = 0;
			ED_Free(edict);
			continue;
		}
	}

	// Update client data struct for every edict.
	for (i=0 ; i< sv.num_edicts; i++) {
		edict = EDICT_NUM(i);
		if ( edict->free ) {
			continue;
		}

		VectorCopy(edict->v.origin, edict->s.origin);
		VectorCopy(edict->v.angles, edict->s.angles);
		edict->s.modelindex= edict->v.model == 0 ? 0 : edict->v.modelindex;
		edict->s.frame = edict->v.frame;
	}

	for(i = 0; i < svs.maxclients; i++){
		client = &svs.clients[i];
		if(!client->active || client->edict == NULL) {
			continue;
		}

		client->ps.clientNum = i;

		edict = svs.clients[i].edict;

		if ( edict->v.fixangle )
		{
			VectorCopy(edict->v.angles, edict->v.v_angle);
			edict->v.fixangle = 0;
		}

		if (edict->v.dmg_take || edict->v.dmg_save)
		{
			other = PROG_TO_EDICT(edict->v.dmg_inflictor);

			event= G_CreateEventEdict(edict->v.origin, svc_damage);
			for (j=0 ; j<3 ; j++)
				event->s.origin2[j]= other->v.origin[j] + 0.5*(other->v.mins[j] + other->v.maxs[j]);
			event->s.constantLight= (((int)edict->v.dmg_save) & 255) | ((((int)edict->v.dmg_take) & 255) << 8);
			edict->v.dmg_take = 0;
			edict->v.dmg_save = 0;
		}

		VectorCopy(edict->v.origin, client->ps.origin);
		VectorCopy(edict->v.v_angle, client->ps.viewangles);
		VectorCopy(edict->v.velocity, client->ps.velocity);
		client->ps.viewheight = edict->v.view_ofs[2];
		client->ps.weapon = SV_ModelIndex(pr_strings + edict->v.weaponmodel);
		client->ps.weaponstate = edict->v.weaponframe; // Put weapon frame into "weaponstate" field.
		client->ps.stats[STAT_HEALTH] = edict->v.health;
		client->ps.stats[Q3_STAT_ARMOR] = edict->v.armorvalue;
		client->ps.stats[STAT_ACTIVE_WEAPON] = edict->v.weapon;
		client->ps.stats[Q3_STAT_CURRENT_AMMO] = edict->v.currentammo;
		client->ps.stats[Q3_STAT_SHELLS] = edict->v.ammo_shells;
		client->ps.stats[Q3_STAT_NAILS] = edict->v.ammo_nails;
		client->ps.stats[Q3_STAT_ROCKETS] = edict->v.ammo_rockets;
		client->ps.stats[Q3_STAT_CELLS] = edict->v.ammo_cells;
		client->ps.stats[Q3_STAT_TOTAL_SECRETS] = pr_global_struct->total_secrets;
		client->ps.stats[Q3_STAT_TOTAL_MONSTERS] = pr_global_struct->total_monsters;
		client->ps.stats[Q3_STAT_SECRETS] = pr_global_struct->found_secrets;
		client->ps.stats[Q3_STAT_MONSTERS] = pr_global_struct->killed_monsters;

		{
			eval_t* val;
			int items;
			val = GetEdictFieldValue(edict, "items2");

			if (val)
				items = (int)edict->v.items | ((int)val->_float << 23);
			else
				items = (int)edict->v.items | ((int)pr_global_struct->serverflags << 28);

			client->ps.stats[STAT_ITEMS_LO] = items & 65535;
			client->ps.stats[STAT_ITEMS_HI] = items >> 16;
		}
	}
}
