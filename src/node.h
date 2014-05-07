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
#ifndef NODE_H_
#define NODE_H_

#include "xform_node.h"
#include "object.h"
#include "aabox.h"

namespace g3dimpl {

class Node : public XFormNode {
private:
	Object *obj;

	mutable AABox bbox;
	mutable bool bbox_valid;

public:
	Node();

	void set_object(Object *obj);
	Object *get_object();
	const Object *get_object() const;

	const AABox &get_bounds() const;
};

void delete_node_tree(Node *n);

}	// namespace g3dimpl

#endif	// NODE_H_
