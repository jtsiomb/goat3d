#include "object.h"

Object::Object()
{
	material = 0;
	mesh = 0;
}

void Object::set_mesh(Mesh *mesh)
{
	this->mesh = mesh;
}

Mesh *Object::get_mesh() const
{
	return mesh;
}

void Object::set_material(Material *mtl)
{
	material = mtl;
}

Material *Object::get_material() const
{
	return material;
}
