#include <float.h>
#include <algorithm>
#include "aabox.h"

using namespace g3dimpl;

AABox::AABox()
	: bmin(FLT_MAX, FLT_MAX, FLT_MAX), bmax(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

AABox::AABox(const Vector3 &b0, const Vector3 &b1)
	: bmin(b0), bmax(b1)
{
}

bool AABox::operator ==(const AABox &rhs) const
{
	return bmin == rhs.bmin && bmax == rhs.bmax;
}

bool AABox::operator !=(const AABox &rhs) const
{
	return !(*this == rhs);
}

AABox g3dimpl::aabox_union(const AABox &a, const AABox &b)
{
	Vector3 bmin, bmax;

	for(int i=0; i<3; i++) {
		bmin[i] = std::min(a.bmin[i], b.bmin[i]);
		bmax[i] = std::max(a.bmax[i], b.bmax[i]);
	}

	return AABox(bmin, bmax);
}
