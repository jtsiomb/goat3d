/*
goat3d - 3D scene, character, and animation file format library.
Copyright (C) 2013-2014  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <string>
#include "goat3d.h"
#include "goat3d_impl.h"
#include "log.h"

#ifndef _MSC_VER
#include <alloca.h>
#else
#include <malloc.h>
#endif

using namespace g3dimpl;

struct goat3d_material : public Material {};
struct goat3d_mesh : public Mesh {};
struct goat3d_light : public Light {};
struct goat3d_camera : public Camera {};
struct goat3d_node : public Node {};


static long read_file(void *buf, size_t bytes, void *uptr);
static long write_file(const void *buf, size_t bytes, void *uptr);
static long seek_file(long offs, int whence, void *uptr);

extern "C" {

GOAT3DAPI struct goat3d *goat3d_create(void)
{
	goat3d *goat = new goat3d;
	goat->flags = 0;
	goat->search_path = 0;
	goat->scn = new Scene;
	goat->scn->goat = goat;

	goat3d_setopt(goat, GOAT3D_OPT_SAVEXML, 1);
	return goat;
}

GOAT3DAPI void goat3d_free(struct goat3d *g)
{
	delete g->search_path;
	delete g->scn;
	delete g;
}

GOAT3DAPI void goat3d_setopt(struct goat3d *g, enum goat3d_option opt, int val)
{
	if(val) {
		g->flags |= (1 << (int)opt);
	} else {
		g->flags &= ~(1 << (int)opt);
	}
}

GOAT3DAPI int goat3d_getopt(const struct goat3d *g, enum goat3d_option opt)
{
	return (g->flags >> (int)opt) & 1;
}

GOAT3DAPI int goat3d_load(struct goat3d *g, const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if(!fp) {
		logmsg(LOG_ERROR, "failed to open file \"%s\" for reading: %s\n", fname, strerror(errno));
		return -1;
	}

	/* if the filename contained any directory components, keep the prefix
	 * to use it as a search path for external mesh file loading
	 */
	g->search_path = new char[strlen(fname) + 1];
	strcpy(g->search_path, fname);

	char *slash = strrchr(g->search_path, '/');
	if(slash) {
		*slash = 0;
	} else {
		if((slash = strrchr(g->search_path, '\\'))) {
			*slash = 0;
		} else {
			delete [] g->search_path;
			g->search_path = 0;
		}
	}

	int res = goat3d_load_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_save(const struct goat3d *g, const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	if(!fp) {
		logmsg(LOG_ERROR, "failed to open file \"%s\" for writing: %s\n", fname, strerror(errno));
		return -1;
	}

	int res = goat3d_save_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_load_file(struct goat3d *g, FILE *fp)
{
	goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_load_io(g, &io);
}

GOAT3DAPI int goat3d_save_file(const struct goat3d *g, FILE *fp)
{
	goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_save_io(g, &io);
}

GOAT3DAPI int goat3d_load_io(struct goat3d *g, struct goat3d_io *io)
{
	if(!g->scn->load(io)) {
		if(!g->scn->loadxml(io)) {
			return -1;
		}
	}
	return 0;
}

GOAT3DAPI int goat3d_save_io(const struct goat3d *g, struct goat3d_io *io)
{
	if(goat3d_getopt(g, GOAT3D_OPT_SAVEXML)) {
		return g->scn->savexml(io) ? 0 : -1;
	}
	return g->scn->save(io) ? 0 : -1;
}

/* save/load animations */
GOAT3DAPI int goat3d_load_anim(struct goat3d *g, const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if(!fp) {
		return -1;
	}

	int res = goat3d_load_anim_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_save_anim(const struct goat3d *g, const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	if(!fp) {
		return -1;
	}

	int res = goat3d_save_anim_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_load_anim_file(struct goat3d *g, FILE *fp)
{
	goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_load_anim_io(g, &io);
}

GOAT3DAPI int goat3d_save_anim_file(const struct goat3d *g, FILE *fp)
{
	goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_save_anim_io(g, &io);
}

GOAT3DAPI int goat3d_load_anim_io(struct goat3d *g, struct goat3d_io *io)
{
	if(!g->scn->load_anim(io)) {
		if(!g->scn->load_anim_xml(io)) {
			return -1;
		}
	}
	return 0;
}

GOAT3DAPI int goat3d_save_anim_io(const struct goat3d *g, struct goat3d_io *io)
{
	if(goat3d_getopt(g, GOAT3D_OPT_SAVEXML)) {
		return g->scn->save_anim_xml(io) ? 0 : -1;
	}
	return g->scn->save_anim(io) ? 0 : -1;
}


GOAT3DAPI int goat3d_set_name(struct goat3d *g, const char *name)
{
	g->scn->set_name(name);
	return 0;
}

GOAT3DAPI const char *goat3d_get_name(const struct goat3d *g)
{
	return g->scn->get_name();
}

GOAT3DAPI void goat3d_set_ambient(struct goat3d *g, const float *amb)
{
	g->scn->set_ambient(Vector3(amb[0], amb[1], amb[2]));
}

GOAT3DAPI void goat3d_set_ambient3f(struct goat3d *g, float ar, float ag, float ab)
{
	g->scn->set_ambient(Vector3(ar, ag, ab));
}

GOAT3DAPI const float *goat3d_get_ambient(const struct goat3d *g)
{
	return &g->scn->get_ambient().x;
}

GOAT3DAPI void goat3d_get_bounds(const struct goat3d *g, float *bmin, float *bmax)
{
	AABox bbox = g->scn->get_bounds();
	for(int i=0; i<3; i++) {
		bmin[i] = bbox.bmin[i];
		bmax[i] = bbox.bmax[i];
	}
}

// ---- materials ----
GOAT3DAPI void goat3d_add_mtl(struct goat3d *g, struct goat3d_material *mtl)
{
	g->scn->add_material(mtl);
}

GOAT3DAPI int goat3d_get_mtl_count(struct goat3d *g)
{
	return g->scn->get_material_count();
}

GOAT3DAPI struct goat3d_material *goat3d_get_mtl(struct goat3d *g, int idx)
{
	return (goat3d_material*)g->scn->get_material(idx);
}

GOAT3DAPI struct goat3d_material *goat3d_get_mtl_by_name(struct goat3d *g, const char *name)
{
	return (goat3d_material*)g->scn->get_material(name);
}

GOAT3DAPI struct goat3d_material *goat3d_create_mtl(void)
{
	return new goat3d_material;
}

GOAT3DAPI void goat3d_destroy_mtl(struct goat3d_material *mtl)
{
	delete mtl;
}

GOAT3DAPI void goat3d_set_mtl_name(struct goat3d_material *mtl, const char *name)
{
	mtl->name = std::string(name);
}

GOAT3DAPI const char *goat3d_get_mtl_name(const struct goat3d_material *mtl)
{
	return mtl->name.c_str();
}

GOAT3DAPI void goat3d_set_mtl_attrib(struct goat3d_material *mtl, const char *attrib, const float *val)
{
	(*mtl)[attrib].value = Vector4(val[0], val[1], val[2], val[3]);
}

GOAT3DAPI void goat3d_set_mtl_attrib1f(struct goat3d_material *mtl, const char *attrib, float val)
{
	goat3d_set_mtl_attrib4f(mtl, attrib, val, 0, 0, 1);
}

GOAT3DAPI void goat3d_set_mtl_attrib3f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b)
{
	goat3d_set_mtl_attrib4f(mtl, attrib, r, g, b, 1);
}

GOAT3DAPI void goat3d_set_mtl_attrib4f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b, float a)
{
	(*mtl)[attrib].value = Vector4(r, g, b, a);
}

GOAT3DAPI const float *goat3d_get_mtl_attrib(struct goat3d_material *mtl, const char *attrib)
{
	return &(*mtl)[attrib].value.x;
}

GOAT3DAPI void goat3d_set_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib, const char *mapname)
{
	(*mtl)[attrib].map = clean_filename(mapname);
}

GOAT3DAPI const char *goat3d_get_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib)
{
	return (*mtl)[attrib].map.c_str();
}

// ---- meshes ----
GOAT3DAPI void goat3d_add_mesh(struct goat3d *g, struct goat3d_mesh *mesh)
{
	g->scn->add_mesh(mesh);
}

GOAT3DAPI int goat3d_get_mesh_count(struct goat3d *g)
{
	return g->scn->get_mesh_count();
}

GOAT3DAPI struct goat3d_mesh *goat3d_get_mesh(struct goat3d *g, int idx)
{
	return (goat3d_mesh*)g->scn->get_mesh(idx);
}

GOAT3DAPI struct goat3d_mesh *goat3d_get_mesh_by_name(struct goat3d *g, const char *name)
{
	return (goat3d_mesh*)g->scn->get_mesh(name);
}

GOAT3DAPI struct goat3d_mesh *goat3d_create_mesh(void)
{
	return new goat3d_mesh;
}

GOAT3DAPI void goat3d_destroy_mesh(struct goat3d_mesh *mesh)
{
	delete mesh;
}

GOAT3DAPI void goat3d_set_mesh_name(struct goat3d_mesh *mesh, const char *name)
{
	mesh->name = std::string(name);
}

GOAT3DAPI const char *goat3d_get_mesh_name(const struct goat3d_mesh *mesh)
{
	return mesh->name.c_str();
}

GOAT3DAPI void goat3d_set_mesh_mtl(struct goat3d_mesh *mesh, struct goat3d_material *mtl)
{
	mesh->material = mtl;
}

GOAT3DAPI struct goat3d_material *goat3d_get_mesh_mtl(struct goat3d_mesh *mesh)
{
	return (goat3d_material*)mesh->material;
}

GOAT3DAPI int goat3d_get_mesh_attrib_count(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib)
{
	return (int)mesh->vertices.size();
}

GOAT3DAPI int goat3d_get_mesh_face_count(struct goat3d_mesh *mesh)
{
	return (int)mesh->faces.size();
}

// VECDATA is in goat3d_impl.h
GOAT3DAPI void goat3d_set_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, const void *data, int vnum)
{
	if(attrib == GOAT3D_MESH_ATTR_VERTEX) {
		mesh->vertices = VECDATA(Vector3, data, vnum);
		return;
	}

	if(vnum != (int)mesh->vertices.size()) {
		logmsg(LOG_ERROR, "trying to set mesh attrib data with number of elements different than the vertex array\n");
		return;
	}

	switch(attrib) {
	case GOAT3D_MESH_ATTR_NORMAL:
		mesh->normals = VECDATA(Vector3, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_TANGENT:
		mesh->tangents = VECDATA(Vector3, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_TEXCOORD:
		mesh->texcoords = VECDATA(Vector2, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		mesh->skin_weights = VECDATA(Vector4, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		mesh->skin_matrices = VECDATA(Int4, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_COLOR:
		mesh->colors = VECDATA(Vector4, data, vnum);
	default:
		break;
	}
}

GOAT3DAPI void goat3d_add_mesh_attrib1f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float val)
{
	goat3d_add_mesh_attrib4f(mesh, attrib, val, 0, 0, 1);
}

GOAT3DAPI void goat3d_add_mesh_attrib2f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float x, float y)
{
	goat3d_add_mesh_attrib4f(mesh, attrib, x, y, 0, 1);
}

GOAT3DAPI void goat3d_add_mesh_attrib3f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float x, float y, float z)
{
	goat3d_add_mesh_attrib4f(mesh, attrib, x, y, z, 1);
}

GOAT3DAPI void goat3d_add_mesh_attrib4f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float x, float y, float z, float w)
{
	switch(attrib) {
	case GOAT3D_MESH_ATTR_VERTEX:
		mesh->vertices.push_back(Vector3(x, y, z));
		break;
	case GOAT3D_MESH_ATTR_NORMAL:
		mesh->normals.push_back(Vector3(x, y, z));
		break;
	case GOAT3D_MESH_ATTR_TANGENT:
		mesh->tangents.push_back(Vector3(x, y, z));
		break;
	case GOAT3D_MESH_ATTR_TEXCOORD:
		mesh->texcoords.push_back(Vector2(x, y));
		break;
	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		mesh->skin_weights.push_back(Vector4(x, y, z, w));
		break;
	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		mesh->skin_matrices.push_back(Int4(x, y, z, w));
		break;
	case GOAT3D_MESH_ATTR_COLOR:
		mesh->colors.push_back(Vector4(x, y, z, w));
	default:
		break;
	}
}

GOAT3DAPI void *goat3d_get_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib)
{
	return goat3d_get_mesh_attrib(mesh, attrib, 0);
}

GOAT3DAPI void *goat3d_get_mesh_attrib(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, int idx)
{
	switch(attrib) {
	case GOAT3D_MESH_ATTR_VERTEX:
		return mesh->vertices.empty() ? 0 : (void*)&mesh->vertices[idx];
	case GOAT3D_MESH_ATTR_NORMAL:
		return mesh->normals.empty() ? 0 : (void*)&mesh->normals[idx];
	case GOAT3D_MESH_ATTR_TANGENT:
		return mesh->tangents.empty() ? 0 : (void*)&mesh->tangents[idx];
	case GOAT3D_MESH_ATTR_TEXCOORD:
		return mesh->texcoords.empty() ? 0 : (void*)&mesh->texcoords[idx];
	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		return mesh->skin_weights.empty() ? 0 : (void*)&mesh->skin_weights[idx];
	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		return mesh->skin_matrices.empty() ? 0 : (void*)&mesh->skin_matrices[idx];
	case GOAT3D_MESH_ATTR_COLOR:
		return mesh->colors.empty() ? 0 : (void*)&mesh->colors[idx];
	default:
		break;
	}
	return 0;
}


GOAT3DAPI void goat3d_set_mesh_faces(struct goat3d_mesh *mesh, const int *data, int num)
{
	mesh->faces = VECDATA(Face, data, num);
}

GOAT3DAPI void goat3d_add_mesh_face(struct goat3d_mesh *mesh, int a, int b, int c)
{
	Face face;
	face.v[0] = a;
	face.v[1] = b;
	face.v[2] = c;
	mesh->faces.push_back(face);
}

GOAT3DAPI int *goat3d_get_mesh_faces(struct goat3d_mesh *mesh)
{
	return goat3d_get_mesh_face(mesh, 0);
}

GOAT3DAPI int *goat3d_get_mesh_face(struct goat3d_mesh *mesh, int idx)
{
	return mesh->faces.empty() ? 0 : mesh->faces[idx].v;
}

// immedate mode state
static enum goat3d_im_primitive im_prim;
static struct goat3d_mesh *im_mesh;
static Vector3 im_norm, im_tang;
static Vector2 im_texcoord;
static Vector4 im_skinw, im_color = Vector4(1, 1, 1, 1);
static Int4 im_skinmat;
static bool im_use[NUM_GOAT3D_MESH_ATTRIBS];


GOAT3DAPI void goat3d_begin(struct goat3d_mesh *mesh, enum goat3d_im_primitive prim)
{
	mesh->vertices.clear();
	mesh->normals.clear();
	mesh->tangents.clear();
	mesh->texcoords.clear();
	mesh->skin_weights.clear();
	mesh->skin_matrices.clear();
	mesh->colors.clear();
	mesh->faces.clear();

	im_mesh = mesh;
	memset(im_use, 0, sizeof im_use);

	im_prim = prim;
}

GOAT3DAPI void goat3d_end(void)
{
	switch(im_prim) {
	case GOAT3D_TRIANGLES:
		{
			int num_faces = (int)im_mesh->vertices.size() / 3;
			im_mesh->faces.resize(num_faces);

			int vidx = 0;
			for(int i=0; i<num_faces; i++) {
				im_mesh->faces[i].v[0] = vidx++;
				im_mesh->faces[i].v[1] = vidx++;
				im_mesh->faces[i].v[2] = vidx++;
			}
		}
		break;

	case GOAT3D_QUADS:
		{
			int num_quads = (int)im_mesh->vertices.size() / 4;
			im_mesh->faces.resize(num_quads * 2);

			int vidx = 0;
			for(int i=0; i<num_quads; i++) {
				im_mesh->faces[i * 2].v[0] = vidx;
				im_mesh->faces[i * 2].v[1] = vidx + 1;
				im_mesh->faces[i * 2].v[2] = vidx + 2;

				im_mesh->faces[i * 2 + 1].v[0] = vidx;
				im_mesh->faces[i * 2 + 1].v[1] = vidx + 2;
				im_mesh->faces[i * 2 + 1].v[2] = vidx + 3;

				vidx += 4;
			}
		}
		break;

	default:
		return;
	};
}

GOAT3DAPI void goat3d_vertex3f(float x, float y, float z)
{
	im_mesh->vertices.push_back(Vector3(x, y, z));
	if(im_use[GOAT3D_MESH_ATTR_NORMAL]) {
		im_mesh->normals.push_back(im_norm);
	}
	if(im_use[GOAT3D_MESH_ATTR_TANGENT]) {
		im_mesh->tangents.push_back(im_tang);
	}
	if(im_use[GOAT3D_MESH_ATTR_TEXCOORD]) {
		im_mesh->texcoords.push_back(im_texcoord);
	}
	if(im_use[GOAT3D_MESH_ATTR_SKIN_WEIGHT]) {
		im_mesh->skin_weights.push_back(im_skinw);
	}
	if(im_use[GOAT3D_MESH_ATTR_SKIN_MATRIX]) {
		im_mesh->skin_matrices.push_back(im_skinmat);
	}
	if(im_use[GOAT3D_MESH_ATTR_COLOR]) {
		im_mesh->colors.push_back(im_color);
	}
}

GOAT3DAPI void goat3d_normal3f(float x, float y, float z)
{
	im_norm = Vector3(x, y, z);
	im_use[GOAT3D_MESH_ATTR_NORMAL] = true;
}

GOAT3DAPI void goat3d_tangent3f(float x, float y, float z)
{
	im_tang = Vector3(x, y, z);
	im_use[GOAT3D_MESH_ATTR_TANGENT] = true;
}

GOAT3DAPI void goat3d_texcoord2f(float x, float y)
{
	im_texcoord = Vector2(x, y);
	im_use[GOAT3D_MESH_ATTR_TEXCOORD] = true;
}

GOAT3DAPI void goat3d_skin_weight4f(float x, float y, float z, float w)
{
	im_skinw = Vector4(x, y, z, w);
	im_use[GOAT3D_MESH_ATTR_SKIN_WEIGHT] = true;
}

GOAT3DAPI void goat3d_skin_matrix4i(int x, int y, int z, int w)
{
	im_skinmat.x = x;
	im_skinmat.y = y;
	im_skinmat.z = z;
	im_skinmat.w = w;
	im_use[GOAT3D_MESH_ATTR_SKIN_MATRIX] = true;
}

GOAT3DAPI void goat3d_color3f(float x, float y, float z)
{
	goat3d_color4f(x, y, z, 1.0f);
}

GOAT3DAPI void goat3d_color4f(float x, float y, float z, float w)
{
	im_color = Vector4(x, y, z, w);
	im_use[GOAT3D_MESH_ATTR_COLOR] = true;
}

/* lights */
GOAT3DAPI void goat3d_add_light(struct goat3d *g, struct goat3d_light *lt)
{
	g->scn->add_light(lt);
}

GOAT3DAPI int goat3d_get_light_count(struct goat3d *g)
{
	return g->scn->get_light_count();
}

GOAT3DAPI struct goat3d_light *goat3d_get_light(struct goat3d *g, int idx)
{
	return (goat3d_light*)g->scn->get_light(idx);
}

GOAT3DAPI struct goat3d_light *goat3d_get_light_by_name(struct goat3d *g, const char *name)
{
	return (goat3d_light*)g->scn->get_light(name);
}


GOAT3DAPI struct goat3d_light *goat3d_create_light(void)
{
	return new goat3d_light;
}

GOAT3DAPI void goat3d_destroy_light(struct goat3d_light *lt)
{
	delete lt;
}


/* cameras */
GOAT3DAPI void goat3d_add_camera(struct goat3d *g, struct goat3d_camera *cam)
{
	g->scn->add_camera(cam);
}

GOAT3DAPI int goat3d_get_camera_count(struct goat3d *g)
{
	return g->scn->get_camera_count();
}

GOAT3DAPI struct goat3d_camera *goat3d_get_camera(struct goat3d *g, int idx)
{
	return (goat3d_camera*)g->scn->get_camera(idx);
}

GOAT3DAPI struct goat3d_camera *goat3d_get_camera_by_name(struct goat3d *g, const char *name)
{
	return (goat3d_camera*)g->scn->get_camera(name);
}

GOAT3DAPI struct goat3d_camera *goat3d_create_camera(void)
{
	return new goat3d_camera;
}

GOAT3DAPI void goat3d_destroy_camera(struct goat3d_camera *cam)
{
	delete cam;
}



// node
GOAT3DAPI void goat3d_add_node(struct goat3d *g, struct goat3d_node *node)
{
	g->scn->add_node(node);
}

GOAT3DAPI int goat3d_get_node_count(struct goat3d *g)
{
	return g->scn->get_node_count();
}

GOAT3DAPI struct goat3d_node *goat3d_get_node(struct goat3d *g, int idx)
{
	return (goat3d_node*)g->scn->get_node(idx);
}

GOAT3DAPI struct goat3d_node *goat3d_get_node_by_name(struct goat3d *g, const char *name)
{
	return (goat3d_node*)g->scn->get_node(name);
}

GOAT3DAPI struct goat3d_node *goat3d_create_node(void)
{
	return new goat3d_node;
}

GOAT3DAPI void goat3d_set_node_name(struct goat3d_node *node, const char *name)
{
	node->set_name(name);
}

GOAT3DAPI const char *goat3d_get_node_name(const struct goat3d_node *node)
{
	return node->get_name();
}

GOAT3DAPI void goat3d_set_node_object(struct goat3d_node *node, enum goat3d_node_type type, void *obj)
{
	node->set_object((Object*)obj);
}

GOAT3DAPI void *goat3d_get_node_object(const struct goat3d_node *node)
{
	return (void*)node->get_object();
}

GOAT3DAPI enum goat3d_node_type goat3d_get_node_type(const struct goat3d_node *node)
{
	const Object *obj = node->get_object();
	if(dynamic_cast<const Mesh*>(obj)) {
		return GOAT3D_NODE_MESH;
	}
	if(dynamic_cast<const Light*>(obj)) {
		return GOAT3D_NODE_LIGHT;
	}
	if(dynamic_cast<const Camera*>(obj)) {
		return GOAT3D_NODE_CAMERA;
	}

	return GOAT3D_NODE_NULL;
}

GOAT3DAPI void goat3d_add_node_child(struct goat3d_node *node, struct goat3d_node *child)
{
	node->add_child(child);
}

GOAT3DAPI int goat3d_get_node_child_count(const struct goat3d_node *node)
{
	return node->get_children_count();
}

GOAT3DAPI struct goat3d_node *goat3d_get_node_child(const struct goat3d_node *node, int idx)
{
	return (goat3d_node*)node->get_child(idx);
}

GOAT3DAPI struct goat3d_node *goat3d_get_node_parent(const struct goat3d_node *node)
{
	return (goat3d_node*)node->get_parent();
}

GOAT3DAPI void goat3d_use_anim(struct goat3d_node *node, int idx)
{
	node->use_animation(idx);
}

GOAT3DAPI void goat3d_use_anims(struct goat3d_node *node, int aidx, int bidx, float t)
{
	node->use_animation(aidx, bidx, t);
}

GOAT3DAPI void goat3d_use_anim_by_name(struct goat3d_node *node, const char *name)
{
	node->use_animation(name);
}

GOAT3DAPI void goat3d_use_anims_by_name(struct goat3d_node *node, const char *aname, const char *bname, float t)
{
	node->use_animation(aname, bname, t);
}

GOAT3DAPI int goat3d_get_active_anim(struct goat3d_node *node, int which)
{
	return node->get_active_animation_index(which);
}

GOAT3DAPI float goat3d_get_active_anim_mix(struct goat3d_node *node)
{
	return node->get_active_animation_mix();
}

GOAT3DAPI int goat3d_get_anim_count(struct goat3d_node *node)
{
	return node->get_animation_count();
}

GOAT3DAPI void goat3d_add_anim(struct goat3d_node *root)
{
	root->add_animation();
}

GOAT3DAPI void goat3d_set_anim_name(struct goat3d_node *root, const char *name)
{
	root->set_animation_name(name);
}

GOAT3DAPI const char *goat3d_get_anim_name(struct goat3d_node *node)
{
	return node->get_animation_name();
}

GOAT3DAPI void goat3d_set_node_position(struct goat3d_node *node, float x, float y, float z, long tmsec)
{
	node->set_position(Vector3(x, y, z), tmsec);
}

GOAT3DAPI void goat3d_set_node_rotation(struct goat3d_node *node, float qx, float qy, float qz, float qw, long tmsec)
{
	node->set_rotation(Quaternion(qw, qx, qy, qz), tmsec);
}

GOAT3DAPI void goat3d_set_node_scaling(struct goat3d_node *node, float sx, float sy, float sz, long tmsec)
{
	node->set_scaling(Vector3(sx, sy, sz), tmsec);
}

GOAT3DAPI void goat3d_set_node_pivot(struct goat3d_node *node, float px, float py, float pz)
{
	node->set_pivot(Vector3(px, py, pz));
}


GOAT3DAPI void goat3d_get_node_position(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr, long tmsec)
{
	Vector3 pos = node->get_position(tmsec);
	*xptr = pos.x;
	*yptr = pos.y;
	*zptr = pos.z;
}

GOAT3DAPI void goat3d_get_node_rotation(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr, float *wptr, long tmsec)
{
	Quaternion q = node->get_rotation(tmsec);
	*xptr = q.v.x;
	*yptr = q.v.y;
	*zptr = q.v.z;
	*wptr = q.s;
}

GOAT3DAPI void goat3d_get_node_scaling(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr, long tmsec)
{
	Vector3 scale = node->get_scaling(tmsec);
	*xptr = scale.x;
	*yptr = scale.y;
	*zptr = scale.z;
}

GOAT3DAPI void goat3d_get_node_pivot(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr)
{
	Vector3 pivot = node->get_pivot();
	*xptr = pivot.x;
	*yptr = pivot.y;
	*zptr = pivot.z;
}


GOAT3DAPI void goat3d_get_node_matrix(const struct goat3d_node *node, float *matrix, long tmsec)
{
	node->get_xform(tmsec, (Matrix4x4*)matrix);
}


}	// extern "C"


static long read_file(void *buf, size_t bytes, void *uptr)
{
	return (long)fread(buf, 1, bytes, (FILE*)uptr);
}

static long write_file(const void *buf, size_t bytes, void *uptr)
{
	return (long)fwrite(buf, 1, bytes, (FILE*)uptr);
}

static long seek_file(long offs, int whence, void *uptr)
{
	if(fseek((FILE*)uptr, offs, whence) == -1) {
		return -1;
	}
	return ftell((FILE*)uptr);
}

std::string g3dimpl::clean_filename(const char *str)
{
	const char *last_slash = strrchr(str, '/');
	if(!last_slash) {
		last_slash = strrchr(str, '\\');
	}

	if(last_slash) {
		str = last_slash + 1;
	}

	char *buf = (char*)alloca(strlen(str) + 1);
	char *dest = buf;
	while(*str) {
		char c = *str++;
		*dest++ = tolower(c);
	}
	*dest = 0;
	return buf;
}
