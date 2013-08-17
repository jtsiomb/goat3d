#ifndef NODE_H_
#define NODE_H_

#include <string>
#include <vector>

class Node {
private:
	std::string name;
	Node *parent;
	std::vector<Node*> children;

public:
	Node();
	virtual ~Node();

	virtual void set_name(const char *name);
	virtual const char *get_name() const;

	virtual void add_child(Node *c);
	virtual int get_num_children() const;

	virtual Node *get_child(int idx) const;
	virtual Node *get_child(const char *name) const;
	virtual Node *get_descendant(const char *name) const;

	virtual Node *get_parent() const;
	// passing 0 will return the root
	virtual Node *get_ancestor(const char *name) const;
};

#endif	// NODE_H_
