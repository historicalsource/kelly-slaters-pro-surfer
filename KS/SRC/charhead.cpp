////////////////////////////////////////////////////////////////////////////////
/*
  charhead.cpp

  Home of the character_head_fcs class, the force control system which will direct
  the heads of various characters.  

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
//!#include "charhead.h"
#include "actuator.h"
#include "hwmath.h"
#include "msgboard.h"
#include "wds.h"
#include "geometry.h"
#include "hinge.h"
//!#include "attrib.h"
//!#include "limb.h"

////////////////////////////////////////////////////////////////////////////////
//    character_head_fcs
////////////////////////////////////////////////////////////////////////////////

// This fcs currently works for only a simple head, that is, one neck segment and one head segment.
// We expect to generalize it to a multi-jointed neck (e.g. Dragon)

//  Public functions

character_head_fcs::character_head_fcs(character * bob, anim_id_t _neck_base_id, anim_id_t _head_id) : force_control_system()
{
  owner = bob;
  neck_ids.push_back(_neck_base_id);
  neck_ptrs.push_back(owner->limb_ptr(neck_ids[neck_ids.size()-1]));
  head_id = _head_id; //owner->limb_ptr(neck_id)->get_first_child()->get_id();
  limb* neck_base = owner->limb_ptr( _neck_base_id );
  vector<limb*>::const_iterator i = neck_base->get_children().begin();
  vector<limb*>::const_iterator i_end = neck_base->get_children().end();
  limb* next_neck = NULL;
  // if the base neck limb has a child that is not a head or a shoulder (the
  // latter is only applicable to Character Studio Bipeds), assume it's another
  // neck segment
  for ( ; i!=i_end; ++i )
  {
    int id = (*i)->get_id();
    if ( id!=head_id && id!=LEFT_SHOULDER && id!=RIGHT_SHOULDER )
    {
      next_neck = *i;
      break;
    }
  }
  // if there are multiple neck segments, assume each segment's first child is
  // either the head (terminal node) or another neck segment
  if ( next_neck )
  {
    int neck_id = next_neck->get_id();
    while ( neck_id != head_id )
    {
      neck_ptrs.push_back(next_neck);
      neck_ids.push_back(neck_id);
      stringx name = (next_neck->get_body()->get_id()).get_val();
      next_neck = next_neck->get_first_child();
      neck_id = next_neck->get_id();
    }
  }

  limb * jaw;
  if (owner->limb_valid(JAW) && ((jaw = owner->limb_ptr(JAW))->get_parent() == owner->limb_ptr(head_id) ) )
  {
    jaw_id = jaw->get_id();
    /*
    // kludge to allow biting  E399
    if (jaw_id!=JAW)
      jaw_id = -1;
      */
  }
  else
    jaw_id = -1;
  reset_targets();
  theta_control_psi_offset = 0;
}


void character_head_fcs::frame_advance(time_value_t time_inc)
{
  if(owner->is_flagged(EFLAG_INVISIBLE_PHYSICS) && !owner->do_invisible_physics())
    return;

  force_control_system::frame_advance(time_inc);
  rational_t neck_fraction = 1.0f/neck_ids.size();
  if (owner->limb_valid(head_id) && (neck_target_extend || neck_target_theta || neck_target_psi || head_target_psi || head_target_phi))
  {
    vector <limb *>::iterator np;
    vector <int>::iterator idx;

    rational_t head_psi_low;
    rational_t head_psi_high;
    rational_t head_theta_high;
    rational_t head_psi_body_scale;
    rational_t head_theta_body_scale;
    head_psi_high = owner->get_hard_attrib()->get_head_psi_high();
    head_psi_low = owner->get_hard_attrib()->get_head_psi_low();
    head_theta_high = owner->get_hard_attrib()->get_head_theta_high();
    head_psi_body_scale = owner->get_hard_attrib()->get_head_psi_body_scale();
    head_theta_body_scale = owner->get_hard_attrib()->get_head_theta_body_scale();

    for (np=neck_ptrs.begin(), idx=neck_ids.begin();np!=neck_ptrs.end();np++, idx++)
      (*np) = owner->limb_ptr(*idx);
    //limb * neck = owner->limb_ptr(neck_ids[0]);
    limb * head = owner->limb_ptr(head_id);
    limb * jaw = NULL;
    if (jaw_id!=-1)
       jaw = owner->limb_ptr(jaw_id);

    rational_t neck_psi_fraction = (neck_ptrs.size()==0)?1.0f:0.5f;
    int i=0;
    for (i=0, np=neck_ptrs.begin();np!=neck_ptrs.end();np++, i++)
    {
      limb * neck = *np;
      // Dragon kludge
      rational_t theta_scale = (neck_ptrs.size()>1)?1.0f:(3.0f/4.0f);

      if (neck_ptrs.size()>1)
      {
        rational_t sinx, cosx;
        fast_sin_cos_approx( neck_target_extend+theta_control_psi_offset, &sinx, &cosx );
        neck->get_hinge(0)->get_actuator()->fcs_adjust_target(neck_fraction * neck_target_theta*theta_scale*cosx);
        neck->get_hinge(2)->get_actuator()->fcs_adjust_target(neck_fraction * neck_target_theta*theta_scale*sinx);
      }
      else
      {
        neck->get_hinge(0)->get_actuator()->fcs_adjust_target(neck_fraction * neck_target_theta*theta_scale);
      }
      if (i<2)
        neck->get_hinge(1)->get_actuator()->fcs_adjust_target(neck_psi_fraction * (neck_target_extend) );
      neck->get_hinge(1)->get_actuator()->fcs_adjust_target(neck_fraction * (neck_target_psi));
    }
    head->get_hinge(0)->get_actuator()->fcs_adjust_target(head_target_psi);
    head->get_hinge(1)->get_actuator()->fcs_adjust_target(head_target_phi);

    if (jaw)
    {
      rational_t target_psi = jaw_target_psi*owner->get_hard_attrib()->get_head_jaw_open_psi();
      jaw->get_hinge(0)->get_actuator()->fcs_adjust_target(target_psi);
    }
    limb * abdomen = owner->limb_ptr(ABDOMEN);
    limb * chest = owner->limb_ptr(CHEST);

    chest->get_hinge(1)->get_actuator()->fcs_adjust_target(head_psi_body_scale*neck_target_psi/2);
    abdomen->get_hinge(1)->get_actuator()->fcs_adjust_target(head_psi_body_scale*neck_target_psi/2);

    chest->get_hinge(0)->get_actuator()->fcs_adjust_target(head_theta_body_scale*neck_target_theta/8);
    abdomen->get_hinge(0)->get_actuator()->fcs_adjust_target(head_theta_body_scale*neck_target_theta/8);
  }

  reset_targets();
}


void character_head_fcs::controller_set_abs_look_at(vector3d const & target)
{
  if (!locked)
  {
//    controller_set_rel_look_at(owner->get_abs_po().inverse_xform(target));
    controller_set_rel_look_at(owner->get_abs_po().fast_inverse_xform(target));
  }
}


void character_head_fcs::controller_set_abs_look_at(character * chr)
{
  if (!locked)
  {
    vector3d src_pos = owner->limb_ptr(HEAD)->get_body()->get_abs_position();

//    controller_set_rel_look_at(owner->get_abs_po().non_affine_inverse_xform(chr->limb_ptr(HEAD)->get_body()->get_abs_position()-src_pos));
    controller_set_rel_look_at(owner->get_abs_po().fast_non_affine_inverse_xform(chr->limb_ptr(HEAD)->get_body()->get_abs_position()-src_pos));
  }
}


void character_head_fcs::controller_set_rel_look_at(vector3d const & target)
{
  rational_t theta, psi;

  if (!locked)
  {
    rational_t head_psi_low;
    rational_t head_psi_high;
    rational_t head_theta_high;
    rational_t head_psi_body_scale;
    rational_t head_theta_body_scale;
    head_psi_high = owner->get_hard_attrib()->get_head_psi_high();
    head_psi_low = owner->get_hard_attrib()->get_head_psi_low();
    head_theta_high = owner->get_hard_attrib()->get_head_theta_high();
    head_psi_body_scale = owner->get_hard_attrib()->get_head_psi_body_scale();
    head_theta_body_scale = owner->get_hard_attrib()->get_head_theta_body_scale();

    theta = safe_atan2(-target.x, target.z);
    psi = safe_atan2(target.y, target.xz_length());
    limb * waist = owner->limb_ptr(WAIST);
    limb * abdomen = owner->limb_ptr(ABDOMEN);
    limb * chest = owner->limb_ptr(CHEST);
    rational_t body_theta_offset = chest->get_hinge(0)->get_val() + abdomen->get_hinge(0)->get_val() + waist->get_hinge(0)->get_val();

    if (theta>-head_theta_high*1.1f+body_theta_offset &&
        theta<head_theta_high*1.1f+body_theta_offset)
    {
      if (theta>head_theta_high+body_theta_offset) 
        theta = head_theta_high+body_theta_offset;
      else if (theta<-head_theta_high+body_theta_offset)
        theta = -head_theta_high+body_theta_offset;
      if (psi>head_psi_high)
        psi = head_psi_high;
      else if (psi<head_psi_low)
        psi = head_psi_low;

      //neck_target_extend = 0;
      neck_target_theta += theta;
      neck_target_psi += psi*0.5F;
      head_target_psi += psi*0.5F;
      head_target_phi = 0;
    }
    else
      reset_targets();
  }
}

void character_head_fcs::reset_targets()
{
  if (!reset_locked)
  {
    neck_target_extend = 0;
    neck_target_theta = 0;
    neck_target_psi = 0;
    head_target_psi = -neck_target_extend;
    head_target_phi = 0;
    jaw_target_psi = 0;
    theta_control_psi_offset = 0;
  }
}
