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
	struct material *materials;
	union object *meshes;
	union object *lights;
	union object *cameras;
	struct anm_node *nodes;

	struct aabox bbox;
};

extern int goat3d_log_level;

int goat3d_init(struct goat3d *g);
void goat3d_destroy(struct goat3d *g);

	Scene();
	~Scene();

	void clear();

	void set_name(const char *name);
	const char *get_name() const;

	void set_ambient(const Vector3 &amb);
	const Vector3 &get_ambient() const;

	void add_material(Material *mat);
	Material *get_material(int idx) const;
	Material *get_material(const char *name) const;
	int get_material_count() const;

	void add_mesh(Mesh *mesh);
	Mesh *get_mesh(int idx) const;
	Mesh *get_mesh(const char *name) const;
	int get_mesh_count() const;

	void add_light(Light *light);
	Light *get_light(int idx) const;
	Light *get_light(const char *name) const;
	int get_light_count() const;

	void add_camera(Camera *cam);
	Camera *get_camera(int idx) const;
	Camera *get_camera(const char *name) const;
	int get_camera_count() const;

	void add_node(Node *node);
	Node *get_node(int idx) const;
	Node *get_node(const char *name) const;
	int get_node_count() const;

	const AABox &get_bounds() const;

	bool load(goat3d_io *io);
	bool save(goat3d_io *io) const;

	bool loadxml(goat3d_io *io);
	bool savexml(goat3d_io *io) const;

	bool load_anim(goat3d_io *io);
	bool save_anim(goat3d_io *io) const;

	bool load_anim_xml(goat3d_io *io);
	bool save_anim_xml(goat3d_io *io) const;
};

void io_fprintf(goat3d_io *io, const char *fmt, ...);
void io_vfprintf(goat3d_io *io, const char *fmt, va_list ap);

char *g3dimpl_clean_filename(char *str);

}	// namespace g3dimpl

#endif	// GOAT3D_IMPL_H_
