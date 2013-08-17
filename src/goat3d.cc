#include "goat3d.h"
#include "goat3d_impl.h"

static void delete_node_tree(Node *n);

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

static void delete_node_tree(Node *n)
{
	for(int i=0; i<n->get_num_children(); i++) {
		delete_node_tree(n->get_child(i));
	}
	delete n;
}

void Scene::set_name(const char *name)
{
	this->name = name;
}

const char *Scene::get_name() const
{
	return name.c_str();
}
