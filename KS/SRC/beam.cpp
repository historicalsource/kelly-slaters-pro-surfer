// beam.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
#include "global.h"

#include "beam.h"
#include "pmesh.h"
#include "wds.h"
#include "terrain.h"
#include "vm_thread.h"
#include "app.h"
#include "collide.h"
#include "forceflags.h"
#include "game.h"
#include "matfac.h"

const rational_t BEAM_DEFAULT_THICKNESS = 0.05f;
const rational_t BEAM_DEFAULT_MAX_LENGTH = 30.0f;
const color32 BEAM_DEFAULT_COLOR( 255, 0, 0, 128 );


beam::beam( const entity_id& _id,
            unsigned int _flags,
            entity_flavor_t _flavor )
:   entity( _id, _flavor, _flags )
{
  construct( BEAM_DEFAULT_THICKNESS, BEAM_DEFAULT_MAX_LENGTH, BEAM_DEFAULT_COLOR );
}


beam::~beam()
{
  purge_effects();
  if ( my_material != NULL )
  {
    material_bank.delete_instance( my_material );
    my_material = NULL;
  }
}


void beam::purge_effects()
{
  if ( !effects.empty() )
  {
    vector<beam_effect*>::iterator i = effects.begin();
    for ( ; i!=effects.end(); ++i )
    {
      beam_effect *eff = *i;
      if ( eff )
        delete eff;
    }
    effects.resize(0);
  }
}


void beam::construct( rational_t _thickness, rational_t _max_length, const color32& _my_color )
{
  set_thickness( _thickness );
  set_max_length( _max_length );
  set_beam_color( _my_color );
  beam_flags = 0;
  curr_len = 0;
  // need an instance of the beam pmesh
  vr_pmesh* m = vr_pmesh_bank.find_instance( "_beam" );
  if ( !m )
  {
    // set up the beam pmesh object
    m = NEW vr_pmesh;
    m->make_rectangle();
    m->shrink_memory_footprint();
    // insert the beam pmesh object into the instance bank
    vr_pmesh_bank.insert_new_object( m, "_beam" );
  }
  my_visrep = vr_pmesh_bank.new_instance( m );
  entity::set_flag( EFLAG_GRAPHICS, true );
  entity::set_flag( EFLAG_GRAPHICS_VISIBLE, true );

  effect_id_counter = 0;

  my_material = NULL;
  additive = false;

  uv_anim = vector2d(0.0f, 0.0f);
  uv_coords[0] = vector2d(0.0f, 0.0f);
  uv_coords[1] = vector2d(1.0f, 1.0f);
  tiles_per_meter = -1.0f;
}


/////////////////////////////////////////////////////////////////////////////
// Instancing

entity* beam::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  beam* new_beam = NEW beam( _id, _flags );
  new_beam->copy_instance_data( *this );
  return (entity*)new_beam;
}


void beam::copy_instance_data( const beam& b )
{
  entity::copy_instance_data( b );

  additive = b.additive;
  set_thickness( b.get_thickness() );
  set_max_length( b.get_max_length() );
  set_beam_color( b.get_beam_color() );
  if ( b.my_material != NULL )
    my_material = material_bank.new_instance( b.my_material );

  effects.resize(0);
  if ( !b.effects.empty() )
  {
    vector<beam_effect*>::const_iterator i = b.effects.begin();
    for ( ; i!=b.effects.end(); ++i )
    {
      if ( (*i) )
        effects.push_back((*i)->make_instance(this));
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// entity_maker caching interface

void beam::acquire( unsigned int _flags )
{
  entity::acquire( _flags );
  set_thickness( BEAM_DEFAULT_THICKNESS );
  set_max_length( BEAM_DEFAULT_MAX_LENGTH );
  set_beam_color( BEAM_DEFAULT_COLOR );
  beam_flags = 0;
  curr_len = 0;
  additive = false;
  entity::set_flag( EFLAG_GRAPHICS, true );
  entity::set_flag( EFLAG_GRAPHICS_VISIBLE, true );
  effect_id_counter = 0;
}

void beam::release()
{
  entity::release();
  purge_effects();

  if ( my_material != NULL )
  {
    material_bank.delete_instance( my_material );
    my_material = NULL;
  }
}


void beam::set_thickness( rational_t _thickness )
{
  thickness = _thickness;
}


void beam::set_max_length( rational_t _max_length )
{
  max_length = _max_length;
}


void beam::set_beam_color( const color32& _my_color )
{
  my_color = _my_color;
}


void beam::set_point_to_point(const vector3d &pt1, const vector3d &pt2)
{
  // point1 to point2
  vector3d vec = pt2 - pt1;

  // length of beam
  rational_t len = vec.length();
  set_max_length(len);

  if(len > 0)
  {
    if(vec.x == 0.0f && vec.y == 0.0f)
    {
      if(vec.z > 0.0f)
      {
        set_rel_orientation(po_identity_matrix.get_orientation());
      }
      else if(vec.z < 0.0f)
      {
        po the_po;
        the_po.set_rot(YVEC, PI);
        set_rel_orientation(the_po.get_orientation());
      }

      set_rel_position(pt1);
    }
    else
    {
      // normalize the vec
      len = (1.0f / len);
      vec.x *= len;
      vec.y *= len;
      vec.z *= len;

      // set the orientation
      po the_po;
      rational_t dot_prod = dot(vec, ZVEC);
      vector3d cross_prod = cross(vec, ZVEC);

      if (dot_prod < -1)
      {
nglPrintf("Woah, bad dot product, man.  dot %g, vec (%g %g %g), %g\n", dot_prod, vec.x, vec.y, vec.z, len);
        dot_prod = -1;
      }
      if (dot_prod > 1)
      {
nglPrintf("Woah, bad dot product, man.  dot %g, vec (%g %g %g), %g\n", dot_prod, vec.x, vec.y, vec.z, len);
        dot_prod = 1;
      }

      the_po.set_rot((cross_prod), fast_acos(dot_prod));
      set_rel_orientation(the_po.get_orientation());

      // set the position
      set_rel_position(pt1);
    }
  }
}

void beam::set_texture( const stringx& file )
{
#ifdef TARGET_PS2
#else
  if ( my_material != NULL )
    material_bank.delete_instance( my_material );
  my_material = material_bank.new_instance( material(file) );
  assert( my_material );
#endif
}

void beam::set_texture( material* mat )
{
#ifdef TARGET_PS2
#else
  if ( my_material != NULL )
    material_bank.delete_instance( my_material );
  my_material = material_bank.new_instance( mat );
  assert( my_material );
#endif
}

void beam::set_texture( mat_fac* mat )
{
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
#else
  set_texture(mat->get_material());
#endif
}

void beam::frame_advance( time_value_t t )
{
#ifdef BEAMCULL
  bool hit_hero = false;
  const po& cur_po = get_abs_po();
  const vector3d& p0 = get_abs_position();
  const vector3d& zfacing = cur_po.get_z_facing();

  static_len = max_length;
  vector3d p1 = p0 + zfacing * max_length;
  static_endpoint = impact_point = p1;

  clear_beam_flag( HIT_WORLD );
  if ( !is_beam_flagged(LAST_FRAME_VALID) || last_po!=cur_po )
  {
    // collide with terrain
//    vector3d hit_norm; // unused -- remove me?
    if ( !is_beam_flagged(NO_CLIP_TO_WORLD) && g_world_ptr->get_the_terrain().find_intersection( p0, p1, impact_point, impact_normal ) )
    {
      p1 = static_endpoint = impact_point;
      set_beam_flag( HIT_WORLD );
    }

    // nonmoving entity collision could be moved here for optimzation purposes

    set_beam_flag( LAST_FRAME_VALID );
  }

  // collide with beamable entities in my region(s)
  if ( !is_beam_flagged( NO_CLIP_TO_BEAMABLE ) )
  {
    entity::prepare_for_visiting();
    region_node_pset::const_iterator ri = get_regions().begin();
    region_node_pset::const_iterator ri_end = get_regions().end();
    for ( ; ri!=ri_end; ++ri )
    {
	    region* r = (*ri)->get_data();
	    region::entity_list::const_iterator ei = r->get_possible_collide_entities().begin();
	    region::entity_list::const_iterator ei_end = r->get_possible_collide_entities().end();
	    for ( ; ei!=ei_end; ++ei )
	    {
		    entity *e = *ei;
        if ( e && e->are_collisions_active() && !e->already_visited() )
        {
          e->visit();
		      if ( e->is_beamable() )
		      {
            if ( collide_segment_entity(p0, p1, e, &impact_point, &impact_normal) )
              p1 = static_endpoint = impact_point;
		      }
        }
	    }
    }
  }

  // collide with hero
  if ( curr_len > 0.01f )
  {
    entity* hero = g_world_ptr->get_hero_ptr();

    if ( /*!   (is_beam_flagged(DETECTS_STEALTH) || !hero->is_stealth())
      && !*/ collide_segment_entity(p0, p1, hero, &impact_point, &impact_normal) )
    {
      hit_hero = true;
      if ( !is_beam_flagged(NO_CLIP_TO_HERO) )
        p1 = static_endpoint = impact_point;
    }

    if ( hit_hero )
    {
      if ( !is_beam_flagged(NO_CLIP_TO_HERO) )
      {
        impact_point = get_abs_position() + curr_len*get_abs_po().get_z_facing();
        impact_normal = -get_abs_po().get_z_facing();
      }
      if ( !is_beam_flagged( HIT_HERO ) )
      {
        set_beam_flag( HIT_HERO );
        raise_signal( ENTER );
      }
    }
    else if ( is_beam_flagged( HIT_HERO ) )
    {
      clear_beam_flag( HIT_HERO );
      raise_signal( LEAVE );
    }
  }

  static_len = dot( p1-p0, zfacing );
  curr_len = static_len;

  vector<beam_effect*>::iterator i;
  for ( i=effects.begin(); i!=effects.end(); )
  {
    beam_effect *eff = *i;
    if ( eff != NULL )
    {
      eff->frame_advance( t );
      if ( eff->is_dead() )
      {
        delete eff;
        i = effects.erase( i );
      }
      else
        ++i;
    }
  }

  vector2d uv_inc(uv_anim.x*t, uv_anim.y*t);
  uv_coords[0] += uv_inc;
  uv_coords[1] += uv_inc;

  while(uv_coords[0].x >= 1.0f)
  {
    uv_coords[0].x -= 1.0f;
    uv_coords[1].x -= 1.0f;
  }
  while(uv_coords[0].x <= -1.0f)
  {
    uv_coords[0].x += 1.0f;
    uv_coords[1].x += 1.0f;
  }
  while(uv_coords[0].y >= 1.0f)
  {
    uv_coords[0].y -= 1.0f;
    uv_coords[1].y -= 1.0f;
  }
  while(uv_coords[0].y <= -1.0f)
  {
    uv_coords[0].y += 1.0f;
    uv_coords[1].y += 1.0f;
  }
#endif
}

render_flavor_t beam::render_passes_needed() const
{
  render_flavor_t passes=entity::render_passes_needed();
  if (my_color.c.a<255 || additive)
    passes |= RENDER_TRANSLUCENT_PORTION;
  return passes;
}

void beam::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
//#pragma todo("This needs PS2 equivalent  jdf 3/21/01")
#ifdef TARGET_PS2
  return;
#else
  if ( curr_len > 0 )
  {
    camera* curcam = app::inst()->get_game()->get_current_view_camera();
    const vector3d& curpos = get_abs_position();
    const po& curpo = get_abs_po();
    // in local space, compute xy normal from origin to camera
    vector3d bx = curcam->get_abs_position() - curpos;
    bx = vector3d( dot(bx,curpo.get_x_facing()), dot(bx,curpo.get_y_facing()), 0 );
    rational_t d2 = bx.length2();
    if ( d2 > 0.001f )
    {
      bx *= fast_recip_sqrt( d2 );
      // in local space, vertical edge of beam's visual rectangle corresponds to
      // (ZVEC^bx)*thickness
      rational_t r = thickness * 0.5f;
      vector3d by( -bx.y*r, bx.x*r, 0 );
      // compute corner points
      assert( get_vrep()->get_type() == VISREP_PMESH );
      vr_pmesh* my_pmesh = static_cast<vr_pmesh*>( get_vrep() );
      vector3d p1( 0, 0, curr_len );
      my_pmesh->set_xvert_unxform_pos( 0, -by );
      my_pmesh->set_xvert_unxform_pos( 1, by );
      my_pmesh->set_xvert_unxform_pos( 2, p1+by );
      my_pmesh->set_xvert_unxform_pos( 3, p1-by );
      // set color
      my_pmesh->set_xvert_unxform_diffuse( 0, my_color );
      my_pmesh->set_xvert_unxform_diffuse( 1, my_color );
      my_pmesh->set_xvert_unxform_diffuse( 2, my_color );
      my_pmesh->set_xvert_unxform_diffuse( 3, my_color );
      my_pmesh->set_has_translucent_verts( my_color.c.a < 255 );

      texture_coord tc;

      if(tiles_per_meter > 0.0f)
      {
        rational_t tile_mod = ((curr_len*tiles_per_meter)-1.0f);

        tc = texture_coord(uv_coords[0].x, uv_coords[0].y );
        my_pmesh->set_xvert_tc(0, tc);

        tc = texture_coord(uv_coords[1].x, uv_coords[0].y );
        my_pmesh->set_xvert_tc(1, tc);

        tc = texture_coord(uv_coords[1].x, uv_coords[1].y+tile_mod );
        my_pmesh->set_xvert_tc(2, tc);

        tc = texture_coord(uv_coords[0].x, uv_coords[1].y+tile_mod );
        my_pmesh->set_xvert_tc(3, tc);
      }
      else
      {
        tc = texture_coord(uv_coords[0].x, uv_coords[0].y);
        my_pmesh->set_xvert_tc(0, tc);

        tc = texture_coord(uv_coords[1].x, uv_coords[0].y);
        my_pmesh->set_xvert_tc(1, tc);

        tc = texture_coord(uv_coords[1].x, uv_coords[1].y );
        my_pmesh->set_xvert_tc(2, tc);

        tc = texture_coord(uv_coords[0].x, uv_coords[1].y );
        my_pmesh->set_xvert_tc(3, tc);
      }



      // set_texture
      material *old_material = NULL;
      if(my_material != NULL)
      {
        old_material = my_pmesh->materials[0];
        my_pmesh->materials[0] = my_material;
      }

      light_manager* lm = get_light_set();
      entity::render_heart( detail, flavor|RENDER_NO_LIGHTS, lm, (additive ? FORCE_ADDITIVE_BLENDING : 0), entity_translucency_pct );
      // render without lighting
//      entity::render( detail, flavor|RENDER_NO_LIGHTS, entity_translucency_pct );

      if(old_material != NULL)
        my_pmesh->materials[0] = old_material;
    }
  }
#endif
}

unsigned short beam::add_color_effect(const color32& start_color, const color32& end_color, rational_t the_duration, rational_t the_delay, rational_t the_loop_delay, bool invert_loop)
{
  beam_effect *effect = NEW beam_effect(this);

  effect->set_color_delta(start_color, end_color, the_duration, the_delay, the_loop_delay, invert_loop);

  return(add_effect(effect));
}

unsigned short beam::add_width_effect(rational_t start_width, rational_t end_width, rational_t the_duration, rational_t the_delay, rational_t the_loop_delay, bool invert_loop)
{
  beam_effect *effect = NEW beam_effect(this);

  effect->set_width_delta(start_width, end_width, the_duration, the_delay, the_loop_delay, invert_loop);

  return(add_effect(effect));
}

unsigned short beam::add_alpha_effect(uint8 start_alpha, uint8 end_alpha, rational_t the_duration, rational_t the_delay, rational_t the_loop_delay, bool invert_loop)
{
  beam_effect *effect = NEW beam_effect(this);

  effect->set_alpha_delta(start_alpha, end_alpha, the_duration, the_delay, the_loop_delay, invert_loop);

  return(add_effect(effect));
}

unsigned short beam::add_effect( beam_effect* eff )
{
  if ( eff != NULL )
  {
    eff->set_id( effect_id_counter++ );
    effects.push_back( eff );
    return eff->get_id();
  }
  else
    return (unsigned short)-1;
}

void beam::kill_effect(unsigned short id, bool apply_target_vals)
{
  vector<beam_effect*>::iterator i = effects.begin();
  vector<beam_effect*>::const_iterator i_end = effects.end();
  for ( ; i!=i_end; i++)
  {
    beam_effect *eff = *i;
    if ( eff && eff->get_id()==id )
    {
      eff->kill( apply_target_vals );
      break;
    }
  }
}

void beam::kill_all_effects( bool apply_target_vals )
{
  vector<beam_effect*>::iterator i = effects.begin();
  vector<beam_effect*>::const_iterator i_end = effects.end();
  for ( ; i!=i_end; ++i )
  {
    beam_effect *eff = *i;
    if ( eff )
      eff->kill( apply_target_vals );
  }
}

void beam::set_visible(bool a)
{
#ifdef BEAMCULL
  bool changed = entity::is_visible() != a;
  entity::set_visible(a);
  if(changed)
    entity::region_update_poss_active();
#endif
}


/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void beam::register_signals()
{
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "beam_signals.h"
  #undef MAC
}

static const char* signal_names[] =
{
  #define MAC(label,str)  str,
  #include "beam_signals.h"
  #undef MAC
};

unsigned short beam::get_signal_id( const char *name )
{
  unsigned idx;

  for( idx = 0; idx < (sizeof(signal_names)/sizeof(char*)); idx++ )
  {
    int offset = strlen(signal_names[idx])-strlen(name);

    if( offset > (int)strlen( signal_names[idx] ) )
      continue;

    if( !strcmp(name,&signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
  }

  // not found
  return entity::get_signal_id( name );
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* beam::get_signal_name( unsigned short idx ) const
{
  assert( idx < N_SIGNALS );
  if ( idx <= PARENT_SYNC_DUMMY )
    return entity::get_signal_name( idx );
  else
    return signal_names[idx-PARENT_SYNC_DUMMY-1];
}


beam_effect::beam_effect(beam *the_beam)
{
  my_beam = the_beam;
  effect = NULL;

  mode = EFFECT_DEAD;
  timer = 0.0f;
  duration = 0.0f;
  loop_delay = -1.0f;

  effect = NULL;

  id = (unsigned short)-1;
}

beam_effect::~beam_effect()
{
  dump();
}

void beam_effect::dump()
{
  if(effect != NULL)
  {
    delete effect;
    effect = NULL;
  }
}

void beam_effect::set_color_delta(const color32& start_color, const color32& end_color, rational_t the_duration, rational_t the_delay, rational_t the_loop_delay, bool invert_loop)
{
  dump();

  effect = NEW beam_effect_color();
  beam_effect_color *color_effect = (beam_effect_color *)effect;

  color_effect->start[0] = start_color.get_red();
  color_effect->target[0] = end_color.get_red();

  color_effect->start[1] = start_color.get_green();
  color_effect->target[1] = end_color.get_green();

  color_effect->start[2] = start_color.get_blue();
  color_effect->target[2] = end_color.get_blue();

  duration = the_duration;

  if(duration > 0)
  {
    color_effect->delta[0] = (color_effect->target[0] - color_effect->start[0]) / duration;
    color_effect->delta[1] = (color_effect->target[1] - color_effect->start[1]) / duration;
    color_effect->delta[2] = (color_effect->target[2] - color_effect->start[2]) / duration;
  }

  if(invert_loop)
    mode = EFFECT_INVERTED_DELAY;
  else
    mode = EFFECT_DELAY;

  loop_delay = the_loop_delay;
  timer = the_delay;
}

void beam_effect::set_width_delta(rational_t start_width, rational_t end_width, rational_t the_duration, rational_t the_delay, rational_t the_loop_delay, bool invert_loop)
{
  dump();

  effect = NEW beam_effect_width();
  beam_effect_width *width_effect = (beam_effect_width *)effect;

  width_effect->start = start_width;
  width_effect->target = end_width;

  duration = the_duration;

  if(duration > 0)
    width_effect->delta = (width_effect->target - width_effect->start) / duration;

  if(invert_loop)
    mode = EFFECT_INVERTED_DELAY;
  else
    mode = EFFECT_DELAY;

  loop_delay = the_loop_delay;
  timer = the_delay;
}

void beam_effect::set_alpha_delta(uint8 start_alpha, uint8 end_alpha, rational_t the_duration, rational_t the_delay, rational_t the_loop_delay, bool invert_loop)
{
  dump();

  effect = NEW beam_effect_alpha();
  beam_effect_alpha *alpha_effect = (beam_effect_alpha *)effect;

  alpha_effect->start = start_alpha;
  alpha_effect->target = end_alpha;

  duration = the_duration;

  if(duration > 0)
  {
    alpha_effect->delta = (alpha_effect->target - alpha_effect->start) / duration;
  }

  if(invert_loop)
    mode = EFFECT_INVERTED_DELAY;
  else
    mode = EFFECT_DELAY;

  loop_delay = the_loop_delay;
  timer = the_delay;
}

void beam_effect::frame_advance( time_value_t t )
{
  if(my_beam && effect && is_alive())
  {
    timer -= t;

    // if we're >= 1/2 second behind, catch up, real quick
    // should never happen though, unless framerate drops to below 2 FPS or delays are too small!!!
    if(timer <= -0.5f)
      timer = 0.0f;

    if(is_delaying())
    {
      if(timer <= 0.0f)
      {
        set_active();
        effect->apply_start_vals(my_beam);

        timer = duration + timer;
      }
    }
    else if(is_active())
    {
      if(timer <= 0.0f)
      {
        effect->apply_target_vals(my_beam);

        if(is_looping())
        {
          set_delaying();

          if(is_inverted())
            effect->reverse();

          timer = loop_delay + timer;
        }
        else
          mode = EFFECT_DEAD;
      }
      else
      {
        effect->apply_delta_vals(my_beam, t);
      }
    }
  }
}

void beam_effect::set_active()
{
  if(mode == EFFECT_DELAY)
    mode = EFFECT_ACTIVE;
  else if(mode == EFFECT_INVERTED_DELAY)
    mode = EFFECT_INVERTED_ACTIVE;
}

void beam_effect::set_delaying()
{
  if(mode == EFFECT_ACTIVE)
    mode = EFFECT_DELAY;
  else if(mode == EFFECT_INVERTED_ACTIVE)
    mode = EFFECT_INVERTED_DELAY;
}


void beam_effect::kill( bool apply_target_vals )
{
  if ( apply_target_vals && my_beam && effect )
    effect->apply_target_vals( my_beam );
  mode = EFFECT_DEAD;
}

beam_effect* beam_effect::make_instance(beam *the_beam)
{
  beam_effect *eff = NEW beam_effect(the_beam);

  if ( effect )
    eff->effect = effect->make_instance();
  else
    eff->effect = NULL;

  eff->id = id;
  eff->mode = mode;
  eff->timer = timer;
  eff->loop_delay = loop_delay;
  eff->duration = duration;

  return(eff);
}

void beam_effect::read_width_chunk( chunk_file& fs )
{
  rational_t start_width = 0.02f;
  rational_t end_width = 0.075f;
  rational_t the_duration = 1.0f;
  rational_t the_delay = 0.0f;
  rational_t the_loop_delay = -1.0f;
  bool invert_loop = false;

  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "start")
    {
      serial_in(fs, &start_width);
    }
    else if(label == "end")
    {
      serial_in(fs, &end_width);
    }
    else if(label == "duration")
    {
      serial_in(fs, &the_duration);
    }
    else if(label == "delay")
    {
      serial_in(fs, &the_delay);
    }
    else if(label == "loop_delay")
    {
      serial_in(fs, &the_loop_delay);
    }
    else if(label == "invert_loop")
    {
//      serial_in(fs, &invert_loop);
      invert_loop = true;
    }
    else if(label == "looping")
    {
//      bool loop = false;
//      serial_in(fs, &loop);

      if(the_loop_delay < 0.0f)
        the_loop_delay = 0.0f;
      else
        the_loop_delay = -1.0f;
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in width section of beam_effect" );
    }

    serial_in( fs, &label );
  }

    set_width_delta(start_width, end_width, the_duration, the_delay, the_loop_delay, invert_loop);
}

void beam_effect::read_alpha_chunk( chunk_file& fs )
{
  int start_alpha = 255;
  int end_alpha = 0;
  rational_t the_duration = 1.0f;
  rational_t the_delay = 0.0f;
  rational_t the_loop_delay = -1.0f;
  bool invert_loop = false;

  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "start")
    {
      serial_in(fs, &start_alpha);
    }
    else if(label == "end")
    {
      serial_in(fs, &end_alpha);
    }
    else if(label == "duration")
    {
      serial_in(fs, &the_duration);
    }
    else if(label == "delay")
    {
      serial_in(fs, &the_delay);
    }
    else if(label == "loop_delay")
    {
      serial_in(fs, &the_loop_delay);
    }
    else if(label == "invert_loop")
    {
//      serial_in(fs, &invert_loop);
      invert_loop = true;
    }
    else if(label == "looping")
    {
//      bool loop = false;
//      serial_in(fs, &loop);

      if(the_loop_delay < 0.0f)
        the_loop_delay = 0.0f;
      else
        the_loop_delay = -1.0f;
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in alpha section of beam_effect" );
    }

    serial_in( fs, &label );
  }

  set_alpha_delta((uint8)start_alpha, (uint8)end_alpha, the_duration, the_delay, the_loop_delay, invert_loop);
}

void beam_effect::read_color_chunk( chunk_file& fs )
{
  int start_r = 255;
  int start_g = 0;
  int start_b = 0;

  int end_r = 0;
  int end_g = 255;
  int end_b = 0;

  rational_t the_duration = 1.0f;
  rational_t the_delay = 0.0f;
  rational_t the_loop_delay = -1.0f;
  bool invert_loop = false;

  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "start")
    {
      serial_in(fs, &start_r);
      serial_in(fs, &start_g);
      serial_in(fs, &start_b);
    }
    else if(label == "end")
    {
      serial_in(fs, &end_r);
      serial_in(fs, &end_g);
      serial_in(fs, &end_b);
    }
    else if(label == "duration")
    {
      serial_in(fs, &the_duration);
    }
    else if(label == "delay")
    {
      serial_in(fs, &the_delay);
    }
    else if(label == "loop_delay")
    {
      serial_in(fs, &the_loop_delay);
    }
    else if(label == "invert_loop")
    {
//      serial_in(fs, &invert_loop);
      invert_loop = true;
    }
    else if(label == "looping")
    {
//      bool loop = false;
//      serial_in(fs, &loop);

      if(the_loop_delay < 0.0f)
        the_loop_delay = 0.0f;
      else
        the_loop_delay = -1.0f;
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in color section of beam_effect" );
    }

    serial_in( fs, &label );
  }

    set_color_delta(color32((uint8)start_r, (uint8)start_g, (uint8)start_b, 255), color32((uint8)end_r, (uint8)end_g, (uint8)end_b, 255), the_duration, the_delay, the_loop_delay, invert_loop);
}

bool beam_effect::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  if(label == "alpha:")
    read_alpha_chunk(fs);
  else if(label == "width:")
    read_width_chunk(fs);
  else if(label == "color:")
    read_color_chunk(fs);
  else
    error( fs.get_filename() + ": unknown beam_effect type '" + label + "'" );

  return(true);
}


// compute all regions hit by this beam
void beam::compute_sector( terrain& ter, bool use_high_res_intersect )
{
#ifdef BEAMCULL
  set_needs_compute_sector(false);

  if ( !entity::is_flagged(EFLAG_REGION_FORCED)
    && ( !entity::is_flagged(LAST_FRAME_VALID) || last_po!=get_abs_po() )
    )
  {
    sector* sec = ter.find_sector( get_abs_position() );
    my_sector = sec;
//    entity::set_flag( EFLAG_MISC_COMP_SECT_THIS_FRAME, true );
    if ( sec )
    {
      // check all regions recursively
      vector<region_node*> new_regions;
      new_regions.resize(0);
      region::prepare_for_visiting();
      region_node* rn = sec->get_region();

      center_region = rn;

      assert( rn );
      _intersect( rn, new_regions );
      // remove me from obsolete regions and add me to NEW ones
      region_node_pset::iterator i,j;
      for ( i=in_regions.begin(); i!=in_regions.end(); )
      {
        region_node* r = *i;
        if ( !r->get_data()->already_visited() )
        {
          remove_me_from_region( r->get_data() );
          j = i;
          ++j;
          in_regions.erase( i );
          i = j;
        }
        else
          ++i;
      }
      vector<region_node*>::const_iterator k = new_regions.begin();
      vector<region_node*>::const_iterator k_end = new_regions.end();
      for ( ; k!=k_end; ++k )
        add_region( *k );
    }
    else
    {
      center_region = NULL;
    }
  }
#endif
}

void beam::_intersect( region_node* rn, vector<region_node*>& new_regions ) const
{
#ifdef BEAMCULL
  rn->get_data()->visit();
  new_regions.push_back( rn );
  // check for intersection with portals leading from this region
  edge_iterator tei = rn->begin();
  edge_iterator tei_end = rn->end();
  for ( ; tei!=tei_end; ++tei )
  {
    // don't bother with regions we've already visited
    region_node* dest = (*tei).get_dest();
    if ( !dest->get_data()->already_visited() )
    {
      // also, don't pass through inactive portals
      portal* port = (*tei).get_data();
      if ( port->is_active() && port->touches_segment(get_abs_position(),static_endpoint) )
      {
        _intersect( dest, new_regions );
      }
    }
  }
#endif
}




/*
  rational_t thickness;      // thickness of beam
  rational_t max_length;     // maximum length of beam
  color32 my_color;          // color of beam

  vector<beam_effect*> effects;

//#ifdef TWO_PASS_RENDER
//  render_flavor_t my_render_flavor;
//#endif

  unsigned int flags;  // beam-specific flags

  po last_po;                       // po from last frame
  vector3d static_endpoint;         // endpoint computed based on static collisions
  rational_t static_len;            // beam length corresponding to static_endpoint
  rational_t curr_len;              // final beam length computed this frame

  vector3d impact_point;
  vector3d impact_normal;

  unsigned short effect_id_counter; // id counter for effects

  material* my_material;

*/
bool beam::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("beam:") )
  {
    stringx label;
    for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
      handle_enx_chunk( fs, label );

    compute_sector(g_world_ptr->get_the_terrain());

    return true;
  }

  return (entity::parse_instance( pcf, fs ) );
}

bool beam::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  if(label == "length")
  {
    serial_in(fs, &max_length);
    return(true);
  }
  else if(label == "width" || label == "thickness")
  {
    serial_in(fs, &thickness);
    return(true);
  }
  else if(label == "additive")
  {
    additive = true;
    return(true);
  }
  else if(label == "dir")
  {
    vector3d dir;
    serial_in(fs, &dir);
    dir.normalize();

    set_point_to_point(get_abs_position(), get_abs_position() + (dir*max_length));

    return(true);
  }
  else if(label == "color")
  {
    vector4d color;
    serial_in(fs, &color);

    my_color.set_red((unsigned char)(color.x*255.0f));
    my_color.set_green((unsigned char)(color.y*255.0f));
    my_color.set_blue((unsigned char)(color.z*255.0f));
    my_color.set_alpha((unsigned char)(color.w*255.0f));

    return(true);
  }
  else if(label == "texture")
  {
    serial_in(fs, &label);
    set_texture(label);

    return(true);
  }
  else if(label == "effects:")
  {
    for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
    {
      beam_effect *eff = NEW beam_effect(NULL);
      eff->handle_enx_chunk(fs, label);

      add_effect(eff);
    }

    return(true);
  }
  else if(label == "collision:")
  {
    for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
    {
      label.to_upper();

      if(label == "HERO")
        clear_beam_flag(NO_CLIP_TO_HERO);
      else if(label == "NO_HERO")
        set_beam_flag(NO_CLIP_TO_HERO);

      else if(label == "BEAMABLE")
        clear_beam_flag(NO_CLIP_TO_BEAMABLE);
      else if(label == "NO_BEAMABLE")
        set_beam_flag(NO_CLIP_TO_BEAMABLE);

      else if(label == "WORLD")
        clear_beam_flag(NO_CLIP_TO_WORLD);
      else if(label == "NO_WORLD")
        set_beam_flag(NO_CLIP_TO_WORLD);

      else if(label == "ALL")
        clear_beam_flag(NO_CLIPPING);
      else if(label == "NONE")
        set_beam_flag(NO_CLIPPING);
    }

    return(true);
  }


  return(entity::handle_enx_chunk( fs, label ));
}
