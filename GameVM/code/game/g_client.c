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
		client->ps.delta_angles[i] = cmdAngle - client->pers.cmd.angles[i];
	}
	VectorCopy (angle, client->ps.viewangles);
}

void ClientUserinfoChanged( int clientNum ) {
}

char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	gclient_t	*client;

	client = level.clients + clientNum;
	memset( client, 0, sizeof(*client) );

	client->pers.connected = CON_CONNECTING;

	// get and distribute relevant parameters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

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
	gclient_t	*client;
	int			flags;

	client = level.clients + clientNum;

	client->pers.connected = CON_CONNECTED;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

	// locate ent at a spawn point
	ClientSpawn( client );

	G_LogPrintf( "ClientBegin: %i\n", clientNum );
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gclient_t* client) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	int		i;
	clientPersistant_t	saved;
	int		persistant[MAX_PERSISTANT];
	int		savedPing;
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];

	index = client - level.clients;

	VectorClear(spawn_origin);

	// clear everything but the persistant data

	saved = client->pers;
	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	eventSequence = client->ps.eventSequence;

	Com_Memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	client->ps.eventSequence = eventSequence;

	trap_GetUserinfo( index, userinfo, sizeof(userinfo) );

	client->ps.clientNum = index;

	// health will count down towards max_health
	client->ps.stats[STAT_HEALTH] = 125;

	VectorCopy( spawn_origin, client->ps.origin );

	trap_GetUsercmd( client - level.clients, &client->pers.cmd );
	SetClientViewAngle( client, spawn_angles );
	// don't allow full run speed for a bit
	client->ps.pm_time = 100;

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;

	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	ClientEndFrame( client );
}

void ClientDisconnect( int clientNum ) {
	gclient_t	*client;

	client = level.clients + clientNum;
	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	client->pers.connected = CON_DISCONNECTED;
}


