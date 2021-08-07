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


void G_Noclip_f (void)
{
	int clientNum;

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	clientNum = NUM_FOR_EDICT(sv_player) - 1;
	if (sv_player->v.movetype != MOVETYPE_NOCLIP)
	{
		sv_player->v.movetype = MOVETYPE_NOCLIP;
		SV_SendPrint ( clientNum, "noclip ON\n");
	}
	else
	{
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_SendPrint ( clientNum, "noclip OFF\n");
	}
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if( Q_stricmp( cmd, "noclip" ) == 0 )
	{
		G_Noclip_f();
	}
	else
		trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}
