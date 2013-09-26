#ifndef LOG_H_
#define LOG_H_

enum { LOG_ERROR, LOG_INFO };

void logmsg(int prio, const char *fmt, ...);

#endif	// LOG_H_
