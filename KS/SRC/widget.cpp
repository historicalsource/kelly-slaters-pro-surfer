// widget.cpp :: basic interface rendering

#include "global.h"

#include "app.h"
#include "game.h"
#include "camera.h"
#include "geomgr.h"
#include "iri.h"
#include "osdevopts.h"
#include "ostimer.h"
#include "visrep.h"
#include "pmesh.h"
#include "hwrasterize.h"
#include "widget.h"
#include "renderflav.h"
#include "commands.h"
#include "inputmgr.h"
#include "hwmath.h"
#include "localize.h"
#include "entity.h"
#include "entityflags.h"
#include "conglom.h"
#include "entity_maker.h"
#include "widget_entity.h"
#include "vertwork.h"
#include "profiler.h"
#include "text_font.h"
#include "ksngl.h"	// For KSNGL_CreateScratchMesh (dc 06/06/02)

#include "ngl_support.h"

#include <cstdarg>

#if defined(TARGET_XBOX)
#include "ngl.h"
#endif /* TARGET_XBOX JIV DEBUG */

matrix4x4 g_projection_matrix;

// we start at the near clipping plane and move closer and closer to the camera
// ergo, anything in the rhw_layers are in front of any normally rendered stuff
// for those at home:
// interface widget depth is initially set as an rhw, which is reciprocal of w, which
// is proportional to z.  This then gets "converted" to a 0 to 1 z where 0 is touching
// the camera and 1 is the ordinarily near clip plane.  At this point the original meaning
// of w is lost, but the order is maintained.  Then when rendering it gets
// converted to the renderer's z, where 0 is touching the camera and 1 is the far clip plane.
// I THINK.
// If I were to do it all over again, we'd store interface widget layers as distances from
// the camera.  This would be consistent across platforms.  Then, internally, platforms could
// engage in their own evil internal schemes to get them into z-and-w coordinates that make
// sense.
const rational_t widget::rhw_layer_ranges[NUM_RHW_LAYERS][2] =
{
  { PROJ_NEAR_PLANE_RHW, 69.0f },   // RHW0 - 2d (bitmaps)           (default for script bitmaps)
  { 70.0f,  169.0f },               // RHW1 - 2d                     (default for interface bitmaps)
  { 170.0f, 319.0f },               // RHW2 - 3d (vreps & entities)  (default for vreps)
  { 320.0f, 469.0f },               // RHW3 - 3d
  { 470.0f, 619.0f },               // RHW4 - 3d
  { 620.0f, 769.0f },               // RHW5 - 3d
  { 770.0f, 819.0f },               // RHW6 - 2d & 3d                (default for text & pda bitmaps)
  { 820.0f, 869.0f },               // RHW7 - 2d & 3d
  { 870.0f, 919.0f },               // RHW8 - 2d & 3d
  { 920.0f, 969.0f },               // RHW9 - 2d & 3d
  { 13000.0f, 13099.0f },           // RHW_OVER_PFE1 - 2d over pfe   (popups)
  { 13100.0f, 13199.0f },           // RHW_OVER_PFE2 - 2d over pfe   (popups)
};

widget::rhw_layer_e widget::rhw_2d_layer = widget::RHW1;
widget::rhw_layer_e widget::rhw_3d_layer = widget::RHW2;
widget::rhw_layer_e widget::last_rhw_2d_layer = widget::RHW1;
widget::rhw_layer_e widget::last_rhw_3d_layer = widget::RHW2;


rational_t widget::rhw_2d_val[NUM_RHW_LAYERS]; // all these are reset when unparented widget is constructed
bool widget::rendering_prepared = false;

//-----------------------------------------------------------------

time_value_t wevent::active_time_elapsed()
{
  if ( elapsed < wait_time )
    return 0.0f;
  else if ( elapsed > wait_time + duration )
    return duration;
  else
    return ( elapsed - wait_time );
}

time_value_t wevent::active_time_left()
{
  if ( elapsed < wait_time )
    return duration;
  else if ( elapsed >= wait_time + duration )
    return 0.0f;
  else
    return ( (duration + wait_time) - elapsed );
}

time_value_t wevent::total_time_left()
{
  if ( elapsed >= wait_time + duration )
    return 0.0f;
  else
    return ( (duration + wait_time) - elapsed );
}

time_value_t wevent::time_till_active()
{
  if ( elapsed >= wait_time )
    return 0.0f;
  else
    return ( wait_time - elapsed );
}


void wevent::set_time_left( time_value_t t )
{
  assert( t >= 0.0f );

  // only sets if there still is time left
  if ( !no_time_left() )
  {
    if ( has_been_activated() || time_till_active() <= t )
    {
      duration = t + elapsed - wait_time;
    }
    else
    {
      duration = 0.0f;
      wait_time = t + elapsed;
    }
  }
}

rational_t wevent::get_lerp( time_value_t t )
{
  rational_t lerp;

  if (elapsed < wait_time)
    lerp = 0.0f;
  else if ( duration == 0.0f || active_time_left() == 0.0f )
  {
    lerp = 1.0f;
  }
  else
  {
    if ( owner->linear_animation )
    {
      lerp = t / active_time_left();
    }
    else
    {
      lerp = active_time_elapsed() / duration;
    }
    if (lerp > 1.0f) lerp = 1.0f;
  }


  // print lerp to screen
#if 0
  char work_str[256];
  sprintf( work_str, "LERP: %2.5f, ELAPSED: %2.5f", lerp, elapsed );
  hw_rasta::inst()->print( work_str, vector2di(250,52) ); // 450, 60
#endif

  return lerp;
}


void move_wevent::do_wevent( rational_t lerp )
{
  // translate the object toward the destination based on the elapsed time
  owner->move_to( owner->x + ( x - owner->x ) * lerp, owner->y + ( y - owner->y ) * lerp );
}


void color_wevent::do_wevent( rational_t lerp )
{
  // blend the color toward the destination color based on the elapsed time
  for ( int i = 0; i < 4; ++i )
  {
    color newcol;
    newcol.r = owner->col[i].r + (mycolor.r - owner->col[i].r) * lerp;
    newcol.g = owner->col[i].g + (mycolor.g - owner->col[i].g) * lerp;
    newcol.b = owner->col[i].b + (mycolor.b - owner->col[i].b) * lerp;
    newcol.a = owner->col[i].a + (mycolor.a - owner->col[i].a) * lerp;
    owner->set_color( newcol );
  }
}


void rotate_wevent::do_wevent( rational_t lerp )
{
  owner->rotate_to( owner->angle + ( angle - owner->angle ) * lerp );
}


void scale_wevent::do_wevent( rational_t lerp )
{
  owner->scale_to( owner->S[0] + (sx - owner->S[0]) * lerp, owner->S[1] + (sy - owner->S[1]) * lerp );
}


void wevent::frame_advance( time_value_t time_inc )
{
  elapsed += time_inc;

  do_wevent( get_lerp( time_inc ) );
}


//-----------------------------------------------------------------


widget::widget( const char *_widget_name, widget *_parent, short _x, short _y )
  : widget_name( stringx( _widget_name ) ), parent( _parent ), x( _x ), y( _y )
{
  int i;

  type = WTYPE_Other;

  flags = 0;                // widgets start out hidden
  linear_animation = true;
  use_proj_matrix = false;
  next_state = WSTATE_None;
  state_wait_time = 0.0f;

  base_x = 0;
  base_y = 0;
  orig_x = orig_y = 0;
  base_S[0] = base_S[1] = S[0] = S[1] = 1.0f;
  R[0][0] = R[1][1] = 1.0f;
  R[0][1] = R[1][0] = 0.0f;
  base_angle = angle = 0.0f;
  for ( i = 0; i < 4; ++i )
  {
    base_col[i].r = base_col[i].g = base_col[i].b = base_col[i].a = 1.0f;
    col[i].r = col[i].g = col[i].b = col[i].a = 1.0f;
  }

  if ( parent )
  {
    parent->add_child( this );
  }
  else
  {
    // reset 2d rhw vals if unparented widget
    for ( i = 0; i < NUM_RHW_LAYERS; ++i )
    {
      rhw_2d_val[i] = rhw_layer_ranges[i][0];
    }
  }

  update_pos();
  update_scale();
  update_rot();
  update_col();
}




widget::~widget()
{
  flush();

  // delete children
  while ( children.size() )
  {
    delete *children.begin();
    children.erase( children.begin() );
  }
}



void widget::show()
{
  if ( !is_shown() )
  {
    set_flag( WFLAG_Shown, true );
    if ( ignoring_parent_showing() )
    {
      set_flag( WFLAG_Override_Ignore_Showing, false );
    }
  }

  // children
  widget_list_t::iterator it;
  for( it = children.begin(); it != children.end(); ++it )
  {
    if ( !(*it)->ignoring_parent() &&
        (!(*it)->ignoring_parent_showing() || (*it)->override_ignore_showing()) )
    {
      (*it)->show();
    }
  }
}



void widget::hide()
{
  if ( is_shown() )
  {
    flush();

    if ( is_shown() && ignoring_parent_showing() )
    {
      set_flag( WFLAG_Override_Ignore_Showing, true );
    }

    set_flag( WFLAG_Shown, false );
  }

  // children
  widget_list_t::iterator it;
  for( it = children.begin(); it != children.end(); ++it )
  {
    if ( !(*it)->ignoring_parent() )
    {
      (*it)->hide();
    }
  }
}


// advance all the wevents queued for this widget
void widget::frame_advance( time_value_t time_inc )
{
  // state processing (a timed way to hide and show)
  if ( next_state != WSTATE_None )
  {
    state_wait_time -= time_inc;
    if ( state_wait_time <= 0.0f )
    {
      switch ( next_state )
      {
        case WSTATE_Show:
          show();
          break;
        case WSTATE_Hide:
          hide();
          break;
        default:
          break;
      }
      next_state = WSTATE_None;
      state_wait_time = 0.0f;
    }
  }

  if ( is_shown() )
  {
    // wevent processing
    wevent_list_t::iterator it = wevent_run_list.begin();

    while( it != wevent_run_list.end() )
    {
      (*it)->frame_advance( time_inc );
      wevent_list_t::iterator del = it;
      ++it;

      // complete? remove from list
      if ( (*del)->no_time_left() )
      {
        delete *del;
        wevent_run_list.erase( del );
      }
    }

    // children
    widget_list_t::iterator child;
    for( child = children.begin(); child != children.end(); ++child )
    {
      (*child)->frame_advance( time_inc );
    }
  }
}


void widget::render()
{
  /*
  if ( !parent )
  {
    prepare_to_render();
  }
  */

  //if ( use_proj_matrix )
  //{
  //  geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, g_projection_matrix);
  //}

  // children
  widget_list_t::iterator child;
  for( child = children.begin(); child != children.end(); ++child )
  {
    if ( (*child)->is_shown() )
    {
      (*child)->render();
    }
  }

  //if ( use_proj_matrix )
  //{
  //  geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, identity_matrix);
  //}

  /*
  if ( !parent )
  {
    finish_render();
  }
  */
}



void widget::message_handler( message_id_t message, message_id_t overflow, rational_t parm0, rational_t parm1 )
{
  // children
  widget_list_t::iterator child;
  for( child = children.begin(); child != children.end(); ++child )
  {
    if ( (*child)->is_shown() )
    {
      (*child)->message_handler( message, overflow, parm0, parm1 );
    }
  }
}


void widget::prepare_to_render()
{
  hw_rasta::inst()->send_start( hw_rasta::PT_TRANS_POLYS );

  hw_rasta::inst()->set_zbuffering( true, true );

  // save the projection matrix
  g_projection_matrix = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_PROJECTION];

  geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
  geometry_manager::inst()->set_xform(geometry_manager::XFORM_WORLD_TO_VIEW, identity_matrix);
  matrix4x4 ortho;
  ortho.make_scale(vector3d(1.0f,1.0f/PROJ_ASPECT,1.0f));
  geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, ortho);

  //geometry_manager::inst()->set_cop(hw_rasta::inst()->get_screen_width()*0.5f,hw_rasta::inst()->get_screen_height()*0.5f, INTERFACE_MIN_Z, INTERFACE_MAX_Z);
  rendering_prepared = true;
}


void widget::finish_render()
{
  rendering_prepared = false;
  //geometry_manager::inst()->set_cop(hw_rasta::inst()->get_screen_width()*0.5f,hw_rasta::inst()->get_screen_height()*0.5f, WORLD_MIN_Z, WORLD_MAX_Z);

  geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, g_projection_matrix);

  hw_rasta::inst()->set_zbuffering( true, false );
}



void widget::add_child( widget *child )
{
  children.push_back( child );
}

void widget::remove_child( widget *child )
{
  children.remove( child );
}


// add NEW wevent to queue for this widget; resolve overlap in time with other wevents of same type
void widget::add_wevent( wevent* e )
{
  bool found_insert_pt = false;
  wevent_list_t::iterator it = wevent_run_list.begin(),
                         end_it = wevent_run_list.end(),
                         save_it = wevent_run_list.end();

  // process overlaps between wevents of same type, find insertion point
  for( ; it != end_it; ++it )
  {
    if ( !found_insert_pt && e->time_till_active() < (*it)->time_till_active() )
    {
      save_it = it;
      found_insert_pt = true;
    }
    if ( found_insert_pt )
    {
      // avoid overlap in time with following wevents of same type
      if ( (*it)->get_type() == e->get_type() && e->total_time_left() > (*it)->time_till_active() )
      {
         e->set_time_left( (*it)->time_till_active() );
      }
    }
    else
    {
      // avoid overlap in time with previous wevents of same type
      if ( (*it)->get_type() == e->get_type() && (*it)->total_time_left() > e->time_till_active() )
      {
         (*it)->set_time_left( e->time_till_active() );
      }
    }
  }
  if ( save_it == end_it )
  {
    // if not inserted, put at end of list
    wevent_run_list.push_back( e );
  }
  else
  {
    wevent_run_list.insert( save_it, e );
  }
}



void widget::flush()
{
  while ( wevent_run_list.size() )
  {
    wevent_list_t::iterator we = wevent_run_list.begin();
    (*we)->do_wevent( 1.0f );
    delete *we;
    wevent_run_list.erase( we );
  }

  next_state = WSTATE_None;
  state_wait_time = 0.0f;

  // children
  widget_list_t::iterator it;
  for( it = children.begin(); it != children.end(); ++it )
  {
    (*it)->flush();
  }
}



void widget::update_pos()
{
  if ( parent && !ignoring_parent() )
  {
    base_x = parent->get_abs_x();
    base_y = parent->get_abs_y();
  }

  abs_x = x + base_x;
  abs_y = y + base_y;

  // recurse through children
  widget_list_t::iterator child;
  for( child = children.begin(); child != children.end(); ++child )
  {
    (*child)->update_pos();
  }
}


void widget::update_scale()
{
  if ( parent && !ignoring_parent() )
  {
    base_S[0] = parent->get_abs_scale( 0 );
    base_S[1] = parent->get_abs_scale( 1 );
  }

  abs_S[0] = S[0] * base_S[0];
  abs_S[1] = S[1] * base_S[1];

  // recurse through children
  widget_list_t::iterator child;
  for( child = children.begin(); child != children.end(); ++child )
  {
    (*child)->update_scale();
  }
}



void widget::update_rot()
{
  if ( parent && !ignoring_parent() )
  {
    base_angle = parent->get_abs_angle();
  }

  abs_angle = angle + base_angle;

  rational_t ct, st;
  fast_sin_cos_approx( abs_angle, &st, &ct );

  R[0][0] = ct;
  R[0][1] = -st;
  R[1][0] = st;
  R[1][1] = ct;

  // recurse through children
  widget_list_t::iterator child;
  for( child = children.begin(); child != children.end(); ++child )
  {
    (*child)->update_rot();
  }
}


void widget::update_col()
{
  int i;
  if ( parent && !ignoring_parent() )
  {
    for ( i = 0; i < 4; ++i )
    {
      base_col[i].r = parent->get_abs_col( i ).r;
      base_col[i].g = parent->get_abs_col( i ).g;
      base_col[i].b = parent->get_abs_col( i ).b;
      base_col[i].a = parent->get_abs_col( i ).a;
    }
  }

  for ( i = 0; i < 4; ++i )
  {
    abs_col[i].r = col[i].r * base_col[i].r;
    abs_col[i].g = col[i].g * base_col[i].g;
    abs_col[i].b = col[i].b * base_col[i].b;
    abs_col[i].a = col[i].a * base_col[i].a;
  }

  // recurse through children
  widget_list_t::iterator child;
  for( child = children.begin(); child != children.end(); ++child )
  {
    (*child)->update_col();
  }
}


widget *widget::get_first_child()
{
  if ( children.empty() )
  {
    return NULL;
  }
  else
  {
    return ( *(children.begin()) );
  }
}


widget *widget::get_prev_child( widget *start )
{
  widget_list_t::iterator child;
  widget *retval = NULL, *prev = NULL;
  for( child = children.begin(); child != children.end(); ++child )
  {
    if ( start == *child )
    {
      if ( prev )
      {
        retval = prev;
      }
      else
      {
        retval = children.back();
      }
      break;
    }
    prev = *child;
  }
  return ( retval );
}


widget *widget::get_next_child( widget *start )
{
  widget_list_t::iterator child;
  widget *retval = NULL, *next = NULL;
  children.reverse();
  for( child = children.begin(); child != children.end(); ++child )
  {
    if ( start == *child )
    {
      if ( next )
      {
        retval = next;
      }
      else
      {
        retval = children.back();
      }
      break;
    }
    next = *child;
  }
  children.reverse();
  return ( retval );
}



widget *widget::find_child_by_name( stringx name )
{
  widget *retval = NULL;
  widget_list_t::iterator child;
  for( child = children.begin(); child != children.end(); ++child )
  {
    if ( name == (*child)->get_name() )
    {
      retval = *child;
      break;
    }
  }
  return retval;
}


void widget::move_to( short _x, short _y )
{
  x = _x;
  y = _y;
  update_pos();
}


void widget::move_to( time_value_t wt, time_value_t d, short _x, short _y )
{
  move_wevent *e = NEW move_wevent( this, wt, d, _x, _y );
  add_wevent( e );
}



void widget::scale_to( rational_t hs, rational_t vs )
{
  S[0] = hs;
  S[1] = vs;
  update_scale();
}



void widget::scale_to( time_value_t wt, time_value_t d, rational_t hs, rational_t vs )
{
  scale_wevent *e = NEW scale_wevent( this, wt, d, hs, vs );
  add_wevent( e );
}



void widget::rotate_to( rational_t a )
{
  angle = a;
  update_rot();
}



void widget::rotate_to( time_value_t wt, time_value_t d, rational_t a )
{
  rotate_wevent *e = NEW rotate_wevent( this, wt, d, a );
  add_wevent( e );
}



void widget::set_color( color c )
{
  for ( int i = 0; i < 4; ++i )
  {
    col[i] = c;
  }
  update_col();
}



void widget::set_color( color c[4] )
{
  for ( int i = 0; i < 4; ++i )
  {
    col[i] = c[i];
  }
  update_col();
}



void widget::set_color( time_value_t wt, time_value_t d, color c )
{
  color_wevent *e = NEW color_wevent( this, wt, d, c );
  add_wevent( e );
}


void widget::set_color( rational_t r, rational_t g, rational_t b )
{
  set_color( color( r, g, b, col[0].a ) );
}




void widget::fade_to( rational_t alpha )
{
  for ( int i = 0; i < 4; ++i )
  {
    col[i].a = alpha;
  }
  update_col();
}



void widget::fade_to( time_value_t wt, time_value_t d, rational_t alpha )
{
  color c;
  c.r = col[0].r;
  c.g = col[0].g;
  c.b = col[0].b;
  c.a = alpha;
  color_wevent *e = NEW color_wevent( this, wt, d, c );
  add_wevent( e );
}



bool widget::is_faded() const
{
  return ( abs_col[0].a != 1.0f || abs_col[1].a != 1.0f || abs_col[2].a != 1.0f || abs_col[3].a != 1.0f );
}



void widget::transform(rational_t v[2], color &c, int index)
{
  assert(index < 4);
  rational_t xt, yt;

  // scale them
  v[0] *= abs_S[0];
  v[1] *= abs_S[1];
  // rotate them
  xt = v[0] * R[0][0] + v[1] * R[0][1];
  yt = v[0] * R[1][0] + v[1] * R[1][1];
  v[0] = xt;
  v[1] = yt;
  // translate them
  v[0] += abs_x - orig_x;
  v[1] += abs_y - orig_y;

  c.r *= abs_col[index].r;
  c.g *= abs_col[index].g;
  c.b *= abs_col[index].b;
  c.a *= abs_col[index].a;

/*
#ifdef NGL
  // transform to target platform coordinate
  v[0] *= nglGetScreenWidth()/nglGetScreenWidth();
  v[1] *= nglGetScreenHeight()/nglGetScreenHeight();
#endif
*/
}


void widget::ndc(rational_t v[2])
{
  // if the buck stops here NDC them
  v[0] = (v[0] - 320.0f) / 320.0f;
  v[1] = (240.0f - v[1]) / 240.0f;
}


rational_t widget::get_next_rhw_2d_val()
{
  rational_t retval = rhw_2d_val[rhw_2d_layer];
  rhw_2d_val[rhw_2d_layer] += WIDGET_RHW_INC;
#ifdef TARGET_MKS
//  assert( rhw_2d_val[rhw_2d_layer] < rhw_layer_ranges[rhw_2d_layer][1] );
#endif
  return retval;
}


// dreamcast sorting madness

rational_t widget::cur_rhw_midpoint;
rational_t widget::cur_rhw_half_range;
rational_t widget::cur_largest_z;
matrix4x4 widget::cur_special_w_xform;

matrix4x4 widget::calc_special_w_xform( const matrix4x4 &cur_mat, int _x, int _y )
{
#if defined(TARGET_MKS)
  matrix4x4 w_xform;
  static bool first_time = true;
  static matrix4x4 proj_mat;
  if ( first_time )
  {
    proj_mat = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_PROJECTION];
    first_time = false;
  }
  matrix4x4 v2vp;
  asm_m4x4_mul( &proj_mat, &geometry_manager::inst()->get_viewport_xform(), &v2vp );
  asm_m4x4_mul( &cur_mat, &v2vp, &w_xform );
  return w_xform;
#else
  return cur_mat;
#endif
}


rational_t widget::calc_z( const vector3d &xyz )  // for DC sorting calcs in pmesh.cpp
{
  rational_t w = cur_special_w_xform[0][3]*xyz.x+
                 cur_special_w_xform[1][3]*xyz.y+
                 cur_special_w_xform[2][3]*xyz.z+
                 cur_special_w_xform[3][3];
  if ( w == 0.0f ) w = 1.0f;
  return (1.0f / w);
}


// FOR PC SORTING
// convert DC rhw to pc z val
rational_t widget::get_pc_z( rational_t _rhw )
{
/*
#ifdef TARGET_MKS

  return 0.1f;

#elif defined(TARGET_PC)
  */
//#pragma fixme("Why isn't this just 1.0f / rhw?  jdf 4/1/01")
  // it seems like this is a hack to put widgets on the far side of the near clip plane
  // even though their rhw's are on this side.  it puts them anywhere from 0.2f to 1.0f meters
  // away from camera  -jdf 4/1/01
  rational_t min_rhw = widget::rhw_layer_ranges[RHW0][0];
  rational_t max_rhw = widget::rhw_layer_ranges[RHW9][1];

  if ( _rhw > max_rhw )
    _rhw = max_rhw;

  rational_t min_z = PROJ_NEAR_PLANE_D * 2;
  rational_t z_range = 0.999f - min_z;

  rational_t _z = 0.999f - (((_rhw-min_rhw) / (max_rhw-min_rhw)) * z_range);
  return _z;
/*
#elif defined(TARGET_PS2)
#pragma fixme("What is this supposed to represent?  jdf 4/1/01")
  rational_t min_rhw = widget::rhw_layer_ranges[RHW0][0];
  rational_t max_rhw = widget::rhw_layer_ranges[RHW9][1];

  if ( _rhw > max_rhw )
    _rhw = max_rhw;

  rational_t _z = ((_rhw-min_rhw) / (max_rhw-min_rhw));
  return _z;

#endif
  */
}


//-----------------------------------------------------------------

menu_item_widget::menu_item_widget( const char *_widget_name, widget *_parent, short _x, short _y, const char *_item_desc )
  : widget( _widget_name, _parent, _x, _y )
{
  subtype = WSUBTYPE_None;
  selected = false;
  skip = false;
  item_desc = stringx( _item_desc );
  if ( get_parent()->get_type() == WTYPE_Menu )
  {
    index = ((menu_widget*)get_parent())->get_num_items() - 1;
  }
  else
  {
    index = 0;
  }
}


void menu_item_widget::select( bool initial )
{
  selected = true;

  // set NEW focus? (only if we are parented by a menu_widget)
  if ( get_parent()->get_type() == WTYPE_Menu )
  {
    menu_widget *p = (menu_widget*)get_parent();
    menu_item_widget *f = p->sel_item;
    p->sel_item = this;
    // deselect old sel_item if there is one
    if ( f && f != this )
    {
      f->deselect( initial );
    }
  }
}

rational_t menu_item_widget::get_width()
{
  return 100.0f;
}

rational_t menu_item_widget::get_height()
{
  return 100.0f;
}


//-----------------------------------------------------------------


menu_widget::menu_widget( const char *_widget_name, widget *_parent, short _x, short _y, message_id_t _to_prev, message_id_t _to_next )
  : widget( _widget_name, _parent, _x, _y )
{
  type = WTYPE_Menu;
  to_prev = _to_prev;
  to_next = _to_next;
  sel_item = NULL;
  default_sel_index = 0;
  num_items = 0;
  change_made = false;
}


void menu_widget::init()
{
  change_made = false;

  widget_list_t::iterator it;
  for( it = children.begin(); it != children.end(); ++it )
  {
    ((menu_item_widget*)(*it))->deselect( true ); // first deselect everybody
  }
  sel_item = find_item_by_index( default_sel_index );

  // select the sel_item (to make sure initial focus-selection stuff is performed, whatever that may be)
  if ( sel_item )
  {
    sel_item->select( true );
  }
}


void menu_widget::show()
{
  if ( !is_shown() )
  {
    init();
    widget::show();
  }
}


void menu_widget::message_handler( message_id_t message, message_id_t overflow, rational_t parm0, rational_t parm1 )
{
  if ( !sel_item ) return;

  if ( message & to_prev )
  {
    get_prev_item()->select();
    message &= ~to_prev;  // kill this message before moving down
    change_made = true;
  }
  else if ( message & to_next )
  {
    get_next_item()->select();
    message &= ~to_next;  // kill this message before moving down
    change_made = true;
  }

  sel_item->message_handler( message, overflow, parm0, parm1 );


  if( message & WMESS_OVERFLOW )
  {
    if( overflow & WMESS_MS_Press )
    {
      widget_list_t::iterator child;
      for( child = children.begin(); child != children.end(); ++child )
      {
        // PEH: THIS CHECK DISALLOWS DOUBLE-DIPPING OF THE CHILDREN
        if( *child != sel_item )
        {
          (*child)->message_handler( WMESS_OVERFLOW, WMESS_MS_Press, parm0, parm1 );
        }
      }
    }
  }
}


menu_item_widget *menu_widget::get_prev_item() const
{
  if ( !sel_item ) return ( NULL );

  int num_tested = 0;
  widget *prev = NULL;
  widget *start = (widget *)sel_item;

  while ( (!prev || ((menu_item_widget*)prev)->skip_it()) && num_tested < num_items )
  {
    if ( children.front() == start )
    {
      prev = children.back(); // for wraparound
    }
    else
    {
      widget_list_t::const_iterator it;
      for( it = children.begin(); it != children.end(); ++it )
      {
        if ( *it == start )
        {
          break;
        }
        prev = *it;
      }
    }
    start = prev;
    ++num_tested;
  }
  return ( (menu_item_widget *)prev );
}

menu_item_widget *menu_widget::get_next_item() const
{
  if ( !sel_item ) return ( NULL );

  int num_tested = 0;
  widget *next = NULL;
  widget *start = (widget *)sel_item;

  while ( (!next || ((menu_item_widget*)next)->skip_it()) && num_tested < num_items )
  {
    if ( children.back() == start )
    {
      next = children.front(); // for wraparound
    }
    else
    {
      widget_list_t::const_iterator it;
      for( it = children.begin(); it != children.end(); ++it )
      {
        if ( *it == start )
        {
          next = *(++it);
          break;
        }
      }
    }
    start = next;
    ++num_tested;
  }
  return ( (menu_item_widget *)next );
}


// returns NULL if none found
menu_item_widget *menu_widget::find_item_by_index( int index ) const
{
  menu_item_widget *retval = NULL;

  widget_list_t::const_iterator it;
  for( it = children.begin(); it != children.end(); ++it )
  {
    if ( ((menu_item_widget*)(*it))->get_index() == index )
    {
      retval = (menu_item_widget*)(*it);
      break;
    }
  }

  return ( retval );
}


// returns NULL if none found
menu_item_widget *menu_widget::find_item_by_desc( stringx &desc ) const
{
  menu_item_widget *retval = NULL;

  widget_list_t::const_iterator it;
  for( it = children.begin(); it != children.end(); ++it )
  {
    if ( ((menu_item_widget*)(*it))->get_item_desc() == desc )
    {
      retval = (menu_item_widget*)(*it);
      break;
    }
  }

  return ( retval );
}

rational_t menu_widget::get_width()
{
  return 100.0f;
}

rational_t menu_widget::get_height()
{
  return 100.0f;
}



//-----------------------------------------------------------------



bitmap_widget::bitmap_widget( const char *_widget_name, widget *_parent, short _x, short _y, int n )
  : widget( _widget_name, _parent, _x, _y )
{
  type = WTYPE_Bitmap;

  int i;
  hw = hh = 0.0f;
  nframes = n;
  index = 0;
  playing = false;
  frame_delay = 1.0f / 30.0f;
  frame_time = 0.0f;
  for ( i = 0; i < 4; ++i )
  {
    uv_order[i] = i;
  }

  // set rhw
  rhw = get_next_rhw_2d_val();
  z = get_pc_z( rhw );
}


bitmap_widget::~bitmap_widget()
{
}



void bitmap_widget::scale_to( rational_t hs, rational_t vs )
{
  if ( !is_open() ) return;
  widget::scale_to( hs, vs );
  set_tc();
  set_subrect( 0.0f, 0.0f, w * abs_S[0], h * abs_S[1] );
  // only change origin if it has been set already
  if ( orig_x != 0.0f || orig_y != 0.0f )
  {
    set_origin( short( w * abs_S[0] / 2.0f + 0.5f ), short( h * abs_S[1] / 2.0f + 0.5f ) );
  }
}


bool bitmap_widget::open(const char *name)
{
/*
#ifdef NGL
  filespec foospec( name );
  stringx path = foospec.path.c_str();
  texture_name = foospec.name.c_str();
  nglSetTexturePath( (char*)( path.c_str() ) );
  nglTexture* Tex = nglLoadTextureLock( (char*)( texture_name.c_str() ) );
  if ( Tex )
  {
    w = Tex->Width;
    h = Tex->Height;
  }
  else
    w = h = 0;
#else
  hw_texture *texture;

  mat = NEW material( name,
    MAT_FULL_SELF_ILLUM |
    MAT_NO_FOG,
    TEX_CLAMP_U |
    TEX_CLAMP_V );

  texture = mat.get_texture( 0, MAP_DIFFUSE );

  w = (rational_t)texture->get_original_width();
  h = (rational_t)texture->get_original_height();
#endif
  */
  mat.load_material( name );
  w = mat.get_original_width( 0, MAP_DIFFUSE );
  h = mat.get_original_height( 0, MAP_DIFFUSE );

  set_flag( WFLAG_Open, true );
  hw = w * 0.5f;
  hh = h * 0.5f;
  subrect = rectf(0.0f, 0.0f, w, h);
  set_tc();
  last_time = 0;
  return true;
}



void bitmap_widget::resize( rational_t width, rational_t height )
{
  if ( !is_open() ) return;
  scale_to( width / w, height / h );
}

void bitmap_widget::play()
{
  playing = true;
}

void bitmap_widget::pause()
{
  playing = false;
}

void bitmap_widget::stop()
{
  playing = false;
  index = 0;
}

void bitmap_widget::set_fps( int fps )
{
  frame_delay = 1.0f / (rational_t)fps;
}


void bitmap_widget::flip_horiz()
{
  int i, save_order[4];

  for ( i = 0; i < 4; ++i )
    save_order[i] = uv_order[i];

  uv_order[0] = save_order[1];
  uv_order[1] = save_order[0];
  uv_order[2] = save_order[3];
  uv_order[3] = save_order[2];
}


void bitmap_widget::flip_vert()
{
  int i, save_order[4];

  for ( i = 0; i < 4; ++i )
    save_order[i] = uv_order[i];

  uv_order[0] = save_order[2];
  uv_order[1] = save_order[3];
  uv_order[2] = save_order[0];
  uv_order[3] = save_order[1];
}


void bitmap_widget::set_tc()
{
  if ( !is_open() ) return;

  tc[0] = texture_coord((subrect.get_left () + 0.5f) / ( w * abs_S[0] ), (subrect.get_top   () + 0.5f) / ( h * abs_S[1]) );
  tc[1] = texture_coord((subrect.get_right() - 0.5f) / ( w * abs_S[0] ), (subrect.get_top   () + 0.5f) / ( h * abs_S[1]) );
  tc[2] = texture_coord((subrect.get_left () + 0.5f) / ( w * abs_S[0] ), (subrect.get_bottom() - 0.5f) / ( h * abs_S[1]) );
  tc[3] = texture_coord((subrect.get_right() - 0.5f) / ( w * abs_S[0] ), (subrect.get_bottom() - 0.5f) / ( h * abs_S[1]) );
}

void bitmap_widget::set_layer( rhw_layer_e rhw_layer )
{
  set_rhw_2d_layer(rhw_layer);
  rhw = get_next_rhw_2d_val();
  z = get_pc_z( rhw );
  restore_last_rhw_2d_layer();
}

void bitmap_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() || !is_open() ) return;

  if ( playing )
  {
    last_time += time_inc;
    frame_time += time_inc;
    while ( frame_time > frame_delay )
    {
      frame_time -= frame_delay;
      ++index;
//      if ( index >= nframes ) index = 0;
    }
  }

  widget::frame_advance( time_inc );
}


extern profiler_timer proftimer_widget_sendctx;
extern profiler_timer proftimer_widget_draw;

void bitmap_widget::render()
{
  if ( !is_shown() || !is_open() ) return;

#ifdef TARGET_PC
  float urhw = 1.0f;
#endif

  int                      i;
  rational_t               v[4][2], sw, sh;
  color                    c[4];

  // start out with the local coordinates
  sw = subrect.get_width () / abs_S[0];
  sh = subrect.get_height() / abs_S[1];
  v[0][0] = 0;
  v[0][1] = 0;
  v[1][0] = sw;
  v[1][1] = 0;
  v[2][0] = sw;
  v[2][1] = sh;
  v[3][0] = 0;
  v[3][1] = sh;

  for(i = 0; i < 4; ++i)
  {
    c[i] = color( 1.0f, 1.0f, 1.0f, 1.0f );
    transform(v[i], c[i], i);
#ifdef NGL
    // transform from our origin to yours
    v[i][0] += 2048 - nglGetScreenWidth()/2;
    v[i][1] += 2048 - nglGetScreenHeight()/2;
#endif
  }

  if (c[0].a || c[1].a || c[2].a || c[3].a)
  {
#ifndef NGL
    proftimer_widget_sendctx.start();
    mat.send_context( index, MAP_DIFFUSE, FORCE_TRANSLUCENCY );
    proftimer_widget_sendctx.stop();

    proftimer_widget_draw.start();
#endif

#ifdef NGL

#if 0
    // nglQuad has texture locking issues
    nglQuad q;
    nglInitQuad( &q );
    nglSetQuadZ( &q, z );
    nglSetQuadRect( &q, v[0][0], v[0][1], v[2][0], v[2][1] );
	nglSetQuadColor( &q, NGL_RGBA32( (u_int) (c[0].r * 255), (u_int) (c[0].g * 255), (u_int) (c[0].b * 255), (u_int) (c[0].a * 255) ) );
//    q.BlendMode = NGLBM_OPAQUE;

	nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
    nglSetQuadTex( &q, nglGetTextureA( (char*)texture_name.c_str() ) );
    nglSetQuadUV( &q,
      tc[uv_order[0]].get_u(),
      tc[uv_order[0]].get_v(),
      tc[uv_order[3]].get_u(),
      tc[uv_order[3]].get_v() );
    nglListAddQuad( &q );
#else
	assert(false);	// If we use ScratchMesh here, we should make it locked (dc 08/23/01)
    KSNGL_CreateScratchMesh( 4, mat.get_ngl_material(), false );

    float hfov, cx, cy, nearz, farz;
#if NGL > 0x010700
    nglGetProjectionParams( &hfov, &nearz, &farz );
#else
    nglGetProjectionParams( &hfov, &cx, &cy, &nearz, &farz );
#endif
    float nglZMax = 1;

    // this gets us a canonical z, where 0 is touching camera and 1 is far clip plane
    rational_t zz = geometry_manager::inst()->fix_z_coord(z);
    rational_t inverted_z = 1.0f - zz;


    /*
    float w_in_meters = 1.0f / rhw;
    // I thought our projection W = Z, and yet NEAR_PLANE_W != NEAR_PLANE_D.
    // what gives?
#pragma todo("Figure out why I need to reverse this.  I don't get it.  jdf 4/1/01")
    w_in_meters = PROJ_NEAR_PLANE_W - w_in_meters;
    float normalized_z = w / farz;  // can't use nearz because it's in front of clip plane
    float inverted_z = 1.0f - normalized_z;  // high numbers are near on PS2
    */
    float nglz = inverted_z * nglZMax;

    c[0] /= 2;
    nglMeshWriteVertexPCUV( v[0][0],
      v[0][1],
      nglz,
      c[0].to_color32().to_ulong(),
      tc[uv_order[0]].get_u(),
      tc[uv_order[0]].get_v() );
    nglMeshWriteVertexPCUV( v[1][0],
      v[1][1],
      nglz,
      c[0].to_color32().to_ulong(),
      tc[uv_order[1]].get_u(),
      tc[uv_order[1]].get_v() );
//#pragma fixme("uv order should match vertex order  jdf 4-1-01")
    nglMeshWriteVertexPCUV( v[3][0],
      v[3][1],
      nglz,
      c[0].to_color32().to_ulong(),
      tc[uv_order[2]].get_u(),
      tc[uv_order[2]].get_v() );
    nglMeshWriteVertexPCUV( v[2][0],
      v[2][1],
      nglz,
      c[0].to_color32().to_ulong(),
      tc[uv_order[3]].get_u(),
      tc[uv_order[3]].get_v() );

	// Material must be specified at mesh creation time now.  (dc 06/03/02)
//    KSNGL_ScratchSetMaterial( mat.get_ngl_material() );
    nglMatrix nglmat;
    nglIdentityMatrix(nglmat);
    nglRenderParams nglparams;
    memset( &nglparams, 0, sizeof(nglparams) );
//    nglparams.Flags = NGLP_FULL_MATRIX;	// Obsolete flag.  Was ignored previously.  (dc 05/30/02)

    START_PROF_TIMER( proftimer_render_add_mesh );
    nglListAddMesh( nglCloseMesh(), nglmat, &nglparams );
    STOP_PROF_TIMER( proftimer_render_add_mesh );
    /*
    nglPrintf( "widget %s at %.2f,%.2f,%.2f,%.2f, color %.2f,%.2f,%.2f,%.2f\n",
      mat.get_nglMaterial()->Map->FileName.c_str(),
      v[0][0] - 1792, v[0][1] - 1824, v[3][0] - 1792, v[3][1] - 1824,
      c[0].get_red(), c[0].get_green(), c[0].get_blue(), c[0].get_alpha() );
*/
#endif


#else

    vert_workspace_xformed_quad.lock(4);
    hw_rasta_vert_xformed* vert_it;
    vert_it = vert_workspace_xformed_quad.begin();

  #ifdef TARGET_PC
    // although we've called the D3D set center of projection function to provide a volume
    // that should automatically smash our z's into the front of the viewspace we're not actually
    // using D3D's transform loop, so we need to smash ourselves
    rational_t zz = geometry_manager::inst()->fix_z_coord(z);
  #else
    rational_t zz = z;
  #endif

    //color32 clr = c[0].to_color32(); // slight optimization: assume uniform color on all vertices

    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[0][0],v[0][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[0]];
    vert_it->diffuse = c[0].to_color32();
    ++vert_it;
    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[1][0],v[1][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[1]];
    vert_it->diffuse = c[1].to_color32();
    ++vert_it;
    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[3][0],v[3][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[2]];
    vert_it->diffuse = c[2].to_color32();
    ++vert_it;
    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[2][0],v[2][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[3]];
    vert_it->diffuse = c[3].to_color32();
    ++vert_it;
    vert_workspace_xformed_quad.unlock();
    hw_rasta::inst()->send_vertex_strip(vert_workspace_xformed_quad, 4/*,
           hw_rasta::SEND_VERT_SKIP_CLIP*/);
    proftimer_widget_draw.stop();
#endif
  }

  widget::render();
}

//-----------------------------------------------------------------
/*
buffered_bitmap_widget::buffered_bitmap_widget( const char *_widget_name,
  widget *_parent, short _x, short _y, int n )
  : bitmap_widget( _widget_name, _parent, _x, _y, n )
{
  scr_scale_x = 1.0f;
  scr_scale_y = 1.0f;
}

void buffered_bitmap_widget::set_screen_scale(rational_t xdim, rational_t ydim)
{
  scr_scale_x = xdim;
  scr_scale_y = ydim;
}

void buffered_bitmap_widget::render()
{
  if ( !is_shown() || !is_open() ) return;

  #ifdef TARGET_PC
  float urhw = 1.0f;
  #else
  float urhw = rhw;
  #endif

  int                      i;
  rational_t               v[4][2], sw, sh;
  color                    c[4];

  aggregate_vert_buf *vert_buf = matvertbufs.find(mat, 0, 0);

  // start out with the local coordinates
  sw = subrect.get_width () / abs_S[0] * scr_scale_x;
  sh = subrect.get_height() / abs_S[1] * scr_scale_y;
  v[0][0] = 0;
  v[0][1] = 0;
  v[1][0] = sw;
  v[1][1] = 0;
  v[2][0] = sw;
  v[2][1] = sh;
  v[3][0] = 0;
  v[3][1] = sh;

  for(i = 0; i < 4; ++i)
  {
    c[i] = color(1.0f, 1.0f, 1.0f, 1.0f);
    transform(v[i], c[i], i);
  }

  if (c[0].a || c[1].a || c[2].a || c[3].a)
  {
    //vert_workspace_xformed.lock(4);
    hw_rasta_vert_xformed    *vert_it;
    //vert_it = vert_workspace_xformed.begin();
	vert_buf->lock();
	vert_it = vert_buf->get_quads(1);

  #ifdef TARGET_PC
    rational_t zz = geometry_manager::inst()->fix_z_coord(z);
  #else
    rational_t zz = z;
  #endif

    //color32 clr = c[0].to_color32(); // slight optimization: assume uniform color on all vertices

    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[0][0],v[0][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[0]];
    vert_it->diffuse = c[0].to_color32();
    ++vert_it;
    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[1][0],v[1][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[1]];
    vert_it->diffuse = c[1].to_color32();
    ++vert_it;
    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[3][0],v[3][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[2]];
    vert_it->diffuse = c[2].to_color32();
    ++vert_it;
    vert_it->reset();
    vert_it->set_xyz_rhw(vector4d(v[2][0],v[2][1],zz,urhw));
    vert_it->tc[0] = tc[uv_order[3]];
    vert_it->diffuse = c[3].to_color32();
    ++vert_it;
    vert_buf->unlock();
    //vert_workspace_xformed.unlock();
    //proftimer_widget_draw.start();
    //hw_rasta::inst()->send_vertex_strip(vert_workspace_xformed, 4
    );
    //proftimer_widget_draw.stop();
  }

  widget::render();
}
*/

//-----------------------------------------------------------------
// widget system's interface with pfe_element_text

extern game *g_game_ptr;


text_widget::text_widget( const char *_widget_name, widget *_parent, short _x, short _y, rational_t scale,
             const stringx &str, stringx typeface )
  : widget( _widget_name, _parent, _x, _y )
{
  text_font = NULL;

  init( typeface );

  set_string( str );
//  set_justif( just );
  scale_to( scale, scale );

  // set rhw
  rhw = get_next_rhw_2d_val();
  z = get_pc_z( rhw );
}


text_widget::text_widget( const char *_widget_name, widget *_parent, short _x, short _y, stringx typeface )
  : widget( _widget_name, _parent, _x, _y )
{
  text_font = NULL;

  init( typeface );

  // set rhw
  rhw = get_next_rhw_2d_val();
  z = get_pc_z( rhw );
}

rational_t text_widget::get_width() const
{
  return text_font->text_width(m_tout);
//P    return text->get_width_in_pixels( abs_S[0] );
}

rational_t text_widget::get_height() const
{
  return text_font->text_height(m_tout);
//P    return text->get_height_in_pixels( abs_S[0] );
}

void text_widget::init( stringx &typeface )
{
  if (text_font)
  {
    text_font->unload();
    typeface_close(text_font);
    text_font = NULL;
  }

  type = WTYPE_Text;
  text_font = typeface_open(typeface);
  assert(text_font);
  text_font->load();
/*
  text_font = NEW typeface_def;
  text_font->open(typeface);
  text_font->load();
*/
/*P
  text = NEW pfe_element_text( g_game_ptr->get_pfe() );
  text->set_typeface( typeface, 0 );
  text->intro_end_position = vector2d( abs_x, abs_y );
  text->intro_end_color = color( 1.0f, 1.0f, 1.0f, 1.0f );
  text->intro_end_scale = 1.0f;
  text->load();
P*/
}


text_widget::~text_widget()
{
  if (text_font)
  {
    text_font->unload();
    typeface_close(text_font);
    text_font = NULL;
  }
/*P
  if ( text )
  {
    text->unload();
    delete text;
    text = NULL;
  }
P*/
}

void text_widget::set_string( stringx const &_prelocalized_text )
{
  stringx temp = _prelocalized_text;
  prelocalized_text = temp;
  m_tout = localize_text_safe( prelocalized_text );
//P  text->set_string( localize_text_safe( prelocalized_text ) );
}


void text_widget::frame_advance( time_value_t time_inc )
{
//P  text->advance_frame( time_inc );
  widget::frame_advance( time_inc );
};



void text_widget::render()
{
  if ( !is_shown() ) return;

  color c = abs_col[0];
  rational_t ox = abs_x;
  rational_t oy = abs_y;

  text_font->render(m_tout, c.to_color32(), ox, oy, z, rhw, abs_S[0]);
  widget::render();
}



void text_widget::flush()
{
//P  text->animation_flush();
  widget::flush();
}


void text_widget::set_rhw( rational_t _rhw )
{
  rhw = _rhw;
  widget::rhw_2d_val[rhw_2d_layer] -= WIDGET_RHW_INC;
}


void text_widget::set_layer( rhw_layer_e rhw_layer )
{
  set_rhw_2d_layer(rhw_layer);
  rhw = get_next_rhw_2d_val();
  z = get_pc_z( rhw );
  restore_last_rhw_2d_layer();
}


//-----------------------------------------------------------------



text_block_widget::block_info_t::block_info_t()
{
  scale = DEFAULT_TEXT_SCALE;
  just = JUSTIFY_LEFT;
  typeface = "interface\\maxfont00";
  line_spacing = 10;
  max_lines = 10;
  max_width = -1;
  break_substring = "#";
  text = (char *) 0;
  col = color( 1.0f, 1.0f, 1.0f, 1.0f );
}


text_block_widget::text_block_widget( const char *_widget_name, widget *_parent, short _x, short _y, block_info_t *info_ptr )
  : widget( _widget_name, _parent, _x, _y )
{
  type = WTYPE_TextBlock;
  block_info = *info_ptr;

  num_lines_used = 0;
  stringx temp = localize_text_safe( block_info.text );
  block_info.text = temp;

  // reserve block of rhws based on max_lines
  first_rhw = widget::rhw_2d_val[rhw_2d_layer];
  widget::rhw_2d_val[rhw_2d_layer] += block_info.max_lines * WIDGET_RHW_INC;

  text_font = NULL;
  text_font = typeface_open(block_info.typeface);
  assert(text_font);
  text_font->load();

  fill_lines();
}

text_block_widget::~text_block_widget()
{
  if(text_font)
  {
    text_font->unload();
    typeface_close(text_font);
    text_font = NULL;
  }
}

void text_block_widget::set_block_info( block_info_t *info_ptr )
{
  typeface_def *old_text_font = NULL;
  if(text_font && block_info.typeface != info_ptr->typeface)
  {
    old_text_font = text_font;
    text_font = NULL;
  }

  block_info = *info_ptr;

  if(!text_font)
  {
    text_font = typeface_open(block_info.typeface);
    assert(text_font);
    text_font->load();
  }

  if(old_text_font)
  {
    old_text_font->unload();
    typeface_close(old_text_font);
    old_text_font = NULL;
  }

  fill_lines();
}

void text_block_widget::set_text( stringx const &text )
{
  block_info.text = localize_text_safe( text );
  fill_lines();
}


void text_block_widget::clear_lines()
{
  flush();

  // delete children
  while ( children.size() )
  {
    delete *children.begin();
    children.erase( children.begin() );
  }
  num_lines_used = 0;
}


void text_block_widget::fill_lines()
{
  clear_lines();

  stringx text = block_info.text;
  int cur_pos = 0;

  rational_t rhw_val = first_rhw;

  while ( strlen( text.c_str() ) > 0 && num_lines_used < block_info.max_lines )
  {
    stringx line_text;
    if ( block_info.break_substring != empty_string )
    {
      int break_pos = text.find( block_info.break_substring.c_str() );
      if ( break_pos != stringx::npos )
      {
        line_text = text.substr( 0, break_pos );
        cur_pos = break_pos + strlen( block_info.break_substring.c_str() );
      }
      else
      {
        line_text = text;
        cur_pos = stringx::npos;
      }
    }
    else
    {
      line_text = text;
      cur_pos = stringx::npos;
    }

    if ( line_text != empty_string )
    {
      text_widget *tw = NEW text_widget( "block text", this, 0, num_lines_used * block_info.line_spacing,
                          block_info.scale, 0, block_info.typeface );
      tw->set_color( block_info.col );
      tw->set_rhw( rhw_val );
      rhw_val += WIDGET_RHW_INC;

      if ( is_shown() )
        tw->show();
      tw->set_string( line_text );

      // check width
      bool done = false;
      while ( block_info.max_width != -1 && !done && tw->get_width() > block_info.max_width )
      {
        int break_pos = line_text.rfind(' '); // line_text should begin at same point as text
        if ( break_pos == stringx::npos )
        {
          done = true;
        }
        else
        {
          stringx temp = line_text.substr( 0, break_pos );
          line_text = temp;
          tw->set_string( line_text );
          cur_pos = break_pos + 1;
        }
      }

      ++num_lines_used;
    }
    stringx temp = text.substr( cur_pos, stringx::npos );
    text = temp;
  }
}

void text_block_widget::set_color( color c )
{
  block_info.col = c;

  widget_list_t::const_iterator it = children.begin();
  widget_list_t::const_iterator it_end = children.end();
  for( ; it != it_end; ++it )
  {
    ((text_widget*)(*it))->set_color(c);
  }
}

void text_block_widget::set_scale( rational_t s )
{
  block_info.scale = s;

  widget_list_t::const_iterator it = children.begin();
  widget_list_t::const_iterator it_end = children.end();
  for( ; it != it_end; ++it )
  {
    ((text_widget*)(*it))->scale_to(s,s);
  }
}

void text_block_widget::set_line_spacing( rational_t s )
{
  block_info.line_spacing = (short)s;

  widget_list_t::const_iterator it = children.begin();
  widget_list_t::const_iterator it_end = children.end();
  for( int i = 0; it != it_end; ++it,++i)
  {
    ((text_widget*)(*it))->move_to(0,s*i);
  }
}

rational_t text_block_widget::get_width() const
{
  rational_t widest = 0.0f;
  widget_list_t::const_iterator it = children.begin();
  widget_list_t::const_iterator it_end = children.end();
  for( ; it != it_end; ++it )
  {
    rational_t width = ((text_widget*)(*it))->get_width();
    if ( width > widest )
    {
      widest = width;
    }
  }
  return widest;
}


rational_t text_block_widget::get_height() const
{
  rational_t height = 0.0f;

  if ( num_lines_used > 0 )
  {
    height = (num_lines_used - 1) * block_info.line_spacing;
    text_widget *last = (text_widget*)(children.back());
    if ( last )
    {
      height += last->get_height();
    }
  }

  return height;
}

void text_block_widget::set_layer( rhw_layer_e rhw_layer )
{
  set_rhw_2d_layer(rhw_layer);

  widget_list_t::const_iterator it = children.begin();
  widget_list_t::const_iterator it_end = children.end();
  for( ; it != it_end; ++it )
  {
    ((text_widget*)(*it))->rhw = get_next_rhw_2d_val();
    ((text_widget*)(*it))->z = get_pc_z( ((text_widget*)(*it))->rhw );
  }

  restore_last_rhw_2d_layer();
}

//-----------------------------------------------------------------

#ifdef NGL
vrep_widget::vrep_widget( const char *_widget_name, widget *_parent, short _x, short _y, nglMesh* _mesh )
:   widget( _widget_name, _parent, _x, _y )
{
  init();

  if ( _mesh )
    mesh = _mesh;
  else
    mesh = NULL;
}

#else

vrep_widget::vrep_widget( const char *_widget_name, widget *_parent, short _x, short _y, visual_rep* vr )
:   widget( _widget_name, _parent, _x, _y )
{
  init();

  if ( vr )
  {
    vrep = new_visrep_instance( vr );
  }
  else
  {
    vrep = NULL;
  }
}
#endif

vrep_widget::vrep_widget( char *filename, const char *_widget_name, widget *_parent, short _x, short _y )
:   widget( _widget_name, _parent, _x, _y )
{
  init();

#ifdef NGL
  filespec spec( filename );
  nglSetMeshPath( (char*)spec.path.c_str() );
  mesh = nglGetMesh( (char*)spec.name.c_str() );
#else
  vrep = load_new_visual_rep( stringx(filename), 0 );
  if( vrep && vrep->get_type()==VISREP_PMESH )
  {
    static_cast<vr_pmesh*>(vrep)->shrink_memory_footprint();
  }
#endif
}

void vrep_widget::init()
{
  type = WTYPE_Vrep;
  screen_radius = 32.0f;
  axis = vector3d( 0.0f, -1.0f, 0.0f );
  ax = ay = az = 0.0f;
  rps = 0.0f;

#ifdef NGL
  mesh = NULL;
#endif

  rhw_half_range = (rhw_layer_ranges[rhw_3d_layer][1]-rhw_layer_ranges[rhw_3d_layer][0])/2;
  rhw_midpoint = rhw_layer_ranges[rhw_3d_layer][0] + rhw_half_range;
  z = get_pc_z( rhw_midpoint );
}

void vrep_widget::set_layer( rhw_layer_e rhw_layer )
{
  set_rhw_3d_layer(rhw_layer);

  rhw_half_range = (rhw_layer_ranges[rhw_3d_layer][1]-rhw_layer_ranges[rhw_3d_layer][0])/2;
  rhw_midpoint = rhw_layer_ranges[rhw_3d_layer][0] + rhw_half_range;
  z = get_pc_z( rhw_midpoint );

  restore_last_rhw_3d_layer();
}


vrep_widget::~vrep_widget()
{
#ifdef NGL
  if ( mesh )
#if defined(TARGET_GC)
    nglReleaseMesh( mesh );
#else // defined(TARGET_XBOX)
    nglReleaseMeshFile( mesh->Name );
#endif // defined(TARGET_XBOX)

#else // NGL
  if ( vrep )
  {
    unload_visual_rep( vrep );
  }
#endif // NGL
}


void vrep_widget::show()
{
  widget::show();
  update_rot();
}

#ifdef NGL

void vrep_widget::set_mesh( nglMesh* m )
{
  mesh = m;
}

#else

void vrep_widget::set_vrep(visual_rep *vr)
{
  if ( vrep )
  {
    unload_visual_rep( vrep );
  }
  if ( vr )
  {
    vrep = new_visrep_instance( vr );
    update_rot();
  }
  else
  {
    vrep = NULL;
  }
}

void vrep_widget::set_vrep( const stringx &filename )
{
  if ( vrep )
  {
    unload_visual_rep( vrep );
  }
  vrep = load_new_visual_rep( stringx(filename), 0 );
  if( vrep && vrep->get_type()==VISREP_PMESH )
  {
    static_cast<vr_pmesh*>(vrep)->shrink_memory_footprint();
  }
  update_rot();
}
#endif

void vrep_widget::frame_advance( time_value_t time_inc )
{
  // update any rotation
  if ( rps )
  {
    angle += rps * time_inc;
    angle = fmodf(angle,2.0f*PI);
    update_rot();
  }

  widget::frame_advance( time_inc );

#ifndef NGL
  // call pmesh frame advance for uv animation update
  if( vrep && vrep->get_type()==VISREP_PMESH )
    static_cast<vr_pmesh*>(vrep)->anim_uvs( time_inc );
#endif
}

#if defined (TARGET_PC)
  rational_t g_scale_factor = 5.0f;  // 5.14f
#else
  rational_t g_scale_factor = 10.0f;  // 12.0f
#endif

rational_t farzzz = 1.0f;
rational_t nearzzz = 0.2f;

void vrep_widget::render()
{
  if ( !is_shown() )
    return;

#ifdef NGL
  if ( mesh )
  {
    nglMatrix nglViewToScreen;
    nglGetMatrix (nglViewToScreen, NGLMTX_VIEW_TO_SCREEN);

    nglMatrix m;
    nglRenderParams p;
//    p.Flags = NGLP_FULL_MATRIX;	// Obsolete flag.  Was ignored previously.  (dc 05/30/02)

//#pragma fixme("The kludge to get/set the projection params is \"Ugly\", so saith Jaime (4/3/01)")
    float hfov, cx, cy, nearz, farz;
#if NGL > 0x010700
    nglGetProjectionParams( &hfov, &nearz, &farz );
#else
    nglGetProjectionParams( &hfov, &cx, &cy, &nearz, &farz );
#endif

    color cl = color(1.0f, 1.0f, 1.0f, 1.0f);
    rational_t v[2] = { 0.0f, 0.0f };
    transform(v, cl, 0);

//#pragma fixme("This allows for stuff to clip through geometry.  That's bad.  (GT--4/3/01)")
    ksnglSetOrthoMatrix( v[0], v[1], nearzzz, farzzz ); //!!!!!VREPWIDGET_MIN_Z, VREPWIDGET_MAX_Z );

    nglMulMatrix( m, nglViewToScreen, native_to_ngl( mat ) );

    START_PROF_TIMER( proftimer_render_add_mesh );
    nglListAddMesh( mesh, m, &p );
    STOP_PROF_TIMER( proftimer_render_add_mesh );

    ksnglSetPerspectiveMatrix( hfov, cx, cy, nearz, farz );
  }
#else
  if ( vrep )
  {
    float prevcopx,prevcopy,prevminz,prevmaxz;
    geometry_manager::inst()->get_cop(&prevcopx,&prevcopy,&prevminz,&prevmaxz);

    color cl = color(1.0f, 1.0f, 1.0f, 1.0f);
    rational_t v[2] = { 0.0f, 0.0f };
    transform(v, cl, 0);
    geometry_manager::inst()->set_cop(v[0], v[1], VREPWIDGET_MIN_Z, VREPWIDGET_MAX_Z);

    cur_rhw_midpoint = rhw_midpoint;
    cur_rhw_half_range = rhw_half_range;
    cur_special_w_xform = special_w_xform;
    cur_largest_z = largest_z;

/*
    po render_po;
    render_po.get_matrix() = mat;
    rational_t scale = render_po.get_scale();
    vector3d pos = render_po.get_position();
    render_po.fixup();
    render_po.scale(scale);
    render_po.set_position(pos);
*/

    instance_render_info iri(vrep->get_max_faces(0), mat, 0.0f, NULL, 0, cl.to_color32(), FORCE_TRANSLUCENCY|FORCE_SKIP_CLIP);
    vrep->render_instance(RENDER_RAW_ICON_THROUGHPUT|RENDER_TRANSLUCENT_PORTION, &iri);

    geometry_manager::inst()->set_cop(prevcopx,prevcopy,prevminz,prevmaxz);
  }

  widget::render();
#endif
}



void vrep_widget::set_rotation(vector3d &u, rational_t a, rational_t s)
{
  axis = u;
  angle = a;
  rps = s;
  update_rot();
}


void vrep_widget::set_rotation( rational_t _ax, rational_t _ay, rational_t _az )
{
  ax = _ax;
  ay = _ay;
  az = _az;
  update_rot();
}


void vrep_widget::update_pos()
{
  widget::update_pos();
  update_mat();
}


void vrep_widget::update_scale()
{
  widget::update_scale();
  update_mat();
}


void vrep_widget::update_rot()
{
  widget::update_rot();

  matrix4x4 R, Rx, Ry, Rz;
  R.make_rotate( axis, angle );
  Rx.make_rotate( vector3d( 1, 0, 0 ), ax );
  Ry.make_rotate( vector3d( 0, 1, 0 ), ay );
  Rz.make_rotate( vector3d( 0, 0, 1 ), az );
  rot_matrix = R * Rx * Ry * Rz;

  // copied from po::fixup()
/*
  vector3d new_x(rot_matrix[0][0],rot_matrix[1][0],rot_matrix[2][0]);
  vector3d new_y(rot_matrix[0][1],rot_matrix[1][1],rot_matrix[2][1]);
  vector3d new_z;
  new_x.normalize();
  new_z = cross(new_x,new_y);
  new_z.normalize();
  new_y = cross(new_z,new_x);
  rot_matrix[0][0] = new_x.x;
  rot_matrix[1][0] = new_x.y;
  rot_matrix[2][0] = new_x.z;
  rot_matrix[0][1] = new_y.x;
  rot_matrix[1][1] = new_y.y;
  rot_matrix[2][1] = new_y.z;
  rot_matrix[0][2] = new_z.x;
  rot_matrix[1][2] = new_z.y;
  rot_matrix[2][2] = new_z.z;
*/

  update_mat();
}

bool g_special_sorting = true;
void vrep_widget::update_mat()
{
  // Okay, I think I have this code duplicated to work in NGL, but what does it do?
#ifdef NGL
  if ( !mesh )
    return;
  vector3d center( mesh->SphereCenter[0], mesh->SphereCenter[1],
    mesh->SphereCenter[2] );
  float radius = mesh->SphereRadius;
#else
  if ( !vrep )
    return;
  vector3d center = -vrep->get_center(0);
  float radius = vrep->get_radius(0);
#endif

  matrix4x4 S1, T0, T1;
  rational_t s, fSin, fCos;

  const rational_t SPECIAL_NEAR_DIST = PROJ_NEAR_PLANE_D;

  fast_sin_cos_approx( (PROJ_FIELD_OF_VIEW * 0.5f), &fSin, &fCos );
  s = ((screen_radius / 320.0f) * (SPECIAL_NEAR_DIST) * fSin) / (radius * fCos);

  s *= g_scale_factor;

#ifndef NGL
  // TODO : Scaling support in NGL.
  S1.make_scale(vector3d(s * abs_S[0], s * abs_S[0], s * abs_S[0]));
#else
  S1.make_scale(vector3d(s * abs_S[0], s * abs_S[0], s * abs_S[0]));
//  S1 = identity_matrix;
#endif

  T0.make_translate(-center);
//#pragma todo("This is probably not the ideal solution. (GT--4/3/01)")
  float kludged_z = z;
#ifdef NGL
  kludged_z *= VREPWIDGET_MAX_Z;
#endif
  T1.make_translate(vector3d(0.0f, 0.0f, kludged_z ));
  mat = T0 * rot_matrix * S1 * T1;
}


void vrep_widget::calc_largest_z()
{
  largest_z = 0.0f;
#ifndef NGL
  if ( vrep && vrep->get_type() == VISREP_PMESH )
  {
    cur_special_w_xform = special_w_xform;
    vr_pmesh *mesh = (vr_pmesh*)vrep;
    int num_verts = mesh->get_num_wedges();
    for ( int i = 0; i < num_verts; ++i )
    {
      // apply w_xform to z
      rational_t abs_z = abs((int)calc_z( mesh->get_xvert_unxform_pos(i) ));
      if ( abs_z > largest_z )
        largest_z = abs_z;
    }
  }
#endif
}


//------------------------------------------------------------------


background_widget::background_widget( const char *_widget_name, widget *_parent, short _x, short _y, const stringx &file_prefix )
  : widget( _widget_name, _parent, _x, _y )
{
  const int wu = nglGetScreenWidth() / 5;
  const int hu = 256;

  const int tw[] = { wu, 2*wu, 2*wu, wu, 2*wu, 2*wu };
  const int tx[] = { 0,    wu, 3*wu,  0,   wu, 3*wu };
  const int ty[] = { 0,     0,    0, hu,   hu,   hu };

  for ( int i = 0; i < 6; ++i )
  {
    widget::set_rhw_2d_layer( widget::RHW0 );
    bitmap_widget *bitmap = NEW bitmap_widget( "bg", this, tx[i], ty[i], 1 );
    stringx filename = file_prefix + stringx(i);
    bitmap->open( filename.c_str() );
    bitmap->resize( tw[i], hu );
    widget::restore_last_rhw_2d_layer();
  }
}


//------------------------------------------------------------------



box_widget::box_widget( const char *_widget_name, widget *_parent, short _x, short _y, rational_t _w, rational_t _h,
                       const stringx& corner_name, const stringx& side_name, rational_t _w_thick, rational_t _h_thick )
  : widget( _widget_name, _parent, _x, _y )
{
  for ( int i = 0; i < 4; ++i )
  {
    corner[i] = NEW bitmap_widget( "box-corner", this, 0, 0, 1 );
    corner[i]->open( corner_name.c_str() );

    side[i] = NEW bitmap_widget( "box-side", this, 0, 0, 1 );
    side[i]->open( side_name.c_str() );
  }

  corner[0]->flip_horiz();
  corner[3]->flip_horiz();
  corner[3]->flip_vert();
  corner[2]->flip_vert();

  resize( _w, _h, _w_thick, _h_thick );
}


void box_widget::resize( rational_t _w, rational_t _h, rational_t _w_thick, rational_t _h_thick )
{
  w = _w;
  h = _h;
  w_thick = _w_thick;
  h_thick = _h_thick;

  for ( int i = 0; i < 4; ++i )
  {
    corner[i]->resize( _w_thick, _h_thick );
  }

  side[0]->resize( w - w_thick * 2.0f, h_thick );
  side[0]->move_to( short( w_thick ), 0 );
  corner[0]->move_to( 0, 0 );

  side[1]->resize( w_thick, h - h_thick * 2.0f );
  side[1]->move_to( short( w ) - short( w_thick ), short( h_thick ) );
  corner[1]->move_to( short( w ) - short( w_thick ), 0 );

  side[2]->resize( w - w_thick * 2.0f, h_thick );
  side[2]->move_to( short( w_thick ), short( h ) - short( h_thick ) );
  corner[2]->move_to( short( w ) - short( w_thick ), short( h ) - short( h_thick ) );

  side[3]->resize( w_thick , h - h_thick * 2.0f );
  side[3]->move_to( 0, short( h_thick ) );
  corner[3]->move_to( 0, short( h ) - short( h_thick ) );
}




//------------------------------------------------------------------

bar_widget::bar_widget( const char *_widget_name, widget *_parent, short _x, short _y, widget_dir_e _dir )
  : widget( _widget_name, _parent, _x, _y ), dir( _dir ), val( -1.0f ), full_val( 1.0f )
{
  x_fac = y_fac = 0;

  if ( dir == WDIR_Left )
  {
    x_fac = -1;
  }
  else if (dir == WDIR_Right )
  {
    x_fac = 1;
  }
  else if (dir == WDIR_Up )
  {
    y_fac = 1;
  }
  else if (dir == WDIR_Down )
  {
    y_fac = -1;
  }
}


//------------------------------------------------------------------


fluid_bar::fluid_bar( const char *_widget_name, widget *_parent, short _x, short _y, widget_dir_e _dir, rational_t _w, rational_t _h, const stringx& _name )
  : bar_widget( _widget_name, _parent, _x, _y, _dir )
{
  w = _w;
  h = _h;
  name = _name;
  full_val = old_full_val = 1.0f;
  to_val = old_to_val = 1.0f;
  cur_val = old_cur_val = 0.0f;
  fill_rate = empty_rate = special_rate = old_fill_rate = old_empty_rate = 1.0f;
  update = true;
  using_special_rate = false;
  col[0] = col[1] = col[2] = col[3] = color( 1.0f, 1.0f, 1.0f, 1.0f );

  bar_map = NEW bitmap_widget( "fluid-bar_map", this, 0, 0, 1 );
  bar_map->open( name.c_str() );
  bar_map->resize( w, h );
  bar_map->set_color( col[0] );
}


void fluid_bar::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

  assert( full_val >= 0.0f );
  assert( to_val >= 0.0f && to_val <= full_val );
  assert( cur_val >= 0.0f && cur_val <= full_val );

  if ( update || cur_val != to_val )
  {
    if ( cur_val < to_val )
    {
      if ( using_special_rate )
      {
        cur_val += time_inc * full_val * special_rate;
      }
      else
      {
        cur_val += time_inc * full_val * fill_rate;
      }

      if ( cur_val > to_val )
      {
        cur_val = to_val;
      }
    }
    else
    {
      if ( using_special_rate )
      {
        cur_val -= time_inc * full_val * special_rate;
      }
      else
      {
        cur_val -= time_inc * full_val * empty_rate;
      }

      if ( cur_val < to_val )
      {
        cur_val = to_val;
      }
    }

    int adj_w = int( w * ( cur_val / full_val ) + 0.5f );
    int adj_h = int( h * ( cur_val / full_val ) + 0.5f );

    int x0 = 0, y0 = 0, x1 = int( w ), y1 = int( h );

    // determine subrect to show
    if ( dir == WDIR_Left )
    {
      x0 = int( w ) - adj_w;
    }
    else if ( dir == WDIR_Right )
    {
      x1 = adj_w;
    }
    else if ( dir == WDIR_Up )
    {
      y0 = int( h ) - adj_h;
    }
    else if ( dir == WDIR_Down )
    {
      y1 = adj_h;
    }

    bar_map->set_subrect( x0, y0, x1, y1 );
    bar_map->move_to( x0, y0 );

    update = false;
    old_full_val = full_val;
    old_to_val = to_val;
    old_cur_val = cur_val;
    old_fill_rate = fill_rate;
    old_empty_rate = empty_rate;

    if ( cur_val == to_val )
    {
      using_special_rate = false;
    }
  }

  widget::frame_advance( time_inc );
}


void fluid_bar::resize( rational_t _w, rational_t _h )
{
  w = _w;
  h = _h;

  if ( bar_map )
  {
    bar_map->resize( w, h );
    update = true;
  }
}

void fluid_bar::render()
{
  hw_rasta::inst()->send_start( hw_rasta::PT_TRANS_POLYS );

  hw_rasta::inst()->set_zbuffering( true, true );

  bar_widget::render();
}


//------------------------------------------------------------------

// widget used for positioning of other widgets:
//
// right + x engages it for interface; right + y for pda
// left + x removes it
// d-pad moves through widget tree
// left + d-pad changes coordinates


layout_widget::layout_widget( const char *_widget_name, widget *_parent, short _x, short _y )
  : widget( _widget_name, _parent, _x, _y )
{
  cur_widget = parent;

  set_rhw_2d_layer( RHW_OVER_PFE1 );

  rational_t bg_w = 300.0f, bg_h = 75.0f;
  short bg_x = 0, bg_y = 0; //bg_x = 315, bg_y = 90;
  short text_x = bg_x + short(bg_w) / 2;
  short text1_y = bg_y + 10, text2_y = bg_y + 30, text3_y = bg_y + 50, text4_y = bg_y + 70, text5_y = bg_y + 90, text6_y = bg_y + 110;
  bg = NEW bitmap_widget( "layout-bg", this, bg_x, bg_y, 1 );
  bg->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
  bg->resize( bg_w, bg_h );
  bg->set_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );

  name = NEW text_widget( "layout-name", this, text_x, text1_y, 0.7f );
  pos = NEW text_widget( "layout-pos", this, text_x, text2_y, 0.8f );
  abs_pos = NEW text_widget( "layout-abs pos", this, text_x, text3_y, 0.8f );
  x_angle = NEW text_widget( "layout-x ang", this, text_x, text4_y, 0.7f );
  y_angle = NEW text_widget( "layout-y ang", this, text_x, text5_y, 0.7f );
  z_angle = NEW text_widget( "layout-z ang", this, text_x, text6_y, 0.7f );

  restore_last_rhw_2d_layer();

  update_text();

  hide();
  ignore_parent_showing();
}


#define WAIT_TIME     0.1f  // between position changes
#define SPEEDUP_TIME  1.0f  // after this amount of time of button press, things speed up

void layout_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

  static time_value_t elapsed = 0.0f, elapsed2 = 0.0f;
  static time_value_t wt = 0.1f;

  elapsed += time_inc;
  elapsed2 += time_inc;
  if ( elapsed2 > SPEEDUP_TIME )
  {
    elapsed2 = 0.0f;
    wt = 0.0f;
  }

  // deactivate?
  if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, ACTION_ZOOMIN ) == AXIS_MAX &&
       input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, NANOTECH ) == AXIS_MAX )
  {
    elapsed = elapsed2 = 0.0f;
    wt = WAIT_TIME;
    hide();
  }
  else
  {
    // up a level?
    if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PLAYER_STRAFE_LEFT ) != AXIS_MAX )
    {
      elapsed2 = 0.0f;
      wt = WAIT_TIME;

      if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_UPDOWN ) == AXIS_MIN &&
           input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PFE_UPDOWN ) == AXIS_MIN )
      {
        if ( cur_widget->get_parent() )
        {
          cur_widget = cur_widget->get_parent();
          update_text();
        }
      }
      // down a level?
      else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_UPDOWN ) == AXIS_MAX &&
                input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PFE_UPDOWN ) == AXIS_MAX )
      {
        if ( cur_widget->get_first_child() )
        {
          cur_widget = cur_widget->get_first_child();
          update_text();
        }
      }
      // prev on this level?
      else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MIN &&
                input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MIN )
      {
        if ( cur_widget->get_parent() && cur_widget->get_parent()->get_prev_child( cur_widget ) )
        {
          cur_widget = cur_widget->get_parent()->get_prev_child( cur_widget );
          update_text();
        }
      }
      // next on this level?
      else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MAX &&
                input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MAX )
      {
        if ( cur_widget->get_parent() && cur_widget->get_parent()->get_next_child( cur_widget ) )
        {
          cur_widget = cur_widget->get_parent()->get_next_child( cur_widget );
          update_text();
        }
      }
    }
    else
    {
      if ( elapsed > wt )
      {
        bool updated = false;
        elapsed = 0.0f;

        // change position?
        if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MIN )
        {
          update_position( -1, 0 );
          updated = true;
        }
        else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MAX )
        {
          update_position( 1, 0 );
          updated = true;
        }

        if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_UPDOWN ) == AXIS_MIN )
        {
          if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, SHOOT_PUNCH ) == AXIS_MAX )
          {
            update_rotation( -1, 0, 0 );
            updated = true;
          }
          else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, KICK_ZOOMOUT ) == AXIS_MAX )
          {
            update_rotation( 0, -1, 0 );
            updated = true;
          }
          else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, ACTION_ZOOMIN ) == AXIS_MAX )
          {
            update_rotation( 0, 0, -1 );
            updated = true;
          }
          else
          {
            update_position( 0, -1 );
            updated = true;
          }
        }
        else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_UPDOWN ) == AXIS_MAX )
        {
          if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, SHOOT_PUNCH ) == AXIS_MAX )
          {
            update_rotation( 1, 0, 0 );
            updated = true;
          }
          else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, KICK_ZOOMOUT ) == AXIS_MAX )
          {
            update_rotation( 0, 1, 0 );
            updated = true;
          }
          else if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, ACTION_ZOOMIN ) == AXIS_MAX )
          {
            update_rotation( 0, 0, 1 );
            updated = true;
          }
          else
          {
            update_position( 0, 1 );
            updated = true;
          }
        }

        if ( !updated )
        {
          elapsed2 = 0.0f;
          wt = WAIT_TIME;
        }
      }
    }
  }

  widget::frame_advance( time_inc );
}


void layout_widget::update_text()
{
  if ( cur_widget )
  {
    name->set_string( cur_widget->get_name() );
    char buf[50];
    sprintf( buf, "x %i, y %i", cur_widget->get_x(), cur_widget->get_y() );
    pos->set_string( stringx( buf ) );
    sprintf( buf, "abs x = %i, abs y = %i", cur_widget->get_abs_x(), cur_widget->get_abs_y() );
    abs_pos->set_string( stringx( buf ) );
  }
}


void layout_widget::update_position( short dx, short dy )
{
  if ( cur_widget )
  {
    cur_widget->move_to( cur_widget->get_x() + dx, cur_widget->get_y() + dy );
    char buf[50];
    sprintf( buf, "x %i, y %i", cur_widget->get_x(), cur_widget->get_y() );
    pos->set_string( stringx( buf ) );
    sprintf( buf, "abs x = %i, abs y = %i", cur_widget->get_abs_x(), cur_widget->get_abs_y() );
    abs_pos->set_string( stringx( buf ) );
  }
}


void layout_widget::update_rotation( short dax, short day, short daz )
{
  if ( cur_widget )
  {
    if ( cur_widget->get_type() == WTYPE_Vrep )
    {
      vrep_widget *cw = (vrep_widget*)cur_widget;
      cw->set_rotation( cw->get_ax() + rational_t( rational_t(dax) * PI / 180.0f ),
                        cw->get_ay() + rational_t( rational_t(day) * PI / 180.0f ),
                        cw->get_az() + rational_t( rational_t(daz) * PI / 180.0f ) );
      char buf[50];
      sprintf( buf, "ang x = %i", short(cw->get_ax() * 180.0f / PI) );
      x_angle->set_string( stringx( buf ) );
      sprintf( buf, "ang x = %i", short(cw->get_ay() * 180.0f / PI) );
      y_angle->set_string( stringx( buf ) );
      sprintf( buf, "ang x = %i", short(cw->get_az() * 180.0f / PI) );
      z_angle->set_string( stringx( buf ) );
    }
    else if ( cur_widget->get_type() == WTYPE_Entity )
    {
      entity_widget *cw = (entity_widget*)cur_widget;
      cw->set_rotation( cw->get_ax() + rational_t( rational_t(dax) * PI / 180.0f ),
                        cw->get_ay() + rational_t( rational_t(day) * PI / 180.0f ),
                        cw->get_az() + rational_t( rational_t(daz) * PI / 180.0f ) );
      char buf[50];
      sprintf( buf, "ang x = %i", short(cw->get_ax() * 180.0f / PI) );
      x_angle->set_string( stringx( buf ) );
      sprintf( buf, "ang y = %i", short(cw->get_ay() * 180.0f / PI) );
      y_angle->set_string( stringx( buf ) );
      sprintf( buf, "ang z = %i", short(cw->get_az() * 180.0f / PI) );
      z_angle->set_string( stringx( buf ) );
    }
    else
    {
      x_angle->set_string( stringx( 0 ) );
      y_angle->set_string( stringx( 0 ) );
      z_angle->set_string( stringx( 0 ) );
    }
  }
}

