#ifndef LOG_H_
#define LOG_H_

namespace g3dimpl {

enum { LOG_ERROR, LOG_INFO };

void logmsg(int prio, const char *fmt, ...);

}	// namespace g3dimpl

#endif	// LOG_H_
