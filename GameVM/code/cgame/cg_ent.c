#include "cg_local.h"

#define	EF_ROCKET	1			// leave a trail
#define	EF_GRENADE	2			// leave a trail
#define	EF_GIB		4			// leave a trail
#define	EF_ROTATE	8			// rotate (bonus items)
#define	EF_TRACER	16			// green split trail
#define	EF_ZOMGIB	32			// small blood trail
#define	EF_TRACER2	64			// orange split trail + rotate
#define	EF_TRACER3	128			// purple trail

void CG_SetAmbientSound( entityState_t *ent )
{
	int  soundNum, volume, attenuation;

	soundNum = ent->weapon;
	volume = ent->legsAnim;
	attenuation = ent->torsoAnim;

	trap_S_AddLoopingSound(ent->number, ent->origin, vec3_origin, cgs.gameSounds[soundNum]);
}

void CG_UpdateEntities (void)
{
	int i, flags, effects, num, modelindex;
	entityState_t*	entState;
	centity_t*	ent;
	float bobjrotate;
	dlight_t*	dl;

	bobjrotate = AngleNormalize360(cg.time / 10.0);

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		modelindex = CG_GetModelIndex(entState);
		if(modelindex == 0)
			continue;

		num = entState->number;
		ent = &cg_entities[entState->number];

		// DOTO - fix this. This is an ugly hack. We should somehow reset old origin if entity was deallocated.
		// Also we should imerpoalate origin (somehow) to get smoother rocket trails.
		if(ent->oldorigin[0] == 0.0f && ent->oldorigin[1] == 0.0f && ent->oldorigin[2] == 0.0f)
			continue;

		if ( entState->solid == SOLID_BMODEL )
			continue;


		flags = cgs.gameModels[modelindex].flags;
		effects = entState->eFlags;

		if (flags & EF_ROTATE)
			ent->angles[YAW] = bobjrotate;

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
			R_RocketTrail (ent->oldorigin, ent->origin, 2);
		else if (flags & EF_ZOMGIB)
			R_RocketTrail (ent->oldorigin, ent->origin, 4);
		else if (flags & EF_TRACER)
			R_RocketTrail (ent->oldorigin, ent->origin, 3);
		else if (flags & EF_TRACER2)
			R_RocketTrail (ent->oldorigin, ent->origin, 5);
		else if (flags & EF_ROCKET)
		{
			R_RocketTrail (ent->oldorigin, ent->origin, 0);
			dl = CL_AllocDlight (num);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 200;
			dl->die = cg.time + 10;
		}
		else if (flags & EF_GRENADE)
			R_RocketTrail (ent->oldorigin, ent->origin, 1);
		else if (flags & EF_TRACER3)
			R_RocketTrail (ent->oldorigin, ent->origin, 6);


		if( entState->frame == 65535 ) // Client-side animation
		{
			// Assume 10 frames/s monotonic animation.
			ent->frame = cg.time / 100 % cgs.gameModels[modelindex].numFrames;
		}
	}
}

int CG_GetModelIndex( entityState_t *ent )
{
	return ent->otherEntityNum; // Use not "modelindex", but different field for transmission of model index to client.
}
