#ifndef NODE_H_
#define NODE_H_

#include "xform_node.h"
#include "object.h"

namespace g3dimpl {

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

}	// namespace g3dimpl

#endif	// NODE_H_
