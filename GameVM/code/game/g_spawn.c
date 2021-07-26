/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//

#include "g_local.h"

// spawn variables
static int		numSpawnVars;
static char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
static int		numSpawnVarChars;
static char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string ) {
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = G_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	
	return newb;
}

/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specific spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars( void ) {
	int			i;
	qboolean	anglehack;
	char		keyname[256];
	char		value[256];
	int			n;
	ddef_t		*key;
	edict_t		*edict;
	dfunction_t	*func;

	edict = ED_Alloc ();

	for ( i = 0 ; i < numSpawnVars ; i++ ) {
		strcpy(keyname, spawnVars[i][0]);
		strcpy(value, spawnVars[i][1]);

		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		if (!strcmp(keyname, "angle"))
		{
			strcpy (keyname, "angles");
			anglehack = qtrue;
		}
		else
			anglehack = qfalse;

		if (!strcmp(keyname, "light"))
			strcpy (keyname, "light_lev");	// hack for single light def

		// another hack to fix heynames with trailing spaces
		n = strlen(keyname);
		while (n && keyname[n-1] == ' ')
		{
			keyname[n-1] = 0;
			n--;
		}

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		key = ED_FindField (keyname);
		if (!key)
		{
			G_Printf ("'%s' is not a field\n", keyname);
			continue;
		}

		if (anglehack)
		{
			char	temp[256];
			strcpy (temp, value);
			Com_sprintf (value, MAX_STRING_TOKENS, "0 %s 0", temp);
		}

		if (!ED_ParseEpair ((void *)&edict->v, key, value))
			G_Error ("ED_ParseEdict: parse error");
	}

	if (!edict->v.classname)
	{
		G_Printf ("No classname for:\n");
		ED_Print (edict);
		ED_Free(edict);
		return;
	}

	func = ED_FindFunction ( pr_strings + edict->v.classname );
	if (!func)
	{
		G_Printf ("No spawn function for:\n");
		ED_Print (edict);
		ED_Free (edict);
		return;
	}

	pr_global_struct->self = EDICT_TO_PROG(edict);
	PR_ExecuteProgram (func - pr_functions);

	// move editor origin to pos
	VectorCopy( edict->v.origin, edict->s.pos.trBase );
	VectorCopy( edict->v.origin, edict->r.currentOrigin );
	VectorCopy( edict->v.origin, edict->s.origin );

	trap_LinkEntity (edict);
}


/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) {
	int		l;
	char	*dest;

	l = strlen( string );
	if ( numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		G_Error( "G_AddSpawnVarToken: MAX_SPAWN_VARS_CHARS" );
	}

	dest = spawnVarChars + numSpawnVarChars;
	memcpy( dest, string, l+1 );

	numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( void ) {
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];

	numSpawnVars = 0;
	numSpawnVarChars = 0;

	// parse the opening brace
	if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		G_Error( "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 ) {	
		// parse key
		if ( !trap_GetEntityToken( keyname, sizeof( keyname ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' ) {
			break;
		}
		
		// parse value	
		if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( com_token[0] == '}' ) {
			G_Error( "G_ParseSpawnVars: closing brace without data" );
		}
		if ( numSpawnVars == MAX_SPAWN_VARS ) {
			G_Error( "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		spawnVars[ numSpawnVars ][0] = G_AddSpawnVarToken( keyname );
		spawnVars[ numSpawnVars ][1] = G_AddSpawnVarToken( com_token );
		numSpawnVars++;
	}

	return qtrue;
}

/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString( void ) {
	sv.max_edicts = MAX_EDICTS;

	sv.edicts = G_Alloc (sv.max_edicts*pr_edict_size);
	memset(sv.edicts, 0, sv.max_edicts*pr_edict_size);

	numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if ( !G_ParseSpawnVars() ) {
		G_Error( "SpawnEntities: no entities" );
	}

	// parse ents
	while( G_ParseSpawnVars() ) {
		G_SpawnGEntityFromSpawnVars();
	}	
}

