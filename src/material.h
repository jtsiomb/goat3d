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
#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <string>
#include <map>
#include <vmath/vmath.h>

namespace g3dimpl {

struct MaterialAttrib {
	Vector4 value;
	std::string map;

	MaterialAttrib() : value(1, 1, 1, 1) {}
};

class Material {
private:
	static MaterialAttrib def_attr;

	std::map<std::string, MaterialAttrib> attrib;

public:
	std::string name;

	int get_attrib_count() const;
	const char *get_attrib_name(int idx) const;

	MaterialAttrib &operator [](int idx);
	const MaterialAttrib &operator [](int idx) const;

	MaterialAttrib &operator [](const std::string &name);
	const MaterialAttrib &operator [](const std::string &name) const;
};

}	// namespace g3dimpl

#endif	// MATERIAL_H_
