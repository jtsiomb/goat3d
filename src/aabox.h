#ifndef AABOX_H_
#define AABOX_H_

#include <vmath/vmath.h>

namespace g3dimpl {

class AABox {
public:
	Vector3 bmin, bmax;

	AABox();
	AABox(const Vector3 &b0, const Vector3 &b1);

	bool operator ==(const AABox &rhs) const;
	bool operator !=(const AABox &rhs) const;
};

AABox aabox_union(const AABox &a, const AABox &b);

}

#endif	// AABOX_H_
