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
#ifndef CAMERA_H_
#define CAMERA_H_

#include "object.h"

namespace g3dimpl {

class Camera : public Object {
public:
	float near_clip, far_clip;

	Camera();
};

class TargetCamera : public Camera {
public:
	Vector3 target, up;

	TargetCamera();
};

}	// namespace g3dimpl

#endif	// CAMERA_H_
