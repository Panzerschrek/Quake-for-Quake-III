#include <string.h>

int forcesidesvisible = 0;

void Com_Memset (void* dest, const int val, const size_t count)
{
	memset(dest, val, count);
}

void Com_Memcpy (void* dest, const void* src, const size_t count)
{
	memcpy(dest, src, count);
}

int	COM_Compress( char *data_p )
{
	return strlen(data_p);
}

// This file contains stubs to force building of "q3_bsp.c".
// Actually, we need only "Q3_WriteBSPFile" function, other functions are not needed and we can freely break them.

void WindingArea(){}
void CopyWinding(){}
void BaseWindingForPlane(){}
void FreeWinding(){}
void ChopWindingInPlace(){}
void WindingIsTiny (){}
void WindingError(){}
void LoadQuakeFile(){}
