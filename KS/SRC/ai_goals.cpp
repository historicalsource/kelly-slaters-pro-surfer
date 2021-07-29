#include "global.h"

#include "ai_goals.h"
#include "ai_interface.h"
#include "ai_locomotion.h"
// BIGCULL #include "ai_locomotion_walk.h"
// BIGCULL #include "ai_senses.h"
#include "animation_interface.h"
#include "ai_actions.h"
// BIGCULL #include "ai_actions_combat.h"
#include "entity.h"
#include "collide.h"
#include "random.h"
#include "item.h"
#include "wds.h"
#include "terrain.h"
 // BIGCULL #include "gun.h"
// BIGCULL #include "thrown_item.h"
#include "hwaudio.h"
// BIGCULL#include "switch_obj.h"
//#include "sound_interface.h"
// BIGCULL #include "damage_interface.h"
#include "physical_interface.h"
// BIGCULL #include "ai_communication.h"
#ifdef GCCULL
#include "ai_voice.h"
#endif
// BIGCULL #include "ai_constants.h"

// BIGCULL #include "spiderman_controller.h"

extern rational_t g_gravity;

ai_goal::ai_goal(ai_interface *own)
{
  owner = own;
  priority = 0.0f;
  priority_modifier = 1.0f;
  in_service = false;
  assert(owner);

  type = pstring("NONE");
}

ai_goal::~ai_goal()
{
}

rational_t ai_goal::frame_advance(time_value_t t)
{
  // NB: TWO RETURN VALUES ARE HAPPENING HERE
  list<ai_action *>::iterator i = actions.begin();
  while(i != actions.end())
  {
    ai_action *action = (*i);
    assert(action->is_in_service());

    if( action->frame_advance( t ) )
    {
      action->going_out_of_service();
      delete action;

      i = actions.erase(i);
    }
    else
      ++i;
  }

  return(priority);
}

entity *ai_goal::get_my_entity() const
{
  return(owner->get_my_entity());
}

void ai_goal::going_into_service()
{
  assert(!in_service);
  in_service = true;
}

void ai_goal::dump_actions()
{
  list<ai_action *>::iterator i = actions.begin();
  while(i != actions.end())
  {
    ai_action *action = (*i);

    if( action->is_in_service() )
      action->going_out_of_service();

    delete action;

    i = actions.erase(i);
  }
}

void ai_goal::dump_action(ai_action *act)
{

  if( !act ) {
    return;
  }

  list<ai_action *>::iterator i = actions.begin();
  while(i != actions.end())
  {
    if((*i) == act)
    {
      if( (*i)->is_in_service() )
        (*i)->going_out_of_service();

      delete (*i);

      i = actions.erase(i);

      return;
    }
    else
      ++i;
  }
}

unsigned int ai_goal::add_action(ai_action *act)
{
  if(act != NULL)
  {
    act->going_into_service();
    actions.push_back(act);

    return(act->get_id());
  }

  return(0);
}


bool ai_goal::running_action(unsigned int id)
{
  list<ai_action *>::iterator i = actions.begin();
  while(i != actions.end())
  {
    if((*i)->get_id() == id)
      return(true);

    ++i;
  }

  return(false);
}

void ai_goal::dump_action(unsigned int id)
{
  list<ai_action *>::iterator i = actions.begin();
  while(i != actions.end())
  {
    if((*i)->get_id() == id)
    {
      if( (*i)->is_in_service() )
        (*i)->going_out_of_service();

      delete (*i);

      i = actions.erase(i);

      return;
    }
  }
}

void ai_goal::going_out_of_service()
{
  assert(in_service);
  in_service = false;

  dump_actions();
}

void ai_goal::read_data(chunk_file& fs)
{
  stringx label;
  for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
  {
    handle_chunk(fs, label);
  }
}

void ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
}

bool ai_goal::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("PRIORITY_MOD", priority_modifier);

  return(false);
}

bool ai_goal::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("PRIORITY_MOD", priority_modifier);

  return(false);
}

bool ai_goal::set_str(const pstring &att, const stringx &val)
{
  return(false);
}

bool ai_goal::get_str(const pstring &att, stringx &val)
{
  return(false);
}


#if 0 // BIGCULL
disable_ai_goal::disable_ai_goal(ai_interface *own)
  : idle_ai_goal(own)
{
  type = pstring("DISABLE");
}

disable_ai_goal::~disable_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(disable);

rational_t disable_ai_goal::frame_advance(time_value_t t)
{
  return(ai_goal::frame_advance(t));
}

rational_t disable_ai_goal::calculate_priority(time_value_t t)
{
  if(owner->is_disabled())
    priority = 999.0f;
  else
    priority = 0.0f;

  return(priority);
}

void disable_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  stringx idle_str("IDLE");
  get_ai_interface()->play_animation(idle_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));
}

void disable_ai_goal::going_out_of_service()
{
  get_my_entity()->kill_anim( AI_ANIM_IDLE );

  ai_goal::going_out_of_service();
}




death_ai_goal::death_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("DEATH");
  fade_time = 1.0f;
  fade = true;
}

death_ai_goal::~death_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(death);

rational_t death_ai_goal::frame_advance(time_value_t t)
{
  if(fade && fade_timer > 0.0f && get_my_entity()->anim_finished( AI_ANIM_ABSOLUTE ))
  {
    get_my_entity()->set_collisions_active(false);

    fade_timer -= t;
    if(fade_timer <= 0.0f)
    {
      color32 col = ren_col;
      col.set_alpha(0);
      get_my_entity()->set_render_color(col);

      get_my_entity()->raise_signal(entity::FADED_OUT);
      get_my_entity()->set_visible(false);
    }
    else
    {
      color32 col = ren_col;
      col.set_alpha(ren_col.get_alpha() * (fade_timer / fade_time));
      get_my_entity()->set_render_color(col);
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t death_ai_goal::calculate_priority(time_value_t t)
{
  priority = get_my_entity()->is_alive() ? 0.0f : 1000.0f;
  return(priority);
}

void death_ai_goal::going_into_service()
{
  ai_goal::going_into_service();
  damage_info dmg = get_my_entity()->damage_ifc()->get_last_dmg_info();

  if(!owner->is_prone_explosion())
  {
    stringx death_str("DEATH");

    if(dmg.flags & _DF_DEATHBLOW)
    {
      if(((dmg.wounded_anim != empty_string && get_my_entity()->has_animation_ifc()) ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(dmg.wounded_anim) : empty_string) != empty_string)
        death_str = dmg.wounded_anim;
      else if(owner->is_prone_front())
        death_str = stringx("PRONE_WOUNDED_FRONT");
      else if(owner->is_prone_back())
        death_str = stringx("PRONE_WOUNDED_BACK");
    }

    const stringx &res = get_my_entity()->has_animation_ifc() ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(death_str) : empty_string;

    get_my_entity()->kill_anim( AI_ANIM_ABSOLUTE );
    if( res != empty_string )
      get_my_entity()->play_anim( AI_ANIM_ABSOLUTE, res, 0.0f, ANIM_RELATIVE_TO_START | ANIM_TWEEN);
  }
  else
  {
    for(int i=0; i<MAX_ANIM_SLOTS; ++i)
      get_my_entity()->kill_anim( i );
  }

  owner->set_prone_explosion(false);
  get_my_entity()->set_stationary(true);
  owner->set_prone_wounded(false);

  ai_radio_message msg(owner, _RADIO_DEATH, 1.0f);
  msg.set_msg_ent(get_my_entity());
  msg.set_msg_pos(get_my_entity()->get_abs_position());
  msg.set_msg_dir(dmg.dir);
  owner->broadcast_message(_CHANNEL_ALLY, msg);

  owner->set_head_disabled(true);

  ren_col = get_my_entity()->get_render_color();
  coll_active = get_my_entity()->are_collisions_active();
  fade_timer = fade_time;
}

void death_ai_goal::going_out_of_service()
{
  get_my_entity()->set_collisions_active(coll_active);
  get_my_entity()->set_render_color(ren_col);
  get_my_entity()->set_visible(true);

  get_my_entity()->set_stationary(false);
  owner->set_prone_wounded(false);

  get_my_entity()->kill_anim( AI_ANIM_ABSOLUTE );

  owner->set_head_disabled(false);

  ai_goal::going_out_of_service();
}


void death_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "no_fade")
    fade = false;
  else if(label == "fade")
    fade = true;
  else
    ai_goal::handle_chunk(fs, label);
}


bool death_ai_goal::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("FADE", fade);

  return(ai_goal::set_num(att, val));
}

bool death_ai_goal::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("FADE", fade);

  return(ai_goal::get_num(att, val));
}









wounded_ai_goal::wounded_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("wounded");
  first_frame = true;
  knock_down = 0;
  synced = false;
  combo = false;
}

wounded_ai_goal::~wounded_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(wounded);

rational_t wounded_ai_goal::frame_advance(time_value_t t)
{
  if(((!first_frame && owner->wounded()) || owner->sync_wounded()) && !synced)
  {
    going_out_of_service();
    going_into_service();
  }

  first_frame = false;

  ai_goal::frame_advance(t);

  if(knock_down != 0)
  {
		switch (knock_down)
		{
		// knock down front
		case 2:
			owner->set_prone_front(true);
			owner->set_prone_back(false);
			owner->set_prone_explosion(false);
			knock_down = 2;
			break;

		// default assumes knock down on the back
		case 1:
		default:
			owner->set_prone_front(false);
			owner->set_prone_back(true);
			owner->set_prone_explosion(false);
			knock_down = 1;
			break;
		}
  }

  if(actions.empty())
  {
    if(owner->sync_wounded())
    {
      owner->set_sync_wounded(false);
      synced = false;
    }
  }

  return(priority);
}

rational_t wounded_ai_goal::calculate_priority(time_value_t t)
{
  priority = (!actions.empty() || owner->sync_wounded() || (owner->allow_wounded() && owner->wounded())) ? 2.0f : 0.0f;
  return(priority);
}

void wounded_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  synced = owner->sync_wounded();

  damage_info dmg = get_my_entity()->damage_ifc()->get_last_dmg_info();
  stringx wounded_str = dmg.wounded_anim;

  if(owner->sync_wounded())
    wounded_str = g_spiderman_controller_ptr->get_combo_wounded_anim();

  combo = (wounded_str != empty_string);

  const stringx &res = (wounded_str != empty_string && get_my_entity()->has_animation_ifc()) ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(wounded_str) : empty_string;

  if(wounded_str == empty_string || res == empty_string || ((owner->is_prone() || knock_down != 0) && !(dmg.flags & _DF_PRONEHIT) && !owner->sync_wounded()))
  {
    if(owner->is_prone())
      owner->set_prone_wounded(true);

    if(owner->is_prone_front())
      wounded_str = stringx("PRONE_WOUNDED_FRONT");
    else if(owner->is_prone_back())
      wounded_str = stringx("PRONE_WOUNDED_BACK");
    else if(knock_down == 0)
      wounded_str = stringx("WOUNDED");
    else
      wounded_str = empty_string;
  }

  if(!owner->was_prone_wounded() && wounded_str != empty_string)
  {
    get_my_entity()->kill_anim(AI_ANIM_WOUNDED);

    anim_ai_action *act = NEW anim_ai_action(this);
    act->setup(wounded_str, AI_ANIM_WOUNDED, empty_string, false, false, true, !combo);
    add_action(act);
  }

  if(knock_down == 0)
	{
		if (dmg.flags & _DF_KNOCKDOWN)
			knock_down = 1;
		else if (dmg.flags & _DF_KNOCKDOWNFRONT)
			knock_down = 2;
	}

  first_frame = true;

  ai_radio_message msg(owner, _RADIO_WOUNDED, 1.0f);
  msg.set_msg_ent(get_my_entity());
  msg.set_msg_pos(get_my_entity()->get_abs_position());
  msg.set_msg_dir(dmg.dir);
  owner->broadcast_message(_CHANNEL_ALLY, msg);

  if(combo)
    get_my_entity()->set_stationary(true);

  owner->set_head_disabled(true);
}

void wounded_ai_goal::going_out_of_service()
{
  first_frame = true;
  synced = false;

  if(combo)
  {
    combo = false;
    get_my_entity()->set_stationary(false);
  }

  owner->set_head_disabled(false);

  ai_goal::going_out_of_service();
}









exploded_ai_goal::exploded_ai_goal(ai_interface *own)
  : ai_goal(own)
{
	flying = false;
  running = false;
	recovered_from_fall = false;
  launch_dir = ZVEC;
  launch_force = 1.0f;
  hit_from_behind = false;
  blast_pos = ZEROVEC;
  grav_enabled = true;
  grav_mod = 1.0f;

  launch_f_animation = "EXPLODE_LAUNCH_FRONT";
  launch_b_animation = "EXPLODE_LAUNCH_BACK";

  type = pstring("exploded");
}

exploded_ai_goal::~exploded_ai_goal()
{
}


AI_GOAL_MAKE_COPY_MAC(exploded);


rational_t exploded_ai_goal::frame_advance(time_value_t t)
{
	entity *ent = get_my_entity();
	bool has_physical_ifc = ent->has_physical_ifc();

  //if(actions.empty() || (has_physical_ifc && ent->physical_ifc()->get_ext_collided_last_frame()))
  {
    bool landed = has_physical_ifc ? ent->physical_ifc()->is_effectively_standing() : false;

    vector3d dir = ZEROVEC;

    if(!landed && has_physical_ifc && ent->physical_ifc()->get_ext_collided_last_frame())
    {
      dir = (has_physical_ifc && ent->physical_ifc()->get_ext_collided_last_frame()) ? ent->physical_ifc()->get_last_collision_normal() : -ent->get_abs_po().get_facing();
      dir.normalize();

      if(dir.length2() <= 0.01f)
      {
        dir = -(ent->get_abs_position() - ent->get_last_position());
        dir.normalize();
      }

      landed = (dot(dir, YVEC) >= 0.75f);
    }

    if(!landed && ent->get_primary_region() != NULL)
    {
      vector3d start = ent->get_abs_position();
      vector3d end = start - (YVEC*1.0f);
      vector3d hit, hitn;

      bool coll_active = ent->are_collisions_active();
      ent->set_collisions_active(false, false);
      landed = find_intersection(start, end, ent->get_primary_region(), (FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY | FI_COLLIDE_ENTITY_NO_CAPSULES), &hit, &hitn, NULL, NULL);
      ent->set_collisions_active(coll_active, false);
    }

		if( actions.empty() && !flying && !landed )
		{
			flying = true;

      stringx anim_to_find;
      if ( hit_from_behind )
        anim_to_find = "EXPLODE_FLY_FRONT";
      else
        anim_to_find = "EXPLODE_FLY_BACK";

      anim_ai_action *action = NEW anim_ai_action(this);
      action->setup(anim_to_find, AI_ANIM_FULL_BODY, empty_string, false, false, true);
      add_action(action);
		}

    if(/*num_bounces < 2 &&*/ !landing && !landed && has_physical_ifc &&
			ent->physical_ifc()->get_ext_collided_last_frame() &&
			__fabs(dot(dir, YVEC)) < 0.75f)
    {
			dump_actions();
      hit_from_behind = ( dot(dir,ent->get_abs_po().get_facing()) > 0.0f );

      stringx anim_to_find;
      if ( hit_from_behind )
        anim_to_find = "EXPLODE_FLY_FRONT";
      else
        anim_to_find = "EXPLODE_FLY_BACK";

      anim_ai_action *action = NEW anim_ai_action(this);
      action->setup(anim_to_find, AI_ANIM_FULL_BODY, empty_string, false, false, true);
      add_action(action);

      dir.y = 0.0f;
      dir.normalize();
      ent->physical_ifc()->apply_force_increment(dir*(launch_force*0.5f), physical_interface::INSTANT);

      ++num_bounces;
    }

    if(running && !landing && landed)
    {
			dump_actions();
      landing = true;

      stringx anim_to_find;
      if ( hit_from_behind )
				anim_to_find = "EXPLODE_LAND_FRONT";
      else
        anim_to_find = "EXPLODE_LAND_BACK";

      if(ent->has_physical_ifc())
        ent->physical_ifc()->set_standing(true);

      anim_ai_action *action = NEW anim_ai_action(this);
      action->setup(anim_to_find, AI_ANIM_FULL_BODY, empty_string, false, false, true);
      add_action(action);
    }

    if(actions.empty() && landing)
    {
      owner->set_prone_back(!hit_from_behind);
      owner->set_prone_front(hit_from_behind);
      owner->set_prone_explosion(true);
      running = false;
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t exploded_ai_goal::calculate_priority(time_value_t t)
{
	recovered_from_fall = (running && (blast_pos.y - get_my_entity()->get_abs_position().y) > 3.0f) ? true : false;

  if(owner->allow_wounded() && get_my_entity()->has_physical_ifc() && ((running && !recovered_from_fall) || (owner->wounded() && get_my_entity()->damage_ifc()->get_last_dmg_info().type == DAMAGE_EXPLOSIVE)))
  {
    if(!running)
    {
      damage_info dmg = get_my_entity()->damage_ifc()->get_last_dmg_info();
      launch_dir = dmg.dir;
      launch_dir.y = 0.0f;
      launch_dir.normalize();

      launch_force = 6.0f*(get_my_entity()->has_physical_ifc() ? (get_my_entity()->physical_ifc()->get_mass()) : 1.0f);
    }

    priority = 1001.0f;
  }
  else
		priority = 0.0f;

  if(!in_service)
    running = false;

  return(priority);
}

void exploded_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  owner->goto_position(get_my_entity()->get_abs_position(), 25.0f, true, false, true);

  grav_mod = get_my_entity()->has_physical_ifc() ? get_my_entity()->physical_ifc()->get_gravity_multiplier() : 1.0f;

  running = true;
	flying = false;
  landing = false;
	recovered_from_fall = false;

  // hit from front or back?
  hit_from_behind = ( dot(launch_dir,get_my_entity()->get_abs_po().get_facing()) > 0.0f );

  vector3d face_target = get_my_entity()->get_abs_position() + (hit_from_behind ? launch_dir : -launch_dir);
  owner->apply_rotation(owner->get_xz_rotation_to_point(face_target));

  stringx anim_to_find;

  if ( hit_from_behind )
    anim_to_find = launch_f_animation;
  else
    anim_to_find = launch_b_animation;

  anim_ai_action *action = NEW anim_ai_action(this);
  action->setup(anim_to_find, AI_ANIM_FULL_BODY, empty_string, false, false, true);
  add_action(action);

  if ( get_my_entity()->has_physical_ifc() )
  {
    get_my_entity()->set_stationary(false);
    get_my_entity()->physical_ifc()->enable();
    get_my_entity()->physical_ifc()->set_standing(false);
    get_my_entity()->physical_ifc()->set_ext_collided_last_frame(false);
    get_my_entity()->physical_ifc()->set_collided_last_frame(false);

    grav_enabled = get_my_entity()->physical_ifc()->is_gravity();
    get_my_entity()->physical_ifc()->set_gravity(true);
    get_my_entity()->physical_ifc()->set_gravity_multiplier(2.0f);


    vector3d launch_vec = (launch_dir*launch_force) + (YVEC*g_gravity*0.75f*(get_my_entity()->has_physical_ifc() ? (get_my_entity()->physical_ifc()->get_mass()) : 1.0f));
    get_my_entity()->physical_ifc()->apply_force_increment(launch_vec, physical_interface::INSTANT);
  }

  blast_pos = get_my_entity()->get_abs_position();

  owner->set_prone_wounded(false);

  num_bounces = 0;

  owner->set_head_disabled(true);
}

void exploded_ai_goal::going_out_of_service()
{
  running = false;

  owner->set_head_disabled(false);

  if(get_my_entity()->has_physical_ifc())
  {
    get_my_entity()->physical_ifc()->set_gravity_multiplier(grav_mod);
    get_my_entity()->physical_ifc()->set_gravity(grav_enabled);
//    get_my_entity()->physical_ifc()->set_velocity(ZEROVEC);
  }

  ai_goal::going_out_of_service();
}









prone_ai_goal::prone_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("prone");
  recover_timer = 0.0f;
  prone_time = 1.25f;
  prone_time_var = 0.25f;
}

prone_ai_goal::~prone_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(prone);

rational_t prone_ai_goal::frame_advance(time_value_t t)
{
  recover_timer -= t;
  if(recover_timer <= 0.0f)
  {
    recover_timer = 0.0f;

    if(actions.empty())
    {
      stringx get_up_anim(owner->is_prone_front() ? "GET_UP_FRONT" : "GET_UP_BACK");

      get_my_entity()->kill_anim(AI_ANIM_FULL_BODY);

      anim_ai_action *act = NEW anim_ai_action(this);
      get_my_entity()->kill_anim(AI_ANIM_FULL_BODY);
      act->setup(get_up_anim, AI_ANIM_FULL_BODY, empty_string, false, false, true);
      add_action(act);

      owner->set_prone_back(false);
      owner->set_prone_front(false);
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t prone_ai_goal::calculate_priority(time_value_t t)
{
  priority = (owner->is_prone() || !actions.empty()) ? 1.9f : 0.0f;
  return(priority);
}

void prone_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  owner->set_prone_explosion(false);

  damage_info dmg = get_my_entity()->damage_ifc()->get_last_dmg_info();

  stringx prone_str = owner->is_prone_front() ? stringx("PRONE_FRONT") : stringx("PRONE_BACK");
  get_ai_interface()->play_animation(prone_str, AI_ANIM_FULL_BODY, (ANIM_RELATIVE_TO_START | ANIM_AUTOKILL | ANIM_LOOPING | ANIM_TWEEN));

  recover_timer = owner->was_prone_wounded() ? 0.2f : VARIANCE(prone_time, prone_time_var);
  owner->set_prone_wounded(false);

  get_my_entity()->set_stationary(true);

  owner->set_head_disabled(true);
}

void prone_ai_goal::going_out_of_service()
{
  get_my_entity()->kill_anim(AI_ANIM_FULL_BODY);
  get_my_entity()->set_stationary(false);
  owner->set_head_disabled(false);

  ai_goal::going_out_of_service();
}

void prone_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "prone_time")
    serial_in(fs, &prone_time);
  else if(label == "prone_time_var")
    serial_in(fs, &prone_time_var);
  else
    ai_goal::handle_chunk(fs, label);
}


bool prone_ai_goal::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("PRONE_TIME", prone_time);
  IFC_INTERNAL_SET_MACRO("PRONE_TIME_VAR", prone_time_var);

  return(ai_goal::set_num(att, val));
}

bool prone_ai_goal::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("PRONE_TIME", prone_time);
  IFC_INTERNAL_GET_MACRO("PRONE_TIME_VAR", prone_time_var);

  return(ai_goal::get_num(att, val));
}








idle_ai_goal::idle_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("IDLE");
}

idle_ai_goal::~idle_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(idle);

rational_t idle_ai_goal::frame_advance(time_value_t t)
{
  return(ai_goal::frame_advance(t));
}

rational_t idle_ai_goal::calculate_priority(time_value_t t)
{
  priority = 0.01f;
  return(priority);
}

void idle_ai_goal::going_into_service()
{
  ai_goal::going_into_service();
  stringx idle_str("IDLE");
  get_ai_interface()->play_animation(idle_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));
}

void idle_ai_goal::going_out_of_service()
{
  if(!get_my_entity()->playing_scene_anim())
    get_my_entity()->kill_anim( AI_ANIM_IDLE );

  ai_goal::going_out_of_service();
}






coward_ai_goal::coward_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("COWARD");
  min_fear_cower = 100.0f;
  min_fear_run = 50.0f;

  running = false;
  cowering = false;
}

coward_ai_goal::~coward_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(coward);

rational_t coward_ai_goal::frame_advance(time_value_t t)
{
  return(ai_goal::frame_advance(t));
}

rational_t coward_ai_goal::calculate_priority(time_value_t t)
{
  priority = (owner->get_fear() >= min_fear_cower || owner->get_fear() >= min_fear_run || (running && !owner->reached_dest())) ? 1.1f : 0.0f;
  return(priority);
}

void coward_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  rational_t fear = owner->get_fear();

  running = false;
  cowering = false;

  if(fear >= min_fear_run && fear < min_fear_cower)
  {
    vector3d pos;

    if(owner->wounded())
    {
      if(owner->get_dmg_info().attacker)
        running = owner->find_cover_pt(owner->get_dmg_info().attacker->get_abs_position(), pos);

      if(!running)
        running = owner->find_cover_pt(owner->get_dmg_info().loc, pos);
    }

    if(!running)
      running = owner->find_cover_pt(get_my_entity()->get_abs_position(), pos);

    if(running)
    {
      owner->goto_position(pos, 1.5f);

      get_ai_interface()->get_voice()->say(_VOICE_RUN);
    }
  }

  if(!running && fear >= min_fear_cower)
  {
    stringx cower_str("COWER");
    get_ai_interface()->play_animation(cower_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));

    get_ai_interface()->get_voice()->say(_VOICE_COWER);

    cowering = true;
  }
}

void coward_ai_goal::going_out_of_service()
{
  if(!get_my_entity()->playing_scene_anim())
    get_my_entity()->kill_anim( AI_ANIM_IDLE );

  running = false;
  cowering = false;

  ai_goal::going_out_of_service();
}

void coward_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "min_fear_cower")
    serial_in(fs, &min_fear_cower);
  else if(label == "min_fear_run")
    serial_in(fs, &min_fear_run);
  else
    ai_goal::handle_chunk(fs, label);
}

bool coward_ai_goal::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("MIN_FEAR_COWER", min_fear_cower);
  IFC_INTERNAL_SET_MACRO("MIN_FEAR_RUN", min_fear_run);

  return(false);
}

bool coward_ai_goal::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("MIN_FEAR_COWER", min_fear_cower);
  IFC_INTERNAL_GET_MACRO("MIN_FEAR_RUN", min_fear_run);

  return(false);
}





threaten_ai_goal::threaten_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("threaten");
  attack_time = 4.0f;
  attack_time_var = 1.0f;

  threaten_time = 3.0f;
  threaten_time_var = 1.5f;

  target = NULL;

  threaten_team = ai_interface::_TEAM_NEUT;
}

threaten_ai_goal::~threaten_ai_goal()
{
}


AI_GOAL_MAKE_COPY_MAC(threaten);


rational_t threaten_ai_goal::frame_advance(time_value_t t)
{
  attack_timer -= t;
  if(attack_timer < 0.0f)
    attack_timer = 0.0f;

  threaten_timer -= t;
  if(threaten_timer < 0.0f)
    threaten_timer = 0.0f;

  if(target && target->is_alive())
  {
    owner->force_lookat(target);

    rational_t dist = (get_my_entity()->get_abs_position() - target->get_abs_position()).length2();

    if(dist <= (2.0f*2.0f) )
    {
      if(attack_timer <= 0.0f && actions.empty())
      {
        owner->set_target(target);
        attack_timer = attack_time + (attack_time_var*PLUS_MINUS_ONE);

        attack_ai_action *a = NEW attack_ai_action(this);
        a->setup("PUNCH", "", AI_ANIM_ATTACK);
        add_action(a);
      }
    }
    else
    {
      owner->goto_position(target->get_abs_position(), 1.5f, true);
    }

    if(dist <= (3.0f*3.0f) )
    {
      if(threaten_timer <= 0.0f && actions.empty())
      {
        owner->set_target(target);
        threaten_timer = threaten_time + (threaten_time_var*PLUS_MINUS_ONE);

        anim_ai_action *a = NEW anim_ai_action(this);
        a->setup("THREATEN", AI_ANIM_ATTACK, "THREATEN", false, false, false);
        add_action(a);

        ai_radio_message msg(owner, _RADIO_THREATEN, 1.0f);
        owner->send_message(target->ai_ifc(), msg);
      }
    }

    if(!owner->xz_facing_point(target->get_abs_position(), DEG_TO_RAD(30.0f)))
    {
      rational_t rads = owner->get_xz_rotation_to_point(target->get_abs_position());
      rational_t max_rads = DEG_TO_RAD(180.0f) * t;

      if(rads > max_rads)
        rads = max_rads;
      else if(rads < -max_rads)
        rads = -max_rads;

      owner->apply_rotation(rads);
    }
  }
  else
  {
    target = NULL;
    owner->release_lookat();
  }

  return(ai_goal::frame_advance(t));
}

rational_t threaten_ai_goal::calculate_priority(time_value_t t)
{
  if(target == NULL || !target->is_alive())
  {
    target = NULL;

    ai_interface *ai = owner->find_ai_by_team(threaten_team, (ai_interface::_FIND_AI_NEAREST | ai_interface::_FIND_AI_LOS));
    if(ai)
      target = ai->get_my_entity();
  }

  if(target && target->is_alive())
    priority = 0.6f;
  else
    priority = 0.0f;

  return(priority);
}

void threaten_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  stringx combat_idle_str("COMBAT_IDLE");
  get_ai_interface()->play_animation(combat_idle_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));

  attack_timer = attack_time + (attack_time_var*PLUS_MINUS_ONE);
  threaten_timer = threaten_time + (threaten_time_var*PLUS_MINUS_ONE);
}

void threaten_ai_goal::going_out_of_service()
{
  if(!get_my_entity()->playing_scene_anim())
    get_my_entity()->kill_anim( AI_ANIM_IDLE );

  target = NULL;
  owner->release_lookat();

  ai_goal::going_out_of_service();
}


void threaten_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "attack_time")
    serial_in(fs, &attack_time);
  else if(label == "attack_time_var")
    serial_in(fs, &attack_time_var);
  else if(label == "threaten_time")
    serial_in(fs, &threaten_time);
  else if(label == "threaten_time_var")
    serial_in(fs, &threaten_time_var);
  else
    ai_goal::handle_chunk(fs, label);
}

bool threaten_ai_goal::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("ATTACK_TIME", attack_time);
  IFC_INTERNAL_SET_MACRO("ATTACK_TIME_VAR", attack_time_var);

  IFC_INTERNAL_SET_MACRO("THREATEN_TIME", threaten_time);
  IFC_INTERNAL_SET_MACRO("THREATEN_TIME_VAR", threaten_time_var);

  return(false);
}

bool threaten_ai_goal::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("ATTACK_TIME", attack_time);
  IFC_INTERNAL_GET_MACRO("ATTACK_TIME_VAR", attack_time_var);

  IFC_INTERNAL_GET_MACRO("THREATEN_TIME", threaten_time);
  IFC_INTERNAL_GET_MACRO("THREATEN_TIME_VAR", threaten_time_var);

  return(false);
}
/*
bool threaten_ai_goal::set_str(const pstring &att, const stringx &val)
{
  static pstring check_leader("LEADER");
  if(att == check_leader)
  {
    // reset leader
    leader_name = val;
    leader = NULL;

    return(true);
  }

  return(false);
}

bool threaten_ai_goal::get_str(const pstring &att, stringx &val)
{
  IFC_INTERNAL_GET_MACRO("LEADER", leader_name);
  return(false);
}

*/





guard_ai_goal::guard_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("GUARD");
  first_entry = true;
  returning = false;
  changed = false;
}

guard_ai_goal::~guard_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(guard);

rational_t guard_ai_goal::frame_advance(time_value_t t)
{
  if(changed)
  {
    owner->goto_position(guard_pos, 2.0f, true);
    returning = true;
    changed = false;
  }

  if(returning && owner->reached_dest())
  {
    owner->face_dir(guard_dir);

    returning = false;
  }

  return(ai_goal::frame_advance(t));
}

rational_t guard_ai_goal::calculate_priority(time_value_t t)
{
  if(owner->new_radio_message() && owner->get_last_message().get_msg() == _RADIO_GUARD)
  {
    guard_pos = owner->get_last_message().get_msg_pos();
    guard_dir = owner->get_last_message().get_msg_dir();
    changed = true;
  }

  priority = 0.015f;
  return(priority);
}

void guard_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  stringx idle_str("GUARD");
  get_ai_interface()->play_animation(idle_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));

  if(first_entry)
  {
    first_entry = false;
    returning = false;
    guard_pos = get_my_entity()->get_abs_position();
    guard_dir = get_my_entity()->get_abs_po().get_facing();
  }
  else
  {
    owner->goto_position(guard_pos, 2.0f, true);
    returning = true;
  }
}

void guard_ai_goal::going_out_of_service()
{
  if(!get_my_entity()->playing_scene_anim())
    get_my_entity()->kill_anim( AI_ANIM_IDLE );

  ai_goal::going_out_of_service();
}






patrol_ai_goal::patrol_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("PATROL");
}

patrol_ai_goal::~patrol_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(patrol);

rational_t patrol_ai_goal::frame_advance(time_value_t t)
{
  if(owner->get_current_path_graph() && (owner->reached_dest() || reset_patrol))
  {
    bool ok = false;
    vector3d delta = (get_my_entity()->get_abs_position() - next_patrol);
    delta.y = 0.0f;

    rational_t patrol_radius = owner->get_patrol_radius();


    if( reset_patrol )// || delta.length2() > (patrol_radius*patrol_radius))
    {
      reset_patrol = false;
      patrol_pos = get_my_entity()->get_abs_position();

      ok = owner->get_nearest_patrol_point(patrol_pos, next_patrol);
      last_pos = next_patrol;
    }
    else
    {
      ok = owner->get_next_patrol_point(last_pos, patrol_pos, next_patrol);

      if(!ok)
        ok = owner->get_nearest_patrol_point(patrol_pos, next_patrol);

      last_pos = patrol_pos;
      patrol_pos = next_patrol;
    }

    if(ok)
      owner->goto_position(next_patrol, patrol_radius, running_speed, false);
  }

  return(ai_goal::frame_advance(t));
}

rational_t patrol_ai_goal::calculate_priority(time_value_t t)
{
  priority = 0.02f;
  return(priority);
}

void patrol_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  patrol_pos = last_pos = get_my_entity()->get_abs_position();
  reset_patrol = true;
  running_speed = false;
}

void patrol_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
  reset_patrol = true;
}










follow_ai_goal::follow_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("FOLLOW");
  max_dist = 6.0f;
  leader = NULL;
  leader_name = empty_string;
}

follow_ai_goal::~follow_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(follow);

rational_t follow_ai_goal::frame_advance(time_value_t t)
{
  if(leader)
  {
    vector3d hit, hitn;
    vector3d inter2 = leader->get_abs_position()-(YVEC*1000.0f);
    find_intersection( leader->get_abs_position(), inter2,
                       leader->get_primary_region(),
                       FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY,
                       &hit, &hitn,
                       NULL, NULL );

    vector3d delta = ((hit+hitn) - get_my_entity()->get_abs_position());
    rational_t len2 = delta.xz_length2();

    vector3d pos = (hit+hitn);

    bool see_leader = owner->get_eyes()->check_pos(pos, 9999.0f);

    if(owner->reached_dest() && (len2 > (max_dist*max_dist) || !see_leader))
    {
      owner->goto_position(pos, see_leader ? max_dist*0.75f : 2.0f, true);
    }
    else if(len2 <= ((max_dist*0.75f)*(max_dist*0.75f)) && see_leader)
      owner->goto_position(get_my_entity()->get_abs_position(), 25.0f, true);

    owner->force_lookat(leader);
  }
  else
    owner->release_lookat();

  return(ai_goal::frame_advance(t));
}

rational_t follow_ai_goal::calculate_priority(time_value_t t)
{
  if(!leader && !leader_name.empty())
  {
    entity *ent = g_world_ptr->get_entity(leader_name);

    if(ent != NULL)
      leader = ent;
    else
      error("'%s' is not a valid leader entity (follow goal)", leader_name.c_str());
  }

  if(leader)
  {
    if(in_service && !owner->reached_dest())
    {
      priority = 0.15f;
    }
    else
    {
      rational_t len2 = 0.0f;
      vector3d hit = ZEROVEC, hitn = YVEC;

      vector3d inter2 = leader->get_abs_position()-(YVEC*1000.0f);
      find_intersection( leader->get_abs_position(), inter2,
                         leader->get_primary_region(),
                         FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY,
                         &hit, &hitn,
                         NULL, NULL );

      vector3d delta = ((hit+hitn) - get_my_entity()->get_abs_position());
      len2 = delta.xz_length2();

      vector3d pos = hit+hitn;

      if((len2 > (max_dist*max_dist) || !owner->get_eyes()->check_pos(pos, 9999.0f)))
        priority = 0.15f;
      else
        priority = 0.0f;
    }
  }
  else
    priority = 0.0f;

  return(priority);
}

void follow_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  assert(leader);

  vector3d hit, hitn;
  vector3d inter2 = leader->get_abs_position()-(YVEC*1000.0f);
  find_intersection( leader->get_abs_position(), inter2,
                     leader->get_primary_region(),
                     FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY,
                     &hit, &hitn,
                     NULL, NULL );

  vector3d pos = hit+hitn;
  owner->goto_position(pos, owner->get_eyes()->check_pos(pos, 9999.0f) ? max_dist*0.75f : 2.0f, true);
}

void follow_ai_goal::going_out_of_service()
{
  owner->release_lookat();
  ai_goal::going_out_of_service();
}

void follow_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "max_dist")
    serial_in(fs, &max_dist);
  else if(label == "leader")
  {
    leader = NULL;

    serial_in(fs, &leader_name);
    leader_name.to_upper();
  }
  else
    ai_goal::handle_chunk(fs, label);
}

bool follow_ai_goal::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("MAX_DIST", max_dist);

  return(false);
}

bool follow_ai_goal::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("MAX_DIST", max_dist);

  return(false);
}

bool follow_ai_goal::set_str(const pstring &att, const stringx &val)
{
  static pstring check_leader("LEADER");
  if(att == check_leader)
  {
    // reset leader
    leader_name = val;
    leader = NULL;

    return(true);
  }

  return(false);
}

bool follow_ai_goal::get_str(const pstring &att, stringx &val)
{
  IFC_INTERNAL_GET_MACRO("LEADER", leader_name);
  return(false);
}







slugger_ai_goal::slugger_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  reload = 0.0f;
  type = pstring("slugger");
}

slugger_ai_goal::~slugger_ai_goal()
{
}


AI_GOAL_MAKE_COPY_MAC(slugger);


rational_t slugger_ai_goal::frame_advance(time_value_t t)
{
  reload -= t;
  if(reload < 0.0f)
    reload = 0.0f;

  if(owner->get_threat() && owner->get_threat()->is_alive())
  {
    if( (get_my_entity()->get_abs_position() - owner->get_threat()->get_abs_position()).length2() <= (2.0f*2.0f) )
    {
      if(reload <= 0.0f)
      {
        owner->set_target(owner->get_threat());
        reload = 1.0f + (__fabs(PLUS_MINUS_ONE)*2.0f);
        attack_ai_action *a = NEW attack_ai_action(this);
        a->setup("PUNCH", "", AI_ANIM_ATTACK);
        add_action(a);
      }
    }
    else
    {
      owner->goto_position(owner->get_threat()->get_abs_position(), 1.5f, true);
    }

    if(!owner->xz_facing_point(owner->get_threat()->get_abs_position(), DEG_TO_RAD(60.0f)))
    {
      rational_t rads = owner->get_xz_rotation_to_point(owner->get_threat()->get_abs_position());
      rational_t max_rads = DEG_TO_RAD(180.0f) * t;

      if(rads > max_rads)
        rads = max_rads;
      else if(rads < -max_rads)
        rads = -max_rads;

      owner->apply_rotation(rads);
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t slugger_ai_goal::calculate_priority(time_value_t t)
{
  if(owner->new_threat())
    priority = 1.0f;
  else if(owner->get_threat() && owner->get_threat()->is_alive())
    priority = 0.75f;
  else
    priority = 0.0f;


  return(priority);
}

void slugger_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  stringx combat_idle_str("COMBAT_IDLE");

  get_ai_interface()->play_animation(combat_idle_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));
}

void slugger_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
  if(!get_my_entity()->playing_scene_anim())
    get_my_entity()->kill_anim( AI_ANIM_IDLE );
}












































moving_shooter2_ai_goal::moving_shooter2_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  reload = 0.0f;
  move_timer = 0.0f;
  type = pstring("moving_shooter2");
  accuracy = 0.5f;
}

moving_shooter2_ai_goal::~moving_shooter2_ai_goal()
{
}


AI_GOAL_MAKE_COPY_MAC(moving_shooter2);


rational_t moving_shooter2_ai_goal::frame_advance(time_value_t t)
{
  reload -= t;
  if(reload < 0.0f)
    reload = 0.0f;

  move_timer -= t;
  if(move_timer < 0.0f)
    move_timer = 0.0f;

  if(owner->get_threat() && owner->get_threat()->is_alive())
  {
    rational_t len2 = (get_my_entity()->get_abs_position() - owner->get_threat()->get_abs_position()).length2();

    if(reload <= 0.0f)
    {
      bool valid = owner->compute_combat_target(owner->get_threat()->get_abs_position(), NULL, accuracy);
      if(valid)
      {
        reload = 1.0f + (__fabs(PLUS_MINUS_ONE)*2.0f);

        owner->set_target(owner->get_threat());
        use_item_ai_action *a = NEW use_item_ai_action(this);
        add_action(a);
      }
    }

    if(move_timer <= 0.0f)
    {
      vector3d pt = get_my_entity()->get_abs_position();
      vector3d right = get_my_entity()->get_abs_po().get_x_facing();
      vector3d front = get_my_entity()->get_abs_po().get_z_facing();

      rational_t a = __fabs(PLUS_MINUS_ONE);
      rational_t b = 1.0f - a;

      switch(random(2))
      {
        case 0:
          pt += ((right*a)-(front*b))*1.5f;
          break;

        case 1:
          pt += ((right*-a)-(front*b))*1.5f;
          break;

        default:
          break;
      }

      owner->jockey_to(pt);

      move_timer = 2.0f + PLUS_MINUS_ONE;
    }

    if(owner->reached_dest() && len2 > (15.0f*15.0f))
      owner->goto_position(owner->get_threat()->get_abs_position(), 10.0f);
    else
      owner->get_locomotion()->set_facing(owner->get_threat()->get_abs_position() - get_my_entity()->get_abs_position(), 0.5f);
  }

  return(ai_goal::frame_advance(t));
}

rational_t moving_shooter2_ai_goal::calculate_priority(time_value_t t)
{
  if(owner->new_threat())
    priority = 1.0f;
  else if(owner->get_threat() && owner->get_threat()->is_alive())
    priority = 0.75f;
  else
    priority = 0.0f;


  return(priority);
}

void moving_shooter2_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  assert(owner->get_threat() != NULL);

  stringx combat_idle_str("COMBAT_IDLE");
  get_ai_interface()->play_animation(combat_idle_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));
}

void moving_shooter2_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();

  if(!get_my_entity()->playing_scene_anim())
    get_my_entity()->kill_anim( AI_ANIM_IDLE );

  owner->goto_position(get_my_entity()->get_abs_position(), 3.0f, true);
}

void moving_shooter2_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "accuracy")
    serial_in(fs, &accuracy);
  else
    ai_goal::handle_chunk(fs, label);
}


















camper_ai_goal::camper_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  reload = 0.0f;
  accuracy = 0.5f;

  type = pstring("camper");
}

camper_ai_goal::~camper_ai_goal()
{
}


AI_GOAL_MAKE_COPY_MAC(camper);


rational_t camper_ai_goal::frame_advance(time_value_t t)
{
  reload -= t;
  if(reload < 0.0f)
    reload = 0.0f;

  if(owner->get_threat() && owner->get_threat()->is_alive())
  {
    if(reload <= 0.0f)
    {
      bool valid = owner->compute_combat_target(owner->get_threat()->get_abs_position(), NULL, accuracy);
      if(valid)
      {
        reload = 1.0f + (__fabs(PLUS_MINUS_ONE)*2.0f);

        owner->set_target(owner->get_threat());
        use_item_ai_action *a = NEW use_item_ai_action(this);
        add_action(a);
      }
    }
  }

  if(!owner->xz_facing_point(owner->get_threat()->get_abs_position(), DEG_TO_RAD(15.0f)) && actions.empty())
  {
    owner->face_point(owner->get_threat()->get_abs_position());
  }

  return(ai_goal::frame_advance(t));
}

rational_t camper_ai_goal::calculate_priority(time_value_t t)
{
  if(owner->get_threat() && owner->get_threat()->is_alive())
    priority = 1.0f;
  else
    priority = 0.0f;


  return(priority);
}

void camper_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  assert(owner->get_threat() != NULL);

  stringx combat_idle_str("COMBAT_IDLE");
  get_ai_interface()->play_animation(combat_idle_str, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));

  owner->goto_position(get_my_entity()->get_abs_position(), 3.0f, true);
}

void camper_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();

  if(!get_my_entity()->playing_scene_anim())
    get_my_entity()->kill_anim( AI_ANIM_IDLE );
}

void camper_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "accuracy")
    serial_in(fs, &accuracy);
  else
    ai_goal::handle_chunk(fs, label);
}


bool camper_ai_goal::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("ACCURACY", accuracy);

  return(false);
}

bool camper_ai_goal::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("ACCURACY", accuracy);

  return(false);
}
















investigate_ai_goal::investigate_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("investigate");
  complete = true;
}

investigate_ai_goal::~investigate_ai_goal()
{
}


AI_GOAL_MAKE_COPY_MAC(investigate);


rational_t investigate_ai_goal::frame_advance(time_value_t t)
{
  if(owner->new_cue() || owner->new_threat() || owner->lost_threat() || (owner->new_radio_message() && (owner->get_last_message().get_msg() == _RADIO_SEARCH || (owner->get_last_message().get_msg() == _RADIO_ENEMY_SIGHTED && owner->same_team_id(owner->get_last_message().get_sender()))) ) )
  {
    going_out_of_service();
    going_into_service();
  }

  if(owner->reached_dest())
  {
    owner->release_lookat();

    if(search)
    {
      search = false;
      search_ai_action *s = NEW search_ai_action(this);
      s->setup(6.0f + (PLUS_MINUS_ONE*2.0f));
      add_action(s);

      did_search = true;
    }

    if(actions.empty())
    {
      owner->set_investigated_cue(true);
      complete = true;
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t investigate_ai_goal::calculate_priority(time_value_t t)
{
  bool do_investigate = !complete || (owner->new_cue() || owner->new_threat() || owner->lost_threat());
//  bool priority_boost = false;

  if(!do_investigate && owner->new_radio_message())
  {
    switch(owner->get_last_message().get_msg())
    {
      case _RADIO_SEARCH:
      {
        do_investigate = true;
      }
      break;

      case _RADIO_HELP:
      case _RADIO_ENEMY_SIGHTED:
      {
        if(owner->same_team_id(owner->get_last_message().get_sender()))
          do_investigate = true;
      }
      break;

      default:
        break;
    }
  }


  if(do_investigate)
    priority = 0.1f;
  else
    priority = 0.0f;

  return(priority);
}


extern region_node *get_region_node(vector3d &StartCoords, vector3d &EndCoords);

void investigate_ai_goal::going_into_service()
{
  ai_goal::going_into_service();
  complete = false;
  did_search = false;

  if(owner->new_cue())
  {
    get_ai_interface()->get_voice()->say(owner->get_current_cue().is_visual() ? _VOICE_NEW_CUE_VIS : _VOICE_NEW_CUE_AUD);

    target = owner->get_current_cue().get_pos();
    assert( target.is_valid() );
    search = owner->get_current_cue().search();
  }
  else if(owner->get_threat() != NULL)
  {
    target = owner->get_threat()->get_abs_position();
    assert( target.is_valid() );
    search = true;
  }
  else if(owner->lost_threat())
  {
    target = owner->get_last_threat_pos();
    assert( target.is_valid() );
    search = true;
  }
  else if(owner->new_radio_message() && (owner->get_last_message().get_msg() == _RADIO_SEARCH || ((owner->get_last_message().get_msg() == _RADIO_ENEMY_SIGHTED || owner->get_last_message().get_msg() == _RADIO_HELP) && owner->same_team_id(owner->get_last_message().get_sender()))))
  {
    target = owner->get_last_message().get_msg_pos();
    assert( target.is_valid() );
    search = (owner->get_last_message().get_msg() == _RADIO_SEARCH);
  }
  else
  {
    complete = true;
    assert(0);
  }

  switch(owner->get_locomotion_type())
  {
    case LOCOMOTION_HELI:
    {
      vector3d end = target + (YVEC*5.0f);
      region_node *reg = get_region_node(target, end);
      if(reg)
      {
        vector3d hit, hitn;

        if ( find_intersection( target, end,
                                 reg,
                                 FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                                 &hit, &hitn,
                                 NULL, NULL ) )
        {
          target = hit + hitn * 2.5f;
        }
        else
          target = end;
      }
      assert( target.is_valid() );
    }
    break;

    case LOCOMOTION_WALK:
    {
      vector3d end = target - (YVEC*50.0f);
      region_node *reg = get_region_node(target, end);

      if(reg)
      {
        vector3d hit, hitn;

        if ( find_intersection( target, end,
                                 reg,
                                 FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                                 &hit, &hitn,
                                 NULL, NULL ) )
        {
          target = hit + hitn;
        }
      }
      assert( target.is_valid() );
    }
    break;

    default:
      break;
  }

  assert(target.is_valid());
  owner->goto_position(target, 1.5f, true);
  owner->force_lookat(target);
}

void investigate_ai_goal::going_out_of_service()
{
  complete = true;

  owner->release_lookat();

  owner->goto_position(get_my_entity()->get_abs_position(), 3.0f, true);

  ai_goal::going_out_of_service();
}







alarm_pusher_ai_goal *alarm_pusher_ai_goal::pushers[2] = { NULL, NULL };
alarm_pusher_ai_goal::alarm_pusher_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("alarm_pusher");
  go_hit_alarm = false;
  alarm_button = NULL;
}

alarm_pusher_ai_goal::~alarm_pusher_ai_goal()
{
}


AI_GOAL_MAKE_COPY_MAC(alarm_pusher);


rational_t alarm_pusher_ai_goal::frame_advance(time_value_t t)
{
  if(owner->reached_dest())
  {
    if(alarm_button && alarm_button->get_state() == switch_obj::_SWITCH_OFF)
    {
      alarm_button->flick();
      go_hit_alarm = false;
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t alarm_pusher_ai_goal::calculate_priority(time_value_t t)
{
  if(alarm_button && alarm_button->get_state() == switch_obj::_SWITCH_OFF && (go_hit_alarm || (owner->investigated_cue() || owner->new_threat() || owner->lost_threat())))
  {
    if(pushers[0] == NULL || pushers[0] == this)
      priority = 1.5f;
    else if(pushers[1] == NULL || pushers[1] == this)
      priority = 0.15f;
    else
      priority = 0.0f;
  }
  else
    priority = 0.0f;

  return(priority);
}

void alarm_pusher_ai_goal::going_into_service()
{
  ai_goal::going_into_service();
  go_hit_alarm = true;

  assert(alarm_button);

  if(pushers[0] == NULL)
    pushers[0] = this;
  else if(pushers[1] == NULL)
    pushers[1] = this;
  else
    assert(0);

  owner->goto_position(alarm_button->get_abs_position(), 1.0f, true);
  owner->force_lookat(alarm_button->get_abs_position());
}

void alarm_pusher_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
  go_hit_alarm = false;

  if(pushers[0] == this)
  {
    pushers[0] = pushers[1];
    pushers[1] = NULL;
  }
  else if(pushers[1] == this)
    pushers[1] = NULL;
  else
    assert(0);

  owner->goto_position(get_my_entity()->get_abs_position(), 3.0f, true);
  owner->release_lookat();
}

void alarm_pusher_ai_goal::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "switch")
  {
    serial_in(fs, &label);
    label.to_upper();
    entity *ent = g_world_ptr->get_entity(label);

    if(ent != NULL && ent->get_flavor() == ENTITY_SWITCH)
      alarm_button = (switch_obj *)ent;
    else
      error("'%s' is not a valid switch object");
  }
  else
    ai_goal::handle_chunk(fs, label);
}








cover_test_ai_goal::cover_test_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("cover_test");
}

cover_test_ai_goal::~cover_test_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(cover_test);

rational_t cover_test_ai_goal::frame_advance(time_value_t t)
{
  if(owner->reached_dest() && owner->get_num_attackers() > 0)
  {
    vector3d pos;
    owner->find_cover_pt(owner->get_attacker(0)->get_my_entity()->get_abs_position(), pos);
    owner->goto_position(pos, 1.0f, true);
  }

  return(ai_goal::frame_advance(t));
}

rational_t cover_test_ai_goal::calculate_priority(time_value_t t)
{
  priority = 10.0f;
  return(priority);
}

void cover_test_ai_goal::going_into_service()
{
  ai_goal::going_into_service();
}

void cover_test_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
}






movement_test_ai_goal::movement_test_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("movement_test");
  for(int i=0; i<10; ++i)
    timers[i] = 0.0f;
}

movement_test_ai_goal::~movement_test_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(movement_test);

vector3d mdir = ZVEC;
rational_t mang = 0.0f;


rational_t movement_test_ai_goal::frame_advance(time_value_t t)
{
  static int mode = 0;
  for(int i=0; i<10; ++i)
  {
    timers[i] -= t;
    if(timers[i] < 0.0f)
      timers[i] = 0.0f;
  }

  vector3d pos = get_my_entity()->get_abs_position();
  //vector3d right = get_my_entity()->get_abs_po().get_x_facing();
  //vector3d forward = get_my_entity()->get_abs_po().get_z_facing();

  po the_po;
  the_po.set_rotate_y(t*PI*0.25f);
  mdir = the_po.non_affine_slow_xform(mdir);
//  the_po.set_rotate_y(((mode*2)+1)*PI/8.0f);
//  mdir = the_po.non_affine_slow_xform(forward);


  if(timers[0] <= 0.0f)
  {
    vector3d pt;

    switch(mode)
    {
/*
      case 0:
        pt = pos + forward*3.0f;
        break;

      case 1:
        pt = (forward+right);
        pt.normalize();
        pt = pos + (pt)*3.0f;
        break;

      case 2:
        pt = pos + right*3.0f;
        break;

      case 3:
        pt = (forward-right);
        pt.normalize();
        pt = pos + (pt)*3.0f;
        break;
*/
      default:
        pt = pos + mdir*10.0f;// *(forward*PLUS_MINUS_ONE*5.0f) + (right*PLUS_MINUS_ONE*5.0f);
        break;
    }

    owner->jockey_to(pt);
//    extern void clear_debug_spheres();
//    extern void add_debug_sphere(vector3d pos, rational_t radius);
//    clear_debug_spheres();
//    add_debug_sphere(pt, 1.0f);

    mode++;
//    if(mode >= 4)
//      mode = 0;

//    timers[0] = 5.0f;
    timers[0] = 0.0f;
  }

  return(ai_goal::frame_advance(t));
}

rational_t movement_test_ai_goal::calculate_priority(time_value_t t)
{
  priority = 10.0f;
  return(priority);
}

void movement_test_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  stringx anima("IDLE");
  get_ai_interface()->play_animation(anima, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));
}

void movement_test_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
}












path_test_ai_goal::path_test_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("path_test");
  timer = 15.0f;
}

path_test_ai_goal::~path_test_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(path_test);


rational_t path_test_ai_goal::frame_advance(time_value_t t)
{
  if(!moving)
  {
    timer -= t;
    if(timer <= 0.0f)
    {
      owner->goto_position(g_world_ptr->get_hero_ptr()->get_abs_position());
      moving = true;
    }
  }

  if(moving)
  {
    if(owner->reached_dest())
    {
      moving = false;
      timer = 15.0f;
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t path_test_ai_goal::calculate_priority(time_value_t t)
{
  priority = 10.0f;
  return(priority);
}

void path_test_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  stringx anima("IDLE");
  get_ai_interface()->play_animation(anima, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));

  moving = false;
  timer = 15.0f;
}

void path_test_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
}












gun_test_ai_goal::gun_test_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("gun_test");
}

gun_test_ai_goal::~gun_test_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(gun_test);


rational_t gun_test_ai_goal::frame_advance(time_value_t t)
{
  get_ai_interface()->set_aimer_target(r_arm_aimer, owner->get_threat()->get_abs_position());
  get_ai_interface()->set_aimer_target(l_arm_aimer, owner->get_threat()->get_abs_position());

  if(actions.empty())
  {
    test_timer1 -= t;
    if(test_timer1 <= 0.0f)
    {
      gun *gunny = (gun *)get_my_entity()->find_item_by_id("r_gun");
      if(gunny)
      {
        fire_gun_ai_action *act = NEW fire_gun_ai_action(this);

        if(rounds)
          act->setup_rounds(gunny, 3);
        else
          act->setup_time(gunny, 2.0f);

        add_action(act);
      }

      gun *gunny2 = (gun *)get_my_entity()->find_item_by_id("l_gun");
      if(gunny2)
      {
        fire_gun_ai_action *act = NEW fire_gun_ai_action(this);

        if(rounds)
          act->setup_rounds(gunny2, 3);
        else
          act->setup_time(gunny2, 2.0f);

        add_action(act);
      }

      rounds = !rounds;

      test_timer1 = 3.0f;
    }
  }

  return(ai_goal::frame_advance(t));
}

rational_t gun_test_ai_goal::calculate_priority(time_value_t t)
{
  priority = owner->get_threat() ? 1.0f : 0.0f;
  return(priority);
}

void gun_test_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

  stringx anima("IDLE");
  get_ai_interface()->play_animation(anima, AI_ANIM_IDLE, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_TWEEN));

  gun *gunny = (gun *)get_my_entity()->find_item_by_id("r_gun");
  if(gunny)
  {
    stringx anim_to_find = "GUN_AIM_R";
    get_ai_interface()->play_animation(anim_to_find, AI_ANIM_AIM_A, (ANIM_TWEEN | ANIM_RELATIVE_TO_START | ANIM_NONCOSMETIC | ANIM_LOOPING));
    get_ai_interface()->set_aimer_active(r_arm_aimer, true);
    get_ai_interface()->set_aimer_target(r_arm_aimer, owner->get_threat()->get_abs_position());

    if(!gunny->is_targeting_active())
      gunny->activate_targeting();
  }

  gun *gunny2 = (gun *)get_my_entity()->find_item_by_id("l_gun");
  if(gunny2)
  {
    stringx anim_to_find = "GUN_AIM_L";
    get_ai_interface()->play_animation(anim_to_find, AI_ANIM_AIM_B, (ANIM_TWEEN | ANIM_RELATIVE_TO_START | ANIM_NONCOSMETIC | ANIM_LOOPING));
    get_ai_interface()->set_aimer_active(l_arm_aimer, true);
    get_ai_interface()->set_aimer_target(l_arm_aimer, owner->get_threat()->get_abs_position());

    if(!gunny2->is_targeting_active())
      gunny2->activate_targeting();
  }

  test_timer1 = 3.0f;
  rounds = true;
}

void gun_test_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();

  get_ai_interface()->set_aimer_active(r_arm_aimer, false);
  get_ai_interface()->set_aimer_active(l_arm_aimer, false);

  gun *gunny = (gun *)get_my_entity()->find_item_by_id("r_gun");
  if(gunny && gunny->is_targeting_active())
    gunny->deactivate_targeting();

  gun *gunny2 = (gun *)get_my_entity()->find_item_by_id("l_gun");
  if(gunny2 && gunny2->is_targeting_active())
    gunny2->deactivate_targeting();

  get_my_entity()->kill_anim( AI_ANIM_IDLE );
  get_my_entity()->kill_anim( AI_ANIM_AIM_A );
  get_my_entity()->kill_anim( AI_ANIM_AIM_B );
}












anim_test_ai_goal::anim_test_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("anim_test");
  for(int i=0; i<10; ++i)
    timers[i] = 0.0f;

  blenda = 1.0f;
  blendb = 0.0f;
  blenda_dir = -1.0f;
  blendb_dir = 1.0f;
}

anim_test_ai_goal::~anim_test_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(anim_test);

rational_t anim_test_ai_goal::frame_advance(time_value_t t)
{
  for(int i=0; i<10; ++i)
  {
    timers[i] -= t;
    if(timers[i] < 0.0f)
      timers[i] = 0.0f;
  }

  blenda += blenda_dir*t*0.1f;
  if(blenda <= 0.0f)
  {
    blenda = 0.0f;
    blenda_dir = -blenda_dir;
  }
  if(blenda >= 1.0f)
  {
    blenda = 1.0f;
    blenda_dir = -blenda_dir;
  }

  blendb += blendb_dir*t*0.1f;
  if(blendb <= 0.0f)
  {
    blendb = 0.0f;
    blendb_dir = -blendb_dir;
  }
  if(blendb >= 1.0f)
  {
    blendb = 1.0f;
    blendb_dir = -blendb_dir;
  }

  entity_anim_tree *tree = get_my_entity()->get_anim_tree( AI_ANIM_FULL_BODY );


  tree->set_blend(blenda, blendb);


  return(ai_goal::frame_advance(t));
}

rational_t anim_test_ai_goal::calculate_priority(time_value_t t)
{
  priority = 10.0f;
  return(priority);
}

void anim_test_ai_goal::going_into_service()
{
  stringx anima("FLY_ASCEND");
  stringx animb("FLY_FORWARD");
  const stringx &resa = get_my_entity()->has_animation_ifc() ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(anima) : empty_string;
  const stringx &resb = get_my_entity()->has_animation_ifc() ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(animb) : empty_string;

  get_my_entity()->kill_anim( AI_ANIM_FULL_BODY );
  if( resa != empty_string && resb != empty_string )
    get_my_entity()->play_anim( AI_ANIM_FULL_BODY, resa, resb, blenda, blendb, 0.0f, ANIM_RELATIVE_TO_START | ANIM_LOOPING);

  ai_goal::going_into_service();
}

void anim_test_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
}







#endif
