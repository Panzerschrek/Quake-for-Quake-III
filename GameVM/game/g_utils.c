#include "g_local.h"

static int unique_event_id = 1;

edict_t* G_CreateEventEdict(vec3_t origin, int eventType)
{
	edict_t* e;
	int i;

	e = ED_Alloc();

	VectorCopy(origin, e->v.origin);
	VectorCopy(origin, e->s.origin);
	VectorCopy(origin, e->r.currentOrigin);

	for( i= 0; i < 3; ++i )
		e->r.mins[i]= e->r.maxs[i]= 0;

	e->eventTime = level.time;
	e->s.event = eventType;
	e->r.contents= 0;

	// Give unique ids for all events to distinguish different events for same entity number on client.

	e->s.constantLight = unique_event_id;
	++unique_event_id;

	trap_LinkEntity(e);

	return e;
}

edict_t* G_CreateGlobalEventEdict(vec3_t origin, int eventType)
{
	edict_t* e;
	int i;

	e = ED_Alloc();

	VectorCopy(origin, e->v.origin);
	VectorCopy(origin, e->s.origin);
	VectorCopy(origin, e->r.currentOrigin);

	for( i= 0; i < 3; ++i )
	{
		e->r.mins[i]= -99999;
		e->r.maxs[i]= +99999;
	}

	e->eventTime = level.time;
	e->s.event = eventType;

	// Give unique ids for all events to distinguish different events for same entity number on client.

	e->s.constantLight = unique_event_id;
	++unique_event_id;

	trap_LinkEntity(e);

	return e;
}
