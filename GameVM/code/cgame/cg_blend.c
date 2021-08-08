#include "cg_local.h"

/*
==============================================================================

						PALETTE FLASHES

==============================================================================
*/


cshift_t	cshift_empty = { {130,80,50}, 0 };
cshift_t	cshift_water = { {130,80,50}, 128 };
cshift_t	cshift_slime = { {0,25,5}, 150 };
cshift_t	cshift_lava = { {255,80,0}, 150 };

static float v_blend[4];


/*
==================
V_cshift_f
==================
*/
void V_cshift_f (int r, int g, int b, int a)
{
	cshift_empty.destcolor[0] = r;
	cshift_empty.destcolor[1] = g;
	cshift_empty.destcolor[2] = g;
	cshift_empty.percent = a;
}


/*
==================
V_BonusFlash_f

When you run over an item, the server sends this command
==================
*/
void V_BonusFlash_f (void)
{
	cg.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	cg.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	cg.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	cg.cshifts[CSHIFT_BONUS].percent = 50;
}

void V_ParseDamage(entityState_t* event)
{
	int		armor, blood;
	float	count;

	armor = event->constantLight & 255;
	blood = (event->constantLight >> 8) & 255;

	count = blood*0.5 + armor*0.5;
	if (count < 10)
		count = 10;

	cg.faceanimtime = cg.time + 200;		// but sbar face into pain frame

	cg.cshifts[CSHIFT_DAMAGE].percent += 3*count;
	if (cg.cshifts[CSHIFT_DAMAGE].percent < 0)
		cg.cshifts[CSHIFT_DAMAGE].percent = 0;
	if (cg.cshifts[CSHIFT_DAMAGE].percent > 150)
		cg.cshifts[CSHIFT_DAMAGE].percent = 150;

	if (armor > blood)
	{
		cg.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		cg.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		cg.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		cg.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		cg.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		cg.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		cg.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		cg.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		cg.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}
}

void V_CalcBlend (void)
{
	float	r, g, b, a, a2;
	int		j;

	r = 0;
	g = 0;
	b = 0;
	a = 0;

	for (j=0 ; j<NUM_CSHIFTS ; j++)
	{
		a2 = cg.cshifts[j].percent / 255.0;

		if (!a2)
			continue;

		a = a + a2*(1-a);
		a2 = a2/a;
		r = r*(1-a2) + cg.cshifts[j].destcolor[0]*a2;
		g = g*(1-a2) + cg.cshifts[j].destcolor[1]*a2;
		b = b*(1-a2) + cg.cshifts[j].destcolor[2]*a2;
	}

	v_blend[0] = r/255.0;
	v_blend[1] = g/255.0;
	v_blend[2] = b/255.0;
	v_blend[3] = a;
	if (v_blend[3] > 1)
		v_blend[3] = 1;
	if (v_blend[3] < 0)
		v_blend[3] = 0;
}


/*
=============
V_SetContentsColor

Underwater, lava, etc each has a color shift
=============
*/
void V_SetContentsColor (int contents)
{
	switch (contents)
	{
	case 0:
	case CONTENTS_SOLID:
		cg.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		break;
	case CONTENTS_LAVA:
		cg.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case CONTENTS_SLIME:
		cg.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	default:
		cg.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}
}

/*
=============
V_CalcPowerupCshift
=============
*/
void V_CalcPowerupCshift (void)
{
	if (GetItems() & IT_QUAD)
	{
		cg.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cg.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		cg.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		cg.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else if (GetItems() & IT_SUIT)
	{
		cg.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cg.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cg.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cg.cshifts[CSHIFT_POWERUP].percent = 20;
	}
	else if (GetItems() & IT_INVISIBILITY)
	{
		cg.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		cg.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		cg.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		cg.cshifts[CSHIFT_POWERUP].percent = 100;
	}
	else if (GetItems() & IT_INVULNERABILITY)
	{
		cg.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		cg.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cg.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cg.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else
		cg.cshifts[CSHIFT_POWERUP].percent = 0;
}

void V_UpdatePalette (void)
{
	int		i, j;
	qboolean	new;
	byte	*basepal, *newpal;
	byte	pal[768];
	int		r,g,b,a;
	qboolean force;

	V_CalcPowerupCshift ();

	new = qfalse;

	for (i=0 ; i<NUM_CSHIFTS ; i++)
	{
		if (cg.cshifts[i].percent != cg.prev_cshifts[i].percent)
		{
			new = qtrue;
			cg.prev_cshifts[i].percent = cg.cshifts[i].percent;
		}
		for (j=0 ; j<3 ; j++)
			if (cg.cshifts[i].destcolor[j] != cg.prev_cshifts[i].destcolor[j])
			{
				new = qtrue;
				cg.prev_cshifts[i].destcolor[j] = cg.cshifts[i].destcolor[j];
			}
	}

	// drop the damage value
	cg.cshifts[CSHIFT_DAMAGE].percent -= cg.frametime/1000.0f*150;
	if (cg.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cg.cshifts[CSHIFT_DAMAGE].percent = 0;

	// drop the bonus value
	cg.cshifts[CSHIFT_BONUS].percent -= cg.frametime/1000.0f*100;
	if (cg.cshifts[CSHIFT_BONUS].percent <= 0)
		cg.cshifts[CSHIFT_BONUS].percent = 0;

	V_CalcBlend ();
}


void CG_DrawPolyBlend(void) {
	V_SetContentsColor(cg.viewContents);
	V_UpdatePalette();
	if(v_blend[3] == 0.0f)
		return;

	trap_R_SetColor(v_blend);
	trap_R_DrawStretchPic(0.0f, 0.0f, cg.refdef.width, cg.refdef.height, 0.0f, 0.0f, 1.0f, 1.0f, cgs.fullscreen_blend);
	trap_R_SetColor(NULL);
}
