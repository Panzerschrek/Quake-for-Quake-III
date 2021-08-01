#include "cg_local.h"
#include "../game/protocol.h"

static void CG_StartSoundEvent( entityState_t *ent )
{
	int entityNum;

	entityNum = ent->eFlags;
	//Com_Printf("start sound with index %d for entity %d\n", ent->weapon, entityNum);
	// TODO - fix this. We probavly should not set position for entity-linked sounds.
	trap_S_StartSound(cg_entities[entityNum].origin, entityNum, CHAN_BODY, cgs.gameSounds[ent->weapon]);
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
