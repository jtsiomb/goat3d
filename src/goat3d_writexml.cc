/*
goat3d - 3D scene, character, and animation file format library.
Copyright (C) 2013-2014  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <list>
#include <stdarg.h>
#include "goat3d_impl.h"
#include "anim/anim.h"
#include "log.h"

using namespace g3dimpl;

static bool write_material(const Scene *scn, goat3d_io *io, const Material *mat, int level);
static bool write_mesh(const Scene *scn, goat3d_io *io, const Mesh *mesh, int idx, int level);
static bool write_light(const Scene *scn, goat3d_io *io, const Light *light, int level);
static bool write_camera(const Scene *scn, goat3d_io *io, const Camera *cam, int level);
static bool write_node(const Scene *scn, goat3d_io *io, const Node *node, int level);
static bool write_node_anim(goat3d_io *io, const XFormNode *node, int level);
static void xmlout(goat3d_io *io, int level, const char *fmt, ...);

bool Scene::savexml(goat3d_io *io) const
{
	xmlout(io, 0, "<scene>\n");

	// write environment stuff
	xmlout(io, 1, "<env>\n");
	xmlout(io, 2, "<ambient float3=\"%g %g %g\"/>\n", ambient.x, ambient.y, ambient.z);
	xmlout(io, 1, "</env>\n\n");

	for(size_t i=0; i<materials.size(); i++) {
		write_material(this, io, materials[i], 1);
	}
	for(size_t i=0; i<meshes.size(); i++) {
		write_mesh(this, io, meshes[i], (int)i, 1);
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

bool Scene::save_anim_xml(goat3d_io *io) const
{
	xmlout(io, 0, "<anim>\n");

	if(!nodes.empty()) {
		const char *anim_name = nodes[0]->get_animation_name();
		if(anim_name && *anim_name) {
			xmlout(io, 1, "<name string=\"%s\"/>\n", anim_name);
		}
	}

	for(size_t i=0; i<nodes.size(); i++) {
		write_node_anim(io, nodes[i], 1);
	}

	xmlout(io, 0, "</anim>\n");
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
		xmlout(io, level + 2, "<val float4=\"%g %g %g %g\"/>\n", attr.value.x,
				attr.value.y, attr.value.z, attr.value.w);
		if(!attr.map.empty()) {
			xmlout(io, level + 2, "<map string=\"%s\"/>\n", attr.map.c_str());
		}
		xmlout(io, level + 1, "</attr>\n");
	}
	xmlout(io, level, "</mtl>\n\n");
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

	if(!mesh->save(mesh_filename)) {
		return false;
	}

	// then refer to that filename in the XML tags
	xmlout(io, level, "<mesh>\n");
	xmlout(io, level + 1, "<name string=\"%s\"/>\n", mesh->name.c_str());
	if(mesh->material) {
		xmlout(io, level + 1, "<material string=\"%s\"/>\n", mesh->material->name.c_str());
	}
	xmlout(io, level + 1, "<file string=\"%s\"/>\n", clean_filename(mesh_filename).c_str());
	xmlout(io, level, "</mesh>\n\n");
	return true;
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
	xmlout(io, level, "<node>\n");
	xmlout(io, level + 1, "<name string=\"%s\"/>\n", node->get_name());

	const XFormNode *parent = node->get_parent();
	if(parent) {
		xmlout(io, level + 1, "<parent string=\"%s\"/>\n", parent->get_name());
	}

	const char *type = 0;
	const Object *obj = node->get_object();
	if(dynamic_cast<const Mesh*>(obj)) {
		type = "mesh";
	} else if(dynamic_cast<const Light*>(obj)) {
		type = "light";
	} else if(dynamic_cast<const Camera*>(obj)) {
		type = "camera";
	}

	if(type) {
		xmlout(io, level + 1, "<%s string=\"%s\"/>\n", type, obj->name.c_str());
	}

	Vector3 pos = node->get_node_position();
	Quaternion rot = node->get_node_rotation();
	Vector3 scale = node->get_node_scaling();
	Vector3 pivot = node->get_pivot();

	Matrix4x4 xform;
	node->get_node_xform(0, &xform);

	xmlout(io, level + 1, "<pos float3=\"%g %g %g\"/>\n", pos.x, pos.y, pos.z);
	xmlout(io, level + 1, "<rot float4=\"%g %g %g %g\"/>\n", rot.v.x, rot.v.y, rot.v.z, rot.s);
	xmlout(io, level + 1, "<scale float3=\"%g %g %g\"/>\n", scale.x, scale.y, scale.z);
	xmlout(io, level + 1, "<pivot float3=\"%g %g %g\"/>\n", pivot.x, pivot.y, pivot.z);

	xmlout(io, level + 1, "<matrix0 float4=\"%g %g %g %g\"/>\n", xform[0][0], xform[0][1], xform[0][2], xform[0][3]);
	xmlout(io, level + 1, "<matrix1 float4=\"%g %g %g %g\"/>\n", xform[1][0], xform[1][1], xform[1][2], xform[1][3]);
	xmlout(io, level + 1, "<matrix2 float4=\"%g %g %g %g\"/>\n", xform[2][0], xform[2][1], xform[2][2], xform[2][3]);

	xmlout(io, level, "</node>\n");
	return true;
}

static bool write_node_anim(goat3d_io *io, const XFormNode *node, int level)
{
	/* NOTE: the order of names must correspond to the order of the
	 * XFormNode::POSITION_TRACK/ROTATION_TRACK/SCALING_TRACK enum
	 */
	static const char *attr_names[] = { "position", "rotation", "scaling" };

	// for each of: position/rotation/scaling
	for(int i=0; i<3; i++) {
		int num_keys = node->get_key_count(i);
		if(!num_keys) continue;

		xmlout(io, level + 1, "<track>\n");
		xmlout(io, level + 2, "<node string=\"%s\"/>\n", node->get_name());
		xmlout(io, level + 2, "<attr string=\"%s\"/>\n\n", attr_names[i]);

		// for each key in that track
		for(int j=0; j<num_keys; j++) {
			long tm = node->get_key_time(i, j);

			float value[4];
			int num_elems = node->get_key_value(i, j, value);

			if(num_elems == 3) {
				xmlout(io, level + 2, "<key><time int=\"%ld\"/><value float3=\"%g %g %g\"/></key>\n",
						tm, value[0], value[1], value[2]);
			} else {
				xmlout(io, level + 2, "<key><time int=\"%ld\"/><value float4=\"%g %g %g %g\"/></key>\n",
						tm, value[0], value[1], value[2], value[3]);
			}
		}

		xmlout(io, level + 1, "</track>\n");
	}
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
