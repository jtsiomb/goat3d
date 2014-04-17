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
#ifndef LIGHT_H_
#define LIGHT_H_

#include <vmath/vmath.h>
#include "object.h"

namespace g3dimpl {

class Light : public Object {
public:
	Vector3 color;
	Vector3 attenuation;
	float max_dist;

	Light();
};

class DirLight : public Light {
public:
	Vector3 dir;

	DirLight();
};

class SpotLight : public DirLight {
public:
	float inner_cone, outer_cone;

	SpotLight();
};

}	// namespace g3dimpl

#endif	// LIGHT_H_
