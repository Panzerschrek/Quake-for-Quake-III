#include "quakedef.h"

server_t		sv;
server_static_t	svs;

// Other gobal stuff.

int	current_skill;

client_t	*host_client;			// current client

cvar_t	skill = {"skill","1"};						// 0 - 3
cvar_t	deathmatch = {"deathmatch","0"};			// 0, 1, or 2
cvar_t	coop = {"coop","0"};			// 0 or 1
