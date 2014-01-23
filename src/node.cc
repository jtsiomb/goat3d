#include <algorithm>
#include <string.h>
#include "node.h"

using namespace g3dimpl;

Node::Node()
{
	obj = 0;
}

void Node::set_object(Object *obj)
{
	this->obj = obj;
}

Object *Node::get_object()
{
	return obj;
}

const Object *Node::get_object() const
{
	return obj;
}

void g3dimpl::delete_node_tree(Node *n)
{
	if(!n) return;

	for(int i=0; i<n->get_children_count(); i++) {
		delete_node_tree((Node*)n->get_child(i));
	}
	delete n;
}
