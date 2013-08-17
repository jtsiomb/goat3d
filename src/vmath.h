#ifndef VMATH_H_
#define VMATH_H_

class Vector3 {
public:
	float x, y, z;

	Vector3() : x(0), y(0), z(0) {}
	Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

class Vector4 {
public:
	float x, y, z, w;

	Vector4() : x(0), y(0), z(0), w(1) {}
	Vector4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

#endif	// VMATH_H_
