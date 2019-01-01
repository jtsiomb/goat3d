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
#ifndef OBJECT_H_
#define OBJECT_H_

#include <cgmath/cgmath.h>
#include <anim/anim.h>
#include "goat3d.h"
#include "aabox.h"

enum {
	OBJTYPE_UNKNOWN,
	OBJTYPE_MESH,
	OBJTYPE_LIGHT,
	OBJTYPE_CAMERA
};

enum {
	LTYPE_POINT,
	LTYPE_DIR,
	LTYPE_SPOT
};

enum {
	CAMTYPE_PRS,
	CAMTYPE_TARGET
};


struct face {
	int v[3];
};

typedef struct int4 {
	int x, y, z, w;
} int4;

struct material_attrib {
	char *name;
	cgm_vec4 value;
	char *map;
};

struct goat3d_material {
	char *name;
	struct material_attrib *attrib;	/* dynarr */
};


#define OBJECT_COMMON	\
	int type; \
	char *name; \
	cgm_vec3 pos; \
	cgm_quat rot; \
	cgm_vec3 scale; \
	void *next

struct object {
	OBJECT_COMMON;
};

struct goat3d_mesh {
	OBJECT_COMMON;
	struct goat3d_material *mtl;

	/* dynamic arrays */
	cgm_vec3 *vertices;
	cgm_vec3 *normals;
	cgm_vec3 *tangents;
	cgm_vec2 *texcoords;
	cgm_vec4 *skin_weights;
	int4 *skin_matrices;
	cgm_vec4 *colors;
	struct face *faces;
	struct anm_node **bones;
};

struct goat3d_light {
	OBJECT_COMMON;
	int ltype;
	cgm_vec3 color;
	cgm_vec3 attenuation;
	float max_dist;
	cgm_vec3 dir;					/* for LTYPE_DIR */
	float inner_cone, outer_cone;	/* for LTYPE_SPOT */
};

struct goat3d_camera {
	OBJECT_COMMON;
	int camtype;
	float near_clip, far_clip;
	cgm_vec3 target, up;
};

struct goat3d_node {
	struct anm_node anm;	/* keep this at 0 offset */
	enum goat3d_node_type type;
	void *obj;
	int child_count;
};

int g3dimpl_obj_init(struct object *o, int type);
void g3dimpl_obj_destroy(struct object *o);

void g3dimpl_mesh_bounds(struct aabox *bb, struct goat3d_mesh *m, float *xform);

int g3dimpl_mtl_init(struct goat3d_material *mtl);
void g3dimpl_mtl_destroy(struct goat3d_material *mtl);
struct material_attrib *g3dimpl_mtl_findattr(struct goat3d_material *mtl, const char *name);
struct material_attrib *g3dimpl_mtl_getattr(struct goat3d_material *mtl, const char *name);

void g3dimpl_node_bounds(struct aabox *bb, struct anm_node *n);

#endif	/* OBJECT_H_ */
