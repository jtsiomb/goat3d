#ifndef GOAT3D_H_
#define GOAT3D_H_

#include <stdio.h>
#include <stdlib.h>

#define GOAT3D_MAT_ATTR_DIFFUSE			"diffuse"
#define GOAT3D_MAT_ATTR_SPECULAR		"specular"
#define GOAT3D_MAT_ATTR_SHININESS		"shininess"
#define GOAT3D_MAT_ATTR_NORMAL			"normal"
#define GOAT3D_MAT_ATTR_BUMP			"bump"
#define GOAT3D_MAT_ATTR_REFLECTION		"reflection"
#define GOAT3D_MAT_ATTR_TRANSMISSION	"transmission"
#define GOAT3D_MAT_ATTR_IOR				"ior"

enum goat3d_mesh_attrib {
	GOAT3D_MESH_ATTR_VERTEX,
	GOAT3D_MESH_ATTR_NORMAL,
	GOAT3D_MESH_ATTR_TANGENT,
	GOAT3D_MESH_ATTR_TEXCOORD,
	GOAT3D_MESH_ATTR_SKIN_WEIGHT,
	GOAT3D_MESH_ATTR_SKIN_MATRIX,
	GOAT3D_MESH_ATTR_COLOR,

	NUM_GOAT3D_MESH_ATTRIBS
};

/* immediate mode mesh construction primitive type */
enum goat3d_im_primitive {
	GOAT3D_TRIANGLES,
	GOAT3D_QUADS
};


enum goat3d_option {
	GOAT3D_OPT_SAVEXML,		/* save in XML format */

	NUM_GOAT3D_OPTIONS
};

struct goat3d;
struct goat3d_material;
struct goat3d_mesh;
struct goat3d_light;
struct goat3d_camera;
struct goat3d_node;

struct goat3d_io {
	void *cls;	/* closure data */

	long (*read)(void *buf, size_t bytes, void *uptr);
	long (*write)(const void *buf, size_t bytes, void *uptr);
	long (*seek)(long offs, int whence, void *uptr);
};

#ifdef __cplusplus
extern "C" {
#endif

/* construction/destruction */
struct goat3d *goat3d_create(void);
void goat3d_free(struct goat3d *g);

void goat3d_setopt(struct goat3d *g, enum goat3d_option opt, int val);
int goat3d_getopt(const struct goat3d *g, enum goat3d_option opt);

/* load/save */
int goat3d_load(struct goat3d *g, const char *fname);
int goat3d_save(const struct goat3d *g, const char *fname);

int goat3d_load_file(struct goat3d *g, FILE *fp);
int goat3d_save_file(const struct goat3d *g, FILE *fp);

int goat3d_load_io(struct goat3d *g, struct goat3d_io *io);
int goat3d_save_io(const struct goat3d *g, struct goat3d_io *io);

/* misc scene properties */
int goat3d_set_name(struct goat3d *g, const char *name);
const char *goat3d_get_name(const struct goat3d *g);

void goat3d_set_ambient(struct goat3d *g, const float *ambient);
void goat3d_set_ambient3f(struct goat3d *g, float ar, float ag, float ab);
const float *goat3d_get_ambient(const struct goat3d *g);

/* materials */
struct goat3d_material *goat3d_create_mtl(void);
void goat3d_destroy_mtl(struct goat3d_material *mtl);

void goat3d_set_mtl_name(struct goat3d_material *mtl, const char *name);
const char *goat3d_get_mtl_name(const struct goat3d_material *mtl);

void goat3d_set_mtl_attrib(struct goat3d_material *mtl, const char *attrib, const float *val);
void goat3d_set_mtl_attrib1f(struct goat3d_material *mtl, const char *attrib, float val);
void goat3d_set_mtl_attrib3f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b);
void goat3d_set_mtl_attrib4f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b, float a);
const float *goat3d_get_mtl_attrib(struct goat3d_material *mtl, const char *attrib);

void goat3d_set_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib, const char *mapname);
const char *goat3d_get_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib);

void goat3d_add_mtl(struct goat3d *g, struct goat3d_material *mtl);

/* meshes */
struct goat3d_mesh *goat3d_create_mesh(void);
void goat3d_destroy_mesh(struct goat3d_mesh *mesh);

void goat3d_set_mesh_name(struct goat3d_mesh *mesh, const char *name);
const char *goat3d_get_mesh_name(const struct goat3d_mesh *mesh);

void goat3d_set_mesh_mtl(struct goat3d_mesh *mesh, struct goat3d_material *mtl);
struct goat3d_material *goat3d_get_mesh_mtl(struct goat3d_mesh *mesh);

int goat3d_get_mesh_attrib_count(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib);
int goat3d_get_mesh_face_count(struct goat3d_mesh *mesh);

/* sets all the data for a single vertex attribute array in one go.
 * vnum is the number of *vertices* to be set, not the number of floats, ints or whatever
 * data is expected to be something different depending on the attribute:
 *  - GOAT3D_MESH_ATTR_VERTEX       - 3 floats per vertex
 *  - GOAT3D_MESH_ATTR_NORMAL       - 3 floats per vertex
 *  - GOAT3D_MESH_ATTR_TANGENT      - 3 floats per vertex
 *  - GOAT3D_MESH_ATTR_TEXCOORD     - 2 floats per vertex
 *  - GOAT3D_MESH_ATTR_SKIN_WEIGHT  - 4 floats per vertex
 *  - GOAT3D_MESH_ATTR_SKIN_MATRIX  - 4 ints per vertex
 *  - GOAT3D_MESH_ATTR_COLOR        - 4 floats per vertex
 */
void goat3d_set_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, const void *data, int vnum);
/* returns a pointer to the beginning of the requested mesh attribute array */
void *goat3d_get_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib);
/* returns a pointer to the requested mesh attribute */
void *goat3d_get_mesh_attrib(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, int idx);

/* sets all the faces in one go. data is an array of 3 int vertex indices per face */
void goat3d_set_mesh_faces(struct goat3d_mesh *mesh, const int *data, int fnum);
/* returns a pointer to the beginning of the face index array */
int *goat3d_get_mesh_faces(struct goat3d_mesh *mesh);
/* returns a pointer to a face index */
int *goat3d_get_mesh_face(struct goat3d_mesh *mesh, int idx);

/* immediate mode OpenGL-like interface for setting mesh data
 *  NOTE: using this interface will result in no vertex sharing between faces
 * NOTE2: the immedate mode interface is not thread-safe, either use locks, or don't
 *        use it at all in multithreaded situations.
 */
void goat3d_begin(struct goat3d_mesh *mesh, enum goat3d_im_primitive prim);
void goat3d_end(void);
void goat3d_vertex3f(float x, float y, float z);
void goat3d_normal3f(float x, float y, float z);
void goat3d_tangent3f(float x, float y, float z);
void goat3d_texcoord2f(float x, float y);
void goat3d_skin_weight4f(float x, float y, float z, float w);
void goat3d_skin_matrix4i(int x, int y, int z, int w);
void goat3d_color3f(float x, float y, float z);
void goat3d_color4f(float x, float y, float z, float w);

void goat3d_add_mesh(struct goat3d *g, struct goat3d_mesh *mesh);

int goat3d_get_mesh_count(struct goat3d *g);
struct goat3d_mesh *goat3d_get_mesh(struct goat3d *g, int idx);

#ifdef __cplusplus
}
#endif

#endif	/* GOAT3D_H_ */
