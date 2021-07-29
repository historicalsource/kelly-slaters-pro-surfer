#include "ik_object.h"

void ik_object::InitIK(entity *floor, conglomerate *biped)
{
	floor_obj = floor;
	biped_obj = biped;
	assert(biped_obj != NULL);

	pelvis = biped_obj->get_member("BIP01 PELVIS");
	l_thigh = biped_obj->get_member("BIP01 L THIGH");
	l_calf = biped_obj->get_member("BIP01 L CALF");
	l_foot = biped_obj->get_member("BIP01 L FOOT");
	r_thigh = biped_obj->get_member("BIP01 R THIGH");
	r_calf = biped_obj->get_member("BIP01 R CALF");
	r_foot = biped_obj->get_member("BIP01 R FOOT");

	float thigh_len = l_calf->get_rel_position().length();
	float calf_len = l_foot->get_rel_position().length();
	float inv_2_thigh_len = 0.5f/thigh_len;
	float inv_2_calf_len = 0.5f/calf_len;
	float thigh_len2_calf_len2 = thigh_len*thigh_len - calf_len*calf_len;

	l_thigh_ik_invc = thigh_len2_calf_len2 * inv_2_thigh_len;
	l_thigh_ik_c = inv_2_thigh_len;
	l_calf_ik_invc = -thigh_len2_calf_len2 * inv_2_calf_len;
	l_calf_ik_c = inv_2_calf_len;

	thigh_len = r_calf->get_rel_position().length();
	calf_len = r_foot->get_rel_position().length();
	inv_2_thigh_len = 0.5f/thigh_len;
	inv_2_calf_len = 0.5f/calf_len;
	thigh_len2_calf_len2 = thigh_len*thigh_len - calf_len*calf_len;

	r_thigh_ik_invc = thigh_len2_calf_len2 * inv_2_thigh_len;
	r_thigh_ik_c = inv_2_thigh_len;
	r_calf_ik_invc = -thigh_len2_calf_len2 * inv_2_calf_len;
	r_calf_ik_c = inv_2_calf_len;
}


float g_doik = true;
float g_ik_delta = 0.05f;
float g_ik_add = -0.04f;
extern float g_frame_by_frame;
float g_min_foot_height = 0.095f;
void ik_object::PerformIK(void)
{
	assert(floor_obj != NULL);

	// Do the leg IK
	vector3d bpos =  floor_obj->get_abs_position();
	vector3d rfoot_pos;
	vector3d lfoot_pos;
	vector3d r_foot_dir;
	vector3d l_foot_dir;

	vector3d calf_vec, thigh_vec, thigh_foot_vec, cvec, cvec2, bone_vec;
	bone_vec = vector3d(1, 0, 0);

	lfoot_pos = floor_obj->get_abs_po().non_affine_inverse_xform(l_foot->get_abs_position() - bpos);
	rfoot_pos = floor_obj->get_abs_po().non_affine_inverse_xform(r_foot->get_abs_position() - bpos);

	if (rfoot_pos.y < g_min_foot_height)
		rfoot_pos.y = g_min_foot_height;

	if (lfoot_pos.y < g_min_foot_height)
		lfoot_pos.y = g_min_foot_height;

	lfoot_pos = bpos + floor_obj->get_abs_po().non_affine_slow_xform(lfoot_pos);
	rfoot_pos = bpos + floor_obj->get_abs_po().non_affine_slow_xform(rfoot_pos);


	calf_vec = l_foot->get_abs_position() - l_calf->get_abs_position();
	thigh_vec = l_calf->get_abs_position() - l_thigh->get_abs_position();
	thigh_foot_vec = l_foot->get_abs_position() - l_thigh->get_abs_position();
	cvec = cross(thigh_vec, calf_vec);
	cvec.normalize();
	cvec2 = cross(thigh_foot_vec, cvec);
	cvec2.normalize();
	l_foot_dir = l_foot->get_abs_po().non_affine_slow_xform(vector3d (0,0,-1));
	l_foot_dir -= cvec*dot(cvec, l_foot_dir);
	l_foot_dir.normalize();
	if (dot(l_foot_dir, cvec2) <= g_ik_delta)
	{
		l_foot_dir -= (cvec2*dot(cvec2, l_foot_dir) + g_ik_add*cvec2);
	}

	if (g_doik)
		do_leg_IK (false, lfoot_pos, l_foot_dir);

	calf_vec = r_foot->get_abs_position() - r_calf->get_abs_position();
	thigh_vec = r_calf->get_abs_position() - r_thigh->get_abs_position();
	thigh_foot_vec = r_foot->get_abs_position() - r_thigh->get_abs_position();
	cvec = cross(thigh_vec, calf_vec);
	cvec.normalize();
	cvec2 = cross(thigh_foot_vec, cvec);
	cvec2.normalize();
	r_foot_dir = r_foot->get_abs_po().non_affine_slow_xform(vector3d (0,0,-1));
	r_foot_dir -= cvec*dot(cvec, r_foot_dir);
	r_foot_dir.normalize();
	if (dot(r_foot_dir, cvec2) <= g_ik_delta)
	{
		r_foot_dir -= (cvec2*dot(cvec2, r_foot_dir) + g_ik_add*cvec2);
	}

	if (g_doik)
		do_leg_IK (true, rfoot_pos, r_foot_dir);
}

void ik_object::do_leg_IK (bool right_leg, const vector3d& foot_position, const vector3d& foot_direction)
{
	entity *thigh, *calf, *foot;
	float thigh_ik_invc;
	float thigh_ik_c;
	float calf_ik_invc;
	float calf_ik_c;

	if (right_leg)
	{
		calf = r_calf;
		thigh = r_thigh;
		foot = r_foot;
		thigh_ik_invc = r_thigh_ik_invc;
		thigh_ik_c = r_thigh_ik_c;
		calf_ik_invc = r_calf_ik_invc;
		calf_ik_c = r_calf_ik_c;
	}
	else
	{
		calf = l_calf;
		thigh = l_thigh;
		foot = l_foot;
		thigh_ik_invc = l_thigh_ik_invc;
		thigh_ik_c = l_thigh_ik_c;
		calf_ik_invc = l_calf_ik_invc;
		calf_ik_c = l_calf_ik_c;
	}

	float su, cu, sl, cl;
	vector3d dir = foot_position - thigh->get_abs_position();

	float C, inv_C;

	C = dir.length();
	inv_C = 1.0f/C;
	dir *= inv_C;

	cu = thigh_ik_invc * inv_C + thigh_ik_c * C;
	cl = calf_ik_invc * inv_C + calf_ik_c * C;

	if (cu < -1.0f) cu=-1.0f;
	else if (cu > 1.0f) cu=1.0f;
	if (cl < -1.0f) cl=-1.0f;
	else if (cl > 1.0f) cl=1.0f;

	su = __fsqrt(1-cu*cu);
	sl = __fsqrt(1-cl*cl);

	vector3d up, right;

	right = cross (dir, foot_direction);
	right.normalize ();
	up = cross (right, dir);

	po parent_world, rot, mchg (dir, up, right, vector3d (0,0,0)), fix (1.0f, 0.0f, 0.0f,	  0.0f, 0.0f, -1.0f,		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	cu = -cu;
	su = -su;
	cl = -cl;
	sl = -sl;

	// thigh
	rot = po (cu, su, 0, -su, cu, 0, 0,0,1, 0,0,0);
	parent_world = pelvis->get_abs_po ();
	parent_world.set_position (vector3d (0,0,0));
	parent_world = parent_world.inverse ();
	thigh->set_rel_orientation ((fix*rot * (mchg * parent_world)).get_orientation ());

	// calf
	rot = po (cl, -sl, 0, sl, cl, 0, 0,0,1, 0,0,0);
	parent_world = thigh->get_abs_po ();
	parent_world.set_position (vector3d (0,0,0));
	parent_world = parent_world.inverse ();
	calf->set_rel_orientation ((fix*rot * (mchg * parent_world)).get_orientation ());

	// foot
	/*up = cross (right, foot_direction);
	rot = po (up, -right, -1*foot_direction, vector3d (0,0,0));
	parent_world = calf->get_abs_po ();
	parent_world.set_position (vector3d (0,0,0));
	parent_world = parent_world.inverse ();
	foot->set_rel_orientation ((rot * parent_world).get_orientation ());*/
}