// conglom.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
#ifndef CONGLOM_H
#define CONGLOM_H


#include "entity.h"
#include "refptr.h"

class light_manager;
// BIGCULL class scanner;
typedef vector<entity*> pentity_vector;


class conglomerate : public entity
{
  // Data
 private:
  // Members are controlled by the world, but referenced here in order to
  // preserve their association as a group of entities loaded from one .ent
  // file.  This allows the user to refer to any instance of the group by a
  // given entity_id, and also allows the group to be recursively instanced.
  pentity_vector members;
  vector<stringx
		#ifdef TARGET_PS2
	       ,malloc_alloc
		#endif
	       > names;
  vector<char> parents;  // parent indices; used by make_instance()
  refptr<light_manager> lightmgr;

 public:
  virtual void apply_destruction_fx();

  // Generic Constructors
 public:
  conglomerate( const entity_id& _id, unsigned int _flags );
  conglomerate( const entity_id& _id = ANONYMOUS,
                entity_flavor_t _flavor = ENTITY_CONGLOMERATE,
                unsigned int _flags = 0 );

  virtual ~conglomerate();

  // File I/O
 public:
  conglomerate( chunk_file& fs,
                const entity_id& _id,
                entity_flavor_t _flavor = ENTITY_CONGLOMERATE,
                unsigned int _flags = 0 );
  // This function allows parsing instance data according to entity type.
  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );
 private:
  void read_node( chunk_file& fs, entity* _parent, bool has_skeleton );

  // Instancing
 public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
 protected:
  void copy_instance_data( const conglomerate& b );

 /////////////////////////////////////////////////////////////////////////////
 // entity class identification
 public:
   virtual bool is_a_conglomerate() const { return true; }

  // Misc.
 public:
  // ifl operations
  virtual void ifl_lock(int);
  virtual void ifl_pause();
  virtual void ifl_play();
  virtual void set_render_color( const color32 new_color );

  virtual void set_visible( bool a );
  virtual bool is_still_visible() const;  // accounts for lingering particles; see particle.h

  virtual rational_t terrain_radius() const;

  virtual void force_region( region_node* r );
  virtual void force_current_region();
  virtual void unforce_regions();

  virtual void frame_advance(time_value_t t);

  virtual light_manager* get_light_set() const;

  virtual void set_min_detail(int md);

  virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );

  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );
  virtual void rendershadow( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct, rational_t scale );

	void updatelighting( time_value_t t, const int playerID );

  void add_member( entity* ent, const stringx& nodename );

  entity* get_member_by_flavor( entity_flavor_t flav );
// BIGCULL  scanner* get_scanner() { return((scanner *)get_member_by_flavor(ENTITY_SCANNER)); }

  const pentity_vector &get_members() const { return members; }

  // get pointer to member (found by node name)
  entity* get_member( const stringx& nodename );
  const stringx& get_member_nodename( entity *member );
  bool has_member(entity *ent) const;

  entity* get_child( entity *ent, entity *prev_child = NULL );
  entity* get_child( const stringx& nodename, entity *prev_child = NULL ) { return(get_child(get_member(nodename), prev_child)); }

  void add_members_to_bones( entity* ent, matrix4x4* bones, int* num_bones );

  virtual void compute_sector( terrain& ter, bool use_high_res_intersect = false );
  virtual void compute_bounding_box();
  void compute_radius();

  virtual void frame_done_including_members();

  // entity_maker caching interface
  virtual void acquire( unsigned int _flags );
  virtual void release();

  virtual void set_ext_flag_recursive(register unsigned int f, register bool set);

};


#endif  // CONGLOM_H
