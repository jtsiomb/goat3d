#include "light.h"

using namespace g3dimpl;

Light::Light()
	: color(1, 1, 1), attenuation(1, 0, 0)
{
	max_dist = 0.0;
}

DirLight::DirLight()
	: dir(0, 0, 1)
{
}

SpotLight::SpotLight()
{
	inner_cone = DEG_TO_RAD(30);
	outer_cone = DEG_TO_RAD(45);
}
