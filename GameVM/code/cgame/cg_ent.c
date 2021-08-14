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
	int i;
	for( i = 0; i < cg.snap.numEntities; ++i )
	{
	}
}
