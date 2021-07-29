#ifndef ITEM_H
#define ITEM_H


#include "algebra.h"
#include "stringx.h"
#include "instance.h"
#include "beam.h"

//!class character;
class character_attributes;
class vm_executable;


// Item system must satisfy the following requirements:
//    No creation of items during gameplay.
//    Multiple copies of items in a person's inventory become enumerated.
//
// To accomplish this, we use the following algorithm:
//    Item objects are always either in the wds, or in a character's inventory.
//    An Item added to the world is simply added normally to the wds, with it's count == 1.
//    An Item added to a character the first time is added normally to the character's inv, with it's count == 1.
//    Subsequent copies of that item added to the character are not created, but the count is incremented.
//    Items that are expended have their count reduced to zero.
//    An item that is transferred from wds to character:
//      If character doesn't already have a like item, the item is moved normally.
//      If character has a like item, item in wds is expended and count of iten in character inv is incremented.
//    An item that is transferred from character to wds:
//      If item has a count of 1, item is moved normally.
//      Any other case generates an error.
//    Item lists destroy their items when they are destroyed, regardless of count (even 0).
class item : public entity
{
// Types
public:
  enum usage_t
  {
    INVALID = -1,
    INSTANT,
    INVENTORY,
    UTILITY,
    GUN,
    THROWN,
    MELEE,
    AMMO,
    HEALTH,
    ARMOR,
    ENERGY,
    PERMANENT
  };

// Constructors
public:
  item( const entity_id& _id, unsigned int _flags );

  item( const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

  virtual ~item();

	// This function allows the entity to any post-level-load initialization
	virtual void initialize();

  /////////////////////////////////////////////////////////////////////////////
  // entity class identification
  public:
    virtual bool is_an_item() const { return true; }

// NEWENT File I/O
public:
  item( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

//    virtual void read_enx( chunk_file& fs );
  virtual bool handle_enx_chunk( chunk_file& fs, stringx& lstr );
  void read_item_data( chunk_file& fs, stringx& lstr );

// Old File I/O
public:
  item( const stringx& item_type_filename,
        const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_PHYSICAL,
        bool _active = INACTIVE,
        bool _stationary = false,
        bool delete_stream = SKIP_DELETE_STREAM );

// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  virtual void copy_instance_data( const item& b );

// Misc.
public:
  const stringx& get_name() const { return name; }
  usage_t get_usage_type() const { return usage_type; }
  bool is_same_item( const item& b ) const;

  virtual int get_count() const {return count;}
  virtual void inc_count() {count++;}
  virtual void dec_count() {count--;}
  virtual void set_count(int c){count=c;}

  virtual bool is_usable() const { return get_count() > 0; }

  virtual int get_number() const {return count;}

  virtual void frame_advance(time_value_t t);
  virtual void render(camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct);

  bool check_for_pickup();
  virtual bool give_to_entity(entity * target);

  void spawn_item_script();

  virtual void preload();

  virtual void apply_effects( entity* target );

  bool is_picked_up();
  void set_picked_up(bool x){picked_up = x;}
  void set_pickup_timer(rational_t t){pickup_timer = t;}

  virtual bool is_ammo()   const { return (get_usage_type() == AMMO); }
  virtual bool is_health() const { return (get_usage_type() == HEALTH); }
  virtual bool is_armor()  const { return (get_usage_type() == ARMOR); }
  virtual bool is_brain_weapon() const { return(false); }

  virtual void holster(bool make_visible = true)    {}
  virtual void draw(bool make_visible = true)       {}
  virtual void hide()       {}
  virtual void show()       {}

  bool is_linked() const    { return(linked); }
  void set_linked(bool l)   { linked = l; }

  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );

//!  virtual int get_max_allowed(character *chr);

  rational_t get_interface_orientation() const { return interface_orientation; }

#if _ENABLE_WORLD_EDITOR
  int get_original_count() { return original_count; }
  void set_original_count(int c) { original_count = c; }
#endif

protected:
  void spawn_preload_script();

  bool preload_script_called;
  bool item_script_called;
  bool linked;

  usage_t usage_type;
  stringx name;
  int count;
  int default_count;
  bool picked_up;
  rational_t pickup_timer;

  rational_t icon_scale;  // scale for displaying item in world (before picked up)

  rational_t interface_orientation;  // orientation on y in degrees

//  int dread_net_use_cue;

  int max_num;
//  bool need_to_initialize;  // this flag tells us if the item needs to perform special post-load initialization

#if _ENABLE_WORLD_EDITOR
  int original_count;
#endif

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
    #include "item_signals.h"
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

inline item* find_item( const entity_id& id , bool unknown_ok = FIND_ENTITY_UNKNOWN_NOT_OK )
  {
  return (item*)entity_manager::inst()->find_entity( id, ENTITY_ITEM, unknown_ok );
  }


#ifdef _DEBUG
  #define _VIS_ITEM_DEBUG_HELPER 0 // BIGCULL
#else
  #define _VIS_ITEM_DEBUG_HELPER 0
#endif

#if _VIS_ITEM_DEBUG_HELPER
  extern bool g_render_vis_item_debug_info;
  extern void render_vis_item_debug_info();
#endif


class visual_item : public entity
{
protected:
  friend class gun;
  friend class thrown_item;
  friend class item;
  friend class grenade;
//!  friend class character;

public:
#if _VIS_ITEM_DEBUG_HELPER
  static beam *visual_item_beamer;
  bool render_axis;
#endif

  visual_item( const entity_id& _id, unsigned int _flags );
  virtual ~visual_item();

  /////////////////////////////////////////////////////////////////////////////
  // entity class identification
  virtual bool is_a_visual_item() const { return true; }

  virtual void set_placement(entity *owner, const stringx& limb, const po& offset, bool drawn);

#if _VIS_ITEM_DEBUG_HELPER
  virtual void set_placement(entity *owner, const stringx& limb, rational_t s, const vector3d& p, const vector3d& r, bool drawn);
  virtual void alter_placement(rational_t s, const vector3d& p, const vector3d& r);
#endif

  virtual light_manager* get_light_set();

  virtual render_flavor_t render_passes_needed() const;

  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );

  void set_owner( entity *pOwner ) { owner = pOwner;}

private:
  entity *owner;
};

class morphable_item_range
{
protected:
  morphable_item_range()
  {
    low = 0;
    high = -1;
    vis_rep = stringx();
  }

  ~morphable_item_range()
  {
  }

  void copy_instance_data(morphable_item_range *b)
  {
    low = b->low;
    high = b->high;
    vis_rep = b->vis_rep;
  }

  morphable_item_range *make_instance()
  {
    morphable_item_range *r = NEW morphable_item_range();
    r->copy_instance_data(this);
    return(r);
  }

  bool in_range(int num) { return(num >= low && (num <= high || high == -1)); }

  int low;
  int high;

  stringx vis_rep;


  friend class morphable_item;
};

class morphable_item : public item
{
public:
  morphable_item( const entity_id& _id, unsigned int _flags );

  morphable_item( const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

  virtual ~morphable_item();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_morphable_item() const { return true; }

// NEWENT File I/O
public:
  morphable_item( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

  void read_morph_ranges( chunk_file& fs, stringx& label );

// Old File I/O
public:
  morphable_item( const stringx& item_type_filename,
        const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_PHYSICAL,
        bool _active = INACTIVE,
        bool _stationary = false );

public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  virtual void copy_instance_data( const morphable_item& b );

  vector<morphable_item_range *> ranges;
  int old_count;

  void set_range_visrep(int cnt);
  void dump_ranges();

public:
  virtual void frame_advance(time_value_t t);
};


#endif
