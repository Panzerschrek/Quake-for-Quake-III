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

void G_FilebufLoadFile (const char* file_path)
{
	fileHandle_t	f;

	f = 0;
	trap_FS_FOpenFile(file_path, &f, FS_READ);
	if( f == 0 )
		G_Error("Failed to open \"%s\"\n", file_path);

	// The engine does not provide a way to get file size. So, zero buffer to let reader detect end of file.
	memset(filebuf, 0, sizeof(filebuf));
	trap_FS_Read(filebuf, sizeof(filebuf), f);
	filebuf_pos = 0;

	trap_FS_FCloseFile(f);
}

void G_FilebufSaveToFile (const char* file_path)
{
	fileHandle_t	f;

	f = 0;
	trap_FS_FOpenFile(file_path, &f, FS_WRITE);
	if( f == 0 )
		G_Error("Failed to open \"%s\"\n", file_path);

	trap_FS_Write(filebuf, filebuf_pos, f);
	trap_FS_FCloseFile(f);
}

const char* G_FilebufReadLine (void)
{
	char* res;
	res = ((char*)filebuf) + filebuf_pos;
	while(filebuf_pos < sizeof(filebuf) && filebuf[filebuf_pos] != '\n')
		++filebuf_pos;

	if(filebuf_pos < sizeof(filebuf))
	{
		filebuf[filebuf_pos] = 0;
		++filebuf_pos;
		return res;
	}
	return "";
}

char G_FilebufGetChar (void)
{
	char res;
	if (filebuf_pos >= sizeof(filebuf))
		return 0;
	res = filebuf[filebuf_pos];
	++filebuf_pos;
	return res;
}
