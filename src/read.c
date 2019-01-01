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
#include <assert.h>
#include <treestore.h>
#include "goat3d.h"
#include "goat3d_impl.h"
#include "log.h"
#include "dynarr.h"

struct key {
	long tm;
	cgm_vec4 val;
};

static struct goat3d_material *read_material(struct goat3d *g, struct ts_node *tsmtl);
static char *read_material_attrib(struct material_attrib *attr, struct ts_node *tsmattr);
struct goat3d_mesh *read_mesh(struct goat3d *g, struct ts_node *tsmesh);

int g3dimpl_scnload(struct goat3d *g, struct goat3d_io *io)
{
	struct ts_io tsio;
	struct ts_node *tsroot, *c;

	tsio.data = io->cls;
	tsio.read = io->read;
	tsio.write = io->write;

	if(!(tsroot = ts_load_io(&tsio))) {
		goat3d_logmsg(LOG_ERROR, "failed to load scene\n");
		return -1;
	}
	if(strcmp(tsroot->name, "scene") != 0) {
		goat3d_logmsg(LOG_ERROR, "invalid scene file, root node is not \"scene\"\n");
		ts_free_tree(tsroot);
		return -1;
	}

	/* read all materials */
	c = tsroot->child_list;
	while(c) {
		if(strcmp(c->name, "mtl") == 0) {
			struct goat3d_material *mtl = read_material(g, c);
			if(mtl) {
				goat3d_add_mtl(g, mtl);
			}
		}
		c = c->next;
	}

	/* read all meshes */
	c = tsroot->child_list;
	while(c) {
		if(strcmp(c->name, "mesh") == 0) {
			struct goat3d_mesh *mesh = read_mesh(g, c);
			if(mesh) {
				goat3d_add_mesh(g, mesh);
			}
		}
		c = c->next;
	}

	/* read all nodes */
	/* TODO */

	ts_free_tree(tsroot);
	return 0;
}

int g3dimpl_anmload(struct goat3d *g, struct goat3d_io *io)
{
	/*
	struct ts_io tsio;
	tsio.data = io->cls;
	tsio.read = io->read;
	tsio.write = io->write;
	*/
	return -1;
}

static struct goat3d_material *read_material(struct goat3d *g, struct ts_node *tsmtl)
{
	struct goat3d_material *mtl;
	struct material_attrib mattr, *arr;
	struct ts_node *c;
	const char *str;

	if(!(mtl = malloc(sizeof *mtl)) || g3dimpl_mtl_init(mtl) == -1) {
		goat3d_logmsg(LOG_ERROR, "read_material: failed to allocate material\n");
		return 0;
	}

	if(!(str = ts_get_attr_str(tsmtl, "name", 0)) || !*str) {
		goat3d_logmsg(LOG_WARNING, "read_material: ignoring material without a name\n");
		g3dimpl_mtl_destroy(mtl);
		return 0;
	}

	/* read all material attributes */
	c = tsmtl->child_list;
	while(c) {
		if(strcmp(c->name, "attr") == 0) {
			if(read_material_attrib(&mattr, c)) {
				if(!(arr = dynarr_push(mtl->attrib, &mattr))) {
					goat3d_logmsg(LOG_ERROR, "read_material: failed to resize material attribute array\n");
					g3dimpl_mtl_destroy(mtl);
					return 0;
				}
				mtl->attrib = arr;
			}
		}
		c = c->next;
	}

	if(dynarr_empty(mtl->attrib)) {
		goat3d_logmsg(LOG_WARNING, "read_material: ignoring empty material: %s\n", mtl->name);
		g3dimpl_mtl_destroy(mtl);
		return 0;
	}
	return mtl;
}

static char *read_material_attrib(struct material_attrib *attr, struct ts_node *tsnode)
{
	int i;
	struct ts_attr *tsattr;
	const char *name, *map;

	memset(attr, 0, sizeof *attr);

	if((tsattr = ts_get_attr(tsnode, "val"))) {
		attr->value.w = 1.0f;	/* default W to 1 if we get less than a float4 */

		switch(tsattr->val.type) {
		case TS_NUMBER:
			attr->value.x = tsattr->val.fnum;
			break;
		case TS_VECTOR:
			assert(tsattr->val.vec_size <= 4);
			for(i=0; i<tsattr->val.vec_size; i++) {
				(&attr->value.x)[i] = tsattr->val.vec[i];
			}
			break;
		default: /* no valid val attribute found */
			return 0;
		}
	}

	if(!(name = ts_get_attr_str(tsnode, "name", 0)) || !*name) {
		return 0;
	}
	if(!(attr->name = malloc(strlen(name) + 1))) {
		goat3d_logmsg(LOG_ERROR, "read_material_attrib: failed to allocate name\n");
		return 0;
	}
	strcpy(attr->name, name);

	if((map = ts_get_attr_str(tsnode, "map", 0)) && *map) {
		if(!(attr->map = malloc(strlen(map) + 1))) {
			goat3d_logmsg(LOG_ERROR, "read_material_attrib: failed to allocate map name\n");
			free(attr->name);
			return 0;
		}
		strcpy(attr->map, map);
	}
	return attr->name;
}

struct goat3d_mesh *read_mesh(struct goat3d *g, struct ts_node *tsmesh)
{
	return 0;	/* TODO */
}
