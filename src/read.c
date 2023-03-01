/*
goat3d - 3D scene, and animation file format library.
Copyright (C) 2013-2023  John Tsiombikas <nuclear@member.fsf.org>

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
static struct goat3d_mesh *read_mesh(struct goat3d *g, struct ts_node *tsmesh);

static int read_veclist(void *arr, int dim, const char *nodename, const char *attrname, struct ts_node *tsnode);
static int read_intlist(void *arr, int dim, const char *nodename, const char *attrname, struct ts_node *tsnode);
static int read_bonelist(struct goat3d *g, struct goat3d_node **arr, struct ts_node *tsnode);

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
		free(mtl);
		goat3d_logmsg(LOG_ERROR, "read_material: failed to allocate material\n");
		return 0;
	}

	if(!(str = ts_get_attr_str(tsmtl, "name", 0)) || !*str) {
		/* XXX wait, we can refer to materials by index, why is the name important? */
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

static struct goat3d_mesh *read_mesh(struct goat3d *g, struct ts_node *tsmesh)
{
	struct goat3d_mesh *mesh;
	struct goat3d_material *mtl;
	struct ts_node *c;
	const char *str;
	int num;

	if(!(mesh = malloc(sizeof *mesh)) || g3dimpl_obj_init((struct object*)mesh, OBJTYPE_MESH)) {
		goat3d_logmsg(LOG_ERROR, "read_mesh: failed to allocate mesh\n");
		goto fail;
	}

	if((str = ts_get_attr_str(tsmesh, "name", 0))) {
		goat3d_set_mesh_name(mesh, str);
	}

	/* material reference */
	if((num = ts_get_attr_num(tsmesh, "material", -1)) >= 0) {
		if((mtl = goat3d_get_mtl(g, num))) {
			goat3d_set_mesh_mtl(mesh, mtl);
		} else {
			goat3d_logmsg(LOG_WARNING, "read_mesh: mesh %s refers to invalid material: %d\n",
					mesh->name, num);
		}
	} else if((str = ts_get_attr_str(tsmesh, "material", 0))) {
		if((mtl = goat3d_get_mtl_by_name(g, str))) {
			goat3d_set_mesh_mtl(mesh, mtl);
		} else {
			goat3d_logmsg(LOG_WARNING, "read_mesh: mesh %s refers to invalid material: %s\n",
					mesh->name, str);
		}
	}

	/* external mesh data */
	if((str = ts_get_attr_str(tsmesh, "file", 0))) {
		const char *fname = str;
		char *pathbuf;

		if(g->search_path) {
			pathbuf = alloca(strlen(str) + strlen(g->search_path) + 2);
			sprintf(pathbuf, "%s/%s", g->search_path, str);
			fname = pathbuf;
		}

		if(g3dimpl_loadmesh(mesh, fname) == -1) {
			goat3d_logmsg(LOG_ERROR, "read_mesh: failed to load external mesh: %s\n", fname);
			goto fail;
		}

		/* done loading, we can't have both an external mesh AND internal data,
		 * if there's anything else hanging under this tsnode, ignore it.
		 */
		if(tsmesh->child_list) {
			goat3d_logmsg(LOG_WARNING, "read_mesh: external mesh node also has unexpected children; ignoring.\n");
		}
		return mesh;
	}

	c = tsmesh->child_list;
	while(c) {
		if(strcmp(c->name, "vertex_list") == 0) {
			if(read_veclist(mesh->vertices, 3, "vertex", "pos", c) == -1) {
				goat3d_logmsg(LOG_ERROR, "read_mesh: failed to read vertex array for mesh %s\n",
						mesh->name);
				goto fail;
			}

		} else if(strcmp(c->name, "normal_list") == 0) {
			if(read_veclist(mesh->normals, 3, "normal", "dir", c) == -1) {
				goat3d_logmsg(LOG_WARNING, "read_mesh: failed to read normals array for mesh %s\n",
						mesh->name);
			}

		} else if(strcmp(c->name, "tangent_list") == 0) {
			if(read_veclist(mesh->tangents, 3, "tangent", "dir", c) == -1) {
				goat3d_logmsg(LOG_WARNING, "read_mesh: failed to read tangents array for mesh %s\n",
						mesh->name);
			}

		} else if(strcmp(c->name, "texcoord_list") == 0) {
			if(read_veclist(mesh->texcoords, 2, "texcoord", "uv", c) == -1) {
				goat3d_logmsg(LOG_WARNING, "read_mesh: failed to read texcoord array for mesh %s\n",
						mesh->name);
			}

		} else if(strcmp(c->name, "skinweight_list") == 0) {
			if(read_veclist(mesh->skin_weights, 4, "skinweight", "weights", c) == -1) {
				goat3d_logmsg(LOG_WARNING, "read_mesh: failed to read skin weights array for mesh %s\n",
						mesh->name);
			}

		} else if(strcmp(c->name, "skinmatrix_list") == 0) {
			if(read_intlist(mesh->skin_matrices, 4, "skinmatrix", "idx", c) == -1) {
				goat3d_logmsg(LOG_WARNING, "read_mesh: failed to read skin matrix index array for mesh %s\n",
						mesh->name);
			}

		} else if(strcmp(c->name, "color_list") == 0) {
			if(read_veclist(mesh->colors, 4, "color", "color", c) == -1) {
				goat3d_logmsg(LOG_WARNING, "read_mesh: failed to read color array for mesh %s\n",
						mesh->name);
			}

		} else if(strcmp(c->name, "bone_list") == 0) {
			if(read_bonelist(g, mesh->bones, c) == -1) {
				goat3d_logmsg(LOG_WARNING, "read_mesh: failed to read bones array for mesh %s\n",
						mesh->name);
			}

		} else if(strcmp(c->name, "face_list") == 0) {
			if(read_intlist(mesh->faces, 3, "face", "idx", c) == -1) {
				goat3d_logmsg(LOG_ERROR, "read_mesh: failed to read faces array for mesh %s\n",
						mesh->name);
				goto fail;
			}
		}
		c = c->next;
	}
	return mesh;

fail:
	g3dimpl_obj_destroy((struct object*)mesh);
	return 0;
}


static int read_veclist(void *arr, int dim, const char *nodename, const char *attrname, struct ts_node *tslist)
{
	int i, size;
	struct ts_node *c;
	struct ts_attr *attr;
	float vec[4];

	dynarr_clear(arr);

	if((size = ts_get_attr_int(tslist, "list_size", -1)) <= 0) {
		goat3d_logmsg(LOG_WARNING, "read_veclist: list_size attribute missing or invalid\n");
		size = -1;
	}

	c = tslist->child_list;
	while(c) {
		if(strcmp(c->name, nodename) != 0) {
			c = c->next;
			continue;
		}

		if((attr = ts_get_attr(c, attrname)) && attr->val.type == TS_VECTOR) {
			for(i=0; i<dim; i++) {
				if(i >= attr->val.vec_size) {
					vec[i] = 0;
				} else {
					vec[i] = attr->val.vec[i];
				}
			}

			if(!(arr = dynarr_push(arr, vec))) {
				goat3d_logmsg(LOG_ERROR, "read_veclist: failed to resize %s array\n",
						nodename);
				return -1;
			}
		}
		c = c->next;
	}

	if(size > 0 && dynarr_size(arr) != size) {
		goat3d_logmsg(LOG_WARNING, "read_veclist: expected %d items, read %d\n", size, dynarr_size(arr));
	}
	return 0;
}

static int read_intlist(void *arr, int dim, const char *nodename, const char *attrname, struct ts_node *tslist)
{
	int i, size;
	struct ts_node *c;
	struct ts_attr *attr;
	int ivec[4];

	dynarr_clear(arr);

	if((size = ts_get_attr_int(tslist, "list_size", -1)) <= 0) {
		goat3d_logmsg(LOG_WARNING, "read_intlist: list_size attribute missing or invalid\n");
		size = -1;
	}

	c = tslist->child_list;
	while(c) {
		if(strcmp(c->name, nodename) != 0) {
			c = c->next;
			continue;
		}

		if((attr = ts_get_attr(c, attrname)) && attr->val.type == TS_VECTOR) {
			for(i=0; i<dim; i++) {
				if(i >= attr->val.vec_size) {
					ivec[i] = 0;
				} else {
					ivec[i] = attr->val.vec[i];
				}
			}

			if(!(arr = dynarr_push(arr, ivec))) {
				goat3d_logmsg(LOG_ERROR, "read_intlist: failed to resize %s array\n",
						nodename);
				return -1;
			}
		}
		c = c->next;
	}

	if(size > 0 && dynarr_size(arr) != size) {
		goat3d_logmsg(LOG_WARNING, "read_intlist: expected %d items, read %d\n", size, dynarr_size(arr));
	}
	return 0;
}

static int read_bonelist(struct goat3d *g, struct goat3d_node **arr, struct ts_node *tslist)
{
	int size, idx;
	struct ts_node *c;
	struct goat3d_node *bone;
	const char *str;

	dynarr_clear(arr);

	if((size = ts_get_attr_int(tslist, "list_size", -1)) <= 0) {
		goat3d_logmsg(LOG_WARNING, "read_bonelist: list_size attribute missing or invalid\n");
		size = -1;
	}

	c = tslist->child_list;
	while(c) {
		if(strcmp(c->name, "bone") != 0) {
			c = c->next;
			continue;
		}

		bone = 0;

		if((idx = ts_get_attr_int(c, "bone", -1)) >= 0) {
			if(!(bone = goat3d_get_node(g, idx))) {
				goat3d_logmsg(LOG_ERROR, "read_bonelist: reference to invalid bone: %d\n", idx);
				return -1;
			}

		} else if((str = ts_get_attr_str(c, "bone", 0))) {
			if(!(bone = goat3d_get_node_by_name(g, str))) {
				goat3d_logmsg(LOG_ERROR, "read_bonelist: reference to invalid bone: %s\n", str);
				return -1;
			}
		}

		if(bone && !(arr = dynarr_push(arr, &bone))) {
			goat3d_logmsg(LOG_ERROR, "read_bonelist: failed to resize bone array\n");
			return -1;
		}
		c = c->next;
	}

	if(size > 0 && dynarr_size(arr) != size) {
		goat3d_logmsg(LOG_WARNING, "read_bonelist: expected %d items, read %d\n", size, dynarr_size(arr));
	}
	return 0;
}
