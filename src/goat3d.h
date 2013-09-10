#ifndef GOAT3D_H_
#define GOAT3D_H_

#include <stdio.h>
#include <stdlib.h>

struct goat3d;

struct goat3d_io {
	void *cls;	/* closure data */

	long (*read)(void *buf, size_t bytes, void *uptr);
	long (*write)(const void *buf, size_t bytes, void *uptr);
	long (*seek)(long offs, int whence, void *uptr);
};

struct goat3d_vec3 { float x, y, z; };
struct goat3d_vec4 { float x, y, z, w; };


#ifdef __cplusplus
extern "C" {
#endif

/* construction/destruction */
struct goat3d *goat3d_create();
void goat3d_free(struct goat3d *g);

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

void goat3d_set_ambient(struct goat3d *g, float x, float y, float z);

/* helpers */
struct goat3d_vec3 goat3d_vec3(float x, float y, float z);
struct goat3d_vec4 goat3d_vec4(float x, float y, float z, float w);


#ifdef __cplusplus
}
#endif

#endif	/* GOAT3D_H_ */
