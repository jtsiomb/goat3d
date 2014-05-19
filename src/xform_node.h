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
#ifndef XFORM_NODE_H_
#define XFORM_NODE_H_

#include <vector>
#include "vmath/vector.h"
#include "vmath/quat.h"
#include "vmath/matrix.h"


struct anm_node;
struct anm_track;

namespace g3dimpl {

enum Interp { INTERP_STEP, INTERP_LINEAR, INTERP_CUBIC };
enum Extrap { EXTRAP_EXTEND, EXTRAP_CLAMP, EXTRAP_REPEAT };

// NOTE: all time arguments are milliseconds

class XFormNode {
private:
	struct anm_node *anm;
	std::vector<XFormNode*> children;
	XFormNode *parent;

	Interp interp;
	Extrap extrap;

	Matrix4x4 local_matrix;
	Matrix4x4 bone_matrix;

	XFormNode(const XFormNode &node) {}
	XFormNode &operator =(const XFormNode &node) { return *this; }

public:
	enum { POSITION_TRACK, ROTATION_TRACK, SCALING_TRACK };

	XFormNode();
	virtual ~XFormNode();

	// retrieve the pointer to the underlying libanim data structure
	virtual struct anm_node *get_libanim_node() const;

	virtual void set_name(const char *name);
	virtual const char *get_name() const;

	virtual void set_interpolator(Interp in);
	virtual Interp get_interpolator() const;
	virtual void set_extrapolator(Extrap ex);
	virtual Extrap get_extrapolator() const;

	virtual XFormNode *get_parent();
	virtual const XFormNode *get_parent() const;

	virtual XFormNode *get_root();
	virtual const XFormNode *get_root() const;

	// children management
	virtual void add_child(XFormNode *child);
	virtual void remove_child(XFormNode *child);

	virtual int get_children_count() const;
	virtual XFormNode *get_child(int idx);
	virtual const XFormNode *get_child(int idx) const;


	virtual void use_animation(int idx);
	virtual void use_animation(const char *name);
	virtual void use_animation(int aidx, int bidx, float t);
	virtual void use_animation(const char *aname, const char *bname, float t);

	virtual int get_active_animation_index(int which = 0) const;
	virtual float get_active_animation_mix() const;

	virtual int get_animation_count() const;

	// add a new empty animation slot (recursive)
	virtual void add_animation(const char *name = 0);

	// set/get the current animation name (set is recursive)
	virtual void set_animation_name(const char *name);
	virtual const char *get_animation_name() const;

	virtual bool get_timeline_bounds(long *start, long *end);

	// raw keyframe retrieval without interpolation
	// NOTE: trackid parameters correspond to the values of the unnamed enumeration at the top

	virtual int get_key_count(int trackid) const;
	virtual int get_position_key_count() const;
	virtual int get_rotation_key_count() const;
	virtual int get_scaling_key_count() const;

	virtual long get_key_time(int trackid, int idx) const;
	virtual long get_position_key_time(int idx) const;
	virtual long get_rotation_key_time(int idx) const;
	virtual long get_scaling_key_time(int idx) const;

	/* writes the key value through the val pointer, and returns the number
	 * of elements in that value (3 for pos/scale, 4 for rotation).
	 */
	virtual int get_key_value(int trackid, int idx, float *val) const;
	virtual Vector3 get_position_key_value(int idx) const;
	virtual Quaternion get_rotation_key_value(int idx) const;
	virtual Vector3 get_scaling_key_value(int idx) const;


	virtual void set_position(const Vector3 &pos, long tmsec = 0);
	virtual Vector3 get_node_position(long tmsec = 0) const;

	virtual void set_rotation(const Quaternion &quat, long tmsec = 0);
	virtual Quaternion get_node_rotation(long tmsec = 0) const;

	virtual void set_scaling(const Vector3 &pos, long tmsec = 0);
	virtual Vector3 get_node_scaling(long tmsec = 0) const;

	// these take hierarchy into account
	virtual Vector3 get_position(long tmsec = 0) const;
	virtual Quaternion get_rotation(long tmsec = 0) const;
	virtual Vector3 get_scaling(long tmsec = 0) const;

	virtual void set_pivot(const Vector3 &pivot);
	virtual Vector3 get_pivot() const;

	// the local matrix is concatenated with the regular node/anim matrix
	virtual void set_local_matrix(const Matrix4x4 &mat);
	virtual const Matrix4x4 &get_local_matrix() const;

	// for bone nodes, the transformation of the bone in bind position
	virtual void set_bone_matrix(const Matrix4x4 &bmat);
	virtual const Matrix4x4 &get_bone_matrix() const;

	// node transformation alone
	virtual void get_node_xform(long tmsec, Matrix4x4 *mat, Matrix4x4 *inv_mat = 0) const;

	// node transformation taking hierarchy into account
	virtual void get_xform(long tmsec, Matrix4x4 *mat, Matrix4x4 *inv_mat = 0) const;
};


class Track {
private:
	struct anm_track *trk;
	Interp interp;
	Extrap extrap;

public:
	Track();
	~Track();

	Track(const Track &trk);
	Track &operator =(const Track &trk);

	void set_interpolator(Interp in);
	Interp get_interpolator() const;
	void set_extrapolator(Extrap ex);
	Extrap get_extrapolator() const;

	void set_default(double def);

	void set_value(float val, long tmsec = 0);
	float get_value(long tmsec = 0) const;

	// the same as get_value
	float operator ()(long tmsec = 0) const;
};

class Track3 {
private:
	Track track[3];

public:
	void set_interpolator(Interp in);
	Interp get_interpolator() const;
	void set_extrapolator(Extrap ex);
	Extrap get_extrapolator() const;

	void set_default(const Vector3 &def);

	void set_value(const Vector3 &val, long tmsec = 0);
	Vector3 get_value(long tmsec = 0) const;

	// the same as get_value
	Vector3 operator ()(long tmsec = 0) const;
};

}	// namespace g3dimpl

#endif	/* XFORM_NODE_H_ */
