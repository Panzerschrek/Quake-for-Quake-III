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

// Copied from "qwalk" code.
typedef struct md3_header_s
{
	char ident[4]; /* "IDP3" */
	int version; /* 15 */

	char name[64];

	int flags; /* unused by quake3, darkplaces uses it for quake-style modelflags (rocket trails, etc.) */

	int num_frames;
	int num_tags;
	int num_meshes;
	int num_skins; /* apparently unused */

/* lump offsets are relative to start of header (start of file) */
	int lump_frameinfo;
	int lump_tags;
	int lump_meshes;
	int lump_end;
} md3_header_t;

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

cg_t			cg;
cgs_t			cgs;
centity_t		cg_entities[MAX_GENTITIES];
beam_t			cg_beams[MAX_BEAMS];
dlight_t		cg_dlights[MAX_DLIGHTS];
sbar_t			sbar;

vmCvar_t		cl_bob;
vmCvar_t		cl_bobcycle;
vmCvar_t		cl_bobup;

vmCvar_t	cl_rollspeed;
vmCvar_t	cl_rollangle;

vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_sbar_scale;
vmCvar_t	cg_sbar_lines;

vmCvar_t	teamplay; // PANZER TODO - register it?

vmCvar_t	r_particle_size;

vmCvar_t	sv_gravity;

vmCvar_t	v_kicktime;
vmCvar_t	v_kickroll;
vmCvar_t	v_kickpitch;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
	{ &cl_bob, "cl_bob", "0.02", CVAR_SERVERINFO | CVAR_ARCHIVE },
	{ &cl_bobcycle, "cl_bobcycle", "0.6", CVAR_SERVERINFO | CVAR_ARCHIVE },
	{ &cl_bobup, "cl_bobup", "0.5", CVAR_SERVERINFO | CVAR_ARCHIVE },
	{ &cl_rollspeed, "cl_rollspeed", "200", CVAR_SERVERINFO | CVAR_ARCHIVE },
	{ &cl_rollangle, "cl_rollangle", "2.0", CVAR_SERVERINFO | CVAR_ARCHIVE },
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_sbar_scale, "cg_sbar_scale", "1", CVAR_ARCHIVE },
	{ &cg_sbar_lines, "cg_sbar_lines", "2", CVAR_ARCHIVE },
	{ &r_particle_size, "r_particle_size", "2", CVAR_ARCHIVE },
	{ &sv_gravity, "sv_gravity", "800",  CVAR_ARCHIVE }, // PANZER TODO - maybe read this from server info?
	{ &v_kicktime, "v_kicktime", "0.5", CVAR_ARCHIVE },
	{ &v_kickroll, "v_kickroll", "0.6", CVAR_ARCHIVE },
	{ &v_kickpitch, "v_kickpitch", "0.6", CVAR_ARCHIVE },

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

void CG_StartMusic(int trackIndex, int loopTrackIndex) {
	char trackName[64]= "music/track00";
	char loopTrackName[64]= "music/track00";

	trackName[11]= '0' + trackIndex / 10;
	trackName[12]= '0' + trackIndex % 10;
	loopTrackName[11]= '0' + loopTrackIndex / 10;
	loopTrackName[12]= '0' + loopTrackIndex % 10;
	trap_S_StartBackgroundTrack(trackName, loopTrackName);
}


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterResources( void ) {
	int		i;

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
		int		n;
		fileHandle_t f;
		gameModel_t*	outModel;

		strcpy(modelName, CG_ConfigString( CS_MODELS+i ));
		if ( !modelName[0] ) {
			break;
		}

		outModel = &cgs.gameModels[i];
		outModel->flags= 0;
		outModel->numFrames = 1;

		// Fix model names - replace "mdl" extension with "md3" - native Quake3 model format.
		n= strlen(modelName);
		if(
			(n >= 4 && modelName[n-4] == '.' && modelName[n-3] == 'm' && modelName[n-2] == 'd' && modelName[n-1] == 'l') ||
			(n >= 4 && modelName[n-4] == '.' && modelName[n-3] == 'b' && modelName[n-2] == 's' && modelName[n-1] == 'p'))
		{
			modelName[n-3]= 'm';
			modelName[n-2]= 'd';
			modelName[n-1]= '3';
			modelName[n-4] = '.';

			// Read md3 header to extract models flags and frame numbers.
			// These values are needed for client side animation/effects.
			f = 0;
			trap_FS_FOpenFile(modelName, &f, FS_READ);
			if( f != 0 )
			{
				md3_header_t header;
				memset(&header, 0, sizeof(header));
				trap_FS_Read(&header, sizeof(header), f);
				outModel->flags= header.flags;
				outModel->numFrames= header.num_frames;
				trap_FS_FCloseFile(f);
			}
		}

		outModel->handle = trap_R_RegisterModel( modelName );
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

	// Register models for client entities.
	cgs.bolt = trap_R_RegisterModel("progs/bolt.md3");
	cgs.bolt2 = trap_R_RegisterModel("progs/bolt2.md3");
	cgs.bolt3 = trap_R_RegisterModel("progs/bolt3.md3");
	cgs.beam = trap_R_RegisterModel("progs/beam.md3");

	cgs.particle = trap_R_RegisterShader("quake_for_quake3_gfx/particle");
	cgs.fullscreen_blend = trap_R_RegisterShaderNoMip("quake_for_quake3_gfx/fullscreen_blend");

	{
		const char* musicIndexStr = CG_ConfigString(CS_MUSIC);
		if(musicIndexStr && musicIndexStr[0] )
		{
			int music_index = atoi(musicIndexStr);
			if(music_index > 0 && music_index < 100 )
				CG_StartMusic(music_index, music_index);
		}
	}

	// Register local sounds.
	cgs.sfx_wizhit = trap_S_RegisterSound ("sound/wizard/hit.wav", qfalse);
	cgs.sfx_knighthit = trap_S_RegisterSound ("sound/hknight/hit.wav", qfalse);
	cgs.sfx_tink1 = trap_S_RegisterSound ("sound/weapons/tink1.wav", qfalse);
	cgs.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/ric1.wav", qfalse);
	cgs.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/ric2.wav", qfalse);
	cgs.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/ric3.wav", qfalse);
	cgs.sfx_r_exp3 = trap_S_RegisterSound ("sound/weapons/r_exp3.wav", qfalse);
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
	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof( cg_entities ) );
	memset (cg_beams, 0, sizeof(cg_beams));
	memset (cg_dlights, 0, sizeof(cg_dlights));

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
	R_InitParticles();

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

int GetItems()
{
	return (cg.snap.ps.stats[STAT_ITEMS_LO] & 65535) | (cg.snap.ps.stats[STAT_ITEMS_HI] << 16);
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

