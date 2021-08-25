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
void ClientThink_real( client_t *client ) {
	int			msec;
	int			i;
	usercmd_t	*ucmd;
	float		angle, angle_delta;

	// don't think if the client is not yet active (and thus not yet spawned in)
	if (!client->connected) {
		return;
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &client->cmd;

	msec = ucmd->serverTime - client->ps.commandTime;
	client->ps.commandTime= ucmd->serverTime;
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

	// Do not reset weapon select flag - this should do QuakeC code on next physics tick.
	if (ucmd->weapon != 0)
		sv_player->v.impulse = ucmd->weapon;

	if( client->ps.pm_type == PM_NORMAL ) // Do not rotate camera in intermission mode.
	{
		// Changed "v_angle" using delta calucaltion based on user command.
		for(i= 0; i < 3; ++i)
		{
			angle= ((float)ucmd->angles[i]) * (360.0f / 65536.0f);
			angle_delta = angle - client->prev_cmd_angles[i];
			client->prev_cmd_angles[i] = angle;
			sv_player->v.v_angle[i]+= angle_delta;
		}

		// Clip and correct angles.
		sv_player->v.v_angle[YAW] = AngleNormalize360(sv_player->v.v_angle[YAW]);
		sv_player->v.v_angle[PITCH] = AngleNormalize360(sv_player->v.v_angle[PITCH]);
		if(sv_player->v.v_angle[PITCH] > 180.0f )
			sv_player->v.v_angle[PITCH]-= 360.0f;

		if( sv_player->v.v_angle[PITCH] > 80 )
			sv_player->v.v_angle[PITCH] = 80;
		if( sv_player->v.v_angle[PITCH] < -70 )
			sv_player->v.v_angle[PITCH] = -70;

		if (sv_player->v.v_angle[ROLL] > 50)
			sv_player->v.v_angle[ROLL] = 50;
		if (sv_player->v.v_angle[ROLL] < -50)
			sv_player->v.v_angle[ROLL] = -50;
	}

	SV_ClientThink();

}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	client_t *client;

	client = svs.clients + clientNum;
	trap_GetUsercmd( clientNum, &client->cmd );

	ClientThink_real( client );
}
