#include "goat3d.h"
#include "goat3d_impl.h"

Scene::Scene()
	: name("unnamed"), ambient(0.05, 0.05, 0.05)
{
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
		delete_node_tree(nodes[i]);
	}
	nodes.clear();

	name = "unnamed";
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

void Scene::add_mesh(Mesh *mesh)
{
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

bool Scene::load(goat3d_io *io)
{
	return false;
}

bool Scene::save(goat3d_io *io) const
{
	return false;
}
