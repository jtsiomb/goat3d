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
