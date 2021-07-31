#include "g_local.h"

/*
===============
V_CalcRoll

Used by view and sv_user
===============
*/

float V_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;
	vec3_t	forward, right, up;

	AngleVectors (angles, forward, right, up);
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	value = cl_rollangle.value;
//	if (cl.inwater)
//		value *= 6;

	if (side < cl_rollspeed.value)
		side = side * value / cl_rollspeed.value;
	else
		side = value;

	return side*sign;

}
