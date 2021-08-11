#include "g_local.h"


#define	SAVEGAME_VERSION	5


static void G_SavegameComment (char *text)
{
	int		i;
	char	kills[20];
	char	*mapname;

	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		text[i] = ' ';

	// PANZER TODO - check for buffer overflow?
	mapname = pr_strings + sv.edicts->v.message;
	memcpy (text, mapname, strlen(mapname));

	Com_sprintf (kills, sizeof(kills), "kills:%3i/%3i", (int)pr_global_struct->killed_monsters, (int)pr_global_struct->total_monsters);
	memcpy (text+22, kills, strlen(kills));

	// convert space to _ to make stdio happy
	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		if (text[i] == ' ')
			text[i] = '_';
	text[SAVEGAME_COMMENT_LENGTH] = '\0';
}

void G_SaveGame (const char* savename)
{
	char	name[256];
	fileHandle_t	f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];

	// PANZER TODO - return these checks back
	/*
	if (cl.intermission)
	{
		Con_Printf ("Can't save in intermission.\n");
		return;
	}


	if (svs.maxclients != 1)
	{
		Con_Printf ("Can't save multiplayer games.\n");
		return;
	}
	*/

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.health <= 0) )
		{
			G_Printf ("Can't savegame with a dead player\n");
			return;
		}
	}

	Com_sprintf (name, sizeof(name), "%s.sav", savename);
	G_Printf ("Saving game to %s...\n", name);
	f = 0;
	trap_FS_FOpenFile(name, &f, FS_WRITE);
	if (!f)
	{
		G_Printf ("ERROR: couldn't open.\n");
		return;
	}

	G_FilebufReset();

	G_FilebufWrite("%i\n", SAVEGAME_VERSION);
	G_SavegameComment (comment);
	G_FilebufWrite("%s\n", comment);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		G_FilebufWrite("%f\n", svs.clients->spawn_parms[i]);
	G_FilebufWrite("%d\n", current_skill);
	G_FilebufWrite("%s\n", sv.name);
	G_FilebufWrite("%f\n",sv.time);

// write the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		if (sv.lightstyles[i])
			G_FilebufWrite("%s\n", sv.lightstyles[i]);
		else
			G_FilebufWrite("m\n");
	}

	ED_WriteGlobals ();
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ED_Write (EDICT_NUM(i));
	}

	trap_FS_Write(G_FilebufGetData(), G_FilebufGetSize(), f);
	trap_FS_FCloseFile(f);

	G_Printf ("done.\n");
}
