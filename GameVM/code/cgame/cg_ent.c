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
	int i, flags;
	entityState_t*	entState;
	centity_t*	ent;
	float bobjrotate;

	bobjrotate = AngleNormalize360(cg.time / 10.0);

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		if(entState->modelindex == 0)
			continue;

		ent = &cg_entities[entState->number];

		if ( entState->solid == SOLID_BMODEL )
		{
		}
		else
		{
			flags = cgs.gameModelsFlags[entState->modelindex];
			if (flags & EF_ROTATE)
				ent->angles[YAW] = bobjrotate;
		}
	}
}
