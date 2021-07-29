#ifndef CHARHEAD_H
#define CHARHEAD_H

#include "fcs.h"
class limb;

//!class character;

class character_head_fcs : public force_control_system
  {
  public:
    character_head_fcs(character * bob, anim_id_t _neck_base_id, anim_id_t _head_id);
    virtual ~character_head_fcs() {}

    void controller_set_neck_target_extend(rational_t t)
      {
      if (!locked)
        neck_target_extend = t;
      }

    void controller_set_neck_target_theta(rational_t t)
      {
      if (!locked)
        neck_target_theta = t;
      }

    void controller_set_neck_target_psi(rational_t t)
      {
      if (!locked)
        neck_target_psi = t;
      }

    void controller_set_head_target_psi(rational_t t)
      {
      if (!locked)
        head_target_psi = t;
      }

    void controller_set_head_target_phi(rational_t t)
      {
      if (!locked)
        head_target_phi = t;
      }

    void controller_set_jaw_target_psi(rational_t t)
      {
      if (!locked)
        jaw_target_psi = t;
      }


    void controller_set_theta_control_psi_offset(rational_t t)
      {
      if (!locked)
        theta_control_psi_offset = t;
      }


    // These are a higher level way of specifying the above angles.  The fcs will compute them to get 
    // the desired result
    void controller_set_abs_look_at(vector3d const & target);
    void controller_set_abs_look_at(character * chr);
    void controller_set_rel_look_at(vector3d const & target);

    virtual void frame_advance(time_value_t time_inc);

  protected:
    character * owner;
    rational_t neck_target_extend, neck_target_theta, neck_target_psi, head_target_psi, head_target_phi, jaw_target_psi;
    rational_t theta_control_psi_offset;
    vector <int> neck_ids;
    vector <limb *> neck_ptrs;
    int head_id, jaw_id;
    int seg_count;

    virtual void reset_targets();
  };

#endif