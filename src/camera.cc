#include "camera.h"

using namespace g3dimpl;

Camera::Camera()
{
	near_clip = 0.5;
	far_clip = 500.0;
}

TargetCamera::TargetCamera()
	: target(0, 0, 0), up(0, 1, 0)
{
	pos = Vector3(0, 0, 10);
}
