#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <shlobj.h>
#include "max.h"
#include "impexp.h"		// SceneExport
#include "iparamb2.h"	// ClassDesc2
#include "plugapi.h"
#include "IGame.h"
#include "IGameExport.h"
#include "IConversionmanager.h"
#include "config.h"


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


#define VERSION(major, minor) \
	((major) * 100 + ((minor) < 10 ? (minor) * 10 : (minor)))

static FILE *logfile;
static HINSTANCE hinst;

class GoatExporter : public SceneExport {
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

	bool export_materials(FILE *fp);
};


int GoatExporter::ExtCount()
{
	return 1;
}

const TCHAR *GoatExporter::Ext(int n)
{
	return L"txt";
}

const TCHAR *GoatExporter::LongDesc()
{
	return L"Goat3D scene file";
}

const TCHAR *GoatExporter::ShortDesc()
{
	return L"Goat3D";
}

const TCHAR *GoatExporter::AuthorName()
{
	return L"John Tsiombikas";
}

const TCHAR *GoatExporter::CopyrightMessage()
{
	return L"Copyright 2013 (C) John Tsiombikas - GNU General Public License v3, see COPYING for details.";
}

const TCHAR *GoatExporter::OtherMessage1()
{
	return L"other1";
}

const TCHAR *GoatExporter::OtherMessage2()
{
	return L"other2";
}

unsigned int GoatExporter::Version()
{
	return VERSION(VER_MAJOR, VER_MINOR);
}

void GoatExporter::ShowAbout(HWND win)
{
	MessageBoxA(win, "Goat3D exporter plugin", "About this plugin", 0);
}

int GoatExporter::DoExport(const MCHAR *name, ExpInterface *eiface, Interface *iface,
		BOOL non_interactive, DWORD opt)
{
	char fname[512];
	wcstombs(fname, name, sizeof fname - 1);

	FILE *fp = fopen(fname, "wb");
	if(!fp) {
		fprintf(logfile, "failed to open %s for writting: %s", fname, strerror(errno));
		return IMPEXP_FAIL;
	}

	if(!(igame = GetIGameInterface())) {
		fprintf(logfile, "failed to get the igame interface\n");
		fclose(fp);
		return IMPEXP_FAIL;
	}
	IGameConversionManager *cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::IGAME_OGL);
	igame->InitialiseIGame();
	igame->SetStaticFrame(0);

	export_materials(fp);

	fclose(fp);

	return IMPEXP_SUCCESS;
}

bool GoatExporter::export_materials(FILE *fp)
{
	IGameProperty *prop;

	int num_mtl = igame->GetRootMaterialCount();
	fprintf(fp, "number of materials: %d\n", num_mtl);

	for(int i=0; i<num_mtl; i++) {
		IGameMaterial *mtl = igame->GetRootMaterial(i);
		if(mtl) {
			Point3 diffuse(1, 1, 1);
			Point3 specular(0, 0, 0);
			float shin = 1.0, sstr = 1.0;
			char name[512] = "unnamed";

			const MCHAR *wname = mtl->GetMaterialName();
			if(wname) {
				wcstombs(name, wname, sizeof name - 1);
			}

			if((prop = mtl->GetDiffuseData())) {
				prop->GetPropertyValue(diffuse);
			}
			if((prop = mtl->GetSpecularData())) {
				prop->GetPropertyValue(specular);
			}
			if((prop = mtl->GetSpecularLevelData())) {
				prop->GetPropertyValue(sstr);
			}
			if((prop = mtl->GetGlossinessData())) {
				prop->GetPropertyValue(shin);
			}

			fprintf(fp, "Material %d (%s):\n", i, name);
			fprintf(fp, "  diffuse: %f %f %f\n", diffuse[0], diffuse[1], diffuse[2]);
			fprintf(fp, "  specular: %f %f %f\n", specular[0] * sstr, specular[1] * sstr, specular[2] * sstr);
			fprintf(fp, "  shininess: %f\n", shin * 100.0);

			for(int j=0; j<mtl->GetNumberOfTextureMaps(); j++) {
				IGameTextureMap *tex = mtl->GetIGameTextureMap(j);
				const MCHAR *wfname = tex->GetBitmapFileName();
				if(wfname) {
					char fname[512];
					wcstombs(fname, wfname, sizeof fname - 1);
					fprintf(fp, "  texture%d: %s\n", j, fname);
				}
			}
		}
	}

	return true;
}

// ------------------------------------------

class GoatClassDesc : public ClassDesc2 {
public:
	int IsPublic() { return TRUE; }
	void *Create(BOOL loading = FALSE) { return new GoatExporter; }
	const TCHAR *ClassName() { return L"GoatExporter"; }
	SClass_ID SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID ClassID() { return Class_ID(0x77050f0d, 0x7d4c5ab5); }
	const TCHAR *Category() { return L"Mutant Stargoat"; }

	const TCHAR *InternalName() { return L"GoatExporter"; }
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
	return L"test exporter";
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
	strcat(path, "/testexp.log");

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