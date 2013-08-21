#ifndef NODE_H_
#define NODE_H_

#include "xform_node.h"
#include "object.h"

class Node : public XFormNode {
private:
	Object *obj;

public:
	Node();

	void set_object(Object *obj);
	Object *get_object();
	const Object *get_object() const;
};

void delete_node_tree(Node *n);

#endif	// NODE_H_
