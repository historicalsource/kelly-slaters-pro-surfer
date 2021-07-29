// scanner.h
#ifndef _SCANNER_H
#define _SCANNER_H

#include "entity.h"

//#ifdef TARGET_PC
#include "renderflav.h"
//#define TWO_PASS_RENDER
//#endif


const rational_t _SCANNER_DEFAULT_MAX_APERTURE = PI * 0.5f;
const rational_t _SCANNER_DEFAULT_MAX_SWEEP    = PI * 0.5f;
const rational_t _SCANNER_DEFAULT_SPEED        = PI * 0.5f;
const rational_t _SCANNER_DEFAULT_MAX_LENGTH   = 25.0f;
const rational_t _SCANNER_DEFAULT_THICKNESS    = 0.1f;
#ifdef TARGET_PC
const color32 _SCANNER_DEFAULT_COLOR( 0, 255, 0, 160 );
#else
const color32 _SCANNER_DEFAULT_COLOR( 0, 255, 0, 100 );
#endif


class vr_pmesh;
class ai_interface;
class scanner : public entity
{
///////////////////////////////////////////////////////////////////////////////
// Types
///////////////////////////////////////////////////////////////////////////////
public:
  enum scan_flags_t
  {
    SPINNER         = 0x00000001,  // if true, scanner will spin about the focal axis rather than sweeping
    OPTIMIZED       = 0x00000002,  // if true, scanner was optimized
    REVERSE         = 0x00000004   // if true, scanner runs in reverse (negative starting speed)
  };

///////////////////////////////////////////////////////////////////////////////
// Data
///////////////////////////////////////////////////////////////////////////////
private:
  vector3d lookat;           // center of focus
  rational_t max_aperture;   // max arc (radians) of sheet top and bottom
  rational_t max_sweep;      // max extent (radians) of sweep
	rational_t max_length;     // max distance the scanner beam can travel
  unsigned int scan_flags;   // scanner control flags
  rational_t curr_aperture;  // current arc (radians) of sheet top and bottom
  rational_t curr_sweep;     // current extent (radians) of sweep
  rational_t speed;          // speed of sweep/spin (radians per second)
  rational_t angle;          // current angle of scan
  rational_t thickness;      // thickness of the painted line
  color32 my_color;          // color of sheet and line
  vector3d sheet_offset;     // offset from origin to the emitting point of the scan sheet
  bool contact;              // true if scanner is in contact with hero
  rational_t contact_color_ramp;
  bool tag_mode;             // turn on to scan for polys affected by scanner
  vector<region_node *> scan_regs;

  friend class ScannerCommand;
  friend class ScannerDialog;
///////////////////////////////////////////////////////////////////////////////
// Construction
///////////////////////////////////////////////////////////////////////////////
public:
  scanner( chunk_file& fs,
           const entity_id& _id,
           entity_flavor_t _flavor = ENTITY_SCANNER,
           unsigned int _flags = 0 );
  scanner( const entity_id& _id,
           entity_flavor_t _flavor = ENTITY_SCANNER,
           unsigned int _flags = 0 );
  virtual ~scanner();
	virtual void initialize();
  // This function allows parsing instance data according to entity type.
  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );
  virtual bool handle_enx_chunk(chunk_file &fs, stringx &label);
	// Build the list of polys, only AFTER the entity has been added to the proper regions - usually called by initialize()
	void build_poly_list(bool optimize = true);
private:
  void initialize_variables();

  // Instancing
public:
  virtual entity *make_instance(const entity_id &_id, unsigned int _flags) const;
protected:
  void copy_instance_data(const scanner &b);

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_scanner() const { return true; }

///////////////////////////////////////////////////////////////////////////////
// Scanner settings interface
///////////////////////////////////////////////////////////////////////////////
public:
  const vector3d& get_lookat()  const { return lookat; }
  rational_t get_max_aperture() const { return max_aperture; }
  rational_t get_max_sweep()    const { return max_sweep; }
  bool is_flagged( scan_flags_t f ) const { return (scan_flags & f); }
  rational_t get_aperture()     const { return curr_aperture; }
  rational_t get_sweep()        const { return curr_sweep; }
  rational_t get_speed()        const { return speed; }
  rational_t get_thickness()    const { return thickness; }

  void set_flag( scan_flags_t f, bool tf )    { scan_flags = tf? (scan_flags|f) : (scan_flags&~f); }
  void set_aperture( rational_t _aperture )   { curr_aperture = min( _aperture, max_aperture ); }
  void set_speed( rational_t _speed )         { speed = _speed; }
  void set_thickness( rational_t _thickness ) { thickness = _thickness; }
  void set_color( const color32& _color );
  void set_sheet_offset( const vector3d& _sheet_offset )  { sheet_offset = _sheet_offset; }

  void set_sweep( rational_t _sweep )
  {
    if ( _sweep >= PI*2-0.001f )  // allow for numerical error
      _sweep = PI*2;
    curr_sweep = min( _sweep, max_sweep );
  }

  void reset();

  virtual void set_visible( bool a );
  virtual bool possibly_active() const        { return(is_visible()); }

///////////////////////////////////////////////////////////////////////////////
// Event signals
///////////////////////////////////////////////////////////////////////////////
public:
  // enum of local signal ids (for coding convenience and readability)
  enum signal_id_t
  {
    // a descendant class uses the following line to append its local signal ids after the parent's
    PARENT_SYNC_DUMMY = entity::N_SIGNALS - 1,
    #define MAC(label,str)  label,
    // replace "entity" with whatever is appropriate
    #include "scanner_signals.h"
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

	// Internal utility functions
	void render_line(vector3d v0, vector3d v1, rational_t thickness, vector3d normal, render_flavor_t flavor, rational_t entity_translucency_pct);
	//void render_tri(vector3d v0, vector3d v1, vector3d v2, render_flavor_t flavor, rational_t entity_translucency_pct);
	void render_quad(vector3d v0, vector3d v1, vector3d v2, vector3d v3, render_flavor_t flavor, rational_t entity_translucency_pct);

  void clear_poly_list();

	// segment management inlines
	void clear_seg_list()
		{ for (segment *s = segments, *next; s; s = next) { next = s->next; delete s; } segments = NULL; assert(!pending && !pending_check); }

	void add_segment(vector3d p0, vector3d p1, vector3d normal, rational_t a0, rational_t a1, rational_t d0, rational_t d1, int list, int poly)
	{
		segment *new_seg = NEW segment;
		new_seg->p0 = p0; new_seg->p1 = p1;
		new_seg->normal = normal;
		new_seg->a0 = a0; new_seg->a1 = a1;
		new_seg->d0 = d0; new_seg->d1 = d1;
    new_seg->list = list; new_seg->poly = poly;
		new_seg->next = segments;
		segments = new_seg;
	}

	void add_segment_pending(vector3d p0, vector3d p1, vector3d normal, rational_t a0, rational_t a1, rational_t d0, rational_t d1, int list, int poly)
	{
		segment *new_seg = NEW segment;
		new_seg->p0 = p0; new_seg->p1 = p1;
		new_seg->normal = normal;
		new_seg->a0 = a0; new_seg->a1 = a1;
		new_seg->d0 = d0; new_seg->d1 = d1;
    new_seg->list = list; new_seg->poly = poly;
		new_seg->next = pending;
		pending = new_seg;
	}

	void add_segment_pending_check(vector3d p0, vector3d p1, vector3d normal, rational_t a0, rational_t a1, rational_t d0, rational_t d1, int list, int poly)
	{
		segment *new_seg = NEW segment;
		new_seg->p0 = p0; new_seg->p1 = p1;
		new_seg->normal = normal;
		new_seg->a0 = a0; new_seg->a1 = a1;
		new_seg->d0 = d0; new_seg->d1 = d1;
    new_seg->list = list; new_seg->poly = poly;
		new_seg->next = pending_check;
		pending_check = new_seg;
	}

	bool check_segment(vector3d &p0, vector3d &p1, vector3d &normal, rational_t &a0, rational_t &a1, rational_t &d0, rational_t &d1, int list, int poly, bool occlude = true);

  void concatenate_segments();
  void render_sheet( rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );

	// Internal data
	static vr_pmesh *the_pmesh;  // used by all scanners for visrep

	struct polygon
	{
		vector3d verts[3];
		vector3d normal;
    bool active;
	};

  struct poly_list
  {
    const po *xform_po;      // pointer to the matrix for this poly list (NULL means don't xform)
    polygon *polys;          // actual list of polys
    int num_polys;
  };

  poly_list *poly_lists;
	int num_lists;

//#ifdef TWO_PASS_RENDER
//	render_flavor_t my_render_flavor;
//#endif

	struct segment
	{
		vector3d p0, p1, normal;
		rational_t a0, a1, d0, d1;
    int list, poly;              // indicies into poly_lists for tag_mode
		struct segment *next;
	};

  bool seg_in_front_of(vector3d &p0, vector3d &p1, rational_t d0, rational_t d1, segment *s);

	segment *segments;         // linked list of sheet segments generated this frame
	segment *pending;          // pending addition to segment list
	segment *pending_check;    // pending check and then addition to segment list

protected:
  // This virtual function, used only for debugging purposes, returns the
  // name of the given local signal
  virtual const char* get_signal_name( unsigned short idx ) const;

  vr_pmesh *sheet_pmesh;

  // these internal members control the automatic alarm sound
  static int detected_count;
  void INC_detector();
  void DEC_detector();

  // this is called only when the scanner's contact status changes (ENTER or LEAVE)
  void set_contact( bool _contact );

  virtual bool check_scan_collisions(const po &inv_po, rational_t aperture, rational_t sin_aperture, rational_t cos_aperture);
  virtual bool check_scan_collision_ent(entity *ent, const po &inv_po, rational_t aperture, rational_t sin_aperture, rational_t cos_aperture);

  ai_interface *owner;
  vector3d last_ai_pos;
  entity *scan_target;

///////////////////////////////////////////////////////////////////////////////
// World interface
///////////////////////////////////////////////////////////////////////////////
public:
  virtual void frame_advance( time_value_t t );
  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );
  // Used by render loop to skip translucent pass on entities that don't need it.
  virtual render_flavor_t render_passes_needed() const;
  virtual rational_t get_visual_radius() const;

  virtual void set_active( bool a );
  virtual void set_ai_owner(ai_interface *own);

  entity *get_scan_target() const { return(scan_target); }
};


#endif  // _SCANNER_H

