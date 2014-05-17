#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <map>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include "max.h"
#include "impexp.h"		// SceneExport
#include "iparamb2.h"	// ClassDesc2
#include "plugapi.h"
#include "IGame.h"
#include "IGameExport.h"
#include "IGameControl.h"
#include "IConversionmanager.h"
#include "goat3d.h"
#include "config.h"
#include "logger.h"
#include "resource.h"


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


#define COPYRIGHT	\
	L"Copyright 2014 (C) John Tsiombikas - GNU General Public License v3, see COPYING for details."
#define VERSION(major, minor) \
	((major) * 100 + ((minor) < 10 ? (minor) * 10 : (minor)))

static INT_PTR CALLBACK scene_gui_handler(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam);
static INT_PTR CALLBACK anim_gui_handler(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam);
static void get_position_keys(IGameControl *ctrl, goat3d_node *node);
static void get_xyz_position_keys(IGameControl *ctrl, goat3d_node *node);
static void get_rotation_keys(IGameControl *ctrl, goat3d_node *node);
static void get_euler_keys(IGameControl *ctrl, goat3d_node *node);
static void get_scaling_keys(IGameControl *ctrl, goat3d_node *node);
static const char *max_string(const MCHAR *wstr);

HINSTANCE hinst;

class GoatExporter : public SceneExport {
private:
	std::map<IGameMaterial*, goat3d_material*> mtlmap;
	std::map<IGameNode*, goat3d_node*> nodemap;

public:
	IGameScene *igame;

	int DoExport(const MCHAR *name, ExpInterface *eiface, Interface *iface, BOOL silent = FALSE, DWORD opt = 0);

	void process_materials(goat3d *goat);

	void process_node(goat3d *goat, goat3d_node *parent, IGameNode *maxnode);

	void process_mesh(goat3d *goat, goat3d_mesh *mesh, IGameObject *maxobj);
	void process_light(goat3d *goat, goat3d_light *light, IGameObject *maxobj);
	void process_camera(goat3d *goat, goat3d_camera *cam, IGameObject *maxobj);


	int ExtCount() { return 1; }
	const TCHAR *Ext(int n) { return L"goatsce"; }
	const TCHAR *LongDesc() { return L"Goat3D scene file"; }
	const TCHAR *ShortDesc() { return L"Goat3D"; }
	const TCHAR *AuthorName() { return L"John Tsiombikas"; }
	const TCHAR *CopyrightMessage() { return COPYRIGHT; }
	const TCHAR *OtherMessage1() { return L"other1"; }
	const TCHAR *OtherMessage2() { return L"other2"; }
	unsigned int Version() { return VERSION(VER_MAJOR, VER_MINOR); }
	void ShowAbout(HWND win) { MessageBoxA(win, "Goat3D exporter plugin", "About this plugin", 0); }
};

class GoatAnimExporter : public GoatExporter {
private:
public:
	int DoExport(const MCHAR *name, ExpInterface *eiface, Interface *iface, BOOL silent = FALSE, DWORD opt = 0);

	const TCHAR *Ext(int n) { return L"goatanm"; }
	const TCHAR *LongDesc() { return L"Goat3D animation file"; }
};


// ---- GoatExporter implementation ----

int GoatExporter::DoExport(const MCHAR *name, ExpInterface *eiface, Interface *iface,
		BOOL non_interactive, DWORD opt)
{
	if(!DialogBox(hinst, MAKEINTRESOURCE(IDD_GOAT_SCE), 0, scene_gui_handler)) {
		return IMPEXP_CANCEL;
	}

	mtlmap.clear();
	nodemap.clear();

	char fname[512];
	wcstombs(fname, name, sizeof fname - 1);
	for(int i=0; fname[i]; i++) {
		fname[i] = tolower(fname[i]);
	}
	char *basename = (char*)alloca(strlen(fname) + 1);
	strcpy(basename, fname);
	char *suffix = strrchr(basename, '.');
	if(suffix) *suffix = 0;

	maxlog("Exporting Goat3D Scene (text) file: %s\n", fname);
	if(!(igame = GetIGameInterface())) {
		maxlog("failed to get the igame interface\n");
		return IMPEXP_FAIL;
	}
	IGameConversionManager *cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::IGAME_OGL);
	igame->InitialiseIGame();

	goat3d *goat = goat3d_create();
	goat3d_set_name(goat, basename);

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

	if(parent) {
		goat3d_add_node_child(parent, node);
	}

	const char *name = max_string(maxnode->GetName());
	if(name) {
		goat3d_set_node_name(node, name);
	}

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

	// grab the animation data
	if(!dynamic_cast<GoatAnimExporter*>(this)) {
		// no animation, just get the static PRS
		GMatrix maxmatrix = maxnode->GetLocalTM();
		Point3 trans = maxmatrix.Translation();
		Quat rot = maxmatrix.Rotation();
		Point3 scale = maxmatrix.Scaling();

		goat3d_set_node_position(node, trans.x, trans.y, trans.z, 0);
		goat3d_set_node_rotation(node, rot.x, rot.y, rot.z, -rot.w, 0);
		goat3d_set_node_scaling(node, scale.x, scale.y, scale.z, 0);

	} else {
		// exporting animations (if available)
		// TODO sample keys if requested
		IGameControl *ctrl = maxnode->GetIGameControl();
		if(ctrl) {
			if(ctrl->IsAnimated(IGAME_POS) || ctrl->IsAnimated(IGAME_POS_X) ||
					ctrl->IsAnimated(IGAME_POS_Y) || ctrl->IsAnimated(IGAME_POS_Z)) {
				get_position_keys(ctrl, node);
			}
			if(ctrl->IsAnimated(IGAME_ROT) || ctrl->IsAnimated(IGAME_EULER_X) ||
					ctrl->IsAnimated(IGAME_EULER_Y) || ctrl->IsAnimated(IGAME_EULER_Z)) {
				get_rotation_keys(ctrl, node);
			}
			if(ctrl->IsAnimated(IGAME_SCALE)) {
				get_scaling_keys(ctrl, node);
			}
		} else {
			maxlog("%s: failed to get IGameControl for node: %s\n", __FUNCTION__, name);
		}
	}

	for(int i=0; i<maxnode->GetChildCount(); i++) {
		process_node(goat, node, maxnode->GetNodeChild(i));
	}
}

#define KEY_TIME(key)	((long)(TicksToSec(key.t) * 1000.0))

static void get_position_keys(IGameControl *ctrl, goat3d_node *node)
{
	const char *nodename = goat3d_get_node_name(node);
	IGameKeyTab keys;

	if(ctrl->GetLinearKeys(keys, IGAME_POS)) {
		maxlog("node %s: getting %d linear position keys\n", nodename, keys.Count());
		for(int i=0; i<keys.Count(); i++) {
			Point3 p = keys[i].linearKey.pval;
			goat3d_set_node_position(node, p.x, p.y, p.z, KEY_TIME(keys[i]));
		}
	} else if(ctrl->GetBezierKeys(keys, IGAME_POS)) {
		maxlog("node %s: getting %d bezier position keys\n", nodename, keys.Count());
		for(int i=0; i<keys.Count(); i++) {
			Point3 p = keys[i].bezierKey.pval;
			goat3d_set_node_position(node, p.x, p.y, p.z, KEY_TIME(keys[i]));
		}
	} else if(ctrl->GetTCBKeys(keys, IGAME_POS)) {
		maxlog("node %s: getting %d tcb position keys\n", nodename, keys.Count());
		for(int i=0; i<keys.Count(); i++) {
			Point3 p = keys[i].tcbKey.pval;
			goat3d_set_node_position(node, p.x, p.y, p.z, KEY_TIME(keys[i]));
		}
	} else {
		get_xyz_position_keys(ctrl, node);
	}
}

static void get_xyz_position_keys(IGameControl *ctrl, goat3d_node *node)
{
	const char *nodename = goat3d_get_node_name(node);
	IGameKeyTab keys;
	IGameControlType postype[] = {IGAME_POS_X, IGAME_POS_Y, IGAME_POS_Z};
	std::map<long, Point3> pos;

	for(int i=0; i<3; i++) {
		if(ctrl->GetLinearKeys(keys, postype[i])) {
			maxlog("node %s: getting %d linear position %c keys\n", nodename, keys.Count(), "xyz"[i]);
			for(int j=0; j<keys.Count(); j++) {
				long tm = KEY_TIME(keys[j]);
				Point3 v = pos[tm];
				v[i] = keys[j].linearKey.fval;
				pos[tm] = v;
			}
		} else if(ctrl->GetBezierKeys(keys, postype[i])) {
			maxlog("node %s: getting %d bezier position %c keys\n", nodename, keys.Count(), "xyz"[i]);
			for(int j=0; j<keys.Count(); j++) {
				long tm = KEY_TIME(keys[j]);
				Point3 v = pos[tm];
				v[i] = keys[j].bezierKey.fval;
				pos[tm] = v;
			}
		} else if(ctrl->GetTCBKeys(keys, postype[i])) {
			maxlog("node %s: getting %d tcb position %c keys\n", nodename, keys.Count(), "xyz"[i]);
			for(int j=0; j<keys.Count(); j++) {
				long tm = KEY_TIME(keys[j]);
				Point3 v = pos[tm];
				v[i] = keys[j].tcbKey.fval;
				pos[tm] = v;
			}
		}
	}

	std::map<long, Point3>::iterator it = pos.begin();
	while(it != pos.end()) {
		Point3 p = it->second;
		goat3d_set_node_position(node, p.x, p.y, p.z, it->first);
		++it;
	}
}

static void get_rotation_keys(IGameControl *ctrl, goat3d_node *node)
{
	const char *nodename = goat3d_get_node_name(node);
	IGameKeyTab rkeys;

	if(ctrl->GetLinearKeys(rkeys, IGAME_ROT)) {
		maxlog("node %s: getting %d linear rotation keys\n", nodename, rkeys.Count());
		for(int i=0; i<rkeys.Count(); i++) {
			Quat q = rkeys[i].linearKey.qval;
			goat3d_set_node_rotation(node, q.x, q.y, q.z, -q.w, KEY_TIME(rkeys[i]));
		}
	} else if(ctrl->GetBezierKeys(rkeys, IGAME_ROT)) {
		maxlog("node %s: getting %d bezier rotation keys\n", nodename, rkeys.Count());
		for(int i=0; i<rkeys.Count(); i++) {
			Quat q = rkeys[i].bezierKey.qval;
			goat3d_set_node_rotation(node, q.x, q.y, q.z, -q.w, KEY_TIME(rkeys[i]));
		}
	} else if(ctrl->GetTCBKeys(rkeys, IGAME_ROT)) {
		maxlog("node %s: getting %d TCB rotation keys\n", nodename, rkeys.Count());
		for(int i=0; i<rkeys.Count(); i++) {
			Quat q(rkeys[i].tcbKey.aval);
			goat3d_set_node_rotation(node, q.x, q.y, q.z, -q.w, KEY_TIME(rkeys[i]));
		}
	} else {
		get_euler_keys(ctrl, node);
	}
}

static void get_euler_keys(IGameControl *ctrl, goat3d_node *node)
{
	const char *nodename = goat3d_get_node_name(node);
	IGameKeyTab keys;
	IGameControlType eulertype[] = {IGAME_EULER_X, IGAME_EULER_Y, IGAME_EULER_Z};
	std::map<long, Point3> euler;

	for(int i=0; i<3; i++) {
		if(ctrl->GetLinearKeys(keys, eulertype[i])) {
			maxlog("node %s: getting %d linear euler %c keys\n", nodename, keys.Count(), "xyz"[i]);
			for(int j=0; j<keys.Count(); j++) {
				long tm = KEY_TIME(keys[j]);
				Point3 v = euler[tm];
				v[i] = keys[j].linearKey.fval;
				euler[tm] = v;
			}
		} else if(ctrl->GetBezierKeys(keys, eulertype[i])) {
			maxlog("node %s: getting %d bezier euler %c keys\n", nodename, keys.Count(), "xyz"[i]);
			for(int j=0; j<keys.Count(); j++) {
				long tm = KEY_TIME(keys[j]);
				Point3 v = euler[tm];
				v[i] = keys[j].bezierKey.fval;
				euler[tm] = v;
			}
		} else if(ctrl->GetTCBKeys(keys, eulertype[i])) {
			maxlog("node %s: getting %d tcb euler %c keys\n", nodename, keys.Count(), "xyz"[i]);
			for(int j=0; j<keys.Count(); j++) {
				long tm = KEY_TIME(keys[j]);
				Point3 v = euler[tm];
				v[i] = keys[j].tcbKey.fval;
				euler[tm] = v;
			}
		}
	}

	int order = ctrl->GetEulerOrder();
	std::map<long, Point3>::iterator it = euler.begin();
	while(it != euler.end()) {
		Quat q;
		EulerToQuat(it->second, q, order);
		goat3d_set_node_rotation(node, q.x, q.y, q.z, -q.w, it->first);
		++it;
	}
}

static void get_scaling_keys(IGameControl *ctrl, goat3d_node *node)
{
	const char *nodename = goat3d_get_node_name(node);
	IGameKeyTab keys;

	// XXX the way I'm using the ScaleValue is wrong, but fuck it...

	if(ctrl->GetLinearKeys(keys, IGAME_SCALE)) {
		maxlog("node %s: getting %d linear scaling keys\n", nodename, keys.Count());
		for(int i=0; i<keys.Count(); i++) {
			ScaleValue s = keys[i].linearKey.sval;
			goat3d_set_node_scaling(node, s.s.x, s.s.y, s.s.z, KEY_TIME(keys[i]));
		}
	} else if(ctrl->GetBezierKeys(keys, IGAME_SCALE)) {
		maxlog("node %s: getting %d bezier scaling keys\n", nodename, keys.Count());
		for(int i=0; i<keys.Count(); i++) {
			ScaleValue s = keys[i].bezierKey.sval;
			goat3d_set_node_scaling(node, s.s.x, s.s.y, s.s.z, KEY_TIME(keys[i]));
		}
	} else if(ctrl->GetTCBKeys(keys, IGAME_SCALE)) {
		maxlog("node %s: getting %d tcb scaling keys\n", nodename, keys.Count());
		for(int i=0; i<keys.Count(); i++) {
			ScaleValue s = keys[i].tcbKey.sval;
			goat3d_set_node_scaling(node, s.s.x, s.s.y, s.s.z, KEY_TIME(keys[i]));
		}
	}
}

static bool get_anim_bounds(IGameNode *node, long *tstart, long *tend);
static bool get_node_anim_bounds(IGameNode *node, long *tstart, long *tend);

static bool get_anim_bounds(IGameScene *igame, long *tstart, long *tend)
{
	long tmin = LONG_MAX;
	long tmax = LONG_MIN;

	int num_nodes = igame->GetTopLevelNodeCount();
	for(int i=0; i<num_nodes; i++) {
		long t0, t1;
		if(get_anim_bounds(igame->GetTopLevelNode(i), &t0, &t1)) {
			if(t0 < tmin) tmin = t0;
			if(t1 > tmax) tmax = t1;
		}
	}

	if(tmin != LONG_MAX) {
		*tstart = tmin;
		*tend = tmax;
		return true;
	}
	return false;
}

static bool get_anim_bounds(IGameNode *node, long *tstart, long *tend)
{
	long tmin = LONG_MAX;
	long tmax = LONG_MIN;

	get_node_anim_bounds(node, &tmin, &tmax);

	int num_children = node->GetChildCount();
	for(int i=0; i<num_children; i++) {
		long t0, t1;
		if(get_anim_bounds(node->GetNodeChild(i), &t0, &t1)) {
			if(t0 < tmin) tmin = t0;
			if(t1 > tmax) tmax = t1;
		}
	}

	if(tmin != LONG_MAX) {
		*tstart = tmin;
		*tend = tmax;
		return true;
	}
	return false;
}

static bool get_node_anim_bounds(IGameNode *node, long *tstart, long *tend)
{
	static const IGameControlType ctypes[] = {
		IGAME_POS, IGAME_POS_X, IGAME_POS_Y, IGAME_POS_Z,
		IGAME_ROT, IGAME_EULER_X, IGAME_EULER_Y, IGAME_EULER_Z,
		IGAME_SCALE
	};

	// NOTE: apparently if I don't call GetIGameObject, then GetIGameControl always returns null...
	node->GetIGameObject();
	IGameControl *ctrl = node->GetIGameControl();
	if(!ctrl) {
		maxlog("%s: failed to get IGameControl for node: %s\n", __FUNCTION__, max_string(node->GetName()));
		return false;
	}

	IGameKeyTab keys;
	long t0, t1;
	long tmin = LONG_MAX;
	long tmax = LONG_MIN;

	for(int i=0; i<sizeof ctypes / sizeof *ctypes; i++) {
		if(ctrl->GetBezierKeys(keys, ctypes[i]) && keys.Count()) {
			t0 = KEY_TIME(keys[0]);
			t1 = KEY_TIME(keys[keys.Count() - 1]);
			if(t0 < tmin) tmin = t0;
			if(t1 > tmax) tmax = t1;
		}
		if(ctrl->GetLinearKeys(keys, ctypes[i]) && keys.Count()) {
			t0 = KEY_TIME(keys[0]);
			t1 = KEY_TIME(keys[keys.Count() - 1]);
			if(t0 < tmin) tmin = t0;
			if(t1 > tmax) tmax = t1;
		}
		if(ctrl->GetTCBKeys(keys, ctypes[i]) && keys.Count()) {
			t0 = KEY_TIME(keys[0]);
			t1 = KEY_TIME(keys[keys.Count() - 1]);
			if(t0 < tmin) tmin = t0;
			if(t1 > tmax) tmax = t1;
		}
	}

	if(tmin != LONG_MAX) {
		*tstart = tmin;
		*tend = tmax;
		return true;
	}
	return false;
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
	// TODO
}

void GoatExporter::process_camera(goat3d *goat, goat3d_camera *cam, IGameObject *maxobj)
{
	// TODO
}

static INT_PTR CALLBACK scene_gui_handler(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg) {
	case WM_INITDIALOG:
		CheckDlgButton(win, IDC_GOAT_NODES, 1);
		CheckDlgButton(win, IDC_GOAT_MESHES, 1);
		CheckDlgButton(win, IDC_GOAT_LIGHTS, 1);
		CheckDlgButton(win, IDC_GOAT_CAMERAS, 1);
		break;

	case WM_COMMAND:
		switch(LOWORD(wparam)) {
		case IDOK:
			EndDialog(win, 1);
			break;

		case IDCANCEL:
			EndDialog(win, 0);
			break;

		default:
			return 0;
		}
		break;

	default:
		return 0;
	}

	return 1;
}



// ---- GoatAnimExporter implementation ----
static long tstart, tend;

int GoatAnimExporter::DoExport(const MCHAR *name, ExpInterface *eiface, Interface *iface, BOOL silent, DWORD opt)
{
	if(!(igame = GetIGameInterface())) {
		maxlog("failed to get the igame interface\n");
		return IMPEXP_FAIL;
	}
	IGameConversionManager *cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::IGAME_OGL);
	igame->InitialiseIGame();
	igame->SetStaticFrame(0);

	tstart = tend = 0;
	get_anim_bounds(igame, &tstart, &tend);

	if(!DialogBox(hinst, MAKEINTRESOURCE(IDD_GOAT_ANM), 0, anim_gui_handler)) {
		igame->ReleaseIGame();
		return IMPEXP_CANCEL;
	}

	char fname[512];
	wcstombs(fname, name, sizeof fname - 1);
	for(int i=0; fname[i]; i++) {
		fname[i] = tolower(fname[i]);
	}

	maxlog("Exporting Goat3D Animation (text) file: %s\n", fname);

	goat3d *goat = goat3d_create();

	// process all nodes
	for(int i=0; i<igame->GetTopLevelNodeCount(); i++) {
		IGameNode *node = igame->GetTopLevelNode(i);
		process_node(goat, 0, node);
	}

	if(goat3d_save_anim(goat, fname) == -1) {
		goat3d_free(goat);
		igame->ReleaseIGame();
		return IMPEXP_FAIL;
	}

	goat3d_free(goat);
	igame->ReleaseIGame();
	return IMPEXP_SUCCESS;
}

static INT_PTR CALLBACK anim_gui_handler(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			wchar_t buf[128];
			CheckDlgButton(win, IDC_GOAT_ANM_FULL, BST_CHECKED);
			CheckDlgButton(win, IDC_RAD_KEYS_ORIG, BST_CHECKED);
			wsprintf(buf, L"%ld", tstart);
			SetDlgItemText(win, IDC_EDIT_TSTART, buf);
			wsprintf(buf, L"%ld", tend);
			SetDlgItemText(win, IDC_EDIT_TEND, buf);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wparam)) {
		case IDOK:
			EndDialog(win, 1);
			break;

		case IDCANCEL:
			EndDialog(win, 0);
			break;

		default:
			return 0;
		}
		break;

	default:
		return 0;
	}

	return 1;
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

class GoatAnimClassDesc : public ClassDesc2 {
public:
	int IsPublic() { return TRUE; }
	void *Create(BOOL loading = FALSE) { return new GoatAnimExporter; }
	const TCHAR *ClassName() { return L"GoatAnimExporter"; }
	SClass_ID SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID ClassID() { return Class_ID(0x51b94924, 0x2e0332f3); }
	const TCHAR *Category() { return L"Mutant Stargoat"; }

	const TCHAR *InternalName() { return L"GoatAnimExporter"; }
	HINSTANCE HInstance() { return hinst; }
};

// TODO: make 2 class descriptors, one for goat3d, one for goat3danim
static GoatClassDesc class_desc;
static GoatAnimClassDesc anim_class_desc;

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
	switch(i) {
	case 0:
		return &class_desc;
	case 1:
		return &anim_class_desc;
	default:
		break;
	}
	return 0;
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

	maxlog_open(path);
	return TRUE;
}

__declspec(dllexport) int LibShutdown()
{
	maxlog_close();
	return TRUE;
}

}	// extern "C"


static const char *max_string(const MCHAR *wstr)
{
	if(!wstr) return 0;
	static char str[512];
	wcstombs(str, wstr, sizeof str - 1);
	return str;
}
