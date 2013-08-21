#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include "object.h"
#include "material.h"

class Node;

struct Face {
	int v[3];
};

struct Int4 {
	int x, y, z, w;
};

class Mesh : public Object {
public:
	Material *material;

	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<Vector3> texcoords;
	std::vector<Vector4> skin_weights;
	std::vector<Int4> skin_matrices;
	std::vector<Vector4> colors;
	std::vector<Node*> bones;
	std::vector<Face> faces;

	Mesh();
	virtual ~Mesh();

	virtual void set_material(Material *mat);
	virtual Material *get_material();
	virtual const Material *get_material() const;
};

#endif	// MESH_H_
