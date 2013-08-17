#ifndef OBJECT_H_
#define OBJECT_H_

#include "node.h"
#include "mesh.h"
#include "material.h"

class Object : public Node {
private:
	Material *material;
	Mesh *mesh;

public:
	Object();

	void set_mesh(Mesh *mesh);
	Mesh *get_mesh() const;

	void set_material(Material *mtl);
	Material *get_material() const;
};

#endif	// OBJECT_H_
