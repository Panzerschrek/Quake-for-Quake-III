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

void CG_ProcessSnapshots( void ) {
	int				i, j, n, event_unique_id;
	int snapshotTime;
	int items;
	entityState_t* entState;
	centity_t*	cent;
	trap_GetCurrentSnapshotNumber( &n, &snapshotTime );
	trap_GetSnapshot( n, &cg.snap);

	CG_ExecuteNewServerCommands( cg.snap.serverCommandSequence );

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		n = cg.snap.entities[i].number;
		cent = &cg_entities[n];

		if ( entState->solid == SOLID_BMODEL )
		{
			vec3_t origin;
			VectorAdd(entState->origin, cgs.inlineModelMidpoints[ entState->modelindex ], origin);
			trap_S_UpdateEntityPosition(n, origin);
		}
		else
			trap_S_UpdateEntityPosition(n, entState->origin);

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
}

