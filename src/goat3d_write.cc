#include "goat3d_impl.h"
#include "chunk.h"

using namespace g3dimpl;

/*
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
*/

bool Scene::save(goat3d_io *io) const
{
	/*
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
	*/
	return false;
}

bool Scene::save_anim(goat3d_io *io) const
{
	return false;
}


#if 0
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
#endif
