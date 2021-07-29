////////////////////////////////////////////////////////////////////////////////
/*
  visrep

  base class for physical_entities 3D visual representations,
  plus functions for managing memory storage
*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "visrep.h"

#include "pmesh.h"
#include "billboard.h"
#include "oserrmsg.h"
#include "iri.h"


visual_rep::visual_rep( visrep_t _type, bool _instanced )
  : type(_type),
    min_detail_dist(12), max_detail_dist(1), instanced(_instanced)
{}

void visual_rep::render_skin( render_flavor_t render_flavor,
                          const instance_render_info* iri,
                          const po* bones,
                          int num_bones )
{}

void visual_rep::render_batch(render_flavor_t flavor,
                            instance_render_info* viri,
                            int num_instances)
{
  for ( ; --num_instances>=0; ++viri)
    render_instance(flavor,viri);
}


rational_t visual_rep::compute_xz_radius_rel_center( const po& xform ) { return 0; }

int visual_rep::get_min_faces(time_value_t delta_t) const { return 1; }
int visual_rep::get_max_faces(time_value_t delta_t) const { return 1; }


void visual_rep::set_distance_fade_ok(bool v)
{}

bool visual_rep::get_distance_fade_ok() const
{
  return true;
}

float visual_rep::time_value_to_frame( time_value_t t ) { return t*30.0F; }

bool visual_rep::kill_me() { return false; }

void visual_rep::set_light_method( light_method_t lmt ) {}


int visual_rep::get_anim_length() const
{
  return 1;
}

render_flavor_t visual_rep::render_passes_needed() const
{
  return 0;
}

// utility function for turning file name into visual_rep type:
visrep_t visual_rep_name_to_type( const stringx& visrep_name )
{
  stringx lower_name( visrep_name );
  lower_name.to_lower( );

  if( ( lower_name.find( ".tpm" ) != (int) string::npos) ||      // keeping TPM for backwards compatibility
      ( lower_name.find( ".txtmesh" ) != (int) string::npos ) ||
      ( lower_name.find( ".ter" ) != (int) string::npos ) )
  {
    return VISREP_PMESH;
  }
  else if( lower_name.find( ".tbb" ) != (int) string::npos )
  {
    return VISREP_BILLBOARD;
  }
  else
  {
    stringx composite = stringx( "Unknown 3D file extension: " ) + lower_name;
    error( composite.c_str( ) );
  }

  return static_cast<visrep_t>( -1 );
}


visual_rep* load_new_visual_rep( chunk_file& fs,
                                 const stringx& visrep_name,
                                 visrep_t _flavor,
                                 unsigned additional_flags )
{
  visual_rep * outval = 0;
  error_context::inst()->push_context(visrep_name);
  switch ( _flavor )
  {
    case VISREP_PMESH:
      outval = vr_pmesh_bank.new_instance( fs, visrep_name, additional_flags );
      break;
    case VISREP_BILLBOARD:
      outval = vr_billboard_bank.new_instance( fs, visrep_name, additional_flags );  // currently, additional flags is unused (9/27)
      break;
    default:
      assert(0);
  }
  error_context::inst()->pop_context();
  return outval;
}


// for loading visual_reps at the beginning of the level:
visual_rep* load_new_visual_rep( const stringx& visrep_name, unsigned additional_flags )
{
  visual_rep * outval = NULL;
  error_context::inst()->push_context(visrep_name);
  visrep_t flavor = visual_rep_name_to_type( visrep_name );
  if( flavor==VISREP_PMESH)
  {
    vr_pmesh* vr = vr_pmesh_bank.new_instance( visrep_name, additional_flags );
    outval = vr;
  }
  else if( flavor==VISREP_BILLBOARD )
  {
    outval = vr_billboard_bank.new_instance( visrep_name, 0 );   // currently, additional flags is unused (9/27)
  }
  else
  {
    assert(0);
  }
  error_context::inst()->pop_context();
  return outval;
}


visual_rep* new_visrep_instance( visual_rep* vrep )
{
  switch ( vrep->get_type() )
  {
    case VISREP_PMESH:
      return vr_pmesh_bank.new_instance( static_cast<vr_pmesh*>(vrep) );
    case VISREP_BILLBOARD:
      return vr_billboard_bank.new_instance( static_cast<vr_billboard*>(vrep) );
    default:
      assert(0);
  }
  assert(0);
  return NULL;
}

visual_rep* new_visrep_copy( visual_rep* vrep )
{
  switch ( vrep->get_type() )
  {
  case VISREP_PMESH:
//    return NEW vr_pmesh( *static_cast<vr_pmesh*>(vrep) );
    // copy constructor not implemented in pmesh.
    assert(0);
    break;
  case VISREP_BILLBOARD:
    return NEW vr_billboard( *static_cast<vr_billboard*>(vrep) );
  default:
    assert(0);
  }
  assert(0);
  return NULL;
}

void unload_visual_rep( visual_rep* discard )
{
  if(discard)  // maintaining C++ convention of being able to delete NULL
  {
    if (!discard->is_instanced())
    {
      delete discard;
    }
    else if( discard->get_type()==VISREP_PMESH)
    {
      vr_pmesh_bank.delete_instance( static_cast<vr_pmesh*>(discard) );
    }
    else if( discard->get_type()==VISREP_BILLBOARD)
    {
      vr_billboard_bank.delete_instance( static_cast<vr_billboard*>(discard) );
    }
    else
    {
      assert(false);
    }
  }
}

// for finding visual_reps mid-game, where you aren't allowed to
// do disk access.  This function fails if the visual_rep hasn't
// been loaded.
visual_rep* find_visual_rep( const stringx& visrep_name )
{
  visrep_t flavor = visual_rep_name_to_type( visrep_name );
  if ( flavor == VISREP_PMESH )
  {
    if ( vr_pmesh_bank.find_instance_by_filename( visrep_name ) )
      return vr_pmesh_bank.new_instance( visrep_name, 0 );
  }
  else
  {
    // use me when you wanna come
    if ( vr_billboard_bank.find_instance_by_filename( visrep_name ) )
      return vr_billboard_bank.new_instance( visrep_name, 0 );
  }
  stringx composite = stringx("3D Animation hasn't been loaded: ") + visrep_name;
  error( composite.c_str() );
  return NULL;
}

time_value_t visual_rep::get_ending_time() const
{
  return LARGE_TIME;
}


bool visual_rep::is_uv_animated() const
{
  return false;
}
