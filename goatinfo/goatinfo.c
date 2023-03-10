#include <stdio.h>
#include <string.h>
#include "goat3d.h"

enum {
	INFO_MTL	= 0x01,
	INFO_MESH	= 0x02,
	INFO_LIGHT	= 0x04,
	INFO_CAM	= 0x08,
	INFO_NODE	= 0x10,
	INFO_ANIM	= 0x20
};

void print_overview(struct goat3d *g);
void print_mtl(struct goat3d *g);
void print_mesh(struct goat3d *g);
void print_light(struct goat3d *g);
void print_cam(struct goat3d *g);
void print_node(struct goat3d *g);
void print_anim(struct goat3d *g);

static const char *usagestr = "Usage: %s [options] <filename>\n"
	"Options:\n"
	"  -mtl: print details about materials in the file\n"
	"  -mesh: print details about meshes in the file\n"
	"  -light: print details about lights in the file\n"
	"  -cam: print details about cameras in the file\n"
	"  -node: print details about the nodes in the file\n"
	"  -anim: print details about animations in the file\n"
	"  -h,-help: print usage help and exit\n";

int main(int argc, char **argv)
{
	int i;
	unsigned int mode = 0;
	const char *fname = 0;
	struct goat3d *g;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-mtl") == 0) {
				mode |= INFO_MTL;
			} else if(strcmp(argv[i], "-mesh") == 0) {
				mode |= INFO_MESH;
			} else if(strcmp(argv[i], "-light") == 0) {
				mode |= INFO_LIGHT;
			} else if(strcmp(argv[i], "-cam") == 0) {
				mode |= INFO_CAM;
			} else if(strcmp(argv[i], "-node") == 0) {
				mode |= INFO_NODE;
			} else if(strcmp(argv[i], "-anim") == 0) {
				mode |= INFO_ANIM;
			} else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help")) {
				printf(usagestr, argv[0]);
				return 0;
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return 1;
			}
		} else {
			if(fname) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				return 1;
			}
			fname = argv[i];
		}
	}

	if(!fname) {
		fprintf(stderr, "pass the filename of a goat3d scene file\n");
		return 1;
	}

	if(!(g = goat3d_create()) || goat3d_load(g, fname) == -1) {
		return 1;
	}

	print_overview(g);

	if(mode & INFO_MTL) {
		print_mtl(g);
	}
	if(mode & INFO_MESH) {
		print_mesh(g);
	}
	if(mode & INFO_LIGHT) {
		print_light(g);
	}
	if(mode & INFO_CAM) {
		print_cam(g);
	}
	if(mode & INFO_NODE) {
		print_node(g);
	}
	if(mode & INFO_ANIM) {
		print_anim(g);
	}

	goat3d_free(g);
	return 0;
}


void print_overview(struct goat3d *g)
{
	float bmin[3], bmax[3];
	int nmtl, nmesh, nlt, ncam, nnode, nanim;

	printf("Scene: \"%s\"\n", goat3d_get_name(g));

	goat3d_get_bounds(g, bmin, bmax);
	printf("Bounds: %f %f %f - %f %f %f\n", bmin[0], bmin[1], bmin[2], bmax[0],
			bmax[1], bmax[2]);

	nmtl = goat3d_get_mtl_count(g);
	nmesh = goat3d_get_mesh_count(g);
	nlt = goat3d_get_light_count(g);
	ncam = goat3d_get_camera_count(g);
	nnode = goat3d_get_node_count(g);
	nanim = goat3d_get_anim_count(g);

	printf("Contains:\n");
	if(nmtl) {
		printf(" - %d materials\n", nmtl);
	}
	if(nmesh) {
		printf(" - %d meshes\n", nmesh);
	}
	if(nlt) {
		printf(" - %d lights\n", nlt);
	}
	if(ncam) {
		printf(" - %d cameras\n", ncam);
	}
	if(nnode) {
		printf(" - %d nodes\n", nnode);
	}
	if(nanim) {
		printf(" - %d animations\n", nanim);
	}
	putchar('\n');
}

void print_mtl(struct goat3d *g)
{
	int i, j, num_mtl, num_attr;
	struct goat3d_material *mtl;

	num_mtl = goat3d_get_mtl_count(g);
	for(i=0; i<num_mtl; i++) {
		mtl = goat3d_get_mtl(g, i);
		num_attr = goat3d_get_mtl_attrib_count(mtl);

		printf("Material %d: \"%s\" (%d attributes)\n", i, goat3d_get_mtl_name(mtl),
				num_attr);

		for(j=0; j<num_attr; j++) {
			printf(" - %s\n", goat3d_get_mtl_attrib_name(mtl, j));
		}
	}
	putchar('\n');
}

void print_mesh(struct goat3d *g)
{
	int i, num_meshes;
	struct goat3d_mesh *mesh;
	int nverts, total_verts = 0;
	int nfaces, total_faces = 0;

	num_meshes = goat3d_get_mesh_count(g);
	for(i=0; i<num_meshes; i++) {
		mesh = goat3d_get_mesh(g, i);

		nverts = goat3d_get_mesh_vertex_count(mesh);
		nfaces = goat3d_get_mesh_face_count(mesh);

		printf("Mesh %d: \"%s\" (%d verts/%d faces)\n", i, goat3d_get_mesh_name(mesh),
				nverts, nfaces);
		total_verts += nverts;
		total_faces += nfaces;
	}

	printf("\nTotal verts: %d / total faces: %d\n\n", total_verts, total_faces);
}

void print_light(struct goat3d *g)
{
	int i, num_lt;
	struct goat3d_light *lt;

	num_lt = goat3d_get_light_count(g);
	for(i=0; i<num_lt; i++) {
		lt = goat3d_get_light(g, i);

		printf("Light %d: \"%s\"\n", i, goat3d_get_light_name(lt));
	}

	putchar('\n');
}

void print_cam(struct goat3d *g)
{
	int i, num_cam;
	struct goat3d_camera *cam;

	num_cam = goat3d_get_camera_count(g);
	for(i=0; i<num_cam; i++) {
		cam = goat3d_get_camera(g, i);

		printf("Camera %d: \"%s\"\n", i, goat3d_get_camera_name(cam));
	}

	putchar('\n');
}

void print_node_rec(struct goat3d_node *n, int lvl)
{
	int i, num_child;
	struct goat3d_node *c;
	void *obj;

	for(i=0; i<lvl; i++) fputs(" ", stdout);

	printf(" - \"%s\"", goat3d_get_node_name(n));
	if((obj = goat3d_get_node_object(n))) {
		/* the type doesn't matter, objects are polymorphic for their common attributes */
		printf(" [%s]\n", goat3d_get_mesh_name(obj));
	} else {
		putchar('\n');
	}

	num_child = goat3d_get_node_child_count(n);
	for(i=0; i<num_child; i++) {
		c = goat3d_get_node_child(n, i);
		print_node_rec(c, lvl + 1);
	}
}

void print_node(struct goat3d *g)
{
	int i, num_nodes;
	struct goat3d_node *node;

	num_nodes = goat3d_get_node_count(g);
	for(i=0; i<num_nodes; i++) {
		node = goat3d_get_node(g, i);

		if(!goat3d_get_node_parent(node)) {
			print_node_rec(node, 0);
		}
	}

	putchar('\n');
}

void print_anim(struct goat3d *g)
{
	int i, num_anim, num_trk;
	struct goat3d_anim *anim;

	num_anim = goat3d_get_anim_count(g);
	for(i=0; i<num_anim; i++) {
		anim = goat3d_get_anim(g, i);

		num_trk = goat3d_get_anim_track_count(anim);

		printf("Anim %d: \"%s\" (%d tracks)\n", i, goat3d_get_anim_name(anim),
				num_trk);
	}

	putchar('\n');
}
