#include "global.h"

#include "physical_interface.h"
#include "guidance_sys.h"
#include "entity.h"
#include "wds.h"
#include "terrain.h"
#if defined(TARGET_XBOX)
#include "profiler.h"
#endif /* TARGET_XBOX JIV DEBUG */


void guidance_system::launch(const vector3d &dir, rational_t force)
{
  assert(owner && owner->is_enabled() && !owner->is_suspended());

  owner->set_velocity(ZEROVEC);
  owner->set_angular_velocity(ZEROVEC);
  owner->set_acceleration_factor(ZEROVEC);
  owner->set_acceleration_correction_factor(ZEROVEC);
  owner->set_last_acceleration_correction_factor(ZEROVEC);

  owner->apply_force_increment(dir*force, physical_interface::INSTANT);
}





rocket_guidance_sys::rocket_guidance_sys(physical_interface *_owner)
  : guidance_system(_owner)
{
  target = NULL;
  target_pos = ZEROVEC;
  launch_force = 0.0f;

  wobble_timer = 0.0f;
  guidance_delay = 0.0f;
  accel_delay = 0.0f;

  guided_accuracy = 1.0f;
  turn_factor = 0.35f;
  accel_factor = 0.0f;

  full_wobble_timer = 2.0f;
  full_accel_delay = 0.0f;
  full_guidance_delay = 0.0f;

  wobble_timer_var = 0.0f;
  guidance_delay_var = 0.0f;
  accel_delay_var = 0.0f;
}

rocket_guidance_sys::~rocket_guidance_sys()
{
}

void rocket_guidance_sys::wobble()
{
  assert(wobble_timer <= 0.0f);

  vector3d axis = vector3d(PLUS_MINUS_ONE, PLUS_MINUS_ONE, PLUS_MINUS_ONE);
  axis.normalize();

  owner->set_angular_velocity(axis * (PI * get_turn_factor()) * ((random(2)) ? 1 : -1));
  wobble_timer = get_wobble_timer_with_var();
}

void rocket_guidance_sys::launch(const vector3d &dir, rational_t force)
{
  launch_force = force;

  guidance_delay = get_guidance_delay_with_var();
  accel_delay = get_accel_delay_with_var();
  wobble_timer = 0.0f;

  owner->set_gravity(false);

  guidance_system::launch(dir, force);
}

void rocket_guidance_sys::frame_advance(time_value_t t)
{
  START_PROF_TIMER(proftimer_guidance_sys);

  assert(owner && owner->is_enabled() && !owner->is_suspended());

  wobble_timer -= t;
  if(wobble_timer <= 0.0f)
    wobble_timer = 0.0f;

  guidance_delay -= t;
  if(guidance_delay <= 0.0f)
    guidance_delay = 0.0f;

  accel_delay -= t;
  if(accel_delay <= 0.0f)
    accel_delay = 0.0f;

  vector3d pos = owner->get_my_entity()->get_abs_position();

  if(is_guided() && (target == NULL || !target->allow_targeting() || (!target->is_visible() && !target->is_hero())))
  {
    // acquire a NEW target if possible
    target = NULL;

    if(guidance_delay <= 0.0f)
    {
      static vector<region_node*> regs;
      regs.resize(0);

      rational_t min_len2 = -1.0f;

      // build list of regions that can be affected by blast
      build_region_list_radius(&regs, owner->get_my_entity()->get_primary_region(), pos, 20.0f);

      // build list of entities that are affected by blast
      vector<region_node*>::iterator rn = regs.begin();
      vector<region_node*>::iterator rn_end = regs.end();
      for ( ; rn!=rn_end; ++rn )
      {
        region* r = (*rn)->get_data();

        vector<entity*>::const_iterator i = r->get_entities().begin();
        vector<entity*>::const_iterator i_end = r->get_entities().end();

        for ( ; i!=i_end; ++i )
        {
          entity* e = *i;
          if ( e && (e->is_visible() || e->is_hero()) && e->allow_targeting())
          {
            rational_t len2 = (pos - e->get_abs_position()).length2();
            if ( min_len2 == -1.0f || len2 < min_len2 )
            {
              target = e;
              min_len2 = len2;
            }
          }
        }
      }
    }

    if(target == NULL)
      guidance_delay = get_guidance_delay_with_var();
  }

  if((is_point_guided() || is_guided()) && guidance_delay <= 0.0f)
  {
    if(target != NULL || is_point_guided())
    {
      if(target != NULL && is_guided())
        target_pos = target->get_abs_position();

      vector3d delta = target_pos - pos;
      vector3d face = owner->get_my_entity()->get_abs_po().get_facing();

      rational_t dist = delta.length();
      if(dist > 0.0f)
      {
        delta *= 1.0f/dist;

        face.normalize();
        rational_t dot_prod = dot(delta, face);

        if(get_guided_accuracy() < 0.0f)
        {
          po the_po = po_identity_matrix;
          the_po.set_facing(delta);
          the_po.set_position(pos);

          owner->get_my_entity()->set_rel_po(the_po);
          owner->set_angular_velocity(ZEROVEC);
        }
        else if(get_guided_accuracy() >= 1.0f)
        {
          if(dot_prod < 1.0f)
          {
            vector3d axis = cross(delta, face);
            axis.normalize();

            rational_t max_ang = t * (PI * get_turn_factor());
            rational_t ang = fast_acos(dot_prod);
            if(ang > max_ang)
              ang = max_ang;
            if(ang < -max_ang)
              ang = -max_ang;

            po rot_po = po_identity_matrix;
            rot_po.set_rot(axis, ang);

            vector3d f = rot_po.non_affine_slow_xform(face);
            assert(f.is_valid());
            po the_po = po_identity_matrix;
            the_po.set_facing(f);
            the_po.set_position(pos);

            owner->get_my_entity()->set_rel_po(the_po);
            owner->set_angular_velocity(ZEROVEC);
          }
        }
        else if(get_guided_accuracy() > 0.0f && dot_prod <= get_guided_accuracy())
        {
          vector3d axis = cross(delta, face);
          axis.normalize();
          owner->set_angular_velocity(axis * (PI * get_turn_factor()));
        }
        else if(wobble_timer <= 0.0f)
          wobble();
      }
    }
    else if(wobble_timer <= 0.0f)
      wobble();
  }

  owner->set_velocity(ZEROVEC);
  vector3d face = owner->get_my_entity()->get_abs_po().get_facing();
  face.normalize();
  owner->apply_force_increment(face*launch_force, physical_interface::INSTANT);

  if(accel_delay <= 0.0f)
    launch_force += get_accel_factor()*t;

  STOP_PROF_TIMER(proftimer_guidance_sys);
}


void rocket_guidance_sys::copy(rocket_guidance_sys *sys)
{
  set_guided(sys->is_guided());
  set_point_guided(sys->is_point_guided());

  guided_accuracy = sys->guided_accuracy;
  accel_factor = sys->accel_factor;
  turn_factor = sys->turn_factor;

  full_wobble_timer = sys->full_wobble_timer;
  full_accel_delay = sys->full_accel_delay;
  full_guidance_delay = sys->full_guidance_delay;

  wobble_timer_var = sys->wobble_timer_var;
  guidance_delay_var = sys->guidance_delay_var;
  accel_delay_var = sys->accel_delay_var;
}


void rocket_guidance_sys::set_target(entity *ent)
{
  target = ent;

  if(target)
    set_target_pos(target->get_abs_position());
}
