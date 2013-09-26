#include <stdio.h>
#include "goat3d.h"

static struct goat3d_mesh *create_box(void);

int main(void)
{
	struct goat3d *goat;
	struct goat3d_material *mtl;
	struct goat3d_mesh *mesh;

	goat = goat3d_create();

	mtl = goat3d_create_mtl();
	goat3d_set_mtl_name(mtl, "mat");
	goat3d_set_mtl_attrib4f(mtl, GOAT3D_MAT_ATTR_DIFFUSE, 1, 0, 0, 1);
	goat3d_add_mtl(goat, mtl);

	mesh = create_box();
	goat3d_add_mesh(goat, mesh);

	goat3d_setopt(goat, GOAT3D_OPT_SAVEXML, 1);
	goat3d_save(goat, "foo.xml");

	goat3d_free(goat);
	return 0;
}

static struct goat3d_mesh *create_box(void)
{
	struct goat3d_mesh *mesh = goat3d_create_mesh();

	goat3d_begin(mesh, GOAT3D_QUADS);
	// +X
	goat3d_normal3f(1, 0, 0);
	goat3d_vertex3f(-1, -1, 1);
	goat3d_vertex3f(-1, -1, -1);
	goat3d_vertex3f(-1, 1, -1);
	goat3d_vertex3f(-1, 1, 1);

	// -X
	goat3d_normal3f(-1, 0, 0);
	goat3d_vertex3f(-1, -1, -1);
	goat3d_vertex3f(-1, -1, 1);
	goat3d_vertex3f(-1, 1, 1);
	goat3d_vertex3f(-1, 1, -1);

	// +Y
	goat3d_normal3f(0, 1, 0);
	goat3d_vertex3f(-1, 1, 1);
	goat3d_vertex3f(1, 1, 1);
	goat3d_vertex3f(1, 1, -1);
	goat3d_vertex3f(-1, 1, -1);

	// -Y
	goat3d_normal3f(0, -1, 0);
	goat3d_vertex3f(-1, -1, -1);
	goat3d_vertex3f(1, -1, -1);
	goat3d_vertex3f(1, -1, 1);
	goat3d_vertex3f(-1, -1, 1);

	// +Z
	goat3d_normal3f(0, 0, 1);
	goat3d_vertex3f(-1, -1, 1);
	goat3d_vertex3f(1, -1, 1);
	goat3d_vertex3f(1, -1, 1);
	goat3d_vertex3f(-1, -1, 1);

	// -Z
	goat3d_normal3f(0, 0, -1);
	goat3d_vertex3f(-1, -1, 1);
	goat3d_vertex3f(1, -1, 1);
	goat3d_vertex3f(1, -1, -1);
	goat3d_vertex3f(-1, -1, -1);
	goat3d_end();

	return mesh;
}
