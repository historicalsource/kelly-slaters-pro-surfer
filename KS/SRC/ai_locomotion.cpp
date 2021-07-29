#include "global.h"

#include "ai_locomotion.h"
#include "ai_interface.h"
// BIGCULL #include "ai_constants.h"
#include "entity.h"
#include "collide.h"
#include "hard_attrib_interface.h"
#include "animation_interface.h"
#include "wds.h"
#include "terrain.h"
#include "profiler.h"
#include "random.h"

const rational_t safe_atan2_ZVEC = safe_atan2( 0.0f, 1.0f );

const char* ai_locomotion::jockey_anims[_JOCKEY_ANIMS] =
{
  "JOCKEY_F",
  "JOCKEY_FR",
  "JOCKEY_R",
  "JOCKEY_BR",
  "JOCKEY_B",
  "JOCKEY_BL",
  "JOCKEY_L",
  "JOCKEY_FL",
  "FLY_IDLE"
};

ai_locomotion::ai_locomotion(ai_interface *own)
{
  owner = own;
  assert(owner);

  current_path_graph = NULL;
  current_path.clear();

  patrol_id = 0;
  xz_movement = false;

  start_pos = ZEROVEC;
  last_pos = ZEROVEC;
  target_pos = ZEROVEC;
  tgtrange = 1.0f;
  running_speed = false;

  patrol_radius = 0.5f;
  in_service = false;
  wait_until_reached = false;

  facing = false;
  playing_face_anim = false;
  face_dir = ZVEC;

  type = LOCOMOTION_NONE;

  turn_speed = 1.0f;

  repulsion_wait_timer = 0.0f;
  repulsion_wait = false;


  jockey_anim_a = _JOCKEY_ANIMS;
  jockey_anim_b = _JOCKEY_ANIMS;
  jockey_timer = -1.0f;
  jockey = false;
  jockey_speed = 2.0f;

  use_45_jockey = true;
  wait_for_facing = false;
}

ai_locomotion::~ai_locomotion()
{
}

entity *ai_locomotion::get_my_entity() const
{
  return(owner->get_my_entity());
}

ai_locomotion *ai_locomotion::make_copy(ai_interface *own)
{
  ai_locomotion *loco = NEW ai_locomotion(own);
  loco->copy(this);
  return((ai_locomotion *)loco);
}

void ai_locomotion::copy(ai_locomotion *b)
{
  patrol_radius = b->patrol_radius;
  patrol_id = b->patrol_id;
  xz_movement = b->xz_movement;
}


bool ai_locomotion::set_destination(const vector3d &pos, rational_t radius, bool fast, bool path_find, bool force_finish)
{
  if(force_finish || !wait_until_reached)
  {
    wait_until_reached = force_finish;
    start_pos = get_my_entity()->get_abs_position();
    last_pos = start_pos;
    target_pos = pos;
    tgtrange = radius;

    if((start_pos - target_pos).length2() <= (tgtrange*tgtrange))
    {
      if(jockey)
        stop_jockey();

      if(in_service)
        going_out_of_service();

      running_speed = fast;
    }
    else
    {
      if(jockey)
        stop_jockey();

      if(in_service && running_speed != fast)
        going_out_of_service();

      running_speed = fast;

      facing = false;

      if(!in_service)
        going_into_service();
    }

    return(true);
  }
  else
    return(false);
}

bool ai_locomotion::set_facing(const vector3d &dir, rational_t mod)
{
  if(!in_service)
  {
    facing = true;
    face_dir = dir;
    face_dir.normalize();

    turn_speed = mod;
  }
  else
    facing = false;

  return(facing);
}


bool ai_locomotion::set_facing_point(const vector3d &pt, rational_t mod)
{
  if(!in_service)
  {
    facing = true;

    face_dir = pt - get_my_entity()->get_abs_position();
    face_dir.normalize();

    turn_speed = mod;
  }
  else
    facing = false;

  return(facing);
}


bool ai_locomotion::frame_advance(time_value_t t)
{
  bool reached = true;

  if(!get_my_entity()->is_alive() || get_my_entity()->playing_scene_anim())
  {
    if(jockey)
      stop_jockey();

    if(in_service)
      going_out_of_service();

#if 0 // BIGCULL
    if(!get_my_entity()->anim_finished(AI_ANIM_LOCOMOTION))
      get_my_entity()->kill_anim(AI_ANIM_LOCOMOTION);
    if(!get_my_entity()->anim_finished(AI_ANIM_JOCKEY))
      get_my_entity()->kill_anim(AI_ANIM_JOCKEY);
#endif // BIGCULL
  }
  else
  {
    if(repulsion_wait)
    {
      repulsion_wait_timer -= t;
      if(repulsion_wait_timer <= 0.0f)
      {
        going_out_of_service();
        going_into_service();
      }

      reached = false;
    }
    else
    {
      if(process_movement(t))
      {
        if(in_service)
          going_out_of_service();

        wait_until_reached = false;

        reached = true;
      }
      else
        reached = false;
    }

    if(facing)
    {
      if(!playing_face_anim && get_my_entity()->anim_finished( AI_ANIM_LOCOMOTION ) && !jockey)
      {
        static const char* anim = "FACING";
        get_ai_interface()->play_animation(anim, AI_ANIM_LOCOMOTION, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_NONCOSMETIC | ANIM_TWEEN));

        playing_face_anim = true;
      }

      rational_t target_angle = get_xz_rotation_to_dir(face_dir);
      rational_t step_angle = target_angle;
      rational_t pin = (get_my_entity()->has_hard_attrib_ifc() ? get_my_entity()->hard_attrib_ifc()->get_max_turn_rate() : 5.0f) * t * turn_speed;
      if( target_angle > pin )
        step_angle = pin;
      else if( target_angle < -pin )
        step_angle = -pin;
      else
      {
        facing = false;

        if(playing_face_anim)
        {
          get_my_entity()->kill_anim( AI_ANIM_LOCOMOTION );
          playing_face_anim = false;
        }
      }

      apply_rotation( step_angle );
    }

    if(wait_for_facing)
    {
      assert(!in_service);

      reached = false;

      if(!facing)
        going_into_service();
    }

    if(jockey)
    {
      if(jockey_timer >= 0.0f)
      {
        jockey_timer -= t;
        if(jockey_timer <= 0.0f)
          stop_jockey();
        else
          adjust_jockey_animation(jockey_dir, t);
      }
      else
      {
        vector3d delta = jockey_pos - get_my_entity()->get_abs_position();

        rational_t length = 0.0f;

        switch(type)
        {
          case LOCOMOTION_WALK:
            length = delta.xz_length2();
            break;

          default:
            length = delta.length2();
            break;
        }

        if(dot(delta, jockey_dir) <= 0.0f || length <= (0.2f*0.2f))
          stop_jockey();
        else if(delta.xz_length2() >= 1.0f || type != LOCOMOTION_WALK)
        {
          delta.normalize();
          jockey_dir = delta;
          adjust_jockey_animation(jockey_dir, t);
        }
      }

      if(jockey)
      {
        if((last_jockey_pos - get_my_entity()->get_abs_position()).length2() <= 0.25f)
          jockey_stuck_timer += t;
        else
        {
          last_jockey_pos = get_my_entity()->get_abs_position();
          jockey_stuck_timer = 0.0f;
        }

        if(jockey_stuck_timer > 0.75f)
          stop_jockey();
      }

      if(!in_service)
        reached = !jockey;
    }
    else
      jockey_stuck_timer = 0.0f;
  }

  return(reached);
}


/*
#define _LOOKUP_SIZE        4096
#define _LOOKUP_SIZE_DIV2   (_LOOKUP_SIZE / 2)
#define _LOOKUP_MOD         ((float)_LOOKUP_SIZE_DIV2)
#define _LOOKUP_MOD_INV     (1.0f / _LOOKUP_MOD)
#define cos_to_lookup(a)    (((int)((a)*_LOOKUP_MOD))+_LOOKUP_SIZE_DIV2)
#define lookup_to_cos(a)    (((float)((a)-_LOOKUP_SIZE_DIV2))*_LOOKUP_MOD_INV)

rational_t acos_lookup_table[_LOOKUP_SIZE+1];
void init_acos_table()
{
  static bool table_initted = false;

  if(!table_initted)
  {
    table_initted = true;
    for(int i=0; i<_LOOKUP_SIZE+1; ++i)
      acos_lookup_table[i] = acos(lookup_to_cos(i));
  }
}

inline rational_t _fast_acos_lookup(rational_t a)
{
  return(acos_lookup_table[cos_to_lookup(a)]);
}


#ifdef TARGET_PS2
inline rational_t fast_atan2(rational_t x, rational_t y)
{
  rational_t res = 0.0f;

  if(x != 0.0f && y != 0.0f)
  {
    res = (x*x) + (y*y);
    res = __fsqrt(res);
    res = y / res;

    res = acos_lookup_table[(int)((res+1.0f)*_LOOKUP_MOD)];

    if(x < 0.0f)
      res = -res;
  }

  return(res);
}
#else
inline rational_t fast_atan2(rational_t x, rational_t y)
{
  rational_t res = 0.0f;

  if(x != 0.0f && y != 0.0f)
  {
    res = (x*x) + (y*y);
    res = __fsqrt(res);
    res = y / res;

    res = acos_lookup_table[(int)((res+1.0f)*_LOOKUP_MOD)];

    if(x < 0.0f)
      res = -res;
  }

  return(res);
}
#endif
*/

extern profiler_timer proftimer_adv_AI_rot_point;

rational_t ai_locomotion::get_xz_rotation_to_point( const vector3d &target_pos ) const
{
  START_PROF_TIMER( proftimer_adv_AI_rot_point );

  vector3d face = get_my_entity()->get_abs_po().get_facing();
  vector3d invvec = target_pos - get_my_entity()->get_abs_position();

  rational_t diff = safe_atan2( -invvec.x, invvec.z ) - safe_atan2( -face.x, face.z );

  if( diff > PI )
    diff -= _2PI;
  if( diff < -PI )
    diff += _2PI;

  STOP_PROF_TIMER( proftimer_adv_AI_rot_point );

  return( diff );
}

rational_t ai_locomotion::get_abs_xz_rotation_to_point( const vector3d &target_pos ) const
{
  START_PROF_TIMER( proftimer_adv_AI_rot_point );

  vector3d invvec = target_pos - get_my_entity()->get_abs_position();

  rational_t diff = safe_atan2( -invvec.x, invvec.z ) - safe_atan2_ZVEC;

  if( diff > PI )
    diff -= _2PI;
  if( diff < -PI )
    diff += _2PI;

  STOP_PROF_TIMER( proftimer_adv_AI_rot_point );

  return( diff );
}

rational_t ai_locomotion::get_xz_rotation_to_dir( const vector3d &dir ) const
{
  START_PROF_TIMER( proftimer_adv_AI_rot_point );

  vector3d face = get_my_entity()->get_abs_po().get_facing();

  rational_t diff = safe_atan2( -dir.x, dir.z ) - safe_atan2( -face.x, face.z );

  if( diff > PI )
    diff -= _2PI;
  if( diff < -PI )
    diff += _2PI;

  STOP_PROF_TIMER( proftimer_adv_AI_rot_point );

  return( diff );
}

rational_t ai_locomotion::get_abs_xz_rotation_to_dir( const vector3d &dir ) const
{
  START_PROF_TIMER( proftimer_adv_AI_rot_point );

  rational_t diff = safe_atan2( -dir.x, dir.z ) - safe_atan2_ZVEC;

  if( diff > PI )
    diff -= _2PI;
  if( diff < -PI )
    diff += _2PI;

  STOP_PROF_TIMER( proftimer_adv_AI_rot_point );

  return( diff );
}

bool ai_locomotion::xz_facing_point( const vector3d &target_pos, rational_t rads ) const
{
  return( __fabs( get_xz_rotation_to_point(target_pos) ) <= (rads*0.5f) );
}

void ai_locomotion::apply_rotation( rational_t rot )
{
/*
  po nupo;
  nupo.set_rotate_y( rot );
  po &mypo = get_my_entity()->get_rel_po();
  vector3d pos = mypo.get_position();
  mypo.set_position(ZEROVEC);
  mypo = mypo * nupo;
  mypo.fixup();
  mypo.set_position(pos);
  get_my_entity()->set_rel_po_no_children( mypo );
*/
  static rational_t sine;
  static rational_t cosine;
  static rational_t tmp;

  fast_sin_cos_approx( rot, &sine, &cosine );
  matrix4x4 &m = (matrix4x4 &)get_my_entity()->get_rel_po().get_matrix();

  tmp = m.x.x;
  m.x.x = tmp*cosine + m.x.z*(-sine);
  m.x.z = tmp*sine + m.x.z*cosine;

  tmp = m.y.x;
  m.y.x = tmp*cosine + m.y.z*(-sine);
  m.y.z = tmp*sine + m.y.z*cosine;

  tmp = m.z.x;
  m.z.x = tmp*cosine + m.z.z*(-sine);
  m.z.z = tmp*sine + m.z.z*cosine;
}


bool ai_locomotion::set_path( const vector3d& dest, rational_t additional_weight_mod, bool force_path )
{
  START_PROF_TIMER(proftimer_adv_AI_pathfinding);
  clear_path();

  bool path_it = false;

  ai_polypath *global_path = g_world_ptr->get_world_path();
  if(global_path != NULL)
  {
    const vector3d &start = get_my_entity()->get_abs_position();
    ai_polypath_cell *start_cell = global_path->closest_cell(start, get_my_entity()->get_primary_region(), get_my_entity());
    ai_polypath_cell *dest_cell  = global_path->closest_cell(dest, NULL, get_my_entity());

    assert(start_cell && dest_cell);

    if(!global_path->los_test(start_cell, start, dest_cell, dest))
      path_it = global_path->find_path(my_path, start_cell, start, dest_cell, dest);
  }

/*
  if ( current_path_graph != NULL )
  {
    bool coll_active = get_my_entity()->are_collisions_active();
    get_my_entity()->set_collisions_active(false, false);

    // find the path!
    const vector3d& pos = get_my_entity()->get_abs_position();
    region_node* rn = get_my_entity()->get_primary_region();
    vector3d hitp, hitn;
    bool intersect = false;
    if ( !rn
//      || (force_path || forced_path())
      || (!get_my_entity()->is_active() || !get_my_entity()->is_in_active_region())
      || __fabs(pos.y - dest.y) >= 1.5f
      || (intersect = find_intersection(pos,dest,rn,FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,&hitp,&hitn))
      )
    {
      if(!intersect || (dest - hitp).length2() >= (2.0f*2.0f))
      {
        if ( current_path_graph->find_path( pos, dest, &current_path ) )
        {
          current_path_graph->use_path( &current_path, additional_weight_mod );
          path_it = true;
        }
      }
    }

    get_my_entity()->set_collisions_active(coll_active, false);
  }
*/

  STOP_PROF_TIMER(proftimer_adv_AI_pathfinding);

  return path_it;
}

void ai_locomotion::set_goto_path(rational_t mod, bool force_path)
{
  clear_path();
  use_path = set_path(target_pos, mod, force_path);

  path_tries++;
}

void ai_locomotion::clear_path()
{
  if(current_path_graph != NULL && current_path.get_num_waypoints() > 0)
    current_path_graph->restore_path(&current_path);

  current_path.clear();
}

bool ai_locomotion::crossed_point(const vector3d test_point, const vector3d &cur_pos, rational_t radius, bool force_xz)
{
  if(radius < 0.0f)
    return(false);

  vector3d delta = test_point - cur_pos;
  vector3d move_delta = test_point - start_pos;

  if(force_xz || __fabs(delta.y) < 1.0f)
  {
    move_delta.y = 0.0f;
    delta.y = 0.0f;
  }

  return(delta.length2() <= (radius*radius) || dot(delta, move_delta) < 0.0f);

/*
  vector3d delta = ( test_point - cur_pos );
  vector3d test_delta = ( test_point - last_pos );
  vector3d move = (cur_pos - last_pos);
  rational_t test_len = 0.0f;

  if ( force_xz || __fabs(delta.y) < 1.0f )
  {
    delta.y = 0.0f;
    test_delta.y = 0.0f;
    move.y = 0.0f;
  }

  vector3d dottmp = (test_point - start_pos);
  return(delta.length2() <= radius*radius || (test_len = test_delta.length() - radius) <= 0.0f || move.length2() >= test_len*test_len || dot(delta, dottmp) < 0.0f);
*/
}

bool ai_locomotion::get_next_patrol_point(const vector3d &last_pos, const vector3d &cur_pos, vector3d &next)
{
  if(current_path_graph != NULL)
    return(current_path_graph->get_next_patrol_point(last_pos, cur_pos, next, patrol_id));
  else
    next = cur_pos;

  return(false);
}

bool ai_locomotion::get_nearest_patrol_point(const vector3d &cur_pos, vector3d &patrol_pt)
{
  if(current_path_graph != NULL)
    return(current_path_graph->get_nearest_patrol_point(cur_pos, patrol_pt, patrol_id));
  else
    patrol_pt = cur_pos;

  return(false);
}



void ai_locomotion::read_data(chunk_file& fs)
{
  stringx label;
  for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
  {
    label.to_lower();
    handle_chunk(fs, label);
  }
}

void ai_locomotion::handle_chunk(chunk_file& fs, stringx &label)
{
  if(label == "xz_movement")
  {
    xz_movement = true;
  }
  else if(label == "patrol_radius")
  {
    serial_in(fs, &patrol_radius);
  }
  else if(label == "jockey_speed")
  {
    serial_in(fs, &jockey_speed);
  }
  else if(label == "no_45_jockey_anims")
  {
    use_45_jockey = false;
  }
  else if(label == "use_45_jockey_anims")
  {
    use_45_jockey = true;
  }
}





void ai_locomotion::jockey_to(const vector3d &pt)
{
  if(!in_service)
  {
    jockey_timer = -1.0f;

    jockey_pos = pt;

    jockey_dir = jockey_pos - get_my_entity()->get_abs_position();
    jockey_dir.normalize();

    jockey = true;

    adjust_jockey_animation(jockey_dir, 0.0f);

    jockey_stuck_timer = 0.0f;
    last_jockey_pos = get_my_entity()->get_abs_position();
  }
}

void ai_locomotion::jockey_dir_time(const vector3d &dir, time_value_t t)
{
  if(!in_service && t > 0.0f)
  {
    jockey_timer = t;

    jockey_dir = dir;
    jockey_dir.normalize();
    jockey_pos = get_my_entity()->get_abs_position() + dir;

    jockey = true;

    adjust_jockey_animation(jockey_dir, 0.0f);

    jockey_stuck_timer = 0.0f;
    last_jockey_pos = get_my_entity()->get_abs_position();
  }
}

void ai_locomotion::stop_jockey()
{
  if(jockey)
  {
    get_my_entity()->kill_anim( AI_ANIM_JOCKEY );

    jockey_anim_a = _JOCKEY_ANIMS;
    jockey_anim_b = _JOCKEY_ANIMS;

    jockey_timer = -1.0f;

    jockey = false;

    jockey_stuck_timer = 0.0f;
    last_jockey_pos = get_my_entity()->get_abs_position();
  }
}

bool ai_locomotion::using_animation() const
{
  switch(type)
  {
    case LOCOMOTION_WALK:
    {
      entity_anim_tree* anm = get_my_entity()->get_anim_tree( jockey ? AI_ANIM_JOCKEY : AI_ANIM_LOCOMOTION );
      return(anm!=NULL && g_world_ptr->eligible_for_frame_advance(anm));
    }
    break;

    case LOCOMOTION_DIRECT:
    case LOCOMOTION_WINGED:
    case LOCOMOTION_HELI:
    default:
      return(false);
      break;
  }

  return(false);
}

void ai_locomotion::adjust_jockey_animation(const vector3d &dir, time_value_t t)
{
  switch(type)
  {
    case LOCOMOTION_HELI:
    case LOCOMOTION_DIRECT:
      break;

    case LOCOMOTION_WINGED:
    {
      eJockeyAnim anim_a = _WINGED_IDLE;

      entity_anim_tree *anim_tree = get_my_entity()->get_anim_tree(AI_ANIM_JOCKEY);
      bool anim_change = jockey_anim_a != anim_a || !anim_tree || anim_tree->is_finished();

      if(anim_change)
      {
        jockey_anim_a = anim_a;

        const stringx &res  = get_my_entity()->has_animation_ifc() ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(jockey_anims[jockey_anim_a]) : empty_string;

        rational_t time = (anim_tree != NULL) ? anim_tree->get_time() : 0.0f;

        get_my_entity()->kill_anim( AI_ANIM_JOCKEY );
        if( res != empty_string)
          anim_tree = get_my_entity()->play_anim( AI_ANIM_JOCKEY, res, time, ANIM_TWEEN | ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_NONCOSMETIC);
      }
    }
    break;

    default:
    {
      rational_t heading = get_xz_rotation_to_dir(dir);

      eJockeyAnim anim_a;
      eJockeyAnim anim_b;
      rational_t blend_b;
      rational_t blend_a;

      bool use_right = (heading < 0.0f);
      heading = __fabs(heading);

      if(use_45_jockey)
      {
        if(heading <= DEG_TO_RAD(45.0f))
        {
          anim_a = _FORWARD;
          anim_b = use_right ? _FORWARD_R : _FORWARD_L;
          blend_b = heading / DEG_TO_RAD(45.0f);
          blend_a = 1.0f - blend_b;
        }
        else if(heading <= DEG_TO_RAD(90.0f))
        {
          anim_a = use_right ? _FORWARD_R : _FORWARD_L;
          anim_b = use_right ? _RIGHT : _LEFT;
          blend_b = (heading - DEG_TO_RAD(45.0f)) / DEG_TO_RAD(45.0f);
          blend_a = 1.0f - blend_b;
        }
        else if(heading <= DEG_TO_RAD(135.0f))
        {
          anim_a = use_right ? _RIGHT : _LEFT;
          anim_b = use_right ? _BACKWARD_R : _BACKWARD_L;
          blend_b = (heading - DEG_TO_RAD(90.0f)) / DEG_TO_RAD(45.0f);
          blend_a = 1.0f - blend_b;
        }
        else
        {
          anim_a = use_right ? _BACKWARD_R : _BACKWARD_L;
          anim_b = _BACKWARD;
          blend_b = (heading - DEG_TO_RAD(135.0f)) / DEG_TO_RAD(45.0f);
          blend_a = 1.0f - blend_b;
        }
      }
      else
      {
        if(heading <= DEG_TO_RAD(90.0f))
        {
          anim_a = _FORWARD;
          anim_b = use_right ? _RIGHT : _LEFT;
          blend_b = heading / DEG_TO_RAD(90.0f);
          blend_a = 1.0f - blend_b;
        }
        else
        {
          anim_a = use_right ? _RIGHT : _LEFT;
          anim_b = _BACKWARD;
          blend_b = (heading - DEG_TO_RAD(90.0f)) / DEG_TO_RAD(90.0f);
          blend_a = 1.0f - blend_b;
        }
      }

      entity_anim_tree *anim_tree = get_my_entity()->get_anim_tree(AI_ANIM_JOCKEY);
      bool anim_change = (jockey_anim_a != anim_a || jockey_anim_b != anim_b) || !anim_tree || anim_tree->is_finished();

      if(anim_change)
      {
        jockey_anim_a = anim_a;
        jockey_anim_b = anim_b;

        const stringx &res  = get_my_entity()->has_animation_ifc() ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(jockey_anims[jockey_anim_a]) : empty_string;
        const stringx &res2 = get_my_entity()->has_animation_ifc() ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(jockey_anims[jockey_anim_b]) : empty_string;

        rational_t time = (anim_tree != NULL) ? anim_tree->get_time() : 0.0f;

        get_my_entity()->kill_anim( AI_ANIM_JOCKEY );
        if( res != empty_string && res2 != empty_string)
          anim_tree = get_my_entity()->play_anim( AI_ANIM_JOCKEY, res, res2, blend_a, blend_b, time, ANIM_TWEEN | ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_NONCOSMETIC);
      }
      else if(anim_tree)
      {
        anim_tree->set_blend(blend_a, blend_b);
      }
    }
    break;
  }


  entity_anim_tree *anim_tree = get_my_entity()->get_anim_tree(AI_ANIM_JOCKEY);
  bool using_animation = (anim_tree != NULL && !anim_tree->is_finished()) && g_world_ptr->eligible_for_frame_advance(anim_tree);
  if ( !using_animation || (type == LOCOMOTION_DIRECT || type == LOCOMOTION_WINGED || type == LOCOMOTION_HELI) )
  {
    // send character toward local_dest at appropriate speed
    vector3d delta = (dir*jockey_speed*t);
    vector3d herenow = get_my_entity()->get_abs_position();

    // Cancel the y component of the movement on active characters
    // This is beacuse they are now allowed to climb walls due to sliding up them (Y axis movement).
    // Normally an animation moves them in the XZ only. This will make their translation act
    // like a normal animation (JDB 9/11/00)
    if(xz_movement_only() || (get_my_entity()->has_physical_ifc() && get_my_entity()->is_active() && type == LOCOMOTION_WALK))
      delta.y = 0.0f;

    if(type == LOCOMOTION_DIRECT || type == LOCOMOTION_WINGED || type == LOCOMOTION_HELI)
    {
      vector3d hit, hit_n;
      vector3d inter2 = herenow + delta;

      bool coll_active = get_my_entity()->are_collisions_active();
      get_my_entity()->set_collisions_active(false, false);
      if(find_intersection( herenow,
                            inter2,
                            get_my_entity()->get_primary_region(),
                            FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY,
                            &hit, &hit_n ))
      {
        delta.x = 0.0f;
        delta.z = 0.0f;
      }
      get_my_entity()->set_collisions_active(coll_active, false);
    }

    vector3d thepos = herenow+delta;
    get_my_entity()->set_rel_position_no_children(thepos);

    if(get_my_entity()->is_active())
    {
      // apply translation
      get_my_entity()->set_frame_delta_trans(delta, t);
    }
    else
      get_my_entity()->update_abs_po();


    // might need to do a compute_sector()
    if ( !get_my_entity()->is_active() )
      conditional_compute_sector(get_my_entity()->get_primary_region(), last_pos, get_my_entity()->get_abs_position());
  }
}

void ai_locomotion::conditional_compute_sector(region_node *rgn, const vector3d &p1, const vector3d &p2)
{
  get_my_entity()->set_needs_compute_sector(false);
  // check for intersection with portals leading from this region
  if(rgn != NULL)
  {
    sphere me_sphere( p1, get_my_entity()->get_radius() );
    if(me_sphere.get_r() < 1.0f)
      me_sphere.set_r(1.0f);

    edge_iterator tei = rgn->begin();
    edge_iterator tei_end = rgn->end();
    for ( ; tei!=tei_end; ++tei )
    {
      portal* port = (*tei).get_data();
      if ( port )
      {
        if (port->touches_sphere(me_sphere) || port->touches_segment(p1, p2))
        {
          get_my_entity()->compute_sector( g_world_ptr->get_the_terrain() );
          return;
        }
      }
/*
      region_node* dest = (*tei).get_dest();
      portal* port = (*tei).get_data();
      if ( dest && port && port->is_active() )
      {
        region* dr = dest->get_data();
        // don't bother with inactive regions or those we've already visited
        if ( dr && dr->is_active() && (port->touches_sphere(me_sphere) || port->touches_segment(p1, p2)))
        {
          get_my_entity()->compute_sector( g_world_ptr->get_the_terrain() );
          return;
        }
      }
*/
    }
  }
  else
    get_my_entity()->compute_sector( g_world_ptr->get_the_terrain() );
}



bool ai_locomotion::process_repulsion(time_value_t delta_t)
{
  // Too many problems to fix right now (not necessary yet anyway, just wanted to hurry it in, JDB.
  return(false);
/*
  // calc point in front of character
  const vector3d& herenow = get_my_entity()->get_abs_position();

  rational_t repulsion_rad = (get_my_entity()->has_hard_attrib_ifc() ? get_my_entity()->hard_attrib_ifc()->get_repulsion_radius() : (get_my_entity()->get_radius() + 1.0f));
  vector3d rel_test_pos = get_my_entity()->get_abs_po().get_facing() * repulsion_rad;
  vector3d test_pos = rel_test_pos + herenow;

  vector<ai_interface *>::iterator ai     = ai_interface::all_ai_interfaces.begin();
  vector<ai_interface *>::iterator ai_end = ai_interface::all_ai_interfaces.end();
  while(ai != ai_end)
  {
    ai_interface *control_ptr = (*ai);
    ++ai;

    if( control_ptr != NULL &&
        control_ptr != owner &&
        control_ptr->get_my_entity()->is_visible() &&
        !control_ptr->get_my_entity()->is_hero() &&
        !owner->is_enemy(control_ptr)
        )
    {
      rational_t other_repulsion_rad = (control_ptr->get_my_entity()->has_hard_attrib_ifc() ? control_ptr->get_my_entity()->hard_attrib_ifc()->get_repulsion_radius() : (control_ptr->get_my_entity()->get_radius() + 1.0f));
      vector3d vect = control_ptr->get_my_entity()->get_abs_position();
      vect -= test_pos;

      if( vect.length2() < other_repulsion_rad*other_repulsion_rad )
      {
        vector3d relpos = control_ptr->get_my_entity()->get_abs_position() - herenow;
        rational_t angle = angle_between( relpos, rel_test_pos );
        rational_t dotp = 1.0f;

        if( __fabs( angle ) >= AI_REPULSION_ANGLE_THRESHOLD )
        {
          vector3d perpv( -rel_test_pos.z, rel_test_pos.y, rel_test_pos.x );
          dotp = dot( perpv, relpos );
        }

        vect.y = 0;

        // avoidance if they run sideways into each other
        angle = angle_between( control_ptr->get_my_entity()->get_abs_po().get_facing(), rel_test_pos );

        if( control_ptr->get_locomotion() && control_ptr->get_locomotion()->in_service && !control_ptr->get_locomotion()->repulsion_wait )
        {
          if( (angle < PI*0.5f) && (angle > PI*0.2) )
            dotp = -dotp;

          // do nothing if they are behind each other (to solve the problem of characters getting stuck in tight areas)
          if( angle < AI_REPULSION_FOLLOW_ANGLE )
            continue;
        }

        if( dotp > 0.0f )
        {
          vect.x = relpos.z;
          vect.z = -relpos.x;
        }
        else
        {
          vect.x = -relpos.z;
          vect.z = relpos.x;
        }

        vect.normalize();
        vect *= (repulsion_rad*2.0f);

        local_dest = control_ptr->get_my_entity()->get_abs_position() + vect;
        repulsion_local_dest = local_dest;
        repulsion_timer = AI_REPULSION_HOLD_TIME;

        vector<ai_interface *>::iterator ai2 = ai_interface::all_ai_interfaces.begin();
        while(ai2 != ai_end)
        {
          ai_interface *control_ptr2 = (*ai2);
          ++ai2;

          if( control_ptr2 != NULL &&
              control_ptr2 != control_ptr &&
              control_ptr2 != owner &&
              control_ptr2->get_my_entity()->is_visible() &&
              !control_ptr2->get_my_entity()->is_hero() &&
              !owner->is_enemy(control_ptr2)
              )
          {
            rational_t other_repulsion_rad2 = (control_ptr2->get_my_entity()->has_hard_attrib_ifc() ? control_ptr2->get_my_entity()->hard_attrib_ifc()->get_repulsion_radius() : (control_ptr2->get_my_entity()->get_radius() + 1.0f));
            vector3d vect2 = control_ptr2->get_my_entity()->get_abs_position();
            vect2 -= local_dest;

            if( vect2.length2() < other_repulsion_rad2*other_repulsion_rad2 )
            {
              repulsion_wait_timer = AI_REPULSION_WAIT_TIME;
              repulsion_timer = 0.0f;
              repulsion_wait = true;

              static const char* anim_idle( "IDLE" );

              const stringx &res = get_my_entity()->has_animation_ifc() ? get_my_entity()->animation_ifc()->extract_random_anim_info_id_map_anim(anim_idle) : empty_string;

              if( res != empty_string )
                get_my_entity()->play_anim( AI_ANIM_LOCOMOTION, res, 0.0f, ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_NONCOSMETIC | ANIM_TWEEN);
              else
                get_my_entity()->kill_anim( AI_ANIM_LOCOMOTION );

              return(true);
            }
          }
        }
      }
    }
  }
  return(false);
//*/
}



void ai_locomotion::going_into_service()
{
  assert(!in_service);
  in_service = true;
  wait_for_facing = false;
}

void ai_locomotion::going_out_of_service()
{
  assert(in_service);

//  clear_path();
  repulsion_wait_timer = 0.0f;
  repulsion_wait = false;

//  get_my_entity()->kill_anim( AI_ANIM_LOCOMOTION );

  in_service = false;
}

bool ai_locomotion::set_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("PATROL_RADIUS", patrol_radius);
  IFC_INTERNAL_SET_MACRO("JOCKEY_SPEED", jockey_speed);

  return(false);
}

bool ai_locomotion::get_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("PATROL_RADIUS", patrol_radius);
  IFC_INTERNAL_GET_MACRO("JOCKEY_SPEED", jockey_speed);

  return(false);
}
