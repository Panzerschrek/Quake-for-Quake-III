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
// cg_snapshot.c -- things that happen on snapshot transition,
// not necessarily every single rendered frame

#include "cg_local.h"

static void CG_UpdateEntitiesEffects( int updateTimeDelta ) {
	int i, flags, effects, num;
	float	updateTimeDeltaS;
	entityState_t*	entState;
	centity_t*	ent;
	vec3_t		oldorigin, move_delta;
	dlight_t*	dl;

	updateTimeDeltaS = updateTimeDelta / 1000.0f;

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		if(entState->modelindex == 0)
			continue;

		num = entState->number;
		ent = &cg_entities[entState->number];

		if ( entState->solid == SOLID_BMODEL )
			continue;

		flags = cgs.gameModels[entState->modelindex].flags;
		effects = entState->eFlags;

		// Assume linear moving. For frequences > 10hz gravity-related speed change is insignificant.
		VectorScale( entState->pos.trDelta, updateTimeDeltaS, move_delta );
		VectorSubtract( ent->origin, move_delta, oldorigin );

		// PANZER TODO - fix this.
		//if (effects & EF_BRIGHTFIELD)
		//	R_EntityParticles (ent);

		if (effects & EF_MUZZLEFLASH)
		{
			vec3_t		fv, rv, uv;

			dl = CL_AllocDlight (num);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			AngleVectors (ent->angles, fv, rv, uv);

			VectorMA (dl->origin, 18, fv, dl->origin);
			dl->radius = 200 + (rand()&31);
			dl->minlight = 32;
			dl->die = cg.time + 100;
		}
		if (effects & EF_BRIGHTLIGHT)
		{
			dl = CL_AllocDlight (num);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			dl->radius = 400 + (rand()&31);
			dl->die = cg.time + 10;
		}
		if (effects & EF_DIMLIGHT)
		{
			dl = CL_AllocDlight (num);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200 + (rand()&31);
			dl->die = cg.time + 10;
		}

		if (flags & EF_GIB)
			R_RocketTrail (oldorigin, ent->origin, 2);
		else if (flags & EF_ZOMGIB)
			R_RocketTrail (oldorigin, ent->origin, 4);
		else if (flags & EF_TRACER)
			R_RocketTrail (oldorigin, ent->origin, 3);
		else if (flags & EF_TRACER2)
			R_RocketTrail (oldorigin, ent->origin, 5);
		else if (flags & EF_ROCKET)
		{
			R_RocketTrail (oldorigin, ent->origin, 0);
			dl = CL_AllocDlight (num);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 200;
			dl->die = cg.time + 10;
		}
		else if (flags & EF_GRENADE)
			R_RocketTrail (oldorigin, ent->origin, 1);
		else if (flags & EF_TRACER3)
			R_RocketTrail (oldorigin, ent->origin, 6);
	}
}

static void CG_UpdateEntitiesWithNewSnapshot (void) {
	int				i, j, n, event_unique_id;
	entityState_t* entState;
	centity_t*	cent;

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		n = cg.snap.entities[i].number;
		cent = &cg_entities[n];

		VectorCopy(entState->origin, cent->origin);

		if ( entState->solid == SOLID_BMODEL )
		{
			vec3_t origin;
			VectorAdd(cent->origin, cgs.inlineModelMidpoints[entState->modelindex], origin);
			trap_S_UpdateEntityPosition(n, origin);
		}
		else
			trap_S_UpdateEntityPosition(n, cent->origin);

		event_unique_id = entState->constantLight;
		if( entState->event != 0 && cent->prev_unique_event_id != event_unique_id )
		{
			cent->prev_unique_event_id = event_unique_id;
			CG_CheckEvents( &cg.snap.entities[i] );
		}

		if (entState->modelindex == 0)
			cent->modelindex = 0;
		else if (cent->modelindex != entState->modelindex)
		{
			// Reset frames interpolation if model index changed.
			cent->modelindex = entState->modelindex;
			cent->oldframe = cent->frame = entState->frame;
			cent->framelerp = 1.0f;

			// Reset angles interpolation too.
			VectorCopy(entState->angles, cent->angles);
			VectorCopy(entState->angles, cent->oldangles);
		}
		else if(cent->frame != entState->frame)
		{
			// Reset frames interpolation if target frame changed.
			cent->oldframe= cent->frame;
			cent->frame= entState->frame;
			cent->framelerp= 0.0f;
		}

		if (cent->modelindex != 0 &&
			!(cent->angles[0] == entState->angles[0] && cent->angles[1] == entState->angles[1] && cent->angles[2] == entState->angles[2]))
		{
			// Calculate last interpolated angles, save them into old angles, set target angle and reset interpolation.
			for (j= 0; j < 3; ++j)
			{
				float angle, oldangle, delta;
				angle= AngleNormalize360(cent->angles[j]);
				oldangle= AngleNormalize360(cent->oldangles[j]);
				delta= AngleNormalize180( angle - oldangle );

				cent->oldangles[j]= oldangle + delta * cent->anglelerp;
				cent->angles[j]= entState->angles[j];
			}
			cent->anglelerp= 0.0f;
		}
	}
}

void CG_ProcessSnapshots( void ) {
	int				i;
	int				snapshotTime;
	int				prevServerTime;
	int				items;
	int				snapshotNum, currentSnapshotNum, processSnapshotNum;

	// Try to process more than one server snaposhots, not only last to properly process server commands and effects.
	// But do not process to much snapshots at once.
	const int maxSnapshotsToProcess = 5;

	trap_GetCurrentSnapshotNumber( &currentSnapshotNum, &snapshotTime );
	if(currentSnapshotNum == cg.last_snap_num)
		return;

	processSnapshotNum = currentSnapshotNum - maxSnapshotsToProcess;
	if (processSnapshotNum <= cg.last_snap_num)
		processSnapshotNum = cg.last_snap_num + 1;

	for (snapshotNum = processSnapshotNum; snapshotNum <= currentSnapshotNum; ++snapshotNum)
	{
		prevServerTime = cg.snap.serverTime;
		trap_GetSnapshot( snapshotNum, &cg.snap );

		CG_ExecuteNewServerCommands( cg.snap.serverCommandSequence );
		CG_UpdateEntitiesWithNewSnapshot();

		CG_UpdateEntitiesEffects( cg.snap.serverTime - prevServerTime );
	}

	cg.last_snap_num = currentSnapshotNum;

	items = GetItems();
	if(cg.old_items != items)
	{
		for (i=0 ; i<32 ; i++)
			if ( (items & (1<<i)) && !(cg.old_items & (1<<i)))
				cg.item_gettime[i] = cg.time;
		cg.old_items = items;
	}
}

