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
/*
=======================================================================

USER INTERFACE MAIN

=======================================================================
*/

#include "ui_local.h"

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
Q_EXPORT intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {
	switch ( command ) {
	case UI_GETAPIVERSION:
		return UI_API_VERSION;

	case UI_INIT:
		M_Init();
		return 0;

	case UI_SHUTDOWN:
		UI_Shutdown();
		return 0;

	case UI_KEY_EVENT:
		UI_KeyEvent( arg0, arg1 );
		return 0;

	case UI_MOUSE_EVENT:
		UI_MouseEvent( arg0, arg1 );
		return 0;

	case UI_REFRESH:
		UI_Refresh( arg0 );
		return 0;

	case UI_IS_FULLSCREEN:
		return UI_IsFullscreen();

	case UI_SET_ACTIVE_MENU:
		UI_SetActiveMenu( arg0 );
		return 0;

	case UI_CONSOLE_COMMAND:
		return UI_ConsoleCommand(arg0);

	case UI_DRAW_CONNECT_SCREEN:
		M_DrawLoadingScreen();
		return 0;

	case UI_HASUNIQUECDKEY:				// mod authors need to observe this
		return qtrue;  // change this to qfalse for mods!
	}

	return -1;
}

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
	return M_InputGrabbed() && !M_GetInGame();
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) {
	switch ( menu ) {
	case UIMENU_NONE:
		M_SetInGame(qtrue);
		M_UngrabInput();
		return;
	case UIMENU_MAIN:
		M_SetInGame(qfalse);
		M_Menu_Main_f();
		return;
	case UIMENU_NEED_CD:
		M_SetInGame(qfalse);
		return;
	case UIMENU_BAD_CD_KEY:
		M_SetInGame(qfalse);
		return;
	case UIMENU_INGAME:
		M_SetInGame(qtrue);
		M_Menu_Main_f();
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

/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	cmd[MAX_STRING_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if (!strcmp(cmd, "menu_main"))
		M_Menu_Main_f();
	else if (!strcmp(cmd, "menu_singleplayer"))
		M_Menu_SinglePlayer_f();
	else if (!strcmp(cmd, "menu_load"))
		M_Menu_Load_f();
	else if (!strcmp(cmd, "menu_save"))
		M_Menu_Save_f();
	else if (!strcmp(cmd, "menu_multiplayer"))
		M_Menu_MultiPlayer_f();
	else if (!strcmp(cmd, "menu_setup"))
		M_Menu_Setup_f();
	else if (!strcmp(cmd, "menu_options"))
		M_Menu_Options_f();
	else if (!strcmp(cmd, "menu_keys"))
		M_Menu_Keys_f();
	else if (!strcmp(cmd, "menu_video"))
		M_Menu_Video_f();
	else if (!strcmp(cmd, "help"))
		M_Menu_Help_f();
	else if (!strcmp(cmd, "menu_quit"))
		M_Menu_Quit_f();
	else
		return qfalse;

	return qtrue;
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
