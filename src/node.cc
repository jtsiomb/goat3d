#include <algorithm>
#include <string.h>
#include "node.h"

Node::Node()
{
	parent = 0;
}

Node::~Node()
{
}

void Node::set_name(const char *name)
{
	this->name = name;
}

const char *Node::get_name() const
{
	return name.c_str();
}

void Node::add_child(Node *c)
{
	// make sure we don't add it twice
	if(std::find(children.begin(), children.end(), c) != children.end()) {
		return;
	}
	children.push_back(c);
	c->parent = this;
}

int Node::get_num_children() const
{
	return (int)children.size();
}

Node *Node::get_child(int idx) const
{
	if(idx < 0 || idx >= get_num_children()) {
		return 0;
	}
	return children[idx];
}

Node *Node::get_child(const char *name) const
{
	for(size_t i=0; i<children.size(); i++) {
		if(strcmp(children[i]->get_name(), name) == 0) {
			return children[i];
		}
	}
	return 0;
}

Node *Node::get_descendant(const char *name) const
{
	Node *c = get_child(name);
	if(c) {
		return c;
	}

	// depth first search might not be ideal in this case, but it's the simplest
	for(size_t i=0; i<children.size(); i++) {
		if((c = children[i]->get_descendant(name))) {
			return c;
		}
	}
	return 0;
}

Node *Node::get_parent() const
{
	return parent;
}

Node *Node::get_ancestor(const char *name) const
{
	Node *n = (Node*)this;

	if(!name) {
		// just return the root
		while(n->parent) {
			n = n->parent;
		}
		return n;
	}

	// otherwise we're looking for a specific ancestor
	while(n && strcmp(n->get_name(), name) != 0) {
		n = n->parent;
	}
	return n;
}
