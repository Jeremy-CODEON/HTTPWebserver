#include "core.h"

void core::unix_error(char* msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}
