#include <algorithm>
#include <string.h>
#include "node.h"

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

void delete_node_tree(Node *n)
{
	for(int i=0; i<n->get_children_count(); i++) {
		delete_node_tree((Node*)n->get_child(i));
	}
	delete n;
}
