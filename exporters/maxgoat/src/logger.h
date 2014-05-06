#ifndef MAXLOGGER_H_
#define MAXLOGGER_H_

bool maxlog_open(const char *fname);
void maxlog_close();
void maxlog(const char *fmt, ...);

#endif	/* MAXLOGGER_H_ */
