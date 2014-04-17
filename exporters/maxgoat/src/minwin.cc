#include <string.h>
#include <windows.h>
#include "minwin.h"
#include "logger.h"

#define GOATSCE_WCLASS	"goatsce-window"

struct MinWidget {
	HWND win;

	MWCallback cbfunc;
	void *cbcls;

	MinWidget() { win = 0; cbfunc = 0; cbcls = 0; }
};

static void init();
static MinWidget *createwin(MinWidget *parent, const char *cls, const char *name,
		unsigned int style, int x, int y, int xsz, int ysz);
static LRESULT CALLBACK handle_msg(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam);

extern HINSTANCE hinst;	// defined in maxgoat.cc

void mw_set_callback(MinWidget *w, MWCallback func, void *cls)
{
	w->cbfunc = func;
	w->cbcls = cls;
}

MinWidget *mw_create_window(MinWidget *parent, const char *name, int x, int y, int xsz, int ysz)
{
	unsigned int style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	if(parent) {
		style |= WS_CHILD;
	}

	maxlog("creating window: %s\n", name);

	MinWidget *w = createwin(parent, GOATSCE_WCLASS, name, style, x, y, xsz, ysz);
	return w;
}

MinWidget *mw_create_button(MinWidget *parent, const char *text, int x, int y, int xsz, int ysz)
{
	unsigned int style = BS_PUSHBUTTON | WS_VISIBLE;
	if(parent) {
		style |= WS_CHILD;
	}

	maxlog("creating button: %s\n", text);

	MinWidget *w = createwin(parent, "BUTTON", text, style, x, y, xsz, ysz);
	return w;
}

MinWidget *mw_create_checkbox(MinWidget *parent, const char *text, int x, int y, int xsz, int ysz, bool checked)
{
	unsigned int style = BS_CHECKBOX | WS_VISIBLE;
	if(parent) {
		style |= WS_CHILD;
	}

	maxlog("creating checkbox: %s\n", text);

	MinWidget *w = createwin(parent, "CHECKBOX", text, style, x, y, xsz, ysz);
	return w;
}

static DWORD WINAPI gui_thread_func(void *cls);

void mw_test()
{
	init();

	HANDLE thread = CreateThread(0, 0, gui_thread_func, 0, 0, 0);
	//WaitForSingleObject(thread, 5000);
}

static DWORD WINAPI gui_thread_func(void *cls)
{
	MinWidget *win = mw_create_window(0, "test window!", -1, -1, 400, 400);
	MinWidget *bn_ok = mw_create_button(win, "Ok", 50, 100, 150, 40);
	MinWidget *bn_cancel = mw_create_button(win, "Cancel", 250, 100, 150, 40);
	MinWidget *ck_lights = mw_create_checkbox(win, "Export lights", 20, 20, 250, 40, true);
	MinWidget *ck_cameras = mw_create_checkbox(win, "Export cameras", 20, 60, 250, 40, true);

	MSG msg;
	while(GetMessage(&msg, win->win, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(win->win);
	delete bn_ok;
	delete bn_cancel;
	delete ck_lights;
	delete ck_cameras;
	delete win;

	return 0;
}

static void init()
{
	static bool done_init;
	if(done_init) {
		return;
	}
	done_init = true;

	size_t sz = mbstowcs(0, GOATSCE_WCLASS, 0);
	wchar_t *cname = new wchar_t[sz + 1];
	mbstowcs(cname, GOATSCE_WCLASS, sz + 1);

	WNDCLASS wc;
	memset(&wc, 0, sizeof wc);
	wc.lpszClassName = cname;
	wc.hInstance = hinst;
	wc.lpfnWndProc = handle_msg;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(0, IDC_ARROW);

	RegisterClass(&wc);
}

static MinWidget *createwin(MinWidget *parent, const char *cls, const char *name,
		unsigned int style, int x, int y, int xsz, int ysz)
{
	init();

	MinWidget *w = new MinWidget;

	size_t sz = mbstowcs(0, cls, 0);
	wchar_t *wcls = new wchar_t[sz + 1];
	mbstowcs(wcls, cls, sz + 1);

	sz = mbstowcs(0, name, 0);
	wchar_t *wname = new wchar_t[sz + 1];
	mbstowcs(wname, name, sz + 1);

	if(x <= 0) x = CW_USEDEFAULT;
	if(y <= 0) y = CW_USEDEFAULT;

	w->win = CreateWindow(wcls, wname, style, x, y, xsz, ysz, parent ? parent->win : 0, 0, hinst, 0);

	delete [] wcls;
	delete [] wname;

	if(!w->win) {
		delete w;
		return 0;
	}
	return w;
}


static LRESULT CALLBACK handle_msg(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg) {
	case WM_CLOSE:
		DestroyWindow(win);
		break;

	default:
		return DefWindowProc(win, msg, wparam, lparam);
	}

	return 0;
}