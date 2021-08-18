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

/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
static void V_CalcViewRoll (void)
{
	float		side;

	side = V_CalcRoll (cg.refdefViewAngles, cg.snap.ps.velocity);
	cg.refdefViewAngles[ROLL] += side;


	if (v_dmg_time > 0)
	{
		cg.refdefViewAngles[ROLL] += v_dmg_time/v_kicktime.value*v_dmg_roll;
		cg.refdefViewAngles[PITCH] += v_dmg_time/v_kicktime.value*v_dmg_pitch;
		v_dmg_time -= cg.frametime / 1000.0f;
	}

	if (cg.snap.ps.stats[STAT_HEALTH] <= 0)
	{
		cg.refdefViewAngles[ROLL] = 80;	// dead view angle
		return;
	}
}

/*
===============
V_CalcBob

===============
*/
float V_CalcBob (void)
{
	float	bob;
	float	cycle, timeS;

	timeS= cg.time / 1000.0f;

	cycle = timeS - (int)(timeS/cl_bobcycle.value)*cl_bobcycle.value;
	cycle /= cl_bobcycle.value;
	if (cycle < cl_bobup.value)
		cycle = M_PI * cycle / cl_bobup.value;
	else
		cycle = M_PI + M_PI*(cycle-cl_bobup.value)/(1.0 - cl_bobup.value);

// bob is proportional to velocity in the xy plane
// (don't count Z, or jumping messes it up)

	bob = sqrt(cg.snap.ps.velocity[0]*cg.snap.ps.velocity[0] + cg.snap.ps.velocity[1]*cg.snap.ps.velocity[1]) * cl_bob.value;
//Con_Printf ("speed: %5.1f\n", Length(cl.velocity));
	bob = bob*0.3 + bob*0.7*sin(cycle);
	if (bob > 4)
		bob = 4;
	else if (bob < -7)
		bob = -7;
	return bob;

}

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
	float			bob;

	origin = cg.refdef.vieworg;

	bob = V_CalcBob();

	// add view height
	origin[2] += cg.snap.ps.viewheight + V_CalcBob();
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
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	// calculate size of 3D view
	CG_CalcVrect();

	VectorCopy( cg.snap.ps.origin, cg.refdef.vieworg );
	VectorCopy( cg.snap.ps.viewangles, cg.refdefViewAngles );

	CG_OffsetFirstPersonView();

	V_CalcViewRoll();

	// position eye relative to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

	CG_CalcFov();
}

void V_AddIdle (void)
{
	// PANZER TODO - create cvars for these constants.
	const float v_idlescale = 1.0f;
	const float v_iroll_cycle = 0.5f;
	const float v_ipitch_cycle = 1.0f;
	const float v_iyaw_cycle = 2.0f;
	const float v_iroll_level = 0.1f;
	const float v_ipitch_level = 0.3f;
	const float v_iyaw_level = 0.3f;

	cg.refdefViewAngles[ROLL] += v_idlescale * sin(cg.time / 1000.0f * v_iroll_cycle) * v_iroll_level;
	cg.refdefViewAngles[PITCH] += v_idlescale * sin(cg.time / 1000.0f * v_ipitch_cycle) * v_ipitch_level;
	cg.refdefViewAngles[YAW] += v_idlescale * sin(cg.time / 1000.0f * v_iyaw_cycle) * v_iyaw_level;
}

static void CG_CalcIntermissionViewValues( void ) {
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	// calculate size of 3D view
	CG_CalcVrect();

	VectorCopy( cg.snap.ps.origin, cg.refdef.vieworg );
	VectorCopy( cg.snap.ps.viewangles, cg.refdefViewAngles );

	V_AddIdle();

	// position eye relative to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

	CG_CalcFov();
}

#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4
static int CorrectUnwderwaterView(void) {
	int inwater;
	float v, phase;

	inwater = ( cg.viewContents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) != 0;
	if(inwater )
	{
		phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
		v = WAVE_AMPLITUDE * sin( phase );
		cg.refdef.fov_x += v;
		cg.refdef.fov_y -= v;
	}

	return inwater;
}

void CG_AddEntities()
{
	int				num, modelindex;
	entityState_t*	in_ent_state;
	centity_t*		in_ent;
	refEntity_t		out_ent;
	vec3_t			anglesCorrected;

	// Directly take entities from snapshot and add them to scene.

	for ( num = 0 ; num < cg.snap.numEntities ; num++ ) {
		in_ent_state = &cg.snap.entities[num];
		modelindex= in_ent_state->modelindex;

		if( in_ent_state->loopSound )
			CG_SetAmbientSound( in_ent_state );

		if( modelindex == 0 )
			continue;

		if( in_ent_state->number == cg.viewentity )
			continue; // Do not draw player itself.

		in_ent = &cg_entities[in_ent_state->number];

		memset (&out_ent, 0, sizeof(out_ent));
		VectorCopy( in_ent->origin, out_ent.origin);
		VectorCopy( in_ent->origin, out_ent.oldorigin);

		// I do not know why, but i need to revert pitch to get correct models orientation.
		anglesCorrected[PITCH]= -AngleNormalize360(in_ent->angles[PITCH]);
		anglesCorrected[YAW]= AngleNormalize360(in_ent->angles[YAW]);
		anglesCorrected[ROLL]= AngleNormalize360(in_ent->angles[ROLL]);
		AnglesToAxis( anglesCorrected, out_ent.axis );

		if ( in_ent_state->solid == SOLID_BMODEL )
		{
			out_ent.hModel = cgs.inlineDrawModel[modelindex];

			// Use special shader for textures like buttons with two layers modulated by "entity"/"1-entity" color.
			// Such approach allows us to switch between two sets of textures on brush model.

			if (in_ent->frame != 0) // Non-zero frame is a flag for alternative textures.
			{
				out_ent.shaderRGBA[0] = 0;
				out_ent.shaderRGBA[1] = 0;
				out_ent.shaderRGBA[2] = 0;
				out_ent.shaderRGBA[3] = 0;
			}
			else
			{
				out_ent.shaderRGBA[0] = 255;
				out_ent.shaderRGBA[1] = 255;
				out_ent.shaderRGBA[2] = 255;
				out_ent.shaderRGBA[3] = 255;
			}
		}
		else
		{
			out_ent.hModel= cgs.gameModels[modelindex].handle;
			out_ent.frame = out_ent.oldframe = in_ent->frame;
		}

		trap_R_AddRefEntityToScene(&out_ent);
	}
}
void CG_AndAddTEnts (void)
{
	int			i;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	refEntity_t	out_ent;
	vec3_t		angles;
	float		yaw, pitch;
	float		forward;

// update lightning
	for (i=0, b=cg_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cg.time)
			continue;

	// if coming from the player, update the start position
		if (b->entity == cg.viewentity)
		{
			VectorCopy (cg.snap.ps.origin, b->start);
		}

	// calculate pitch and yaw
		VectorSubtract (b->end, b->start, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(dist[1], dist[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int) (atan2(dist[2], forward) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

	// add new entities for the lightning
		VectorCopy (b->start, org);
		d = VectorNormalize(dist);
		while (d > 0)
		{
			memset (&out_ent, 0, sizeof(out_ent));
			VectorCopy( org, out_ent.origin);
			out_ent.hModel = b->model;
			angles[0] = pitch;
			angles[1] = yaw;
			angles[2] = rand()%360;
			AnglesToAxis( angles, out_ent.axis );

			trap_R_AddRefEntityToScene(&out_ent);

			for (i=0 ; i<3 ; i++)
				org[i] += dist[i]*30;
			d -= 30;
		}
	}

}

static void CG_AddDlights(void) {
	int		i;
	dlight_t	*l;

	l = cg_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (l->die < cg.time || !l->radius)
			continue;
		trap_R_AddLightToScene(l->origin, l->radius, 1.0f, 1.0f, 1.0f );
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t		hand;
	int				i;
	float			bob;
	vec3_t			forward, right, up;
	vec3_t			angles;
	vec3_t			origin;

	bob = V_CalcBob();

	VectorCopy(ps->origin, origin);

	angles[PITCH] = -cg.snap.ps.viewangles[PITCH];	// because entity pitches are
											//  actually backward
	angles[YAW] = cg.snap.ps.viewangles[YAW];
	angles[ROLL] = cg.snap.ps.viewangles[ROLL];

	AngleVectors (angles, forward, right, up);

	for (i=0 ; i<3 ; i++)
		origin[i] += forward[i]*bob*0.4;
	origin[2] += bob;

	// PANZER TODO - adjust veapon position based on status bar size.

	memset (&hand, 0, sizeof(hand));
	VectorCopy(origin, hand.origin);
	VectorCopy(origin, hand.oldorigin);
	hand.origin[2] += ps->viewheight;
	hand.oldorigin[2] += ps->viewheight;
	AnglesToAxis( ps->viewangles, hand.axis );

	hand.hModel = cgs.gameModels[ps->weapon].handle;
	hand.frame= hand.oldframe= ps->weaponstate; // Use "weaponstate" for weapon frame.
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	trap_R_AddRefEntityToScene(&hand);
}

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {

	int inwater;

	cg.frametime= serverTime - cg.time;
	cg.time = serverTime;

	// update cvars
	CG_UpdateCvars();

	// any looped sounds will be respecified as entities
	// are added to the render list
	trap_S_ClearLoopingSounds(qfalse);

	// clear all the render lists
	trap_R_ClearScene();

	// let the client system know what our weapon and zoom settings are
	trap_SetUserCmdValue( cg.weaponSelect, 1.0f );
	cg.weaponSelect = 0;

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();
	if(cg.last_snap_num == 0)
		return; // Did not recieve any snapshot yet.

	cg.viewentity = cg.snap.ps.clientNum + 1;

	CL_DecayLights();
	CG_UpdateEntities();

	CG_AddEntities();
	CG_AndAddTEnts();
	CG_AddDlights();
	R_DrawParticles();

	// build cg.refdef
	if( cg.snap.ps.pm_type == PM_NORMAL )
		CG_CalcViewValues();
	else
		CG_CalcIntermissionViewValues();

	cg.viewContents = trap_CM_PointContents( cg.refdef.vieworg, 0 );

	inwater = CorrectUnwderwaterView();

	if( cg.snap.ps.pm_type == PM_NORMAL )
		CG_AddViewWeapon( &cg.snap.ps );

	cg.refdef.time = cg.time;
	memcpy( cg.refdef.areamask, cg.snap.areamask, sizeof( cg.refdef.areamask ) );

	// update audio positions
	trap_S_Respatialize( cg.snap.ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

	trap_R_RenderScene( &cg.refdef );

	CG_DrawPolyBlend();

	if( cg.snap.ps.pm_type == PM_NORMAL )
	{
		Sbar_Draw();
		DrawCrosshair();
		DrawCenterPrint();

		{
			char buff[16];
			trap_Cvar_VariableStringBuffer("cl_paused", buff, sizeof(buff));
			if (atof(buff) != 0.0f)
				Sbar_Pause();
		}
	}
	else if( cg.snap.ps.pm_type == PM_INTERMISSION )
		Sbar_IntermissionOverlay();
	else if( cg.snap.ps.pm_type == PM_INTERMISSION_FINALE )
	{
		Sbar_FinaleOverlay();
		DrawCenterPrint();
	}
}
