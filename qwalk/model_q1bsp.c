#include "global.h"
#include "model.h"

#undef LittleShort
#undef LittleLong
#undef LittleFloat
#define copystring Q_copystring

#define false Q_false
#define true Q_true
#include "l_cmd.h"
#include "l_bsp_q1.h"


bool_t model_q1bsp_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	// TODO
}
