#include "cg_local.h"
#include "../game/protocol.h"

static void CG_StartSoundEvent( entityState_t *ent )
{
	int entityNum, soundNum;

	entityNum = ent->eFlags;
	soundNum = ent->weapon;
	//Com_Printf("start sound with index %d for entity %d\n", ent->weapon, entityNum);
	trap_S_StartSound(NULL, entityNum, CHAN_ITEM, cgs.gameSounds[soundNum]);
}

void CG_CheckEvents( entityState_t *ent )
{
	switch(ent->event)
	{
	case svc_sound:
		CG_StartSoundEvent(ent);
		break;
	}
}
