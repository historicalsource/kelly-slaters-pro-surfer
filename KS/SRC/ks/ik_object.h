#ifndef _IK_OBJECT_H
#define _IK_OBJECT_H

#include "types.h"
#include "entity.h"
#include "conglom.h"

class ik_object
{
public:
	ik_object(entity *floor, conglomerate *biped) { InitIK(floor, biped); }
	ik_object() { floor_obj = biped_obj = NULL; }
	~ik_object() {};
	void InitIK(entity *floor, conglomerate *biped);
	void SetFloorObj(entity *floor) { floor_obj = floor; }
	void SetBipedObj(conglomerate *biped) { biped_obj = biped; }
	void PerformIK(void);

private:
	void do_leg_IK (bool right_leg, const vector3d& foot_position, const vector3d& foot_direction);

	entity *floor_obj, *pelvis, *l_thigh, *l_calf, *l_foot, *r_thigh, *r_calf, *r_foot;
	conglomerate *biped_obj;
	float r_thigh_ik_invc;
	float r_thigh_ik_c;
	float r_calf_ik_invc;
	float r_calf_ik_c;

	float l_thigh_ik_invc;
	float l_thigh_ik_c;
	float l_calf_ik_invc;
	float l_calf_ik_c;
};

#endif // _IK_OBJECT_H