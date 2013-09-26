#include <stdarg.h>
#include "goat3d_impl.h"
#include "chunk.h"
#include "openctm.h"

static bool write_material(const Scene *scn, goat3d_io *io, const Material *mat, int level);
static bool write_mesh(const Scene *scn, goat3d_io *io, const Mesh *mesh, int idx, int level);
static void write_ctm_mesh(const Mesh *mesh, const char *fname);
static bool write_light(const Scene *scn, goat3d_io *io, const Light *light, int level);
static bool write_camera(const Scene *scn, goat3d_io *io, const Camera *cam, int level);
static bool write_node(const Scene *scn, goat3d_io *io, const Node *node, int level);
static void xmlout(goat3d_io *io, int level, const char *fmt, ...);

bool Scene::savexml(goat3d_io *io) const
{
	xmlout(io, 0, "<scene>\n");

	// write environment stuff
	xmlout(io, 1, "<env>\n");
	xmlout(io, 1, "</env>\n");

	for(size_t i=0; i<materials.size(); i++) {
		write_material(this, io, materials[i], 1);
	}
	for(size_t i=0; i<meshes.size(); i++) {
		write_mesh(this, io, meshes[i], i, 1);
	}
	for(size_t i=0; i<lights.size(); i++) {
		write_light(this, io, lights[i], 1);
	}
	for(size_t i=0; i<cameras.size(); i++) {
		write_camera(this, io, cameras[i], 1);
	}
	for(size_t i=0; i<nodes.size(); i++) {
		write_node(this, io, nodes[i], 1);
	}

	xmlout(io, 0, "</scene>\n");
	return true;
}

static bool write_material(const Scene *scn, goat3d_io *io, const Material *mat, int level)
{
	xmlout(io, level, "<mtl>\n");
	xmlout(io, level + 1, "<name string=\"%s\"/>\n", mat->name.c_str());

	for(int i=0; i<mat->get_attrib_count(); i++) {
		xmlout(io, level + 1, "<attr>\n");
		xmlout(io, level + 2, "<name string=\"%s\"/>\n", mat->get_attrib_name(i));

		const MaterialAttrib &attr = (*mat)[i];
		xmlout(io, level + 2, "<val float4=\"%.3f %.3f %.3f\"/>\n", attr.value.x,
				attr.value.y, attr.value.z, attr.value.w);
		if(!attr.map.empty()) {
			xmlout(io, level + 2, "<map string=\"%s\"/>\n", attr.map.c_str());
		}
		xmlout(io, level + 1, "</attr>\n");
	}
	xmlout(io, level, "</mtl>\n");
	return true;
}

static bool write_mesh(const Scene *scn, goat3d_io *io, const Mesh *mesh, int idx, int level)
{
	// first write the external (openctm) mesh file
	const char *prefix = scn->get_name();
	if(!prefix) {
		prefix = "goat";
	}

	char *mesh_filename = (char*)alloca(strlen(prefix) + 32);
	sprintf(mesh_filename, "%s-mesh%04d.ctm", prefix, idx);

	write_ctm_mesh(mesh, mesh_filename);

	// then refer to that filename in the XML tags
	xmlout(io, level, "<mesh>\n");
	xmlout(io, level + 1, "<name string=\"%s\"/>\n", mesh->name.c_str());
	xmlout(io, level + 1, "<material string=\"%s\"/>\n", mesh->material->name.c_str());
	xmlout(io, level + 1, "<file string=\"%s\"/>\n", mesh_filename);
	xmlout(io, level, "</mesh>\n");
	return true;
}

static void write_ctm_mesh(const Mesh *mesh, const char *fname)
{
	int vnum = (int)mesh->vertices.size();

	CTMcontext ctm = ctmNewContext(CTM_EXPORT);

	// vertices, normals, and face-vertex indices
	ctmDefineMesh(ctm, &mesh->vertices[0].x, vnum, (CTMuint*)mesh->faces[0].v,
			mesh->faces.size(), mesh->normals.empty() ? 0 : &mesh->normals[0].x);

	// texture coordinates
	if(!mesh->texcoords.empty()) {
		CTMfloat *uvarray = new CTMfloat[vnum * 2 * sizeof *uvarray];
		CTMfloat *uvptr = uvarray;

		for(int i=0; i<vnum; i++) {
			*uvptr++ = mesh->texcoords[i].x;
			*uvptr++ = mesh->texcoords[i].y;
		}
		ctmAddUVMap(ctm, uvarray, "texcoord", 0);
		delete [] uvarray;
	}

	// vertex colors
	if(!mesh->colors.empty()) {
		ctmAddAttribMap(ctm, &mesh->colors[0].x, "color");
	}

	// skin weights
	if(!mesh->skin_weights.empty()) {
		ctmAddAttribMap(ctm, &mesh->skin_weights[0].x, "skin_weight");
	}

	// if either of the non-float4 attributes are present we need to make a tmp array
	CTMfloat *attr_array = 0;
	if(!mesh->tangents.empty() || !mesh->skin_matrices.empty()) {
		attr_array = new CTMfloat[vnum * 4 * sizeof *attr_array];
	}

	// tangents
	if(!mesh->tangents.empty()) {
		CTMfloat *ptr = attr_array;

		for(int i=0; i<vnum; i++) {
			*ptr++ = mesh->tangents[i].x;
			*ptr++ = mesh->tangents[i].y;
			*ptr++ = mesh->tangents[i].z;
			*ptr++ = 1.0;
		}
		ctmAddAttribMap(ctm, attr_array, "tangent");
	}

	// skin matrix indices (4 per vertex)
	if(!mesh->skin_matrices.empty()) {
		CTMfloat *ptr = attr_array;

		for(int i=0; i<vnum; i++) {
			*ptr++ = (float)mesh->skin_matrices[i].x;
			*ptr++ = (float)mesh->skin_matrices[i].y;
			*ptr++ = (float)mesh->skin_matrices[i].z;
			*ptr++ = (float)mesh->skin_matrices[i].w;
		}
		ctmAddAttribMap(ctm, attr_array, "skin_matrix");
	}

	delete [] attr_array;

	/* TODO find a way to specify the nodes participating in the skinning of this mesh
	 * probably in the comment field?
	 */

	ctmSave(ctm, fname);

	ctmFreeContext(ctm);
}

static bool write_light(const Scene *scn, goat3d_io *io, const Light *light, int level)
{
	return true;
}

static bool write_camera(const Scene *scn, goat3d_io *io, const Camera *cam, int level)
{
	return true;
}

static bool write_node(const Scene *scn, goat3d_io *io, const Node *node, int level)
{
	return true;
}


static void xmlout(goat3d_io *io, int level, const char *fmt, ...)
{
	for(int i=0; i<level; i++) {
		io_fprintf(io, "  ");
	}

	va_list ap;
	va_start(ap, fmt);
	io_vfprintf(io, fmt, ap);
	va_end(ap);
}
