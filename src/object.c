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
#include "object.h"
#include "dynarr.h"

int g3dimpl_obj_init(union object *o, int type)
{
	struct mesh *m;
	struct light *lt;
	struct camera *cam;

	memset(o, 0, sizeof *o);
	o->any.type = type;
	cgm_qcons(&o->any.rot, 0, 0, 0, 1);
	cgm_vcons(&o->any.scale, 1, 1, 1);

	switch(type) {
	case OBJTYPE_MESH:
		m = &o->mesh;
		if(!(m->vertices = dynarr_alloc(0, sizeof *m->vertices))) break;
		if(!(m->normals = dynarr_alloc(0, sizeof *m->normals))) break;
		if(!(m->tangents = dynarr_alloc(0, sizeof *m->tangents))) break;
		if(!(m->texcoords = dynarr_alloc(0, sizeof *m->texcoords))) break;
		if(!(m->skin_weights = dynarr_alloc(0, sizeof *m->skin_weights))) break;
		if(!(m->skin_matrices = dynarr_alloc(0, sizeof *m->skin_matrices))) break;
		if(!(m->colors = dynarr_alloc(0, sizeof *m->colors))) break;
		if(!(m->faces = dynarr_alloc(0, sizeof *m->faces))) break;
		if(!(m->bones = dynarr_alloc(0, sizeof *m->bones))) break;
		return 0;

	case OBJTYPE_LIGHT:
		lt = &o->light;
		cgm_vcons(&lt->color, 1, 1, 1);
		cgm_vcons(&lt->attenuation, 1, 0, 0);
		cgm_vcons(&lt->dir, 0, 0, 1);
		lt->inner_cone = cgm_deg_to_rad(30);
		lt->outer_cone = cgm_deg_to_rad(45);
		return 0;

	case OBJTYPE_CAMERA:
		cam = &o->cam;
		cam->near_clip = 0.5f;
		cam->far_clip = 500.0f;
		cgm_vcons(&cam->up, 0, 1, 0);
		return 0;

	default:
		break;
	}

	g3dimpl_obj_destroy(o);
	return -1;
}

void g3dimpl_obj_destroy(union object *o)
{
	switch(o->any.type) {
	case OBJTYPE_MESH:
		dynarr_free(o->mesh.vertices);
		dynarr_free(o->mesh.normals);
		dynarr_free(o->mesh.tangents);
		dynarr_free(o->mesh.texcoords);
		dynarr_free(o->mesh.skin_weights);
		dynarr_free(o->mesh.skin_matrices);
		dynarr_free(o->mesh.colors);
		dynarr_free(o->mesh.faces);
		dynarr_free(o->mesh.bones);
		break;

	default:
		break;
	}
}

void g3dimpl_mesh_bounds(struct aabox *bb, struct mesh *m, float *xform)
{
	int i, nverts;

	g3dimpl_aabox_init(bb);

	nverts = dynarr_size(m->vertices);
	for(i=0; i<nverts; i++) {
		cgm_vec3 v = m->vertices[i];
		if(xform) cgm_vmul_m4v3(&v, xform);

		if(v.x < bb->bmin.x) bb->bmin.x = v.x;
		if(v.y < bb->bmin.y) bb->bmin.y = v.y;
		if(v.z < bb->bmin.z) bb->bmin.z = v.z;

		if(v.x > bb->bmax.x) bb->bmax.x = v.x;
		if(v.y > bb->bmax.y) bb->bmax.y = v.y;
		if(v.z > bb->bmax.z) bb->bmax.z = v.z;
	}
}

struct material_attrib *g3dimpl_material_findattr(struct material *mtl, const char *name)
{
	int i, num = dynarr_size(mtl->attrib);

	for(i=0; i<num; i++) {
		if(strcmp(mtl->attrib[i].name, name) == 0) {
			return mtl->attrib + i;
		}
	}
	return 0;
}

void g3dimpl_node_bounds(struct aabox *bb, struct anm_node *n)
{
	union object *obj = n->data;
	struct anm_node *cn = n->child;
	float *xform = anm_get_matrix(n, 0, 0);

	if(obj) {
		g3dimpl_mesh_bounds(bb, &obj->mesh, xform);
	} else {
		g3dimpl_aabox_init(bb);
	}

	while(cn) {
		struct aabox cbox;
		g3dimpl_node_bounds(&cbox, cn);
		g3dimpl_aabox_union(bb, bb, &cbox);
		cn = cn->next;
	}
}
