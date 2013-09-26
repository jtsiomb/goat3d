#include <stdio.h>
#include <stdarg.h>
#include "log.h"

int goat_log_level = 256;

void logmsg(int prio, const char *fmt, ...)
{
	va_list ap;

	if(goat_log_level < prio) {
		return;
	}

	fprintf(stderr, "goat3d: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
