/*
goat3d - 3D scene, and animation file format library.
Copyright (C) 2013-2019  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef GOAT3D_IMPL_H_
#define GOAT3D_IMPL_H_

#include <cgmath/cgmath.h>
#include "goat3d.h"
#include "object.h"
#include "aabox.h"

struct goat3d {
	unsigned int flags;
	char *search_path;

	char *name;
	cgm_vec3 ambient;

	/* dynamic arrays */
	struct goat3d_material **materials;
	struct goat3d_mesh **meshes;
	struct goat3d_light **lights;
	struct goat3d_camera **cameras;
	struct goat3d_node **nodes;

	struct aabox bbox;
	int bbox_valid;
};

extern int goat3d_log_level;

int goat3d_init(struct goat3d *g);
void goat3d_destroy(struct goat3d *g);

void goat3d_clear(struct goat3d *g);

/*
void io_fprintf(goat3d_io *io, const char *fmt, ...);
void io_vfprintf(goat3d_io *io, const char *fmt, va_list ap);
*/

char *g3dimpl_clean_filename(char *str);

int g3dimpl_scnload(struct goat3d *g, struct goat3d_io *io);
int g3dimpl_anmload(struct goat3d *g, struct goat3d_io *io);

int g3dimpl_scnsave(const struct goat3d *g, struct goat3d_io *io);
int g3dimpl_anmsave(const struct goat3d *g, struct goat3d_io *io);

int g3dimpl_loadmesh(struct goat3d_mesh *mesh, const char *fname);

#endif	/* GOAT3D_IMPL_H_ */
