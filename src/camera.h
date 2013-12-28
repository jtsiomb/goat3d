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
