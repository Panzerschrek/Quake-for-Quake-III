#include "cg_local.h"

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
	int i, modelindex;
	entityState_t*	entState;
	centity_t*	ent;
	float bobjrotate;

	bobjrotate = AngleNormalize360(cg.time / 10.0);

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		modelindex = CG_GetModelIndex(entState);
		if(modelindex == 0)
			continue;

		ent = &cg_entities[entState->number];

		if ( entState->solid == SOLID_BMODEL )
			continue;


		if (cgs.gameModels[modelindex].flags & EF_ROTATE)
			ent->angles[YAW] = bobjrotate;

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
