#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <map>
#include <windows.h>
#include <shlobj.h>
#include "max.h"
#include "impexp.h"		// SceneExport
#include "iparamb2.h"	// ClassDesc2
#include "plugapi.h"
#include "IGame.h"
#include "IGameExport.h"
#include "IConversionmanager.h"


#pragma comment (lib, "core.lib")
#pragma comment (lib, "geom.lib")
#pragma comment (lib, "gfx.lib")
#pragma comment (lib, "mesh.lib")
#pragma comment (lib, "maxutil.lib")
#pragma comment (lib, "maxscrpt.lib")
#pragma comment (lib, "paramblk2.lib")
#pragma comment (lib, "msxml2.lib")
#pragma comment (lib, "igame.lib")
#pragma comment (lib, "comctl32.lib")


#define VER_MAJOR	1
#define VER_MINOR	0
#define VERSION(major, minor) \
	((major) * 100 + ((minor) < 10 ? (minor) * 10 : (minor)))

typedef int (*PluginInitFunc)();
typedef int (*PluginShutdownFunc)();
typedef ClassDesc *(*PluginClassDescFunc)(int);

static FILE *logfile;
static HINSTANCE hinst;

class GoatExporterStub : public SceneExport {
private:

public:
	IGameScene *igame;

	int ExtCount();
	const TCHAR *Ext(int n);
	const TCHAR *LongDesc();
	const TCHAR *ShortDesc();
	const TCHAR *AuthorName();
	const TCHAR *CopyrightMessage();
	const TCHAR *OtherMessage1();
	const TCHAR *OtherMessage2();
	unsigned int Version();
	void ShowAbout(HWND win);

	int DoExport(const MCHAR *name, ExpInterface *eiface, Interface *iface, BOOL silent = FALSE, DWORD opt = 0);
};


int GoatExporterStub::ExtCount()
{
	return 1;
}

const TCHAR *GoatExporterStub::Ext(int n)
{
	return L"xml";
}

const TCHAR *GoatExporterStub::LongDesc()
{
	return L"Goat3D scene file";
}

const TCHAR *GoatExporterStub::ShortDesc()
{
	return L"Goat3D";
}

const TCHAR *GoatExporterStub::AuthorName()
{
	return L"John Tsiombikas";
}

const TCHAR *GoatExporterStub::CopyrightMessage()
{
	return L"Copyright 2013 (C) John Tsiombikas - GNU General Public License v3, see COPYING for details.";
}

const TCHAR *GoatExporterStub::OtherMessage1()
{
	return L"other1";
}

const TCHAR *GoatExporterStub::OtherMessage2()
{
	return L"other2";
}

unsigned int GoatExporterStub::Version()
{
	return VERSION(VER_MAJOR, VER_MINOR);
}

void GoatExporterStub::ShowAbout(HWND win)
{
	MessageBoxA(win, "Goat3D exporter stub", "About this plugin", 0);
}

static const char *find_dll_dir()
{
	static char path[MAX_PATH];

	HMODULE dll;
	if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCSTR)find_dll_dir, &dll)) {
		return 0;
	}
	GetModuleFileNameA(dll, path, sizeof path);

	char *last_slash = strrchr(path, '\\');
	if(last_slash && last_slash[1]) {
		*last_slash = 0;
	}

	return path;
}

/* TODO: open a dialog, let the user select goat3d or goat3danim, then load the correct dll
 */
int GoatExporterStub::DoExport(const MCHAR *name, ExpInterface *eiface, Interface *iface,
		BOOL non_interactive, DWORD opt)
{
	const char *dll_fname = "maxgoat.dll";
	char *dll_path;
	HMODULE dll = 0;
	PluginInitFunc init;
	PluginShutdownFunc shutdown;
	PluginClassDescFunc get_class_desc;
	ClassDesc *desc;
	SceneExport *ex;
	int result = IMPEXP_FAIL;

	const char *plugdir = find_dll_dir();
	if(plugdir) {
		dll_path = new char[strlen(dll_fname) + strlen(plugdir) + 2];
		sprintf(dll_path, "%s\\%s", plugdir, dll_fname);
	} else {
		dll_path = new char[strlen(dll_fname) + 1];
		strcpy(dll_path, dll_fname);
	}

	if(!(dll = LoadLibraryA(dll_path))) {
		fprintf(logfile, "failed to load exporter: %s\n", dll_path);
		goto done;
	}

	if(!(get_class_desc = (PluginClassDescFunc)GetProcAddress(dll, "LibClassDesc"))) {
		fprintf(logfile, "maxgoat.dll is invalid (no LibClassDesc function)\n");
		goto done;
	}

	// first initialize the library
	if((init = (PluginInitFunc)GetProcAddress(dll, "LibInitialize"))) {
		if(!init()) {
			fprintf(logfile, "exporter initialization failed!\n");
			goto done;
		}
	}

	// TODO: pass 1 for anim
	if(!(desc = get_class_desc(0))) {
		fprintf(logfile, "failed to get the class descriptor\n");
		goto done;
	}

	if(!(ex = (SceneExport*)desc->Create())) {
		fprintf(logfile, "failed to create exporter class instance\n");
		goto done;
	}

	result = ex->DoExport(name, eiface, iface);
	delete ex;

	if((shutdown = (PluginShutdownFunc)GetProcAddress(dll, "LibShutdown"))) {
		shutdown();
	}

done:
	delete [] dll_path;
	if(dll) {
		FreeLibrary(dll);
	}
	return result;
}


// ------------------------------------------

class GoatClassDesc : public ClassDesc2 {
public:
	int IsPublic() { return TRUE; }
	void *Create(BOOL loading = FALSE) { return new GoatExporterStub; }
	const TCHAR *ClassName() { return L"GoatExporterStub"; }
	SClass_ID SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID ClassID() { return Class_ID(0x2e4e6311, 0x2b154d91); }
	const TCHAR *Category() { return L"Mutant Stargoat"; }

	const TCHAR *InternalName() { return L"GoatExporterStub"; }
	HINSTANCE HInstance() { return hinst; }
};

static GoatClassDesc class_desc;

BOOL WINAPI DllMain(HINSTANCE inst_handle, ULONG reason, void *reserved)
{
	if(reason == DLL_PROCESS_ATTACH) {
		hinst = inst_handle;
		DisableThreadLibraryCalls(hinst);
	}
	return TRUE;
}

extern "C" {

__declspec(dllexport) const TCHAR *LibDescription()
{
	return L"Goat3D exporter stub";
}

__declspec(dllexport) int LibNumberClasses()
{
	return 1;
}

__declspec(dllexport) ClassDesc *LibClassDesc(int i)
{
	return i == 0 ? &class_desc : 0;
}

__declspec(dllexport) ULONG LibVersion()
{
	return Get3DSMAXVersion();
}

__declspec(dllexport) int LibInitialize()
{
	static char path[1024];

	SHGetFolderPathA(0, CSIDL_PERSONAL, 0, 0, path);
	strcat(path, "/testexpstub.log");

	if((logfile = fopen(path, "w"))) {
		setvbuf(logfile, 0, _IONBF, 0);
	}
	return TRUE;
}

__declspec(dllexport) int LibShutdown()
{
	if(logfile) {
		fclose(logfile);
		logfile = 0;
	}
	return TRUE;
}

}	// extern "C"