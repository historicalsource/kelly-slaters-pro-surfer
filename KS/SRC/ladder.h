#ifndef LADDER_H
#define LADDER_H


//
// A climber on a ladder will face positive Z in the ladder's local space.  The ladder will be build so that
// The visual "front" of the ladder points along the negative z.
// Ladders are always vertical.
//

//#include "physent.h"


class ladder : public entity
  {
  public:
    ladder( const entity_id& _id, unsigned int _flags );

    ladder( const entity_id& _id = ANONYMOUS,
            entity_flavor_t _flavor = ENTITY_LADDER,
            unsigned int _flags = 0 );

    ladder(const entity_id & eid, rational_t _height, entity_flavor_t _flavor = ENTITY_LADDER);

  protected:
    void initialize_variables();

  /////////////////////////////////////////////////////////////////////////////
  // entity class identification
  public:
    virtual bool is_a_ladder() const { return true; }

  // NEWENT File I/O
  public:
    ladder( chunk_file& fs,
            const entity_id& _id,
            entity_flavor_t _flavor = ENTITY_LADDER,
            unsigned int _flags = 0 );
    virtual ~ladder() {}

  // Old File I/O
  public:
    ladder( const stringx& entity_fname,
                     const entity_id& _id = ANONYMOUS,
                     entity_flavor_t _flavor = ENTITY_LADDER );

  // Instancing
  public:
    virtual entity* make_instance( const entity_id& _id,
                                   unsigned int _flags ) const;
  protected:
    void copy_instance_data( const ladder& b );
    
    virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );

  // Misc.
  public:
    enum ladder_type
    {
    VERTICAL_LADDER = 0,
    HORIZONTAL_LADDER,
    ZIP_LINE,
    UNDEFINED_LADDER
    };

    // these virtual functions allow types descended from entity to be
    // recognized when adding them to regions, so that the region class can
    // maintain lists of different entity types as desired
    virtual void add_me_to_region( region* r );
    virtual void remove_me_from_region( region* r );

    virtual vector3d get_top_mount_position();
    virtual vector3d get_bottom_mount_position();

    virtual vector3d get_top_dismount_position();
    virtual vector3d get_bottom_dismount_position();

    virtual ladder_type get_ladder_type() { return(lad_type); }

    const stringx& get_climb_up_r_anim() { return(climb_up_r_anim); }
    const stringx& get_climb_up_l_anim() { return(climb_up_l_anim); }

    const stringx& get_climb_down_r_anim() { return(climb_down_r_anim); }
    const stringx& get_climb_down_l_anim() { return(climb_down_l_anim); }

    const stringx& get_mount_top_anim()    { return(mount_top_anim); }
    const stringx& get_mount_bottom_anim() { return(mount_bottom_anim); }

    const stringx& get_dismount_top_l_anim() { return(dismount_top_l_anim); }
    const stringx& get_dismount_top_r_anim() { return(dismount_top_r_anim); }

    const stringx& get_dismount_bottom_l_anim() { return(dismount_bottom_l_anim); }
    const stringx& get_dismount_bottom_r_anim() { return(dismount_bottom_r_anim); }

    float get_gravity_booster() { return(gravity_booster); }

    virtual bool allow_mount(entity *ent);

    virtual void load_anims(entity *chr);

  // Data
  private:
    vector3d top_mount_offset;
    vector3d bottom_mount_offset;

    vector3d top_dismount_offset;
    vector3d bottom_dismount_offset;

    float gravity_booster;


    stringx climb_up_r_anim;
    stringx climb_up_l_anim;

    stringx climb_down_r_anim;
    stringx climb_down_l_anim;

    stringx mount_top_anim;
    stringx mount_bottom_anim;

    stringx dismount_top_l_anim;
    stringx dismount_top_r_anim;

    stringx dismount_bottom_l_anim;
    stringx dismount_bottom_r_anim;

    ladder_type lad_type;
  };


inline ladder * find_ladder( const entity_id& id )
  {
  return (ladder*)entity_manager::inst()->find_entity( id, ENTITY_LADDER );
  }


#endif
