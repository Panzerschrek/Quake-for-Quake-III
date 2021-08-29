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
	float bobjrotate, lerpdelta;

	bobjrotate = AngleNormalize360(cg.time / 10.0);

	// Switch frames and iterpolate angles in 1/10 s - as default frames update frequency in QuakeC.
	lerpdelta = cg.frametime / 100.0f;

	for( i = 0; i < cg.snap.numEntities; ++i )
	{
		entState = &cg.snap.entities[i];
		modelindex = entState->modelindex;
		if(modelindex == 0)
			continue;

		ent = &cg_entities[entState->number];

		if ( entState->solid == SOLID_BMODEL )
			continue;

		if (cgs.gameModels[modelindex].flags & EF_ROTATE)
		{
			ent->angles[YAW] = ent->oldangles[YAW] = bobjrotate;
			ent->anglelerp= 1.0f;
		}
		else
		{
			ent->anglelerp+= lerpdelta;
			if( ent->anglelerp > 1.0f )
				ent->anglelerp= 1.0f;
		}

		if( entState->frame == 65535 ) // Client-side animation
		{
			// Assume 10 frames/s monotonic animation.
			// shift phase based on entity number
			ent->frame = ( cg.time / 100 + entState->number * 79 ) % cgs.gameModels[modelindex].numFrames;
			ent->oldframe = ent->frame;
			ent->framelerp= 1.0f;
		}
		else
		{
			ent->framelerp+= lerpdelta;
			if(ent->framelerp > 1.0f)
				ent->framelerp= 1.0f;
		}
	}
}
