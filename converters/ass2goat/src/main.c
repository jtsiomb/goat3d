#include <stdio.h>
#include <stdlib.h>
#include "goat3d.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

int convert(const char *infname, const char *outfname);
void process_material(struct goat3d_material *mtl, struct aiMaterial *aimtl);
void process_node(struct goat3d *goat, struct goat3d_node *parent, struct aiNode *ainode);

int main(int argc, char **argv)
{
	int i, num_done = 0;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
		} else {
			char *lastdot;
			char *outfname = malloc(strlen(argv[i]) + 4);
			strcpy(outfname, argv[i]);

			if((lastdot = strrchr(outfname, '.'))) {
				*lastdot = 0;
			}
			strcat(outfname, ".xml");

			printf("converting %s -> %s\n", argv[i], outfname);
			convert(argv[i], outfname);
			num_done++;
		}
	}

	if(!num_done) {
		fprintf(stderr, "you must specify a 3D scene file to convert\n");
		return 1;
	}

	return 0;
}

#define PPFLAGS	\
	(aiProcess_Triangulate | \
	 aiProcess_GenNormals | \
	 aiProcess_JoinIdenticalVertices | \
	 aiProcess_CalcTangentSpace | \
	 aiProcess_LimitBoneWeights | \
	 aiProcess_GenUVCoords)

int convert(const char *infname, const char *outfname)
{
	int i;
	const struct aiScene *aiscn;
	struct goat3d *goat;

	if(!(aiscn = aiImportFile(infname, PPFLAGS))) {
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
