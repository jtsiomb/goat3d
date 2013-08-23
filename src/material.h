#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <string>
#include <map>
#include <vmath/vmath.h>

struct MaterialAttrib {
	Vector4 value;
	std::string map;
};

#define MAT_ATTR_DIFFUSE		"diffuse"
#define MAT_ATTR_SPECULAR		"specular"
#define MAT_ATTR_SHININESS		"shininess"
#define MAT_ATTR_NORMAL			"normal"
#define MAT_ATTR_BUMP			"bump"
#define MAT_ATTR_REFLECTION		"reflection"
#define MAT_ATTR_TRANSMISSION	"transmission"
#define MAT_ATTR_IOR			"ior"

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
