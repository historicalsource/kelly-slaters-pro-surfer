// ENTITY.H
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
#ifndef ENTITY_H
#define ENTITY_H

#include "frame_info.h"
#include "entflavor.h"
#include "algebra.h"
#include "chunkfile.h"
#include "entityid.h"
#include "po.h"
#include "ostimer.h"
#include "graph.h"

#include "visrep.h"
#include "script_object.h"
#include "instance.h"
#include "fast_vector.h"
#include "anim.h"
#include "signals.h"
#include "project.h"
#include "region_graph.h"
//#include "brain_enums.h"
#include "staticmem.h"

#ifdef TARGET_PS2
#include "ngl_ps2.h"
#endif

#if defined(TARGET_XBOX) || defined(TARGET_GC)
// JIV FIXME: messed up
#include "ngl.h"
#endif /* XBOX */
#include <map>


class entity;
class entity_manager;
class world_dynamics_system;
class terrain;
class sector;
class region;
class portal;
class collision_geometry;
class collision_capsule;
class chunk_file;
class collision_geometry;
class light_manager;
class entity_anim;
class entity_anim_tree;
class bounding_box;
class motion_blur_info;
class vm_thread;
class item;
// BIGCULL class handheld_item;
class material;
//class sound_emitter;
class entity_controller;
class entity_widget;
class entity_pool;
class entity_track_tree;
class camera;

//typedef graph<stringx,region*,portal*> region_graph;
//typedef region_graph::node region_node;

typedef vector<material*> material_list;
struct material_set
{
  stringx         *name;
  material_list   *data;
};

#ifdef DEBUG
extern int g_getpo_counter;
extern int g_buildpo_counter;
#endif

// region_node_pset used for list of regions intersected by entity
//typedef set<region_node*
//	,	less<region_node *>
//	,	malloc_alloc        //__STL_DEFAULT_ALLOCATOR(region_node *)
//	> region_node_pset;

extern vector3d global_ZEROVEC;

///////////////////////////////////////////////////////////////////////////////
// anim_id stuff
///////////////////////////////////////////////////////////////////////////////

typedef short anim_id_t;

// pre-defined anim ids
enum
{
  NO_ID = -1,
#define MAC(a) a,
#include "anim_ids.h"
#undef MAC
  NUM_PREDEFINED_ANIM_IDS
};





// support
class anim_id_manager : public singleton
{
 public:
  // returns a pointer to the single instance
  DECLARE_SINGLETON( anim_id_manager )

	static void stl_prealloc(void);
  // Types
 public:
  //typedef vector<stringx> label_list;
  typedef vector<stringx
	  #ifdef TARGET_PS2
                 ,malloc_alloc
  	#endif
                 > label_list;
  typedef map< stringx, anim_id_t > label_map_t;
  //typedef map< stringx, anim_id_t, less<stringx>, malloc_alloc > label_map_t;

  // Data
 private:
  label_list labels;
  // This is a duplication of the data in labels, so we can do fast string searches.
  label_map_t label_map;
  stringx NO_ID_label;

  // Constructors (not public in a singleton)
 private:
  anim_id_manager();
  anim_id_manager( const anim_id_manager& );
  anim_id_manager& operator=( const anim_id_manager& );

  // Methods
 public:
  anim_id_t anim_id( const char* _label );
  anim_id_t anim_id( const stringx& _label );
  anim_id_t anim_id( const char* _label, short id );
  anim_id_t anim_id( const stringx& _label, short id );
  int get_num_ids() const { return labels.size(); }
  anim_id_t find_id( const stringx& _label );

  const stringx& get_label( anim_id_t id ) const
  {
    if ( id == NO_ID )
      return NO_ID_label;
    return labels[id];
  }

  void purge();
  void stl_dealloc();
};

inline const stringx& to_string( anim_id_t id )
{
  return anim_id_manager::inst()->get_label( id );
}


///////////////////////////////////////////////////////////////////////////////
// entity_id class
///////////////////////////////////////////////////////////////////////////////

// I'm just using strings for now but for efficiency we should
// probably do something like we did in DBTS 1.
class entity_id
{
 public:
  entity_id() : val((unsigned)-1) {}   // will be equal to entity_id("unreg")
  entity_id(const char* name);
  void set_entity_id(const char* name);
  bool operator==(const entity_id& e) const
  {
    return val==e.val;
  }
  bool operator<(const entity_id& e) const {return val < e.val;}
  stringx get_val() const;

  static entity_id &make_unique_id();
  static void delete_entity_id(entity_id id);
  unsigned int get_numerical_val() const {return val;}

  static entity_id &make_entity_id(const char *foo)
  {
    static entity_id ret;
    ret.set_entity_id(foo);
    return ret;
  }

 private:
  unsigned int val;

  friend class entity_manager;
#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& io, const entity_id* eid);
#endif
  friend void serial_in(chunk_file& io, entity_id* eid);
};

#if !defined(NO_SERIAL_OUT)
void serial_out(chunk_file& io, const entity_id* eid);
#endif
void serial_in(chunk_file& io, entity_id* eid);

const entity_id ANONYMOUS;

enum
{
  FIND_ENTITY_UNKNOWN_NOT_OK = false,
  FIND_ENTITY_UNKNOWN_OK = true
};



///////////////////////////////////////////////////////////////////////////////
// entity_manager class
///////////////////////////////////////////////////////////////////////////////
typedef map< entity_id, entity*
  , less<entity_id>
	#ifdef TARGET_PS2
	, malloc_alloc
	#endif
> entity_map;


//class entity_manager : public map< entity_id, entity* >, public singleton
class entity_manager : entity_map, public singleton
{
 public:
  entity* find_entity( const entity_id& target_entity, entity_flavor_t flavor, bool unknown = FIND_ENTITY_UNKNOWN_NOT_OK );
#ifdef SPIDEY_SIM
  entity* find_nearest_entity_to_line( const vector3d& pos1, const vector3d& pos2, entity_flavor_t flavor );
#endif /* SPIDEY_SIM */

  // delivers a warning about entities that aren't in the world.
  void check_entity_sectors();
  void purge();
  void reset_name_to_number_map();

  // returns a pointer to the single instance
  DECLARE_SINGLETON(entity_manager)

	static void stl_prealloc(void);

 private:
  entity_manager();
  ~entity_manager();

  void register_entity(entity*);
  void deregister_entity(entity*);

  friend class entity;
  friend class entity_id;
  friend bool ks_fx_create_lightning (float dt, void** data, const char *name, float start, float total);

  name_to_number_map name_to_number;
  int number_server;
};


///////////////////////////////////////////////////////////////////////////////
// entity class
///////////////////////////////////////////////////////////////////////////////

extern const char* entity_flavor_names[NUM_ENTITY_FLAVORS+1];

// return entity_flavor_t corresponding to given string (NUM_ENTITY_FLAVORS if not found)
entity_flavor_t to_entity_flavor_t( const stringx& s );

enum entity_flag_t
{
  EFLAG_PHYSICS                              = 0x00000001,   // Object has physical properties, IE it has a colgeom.
  EFLAG_PHYSICS_COLLISIONS_ACTIVE            = 0x00000002,   // Object can collide with other objects.  Use this to disable physics for an object.
  EFLAG_PHYSICS_MOVING                       = 0x00000004,   // This object can move, meaning it has its own velocity, mass, etc.
  EFLAG_PHYSICS_WALKABLE                     = 0x00000008,   // Characters can walk on this object.

  EFLAG_PHYSICS_COLLIDE_ACTORS               = 0x00000010,   // This object collides with actors.
  EFLAG_PHYSICS_COLLIDE_TERRAIN              = 0x00000020,   // This object collides with terrain.
  EFLAG_PHYSICS_STICKY                       = 0x00000040,   // I have no clue.  Ask Pete.
  EFLAG_INVISIBLE_PHYSICS                    = 0x00000080,   // Do only partial physics if character is invisible.

  EFLAG_GRAPHICS                             = 0x00000100,   // Object has graphical properties - it has a visrep.
  EFLAG_GRAPHICS_VISIBLE                     = 0x00000200,   // Object should be drawn.
  EFLAG_GRAPHICS_MOTION_BLUR                 = 0x00000400,   // Object should have a motion blur drawn
  EFLAG_GRAPHICS_MOTION_TRAIL                = 0x00000800,	// Object should have a motion trail drawn

  EFLAG_MISC_REPULSION                       = 0x00001000,   // Non-player characters will avoid this object.
  EFLAG_MISC_SCRIPTED                        = 0x00002000,   // This object has a script.
  EFLAG_MISC_IN_VISIBLE_REGION               = 0x00004000,   // Each frame during _build_render_data this gets appropriately set
  EFLAG_MISC_COMP_SECT_THIS_FRAME            = 0x00008000,	// Set by compute_sector to let peopl know it was done.

  EFLAG_MISC_IN_USE                          = 0x00010000,	// used to be bool in physent
  EFLAG_ACTIVE			                     = 0x00020000,	// determines whether the entity may frame_advance this turn
  EFLAG_MISC_MEMBER_OF_VARIANT               = 0x00040000, // papa was a rolling stone
  EFLAG_MISC_CAST_SHADOW                     = 0x00080000,   // casts shadow

  EFLAG_PHYSENT_EXTERNALLY_CONTROLLED        = 0x00100000,   // For physents
  EFLAG_PHYSENT_ANGLE_EXTERNALLY_CONTROLLED  = 0x00200000,   // For physents
  EFLAG_PHYSENT_MOTION_EXTERNALLY_CONTROLLED = 0x00400000,   // For physents
  EFLAG_MISC_RAW_NONSTATIC                   = 0x00800000,                // Holds the NONSTATIC status from the .SCN /.SIN file

  EFLAG_MISC_NO_CLIP_NEEDED                  = 0x01000000,	// set by build render data to exclude far ents from clipping.
  EFLAG_MISC_NO_LOD                          = 0x02000000,	// disable LOD calculations.
  EFLAG_GRAPHICS_NO_DISTANCE_CLIP            = 0x04000000,  // If the object is in a visible region, draw it regardless of distance from the camera.
  EFLAG_MISC_NONSTATIC                       = 0x08000000, // if false, entities with this flag will be created on the static heap.
  //    EFLAG_GRAPHICS_FORCE_LIGHT                 = 0x02000000, // object should be lit rather than relying on vertex color  :  this is no longer handled by entity
  //    flags but in the entity's visrep

  EFLAG_REGION_FORCED                        = 0x10000000, // replaced the region_forced bool.  means the entity is forced to a region.
  EFLAG_COLGEOM_INSTANCED                    = 0x20000000, // replaced the colgeom_instanced bool.  means the entity's colgeom is shared with other entities (in an instanced bank).
  EFLAG_MISC_NOKILLME                        = 0x40000000, // if true, then cell death doesn't cause a delete, used in pooled particle emitters
  EFLAG_MISC_REUSEME                         = 0x80000000, // if true, ready for pool re-use

  // this is a mask that specifies which of the above flags get copied on a make_instance()
  EFLAG_COPY_MASK = EFLAG_GRAPHICS|EFLAG_GRAPHICS_VISIBLE|EFLAG_MISC_CAST_SHADOW|EFLAG_COLGEOM_INSTANCED,
};

enum entity_ext_flag_t
{
  EFLAG_EXT_TARGETABLE                       = 0x00000001,
  EFLAG_EXT_TARGET_TYPE                      = 0x00000002, // true = mechanical false = bio (default)
  EFLAG_EXT_ACTIONABLE                       = 0x00000004,
  EFLAG_EXT_ACTION_USES_FACING               = 0x00000008,

  EFLAG_EXT_SCANABLE                         = 0x00000010, // true if scanners should paint this entity
  EFLAG_EXT_BEAMABLE                         = 0x00000020, // true if beams should hit this entity
  EFLAG_EXT_IS_DOOR                          = 0x00000040, // entity is a door
  EFLAG_EXT_DOOR_CLOSED                      = 0x00000080, // door is closed (IS_DOOR must be set to use)

  EFLAG_EXT_SPAWNED                          = 0x00000100, // Object was spawned from the console (ie, no item pickup, etc.)
  EFLAG_EXT_SIN                              = 0x00000200, // Object was created from SIN file ('0' means SCN file)
  EFLAG_EXT_INSTANCE                         = 0x00000400, // Object was created from a make instance (i.e. clone)
  EFLAG_EXT_MEMBER                           = 0x00000800, // Object is member of a conglomerate

  EFLAG_EXT_NEEDS_EXPORT                     = 0x00001000, // Object altered and needs export (mainly for SCN entities)
  EFLAG_EXT_INVULNERABLE                     = 0x00002000, // Object is invulnerable
  EFLAG_EXT_CRAWL_PINPOINT                   = 0x00004000, // Use crawl markers position as crawl reference
  EFLAG_EXT_GRAVITY                          = 0x00008000, // Enable gravity

  EFLAG_EXT_TIME_LIMITED                     = 0x00010000, // Time limited
  EFLAG_EXT_NONTARGET                        = 0x00020000, // disallow combat targeting
  EFLAG_EXT_WAS_VISIBLE                      = 0x00040000, // needs to set visible on actor::set_visible(true) call
  EFLAG_EXT_PRELOADED                        = 0x00080000, // entity has performed it's preload() call.

  EFLAG_EXT_ENX_WALKABLE                     = 0x00100000, // entity is walkable in the enx
  EFLAG_EXT_AI_INVISIBLE                     = 0x00200000, // entity is invisible to AI's
  EFLAG_EXT_UNUSED                           = 0x00400000, // entity is currently unused (in the entity_maker caching system)

  EFLAG_EXT_NONCRAWLABLE                     = 0x00800000, // Cannot be crawled on by spiderman
  EFLAG_EXT_AI_COVER                         = 0x01000000, // Can be used as cover for AI
  EFLAG_EXT_DONT_OPTIMIZE                    = 0x02000000, // Don't optimize verts
  EFLAG_EXT_SCENE_ANIM                       = 0x04000000, // Scene anim is playing on this ent
  EFLAG_EXT_SMALLCRAWL                       = 0x08000000, // Spiderman uses tight crawl

  EFLAG_EXT_AI_LOS_SEE_THROUGH               = 0x10000000, // AI sight goes through it (default off)
  EFLAG_EXT_NEEDS_COMPUTE_SECTOR             = 0x20000000, // compute sector needed (i.e. moved)

  EFLAG_EXT_MUSTCRAWL                        = 0x40000000, // must be crawling

  EFLAG_EXT_COPY_MASK                        = 0xFFFFFFFF & ~(EFLAG_EXT_PRELOADED | EFLAG_EXT_MEMBER | EFLAG_EXT_WAS_VISIBLE | EFLAG_EXT_INSTANCE | EFLAG_EXT_SIN | EFLAG_EXT_SPAWNED)
};


enum
{
  TARGET_TYPE_BIO = false,
  TARGET_TYPE_MECHANICAL = true
};


enum
{
  SKIP_DELETE_STREAM = 0,
  DELETE_STREAM = 1
};

class destroyable_info;


enum
{
  ANIM_PRIMARY_A = 0,
  ANIM_PRIMARY_B,
  ANIM_SECONDARY_A,
  ANIM_SECONDARY_B,
  ANIM_SECONDARY_C,
  ANIM_TERTIARY_A,
  ANIM_TERTIARY_B,
  ANIM_TERTIARY_C,
  ANIM_LIPSYNC_A,		// animation channel dedicated to Lipsync anims
  ANIM_SCENE,
  MAX_ANIM_SLOTS
};
#define ANIM_PRIMARY ANIM_PRIMARY_A


/***************************************************************************************************/
/* T H E   N E W                                                                                   */
/***************************************************************************************************/


#include "bone.h"

class ai_interface;
class animation_interface;
// BIGCULL class damage_interface;
class hard_attrib_interface;
class owner_interface;
class physical_interface;
class render_interface;
class script_data_interface;
class skeleton_interface;
class slave_interface;
class soft_attrib_interface;
//class sound_interface;
class time_interface;
class box_trigger_interface;

class entity : public bone
{
  public:
  stringx fileName;

  protected:
    unsigned int flags;
    entity_flavor_t flavor;
    entity_id id;
    stringx parsedName;
    entity_anim * my_animation;
    rational_t radius;


	int MaterialMask;   //  this is used to mask off materials
	int TextureFrame;   //  this is used to lock texture frames of ifl's
	bool cull_entity;		//	this flag will tell NGL to cull this entity

  bool use_uv_scrolling;
  float scroll_u;
  float scroll_v;

  /*** Interfaces ***/
   ENTITY_INTERFACE(ai)
   ENTITY_INTERFACE(animation)
// BIGCULL    ENTITY_INTERFACE(damage)
   ENTITY_INTERFACE(hard_attrib)
   ENTITY_INTERFACE(owner)
   ENTITY_INTERFACE(physical)
   ENTITY_INTERFACE(render)
#ifdef ECULL
   ENTITY_INTERFACE(script_data)
#endif
   ENTITY_INTERFACE(skeleton)
   ENTITY_INTERFACE(slave)
   ENTITY_INTERFACE(soft_attrib)
#ifdef ECULL
   ENTITY_INTERFACE(sound)
#endif
   ENTITY_INTERFACE(time)
#ifdef ECULL
   ENTITY_INTERFACE(box_trigger)
#endif

  /*** Interface support ***/
  public:
    virtual bool get_ifc_num(const pstring &att, rational_t &val);
    virtual bool set_ifc_num(const pstring &att, rational_t val);
    virtual bool get_ifc_vec(const pstring &att, vector3d &val);
    virtual bool set_ifc_vec(const pstring &att, const vector3d &val);
    virtual bool get_ifc_str(const pstring &att, stringx &val);
    virtual bool set_ifc_str(const pstring &att, const stringx &val);

  public:
    /*** Virtual Interfaces =) ***/
    virtual void frame_advance( time_value_t t );
    virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );
    virtual void rendershadow( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct, rational_t scale );

		void updatelighting( time_value_t t, const int playerID );

    /*** Entity Methods ***/
    entity(void *fixup_offset); // static loading constructor
    void set_parsed_name(stringx _name) {parsedName = _name; parsedName.to_lower(); };
    stringx get_parsed_name() {return parsedName;};
	void SetMaterialMask(int mask) { MaterialMask |= mask; }
	int GetMaterialMask(void) const { return MaterialMask; }
	void SetTextureFrame(int frame) { TextureFrame = frame; }
	int GetTextureFrame(void) const { return TextureFrame; }
  void set_texture_scroll (float u, float v) { use_uv_scrolling = true; scroll_u = u; scroll_v = v; };
  void SetCull(bool cull) { cull_entity = cull; }


/***************************************************************************************************/
/* T H E   O L D                                                                                   */
/***************************************************************************************************/
protected:
//  entity_flavor_t flavor;
//  po              my_po;    // THIS LUCKILY IS 8-BYTE ALIGNED.  KEEP IT THAT WAY

// Constructors
public:
  entity( const entity_id& _id, unsigned int _flags );

  // The last_po_init_val is simply a way of telling the constructor to create a last_po
  // I am doing it this way to avoid the potential ambiguity of a bool third parameter
  // with regard to confusion with the other constructors here.
  entity( const entity_id& _id, unsigned int _flags, const po & last_po_init_val );

  entity( const entity_id& _id = ANONYMOUS,
          entity_flavor_t _flavor = ENTITY_ENTITY,
          unsigned int _flags = 0 );

  entity( const entity_id& _id,
          entity_flavor_t _flavor,
          anim_id_t _anim_id );

  entity( const entity_id& _id,
          entity_flavor_t _flavor,
          const char* _anim_id );

  virtual ~entity();
  void destruct();

	// This function allows the entity to any post-level-load initialization
	virtual void initialize();

private:
  void _construct( const entity_id& _id,
                   entity_flavor_t _flavor,
                   anim_id_t _anim_id,
                   unsigned int _flags = 0,
                   const po& _my_po = po_identity_matrix);

// NEWENT File I/O
public:
  entity( chunk_file& fs,
          const entity_id& _id,
          entity_flavor_t _flavor = ENTITY_ENTITY,
          unsigned int _flags = 0 );

  virtual void read_enx( chunk_file& fs );
//  virtual void read_enx( chunk_file& fs, stringx& lstr );

  virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );

  // This function allows parsing instance data according to entity type.
  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );

// Old File I/O
public:
  entity( const stringx& entity_fname,
          const entity_id& _id = ANONYMOUS,
          entity_flavor_t _flavor = ENTITY_ENTITY,
          bool delete_stream = DELETE_STREAM);

// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const entity& b );

/////////////////////////////////////////////////////////////////////////////
// Interfaces
//
//   The entity class has a very large interface, most of it unimplemented.  Any class that derives
//   from entity can implement the parts it wants to and set flags specifying that it implements those
//   parts.
//
//   The entity class itself implements a small subset that are required to be implemented by everyone,
//   mostly for speed.
//
//   Each interface function is grouped according to what flags must be set in the entity before the
//   function becomes valid to call.  That means that before calling a non ALWAYS AVAILABLE function
//   you must check the flags.
/////////////////////////////////////////////////////////////////////////////

public:
  bool is_flagged(register unsigned int f) const { return flags & f; }
  void set_flag(register unsigned int f, register bool set) { if(set) flags|=f; else flags&=~f; }
  unsigned int get_flags() const { return flags; }
  void set_flags( unsigned int f ) { flags = f; }

  bool is_ext_flagged(register unsigned int f) const { return ext_flags & f; }
  void set_ext_flag(register unsigned int f, register bool set) { if(set) ext_flags|=f; else ext_flags&=~f; }
  unsigned int get_ext_flags() const { return ext_flags; }
  void set_ext_flags( unsigned int f ) { ext_flags = f; }

  virtual void set_ext_flag_recursive(register unsigned int f, register bool set) { set_ext_flag(f, set); }

  void copy_flags( const entity& b );

/////////////////////////////////////////////////////////////////////////////
// Created entities (as opposed to those loaded from the .SCN file
/////////////////////////////////////////////////////////////////////////////
public:
  virtual void set_created_entity_default_active_status();

/////////////////////////////////////////////////////////////////////////////
// Optimization of entities (after everything is loaded and initialized)
/////////////////////////////////////////////////////////////////////////////
public:
  virtual void optimize();

/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////
public:
  // enum of local signal ids (for coding convenience and readability)
  enum signal_id_t
  {
    // a descendant class uses the following line to append its local signal ids after the parent's
    PARENT_SYNC_DUMMY = signaller::N_SIGNALS - 1,
#define MAC(label,str)  label,
    // replace "entity" with whatever is appropriate
#include "entity_signals.h"
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

  void clear_all_raised_signals();
  void clear_signal_raised(unsigned short sig_id);
  bool signal_raised(unsigned short sig_id);

  void clear_signal_raised(const char *name)        { clear_signal_raised(get_signal_id(name)); }
  bool signal_raised(const char *name)              { return(signal_raised(get_signal_id(name))); }

private:
  // Every descendant of signaller that expects to generate signals and has
  // defined its own local list of signal ids should implement this virtual
  // function for the construction of the signal list, so that it will reserve
  // exactly the number of signal pointers required, on demand.
  virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

  friend void entity_signal_callback_raiser(signaller* sig, const char* sig_id);
  unsigned int signals_raised[2];

protected:
  // This virtual function, used only for debugging purposes, returns the
  // name of the given local signal
  virtual const char* get_signal_name( unsigned short idx ) const;

/////////////////////////////////////////////////////////////////////////////
// Script interface
/////////////////////////////////////////////////////////////////////////////
public:
  // spawn the named global script function with this entity as a parameter;
  // the function must match the prototype:  foo(entity e);
  vm_thread* spawn_entity_script_function( const stringx& function_name ) const;
  static void exec_preload_function(const stringx &preload_func);

/////////////////////////////////////////////////////////////////////////////
// Movement histry interface
/////////////////////////////////////////////////////////////////////////////
public:
  struct movement_info
  {
    // Walkable stuff
    bool frame_delta_valid;
    bool last_frame_delta_valid;
    po frame_delta;
    time_value_t frame_time;
    movement_info()
    {
      frame_delta_valid=false;
      last_frame_delta_valid=false;
    }

    STATICALLOCCLASSHEADER
  };

/////////////////////////////////////////////////////////////////////////////
// Physics interface
/////////////////////////////////////////////////////////////////////////////
public:
  enum
  {
    INACTIVE = 0,
    ACTIVE = 1
  };

  enum
  {
    COLLIDE_WITH_ACTORS=1,
    COLLIDE_WITH_TERRAIN=2
  };


  // ALWAYS AVAILABLE
  virtual void set_radius( rational_t r )           { radius = r; }
  virtual rational_t get_radius() const             { return radius; }

//  const po& get_po() const;
/*  inline const po& get_po_inline() const
  {
    if ( pi && pi->parent )
      return pi->my_abs_po;
    else
      return my_po;
  }


  void update_abs_po();
  void update_abs_po_including_limbs();

  const vector3d& get_abs_position() const
  {
    return get_po().get_abs_position();
  }

  inline const vector3d& get_position_inline() const
  {
    return get_po_inline().get_abs_position();
  }


  const po& get_rel_po() const { return my_po; }
  void set_rel_po(const po & the_po) { my_po = the_po; po_changed(); }
  const vector3d& get_rel_position() const { return my_po.get_abs_position(); }

  void set_rel_position(const vector3d& p);
  void set_abs_position(const vector3d &p);

  void set_rel_orientation(const orientation& o);
  void set_rel_orientation(const po& p);

  void set_rel_po(const po & the_po);

*/



  void get_direction(vector3d* target) const;

  // set entity orientation such that it is facing the given world-coordinate point
  void look_at( const vector3d& abs_pos );

  short get_bone_idx() const           { return bone_idx; }
  void set_bone_idx( short _bone_idx ) { bone_idx = _bone_idx; }

  void set_preloaded( bool s ) { if(s) ext_flags |= EFLAG_EXT_PRELOADED; else ext_flags &= ~EFLAG_EXT_PRELOADED; }
  bool was_preloaded() const { return((ext_flags & EFLAG_EXT_PRELOADED)); }

  void set_invulnerable( bool s ) { if(s) ext_flags |= EFLAG_EXT_INVULNERABLE; else ext_flags &= ~EFLAG_EXT_INVULNERABLE; }
  bool is_invulnerable() const { return((ext_flags & EFLAG_EXT_INVULNERABLE)); }

  bool is_auto_targetable() const { return((ext_flags & EFLAG_EXT_TARGETABLE) && !is_invulnerable()); }
  void set_auto_targetable(bool s) { if(s) ext_flags |= EFLAG_EXT_TARGETABLE; else ext_flags &=~ EFLAG_EXT_TARGETABLE; }

  bool is_combat_target() const   { return(!(ext_flags & EFLAG_EXT_NONTARGET)); }
  void set_combat_target(bool s)  { if(!s) ext_flags |= EFLAG_EXT_NONTARGET; else ext_flags &= ~EFLAG_EXT_NONTARGET; }

  bool is_visible_to_AI() const           { return(!(ext_flags & EFLAG_EXT_AI_INVISIBLE)); }
  void set_visible_to_AI(bool v)          { if(!v) ext_flags |= EFLAG_EXT_AI_INVISIBLE; else ext_flags &= ~EFLAG_EXT_AI_INVISIBLE; }

  bool is_crawlable() const               { return(!(ext_flags & EFLAG_EXT_NONCRAWLABLE)); }
  void set_crawlable( bool a )            { if(!a) ext_flags|=EFLAG_EXT_NONCRAWLABLE; else ext_flags&=~EFLAG_EXT_NONCRAWLABLE; }

  bool is_smallcrawl() const               { return(ext_flags & EFLAG_EXT_SMALLCRAWL); }
  void set_smallcrawl( bool a )            { if(a) ext_flags|=EFLAG_EXT_SMALLCRAWL; else ext_flags&=~EFLAG_EXT_SMALLCRAWL; }

  bool is_mustcrawl() const               { return(ext_flags & EFLAG_EXT_MUSTCRAWL); }
  void set_mustcrawl( bool a )            { if(a) ext_flags|=EFLAG_EXT_MUSTCRAWL; else ext_flags&=~EFLAG_EXT_MUSTCRAWL; }

  bool is_ai_cover() const               { return(ext_flags & EFLAG_EXT_AI_COVER); }
  void set_ai_cover( bool a )            { if(a) ext_flags|=EFLAG_EXT_AI_COVER; else ext_flags&=~EFLAG_EXT_AI_COVER; }

  bool is_ai_los_block() const               { return(!(ext_flags & EFLAG_EXT_AI_LOS_SEE_THROUGH)); }
  void set_ai_los_block( bool a )            { if(!a) ext_flags|=EFLAG_EXT_AI_LOS_SEE_THROUGH; else ext_flags&=~EFLAG_EXT_AI_LOS_SEE_THROUGH; }

  bool needs_compute_sector() const           { return(ext_flags & EFLAG_EXT_NEEDS_COMPUTE_SECTOR); }
  void set_needs_compute_sector( bool a )     { if(a) ext_flags|=EFLAG_EXT_NEEDS_COMPUTE_SECTOR; else ext_flags&=~EFLAG_EXT_NEEDS_COMPUTE_SECTOR; }

  bool playing_scene_anim() const               { return(ext_flags & EFLAG_EXT_SCENE_ANIM); }
  void set_playing_scene_anim( bool a )         { if(a) ext_flags|=EFLAG_EXT_SCENE_ANIM; else ext_flags&=~EFLAG_EXT_SCENE_ANIM; }

  // currently only support two types (bio and mechanical). easy to expand though. just don't use the flag, and add an enum var.
  int get_target_type() const { return((ext_flags & EFLAG_EXT_TARGET_TYPE) != 0); }
  void set_target_type(int s) { if(s) ext_flags |= EFLAG_EXT_TARGET_TYPE; else ext_flags &=~ EFLAG_EXT_TARGET_TYPE; }

  bool is_actionable() const { return((ext_flags & EFLAG_EXT_ACTIONABLE)); }
  void set_actionable(bool s) { if(s) ext_flags |= EFLAG_EXT_ACTIONABLE; else ext_flags &=~ EFLAG_EXT_ACTIONABLE; }

  bool action_uses_facing() const { return((ext_flags & EFLAG_EXT_ACTION_USES_FACING)); }
  void set_action_uses_facing(bool s) { if(s) ext_flags |= EFLAG_EXT_ACTION_USES_FACING; else ext_flags &=~ EFLAG_EXT_ACTION_USES_FACING; }

  bool is_scannable() const { return((ext_flags & EFLAG_EXT_SCANABLE)); }
  void set_scannable(bool s) { if(s) ext_flags |= EFLAG_EXT_SCANABLE; else ext_flags &=~ EFLAG_EXT_SCANABLE; }

  bool is_beamable() const { return((ext_flags & EFLAG_EXT_BEAMABLE)); }
  void set_beamable(bool s) { if(s) ext_flags |= EFLAG_EXT_BEAMABLE; else ext_flags &=~ EFLAG_EXT_BEAMABLE; }

  bool was_spawned() const            { return (ext_flags & EFLAG_EXT_SPAWNED); }
  void set_spawned( bool a )          { { if(a) ext_flags|=EFLAG_EXT_SPAWNED; else ext_flags&=~EFLAG_EXT_SPAWNED; } }

  bool from_sin_file() const            { return (ext_flags & EFLAG_EXT_SIN); }
  void set_from_sin_file( bool a )          { { if(a) ext_flags|=EFLAG_EXT_SIN; else ext_flags&=~EFLAG_EXT_SIN; } }

  bool needs_export() const            { return (ext_flags & EFLAG_EXT_NEEDS_EXPORT); }
  void set_needs_export( bool a )          { { if(a) ext_flags|=EFLAG_EXT_NEEDS_EXPORT; else ext_flags&=~EFLAG_EXT_NEEDS_EXPORT; } }

  bool is_a_clone() const            { return (ext_flags & EFLAG_EXT_INSTANCE); }
  void set_clone( bool a )          { { if(a) ext_flags|=EFLAG_EXT_INSTANCE; else ext_flags&=~EFLAG_EXT_INSTANCE; } }

  bool is_conglom_member() const            { return (ext_flags & EFLAG_EXT_MEMBER); }

  bool is_door() const { return((ext_flags & EFLAG_EXT_IS_DOOR)); }
  void set_door( bool d );

  bool is_door_closed() const { return((ext_flags & EFLAG_EXT_DOOR_CLOSED)); }
  void set_door_closed( bool d );

  // Compatibility flags stuff
/*
  virtual bool is_physical() const            { return  flags & EFLAG_PHYSICS; }
  virtual void set_physical( bool p )         { { if(p) flags|=EFLAG_PHYSICS; else flags&=~EFLAG_PHYSICS; set_gravity(p); } }

  virtual bool is_gravity() const             { return is_ext_flagged(EFLAG_EXT_GRAVITY);}
  virtual void set_gravity( bool torf )       { if(torf) ext_flags |= EFLAG_EXT_GRAVITY; else ext_flags &= ~EFLAG_EXT_GRAVITY; }
*/

  virtual bool is_time_limited() const             { return is_ext_flagged(EFLAG_EXT_TIME_LIMITED);}
  virtual void set_time_limited( bool torf )       { assert(torf || !is_time_limited()); if(torf) ext_flags |= EFLAG_EXT_TIME_LIMITED; else ext_flags &= ~EFLAG_EXT_TIME_LIMITED; }

  virtual bool is_active() const              { return flags & EFLAG_ACTIVE; }
  virtual void set_active( bool a );

  enum force_active_t
  {
    FORCE_ACTIVE_NONE = 0,
    FORCE_ACTIVE_FALSE = -1,
    FORCE_ACTIVE_TRUE = 1
  };

  virtual force_active_t get_forced_active() const { return FORCE_ACTIVE_NONE; }

  virtual bool are_collisions_active() const      { return flags & EFLAG_PHYSICS_COLLISIONS_ACTIVE; }
  virtual void set_collisions_active( bool a, bool update_reg = true );

  virtual bool is_sticky() const              { return flags & EFLAG_PHYSICS_STICKY; }
  virtual void set_sticky( bool s )           { { if(s) flags|=EFLAG_PHYSICS_STICKY; else flags&=~EFLAG_PHYSICS_STICKY; } }

  virtual bool is_stationary() const          { return !(flags & EFLAG_PHYSICS_MOVING); }
  virtual void set_stationary( bool s )       { { if(!s) flags|=EFLAG_PHYSICS_MOVING; else flags&=~EFLAG_PHYSICS_MOVING; } }

  virtual bool is_walkable() const            { return flags & EFLAG_PHYSICS_WALKABLE; }
  virtual void set_walkable( bool s )         { { if(s) flags|=EFLAG_PHYSICS_WALKABLE; else flags&=~EFLAG_PHYSICS_WALKABLE; } }

  virtual bool is_repulsion() const           { return flags & EFLAG_MISC_REPULSION; }
  virtual void set_repulsion( bool s )        { { if(s) flags|=EFLAG_MISC_REPULSION; else flags&=~EFLAG_MISC_REPULSION; } }

  virtual bool is_visible() const             { return flags & EFLAG_GRAPHICS_VISIBLE; }
  virtual void set_visible( bool a );

  unsigned get_max_lights() const             { return max_lights; }
  void set_max_lights(unsigned ml);

  enum
  {
    FAMILYVIS_CUR_VARIANT_ONLY=1
  };
  // set me and all my children's visibility
  void set_family_visible( bool a, bool current_variant_only=false );

  // needed by particle_generator and, thus, conglomerate also
  virtual bool is_still_visible() const { return is_visible(); }

  virtual bool is_motion_blurred() const      { return flags & EFLAG_GRAPHICS_MOTION_BLUR; }

  void         allocate_motion_info();
  virtual void activate_motion_blur(  int _blur_min_alpha,
                                      int _blur_max_alpha,
                                      int _num_blur_images,
                                      float _blur_spread );
  virtual void deactivate_motion_blur();

  virtual bool is_motion_trailed() const      { return flags & EFLAG_GRAPHICS_MOTION_TRAIL; }
  virtual void activate_motion_trail( int _trail_length,
                                      color32 _trail_color,
                                      int _trail_min_alpha,
                                      int _trail_max_alpha,
                                      const vector3d& tip
                                      );
  virtual void deactivate_motion_trail();

  virtual bool get_externally_controlled() const
    { return flags & EFLAG_PHYSENT_EXTERNALLY_CONTROLLED; }
  void set_externally_controlled( bool ec )
    { { if(ec) flags|=EFLAG_PHYSENT_EXTERNALLY_CONTROLLED; else flags&=~EFLAG_PHYSENT_EXTERNALLY_CONTROLLED; } }

  bool get_angle_externally_controlled() const
    { return flags & EFLAG_PHYSENT_ANGLE_EXTERNALLY_CONTROLLED; }
  void set_angle_externally_controlled( bool ec )
    { { if(ec) flags|=EFLAG_PHYSENT_ANGLE_EXTERNALLY_CONTROLLED; else flags&=~EFLAG_PHYSENT_ANGLE_EXTERNALLY_CONTROLLED; } }

  bool get_motion_externally_controlled() const
    { return flags & EFLAG_PHYSENT_MOTION_EXTERNALLY_CONTROLLED; }
  void set_motion_externally_controlled( bool ec )
    { { if(ec) flags|=EFLAG_PHYSENT_MOTION_EXTERNALLY_CONTROLLED; else flags&=~EFLAG_PHYSENT_MOTION_EXTERNALLY_CONTROLLED; } }

  void check_nonstatic();

  bool foreign_controller_active() const { return flags & (EFLAG_PHYSENT_EXTERNALLY_CONTROLLED|EFLAG_PHYSENT_ANGLE_EXTERNALLY_CONTROLLED|EFLAG_PHYSENT_MOTION_EXTERNALLY_CONTROLLED); }

  virtual bool get_in_use() const { return flags & EFLAG_MISC_IN_USE; }
  virtual void set_in_use(bool b) { { if(b) flags|=EFLAG_MISC_IN_USE; else flags&=~EFLAG_MISC_IN_USE; } }

  // EFLAG_PHYSICS
  virtual collision_geometry * get_colgeom() const    { return colgeom; }

  bool has_entity_collision() const;
  bool has_camera_collision() const;

  // uses replacement_po instead of the usual po if it is non-NULL
  virtual void update_colgeom(po * replacement_po = NULL);
  virtual void invalidate_colgeom();
  virtual collision_geometry* get_updated_colgeom(po * replacement_po = NULL, rational_t radius_scale=1);
  void delete_colgeom();
  void delete_visrep();

  // special function support for entity-entity collision algorithm
  void set_colgeom(collision_geometry * const _colgeom) { colgeom = _colgeom; }

  // This gets a special collsion capsule for purposes of being damaged.
  virtual collision_capsule* get_damage_capsule()           { assert(false); return 0; }
  virtual collision_capsule* get_updated_damage_capsule()   { assert(false); return 0; }
  virtual rational_t get_inter_capsule_radius_scale() {return 1.0f;}


  // EFLAG_PHYSICS | EFLAG_PHYSICS_MOVING
  virtual void get_velocity(vector3d* target) const;                            // IMPLEMENTED
  virtual void get_angular_velocity(vector3d* target) const;                    // IMPLEMENTED

/*
  virtual rational_t get_mass() const                               { assert(false); return 0; }
  virtual void set_mass( rational_t m )                             { assert(false); }

  virtual rational_t get_volume() const                             { assert(false); return 0; }

  virtual void get_c_o_m(vector3d* target) const                		{ *target = ZEROVEC;}
  virtual void set_c_o_m( const vector3d& c )                       { assert(false); }

  virtual void set_volume( rational_t v )                           { assert(false); }

  virtual void set_I_vector( const vector3d& I )                    { assert(false); }

  virtual void set_velocity( const vector3d& v )                    { assert(false); }
  virtual void set_angular_velocity( const vector3d& v )            { assert(false); }

  // This is for assessing impact damage (as in falling)
  virtual void set_velocity_with_impact( const vector3d& v )        { assert(false); }
  virtual void set_velocity_without_impact( const vector3d& v )     { assert(false); }
*/

  virtual rational_t get_water_dist() const { return 1000; }
  virtual rational_t get_underwater_pct() const { return 0; }
  virtual const vector3d& get_water_normal() const { return ZEROVEC; }
  virtual time_value_t get_underwater_time() const { return 0; }

  // Stuff for tracking movment of stuff that moves apart from by physics
  vector3d get_frame_abs_delta_position( bool first_time = true, const vector3d& rel_delta_pos = ZEROVEC ) const;
  virtual vector3d get_last_position() const;

    // for trailing collision_capsules
  const po& get_last_po();
  void set_last_po( const po& the_po );

//  virtual bool using_velocity() const{return false;}
//  virtual void update_unused_velocity(time_value_t increment){} // this is correct.  Entities always compute this if asked.
  //    virtual void save_current_position(void)                          { assert(false); }

  virtual void get_effective_collision_velocity( vector3d* target, const vector3d& loc ) const  { *target=ZEROVEC; assert(false); }
  virtual rational_t get_effective_collision_mass( const vector3d& loc, const vector3d& dir ) const { assert(false); return 0; }

  virtual void get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const;
  vector3d get_updated_closest_point_along_dir( const vector3d& axis );

//  virtual unsigned int get_collision_flags()        { assert(false); return 0; }
//  virtual void set_collision_flags(unsigned int cf) { assert(false); }

//  virtual bool is_on_the_ground()const        { assert(false); return false; }
//  virtual void set_on_the_ground( bool s )    { assert(false); }

//  virtual bool get_collided_last_frame()          { assert(false); return false; }
//  virtual void set_collided_last_frame( bool c )  {}

  virtual bool is_picked_up() { assert(false); return false; }

  // Note:  loc has no effect on physical_entity version of this function,
  // which assumes the force is applied to the center of mass.
  // Derived classes such as rigid_body can and do use this information
  // to accurately apply the force to a location other than the c_o_m.
  // Use of IGNORE_LOC dummy value for loc (instead of a separate flag)
  // yielded the most natural syntax for the caller.
  //virtual void apply_force_increment( const vector3d& f, force_type ft, const vector3d& loc = IGNORE_LOC, int mods=0  ) { assert(false); }
//  virtual const vector3d& get_acceleration_factor() const { assert(0);/*returning local refernce!*/ return global_ZEROVEC; }
//  virtual void set_acceleration_factor( const vector3d& v )	{ assert(0); }
//  virtual void add_acceleration_factor( const vector3d& v )	{ assert(0); }

  virtual void phys_render( time_value_t t=0.0f, bool shadow = false ){} // this is proper.  Meaningful for actors
//  virtual rational_t compute_energy() { assert(false); return 0; }

  void change_visrep( const stringx& new_visrep_name );

/////////////////////////////////////////////////////////////////////////////
// convenience for any algorithm that wants to avoid re-visiting entities
/////////////////////////////////////////////////////////////////////////////
protected:
  static unsigned int visit_key;
  unsigned int visited;
public:
  // this function prepares for a NEW visitation sequence
  static void prepare_for_visiting() { visit_key++; }
  void visit() { visited = visit_key; }
  void unvisit() { visited = visit_key-1; }
  bool already_visited() const { return (visited == visit_key); }

protected:
  static unsigned int visit_key2;
  unsigned int visited2;
public:
  // this function prepares for a NEW visitation sequence
  static void prepare_for_visiting2() { visit_key2++; }
  void visit2() { visited2 = visit_key2; }
  void unvisit2() { visited2 = visit_key2-1; }
  bool already_visited2() const { return (visited2 == visit_key2); }

#ifdef ECULL
  sound_emitter* get_emitter();
#endif

/////////////////////////////////////////////////////////////////////////////
// Graphics interface
/////////////////////////////////////////////////////////////////////////////
public:
  // EFLAG_MISC_GRAPHICS
  virtual time_value_t get_visrep_ending_time() const { assert(my_visrep); return my_visrep->get_ending_time(); }

  virtual vector3d get_visual_center() const;
  virtual rational_t get_visual_radius() const;
  virtual rational_t get_visual_xz_radius_rel_center() const { return vis_xz_rad_rel_center; }
  rational_t get_visual_max_x_extent_rel_center() const { return get_visual_center().x + get_visual_xz_radius_rel_center();}
  rational_t get_visual_min_x_extent_rel_center() const { return get_visual_center().x - get_visual_xz_radius_rel_center();}
  rational_t get_visual_max_z_extent_rel_center() const { return get_visual_center().z + get_visual_xz_radius_rel_center();}
  rational_t get_visual_min_z_extent_rel_center() const { return get_visual_center().z - get_visual_xz_radius_rel_center();}

  bool is_statically_sortable()
  {
    return get_flavor()==ENTITY_CONGLOMERATE ||
      (get_flavor()==ENTITY_ENTITY && !is_flagged(EFLAG_MISC_NONSTATIC));
  }

  bool is_statically_visually_sortable();

  void compute_visual_xz_radius_rel_center();

//  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );

  void render_heart( rational_t detail, render_flavor_t flavor, light_manager* light_set, unsigned force_flags, rational_t entity_translucency_pct );

  virtual visual_rep *get_vrep() const { return my_visrep; }
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
	void load_lores_mesh( const char *name );
	void load_shadow_mesh( const char *name );
	void set_force_hi_res(bool fhr) { force_hi_res = fhr; }
	bool get_force_hi_res() { return force_hi_res; }

  virtual nglMesh *get_mesh() const { return my_mesh; }
  virtual void     set_mesh(nglMesh *m)  { my_mesh = m; }
  virtual nglMesh *get_lores_mesh() const { return lores_mesh; }
  virtual void     set_lores_mesh(nglMesh *m)  { lores_mesh=m; }
  virtual nglMesh *get_shadow_mesh() const { return shadow_mesh; }
	void set_mesh_texture( nglTexture *tex );
	void set_mesh_distance( nglVector &Center, float Radius, float forcedist=-1.0f );
	u_int get_mesh_flags( void );
	void set_mesh_flags( u_int flags );
	u_int get_mesh_matflags( void );
	void set_mesh_matflags( u_int flags );
	void set_mesh_matflagbits( u_int flags );
	void clear_mesh_matflagbits( u_int flags );

	void set_zbias( int newz );

#ifndef TARGET_XBOX
	void set_specular_env_level( float splev );
#endif

#endif
  // cross platform functions
  bool has_mesh();
  int  num_mesh_bones();

  // maximum detail level of my mesh
  //int get_max_detail() const;
  virtual void set_fade_away( bool fade ) { assert(false); }
  virtual bool get_fade_away() const { return false; }
  int get_min_polys() const;
  int get_max_polys() const;

  // script interface to TAMs
  /*
  virtual bool tam_is_playing();
  virtual void tam_play();
  virtual void tam_stop();
  virtual void tam_set_left_frame_marker( int left );
  virtual void tam_set_right_frame_marker( int right );
  virtual void tam_set_frame_marker_full();
  virtual void tam_set_direction( float newdir );
  virtual void tam_set_end_event( int event );
  virtual void tam_suicide();
  virtual void tam_set_force_frame( float frame, time_value_t t );
  */

/////////////////////////////////////////////////////////////////////////////
// Identification interface
/////////////////////////////////////////////////////////////////////////////
public:
  virtual stringx const& get_filename() const { assert(false); return sendl; } // <<<< only provided by physents.  need to deal with this.
  virtual stringx const& get_dirname() const { assert(false); return sendl; }
  virtual bool has_dirname() const { return false; }

  entity_id get_id() const {return id;}
  stringx get_name() const { return id.get_val(); }
  entity_flavor_t get_flavor() const {return flavor;}

  anim_id_t get_anim_id() const { return anim_id; }
  void set_anim_id( const char* _anim_id )
  {
    anim_id = anim_id_manager::inst()->anim_id( _anim_id );
  }
  void set_anim_id( const stringx& _anim_id )
  {
    anim_id = anim_id_manager::inst()->anim_id( _anim_id );
  }
  void set_anim_id( anim_id_t _anim_id ) { anim_id = _anim_id; }

  int get_min_detail() const { return min_detail; }
  virtual void set_min_detail(int md);
public:
  virtual bool is_an_entity() const { return(true); }

  virtual bool is_a_beam() const { return false; }

  virtual bool is_a_camera() const { return false; }
  virtual bool is_a_station_camera() const { return false; }
  virtual bool is_a_game_camera() const { return false; }
  virtual bool is_a_marky_camera() const { return false; }
  virtual bool is_a_mouselook_camera() const { return false; }
  virtual bool is_a_sniper_camera() const { return false; }

  virtual bool is_a_conglomerate() const { return false; }
  virtual bool is_a_turret() const { return false; }

  virtual bool is_a_ladder() const { return false; }

  virtual bool is_a_light_source() const { return false; }

  virtual bool is_a_limb_body() const { return false; }

  virtual bool is_a_marker() const { return false; }
  virtual bool is_a_rectangle_marker() const { return false; }
  virtual bool is_a_cube_marker() const      { return false; }
  virtual bool is_a_crawl_marker() const     { return false; }

  virtual bool is_a_particle_generator() const { return false; }

  virtual bool is_a_physical_entity() const { return false; }

//!  virtual bool is_an_actor() const { return false; }
//!  virtual bool is_a_character() const { return false; }

  virtual bool is_a_crate() const { return false; }

  virtual bool is_an_item() const { return false; }
  virtual bool is_a_visual_item() const { return false; }
  virtual bool is_a_handheld_item() const { return false; }
  virtual bool is_a_gun() const { return false; }
  virtual bool is_a_thrown_item() const { return false; }
  virtual bool is_a_melee_item() const { return false; }
  virtual bool is_a_morphable_item() const { return false; }

  virtual bool is_a_projectile() const { return false; }

  virtual bool is_a_rigid_body() const { return false; }

  virtual bool is_a_grenade() const { return false; }
  virtual bool is_a_rocket() const { return false; }

  virtual bool is_a_scanner() const { return false; }

  virtual bool is_a_sky() const { return false; }

/////////////////////////////////////////////////////////////////////////////
// World interface
/////////////////////////////////////////////////////////////////////////////
public:
  // ALWAYS AVAILABLE
  // get older:
  virtual void advance_age(time_value_t t);

//  virtual void frame_advance( time_value_t t );
  virtual void frame_done() {}

  virtual bool add_position_increment( vector3d& v ) { assert(false); return false; }

  // virtual functions allow descendants to assert different notions of
  // position and radius for the purposes of computing entity's terrain
  // locale (i.e., sector and region(s) occupied)
  virtual const vector3d& terrain_position() const { return get_abs_position(); }
  virtual rational_t terrain_radius() const { return get_visual_radius(); }

  // this exists so characters can override it and attack their collision geometry
  // to their waist rather than to the actor
  virtual const po& get_colgeom_root_po() const { return get_abs_po(); }
  virtual const entity * get_colgeom_root() const { return this; }

  // these virtual functions allow types descended from entity to be
  // recognized when adding them to regions, so that the region class can
  // maintain lists of different entity types as desired
  virtual void add_me_to_region( region* r );
  // NOTE: any descendant of entity that overloads add_me_to_region() and
  // remove_me_from_region() must call the cleanup method remove_from_terrain()
  // in its destructor; the reason for this is that if we wait and let the
  // parent class (e.g., entity) call this method, it will end up using the
  // parent's version of remove_me_from_region() (which makes sense when you
  // think about it, since by that point the descendant class has already been
  // destroyed)
  virtual void remove_me_from_region( region* r );

  // compute_sector will determine what sector of the terrain BSP tree the
  // entity is in (using terrain_position(); see above), as well as all the
  // regions possibly occupied by the entity (if terrain_x() is greater
  // than zero)
  virtual void compute_sector( terrain& ter, bool use_high_res_intersect = false );

  bool has_valid_sector() const
  {
  /*if (pi && pi->parent) return pi->parent->has_valid_sector();
    else */return my_sector? true : false;
  }
  sector* get_sector() const { return my_sector; }

  // last region that was physically occupied by the entity;
  // NOTE: will return NULL if the entity has never occupied a valid sector
  // or is currently forced to at least one region (see force_region())
  virtual region_node* get_region() const { return center_region; }

  region_node* get_primary_region() const;

  int is_in_active_region();

    // update intersected regions list
  bool add_region( region_node* r );
  void remove_from_regions();
  void remove_from_terrain();

  // get list of regions intersected by this object
#ifndef REGIONCULL
  const region_node_pset& get_regions() const { return in_regions; }
#endif

  // force or un-force region membership
  virtual void force_region( region_node* r );
  virtual void force_current_region();
  virtual void unforce_regions();
  // put me into same region(s) as the given entity
  virtual void force_regions( entity* e );

/////////////////////////////////////////////////////////////////////////////
// Parenting interface
/////////////////////////////////////////////////////////////////////////////
public:
/*  typedef list<entity*> child_list;

  // this is the same as calling set_parent(NULL) or p->remove_child(this).
  virtual void clear_parent();
  entity* get_parent() const { return pi ? pi->parent : 0; }
  entity* get_flavor_parent(entity_flavor_t flav);
  // this is the same as calling p->add_child(this).
  void set_parent( entity* p );
  // change parent without affecting position in world
  void set_parent_rel( entity* p );

  void add_child( entity* e )     { e->set_parent( this ); }
  void remove_child( entity* e )  { e->clear_parent(); }

  bool has_children() const { return ci ? !ci->children.empty() : false; }
  int get_num_children() const { return ci ? ci->children.size() : 0; }
  const child_list& get_children() const { assert(ci); return ci->children; }
  void clear_ci() { ci = NULL; }

private:
  void create_parent_info( entity* p );
*/
/////////////////////////////////////////////////////////////////////////////
// Motion Blur interface
/////////////////////////////////////////////////////////////////////////////
public:
  virtual void record_motion();

/////////////////////////////////////////////////////////////////////////////
// Marky Cam interface
/////////////////////////////////////////////////////////////////////////////
public:
  virtual void camera_set_target( const vector3d& pos )  { assert(false); }
  virtual void camera_set_roll( rational_t angle )     { assert(false); }
  virtual void camera_set_collide_with_world( bool v ) { assert(false); }

  // logarithmically transitions to the target parameters.  returns true when there.
  virtual bool camera_slide_to( const vector3d& new_pos, const vector3d& new_target, rational_t new_roll, rational_t speed ) { assert( false ); return false; }

  // places the camera somewhere in a circle around the center, looking at the center.
  virtual bool camera_slide_to_orbit( const vector3d& center, rational_t range, rational_t theta, rational_t psi, rational_t speed ) { assert( false ); return false; }
  virtual void camera_orbit( const vector3d& center, rational_t range, rational_t theta, rational_t psi ) { assert( false ); }

/////////////////////////////////////////////////////////////////////////////
// Light Interface (This should probably be turned into an interface!!! - JDB)
/////////////////////////////////////////////////////////////////////////////
public:
  virtual const color& get_color() const;
  virtual void         set_color(const color& c);

  virtual const color& get_additive_color() const;
  virtual void         set_additive_color(const color& c);

  virtual rational_t get_near_range() const;
  virtual void       set_near_range(rational_t _r);

  virtual rational_t get_cutoff_range() const;
  virtual void       set_cutoff_range(rational_t _r);

  // not technically an interface to a light, but interface
  // to characters influenced by lights:
  virtual light_manager* get_light_set() const;
  virtual void create_light_set();

protected:
  light_manager* my_light_mgr;

/////////////////////////////////////////////////////////////////////////////
// Scripted movement interface
/////////////////////////////////////////////////////////////////////////////
public:
  movement_info * get_movement_info() const { return mi; }
  virtual bool is_frame_delta_valid() const { return mi && mi->frame_delta_valid; }
  virtual bool is_last_frame_delta_valid() const { return mi && mi->last_frame_delta_valid; }
  virtual const po & get_frame_delta() const { assert(mi); return mi->frame_delta; }
  virtual void set_frame_delta(po const & bob, time_value_t t);
  virtual void set_frame_delta_trans(const vector3d &bob, time_value_t t);
  virtual void invalidate_frame_delta();

  // Used by render loop to skip translucent pass on entities that don't need it.
  virtual render_flavor_t render_passes_needed() const;

  // aging stuff
  void rebirth();
  time_value_t get_age() const;
  void set_age(time_value_t);
  virtual time_value_t get_programmed_cell_death() const
  { return programmed_cell_death; }
  void set_programmed_cell_death( time_value_t t )
  { programmed_cell_death = t; }


/////////////////////////////////////////////////////////////////////////////
// Animation interface
/////////////////////////////////////////////////////////////////////////////

// hierarchical entity animation support
protected:
  entity_anim_tree* anim_trees[MAX_ANIM_SLOTS];
public:
  void load_anim( const stringx& filename ) const;
  void unload_anim( const stringx& filename ) const;

  // play a hierarchical animation in slot 0
	void make_animateable( bool onOff=true );
  entity_anim_tree* play_loop_anim( const stringx& filename,
                                    unsigned short anim_flags = 0,
                                    short loop = -1 );
  entity_anim_tree* play_anim( const stringx& filename,
                               time_value_t start_time,
                               unsigned short anim_flags = 0,
                               short loop = -1 );
  // play a hierarchical animation in the given slot (used for secondary animations)
  entity_anim_tree* play_anim( int slot,
                               const stringx& filename,
                               time_value_t start_time,
                               unsigned short anim_flags = 0,
                               short loop = -1 );
  entity_anim_tree* play_anim( int slot,
                               const stringx& _name,
                               const entity_track_tree& track,
                               time_value_t start_time,
                               unsigned short anim_flags = 0,
                               short loop = -1 );

  // play a hierarchical animation in the given slot (used for secondary animations)
  entity_anim_tree* play_anim( int slot,
                               const stringx& filenamea,
                               const stringx& filenameb,
                               rational_t blenda,
                               rational_t blendb,
                               time_value_t start_time,
                               unsigned short anim_flags = 0,
                               short loop = -1 );
  entity_anim_tree* play_anim( int slot,
                               const stringx& _name,
                               const entity_track_tree& tracka,
                               const entity_track_tree& trackb,
                               rational_t blenda,
                               rational_t blendb,
                               time_value_t start_time,
                               unsigned short anim_flags = 0,
                               short loop = -1 );


  bool has_anim_trees() const { return true; /*(anim_trees != NULL);*/ }
  entity_anim_tree* get_anim_tree( int slot ) const;
  void kill_anim( int slot );
  bool anim_finished( int slot ) const;
  // this function should ONLY be called by the entity_anim_tree destructor
  void clear_anim( entity_anim_tree* a );
  // this function causes the internal animation nodes of each attached hierarchical
  // animation to be destroyed (made necessary by the actor limb_tree_pool system)
  void deconstruct_anim_trees();
  // this function reconstructs all attached hierarchical animations
  void reconstruct_anim_trees();

// interface for currently-attached entity anim (this node only)
protected:
  entity_anim* current_anim;

public:
  // attach entity to given animation
  virtual bool attach_anim( entity_anim* new_anim );
  // detach entity from current animation
  virtual void detach_anim();
  // return pointer to current animation
  entity_anim* get_anim() const { return current_anim; }


/////////////////////////////////////////////////////////////////////////////
// entity_maker interface
private:
  entity_widget *owning_widget;
  entity_pool* my_entity_pool;

public:
  // if owned by a widget, assume this entity and its animations are not in world
  void set_owning_widget( entity_widget *_owning_widget ) { owning_widget = _owning_widget; }
  entity_widget *get_owning_widget() const { return owning_widget; }

  void set_entity_pool( entity_pool* ep ) { my_entity_pool = ep; }
  entity_pool* get_entity_pool() const { return my_entity_pool; }

  // this function will be called when an entity is acquired from the entity_maker cache
  virtual void acquire( unsigned int _flags );
  // this function will be called when the entity is released to the entity_maker cache
  virtual void release();



/////////////////////////////////////////////////////////////////////////////
// character interface
/////////////////////////////////////////////////////////////////////////////
public:
  virtual rational_t get_hit_points() const { return 1.0f; }
  virtual rational_t get_full_hit_points() const { return 1.0f; }


/////////////////////////////////////////////////////////////////////////////
// container info (for storing items)
/////////////////////////////////////////////////////////////////////////////
private:
  typedef vector<item*> item_list_t;
  struct container_info
  {
    item_list_t items;
  };
  container_info* coninfo;
public:
  bool is_container() const { return coninfo? true : false; }

  // returns false if already in list (item's quantity will be incremented)
  virtual bool add_item( item* it );

  int get_num_items() const { return coninfo? coninfo->items.size() : 0; }
  // returns NULL if index is out-of=range
  item* get_item( unsigned int n ) const;

  // returns NULL if like item is not found
  item* find_like_item( item* it ) const;
  item* find_item_by_name( const stringx &name ) const;
// BIGCULL  handheld_item* find_item_by_id( const stringx &id ) const;

  // returns -1 if item is not found in list
  int get_item_index( item* it ) const;

  // returns the next item in the list after the given one (wraps around);
  // returns NULL if given item is not found
  item* get_next_item( item* it ) const;

  // returns the previous item in the list before the given one (wraps around);
  // returns NULL if given item is not found
  item* get_prev_item( item* it ) const;

  // vomit all contained items into the world
  void disgorge_items(entity *target = NULL);

  // use the item
  virtual void use_item( item* it );

  item *get_last_item_used() const { return last_item_used; }

/////////////////////////////////////////////////////////////////////////////
// bounding-box info (only used by entities with collision mesh)
/////////////////////////////////////////////////////////////////////////////
protected:
  bounding_box* bbi;
public:
  virtual void compute_bounding_box();
  bool has_bounding_box() const { return bbi != NULL; }
  const bounding_box& get_bounding_box() const { return *bbi; }

/*
/////////////////////////////////////////////////////////////////////////////
// Internal entity implementation stuff
/////////////////////////////////////////////////////////////////////////////
public:
  struct parent_info
  {
    po       my_abs_po;    // this has to come at beginning to ensure 8-byte alignment
    entity*  parent;
    int      local_key;
    int      parent_key;

    parent_info()
    {
      parent = NULL;
      local_key = 1;
      parent_key = 0;
    }
  };

  struct children_info
  {
    child_list children;
  };
*/
private:
  void _intersect( region_node* r, bool use_high_res_intersect );
  void _update_regions();
  void _set_region_forced_status();

protected:
  int get_random_ifl_frame_boost() const;
  // Data
protected:
  // used by the read_info functions derived classes.  Opened by the physical_entity constructor,
  // closed by the base class constructor
  static chunk_file * my_fs;

//  unsigned int flags;

//  entity_id  id;
  anim_id_t  anim_id;

  visual_rep* my_visrep;
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
  nglMesh *shadow_mesh;
  nglMesh *lores_mesh;
  nglMesh *my_mesh;
	bool     usezbias;
	float    zbias;
	bool force_hi_res;


#endif
  rational_t vis_xz_rad_rel_center;

  collision_geometry* colgeom;

    // parent/children info
//  parent_info   * pi;
//  children_info * ci;

    // motion blur;
  motion_blur_info * mbi;

  // sound
#ifdef ECULL
  sound_emitter * emitter;
#endif

  // movement by script
  movement_info * mi;

  int min_detail;

  // terrain locale data
  sector*          my_sector;
  region_node*     center_region;
#ifndef REGIONCULL
  region_node_pset in_regions;
#endif

  // convenience for any algorithm that wants to avoid re-visiting entities
  //bool             visited;

  //  int random_ifl_frame_boost has been replaced by a method which samples a randomized table
  // of offsets indexed by the last digit of the entity's entity_id.


  // for trailing capsules.  We use this only for capsule-colgeom having entities.
  po * last_po;

	// portal which door affects (if entity is flagged IS_DOOR)
	portal *door_portal;

//  damage_info dmg_info;
  rational_t  target_timer;
  rational_t  damage_resist_modifier;

  item *last_item_used;

  bool suspended;
  bool suspended_active_status;

  entity_controller * my_controller;

  unsigned max_lights;

private:
//  rational_t radius;

  //    time_value_t        age;
  time_value_t programmed_cell_death;
  unsigned short bone_idx;

  unsigned int ext_flags;

  destroyable_info *destroy_info;

  stringx character_action_anim;
  entity *action_character;

  entity* current_target;  // pointer to current combat target (gun or melee); can be NULL
  vector3d current_target_pos;
  vector3d current_target_norm;

public:
//! begin stuff from physent
  bool point_in_radius( const vector3d& p ) const
  {
    fp r=get_radius();
    return ( ( get_abs_position() - p ).length2() <= r*r );
  }
//! end stuff from physent

  // pointer to current combat target (gun or melee); can be NULL
  vector3d get_current_target_pos() const { return current_target_pos; }
  void set_current_target_pos( const vector3d &c ) { current_target_pos = c; }

  entity* get_current_target() const { return current_target; }
  void set_current_target( entity* c ) { current_target = c; if(current_target) set_current_target_pos(current_target->get_abs_position());}

  vector3d get_current_target_norm() const { return current_target_norm; }
  void set_current_target_norm( const vector3d &c ) { current_target_norm = c; }

  virtual void apply_destruction_fx();
  virtual bool has_destroy_info() const { return (destroy_info != NULL); }
  virtual bool is_destroyable() const;
  virtual destroyable_info *get_destroy_info() const { return destroy_info; }
  virtual void create_destroy_info();

/*!  virtual void activate_by_character(character *chr);
  virtual character *action_activated_by() const { return action_character; }
  virtual const stringx& get_character_action_anim() const { return character_action_anim; }
!*/
/*
  int get_damage_amt() const            { return(dmg_info.damage); }
  const vector3d& get_damage_loc() const { return(dmg_info.loc); }
  const vector3d& get_damage_dir() const { return(dmg_info.dir); }
  int get_damage_type() const           { return(dmg_info.type); }
  int get_damage_flags() const          { return(dmg_info.flags); }
  entity* get_damage_attacker() const   { return(dmg_info.attacker); }
  item* get_damage_item() const         { return(dmg_info.attacker_itm); }
  bool get_damage_push_wounded() const  { return(dmg_info.push_wounded); }
  bool get_damage_push_death() const    { return(dmg_info.push_death); }
  void set_damage_push_wounded(bool p)  { dmg_info.push_wounded = p; }
  void set_damage_push_death(bool p)    { dmg_info.push_death = p; }
*/
  rational_t get_damage_resist_mod() const { return(damage_resist_modifier); }
  void set_damage_resist_mod(rational_t mod = 1.0f) { damage_resist_modifier = mod; if(damage_resist_modifier < 0) damage_resist_modifier = 0.0f; if(damage_resist_modifier > 1.0f) damage_resist_modifier = 1.0f; }

public:

  virtual void apply_damage(int damage, const vector3d &pos, const vector3d &norm, int _damage_type = 0, entity *attacker = NULL, int dmg_flags = 0 );
  virtual void copy_visrep(entity *ent);

  virtual bool allow_targeting() const;

  virtual bool test_combat_target( const vector3d& p0, const vector3d& p1,
                                   vector3d* impact_pos, vector3d* impact_normal,
                                   rational_t default_radius = 1.0f, bool rear_cull = true ) const;

  virtual vector3d get_detonate_position() const { return get_abs_position(); }

  virtual void add_signal_callbacks();

  // position at start of frame (in world coords);
  // used by wds to determine when compute_sector() is needed
  rational_t last_compute_sector_position_hash;

  virtual bool get_distance_fade_ok() const;

  region_node * update_region(bool parent_computed = false);

  rational_t get_target_timer() const { return(target_timer); }
  void inc_target_timer(rational_t inc) { target_timer += inc; }
  void set_target_timer(rational_t val) { target_timer = val; }

  virtual void suspend();
  virtual void unsuspend();
  bool is_suspended(){return suspended;}

  entity_controller * get_controller(){return my_controller;}
  void set_controller(entity_controller * c);
//  brain * get_brain();

  bool is_hero() const;
  virtual bool possibly_active() const;
  virtual bool possibly_aging() const;
  void region_update_poss_active();
  void region_update_poss_render();
  void region_update_poss_collide();

  virtual void set_control_active(bool a);

  virtual bool is_alive() const;
  virtual bool is_dying() const;
  virtual bool is_alive_or_dying() const;

  virtual void preload();



/////////////////////////////////////////////////////////////////////////////
// ifl file operations
public:
  virtual void ifl_play();
  virtual void ifl_lock(int frame_index);
  virtual void ifl_pause();

//Information for frame locking at renderering.
public:
  frame_info frame_time_info;

#if _ENABLE_WORLD_EDITOR
  unsigned int scene_flags;
  stringx ent_filename;
#endif

  void process_extra_scene_flags(unsigned int scn_flags);

protected:
  color32       render_color;
  vector3d      render_scale;
private:
  material_set  *alternative_materials;
public:
  void set_alternative_materials( material_set *arg );
  void set_alternative_materials( const stringx& alt_mat_name );
  material_set *get_alternative_material_set() const { return alternative_materials; }
  material_list *get_alternative_materials() const { return alternative_materials ? alternative_materials->data : NULL; }

  void set_render_color(const color32 new_color ) { render_color = new_color; }
  void set_render_color(const uint8 _r, const uint8 _g, const uint8 _b, const uint8 _a) { render_color.c.r = _r; render_color.c.g = _g; render_color.c.b = _b; render_color.c.a = _a; }
  color32 get_render_color() const { return render_color; }

  virtual void set_render_scale( const vector3d& s ) { render_scale = s; }
  vector3d get_render_scale() const { return render_scale; }

  // searchers
public:
  typedef list<entity*> entity_search_list;
  static entity_search_list found_entities;

  enum
  {
    // selection criteria
    _FIND_ENT_ANY          = 0x00000001,
    _FIND_ENT_AI_IFC       = 0x00000002,
// BIGCULL     _FIND_ENT_DAMAGE_IFC   = 0x00000004,
    _FIND_ENT_PHYSICAL_IFC = 0x00000008,

    // modifiers
    _FIND_ENT_VISIBLE      = 0x00010000,
    _FIND_ENT_INVISIBLE    = 0x00020000,
    _FIND_ENT_ALIVE        = 0x00040000,
    _FIND_ENT_DEAD         = 0x00080000,
    _FIND_ENT_ACTIVE       = 0x00100000,
    _FIND_ENT_INACTIVE     = 0x00200000,
    _FIND_ENT_ACTIVE_RGN   = 0x00400000,
    _FIND_ENT_INACTIVE_RGN = 0x00800000,
  } eEntitySearchFlags;

  inline bool match_search_flags(int flags)
  {
    return(
            (
              (flags & _FIND_ENT_ANY) ||
// BIGCULL               (flags & _FIND_ENT_DAMAGE_IFC) && has_damage_ifc() ||
              (flags & _FIND_ENT_PHYSICAL_IFC) && has_physical_ifc() ||
              (flags & _FIND_ENT_AI_IFC) && has_ai_ifc()
            )
           &&
            (
              (!(flags & _FIND_ENT_VISIBLE) || is_visible()) &&
              (!(flags & _FIND_ENT_INVISIBLE) || !is_visible()) &&
              (!(flags & _FIND_ENT_ALIVE) || is_alive()) &&
              (!(flags & _FIND_ENT_DEAD) || !is_alive()) &&
              (!(flags & _FIND_ENT_ACTIVE) || is_active()) &&
              (!(flags & _FIND_ENT_INACTIVE) || !is_active()) &&
              (!(flags & _FIND_ENT_ACTIVE_RGN) || is_in_active_region()) &&
              (!(flags & _FIND_ENT_INACTIVE_RGN) || !is_in_active_region())
            )
          );
  }

  static int find_entities(int flags, const vector3d &pos, rational_t radius, region_node *reg = NULL, bool only_active_portals = true);
  inline static int find_entities(int flags, entity *ent, rational_t radius, bool only_active_portals = true)
  {
    return(find_entities(flags, ent->get_abs_position(), radius, ent->get_primary_region(), only_active_portals));
  }

  static int find_entities(int flags);

	// weird multi-hero thing to try to fix the lights
	private:
	int which_hero;
	public:
	int get_hero_id( void );
	void set_hero_id( int heroId ) { which_hero=heroId; }

	// daaaaaaang that IS huge

	// This is a weird kelly slater specific multiple scene thingy
	private:
	bool in_underwater_scene;
	public:
	void put_in_underwater_scene( bool onOff=true ) { in_underwater_scene=onOff; }
	bool is_in_underwater_scene( void ) { return in_underwater_scene; }

}; // class entity (huge isn't it?)


class destroyable_info
{
protected:
  short flags;

  rational_t destroy_lifetime;
#ifdef ECULL
  stringx destroy_sound;
#endif
  stringx destroy_fx;
  stringx destroy_script;
  stringx destroyed_visrep;
  stringx preload_script;

  visual_rep *destroyed_mesh;

  int hit_points;

  virtual void copy_instance_data(destroyable_info* data);

  entity *owner;

//  int dread_net_cue;

public:
  enum eDestroyInfoFlags
  {
    _HIT_POINTS         = 0x0001,
    _DESTROY_FX         = 0x0002,
    _DESTROY_SCRIPT     = 0x0004,
    _DESTROY_SOUND      = 0x0008,
    _DESTROYED_VISREP   = 0x0010,
    _SINGLE_BLOW        = 0x0020,
    _REMAIN_VISIBLE     = 0x0040,
    _REMAIN_ACTIVE      = 0x0080,
    _PRELOAD_SCRIPT     = 0x0100,
    _PRELOAD_SCRIPT_RAN = 0x0200,
    _NO_COLLISION       = 0x0400,
    _REMAIN_COLLISION   = 0x0800
  };

  destroyable_info(entity *ent);
  virtual ~destroyable_info();

  void read_enx_data( chunk_file& fs, stringx& lstr );
  void reset();

  int apply_damage(int damage, const vector3d &pos, const vector3d &norm);

  void apply_destruction_fx();

  virtual void preload();

  bool is_flagged(eDestroyInfoFlags f) const { return (flags & f) != 0; }
  void set_flag(eDestroyInfoFlags f, bool val = true) { if(val) flags |= f; else flags &= ~f; }

  bool has_hit_points()        const { return is_flagged(_HIT_POINTS); }
  bool has_destroy_fx()        const { return is_flagged(_DESTROY_FX); }
  bool has_destroy_script()    const { return is_flagged(_DESTROY_SCRIPT); }
  bool has_preload_script()    const { return is_flagged(_PRELOAD_SCRIPT); }
  bool has_preload_script_run()const { return is_flagged(_PRELOAD_SCRIPT_RAN); }
#ifdef ECULL
  bool has_destroy_sound()     const { return is_flagged(_DESTROY_SOUND); }
#endif
  bool has_destroyed_visrep()  const { return is_flagged(_DESTROYED_VISREP) && destroyed_mesh!=NULL; }
  bool destroyed_visrep_flag() const { return is_flagged(_DESTROYED_VISREP); }
  bool is_single_blow()        const { return is_flagged(_SINGLE_BLOW); }
  bool remain_visible()        const { return is_flagged(_REMAIN_VISIBLE); }
  bool no_collision()          const { return is_flagged(_NO_COLLISION); }
  bool remain_collision()      const { return is_flagged(_REMAIN_COLLISION); }
  bool remain_active()         const { return is_flagged(_REMAIN_ACTIVE); }

  int get_hit_points()              const { return hit_points; }
  rational_t get_destroy_lifetime() const { return destroy_lifetime; }

#ifdef ECULL
  const stringx& get_destroy_sound()       const { return destroy_sound; }
#endif
  const stringx& get_destroy_fx()          const { return destroy_fx; }
  const stringx& get_destroy_script()      const { return destroy_script; }
  const stringx& get_preload_script()      const { return preload_script; }
  const stringx& get_destroyed_visrep()    const { return destroyed_visrep; }

  void set_hit_points(int hp)               { if(hp > 0) { set_flag(_HIT_POINTS, true); hit_points = hp; } else set_flag(_HIT_POINTS, false); }
  void set_has_hit_points(bool val)         { set_flag(_HIT_POINTS, val); }
  void set_single_blow(bool val)            { set_flag(_SINGLE_BLOW, val); }
  void set_remain_visible(bool val)         { set_flag(_REMAIN_VISIBLE, val); }
  void set_no_collision(bool val)           { set_flag(_NO_COLLISION, val); }
  void set_remain_collision(bool val)       { set_flag(_REMAIN_COLLISION, val); }
  void set_remain_active(bool val)          { set_flag(_REMAIN_ACTIVE, val); }
  void set_destroy_lifetime(rational_t val) { destroy_lifetime = val; }
#ifdef ECULL
  void set_has_destroy_sound(bool val)      { set_flag(_DESTROY_SOUND, val); }
#endif
  void set_has_destroy_fx(bool val)         { set_flag(_DESTROY_FX, val); }
  void set_has_destroy_script(bool val)     { set_flag(_DESTROY_SCRIPT, val); }
  void set_has_preload_script(bool val)     { set_flag(_PRELOAD_SCRIPT, val); }
  void set_has_preload_script_run(bool val) { set_flag(_PRELOAD_SCRIPT_RAN, val); }
  void set_has_destroyed_visrep(bool val)   { set_flag(_DESTROYED_VISREP, val); }
#ifdef ECULL
  void set_destroy_sound(const stringx& val)    { if(val.length() > 0) { set_flag(_DESTROY_SOUND, true); destroy_sound = val; } else set_flag(_DESTROY_SOUND, false); }
#endif
  void set_destroy_fx(const stringx& val)       { if(val.length() > 0) { set_flag(_DESTROY_FX, true); destroy_fx = val; } else set_flag(_DESTROY_FX, false); }
  void set_destroy_script(const stringx& val)   { if(val.length() > 0) { set_flag(_DESTROY_SCRIPT, true); destroy_script = val; } else set_flag(_DESTROY_SCRIPT, false); }
  void set_preload_script(const stringx& val)   { if(val.length() > 0) { set_flag(_PRELOAD_SCRIPT, true); destroy_script = val; } else set_flag(_PRELOAD_SCRIPT, false); set_flag(_PRELOAD_SCRIPT_RAN, false); }
  void set_destroyed_visrep(const stringx& val) { if(val.length() > 0) { set_flag(_DESTROYED_VISREP, true); destroyed_visrep = val; } else set_flag(_DESTROYED_VISREP, false); }

  visual_rep *get_destroyed_mesh()   const { return destroyed_mesh; }
  entity *get_owner()                const { return owner; }

  destroyable_info* make_instance(entity *ent);
};

void init_random_ifl_frame_boost_table();

#endif // ENTITY_H
