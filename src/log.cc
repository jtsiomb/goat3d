#include <stdio.h>
#include <stdarg.h>
#include "log.h"

int goat_log_level = 256;

void logmsg(int prio, const char *fmt, ...)
{
	fprintf(stderr, "goat3d error: ");

	va_list ap;

	if(prio < goat_log_level) {
		return;
	}

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
