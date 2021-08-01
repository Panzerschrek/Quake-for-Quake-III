#include "g_local.h"

server_t		sv;
server_static_t	svs;

// Other gobal stuff.

int	current_skill;

client_t	*host_client;			// current client

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
==================
SV_StartParticle

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, v;

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;
	MSG_WriteByte (&sv.datagram, svc_particle);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	for (i=0 ; i<3 ; i++)
	{
		v = dir[i]*16;
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		MSG_WriteChar (&sv.datagram, v);
	}
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, color);
}

#define DEFAULT_SOUND_PACKET_VOLUME 255
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

// TODO - make this varaible accessible in other places.
static int unique_event_id = 1;


/*
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
allready running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

==================
*/
void SV_StartSound (edict_t *entity, int channel, char *sample, int volume,
	float attenuation)
{
	int         sound_num;
	int			ent;
	edict_t*	event_edict;

	if (volume < 0 || volume > 255)
	{
		G_Error ("SV_StartSound: volume = %i", volume);
		return;
	}

	if (attenuation < 0 || attenuation > 4)
	{
		G_Error ("SV_StartSound: attenuation = %f", attenuation);
		return;
	}

	if (channel < 0 || channel > 7)
	{
		G_Error ("SV_StartSound: channel = %i", channel);
		return;
	}

// find precache number for sound
	for (sound_num=1 ; sound_num<MAX_SOUNDS
		&& sv.sound_precache[sound_num] ; sound_num++)
		if (!strcmp(sample, sv.sound_precache[sound_num]))
			break;

	if ( sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num] )
	{
		G_Printf ("SV_StartSound: %s not precacheed\n", sample);
		return;
	}

	ent = NUM_FOR_EDICT(entity);

	event_edict = ED_Alloc();
	VectorCopy(entity->v.origin, event_edict->v.origin);
	VectorCopy(entity->v.origin, event_edict->s.origin);

	event_edict->eventTime = level.time;
	event_edict->s.event = svc_sound;
	// Reuse some fileds for event params.
	event_edict->s.eventParm = ent;
	event_edict->s.powerups = volume;
	event_edict->s.weapon = attenuation;
	event_edict->s.legsAnim = channel;

	// Give unique ids for all events to distinguish different events for same entity number on client.
	event_edict->s.torsoAnim = unique_event_id;
	++unique_event_id;

	trap_LinkEntity(event_edict);
}


/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (char *name)
{
	int		i;

	if (!name || !name[0])
		return 0;

	for (i=0 ; i<MAX_MODELS && sv.model_precache[i] ; i++)
		if (!strcmp(sv.model_precache[i], name))
			return i;
	if (i==MAX_MODELS || !sv.model_precache[i])
		G_Error ("SV_ModelIndex: model %s not precached", name);
	return i;
}
