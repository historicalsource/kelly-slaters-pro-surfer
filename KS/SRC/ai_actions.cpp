#include "global.h"

#include "ai_actions.h"
#include "ai_goals.h"
#include "ai_interface.h"
#include "ai_locomotion.h"
#ifdef GCCULL
#include "ai_voice.h"
#endif
// BIGCULL #include "damage_interface.h"
#include "hard_attrib_interface.h"
#include "animation_interface.h"
//#include "sound_interface.h"
#include "physical_interface.h"
#include "entity.h"
#include "collide.h"
#include "random.h"
#include "item.h"

unsigned int ai_action::action_id_counter = 0;

ai_action::ai_action(ai_goal *own)
 : flags(0)
{
  id = ++action_id_counter;

  assert(id > 0);

  owner = own;
}

void ai_action::going_out_of_service()
{
  assert(is_in_service());
  set_flag(IN_SERVICE, false);
}

void ai_action::going_into_service()
{
  assert(!is_in_service());
  set_flag(IN_SERVICE, true);
}

ai_interface *ai_action::get_ai_interface() const
{
  return(owner->get_ai_interface());
}

entity *ai_action::get_my_entity() const
{
  return(owner->get_my_entity());
}










anim_ai_action::anim_ai_action(ai_goal *own)
  : ai_action(own)
{
  safety_checks = false;
}

//---------------------------------------------------------------
bool anim_ai_action::frame_advance( time_value_t delta_t )
{
  if(safety_checks)
  {
    vector3d hit_loc;
    if(!in_world(get_my_entity()->get_abs_position(), 0.25f, ZEROVEC, get_my_entity()->get_primary_region(), hit_loc))
    {
      if(get_my_entity()->has_parent())
      {
        vector3d posn = get_my_entity()->link_ifc()->get_parent()->get_abs_po().inverse().slow_xform(safe_pos);
        get_my_entity()->set_rel_position(posn);
      }
      else
        get_my_entity()->set_rel_position(safe_pos);
    }
    else
      safe_pos = get_my_entity()->get_abs_position();
  }

  return( get_my_entity()->anim_finished(anim_slot) );
}

//--------------------------------------------------------------
void anim_ai_action::going_out_of_service()
{
  if(looping)
    get_my_entity()->kill_anim( anim_slot );

  ai_action::going_out_of_service();
}

//--------------------------------------------------------------
void anim_ai_action::going_into_service()
{
  ai_action::going_into_service();

  get_ai_interface()->play_animation(anim_to_find, anim_slot, (ANIM_RELATIVE_TO_START | ANIM_AUTOKILL | (tween ? ANIM_TWEEN : 0) | (non_cosmetic ? ANIM_NONCOSMETIC : 0) | (looping ? ANIM_LOOPING : 0) | (reverse ? ANIM_REVERSE : 0)), &anim_damage_value, &anim_recover, &anim_recover_var, &anim_flags);

#ifdef GCCULL
  if(sound_grp.length() > 0)
  {
    static pstring snd;
    snd = sound_grp;

    get_ai_interface()->get_voice()->say(snd);
  }
#endif

  if(safety_checks)
  {
    vector3d hit_loc;
    if(get_my_entity()->get_primary_region() != NULL && in_world(get_my_entity()->get_abs_position(), 0.25f, ZEROVEC, get_my_entity()->get_primary_region(), hit_loc))
    {
      safety_checks = true;
      safe_pos = get_my_entity()->get_abs_position();
    }
    else
      safety_checks = false;
  }
}

void anim_ai_action::setup(const stringx &play_anim, int slot, const stringx &sound, bool loop, bool rev, bool _non_cosmetic, bool _tween)
{
  anim_to_find = play_anim;
  sound_grp = sound;
  anim_slot = slot;
  looping = loop;
  reverse = rev;
  non_cosmetic = _non_cosmetic;
  tween = _tween;

  anim_damage_value = 0;
  anim_recover = 0.0f;
  anim_recover_var = 0.0f;
  anim_flags = 0;
}




#if 0 // BIGCULL


attack_ai_action::attack_ai_action(ai_goal *own)
  : anim_ai_action(own)
{
  attack_type = DAMAGE_MELEE;
  ext_range = 0.0f;
  damage_mod = 1.0f;
}

bool attack_ai_action::frame_advance( time_value_t delta_t )
{
  if(get_my_entity()->signal_raised(entity::ATTACK))
  {
    get_my_entity()->clear_signal_raised(entity::ATTACK);

    entity *pc = get_ai_interface()->get_target();
    if(pc)
    {
      vector3d mypos = get_my_entity()->get_abs_position();
      vector3d pcpos = pc->get_abs_position();
      vector3d vec = (pcpos - mypos);
      rational_t len = vec.length();
      if(len < 0.000001f)
        vec = get_my_entity()->get_abs_po().get_facing();
      else
        vec /= len;

      assert(pc->has_damage_ifc());

      rational_t rad = (get_my_entity()->get_radius()+pc->get_radius())+ext_range;
      if( len < rad )
      {
        rational_t ab = angle_between( vec, get_my_entity()->get_abs_po().get_facing() );

        if( ab <= 1.570795f )
        {
          int damage = (int)((((rational_t)anim_damage_value)*damage_mod)+0.5f);
          pc->damage_ifc()->apply_damage( get_my_entity(), damage, attack_type, mypos, vec );

          if(pc->has_sound_ifc())
          {
            pstring p("IMPACT");
            pc->sound_ifc()->play_3d_sound_grp( p );
          }
        }
      }
    }
  }

  return( anim_ai_action::frame_advance( delta_t ) );
}

void attack_ai_action::going_out_of_service()
{
  get_my_entity()->raise_signal( entity::ATTACK_END );

  anim_ai_action::going_out_of_service();
//  get_ai_interface()->set_allow_wounded(true);
}

void attack_ai_action::going_into_service()
{
  anim_ai_action::going_into_service();

  get_my_entity()->raise_signal( entity::ATTACK_BEGIN );
  get_my_entity()->clear_signal_raised(entity::ATTACK);
//  get_ai_interface()->set_allow_wounded(false);
}


void attack_ai_action::setup(const stringx &play_anim, const stringx &sound, int slot, eDamageType type, rational_t dmg_mod, rational_t range)
{
  anim_ai_action::setup(play_anim, slot, sound, false, false, true);
  attack_type = type;
  ext_range = range;
  damage_mod = dmg_mod;
}




search_ai_action::search_ai_action(ai_goal *own)
  : ai_action(own)
{
}

bool search_ai_action::frame_advance( time_value_t delta_t )
{
  duration -= delta_t;

  if(get_ai_interface()->reached_dest())
  {
    bool valid = false;
    bool dir_taken[4] = { false, false, false, false };
    int num_tries = 0;
    vector3d dir;
    rational_t dist = 0.0f;

    while(!valid)
    {
      if(num_tries >= 4)
        break;

      valid = true;

      int _d = random(4);
      while(dir_taken[_d])
        _d = random(4);

      dir_taken[_d] = true;
      ++num_tries;

      switch(_d)
      {
        case 0:
          dir = get_my_entity()->get_abs_po().get_facing();
          break;

        case 1:
          dir = -get_my_entity()->get_abs_po().get_facing();
          break;

        case 2:
          dir = get_my_entity()->get_abs_po().get_x_facing();
          break;

        case 3:
          dir = -get_my_entity()->get_abs_po().get_x_facing();
          break;
      }

      po the_po;
      the_po.set_rotate_y(DEG_TO_RAD(15.0f) * PLUS_MINUS_ONE);
      dir = the_po.non_affine_slow_xform(dir);

      vector3d hit, hit_n;
      bool coll_active = get_my_entity()->are_collisions_active();
      get_my_entity()->set_collisions_active(false, false);
      vector3d inter2 = get_my_entity()->get_abs_position() + (dir*7.0f);
      bool intersect = find_intersection( get_my_entity()->get_abs_position(), inter2,
                                          get_my_entity()->get_primary_region(),
                                          FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                                          &hit, &hit_n );
      get_my_entity()->set_collisions_active(coll_active, false);

      if(intersect)
      {
        rational_t len = (get_my_entity()->get_abs_position() - hit).length();
        if(len <= (2.0f))
          valid = false;
        else
          dist = len - 1.5f;
      }
      else
      {
        dist = 4.0f + PLUS_MINUS_ONE*1.0f;
      }
    }

    vector3d target = get_my_entity()->get_abs_position() + (valid ? (dir*dist) : ZEROVEC);

    switch(get_ai_interface()->get_locomotion_type())
    {
      case LOCOMOTION_HELI:
      {
        vector3d end = target + (YVEC*5.0f*PLUS_MINUS_ONE);
        vector3d hit, hitn;
        if ( find_intersection( get_my_entity()->get_abs_position(), end,
                                 get_my_entity()->get_primary_region(),
                                 FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                                 &hit, &hitn,
                                 NULL, NULL ) )
        {
          target = hit + hitn * 2.5f;
        }
        else
          target = end;
      }
      break;
      default:
        break;
    }

    get_ai_interface()->goto_position(target, 1.5f, true);
  }

  return( duration <= 0.0f );
}

void search_ai_action::going_out_of_service()
{
  ai_action::going_out_of_service();
}

void search_ai_action::going_into_service()
{
  ai_action::going_into_service();
}


void search_ai_action::setup( rational_t time )
{
//  anim_ai_action::setup("SEARCH", AI_ANIM_LOCO_OVERRIDE, empty_string, false, false, true);
  duration = time;
}









//-------------------------------------Long Jump--------------------------------------------------------
jump_ai_action::jump_ai_action(ai_goal *own)
 : ai_action(own)
{
  force = 10.0f;
  type = _JUMP_REL;
}

void jump_ai_action::setup_rel(rational_t _force)
{
  force = _force;
  type = _JUMP_REL;
}

void jump_ai_action::setup_abs(const vector3d &_dir, rational_t _force)
{
  force = _force;
  dir = _dir;
  type = _JUMP_ABS;
}

void jump_ai_action::setup_targ(const vector3d &target_pos, rational_t xzvel)
{
  force = xzvel;
  dir = target_pos;
  type = _JUMP_TARG;
}

bool jump_ai_action::frame_advance( time_value_t delta_t )
{
  switch(state)
  {
    case 0:
    {
      if(get_my_entity()->signal_raised(entity::ANIM_ACTION) || get_my_entity()->anim_finished(AI_ANIM_FULL_BODY))
      {
        get_my_entity()->clear_signal_raised(entity::ANIM_ACTION);

        if(get_my_entity()->has_physical_ifc())
        {
          get_my_entity()->set_stationary(false);
          get_my_entity()->physical_ifc()->enable();
          get_my_entity()->physical_ifc()->set_standing(false);

          vector3d launch_vec = dir*force;
          get_my_entity()->physical_ifc()->apply_force_increment(launch_vec, physical_interface::INSTANT);
        }

        ++state;
      }
    }
    break;

    case 1:
    {
      if(get_my_entity()->anim_finished(AI_ANIM_FULL_BODY))
      {
        stringx fly_anim("JUMP_FLY");
        get_ai_interface()->play_animation(fly_anim, AI_ANIM_FULL_BODY, (ANIM_RELATIVE_TO_START | ANIM_NONCOSMETIC));

        ++state;
      }
    }
    break;

    case 2:
    {
      bool landed = (get_my_entity()->get_primary_region() == NULL) || !get_my_entity()->has_physical_ifc() || get_my_entity()->physical_ifc()->is_effectively_standing();

      vector3d hit, hitn;
      if(!landed && (!get_my_entity()->has_physical_ifc() || get_my_entity()->physical_ifc()->get_velocity().y < 0.0f))
      {
        vector3d start = get_my_entity()->get_abs_position();
        vector3d end = start - YVEC*1.0f;

        landed = find_intersection(start, end, get_my_entity()->get_primary_region(), (FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY), &hit, &hitn, NULL, NULL);
      }

      if(landed)
      {
        if(get_my_entity()->has_physical_ifc())
          get_my_entity()->physical_ifc()->set_standing(true);

        stringx land_anim("JUMP_LAND");
        get_ai_interface()->play_animation(land_anim, AI_ANIM_FULL_BODY, (ANIM_TWEEN | ANIM_RELATIVE_TO_START | ANIM_NONCOSMETIC));

        ++state;
      }
    }
    break;

    case 3:
    {
      if(get_my_entity()->anim_finished(AI_ANIM_FULL_BODY))
        ++state;
    }
    break;

    default:
      assert(0);
      break;
  }

  return(state >= 4);
}

void jump_ai_action::going_out_of_service()
{
  get_my_entity()->kill_anim( AI_ANIM_FULL_BODY );

  get_ai_interface()->set_head_disabled(false);

  get_ai_interface()->set_allow_wounded(allow_wounded);

  if(get_my_entity()->has_physical_ifc())
    get_my_entity()->physical_ifc()->set_gravity_multiplier(grav_mod);

  ai_action::going_out_of_service();
}

void jump_ai_action::going_into_service()
{
  ai_action::going_into_service();

  grav_mod = get_my_entity()->has_physical_ifc() ? get_my_entity()->physical_ifc()->get_gravity_multiplier() : 1.0f;
  if(get_my_entity()->has_physical_ifc())
    get_my_entity()->physical_ifc()->set_gravity_multiplier(1.25f);

  switch(type)
  {
    case _JUMP_REL:
    {
      dir = get_my_entity()->get_abs_po().get_facing();
      dir.y = 0.0f;
      dir.normalize();

      dir = YVEC + dir;
      dir.normalize();
    }
    break;

    case _JUMP_TARG:
    {
      dir = get_ai_interface()->calculate_jump_vector_xzvel(dir, force);

      force = dir.length();
      dir *= (1.0f / force);
    }
    break;

    default:
      break;
  }

  state = 0;
  stringx launch_anim = stringx("JUMP_LAUNCH");
  get_ai_interface()->play_animation(launch_anim, AI_ANIM_FULL_BODY, (ANIM_TWEEN | ANIM_RELATIVE_TO_START | ANIM_NONCOSMETIC));

  get_my_entity()->clear_signal_raised(entity::ANIM_ACTION);

  get_ai_interface()->set_head_disabled(true);

  allow_wounded = get_ai_interface()->allow_wounded();
  get_ai_interface()->set_allow_wounded(false);
}








void dodge_ai_action_damage_callback(signaller* sig, const char* data)
{
  ((entity *)sig)->damage_ifc()->set_damage_amt(0);
}

dodge_ai_action::dodge_ai_action(ai_goal *own)
  : anim_ai_action(own)
{
}

void dodge_ai_action::going_out_of_service()
{
  anim_ai_action::going_out_of_service();
  get_ai_interface()->set_allow_wounded(was_allow_wounded);

  if(!allow_damage)
    get_my_entity()->signal_ptr( entity::DAMAGED )->kill_callback(sig_id);
}

void dodge_ai_action::going_into_service()
{
  anim_ai_action::going_into_service();

  was_allow_wounded = get_ai_interface()->allow_wounded();
  get_ai_interface()->set_allow_wounded(was_allow_wounded && allow_wounded);

  if(!allow_damage)
    sig_id = get_my_entity()->signal_ptr( entity::DAMAGED )->add_callback( dodge_ai_action_damage_callback, (char *)this );
}


void dodge_ai_action::setup(eDodgeDir dir, bool _allow_damage, bool _allow_wounded)
{
  stringx anim_to_find;
  stringx sound = "DODGE";

  switch(dir)
  {
    case _DODGE_L:
      anim_to_find = stringx("DODGE_L");
    break;

    case _DODGE_R:
      anim_to_find = stringx("DODGE_R");
    break;

    case _DODGE_F:
      anim_to_find = stringx("DODGE_F");
    break;

    case _DODGE_B:
      anim_to_find = stringx("DODGE_B");
    break;

    case _DODGE_LR:
      anim_to_find = (ZERO_TO_ONE < 0.5f) ? stringx("DODGE_L") : stringx("DODGE_R");
    break;

    case _DODGE_FB:
      anim_to_find = (ZERO_TO_ONE < 0.5f) ? stringx("DODGE_F") : stringx("DODGE_B");
    break;

    case _DODGE_LRB:
    {
      rational_t roll = ZERO_TO_ONE;
      if(roll <= 0.33f)
        anim_to_find = stringx("DODGE_L");
      else if(roll <= 0.66f)
        anim_to_find = stringx("DODGE_R");
      else
        anim_to_find = stringx("DODGE_B");
    }
    break;

    case _DODGE_LRF:
    {
      rational_t roll = ZERO_TO_ONE;
      if(roll <= 0.33f)
        anim_to_find = stringx("DODGE_L");
      else if(roll <= 0.66f)
        anim_to_find = stringx("DODGE_R");
      else
        anim_to_find = stringx("DODGE_F");
    }
    break;

    case _DODGE:
    default:
    {
      rational_t roll = ZERO_TO_ONE;
      if(roll <= 0.25f)
        anim_to_find = stringx("DODGE_L");
      else if(roll <= 0.5f)
        anim_to_find = stringx("DODGE_R");
      else if(roll <= 0.75f)
        anim_to_find = stringx("DODGE_F");
      else
        anim_to_find = stringx("DODGE_B");
    }
    break;
  }

  anim_ai_action::setup(anim_to_find, AI_ANIM_FULL_BODY, sound, false, false, true);

  allow_damage = _allow_damage;
  allow_wounded = _allow_wounded;
}







use_item_ai_action::use_item_ai_action(ai_goal *own)
  : ai_action(own)
{
  used_item = false;
}

item *use_item_ai_action::get_usable_item()
{
  int num = get_my_entity()->get_num_items();

  for(int i=0; i<num; i++)
  {
    // returns NULL if index is out-of=range
    item* itm = get_my_entity()->get_item(i);
    if(itm->is_brain_weapon())
      return(itm);
  }

  return(NULL);
}

bool use_item_ai_action::frame_advance( time_value_t delta_t )
{
  item *itm = get_usable_item();
  if(itm == NULL)
    return(true);

  assert( itm && itm->is_a_handheld_item() );

  // temp
  if ( !used_item )
  {
    used_item = true;
    get_my_entity()->use_item(itm);
  }

  return (true);
}

void use_item_ai_action::going_out_of_service()
{
  ai_action::going_out_of_service();
//  get_ai_interface()->set_allow_wounded(true);
}

void use_item_ai_action::going_into_service()
{
  ai_action::going_into_service();
//  get_ai_interface()->set_allow_wounded(false);
}



#endif // BIGCULL
