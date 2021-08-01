#include "cg_local.h"
#include "../game/protocol.h"

// See defs.qc
static int Q1ToQ3Channel(int q1_channel)
{
	switch(q1_channel)
	{
	case 0: return CHAN_AUTO;
	case 1: return CHAN_WEAPON;
	case 2: return CHAN_VOICE;
	case 3: return CHAN_ITEM;
	case 4: return CHAN_BODY;
	};

	return CHAN_AUTO;
}

static void CG_StartSoundEvent( entityState_t *ent )
{
	int entityNum, soundNum, channel, attenuation;

	entityNum = ent->eFlags;
	soundNum = ent->weapon;
	channel = Q1ToQ3Channel(ent->legsAnim);
	attenuation = ent->torsoAnim;

	// See defs.qc
	switch(attenuation)
	{
	case 0:
		trap_S_StartLocalSound(entityNum, channel);
		break;
	case 1:
	case 2:
	case 3:
	default:
		trap_S_StartSound(NULL, entityNum, channel, cgs.gameSounds[soundNum]);
		break;

	}
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
