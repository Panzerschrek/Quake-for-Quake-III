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
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include "q_client.h"


void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1, arg2 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return 0;
	case CG_LAST_ATTACKER:
		return 0;
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, arg1);
		return 0;
	case CG_MOUSE_EVENT:
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0);
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}

// PANZER TODO - init this.
qboolean		standard_quake, rogue, hipnotic;

cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
sbar_t				sbar;

client_state_t		cl;

vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_sbar_scale;
vmCvar_t	cg_sbar_lines;

vmCvar_t	teamplay; // PANZER TODO - register it?

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_sbar_scale, "cg_sbar_scale", "1", CVAR_ARCHIVE },
	{ &cg_sbar_lines, "cg_sbar_lines", "2", CVAR_ARCHIVE },
};

static int  cvarTableSize = ARRAY_LEN( cvarTable );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Update( cv->vmCvar );
	}

	trap_Cvar_Set( "teamoverlay", "0" );
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Error( int level, const char *error, ... ) {
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
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}

static void CG_StartMusic(int index) {
	char trackName[64]= "music/track00";

	trackName[11]= '0' + index / 10;
	trackName[12]= '0' + index % 10;
	Com_Printf("Starting music track \"%s\"\n", trackName);
	trap_S_StartBackgroundTrack(trackName, trackName);
}


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterResources( void ) {
	int		i, j;

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	trap_R_LoadWorldMap( cgs.mapname );

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		char	modelName[MAX_OSPATH];
		int n;

		strcpy(modelName, CG_ConfigString( CS_MODELS+i ));
		if ( !modelName[0] ) {
			break;
		}

		// Fix model names - replace "mdl" extension with "md3" - native Quake3 model format.
		n= strlen(modelName);
		if(
			(n >= 4 && modelName[n-4] == '.' && modelName[n-3] == 'm' && modelName[n-2] == 'd' && modelName[n-1] == 'l') ||
			(n >= 4 && modelName[n-4] == '.' && modelName[n-3] == 'b' && modelName[n-2] == 's' && modelName[n-1] == 'p'))
		{
			modelName[n-3]= 'm';
			modelName[n-2]= 'd';
			modelName[n-1]= '3';
			modelName[n-4] = 0;
		}

		cgs.gameModels[i] = trap_R_RegisterModel( modelName );
	}

	// register all the server specified sounds
	for (i=1 ; i< MAX_SOUNDS; i++) {
		const char* soundName;
		char	soundFileName[MAX_OSPATH];
		soundName = CG_ConfigString( CS_SOUNDS + i );
		if ( !soundName[0] ) {
			break;
		}
		Com_sprintf(soundFileName, sizeof(soundFileName), "sound/%s", soundName);

		cgs.gameSounds[i] = trap_S_RegisterSound( soundFileName, qfalse );
	}

	{
		const char* musicIndexStr = CG_ConfigString(CS_MUSIC);
		if(musicIndexStr && musicIndexStr[0] )
		{
			int music_index = atoi(musicIndexStr);
			if(music_index > 0 && music_index < 100 )
				CG_StartMusic(music_index);
		}
	}
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	*s;

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof( cg_entities ) );

	cg.clientNum = clientNum;

	cgs.serverCommandSequence = serverCommandSequence;

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	CG_ParseServerinfo();

	trap_CM_LoadMap( cgs.mapname );

	cg.loading = qtrue;		// force players to load instead of defer

	CG_RegisterResources();
	Sbar_Init();

	cg.loading = qfalse;	// future players will be deferred

	trap_S_ClearLoopingSounds( qtrue );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}


/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
void CG_EventHandling(int type) {
}



void CG_KeyEvent(int key, qboolean down) {
}

void CG_MouseEvent(int x, int y) {
}

