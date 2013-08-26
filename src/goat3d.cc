#include "goat3d.h"
#include "goat3d_impl.h"
#include "chunk.h"

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

int Scene::get_material_count() const
{
	return (int)materials.size();
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


bool Scene::load(goat3d_io *io)
{
	return false;
}

static long save_env(const Scene *scn, long offset, goat3d_io *io);
static long save_materials(const Scene *scn, long offset, goat3d_io *io);
static long save_material(const Material *mat, long offset, goat3d_io *io);
static long save_mat_attrib(const char *name, const MaterialAttrib &attr, long offset, goat3d_io *io);
static long save_meshes(const Scene *scn, long offset, goat3d_io *io);
static long save_lights(const Scene *scn, long offset, goat3d_io *io);
static long save_cameras(const Scene *scn, long offset, goat3d_io *io);
static long save_nodes(const Scene *scn, long offset, goat3d_io *io);

static long write_chunk_float(int id, float val, long offs, goat3d_io *io);
static long write_chunk_float3(int id, const Vector3 &vec, long offs, goat3d_io *io);
static long write_chunk_float4(int id, const Vector4 &vec, long offs, goat3d_io *io);

bool Scene::save(goat3d_io *io) const
{
	long res;

	ChunkHeader hdr;
	hdr.id = CNK_SCENE;
	hdr.size = sizeof hdr;

	if((res = save_env(this, hdr.size, io)) < 0) {
		return false;
	}
	hdr.size += res;

	if((res = save_materials(this, hdr.size, io)) < 0) {
		return false;
	}
	hdr.size += res;

	if((res = save_meshes(this, hdr.size, io)) < 0) {
		return false;
	}
	hdr.size += res;

	if((res = save_lights(this, hdr.size, io)) < 0) {
		return false;
	}
	hdr.size += res;

	if((res = save_cameras(this, hdr.size, io)) < 0) {
		return false;
	}
	hdr.size += res;

	if((res = save_nodes(this, hdr.size, io)) < 0) {
		return false;
	}
	hdr.size += res;

	// now go back and write the root chunk
	io->seek(0, SEEK_SET, io->cls);
	if(io->write(&hdr, sizeof hdr, io->cls) < (ssize_t)sizeof hdr) {
		return false;
	}

	return true;
}


static long save_env(const Scene *scn, long offset, goat3d_io *io)
{
	long res;

	ChunkHeader hdr;
	hdr.id = CNK_ENV;
	hdr.size = sizeof hdr;

	if((res = write_chunk_float3(CNK_ENV_AMBIENT, scn->get_ambient(), offset, io)) < 0) {
		return -1;
	}
	hdr.size += res;

	// TODO add fog chunk

	io->seek(offset, SEEK_SET, io->cls);
	if(io->write(&hdr, sizeof hdr, io->cls) < (ssize_t)sizeof hdr) {
		return -1;
	}
	return hdr.size;
}

static long save_materials(const Scene *scn, long offset, goat3d_io *io)
{
	long res;

	ChunkHeader hdr;
	hdr.id = CNK_MTL_LIST;
	hdr.size = sizeof hdr;

	for(int i=0; i<scn->get_material_count(); i++) {
		if((res = save_material(scn->get_material(i), offset + hdr.size, io)) < 0) {
			return -1;
		}
		hdr.size += res;
	}

	io->seek(offset, SEEK_SET, io->cls);
	if(io->write(&hdr, hdr.size, io->cls) < hdr.size) {
		return -1;
	}
	return hdr.size;
}

static long save_material(const Material *mat, long offset, goat3d_io *io)
{
	long res;

	ChunkHeader hdr;
	hdr.id = CNK_MTL;
	hdr.size = sizeof hdr;

	for(int i=0; i<mat->get_attrib_count(); i++) {
		const char *name = mat->get_attrib_name(i);
		if((res = save_mat_attrib(name, (*mat)[i], offset + hdr.size, io)) < 0) {
			return -1;
		}
		hdr.size += res;
	}

	io->seek(offset, SEEK_SET, io->cls);
	if(io->write(&hdr, hdr.size, io->cls) < hdr.size) {
		return -1;
	}
	return hdr.size;
}

static long save_mat_attrib(const char *name, const MaterialAttrib &attr, long offset, goat3d_io *io)
{
	long res;

	ChunkHeader hdr;
	hdr.id = CNK_MTL_ATTR;
	hdr.size = sizeof hdr;

	// TODO cont.
	return -1;
}

static long save_meshes(const Scene *scn, long offset, goat3d_io *io)
{
	return 0;
}

static long save_lights(const Scene *scn, long offset, goat3d_io *io)
{
	return 0;
}

static long save_cameras(const Scene *scn, long offset, goat3d_io *io)
{
	return 0;
}

static long save_nodes(const Scene *scn, long offset, goat3d_io *io)
{
	return 0;
}

static long write_chunk_float(int id, float val, long offs, goat3d_io *io)
{
	int size = sizeof(ChunkHeader) + sizeof val;
	char *buf = (char*)alloca(size);

	Chunk *c = (Chunk*)buf;
	c->hdr.id = id;
	c->hdr.size = size;
	*(float*)c->data = val;

	io->seek(offs, SEEK_SET, io->cls);
	if(io->write(buf, size, io->cls) < size) {
		return -1;
	}
	return size;
}

static long write_chunk_float3(int id, const Vector3 &vec, long offs, goat3d_io *io)
{
	int size = sizeof(ChunkHeader) + sizeof vec;
	char *buf = (char*)alloca(size);

	Chunk *c = (Chunk*)buf;
	c->hdr.id = id;
	c->hdr.size = size;
	*(Vector3*)c->data = vec;

	io->seek(offs, SEEK_SET, io->cls);
	if(io->write(buf, size, io->cls) < size) {
		return -1;
	}
	return size;
}

static long write_chunk_float4(int id, const Vector4 &vec, long offs, goat3d_io *io)
{
	int size = sizeof(ChunkHeader) + sizeof vec;
	char *buf = (char*)alloca(size);

	Chunk *c = (Chunk*)buf;
	c->hdr.id = id;
	c->hdr.size = size;
	*(Vector4*)c->data = vec;

	io->seek(offs, SEEK_SET, io->cls);
	if(io->write(buf, size, io->cls) < size) {
		return -1;
	}
	return size;
}
