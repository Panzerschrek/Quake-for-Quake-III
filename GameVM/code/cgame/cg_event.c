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

// TODO - move this function into separate file.
void CG_SetAmbientSound( entityState_t *ent )
{
	int  soundNum, volume, attenuation;

	soundNum = ent->weapon;
	volume = ent->legsAnim;
	attenuation = ent->torsoAnim;

	trap_S_AddLoopingSound(ent->number, ent->origin, vec3_origin, cgs.gameSounds[soundNum]);
}

static void CG_ProcessParticle( entityState_t *ent )
{
	R_RunParticleEffect(ent->origin, ent->origin2, ent->weapon, ent->constantLight);
}

static void CG_ProcessBeam( entityState_t *ent, qhandle_t model )
{
	beam_t	*b;
	int		i;
	int		entityNum;

	entityNum= ent->constantLight;

	// override any beam with the same entity
	for (i=0, b=cg_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == entityNum)
		{
			b->entity = entityNum;
			b->model = model;
			b->endtime = cg.time + 200;
			VectorCopy (ent->origin, b->start);
			VectorCopy (ent->origin2, b->end);
			return;
		}

	// find a free beam
	for (i=0, b=cg_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cg.time)
		{
			b->entity = entityNum;
			b->model = model;
			b->endtime = cg.time + 200;
			VectorCopy (ent->origin, b->start);
			VectorCopy (ent->origin2, b->end);
			return;
		}
	}
	Com_Printf ("beam list overflow!\n");
}

static void CG_ProcessTEnt( entityState_t *ent )
{
	int rnd;
	int colorStart, colorLength;

	// PANZER TODO - generate dynamic lights.

	switch(ent->eType)
	{
	case TE_WIZSPIKE:			// spike hitting wall
		R_RunParticleEffect (ent->origin, vec3_origin, 20, 30);
		trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_wizhit);
		break;

	case TE_KNIGHTSPIKE:			// spike hitting wall
		R_RunParticleEffect (ent->origin, vec3_origin, 226, 20);
		trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_knighthit);
		break;

	case TE_SPIKE:			// spike hitting wall
		//R_RunParticleEffect (pos, vec3_origin, 0, 10);
		if ( rand() % 5 )
			trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_tink1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_ric1);
			else if (rnd == 2)
				trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_ric2);
			else
				trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_ric3);
		}
		break;


	case TE_SUPERSPIKE:			// super spike hitting wall
		R_RunParticleEffect (ent->origin, vec3_origin, 0, 20);
		if ( rand() % 5 )
			trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_tink1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_ric1);
			else if (rnd == 2)
				trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_ric2);
			else
				trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_ric3);
		}
		break;

	case TE_GUNSHOT:			// bullet hitting wall
		R_RunParticleEffect (ent->origin, vec3_origin, 0, 20);
		break;

	case TE_EXPLOSION:			// rocket explosion
		R_ParticleExplosion (ent->origin);
		//dl = CL_AllocDlight (0);
		//VectorCopy (pos, dl->origin);
		//dl->radius = 350;
		//dl->die = cl.time + 0.5;
		//dl->decay = 300;
		trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_r_exp3);
		break;

	case TE_TAREXPLOSION:			// tarbaby explosion
		R_BlobExplosion (ent->origin);

		trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_r_exp3);
		break;

	case TE_LIGHTNING1:				// lightning bolts
		CG_ProcessBeam (ent, cgs.bolt);
		break;

	case TE_LIGHTNING2:				// lightning bolts
		CG_ProcessBeam (ent, cgs.bolt2);
		break;

	case TE_LIGHTNING3:				// lightning bolts
		CG_ProcessBeam (ent, cgs.bolt3);
		break;

// PGM 01/21/97
	case TE_BEAM:				// grappling hook beam
		CG_ProcessBeam (ent, cgs.beam);
		break;
// PGM 01/21/97

	case TE_LAVASPLASH:
		R_LavaSplash (ent->origin);
		break;

	case TE_TELEPORT:
		R_TeleportSplash (ent->origin);
		break;

	case TE_EXPLOSION2:				// color mapped explosion
		//colorStart = MSG_ReadByte ();
		//colorLength = MSG_ReadByte ();
		colorStart = 5;
		colorLength = 16; // PANZER TODO - fix this
		R_ParticleExplosion2 (ent->origin, colorStart, colorLength);
		//dl = CL_AllocDlight (0);
		//VectorCopy (pos, dl->origin);
		//dl->radius = 350;
		//dl->die = cl.time + 0.5;
		//dl->decay = 300;
		trap_S_StartSound(ent->origin, ENTITYNUM_NONE, CHAN_AUTO, cgs.sfx_r_exp3);
		break;

	default:
		Com_Printf ("CG_ProcessTEnt: bad type");
	};
}

void CG_CheckEvents( entityState_t *ent )
{
	switch(ent->event)
	{
	case svc_sound:
		CG_StartSoundEvent(ent);
		break;
	case svc_particle:
		CG_ProcessParticle(ent);
		break;
	case svc_temp_entity:
		CG_ProcessTEnt(ent);
	}
}
