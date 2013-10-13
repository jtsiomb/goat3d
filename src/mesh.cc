#include "goat3d_impl.h"
#include "mesh.h"
#include "openctm.h"
#include "log.h"

Int4::Int4()
{
	x = y = z = w = 0;
}

Int4::Int4(int x, int y, int z, int w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Mesh::Mesh()
{
	material = 0;
}

bool Mesh::load(const char *fname)
{
	CTMcontext ctm = ctmNewContext(CTM_IMPORT);

	ctmLoad(ctm, fname);
	if(ctmGetError(ctm) != CTM_NONE) {
		logmsg(LOG_ERROR, "failed to load ctm mesh: %s\n", fname);
		ctmFreeContext(ctm);
		return false;
	}

	int vnum = ctmGetInteger(ctm, CTM_VERTEX_COUNT);
	int fnum = ctmGetInteger(ctm, CTM_TRIANGLE_COUNT);

	const CTMfloat *vertices = ctmGetFloatArray(ctm, CTM_VERTICES);
	if(!vertices) {
		logmsg(LOG_ERROR, "failed to load ctm mesh: %s: no vertices found!\n", fname);
		ctmFreeContext(ctm);
		return false;
	}

	const CTMuint *indices = ctmGetIntegerArray(ctm, CTM_INDICES);
	if(!indices) {
		logmsg(LOG_ERROR, "failed to load ctm mesh: %s: no faces found!\n", fname);
		ctmFreeContext(ctm);
		return false;
	}

	const CTMfloat *normals = ctmGetFloatArray(ctm, CTM_NORMALS);
	const CTMfloat *texcoords = ctmGetFloatArray(ctm, CTM_UV_MAP_1);

	CTMenum tangent_id = ctmGetNamedAttribMap(ctm, "tangent");
	const CTMfloat *tangents = tangent_id ? ctmGetFloatArray(ctm, tangent_id) : 0;

	CTMenum skinweight_id = ctmGetNamedAttribMap(ctm, "skin_weight");
	const CTMfloat *skinweights = skinweight_id ? ctmGetFloatArray(ctm, skinweight_id) : 0;

	CTMenum skinmat_id = ctmGetNamedAttribMap(ctm, "skin_matrix");
	const CTMuint *skinmats = skinmat_id ? ctmGetIntegerArray(ctm, skinmat_id) : 0;

	CTMenum color_id = ctmGetNamedAttribMap(ctm, "color");
	const CTMfloat *colors = color_id ? ctmGetFloatArray(ctm, color_id) : 0;

	// now put everything we found into our vectors
	this->vertices = VECDATA(Vector3, vertices, vnum);

	if(texcoords) {
		this->texcoords = VECDATA(Vector2, texcoords, vnum);
	}
	if(normals) {
		this->normals = VECDATA(Vector3, normals, vnum);
	}
	if(skinweights) {
		this->skin_weights = VECDATA(Vector4, skinweights, vnum);
	}
	if(colors) {
		this->colors = VECDATA(Vector4, colors, vnum);
	}

	// the rest need converting
	if(tangents) {
		this->tangents.clear();
		this->tangents.resize(vnum);

		for(int i=0; i<vnum; i++) {
			for(int j=0; j<3; j++) {
				this->tangents[i][j] = tangents[j];
			}
			tangents += 4;
		}
	}
	if(skinmats) {
		this->skin_matrices.clear();
		this->skin_matrices.resize(vnum);

		for(int i=0; i<vnum; i++) {
			this->skin_matrices[i].x = skinmats[0];
			this->skin_matrices[i].y = skinmats[1];
			this->skin_matrices[i].z = skinmats[2];
			this->skin_matrices[i].w = skinmats[3];
		}
	}

	// grab the face data
	this->faces.clear();
	this->faces.resize(fnum);

	for(int i=0; i<fnum; i++) {
		for(int j=0; j<3; j++) {
			this->faces[i].v[j] = indices[j];
		}
		indices += 3;
	}


	ctmFreeContext(ctm);
	return true;
}

bool Mesh::save(const char *fname) const
{
	int vnum = (int)vertices.size();
	int fnum = (int)faces.size();

	if(!vnum || !fnum) {
		return false;
	}

	CTMcontext ctm = ctmNewContext(CTM_EXPORT);

	// vertices, normals, and face-vertex indices
	ctmDefineMesh(ctm, &vertices[0].x, vnum, (CTMuint*)faces[0].v, fnum,
			normals.empty() ? 0 : &normals[0].x);

	// texture coordinates
	if(!texcoords.empty()) {
		ctmAddUVMap(ctm, &texcoords[0].x, "texcoord", 0);
	}

	// vertex colors
	if(!colors.empty()) {
		ctmAddAttribMap(ctm, &colors[0].x, "color");
	}

	// skin weights
	if(!skin_weights.empty()) {
		ctmAddAttribMap(ctm, &skin_weights[0].x, "skin_weight");
	}

	// if either of the non-float4 attributes are present we need to make a tmp array
	CTMfloat *attr_array = 0;
	if(!tangents.empty() || !skin_matrices.empty()) {
		attr_array = new CTMfloat[vnum * 4 * sizeof *attr_array];
	}

	// tangents
	if(!tangents.empty()) {
		CTMfloat *ptr = attr_array;

		for(int i=0; i<vnum; i++) {
			*ptr++ = tangents[i].x;
			*ptr++ = tangents[i].y;
			*ptr++ = tangents[i].z;
			*ptr++ = 1.0;
		}
		ctmAddAttribMap(ctm, attr_array, "tangent");
	}

	// skin matrix indices (4 per vertex)
	if(!skin_matrices.empty()) {
		CTMfloat *ptr = attr_array;

		for(int i=0; i<vnum; i++) {
			*ptr++ = (float)skin_matrices[i].x;
			*ptr++ = (float)skin_matrices[i].y;
			*ptr++ = (float)skin_matrices[i].z;
			*ptr++ = (float)skin_matrices[i].w;
		}
		ctmAddAttribMap(ctm, attr_array, "skin_matrix");
	}

	delete [] attr_array;

	/* TODO find a way to specify the nodes participating in the skinning of this mesh
	 * probably in the comment field?
	 */

	logmsg(LOG_INFO, "saving CTM mesh file: %s\n", fname);
	ctmSave(ctm, fname);

	ctmFreeContext(ctm);
	return true;
}

void Mesh::set_material(Material *mat)
{
	material = mat;
}

Material *Mesh::get_material()
{
	return material;
}

const Material *Mesh::get_material() const
{
	return material;
}
