#include "cg_local.h"

#define Q1_MAX_WEAPONS 8

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	// TODO - check for available weapons?
	cg.weaponSelect = ( cg.weaponSelect + Q1_MAX_WEAPONS + 1 ) % Q1_MAX_WEAPONS;
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	// TODO - check for available weapons?
	cg.weaponSelect = ( cg.weaponSelect + Q1_MAX_WEAPONS - 1 ) % Q1_MAX_WEAPONS;
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
