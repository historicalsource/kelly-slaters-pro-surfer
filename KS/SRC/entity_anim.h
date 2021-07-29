#ifndef _ENTITY_ANIM_H
#define _ENTITY_ANIM_H


#include "po_anim.h"
#include "signal_anim.h"
#include "instance.h"
#include "entity.h"
#include "staticmem.h"
#include "platform_defines.h"

class entity_track_node;
class entity_track_tree;

void entity_track_tree_from_binary( entity_track_tree* ett, const char* anmx_filename, unsigned int ett_size = 0xffffffff );

enum
{
  BORROWS_DATA,
  OWNS_DATA
};

class entity_track_node
  {
  // Data
  private:
  // WARNING:  do not add, remove or change data types without changing the .ANMX format first
    int32              id;       // anim_id
    int32              owner;    // i'm responsible for deleting my own tracks and children, or not
    int8               pad[sizeof(pstring)-2*sizeof(int)];  // converts a pstring into an int

    entity_track_node* m_child;
    entity_track_node* m_sibling;

    PRS_track*         m_prs_track;
    signal_track*      m_signal_track;
    int32              more_pad[4];  // to make nodes 64 bytes

  // Methods
  public:
    entity_track_node();
    ~entity_track_node();

    inline anim_id_t get_id() const { return id; }

    inline PRS_track*    get_prs_track() const { return m_prs_track; }
    inline signal_track* get_signal_track() const { return m_signal_track; }

    inline const entity_track_node* get_first_child() const { return m_child; }
    inline const entity_track_node* get_next_sibling() const { return m_sibling; }

    void add_child( entity_track_node* good_kid );

    time_value_t compute_duration() const;

#if !defined(NO_SERIAL_IN)
  friend void serial_in( chunk_file& fs, entity_track_node* node, entity_track_tree* tree );
#endif
#if !defined(NO_SERIAL_OUT)
  friend void serial_out( chunk_file& fs, const entity_track_node& node );
#endif
  friend void entity_track_tree_from_binary( entity_track_tree*, const char* fname, unsigned int ett_size );
  friend void debug_compare_nodes( entity_track_node* node1, entity_track_node* node2 );
  };

enum {MAX_ROOT_NODES=4};

class entity_track_tree
  {
  // Data
  private:
  // WARNING:  do not add, remove or change data types without changing the .ANMX format first
    char               file_header[4];
    uint32             version;
    int32              num_root_nodes;
    rational_t         floor_offset;
    time_value_t       duration;
    int32              total_nodes;
    int32              pad[2];  // to align root_nodes on 32 bytes
    entity_track_node  root_nodes[MAX_ROOT_NODES];  // this is somewhat of a misnomer;
    // you can access other nodes by indexing deeper into array

  // Methods
  public:
    entity_track_tree()
      :   num_root_nodes( 0 ),
          duration( 0 )
      {
      version = 0;
      memcpy( file_header, "ANM", 4 );
      }

    ~entity_track_tree();

    // we overload NEW & delete because it could either be allocated via
    // NEW entity_track_tree or NEW char[] if it's binary
    void* operator new( size_t size );
    void operator delete(void* p);

//    bool empty() const { return ( root_list.size() ); }
    entity_track_node* get_root();
    entity_track_node* insert_root();

    inline const entity_track_node* get_root_nodes() const { return root_nodes; }
    inline int get_num_root_nodes() const { return num_root_nodes; }

    inline time_value_t get_duration() const { return duration; }

    inline rational_t get_floor_offset() const { return floor_offset; }

    static const char* binary_extension( void );
    static const char* text_extension( void );
    static const char* extension( void );

  private:
    void _compute_duration();
    void _recursive_compute_duration( const entity_track_node* node, time_value_t& d ) const;

#if !defined(NO_SERIAL_IN)
  friend void serial_in( chunk_file& fs, entity_track_tree* tree );
#endif
#if !defined(NO_SERIAL_OUT)
  friend void serial_out( chunk_file& fs, const entity_track_tree& tree );
#endif
  friend void entity_track_tree_from_binary( entity_track_tree*, const char* fname, unsigned int ett_size );
  friend void debug_compare_nodes( entity_track_node* node1, entity_track_node* node2 );


  };


// entity track instance management
//extern instance_bank<entity_track_tree> entity_track_bank;
//void load_entity_track( const stringx& filename );
//void unload_entity_track( const stringx& filename );
//entity_track_tree* find_entity_track( const stringx& filename );


class aram_id_t;

#ifndef TARGET_GC
#define INVALID_ARAM_ID 0
#endif

struct ett_node
{
  public:
#ifdef __MWERKS__
    ett_node() { memset( this, 0, sizeof( struct ett_node ) ); };
    ett_node(const struct ett_node &n) { memcpy( this, &n, sizeof( struct ett_node ) ); };
#endif //__MWERKS__ Metrowerks 1.3.2 work around.
       // It seems the default copy constructor for a struct takes a non-const argument
    bool stored;
    int ref;
#ifdef TARGET_GC
    aram_id_t aram_id;
#else
    unsigned int aram_id;
#endif
    unsigned int offset;
    unsigned int size;
    entity_track_tree *ett;
};
  
class ett_manager
{
  typedef map< stringx, ett_node > ett_map_t;
  typedef map< stringx, ett_node >::iterator ett_map_itr_t;
  
  public:
    ett_manager();
    ~ett_manager();
    
    entity_track_tree *acquire(const stringx &name);
    void release(const entity_track_tree* ett);
    
    bool load(const stringx& filename);
    bool unload(const stringx& filename);
    
    bool search(const stringx &name);
    
  private:
    entity_track_tree *ett_malloc( int size );
    void ett_free(void *addr);
    
    ett_map_t ett_map;
    // These and stored contingously
    char *bin1_addr;
    char *bin2_addr;
        
    /* Only used for bookkeeping */
    int resident_anms;
    int resident_alloc_size;
    int max_resident_anms;
    int max_resident_alloc_size;
    int cache_hits;
    int cache_misses;
};

// NOTE:  For the sake of convenience, in descendants of entity_anim
// frame_advance() actually writes the result to the entity, so that a separate
// call to get_value() is not required as would normally be the case for an
// anim<> class.

class entity_anim : public anim<entity*>
{
  // Data
  protected:
    friend class entity_anim_tree;

    entity* ent;
    po_anim     *po_anim_ptr;
    signal_anim *signal_anim_ptr;
    int priority;

    quaternion tween_quat;
	vector3d rel_pos;
    //rational_t tween_timer;
    //rational_t tween_duration;

  // CTT 03/23/00: TEMPORARY: once we support entity_anim_tree in the entity class,
  // floor_offset can be removed from entity_anim
  public:
    rational_t floor_offset;

  // Methods
  public:
    entity_anim()
    : anim<entity*>(),
      ent( NULL ),
      po_anim_ptr( NULL ),
      signal_anim_ptr( NULL ),
      floor_offset( 0 )
    {
    }

    entity_anim( entity* _ent )
    : anim<entity*>(),
      ent( _ent ),
      po_anim_ptr( NULL ),
      signal_anim_ptr( NULL ),
      floor_offset( 0 )
    {
    }

    inline void construct( entity* _ent, unsigned short _flags )
    {
      ent = _ent;
      anim<entity*>::construct( _flags );
    }

    entity_anim( entity* _ent,
                 unsigned short _flags,
                 po_anim *po_ani = 0,
                 signal_anim *signal_ani = NULL )
    : anim<entity*>( _flags ),
      ent( _ent ),
      po_anim_ptr( po_ani ),
      signal_anim_ptr( signal_ani ),
      floor_offset( 0 )
    {
    }

    virtual ~entity_anim()
      {
      delete po_anim_ptr;
      delete signal_anim_ptr;
      detach();
      }

    inline entity* get_entity() const { return ent; }

    inline int get_priority() const { return priority; }
    inline void set_priority( int _priority ) { priority = _priority; }

    inline bool has_po_anim() const { return ( po_anim_ptr!=NULL && po_anim_ptr->is_valid() ); }
    inline po_anim* get_po_anim() const { return po_anim_ptr; }
    void set_po_anim( po_anim *pp );

    inline bool has_signal_anim() const { return false; } //( signal_anim_ptr!=NULL && signal_anim_ptr->is_valid() ); }
    inline signal_anim* get_signal_anim() const { return NULL; } //signal_anim_ptr; }
    inline void set_signal_anim( signal_anim *sp ) { signal_anim_ptr = sp; }

    bool attach( const anim_control_t& ac );
    void detach();

    // this function is used to reset the start values of a start-relative
    // animation, based on the current actual state of the entity as well as
    // the current state of the animation;  this is essentially a support
    // function for when an animated entity is moved by some external mechanism
    void reset_start( const anim_control_t& ac );

    // force completion of all absolute motion
    inline void finish_absolute_motion() const {}

    void frame_advance( const anim_control_t& ac );

    inline void set_time( time_value_t t )
    {
      if ( po_anim_ptr ) po_anim_ptr->set_time( t );
      //if ( signal_anim_ptr ) signal_anim_ptr->set_time( t );
    }

	STATICALLOCCLASSHEADER

  };

void entity_anim_tree_stl_prealloc ();
void entity_anim_tree_stl_dealloc ();

#if 1 //ndef __MSL_STL
typedef vector<entity_anim*> pentity_anim_vector;
#else

typedef simplestaticallocator<entity_anim *> pentity_anim_allocator;
typedef vector<entity_anim*,pentity_anim_allocator> pentity_anim_vector;
#endif

class entity_anim_tree : public entity_anim
  {
  // Types
  public:

    enum
      {
      FLAGS_PASSED_TO_SUBANIMS = ANIM_REVERSE | ANIM_LOOPING
      };

  // Data
  private:
    stringx name;
    const entity_track_tree* track;
    rational_t blend_a;
    anim_control_t control;
		#ifdef USINGSTATICSTLALLOCATIONS
    pentity_anim_vector& anims;
		#else
    pentity_anim_vector anims;
		#endif
    rational_t floor_offset;

    const entity_track_tree* trackb;
    rational_t blend_b;
    pentity_anim_vector anims_b;
    anim_control_t control_b;

  // Methods
  public:
    entity_anim_tree( const stringx& _name,
                      entity* _ent,
                      const entity_track_tree& _track,
                      unsigned short anim_flags = 0,
                      time_value_t start_time = 0,
                      int _priority = 0,
                      short loop = -1 );

    entity_anim_tree( const stringx& _name,
                      entity* _ent,
                      const entity_track_tree& _tracka,
                      const entity_track_tree& _trackb,
                      rational_t blenda,
                      rational_t blendb,
                      unsigned short anim_flags = 0,
                      time_value_t start_time = 0,
                      int _priority = 0,
                      short loop = -1 );

    virtual ~entity_anim_tree();

    void construct( const stringx& _name,
                    const entity_track_tree& _track,
                    unsigned short anim_flags,
                    time_value_t start_time,
                    int _priority,
                    short loop );

    void construct( const stringx& _name,
                    const entity_track_tree& _tracka,
                    const entity_track_tree& _trackb,
                    rational_t blenda,
                    rational_t blendb,
                    unsigned short anim_flags,
                    time_value_t start_time,
                    int _priority,
                    short loop );

    // deconstruction utilities:
    void clear_anims();
    void clear_anims_b();

    ///////////////////////////////////////////////////////////////////////////
    // animation control

    inline time_value_t get_time() const { return control.get_time(); }
    inline time_value_t get_duration() const { return control.get_duration(); }
    inline time_value_t get_time_remaining() const { return control.get_duration() - control.get_time(); }
    inline rational_t get_timescale_factor() const { return control.get_timescale_factor(); }
	inline void set_tween_duration(rational_t dur) { control.set_tween_duration(dur); }

    inline bool is_finished() const { return control.is_finished(); }
	inline bool is_done_tween() const { return control.is_done_tween(); }
	inline bool is_tween() const { return control.is_tween(); }

    inline bool is_reverse()        const { return control.is_reverse(); }
    inline bool is_looping()        const { return control.is_looping(); }
    inline bool is_autokill()       const { return control.is_autokill(); }
    inline bool is_suspended()      const { return control.is_suspended(); }
    inline bool is_allow_controls() const { return control.is_allow_controls(); }
    inline bool is_compute_sector() const { return control.is_compute_sector(); }
    inline bool is_noncosmetic()    const { return control.is_noncosmetic(); }
    inline bool was_looped()        const { return control.was_looped(); }
	inline bool was_blended()       const { return (!control.is_tween()
											|| (control.get_tween_timer() >= control.get_tween_duration())); }

    inline void set_suspended( bool torf ) { control.set_flag( ANIM_SUSPENDED, torf ); }

	virtual void set_flag( anim_flags_t f )				{ control.set_flag(f, true); control_b.set_flag(f, true); entity_anim::set_flag(f); }
	virtual void clear_flag( anim_flags_t f )			{ control.set_flag(f, false); control_b.set_flag(f, false); entity_anim::clear_flag(f); }
	virtual void set_flag( anim_flags_t f, bool torf )	{ control.set_flag(f, torf); control_b.set_flag(f, torf); entity_anim::set_flag(f, torf); }

    ///////////////////////////////////////////////////////////////////////////

    // force current_time to given value
    void set_time( time_value_t t );

    void attach();
    void detach();

    void set_blend(rational_t a, rational_t b);
    inline rational_t get_blend_a() const { return(blend_a); }
    inline rational_t get_blend_b() const { return(blend_b); }

    // This notion of deconstructing and reconstructing animations is made
    // necessary by the limb_tree pool system for actors, whereby they can
    // lose their limb trees upon becoming inactive.
    void deconstruct();
    void reconstruct( int _priority );

    // the animation timescale defaults to 1.0f; setting it otherwise changes the
    // effective playback rate of the animation
    void set_timescale_factor( rational_t _timescale_factor );

    void frame_advance( time_value_t t );

    inline void finish_absolute_motion() const
    {
      entity_anim* root_anim = *anims.begin();
      if ( root_anim->is_relative_to_start() )
      {
        // force unparented (root) node animation to end of track
        root_anim->set_time( control.get_duration() );
      }
    }

    inline const stringx& get_name() const { return name; }

    inline const entity_track_tree& get_track() const { return *track; }
    inline const entity_track_tree& get_track_b() const { return *trackb; }

    inline rational_t get_floor_offset() const { return floor_offset; }
    inline void set_floor_offset(rational_t f) { floor_offset = f; }

    void set_priority( int _priority );

    // call this to fix the root start position after you move the entity manually
    void reset_root_position();

    // this function returns the unadjusted (relative) value of the root position anim
    // at the current time
    void get_current_root_relpos( vector3d* destP ) const;
    bool is_root(entity *ent) const;

    void get_current_po( po* dest ) const;
	void debug_print_PRS_to_file(void);

  private:
    bool _recursive_construct( entity* _ent, const entity_track_node* node );
    bool _recursive_reattach( entity* _ent, const entity_track_node* node );

	STATICALLOCCLASSHEADER


friend void entity_anim_tree_stl_prealloc ();

  };


#endif  // _ENTITY_ANIM_H
