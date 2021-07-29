// beam.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
#ifndef _BEAM_H
#define _BEAM_H


#include "entity.h"
#include "color.h"
#include "renderflav.h"
#include "project.h"


class vr_pmesh;
class material;
class mat_fac;

//#if defined( TARGET_PC )
//#define TWO_PASS_RENDER
//#endif

class beam_effect;

class beam : public entity
{
// Types
public:
  enum flags_t
  {
    LAST_FRAME_VALID      = 0x00000001,
    HIT_WORLD             = 0x00000002,
    HIT_HERO              = 0x00000004,
    NO_CLIP_TO_HERO       = 0x00000008,
    NO_CLIP_TO_BEAMABLE   = 0x00000040,
    NO_CLIP_TO_WORLD      = 0x00000080,
    NO_CLIPPING           = NO_CLIP_TO_HERO | NO_CLIP_TO_BEAMABLE | NO_CLIP_TO_WORLD,
    DETECTS_STEALTH       = 0x00000100
  };



// Data
private:
  rational_t thickness;      // thickness of beam
  rational_t max_length;     // maximum length of beam
  color32 my_color;          // color of beam

  vector<beam_effect*> effects;

//#ifdef TWO_PASS_RENDER
//  render_flavor_t my_render_flavor;
//#endif

  unsigned int beam_flags;  // beam-specific flags

  po last_po;                       // po from last frame
  vector3d static_endpoint;         // endpoint computed based on static collisions
  rational_t static_len;            // beam length corresponding to static_endpoint
  rational_t curr_len;              // final beam length computed this frame

  vector3d impact_point;
  vector3d impact_normal;

  vector2d uv_anim;
  vector2d uv_coords[2];
  rational_t tiles_per_meter;


  unsigned short effect_id_counter; // id counter for effects

  material* my_material;
  bool additive;

public:
  beam( const entity_id& _id,
        unsigned int _flags = 0,
        entity_flavor_t _flavor = ENTITY_BEAM );

  virtual ~beam();

  void purge_effects();
  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );
  virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );

private:
  void construct( rational_t _thickness, rational_t _max_length, const color32& _my_color );

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_beam() const { return true; }

/////////////////////////////////////////////////////////////////////////////
// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const beam& b );

/////////////////////////////////////////////////////////////////////////////
// entity_maker caching interface
public:
  virtual void acquire( unsigned int _flags );
  virtual void release();

/////////////////////////////////////////////////////////////////////////////
public:
  void set_beam_flag( flags_t f ) { beam_flags |= f; }
  void clear_beam_flag( flags_t f ) { beam_flags &= ~f; }
  bool is_beam_flagged( flags_t f ) const { return (beam_flags & f); }

  bool hit_world() const { return is_beam_flagged(HIT_WORLD); }
  bool hit_hero() const { return is_beam_flagged(HIT_HERO); }

  // this controls whether the beam is clipped when it hits the hero
  void set_no_clip_to_hero( bool v ) { if (v) set_beam_flag(NO_CLIP_TO_HERO); else clear_beam_flag(NO_CLIP_TO_HERO); }

  const vector3d& get_impact_point() const { return impact_point; }
  const vector3d& get_impact_normal() const { return impact_normal; }

  // set the thickness of the beam
  void set_thickness( rational_t _thickness );
  rational_t get_thickness() const { return thickness; }

  // set the maximum length of the beam
  void set_max_length( rational_t _max_length );
  rational_t get_max_length() const { return max_length; }
  rational_t get_curr_length() const { return curr_len; }

  // set the color of the beam
  void set_beam_color( const color32& _my_color );
  color32 get_beam_color() const { return my_color; }

  virtual void compute_sector( terrain& ter, bool use_high_res_intersect = false );

  // handy dandy point-to-point protocol for beam
  void set_point_to_point(const vector3d &pt1, const vector3d &pt2);

  void set_texture( const stringx& file );
  void set_texture( material* mat );
  void set_texture( mat_fac* mat );

  void set_additive(bool a) { additive = a; }
  bool is_additive() const { return(additive); }

  virtual render_flavor_t render_passes_needed() const;

  virtual void frame_advance( time_value_t t );
  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );

  unsigned short add_color_effect(const color32& start_color, const color32& end_color, rational_t the_duration, rational_t the_delay = 0.0f, rational_t the_loop_delay = -1.0f, bool invert_loop = false);
  unsigned short add_width_effect(rational_t start_width, rational_t end_width, rational_t the_duration, rational_t the_delay = 0.0f, rational_t the_loop_delay = -1.0f, bool invert_loop = false);
  unsigned short add_alpha_effect(uint8 start_alpha, uint8 end_alpha, rational_t the_duration, rational_t the_delay = 0.0f, rational_t the_loop_delay = -1.0f, bool invert_loop = false);
  unsigned short add_effect(beam_effect *eff);
  void kill_effect(unsigned short id, bool apply_target_vals = false);
  void kill_all_effects(bool apply_target_vals = false);

  virtual void set_visible( bool a );
  virtual bool possibly_active() const        { return(is_visible()); }

  const vector2d &get_uv_anim() const         { return(uv_anim); }
  void set_uv_anim(const vector2d &v)         { uv_anim = v; }
  rational_t get_tiles_per_meter() const      { return(tiles_per_meter); }
  void set_tiles_per_meter(rational_t t)      { tiles_per_meter = t; }


private:
  void _intersect( region_node* rn, vector<region_node*>& new_regions ) const;

/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////
public:
  // enum of local signal ids (for coding convenience and readability)
  enum signal_id_t
  {
    // a descendant class uses the following line to append its local signal ids after the parent's
    PARENT_SYNC_DUMMY = entity::N_SIGNALS - 1,
    #define MAC(label,str)  label,
    #include "beam_signals.h"
    #undef MAC
    N_SIGNALS
  };

  // This static function must be implemented by every class which can generate
  // signals, and is called once only by the application for each such class;
  // the effect is to register the name and local id of each signal with the
  // signal_manager.  This call must be performed before any signal objects are
  // actually created for this class (via signaller::signal_ptr(); see signal.h).
  static void register_signals();

  static unsigned short get_signal_id( const char *name );

private:
  // Every descendant of signaller that expects to generate signals and has
  // defined its own local list of signal ids should implement this virtual
  // function for the construction of the signal list, so that it will reserve
  // exactly the number of signal pointers required, on demand.
  virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

protected:
  // This virtual function, used only for debugging purposes, returns the
  // name of the given local signal
  virtual const char* get_signal_name( unsigned short idx ) const;
};




class beam_effect_type
{
protected:
  beam_effect_type() {}
  virtual ~beam_effect_type() {}

  virtual void apply_start_vals(beam *the_beam) {}
  virtual void apply_target_vals(beam *the_beam) {}
  virtual void apply_delta_vals(beam *the_beam, time_value_t t) {}

  virtual void reverse() {}

  virtual beam_effect_type* make_instance() { return(NULL); }

  friend class beam_effect;
};

class beam_effect_width : public beam_effect_type
{
protected:
  beam_effect_width()
  {
  }

  virtual ~beam_effect_width()
  {
  }

  virtual void apply_start_vals(beam *the_beam)
  {
    the_beam->set_thickness(start);
  }

  virtual void apply_target_vals(beam *the_beam)
  {
    the_beam->set_thickness(target);
  }

  virtual void apply_delta_vals(beam *the_beam, time_value_t t)
  {
    the_beam->set_thickness(the_beam->get_thickness() + delta * t);
  }

  virtual void reverse()
  {
    rational_t temp = start;
    start = target;
    target = temp;

    delta = -delta;
  }

  rational_t start;
  rational_t target;
  rational_t delta;

  virtual beam_effect_type* make_instance()
  {
    beam_effect_width *eff = NEW beam_effect_width();

    eff->start  = start;
    eff->target = target;
    eff->delta  = delta;

    return(eff);
  }

  friend class beam_effect;
};

class beam_effect_alpha : public beam_effect_type
{
protected:
  beam_effect_alpha()
  {
  }

  virtual ~beam_effect_alpha()
  {
  }

  virtual void apply_start_vals(beam *the_beam)
  {
    color32 col = the_beam->get_beam_color();
    curr = (rational_t)start;
    the_beam->set_beam_color(color32(col.get_red(), col.get_green(), col.get_blue(), (uint8)(curr + 0.5f)));
  }

  virtual void apply_target_vals(beam *the_beam)
  {
    color32 col = the_beam->get_beam_color();
    curr = (rational_t)target;
    the_beam->set_beam_color(color32(col.get_red(), col.get_green(), col.get_blue(), (uint8)(curr + 0.5f)));
  }

  virtual void apply_delta_vals(beam *the_beam, time_value_t t)
  {
    color32 col = the_beam->get_beam_color();
    curr += delta * t;
    the_beam->set_beam_color(color32(col.get_red(), col.get_green(), col.get_blue(), (uint8)(curr + 0.5f)));
  }

  virtual void reverse()
  {
    uint8 temp = start;
    start = target;
    target = temp;

    delta = -delta;
  }

  uint8 start;
  uint8 target;
  rational_t delta;
  rational_t curr;

  virtual beam_effect_type* make_instance()
  {
    beam_effect_alpha *eff = NEW beam_effect_alpha();

    eff->start  = start;
    eff->target = target;
    eff->delta  = delta;
    eff->curr   = curr;

    return(eff);
  }

  friend class beam_effect;
};

class beam_effect_color : public beam_effect_type
{
protected:
  beam_effect_color()
  {
  }

  virtual ~beam_effect_color()
  {
  }

  virtual void apply_start_vals(beam *the_beam)
  {
    color32 col = the_beam->get_beam_color();
    curr[0] = (rational_t)start[0];
    curr[1] = (rational_t)start[1];
    curr[2] = (rational_t)start[2];
    the_beam->set_beam_color(color32((uint8)(curr[0] + 0.5f), (uint8)(curr[1] + 0.5f), (uint8)(curr[2] + 0.5f), col.get_alpha()));
  }

  virtual void apply_target_vals(beam *the_beam)
  {
    color32 col = the_beam->get_beam_color();
    curr[0] = (rational_t)target[0];
    curr[1] = (rational_t)target[1];
    curr[2] = (rational_t)target[2];
    the_beam->set_beam_color(color32((uint8)(curr[0] + 0.5f), (uint8)(curr[1] + 0.5f), (uint8)(curr[2] + 0.5f), col.get_alpha()));
  }

  virtual void apply_delta_vals(beam *the_beam, time_value_t t)
  {
    color32 col = the_beam->get_beam_color();
    curr[0] += delta[0] * t;
    curr[1] += delta[1] * t;
    curr[2] += delta[2] * t;
    the_beam->set_beam_color(color32((uint8)(curr[0] + 0.5f), (uint8)(curr[1] + 0.5f), (uint8)(curr[2] + 0.5f), col.get_alpha()));
  }

  virtual void reverse()
  {
    for(int i=0; i<3; ++i)
    {
      uint8 temp = start[i];
      start[i] = target[i];
      target[i] = temp;

      delta[i] = -delta[i];
    }
  }

  uint8 start[3];
  uint8 target[3];
  rational_t delta[3];
  rational_t curr[3];

  virtual beam_effect_type* make_instance()
  {
    beam_effect_color *eff = NEW beam_effect_color();

    memcpy(&eff->start[0], &start[0], sizeof(start[0])*3);
    memcpy(&eff->target[0], &target[0], sizeof(target[0])*3);
    memcpy(&eff->delta[0], &delta[0], sizeof(delta[0])*3);
    memcpy(&eff->curr[0], &curr[0], sizeof(curr[0])*3);

    return(eff);
  }

  friend class beam_effect;
};

class beam_effect
{
private:
  enum effect_mode
  {
    EFFECT_DEAD            = 0,
    EFFECT_DELAY           = 1,
    EFFECT_ACTIVE          = 2,
    EFFECT_INVERTED_DELAY  = -1,
    EFFECT_INVERTED_ACTIVE = -2,
  };

  beam *my_beam;
  beam_effect_type *effect;

  unsigned short id;

  char mode;

  rational_t timer;

  rational_t loop_delay;
  rational_t duration;

  virtual void dump();

  virtual void set_active();
  virtual void set_delaying();

public:
  beam_effect(beam *the_beam);
  virtual ~beam_effect();

  virtual void set_color_delta(const color32& start_color, const color32& end_color, rational_t the_duration, rational_t the_delay = 0.0f, rational_t the_loop_delay = -1.0f, bool invert_loop = false);
  virtual void set_width_delta(rational_t start_width, rational_t end_width, rational_t the_duration, rational_t the_delay = 0.0f, rational_t the_loop_delay = -1.0f, bool invert_loop = false);
  virtual void set_alpha_delta(uint8 start_alpha, uint8 end_alpha, rational_t the_duration, rational_t the_delay = 0.0f, rational_t the_loop_delay = -1.0f, bool invert_loop = false);

  virtual void frame_advance( time_value_t t );

  virtual void kill(bool apply_target_vals = false);

  virtual bool is_dead() const          { return(mode == EFFECT_DEAD); }
  virtual bool is_alive() const         { return(mode != EFFECT_DEAD); }

  virtual bool is_delaying() const      { return(mode == EFFECT_DELAY || mode == EFFECT_INVERTED_DELAY); }
  virtual bool is_active() const        { return(mode == EFFECT_ACTIVE || mode == EFFECT_INVERTED_ACTIVE); }

  virtual bool is_looping() const       { return(loop_delay >= 0.0f); }
  virtual bool is_inverted() const      { return(mode == EFFECT_INVERTED_DELAY || mode == EFFECT_INVERTED_ACTIVE); }

  virtual unsigned short get_id() const     { return(id); }
  virtual void set_id(unsigned short idx)   { id = idx; }

  virtual beam_effect* make_instance(beam *the_beam);

  virtual void read_width_chunk( chunk_file& fs);
  virtual void read_alpha_chunk( chunk_file& fs);
  virtual void read_color_chunk( chunk_file& fs);
  virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );
};



#endif  // _BEAM_H
