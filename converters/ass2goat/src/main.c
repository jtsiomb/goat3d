#include <stdio.h>
#include <stdlib.h>
#include "goat3d.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

enum {
	CONV_SCENE,
	CONV_ANIM
};

int convert(const char *infname);
int convert_anim(const char *infname);
void process_material(struct goat3d_material *mtl, struct aiMaterial *aimtl);
void process_node(struct goat3d *goat, struct goat3d_node *parent, struct aiNode *ainode);
int process_anim(struct goat3d *goat, struct aiAnimation *aianim);
static int output_filename(char *buf, int bufsz, const char *fname, const char *suffix);
static long assimp_time(const struct aiAnimation *anim, double aitime);

int main(int argc, char **argv)
{
	int i, num_done = 0;
	int conv_targ = CONV_SCENE;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] != 0) {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return 1;
			}

			switch(argv[i][1]) {
			case 'a':
				conv_targ = CONV_ANIM;
				break;

			case 's':
				conv_targ = CONV_SCENE;
				break;

			default:
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return 1;
			}

		} else {
			if(conv_targ == CONV_SCENE) {
				convert(argv[i]);
			} else {
				convert_anim(argv[i]);
			}
			num_done++;
		}
	}

	if(!num_done) {
		fprintf(stderr, "you must specify a 3D scene file to convert\n");
		return 1;
	}

	return 0;
}

#define SCE_PPFLAGS	\
	(aiProcess_Triangulate | \
	 aiProcess_GenNormals | \
	 aiProcess_JoinIdenticalVertices | \
	 aiProcess_CalcTangentSpace | \
	 aiProcess_LimitBoneWeights | \
	 aiProcess_GenUVCoords)

#define ANM_PPFLAGS \
	(aiProcess_LimitBoneWeights)

int convert(const char *infname)
{
	int i, bufsz;
	const struct aiScene *aiscn;
	struct goat3d *goat;
	char *outfname;

	bufsz = output_filename(0, 0, infname, "goat3d");
	outfname = alloca(bufsz);
	output_filename(outfname, bufsz, infname, "goat3d");
	printf("converting %s -> %s\n", infname, outfname);


	if(!(aiscn = aiImportFile(infname, SCE_PPFLAGS))) {
		fprintf(stderr, "failed to import %s\n", infname);
		return -1;
	}

	goat = goat3d_create();

	for(i=0; i<(int)aiscn->mNumMaterials; i++) {
		struct aiMaterial *aimat = aiscn->mMaterials[i];
		struct goat3d_material *mat = goat3d_create_mtl();

		process_material(mat, aimat);
		goat3d_add_mtl(goat, mat);
	}

	for(i=0; i<(int)aiscn->mRootNode->mNumChildren; i++) {
		process_node(goat, 0, aiscn->mRootNode->mChildren[i]);
	}

	goat3d_save(goat, outfname);
	goat3d_free(goat);
	aiReleaseImport(aiscn);
	return 0;
}

int convert_anim(const char *infname)
{
	int i, bufsz;
	const struct aiScene *aiscn;
	struct goat3d *goat;
	char *outfname;

	bufsz = output_filename(0, 0, infname, "goatanim");
	outfname = alloca(bufsz);
	output_filename(outfname, bufsz, infname, "goatanim");
	printf("converting %s -> %s\n", infname, outfname);


	if(!(aiscn = aiImportFile(infname, ANM_PPFLAGS))) {
		fprintf(stderr, "failed to import %s\n", infname);
		return -1;
	}

	goat = goat3d_create();

	for(i=0; i<(int)aiscn->mRootNode->mNumChildren; i++) {
		process_node(goat, 0, aiscn->mRootNode->mChildren[i]);
	}

	for(i=0; i<aiscn->mNumAnimations; i++) {
		if(process_anim(goat, aiscn->mAnimations[i]) == -1) {
			return -1;
		}
	}

	goat3d_save_anim(goat, outfname);
	goat3d_free(goat);
	aiReleaseImport(aiscn);
	return 0;
}

void process_material(struct goat3d_material *mtl, struct aiMaterial *aimtl)
{
	struct aiString aistr;
	struct aiColor4D color;
	float val;

	if(aiGetMaterialString(aimtl, AI_MATKEY_NAME, &aistr) == aiReturn_SUCCESS) {
		goat3d_set_mtl_name(mtl, aistr.data);
	}

	if(aiGetMaterialColor(aimtl, AI_MATKEY_COLOR_DIFFUSE, &color) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib3f(mtl, GOAT3D_MAT_ATTR_DIFFUSE, color.r, color.g, color.b);
	}

	if(aiGetMaterialColor(aimtl, AI_MATKEY_COLOR_SPECULAR, &color) == aiReturn_SUCCESS) {
		float sstr = 1.0;
		aiGetMaterialFloatArray(aimtl, AI_MATKEY_SHININESS_STRENGTH, &sstr, 0);
		goat3d_set_mtl_attrib3f(mtl, GOAT3D_MAT_ATTR_SPECULAR, color.r * sstr, color.g * sstr, color.b * sstr);
	}

	if(aiGetMaterialFloatArray(aimtl, AI_MATKEY_BUMPSCALING, &val, 0) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib3f(mtl, GOAT3D_MAT_ATTR_BUMP, val, val, val);
	}

	if(aiGetMaterialFloatArray(aimtl, AI_MATKEY_REFLECTIVITY, &val, 0) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_REFLECTION, val);
	}

	if(aiGetMaterialFloatArray(aimtl, AI_MATKEY_OPACITY, &val, 0) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_TRANSMISSION, 1.0 - val);
	}

	if(aiGetMaterialString(aimtl, AI_MATKEY_TEXTURE_DIFFUSE(0), &aistr) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_DIFFUSE, aistr.data);
	}
	if(aiGetMaterialString(aimtl, AI_MATKEY_TEXTURE_SPECULAR(0), &aistr) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_SPECULAR, aistr.data);
	}
	if(aiGetMaterialString(aimtl, AI_MATKEY_TEXTURE_SHININESS(0), &aistr) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_SHININESS, aistr.data);
	}
	if(aiGetMaterialString(aimtl, AI_MATKEY_TEXTURE_NORMALS(0), &aistr) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_NORMAL, aistr.data);
	}
	if(aiGetMaterialString(aimtl, AI_MATKEY_TEXTURE_REFLECTION(0), &aistr) == aiReturn_SUCCESS) {
		goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_REFLECTION, aistr.data);
	}
	if(aiGetMaterialString(aimtl, AI_MATKEY_TEXTURE_OPACITY(0), &aistr) == aiReturn_SUCCESS) {
		// TODO this is semantically inverted... maybe add an alpha attribute?
		goat3d_set_mtl_attrib_map(mtl, GOAT3D_MAT_ATTR_TRANSMISSION, aistr.data);
	}
}

void process_node(struct goat3d *goat, struct goat3d_node *parent, struct aiNode *ainode)
{
	int i;
	struct goat3d_node *node;

	node = goat3d_create_node();
	goat3d_set_node_name(node, ainode->mName.data);

	for(i=0; i<ainode->mNumChildren; i++) {
		process_node(goat, node, ainode->mChildren[i]);
	}

	goat3d_add_node(goat, node);
}

int process_anim(struct goat3d *goat, struct aiAnimation *aianim)
{
	int i, j, num_nodes, rnodes_count;
	const char *anim_name;

	if(aianim->mName.length <= 0) {
		anim_name = "unnamed";
	} else {
		anim_name = aianim->mName.data;
	}

	num_nodes = goat3d_get_node_count(goat);

	rnodes_count = 0;
	for(i=0; i<num_nodes; i++) {
		int anim_idx;
		struct goat3d_node *n = goat3d_get_node(goat, i);
		/* skip non-root nodes */
		if(goat3d_get_node_parent(n)) {
			break;
		}

		/* then add another animation to those root nodes */
		anim_idx = goat3d_get_anim_count(n);
		goat3d_add_anim(n);
		goat3d_use_anim(n, anim_idx);

		goat3d_set_anim_name(n, anim_name);
	}

	/* for each animation "channel" ... */
	for(i=0; i<(int)aianim->mNumChannels; i++) {
		struct goat3d_node *node;
		struct aiNodeAnim *ainodeanim = aianim->mChannels[i];

		/* find the node it refers to */
		const char *nodename = ainodeanim->mNodeName.data;
		if(!(node = goat3d_get_node_by_name(goat, nodename))) {
			fprintf(stderr, "failed to process animation for unknown node: %s\n", nodename);
			return -1;
		}

		/* add all the keys ... */
		for(j=0; j<(int)ainodeanim->mNumPositionKeys; j++) {
			struct aiVectorKey *key = ainodeanim->mPositionKeys + j;
			long tm = assimp_time(aianim, key->mTime);
			goat3d_set_node_position(node, key->mValue.x, key->mValue.y, key->mValue.z, tm);
		}

		for(j=0; j<(int)ainodeanim->mNumRotationKeys; j++) {
			struct aiQuatKey *key = ainodeanim->mRotationKeys + j;
			long tm = assimp_time(aianim, key->mTime);
			goat3d_set_node_rotation(node, key->mValue.x, key->mValue.y, key->mValue.z, key->mValue.w, tm);
		}

		for(j=0; j<(int)ainodeanim->mNumScalingKeys; j++) {
			struct aiVectorKey *key = ainodeanim->mScalingKeys + j;
			long tm = assimp_time(aianim, key->mTime);
			goat3d_set_node_scaling(node, key->mValue.x, key->mValue.y, key->mValue.z, tm);
		}
	}

	return 0;
}

static int output_filename(char *buf, int bufsz, const char *fname, const char *suffix)
{
	int reqsz, namesz;
	char *tmpfname;
	const char *fname_end, *lastdot;

	lastdot = strrchr(fname, '.');

	fname_end = lastdot ? lastdot : fname + strlen(fname);
	namesz = fname_end - fname;
	reqsz = namesz + strlen(suffix) + 2;	/* plus 1 for the dot */

	if(buf && bufsz) {
		tmpfname = alloca(namesz + 1);
		memcpy(tmpfname, fname, namesz);
		tmpfname[namesz] = 0;

		if(suffix) {
			snprintf(buf, bufsz, "%s.%s", tmpfname, suffix);
		} else {
			strncpy(buf, tmpfname, bufsz);
		}
		buf[bufsz - 1] = 0;
	}

	return reqsz;
}

static long assimp_time(const struct aiAnimation *anim, double aitime)
{
	double sec;
	if(anim->mTicksPerSecond < 1e-6) {
		/* assume time in frames? */
		sec = aitime / 30.0;
	} else {
		sec = aitime / anim->mTicksPerSecond;
	}
	return (long)(sec * 1000.0);
}
