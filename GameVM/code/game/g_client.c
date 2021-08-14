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

void ClientUserinfoChanged( int clientNum ) {
	char			userInfo[1024];
	char			*name;
	client_t		*client;

	trap_GetUserinfo(clientNum, userInfo, sizeof(userInfo));
	name = Info_ValueForKey(userInfo, "name");
	if (name == NULL)
		name = "NULL";

	client = svs.clients + clientNum;

	if(Q_strncmp(name, client->name, sizeof(client->name)))
	{
		if(client->name[0] != 0)
			G_Printf ("%s renamed to %s\n", client->name, name);
		strncpy(client->name, name, sizeof(client->name));
		client->edict->v.netname = ED_NewString (client->name) - pr_strings;
	}
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
	client->connected = qtrue;
	client->active = qfalse;
	client->edict = ent;
	client->ps.clientNum = clientNum;

	client->message.data = client->msgbuf;
	client->message.maxsize = sizeof(client->msgbuf);
	client->message.allowoverflow = qfalse; // PANZER TODO - allow owerflow?

	ClientUserinfoChanged(clientNum);

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
	int i;

	host_client = svs.clients + clientNum;
	sv_player = host_client->edict;

	G_Printf ("Client %d begin\n", clientNum);

	if(!sv.loadgame)
	{
		// set up the edict
		memset (&sv_player->v, 0, progs->entityfields * 4);
		sv_player->v.colormap = NUM_FOR_EDICT(sv_player);
		sv_player->v.team = (host_client->colors & 15) + 1;
		sv_player->v.netname = ED_NewString (host_client->name) - pr_strings;

		// Load spawn params (from session or from default values)
		SV_LoadClientSpawnParms(clientNum);

		// copy spawn parms out of the client_t
		for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
			(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];

		// call the spawn function

		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientConnect);

		PR_ExecuteProgram (pr_global_struct->PutClientInServer);
	}

	// Do not need to do anything here in case of game loading because player is already spawned in such case.

	host_client->active = qtrue;
}

void ClientDisconnect( int clientNum ) {
	// PANZER TODO
}
