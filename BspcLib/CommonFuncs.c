#include <string.h>

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
