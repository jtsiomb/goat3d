#include "mesh.h"

Mesh::Mesh()
{
	material = 0;
}

Mesh::~Mesh() {}

void Mesh::set_material(Material *mat)
{
	material = mat;
}

Material *Mesh::get_material()
{
	return material;
}

const Material *Mesh::get_material() const
{
	return material;
}
