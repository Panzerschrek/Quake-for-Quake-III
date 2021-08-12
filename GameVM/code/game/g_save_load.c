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


void G_LoadGame (const char* savename)
{
	char	name[MAX_OSPATH];
	char	mapname[MAX_QPATH];
	float	time, tfloat;
	char *start, *token;
	static char	str[32768]; // Make static to avoid 32k locals limit in lcc+q3ams.
	int		i, r;
	edict_t	*ent;
	int		entnum;
	int		version;
	float			spawn_parms[NUM_SPAWN_PARMS];

	Com_sprintf (name, sizeof(name), "%s.sav", savename);

	Com_Printf ("Loading game from %s...\n", name);
	G_FilebufLoadFile(name);

	version = atoi(G_FilebufReadLine());
	if (version != SAVEGAME_VERSION)
	{
		Com_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return;
	}
	strcpy(str, G_FilebufReadLine());
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		spawn_parms[i]= atof(G_FilebufReadLine());
// this silliness is so we can load 1.06 save files, which have float skill values
	tfloat = atof(G_FilebufReadLine());
	current_skill = (int)(tfloat + 0.1);

	skill.value = (float)current_skill;
	trap_Cvar_Set ("skill", va("%f", skill.value));

	strcpy(mapname, G_FilebufReadLine());
	time = atof(G_FilebufReadLine());

// load the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		strcpy(str, G_FilebufReadLine());
		sv.lightstyles[i] = G_Alloc (strlen(str)+1);
		strcpy (sv.lightstyles[i], str);
	}

// load the edicts out of the savegame file
	entnum = -1;		// -1 is the globals
	while (1)
	{
		for (i=0 ; i<sizeof(str)-1 ; i++)
		{
			r = G_FilebufGetChar ();
			if (!r)
				goto loop_end;
			str[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i == sizeof(str)-1)
			G_Error ("Loadgame buffer overflow");
		str[i] = 0;
		start = str;
		token = COM_Parse(&start);
		if (!token || !*token)
			break;		// end of file
		if (strcmp(token,"{"))
			G_Error ("First token isn't a brace");

		if (entnum == -1)
		{	// parse the global vars
			ED_ParseGlobals (start);
		}
		else
		{	// parse an edict

			ent = EDICT_NUM(entnum);
			memset (&ent->v, 0, progs->entityfields * 4);
			ent->free = qfalse;
			ED_ParseEdict (start, ent);

		// link it into the bsp tree
			if (!ent->free)
				SV_LinkEdict (ent, qfalse);
		}

		entnum++;
	}
	loop_end:

	sv.num_edicts = entnum;
	sv.time = time;

	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		svs.clients->spawn_parms[i] = spawn_parms[i];
}
