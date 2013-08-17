/*
TODO: add multiple animations per node in libanim (i.e. multiple sets of tracks)
*/
#ifndef XFORM_NODE_H_
#define XFORM_NODE_H_

#include <vector>
#include "vmath/vector.h"
#include "vmath/quat.h"
#include "vmath/matrix.h"

enum Interp { INTERP_STEP, INTERP_LINEAR, INTERP_CUBIC };
enum Extrap { EXTRAP_EXTEND, EXTRAP_CLAMP, EXTRAP_REPEAT };

struct anm_node;
struct anm_track;

// XXX all time arguments are milliseconds

class XFormNode {
private:
	struct anm_node *anm;
	std::vector<XFormNode*> children;

	Interp interp;
	Extrap extrap;

	Matrix4x4 local_matrix;
	Matrix4x4 bone_matrix;

	XFormNode(const XFormNode &node) {}
	XFormNode &operator =(const XFormNode &node) { return *this; }

public:
	XFormNode();
	virtual ~XFormNode();

	void set_name(const char *name);
	const char *get_name() const;

	void set_interpolator(Interp in);
	Interp get_interpolator() const;
	void set_extrapolator(Extrap ex);
	Extrap get_extrapolator() const;

	// children management
	void add_child(XFormNode *child);
	void remove_child(XFormNode *child);

	int get_children_count() const;
	XFormNode *get_child(int idx);
	const XFormNode *get_child(int idx) const;


	void set_position(const Vector3 &pos, long tmsec = 0);
	Vector3 get_node_position(long tmsec = 0) const;

	void set_rotation(const Quaternion &quat, long tmsec = 0);
	Quaternion get_node_rotation(long tmsec = 0) const;

	void set_scaling(const Vector3 &pos, long tmsec = 0);
	Vector3 get_node_scaling(long tmsec = 0) const;

	// these take hierarchy into account
	Vector3 get_position(long tmsec = 0) const;
	Quaternion get_rotation(long tmsec = 0) const;
	Vector3 get_scaling(long tmsec = 0) const;

	void set_pivot(const Vector3 &pivot);
	Vector3 get_pivot() const;

	// the local matrix is concatenated with the regular node/anim matrix
	void set_local_matrix(const Matrix4x4 &mat);
	const Matrix4x4 &get_local_matrix() const;

	// for bone nodes, the transformation of the bone in bind position
	void set_bone_matrix(const Matrix4x4 &bmat);
	const Matrix4x4 &get_bone_matrix() const;

	// node transformation alone
	void get_node_xform(long tmsec, Matrix4x4 *mat, Matrix4x4 *inv_mat = 0) const;

	// node transformation taking hierarchy into account
	void get_xform(long tmsec, Matrix4x4 *mat, Matrix4x4 *inv_mat = 0) const;
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

#endif	/* XFORM_NODE_H_ */
