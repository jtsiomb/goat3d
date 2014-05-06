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
#include <stdio.h>
#include "goat3d.h"
#include "goat3d_impl.h"
#include "tinyxml2.h"
#include "log.h"

using namespace g3dimpl;
using namespace tinyxml2;

static Material *read_material(Scene *scn, XMLElement *xml_mtl);
static const char *read_material_attrib(MaterialAttrib *attr, XMLElement *xml_attr);
static Mesh *read_mesh(Scene *scn, XMLElement *xml_mesh);
static std::string get_name(XMLElement *node, int idx);

bool Scene::loadxml(goat3d_io *io)
{
	long bytes = io->seek(0, SEEK_END, io->cls);
	io->seek(0, SEEK_SET, io->cls);

	char *buf = new char[bytes];
	if(io->read(buf, bytes, io->cls) < bytes) {
		logmsg(LOG_ERROR, "failed to read XML scene file\n");
		delete [] buf;
		return false;
	}

	XMLDocument xml;
	XMLError err = xml.Parse(buf, bytes);
	if(err) {
		logmsg(LOG_ERROR, "failed to parse XML scene file: %s\n%s\n", xml.GetErrorStr1(),
				xml.GetErrorStr2());
		delete [] buf;
		return false;
	}

	XMLElement *root = xml.RootElement();
	if(strcmp(root->Name(), "scene") != 0) {
		logmsg(LOG_ERROR, "invalid XML file, root node is not <scene>\n");
		delete [] buf;
		return false;
	}

	XMLElement *elem;

	// get all materials
	elem = root->FirstChildElement("mtl");
	while(elem) {
		Material *mtl = read_material(this, elem);
		if(mtl) {
			add_material(mtl);
		}
		elem = elem->NextSiblingElement("mtl");
	}

	// get all meshes
	elem = root->FirstChildElement("mesh");
	while(elem) {
		Mesh *mesh = read_mesh(this, elem);
		if(mesh) {
			add_mesh(mesh);
		}
		elem = elem->NextSiblingElement("mesh");
	}

	delete [] buf;
	return true;
}

bool Scene::load_anim_xml(goat3d_io *io)
{
	long bytes = io->seek(0, SEEK_END, io->cls);
	io->seek(0, SEEK_SET, io->cls);

	char *buf = new char[bytes];
	if(io->read(buf, bytes, io->cls) < bytes) {
		logmsg(LOG_ERROR, "failed to read XML animation file\n");
		delete [] buf;
		return false;
	}

	XMLDocument xml;
	XMLError err = xml.Parse(buf, bytes);
	if(err) {
		logmsg(LOG_ERROR, "failed to parse XML animation file: %s\n%s\n", xml.GetErrorStr1(),
				xml.GetErrorStr2());
		delete [] buf;
		return false;
	}

	XMLElement *root = xml.RootElement();
	if(strcmp(root->Name(), "anim") != 0) {
		logmsg(LOG_ERROR, "invalid XML file, root node is not <anim>\n");
		delete [] buf;
		return false;
	}

	XMLElement *elem;

	elem = root->FirstChildElement();
	while(elem) {
		const char *elem_name = elem->Name();

		if(strcmp(elem_name, "name") == 0) {
		} else if(strcmp(elem_name, "attr") == 0) {
		}
		elem = elem->NextSiblingElement();
	}

	delete [] buf;
	return true;
}

static Material *read_material(Scene *scn, XMLElement *xml_mtl)
{
	Material *mtl = new Material;
	mtl->name = get_name(xml_mtl, scn->get_material_count());

	// get all the material attributes in turn
	XMLElement *elem = xml_mtl->FirstChildElement("attr");
	while(elem) {
		MaterialAttrib attr;
		const char *name = read_material_attrib(&attr, elem);
		if(name) {
			(*mtl)[name] = attr;
		}

		elem = elem->NextSiblingElement("attr");
	}

	return mtl;
}

static const char *read_material_attrib(MaterialAttrib *attr, XMLElement *xml_attr)
{
	const char *name;

	XMLElement *elem;
	if((elem = xml_attr->FirstChildElement("name"))) {
		if(!(name = elem->Attribute("string"))) {
			return 0;
		}
	}

	if((elem = xml_attr->FirstChildElement("val"))) {
		if(elem->QueryFloatAttribute("float", &attr->value.x) != XML_NO_ERROR) {
			// try a float3
			const char *valstr = elem->Attribute("float3");
			if(!valstr || sscanf(valstr, "%f %f %f", &attr->value.x, &attr->value.y,
						&attr->value.z) != 3) {
				// try a float4
				valstr = elem->Attribute("float4");
				if(!valstr || sscanf(valstr, "%f %f %f %f", &attr->value.x, &attr->value.y,
							&attr->value.z, &attr->value.w) != 4) {
					// no valid val attribute found
					return 0;
				}
			}
		}
	}

	if((elem = xml_attr->FirstChildElement("map"))) {
		const char *tex = elem->Attribute("string");
		if(tex) {
			attr->map = std::string(tex);
		}
	}

	return name;
}

static Mesh *read_mesh(Scene *scn, XMLElement *xml_mesh)
{
	Mesh *mesh = new Mesh;
	mesh->name = get_name(xml_mesh, scn->get_mesh_count());

	XMLElement *elem;
	if((elem = xml_mesh->FirstChildElement("material"))) {
		int idx;
		if(elem->QueryIntAttribute("int", &idx) == XML_NO_ERROR) {
			mesh->material = scn->get_material(idx);
		} else {
			// try string
			const char *mtlstr = elem->Attribute("string");
			if(mtlstr) {
				mesh->material = scn->get_material(mtlstr);
			}
		}
	}

	/* reading mesh data from XML is not supported, only MESH_FILE can be used to
	 * specify an external mesh file to be loaded
	 */

	if((elem = xml_mesh->FirstChildElement("file"))) {
		const char *fname = elem->Attribute("string");
		if(fname) {
			char *path = (char*)fname;
			if(scn->goat->search_path) {
				path = (char*)alloca(strlen(fname) + strlen(scn->goat->search_path) + 2);
				sprintf(path, "%s/%s", scn->goat->search_path, fname);
			}
			if(!mesh->load(path)) {
				delete mesh;
				return 0;
			}
		}
	}

	return mesh;
}

static std::string get_name(XMLElement *node, int idx)
{
	char buf[64];
	const char *name = 0;

	XMLElement *elem;
	if((elem = node->FirstChildElement("name"))) {
		name = elem->Attribute("string");
	}

	if(!name) {
		sprintf(buf, "mesh%04d", idx);
		name = buf;
	}

	return std::string(name);
}
