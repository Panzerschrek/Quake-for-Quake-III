#include "g_local.h"

// Replacement for "fprintf" functions.

static int filebuf_pos = 0;
#define FILEBUF_SIZE 1024 * 1024
static byte filebuf[FILEBUF_SIZE];

void G_FilebufReset (void)
{
	filebuf_pos = 0;
}

void G_FilebufWrite(const char *fmt, ... )
{
	va_list		argptr;
	char		text[1024];
	int			len;

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	len = strlen(text);
	if (filebuf_pos + len > FILEBUF_SIZE)
	{
		G_Error("Filebuf overflow");
		return;
	}
	memcpy(filebuf + filebuf_pos, text, len);
	filebuf_pos+= len;
}

byte* G_FilebufGetData (void)
{
	return filebuf;
}

int G_FilebufGetSize (void)
{
	return filebuf_pos;
}
