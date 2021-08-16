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
	int i, flags, effects, num, modelindex;
	float	updateTimeDeltaS;
	entityState_t*	entState;
	centity_t*	ent;
	vec3_t		oldorigin, move_delta;
	dlight_t*	dl;

	updateTimeDeltaS = updateTimeDelta / 1000.0f;

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		modelindex = CG_GetModelIndex(entState);
		if(modelindex == 0)
			continue;

		num = entState->number;
		ent = &cg_entities[entState->number];

		if ( entState->solid == SOLID_BMODEL )
			continue;

		flags = cgs.gameModels[modelindex].flags;
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

void CG_ProcessSnapshots( void ) {
	int				i, j, n, event_unique_id;
	int				snapshotTime;
	int				prevServerTime;
	int				items;
	entityState_t* entState;
	centity_t*	cent;

	trap_GetCurrentSnapshotNumber( &n, &snapshotTime );
	if(n == cg.last_snap_num)
		return;

	prevServerTime = cg.snap.serverTime;
	trap_GetSnapshot( n, &cg.snap );
	cg.last_snap_num = n;

	CG_ExecuteNewServerCommands( cg.snap.serverCommandSequence );

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		n = cg.snap.entities[i].number;
		cent = &cg_entities[n];

		VectorCopy(entState->origin, cent->origin);
		VectorCopy(entState->angles, cent->angles);
		cent->frame = entState->frame;

		if ( entState->solid == SOLID_BMODEL )
		{
			vec3_t origin;
			VectorAdd(cent->origin, cgs.inlineModelMidpoints[ CG_GetModelIndex(entState) ], origin);
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
	}

	items = GetItems();
	if(cg.old_items != items)
	{
		for (j=0 ; j<32 ; j++)
			if ( (items & (1<<j)) && !(cg.old_items & (1<<j)))
				cg.item_gettime[j] = cg.time;
		cg.old_items = items;
	}

	// Update local entities only after reciewing new snapshot.
	CG_UpdateEntitiesEffects( cg.snap.serverTime - prevServerTime );
}

