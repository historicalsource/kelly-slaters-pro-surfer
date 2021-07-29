/*
character2.cpp
Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

overflow from character.cpp because of metrowerks limitation

Root class of game characters (Heroes and NPCS).  Characters are distinguished from
other entities by having hit_points, inventories, brains (or manual controllers) etc.
*/
#include "global.h"

//!#include "character.h"

#include "app.h"
//!#include "attrib.h"
#include "hwaudio.h"
#include "brain.h"
//#include "fxman.h"
#include "game.h"
#include "mcs.h"
#include "fcs.h"
#include "controller.h"
#include "msgboard.h"
#include "hwmath.h"
#include "wds.h"
//!#include "charhead.h"
#include "terrain.h"
#include "capsule.h"
#include "vm_thread.h"
#include "hinge.h"
#include "filespec.h"
#include "actuator.h"
#include "billboard.h"
#include "osdevopts.h"
#include "ladder.h"
#include "debug_tools.h"
#include "particle.h"
#include "soundgrp.h"
//P #include "memorycontext.h"
#include "entityflags.h"
#include "shock_mods.h"
#include "profiler.h"
#include "geomgr.h"
#include "collide.h"
#include "forceflags.h"
//!#include "variants.h"
#include "lightmgr.h"

#if defined(TARGET_MKS)
//#include "sg_syhw.h"
//#include "sg_gd.h"
//#include "sg_pdvib.h"
#endif

// BIGCULL #include "gun.h"
// BIGCULL #include "thrown_item.h"
//!#include "char_group.h"
#include "interface.h"
#include "optionsfile.h"

//#if defined(TARGET_MKS)
//extern vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc );
//#endif


//bool g_shield_dropper = true;
//extern stringx none_str;
extern bool g_demigod_cheat;

extern game *g_game_ptr;

//rational_t g_temp1 = 0;
//rational_t g_temp2 = 0;

vector3d g_new_position;
vector3d g_new_diff;
//character * g_chr_this;
vector3d g_cur_standing_gradient;

rational_t ACTIVATION_RADIUS = 50.0f;
//extern rational_t g_front_to_back;
//extern rational_t g_targ_waist_psi;

//rational_t BLOCK_DAMAGE_PCT = 0; //.3;

const color MELEE_OPPONENT_AMBIENT( 1.0f, 1.0f, 1.0f );

void character::read_mcs( const stringx& entity_name )
{
  // read attributes for other motion controllers
  character_head_fcs* head_fcs = NULL;

  // create player controller or brain (as appropriate) for character
  create_controllers();

  if ( limb_valid(WAIST) )
    limb_ptr(WAIST)->delete_colgeom();
  rectify_capsules();
  if (id == entity_id("HERO") )
    get_soft_attrib()->set_team(character_soft_attributes::HERO_TEAM);
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* character::make_instance( const entity_id& _id,
                                  unsigned int _flags ) const
{
  character* newchr = NEW character( _id, _flags );
  newchr->copy_instance_data( *this );
  return (entity*)newchr;
}


///////////////////////////////////////////////////////////////////////////////
// Limb tree support
///////////////////////////////////////////////////////////////////////////////

void character::attach_limb_tree()
{
  // attach limb tree
  actor::attach_limb_tree();
}

void character::release_limb_tree()
{
  // release limb tree
  actor::release_limb_tree();
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

// Algorithm is to make the capsule from waist to chest
void character::rectify_capsules()
{
  // This is a hack to not break anything old while I develop quads.  PTA 3/5/99, 6:30 AM <<<<
  collision_capsule* abscap=(collision_capsule*)get_colgeom();

  actor::rectify_capsules();
  rational_t r, len;

  // Adjust collision capsule by .ATT file
  capsule cap = abscap->get_rel_capsule();

  if ((r=hard_attrib->get_collision_capsule_radius())!=0)
  {
    cap.radius = r;
  }
  if ((len=hard_attrib->get_collision_capsule_length())!=0)
  {
    vector3d diff = cap.end-cap.base;
    if (diff.length2()>0)
      diff.normalize();
    else
      diff = YVEC;
    cap.end = cap.base + diff*len;
  }
  cap.base += hard_attrib->get_collision_capsule_base_ofs();
  cap.end += hard_attrib->get_collision_capsule_end_ofs();

  abscap->set_capsule(cap);

  // Fix up damage capsules based on modified capsule.
  damage_capsule = NEW collision_capsule(this);
  capsule c = cap;
  c.base = -c.end;
  // hackity hack--bump up damage capsule height since
  //  collision capsule is now smaller--cbb 3/7/00
  c.end.y += 1.0f;
  c.radius *= hard_attrib->get_damage_capsule_radius_scale();
  damage_capsule->set_capsule(c);
}


rational_t character::get_bump_radius()
{
  if (get_colgeom()->get_type()==collision_geometry::CAPSULE)
  {
    capsule cap = ((collision_capsule *)get_colgeom())->get_rel_capsule();
    return cap.radius;
  }
  else
    return get_radius();
}

void character::load_attributes(const char * att_filename)
{
  hard_attrib->load_attributes(att_filename);
  compute_derivitive_attributes();
  set_hit_points( get_full_hit_points() );
}


void character::compute_derivitive_attributes()
{
  soft_attrib->set_alive( true );
  if ( get_hard_attrib()->get_is_mechanical() )
    set_target_type( TARGET_TYPE_MECHANICAL );
}


//static character * g_chr;

extern rational_t g_level_time;

#define _MAX_DEATH_ROT_PER_SECOND  (30.0f)*(PI / 180.0f)
void character::frame_advance(time_value_t t)
{
  vector3d char_pos = get_abs_position();

  if( dying_countdown > 0 )//|| this == g_world_ptr->get_hero_ptr())
  {
    vector3d norm = get_cur_ground_normal();
    vector3d my_norm = get_abs_po().get_y_facing();

    norm.normalize();
    my_norm.normalize();

    rational_t dotpr = dot(norm, my_norm);
    if (dotpr < 0.995f)
    {
      rational_t ang = fast_acos(dotpr);

      po p1;
      vector3d crs = cross(norm, my_norm);
      crs.normalize();

      rational_t max_ang = t * _MAX_DEATH_ROT_PER_SECOND;

      p1.set_rot(crs, ang < max_ang ? ang : max_ang);

      vector3d pos = get_abs_po().get_abs_position();
      po newpo = get_abs_po();

      // rotate
      newpo.set_rel_position(ZEROVEC);
      newpo.add_increment(&p1);
      newpo.fixup();
      newpo.set_rel_position(pos);
      set_rel_po(newpo);
    }

    dying_countdown -= t;
    if (dying_countdown <= 0.0f)
    {
      if( this != g_world_ptr->get_hero_ptr() ) kill_me();
      return;
    }

    set_variant_tanslucency( true );
  }

  if (get_paralysis_factor())
  {
    set_velocity(ZEROVEC);
    set_acceleration_correction_factor(ZEROVEC);
    acceleration_factor = ZEROVEC;

    shock_vector = ZEROVEC;
    internal_shock_vector = ZEROVEC;

    set_control_active(false);

    return;
  }

  // update nanotech states
  if (stealth_timer > 0)
  {
    stealth_timer -= t;
    if (stealth_timer <= 0)
      raise_signal(STEALTH_OFF);
  }
  if (turbo_timer > 0)
  {
    turbo_timer -= t;
    if ( turbo_timer <= 0)
      raise_signal(TURBO_OFF);
  }
  if (shot_timer > .0f)
  {
    shot_timer -= t;
    if (shot_timer < .0f) shot_timer = .0f;
  }

  if ( is_active() )
    actor::frame_advance( t );

  if (lightmgr)
  {
    region_node* my_region_node = get_region();
    region* my_region = my_region_node ? my_region_node->get_data() : 0;
    lightmgr->set_bound_sphere(sphere(get_visual_center(),get_visual_radius()));
    lightmgr->frame_advance(my_region, t); // let it update light blending/fading
  }

  /*
  if ( lizard_brain && lizard_brain->is_active() &&
       lizard_brain->get_action()==ACTION_NONE )
  {
    lizard_brain->set_active(false);
    my_controller->set_active(true);
  }
  */

/*!
  if( g_world_ptr->get_hero_ptr()->get_melee_opponent() == this )
  {
    if( !is_selected() )
      set_selected( true );
  }
  else
!*/
    if( is_selected() )
    set_selected( false );

  adjust_collision_capsule();

  // find out if character is between hero and camera
  //
  bool old = render_translucent;
  render_translucent = false;
  if( this != g_world_ptr->get_hero_ptr() && !g_world_ptr->is_marky_cam_enabled() )
  {
    vector3d cam_pos = g_world_ptr->get_chase_cam_ptr()->get_visual_center(); //get_abs_position();
    vector3d hero_pos = g_world_ptr->get_hero_ptr()->get_visual_center();
    vector3d my_pos = get_visual_center();
    vector3d hit;
    vector3d relcam = hero_pos - cam_pos;
    vector3d relchar = hero_pos - my_pos;

    rational_t d = dot( relcam, relchar );

    const collision_capsule *cc = (collision_capsule *)get_colgeom();
    if( collide_segment_capsule( cam_pos, hero_pos, *cc, hit ) && (d > 0) )
    {
      render_translucent = true;

      if( old != render_translucent )
        set_variant_tanslucency( false );
    }

    if( !render_translucent && old )
      reset_variant_translucency();
  }
  else
  {
    if( melee_opponent && (!melee_opponent->is_alive() || !is_in_melee_range( melee_opponent )) )
      melee_opponent = NULL;
  }

  if( !effectively_standing ) air_timer += t;
}

void character::set_variant_tanslucency( bool use_death_fade_alpha, rational_t use_this_alpha_instead )
{
  // adjust translucency of any variant attachments
  if ( current_variant )
  {
    vector<entity*>::const_iterator i = current_variant->get_entities().begin();
    vector<entity*>::const_iterator i_end = current_variant->get_entities().end();
    for ( ; i!=i_end; ++i )
    {
      entity* e = *i;
      color32 c = e->get_render_color();
      uint8 dfa = (use_death_fade_alpha ? get_death_fade_alpha() : use_this_alpha_instead) * 0xFF;
      if ( c.get_alpha() > dfa )
      {
        c.set_alpha( dfa );
        e->set_render_color( c );
      }
    }
  }
}

void character::reset_variant_translucency( void )
{
  if ( current_variant )
  {
    vector<entity*>::const_iterator i = current_variant->get_entities().begin();
    vector<entity*>::const_iterator i_end = current_variant->get_entities().end();
    attachment_list::const_iterator ai = current_variant->get_descriptor()->get_attachments().begin();
    for ( ; i!=i_end; ++i,++ai )
    {
      entity* e = *i;
      attachment_info* a = *ai;
      if ( current_variant->use_alternative_color()
        && a->is_flagged(attachment_info::USE_CHARACTER_COLOR) )
      {
        e->set_render_color( current_variant->get_descriptor()->get_character_color() );
      }
      else
      {
        color32 c = e->get_render_color();
        c.set_alpha( 0xFF );
        e->set_render_color( c );
      }
    }
  }
}

rational_t character::get_death_fade_alpha() const
{
  if ( dying_countdown>0 && dying_countdown<2.0f )
  {
    return dying_countdown * 0.5f;
  }
  return 1.0f;
}


void character::frame_done()
{
  actor::frame_done();
  moved_last_frame = false;
  set_last_po( get_colgeom_root_po() );
}


int character::get_stealth_cost()
{
  switch (g_game_ptr->get_optionsfile()->get_option(GAME_OPT_DIFFICULTY))
  {
    case 0: return get_hard_attrib()->get_stealth_cost_easy();
    case 1: return get_hard_attrib()->get_stealth_cost_medium();
    case 2: return get_hard_attrib()->get_stealth_cost_hard();
  }

  return 100000;
}

rational_t character::get_stealth_time()
{
  switch ( g_game_ptr->get_optionsfile()->get_option(GAME_OPT_DIFFICULTY) )
  {
    case 0: return get_hard_attrib()->get_stealth_time_easy();
    case 1: return get_hard_attrib()->get_stealth_time_medium();
    case 2: return get_hard_attrib()->get_stealth_time_hard();
  }
  return 1000.0f;
}

int character::get_turbo_cost()
{
  switch (g_game_ptr->get_optionsfile()->get_option(GAME_OPT_DIFFICULTY))
  {
    case 0: return get_hard_attrib()->get_turbo_cost_easy();
    case 1: return get_hard_attrib()->get_turbo_cost_medium();
    case 2: return get_hard_attrib()->get_turbo_cost_hard();
  }

  return 100000;
}

rational_t character::get_turbo_time()
{
  switch ( g_game_ptr->get_optionsfile()->get_option(GAME_OPT_DIFFICULTY) )
  {
    case 0: return get_hard_attrib()->get_turbo_time_easy();
    case 1: return get_hard_attrib()->get_turbo_time_medium();
    case 2: return get_hard_attrib()->get_turbo_time_hard();
  }
  return 1000.0f;
}

bool character::set_stealth( bool torf )
{
  if ( torf )
  {
    // engage stealth mode if not already engaged
    int energy = get_nanotech_energy();
    int cost = get_stealth_cost();
    if ( energy && energy>=cost && stealth_timer<=0 )
    {
      set_nanotech_energy( energy - cost  );
      raise_signal( STEALTH_ON );

      if(g_world_ptr->get_dread_net())
        g_world_ptr->get_dread_net()->add_cue(dread_net::get_cue_type("ACTIVATE_STEALTH"), this);

      return true;
    }
  }
  else
  {
    if ( stealth_timer > 0 )
    {
      // disengage stealth mode
      stealth_timer = 0;
      raise_signal( STEALTH_OFF );
      return true;
    }
  }
  return false;
}

void character::set_stealth_timer()
{
  stealth_timer = get_stealth_time();
}


bool character::set_turbo( bool torf )
{
  if ( torf )
  {
    // engage turbo mode if not already engaged
    int energy = get_nanotech_energy();
    int cost = get_turbo_cost();
    if ( energy && energy>=cost && turbo_timer<=0 )
    {
      set_nanotech_energy( energy - cost  );
      raise_signal( TURBO_ON );

      if ( g_world_ptr->get_dread_net() )
        g_world_ptr->get_dread_net()->add_cue( dread_net::get_cue_type("ACTIVATE_TURBO"), this );

      return true;
    }
  }
  else
  {
    // disengage turbo mode
    if ( turbo_timer > 0 )
    {
      turbo_timer = 0;
      raise_signal( TURBO_OFF );
      return true;
    }
  }
  return false;
}

void character::set_turbo_timer()
{
  turbo_timer = get_turbo_time();
}


light_manager* character::get_light_set() const
{
  if (lightmgr) return lightmgr;
  return actor::get_light_set();
}

render_flavor_t character::render_passes_needed() const
{
  #ifndef BUILD_BOOTABLE
  extern int g_render_capsules;
  #endif

  render_flavor_t passes = actor::render_passes_needed();

  if ( ( stealth_timer > 0 )
    || ( get_death_fade_alpha() < 1.0f )
    || ( get_char_translucency() < 1.0f )
    || render_translucent
    )
  {
    passes = RENDER_TRANSLUCENT_PORTION;  // no opaque
  }
  #ifndef BUILD_BOOTABLE
  else if ( g_render_capsules )
  {
    passes |= RENDER_TRANSLUCENT_PORTION;
  }
  #endif

  return passes;
}


rational_t character::get_translucency_for_stealth( rational_t *stealth_pct )
{
  rational_t s_pct = stealth_timer / get_stealth_time();
  if ( stealth_pct )
    *stealth_pct = s_pct;
  if ( s_pct<=0.0F || s_pct>=1.0F )
    return 1.0F;
  // subtle fluctuation in translucency level
  return 0.7F - 0.3f * fabs( fast_sin((s_pct - 0.5F) * (PI * 2.0F / 0.45F)) );
}

rational_t character::get_char_translucency() const
{
  return char_translucency;
}

void character::set_char_translucency(rational_t t)
{
  char_translucency = t;

  if(char_translucency < 0.0f)
    char_translucency = 0.0f;
  if(char_translucency > 1.0f)
    char_translucency = 1.0f;
}

extern profiler_timer proftimer_render_character;

void character::render(rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct)
{
  proftimer_render_character.start();

  #if defined(DEBUG) && 0 // testing
  //actor::render( detail, flavor, entity_translucency_pct * 0.5f, FORCE_TRANSLUCENCY );
  //return;
  #endif

  if ( is_alive_or_dying() /*|| is_scripted_death()*/ || this == g_world_ptr->get_hero_ptr() )
  {
    rational_t stealth_pct, final_pct;

    final_pct = get_translucency_for_stealth( &stealth_pct );
    if ( stealth_pct>0.0F && stealth_pct<1.0F ) // don't begin stealth fx in pause screen!
    {
      assert( flavor & RENDER_TRANSLUCENT_PORTION );
      actor::render( detail, flavor, entity_translucency_pct * final_pct, FORCE_ADDITIVE_BLENDING|FORCE_NO_LIGHT);
    }
    else
    {
      // determine amount of translucency, if any
      rational_t trans_factor = get_char_translucency();
      if ( render_translucent )
        trans_factor = min( trans_factor, (char_type == CHAR_TYPE_IGOR) ? 0.7f : 0.4f);  // keep igor more opaque or otherwise some of his body parts almost disappear
      trans_factor = min( trans_factor, get_death_fade_alpha() );

      if ( trans_factor < 1.0f )
        actor::render( detail, flavor, entity_translucency_pct*trans_factor, FORCE_TRANSLUCENCY );
      else
        actor::render( detail, flavor, entity_translucency_pct );
    }
  }

  proftimer_render_character.stop();
}

void character::apply_damage( int damage, const vector3d &pos, const vector3d &norm, int _damage_type, entity *attacker, int dmg_flags )
{
#define GETTING_SHOT_TIME   0.5f
  vector3d& raw_dir = vector3d(-norm.x, -norm.y, -norm.z);
  int       new_hp  = soft_attrib->get_hit_points();
  int       new_ap  = soft_attrib->get_armor_points();
  bool      damaged = false;
  bool      died    = false;
  static hires_clock_t pain_timer;

  // cases in which we do nothing:
  if ( //!is_active() ||        // not active (doesn't apply anymore)
       !is_alive()              // not alive
    || new_hp <= 0              // have no hit points
    || damage == 0              // receiving no damage
    || is_dying()               // dying
    || (is_hero() && god_mode)  // hero in god mode
    )
  {
    return;
  }

  // set up damage info (before raising signal)
  if ( attacker )
  {
    if ( attacker->is_a_handheld_item() )
    {
      dmg_info.attacker = ((handheld_item *)attacker)->get_owner();
      dmg_info.attacker_itm = (item *)attacker;
    }
    else
    {
      dmg_info.attacker = attacker;
      dmg_info.attacker_itm = NULL;
    }
  }
  else
  {
    dmg_info.attacker = NULL;
    dmg_info.attacker_itm = NULL;
  }

  dmg_info.damage = damage;
  dmg_info.dir = raw_dir;
  dmg_info.loc = pos;
  dmg_info.type = _damage_type;
  dmg_info.push_wounded = true;
  dmg_info.push_death = true;
  dmg_info.flags = dmg_flags;

  // disable the radio if it is a silent kill (before raising the signal)
  if ( dmg_info.type == DAMAGE_SILENT_KILL && get_brain() != NULL)
    get_brain()->disable_radio();

  // raise signal indicating that I have been damaged
  raise_signal( DAMAGED );
  // also raise signal on all char_groups to which I belong
  vector<char_group*>::const_iterator i = char_groups.begin();
  vector<char_group*>::const_iterator i_end = char_groups.end();
  for ( ; i!=i_end; ++i )
    (*i)->raise_signal( char_group::DAMAGED );

  // Damage calculations MUST come AFTER the signal, because the signal can change the damage_resist_modifier in a callback (see bosses)
  if ( is_hero() )
  {
    // difficulty level affects damage dealt to hero
    const rational_t diff_factor[3] = { 0.5f, 1.0f, 1.0f };
    extern game *g_game_ptr;
    damage = int( rational_t(damage) * diff_factor[g_game_ptr->get_optionsfile()->get_option(GAME_OPT_DIFFICULTY)] + 0.5f );
  }
/*!  else if ( attacker && attacker->is_hero() && g_world_ptr->get_hero_ptr()->get_god_mode() )
  {
    // hero in god mode does infinite damage
    damage = 50000;
  }
!*/
  else if ( _damage_type == DAMAGE_SILENT_KILL )
  {
    // a "silent kill" always does exactly the necessary damage
    damage = new_hp + new_ap;
  }
  else
  {
    // otherwise, I might be somewhat resistant
    damage = (int)((((rational_t)damage) * damage_resist_modifier) + 0.5f);
  }

  // place a cap on self-inflicted damage
  if ( attacker && attacker->is_a_handheld_item()
    && ((handheld_item*)attacker)->get_owner() == this
    && damage > get_hard_attrib()->get_max_self_inflicted_damage()
    )
  {
    damage = get_hard_attrib()->get_max_self_inflicted_damage();
  }

  if ( damage < 0 )
    damage = 0;

  // raise appropriate dread_net cue(s)
  switch ( dmg_info.type )
  {
  case DAMAGE_MELEE:
  case DAMAGE_NONBLOCK_MELEE:
  case DAMAGE_KNOCKING_DOWN:
    g_world_ptr->get_dread_net()->add_cue(dread_net::get_cue_type("MELEE_HIT"), dmg_info.attacker );
    break;
  }

  // if I am invulnerable or blocking a blockable attack, do no damage
  if ( is_invulnerable()
    || ( get_controller() && get_controller()->is_blocking() && (dmg_info.type!=DAMAGE_MELEE || (dmg_info.flags&_BRAIN_ANIM_UNBLOCKABLE)) )
    )
  {
    return;
  }

  // do pain flash effect for hero
  if ( is_hero() )
  {
    vector3d from = dmg_info.loc;
    if ( dmg_info.attacker )
      from = dmg_info.attacker->get_abs_position();
    g_game_ptr->get_interface_widget()->indicate_damage( damage, from );
  }

  #ifdef TARGET_PC
  char    outbuf[100];
  sprintf( outbuf, "%s takes %d damage", id.get_val().c_str(), damage );
  app::inst()->get_game()->get_message_board()->post( stringx(outbuf), 2.0F );
  #endif

  damaged = true;
  hit_applied = damage;

  // apply damage
  new_ap -= damage;
  if ( new_ap < 0 )
  {
    new_hp += new_ap;
    new_ap = 0;
  }
  if ( new_hp < 0 )
    new_hp = 0;

  set_hit_points( new_hp );
  set_armor_points( new_ap );

  // play sound, etc.
  if ( new_hp > 0 )
  {
    if(damage > 0)
    {
      // Don't over-do the sound effect if you're really getting mauled.
      rational_t elapse = pain_timer.elapsed();
      if (!is_hero() || (elapse > (dmg_info.attacker && dmg_info.attacker->get_brain() && dmg_info.attacker->get_brain()->is_boss_brain() ? 0.1f : 0.7f) || elapse < 0.001f))
      {
        pain_timer.reset();
        play_sound_group( "pain" );
      }
      if ( this!=g_world_ptr->get_hero_ptr() && dmg_info.push_wounded )
      {
        get_brain()->push_state_wounded( damage );
      }
    }
  }
  else
  {
    died = true;
    new_hp = 0;
    soft_attrib->set_alive( false );
    if ( is_active() && is_scripted_death() )
    {
      // disable brain for scripted death sequence
      get_brain()->set_active( false );
    }
    else
    {
      if ( this!=g_world_ptr->get_hero_ptr() && dmg_info.push_death )
      {
        get_brain()->push_state_dying( damage );
      }
      if ( dmg_info.type != DAMAGE_SILENT_KILL )
        play_sound_group( stringx("death") );
    }
  }

  if ( dmg_info.type == DAMAGE_GUN )
    shot_timer = GETTING_SHOT_TIME;

  if ( died )
  {
    target_timer = 0.0f;

    if ( this == g_world_ptr->get_hero_ptr()->get_current_target() )
    {
      g_world_ptr->get_hero_ptr()->set_current_target( NULL );
    }

    if ( is_active() )
    {
      if( is_hero() || is_scripted_death() )
      {
        set_death_fade( 1000.0f ); // LARGE value means he won't start blinking for a LONG time.  Scripts will change this later (maybe)
      }
      else
      {
        set_death_fade( 3.0f );
      }
    }
    else
      set_death_fade( 0.0f );

    // raise signal indicating that I have been killed
    raise_signal( KILLED );
    // also raise signal on all char_groups to which I belong
    vector<char_group*>::const_iterator cgi = char_groups.begin();
    vector<char_group*>::const_iterator cgi_end = char_groups.end();
    for ( ; cgi!=cgi_end; ++cgi )
    {
      char_group* cg = *cgi;
      cg->raise_signal( char_group::KILLED );
      if ( cg->num_alive() == 0 )
        cg->raise_signal( char_group::ALL_DEAD );
    }

    if(!is_active() && this != g_world_ptr->get_hero_ptr())
      kill_me();
  }
}


void character::set_death_fade( time_value_t t )
{
  dying_countdown = t;
}


void character::kill_me()
{
  set_control_active( false );

#if defined( TARGET_PC )
  char outbuf[100];
  sprintf( outbuf, "%s dies!", id.get_val().c_str() );
  app::inst()->get_game()->get_message_board()->post( stringx(outbuf), 2.0f );
#endif

  set_angular_velocity( ZEROVEC );

  // disgorge my inventory for the hero to enjoy
  disgorge_items();

  // prevent entity from being considered for rendering
  set_visible(false);

  // restore proper color to attachments (death fade affects their alpha)
  reset_variant_translucency();

  // this tells the scripters when a guy's death fade has finished (i.e., it's OK to respawn him)
  raise_signal( FADED_OUT );
}


// function called when this character is added to a char_group
void character::add_char_group( char_group* cg )
{
  // CTT 03/24/00: CAUTION:
  // we assume that each character is added to a given group only once,
  // and never removed!
  char_groups.push_back( cg );
}


void character::resurrect()
{
  soft_attrib->set_alive(true);
  set_hit_points(get_full_hit_points());
  set_collision_flags(COLLIDE_WITH_TERRAIN|COLLIDE_WITH_ACTORS);
  un_reset_locked_control_systems();
  set_control_active(true);
  set_visible(true);
  get_brain()->clear();

  // by default everyone has a radio and is connected....
  get_brain()->connect();
  get_brain()->enable_radio();

  get_brain()->set_variant_ai_defaults( get_variant_descriptor() );
  dying_countdown = 0;
}


bool character::is_climbing() const
{
  return ladder_mcs->is_active();
}


void character::heal(rational_t hit_points)
{
  rational_t d = get_soft_attrib()->get_hit_points();
  d += hit_points;
  if (d>get_full_hit_points())
  {
    d=get_full_hit_points();
  }
  set_hit_points( d );
}


character * character::get_nearest_opponent(bool include_friendlies)
{
  rational_t min_d2 = LARGE_DIST;
  character * best_chr = NULL;
  for (int i=0;i<g_world_ptr->get_num_characters();++i)
  {
    // Loop through the characters, crudely for now PTA
    character * chr = g_world_ptr->get_character(i);
    int chr_team = chr->get_soft_attrib()->get_team();
    if (chr!=this && (include_friendlies || (chr_team!=get_soft_attrib()->get_team() && chr_team!=character_soft_attributes::NEUTRAL_TEAM) ) &&
        chr->is_visible() && chr->is_active() && chr->is_alive()
        /*&& !chr->is_grabbed()*/)
    {
      //      vector3d pos = get_abs_po().inverse_xform(chr->get_abs_position());
      vector3d pos = get_abs_po().fast_inverse_xform(chr->get_abs_position());

      rational_t d2 = pos.length2();
      if (opponent_distances.size()==g_world_ptr->get_num_characters())
        opponent_distances[i] = __fsqrt(d2);
      if (d2<min_d2)
      {
        min_d2 = d2;
        best_chr = chr;
      }
    }
    else
      if (opponent_distances.size()==g_world_ptr->get_num_characters())
        opponent_distances[i] = 1e7f;
  }
  return best_chr;
}

#ifdef DEBUG
//int g_active_chars = 0;
#endif

character * character::get_nearest_visible_opponent(bool include_friendlies)
{
  rational_t closest_z = LARGE_DIST;
  character * best_chr = NULL;
  limb * src_vision_limb, * targ_vision_limb;

  my_enemies_list.clear();

  if (limb_valid(HEAD))
    src_vision_limb = limb_ptr(HEAD);
  else
    src_vision_limb = limb_ptr(WAIST);

  #ifdef DEBUG
  //g_active_chars = g_world_ptr->get_num_active_characters();
  #endif

  for (int i=0;i<g_world_ptr->get_num_active_characters();++i)
  {
    // Loop through the characters, crudely for now PTA
    character * chr = g_world_ptr->get_active_character(i);
    int chr_team = chr->get_soft_attrib()->get_team();
    if (chr!=this && (include_friendlies || (chr_team!=get_soft_attrib()->get_team() && chr_team!=character_soft_attributes::NEUTRAL_TEAM) ) &&
        chr->is_visible() && chr->is_active() && chr->is_alive() )
    {
      //      vector3d pos = get_abs_po().inverse_xform(chr->get_abs_position());
      vector3d pos = get_abs_po().fast_inverse_xform(chr->get_abs_position());

      rational_t d2 = pos.length();
      if (d2<ACTIVATION_RADIUS*ACTIVATION_RADIUS)
      {
        if (chr->limb_valid(HEAD))
          targ_vision_limb = chr->limb_ptr(HEAD);
        else
          targ_vision_limb = chr->limb_ptr(WAIST);
        vector3d dummy_hit, dummy_normal;

        if ( !find_intersection( src_vision_limb->get_abs_position(), targ_vision_limb->get_abs_position(),
                                 get_region(),
                                 FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                                 &dummy_hit, &dummy_normal ) )
        {
          my_enemies_list.push_back( chr );

          if (pos.z<closest_z && pos.z>0)
          {
            closest_z = pos.z;
            best_chr = chr;
          }
        }
      }
    }
  }
  return best_chr;
}

bool character::can_you_see_me( character *chr )
{
  if ( g_world_ptr->get_hero_ptr() == this )
    return find( my_enemies_list.begin(), my_enemies_list.end(), chr ) != my_enemies_list.end();
  return true;
}

ladder* character::get_nearest_ladder()
{
  rational_t closest_z = LARGE_DIST;
  ladder* best_lad = NULL;
  region_node_pset::const_iterator rni = in_regions.begin();
  region_node_pset::const_iterator rni_end = in_regions.end();
  for ( ; rni!=rni_end; ++rni )
  {
    region* r = (*rni)->get_data();
    region::ladder_list::const_iterator li = r->get_ladders().begin();
    region::ladder_list::const_iterator li_end = r->get_ladders().end();
    for ( ; li!=li_end; ++li )
    {
      ladder* lad = *li;
      if (lad)
      {
        //        vector3d pos = get_abs_po().inverse_xform( lad->get_abs_position() );
        vector3d pos = get_abs_po().fast_inverse_xform( lad->get_abs_position() );

        if ( __fabs(pos.z) < closest_z )
        {
          closest_z = __fabs(pos.z);
          best_lad = lad;
        }
      }
    }
  }
  return best_lad;
}


// returns true if the items did NOT already have a copy of the item
bool character::add_item( item* it )
{
  assert( (it->get_number() > 0 || (it->is_a_thrown_item() && ((thrown_item *)it)->is_a_radio_detonator())) );
  bool outval;
  item* my_copy = find_like_item( it );
  if ( my_copy )
  {
    if ( !my_copy->is_a_gun() )
    {
      int max = my_copy->get_max_allowed( this );
      int amt = my_copy->get_number() + it->get_number();
      if (amt > max)
      {
        int delta = max - my_copy->get_number();
        my_copy->set_count( my_copy->get_number() + delta );
        it->set_count( it->get_number() - delta );
      }
      else
      {
        my_copy->set_count( amt );
        it->set_count( 0 );
      }
    }
    else
    {
      // Force pick-up on guns
      it->set_count( 0 );
    }

    if ( !get_cur_item() || get_cur_item()->get_number() <= 0)
      set_cur_item( my_copy );

    outval = false;
  }
  else
  {
    entity::add_item( it );
    my_copy = it;
    if ( !my_copy->is_a_gun() )
    {
      int max = my_copy->get_max_allowed( this );
      int amt = my_copy->get_number();
      if ( amt > max )
        my_copy->set_count( max );
    }

//    my_copy->hide();

    if ( my_copy->is_a_handheld_item() && !((handheld_item*)my_copy)->is_brain_weapon() )
      my_copy->holster();

    if ( !get_cur_item() || get_cur_item()->get_number() <= 0)
    {
      set_cur_item( my_copy );

      if ( my_copy->is_a_handheld_item() && ((handheld_item*)my_copy)->is_brain_weapon() )
        my_copy->draw();
    }
    else if ( get_cur_item() != my_copy )
    {
      if(((handheld_item*)my_copy)->is_brain_weapon())
        my_copy->hide();
      else
      {
//        bool found = false;
        for(int i=0; i<get_num_items(); ++i)
        {
          item *itm = get_item(i);
          if(itm != my_copy && ((itm->is_a_thrown_item() && my_copy->is_a_thrown_item()) || (itm->is_a_gun() && my_copy->is_a_gun() && ((gun *)itm)->is_rifle() == ((gun *)my_copy)->is_rifle())) )
          {
            my_copy->hide();
//            found = true;
            break;
          }
        }

//        if(!found)
//          my_copy->show();
      }
    }

    outval = true;
  }
  return outval;
}


void character::use_item( item* it )
{
  assert( it && it->get_count() > 0 );

  entity::use_item(it);

  if ( !it->is_a_handheld_item() || (!((handheld_item *)it)->is_brain_weapon() && !((handheld_item *)it)->is_a_radio_detonator()) )
    it->dec_count();
}


void character::set_cur_item(item *itm)
{
//  item *old_itm = get_cur_item();
  int old_cur = cur_item;
  cur_item = get_item_index( itm );

  if(cur_item >= 0 && itm->get_number() > 0)
  {
    for(int i=0; i<get_num_items(); ++i)
    {
      item *my_itm = get_item(i);
      if(my_itm != itm && ((my_itm->is_a_thrown_item() && itm->is_a_thrown_item()) || (my_itm->is_a_gun() && itm->is_a_gun() && ((gun *)my_itm)->is_rifle() == ((gun *)itm)->is_rifle())) )
      {
        my_itm->hide();
        break;
      }
    }

/*
    if ( old_itm )
    {
      if((old_itm->is_a_thrown_item() && itm->is_a_thrown_item()) || (old_itm->is_a_gun() && itm->is_a_gun() && ((gun *)old_itm)->is_rifle() == ((gun *)itm)->is_rifle()) )
        old_itm->hide();
    }
*/
    if ( itm && itm->is_a_handheld_item())
      ((handheld_item *)itm)->set_visibility(true);
  }
  else
    cur_item = old_cur;
}


void character::use_current_item()
{
  bool used = false;

  if ( cur_item < get_num_items() )
  {
    item* it = get_item( cur_item );
    if ( it && (it->is_usable() || (it->is_a_handheld_item() && ((handheld_item *)it)->is_brain_weapon())) )
    {
      item::usage_t typ = it->get_usage_type();
      if ( typ == item::INVENTORY ||
           typ == item::UTILITY ||
           ((typ==item::GUN || typ==item::THROWN) && (get_weapon_type()==WEAPON_TYPE_GUN || ((handheld_item *)it)->is_brain_weapon())) )
      {
        use_item( it );
        used = true;

        it = get_item( cur_item );
        if ( it == NULL || it->get_number() == 0 )
        {
          if ( this == g_world_ptr->get_hero_ptr() )
          {
            set_weapon_type( WEAPON_TYPE_NONE );

            if(it != NULL)
            {
              it->holster();
              it->hide();
              if ( it->is_a_gun() )
              {
                // should never happen for guns anyway, but...
                disable_auto_aim_arm_mcs();
                assert(0);
              }
            }

            get_player_controller()->next_inv_item();
          }
          else
          {
            if(it != NULL)
            {
              it->holster();
              if ( it->is_a_gun() )
                disable_auto_aim_arm_mcs();
            }

            int base_cur_item = cur_item;
            do
            {
              cur_item++;
              if ( cur_item >= get_num_items() )
                cur_item = 0;
              it = get_item( cur_item );
            } while ( it->get_number()==0 && cur_item!=base_cur_item );
            if ( get_weapon_type()==WEAPON_TYPE_GUN && it->get_number()>0 )
              it->draw();
          }
        }
      }
    }
  }
}

item *character::get_cur_item() const
{
  return get_item( cur_item );
}

int character::get_cur_item_count() const
{
  item* it = get_item( cur_item );
  if ( it )
    return it->get_count();
  else
    return 0;
}

bool character::remove_permanent_item(item *it)
{
  item_list_t::iterator i;

  for(i = permanent_items.begin(); i != permanent_items.end(); i++)
  {
    if(*i == it)
    {
      permanent_items.erase(i);

      if(this == g_world_ptr->get_hero_ptr())
        get_player_controller()->remove_perm_item();

      return true;
    }
  }

  return false;
}

item *character::get_permanent_item_by_name(stringx label)
{
  item_list_t::iterator i;
  item* it;

  for (i = permanent_items.begin(); i != permanent_items.end(); ++i)
  {
    it = *i;
    if(entity_id(label.c_str()) == it->get_id())
      return it;
  }

  return NULL;
}


void character::add_permanent_item(item *it)
{
  permanent_items.push_back(it);
  it->set_picked_up(true);

  if(this == g_world_ptr->get_hero_ptr())
    get_player_controller()->add_perm_item();
}


// An experiemnt.  Make it more real if its a keeper.
void character::mark_target_for_death(rational_t suck_rad)
{
}


// Return true if this character wants to be active at the moment.
bool character::wants_to_be_active()
{
  bool want = false;
  if ( is_alive_or_dying() && is_visible() )
  {
    if ( get_forced_active() == FORCE_ACTIVE_TRUE  // I have been forced into a permanently active state
      || this == g_world_ptr->get_hero_ptr()       // I am the hero
      || is_flagged(EFLAG_MISC_IN_VISIBLE_REGION)  // My region is visible
      )
    {
      want = true;
    }
    else if ( get_region() && get_forced_active()!=FORCE_ACTIVE_FALSE )
    {
      if ( get_region()->get_data()->is_active() || !get_region()->get_data()->is_locked() )
      {
        rational_t d2 = ( app::inst()->get_game()->get_current_view_camera()->get_abs_position() - get_abs_position() ).length2();
        if ( d2 < ACTIVATION_RADIUS*ACTIVATION_RADIUS )
          want = true;
      }
    }
  }
  return want;
}


void character::set_active( bool a )
{
  if ( !a || !is_active() )
  {
    if ( set_active_first_pass > 0 )
      --set_active_first_pass;
    clear_falling_elevation();
    actor::set_active( a );
    a = actor::is_active();  // actor::set_active(true) can fail!

    // CTT 06/29/00: We used to call set_control_active() here, but henceforth
    // brains are allowed to be active even when the character is not

    // we might or might not hav a limb tree at this point;
    // set the current (handheld) item to an appropriate state
    for(int i=0; i<get_num_items(); ++i)
    {
      item *itm = get_item(i);
      if(itm && itm->is_a_handheld_item() )
      {
        handheld_item* h = static_cast<handheld_item*>( itm );

        if ( a )
        {
          if ( h->is_drawn() )
            h->draw();
          else
            h->holster(false);
        }
        else
        {
          h->detach();
        }

        h->set_visibility(a);
      }
    }
/*
    item* it = get_cur_item();
    if ( it && it->is_a_handheld_item() )
    {
      handheld_item* h = static_cast<handheld_item*>( it );
      if ( a )
      {
        if ( h->is_drawn() )
          h->draw();
        else
          h->holster();
      }
      else
      {
        h->detach();
        h->hide();
      }
    }
*/
    // set suspended state of all threads that are localized to this character
    list<vm_thread *>::const_iterator li,le;
    li = local_thread_list.begin();
    le = local_thread_list.end();
    for ( ; li!=le; ++li)
    {
      (*li)->set_suspended( !a );
    }
  }
}



void character::set_god_mode(bool gm)
{
  if (god_mode != gm)
  {
    god_mode = gm;

    if (god_mode)
      app::inst()->get_game()->get_message_board()->post("God Mode on",2.0f);
    else
      app::inst()->get_game()->get_message_board()->post("God Mode off",2.0f);
  }
}

void character::toggle_god_mode()
{
  set_god_mode(!god_mode);
}


//extern character * who;
#if 0
void character::set_rel_position( const vector3d& p )
{

  actor::set_rel_position(p); // actually actor doesn't override it, so call entity::set_rel_position
  ever_been_planted = false;

  /*
  if ( this != g_world_ptr->get_hero_ptr() )
  {
    if ( get_brain() )
      get_brain()->set_wait_target( p );
  }
  */
  assert(get_abs_position().x>-100000 && get_abs_position().x<100000);
}
#else
void character::po_changed()
{
  ever_been_planted = false;
  entity::po_changed();
}
#endif

void character::set_velocity_with_impact( const vector3d& v )
{
  physical_entity::set_velocity(v);
}


void character::manage_standing()
{
  // adjust vertical position according to slope of floor (last frame)
  // and non-physical horizontal motion of character (this frame)
  if ( is_frame_delta_valid() && get_cur_ground_normal().y>0.70f )
  {
    vector3d delta = get_frame_delta().get_abs_position();
    delta.y = 0;
    rational_t vadj = -dot( get_cur_ground_normal(), delta );
    if ( vadj < 0 )
    {
      vector3d p = get_abs_position();
      p.y += vadj;
      set_rel_position( p );
    }
  }
  // the theory is that a character in non-physical mode doesn't need to compute elevation
  if ( !is_physical() )
  {
    return;
  }
  compute_elevation();
  const vector3d& cp = get_abs_position();
  rational_t floor_delta = cp.y - get_floor_offset() - get_cur_elevation();
  if ( floor_delta < 0 )
  {
    // force the character onto the floor
    vector3d fp = cp;
    fp.y -= floor_delta;
    set_rel_position( fp );
  }
  vector3d vel;
  get_velocity( &vel );
  if ( vel.y<=0.0f && floor_delta<0.05f )
  {
    effectively_standing = true;
    air_timer = .0f;
    // clear horizontal velocity
    vel.x = vel.z = 0;
    // if actually on floor, clear vertical velocity as well
    if ( floor_delta < 0.001f )
      vel.y = 0;
    set_velocity( vel );
  }
  else
    effectively_standing = false;
}


void character::compute_elevation()
{
  last_elevation = cur_elevation;
  last_floor_entity = next_floor_entity;

  if ( is_moving_around() )
  {
    if ( get_hard_attrib()->get_use_surface_types() )
    {
      cur_elevation = g_world_ptr->get_the_terrain().get_elevation( get_abs_position(),
                                                                    cur_normal,
                                                                    get_region(),
                                                                    &current_surface_type );
    }
    else
    {
      cur_elevation = g_world_ptr->get_the_terrain().get_elevation( get_abs_position(),
                                                                    cur_normal,
                                                                    get_region() );
    }
    next_floor_entity = terrain::get_last_elevation_entity();
  }
}


// return desired distance from character's root to floor
rational_t character::get_floor_offset()
{
  // for now at least, this is entirely determined by the root animation
  entity_anim_tree* a = get_anim_tree( ANIM_PRIMARY );
  if ( a!=NULL && a->is_relative_to_start() )
  {
    vector3d relp;
    a->get_current_root_relpos( &relp );
    last_floor_offset = a->get_floor_offset() + relp.y;
  }
  return last_floor_offset;
}


// Behavior Scripts


rational_t character::get_attack_range()
{
  // placeholder <<<<
  return get_radius()*(2.0f/3) + 1.0f;
}

rational_t AUTO_AIM_MAX_RANGE_SCALE = 2.0f;
rational_t AUTO_AIM_MAX_ANGLE = 1.2;
character * character::get_attack_target(rational_t & range, rational_t & angle, character * forced_target)
{
  return NULL;
}


void character::adjust_position_for_moving_floor()
{
  if ( last_floor_entity && (is_effectively_standing() || was_planted) )
  {
    // adjust the character's position according to total motion of floor,
    // just as if he was parented to it
    po new_po = get_abs_po();
    entity* e = last_floor_entity;
    while ( e )
    {
      entity* p = e->get_parent();
      if ( e->is_frame_delta_valid() )
      {
        new_po = new_po * e->get_abs_po().inverse() * e->get_frame_delta() * e->get_abs_po();
      }
      e = p;
    }
    set_rel_po( new_po );
  }
}


rational_t character::get_body_length()
{
  vector3d top;

  top = limb_ptr(CHEST)->get_abs_position();

  vector3d bottom = limb_ptr(WAIST)->get_abs_position();
  return (top-bottom).length();
}

// TOOL Functions


// NOTE:  THIS May not work properly if there are multiples of a character present.
void character::reload_attributes()
{
  load_attributes(attribute_filename.c_str());
}


// This is basically a teleport function.
void character::adjust_position(vector3d p)
{
  if ( p != get_abs_position() )
  {
    center += p - get_abs_position();
    set_rel_position(p);
    compute_sector( g_world_ptr->get_the_terrain() );
  }

  // Lift him up off of the ground
  was_planted = false;
//  set_velocity(ZEROVEC);
  set_last_po(get_colgeom_root_po());
  frame_done();
  clear_falling_elevation();

  //  if (this!=g_world_ptr->get_hero_ptr())
  //    get_brain()->reset_rest_location();

  moved_last_frame = true;
}

//--------------------------------------------------------------
void character::pickup_ent(const entity *ent)
{
  if( carried_ent )
  {
    drop_ent( get_abs_position() );
    // NB: breaks constness here, I'm a rude SOB
    carried_ent = (entity *)ent;
    // set parenting here
    carried_ent->set_parent( this );
  }
}
void character::drop_ent( const vector3d& v )
{
  if( carried_ent )
  {
    // clear parenting here
    carried_ent->clear_parent();
    carried_ent = 0;
    // set carried ent's position
    carried_ent->set_rel_position( v );
  }
}


//--------------------------------------------------------------
/*
  General Priority Behavior
  A) Pri  0 always plays and doesn't bother anyone else,
  it is the default used when no extra options are requested
  B) Pri 1+ with no herd named will not play if any higher priority sounds are playing anywhere
  else will shut off all lower priority sounds then play itself
  C) Pri 1+ and Herd will act as B but only within the herd
*/
//--------------------------------------------------------------
void character::play_sound_group( const stringx& sref, int idx, int pri, const stringx *who, rational_t pitch_variance )
{
  // find the named sound group
  sound_group_list_t::const_iterator sgi = sound_groups.begin();
  sound_group_list_t::const_iterator sgi_end = sound_groups.end();
  for ( ; sgi!=sgi_end; ++sgi )
  {
    if ( (*sgi).first == sref )
      break;
  }
  if ( sgi == sgi_end )
    return;
  // determine which sound to play from the group
  const sound_group* sg = (*sgi).second;
  int nsounds = sg->indices.size();
  if ( idx<0 || idx>=nsounds )
  {
    // no valid explicit sound index was provided;
    // pick randomly from the list
    rational_t r = ((rational_t)(random(100)))/100.0;
    rational_t p = 0.0;
    int i;
    for ( i=0; i<nsounds; ++i )
    {
      p += sg->probabilities[ i ];
      if ( r <= p )
      {
        idx = i;
        break;
      }
    }
  }
  // play the selected sound
  if ( idx>=0 && idx<nsounds )
  {
    if ( who )
      get_emitter()->play_sound( sg->sound_ids[idx], pri, who, pitch_variance );
    else
      get_emitter()->play_sound( sg->sound_ids[idx], pri, pitch_variance );
  }
}

//--------------------------------------------------------------
bool character::sound_group_exists( const stringx& sref )
{
  sound_group_list_t::iterator sgrp;
  for( sgrp = sound_groups.begin(); sgrp!=sound_groups.end(); ++sgrp )
  {
    if( (*sgrp).first == sref )
    {
      return true;
    }
  }
  return false;
}

//--------------------------------------------------------------
int character::get_num_sound_group_sounds( const stringx& sref )
{
  sound_group_list_t::iterator sgrp;
  for( sgrp = sound_groups.begin(); sgrp!=sound_groups.end(); ++sgrp )
  {
    if( (*sgrp).first == sref )
    {
      return( (*sgrp).second->indices.size() );
    }
  }
  return 0;
}


rational_t character::get_full_hit_points()
{
  return soft_attrib->get_full_hit_points();
}

void character::set_full_hit_points(int hp)
{
  soft_attrib->set_full_hit_points(hp);
}

rational_t character::get_hit_points()
{
  return soft_attrib->get_hit_points();
}


rational_t character::get_paralysis_factor()
{
  return get_soft_attrib()->get_paralysis_factor();
}


int character::get_team()
{
  return get_soft_attrib()->get_team();
}


bool character::is_alive() const
{
  return soft_attrib->is_alive();
}

bool character::is_dying() const
{
  return ( !is_alive() && dying_countdown>0 );
}

bool character::is_alive_or_dying() const
{
  return ( is_alive() || dying_countdown>0 );
}


const rational_t ARM_FLAIL_PCT = 3;
const rational_t HEAD_FLAIL_PCT = 3;
const rational_t TAIL_FLAIL_PCT = 3;

#define RAND_PCT (((float) random(1000))/1000)

void character::flail(time_value_t time_inc)
{
  if (my_head)
  {
    my_head->set_reset_locked(true);
    if (RAND_PCT < HEAD_FLAIL_PCT*time_inc)
    {
      my_head->controller_set_neck_target_theta(RAND_PCT*1.5-.75);
      my_head->controller_set_neck_target_psi(RAND_PCT*1.5-.5);
      my_head->controller_set_head_target_psi(RAND_PCT*1.5-.5);
      my_head->controller_set_head_target_phi(RAND_PCT*1.5-.75);
      my_head->controller_set_jaw_target_psi(RAND_PCT*.75);
    }
  }
}


void character::un_reset_locked_control_systems()
{
  if (my_head)
    my_head->set_reset_locked(false);
}


void character::be_dead()
{
  if (my_head)
  {
    my_head->controller_set_neck_target_theta(0);
    my_head->controller_set_neck_target_extend(1);
    my_head->controller_set_neck_target_psi(1);
    my_head->controller_set_head_target_psi(1);
    my_head->controller_set_head_target_phi(1);
    my_head->controller_set_jaw_target_psi(0);
  }
}


const po& character::get_colgeom_root_po()
{
  return get_abs_po();
}


const entity* character::get_colgeom_root()
{
  return this;
}


void character::update_c_o_m()
{
  if ( ever_been_planted && !was_planted && !get_paralysis_factor() )
  {
    vector3d cur_c_o_m_offset = get_center()-get_abs_position();
    vector3d c_o_m_offset_diff = -cur_c_o_m_offset + c_o_m_offset;
    add_position_increment( c_o_m_offset_diff );
    c_o_m_offset = cur_c_o_m_offset;
  }
}

//--------------------------------------------------------------
void character::I_win()
{
  extern bool g_iwin;
  g_iwin = true;
}
void character::I_lose()
{
  extern bool g_ilose;
  g_ilose = true;
}

/////////////////////////////////////////////////////////////////////////////
// Animation interface
/////////////////////////////////////////////////////////////////////////////
// attach entity to given animation (kills any prior attached animation)
bool character::attach_anim( entity_anim* new_anim )
{
  if ( actor::attach_anim( new_anim ) )
    return true;
  return false;
}

// detach entity from current animation
void character::detach_anim()
{
  if ( get_anim() && has_limb_tree() )
  {
    set_angular_velocity( ZEROVEC );

    // make sure the root po has no tilt
    const po& curpo = get_rel_po();
    if ( maintain_vertical && curpo.get_y_facing() != YVEC )
    {
      vector3d zb = curpo.get_z_facing();
      if ( __fabs(zb.y) > 0.95f )
        zb = curpo.get_y_facing();
      zb.y = 0;
      zb.normalize();
      vector3d xb = cross( YVEC, zb );
      po newpo( xb, YVEC, zb, curpo.get_abs_position() );
      set_rel_po( newpo );
    }

    // restore limb hinge values
    assert(get_lptr());
    get_lptr()->restore_hinges_from_po();
  }

  actor::detach_anim();
}


void character::set_velocity( const vector3d& v )
{
  set_velocity_with_impact(v);
}


void character::set_velocity_without_impact( const vector3d& v )
{
  physical_entity::set_velocity(v);
}


rational_t character::get_friction_scale() const
{
  //if ( is_alive() )
  //  return 1.0f;
  return 1.0f;
}


rational_t character::get_cheap_elevation(vector3d point)
{
  vector3d diff = point-get_abs_position();

  rational_t outval = cur_elevation;
  // <<<<
  return outval;
}


player_controller * character::get_player_controller() const
{
  return my_controller->as_player_controller();
}


void character::set_forced_look_at_target( const vector3d& v, entity * _relto )
{
  look_at_target = v;
  look_at_relto = _relto;
  look_at_target_forced =  true;
}


bool character::get_forced_look_at_target( vector3d& targ )
{
  if (look_at_target_forced && look_at_relto)
  {
    //    targ = look_at_relto->get_abs_po().slow_xform(look_at_target);
    targ = look_at_relto->get_abs_po().fast_8byte_xform(look_at_target);
  }
  else
    targ = look_at_target;
  return look_at_target_forced;
}


void character::free_look_at()
{
  look_at_target_forced = false;
}

void character::apply_force_increment( const vector3d& f,
                                       force_type ft,
                                       const vector3d& loc, int mods )
{
  physical_entity::apply_force_increment(f,ft,loc,mods);
  vector3d internal_shock_addition=ZEROVEC;
  vector3d external_shock_addition=ZEROVEC;
  vector3d a = f/get_mass();
  vector3d shock_addition = a;
  if ((mods&SHOCK_REVERSE))
    shock_addition *= -1;
  if (mods&SHOCK_INTERNAL)
  {
    internal_shock_addition = shock_addition;
  }
  else
  {
    external_shock_addition = shock_addition;
  }
  if (mods&SHOCK_FREE)
  {
    external_shock_addition -= shock_addition*0.5f;
    internal_shock_addition += shock_addition*0.5f;
  }
  if (mods&SHOCK_MILD_INTERNAL)
  {
    internal_shock_addition += shock_addition*(1/3.0f);
  }
  if ((mods&SHOCK_ALWAYS_FORWARD))
  {
    //    vector3d zvec = get_abs_po().non_affine_slow_xform(ZVEC);
    vector3d zvec = get_abs_po().fast_8byte_non_affine_xform(ZVEC);

    rational_t backwards = dot(internal_shock_addition,-zvec);
    if (backwards<0)
    {
      internal_shock_addition += 2*zvec*backwards;
    }
  }
  internal_shock_vector += internal_shock_addition;
  shock_vector += external_shock_addition;
}



rational_t character::get_inter_capsule_radius_scale()
{
  return hard_attrib->get_collision_capsule_inter_capsule_radius_scale();
}

bool character::is_control_po_locked()
{
  return control_po_locked;
}

void character::add_local_thread(vm_thread * thr)
{
  character * chr;
  if ((chr = thr->get_local_character())!=NULL)
  {
    chr->remove_local_thread(thr);
  }
  local_thread_list.push_back(thr);
  thr->set_local_character(this);
  thr->set_suspended(!is_active());
}


void character::remove_local_thread(vm_thread * thr)
{
  local_thread_list.remove(thr);
  thr->set_suspended(false);
}


void character::set_visible( bool a )
{
  actor::set_visible( a );

  for(int i=0; i<get_num_items(); ++i)
  {
    item *itm = get_item(i);
    if(itm->is_a_handheld_item() )
      ((handheld_item*)itm)->set_visibility(is_visible());
  }
/*
  item* it = get_cur_item();
  if ( it )
  {
    if ( is_visible() )
      it->show();
    else
      it->hide();
  }
*/
}


int character::get_max_ammo_points() const
{
  assert(soft_attrib);
  return(soft_attrib->get_max_ammo_points());
}

void character::set_max_ammo_points(int max)
{
  assert(soft_attrib);

  if(max < 0)
    max = 0;

  soft_attrib->set_max_ammo_points(max);

  if(ammo_points > get_max_ammo_points())
    ammo_points = get_max_ammo_points();
  else if(ammo_points < 0)
    ammo_points = 0;
}


void character::activate_current_entity()
{
  if(curr_activate_entity != NULL)
  {
    curr_activate_entity->activate_by_character(this);
    curr_activate_entity = NULL;
  }
}

void character::set_current_activate_entity(entity *ent)
{
  curr_activate_entity = ent;
}

entity *character::get_current_activate_entity() const
{
  return curr_activate_entity;
}


void character::play_footstep(anim_id_t foot)
{
  assert(get_current_surface_type() < 8 && get_current_surface_type() >= 0);
//  assert(limb_ptr(foot) && limb_ptr(foot)->get_body());

  limb_body *lb = limb_ptr(foot) ? limb_ptr(foot)->get_body() : NULL;

/*
  if(this->is_hero() && g_world_ptr->get_dread_net())
  {
    switch(this->get_controller()->get_state())
    {
      case character_controller::RUNNING:
        g_world_ptr->get_dread_net()->add_cue(dread_net::get_cue_type("RUNNING"), this);
        break;

      default:
        g_world_ptr->get_dread_net()->add_cue(dread_net::get_cue_type("WALKING"), this);
        break;
    }
  }
*/

  if ( this==g_world_ptr->get_hero_ptr() && !is_running() )
  {
    play_sound_group("footstep_walk", footstep_sound_idx, 0, NULL, 0.5f );
  }
  else
  {
    play_sound_group( "footstep", footstep_sound_idx, 0, NULL, this->is_hero() ? 0.5f : 0.0f );
  }

  // advance and wrap footstep sound sequence counter
  if ( num_footstep_sounds > 0 )
  {
    ++footstep_sound_idx;
    if ( footstep_sound_idx >= num_footstep_sounds )
      footstep_sound_idx = 0;
  }

  if ( lb && get_hard_attrib()->get_use_surface_types() &&
       !g_world_ptr->get_surface_effect_name(get_current_surface_type()).empty() )
  {
    po loc = po_identity_matrix;
    vector3d pos = lb->get_abs_position() + (get_cur_ground_normal() * 0.05f);
    loc.set_rel_position(pos);

    entity *ent = g_world_ptr->add_time_limited_effect( g_world_ptr->get_surface_effect_name(get_current_surface_type()).c_str(), loc, g_world_ptr->get_surface_effect_duration(get_current_surface_type()) );

    if(get_cur_ground_normal() != ent->get_abs_po().get_facing())
    {
      po tmp = po_identity_matrix;

      // set the orientation
      vector3d facing=ent->get_abs_po().get_facing();
      vector3d grnorm=get_cur_ground_normal();
      rational_t dot_prod = dot(grnorm, facing);
      vector3d cross_prod = cross(grnorm, facing);

      tmp.set_rot(cross_prod, fast_acos(dot_prod));

      loc.set_rel_orientation(tmp.get_abs_orientation());
      loc.set_rel_position(pos);

      ent->set_rel_po(loc);
    }
  }
}



bool character::is_moving_around()
{
  return ( ( is_physical() && (velocity.x != 0.0f || velocity.y != 0.0f || velocity.z != 0.0f) )
        || is_frame_delta_valid()
        || is_last_frame_delta_valid()
        || ( get_controller()->is_a_brain() && get_brain()->get_reaction_state()>BRAIN_REACT_IDLE )
        );
}

bool character::is_running() const
{
  return my_controller && my_controller->is_in_running_mode();
}

void character::enable_auto_aim_arm_mcs( int aim_type )
{
  auto_aim_mcs->set_aim_type( aim_type );
  auto_aim_mcs->set_active(true);
}

void character::disable_auto_aim_arm_mcs()
{
  auto_aim_mcs->set_active(false);
}


int character::get_full_armor_points() const
{
  return hard_attrib->get_full_armor_points();
}

int character::get_armor_points() const
{
  return soft_attrib->get_armor_points();
}


void character::set_armor_points(int ap)
{
  if ( g_demigod_cheat && ap<get_armor_points() && this==g_world_ptr->get_hero_ptr() )
    return;

  if(ap > get_full_armor_points())
    ap = get_full_armor_points();

  if(ap < 0)
    ap = 0;

  soft_attrib->set_armor_points(ap);
}

void character::inc_armor_points(int ap)
{
  set_armor_points(get_armor_points() + ap);
}

void character::dec_armor_points(int ap)
{
  if ( g_demigod_cheat && this==g_world_ptr->get_hero_ptr() )
    return;
  set_armor_points(get_armor_points() - ap);
}


int character::get_full_nanotech_energy() const
{
  return hard_attrib->get_full_nanotech_energy();
}

int character::get_nanotech_energy() const
{
  return soft_attrib->get_nanotech_energy();
}

void character::set_nanotech_energy( int ap )
{
  if ( g_demigod_cheat && ap<get_nanotech_energy() && this==g_world_ptr->get_hero_ptr() )
    return;
  if ( ap > get_full_nanotech_energy() )
    ap = get_full_nanotech_energy();
  if ( ap < 0 )
    ap = 0;
  soft_attrib->set_nanotech_energy( ap );
}

void character::inc_nanotech_energy( int ap )
{
  set_nanotech_energy( get_nanotech_energy() + ap );
}

void character::dec_nanotech_energy( int ap )
{
  if ( g_demigod_cheat && this==g_world_ptr->get_hero_ptr() )
    return;
  set_nanotech_energy( get_nanotech_energy() - ap );
}


void character::set_hit_points(rational_t hp)
{
  if ( g_demigod_cheat && hp<get_hit_points() && this==g_world_ptr->get_hero_ptr() )
    return;
  if(hp < 0)
    hp = 0;
  if(hp > get_full_hit_points())
    hp = get_full_hit_points();

  soft_attrib->set_hit_points(hp);
}

void character::inc_hit_points(rational_t hp)
{
  set_hit_points(get_hit_points() + hp);
}

void character::dec_hit_points(rational_t hp)
{
  if ( g_demigod_cheat && this==g_world_ptr->get_hero_ptr() )
    return;
  set_hit_points(get_hit_points() - hp);
}


void character::set_ammo_points( int ap )
{
  if ( g_demigod_cheat && ap<get_ammo_points() && this==g_world_ptr->get_hero_ptr() )
    return;
  ammo_points = ap;
  if ( ammo_points > get_max_ammo_points() )
    ammo_points = get_max_ammo_points();
  else if ( ammo_points < 0 )
    ammo_points = 0;
}

void character::inc_ammo_points( int ap )
{
  ammo_points += ap;
  if ( ammo_points > get_max_ammo_points() )
    ammo_points = get_max_ammo_points();
}

void character::dec_ammo_points( int ap )
{
  if ( g_demigod_cheat && this==g_world_ptr->get_hero_ptr() )
    return;
  ammo_points -= ap;
  if ( ammo_points < 0 )
    ammo_points = 0;
}


bool character::allow_targeting() const
{
  return (soft_attrib->get_hit_points() > 0 && is_combat_target());
}


bool character::test_combat_target( const vector3d& p0, const vector3d& p1,
                                    vector3d* impact_pos, vector3d* impact_normal,
                                    rational_t default_radius ) const
{
  if(limb_capsules.size() > 0)
  {
    bool collided = false;
    rational_t min_dist2 = 0.0f;

    vector<limb_capsule>::const_iterator i = limb_capsules.begin();
    vector<limb_capsule>::const_iterator i_end = limb_capsules.end();

    while(i != i_end)
    {
      assert(limb_valid((*i).limb_id[0]) && limb_valid((*i).limb_id[1]));

      if ( collide_segment_capsule_accurate_result( p0, p1,
                                    get_limb_map()[(*i).limb_id[0]]->get_abs_position(),
                                    get_limb_map()[(*i).limb_id[1]]->get_abs_position(),
                                    (*i).radius, *impact_pos ) )
      {
        vector3d norm = (p0 - *impact_pos);
        rational_t dist2 = norm.length2();

        if(!collided || dist2 < min_dist2)
        {
          *impact_normal = norm;
          impact_normal->normalize();

          min_dist2 = dist2;
        }

        collided = true;
      }

      ++i;
    }

    return collided;
  }
  else
    return entity::test_combat_target(p0, p1, impact_pos, impact_normal, default_radius);
}






// code callback for signal 'character::ACTIVATE_ENTITY'
static void character_activate_entity_callback(signaller* sig, const char *data)
{
  assert(sig && ((entity *)sig)->get_flavor() == ENTITY_CHARACTER);

  ((character *)sig)->activate_current_entity();
}

static void character_anim_action_callback(signaller* sig, const char *data)
{
  assert(sig && ((entity *)sig)->get_flavor() == ENTITY_CHARACTER);
  character* c = static_cast<character*>( sig );
  c->set_anim_action( true );
}

static void character_footstep_callback(signaller* sig, const char *data)
{
  assert(sig && ((entity *)sig)->get_flavor() == ENTITY_CHARACTER);

  ((character *)sig)->play_footstep((anim_id_t)data);
}

static void character_damaged_callback(signaller* sig, const char *data)
{
  assert(sig && ((entity *)sig)->get_flavor() == ENTITY_CHARACTER && sig != g_world_ptr->get_hero_ptr());

  g_world_ptr->get_dread_net()->was_damaged((character *)sig);
}

static void character_killed_callback(signaller* sig, const char *data)
{
  assert(sig && ((entity *)sig)->get_flavor() == ENTITY_CHARACTER && sig != g_world_ptr->get_hero_ptr());

  g_world_ptr->get_dread_net()->was_killed((character *)sig);
}


void character::add_signal_callbacks()
{
  actor::add_signal_callbacks();

  signal* sptr = NULL;

  sptr = signal_ptr(ACTIVATE_ENTITY);
  assert(sptr);
  if( sptr )
    sptr->add_callback(character_activate_entity_callback, NULL);

  sptr = signal_ptr(FOOTSTEP_L);
  assert(sptr);
  if( sptr )
    sptr->add_callback(character_footstep_callback, (char *)LEFT_FOOT);

  sptr = signal_ptr(FOOTSTEP_R);
  assert(sptr);
  if( sptr )
    sptr->add_callback(character_footstep_callback, (char *)RIGHT_FOOT);

  sptr = signal_ptr( ATTACK );
  assert(sptr);
  if( sptr )
    sptr->add_callback( character::signal_callback_attack, (char*)this );

  sptr = signal_ptr( ANIM_ACTION );
  assert(sptr);
  if( sptr )
    sptr->add_callback( character_anim_action_callback, NULL );

  if(get_name() != stringx("HERO"))
  {
    sptr = signal_ptr( DAMAGED );
    assert(sptr);
    if( sptr )
      sptr->add_callback( character_damaged_callback, NULL );

    sptr = signal_ptr( KILLED );
    assert(sptr);
    if( sptr )
      sptr->add_callback( character_killed_callback, NULL );
  }
}


character *character::compute_melee_opponent( bool include_friendlies )
{
  const rational_t MELEE_MIN_DOT_PROD = .573576f, MELEE_MAX_DELTA_Y = 1.5f;

  character   *best_chr = NULL;
  rational_t  best_chr_angle;
  bool        best_chr_is_outside_angle = true;
  int         best_chr_hitpoints = 50000;
  vector3d    my_facing, my_pos;
  rational_t  max_val = .0;

  // The hero only cares about melee opponents
  if( g_world_ptr->get_hero_ptr() != this )
  {
    melee_opponent = NULL;
    return NULL;
  }

  entity *entity_target_pt = 0;
  rational_t entity_target_angle;

  find_entity_target( this, entity_target_pt, entity_target_angle );

  my_enemies_list.clear();

  my_facing = get_abs_po().get_facing();
  my_pos = get_abs_position();

  // used for LOS check
  vector3d p0 = my_pos;
  if ( has_limb_tree() && limb_valid(HEAD) )
    p0 = limb_ptr(HEAD)->get_body()->get_abs_position();
  else
    p0.y += .5f;

  vector<character*>::const_iterator ci = g_world_ptr->get_active_characters().begin();
  vector<character*>::const_iterator ci_end = g_world_ptr->get_active_characters().end();
  for ( ; ci!=ci_end; ++ci )
  {
    character* chr = *ci;
    int chr_team = chr->get_soft_attrib()->get_team();
    if ( chr != this
      && chr->allow_targeting()
      && chr->get_hard_attrib()->get_melee_lockon()
      && chr->is_visible()
      && chr->is_active()
      && chr->is_alive()
      && chr->get_brain() != NULL
      && ( include_friendlies
        || ( chr_team != get_soft_attrib()->get_team()
          && chr_team != character_soft_attributes::NEUTRAL_TEAM
          )
        )
      )
    {
      vector3d p1 = chr->get_abs_position();
      vector3d delta_pos = p1 - my_pos;
      if ( is_in_melee_range( chr ) && delta_pos.y <= MELEE_MAX_DELTA_Y )
      {
        // line of sight check to avoid hitting characters through walls
        if ( chr->is_a_character() )
        {
          if ( chr->has_limb_tree() && chr->limb_valid(HEAD) )
            p1 = chr->limb_ptr(HEAD)->get_body()->get_abs_position();
          else
            p1.y += .5f;
        }
        vector3d impact_point;
        vector3d impact_normal;
        if ( !find_intersection( p0, p1,
                                 get_primary_region(),
                                 FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                                 &impact_point, &impact_normal ) )
        {
          // first check within angle
          vector3d delta_pos_normalized = delta_pos;
          delta_pos_normalized.normalize();
          rational_t dot_prod = dot( delta_pos_normalized, my_facing );
          if ( dot_prod > MELEE_MIN_DOT_PROD )
          {
            // force to boss here
            if ( chr->get_hard_attrib()->get_is_boss_character() )
            {
              best_chr_is_outside_angle = false;
              best_chr = chr;
              best_chr_angle = dot_prod;
              best_chr_hitpoints = 0;
            }
            else if ( best_chr_is_outside_angle )
            {
              best_chr = chr;
              best_chr_angle = dot_prod;
              best_chr_is_outside_angle = false;
              best_chr_hitpoints = chr->get_hit_points();
            }
            else
            {
              if ( chr->get_hit_points() < best_chr_hitpoints )
              {
                best_chr = chr;
                best_chr_angle = dot_prod;
                best_chr_hitpoints = chr->get_hit_points();
              }
            }
          }
          // if no target in front, try behind
          else if ( best_chr_is_outside_angle )
          {
            if ( chr->get_hard_attrib()->get_is_boss_character() )
            {
              best_chr = chr;
              best_chr_angle = dot_prod;
              best_chr_hitpoints = 0;
            }
            else if ( chr->get_hit_points() < best_chr_hitpoints )
            {
              best_chr = chr;
              best_chr_angle = dot_prod;
              best_chr_hitpoints = chr->get_hit_points();
            }
          }
        }
      }
    }
  }

  if( entity_target_pt )
  {
    if( entity_target_angle > best_chr_angle )    // if a breakable entity is in range, hit that first
    return NULL;
  }

  melee_opponent = best_chr;
  return best_chr;
}
