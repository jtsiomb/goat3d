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
#include <stdarg.h>
#include "goat3d.h"
#include "goat3d_impl.h"
#include "chunk.h"

using namespace g3dimpl;

Scene::Scene()
	: name("unnamed"), ambient(0.05, 0.05, 0.05)
{
	goat = 0;
	bbox_valid = false;
}

Scene::~Scene()
{
	clear();
}

void Scene::clear()
{
	for(size_t i=0; i<materials.size(); i++) {
		delete materials[i];
	}
	materials.clear();

	for(size_t i=0; i<meshes.size(); i++) {
		delete meshes[i];
	}
	meshes.clear();

	for(size_t i=0; i<lights.size(); i++) {
		delete lights[i];
	}
	lights.clear();

	for(size_t i=0; i<cameras.size(); i++) {
		delete cameras[i];
	}
	cameras.clear();

	for(size_t i=0; i<nodes.size(); i++) {
		delete nodes[i];
	}
	nodes.clear();

	name = "unnamed";
	bbox_valid = false;
}

void Scene::set_name(const char *name)
{
	this->name = name;
}

const char *Scene::get_name() const
{
	return name.c_str();
}

void Scene::set_ambient(const Vector3 &amb)
{
	ambient = amb;
}

const Vector3 &Scene::get_ambient() const
{
	return ambient;
}

void Scene::add_material(Material *mat)
{
	if(mat->name.empty()) {
		char buf[64];
		sprintf(buf, "material%04d", (int)materials.size());
		mat->name = std::string(buf);
	}
	materials.push_back(mat);
}

Material *Scene::get_material(int idx) const
{
	return idx >=0 && idx < (int)materials.size() ? materials[idx] : 0;
}

Material *Scene::get_material(const char *name) const
{
	for(size_t i=0; i<materials.size(); i++) {
		if(materials[i]->name == std::string(name)) {
			return materials[i];
		}
	}
	return 0;
}

int Scene::get_material_count() const
{
	return (int)materials.size();
}


void Scene::add_mesh(Mesh *mesh)
{
	if(mesh->name.empty()) {
		char buf[64];
		sprintf(buf, "mesh%04d", (int)meshes.size());
		mesh->name = std::string(buf);
	}
	meshes.push_back(mesh);
}

Mesh *Scene::get_mesh(int idx) const
{
	return idx >= 0 && idx < (int)meshes.size() ? meshes[idx] : 0;
}

Mesh *Scene::get_mesh(const char *name) const
{
	for(size_t i=0; i<meshes.size(); i++) {
		if(meshes[i]->name == std::string(name)) {
			return meshes[i];
		}
	}
	return 0;
}

int Scene::get_mesh_count() const
{
	return (int)meshes.size();
}


void Scene::add_light(Light *light)
{
	lights.push_back(light);
}

Light *Scene::get_light(int idx) const
{
	return idx >= 0 && idx < (int)lights.size() ? lights[idx] : 0;
}

Light *Scene::get_light(const char *name) const
{
	for(size_t i=0; i<lights.size(); i++) {
		if(lights[i]->name == std::string(name)) {
			return lights[i];
		}
	}
	return 0;
}

int Scene::get_light_count() const
{
	return (int)lights.size();
}


void Scene::add_camera(Camera *cam)
{
	cameras.push_back(cam);
}

Camera *Scene::get_camera(int idx) const
{
	return idx >= 0 && idx < (int)cameras.size() ? cameras[idx] : 0;
}

Camera *Scene::get_camera(const char *name) const
{
	for(size_t i=0; i<cameras.size(); i++) {
		if(cameras[i]->name == std::string(name)) {
			return cameras[i];
		}
	}
	return 0;
}

int Scene::get_camera_count() const
{
	return (int)cameras.size();
}


void Scene::add_node(Node *node)
{
	nodes.push_back(node);
}

Node *Scene::get_node(int idx) const
{
	return idx >= 0 && idx < (int)nodes.size() ? nodes[idx] : 0;
}

Node *Scene::get_node(const char *name) const
{
	for(size_t i=0; i<nodes.size(); i++) {
		if(strcmp(nodes[i]->get_name(), name) == 0) {
			return nodes[i];
		}
	}
	return 0;
}

int Scene::get_node_count() const
{
	return (int)nodes.size();
}

const AABox &Scene::get_bounds() const
{
	if(!bbox_valid) {
		bbox = AABox();

		for(size_t i=0; i<nodes.size(); i++) {
			if(nodes[i]->get_parent()) {
				continue;
			}

			bbox = aabox_union(bbox, nodes[i]->get_bounds());
		}
		bbox_valid = true;
	}

	return bbox;
}

// Scene::load is defined in goat3d_read.cc
// Scene::loadxml is defined in goat3d_readxml.cc
// Scene::save is defined in goat3d_write.cc
// Scene::savexml is defined in goat3d_writexml.cc


void g3dimpl::io_fprintf(goat3d_io *io, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	io_vfprintf(io, fmt, ap);
	va_end(ap);
}


void g3dimpl::io_vfprintf(goat3d_io *io, const char *fmt, va_list ap)
{
	char smallbuf[256];
	char *buf = smallbuf;
	int sz = sizeof smallbuf;

	int retsz = vsnprintf(buf, sz - 1, fmt, ap);

	if(retsz >= sz) {
		/* C99 mandates that snprintf with a short count should return the
		 * number of characters that *would* be printed.
		 */
		buf = new char[retsz + 1];

		vsnprintf(buf, retsz, fmt, ap);

	} else if(retsz <= 0) {
		/* SUSv2 and microsoft specify that snprintf with a short count
		 * returns an arbitrary value <= 0. So let's try allocating
		 * bigger and bigger arrays until we find the correct size.
		 */
		sz = sizeof smallbuf;
		do {
			sz *= 2;
			if(buf != smallbuf) {
				delete [] buf;
			}
			buf = new char[sz + 1];

			retsz = vsnprintf(buf, sz, fmt, ap);
		} while(retsz <= 0);
	}

	io->write(buf, retsz, io->cls);

	if(buf != smallbuf) {
		delete [] buf;
	}

}
