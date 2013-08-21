#ifndef NODE_H_
#define NODE_H_

#include <string>
#include <vector>
#include "xform_node.h"

class Node : public XFormNode {
private:
	Object *obj;

public:
	Node();
	virtual ~Node();

	void set_object(Object *obj);
	Object *get_object();
	const Object *get_object() const;
};

#endif	// NODE_H_
