#ifndef CHUNK_H_
#define CHUNK_H_

enum {
	CNK_INVALID,		// this shouldn't appear in files
	CNK_SCENE,			// the root chunk

	// general purpose chunks
	CNK_INT,
	CNK_UINT,
	CNK_FLOAT,
	CNK_VEC3,
	CNK_VEC4,
	CNK_STRING,

	// --- first level chunks ---
	// children of CNK_SCENE
	CNK_ENV,			// environmental parameters
	CNK_MTL_LIST,		// material library
	CNK_MESH_LIST,		// all the meshes hang under this chunk
	CNK_LIGHT_LIST,		// likewise for lights
	CNK_CAMERA_LIST,	// likewise for cameras
	CNK_NODE_LIST,		// likewise for nodes
	CNK_ANIM_LIST,		// all animations

	// --- second level chunks ---
	// children of CNK_ENV
	CNK_ENV_AMBIENT,	// ambient color, contains a single CNK_VEC3
	CNK_ENV_FOG,

	// children of CNK_*_LIST
	CNK_MTL,
	CNK_MESH,
	CNK_LIGHT,
	CNK_CAMERA,
	CNK_NODE,

	// --- third level chunks ---
	// children of CNK_FOG
	CNK_FOG_COLOR,		// fog color, contains a single CNK_VEC3
	CNK_FOG_EXP,		// fog exponent, contains a single CNK_REAL

	// children of CNK_MTL
	CNK_MTL_ATTR,		// material attribute, has a CNK_STRING for its name,
						// a CNK_MTL_ATTR_VAL, and optionally a CNK_MTL_ATTR_MAP
	// children of CNK_MTL_ATTR
	CNK_MTL_ATTR_VAL,	// can have a single CNK_FLOAT, CNK_VEC3, or CNK_VEC4
	CNK_MTL_ATTR_MAP,	// has a single CNK_STRING

	// children of CNK_MESH
	// TODO...
};

struct ChunkHeader {
	uint32_t id;
	uint32_t size;
};

struct Chunk {
	ChunkHeader hdr;
	char data[1];
};

#endif	// CHUNK_H_
