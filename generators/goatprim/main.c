#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "goat3d.h"

#define DEF_USUB	16
#define DEF_VSUB	8
#define DEF_SIZE	1.0
#define DEF_OUTER	0.25

enum { BOX, SPHERE, TORUS };

void gen_box(struct goat3d_mesh *mesh, float size);
void gen_sphere(struct goat3d_mesh *mesh, float rad, int usub, int vsub);
void gen_sphere_part(struct goat3d_mesh *mesh, float rad, int usub, int vsub, float umax, float vmax);
void gen_torus(struct goat3d_mesh *mesh, float inner, float outer, int usub, int vsub);
void gen_torus_part(struct goat3d_mesh *mesh, float inner, float outer,
		int usub, int vsub, float umax, float vmin, float vmax);


int main(int argc, char **argv)
{
	int i, prim = BOX;
	int usub = DEF_USUB;
	int vsub = DEF_VSUB;
	float size = DEF_SIZE;
	float outer = DEF_OUTER;
	struct goat3d *goat;
	struct goat3d_material *mtl;
	struct goat3d_mesh *mesh;
	const char *fname = 0;

	for(i=1; i<argc; i++) {
		if(strcmp(argv[i], "-box") == 0) {
			prim = BOX;

		} else if(strcmp(argv[i], "-sphere") == 0) {
			prim = SPHERE;

		} else if(strcmp(argv[i], "-torus") == 0) {
			prim = TORUS;

		} else if(strcmp(argv[i], "-rad") == 0 || strcmp(argv[i], "-size") == 0
				|| strcmp(argv[i], "-inner") == 0) {
			if((size = atof(argv[++i])) == 0.0) {
				fprintf(stderr, "invalid size\n");
				return 1;
			}

		} else if(strcmp(argv[i], "-outer") == 0) {
			if((outer = atof(argv[++i])) == 0.0) {
				fprintf(stderr, "invalid outer radius\n");
				return 1;
			}

		} else if(strcmp(argv[i], "-usub") == 0) {
			if(!(usub = atoi(argv[++i]))) {
				fprintf(stderr, "invalid usub\n");
				return 1;
			}

		} else if(strcmp(argv[i], "-vsub") == 0) {
			if(!(vsub = atoi(argv[++i]))) {
				fprintf(stderr, "invalid vsub\n");
				return 1;
			}

		} else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
			printf("Usage: %s [filename] [options]\n", argv[0]);
			printf("Options:\n");
			printf(" -box          create box (default)\n");
			printf(" -sphere       create sphere\n");
			printf(" -torus        create torus\n");
			printf(" -size <n>, -rad <n>, -inner <n> (default: %g)\n", DEF_SIZE);
			printf(" -outer <n>    torus outer radius (default: %g)\n", DEF_OUTER);
			printf(" -usub <n>     subdivisions along the horizontal direction (default: %d)\n", DEF_USUB);
			printf(" -vsub <n>     subdivisions along the vertical direction (default: %d)\n", DEF_VSUB);
			printf(" -h, -help     print usage information and exit\n");
			return 0;

		} else {
			if(fname) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				return 1;
			}
			fname = argv[i];
		}
	}

	if(!fname) {
		fname = "out.xml";
	}

	goat = goat3d_create();
	goat3d_set_name(goat, fname);

	mtl = goat3d_create_mtl();
	goat3d_set_mtl_name(mtl, "mat");
	goat3d_set_mtl_attrib4f(mtl, GOAT3D_MAT_ATTR_DIFFUSE, 1, 0, 0, 1);
	goat3d_add_mtl(goat, mtl);

	mesh = goat3d_create_mesh();

	switch(prim) {
	case BOX:
		gen_box(mesh, size);
		break;

	case SPHERE:
		gen_sphere(mesh, size, usub, vsub);
		break;

	case TORUS:
		gen_torus(mesh, size, outer, usub, vsub);
		break;

	default:
		return 1;
	}
	goat3d_set_mesh_mtl(mesh, mtl);
	goat3d_add_mesh(goat, mesh);

	goat3d_setopt(goat, GOAT3D_OPT_SAVEXML, 1);
	goat3d_save(goat, fname);

	goat3d_free(goat);
	return 0;
}

void gen_box(struct goat3d_mesh *mesh, float size)
{
	float hsz = size / 2.0;

	goat3d_begin(mesh, GOAT3D_QUADS);
	// +X
	goat3d_normal3f(1, 0, 0);
	goat3d_texcoord2f(0, 0);
	goat3d_vertex3f(hsz, -hsz, hsz);
	goat3d_texcoord2f(1, 0);
	goat3d_vertex3f(hsz, -hsz, -hsz);
	goat3d_texcoord2f(1, 1);
	goat3d_vertex3f(hsz, hsz, -hsz);
	goat3d_texcoord2f(0, 1);
	goat3d_vertex3f(hsz, hsz, hsz);

	// -X
	goat3d_normal3f(-1, 0, 0);
	goat3d_texcoord2f(0, 0);
	goat3d_vertex3f(-hsz, -hsz, -hsz);
	goat3d_texcoord2f(1, 0);
	goat3d_vertex3f(-hsz, -hsz, hsz);
	goat3d_texcoord2f(1, 1);
	goat3d_vertex3f(-hsz, hsz, hsz);
	goat3d_texcoord2f(0, 1);
	goat3d_vertex3f(-hsz, hsz, -hsz);

	// +Y
	goat3d_normal3f(0, 1, 0);
	goat3d_texcoord2f(0, 0);
	goat3d_vertex3f(-hsz, hsz, hsz);
	goat3d_texcoord2f(1, 0);
	goat3d_vertex3f(hsz, hsz, hsz);
	goat3d_texcoord2f(1, 1);
	goat3d_vertex3f(hsz, hsz, -hsz);
	goat3d_texcoord2f(0, 1);
	goat3d_vertex3f(-hsz, hsz, -hsz);

	// -Y
	goat3d_normal3f(0, -1, 0);
	goat3d_texcoord2f(0, 0);
	goat3d_vertex3f(-hsz, -hsz, -hsz);
	goat3d_texcoord2f(1, 0);
	goat3d_vertex3f(hsz, -hsz, -hsz);
	goat3d_texcoord2f(1, 1);
	goat3d_vertex3f(hsz, -hsz, hsz);
	goat3d_texcoord2f(0, 1);
	goat3d_vertex3f(-hsz, -hsz, hsz);

	// +Z
	goat3d_normal3f(0, 0, 1);
	goat3d_texcoord2f(0, 0);
	goat3d_vertex3f(-hsz, -hsz, hsz);
	goat3d_texcoord2f(1, 0);
	goat3d_vertex3f(hsz, -hsz, hsz);
	goat3d_texcoord2f(1, 1);
	goat3d_vertex3f(hsz, hsz, hsz);
	goat3d_texcoord2f(0, 1);
	goat3d_vertex3f(-hsz, hsz, hsz);

	// -Z
	goat3d_normal3f(0, 0, -1);
	goat3d_texcoord2f(0, 0);
	goat3d_vertex3f(hsz, -hsz, -hsz);
	goat3d_texcoord2f(1, 0);
	goat3d_vertex3f(-hsz, -hsz, -hsz);
	goat3d_texcoord2f(1, 1);
	goat3d_vertex3f(-hsz, hsz, -hsz);
	goat3d_texcoord2f(0, 1);
	goat3d_vertex3f(hsz, hsz, -hsz);
	goat3d_end();
}

void gen_sphere(struct goat3d_mesh *mesh, float rad, int usub, int vsub)
{
	gen_sphere_part(mesh, rad, usub, vsub, 1.0, 1.0);
}

#define sphere_vertex(u, v) \
	do { \
		float x, y, z, theta, phi; \
		float costheta, sinphi; \
		theta = (u) * 2.0 * M_PI; \
		phi = (v) * M_PI; \
		costheta = cos(theta); \
		sinphi = sin(phi); \
		x = costheta * sinphi; \
		y = cos(phi); \
		z = sin(theta) * sinphi; \
		goat3d_normal3f(x, y, z); \
		goat3d_texcoord2f(u, v); \
		goat3d_vertex3f(rad * x, rad * y, rad * z); \
	} while(0)

void gen_sphere_part(struct goat3d_mesh *mesh, float rad, int usub, int vsub, float umax, float vmax)
{
	int i, j;
	float u, v, du, dv;

	if(usub < 3) usub = 3;
	if(vsub < 3) vsub = 3;

	du = umax / (float)usub;
	dv = vmax / (float)vsub;

	goat3d_begin(mesh, GOAT3D_QUADS);

	u = 0.0;
	for(i=0; i<usub; i++) {
		v = 0.0;
		for(j=0; j<vsub; j++) {
			sphere_vertex(u, v);
			sphere_vertex(u + du, v);
			sphere_vertex(u + du, v + dv);
			sphere_vertex(u, v + dv);
			v += dv;
		}
		u += du;
	}
	goat3d_end();
}


void gen_torus(struct goat3d_mesh *mesh, float inner, float outer, int usub, int vsub)
{
	gen_torus_part(mesh, inner, outer, usub, vsub, 1.0, 0.0, 1.0);
}

#define torus_vertex(u, v) \
	do { \
		float rx, ry, rz, cx, cy, cz, theta, phi; \
		float costheta, sintheta, sinphi; \
		theta = (u) * 2.0 * M_PI; \
		phi = (v) * 2.0 * M_PI; \
		costheta = cos(theta); \
		sintheta = sin(theta); \
		sinphi = sin(phi); \
		cx = costheta * inner; \
		cy = 0.0f; \
		cz = sintheta * inner; \
		rx = costheta * sinphi; \
		ry = cos(phi); \
		rz = sintheta * sinphi; \
		goat3d_normal3f(rx, ry, rz); \
		goat3d_texcoord2f(u, v); \
		goat3d_vertex3f(outer * rx + cx, outer * ry + cy, outer * rz + cz); \
	} while(0)

void gen_torus_part(struct goat3d_mesh *mesh, float inner, float outer,
		int usub, int vsub, float umax, float vmin, float vmax)
{
	int i, j;
	float u, v, du, dv;

	if(usub < 3) usub = 3;
	if(vsub < 3) vsub = 3;

	du = umax / (float)usub;
	dv = (vmax - vmin) / (float)vsub;

	goat3d_begin(mesh, GOAT3D_QUADS);

	u = 0.0;
	for(i=0; i<usub; i++) {
		v = vmin;
		for(j=0; j<vsub; j++) {
			torus_vertex(u, v);
			torus_vertex(u + du, v);
			torus_vertex(u + du, v + dv);
			torus_vertex(u, v + dv);
			v += dv;
		}
		u += du;
	}
	goat3d_end();
}
