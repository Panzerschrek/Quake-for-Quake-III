#include "cg_local.h"

#define Q1_MAX_WEAPONS 9

// Do not support next/prev weapon as original Quake does, because prev/next weapon logic is too complicated.

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	num = atoi( CG_Argv( 1 ) );

	if( num >= 0 && num < Q1_MAX_WEAPONS )
		cg.weaponSelect = num;
}
