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
#include <stdlib.h>
#include <string.h>
#include <treestore.h>
#include "goat3d_impl.h"
#include "log.h"
#include "dynarr.h"

#define create_tsnode(n, p, nstr) \
	do { \
		int len = strlen(nstr); \
		if(!((n) = ts_alloc_node())) { \
			goat3d_logmsg(LOG_ERROR, "%s: failed to create treestore node\n", __func__); \
			goto err; \
		} \
		if(!((n)->name = malloc(len + 1))) { \
			goat3d_logmsg(LOG_ERROR, "%s: failed to allocate node name string\n", __func__); \
			ts_free_node(n); \
			goto err; \
		} \
		memcpy((n)->name, (nstr), len + 1); \
		if(p) { \
			ts_add_child((p), (n)); \
		} \
	} while(0)

#define create_tsattr(a, n, nstr, atype) \
	do { \
		if(!((a) = ts_alloc_attr())) { \
			goat3d_logmsg(LOG_ERROR, "%s: failed to create treestore attribute\n", __func__); \
			goto err; \
		} \
		if(ts_set_attr_name(a, nstr) == -1) { \
			goat3d_logmsg(LOG_ERROR, "%s: failed to allocate attrib name string\n", __func__); \
			ts_free_attr(a); \
			goto err; \
		} \
		(a)->val.type = (atype); \
		if(n) { \
			ts_add_attr((n), (a)); \
		} \
	} while(0)



int g3dimpl_scnsave(const struct goat3d *g, struct goat3d_io *io)
{
	int i, j, num;
	struct ts_io tsio;
	struct ts_node *tsroot = 0, *tsenv, *tsmtl, *tsmesh, *tslight, *tscam, *tsnode;
	struct ts_node *tsn;
	struct ts_attr *tsa;

	tsio.data = io->cls;
	tsio.read = io->read;
	tsio.write = io->write;

	create_tsnode(tsroot, 0, "scene");

	/* environment */
	create_tsnode(tsenv, tsroot, "env");
	create_tsattr(tsa, tsenv, "ambient", TS_VECTOR);
	ts_set_valuefv(&tsa->val, 3, g->ambient.x, g->ambient.y, g->ambient.z);
	/* TODO: fog */

	num = dynarr_size(g->materials);
	for(i=0; i<num; i++) {
		int num_attr;

		create_tsnode(tsmtl, tsroot, "mtl");
		create_tsattr(tsa, tsmtl, "name", TS_STRING);
		if(ts_set_value_str(&tsa->val, g->materials[i]->name) == -1) {
			goto err;
		}

		num_attr = dynarr_size(g->materials[i]->attrib);
		for(j=0; j<num_attr; j++) {
			struct material_attrib *attr = g->materials[i]->attrib + j;

			create_tsnode(tsn, tsmtl, "attr");
			create_tsattr(tsa, tsn, "name", TS_STRING);
			if(ts_set_value_str(&tsa->val, attr->name) == -1) {
				goto err;
			}
			create_tsattr(tsa, tsn, "val", TS_VECTOR);
			ts_set_valuefv(&tsa->val, 4, attr->value.x, attr->value.y, attr->value.z, attr->value.w);
			if(attr->map) {
				create_tsattr(tsa, tsn, "map", TS_STRING);
				if(ts_set_value_str(&tsa->val, attr->map) == -1) {
					goto err;
				}
			}
		}
	}

	if(ts_save_io(tsroot, &tsio) == -1) {
		goat3d_logmsg(LOG_ERROR, "g3dimpl_scnsave: failed\n");
		goto err;
	}
	return 0;

err:
	ts_free_tree(tsroot);
	return -1;
}

int g3dimpl_anmsave(const struct goat3d *g, struct goat3d_io *io)
{
	return -1;
}
