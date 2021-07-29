#include "global.h"
#if 0

#include "crate.h"
#include "terrain.h"


#define MAX_ERROR .01f


static crate  *for_stack = NULL;


rational_t round( rational_t arg )
{
  rational_t fl = floor(arg);

  if( arg - fl >= .5f ) return ceil( arg );
  else return fl;
}


bool is_axis_aligned( const vector3d& v, rational_t max_error )
{
  if( (is_approx(v.x, 1.0f, max_error) && is_approx(v.y, .0f, max_error) && is_approx(v.z, .0f, max_error)) ||
      (is_approx(v.x, -1.0f, max_error) && is_approx(v.y, .0f, max_error) && is_approx(v.z, .0f, max_error)) ||
      (is_approx(v.x, .0f, max_error) && is_approx(v.y, 1.0f, max_error) && is_approx(v.z, .0f, max_error)) ||
      (is_approx(v.x, .0f, max_error) && is_approx(v.y, -1.0f, max_error) && is_approx(v.z, .0f, max_error)) ||
      (is_approx(v.x, .0f, max_error) && is_approx(v.y, .0f, max_error) && is_approx(v.z, 1.0f, max_error)) ||
      (is_approx(v.x, .0f, max_error) && is_approx(v.y, .0f, max_error) && is_approx(v.z, -1.0f, max_error)) )
      return true;
  else return false;
}


void crate::check_po()
{
#ifdef DEBUG
/*
  // Check position
  vector3d      v = get_abs_position();
  if( !is_approx(v.x, round(v.x), MAX_ERROR) ||
      !is_approx(v.y, round(v.y), MAX_ERROR) ||
      !is_approx(v.z, round(v.z), MAX_ERROR) )
      error( "Misaligned crate position" );
*/
  // Check orientation
  matrix4x4     m = get_abs_po().get_matrix();
  for( int i = 0; i < 3; ++i )
  {
    vector3d v = m[i].get_xyz();
    if( !is_axis_aligned(v, MAX_ERROR) ) error( "Misaligned crate orientation" );
  }
#endif
}


void crate::set_area( const vector2d& upper_left, const vector2d& lower_right, const vector2d& area_offset )
{
  compute_sector( g_world_ptr->get_the_terrain() );
  if( get_region() == NULL ) error( get_name() + " is out of the world" );
  get_region()->get_data()->get_crates().push_back( static_cast<crate*>(this) );
  if( upper_left.x < lower_right.x && upper_left.y > lower_right.y )
  {
    u_left = upper_left;
    l_right = lower_right;
  }
  else if( upper_left.x > lower_right.x && upper_left.y < lower_right.y )
  {
    u_left = lower_right;
    l_right = upper_left;
  }
  else error( "Invalid crate area" );
  offset = area_offset;
}


void crate::init_variables()
{
  available = true;
  nether = NULL;
  set_walkable( true );
  set_stationary( true );
  pAnim = NULL;
  unstacking = stacking = false;
  push_anim_filename = NULL;
  stack_anim_filename = NULL;
  pull_anim_filename = NULL;
  unstack_anim_filename = NULL;
}


crate::crate( const entity_id& _id, entity_flavor_t _flavor, unsigned int _flags )
: entity( _id, _flavor, _flags )
{
  init_variables();
  check_po();
}

crate::crate( chunk_file& fs, const entity_id& _id, entity_flavor_t _flavor, unsigned int _flags )
: entity( fs, _id, _flavor, _flags )
{
  init_variables();
  check_po();
}


entity* crate::make_instance( const entity_id& _id, unsigned int _flags ) const
{
  crate* c = NEW crate( _id, ENTITY_CRATE, _flags );
  c->set_flag( EFLAG_MISC_NONSTATIC, true );
  c->copy_instance_data( *this );
  return (entity *)c;
}


void crate::copy_instance_data( const crate& b )
{
  entity::copy_instance_data( b );
  init_variables();

  if(b.push_anim_filename)
    push_anim_filename = NEW stringx( *b.push_anim_filename );

  if(b.stack_anim_filename)
    stack_anim_filename = NEW stringx( *b.stack_anim_filename );

  if(b.pull_anim_filename)
    pull_anim_filename = NEW stringx( *b.pull_anim_filename );

  if(b.unstack_anim_filename)
    unstack_anim_filename = NEW stringx( *b.unstack_anim_filename );
}


crate *get_crate( region *r, const vector3d& position, bool check_available, bool any )
{
  for( region::crate_list::const_iterator ci = r->get_crates().begin(); ci != r->get_crates().end(); ci++ )
  {
    crate *c = *ci;
    if( any || (check_available && c->is_available()) || (!check_available && !c->is_available()) )
    {
      vector3d v = c->get_abs_position();
      if( is_approx(v.x, position.x, .5f) && is_approx(v.y, position.y, .5f) && is_approx(v.z, position.z, .5f) )
        return c;
    }
  }
  return NULL;
}


crate::~crate()
{
  delete push_anim_filename;
  delete pull_anim_filename;
  delete stack_anim_filename;
  delete unstack_anim_filename;
}


bool crate::handle_enx_chunk( chunk_file& fs, stringx &label )
{
  const stringx prefix = stringx( "characters\\maxsteel\\" );

  if( label == "animations:")
  {
    for( serial_in(fs, &label); label != chunkend_label; serial_in(fs, &label) )
    {
      if( label == "push_anim" )
      {
        assert( push_anim_filename == NULL );
        serial_in( fs, &label );
        label = prefix + label;
        push_anim_filename = NEW stringx( label + ".anm" );
      }
      else if( label == "pull_anim" )
      {
        assert( pull_anim_filename == NULL );
        serial_in( fs, &label );
        label = prefix + label;
        pull_anim_filename = NEW stringx( label + ".anm" );
      }
      else if( label == "stack_anim" )
      {
        assert( stack_anim_filename == NULL );
        serial_in( fs, &label );
        label = prefix + label;
        stack_anim_filename = NEW stringx( label + ".anm" );
      }
      else if( label == "unstack_anim" )
      {
        assert( unstack_anim_filename == NULL );
        serial_in( fs, &label );
        label = prefix + label;
        unstack_anim_filename = NEW stringx( label + ".anm" );
      }
      else error( "Invalid chunk " + label + " in .enx file" );

//      load_anim( label + ".anm" );
    }
    return true;
  }
  else return( entity::handle_enx_chunk(fs, label) );
}


void crate::force_region( region_node* r )
{
}


void crate::stack_it()
{
  assert( for_stack != NULL );
  if( nether != NULL ) nether->set_available( true );
  nether = for_stack;
  for_stack->set_available( false );
  fix_po();
}


void crate::fix_po()
{
  vector3d  v( target_xz.x, .0f, target_xz.y );

  if( nether != NULL ) v.y = nether->get_abs_position().y + 1.0f;
  else
  {
    v.y = get_abs_position().y;
    v.y = .5f + g_world_ptr->get_the_terrain().get_elevation( v, vector3d(YVEC), get_region() );
  }
  set_rel_po( po_identity_matrix );
  set_rel_position( v );
}


void crate::set_target_xz_increment( rational_t x, rational_t z )
{
  assert( (__fabs(x) == 1.0f && z == .0f) || (__fabs(z) == 1.0f && x == .0f) );
  vector3d  v = get_abs_position();
  target_xz.x = v.x + x;
  target_xz.y = v.z + z;
}


void crate::frame_advance( time_value_t t )
{
  if( pAnim != NULL && pAnim->is_finished() )
  {
    g_world_ptr->kill_anim( pAnim );
    pAnim = NULL;
    if( is_stacking() )
    {
      stack_it();
      set_stacking( false );
    }
    else if( is_unstacking() )
    {
      unstack_it();
      set_unstacking( false );
    }
    else fix_po();
    set_available( true );
    set_active( false );
  }
}


const stringx& crate::get_push_anim_filename()
{
  assert( push_anim_filename );
  return *push_anim_filename;
}


const stringx& crate::get_pull_anim_filename()
{
  assert( pull_anim_filename );
  return *pull_anim_filename;
}


const stringx& crate::get_stack_anim_filename()
{
  assert( stack_anim_filename );
  return *stack_anim_filename;
}


const stringx& crate::get_unstack_anim_filename()
{
  assert( unstack_anim_filename );
  return *unstack_anim_filename;
}


void crate::set_facing( const vector2d& facing )
{
  vector3d  pos = get_abs_position();
  po        p( po_identity_matrix );
  p.set_facing( vector3d(facing.x, .0f, facing.y) );
  p.set_position( pos );
  set_rel_po( p );
}


void crate::start_pushing( const vector2d& xz_dir )
{
  set_active( true );
  set_facing( xz_dir );
  assert( pAnim == NULL );
  pAnim = play_anim( get_push_anim_filename(), 0, ANIM_RELATIVE_TO_START );
  if( nether != NULL )
  {
    nether->set_available( true );
    nether = NULL;
  }
  if( for_stack != NULL )
  {
    nether = for_stack;
    nether->set_available( false );
  }
}


void crate::start_stacking( const vector2d& xz_dir )
{
  assert( for_stack != NULL );
  set_available( false );
  for_stack->set_available( false );
  set_active( true );
  set_stacking( true );
  set_facing( xz_dir );
  assert( pAnim == NULL );
  pAnim = play_anim( get_stack_anim_filename(), 0, ANIM_RELATIVE_TO_START );
}


void crate::start_pulling( const vector2d& xz_dir )
{
  set_active( true );
  set_facing( xz_dir );
  assert( pAnim == NULL );
  pAnim = play_anim( get_pull_anim_filename(), 0, ANIM_RELATIVE_TO_START );
  if( nether != NULL )
  {
    nether->set_available( true );
    nether = NULL;
  }
  if( for_stack != NULL )
  {
    nether = for_stack;
    nether->set_available( false );
  }
}


void crate::unstack_it()
{
  assert( nether != NULL );
  nether->set_available( true );
  if( for_stack == NULL ) nether = NULL;
  else
  {
    nether = for_stack;
    nether->set_available( false );
  }
  fix_po();
}


void crate::start_unstacking( const vector2d& xz_dir )
{
  set_available( false );
  set_active( true );
  set_unstacking( true );
  set_facing( xz_dir );
  assert( pAnim == NULL );
  pAnim = play_anim( get_unstack_anim_filename(), 0, ANIM_RELATIVE_TO_START );
}


int get_crate( region *r, const po& p, rational_t delta_dist, rational_t delta_y, rational_t delta_ang, crate **found, vector3d *start_pos, vector3d *start_facing )
{
  rational_t    sin_delta_ang, cos_delta_ang;
  fast_sin_cos_approx( delta_ang, &sin_delta_ang, &cos_delta_ang );
//  rational_t    sin_delta_ang = sin( delta_ang ),
//                cos_delta_ang = cos( delta_ang );
  crate         *c;
  vector3d      v, vp;
  vector3d      position = p.get_position();
  vector3d      direction = p.get_facing();
  int           rv = CRATE_NONE;

  for_stack = NULL;
  direction.y = .0f;
  direction.normalize();
  if( !is_axis_aligned(direction, sin_delta_ang) ) return CRATE_NONE;

  for( region::crate_list::const_iterator ci = r->get_crates().begin(); ci != r->get_crates().end(); ci++ )
  {
    c = *ci;
    if( !c->is_available() ) continue;
    vp = c->get_abs_position();
    if( __fabs(vp.y - position.y) > delta_y ) continue;
    v = vp - position;
    if( v.length2() > delta_dist*delta_dist ) continue;
    v.y = .0f;
    v.normalize();
//    if( !is_axis_aligned(v, sin_delta_ang) ) continue;
    if( dot(direction,v) < cos_delta_ang ) continue;
    // found
    *found = c;
    rv = CRATE_CLIMB;
    if( is_approx(v.x, 1.0f, sin_delta_ang) )
    {
      v.x = 1.0f;
      v.z = 0.0f;
    }
    else if( is_approx(v.x, -1.0f, sin_delta_ang) )
    {
      v.x = -1.0f;
      v.z = 0.0f;
    }
    else
    {
      v.x = 0.0f;
      if( is_approx(v.z, 1.0f, sin_delta_ang) ) v.z = 1.0f;
      else v.z = -1.0f;
    }
    vp += v;
    bool        check_for_pull = false;
    rational_t  floor_y;
    if( vp.x >= c->u_left.x + c->offset.x && vp.z <= c->u_left.y + c->offset.y && vp.x <= c->l_right.x + c->offset.x && vp.z >= c->l_right.y + c->offset.y )
    {
      for_stack = ::get_crate( r, vp );
      if( for_stack != NULL )
      {
        if( c->nether == NULL || position.y > for_stack->get_abs_position().y ) rv = CRATE_STACK;
        else if( c->nether != NULL ) check_for_pull = true;
      }
      else
      {
        if( c->nether == NULL )
        {
          floor_y = g_world_ptr->get_the_terrain().get_elevation( vp, vector3d(YVEC), c->get_region() );
          if( get_crate(r, vp, false) == NULL && floor_y != BOTTOM_OF_WORLD && floor_y <= vp.y - .49f ) rv = CRATE_PUSH;
          else check_for_pull = true;
        }
        else
        {
          for_stack = ::get_crate( r, vp - YVEC );
          if( for_stack != NULL )
          {
            if( c->get_abs_position().y > position.y ) check_for_pull = true;
            else rv = CRATE_PUSH;
          }
          else check_for_pull = true;
        }
      }
    }
    else check_for_pull = true;
    if( check_for_pull )
    {
      vp -= 2 * v;
      if( vp.x >= c->u_left.x + c->offset.x && vp.z <= c->u_left.y + c->offset.y && vp.x <= c->l_right.x + c->offset.x && vp.z >= c->l_right.y + c->offset.y )
      {
        floor_y = g_world_ptr->get_the_terrain().get_elevation( vp - v, vector3d(YVEC), c->get_region() );
        if( floor_y != BOTTOM_OF_WORLD && floor_y <= vp.y - .49f )
        {
          if( c->nether == NULL )
          {
            if( get_crate(r, vp - v, true, true) == NULL ) rv = CRATE_PULL;
          }
          else
          {
            for_stack = get_crate( r, vp - YVEC );
            if( for_stack != NULL )
            {
              if( get_crate( r, vp - v - YVEC ) != NULL ) rv = CRATE_PULL;
            }
            else
            {
              for_stack = get_crate( r, vp - (2* YVEC) );
              if( for_stack != NULL )
              {
                if( get_crate(r, vp - v - (2* YVEC)) != NULL && get_crate(r, vp - v - YVEC, true, true) == NULL ) rv = CRATE_UNSTACK;
              }
              else if( get_crate(r, vp - v - YVEC, true, true) == NULL ) rv = CRATE_UNSTACK;
            }
          }
        }
      }
    }
    break;
  }
  if( rv != CRATE_NONE )
  {
    if( start_pos != NULL )
    {
      vector3d    v = p.get_facing();

      v.x = round( v.x );
      v.y = .0f;
      v.z = round( v.z );

      *start_pos = -0.5f * v + (*found)->get_abs_position();
    }
    if( start_facing != NULL )
    {
      vector3d  v = p.get_facing();
      start_facing->x = round( v.x );
      start_facing->y = .0f;
      start_facing->z = round( v.z );
    }
  }
  return rv;
}


void scan_for_stacked_crates( region *r )
{
  const rational_t  DELTA = .001f;

  if( r->get_crates().size() == 0 ) return;

  rational_t                    level = -BOTTOM_OF_WORLD;
  region::crate_list::iterator  ci, ce = r->get_crates().end();

  for( ci = r->get_crates().begin(); ci != ce; ++ci ) if( (*ci)->get_abs_position().y < level ) level = (*ci)->get_abs_position().y;
  assert( level != -BOTTOM_OF_WORLD );

  bool  found;
  do
  {
    found = false;
    for( ci = r->get_crates().begin(); ci != ce; ++ci )
    {
      crate     *cr = *ci;
      vector3d  cr_pos = cr->get_abs_position();

      if( cr_pos.y >= level ) found = true;
      if( !is_approx(cr_pos.y, level, DELTA) ) continue;

      for( region::crate_list::iterator cci = r->get_crates().begin(); cci != ce; ++cci )
      {
        crate     *ccr = *cci;
        vector3d  ccr_pos = ccr->get_abs_position();

        if( is_approx(ccr_pos.y, level + 1.0f, DELTA) && is_approx(ccr_pos.x, cr_pos.x, DELTA) && is_approx(ccr_pos.z, cr_pos.z, DELTA) )
        {
          // Got you!!!
          cr->set_available( false );
          ccr->nether = cr;
        }
      }
    }
    level += 1.0f;
  }while( found );
}

#endif
