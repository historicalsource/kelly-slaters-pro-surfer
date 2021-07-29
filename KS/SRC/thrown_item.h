#ifndef _THROWN_ITEM_H_
#define _THROWN_ITEM_H_

// BIGCULL #include "handheld_item.h"
//!//#include "character.h"
//#include "beam.h"
#include "random.h"

class thrown_item;
class beam;
//class sound_instance;

// low 2 nibbles for handheld items
// rest for subclasses
enum
{
  _THROWN_BOUNCE            = 0x00000400,
  _THROWN_TIMER             = 0x00001000,
  _THROWN_CONTACT_DETONATE  = 0x00002000,
  _THROWN_ROCKET            = 0x00004000,
  _THROWN_COPY_VISREP       = 0x00008000,
  _THROWN_GUIDED            = 0x00010000,
  _THROWN_STICKY            = 0x00080000,
  _THROWN_RADIO_DETONATE    = 0x00100000,
  _THROWN_STICKY_ORIENT     = 0x00200000,
  _THROWN_LASER_DETONATE    = 0x00400000,
  _THROWN_TRIGGER_RADIUS    = 0x00800000,
  _THROWN_VOLATILE          = 0x02000000,
  _THROWN_HIT_HERO_ONLY     = 0x10000000,
  _THROWN_HIT_AI_ONLY       = 0x20000000,
  _THROWN_EXPLOSIVE         = 0x40000000,
  _THROWN_POINT_GUIDED      = 0x80000000
};


class thrown_item : public handheld_item
{
protected:
  int damage;

  rational_t direct_damage_mod;
  rational_t damage_radius;

  rational_t timer;
  rational_t arm_timer;

  rational_t bounce_factor;
  rational_t slide_factor;

  handheld_item_effect explode_effect;
  handheld_item_effect bounce_effect;
  handheld_item_effect throw_effect;
  handheld_item_effect arm_effect;


  stringx radio_detonator_name;
  stringx radio_detonated_item_name;

  vector3d launch_vec;
  rational_t launch_force;

  vector3d last_detonate_pos;

  virtual void init_defaults();

  grenade *last_grenade_spawned;
  grenade *last_grenade_detonated;
  grenade *last_grenade_armed;

  entity *vis_ent;

  rational_t guided_percent;
  rational_t guided_accuracy;
  rational_t turn_factor;
  rational_t accel_factor;
  rational_t wobble_timer;
  rational_t guidance_delay;
  rational_t accel_delay;
  rational_t wobble_timer_var;
  rational_t guidance_delay_var;
  rational_t accel_delay_var;

  rational_t sticky_offset;

  rational_t trigger_radius;

  friend class grenade;

  vector<grenade *> grenade_pool;
  grenade *get_new_grenade();

  bool affect_switches;

public:
  thrown_item( const entity_id& _id, unsigned int _flags );

  thrown_item( const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

  virtual ~thrown_item();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_thrown_item() const { return true; }

// NEWENT File I/O
public:
  thrown_item( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

  virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );

// Old File I/O
public:
  thrown_item( const stringx& item_type_filename,
        const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_PHYSICAL,
        bool _active = INACTIVE,
        bool _stationary = false );

// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  virtual void copy_instance_data( thrown_item& b );

  void read_bounce_chunk( chunk_file& fs );
  void read_explode_chunk( chunk_file& fs );
  void read_arm_chunk( chunk_file& fs );
  void read_throw_chunk( chunk_file& fs );
  void read_damage_chunk( chunk_file& fs );
  void read_rocket_chunk( chunk_file& fs );

  friend void thrown_item_remove_live_grenade(signaller* sig, const char *data);
  vector<grenade *> live_grenades;

public:
  const vector<grenade *>& get_live_grenades() const { return live_grenades; }

  virtual void apply_effects( entity* ent );

  bool has_bounce() const           { return(get_handheld_flag(_THROWN_BOUNCE)); }
  bool has_timer() const            { return(get_handheld_flag(_THROWN_TIMER)); }
  bool detonate_on_contact() const  { return(get_handheld_flag(_THROWN_CONTACT_DETONATE)); }
  bool is_rocket() const            { return(get_handheld_flag(_THROWN_ROCKET)); }
  bool is_guided() const            { return(get_handheld_flag(_THROWN_GUIDED)); }
  bool is_point_guided() const      { return(get_handheld_flag(_THROWN_POINT_GUIDED)); }
  bool is_sticky() const            { return(get_handheld_flag(_THROWN_STICKY)); }
  bool is_radio_detonated() const   { return(get_handheld_flag(_THROWN_RADIO_DETONATE)); }
  bool is_sticky_orient() const     { return(get_handheld_flag(_THROWN_STICKY_ORIENT)); }
  bool is_laser_detonated() const   { return(get_handheld_flag(_THROWN_LASER_DETONATE) && get_handheld_flag(_THROWN_STICKY) && get_handheld_flag(_THROWN_STICKY_ORIENT)); }
  bool has_trigger_radius() const   { return(get_handheld_flag(_THROWN_TRIGGER_RADIUS)); }
  bool is_volatile() const          { return(get_handheld_flag(_THROWN_VOLATILE)); }
  bool is_explosive() const         { return(get_handheld_flag(_THROWN_EXPLOSIVE)); }

  bool hit_hero_only() const        { return(get_handheld_flag(_THROWN_HIT_HERO_ONLY)); }
  bool hit_ai_only() const          { return(get_handheld_flag(_THROWN_HIT_AI_ONLY)); }
  bool affects_switches() const     { return(affect_switches); }


  void set_detonate_on_contact(bool det)  { set_handheld_flag(_THROWN_CONTACT_DETONATE, det); }

  void set_detonate_position(vector3d pos) { last_detonate_pos = pos; }
  virtual vector3d get_detonate_position() const { return(last_detonate_pos); }

  grenade *get_last_grenade_spawned()  const   { return(last_grenade_spawned); }
  grenade *get_last_grenade_armed()  const     { return(last_grenade_armed); }

  // may not always be a valid pointer. do not access any members of this return, just use for pointer comparisons
  grenade *get_last_grenade_detonated()  const { return(last_grenade_detonated); }

  thrown_item *get_radio_detonator() const;
  thrown_item *get_radio_detonated_item() const;

  void set_launch_vec(const vector3d &vec)  { launch_vec = vec; launch_vec.normalize(); }
  void set_launch_force(rational_t force)   { launch_force = force; }
  const vector3d &get_launch_vec() const    { return(launch_vec); }
  rational_t get_launch_force() const       { return(launch_force); }
  vector3d invert_launch_vec(const vector3d &vec);

  rational_t get_damage_radius() const      { return(damage_radius); }
  rational_t get_guided_accuracy() const    { return(guided_accuracy); }
  rational_t get_turn_factor() const        { return(turn_factor); }
  rational_t get_accel_factor() const       { return(accel_factor); }
  rational_t get_wobble_timer() const       { return(wobble_timer); }
  rational_t get_guidance_delay() const     { return(guidance_delay); }
  rational_t get_accel_delay() const        { return(accel_delay); }
  rational_t get_wobble_timer_var() const   { return(wobble_timer_var); }
  rational_t get_guidance_delay_var() const { return(guidance_delay_var); }
  rational_t get_accel_delay_var() const    { return(accel_delay_var); }
  rational_t get_sticky_offset() const      { return(sticky_offset); }
  rational_t get_trigger_radius() const     { return(trigger_radius); }

  virtual void spawn_grenade(vector3d dir, rational_t force);
//  int get_dread_net_detonate_cue() const    { return(dread_net_detonate_cue); }

  vector3d calculate_target_vector(const vector3d &target, const vector3d &from);

  virtual void frame_advance( time_value_t t );
};

class grenade : public entity
{
protected:
  friend class CPlayer;
  bool ready_for_spawning;
  bool armed;
  bool detonated;
  bool stuck;
  bool allow_hit_owner;
  bool use_owner_last_pos;
  bool stuck_parent_was_alive_last_frame;

  thrown_item *item_owner;
  rational_t timer;
  rational_t arm_timer;
  rational_t grenade_scale;

  rational_t beam_update_timer;

  unsigned int sound_id;
  unsigned int arm_sound_id;

  virtual entity *check_hit(const vector3d &old_pos, const vector3d &new_pos, vector3d &hit_pos);
  virtual bool check_if_hit(entity *ent, const vector3d &old_pos, const vector3d &new_pos, vector3d &hit_pos);
//  virtual void bounce(vector3d old_pos, vector3d new_pos, vector3d hit, vector3d hit_norm, entity *hit_entity, time_value_t t);

  friend class thrown_item;

  entity *visual_entity;
  beam *laser_beam;
  rational_t beam_length;

  virtual void clear(bool force_detonated = false);

public:
  grenade( const entity_id& _id, unsigned int _flags );
  virtual ~grenade();

  thrown_item *get_item_owner() { return(item_owner); }

  virtual void frame_advance( time_value_t t );
  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual void detonate(entity *ent = NULL);
  virtual bool is_a_grenade() const { return true; }
  bool is_ready_to_be_armed() const { return(arm_timer <= 0.0f && (!item_owner || !item_owner->is_laser_detonated() || stuck)); }


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
    #include "grenade_signals.h"
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

/*
class rocket : public grenade
{
protected:
  bool guided;
  bool point_guided;

  vector3d target_pos;
  entity *target;

  rational_t force;

  rational_t wobble_timer;
  rational_t guidance_delay;
  rational_t accel_delay;

  friend class thrown_item;

  virtual void clear();

public:
  rocket( const entity_id& _id, unsigned int _flags );
  virtual ~rocket();

  virtual void frame_advance( time_value_t t );

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_rocket() const { return true; }
};
*/

#endif
