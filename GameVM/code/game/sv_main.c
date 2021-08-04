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


#define DEFAULT_SOUND_PACKET_VOLUME 255
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

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
	int			i;
	vec3_t		origin;
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

	// Start event for entity center to prevent dropping of event entity from snapshots list because of PVS/Areas check.
	for( i = 0; i < 3; ++i )
		origin[i]= entity->v.origin[i] + 0.5f * (entity->v.mins[i] + entity->v.maxs[i]);

	event_edict = G_CreateEventEdict(origin, svc_sound);

	// Reuse some fileds for event params.
	event_edict->s.eFlags = ent;
	event_edict->s.weapon = sound_num;
	event_edict->s.legsAnim = channel;
	event_edict->s.torsoAnim = attenuation;
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

static void SV_ProcessTEnt()
{
	int		type, ent;
	vec3_t	pos, pos2;
	edict_t* eventEntity;

	type = MSG_ReadByte ();
	switch (type)
	{
	case TE_WIZSPIKE:			// spike hitting wall
	case TE_KNIGHTSPIKE:			// spike hitting wall
	case TE_SPIKE:			// spike hitting wall
	case TE_SUPERSPIKE:			// super spike hitting wall
	case TE_GUNSHOT:			// bullet hitting wall
	case TE_EXPLOSION:			// rocket explosion
	case TE_TAREXPLOSION:			// tarbaby explosion
	case TE_LAVASPLASH:
	case TE_TELEPORT:
	case TE_EXPLOSION2:				// color mapped explosion
		// Entities with single param - position
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		eventEntity = G_CreateEventEdict(pos, svc_temp_entity);
		eventEntity->s.eType = type;
		break;

	case TE_LIGHTNING1:				// lightning bolts
	case TE_LIGHTNING2:				// lightning bolts
	case TE_LIGHTNING3:				// lightning bolts
	case TE_BEAM:				// grappling hook beam
		ent = MSG_ReadShort ();
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		pos2[0] = MSG_ReadCoord ();
		pos2[1] = MSG_ReadCoord ();
		pos2[2] = MSG_ReadCoord ();
		eventEntity = G_CreateEventEdict(pos, svc_temp_entity);
		eventEntity->s.eType = type;
		VectorCopy(pos2, eventEntity->s.origin2);
		break;

	default:
		G_Error ("CL_ParseTEnt: bad type");
	}
}

static void SV_ProcessIntermission()
{
	int i;

	i = 0;
	for( i = 0; i < svs.maxclients; ++ i )
	{
		if( !svs.clients[i].active )
			continue;

		svs.clients[i].ps.pm_type = PM_INTERMISSION;
	}
}

void SV_SendStringCommand(int clientNum, const char* commandName, const char* commandText)
{
	char	command[1024];
	char	*p;

	// double quotes are bad
	while ((p = strchr(commandText, '"')) != NULL)
		*p = '\'';

	Com_sprintf( command, sizeof(command), "%s \"%s\"", commandName, commandText );

	trap_SendServerCommand( clientNum, command );
}

void SV_SendPrint(int clientNum, const char* text)
{
	SV_SendStringCommand(clientNum, "print", text);
}

void SV_SendCenterPrint(int clientNum, const char* text)
{
	SV_SendStringCommand(clientNum, "centerprint", text);
}

void SV_SendStuffText(int clientNum, const char* text)
{
	SV_SendStringCommand(clientNum, "stufftext", text);
}

static void SV_ProcessBufferMessages(sizebuf_t* buf, int clientNum /* -1 for global messages */)
{
	int			cmd;
	int			i;

	memcpy(&net_message, buf, sizeof(*buf) );

	MSG_BeginReading();

	while(1)
	{
		cmd = MSG_ReadByte ();
		if (cmd == -1)
		{
			break;		// end of message
		}

		if (msg_badread)
			G_Error ("SV_ProcessBufferMessages: Bad server message");

		// if the high bit of the command byte is set, it is a fast update
		// PANZER TODO - parse fast update.
		if (cmd & 128)
		{
			//SHOWNET("fast update");
			//CL_ParseUpdate (cmd&127);
			continue;
		}

		// other commands
			switch (cmd)
			{
			default:
				G_Error ("SV_ProcessBufferMessages: Illegible server message %d\n", cmd);
				break;

			case svc_nop:
	//			Con_Printf ("svc_nop\n");
				break;

			case svc_time:
				MSG_ReadFloat ();
				break;

			case svc_clientdata:
				MSG_ReadShort ();
				break;

			case svc_version:
				i = MSG_ReadLong ();
				break;

			case svc_disconnect:
				break;

			case svc_print:
				SV_SendPrint(clientNum, MSG_ReadString());
				break;

			case svc_centerprint:
				SV_SendCenterPrint(clientNum, MSG_ReadString());
				break;

			case svc_stufftext:
				SV_SendStuffText(clientNum, MSG_ReadString());
				break;

			case svc_damage:
				// PANZER TODO - transmit damage.
				MSG_ReadByte ();
				MSG_ReadByte ();
				MSG_ReadCoord ();
				MSG_ReadCoord ();
				MSG_ReadCoord ();
				break;

			case svc_serverinfo:
				// Just ignore this - actual QuakeC does not send such message.
				break;

			case svc_setangle:
				MSG_ReadAngle();
				MSG_ReadAngle();
				MSG_ReadAngle();
				break;

			case svc_setview:
				MSG_ReadShort ();
				break;

			case svc_lightstyle:
				MSG_ReadByte ();
				MSG_ReadString();
				break;

			case svc_sound:
				// Just ignore this - actual QuakeC does not send such message.
				break;

			case svc_stopsound:
				MSG_ReadShort();
				break;

			case svc_updatename:
				MSG_ReadByte ();
				break;

			case svc_updatefrags:
				MSG_ReadByte ();
				break;

			case svc_updatecolors:
				MSG_ReadByte ();
				break;

			case svc_particle:
				// Just ignore this - actual QuakeC does not send such message.
				break;

			case svc_spawnbaseline:
				// Just ignore this - actual QuakeC does not send such message.
				break;
			case svc_spawnstatic:
				// Just ignore this - actual QuakeC does not send such message.
				break;
			case svc_temp_entity:
				SV_ProcessTEnt ();
				break;

			case svc_setpause:
				MSG_ReadByte ();
				break;

			case svc_signonnum:
				MSG_ReadByte ();
				break;

			case svc_killedmonster:
				break;

			case svc_foundsecret:
				break;

			case svc_updatestat:
				MSG_ReadByte ();
				break;

			case svc_spawnstaticsound:
				// Just ignore this - actual QuakeC does not send such message.
				break;

			case svc_cdtrack:
				MSG_ReadByte ();
				MSG_ReadByte ();
				break;

			case svc_intermission:
				SV_ProcessIntermission();
				break;

			case svc_finale:
				// TODO - transmit finale.
				break;

			case svc_cutscene:
				// TODO - transmit cutscene.
				MSG_ReadString ();
				break;

			case svc_sellscreen:
				break;
			}

	}

	buf->cursize = 0;
}

void SV_ProcessMessages()
{
	int i;

	SV_ProcessBufferMessages(&sv.datagram, -1);
	SV_ProcessBufferMessages(&sv.reliable_datagram, -1);
	SV_ProcessBufferMessages(&sv.signon, -1);

	for(i = 0; i < svs.maxclients; ++i)
	{
		if( !svs.clients[i].active ){
			continue;
		}

		SV_ProcessBufferMessages(&svs.clients[i].message, i);
	}
}
