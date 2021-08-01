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
	int				i, n, event_id;
	int snapshotTime;
	trap_GetCurrentSnapshotNumber( &n, &snapshotTime );
	trap_GetSnapshot( n, &cg.snap);

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		event_id = cg.snap.entities[i].torsoAnim;
		if( cg_entities[i].prev_unique_event_id != event_id )
		{
			cg_entities[i].should_process_event = qtrue;
			cg_entities[i].prev_unique_event_id = event_id;
		}
		else
			cg_entities[i].should_process_event = qfalse;
	}
}

