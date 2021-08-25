/*
Copyright (C) 1996-1997 Id Software, Inc.
2016 Atröm "Panzerschrek" Kunç.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sbar.c -- status bar code

#include "cg_local.h"
#include "sbar.h"

float sb_scale = 1.0f;

int			sb_updates;		// if >= cg.refdef.numpages, no update needed

#define STAT_MINUS		10	// num frame for '-' stats digit


qboolean	sb_showscores;

int			sb_lines = 32;			// scan lines to draw

extern qboolean		standard_quake, rogue, hipnotic;


void Sbar_MiniDeathmatchOverlay (void);
void Sbar_DeathmatchOverlay (void);
void M_DrawPic (int x, int y, qhandle_t *pic);

/*
===============
Sbar_ShowScores

Tab key down
===============
*/
void Sbar_ShowScores (void)
{
	if (sb_showscores)
		return;
	sb_showscores = qtrue;
	sb_updates = 0;
}

/*
===============
Sbar_DontShowScores

Tab key up
===============
*/
void Sbar_DontShowScores (void)
{
	sb_showscores = qfalse;
	sb_updates = 0;
}


qhandle_t Draw_PicFromWad(const char* str)
{
	char nameTemp[MAX_OSPATH];
	char shaderName[MAX_OSPATH];
	strcpy(nameTemp, str);
	Q_strupr(nameTemp);
	Com_sprintf(shaderName, sizeof(shaderName), "gfx_wad/%s", nameTemp);
	return trap_R_RegisterShaderNoMip(shaderName);
}
/*
===============
Sbar_Init
===============
*/
void Sbar_Init (void)
{
	int		i;

	sbar.complete = trap_R_RegisterShaderNoMip("gfx/complete.tga");
	sbar.inter = trap_R_RegisterShaderNoMip("gfx/inter.tga");
	sbar.finale = trap_R_RegisterShaderNoMip("gfx/finale.tga");
	sbar.conchars = Draw_PicFromWad("conchars");
	sbar.pause = trap_R_RegisterShaderNoMip("gfx/pause.tga");

	for (i=0 ; i<10 ; i++)
	{
		sbar.sb_nums[0][i] = Draw_PicFromWad (va("num_%i",i));
		sbar.sb_nums[1][i] = Draw_PicFromWad (va("anum_%i",i));
	}

	sbar.sb_nums[0][10] = Draw_PicFromWad ("num_minus");
	sbar.sb_nums[1][10] = Draw_PicFromWad ("anum_minus");

	sbar.sb_colon = Draw_PicFromWad ("num_colon");
	sbar.sb_slash = Draw_PicFromWad ("num_slash");

	sbar.sb_weapons[0][0] = Draw_PicFromWad ("inv_shotgun");
	sbar.sb_weapons[0][1] = Draw_PicFromWad ("inv_sshotgun");
	sbar.sb_weapons[0][2] = Draw_PicFromWad ("inv_nailgun");
	sbar.sb_weapons[0][3] = Draw_PicFromWad ("inv_snailgun");
	sbar.sb_weapons[0][4] = Draw_PicFromWad ("inv_rlaunch");
	sbar.sb_weapons[0][5] = Draw_PicFromWad ("inv_srlaunch");
	sbar.sb_weapons[0][6] = Draw_PicFromWad ("inv_lightng");

	sbar.sb_weapons[1][0] = Draw_PicFromWad ("inv2_shotgun");
	sbar.sb_weapons[1][1] = Draw_PicFromWad ("inv2_sshotgun");
	sbar.sb_weapons[1][2] = Draw_PicFromWad ("inv2_nailgun");
	sbar.sb_weapons[1][3] = Draw_PicFromWad ("inv2_snailgun");
	sbar.sb_weapons[1][4] = Draw_PicFromWad ("inv2_rlaunch");
	sbar.sb_weapons[1][5] = Draw_PicFromWad ("inv2_srlaunch");
	sbar.sb_weapons[1][6] = Draw_PicFromWad ("inv2_lightng");

	for (i=0 ; i<5 ; i++)
	{
		sbar.sb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_shotgun",i+1));
		sbar.sb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_sshotgun",i+1));
		sbar.sb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_nailgun",i+1));
		sbar.sb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_snailgun",i+1));
		sbar.sb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_rlaunch",i+1));
		sbar.sb_weapons[2+i][5] = Draw_PicFromWad (va("inva%i_srlaunch",i+1));
		sbar.sb_weapons[2+i][6] = Draw_PicFromWad (va("inva%i_lightng",i+1));
	}

	sbar.sb_ammo[0] = Draw_PicFromWad ("sb_shells");
	sbar.sb_ammo[1] = Draw_PicFromWad ("sb_nails");
	sbar.sb_ammo[2] = Draw_PicFromWad ("sb_rocket");
	sbar.sb_ammo[3] = Draw_PicFromWad ("sb_cells");

	sbar.sb_armor[0] = Draw_PicFromWad ("sb_armor1");
	sbar.sb_armor[1] = Draw_PicFromWad ("sb_armor2");
	sbar.sb_armor[2] = Draw_PicFromWad ("sb_armor3");

	sbar.sb_items[0] = Draw_PicFromWad ("sb_key1");
	sbar.sb_items[1] = Draw_PicFromWad ("sb_key2");
	sbar.sb_items[2] = Draw_PicFromWad ("sb_invis");
	sbar.sb_items[3] = Draw_PicFromWad ("sb_invuln");
	sbar.sb_items[4] = Draw_PicFromWad ("sb_suit");
	sbar.sb_items[5] = Draw_PicFromWad ("sb_quad");

	sbar.sb_sigil[0] = Draw_PicFromWad ("sb_sigil1");
	sbar.sb_sigil[1] = Draw_PicFromWad ("sb_sigil2");
	sbar.sb_sigil[2] = Draw_PicFromWad ("sb_sigil3");
	sbar.sb_sigil[3] = Draw_PicFromWad ("sb_sigil4");

	sbar.sb_faces[4][0] = Draw_PicFromWad ("face1");
	sbar.sb_faces[4][1] = Draw_PicFromWad ("face_p1");
	sbar.sb_faces[3][0] = Draw_PicFromWad ("face2");
	sbar.sb_faces[3][1] = Draw_PicFromWad ("face_p2");
	sbar.sb_faces[2][0] = Draw_PicFromWad ("face3");
	sbar.sb_faces[2][1] = Draw_PicFromWad ("face_p3");
	sbar.sb_faces[1][0] = Draw_PicFromWad ("face4");
	sbar.sb_faces[1][1] = Draw_PicFromWad ("face_p4");
	sbar.sb_faces[0][0] = Draw_PicFromWad ("face5");
	sbar.sb_faces[0][1] = Draw_PicFromWad ("face_p5");

	sbar.sb_face_invis = Draw_PicFromWad ("face_invis");
	sbar.sb_face_invuln = Draw_PicFromWad ("face_invul2");
	sbar.sb_face_invis_invuln = Draw_PicFromWad ("face_inv2");
	sbar.sb_face_quad = Draw_PicFromWad ("face_quad");

	// PANZER - todo - fix it
	//Cmd_AddCommand ("+showscores", Sbar_ShowScores);
	//Cmd_AddCommand ("-showscores", Sbar_DontShowScores);

	sbar.sb_sbar = Draw_PicFromWad ("sbar");
	sbar.sb_ibar = Draw_PicFromWad ("ibar");
	sbar.sb_scorebar = Draw_PicFromWad ("scorebar");

//MED 01/04/97 added new hipnotic weapons
	if (hipnotic)
	{
	  sbar.hsb_weapons[0][0] = Draw_PicFromWad ("inv_laser");
	  sbar.hsb_weapons[0][1] = Draw_PicFromWad ("inv_mjolnir");
	  sbar.hsb_weapons[0][2] = Draw_PicFromWad ("inv_gren_prox");
	  sbar.hsb_weapons[0][3] = Draw_PicFromWad ("inv_prox_gren");
	  sbar.hsb_weapons[0][4] = Draw_PicFromWad ("inv_prox");

	  sbar.hsb_weapons[1][0] = Draw_PicFromWad ("inv2_laser");
	  sbar.hsb_weapons[1][1] = Draw_PicFromWad ("inv2_mjolnir");
	  sbar.hsb_weapons[1][2] = Draw_PicFromWad ("inv2_gren_prox");
	  sbar.hsb_weapons[1][3] = Draw_PicFromWad ("inv2_prox_gren");
	  sbar.hsb_weapons[1][4] = Draw_PicFromWad ("inv2_prox");

	  for (i=0 ; i<5 ; i++)
	  {
		 sbar.hsb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_laser",i+1));
		 sbar.hsb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_mjolnir",i+1));
		 sbar.hsb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_gren_prox",i+1));
		 sbar.hsb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_prox_gren",i+1));
		 sbar.hsb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_prox",i+1));
	  }

	  sbar.hsb_items[0] = Draw_PicFromWad ("sb_wsuit");
	  sbar.hsb_items[1] = Draw_PicFromWad ("sb_eshld");
	}

	if (rogue)
	{
		sbar.rsb_invbar[0] = Draw_PicFromWad ("r_invbar1");
		sbar.rsb_invbar[1] = Draw_PicFromWad ("r_invbar2");

		sbar.rsb_weapons[0] = Draw_PicFromWad ("r_lava");
		sbar.rsb_weapons[1] = Draw_PicFromWad ("r_superlava");
		sbar.rsb_weapons[2] = Draw_PicFromWad ("r_gren");
		sbar.rsb_weapons[3] = Draw_PicFromWad ("r_multirock");
		sbar.rsb_weapons[4] = Draw_PicFromWad ("r_plasma");

		sbar.rsb_items[0] = Draw_PicFromWad ("r_shield1");
		sbar.rsb_items[1] = Draw_PicFromWad ("r_agrav1");

// PGM 01/19/97 - team color border
		sbar.rsb_teambord = Draw_PicFromWad ("r_teambord");
// PGM 01/19/97 - team color border

		sbar.rsb_ammo[0] = Draw_PicFromWad ("r_ammolava");
		sbar.rsb_ammo[1] = Draw_PicFromWad ("r_ammomulti");
		sbar.rsb_ammo[2] = Draw_PicFromWad ("r_ammoplasma");
	}

	sbar.draw_disc = Draw_PicFromWad("disc");
}

// Panzer - move here wrappers

void Draw_Fill (int x, int y, int w, int h, int c)
{
}

void Draw_CharacterScaled (int x, int y, int scale, int num)
{
	float				row, col;

	num &= 255;
	if( num == ' ' )
		return;

	row = num>>4;
	col = num&15;

	trap_R_DrawStretchPic(
		x, y,
		8 * scale, 8 * scale,
		col / 16.0f,
		row / 16.0,
		col / 16.0 + 1.0 / 16.0,
		row / 16.0 + 1.0 / 16.0,
		sbar.conchars);
}

void Draw_StringScaled (int x, int y, int scale, const char *str)
{
	while (*str)
	{
		Draw_CharacterScaled (x, y, scale, *str);
		str++;
		x += 8 * scale;
	}
}


//=============================================================================

// drawing routines are relative to the status bar location


/*
=============
Sbar_DrawPicStretched
=============
*/
void Sbar_DrawPicStretched (float x, float y, float w, float h, qhandle_t pic)
{
	y*= sb_scale;
	if (cg.gametype == GAME_DEATHMATCH)
		trap_R_DrawStretchPic (x * sb_scale /* + ((cg.refdef.width - SBAR_WIDTH)>>1)*/, y + (cg.refdef.height-SBAR_HEIGHT * sb_scale), w * sb_scale, h * sb_scale, 0.0, 0.0, 1.0, 1.0, pic);
	else
		trap_R_DrawStretchPic (x * sb_scale + (cg.refdef.width - SBAR_WIDTH * sb_scale) * 0.5f, y + (cg.refdef.height-SBAR_HEIGHT * sb_scale), w * sb_scale, h * sb_scale, 0.0, 0.0, 1.0, 1.0, pic);
}

/*
=============
Sbar_DrawTransPicStretched
=============
*/
void Sbar_DrawTransPicStretched (float x, float y, float w, float h, qhandle_t pic)
{
	Sbar_DrawPicStretched(x, y, w, h, pic);
}

/*
================
Sbar_DrawCharacter

Draws one solid graphics character
================
*/
void Sbar_DrawCharacter (float x, float y, int num)
{
	x *= sb_scale;
	y *= sb_scale;
	if (cg.gametype == GAME_DEATHMATCH)
		Draw_CharacterScaled ( x/*+ ((cg.refdef.width - SBAR_WIDTH)>>1) */ + 4 , y + cg.refdef.height-SBAR_HEIGHT * sb_scale, sb_scale, num);
	else
		Draw_CharacterScaled ( x + (cg.refdef.width - SBAR_WIDTH * sb_scale) * 0.5f + 4 , y + cg.refdef.height-SBAR_HEIGHT * sb_scale, sb_scale, num);
}

/*
================
Sbar_DrawString
================
*/
void Sbar_DrawString (int x, int y, char *str)
{
	x *= sb_scale;
	y *= sb_scale;
	if (cg.gametype == GAME_DEATHMATCH)
		Draw_StringScaled (x /*+ ((cg.refdef.width - SBAR_WIDTH)>>1)*/, y+ cg.refdef.height-SBAR_HEIGHT * sb_scale, sb_scale, str);
	else
		Draw_StringScaled (x + (cg.refdef.width - SBAR_WIDTH * sb_scale) * 0.5f, y+ cg.refdef.height-SBAR_HEIGHT * sb_scale, sb_scale, str);
}

/*
=============
Sbar_itoa
=============
*/
int Sbar_itoa (int num, char *buf)
{
	char	*str;
	int		pow10;
	int		dig;

	str = buf;

	if (num < 0)
	{
		*str++ = '-';
		num = -num;
	}

	for (pow10 = 10 ; num >= pow10 ; pow10 *= 10)
	;

	do
	{
		pow10 /= 10;
		dig = num/pow10;
		*str++ = '0'+dig;
		num -= dig*pow10;
	} while (pow10 != 1);

	*str = 0;

	return str-buf;
}


/*
=============
Sbar_DrawNum
=============
*/
void Sbar_DrawNum (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	int num_width = 24;
	int num_height= 24;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		Sbar_DrawTransPicStretched (x,y, num_width, num_height, sbar.sb_nums[color][frame]);
		x += 24;
		ptr++;
	}
}

//=============================================================================

int		fragsort[MAX_SCOREBOARD];

char	scoreboardtext[MAX_SCOREBOARD][20];
int		scoreboardtop[MAX_SCOREBOARD];
int		scoreboardbottom[MAX_SCOREBOARD];
int		scoreboardcount[MAX_SCOREBOARD];
int		scoreboardlines;

/*
===============
Sbar_SortFrags
===============
*/
void Sbar_SortFrags (void)
{
	int		i, j, k;

// sort by frags
	scoreboardlines = 0;
	for (i=0 ; i<cg.maxclients ; i++)
	{
		if (cg.scores[i].name[0])
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cg.scores[fragsort[j]].frags < cg.scores[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
}

int	Sbar_ColorForMap (int m)
{
	return m < 128 ? m + 8 : m + 8;
}

/*
===============
Sbar_UpdateScoreboard
===============
*/
void Sbar_UpdateScoreboard (void)
{
	int		i, k;
	int		top, bottom;
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	memset (scoreboardtext, 0, sizeof(scoreboardtext));

	for (i=0 ; i<scoreboardlines; i++)
	{
		k = fragsort[i];
		s = &cg.scores[k];
		Com_sprintf (&scoreboardtext[i][1], 20, "%3i %s", s->frags, s->name);

		top = s->colors & 0xf0;
		bottom = (s->colors & 15) <<4;
		scoreboardtop[i] = Sbar_ColorForMap (top);
		scoreboardbottom[i] = Sbar_ColorForMap (bottom);
	}
}



/*
===============
Sbar_SoloScoreboard
===============
*/
void Sbar_SoloScoreboard (void)
{
	char	str[80];
	int		minutes, seconds, tens, units;
	int		l;

	Com_sprintf (str, sizeof(str), "Monsters:%3i /%3i", cg.snap.ps.stats[STAT_MONSTERS], cg.snap.ps.stats[STAT_TOTALMONSTERS]);
	Sbar_DrawString (8, 4, str);

	Com_sprintf (str, sizeof(str), "Secrets :%3i /%3i", cg.snap.ps.stats[STAT_SECRETS], cg.snap.ps.stats[STAT_TOTALSECRETS]);
	Sbar_DrawString (8, 12, str);

// time
	minutes = cg.snap.ps.stats[STAT_TIME] / 60;
	seconds = cg.snap.ps.stats[STAT_TIME] - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	Com_sprintf (str, sizeof(str), "Time :%3i:%i%i", minutes, tens, units);
	Sbar_DrawString (184, 4, str);

// draw level name
	l = strlen (cg.levelname);
	Sbar_DrawString (232 - l*4, 12, cg.levelname);
}

/*
===============
Sbar_DrawScoreboard
===============
*/
void Sbar_DrawScoreboard (void)
{
	Sbar_SoloScoreboard ();
	if (cg.gametype == GAME_DEATHMATCH)
		Sbar_DeathmatchOverlay ();
#if 0
	int		i, j, c;
	int		x, y;
	int		l;
	int		top, bottom;
	scoreboard_t	*s;

	if (cg.gametype != GAME_DEATHMATCH)
	{
		Sbar_SoloScoreboard ();
		return;
	}

	Sbar_UpdateScoreboard ();

	l = scoreboardlines <= 6 ? scoreboardlines : 6;

	for (i=0 ; i<l ; i++)
	{
		x = 20*(i&1);
		y = i/2 * 8;

		s = &cg.scores[fragsort[i]];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x*8+10 + ((cg.refdef.width - SBAR_WIDTH)>>1), y + cg.refdef.height - SBAR_HEIGHT, 28, 4, top);
		Draw_Fill ( x*8+10 + ((cg.refdef.width - SBAR_WIDTH)>>1), y+4 + cg.refdef.height - SBAR_HEIGHT, 28, 4, bottom);

	// draw text
		for (j=0 ; j<20 ; j++)
		{
			c = scoreboardtext[i][j];
			if (c == 0 || c == ' ')
				continue;
			Sbar_DrawCharacter ( (x+j)*8, y, c);
		}
	}
#endif
}


//=============================================================================

/*
===============
Sbar_DrawInventory
===============
*/
void Sbar_DrawInventory (void)
{
	int		i;
	char	num[6];
	int		time;
	int		flashon;

	const int inv_width= SBAR_WIDTH;
	const int inv_height= 24;
	const int weapon_width[8]= {24, 24, 24, 24, 24, 24, 48, 24 };
	const int weapon_height = inv_height;
	const int sigil_width = 8;
	const int sigil_height = 16;
	const int item_width = 16;
	const int item_height = 16;
	const int addon_item_width = 16;
	const int addon_item_height = 16;

	if (rogue)
	{
		if ( cg.snap.ps.stats[STAT_ACTIVE_WEAPON] >= RIT_LAVA_NAILGUN )
			Sbar_DrawPicStretched (0, -24, inv_width, inv_height, sbar.rsb_invbar[0]);
		else
			Sbar_DrawPicStretched (0, -24, inv_width, inv_height, sbar.rsb_invbar[1]);
	}
	else
	{
		Sbar_DrawPicStretched (0, -24, inv_width, inv_height, sbar.sb_ibar);
	}

// weapons
	for (i=0 ; i<7 ; i++)
	{
		if (GetItems() & (IT_SHOTGUN<<i) )
		{
			time = cg.item_gettime[i];
			flashon = (cg.time - time) / 100;
			if (flashon >= 10)
			{
				if ( cg.snap.ps.stats[STAT_ACTIVE_WEAPON] == (IT_SHOTGUN<<i)  )
					flashon = 1;
				else
					flashon = 0;
			}
			else
				flashon = (flashon%5) + 2;

			Sbar_DrawPicStretched (i*24, -16, weapon_width[i], weapon_height, sbar.sb_weapons[flashon][i]);

			if (flashon > 1)
				sb_updates = 0;		// force update to remove flash
		}
	}

	// PANZER TODO - check addons.

// MED 01/04/97
// hipnotic weapons
	if (hipnotic)
	{
	  int grenadeflashing=0;
	  for (i=0 ; i<4 ; i++)
	  {
		 if (GetItems() & (1<<sbar.hipweapons[i]) )
		 {
			time = cg.item_gettime[sbar.hipweapons[i]];
			flashon = (cg.time - time) / 100;
			if (flashon >= 10)
			{
				if ( cg.snap.ps.stats[STAT_ACTIVE_WEAPON] == (1<<sbar.hipweapons[i]) )
					flashon = 1;
				else
					flashon = 0;
			}
			else
				flashon = (flashon%5) + 2;

			// check grenade launcher
			if (i==2)
			{
			   if (GetItems() & HIT_PROXIMITY_GUN)
			   {
				  if (flashon)
				  {
					 grenadeflashing = 1;
					 Sbar_DrawPicStretched (96, -16, weapon_width[i], weapon_height, sbar.hsb_weapons[flashon][2]);
				  }
			   }
			}
			else if (i==3)
			{
			   if (GetItems() & (IT_SHOTGUN<<4))
			   {
				  if (flashon && !grenadeflashing)
				  {
					 Sbar_DrawPicStretched (96, -16, weapon_width[i], weapon_height, sbar.hsb_weapons[flashon][3]);
				  }
				  else if (!grenadeflashing)
				  {
					 Sbar_DrawPicStretched (96, -16, weapon_width[i], weapon_height, sbar.hsb_weapons[0][3]);
				  }
			   }
			   else
				  Sbar_DrawPicStretched (96, -16, weapon_width[i], weapon_height, sbar.hsb_weapons[flashon][4]);
			}
			else
			   Sbar_DrawPicStretched (176 + (i*24), -16, weapon_width[i], weapon_height, sbar.hsb_weapons[flashon][i]);
			if (flashon > 1)
			   sb_updates = 0;      // force update to remove flash
		 }
	  }
	}

	if (rogue)
	{
	// check for powered up weapon.
		if ( cg.snap.ps.stats[STAT_ACTIVE_WEAPON] >= RIT_LAVA_NAILGUN )
		{
			for (i=0;i<5;i++)
			{
				if (cg.snap.ps.stats[STAT_ACTIVE_WEAPON] == (RIT_LAVA_NAILGUN << i))
				{
					Sbar_DrawPicStretched ((i+2)*24, -16, weapon_width[i], weapon_height, sbar.rsb_weapons[i]);
				}
			}
		}
	}

// ammo counts
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (num, sizeof(num), "%3i",cg.snap.ps.stats[STAT_SHELLS+i] );
		if (num[0] != ' ')
			Sbar_DrawCharacter ( (6*i+1)*8 - 2, -24, 18 + num[0] - '0');
		if (num[1] != ' ')
			Sbar_DrawCharacter ( (6*i+2)*8 - 2, -24, 18 + num[1] - '0');
		if (num[2] != ' ')
			Sbar_DrawCharacter ( (6*i+3)*8 - 2, -24, 18 + num[2] - '0');
	}

	flashon = 0;
   // items
   for (i=0 ; i<6 ; i++)
	  if (GetItems() & (1<<(17+i)))
	  {
		  time = cg.item_gettime[17+i];
		  if (time && time > cg.time - 2000 && flashon )
		  {  // flash frame
			 sb_updates = 0;
		  }
		  else
		  {
		  //MED 01/04/97 changed keys
			 if (!hipnotic || (i>1))
			 {
				Sbar_DrawPicStretched (192 + i*16, -16, item_width, item_height, sbar.sb_items[i]);
			 }
		  }
		  if (time && time > cg.time - 2000)
			 sb_updates = 0;
	  }
   //MED 01/04/97 added hipnotic items
   // hipnotic items
   if (hipnotic)
   {
	  for (i=0 ; i<2 ; i++)
		 if (GetItems() & (1<<(24+i)))
		 {
			time = cg.item_gettime[24+i];
			if (time && time > cg.time - 2000 && flashon )
			{  // flash frame
			   sb_updates = 0;
			}
			else
			{
			   Sbar_DrawPicStretched (288 + i*16, -16, addon_item_width, addon_item_height, sbar.hsb_items[i]);
			}
			if (time && time > cg.time / 1000 - 2000)
			   sb_updates = 0;
		 }
   }

	if (rogue)
	{
	// new rogue items
		for (i=0 ; i<2 ; i++)
		{
			if (GetItems() & (1<<(29+i)))
			{
				time = cg.item_gettime[29+i];

				if (time &&	time > cg.time - 2000 && flashon )
				{	// flash frame
					sb_updates = 0;
				}
				else
				{
					Sbar_DrawPicStretched (288 + i*16, -16, addon_item_width, addon_item_height, sbar.rsb_items[i]);
				}

				if (time &&	time > cg.time - 2000)
					sb_updates = 0;
			}
		}
	}
	else
	{
	// sigils
		for (i=0 ; i<4 ; i++)
		{
			if (GetItems() & (1<<(28+i)))
			{
				time = cg.item_gettime[28+i];
				if (time &&	time > cg.time - 2000 && flashon )
				{	// flash frame
					sb_updates = 0;
				}
				else
					Sbar_DrawPicStretched (SBAR_WIDTH-32 + i*8, -16, sigil_width, sigil_height, sbar.sb_sigil[i]);
				if (time &&	time > cg.time - 2000)
					sb_updates = 0;
			}
		}
	}
}

//=============================================================================

/*
===============
Sbar_DrawFrags
===============
*/
void Sbar_DrawFrags (void)
{
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	int				xofs;
	char			num[12];
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines <= 4 ? scoreboardlines : 4;

	x = 23;
	if (cg.gametype == GAME_DEATHMATCH)
		xofs = 0;
	else
		xofs = (cg.refdef.width - SBAR_WIDTH)>>1;
	y = cg.refdef.height - SBAR_HEIGHT - 23;

	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cg.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill (xofs + x*8 + 10, y, 28, 4, top);
		Draw_Fill (xofs + x*8 + 10, y+4, 28, 3, bottom);

	// draw number
		f = s->frags;
		Com_sprintf (num, sizeof(num), "%3i",f);

		Sbar_DrawCharacter ( (x+1)*8 , -24, num[0]);
		Sbar_DrawCharacter ( (x+2)*8 , -24, num[1]);
		Sbar_DrawCharacter ( (x+3)*8 , -24, num[2]);

		if (k == cg.viewentity - 1)
		{
			Sbar_DrawCharacter (x*8+2, -24, 16);
			Sbar_DrawCharacter ( (x+4)*8-4, -24, 17);
		}
		x+=4;
	}
}

//=============================================================================


/*
===============
Sbar_DrawFace
===============
*/
void Sbar_DrawFace (void)
{
	int		f, anim;

	const int face_width= 24;
	const int face_height= 24;

// PGM 01/19/97 - team color drawing
// PGM 03/02/97 - fixed so color swatch only appears in CTF modes
	if (rogue &&
		(cg.maxclients != 1) &&
		(teamplay.value>3) &&
		(teamplay.value<7))
	{
		int				top, bottom;
		int				xofs;
		char			num[12];
		scoreboard_t	*s;

		s = &cg.scores[cg.viewentity - 1];
		// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		if (cg.gametype == GAME_DEATHMATCH)
			xofs = 113;
		else
			xofs = ((cg.refdef.width - SBAR_WIDTH)>>1) + 113;

		Sbar_DrawPicStretched (112, 0, face_width, face_height, sbar.rsb_teambord);
		Draw_Fill (xofs, cg.refdef.height-SBAR_HEIGHT+3, 22, 9, top);
		Draw_Fill (xofs, cg.refdef.height-SBAR_HEIGHT+12, 22, 9, bottom);

		// draw number
		f = s->frags;
		Com_sprintf (num, sizeof(num), "%3i",f);

		if (top==8)
		{
			if (num[0] != ' ')
				Sbar_DrawCharacter(109, 3, 18 + num[0] - '0');
			if (num[1] != ' ')
				Sbar_DrawCharacter(116, 3, 18 + num[1] - '0');
			if (num[2] != ' ')
				Sbar_DrawCharacter(123, 3, 18 + num[2] - '0');
		}
		else
		{
			Sbar_DrawCharacter ( 109, 3, num[0]);
			Sbar_DrawCharacter ( 116, 3, num[1]);
			Sbar_DrawCharacter ( 123, 3, num[2]);
		}

		return;
	}
// PGM 01/19/97 - team color drawing

	if ( (GetItems() & (IT_INVISIBILITY | IT_INVULNERABILITY) )
	== (IT_INVISIBILITY | IT_INVULNERABILITY) )
	{
		Sbar_DrawPicStretched (112, 0, face_width, face_height, sbar.sb_face_invis_invuln);
		return;
	}
	if (GetItems() & IT_QUAD)
	{
		Sbar_DrawPicStretched (112, 0, face_width, face_height, sbar.sb_face_quad );
		return;
	}
	if (GetItems() & IT_INVISIBILITY)
	{
		Sbar_DrawPicStretched (112, 0, face_width, face_height,  sbar.sb_face_invis );
		return;
	}
	if (GetItems() & IT_INVULNERABILITY)
	{
		Sbar_DrawPicStretched (112, 0, face_width, face_height,  sbar.sb_face_invuln);
		return;
	}

	if (cg.snap.ps.stats[STAT_HEALTH] >= 100)
		f = 4;
	else
		f = cg.snap.ps.stats[STAT_HEALTH] / 20;

	if (cg.time <= cg.faceanimtime)
	{
		anim = 1;
		sb_updates = 0;		// make sure the anim gets drawn over
	}
	else
		anim = 0;
	Sbar_DrawPicStretched (112, 0, face_width, face_height, sbar.sb_faces[f][anim]);
}

static void Sbar_CalculateScale()
{
	float scale_h, scale_v;

	sb_scale = cg_sbar_scale.value;
	if( sb_scale <- 1.0f )
		sb_scale = 1.0f;
	else if( sb_scale <= 1.5f )
		sb_scale = 1.5f;
	else
		sb_scale = (int)(sb_scale + 0.999f);

	scale_h = cg.refdef.width / 320.0f;
	scale_v = cg.refdef.height / 200.0f;
	if( sb_scale > scale_h )
		sb_scale = (int)scale_h;
	if( sb_scale > scale_v )
		sb_scale = (int)scale_v;
}

static void Sbar_CalculateLines()
{
	if(cg_sbar_lines.value <= 0.0f )
		sb_lines = 0;
	else if(cg_sbar_lines.value <= 1.0f )
		sb_lines = 24;
	else
		sb_lines = 48;
}


/*
===============
Sbar_Draw
===============
*/
void Sbar_Draw (void)
{
	const int sbar_width = SBAR_WIDTH;
	const int sbar_height = 24;
	const int ammo_width = 24;
	const int ammo_height = 24;
	const int armor_width = 24;
	const int armor_height = 24;
	const int item_width = 16;
	const int item_height = 16;

	float color[4];
	color[0]= 1.0f;
	color[1]= 1.0f;
	color[2]= 1.0f;
	color[3]= cg_sbar_alpha.value;
	if (color[3] < 0.1f)
		color[3]= 0.1f;
	if (color[3] > 1.0f)
		color[3]= 1.0f;
	trap_R_SetColor(color);

	Sbar_CalculateScale();
	Sbar_CalculateLines();

	sb_updates++;

	if (sb_lines > 24)
	{
		Sbar_DrawInventory ();
		if (cg.maxclients != 1)
			Sbar_DrawFrags ();
	}

	if (sb_showscores || cg.snap.ps.stats[STAT_HEALTH] <= 0)
	{
		Sbar_DrawPicStretched (0, 0, sbar_width, sbar_height, sbar.sb_scorebar);
		Sbar_DrawScoreboard ();
		sb_updates = 0;
	}
	else if (sb_lines)
	{
		Sbar_DrawPicStretched (0, 0, sbar_width, sbar_height, sbar.sb_sbar);

   // keys (hipnotic only)
	  //MED 01/04/97 moved keys here so they would not be overwritten
	  if (hipnotic)
	  {
		 if (GetItems() & IT_KEY1)
			Sbar_DrawPicStretched (209, 3, item_width, item_height, sbar.sb_items[0]);
		 if (GetItems() & IT_KEY2)
			Sbar_DrawPicStretched (209, 12, item_width, item_height, sbar.sb_items[1]);
	  }
   // armor
		if (GetItems() & IT_INVULNERABILITY)
		{
			Sbar_DrawNum (24, 0, 666, 3, 1);
			Sbar_DrawPicStretched (0, 0, armor_width, armor_height, sbar.draw_disc);
		}
		else
		{
			if (rogue)
			{
				Sbar_DrawNum (24, 0, cg.snap.ps.stats[STAT_ARMOR], 3,
								cg.snap.ps.stats[STAT_ARMOR] <= 25);
				if (GetItems() & RIT_ARMOR3)
					Sbar_DrawPicStretched (0, 0, armor_width, armor_height, sbar.sb_armor[2]);
				else if (GetItems() & RIT_ARMOR2)
					Sbar_DrawPicStretched (0, 0, armor_width, armor_height, sbar.sb_armor[1]);
				else if (GetItems() & RIT_ARMOR1)
					Sbar_DrawPicStretched (0, 0, armor_width, armor_height, sbar.sb_armor[0]);
			}
			else
			{
				Sbar_DrawNum (24, 0, cg.snap.ps.stats[STAT_ARMOR], 3
				, cg.snap.ps.stats[STAT_ARMOR] <= 25);
				if (GetItems() & IT_ARMOR3)
					Sbar_DrawPicStretched (0, 0, armor_width, armor_height, sbar.sb_armor[2]);
				else if (GetItems() & IT_ARMOR2)
					Sbar_DrawPicStretched (0, 0, armor_width, armor_height, sbar.sb_armor[1]);
				else if (GetItems() & IT_ARMOR1)
					Sbar_DrawPicStretched (0, 0, armor_width, armor_height, sbar.sb_armor[0]);
			}
		}

	// face
		Sbar_DrawFace ();

	// health
		Sbar_DrawNum (136, 0, cg.snap.ps.stats[STAT_HEALTH], 3
		, cg.snap.ps.stats[STAT_HEALTH] <= 25);

	// ammo icon
		if (rogue)
		{
			if (GetItems() & RIT_SHELLS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[0]);
			else if (GetItems() & RIT_NAILS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[1]);
			else if (GetItems() & RIT_ROCKETS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[2]);
			else if (GetItems() & RIT_CELLS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[3]);
			else if (GetItems() & RIT_LAVA_NAILS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.rsb_ammo[0]);
			else if (GetItems() & RIT_PLASMA_AMMO)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.rsb_ammo[1]);
			else if (GetItems() & RIT_MULTI_ROCKETS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.rsb_ammo[2]);
		}
		else
		{
			if (GetItems() & IT_SHELLS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[0]);
			else if (GetItems() & IT_NAILS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[1]);
			else if (GetItems() & IT_ROCKETS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[2]);
			else if (GetItems() & IT_CELLS)
				Sbar_DrawPicStretched (224, 0, ammo_width, ammo_height, sbar.sb_ammo[3]);
		}

		Sbar_DrawNum (248, 0, cg.snap.ps.stats[STAT_CURRENT_AMMO], 3,
					  cg.snap.ps.stats[STAT_CURRENT_AMMO] <= 10);
	}

	if (cg.refdef.width > SBAR_WIDTH) {
		if (cg.gametype == GAME_DEATHMATCH)
			Sbar_MiniDeathmatchOverlay ();
	}

	trap_R_SetColor(NULL);
}

//=============================================================================

/*
==================
Sbar_IntermissionNumber

==================
*/
void Sbar_IntermissionNumber (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	const int num_width = 24;
	const int num_height = 24;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24 * sb_scale;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		trap_R_DrawStretchPic (x, y, num_width * sb_scale, num_height * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.sb_nums[color][frame]);
		x += 24 * sb_scale;
		ptr++;
	}
}

/*
==================
Sbar_DeathmatchOverlay

==================
*/
void Sbar_DeathmatchOverlay (void)
{
	qhandle_t			pic;
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	scoreboard_t	*s;

#if 0 // PANZER TODO - fix it
	pic = Draw_CachePic ("gfx/ranking.lmp");
	Draw_PicScaled ( ( cg.refdef.width - pic->width * sb_scale ) / 2, 8 * sb_scale, sb_scale, pic);
#endif

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;

	x = 80 * sb_scale + (cg.refdef.width - SBAR_WIDTH * sb_scale) * 0.5f;
	y = 40 * sb_scale;
	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cg.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x, y, 40 * sb_scale, 4 * sb_scale, top);
		Draw_Fill ( x, y + 4 * sb_scale, 40 * sb_scale, 4 * sb_scale, bottom);

	// draw number
		f = s->frags;
		Com_sprintf (num, sizeof(num), "%3i",f);

		Draw_CharacterScaled ( x + 8 * sb_scale , y, sb_scale, num[0]);
		Draw_CharacterScaled ( x + 16 * sb_scale, y, sb_scale, num[1]);
		Draw_CharacterScaled ( x + 24 * sb_scale, y, sb_scale, num[2]);

		if (k == cg.viewentity - 1)
			Draw_CharacterScaled ( x - 8 * sb_scale, y, sb_scale, 12);

	// draw name
		Draw_StringScaled (x+64 * sb_scale, y, sb_scale, s->name);

		y += 10 * sb_scale;
	}
}

/*
==================
Sbar_DeathmatchOverlay

==================
*/
void Sbar_MiniDeathmatchOverlay (void)
{
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	scoreboard_t	*s;
	int				numlines;

	if (cg.refdef.width < 512 * sb_scale || !sb_lines)
		return;

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;
	y = cg.refdef.height - sb_lines * sb_scale;
	numlines = (sb_scale * sb_lines) /8;
	if (numlines < 3)
		return;

	//find us
	for (i = 0; i < scoreboardlines; i++)
		if (fragsort[i] == cg.viewentity - 1)
			break;

	if (i == scoreboardlines) // we're not there
			i = 0;
	else // figure out start
			i = i - numlines/2;

	if (i > scoreboardlines - numlines)
			i = scoreboardlines - numlines;
	if (i < 0)
			i = 0;

	x = 324 * sb_scale;
	for (/* */; i < scoreboardlines && y < cg.refdef.height - 8 * sb_scale ; i++)
	{
		k = fragsort[i];
		s = &cg.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x, y + 1 * sb_scale, 40 * sb_scale, 3 * sb_scale, top);
		Draw_Fill ( x, y + 4 * sb_scale, 40 * sb_scale, 4 * sb_scale, bottom);

	// draw number
		f = s->frags;
		Com_sprintf (num, sizeof(num), "%3i",f);

		Draw_CharacterScaled ( x + 8  * sb_scale, y, sb_scale, num[0]);
		Draw_CharacterScaled ( x + 16 * sb_scale, y, sb_scale, num[1]);
		Draw_CharacterScaled ( x + 24 * sb_scale, y, sb_scale, num[2]);

		if (k == cg.viewentity - 1) {
			Draw_CharacterScaled ( x, y, sb_scale, 16);
			Draw_CharacterScaled ( x + 32 * sb_scale, y, sb_scale, 17);
		}

#if 0
{
	int				total;
	int				n, minutes, tens, units;

	// draw time
		total = cg.completed_time - s->entertime;
		minutes = (int)total/60;
		n = total - minutes*60;
		tens = n/10;
		units = n%10;

		Com_sprintf (num, sizeof(num), "%3i:%i%i", minutes, tens, units);

		Draw_String ( x+48 , y, num);
}
#endif

	// draw name
		Draw_StringScaled (x + 48 * sb_scale, y, sb_scale, s->name);

		y += 8 * sb_scale;
	}
}

/*
==================
Sbar_IntermissionOverlay

==================
*/
void Sbar_IntermissionOverlay (void)
{
	int		dig;
	int		num;
	int		x_ofs;

	const int colon_width = 16;
	const int colon_height = 24;
	const int num_width = 24;
	const int num_height = 24;
	const int slash_width = 16;
	const int slash_height = 24;

	if (cg.gametype == GAME_DEATHMATCH)
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	x_ofs = (cg.refdef.width - SBAR_WIDTH * sb_scale) * 0.5f;
	trap_R_DrawStretchPic (x_ofs + 64 * sb_scale, 24 * sb_scale, 192 * sb_scale, 24 * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.complete);

	trap_R_DrawStretchPic (x_ofs + 0 * sb_scale, 56 * sb_scale, 160 * sb_scale, 144 * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.inter);

// time
	dig = cg.snap.ps.pm_time/60;
	Sbar_IntermissionNumber (x_ofs + 160 * sb_scale, 64 * sb_scale, dig, 3, 0);
	num = cg.snap.ps.pm_time - dig*60;
	trap_R_DrawStretchPic (x_ofs + 234 * sb_scale,64 * sb_scale, colon_width * sb_scale, colon_height * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.sb_colon);
	trap_R_DrawStretchPic (x_ofs + 246 * sb_scale,64 * sb_scale, num_width * sb_scale, num_height * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.sb_nums[0][num/10]);
	trap_R_DrawStretchPic (x_ofs + 266 * sb_scale,64 * sb_scale, num_width * sb_scale, num_height * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.sb_nums[0][num%10]);

	Sbar_IntermissionNumber (x_ofs + 160 * sb_scale, 104 * sb_scale, cg.snap.ps.stats[STAT_SECRETS], 3, 0);
	trap_R_DrawStretchPic (x_ofs + 232 * sb_scale, 104 * sb_scale, slash_width * sb_scale, slash_height * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.sb_slash);
	Sbar_IntermissionNumber (x_ofs + 240 * sb_scale, 104 * sb_scale, cg.snap.ps.stats[STAT_TOTALSECRETS], 3, 0);

	Sbar_IntermissionNumber (x_ofs + 160 * sb_scale, 144 * sb_scale, cg.snap.ps.stats[STAT_MONSTERS], 3, 0);
	trap_R_DrawStretchPic (x_ofs + 232 * sb_scale, 144 * sb_scale, slash_width * sb_scale, slash_height * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.sb_slash);
	Sbar_IntermissionNumber (x_ofs + 240 * sb_scale, 144 * sb_scale, cg.snap.ps.stats[STAT_TOTALMONSTERS], 3, 0);
}

void DrawCrosshair (void)
{
	// PANZER TODO - allow cursor customization.
	Draw_CharacterScaled(
		cg.refdef.x + cg.refdef.width  / 2 - 4,
		cg.refdef.y + cg.refdef.height / 2 - 4,
		1,
		'+');
}

void DrawCenterPrint (void)
{
	int			lines;
	const char*	str;
	int			l;
	int			j;
	int			x, y;
	int			remaining;
	qboolean	intermission;

	// PANZER TODO - allow centerprint time customization
	int printspeed = 8;

	intermission = cg.snap.ps.pm_type == PM_INTERMISSION || cg.snap.ps.pm_type == PM_INTERMISSION_FINALE || cg.snap.ps.pm_type == PM_INTERMISSION_CUTSCENE;

	if(cg.centerPrintStartTime + 2000 < cg.time &&!intermission )
		return;

	lines = 1;
	str = cg.centerPrintString;
	while (*str)
	{
		if (*str == '\n')
			lines++;
		str++;
	}

	if (intermission)
		remaining = printspeed * (cg.time - cg.centerPrintStartTime) / 1000.0;
	else
		remaining = 9999;

	str = cg.centerPrintString;

	if (lines <= 4)
		y = cg.refdef.height*0.35;
	else
		y = 48 * sb_scale;

	do
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (str[l] == '\n' || !str[l])
				break;
		x = (cg.refdef.width - l*8)/2;
		for (j=0 ; j<l ; j++, x+=8)
		{
			Draw_CharacterScaled (x, y, 1, str[j]);
			if (!remaining--)
				return;
		}

		y += 8;

		while (*str && *str != '\n')
			str++;

		if (!*str)
			break;
		str++;		// skip the \n
	} while (1);
}

/*
==================
Sbar_FinaleOverlay

==================
*/
void Sbar_FinaleOverlay (void)
{
	const int width = 288;
	const int height = 24;
	trap_R_DrawStretchPic ( (cg.refdef.width - sb_scale * width)/2, sb_scale * 16, width * sb_scale, height * sb_scale, 0.0f, 0.0f, 1.0f, 1.0f, sbar.finale);
}

void Sbar_Pause (void)
{
	const int width = 128;
	const int height = 24;
	trap_R_DrawStretchPic (
		(cg.refdef.width - sb_scale * width)/2,
		(cg.refdef.height - sb_scale * height)/2,
		width * sb_scale,
		height * sb_scale,
		0.0f,
		0.0f,
		1.0f,
		1.0f, sbar.pause);
}
