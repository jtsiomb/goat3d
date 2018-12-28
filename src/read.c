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
#include <treestore.h>
#include "goat3d.h"
#include "goat3d_impl.h"
#include "log.h"

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
		logmsg(LOG_ERROR, "failed to load scene\n");
		return -1;
	}
	if(strcmp(tsroot->name, "scene") != 0) {
		logmsg(LOG_ERROR, "invalid scene file, root node is not \"scene\"\n");
		ts_free_tree(tsroot);
		return -1;
	}

	/* read all materials */
	c = tsroot->children;
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
	c = tsroot->children;
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
	struct ts_io tsio;
	tsio.data = io->cls;
	tsio.read = io->read;
	tsio.write = io->write;
	return -1;
}

static struct goat3d_material *read_material(struct goat3d *g, struct ts_node *tsmtl)
{
}

static char *read_material_attrib(struct material_attrib *attr, struct ts_node *tsnode)
{
	struct ts_node *child;
	struct ts_attr *tsattr;
	char *str;

	if(!(ts_get_attr_str(tsnode, "name"))) {
		return 0;
	}

}

struct goat3d_mesh *read_mesh(struct goat3d *g, struct ts_node *tsmesh)
{
}
