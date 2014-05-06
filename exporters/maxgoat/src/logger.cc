#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

static FILE *logfile;

bool maxlog_open(const char *fname)
{
	if(!(logfile = fopen(fname, "wb"))) {
		return false;
	}
	setvbuf(logfile, 0, _IONBF, 0);
	return true;
}

void maxlog_close()
{
	if(logfile) {
		fclose(logfile);
	}
}

void maxlog(const char *fmt, ...)
{
	if(!logfile) return;

	va_list ap;
	va_start(ap, fmt);
	vfprintf(logfile, fmt, ap);
	va_end(ap);
}
