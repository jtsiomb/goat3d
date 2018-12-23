/*
goat3d - 3D scene, and animation file format library.
Copyright (C) 2013-2018  John Tsiombikas <nuclear@member.fsf.org>

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
#include "goat3d.h"
#include "goat3d_impl.h"
#include "log.h"
#include "dynarr.h"

static long read_file(void *buf, size_t bytes, void *uptr);
static long write_file(const void *buf, size_t bytes, void *uptr);
static long seek_file(long offs, int whence, void *uptr);
static char *clean_filename(char *str);

GOAT3DAPI struct goat3d *goat3d_create(void)
{
	struct goat3d *g;

	if(!(g = malloc(sizeof *g))) {
		return 0;
	}
	if(goat3d_init(g) == -1) {
		free(g);
		return 0;
	}
	return g;
}

GOAT3DAPI void goat3d_free(struct goat3d *g)
{
	goat3d_destroy(g);
	free(g);
}

int goat3d_init(struct goat3d *g)
{
	memset(g, 0, sizeof *g);

	if(goat3d_set_name(g, "unnamed") == -1) goto err;
	cgm_vcons(&g->ambient, 0.05, 0.05, 0.05);

	if(!(g->materials = dynarr_alloc(0, sizeof *g->materials))) goto err;
	if(!(g->meshes = dynarr_alloc(0, sizeof *g->meshes))) goto err;
	if(!(g->lights = dynarr_alloc(0, sizeof *g->lights))) goto err;
	if(!(g->cameras = dynarr_alloc(0, sizeof *g->cameras))) goto err;
	if(!(g->nodes = dynarr_alloc(0, sizeof *g->nodes))) goto err;

	goat3d_setopt(g, GOAT3D_OPT_SAVEXML, 1);
	return 0;

err:
	goat3d_destroy(g);
	return -1;
}

void goat3d_destroy(struct goat3d *g)
{
	goat3d_clear(g);

	dynarr_free(g->materials);
	dynarr_free(g->meshes);
	dynarr_free(g->lights);
	dynarr_free(g->cameras);
	dynarr_free(g->nodes);
}

void goat3d_clear(struct goat3d *g)
{
	int i, num;

	num = dynarr_size(g->materials);
	for(i=0; i<num; i++) {
		g3dimpl_mtl_destroy(g->materials[i]);
		free(g->materials[i]);
	}
	dynarr_clear(g->materials);

	num = dynarr_size(g->meshes);
	for(i=0; i<num; i++) {
		g3dimpl_obj_destroy((struct object*)g->meshes[i]);
		free(g->meshes[i]);
	}
	dynarr_clear(g->meshes);

	num = dynarr_size(g->lights);
	for(i=0; i<num; i++) {
		g3dimpl_obj_destroy((struct object*)g->lights[i]);
		free(g->lights[i]);
	}
	dynarr_clear(g->lights);

	num = dynarr_size(g->cameras);
	for(i=0; i<num; i++) {
		g3dimpl_obj_destroy((struct object*)g->cameras[i]);
		free(g->cameras[i]);
	}
	dynarr_clear(g->cameras);

	num = dynarr_size(g->nodes);
	for(i=0; i<num; i++) {
		anm_destroy_node(&g->nodes[i]->anm);
		free(g->nodes[i]);
	}
	dynarr_clear(g->nodes);

	goat3d_set_name(g, "unnamed");
	g->bbox_valid = 0;
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
	int len, res;
	char *slash;
	FILE *fp = fopen(fname, "rb");
	if(!fp) {
		goat3d_logmsg(LOG_ERROR, "failed to open file \"%s\" for reading: %s\n", fname, strerror(errno));
		return -1;
	}

	/* if the filename contained any directory components, keep the prefix
	 * to use it as a search path for external mesh file loading
	 */
	len = strlen(fname);
	if(!(g->search_path = malloc(len + 1))) {
		fclose(fp);
		return -1;
	}
	memcpy(g->search_path, fname, len + 1);

	if((slash = strrchr(g->search_path, '/'))) {
		*slash = 0;
	} else {
		if((slash = strrchr(g->search_path, '\\'))) {
			*slash = 0;
		} else {
			free(g->search_path);
			g->search_path = 0;
		}
	}

	res = goat3d_load_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_save(const struct goat3d *g, const char *fname)
{
	int res;
	FILE *fp = fopen(fname, "wb");
	if(!fp) {
		goat3d_logmsg(LOG_ERROR, "failed to open file \"%s\" for writing: %s\n", fname, strerror(errno));
		return -1;
	}

	res = goat3d_save_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_load_file(struct goat3d *g, FILE *fp)
{
	struct goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_load_io(g, &io);
}

GOAT3DAPI int goat3d_save_file(const struct goat3d *g, FILE *fp)
{
	struct goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_save_io(g, &io);
}

GOAT3DAPI int goat3d_load_io(struct goat3d *g, struct goat3d_io *io)
{
	return g3dimpl_scnload(g, io);
}

GOAT3DAPI int goat3d_save_io(const struct goat3d *g, struct goat3d_io *io)
{
	if(goat3d_getopt(g, GOAT3D_OPT_SAVEXML)) {
		goat3d_logmsg(LOG_ERROR, "saving in the original xml format is no longer supported\n");
		return -1;
	} else if(goat3d_getopt(g, GOAT3D_OPT_SAVETEXT)) {
		/* TODO set treestore output format as text */
	}
	return g3dimpl_scnsave(g, io);
}

/* save/load animations */
GOAT3DAPI int goat3d_load_anim(struct goat3d *g, const char *fname)
{
	int res;
	FILE *fp;

	if(!(fp = fopen(fname, "rb"))) {
		return -1;
	}

	res = goat3d_load_anim_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_save_anim(const struct goat3d *g, const char *fname)
{
	int res;
	FILE *fp;

	if(!(fp = fopen(fname, "wb"))) {
		return -1;
	}

	res = goat3d_save_anim_file(g, fp);
	fclose(fp);
	return res;
}

GOAT3DAPI int goat3d_load_anim_file(struct goat3d *g, FILE *fp)
{
	struct goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_load_anim_io(g, &io);
}

GOAT3DAPI int goat3d_save_anim_file(const struct goat3d *g, FILE *fp)
{
	struct goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_save_anim_io(g, &io);
}

GOAT3DAPI int goat3d_load_anim_io(struct goat3d *g, struct goat3d_io *io)
{
	return g3dimpl_anmload(g, io);
}

GOAT3DAPI int goat3d_save_anim_io(const struct goat3d *g, struct goat3d_io *io)
{
	if(goat3d_getopt(g, GOAT3D_OPT_SAVEXML)) {
		goat3d_logmsg(LOG_ERROR, "saving in the original xml format is no longer supported\n");
		return -1;
	} else if(goat3d_getopt(g, GOAT3D_OPT_SAVETEXT)) {
		/* TODO set treestore save format as text */
	}
	return g3dimpl_anmsave(g, io);
}


GOAT3DAPI int goat3d_set_name(struct goat3d *g, const char *name)
{
	int len = strlen(name);

	free(g->name);
	if(!(g->name = malloc(len + 1))) {
		return -1;
	}
	memcpy(g->name, name, len + 1);
	return 0;
}

GOAT3DAPI const char *goat3d_get_name(const struct goat3d *g)
{
	return g->name;
}

GOAT3DAPI void goat3d_set_ambient(struct goat3d *g, const float *amb)
{
	cgm_vcons(&g->ambient, amb[0], amb[1], amb[2]);
}

GOAT3DAPI void goat3d_set_ambient3f(struct goat3d *g, float ar, float ag, float ab)
{
	cgm_vcons(&g->ambient, ar, ag, ab);
}

GOAT3DAPI const float *goat3d_get_ambient(const struct goat3d *g)
{
	return &g->ambient.x;
}

GOAT3DAPI int goat3d_get_bounds(const struct goat3d *g, float *bmin, float *bmax)
{
	int i, num_nodes;
	struct aabox node_bbox;

	if(!g->bbox_valid) {
		g3dimpl_aabox_init((struct aabox*)&g->bbox);

		num_nodes = dynarr_size(g->nodes);
		for(i=0; i<num_nodes; i++) {
			if(g->nodes[i]->anm.parent) {
				continue;
			}
			g3dimpl_node_bounds(&node_bbox, &g->nodes[i]->anm);
			g3dimpl_aabox_union((struct aabox*)&g->bbox, &g->bbox, &node_bbox);
		}
		((struct goat3d*)g)->bbox_valid = 1;
	}

	bmin[0] = g->bbox.bmin.x;
	bmin[1] = g->bbox.bmin.y;
	bmin[2] = g->bbox.bmin.z;
	bmax[0] = g->bbox.bmax.x;
	bmax[1] = g->bbox.bmax.y;
	bmax[2] = g->bbox.bmax.z;
	return 0;
}

// ---- materials ----
GOAT3DAPI int goat3d_add_mtl(struct goat3d *g, struct goat3d_material *mtl)
{
	struct goat3d_material **newarr;
	if(!(newarr = dynarr_push(g->materials, &mtl))) {
		return -1;
	}
	g->materials = newarr;
	return 0;
}

GOAT3DAPI int goat3d_get_mtl_count(struct goat3d *g)
{
	return dynarr_size(g->materials);
}

GOAT3DAPI struct goat3d_material *goat3d_get_mtl(struct goat3d *g, int idx)
{
	return g->materials[idx];
}

GOAT3DAPI struct goat3d_material *goat3d_get_mtl_by_name(struct goat3d *g, const char *name)
{
	int i, num = dynarr_size(g->materials);
	for(i=0; i<num; i++) {
		if(strcmp(g->materials[i]->name, name) == 0) {
			return g->materials[i];
		}
	}
	return 0;
}

GOAT3DAPI struct goat3d_material *goat3d_create_mtl(void)
{
	struct goat3d_material *mtl;
	if(!(mtl = malloc(sizeof *mtl))) {
		return 0;
	}
	g3dimpl_mtl_init(mtl);
	return mtl;
}

GOAT3DAPI void goat3d_destroy_mtl(struct goat3d_material *mtl)
{
	g3dimpl_mtl_destroy(mtl);
	free(mtl);
}

GOAT3DAPI int goat3d_set_mtl_name(struct goat3d_material *mtl, const char *name)
{
	char *tmp;
	int len = strlen(name);
	if(!(tmp = malloc(len + 1))) {
		return -1;
	}
	memcpy(tmp, name, len + 1);
	free(mtl->name);
	mtl->name = tmp;
	return 0;
}

GOAT3DAPI const char *goat3d_get_mtl_name(const struct goat3d_material *mtl)
{
	return mtl->name;
}

GOAT3DAPI int goat3d_set_mtl_attrib(struct goat3d_material *mtl, const char *attrib, const float *val)
{
	struct material_attrib *ma = g3dimpl_mtl_getattr(mtl, attrib);
	if(!ma) return -1;
	cgm_wcons(&ma->value, val[0], val[1], val[2], val[3]);
	return 0;
}

GOAT3DAPI int goat3d_set_mtl_attrib1f(struct goat3d_material *mtl, const char *attrib, float val)
{
	return goat3d_set_mtl_attrib4f(mtl, attrib, val, 0, 0, 1);
}

GOAT3DAPI int goat3d_set_mtl_attrib3f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b)
{
	return goat3d_set_mtl_attrib4f(mtl, attrib, r, g, b, 1);
}

GOAT3DAPI int goat3d_set_mtl_attrib4f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b, float a)
{
	struct material_attrib *ma = g3dimpl_mtl_getattr(mtl, attrib);
	if(!ma) return -1;
	cgm_wcons(&ma->value, r, g, b, a);
	return 0;
}

GOAT3DAPI const float *goat3d_get_mtl_attrib(struct goat3d_material *mtl, const char *attrib)
{
	struct material_attrib *ma = g3dimpl_mtl_findattr(mtl, attrib);
	return ma ? &ma->value.x : 0;
}

GOAT3DAPI int goat3d_set_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib, const char *mapname)
{
	int len;
	char *tmp;
	struct material_attrib *ma;

	len = strlen(mapname);
	if(!(tmp = malloc(len + 1))) {
		return -1;
	}
	memcpy(tmp, mapname, len + 1);

	if(!(ma = g3dimpl_mtl_getattr(mtl, attrib))) {
		free(tmp);
		return -1;
	}
	free(ma->map);
	ma->map = tmp;
	tmp = clean_filename(ma->map);
	if(tmp != ma->map) {
		memmove(ma->map, tmp, len - (tmp - ma->map) + 1);
	}
	return 0;
}

GOAT3DAPI const char *goat3d_get_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib)
{
	struct material_attrib *ma = g3dimpl_mtl_findattr(mtl, attrib);
	return ma->map;
}

// ---- meshes ----
GOAT3DAPI int goat3d_add_mesh(struct goat3d *g, struct goat3d_mesh *mesh)
{
	struct goat3d_mesh **arr;
	if(!(arr = dynarr_push(g->meshes, &mesh))) {
		return -1;
	}
	g->meshes = arr;
	return 0;
}

GOAT3DAPI int goat3d_get_mesh_count(struct goat3d *g)
{
	return dynarr_size(g->meshes);
}

GOAT3DAPI struct goat3d_mesh *goat3d_get_mesh(struct goat3d *g, int idx)
{
	return g->meshes[idx];
}

GOAT3DAPI struct goat3d_mesh *goat3d_get_mesh_by_name(struct goat3d *g, const char *name)
{
	int i, num = dynarr_size(g->meshes);
	for(i=0; i<num; i++) {
		if(strcmp(g->meshes[i]->name, name) == 0) {
			return g->meshes[i];
		}
	}
	return 0;
}

GOAT3DAPI struct goat3d_mesh *goat3d_create_mesh(void)
{
	struct goat3d_mesh *m;

	if(!(m = malloc(sizeof *m))) {
		return 0;
	}
	if(g3dimpl_obj_init((struct object*)m, OBJTYPE_MESH) == -1) {
		free(m);
		return 0;
	}
	return m;
}

GOAT3DAPI void goat3d_destroy_mesh(struct goat3d_mesh *mesh)
{
	g3dimpl_obj_destroy((struct object*)mesh);
	free(mesh);
}

GOAT3DAPI int goat3d_set_mesh_name(struct goat3d_mesh *mesh, const char *name)
{
	char *tmpname;
	int len = strlen(name);

	if(!(tmpname = malloc(len + 1))) {
		return -1;
	}
	memcpy(tmpname, name, len + 1);
	free(mesh->name);
	mesh->name = tmpname;
	return 0;
}

GOAT3DAPI const char *goat3d_get_mesh_name(const struct goat3d_mesh *mesh)
{
	return mesh->name;
}

GOAT3DAPI void goat3d_set_mesh_mtl(struct goat3d_mesh *mesh, struct goat3d_material *mtl)
{
	mesh->mtl = mtl;
}

GOAT3DAPI struct goat3d_material *goat3d_get_mesh_mtl(struct goat3d_mesh *mesh)
{
	return mesh->mtl;
}

GOAT3DAPI int goat3d_get_mesh_attrib_count(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib)
{
	return dynarr_size(mesh->vertices);
}

GOAT3DAPI int goat3d_get_mesh_face_count(struct goat3d_mesh *mesh)
{
	return dynarr_size(mesh->faces);
}

#define SET_VERTEX_DATA(arr, p, n) \
	do { \
		void *tmp = dynarr_resize(arr, n); \
		if(!tmp) { \
			goat3d_logmsg(LOG_ERROR, "failed to resize vertex array (%d)\n", n); \
			return -1; \
		} \
		arr = tmp; \
		memcpy(arr, p, n * sizeof *arr); \
	} while(0)

GOAT3DAPI int goat3d_set_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, const void *data, int vnum)
{
	if(attrib == GOAT3D_MESH_ATTR_VERTEX) {
		SET_VERTEX_DATA(mesh->vertices, data, vnum);
		return 0;
	}

	if(vnum != dynarr_size(mesh->vertices)) {
		goat3d_logmsg(LOG_ERROR, "trying to set mesh attrib data with number of elements different than the vertex array\n");
		return -1;
	}

	switch(attrib) {
	case GOAT3D_MESH_ATTR_NORMAL:
		SET_VERTEX_DATA(mesh->normals, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_TANGENT:
		SET_VERTEX_DATA(mesh->tangents, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_TEXCOORD:
		SET_VERTEX_DATA(mesh->texcoords, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		SET_VERTEX_DATA(mesh->skin_weights, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		SET_VERTEX_DATA(mesh->skin_matrices, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_COLOR:
		SET_VERTEX_DATA(mesh->colors, data, vnum);
	default:
		goat3d_logmsg(LOG_ERROR, "trying to set unknown vertex attrib: %d\n", attrib);
		return -1;
	}
	return 0;
}

GOAT3DAPI int goat3d_add_mesh_attrib1f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float val)
{
	return goat3d_add_mesh_attrib4f(mesh, attrib, val, 0, 0, 1);
}

GOAT3DAPI int goat3d_add_mesh_attrib2f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float x, float y)
{
	return goat3d_add_mesh_attrib4f(mesh, attrib, x, y, 0, 1);
}

GOAT3DAPI int goat3d_add_mesh_attrib3f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float x, float y, float z)
{
	return goat3d_add_mesh_attrib4f(mesh, attrib, x, y, z, 1);
}

GOAT3DAPI int goat3d_add_mesh_attrib4f(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib,
		float x, float y, float z, float w)
{
	float vec[4];
	int4 intvec;
	void *tmp;

	switch(attrib) {
	case GOAT3D_MESH_ATTR_VERTEX:
		cgm_vcons((cgm_vec3*)vec, x, y, z);
		if(!(tmp = dynarr_push(mesh->vertices, vec))) {
			goto err;
		}
		mesh->vertices = tmp;
		break;

	case GOAT3D_MESH_ATTR_NORMAL:
		cgm_vcons((cgm_vec3*)vec, x, y, z);
		if(!(tmp = dynarr_push(mesh->normals, vec))) {
			goto err;
		}
		mesh->normals = tmp;
		break;

	case GOAT3D_MESH_ATTR_TANGENT:
		cgm_vcons((cgm_vec3*)vec, x, y, z);
		if(!(tmp = dynarr_push(mesh->tangents, vec))) {
			goto err;
		}
		mesh->tangents = tmp;
		break;

	case GOAT3D_MESH_ATTR_TEXCOORD:
		cgm_vcons((cgm_vec3*)vec, x, y, 0);
		if(!(tmp = dynarr_push(mesh->texcoords, vec))) {
			goto err;
		}
		mesh->texcoords = tmp;
		break;

	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		cgm_wcons((cgm_vec4*)vec, x, y, z, w);
		if(!(tmp = dynarr_push(mesh->skin_weights, vec))) {
			goto err;
		}
		mesh->skin_weights = tmp;
		break;

	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		intvec.x = x;
		intvec.y = y;
		intvec.z = z;
		intvec.w = w;
		if(!(tmp = dynarr_push(mesh->skin_matrices, &intvec))) {
			goto err;
		}
		mesh->skin_matrices = tmp;
		break;

	case GOAT3D_MESH_ATTR_COLOR:
		cgm_wcons((cgm_vec4*)vec, x, y, z, w);
		if(!(tmp = dynarr_push(mesh->colors, vec))) {
			goto err;
		}
		mesh->colors = tmp;

	default:
		goat3d_logmsg(LOG_ERROR, "trying to add unknown vertex attrib: %d\n", attrib);
		return -1;
	}
	return 0;

err:
	goat3d_logmsg(LOG_ERROR, "failed to push vertex attrib\n");
	return -1;
}

GOAT3DAPI void *goat3d_get_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib)
{
	return goat3d_get_mesh_attrib(mesh, attrib, 0);
}

GOAT3DAPI void *goat3d_get_mesh_attrib(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, int idx)
{
	switch(attrib) {
	case GOAT3D_MESH_ATTR_VERTEX:
		return dynarr_empty(mesh->vertices) ? 0 : mesh->vertices + idx;
	case GOAT3D_MESH_ATTR_NORMAL:
		return dynarr_empty(mesh->normals) ? 0 : mesh->normals + idx;
	case GOAT3D_MESH_ATTR_TANGENT:
		return dynarr_empty(mesh->tangents) ? 0 : mesh->tangents + idx;
	case GOAT3D_MESH_ATTR_TEXCOORD:
		return dynarr_empty(mesh->texcoords) ? 0 : mesh->texcoords + idx;
	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		return dynarr_empty(mesh->skin_weights) ? 0 : mesh->skin_weights + idx;
	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		return dynarr_empty(mesh->skin_matrices) ? 0 : mesh->skin_matrices + idx;
	case GOAT3D_MESH_ATTR_COLOR:
		return dynarr_empty(mesh->colors) ? 0 : mesh->colors + idx;
	default:
		break;
	}
	return 0;
}


GOAT3DAPI int goat3d_set_mesh_faces(struct goat3d_mesh *mesh, const int *data, int num)
{
	void *tmp;
	if(!(tmp = dynarr_resize(mesh->faces, num))) {
		goat3d_logmsg(LOG_ERROR, "failed to resize face array (%d)\n", num);
		return -1;
	}
	mesh->faces = tmp;
	memcpy(mesh->faces, data, num * sizeof *mesh->faces);
	return 0;
}

GOAT3DAPI int goat3d_add_mesh_face(struct goat3d_mesh *mesh, int a, int b, int c)
{
	void *tmp;
	struct face face;

	face.v[0] = a;
	face.v[1] = b;
	face.v[2] = c;

	if(!(tmp = dynarr_push(mesh->faces, &face))) {
		goat3d_logmsg(LOG_ERROR, "failed to add face\n");
		return -1;
	}
	mesh->faces = tmp;
	return 0;
}

GOAT3DAPI int *goat3d_get_mesh_faces(struct goat3d_mesh *mesh)
{
	return goat3d_get_mesh_face(mesh, 0);
}

GOAT3DAPI int *goat3d_get_mesh_face(struct goat3d_mesh *mesh, int idx)
{
	return dynarr_empty(mesh->faces) ? 0 : mesh->faces[idx].v;
}

// immedate mode state
static enum goat3d_im_primitive im_prim;
static struct goat3d_mesh *im_mesh;
static cgm_vec3 im_norm, im_tang;
static cgm_vec2 im_texcoord;
static cgm_vec4 im_skinw, im_color = {1, 1, 1, 1};
static int4 im_skinmat;
static int im_use[NUM_GOAT3D_MESH_ATTRIBS];


GOAT3DAPI void goat3d_begin(struct goat3d_mesh *mesh, enum goat3d_im_primitive prim)
{
	dynarr_clear(mesh->vertices);
	dynarr_clear(mesh->normals);
	dynarr_clear(mesh->tangents);
	dynarr_clear(mesh->texcoords);
	dynarr_clear(mesh->skin_weights);
	dynarr_clear(mesh->skin_matrices);
	dynarr_clear(mesh->colors);
	dynarr_clear(mesh->faces);

	im_mesh = mesh;
	memset(im_use, 0, sizeof im_use);

	im_prim = prim;
}

GOAT3DAPI void goat3d_end(void)
{
	int i, vidx, num_faces, num_quads;
	void *tmp;

	switch(im_prim) {
	case GOAT3D_TRIANGLES:
		{
			num_faces = dynarr_size(im_mesh->vertices) / 3;
			if(!(tmp = dynarr_resize(im_mesh->faces, num_faces))) {
				return;
			}
			im_mesh->faces = tmp;

			vidx = 0;
			for(i=0; i<num_faces; i++) {
				im_mesh->faces[i].v[0] = vidx++;
				im_mesh->faces[i].v[1] = vidx++;
				im_mesh->faces[i].v[2] = vidx++;
			}
		}
		break;

	case GOAT3D_QUADS:
		{
			num_quads = dynarr_size(im_mesh->vertices) / 4;
			if(!(tmp = dynarr_resize(im_mesh->faces, num_quads * 2))) {
				return;
			}
			im_mesh->faces = tmp;

			vidx = 0;
			for(i=0; i<num_quads; i++) {
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
		break;
	}
}

GOAT3DAPI void goat3d_vertex3f(float x, float y, float z)
{
	void *tmp;
	cgm_vec3 v;

	cgm_vcons(&v, x, y, z);
	if(!(tmp = dynarr_push(im_mesh->vertices, &v))) {
		return;
	}
	im_mesh->vertices = tmp;

	if(im_use[GOAT3D_MESH_ATTR_NORMAL]) {
		if((tmp = dynarr_push(im_mesh->normals, &im_norm))) {
			im_mesh->normals = tmp;
		}
	}
	if(im_use[GOAT3D_MESH_ATTR_TANGENT]) {
		if((tmp = dynarr_push(im_mesh->tangents, &im_tang))) {
			im_mesh->tangents = tmp;
		}
	}
	if(im_use[GOAT3D_MESH_ATTR_TEXCOORD]) {
		if((tmp = dynarr_push(im_mesh->texcoords, &im_texcoord))) {
			im_mesh->texcoords = tmp;
		}
	}
	if(im_use[GOAT3D_MESH_ATTR_SKIN_WEIGHT]) {
		if((tmp = dynarr_push(im_mesh->skin_weights, &im_skinw))) {
			im_mesh->skin_weights = tmp;
		}
	}
	if(im_use[GOAT3D_MESH_ATTR_SKIN_MATRIX]) {
		if((tmp = dynarr_push(im_mesh->skin_matrices, &im_skinmat))) {
			im_mesh->skin_matrices = tmp;
		}
	}
	if(im_use[GOAT3D_MESH_ATTR_COLOR]) {
		if((tmp = dynarr_push(im_mesh->colors, &im_color))) {
			im_mesh->colors = tmp;
		}
	}
}

GOAT3DAPI void goat3d_normal3f(float x, float y, float z)
{
	cgm_vcons(&im_norm, x, y, z);
	im_use[GOAT3D_MESH_ATTR_NORMAL] = 1;
}

GOAT3DAPI void goat3d_tangent3f(float x, float y, float z)
{
	cgm_vcons(&im_tang, x, y, z);
	im_use[GOAT3D_MESH_ATTR_TANGENT] = 1;
}

GOAT3DAPI void goat3d_texcoord2f(float x, float y)
{
	im_texcoord.x = x;
	im_texcoord.y = y;
	im_use[GOAT3D_MESH_ATTR_TEXCOORD] = 1;
}

GOAT3DAPI void goat3d_skin_weight4f(float x, float y, float z, float w)
{
	cgm_wcons(&im_skinw, x, y, z, w);
	im_use[GOAT3D_MESH_ATTR_SKIN_WEIGHT] = 1;
}

GOAT3DAPI void goat3d_skin_matrix4i(int x, int y, int z, int w)
{
	im_skinmat.x = x;
	im_skinmat.y = y;
	im_skinmat.z = z;
	im_skinmat.w = w;
	im_use[GOAT3D_MESH_ATTR_SKIN_MATRIX] = 1;
}

GOAT3DAPI void goat3d_color3f(float x, float y, float z)
{
	goat3d_color4f(x, y, z, 1.0f);
}

GOAT3DAPI void goat3d_color4f(float x, float y, float z, float w)
{
	cgm_wcons(&im_color, x, y, z, w);
	im_use[GOAT3D_MESH_ATTR_COLOR] = 1;
}

GOAT3DAPI void goat3d_get_mesh_bounds(const struct goat3d_mesh *mesh, float *bmin, float *bmax)
{
	struct aabox box;

	g3dimpl_mesh_bounds(&box, (struct goat3d_mesh*)mesh, 0);

	bmin[0] = box.bmin.x;
	bmin[1] = box.bmin.y;
	bmin[2] = box.bmin.z;
	bmax[0] = box.bmax.x;
	bmax[1] = box.bmax.y;
	bmax[2] = box.bmax.z;
}

/* lights */
GOAT3DAPI int goat3d_add_light(struct goat3d *g, struct goat3d_light *lt)
{
	struct goat3d_light **arr;
	if(!(arr = dynarr_push(g->lights, &lt))) {
		return -1;
	}
	g->lights = arr;
	return 0;
}

GOAT3DAPI int goat3d_get_light_count(struct goat3d *g)
{
	return dynarr_size(g->lights);
}

GOAT3DAPI struct goat3d_light *goat3d_get_light(struct goat3d *g, int idx)
{
	return g->lights[idx];
}

GOAT3DAPI struct goat3d_light *goat3d_get_light_by_name(struct goat3d *g, const char *name)
{
	int i, num = dynarr_size(g->lights);
	for(i=0; i<num; i++) {
		if(strcmp(g->lights[i]->name, name) == 0) {
			return g->lights[i];
		}
	}
	return 0;
}


GOAT3DAPI struct goat3d_light *goat3d_create_light(void)
{
	struct goat3d_light *lt;

	if(!(lt = malloc(sizeof *lt))) {
		return 0;
	}
	if(g3dimpl_obj_init((struct object*)lt, OBJTYPE_LIGHT) == -1) {
		free(lt);
		return 0;
	}
	return lt;
}

GOAT3DAPI void goat3d_destroy_light(struct goat3d_light *lt)
{
	g3dimpl_obj_destroy((struct object*)lt);
	free(lt);
}


/* cameras */
GOAT3DAPI int goat3d_add_camera(struct goat3d *g, struct goat3d_camera *cam)
{
	struct goat3d_camera **arr;
	if(!(arr = dynarr_push(g->cameras, &cam))) {
		return -1;
	}
	g->cameras = arr;
	return 0;
}

GOAT3DAPI int goat3d_get_camera_count(struct goat3d *g)
{
	return dynarr_size(g->cameras);
}

GOAT3DAPI struct goat3d_camera *goat3d_get_camera(struct goat3d *g, int idx)
{
	return g->cameras[idx];
}

GOAT3DAPI struct goat3d_camera *goat3d_get_camera_by_name(struct goat3d *g, const char *name)
{
	int i, num = dynarr_size(g->cameras);
	for(i=0; i<num; i++) {
		if(strcmp(g->cameras[i]->name, name) == 0) {
			return g->cameras[i];
		}
	}
	return 0;
}

GOAT3DAPI struct goat3d_camera *goat3d_create_camera(void)
{
	struct goat3d_camera *cam;

	if(!(cam = malloc(sizeof *cam))) {
		return 0;
	}
	if(g3dimpl_obj_init((struct object*)cam, OBJTYPE_CAMERA) == -1) {
		free(cam);
		return 0;
	}
	return cam;
}

GOAT3DAPI void goat3d_destroy_camera(struct goat3d_camera *cam)
{
	g3dimpl_obj_destroy((struct object*)cam);
	free(cam);
}



// node
GOAT3DAPI int goat3d_add_node(struct goat3d *g, struct goat3d_node *node)
{
	struct goat3d_node **arr;
	if(!(arr = dynarr_push(g->nodes, &node))) {
		return -1;
	}
	g->nodes = arr;
	return 0;
}

GOAT3DAPI int goat3d_get_node_count(struct goat3d *g)
{
	return dynarr_size(g->nodes);
}

GOAT3DAPI struct goat3d_node *goat3d_get_node(struct goat3d *g, int idx)
{
	return g->nodes[idx];
}

GOAT3DAPI struct goat3d_node *goat3d_get_node_by_name(struct goat3d *g, const char *name)
{
	int i, num = dynarr_size(g->nodes);
	for(i=0; i<num; i++) {
		if(strcmp(g->nodes[i]->anm.name, name) == 0) {
			return g->nodes[i];
		}
	}
	return 0;
}

GOAT3DAPI struct goat3d_node *goat3d_create_node(void)
{
	struct goat3d_node *node;

	if(!(node = malloc(sizeof *node))) {
		return 0;
	}
	if(anm_init_node(&node->anm) == -1) {
		free(node);
		return 0;
	}
	node->type = GOAT3D_NODE_NULL;
	node->obj = 0;
	node->child_count = 0;

	return node;
}

GOAT3DAPI void goat3d_destroy_node(struct goat3d_node *node)
{
	anm_destroy_node(&node->anm);
	free(node);
}

GOAT3DAPI int goat3d_set_node_name(struct goat3d_node *node, const char *name)
{
	return anm_set_node_name(&node->anm, name);
}

GOAT3DAPI const char *goat3d_get_node_name(const struct goat3d_node *node)
{
	return anm_get_node_name((struct anm_node*)&node->anm);
}

GOAT3DAPI void goat3d_set_node_object(struct goat3d_node *node, enum goat3d_node_type type, void *obj)
{
	node->obj = obj;
	node->type = type;
}

GOAT3DAPI void *goat3d_get_node_object(const struct goat3d_node *node)
{
	return node->obj;
}

GOAT3DAPI enum goat3d_node_type goat3d_get_node_type(const struct goat3d_node *node)
{
	return node->type;
}

GOAT3DAPI void goat3d_add_node_child(struct goat3d_node *node, struct goat3d_node *child)
{
	anm_link_node(&node->anm, &child->anm);
	node->child_count++;
}

GOAT3DAPI int goat3d_get_node_child_count(const struct goat3d_node *node)
{
	return node->child_count;
}

GOAT3DAPI struct goat3d_node *goat3d_get_node_child(const struct goat3d_node *node, int idx)
{
	struct anm_node *c = node->anm.child;
	while(c && idx > 0) {
		c = c->next;
	}
	return (struct goat3d_node*)c;
}

GOAT3DAPI struct goat3d_node *goat3d_get_node_parent(const struct goat3d_node *node)
{
	return (struct goat3d_node*)node->anm.parent;
}

GOAT3DAPI void goat3d_use_anim(struct goat3d_node *node, int idx)
{
	anm_use_animation(&node->anm, idx);
}

GOAT3DAPI void goat3d_use_anims(struct goat3d_node *node, int aidx, int bidx, float t)
{
	anm_use_animations(&node->anm, aidx, bidx, t);
}

GOAT3DAPI void goat3d_use_anim_by_name(struct goat3d_node *node, const char *name)
{
	anm_use_animation(&node->anm, anm_find_animation(&node->anm, name));
}

GOAT3DAPI void goat3d_use_anims_by_name(struct goat3d_node *node, const char *aname, const char *bname, float t)
{
	int aidx = anm_find_animation(&node->anm, aname);
	int bidx = anm_find_animation(&node->anm, bname);

	if(aidx == -1) {
		anm_use_animation(&node->anm, bidx);
	}
	if(bidx == -1) {
		anm_use_animation(&node->anm, aidx);
	}
	anm_use_animations(&node->anm, aidx, bidx, t);
}

GOAT3DAPI int goat3d_get_active_anim(struct goat3d_node *node, int which)
{
	return anm_get_active_animation_index(&node->anm, which);
}

GOAT3DAPI float goat3d_get_active_anim_mix(struct goat3d_node *node)
{
	return anm_get_active_animation_mix(&node->anm);
}

GOAT3DAPI int goat3d_get_anim_count(struct goat3d_node *node)
{
	return anm_get_animation_count(&node->anm);
}

GOAT3DAPI void goat3d_add_anim(struct goat3d_node *root)
{
	int idx = anm_get_animation_count(&root->anm);
	anm_add_animation(&root->anm);
	anm_use_animation(&root->anm, idx);
}

GOAT3DAPI void goat3d_set_anim_name(struct goat3d_node *root, const char *name)
{
	anm_set_active_animation_name(&root->anm, name);
}

GOAT3DAPI const char *goat3d_get_anim_name(struct goat3d_node *node)
{
	return anm_get_active_animation_name(&node->anm);
}

enum { POSITION_TRACK, ROTATION_TRACK, SCALING_TRACK };
static const int track_type_base[] = {ANM_TRACK_POS_X, ANM_TRACK_ROT_X, ANM_TRACK_SCL_X};
static const int track_type_nelem[] = {3, 4, 4};

static int get_key_count(struct anm_node *node, int trackid)
{
	struct anm_animation *anim = anm_get_active_animation(node, 0);
	if(anim) {
		return anim->tracks[track_type_base[trackid]].count;
	}
	return 0;
}

static long get_key_time(struct anm_node *node, int trackid, int idx)
{
	struct anm_animation *anim = anm_get_active_animation(node, 0);
	struct anm_keyframe *key = anm_get_keyframe(anim->tracks + track_type_base[trackid], idx);
	return ANM_TM2MSEC(key->time);
}

static int get_key_value(struct anm_node *node, int trackid, int idx, float *val)
{
	struct anm_animation *anim = anm_get_active_animation(node, 0);
	int i, nelem = track_type_nelem[trackid];
	for(i=0; i<nelem; i++) {
		struct anm_keyframe *key = anm_get_keyframe(anim->tracks + track_type_base[trackid] + i, idx);
		val[i] = key->val;
	}
	return nelem;
}

GOAT3DAPI long goat3d_get_anim_timeline(struct goat3d_node *root, long *tstart, long *tend)
{
	int i, nkeys;
	struct anm_node *c;
	long node_start = LONG_MAX;
	long node_end = LONG_MIN;

	for(i=0; i<3; i++) {
		if((nkeys = get_key_count(&root->anm, i)) > 0) {
			long tmp = get_key_time(&root->anm, i, 0);
			if(tmp < node_start) node_start = tmp;
			tmp = get_key_time(&root->anm, i, nkeys - 1);
			if(tmp > node_end) node_end = tmp;
		}
	}

	c = root->anm.child;
	while(c) {
		long cstart, cend;
		if(goat3d_get_anim_timeline((struct goat3d_node*)c, &cstart, &cend)) {
			if(cstart < node_start) node_start = cstart;
			if(cend > node_end) node_end = cend;
		}
		c = c->next;
	}

	if(node_start != LONG_MAX) {
		*tstart = node_start;
		*tend = node_end;
		return node_end - node_start;
	}
	return -1;
}

GOAT3DAPI int goat3d_get_node_position_key_count(struct goat3d_node *node)
{
	return get_key_count(&node->anm, POSITION_TRACK);
}

GOAT3DAPI int goat3d_get_node_rotation_key_count(struct goat3d_node *node)
{
	return get_key_count(&node->anm, ROTATION_TRACK);
}

GOAT3DAPI int goat3d_get_node_scaling_key_count(struct goat3d_node *node)
{
	return get_key_count(&node->anm, SCALING_TRACK);
}

GOAT3DAPI long goat3d_get_node_position_key(struct goat3d_node *node, int idx, float *xptr, float *yptr, float *zptr)
{
	cgm_vec3 pos;
	long tm;

	get_key_value(&node->anm, POSITION_TRACK, idx, &pos.x);
	tm = get_key_time(&node->anm, POSITION_TRACK, idx);

	if(xptr) *xptr = pos.x;
	if(yptr) *yptr = pos.y;
	if(zptr) *zptr = pos.z;
	return tm;
}

GOAT3DAPI long goat3d_get_node_rotation_key(struct goat3d_node *node, int idx, float *xptr, float *yptr, float *zptr, float *wptr)
{
	cgm_quat rot;
	long tm;

	get_key_value(&node->anm, ROTATION_TRACK, idx, &rot.x);
	tm = get_key_time(&node->anm, ROTATION_TRACK, idx);

	if(xptr) *xptr = rot.x;
	if(yptr) *yptr = rot.y;
	if(zptr) *zptr = rot.z;
	if(wptr) *wptr = rot.w;
	return tm;
}

GOAT3DAPI long goat3d_get_node_scaling_key(struct goat3d_node *node, int idx, float *xptr, float *yptr, float *zptr)
{
	cgm_vec3 scale;
	long tm;

	get_key_value(&node->anm, SCALING_TRACK, idx, &scale.x);
	tm = get_key_time(&node->anm, SCALING_TRACK, idx);

	if(xptr) *xptr = scale.x;
	if(yptr) *yptr = scale.y;
	if(zptr) *zptr = scale.z;
	return tm;
}

GOAT3DAPI void goat3d_set_node_position(struct goat3d_node *node, float x, float y, float z, long tmsec)
{
	anm_set_position3f(&node->anm, x, y, z, ANM_MSEC2TM(tmsec));
}

GOAT3DAPI void goat3d_set_node_rotation(struct goat3d_node *node, float qx, float qy, float qz, float qw, long tmsec)
{
	anm_set_rotation4f(&node->anm, qx, qy, qz, qw, ANM_MSEC2TM(tmsec));
}

GOAT3DAPI void goat3d_set_node_scaling(struct goat3d_node *node, float sx, float sy, float sz, long tmsec)
{
	anm_set_scaling3f(&node->anm, sx, sy, sz, ANM_MSEC2TM(tmsec));
}

GOAT3DAPI void goat3d_set_node_pivot(struct goat3d_node *node, float px, float py, float pz)
{
	anm_set_pivot(&node->anm, px, py, pz);
}


GOAT3DAPI void goat3d_get_node_position(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr, long tmsec)
{
	float pos[3];
	anm_get_node_position((struct anm_node*)&node->anm, pos, ANM_MSEC2TM(tmsec));
	*xptr = pos[0];
	*yptr = pos[1];
	*zptr = pos[2];
}

GOAT3DAPI void goat3d_get_node_rotation(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr, float *wptr, long tmsec)
{
	float rot[4];
	anm_get_node_rotation((struct anm_node*)&node->anm, rot, ANM_MSEC2TM(tmsec));
	*xptr = rot[0];
	*yptr = rot[1];
	*zptr = rot[2];
	*wptr = rot[3];
}

GOAT3DAPI void goat3d_get_node_scaling(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr, long tmsec)
{
	float scale[3];
	anm_get_node_scaling((struct anm_node*)&node->anm, scale, ANM_MSEC2TM(tmsec));
	*xptr = scale[0];
	*yptr = scale[1];
	*zptr = scale[2];
}

GOAT3DAPI void goat3d_get_node_pivot(const struct goat3d_node *node, float *xptr, float *yptr, float *zptr)
{
	anm_get_pivot((struct anm_node*)&node->anm, xptr, yptr, zptr);
}


GOAT3DAPI void goat3d_get_node_matrix(const struct goat3d_node *node, float *matrix, long tmsec)
{
	anm_get_node_matrix((struct anm_node*)&node->anm, matrix, ANM_MSEC2TM(tmsec));
}

GOAT3DAPI void goat3d_get_node_bounds(const struct goat3d_node *node, float *bmin, float *bmax)
{
	struct aabox box;
	g3dimpl_node_bounds(&box, (struct anm_node*)&node->anm);

	bmin[0] = box.bmin.x;
	bmin[1] = box.bmin.y;
	bmin[2] = box.bmin.z;
	bmax[0] = box.bmax.x;
	bmax[1] = box.bmax.y;
	bmax[2] = box.bmax.z;
}


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

static char *clean_filename(char *str)
{
	char *last_slash, *ptr;

	if(!(last_slash = strrchr(str, '/'))) {
		last_slash = strrchr(str, '\\');
	}
	if(last_slash) {
		str = last_slash + 1;
	}

	ptr = str;
	while(*ptr) {
		char c = tolower(*ptr);
		*ptr++ = c;
	}
	*ptr = 0;
	return str;
}
