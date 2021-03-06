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

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];
	char	arg1[MAX_TOKEN_CHARS];
	char	mapname[MAX_OSPATH];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if (!strcmp(cmd, "save"))
	{
		trap_Argv( 1, arg1, sizeof( arg1 ) );
		G_SaveGame(arg1);
		return qtrue;
	}
	else if(!strcmp(cmd, "restart"))
	{
		if(!svs.changelevel_issued) {
			svs.changelevel_issued = qtrue;
			trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
			trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", mapname ) );
		}

		return qtrue;
	}

	return qfalse;
}

