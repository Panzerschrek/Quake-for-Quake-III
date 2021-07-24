#include "quakedef.h"

// ClearLink is used for new headnodes
void ClearLink (link_t *l)
{
	l->prev = l->next = l;
}

void RemoveLink (link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

void InsertLinkBefore (link_t *l, link_t *before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
}
void InsertLinkAfter (link_t *l, link_t *after)
{
	l->next = after->next;
	l->prev = after;
	l->prev->next = l;
	l->next->prev = l;
}


void MSG_WriteChar (sizebuf_t *sb, int c){}
void MSG_WriteByte (sizebuf_t *sb, int c){}
void MSG_WriteShort (sizebuf_t *sb, int c){}
void MSG_WriteLong (sizebuf_t *sb, int c){}
void MSG_WriteFloat (sizebuf_t *sb, float f){}
void MSG_WriteString (sizebuf_t *sb, char *s){}
void MSG_WriteCoord (sizebuf_t *sb, float f){}
void MSG_WriteAngle (sizebuf_t *sb, float f){}
