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
#include "goat3d.h"
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

	void export_materials(goat3d *goat);
	void export_meshes(goat3d *goat);
	void process_node(goat3d *goat, IGameNode *maxnode);
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

	if(!(igame = GetIGameInterface())) {
		fprintf(logfile, "failed to get the igame interface\n");
		return IMPEXP_FAIL;
	}
	IGameConversionManager *cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::IGAME_OGL);
	igame->InitialiseIGame();
	igame->SetStaticFrame(0);

	goat3d *goat = goat3d_create();

	export_materials(goat);
	export_meshes(goat);

	if(goat3d_save(goat, fname) == -1) {
		goat3d_free(goat);
		return IMPEXP_FAIL;
	}

	// process all nodes
	for(int i=0; i<igame->GetTopLevelNodeCount(); i++) {
		IGameNode *node = igame->GetTopLevelNode(i);
		process_node(goat, node);
	}

	goat3d_free(goat);
	return IMPEXP_SUCCESS;
}

static const char *max_string(const MCHAR *wstr)
{
	if(!wstr) return 0;
	static char str[512];
	wcstombs(str, wstr, sizeof str - 1);
	return str;
}

void GoatExporter::export_materials(goat3d *goat)
{
	IGameProperty *prop;

	int num_mtl = igame->GetRootMaterialCount();
	for(int i=0; i<num_mtl; i++) {
		IGameMaterial *maxmtl = igame->GetRootMaterial(i);
		if(maxmtl) {
			goat3d_material *mtl = goat3d_create_mtl();

			const char *name = max_string(maxmtl->GetMaterialName());
			if(name) {
				goat3d_set_mtl_name(mtl, name);
			}

			// diffuse
			if((prop = maxmtl->GetDiffuseData())) {
				Point3 diffuse(1, 1, 1);
				prop->GetPropertyValue(diffuse);
				goat3d_set_mtl_attrib3f(mtl, GOAT3D_MAT_ATTR_DIFFUSE, diffuse[0],
					diffuse[1], diffuse[2]);
			}
			// specular
			if((prop = maxmtl->GetSpecularData())) {
				Point3 specular(0, 0, 0);
				prop->GetPropertyValue(specular);

				float sstr = 1.0;
				if((prop = maxmtl->GetSpecularLevelData())) {
					prop->GetPropertyValue(sstr);
				}
				goat3d_set_mtl_attrib3f(mtl, GOAT3D_MAT_ATTR_SPECULAR, specular[0] * sstr,
					specular[1] * sstr, specular[2] * sstr);
			}
			// shininess
			if((prop = maxmtl->GetGlossinessData())) {
				float shin;
				prop->GetPropertyValue(shin);
				goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_SHININESS, shin * 100.0);
			}

			// textures
			for(int j=0; j<maxmtl->GetNumberOfTextureMaps(); j++) {
				IGameTextureMap *tex = maxmtl->GetIGameTextureMap(j);

				const char *fname = max_string(tex->GetBitmapFileName());
				if(!fname) {
					continue;
				}

				int slot = tex->GetStdMapSlot();
				switch(slot) {
				case ID_DI:	// diffuse
					goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_DIFFUSE, fname);
					break;

				case ID_SP:
				case ID_SS:
					goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_SPECULAR, fname);
					break;

				case ID_SH:
					goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_SHININESS, fname);
					break;

				case ID_BU:
					goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_NORMAL, fname);
					break;

				case ID_RL:
					goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_REFLECTION, fname);
					break;

				case ID_RR:
					goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_TRANSMISSION, fname);
					break;

				default:
					break;
				}
			}

			goat3d_add_mtl(goat, mtl);
		}
	}
}

void GoatExporter::export_meshes(goat3d *goat)
{
	Tab<IGameNode*> meshes = igame->GetIGameNodeByType(IGameObject::IGAME_MESH);

	for(int i=0; i<meshes.Count(); i++) {
		const char *name = max_string(meshes[i]->GetName());
	}
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