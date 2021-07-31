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

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gclient_t *client ) {
	int			msec;
	int			i;
	usercmd_t	*ucmd;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (!client->active) {
		return;
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &client->cmd;

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	host_client = client;
	sv_player = host_client->edict;
	host_frametime = msec / 1000.0;

	// read buttons
	sv_player->v.button0 = ucmd->buttons & BUTTON_ATTACK;
	sv_player->v.button2 = ucmd->upmove > 0;
	sv_player->v.impulse = ucmd->weapon;

	for(i= 0; i < 3; ++i)
		host_client->edict->v.angles[i]= ((float)ucmd->angles[i]) * (360.0f / 65536.0f);

	SV_ClientThink();

}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gclient_t *client;

	client = svs.clients + clientNum;
	trap_GetUsercmd( clientNum, &client->cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	client->lastCmdTime = level.time;

	ClientThink_real( client );
	client->cmd.serverTime = level.time;
}


/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gclient_t *client ) {
	int			i;

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( client->ps.powerups[ i ] < level.time ) {
			client->ps.powerups[ i ] = 0;
		}
	}
}


