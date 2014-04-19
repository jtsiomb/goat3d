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
#include <assert.h>
#include <algorithm>
#include "xform_node.h"
#include "anim/anim.h"
#include "anim/track.h"

using namespace g3dimpl;

static inline anm_interpolator track_interpolator(Interp in);
static inline anm_extrapolator track_extrapolator(Extrap ex);

XFormNode::XFormNode()
{
	anm = new anm_node;
	anm_init_node(anm);
	parent = 0;

	// TODO read them from anm to get the correct initial values
	interp = INTERP_LINEAR;
	extrap = EXTRAP_EXTEND;
}

XFormNode::~XFormNode()
{
	anm_destroy_node(anm);
	delete anm;
}

struct anm_node *XFormNode::get_libanim_node() const
{
	return anm;
}

void XFormNode::set_name(const char *name)
{
	anm_set_node_name(anm, name);
}

const char *XFormNode::get_name() const
{
	return anm_get_node_name(anm);
}

void XFormNode::set_interpolator(Interp in)
{
	anm_set_interpolator(anm, track_interpolator(in));
	interp = in;
}

Interp XFormNode::get_interpolator() const
{
	return interp;
}

void XFormNode::set_extrapolator(Extrap ex)
{
	anm_set_extrapolator(anm, track_extrapolator(ex));
	extrap = ex;
}

Extrap XFormNode::get_extrapolator() const
{
	return extrap;
}

XFormNode *XFormNode::get_parent()
{
	return parent;
}

const XFormNode *XFormNode::get_parent() const
{
	return parent;
}

void XFormNode::add_child(XFormNode *child)
{
	if(!child || child == this) return;

	children.push_back(child);
	anm_link_node(anm, child->anm);
	child->parent = this;
}

void XFormNode::remove_child(XFormNode *child)
{
	std::vector<XFormNode*>::iterator it;
	it = std::find(children.begin(), children.end(), child);
	if(it != children.end()) {
		children.erase(it);
		anm_unlink_node(anm, child->anm);

		if(child->parent == this) {
			child->parent = 0;
		}
	}
}

int XFormNode::get_children_count() const
{
	return (int)children.size();
}

XFormNode *XFormNode::get_child(int idx)
{
	if(idx >= 0 && idx < get_children_count()) {
		return children[idx];
	}
	return 0;
}

const XFormNode *XFormNode::get_child(int idx) const
{
	if(idx >= 0 && idx < get_children_count()) {
		return children[idx];
	}
	return 0;
}


void XFormNode::use_animation(int idx)
{
	if(idx >= 0) {
		anm_use_animation(anm, idx);
	}
}

void XFormNode::use_animation(const char *name)
{
	anm_use_animation(anm, anm_find_animation(anm, name));
}

void XFormNode::use_animation(int aidx, int bidx, float t)
{
	anm_use_animations(anm, aidx, bidx, t);
}

void XFormNode::use_animation(const char *aname, const char *bname, float t)
{
	int aidx = anm_find_animation(anm, aname);
	int bidx = anm_find_animation(anm, bname);

	if(aidx == -1) {
		use_animation(bidx);
	}
	if(bidx == -1) {
		use_animation(aidx);
	}
	anm_use_animations(anm, aidx, bidx, t);
}

int XFormNode::get_active_animation_index(int which) const
{
	return anm_get_active_animation_index(anm, which);
}

float XFormNode::get_active_animation_mix() const
{
	return anm_get_active_animation_mix(anm);
}

int XFormNode::get_animation_count() const
{
	return anm_get_animation_count(anm);
}

void XFormNode::add_animation(const char *name)
{
	int idx = get_animation_count();

	anm_add_animation(anm);
	use_animation(idx);

	if(name) {
		set_animation_name(name);
	}
}

void XFormNode::set_animation_name(const char *name)
{
	anm_set_active_animation_name(anm, name);
}

const char *XFormNode::get_animation_name() const
{
	return anm_get_active_animation_name(anm);
}

static const int track_type_base[] = {ANM_TRACK_POS_X, ANM_TRACK_ROT_X, ANM_TRACK_SCL_X};
static const int track_type_nelem[] = {3, 4, 3};

int XFormNode::get_key_count(int trackid) const
{
	struct anm_animation *anim = anm_get_active_animation(anm, 0);
	return anim->tracks[track_type_base[trackid]].count;
}

int XFormNode::get_position_key_count() const
{
	return get_key_count(POSITION_TRACK);
}

int XFormNode::get_rotation_key_count() const
{
	return get_key_count(ROTATION_TRACK);
}

int XFormNode::get_scaling_key_count() const
{
	return get_key_count(SCALING_TRACK);
}

long XFormNode::get_key_time(int trackid, int idx) const
{
	struct anm_animation *anim = anm_get_active_animation(anm, 0);
	struct anm_keyframe *key = anm_get_keyframe(anim->tracks + track_type_base[trackid], idx);
	return ANM_TM2MSEC(key->time);
}

long XFormNode::get_position_key_time(int idx) const
{
	return get_key_time(POSITION_TRACK, idx);
}

long XFormNode::get_rotation_key_time(int idx) const
{
	return get_key_time(ROTATION_TRACK, idx);
}

long XFormNode::get_scaling_key_time(int idx) const
{
	return get_key_time(SCALING_TRACK, idx);
}

int XFormNode::get_key_value(int trackid, int idx, float *val) const
{
	struct anm_animation *anim = anm_get_active_animation(anm, 0);

	int nelem = track_type_nelem[trackid];
	for(int i=0; i<nelem; i++) {
		struct anm_keyframe *key = anm_get_keyframe(anim->tracks + track_type_base[trackid] + i, idx);
		val[i] = key->val;
	}
	return nelem;
}

Vector3 XFormNode::get_position_key_value(int idx) const
{
	float val[3];
	get_key_value(POSITION_TRACK, idx, val);
	return Vector3(val[0], val[1], val[2]);
}

Quaternion XFormNode::get_rotation_key_value(int idx) const
{
	float val[4];
	get_key_value(ROTATION_TRACK, idx, val);
	return Quaternion(val[3], val[0], val[1], val[2]);
}

Vector3 XFormNode::get_scaling_key_value(int idx) const
{
	float val[3];
	get_key_value(SCALING_TRACK, idx, val);
	return Vector3(val[0], val[1], val[2]);
}

void XFormNode::set_position(const Vector3 &pos, long tmsec)
{
	anm_set_position(anm, v3_cons(pos.x, pos.y, pos.z), ANM_MSEC2TM(tmsec));
}

Vector3 XFormNode::get_node_position(long tmsec) const
{
	vec3_t p = anm_get_node_position(anm, ANM_MSEC2TM(tmsec));
	return Vector3(p.x, p.y, p.z);
}

void XFormNode::set_rotation(const Quaternion &quat, long tmsec)
{
	anm_set_rotation(anm, quat_cons(quat.s, quat.v.x, quat.v.y, quat.v.z), ANM_MSEC2TM(tmsec));
}

Quaternion XFormNode::get_node_rotation(long tmsec) const
{
	quat_t q = anm_get_node_rotation(anm, ANM_MSEC2TM(tmsec));
	return Quaternion(q.w, q.x, q.y, q.z);
}

void XFormNode::set_scaling(const Vector3 &pos, long tmsec)
{
	anm_set_scaling(anm, v3_cons(pos.x, pos.y, pos.z), ANM_MSEC2TM(tmsec));
}

Vector3 XFormNode::get_node_scaling(long tmsec) const
{
	vec3_t s = anm_get_node_scaling(anm, ANM_MSEC2TM(tmsec));
	return Vector3(s.x, s.y, s.z);
}

// these take hierarchy into account
Vector3 XFormNode::get_position(long tmsec) const
{
	vec3_t v = anm_get_position(anm, ANM_MSEC2TM(tmsec));
	return Vector3(v.x, v.y, v.z);
}

Quaternion XFormNode::get_rotation(long tmsec) const
{
	quat_t q = anm_get_rotation(anm, tmsec);
	return Quaternion(q.w, q.x, q.y, q.z);
}

Vector3 XFormNode::get_scaling(long tmsec) const
{
	vec3_t v = anm_get_scaling(anm, ANM_MSEC2TM(tmsec));
	return Vector3(v.x, v.y, v.z);
}

void XFormNode::set_pivot(const Vector3 &pivot)
{
	anm_set_pivot(anm, v3_cons(pivot.x, pivot.y, pivot.z));
}

Vector3 XFormNode::get_pivot() const
{
	vec3_t p = anm_get_pivot(anm);
	return Vector3(p.x, p.y, p.z);
}

void XFormNode::set_local_matrix(const Matrix4x4 &mat)
{
	local_matrix = mat;
}

const Matrix4x4 &XFormNode::get_local_matrix() const
{
	return local_matrix;
}

void XFormNode::set_bone_matrix(const Matrix4x4 &bmat)
{
	bone_matrix = bmat;
}

const Matrix4x4 &XFormNode::get_bone_matrix() const
{
	return bone_matrix;
}

#define FOO

void XFormNode::get_node_xform(long tmsec, Matrix4x4 *mat, Matrix4x4 *inv_mat) const
{
	anm_time_t tm = ANM_MSEC2TM(tmsec);

	if(mat) {
		anm_get_node_matrix(anm, (scalar_t(*)[4])mat, tm);
#ifdef FOO
		*mat = local_matrix * *mat;
#else
		*mat = *mat * local_matrix;
#endif
	}
	if(inv_mat) {
		anm_get_inv_matrix(anm, (scalar_t(*)[4])inv_mat, tm);
	}
}

void XFormNode::get_xform(long tmsec, Matrix4x4 *mat, Matrix4x4 *inv_mat) const
{
	anm_time_t tm = ANM_MSEC2TM(tmsec);

	if(mat) {
		anm_get_matrix(anm, (scalar_t(*)[4])mat, tm);
#ifdef FOO
		*mat = local_matrix * *mat;
#else
		*mat = *mat * local_matrix;
#endif
	}
	if(inv_mat) {
		anm_get_inv_matrix(anm, (scalar_t(*)[4])inv_mat, tm);
	}
}


// ---- Track ----

Track::Track()
{
	trk = new anm_track;
	anm_init_track(trk);
}

Track::~Track()
{
	anm_destroy_track(trk);
	delete trk;
}

Track::Track(const Track &rhs)
{
	trk = new anm_track;
	anm_init_track(trk);
	anm_copy_track(trk, rhs.trk);
	interp = rhs.interp;
	extrap = rhs.extrap;
}

Track &Track::operator =(const Track &rhs)
{
	if(&rhs == this) {
		return *this;
	}

	anm_copy_track(trk, rhs.trk);
	interp = rhs.interp;
	extrap = rhs.extrap;
	return *this;
}


void Track::set_interpolator(Interp in)
{
	anm_set_track_interpolator(trk, track_interpolator(in));
	interp = in;
}

Interp Track::get_interpolator() const
{
	return interp;
}

void Track::set_extrapolator(Extrap ex)
{
	anm_set_track_extrapolator(trk, track_extrapolator(ex));
	extrap = ex;
}

Extrap Track::get_extrapolator() const
{
	return extrap;
}

void Track::set_default(double def)
{
	anm_set_track_default(trk, def);
}

void Track::set_value(float val, long tmsec)
{
	anm_set_value(trk, ANM_MSEC2TM(tmsec), val);
}

float Track::get_value(long tmsec) const
{
	return anm_get_value(trk, ANM_MSEC2TM(tmsec));
}

float Track::operator ()(long tmsec) const
{
	return anm_get_value(trk, ANM_MSEC2TM(tmsec));
}


// ---- Track3 ----

void Track3::set_interpolator(Interp in)
{
	for(int i=0; i<3; i++) {
		track[i].set_interpolator(in);
	}
}

Interp Track3::get_interpolator() const
{
	return track[0].get_interpolator();
}

void Track3::set_extrapolator(Extrap ex)
{
	for(int i=0; i<3; i++) {
		track[i].set_extrapolator(ex);
	}
}

Extrap Track3::get_extrapolator() const
{
	return track[0].get_extrapolator();
}

void Track3::set_default(const Vector3 &def)
{
	for(int i=0; i<3; i++) {
		track[i].set_default(def[i]);
	}
}

void Track3::set_value(const Vector3 &val, long tmsec)
{
	for(int i=0; i<3; i++) {
		track[i].set_value(val[i], tmsec);
	}
}

Vector3 Track3::get_value(long tmsec) const
{
	return Vector3(track[0](tmsec), track[1](tmsec), track[2](tmsec));
}

Vector3 Track3::operator ()(long tmsec) const
{
	return Vector3(track[0](tmsec), track[1](tmsec), track[2](tmsec));
}


static inline anm_interpolator track_interpolator(Interp in)
{
	switch(in) {
	case INTERP_STEP:
		return ANM_INTERP_STEP;
	case INTERP_LINEAR:
		return ANM_INTERP_LINEAR;
	case INTERP_CUBIC:
		return ANM_INTERP_CUBIC;
	}

	assert(0);
	return ANM_INTERP_STEP;
}

static inline anm_extrapolator track_extrapolator(Extrap ex)
{
	switch(ex) {
	case EXTRAP_EXTEND:
		return ANM_EXTRAP_EXTEND;
	case EXTRAP_CLAMP:
		return ANM_EXTRAP_CLAMP;
	case EXTRAP_REPEAT:
		return ANM_EXTRAP_REPEAT;
	}

	assert(0);
	return ANM_EXTRAP_EXTEND;
}

