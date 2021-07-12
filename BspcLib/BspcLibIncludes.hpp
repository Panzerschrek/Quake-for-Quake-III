#pragma once

#ifdef __GNUC__
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wcomment"
#endif // __GNUC__

extern "C"
{

#define false Q_false
#define true Q_true

#include "l_cmd.h"
#include "l_math.h"
#include "l_mem.h"
#include "l_log.h"
#include "l_bsp_q1.h"
#include "l_bsp_q3.h"

void	Q3_WriteBSPFile( char *filename );

#undef true
#undef false

} // extern "C"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

