#include <string.h>
#include <windows.h>
#include "minwin.h"

#define GOATSCE_WCLASS	"goatsce-window"

struct MinWidget {
	HWND win;

	MWCallback cbfunc;
	void *cbcls;

	MinWidget() { win = 0; cbfunc = 0; cbcls = 0; }
};

static void init();
static LRESULT CALLBACK handle_msg(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam);

void mw_set_callback(MinWidget *w, MWCallback func, void *cls)
{
	w->cbfunc = func;
	w->cbcls = cls;
}

MinWidget *mw_create_window(MinWidget *parent, const char *name)
{
	MinWidget *w = new MinWidget;
	HINSTANCE inst = GetModuleHandle(0);

	w->win = CreateWindowA(GOATSCE_WCLASS, "Goat3D Scene export options ...", WS_OVERLAPPED,
		CW_USEDEFAULT, CW_USEDEFAULT, 512, 400,	parent ? parent->win : 0, 0, inst, 0);
	ShowWindow(w->win, 1);

	return w;
}

MinWidget *mw_create_button(MinWidget *parent, const char *text, int x, int y, int xsz, int ysz)
{
	MinWidget *bn = new MinWidget;
	HINSTANCE inst = GetModuleHandle(0);

	bn->win = CreateWindowA("BUTTON", text, BS_PUSHBUTTON | BS_TEXT,
		x, y, xsz, ysz,	parent ? parent->win : 0, 0, inst, 0);
	ShowWindow(bn->win, 1);

	return bn;
}

MinWidget *mw_create_checkbox(MinWidget *parent, const char *text, int x, int y, int w, int h, bool checked)
{
	return 0;
}


void mw_test()
{
	MinWidget *win = mw_create_window(0, "test window!");
	MinWidget *bn = mw_create_button(win, "button!", 100, 100, 300, 80);
}

static void init()
{
	static bool done_init;
	if(done_init) {
		return;
	}
	done_init = true;

	HINSTANCE hinst = GetModuleHandle(0);

	WNDCLASSA wc;
	memset(&wc, 0, sizeof wc);
	wc.lpszClassName = GOATSCE_WCLASS;
	wc.hInstance = hinst;
	wc.lpfnWndProc = handle_msg;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClassA(&wc);
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