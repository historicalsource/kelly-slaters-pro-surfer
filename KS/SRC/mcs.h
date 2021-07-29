#ifndef MCS_H
#define MCS_H

#include "mobject.h"
#include "entity.h"
#include "capsule.h"
#include "wds.h"
//!class actor;
//!class character;
class ladder;
class entity_anim;

class motion_control_system:public motion_object
  {
  public:
    motion_control_system();
    virtual ~motion_control_system();

    virtual void frame_advance(time_value_t time_inc)=0;
  protected:
    entity * ent;
  };


//  a motion control system ideal for mouse-look
class theta_and_psi_mcs : public motion_control_system
  {
  public:
    theta_and_psi_mcs(entity* _ent, rational_t _theta=0, rational_t _psi=0);
    virtual ~theta_and_psi_mcs() {}
    
    void set_pan_for_next_frame(rational_t d_theta)
      {
      d_theta_for_next_frame = d_theta;
      }

    void set_tilt_for_next_frame(rational_t d_psi )
      {
      d_psi_for_next_frame = d_psi;
      }

    virtual void frame_advance(time_value_t time_inc);

  private:
    rational_t theta;
    rational_t psi;
    rational_t d_theta_for_next_frame;
    rational_t d_psi_for_next_frame;
  };

/*!
enum
{
  ARM_ONLY = 0,
  UPPER_BODY,
  PROCEDURAL_ARM_ONLY
};
class auto_aim_arm_mcs : public motion_control_system
  {
  private:
    int aim_type;
    virtual void frame_advance1(time_value_t time_inc);
    virtual void frame_advance2(time_value_t time_inc);

  public:
    auto_aim_arm_mcs(entity* _ent);
    virtual ~auto_aim_arm_mcs() {}
    
    virtual void frame_advance(time_value_t time_inc);

    virtual void set_active(bool yorn);

    void set_aim_type( int _aim_type ) { aim_type = _aim_type;}
    int get_aim_type() const { return aim_type;}
  };
!*/

class dolly_and_strafe_mcs : public motion_control_system
  {
  public:
    dolly_and_strafe_mcs(entity* _ent) : motion_control_system(),dolly(0),strafe(0),lift(0) {ent=_ent;}
    virtual ~dolly_and_strafe_mcs() {}

    void set_dolly_for_next_frame(rational_t _dolly) {dolly=_dolly;}
    void set_strafe_for_next_frame(rational_t _strafe) {strafe=_strafe;}
    void set_lift_for_next_frame(rational_t _lift) {lift=_lift;}
    virtual void frame_advance(time_value_t time_inc);
  private:
    rational_t dolly;
    rational_t strafe;
    rational_t lift;

    // camera movement
    void do_dolly(rational_t distance);  // positive forward, negative backward
    void do_lift( rational_t dist);
    void do_strafe( rational_t dist);
  };

/*!
class character_ladder_mcs : public motion_control_system
  {
  public:
    character_ladder_mcs( character* _ent );

    virtual ~character_ladder_mcs();

    virtual void frame_advance( time_value_t time_inc );

    // This function engages the owner to the ladder.
    virtual void controller_engage(ladder * lad);

  protected:
    // Let go of the ladder
    virtual void controller_disengage();

    character* owner;
    ladder* cur_ladder;

    enum phase_t
      {
      PHASE_NONE = 0,
      MOUNT_TRANSITION,
      MOUNT,
      WAIT_MOUNT,
      CLIMB_UP,
      CLIMB_DOWN,
      ADVANCE_CLIMB,
      DISMOUNT,
      WAIT_DISMOUNT
      };

    phase_t phase;
    phase_t last_phase;

    entity_anim_tree *cur_anim;

    virtual void kill_cur_anim() { if(cur_anim) { g_world_ptr->kill_anim(cur_anim); cur_anim = NULL; } }

    virtual void set_phase(phase_t p);

    // transition phase variables
    vector3d owner_pos, mount_pos;
    rational_t owner_theta, mount_theta;

    // climbing variables
    vector3d start_pos, end_pos;
    vector3d force_dir;
    float travel_dist;

    time_value_t next_anim_start_time;

    bool going_up;
    bool using_right_arm;
    bool controller_active_state;
    bool owner_physical_state;
    bool owner_collision_state;


    virtual bool climb_up( time_value_t time_inc );
    virtual bool climb_down( time_value_t time_inc );
    virtual bool advance_climb( time_value_t time_inc );
    virtual bool mount( time_value_t time_inc );
    virtual bool wait_mount( time_value_t time_inc );
    virtual bool dismount( time_value_t time_inc );
    virtual bool wait_dismount( time_value_t time_inc );
    virtual bool mount_transition( time_value_t time_inc );

    // vertical ladder
    virtual bool v_ladder_climb_up( time_value_t time_inc );
    virtual bool v_ladder_climb_down( time_value_t time_inc );
    virtual bool v_ladder_advance_climb( time_value_t time_inc );
    virtual bool v_ladder_mount( time_value_t time_inc );
    virtual bool v_ladder_wait_mount( time_value_t time_inc );
    virtual bool v_ladder_dismount( time_value_t time_inc );
    virtual bool v_ladder_wait_dismount( time_value_t time_inc );
    virtual bool v_ladder_mount_transition( time_value_t time_inc );

    // horizontal ladder
    virtual bool h_ladder_climb_up( time_value_t time_inc );
    virtual bool h_ladder_climb_down( time_value_t time_inc );
    virtual bool h_ladder_advance_climb( time_value_t time_inc );
    virtual bool h_ladder_mount( time_value_t time_inc );
    virtual bool h_ladder_wait_mount( time_value_t time_inc );
    virtual bool h_ladder_dismount( time_value_t time_inc );
    virtual bool h_ladder_wait_dismount( time_value_t time_inc );
    virtual bool h_ladder_mount_transition( time_value_t time_inc );

    // zip line
    virtual bool zip_line_climb_up( time_value_t time_inc );
    virtual bool zip_line_climb_down( time_value_t time_inc );
    virtual bool zip_line_advance_climb( time_value_t time_inc );
    virtual bool zip_line_mount( time_value_t time_inc );
    virtual bool zip_line_wait_mount( time_value_t time_inc );
    virtual bool zip_line_dismount( time_value_t time_inc );
    virtual bool zip_line_wait_dismount( time_value_t time_inc );
    virtual bool zip_line_mount_transition( time_value_t time_inc );
  };
!*/
#endif
