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
#include "material.h"

using namespace g3dimpl;

MaterialAttrib Material::def_attr;

int Material::get_attrib_count() const
{
	return (int)attrib.size();
}

const char *Material::get_attrib_name(int idx) const
{
	if(idx < 0 || idx >= get_attrib_count()) {
		return 0;
	}

	std::map<std::string, MaterialAttrib>::const_iterator it = attrib.begin();
	for(int i=0; i<idx; i++) it++;
	return it->first.c_str();
}

MaterialAttrib &Material::operator [](int idx)
{
	if(idx < 0 || idx >= get_attrib_count()) {
		return def_attr;
	}

	std::map<std::string, MaterialAttrib>::iterator it = attrib.begin();
	for(int i=0; i<idx; i++) it++;
	return it->second;
}

const MaterialAttrib &Material::operator [](int idx) const
{
	if(idx < 0 || idx >= get_attrib_count()) {
		return def_attr;
	}

	std::map<std::string, MaterialAttrib>::const_iterator it = attrib.begin();
	for(int i=0; i<idx; i++) it++;
	return it->second;
}

MaterialAttrib &Material::operator [](const std::string &name)
{
	return attrib[name];
}

const MaterialAttrib &Material::operator [](const std::string &name) const
{
	std::map<std::string, MaterialAttrib>::const_iterator it;
	if((it = attrib.find(name)) != attrib.end()) {
		return it->second;
	}
	return def_attr;
}

