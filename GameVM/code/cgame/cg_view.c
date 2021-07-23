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
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"

static void CG_CalcVrect (void) {

	cg.refdef.width = cgs.glconfig.vidWidth;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight;
	cg.refdef.height &= ~1;

	cg.refdef.x = 0;
	cg.refdef.y = 0;
}

static void CG_OffsetFirstPersonView( void ) {
	float			*origin;
	float			*angles;

	origin = cg.refdef.vieworg;
	angles = cg.refdefViewAngles;

	// if dead, fix the angle and don't add any kick
	if ( cg.snap.ps.stats[STAT_HEALTH] <= 0 ) {
		angles[ROLL] = 40;
		angles[PITCH] = -15;
		angles[YAW] = cg.snap.ps.stats[STAT_DEAD_YAW];
		origin[2] += cg.predictedPlayerState.viewheight;
		return;
	}

	// add view height
	origin[2] += cg.predictedPlayerState.viewheight;
}

static void CG_CalcFov( void ) {
	float	x;
	float	fov_x, fov_y;

	fov_x = 90;

	x = cg.refdef.width / tan( fov_x / 360 * M_PI );
	fov_y = atan2( cg.refdef.height, x );
	fov_y = fov_y * 360 / M_PI;

	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;
}

static void CG_CalcViewValues( void ) {
	playerState_t	*ps;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;

	VectorCopy( ps->origin, cg.refdef.vieworg );
	VectorCopy( ps->viewangles, cg.refdefViewAngles );

	CG_OffsetFirstPersonView();

	// position eye relative to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

	CG_CalcFov();
}

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	cg.time = serverTime;
	cg.demoPlayback = demoPlayback;

	// update cvars
	CG_UpdateCvars();

	// any looped sounds will be respecified as entities
	// are added to the render list
	trap_S_ClearLoopingSounds(qfalse);

	// clear all the render lists
	trap_R_ClearScene();

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( ( cg.snap.snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		return;
	}

	// let the client system know what our weapon and zoom settings are
	trap_SetUserCmdValue( cg.weaponSelect, cg.zoomSensitivity );

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	cg.predictedPlayerState = cg.snap.ps;

	// decide on third person view
	cg.renderingThirdPerson = 0;

	// build cg.refdef
	CG_CalcViewValues();

	cg.refdef.time = cg.time;
	memcpy( cg.refdef.areamask, cg.snap.areamask, sizeof( cg.refdef.areamask ) );

	// update audio positions
	trap_S_Respatialize( cg.snap.ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, 0 );

	trap_R_RenderScene( &cg.refdef );
}
