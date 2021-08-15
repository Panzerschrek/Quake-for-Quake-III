#include "g_local.h"


/*
===============
SV_UnlinkEdict

===============
*/
void SV_UnlinkEdict (edict_t *ent)
{
	trap_UnlinkEntity(ent);
}


/*
====================
SV_TouchLinks
====================
*/
void SV_TouchLinks ( edict_t *ent )
{
	edict_t		*touch;
	int			old_self, old_other;
	int i;

	for(i = 1; i < sv.num_edicts; ++i )
	{
		touch = EDICT_NUM(i);
		if(touch->free)
			continue;

		if (touch == ent)
			continue;
		if (!touch->v.touch || touch->v.solid != SOLID_TRIGGER)
			continue;
		if (ent->v.absmin[0] > touch->v.absmax[0]
		|| ent->v.absmin[1] > touch->v.absmax[1]
		|| ent->v.absmin[2] > touch->v.absmax[2]
		|| ent->v.absmax[0] < touch->v.absmin[0]
		|| ent->v.absmax[1] < touch->v.absmin[1]
		|| ent->v.absmax[2] < touch->v.absmin[2] )
			continue;
		old_self = pr_global_struct->self;
		old_other = pr_global_struct->other;

		pr_global_struct->self = EDICT_TO_PROG(touch);
		pr_global_struct->other = EDICT_TO_PROG(ent);
		pr_global_struct->time = sv.time;
		PR_ExecuteProgram (touch->v.touch);

		pr_global_struct->self = old_self;
		pr_global_struct->other = old_other;
	}
}

/*
===============
SV_LinkEdict

===============
*/
void SV_LinkEdict (edict_t *ent, qboolean touch_triggers)
{
	if (ent == sv.edicts)
	{
		trap_LinkEntity(ent);
		return;		// don't add the world
	}

	if (ent->free)
		return;

// set the abs box
	VectorAdd (ent->v.origin, ent->v.mins, ent->v.absmin);
	VectorAdd (ent->v.origin, ent->v.maxs, ent->v.absmax);

	// Set mins/maxs for Quake 3 code. absmin/absmax are calculated in SV_LinkEntity.
	VectorCopy(ent->v.mins, ent->r.mins);
	VectorCopy(ent->v.maxs, ent->r.maxs);
	VectorCopy(ent->v.origin, ent->r.currentOrigin);

//
// to make items easier to pick up and allow them to be grabbed off
// of shelves, the abs sizes are expanded
//
	if ((int)ent->v.flags & FL_ITEM)
	{
		ent->v.absmin[0] -= 15;
		ent->v.absmin[1] -= 15;
		ent->v.absmax[0] += 15;
		ent->v.absmax[1] += 15;

		ent->r.mins[0] -= 15;
		ent->r.mins[1] -= 15;
		ent->r.maxs[0] += 15;
		ent->r.maxs[1] += 15;
	}

	ent->r.ownerNum = ent->v.owner != 0 ? ent->v.owner : ENTITYNUM_NONE;

	SV_UpdateEdictCollsionType(ent);

	trap_LinkEntity(ent);

// if touch_triggers, touch all entities at this node and decend for more
	if (touch_triggers)
		SV_TouchLinks ( ent );
}

void SV_UpdateEdictCollsionType (edict_t* e)
{
	switch((int)e->v.solid)
	{
	case SOLID_NOT:
		e->r.contents= CONTENTS_CORPSE;
		break;
	case SOLID_TRIGGER:
		e->r.contents= CONTENTS_TRIGGER;
		break;
	case SOLID_BBOX:
	case SOLID_SLIDEBOX:
		e->r.contents= CONTENTS_BODY;
		break;
	case SOLID_BSP:
		e->r.contents= CONTENTS_SOLID;
		break;
	default:
		e->r.contents= 0;
	// TODO - support other cases.
	}
}

/*
==================
SV_PointContents

==================
*/
int SV_PointContents (vec3_t p)
{
	return Contents_Q3_to_Q1(trap_PointContents(p, 0));
}

/*
============
SV_TestEntityPosition

This could be a lot more efficient...
============
*/
edict_t	*SV_TestEntityPosition (edict_t *ent)
{
	trace_t	trace;

	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, ent->v.origin, 0, ent);

	if (trace.startsolid)
		return sv.edicts;

	return NULL;
}


/*
==================
SV_Move
==================
*/
trace_t SV_Move (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int type, edict_t *passedict)
{
	trace_t		trace;
	int			contentmask;
	int			passEdictNum;

// fill in a default trace
	memset (&trace, 0, sizeof(trace_t));
	trace.fraction = 1;
	trace.allsolid = qtrue;
	VectorCopy (end, trace.endpos);

	// TODO - check these values.
	if(type == MOVE_NORMAL)
		contentmask = CONTENTS_SOLID | CONTENTS_BODY;
	else if(type == MOVE_NOMONSTERS)
		contentmask = CONTENTS_SOLID;
	else if(type == MOVE_MISSILE)
		contentmask = CONTENTS_SOLID | CONTENTS_BODY;
	else
		contentmask = CONTENTS_SOLID;

	if(passedict == NULL)
		passEdictNum= ENTITYNUM_NONE;
	if(passedict->v.owner != 0)
	{
		passEdictNum= NUM_FOR_EDICT(PROG_TO_EDICT(passedict->v.owner));
		passedict->r.ownerNum = passEdictNum;
	}
	else
	{
		passEdictNum= NUM_FOR_EDICT(passedict);
		passedict->r.ownerNum= ENTITYNUM_NONE;
	}

	trap_Trace(&trace, start, mins, maxs, end, passEdictNum, contentmask);

	if(trace.entityNum == ENTITYNUM_WORLD)
		trace.entityNum = 0;

	return trace;
}

int Contents_Q1_to_Q3(int contents)
{
	switch(contents)
	{
	case Q1_CONTENTS_EMPTY: return 0;
	case Q1_CONTENTS_SOLID: return CONTENTS_SOLID;
	case Q1_CONTENTS_WATER: return CONTENTS_WATER;
	case Q1_CONTENTS_SLIME: return CONTENTS_SLIME;
	case Q1_CONTENTS_LAVA: return CONTENTS_LAVA;
	case Q1_CONTENTS_SKY: return 0;
	}

	return 0;
}

int Contents_Q3_to_Q1(int contents)
{
	if((contents & CONTENTS_SOLID) != 0)
		return Q1_CONTENTS_SOLID;
	if((contents & CONTENTS_WATER) != 0)
		return Q1_CONTENTS_WATER;
	if((contents & CONTENTS_SLIME) != 0)
		return Q1_CONTENTS_SLIME;
	if((contents & CONTENTS_LAVA) != 0)
		return Q1_CONTENTS_LAVA;

	return Q1_CONTENTS_EMPTY;
}
