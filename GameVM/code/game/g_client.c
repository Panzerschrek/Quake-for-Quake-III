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

	client->message.data = client->msgbuf;
	client->message.maxsize = sizeof(client->msgbuf);
	client->message.allowoverflow = qfalse; // PANZER TODO - allow owerflow?

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

	strcpy (host_client->name, "SomeClient");

	// copy spawn parms out of the client_t

	if(sv.loadgame)
	{
	}
	else
	{
		// set up the edict
		memset (&sv_player->v, 0, progs->entityfields * 4);
		sv_player->v.colormap = NUM_FOR_EDICT(sv_player);
		sv_player->v.team = (host_client->colors & 15) + 1;
		sv_player->v.netname = ED_NewString (host_client->name) - pr_strings;

		// copy spawn parms out of the client_t

		// call the progs to get default spawn parms for the new client
		PR_ExecuteProgram (pr_global_struct->SetNewParms);
		for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
			host_client->spawn_parms[i] = (&pr_global_struct->parm1)[i];

		// call the spawn function

		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientConnect);

		PR_ExecuteProgram (pr_global_struct->PutClientInServer);
	}

	host_client->active = qtrue;
	host_client->spawned = qtrue;
}

void ClientDisconnect( int clientNum ) {
	// PANZER TODO
}
