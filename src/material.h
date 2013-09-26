#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <string>
#include <map>
#include <vmath/vmath.h>

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

#endif	// MATERIAL_H_
