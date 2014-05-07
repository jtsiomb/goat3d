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
#ifndef OBJECT_H_
#define OBJECT_H_

#include <string>
#include <vmath/vmath.h>
#include "aabox.h"

namespace g3dimpl {

class Object {
public:
	std::string name;

	Vector3 pos;
	Quaternion rot;
	Vector3 scale;

	Object() : scale(1, 1, 1) {}
	virtual ~Object() {}

	virtual AABox get_bounds(const Matrix4x4 &xform) const { return AABox(); }
};

}

#endif	// OBJECT_H_
