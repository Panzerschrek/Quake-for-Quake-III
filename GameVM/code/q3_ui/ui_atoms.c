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
/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}


qboolean UI_IsFullscreen( void ) {
	return (trap_Key_GetCatcher() & KEYCATCH_UI) != 0;
}

static void NeedCDAction( qboolean result ) {
	if ( !result ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}

static void NeedCDKeyAction( qboolean result ) {
	if ( !result ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) {
	switch ( menu ) {
	case UIMENU_NONE:
		UI_ForceMenuOff();
		return;
	case UIMENU_MAIN:
		trap_Key_SetCatcher( KEYCATCH_UI );
		return;
	case UIMENU_NEED_CD:
		return;
	case UIMENU_BAD_CD_KEY:
		return;
	case UIMENU_INGAME:
		//trap_Cvar_Set( "cl_paused", "1" ); // PANZER TODO - enable pause
		M_Menu_Main_f();
		trap_Key_SetCatcher( KEYCATCH_UI );
		return;
		
	case UIMENU_TEAM:
	case UIMENU_POSTGAME:
	default:
#ifndef NDEBUG
	  Com_Printf("UI_SetActiveMenu: bad enum %d\n", menu );
#endif
	  break;
	}
}

/*
=================
UI_KeyEvent
=================
*/
void UI_KeyEvent( int key, int down ) {
	if( down )
		M_Keydown(key);
}

/*
=================
UI_MouseEvent
=================
*/
void UI_MouseEvent( int dx, int dy )
{
}

char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *UI_Cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}

/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	*cmd;

	cmd = UI_Argv( 0 );

	return qfalse;
}


void UI_ForceMenuOff (void)
{
	trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
	trap_Key_ClearStates();
	trap_Cvar_Set( "cl_paused", "0" );
}


/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown( void ) {
}


/*
=================
UI_Refresh
=================
*/
void UI_Refresh( int inRealtime )
{
	extern double		realtime, host_time;

	realtime = host_time = inRealtime / 1000.0;
	trap_R_SetColor(NULL);
	M_Draw();
}
