#ifndef MINWIN_H_
#define MINWIN_H_

struct MinWidget;
typedef void (*MWCallback)(MinWidget*, void*);

void mw_set_callback(MinWidget *w, MWCallback func, void *cls);

MinWidget *mw_create_window(MinWidget *parent, const char *name, int x, int y, int xsz, int ysz);
MinWidget *mw_create_button(MinWidget *parent, const char *text, int x, int y, int xsz, int ysz);
MinWidget *mw_create_checkbox(MinWidget *parent, const char *text, int x, int y, int xsz, int ysz, bool checked);

void mw_test();

#endif	// MINWIN_H_