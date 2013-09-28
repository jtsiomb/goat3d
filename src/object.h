#ifndef OBJECT_H_
#define OBJECT_H_

#include <string>
#include <vmath/vmath.h>

class Object {
public:
	std::string name;

	Vector3 pos;
	Quaternion rot;

	virtual ~Object() {};
};

#endif	// OBJECT_H_
