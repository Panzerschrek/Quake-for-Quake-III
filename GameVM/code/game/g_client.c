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

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gclient_t *client, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		client->ps.delta_angles[i] = cmdAngle - client->cmd.angles[i];
	}
	VectorCopy (angle, client->ps.viewangles);
}

void ClientUserinfoChanged( int clientNum ) {
}

char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	edict_t			*ent;
	client_t		*client;
	int				edictnum;

	G_Printf ("Client %d connected\n", clientNum);

	edictnum = clientNum+1;

	ent = EDICT_NUM(edictnum);

	client = svs.clients + clientNum;
	memset(client, 0, sizeof(*client));
	strcpy (client->name, "unconnected");
	client->active = qtrue;
	client->spawned = qfalse;
	client->edict = ent;
	client->ps.clientNum = clientNum;

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	client_t		*client;
	int				edictnum;

	client = svs.clients + clientNum;

	G_Printf ("Client %d entered\n", clientNum);

	edictnum = clientNum+1;

	strcpy (client->name, "SomeClient");

	sv_player = EDICT_NUM(edictnum);

	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (pr_global_struct->ClientConnect);

	PR_ExecuteProgram (pr_global_struct->PutClientInServer);

	client->spawned = qtrue;
}

void ClientDisconnect( int clientNum ) {
	// PANZER TODO
}
