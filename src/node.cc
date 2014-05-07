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
#include <algorithm>
#include <string.h>
#include "node.h"

using namespace g3dimpl;

Node::Node()
{
	obj = 0;
	bbox_valid = false;
}

void Node::set_object(Object *obj)
{
	this->obj = obj;
	bbox_valid = false;
}

Object *Node::get_object()
{
	bbox_valid = false;
	return obj;
}

const Object *Node::get_object() const
{
	return obj;
}

const AABox &Node::get_bounds() const
{
	if(!bbox_valid) {
		bbox = obj ? obj->get_bounds() : AABox();

		for(int i=0; i<get_children_count(); i++) {
			bbox = aabox_union(bbox, ((Node*)get_child(i))->get_bounds());
		}
		bbox_valid = true;
	}

	return bbox;
}

void g3dimpl::delete_node_tree(Node *n)
{
	if(!n) return;

	for(int i=0; i<n->get_children_count(); i++) {
		delete_node_tree((Node*)n->get_child(i));
	}
	delete n;
}
