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

gclient_t		g_clients[MAX_CLIENTS];

vmCvar_t	g_maxclients;
vmCvar_t	g_speed;
vmCvar_t	g_debugAlloc;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;

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

void G_SetModelsConfig()
{
	int		i;
	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
			continue;

		trap_SetConfigstring(CS_MODELS + i, sv.model_precache[i]);
	}
}

void SV_SpawnServer()
{
	memset (&sv, 0, sizeof(sv));

	// load progs to get entity field count
	PR_LoadProgs ();

	// allocate server memory
	sv.max_edicts = MAX_EDICTS;

	sv.edicts = G_Alloc (sv.max_edicts*pr_edict_size);

	sv.state = ss_loading;

	sv.time = 1.0;

	if (coop.value)
		pr_global_struct->coop = coop.value;
	else
		pr_global_struct->deathmatch = deathmatch.value;

	// PANZER TODO - set mapname

	G_SpawnEntitiesFromString();
	pr_global_struct->world = 1;

	sv.active = qtrue;

	// all setup is completed, any further precache statements are errors
	sv.state = ss_active;

	// PANZER TODO - fix it
	// run two frames to allow everything to settle
	host_frametime = 0.1;
	//SV_Physics ();
	//SV_Physics ();
}

/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int					i;

	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAMEVERSION);
	G_Printf ("gamedate: %s\n", PRODUCT_DATE);

	srand( randomSeed );

	G_RegisterCvars();

	G_InitMemory();

	PR_Init();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	SV_SpawnServer();

	// initialize all clients for this game
	memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	level.clients = g_clients;

	// let the server system know where the entites are
	trap_LocateGameData(
		sv.edicts, sv.max_edicts, pr_edict_size,
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	G_SetModelsConfig();

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
	int			i;
	edict_t		*edict;

	host_frametime = ( levelTime - level.time ) / 1000.0;
	pr_global_struct->frametime = host_frametime;
	level.time = levelTime;

	// get any cvar changes
	G_UpdateCvars();

	// Run Quake1 physics (for all entities)
	SV_Physics();

	// Update client data struct for every edict.
	for (i=0 ; i< sv.num_edicts; i++) {
		edict = EDICT_NUM(i);
		if ( edict->free ) {
			continue;
		}

		VectorCopy(edict->v.origin, edict->s.origin);
		VectorCopy(edict->v.angles, edict->s.angles);
		edict->s.modelindex= edict->v.modelindex;
		edict->s.frame = edict->v.frame;
	}
}
