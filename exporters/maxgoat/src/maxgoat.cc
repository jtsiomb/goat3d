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
private:
	std::map<IGameMaterial*, goat3d_material*> mtlmap;
	std::map<IGameNode*, goat3d_node*> nodemap;

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

	void process_materials(goat3d *goat);

	void process_node(goat3d *goat, goat3d_node *parent, IGameNode *maxnode);

	void process_mesh(goat3d *goat, goat3d_mesh *mesh, IGameObject *maxobj);
	void process_light(goat3d *goat, goat3d_light *light, IGameObject *maxobj);
	void process_camera(goat3d *goat, goat3d_camera *cam, IGameObject *maxobj);
};


int GoatExporter::ExtCount()
{
	return 1;
}

const TCHAR *GoatExporter::Ext(int n)
{
	return L"xml";
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
	mtlmap.clear();
	nodemap.clear();

	char fname[512];
	wcstombs(fname, name, sizeof fname - 1);

	fprintf(logfile, "Exporting Goat3D Scene (text) file: %s\n", fname);
	if(!(igame = GetIGameInterface())) {
		fprintf(logfile, "failed to get the igame interface\n");
		return IMPEXP_FAIL;
	}
	IGameConversionManager *cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::IGAME_OGL);
	igame->InitialiseIGame();
	igame->SetStaticFrame(0);

	goat3d *goat = goat3d_create();

	process_materials(goat);

	// process all nodes
	for(int i=0; i<igame->GetTopLevelNodeCount(); i++) {
		IGameNode *node = igame->GetTopLevelNode(i);
		process_node(goat, 0, node);
	}

	if(goat3d_save(goat, fname) == -1) {
		goat3d_free(goat);
		return IMPEXP_FAIL;
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

void GoatExporter::process_materials(goat3d *goat)
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
			mtlmap[maxmtl] = mtl;
		}
	}
}

void GoatExporter::process_node(goat3d *goat, goat3d_node *parent, IGameNode *maxnode)
{
	goat3d_node *node = goat3d_create_node();
	goat3d_add_node(goat, node);

	const char *name = max_string(maxnode->GetName());
	if(name) {
		goat3d_set_node_name(node, name);
	}

	// no animation yet, just get the static PRS
	GMatrix maxmatrix = maxnode->GetObjectTM();
	Point3 trans = maxmatrix.Translation();
	Quat rot = maxmatrix.Rotation();
	Point3 scale = maxmatrix.Scaling();

	goat3d_set_node_position(node, trans.x, trans.y, trans.z, 0);
	goat3d_set_node_rotation(node, rot.x, rot.y, rot.z, rot.w, 0);
	goat3d_set_node_scaling(node, scale.x, scale.y, scale.z, 0);

	IGameObject *maxobj = maxnode->GetIGameObject();
	IGameObject::ObjectTypes type = maxobj->GetIGameType();

	switch(type) {
	case IGameObject::IGAME_MESH:
		{
			goat3d_mesh *mesh = goat3d_create_mesh();
			if(name) goat3d_set_mesh_name(mesh, name);
			goat3d_set_node_object(node, GOAT3D_NODE_MESH, mesh);

			// get the node material and assign it to the mesh
			IGameMaterial *maxmtl = maxnode->GetNodeMaterial();
			goat3d_material *mtl = mtlmap[maxmtl];
			if(mtl) {
				goat3d_set_mesh_mtl(mesh, mtl);
			}

			process_mesh(goat, mesh, maxobj);
			goat3d_add_mesh(goat, mesh);
		}
		break;

	case IGameObject::IGAME_LIGHT:
		{
			goat3d_light *light = goat3d_create_light();
			//if(name) goat3d_set_light_name(light, name);
			goat3d_set_node_object(node, GOAT3D_NODE_LIGHT, light);

			process_light(goat, light, maxobj);
			goat3d_add_light(goat, light);
		}
		break;

	case IGameObject::IGAME_CAMERA:
		{
			goat3d_camera *cam = goat3d_create_camera();
			//if(name) goat3d_set_camera_name(camera, name);
			goat3d_set_node_object(node, GOAT3D_NODE_CAMERA, cam);

			process_camera(goat, cam, maxobj);
			goat3d_add_camera(goat, cam);
		}
		break;

	default:
		// otherwise don't assign an object, essentially treating it as a null node
		break;
	}
		

	for(int i=0; i<maxnode->GetChildCount(); i++) {
		process_node(goat, node, maxnode->GetNodeChild(i));
	}
}

void GoatExporter::process_mesh(goat3d *goat, goat3d_mesh *mesh, IGameObject *maxobj)
{
	IGameMesh *maxmesh = (IGameMesh*)maxobj;

	maxmesh->SetCreateOptimizedNormalList();	// not needed any more according to docs
	maxobj->InitializeData();

	int num_verts = maxmesh->GetNumberOfVerts();
	int num_faces = maxmesh->GetNumberOfFaces();
	//assert(maxmesh->GetNumberOfTexVerts() == num_verts);

	float *vertices = new float[num_verts * 3];
	float *normals = new float[num_verts * 3];
	//float *texcoords = new float[num_verts * 2];
	int *indices = new int[num_faces * 3];

	for(int i=0; i<num_verts; i++) {
		Point3 v = maxmesh->GetVertex(i, true);
		vertices[i * 3] = v.x;
		vertices[i * 3 + 1] = v.y;
		vertices[i * 3 + 2] = v.z;
	}

	for(int i=0; i<maxmesh->GetNumberOfNormals(); i++) {
		Point3 norm = maxmesh->GetNormal(i);

		int vidx = maxmesh->GetNormalVertexIndex(i);
		normals[vidx * 3] = norm.x;
		normals[vidx * 3 + 1] = norm.y;
		normals[vidx * 3 + 2] = norm.z;
	}

	/*for(int i=0; i<maxmesh->GetNumberOfTexVerts(); i++) {
		Point3 tex = maxmesh->GetTexVertex(i);

		texcoords[i * 2] = tex.x;
		texcoords[i * 2 + 1] = tex.y;
	}*/

	// get the faces
	for(int i=0; i<num_faces; i++) {
		FaceEx *face = maxmesh->GetFace(i);
		indices[i * 3] = face->vert[0];
		indices[i * 3 + 1] = face->vert[1];
		indices[i * 3 + 2] = face->vert[2];
		// TODO at some point I'll have to split based on normal/texcoord indices
	}

	goat3d_set_mesh_attribs(mesh, GOAT3D_MESH_ATTR_VERTEX, vertices, num_verts);
	goat3d_set_mesh_attribs(mesh, GOAT3D_MESH_ATTR_NORMAL, normals, num_verts);
	//goat3d_set_mesh_attribs(mesh, GOAT3D_MESH_ATTR_TEXCOORD, texcoords, num_verts);
	goat3d_set_mesh_faces(mesh, indices, num_faces);

	delete [] vertices;
	delete [] normals;
	//delete [] texcoords;
	delete [] indices;
}

void GoatExporter::process_light(goat3d *goat, goat3d_light *light, IGameObject *maxobj)
{
}

void GoatExporter::process_camera(goat3d *goat, goat3d_camera *cam, IGameObject *maxobj)
{
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

// TODO: make 2 class descriptors, one for goat3d, one for goat3danim
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