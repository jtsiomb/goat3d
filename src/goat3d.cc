#include <string.h>
#include <errno.h>
#include "goat3d.h"
#include "goat3d_impl.h"
#include "log.h"

struct goat3d {
	Scene *scn;
	unsigned int flags;
};

struct goat3d_material : public Material {};
struct goat3d_mesh : public Mesh {};
struct goat3d_light : public Light {};
struct goat3d_camera : public Camera {};
struct goat3d_node : public Node {};


static long read_file(void *buf, size_t bytes, void *uptr);
static long write_file(const void *buf, size_t bytes, void *uptr);
static long seek_file(long offs, int whence, void *uptr);

extern "C" {

struct goat3d *goat3d_create(void)
{
	goat3d *goat = new goat3d;
	goat->scn = new Scene;
	return goat;
}

void goat3d_free(struct goat3d *g)
{
	delete g->scn;
	delete g;
}

void goat3d_setopt(struct goat3d *g, enum goat3d_option opt, int val)
{
	if(val) {
		g->flags |= (1 << (int)opt);
	} else {
		g->flags &= ~(1 << (int)opt);
	}
}

int goat3d_getopt(const struct goat3d *g, enum goat3d_option opt)
{
	return (g->flags >> (int)opt) & 1;
}

int goat3d_load(struct goat3d *g, const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if(!fp) {
		logmsg(LOG_ERROR, "failed to open file \"%s\" for reading: %s\n", fname, strerror(errno));
		return -1;
	}

	int res = goat3d_load_file(g, fp);
	fclose(fp);
	return res;
}

int goat3d_save(const struct goat3d *g, const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	if(!fp) {
		logmsg(LOG_ERROR, "failed to open file \"%s\" for writing: %s\n", fname, strerror(errno));
		return -1;
	}

	int res = goat3d_save_file(g, fp);
	fclose(fp);
	return res;
}

int goat3d_load_file(struct goat3d *g, FILE *fp)
{
	goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_load_io(g, &io);
}

int goat3d_save_file(const struct goat3d *g, FILE *fp)
{
	goat3d_io io;
	io.cls = fp;
	io.read = read_file;
	io.write = write_file;
	io.seek = seek_file;

	return goat3d_save_io(g, &io);
}

int goat3d_load_io(struct goat3d *g, struct goat3d_io *io)
{
	if(!g->scn->load(io)) {
		if(g->scn->loadxml(io)) {
			return -1;
		}
	}
	return 0;
}

int goat3d_save_io(const struct goat3d *g, struct goat3d_io *io)
{
	if(goat3d_getopt(g, GOAT3D_OPT_SAVEXML)) {
		return g->scn->savexml(io) ? 0 : -1;
	}
	return g->scn->save(io) ? 0 : -1;
}

int goat3d_set_name(struct goat3d *g, const char *name)
{
	g->scn->set_name(name);
	return 0;
}

const char *goat3d_get_name(const struct goat3d *g)
{
	return g->scn->get_name();
}

void goat3d_set_ambient(struct goat3d *g, const float *amb)
{
	g->scn->set_ambient(Vector3(amb[0], amb[1], amb[2]));
}

void goat3d_set_ambient3f(struct goat3d *g, float ar, float ag, float ab)
{
	g->scn->set_ambient(Vector3(ar, ag, ab));
}

const float *goat3d_get_ambient(const struct goat3d *g)
{
	return &g->scn->get_ambient().x;
}

// ---- materials ----
struct goat3d_material *goat3d_create_mtl(void)
{
	return new goat3d_material;
}

void goat3d_destroy_mtl(struct goat3d_material *mtl)
{
	delete mtl;
}

void goat3d_set_mtl_name(struct goat3d_material *mtl, const char *name)
{
	mtl->name = std::string(name);
}

const char *goat3d_get_mtl_name(const struct goat3d_material *mtl)
{
	return mtl->name.c_str();
}

void goat3d_set_mtl_attrib(struct goat3d_material *mtl, const char *attrib, const float *val)
{
	(*mtl)[attrib].value = Vector4(val[0], val[1], val[2], val[3]);
}

void goat3d_set_mtl_attrib1f(struct goat3d_material *mtl, const char *attrib, float val)
{
	goat3d_set_mtl_attrib4f(mtl, attrib, val, 0, 0, 1);
}

void goat3d_set_mtl_attrib3f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b)
{
	goat3d_set_mtl_attrib4f(mtl, attrib, r, g, b, 1);
}

void goat3d_set_mtl_attrib4f(struct goat3d_material *mtl, const char *attrib, float r, float g, float b, float a)
{
	(*mtl)[attrib].value = Vector4(r, g, b, a);
}

const float *goat3d_get_mtl_attrib(struct goat3d_material *mtl, const char *attrib)
{
	return &(*mtl)[attrib].value.x;
}

void goat3d_set_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib, const char *mapname)
{
	(*mtl)[attrib].map = std::string(mapname);
}

const char *goat3d_get_mtl_attrib_map(struct goat3d_material *mtl, const char *attrib)
{
	return (*mtl)[attrib].map.c_str();
}

void goat3d_add_mtl(struct goat3d *g, struct goat3d_material *mtl)
{
	g->scn->add_material(mtl);
}

// ---- meshes ----
struct goat3d_mesh *goat3d_create_mesh(void)
{
	return new goat3d_mesh;
}

void goat3d_destroy_mesh(struct goat3d_mesh *mesh)
{
	delete mesh;
}

void goat3d_set_mesh_name(struct goat3d_mesh *mesh, const char *name)
{
	mesh->name = std::string(name);
}

const char *goat3d_get_mesh_name(const struct goat3d_mesh *mesh)
{
	return mesh->name.c_str();
}

void goat3d_set_mesh_mtl(struct goat3d_mesh *mesh, struct goat3d_material *mtl)
{
	mesh->material = mtl;
}

struct goat3d_material *goat3d_get_mesh_mtl(struct goat3d_mesh *mesh)
{
	return (goat3d_material*)mesh->material;
}

int goat3d_get_mesh_attrib_count(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib)
{
	return (int)mesh->vertices.size();
}

int goat3d_get_mesh_face_count(struct goat3d_mesh *mesh)
{
	return (int)mesh->faces.size();
}

#if __cplusplus >= 201103L
#define MOVE(x)	std::move(x)
#else
#define MOVE(x) x
#endif

#define VECDATA(type, data, num) \
	MOVE(std::vector<type>((type*)(data), (type*)(data) + (num)))

void goat3d_set_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, const void *data, int vnum)
{
	if(attrib == GOAT3D_MESH_ATTR_VERTEX) {
		mesh->vertices = VECDATA(Vector3, data, vnum);
		return;
	}

	if(vnum != (int)mesh->vertices.size()) {
		logmsg(LOG_ERROR, "trying to set mesh attrib data with number of elements different than the vertex array\n");
		return;
	}

	switch(attrib) {
	case GOAT3D_MESH_ATTR_NORMAL:
		mesh->normals = VECDATA(Vector3, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_TANGENT:
		mesh->tangents = VECDATA(Vector3, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_TEXCOORD:
		mesh->texcoords = VECDATA(Vector2, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		mesh->skin_weights = VECDATA(Vector4, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		mesh->skin_matrices = VECDATA(Int4, data, vnum);
		break;
	case GOAT3D_MESH_ATTR_COLOR:
		mesh->colors = VECDATA(Vector4, data, vnum);
	default:
		break;
	}
}

void *goat3d_get_mesh_attribs(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib)
{
	return goat3d_get_mesh_attrib(mesh, attrib, 0);
}

void *goat3d_get_mesh_attrib(struct goat3d_mesh *mesh, enum goat3d_mesh_attrib attrib, int idx)
{
	switch(attrib) {
	case GOAT3D_MESH_ATTR_VERTEX:
		return mesh->vertices.empty() ? 0 : (void*)&mesh->vertices[idx];
	case GOAT3D_MESH_ATTR_NORMAL:
		return mesh->normals.empty() ? 0 : (void*)&mesh->normals[idx];
	case GOAT3D_MESH_ATTR_TANGENT:
		return mesh->tangents.empty() ? 0 : (void*)&mesh->tangents[idx];
	case GOAT3D_MESH_ATTR_TEXCOORD:
		return mesh->texcoords.empty() ? 0 : (void*)&mesh->texcoords[idx];
	case GOAT3D_MESH_ATTR_SKIN_WEIGHT:
		return mesh->skin_weights.empty() ? 0 : (void*)&mesh->skin_weights[idx];
	case GOAT3D_MESH_ATTR_SKIN_MATRIX:
		return mesh->skin_matrices.empty() ? 0 : (void*)&mesh->skin_matrices[idx];
	case GOAT3D_MESH_ATTR_COLOR:
		return mesh->colors.empty() ? 0 : (void*)&mesh->colors[idx];
	default:
		break;
	}
	return 0;
}


void goat3d_set_mesh_faces(struct goat3d_mesh *mesh, const int *data, int num)
{
	mesh->faces = VECDATA(Face, data, num);
}

int *goat3d_get_mesh_faces(struct goat3d_mesh *mesh)
{
	return goat3d_get_mesh_face(mesh, 0);
}

int *goat3d_get_mesh_face(struct goat3d_mesh *mesh, int idx)
{
	return mesh->faces.empty() ? 0 : mesh->faces[idx].v;
}

// immedate mode state
static enum goat3d_im_primitive im_prim;
static struct goat3d_mesh *im_mesh;
static Vector3 im_norm, im_tang;
static Vector2 im_texcoord;
static Vector4 im_skinw, im_color = Vector4(1, 1, 1, 1);
static Int4 im_skinmat;
static bool im_use[NUM_GOAT3D_MESH_ATTRIBS];


void goat3d_begin(struct goat3d_mesh *mesh, enum goat3d_im_primitive prim)
{
	mesh->vertices.clear();
	mesh->normals.clear();
	mesh->tangents.clear();
	mesh->texcoords.clear();
	mesh->skin_weights.clear();
	mesh->skin_matrices.clear();
	mesh->colors.clear();
	mesh->faces.clear();

	im_mesh = mesh;
	memset(im_use, 0, sizeof im_use);

	im_prim = prim;
}

void goat3d_end(void)
{
	static int tri_offs[] = {0, 1, 2};
	static int quad_offs[] = {0, 1, 2, 0, 2, 3};
	int *index_offs;

	int num_faces, in_face_verts, out_face_verts;
	switch(im_prim) {
	case GOAT3D_TRIANGLES:
		in_face_verts = 3;
		out_face_verts = 3;
		index_offs = tri_offs;
		break;

	case GOAT3D_QUADS:
		in_face_verts = 4;
		out_face_verts = 6;
		index_offs = quad_offs;
		break;

	default:
		return;
	};

	num_faces = (int)im_mesh->vertices.size() / in_face_verts;
	if(!num_faces) {
		return;
	}

	im_mesh->faces.resize(num_faces);

	int vidx = 0;
	for(int i=0; i<num_faces; i++) {
		for(int j=0; j<out_face_verts; j++) {
			im_mesh->faces[i].v[j] = vidx + index_offs[j];
		}
		vidx += 4;
	}
}

void goat3d_vertex3f(float x, float y, float z)
{
	im_mesh->vertices.push_back(Vector3(x, y, z));
	if(im_use[GOAT3D_MESH_ATTR_NORMAL]) {
		im_mesh->normals.push_back(im_norm);
	}
	if(im_use[GOAT3D_MESH_ATTR_TANGENT]) {
		im_mesh->tangents.push_back(im_tang);
	}
	if(im_use[GOAT3D_MESH_ATTR_TEXCOORD]) {
		im_mesh->texcoords.push_back(im_texcoord);
	}
	if(im_use[GOAT3D_MESH_ATTR_SKIN_WEIGHT]) {
		im_mesh->skin_weights.push_back(im_skinw);
	}
	if(im_use[GOAT3D_MESH_ATTR_SKIN_MATRIX]) {
		im_mesh->skin_matrices.push_back(im_skinmat);
	}
	if(im_use[GOAT3D_MESH_ATTR_COLOR]) {
		im_mesh->colors.push_back(im_color);
	}
}

void goat3d_normal3f(float x, float y, float z)
{
	im_norm = Vector3(x, y, z);
}

void goat3d_tangent3f(float x, float y, float z)
{
	im_tang = Vector3(x, y, z);
}

void goat3d_texcoord2f(float x, float y)
{
	im_texcoord = Vector2(x, y);
}

void goat3d_skin_weight4f(float x, float y, float z, float w)
{
	im_skinw = Vector4(x, y, z, w);
}

void goat3d_skin_matrix4i(int x, int y, int z, int w)
{
	im_skinmat.x = x;
	im_skinmat.y = y;
	im_skinmat.z = z;
	im_skinmat.w = w;
}

void goat3d_color3f(float x, float y, float z)
{
	im_color = Vector4(x, y, z, 1.0f);
}

void goat3d_color4f(float x, float y, float z, float w)
{
	im_color = Vector4(x, y, z, w);
}

void goat3d_add_mesh(struct goat3d *g, struct goat3d_mesh *mesh)
{
	g->scn->add_mesh(mesh);
}


}	// extern "C"


static long read_file(void *buf, size_t bytes, void *uptr)
{
	return (long)fread(buf, 1, bytes, (FILE*)uptr);
}

static long write_file(const void *buf, size_t bytes, void *uptr)
{
	return (long)fwrite(buf, 1, bytes, (FILE*)uptr);
}

static long seek_file(long offs, int whence, void *uptr)
{
	if(fseek((FILE*)uptr, offs, whence) == -1) {
		return -1;
	}
	return ftell((FILE*)uptr);
}
