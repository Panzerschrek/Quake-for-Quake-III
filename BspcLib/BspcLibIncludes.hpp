#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef __GNUC__
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wcomment"
#endif // __GNUC__

extern "C"
{

#define false Q_false
#define true Q_true
#define strupr Q_strupr

#include "l_cmd.h"
#include "l_math.h"
#include "l_mem.h"
#include "l_log.h"
#include "l_bsp_q1.h"
#include "l_bsp_q3.h"
#include "q3files.h"
#include "qfiles.h"

void	Q3_WriteBSPFile( char *filename );

#undef true
#undef false
#undef strupr

} // extern "C"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

