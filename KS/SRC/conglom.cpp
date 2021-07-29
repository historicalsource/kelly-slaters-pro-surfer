// conglom.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
#include "global.h"

//!#include "character.h"
#include "conglom.h"
#include "wds.h"
#include "light.h"
#include "lightmgr.h"
//#include "oserrmsg.h"
#include "particle.h"
//!#include "ladder.h"
#include "pmesh.h"
#include "osdevopts.h"
#include "renderflav.h"
#include "iri.h"
#include "colgeom.h"
#include "colmesh.h"
// BIGCULL #include "scanner.h"
#include "controller.h"
// BIGCULL #include "handheld_item.h"
#include "entity_maker.h"
#include "physical_interface.h"
// BIGCULL #include "damage_interface.h"
#include "ai_interface.h"
#include "collide.h"
#include "profiler.h"

#include "ngl_support.h"

#ifdef TARGET_GC
#include <dolphin/mtx.h>
#endif //TARGET_GC

///////////////////////////////////////////////////////////////////////////////
// Generic Constructors
///////////////////////////////////////////////////////////////////////////////

conglomerate::conglomerate( const entity_id& _id, unsigned int _flags )
  : entity( _id, ENTITY_CONGLOMERATE, _flags )
{
}

conglomerate::conglomerate( const entity_id& _id,
                            entity_flavor_t _flavor,
                            unsigned int _flags )
  : entity( _id, _flavor, _flags )
{
}

conglomerate::~conglomerate()
{
  pentity_vector::iterator i;
  for ( i=members.begin(); i!=members.end(); ++i )
  {
    entity *ent = (*i);
//    warning("%s", (ent != NULL ? ent->get_name().c_str() : "NULL"));

    if ( ent->get_bone_idx() >= 0 )
      delete ent;
    else
      g_entity_maker->destroy_entity( ent );
  }

  members.resize(0); // PEH BETA
  names.resize(0); // PEH BETA
  parents.resize(0); // PEH BETA
}

#include "skeleton_interface.h"
#include "hard_attrib_interface.h"
#include "soft_attrib_interface.h"

///////////////////////////////////////////////////////////////////////////////
// File I/O
///////////////////////////////////////////////////////////////////////////////

conglomerate::conglomerate( chunk_file& fs,
                            const entity_id& _id,
                            entity_flavor_t _flavor,
                            unsigned int _flags )
  : entity( fs, _id, _flavor, _flags )
{
  int nbones = num_mesh_bones();
  bool has_skeleton = nbones?true:false;

	//stringx mesh_path = "boards\\entities\\";
	//nglSetMeshPath( (char *)mesh_path.c_str() );
	//stringx texture_path = "boards\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	//nglSetTexturePath( (char *)texture_path.c_str() );

  chunk_flavor cf;
  for( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( cf == chunk_flavor("node") )
      read_node( fs, this, has_skeleton );
    else
      error( fs.get_name() + ": unexpected chunk '" + cf.to_stringx() + "' in conglomerate" );
  }

  // Now that we have the entire conglomerate loaded in, assign our skeleton abs_po buffer and
  // fix up the pointers to use it
  if ( has_skeleton )
  {
    if(!has_skeleton_ifc())
    {
      create_skeleton_ifc();
      skeleton_ifc()->initialize( nbones );
			assert(nbones);
    }
    // FIXME:  just because we have a skeleton doesn't necessarily mean we're
    //  a character
    if(!has_hard_attrib_ifc())
    {
      // allocate a sub-class for the attrib interfaces
      my_hard_attrib_interface = NEW character_hard_attrib_interface(this);
    }
    if(!has_soft_attrib_ifc())
    {
      // allocate a sub-class for the attrib interfaces
      my_soft_attrib_interface = NEW character_soft_attrib_interface(this);
    }

    pentity_vector::iterator i;
    for ( i=members.begin(); i!=members.end(); ++i)
    {
      int k = (*i)->get_bone_idx();

      if( k >= 0 ) {
        // k < 0 if I'm an entity in a conglomerate that is not a bone
        skeleton_ifc()->connect_bone_abs_po(k, (bone *)(*i));
      }

    }
  }
  update_abs_po();

  compute_radius();

  // I'd like skinned conglomerates to be lit, but for the entire
  // steel project the artists have assumed they weren't lit, and there's
  // a bug with entities getting set inactive that's keeping the lightmgr
  // from getting frame_advance'd properly.  Turn this on for the next project!
  if ( num_mesh_bones() )
  {
    // skinned conglomerates need to be lit
    lightmgr = NEW light_manager;
    lightmgr->max_lights = max_lights;
    set_flag( EFLAG_ACTIVE, true ); // otherwise we won't be frame_advance'd
  }

  if(get_colgeom() && get_colgeom()->get_type() == collision_geometry::CAPSULE)
  {
    capsule rel_cap;
    rel_cap.base = vector3d(0.0f, -0.25f, 0.0f);
    rel_cap.end = vector3d(0.0f, 0.4f, 0.0f);
    rel_cap.radius = 0.5f;

    collision_capsule *cap = (collision_capsule *)get_colgeom();
    cap->set_capsule(rel_cap);

    if(!has_physical_ifc())
      create_physical_ifc();

    physical_ifc()->enable();
    physical_ifc()->set_gravity(true);
    physical_ifc()->set_mass(1.0f);
    physical_ifc()->set_standing(true);

    set_stationary(false);
  }
	//lores_mesh=NULL;
	//shadow_mesh=NULL;
}


void conglomerate::read_node( chunk_file& fs, entity* _parent, bool has_skeleton )
{
  chunk_flavor cf;

  // node name
  stringx name;
  serial_in( fs, &name );
  name.to_upper();

  // member id is "conglomerate_id.node_id"
  entity_id _id( (id.get_val()+"_"+name).c_str() );
  entity* ent = NULL;

  serial_in( fs, &cf );

  // optional anim ID
  anim_id_t ent_animid = NO_ID;
  if ( cf == chunk_flavor("animid") )
  {
    stringx idstr;
    serial_in( fs, &idstr );
    ent_animid = anim_id_manager::inst()->anim_id( idstr );
    serial_in( fs, &cf );
  }

  // optional po
  po ent_po = po_identity_matrix;
  if ( cf == chunk_flavor("po") )
  {
    serial_in( fs, &ent_po );
    serial_in( fs, &cf );
  }

  // entity type is one of the following:

  // generic entity
  if ( cf == chunk_flavor("entity") )
  {
    ent = g_entity_maker->create_entity( fs, _id, flags, false );
  }

#if 0 // BIGCULL
  else if ( cf == chunk_flavor("switch") )
  {
    ent = (entity*)g_world_ptr->add_switch_obj( fs, _id, flags );
  }
#endif // BIGCULL
  else if ( cf == chunk_flavor("polytube") )
  {
    ent = (entity*)g_world_ptr->add_polytube( fs, _id, flags );
  }

  else if ( cf == chunk_flavor("lensflare") )
  {
    ent = (entity*)g_world_ptr->add_lensflare( fs, _id, flags );
  }

  // physical entity
  else if ( cf == chunk_flavor("physical") )
  {
    assert( g_entity_maker->get_owning_widget() == NULL );
//    ent = (entity*)g_world_ptr->add_physent( fs, _id, flags );
    assert(0);
  }

  // light source
  else if ( cf == chunk_flavor("light") )
  {
    assert( g_entity_maker->get_owning_widget() == NULL );
    ent = (entity*)g_world_ptr->add_light_source( fs, _id, flags );
  }

  // trigger
  else if( cf == chunk_flavor( "trigger" ) )
  {
    assert( g_entity_maker->get_owning_widget( ) == NULL );
    convex_box binfo;

//#pragma fixme( "replace this with a serial_in( fs, trigger_props* ) or some such. -mkv 4/6/01" )
    for( serial_in( fs, &cf ); cf != CHUNK_END; serial_in( fs, &cf ) )
    {

      if( cf == chunk_flavor( "convxbox" ) )
      {
        serial_in( fs, &binfo );
      }
      else
      {
        // some other data in the future?
      }

    }

    ent = g_world_ptr->add_box_trigger( _id, ent_po, binfo, NULL );
  }

  // particle generator
  else if ( cf == chunk_flavor("partsys") )
  {
    assert( g_entity_maker->get_owning_widget() == NULL );
    // read particle system data (for now, filename only)
    stringx fname;
    serial_in( fs, &fname );
    serial_in( fs, &cf );
    if ( cf != CHUNK_END )
      error( fs.get_name() + ": unexpected chunk '" + cf.to_stringx() + "' in particle_system node" );
    // create particle_generator
    ent = (entity*)g_world_ptr->add_particle_generator( fname, _id, flags );
  }

  // ladder
/*!  else if ( cf == chunk_flavor("ladder") )
  {
    assert( g_entity_maker->get_owning_widget() == NULL );
    ent = (entity*)g_world_ptr->add_ladder( fs, _id, flags );
  }
!*/
  // physical entity
#if 0 // BIGCULL
  else if ( cf == chunk_flavor("scanner") )
  {
    assert( g_entity_maker->get_owning_widget() == NULL );
    ent = (entity*)g_world_ptr->add_scanner( fs, _id, flags );
  }
#endif // BIGCULL
  else
    error( fs.get_name() + ": unexpected chunk '" + cf.to_stringx() + "' in conglomerate" );

  // set entity stuff
  assert( ent );
  ent->set_anim_id( ent_animid );

  // massage 'has_skeleton' if our bone index is < 0
//#pragma fixme("This is incorrect;  it assumes all bones or no bones  jdf 4-19-01")
  if( ent->get_bone_idx() < 0 ) {
    has_skeleton = false;
  }

  assert(has_link_ifc());
  if (has_skeleton) {
      // set skeleton to handle abs_po
      ent->link_ifc()->link_does_not_manage_abs_po();
  }

  ent->set_rel_po_no_children( ent_po );
  ent->link_ifc()->set_parent( _parent );

  // <Reid
  ent->compute_visual_xz_radius_rel_center();
  // Reid>

  // non-uniform scaling is not allowed;
  // uniform scaling is allowed only on entities that have no collision geometry
  check_po( ent );

  // we can't compute bounding boxes here because when the first
  // conglomerate is constructed it doesn't know it's po yet.  Now
  // compute_bounding_box is overloaded for conglomerates--cbb
#if 0
  // <Reid
  // we want each static entity to have an axis-aligned bounding box for collisions
  // (note that the bounding box will only be created if the entity has a cg_mesh)
  if ( ent->get_flavor()==ENTITY_ENTITY && !ent->is_flagged(EFLAG_MISC_NONSTATIC) )
    ent->compute_bounding_box();
  // Reid>
#endif

  // add to member list
  add_member( ent, name );

/*
  stringx parent_name = "<NONE>";

  if(_parent)
    parent_name = _parent->get_name();
*/

  // read children, if any
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( cf == chunk_flavor("node") )
      read_node( fs, ent, has_skeleton );
    else
      error( fs.get_name() + ": unexpected chunk '" + cf.to_stringx() + "' in conglomerate" );
  }
}



void conglomerate::compute_radius()
{
//  set_rel_po( po_identity_matrix );
//  update_abs_po();

  // can take radius from collision mesh, if any
  collision_geometry* cg = get_colgeom();
  if ( cg && cg->get_type()==collision_geometry::MESH )
  {
    set_radius( cg->get_radius() );
  }

  pentity_vector::iterator i = members.begin();
  pentity_vector::iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    rational_t r = (e->get_abs_position()-get_abs_position()).length() + e->get_radius();
    if ( r > get_radius() )
      set_radius( r );
  }
}


// This function allows parsing instance data according to entity type.
// If it recognizes the given chunk_flavor as a chunk of instance
// data for this type, it will parse the data; otherwise it will hand
// the parsing up to the parent class.
bool conglomerate::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("scanner") )
  {
    // look for a member that is a scanner
    pentity_vector::iterator i = members.begin();
    pentity_vector::iterator i_end = members.end();
    for ( ; i!=i_end; i++ )
    {
      if ( (*i)->get_flavor() == ENTITY_SCANNER )
        break;
    }
    if ( i == i_end )
      error( get_id().get_val() + ": parse_instance(): no scanner member found in conglomerate" );
    else
      return (*i)->parse_instance( pcf, fs );
  }
  else
    return entity::parse_instance( pcf, fs );
  return false;
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* conglomerate::make_instance( const entity_id& _id,
                                     unsigned int _flags ) const
{
  conglomerate* newcg = NEW conglomerate( _id, _flags );
  newcg->copy_instance_data( *this );
  return (entity*)newcg;
}


void conglomerate::copy_instance_data( const conglomerate& b )
{
  // copy flags that are not per-instance;
  // this line was copied from entity::copy_instance_data(), but we do it first
  // here because Jason moved that function call to be after member construction;
  // why he did this, I'm not sure
  //
  // I did this because in the entity::copy_instance data, it also copies common
  // brain weapons (ones defined in the 'weapons:' ENX chunk), and the AI Brain. Both of these
  // require that the conglomerate has all of it's members initialized. Either I moved
  // the copy_instance_data call, or duplicate some of the copy code with kludges to avoid
  // certain copies on conglomerates. (JDB)
  entity::copy_flags( b );

  // copy members
	#if 0
		// this causes a memory leak
  members.reserve( b.members.size() );
	#endif
  parents = b.parents;

  pentity_vector::const_iterator i;

  for( i = b.members.begin( ); i != b.members.end( ); ++i )
  {
    entity* ent = (*i);
    // construct NEW entity id
    const stringx& nodename = ent->get_id().get_val();
    int subn = b.get_id().get_val().size();
    stringx newid( id.get_val() + nodename.substr( subn, nodename.length()-subn ) );
    // make NEW instance of entity
    entity* newent = ent->make_instance( entity_id(newid.c_str()), flags );
    newent->set_rel_po( ent->get_rel_po() );
    newent->set_ext_flag(EFLAG_EXT_MEMBER, true);
    members.push_back( newent );

    // add entity to world
    switch ( newent->get_flavor() )
    {
      case ENTITY_ENTITY:
        if(newent->get_bone_idx() < 0)
          g_entity_maker->create_entity( newent );
        break;
      case ENTITY_PHYSICAL:
//        g_world_ptr->add_physent( static_cast<physical_entity*>(newent) );
        assert(0);
        break;
      case ENTITY_LIGHT_SOURCE:
        g_world_ptr->add_light_source( static_cast<light_source*>(newent) );
        break;
      case ENTITY_PARTICLE_GENERATOR:
        g_world_ptr->add_particle_generator( static_cast<particle_generator*>(newent) );
        break;
/*!      case ENTITY_LADDER:
        g_world_ptr->add_ladder( static_cast<ladder*>(newent) );
        break;
!*/
#if 0 // BIGCULL
      case ENTITY_SCANNER:
        g_world_ptr->add_scanner( static_cast<scanner*>(newent) );
        break;
#endif // BIGCULL
      default:
        break;
    }

  }

  names = b.names;

  entity::copy_instance_data( b );

  if (b.lightmgr)
  {
    lightmgr = NEW light_manager;
    lightmgr->max_lights = max_lights;
    set_flag( EFLAG_ACTIVE, true );
  }

  // argh, parallel arrays make using iterators difficult
//#pragma todo( "parallel arrays => design flaw? -mkv 4/6/01" )
//#pragma todo( "often, parallel arrays are more cache friendly than arrays of structures.  Not in this case, though. -jdf")
  int j;

  for( j = 0; j < (int) members.size(); ++j )
  {
    char parent_idx = parents[j];

    // we want to check the *entity's* "has_skeleton",
    // not use the conglomerates "has_skeleton"
    if (members[j]->get_bone_idx() >= 0)
      members[j]->link_ifc()->link_does_not_manage_abs_po();

    if ( parent_idx == (char) 0xFF )
      members[j]->link_ifc()->set_parent( this );
    else
      members[j]->link_ifc()->set_parent( members[parent_idx] );

  }

  bool has_skeleton = num_mesh_bones() ? true : false;

  // Now that we have the entire conglomerate loaded in, assign our skeleton abs_po buffer and
  // fix up the pointers to use it
  if ( has_skeleton )
  {
    if(!has_skeleton_ifc())
    {
      create_skeleton_ifc();
      skeleton_ifc()->initialize( num_mesh_bones() );
      assert( num_mesh_bones() );
    }
    if(!has_hard_attrib_ifc())
    {
      // allocate a sub-class for the attrib interfaces
      my_hard_attrib_interface = NEW character_hard_attrib_interface(this);
    }
    if(!has_soft_attrib_ifc())
    {
      // allocate a sub-class for the attrib interfaces
      my_soft_attrib_interface = NEW character_soft_attrib_interface(this);
    }

    for ( i = members.begin(); i != members.end(); ++i)
    {
      int k = (*i)->get_bone_idx();

      if( k >= 0 ) {
        skeleton_ifc()->connect_bone_abs_po(k, (bone*) (*i) );
      }

    }

  }

  if(get_colgeom() && get_colgeom()->get_type() == collision_geometry::CAPSULE)
  {
    capsule rel_cap;
    rel_cap.base = vector3d(0.0f, -0.25f, 0.0f);
    rel_cap.end = vector3d(0.0f, 0.4f, 0.0f);
    rel_cap.radius = 0.5f;

    collision_capsule *cap = (collision_capsule *)get_colgeom();
    cap->set_capsule(rel_cap);

    if(!has_physical_ifc())
      create_physical_ifc();

    physical_ifc()->enable();
    physical_ifc()->set_gravity(true);
    physical_ifc()->set_mass(1.0f);
    physical_ifc()->set_standing(true);

    set_stationary(false);
  }

#if 0 // BIGCULL
  if(b.has_damage_ifc())
  {
    if(!has_damage_ifc())
      create_damage_ifc();

    damage_ifc()->copy(b.damage_ifc());
  }
#endif // BIGCULL

  update_abs_po();
  compute_radius();
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////


void conglomerate::set_visible( bool a )
{
  entity::set_visible( a );
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->set_visible( a );
  }
}

bool conglomerate::is_still_visible() const
{
  if ( is_visible() )
    return true;
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->is_still_visible() )
      return true;
  }
  return false;
}


rational_t conglomerate::terrain_radius() const
{
  rational_t r = get_visual_radius();
  if ( r<0.1f && get_radius()>r )
    r = get_radius();
  return r;
}


void conglomerate::force_region( region_node* r )
{
  entity::force_region( r );
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->force_region( r );
  }
}


void conglomerate::force_current_region()
{
  entity::force_current_region();
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->force_current_region();
  }
}


void conglomerate::unforce_regions()
{
  entity::unforce_regions();
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->unforce_regions();
  }
}

#include <algorithm>

void conglomerate::add_member( entity* ent, const stringx& nodename )
{
  members.push_back( ent );
  names.push_back( nodename );

  ent->set_ext_flag(EFLAG_EXT_MEMBER, true);

  char pidx = -1;
  assert( ent->has_parent() );
  pentity_vector::iterator ei;
  ei = find( members.begin(), members.end(), ent->link_ifc()->get_parent() );
  if ( ei != members.end() )
    pidx = ei - members.begin();
  parents.push_back( pidx );
}


// get pointer to member by node name (found by reconstructing member id from
// conglomerate id plus node name)
entity* conglomerate::get_member( const stringx& nodename )
{
  //stringx searchname = nodename;
  //searchname.to_upper();
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  vector<stringx>::const_iterator j = names.begin();
  for ( ; i!=i_end; ++i,++j )
  {
    if ( *j == nodename )
      return *i;
  }
  // not found
  return NULL;
}

const stringx& conglomerate::get_member_nodename( entity *member )
{
  if ( member != NULL )
  {
    pentity_vector::const_iterator i = members.begin();
    pentity_vector::const_iterator i_end = members.end();
    vector<stringx>::const_iterator j = names.begin();
    for ( ; i!=i_end; ++i,++j )
    {
      entity* e = *i;
      if ( e == member )
        return *j;
    }
  }
  return empty_string;
}

bool conglomerate::has_member(entity *ent) const
{
  if ( ent != NULL )
  {
    pentity_vector::const_iterator i = members.begin();
    pentity_vector::const_iterator i_end = members.end();
    for ( ; i!=i_end; ++i )
    {
      entity* e = *i;
      if ( e == ent )
        return true;
    }
  }
  return false;
}


entity* conglomerate::get_member_by_flavor( entity_flavor_t flav )
{
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_flavor() == flav )
      return e;
  }
  return NULL;
}

void conglomerate::compute_sector( terrain& ter, bool use_high_res_intersect )
{
  entity::compute_sector( ter, use_high_res_intersect );

  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->update_region( true );
//      e->compute_sector( ter, use_high_res_intersect );
  }
}

void conglomerate::compute_bounding_box()
{
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
    {
      if ( e->is_statically_sortable() )
        e->compute_bounding_box();
    }
  }
}


// *********** ifl operations *************
void conglomerate::ifl_lock(int frame_number)
{
  entity::ifl_lock(frame_number);
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->ifl_lock( frame_number );
  }
}


void conglomerate::ifl_play()
{
  entity::ifl_play();
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->ifl_play();
  }
}


void conglomerate::ifl_pause()
{
  entity::ifl_pause();
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->ifl_pause();
  }
}

void conglomerate::set_render_color( const color32 new_color )
{
  entity::set_render_color( new_color );
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->get_bone_idx() < 0 )
      e->set_render_color( new_color );
  }
}

// *****************************************


bool conglomerate::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  // optional entity flags
  if ( label == "members:" )
  {
    for( serial_in(fs, &label); label != chunkend_label; serial_in(fs, &label) )
    {
      if( label == "member" )
      {
        stringx entname;
        serial_in( fs, &entname );
        entname.to_upper();
        entity *pEnt = get_member( entname );
        if ( pEnt == NULL )
          error( fs.get_name() + ": conglomerate " + get_name() + " does not have a member " + entname );
        for( serial_in(fs, &label); label != chunkend_label; serial_in(fs, &label) )
        {
          if ( label == "flags:" )
          {
            for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
            {
              label.to_lower();
              if ( label == "cast_shadow" )
              {
                pEnt->set_flag( EFLAG_MISC_CAST_SHADOW, true );
              }
              else if ( label == "scanable" )
              {
                if ( pEnt->has_mesh() )
                  pEnt->set_scannable( true );
                else
                  warning( fs.get_name() + ": " + pEnt->get_id().get_val() + ": entity must have mesh visrep to be set as SCANABLE" );
              }
              else if ( label == "beamable" )
              {
                pEnt->set_beamable( true );
              }
              else if ( label == "camera_collision" )
              {
                if ( !pEnt->get_colgeom() || pEnt->get_colgeom()->get_type()!=collision_geometry::MESH )
                  error( pEnt->get_id().get_val() + ": error reading " + fs.get_name() + ": cannot apply collision flags to an entity without a collision mesh" );
                cg_mesh* m = static_cast<cg_mesh*>( pEnt->get_colgeom() );
                m->set_flag( cg_mesh::FLAG_CAMERA_COLLISION, true );
              }
              if ( label == "hide" )
              {
                pEnt->set_visible( false );
              }
            }
          }
          else if ( label == "min_detail:" )
          {
            int mindetaillvl;
            serial_in( fs, &mindetaillvl );
            pEnt->set_min_detail(mindetaillvl);
            for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
            {
            }
          }
          else if ( label == "scanner_info:" )
          {
            error ("Scanners not supported in KS");
#if 0 // BIGCULL
            if ( pEnt->get_flavor() != ENTITY_SCANNER )
              error( fs.get_name() + ": scanner_info: " + pEnt->get_id().get_val() + " is not a scanner" );
            scanner* e = static_cast<scanner*>( pEnt );
            for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
            {
              label.to_lower();
              if ( label == "scan_offset" )
              {
                vector3d v;
                serial_in( fs, &v );
                e->set_sheet_offset( v );
              }
            }
#endif // BIGCULL
          }
          else if ( !pEnt->handle_enx_chunk(fs,label) )
            return false;
        }
      }
    }

    return true;
  }

  return entity::handle_enx_chunk( fs, label );
}

void conglomerate::set_min_detail(int md)
{
  entity::set_min_detail(md);
  for (pentity_vector::iterator it = members.begin();
       it != members.end();
       ++it)
  {
    (*it)->set_min_detail(md);
  }
}

void conglomerate::apply_destruction_fx()
{
  entity::apply_destruction_fx();

  pentity_vector::iterator i;
  for ( i=members.begin(); i!=members.end(); ++i )
  {
    if((*i)->get_bone_idx() < 0)
      (*i)->apply_destruction_fx();
  }
}


entity *conglomerate::get_child( entity *ent, entity *prev_child )
{
  if(ent != NULL)
  {
    pentity_vector::iterator it;

    if(prev_child != NULL)
    {
      it = find( members.begin(), members.end(), prev_child);

      // skip the prev_child if found
      if(it != members.end())
        ++it;
    }
    else
      it = members.begin();

    while( it != members.end() )
    {
      if((*it)->link_ifc()->get_parent() == ent)
        return((*it));

      ++it;
    }
  }

  return NULL;
}


void conglomerate::updatelighting( time_value_t t, const int playerID )
{
  if (lightmgr)
  {
    region_node* my_region_node = get_region();
    region* my_region = my_region_node ? my_region_node->get_data() : 0;
    lightmgr->set_bound_sphere(sphere(get_visual_center(),get_visual_radius()));
    lightmgr->frame_advance(my_region, t, playerID); // let it update light blending/fading
  }
}


void conglomerate::frame_advance(time_value_t t)
{
#if defined(TARGET_XBOX)
  assert( t > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

	int heroNum = 0;
	heroNum = get_hero_id();
	updatelighting(t,heroNum);

  entity::frame_advance(t);

#if 0

  if(0 && get_colgeom() && get_colgeom()->get_type() == collision_geometry::CAPSULE)
  {
    capsule rel_cap;
    entity *ent = this;
    entity *waist = this;
    entity *neck = NULL;

    pentity_vector::iterator it = members.begin();
    while( it != members.end() )
    {

/*
      entity *member = (*it);

      if(member->get_anim_id() == anim_id_manager::inst()->anim_id( "HEAD" ))
        rel_cap.end = member->get_abs_position() - get_abs_position();
      if(member->get_anim_id() == anim_id_manager::inst()->anim_id( "LEFT_FOOT" ))
        rel_cap.base += -(member->get_abs_position() - get_abs_position());
      if(member->get_anim_id() == anim_id_manager::inst()->anim_id( "RIGHT_FOOT" ))
        rel_cap.base += -(member->get_abs_position() - get_abs_position());
*/
//      if((*it)->get_anim_id() == anim_id_manager::inst()->anim_id( "WAIST" ))
//        waist = ent = (*it);
      if((*it)->get_anim_id() == anim_id_manager::inst()->anim_id( "HEAD" ))
        neck = (*it);

      ++it;
    }

/*
    rel_cap.base *= 0.5f;

    vector3d dir = rel_cap.end - rel_cap.base;

    assert(dir.length() > 0);
    dir.normalize();

    rel_cap.base -= dir * 0.4f;
    rel_cap.end += dir * 0.4f;
*/

    vector3d dir = (neck != NULL && waist != NULL) ? neck->get_abs_position() - waist->get_abs_position() : YVEC;
    dir.normalize();
    rel_cap.base = dir * -0.25f;
    rel_cap.end = dir * 0.4f;

    rel_cap.radius = 0.5f;

    collision_capsule *cap = (collision_capsule *)get_colgeom();
    cap->set_capsule(rel_cap);
  }
#endif
}


light_manager* conglomerate::get_light_set() const
{
  if (lightmgr) return lightmgr;
  return entity::get_light_set();
}

static instance_render_info iri;

bool loresmodelbydefault=false;


void conglomerate::render( camera* camera_link, rational_t detail, render_flavor_t render_flavor, rational_t entity_translucency_pct )
{
//  static matrix4x4* bones = (matrix4x4*)os_malloc32x(sizeof(matrix4x4)*MAX_BONES);

//  conglom_bones.reserve(MAX_BONES);
//  conglom_bones.resize(0);

//	  int start_polys = hw_rasta::inst()->get_poly_count();

  if ( !members.empty() && is_visible() )
  {
#ifdef NGL
		float camdist=0.0f;
		if ( camera_link )
		{
			vector3d rel_cam_pos=get_rel_position() - camera_link->get_rel_position();
			camdist=rel_cam_pos.length();
		}

	  nglMesh* mesh=NULL;
	  if (render_flavor & RENDER_SHADOW_MODEL)
		  mesh = get_shadow_mesh();
		if (g_game_ptr->is_splitscreen() ||
		    !force_hi_res && (camdist> 3.0f ||
				loresmodelbydefault ||
				(render_flavor & RENDER_LORES_MODEL)))
		  mesh = get_lores_mesh();
		if ( mesh==NULL )
	  	mesh = get_mesh();
	  if ( !mesh )
	    return;

		const po *ptr_to_bones = NULL;
    if( has_skeleton_ifc() )
    {
		// We need to correct the bounding sphere when the character's animation has a 
		// large offset in it.  Otherwise, two things can go wrong.  The character can
		// get clipped even when he's entirely on screen (this is corrected by hacks 
		// elsewhere as well).  Or, the clipping might be entirely optimized out, because 
		// NGL is sure he's completely on screen.  In this case, you can see strange 
		// "wraparound" artifacts from overflowing screenspace.  (dc 07/11/02)
		bone *pelv_bone = this->get_member("BIP01 PELVIS");
		if (pelv_bone)
		{
			vector3d offset = pelv_bone->get_abs_position() - get_abs_position();
			if (my_mesh) 
			{
				my_mesh->SphereCenter.x = offset.x;
				my_mesh->SphereCenter.y = offset.y;
				my_mesh->SphereCenter.z = offset.z;
			}
			if (lores_mesh) 
			{
				lores_mesh->SphereCenter.x = offset.x;
				lores_mesh->SphereCenter.y = offset.y;
				lores_mesh->SphereCenter.z = offset.z;
			}
		}
		// skin and give ambient factor with TINT
		nglRenderParams params;
		memset(&params,0,sizeof(params));

		if (MaterialMask)
		{
			params.Flags |= NGLP_MATERIAL_MASK;
			params.MaterialMask = MaterialMask;
		}

		if (TextureFrame >= 0)
		{
			params.Flags |= NGLP_TEXTURE_FRAME;
			params.TextureFrame = TextureFrame;
		}

		if (!cull_entity)
			params.Flags |= NGLP_NO_CULLING;

		if ((flip_axis >= 0) && (flip_axis <= 2))
		{
			ptr_to_bones = skeleton_ifc()->get_ptr_to_handed_bones();
			params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// conglom is rendered flipped (dc 04/30/02)
		}
		else
			ptr_to_bones = skeleton_ifc()->get_ptr_to_bones();

		params.Flags |= (NGLP_BONES | NGLP_BONES_WORLD);
		params.NBones = skeleton_ifc()->bone_count();
		params.Bones = (nglMatrix*)ptr_to_bones;

		assert(params.NBones);
		assert(params.Bones);

		color amb = get_light_set()->last_ambient;
		amb = amb * get_render_color().to_color();
		if ( amb.to_color32() != color32_white )
		{
			params.Flags |= NGLP_TINT;
			params.TintColor[0] = amb.get_red();
			params.TintColor[1] = amb.get_green();
			params.TintColor[2] = amb.get_blue();
			params.TintColor[3] = amb.get_alpha();
		}
		vector3d s = get_render_scale();
		// Without NGLP_SCALE, transform will be normalized, which removes the flip for lefties.  (dc 05/31/02)
//		if ( s != vector3d( 1, 1, 1 ) )
		{
			params.Flags |= NGLP_SCALE;
			params.Scale[0] = s.x;
			params.Scale[1] = s.y;
			params.Scale[2] = s.z;
			params.Scale[3] = 1.0f;
			if (s.x * s.y * s.z < 0)
			{
				params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// conglom is rendered flipped (dc 04/30/02)
			}
		}

		params.Flags |= NGLP_LIGHT_CONTEXT;
		params.LightContext = g_world_ptr->get_current_light_context();
		assert(params.LightContext != NULL);
		
		START_PROF_TIMER( proftimer_render_add_mesh );
		nglListAddMesh (mesh, native_to_ngl( get_abs_po() ), &params );
		STOP_PROF_TIMER( proftimer_render_add_mesh );
    }
    else
    {
		nglRenderParams params;
		memset(&params,0,sizeof(params));

		// enable tint if neccessary.  duplicated in entity::render.
		if ( get_render_color() != color32_white )
		{
			params.Flags |= NGLP_TINT;
			color c = get_render_color().to_color();
			params.TintColor[0] = c.get_red();
			params.TintColor[1] = c.get_green();
			params.TintColor[2] = c.get_blue();
			params.TintColor[3] = c.get_alpha();
		}
		vector3d s = get_render_scale();
		// Without NGLP_SCALE, transform will be normalized, which removes the flip for lefties.  (dc 05/31/02)
//		if ( s != vector3d( 1, 1, 1 ) )
		{
			params.Flags |= NGLP_SCALE;
			params.Scale[0] = s.x;
			params.Scale[1] = s.y;
			params.Scale[2] = s.z;
			params.Scale[3] = 1.0f;
			if (s.x * s.y * s.z < 0)
			{
				params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// conglom is rendered flipped (dc 04/30/02)
			}
		}

		params.Flags |= NGLP_LIGHT_CONTEXT;
		params.LightContext = g_world_ptr->get_current_light_context();
		assert(params.LightContext != NULL);
		
		START_PROF_TIMER( proftimer_render_add_mesh );
		nglListAddMesh( mesh, native_to_ngl( get_abs_po() ), &params );
		STOP_PROF_TIMER( proftimer_render_add_mesh );
    }
#else // NGL
    // draw hero entirely in translucent pass
    //if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_SINGLE_MESH ) )
    if(my_visrep != NULL)
    {
      bool need_translucency = (entity_translucency_pct<=0.99f);
      // this test only works because this render function wouldn't have been called at
      // all if we didn't request a pass of this type.  So we should have something to
      // draw here for this pass.
      if ((render_flavor&RENDER_TRANSLUCENT_PORTION) || !need_translucency)
      {
//          int num_bones = 0;
        // refresh them in case there are
        // some bones without bodies
        light_manager* light_set = get_light_set();

        uint32 forceflags = 0;
        if ( !(render_flavor&RENDER_CLIPPED_FULL_DETAIL) )
          forceflags |= FORCE_SKIP_CLIP;

        color32 scalecolor = get_render_color();
        if ( need_translucency )
        {
          forceflags |= FORCE_TRANSLUCENCY;
          float pct = entity_translucency_pct;
          uint8 pctbyte = (uint8)(pct * 255);
          if ( (forceflags&FORCE_ADDITIVE_BLENDING) != 0 )
          {
            scalecolor.c.r = pctbyte;
            scalecolor.c.g = pctbyte;
            scalecolor.c.b = pctbyte;
          }
          else
            scalecolor.c.a = pctbyte;
        }
/*
        pentity_vector::iterator i;
        for ( i=members.begin(); i!=members.end(); ++i )
          add_members_to_bones((*i), bones, &num_bones );
*/
        iri = instance_render_info( detail,
                                  get_abs_po(),    //  * I'd like to avoid these copies <<<<
                                  frame_time_info,
                                  get_region(),
                                  scalecolor,
                                  forceflags,
                                  0,  // camera relative rotation (billboards only, doesn't apply)
                                  1.0f, // particle scale, again doesn't apply
                                  light_set,
                                  get_alternative_materials() );

        my_visrep->render_skin( render_flavor,
          &iri, skeleton_ifc()->get_ptr_to_bones(), skeleton_ifc()->bone_count()); // bones, num_bones );

        #pragma todo("Following is a CHEAP hack at 'real' shadows. Remove after real shadows are done...")
        if(has_ai_ifc() && ai_ifc()->has_adv_dropshadow() && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_SHADOWS))
        {
          bool alpha = false;
          if(render_flavor & RENDER_OPAQUE_PORTION || !(render_flavor & RENDER_TRANSLUCENT_PORTION))
          {
            hw_rasta::inst()->send_start( hw_rasta::PT_TRANS_POLYS );
            render_flavor |= RENDER_TRANSLUCENT_PORTION;
            render_flavor &= ~RENDER_OPAQUE_PORTION;
            alpha = true;
          }

          extern bool g_ignore_nonuniform_scaling;
          g_ignore_nonuniform_scaling = true;

          scalecolor.set_red(0);
          scalecolor.set_green(0);
          scalecolor.set_blue(0);

          forceflags |= FORCE_TRANSLUCENCY;

          po the_po = get_abs_po();
          the_po.set_position(ZEROVEC);

          po scale_po;
          vector3d scale_vec = vector3d(1.0f, 0.01f, 1.0f);
          scale_po.set_scale(scale_vec);

          fast_po_mul(the_po, the_po, scale_po);

          vector3d pos = get_abs_position();
          vector3d end = pos - (YVEC*1000.0f);
          vector3d hit, hitn;

          bool col_active = are_collisions_active();
          set_collisions_active(false, false);
          bool intersect = find_intersection(pos, end, get_primary_region(), FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY, &hit, &hitn);
          set_collisions_active(col_active, false);

          if(intersect)
            pos = hit + (hitn*0.01f);
          else
          {
            pos.y = get_abs_position().y;
            assert(0);
          }

          the_po.set_position(pos);

          po old_rel_po = get_rel_po();


          if(has_parent())
            fast_po_mul(the_po, the_po, link_ifc()->get_parent()->get_abs_po().inverse());
          set_rel_po(the_po);


          bool fix_z = false;//is_hero();
          if(fix_z)
          {
            update_abs_po();
            scalecolor.set_alpha(0);
            iri = instance_render_info( detail,
                                      the_po,    //  * I'd like to avoid these copies <<<<
                                      frame_time_info,
                                      get_region(),
                                      scalecolor,
                                      forceflags,
                                      0,  // camera relative rotation (billboards only, doesn't apply)
                                      1.0f, // particle scale, again doesn't apply
                                      light_set,
                                      get_alternative_materials() );

            hw_rasta::inst()->set_zbuffering(true, true);
            my_visrep->render_skin( render_flavor,
              &iri, skeleton_ifc()->get_ptr_to_bones(), skeleton_ifc()->bone_count()); // bones, num_bones );
            hw_rasta::inst()->set_zbuffering(true, false);
          }

          update_abs_po();
          scalecolor.set_alpha(96);
          iri = instance_render_info( detail,
                                    the_po,    //  * I'd like to avoid these copies <<<<
                                    frame_time_info,
                                    get_region(),
                                    scalecolor,
                                    forceflags,
                                    0,  // camera relative rotation (billboards only, doesn't apply)
                                    1.0f, // particle scale, again doesn't apply
                                    light_set,
                                    get_alternative_materials() );

          my_visrep->render_skin( render_flavor,
            &iri, skeleton_ifc()->get_ptr_to_bones(), skeleton_ifc()->bone_count()); // bones, num_bones );


          set_rel_po(old_rel_po);
          update_abs_po();
          g_ignore_nonuniform_scaling = false;

          if(alpha)
            hw_rasta::inst()->send_start( hw_rasta::PT_OPAQUE_POLYS );
        }
      }
    }
#endif // NGL
  }
}

bool zerotints=true;

int maxshadowbones=40;
bool shadowhasbones=true;

extern float shadow_reflective_value;

void conglomerate::rendershadow( camera* camera_link, rational_t detail, render_flavor_t render_flavor, rational_t entity_translucency_pct, rational_t scale )
{

//  static matrix4x4* bones = (matrix4x4*)os_malloc32x(sizeof(matrix4x4)*MAX_BONES);

//  conglom_bones.reserve(MAX_BONES);
//  conglom_bones.resize(0);

//	  int start_polys = hw_rasta::inst()->get_poly_count();

  if ( !members.empty()
//	  && is_visible()	// want to render shadow without main entity, for debugging (dc 06/23/02)
)
  {
#ifdef NGL
		float camdist=0.0f;
		if ( camera_link )
		{
			vector3d rel_cam_pos=get_abs_position() - camera_link->get_abs_position();
			camdist=rel_cam_pos.length();
		}

	  nglMesh* mesh=NULL;
	  if (render_flavor & RENDER_SHADOW_MODEL)
		  mesh = get_shadow_mesh();
	  if ( (render_flavor & RENDER_LORES_MODEL) )
		  mesh = get_lores_mesh();
		if ( mesh==NULL )
	  	mesh = get_mesh();
	  if ( !mesh )
	    return;

		u_int oflags=mesh->Flags;
//		mesh->Flags=((oflags & (~NGL_LIGHTCAT_MASK) & (~NGLMESH_PERFECT_TRICLIP)) | NGLMESH_REJECT_TRICLIP);

		const po *ptr_to_bones = NULL;
    if( has_skeleton_ifc() )
    {
		// skin and give ambient factor with TINT
		nglRenderParams params;
		memset(&params,0,sizeof(params));

		if (MaterialMask)
		{
			params.Flags |= NGLP_MATERIAL_MASK;
			params.MaterialMask = MaterialMask;
		}

		if (TextureFrame >= 0)
		{
			params.Flags |= NGLP_TEXTURE_FRAME;
			params.TextureFrame = TextureFrame;
		}

		if ((flip_axis >= 0) && (flip_axis <= 2))
		{
			ptr_to_bones = skeleton_ifc()->get_ptr_to_handed_bones();
			params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// conglom is rendered flipped (dc 04/30/02)
		}
		else
			ptr_to_bones = skeleton_ifc()->get_ptr_to_bones();

		params.Flags |= (NGLP_BONES | NGLP_BONES_WORLD);
		params.NBones = skeleton_ifc()->bone_count();
		params.Bones = (nglMatrix*)ptr_to_bones;

		//color amb = get_light_set()->last_ambient;
		//amb = amb * get_render_color().to_color();

		params.Flags |= NGLP_TINT;
		params.TintColor[0] = shadow_reflective_value; //amb.get_red();
		params.TintColor[1] = shadow_reflective_value; //amb.get_green();
		params.TintColor[2] = shadow_reflective_value; //amb.get_blue();
		params.TintColor[3] = entity_translucency_pct;
		// Without NGLP_SCALE, transform will be normalized, which removes the flip for lefties.  (dc 05/31/02)
//		if ( scale != 1.0f )
		{
			params.Flags |= NGLP_SCALE;
			params.Scale[0] = scale;
			params.Scale[1] = scale;
			params.Scale[2] = scale;
			params.Scale[3] = 1.0f;
			if (scale < 0)
			{
				params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// conglom is rendered flipped (dc 04/30/02)
			}
		}
		params.Flags |= NGLP_NO_CULLING;
		params.Flags |= NGLP_NO_LIGHTING;
#ifdef TARGET_PS2
		params.Flags |= NGLP_WRITE_FB_ALPHA;
#endif

		START_PROF_TIMER( proftimer_render_add_mesh );
		nglListAddMesh (mesh, native_to_ngl( get_abs_po() ), &params );
		STOP_PROF_TIMER( proftimer_render_add_mesh );
    }
    else
    {
		nglRenderParams params;
		memset(&params,0,sizeof(params));

		// Without this, transform will be normalized, which removes the flip for lefties.  (dc 05/31/02)
		params.Flags = NGLP_SCALE;
		params.Scale = nglVector(1, 1, 1, 1);

		params.Flags |= NGLP_TINT;
		params.TintColor[0] = shadow_reflective_value; //c.get_red();
		params.TintColor[1] = shadow_reflective_value; //c.get_green();
		params.TintColor[2] = shadow_reflective_value; //c.get_blue();
		params.TintColor[3] = entity_translucency_pct;

		vector3d s = get_render_scale();
		// Without NGLP_SCALE, transform will be normalized, which removes the flip for lefties.  (dc 05/31/02)
//		if ( s != vector3d( 1, 1, 1 ) )
		{
			params.Flags |= NGLP_SCALE;
			params.Scale[0] = s.x;
			params.Scale[1] = s.y;
			params.Scale[2] = s.z;
			params.Scale[3] = 1.0f;
			if (s.x * s.y * s.z < 0)
			{
				params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// conglom is rendered flipped (dc 04/30/02)
			}
		}

		params.Flags |= NGLP_NO_CULLING;
		params.Flags |= NGLP_NO_LIGHTING;
#ifdef TARGET_PS2
		params.Flags |= NGLP_WRITE_FB_ALPHA;
#endif

		START_PROF_TIMER( proftimer_render_add_mesh );
		nglListAddMesh( mesh, native_to_ngl( get_abs_po() ), &params );
		STOP_PROF_TIMER( proftimer_render_add_mesh );
    }
		mesh->Flags=oflags;


#else // NGL
    // draw hero entirely in translucent pass
    //if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_SINGLE_MESH ) )
    if(my_visrep != NULL)
    {
      bool need_translucency = (entity_translucency_pct<=0.99f);
      // this test only works because this render function wouldn't have been called at
      // all if we didn't request a pass of this type.  So we should have something to
      // draw here for this pass.
      if ((render_flavor&RENDER_TRANSLUCENT_PORTION) || !need_translucency)
      {
//          int num_bones = 0;
        // refresh them in case there are
        // some bones without bodies
        light_manager* light_set = get_light_set();

        uint32 forceflags = 0;
        if ( !(render_flavor&RENDER_CLIPPED_FULL_DETAIL) )
          forceflags |= FORCE_SKIP_CLIP;

        color32 scalecolor = get_render_color();
        if ( need_translucency )
        {
          forceflags |= FORCE_TRANSLUCENCY;
          float pct = entity_translucency_pct;
          uint8 pctbyte = (uint8)(pct * 255);
          if ( (forceflags&FORCE_ADDITIVE_BLENDING) != 0 )
          {
            scalecolor.c.r = pctbyte;
            scalecolor.c.g = pctbyte;
            scalecolor.c.b = pctbyte;
          }
          else
            scalecolor.c.a = pctbyte;
        }
/*
        pentity_vector::iterator i;
        for ( i=members.begin(); i!=members.end(); ++i )
          add_members_to_bones((*i), bones, &num_bones );
*/
        iri = instance_render_info( detail,
                                  get_abs_po(),    //  * I'd like to avoid these copies <<<<
                                  frame_time_info,
                                  get_region(),
                                  scalecolor,
                                  forceflags,
                                  0,  // camera relative rotation (billboards only, doesn't apply)
                                  1.0f, // particle scale, again doesn't apply
                                  light_set,
                                  get_alternative_materials() );

        my_visrep->render_skin( render_flavor,
          &iri, skeleton_ifc()->get_ptr_to_bones(), skeleton_ifc()->bone_count()); // bones, num_bones );

        #pragma todo("Following is a CHEAP hack at 'real' shadows. Remove after real shadows are done...")
        if(has_ai_ifc() && ai_ifc()->has_adv_dropshadow() && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_SHADOWS))
        {
          bool alpha = false;
          if(render_flavor & RENDER_OPAQUE_PORTION || !(render_flavor & RENDER_TRANSLUCENT_PORTION))
          {
            hw_rasta::inst()->send_start( hw_rasta::PT_TRANS_POLYS );
            render_flavor |= RENDER_TRANSLUCENT_PORTION;
            render_flavor &= ~RENDER_OPAQUE_PORTION;
            alpha = true;
          }

          extern bool g_ignore_nonuniform_scaling;
          g_ignore_nonuniform_scaling = true;

          scalecolor.set_red(0);
          scalecolor.set_green(0);
          scalecolor.set_blue(0);

          forceflags |= FORCE_TRANSLUCENCY;

          po the_po = get_abs_po();
          the_po.set_position(ZEROVEC);

          po scale_po;
          vector3d scale_vec = vector3d(1.0f, 0.01f, 1.0f);
          scale_po.set_scale(scale_vec);

          fast_po_mul(the_po, the_po, scale_po);

          vector3d pos = get_abs_position();
          vector3d end = pos - (YVEC*1000.0f);
          vector3d hit, hitn;

          bool col_active = are_collisions_active();
          set_collisions_active(false, false);
          bool intersect = find_intersection(pos, end, get_primary_region(), FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY, &hit, &hitn);
          set_collisions_active(col_active, false);

          if(intersect)
            pos = hit + (hitn*0.01f);
          else
          {
            pos.y = get_abs_position().y;
            assert(0);
          }

          the_po.set_position(pos);

          po old_rel_po = get_rel_po();


          if(has_parent())
            fast_po_mul(the_po, the_po, link_ifc()->get_parent()->get_abs_po().inverse());
          set_rel_po(the_po);


          bool fix_z = false;//is_hero();
          if(fix_z)
          {
            update_abs_po();
            scalecolor.set_alpha(0);
            iri = instance_render_info( detail,
                                      the_po,    //  * I'd like to avoid these copies <<<<
                                      frame_time_info,
                                      get_region(),
                                      scalecolor,
                                      forceflags,
                                      0,  // camera relative rotation (billboards only, doesn't apply)
                                      1.0f, // particle scale, again doesn't apply
                                      light_set,
                                      get_alternative_materials() );

            hw_rasta::inst()->set_zbuffering(true, true);
            my_visrep->render_skin( render_flavor,
              &iri, skeleton_ifc()->get_ptr_to_bones(), skeleton_ifc()->bone_count()); // bones, num_bones );
            hw_rasta::inst()->set_zbuffering(true, false);
          }

          update_abs_po();
          scalecolor.set_alpha(96);
          iri = instance_render_info( detail,
                                    the_po,    //  * I'd like to avoid these copies <<<<
                                    frame_time_info,
                                    get_region(),
                                    scalecolor,
                                    forceflags,
                                    0,  // camera relative rotation (billboards only, doesn't apply)
                                    1.0f, // particle scale, again doesn't apply
                                    light_set,
                                    get_alternative_materials() );

          my_visrep->render_skin( render_flavor,
            &iri, skeleton_ifc()->get_ptr_to_bones(), skeleton_ifc()->bone_count()); // bones, num_bones );


          set_rel_po(old_rel_po);
          update_abs_po();
          g_ignore_nonuniform_scaling = false;

          if(alpha)
            hw_rasta::inst()->send_start( hw_rasta::PT_OPAQUE_POLYS );
        }
      }
    }
#endif // NGL
  }

}
/*
void conglomerate::add_members_to_bones( entity* ent, matrix4x4* bones, int* num_bones )
{
  int bone_idx = ent->get_bone_idx();
  assert( bone_idx < MAX_BONES );

  if(bone_idx >= 0)
  {
    matrix4x4* bone_m4x4 = &bones[bone_idx];
    *(po*)bone_m4x4 = ent->get_abs_po();

    // and that's that
    if( (bone_idx+1)>*num_bones)
      *num_bones = bone_idx+1;
  }
}
*/
void conglomerate::frame_done_including_members()
{
  entity::frame_done();

  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    if ( e->is_a_conglomerate() )
      ((conglomerate *)e)->frame_done_including_members();
    else
      e->frame_done();
  }
}


///////////////////////////////////////////////////////////////////////////////
// entity_maker caching interface

void conglomerate::acquire( unsigned int _flags )
{
  entity::acquire( _flags );
  pentity_vector::iterator i = members.begin();
  pentity_vector::iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    assert( e->get_entity_pool() != NULL );
    *i = e->get_entity_pool()->acquire( _flags );
  }
  i = members.begin();
  vector<char>::const_iterator j = parents.begin();
  for ( ; i!=i_end; ++i,++j )
  {
    entity* e = *i;
    unsigned char pidx = (unsigned char)(*j);
    if ( pidx == 0xFF )
      e->link_ifc()->set_parent( this );
    else
      e->link_ifc()->set_parent( members[pidx] );
  }
}

void conglomerate::release()
{
  entity::release();
  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    g_entity_maker->release_entity( e );
//!   e->set_parent_rel( NULL );
    e->link_ifc()->clear_parent();
  }
}


void conglomerate::set_ext_flag_recursive(register unsigned int f, register bool set)
{
  entity::set_ext_flag_recursive(f, set);

  pentity_vector::const_iterator i = members.begin();
  pentity_vector::const_iterator i_end = members.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    e->set_ext_flag_recursive(f, set);
  }
}
