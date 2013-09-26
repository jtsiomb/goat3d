#include "material.h"

MaterialAttrib Material::def_attr;

int Material::get_attrib_count() const
{
	return (int)attrib.size();
}

const char *Material::get_attrib_name(int idx) const
{
	if(idx < 0 || idx >= get_attrib_count()) {
		return 0;
	}

	std::map<std::string, MaterialAttrib>::const_iterator it = attrib.begin();
	for(int i=0; i<idx; i++) it++;
	return it->first.c_str();
}

MaterialAttrib &Material::operator [](int idx)
{
	if(idx < 0 || idx >= get_attrib_count()) {
		return def_attr;
	}

	std::map<std::string, MaterialAttrib>::iterator it = attrib.begin();
	for(int i=0; i<idx; i++) it++;
	return it->second;
}

const MaterialAttrib &Material::operator [](int idx) const
{
	if(idx < 0 || idx >= get_attrib_count()) {
		return def_attr;
	}

	std::map<std::string, MaterialAttrib>::const_iterator it = attrib.begin();
	for(int i=0; i<idx; i++) it++;
	return it->second;
}

MaterialAttrib &Material::operator [](const std::string &name)
{
	return attrib[name];
}

const MaterialAttrib &Material::operator [](const std::string &name) const
{
	std::map<std::string, MaterialAttrib>::const_iterator it;
	if((it = attrib.find(name)) != attrib.end()) {
		return it->second;
	}
	return def_attr;
}

