#ifndef GOAT3D_IMPL_H_
#define GOAT3D_IMPL_H_

#include <string>
#include <vmath/vmath.h>
#include "goat3d.h"
#include "mesh.h"
#include "light.h"
#include "camera.h"
#include "material.h"
#include "node.h"

extern int goat_log_level;

class Scene {
private:
	std::string name;
	Vector3 ambient;

	std::vector<Material*> materials;
	std::vector<Mesh*> meshes;
	std::vector<Light*> lights;
	std::vector<Camera*> cameras;
	std::vector<Node*> nodes;

public:
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

	bool load(goat3d_io *io);
	bool save(goat3d_io *io) const;

	bool loadxml(goat3d_io *io);
	bool savexml(goat3d_io *io) const;
};

void io_fprintf(goat3d_io *io, const char *fmt, ...);
void io_vfprintf(goat3d_io *io, const char *fmt, va_list ap);

#endif	// GOAT3D_IMPL_H_
