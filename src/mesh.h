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
#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include "object.h"
#include "material.h"

namespace g3dimpl {

class Node;

struct Face {
	int v[3];
};

struct Int4 {
	int x, y, z, w;

	Int4();
	Int4(int x, int y, int z, int w);
};

class Mesh : public Object {
public:
	Material *material;

	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<Vector2> texcoords;
	std::vector<Vector4> skin_weights;
	std::vector<Int4> skin_matrices;
	std::vector<Vector4> colors;
	std::vector<Face> faces;

	std::vector<Node*> bones;

	Mesh();

	bool load(const char *fname);
	bool save(const char *fname) const;

	void set_material(Material *mat);
	Material *get_material();
	const Material *get_material() const;

	AABox get_bounds() const;
};

}	// namespace g3dimpl

#endif	// MESH_H_
