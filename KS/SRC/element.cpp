// element.cpp :: graphic elements for basic interface rendering

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
#include "element.h"
#include "renderflav.h"
#include "vertwork.h"
#include "hwmath.h"
#include "text_font.h"

#include <cstdarg>

DEFINE_SINGLETON(element_manager)


#ifdef TARGET_PC
#define INTERFACE_FONT      "interface\\pc_font"
#else
#define INTERFACE_FONT      "interface\\font"
#endif


//refptr<hw_texture> allblack;
font_def       *font;
static float    rhw_depth;

//-----------------------------------------------------------------
void element_manager::create_default_elements()
{
  font = NEW font_def;
  interface_font_name = INTERFACE_FONT;
}


//-----------------------------------------------------------------
void element_manager::restore_default_elements()
{
  //black_texture_id = hw_texture_mgr::inst()->load_texture("interface\\fadetoblack");
  //white_texture = hw_texture_mgr::inst()->load_texture("interface\\white");
  // NOT OPENED AUTOMATICALLY ANY MORE
  // BUT ON-DEMAND
  font->open( interface_font_name.c_str());
}

//-----------------------------------------------------------------
element_manager::element_manager() : interface_font_name("UNTITLED")
{
  next_id = 0;
// element_map[-1] = NULL;
  font = 0;
  enabled = true;
}

//-----------------------------------------------------------------
element_manager::~element_manager()
{
  delete font;
}


//-----------------------------------------------------------------


void element_manager::push_context( const stringx& name )
{
  context* newlist = NEW context( name );
  context_stack.push_front( newlist );
}

void element_manager::pop_context()
{
  element_list_t::iterator it = context_stack.front()->begin();
  element_list_t::iterator it_end = context_stack.front()->end();

  while ( it != it_end )
  {
    // delete removes item from list, destroying the current iterator, hence it++ doesn't work properly in a for loop
    element_list_t::iterator del = it;
    ++it;

    delete *del;
  }

  delete context_stack.front();
  context_stack.pop_front();
}


void element_manager::purge()
{
  while( context_stack.size() )
  {
    pop_context();
  }

//  element_map.erase( element_map.begin(), element_map.end() );
  next_id = 0;
//  element_map[-1] = NULL;
// NB: TEXTURE ALREADY TOASTED BY EVIL OVERLORDS AT THIS TIME
// THOUGH WE THINK WE STILL HOLD SOMETHING NEAR AND DEAR
  if (font->is_valid())
    font->unload();
//  delete font;
//  font = 0;
//  allblack=NULL;
}


void element_manager::frame_advance( time_value_t time_inc )
{
//  element_list_t::iterator it;

  if ( !enabled ) return;

  // print time delta to screen
#if 0
  char work_str[256];
  sprintf( work_str, "time_inc: %2.5f", time_inc );
  hw_rasta::inst()->print( work_str, vector2di(250,52), color32_grey ); // 450, 60
#endif
//#pragma todo("Find out why the element manager is being funky and crashing NY2 (GT-2/3/01)")
/*
  for( it = context_stack.front()->begin(); it != context_stack.front()->end(); ++it )
  {
    (*it)->frame_advance( time_inc );
  }
*/
}


static float g_initial_rhw_depth = PROJ_NEAR_PLANE_RHW;
//static float g_rhw_depth_inc = 0.1f;


void element_manager::render()
{
  if ( !enabled ) return;

  // save the projection matrix
  projection_matrix = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_PROJECTION];

  geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
  geometry_manager::inst()->set_xform(geometry_manager::XFORM_WORLD_TO_VIEW, identity_matrix);
  geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, identity_matrix);

  rhw_depth = g_initial_rhw_depth;
//#pragma todo("Find out why the element manager is being funky and crashing NY2 (GT-2/3/01)")
/*
  element_list_t::iterator it;

  for( it = context_stack.front()->begin(); it != context_stack.front()->end(); ++it )
  {
    if( (*it)->is_visible() )
    {
      (*it)->render();
    }
  }
*/
  geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, projection_matrix);
}


/*

rational_t element_manager::bind_element(element *e)
{
  int id;

  id = next_id++;
  element_map[id] = e;

  return (rational_t)id;
}

element *element_manager::lookup_element(rational_t id)
{
  return element_map[(int)id];
}

*/

//-----------------------------------------------------------------

inline time_value_t event::active_time_elapsed()
{
  if ( elapsed < wait_time )
    return 0.0f;
  else if ( elapsed > wait_time + duration )
    return duration;
  else
    return ( elapsed - wait_time );
}

inline time_value_t event::active_time_left()
{
  if ( elapsed < wait_time )
    return duration;
  else if ( elapsed >= wait_time + duration )
    return 0.0f;
  else
    return ( (duration + wait_time) - elapsed );
}

inline time_value_t event::total_time_left()
{
  if ( elapsed >= wait_time + duration )
    return 0.0f;
  else
    return ( (duration + wait_time) - elapsed );
}

inline time_value_t event::time_till_active()
{
  if ( elapsed >= wait_time )
    return 0.0f;
  else
    return ( wait_time - elapsed );
}


void event::set_time_left( const time_value_t& t )
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

const float event::get_lerp( const time_value_t& t )
{
  float lerp;

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
  hw_rasta::inst()->print( work_str, vector2di(250,52), color32_grey ); // 450, 60
#endif

  return lerp;
}



void color_event::do_event( const float& lerp )
{
  // blend the color toward the destination color based on the elapsed time
  for ( int i = 0; i < 4; ++i )
  {
    owner->mycolor[i].r += (mycolor.r - owner->mycolor[i].r) * lerp;
    owner->mycolor[i].g += (mycolor.g - owner->mycolor[i].g) * lerp;
    owner->mycolor[i].b += (mycolor.b - owner->mycolor[i].b) * lerp;
    owner->mycolor[i].a += (mycolor.a - owner->mycolor[i].a) * lerp;
  }
}

void move_event::do_event( const float& lerp )
{
  // translate the object toward the destination based on the elapsed time
  owner->T[0] += (x - owner->T[0]) * lerp;
  owner->T[1] += (y - owner->T[1]) * lerp;
}

void rotate_event::do_event( const float& lerp )
{
  float st, ct;

  owner->angle += (angle - owner->angle) * lerp;
  fast_sin_cos_approx( owner->angle, &st, &ct );
  owner->R[0][0] = ct; owner->R[0][1] = -st;
  owner->R[1][0] = st; owner->R[1][1] = ct;
}

void scale_event::do_event( const float& lerp )
{
  owner->S[0] += (sx - owner->S[0]) * lerp;
  owner->S[1] += (sy - owner->S[1]) * lerp;
}


void event::frame_advance( time_value_t time_inc )
{
  elapsed += time_inc;

  do_event( get_lerp( time_inc ) );
}


//-----------------------------------------------------------------


element::element( element *p )
{
  parent = p;
  if ( parent )
    parent->subelements.push_back( this );
  else
    element_manager::inst()->push_back_element( this );
  type = ELEMENT_Container;
  flags = 0;
  R[0][0] = R[1][1] = 1.0f;
  R[0][1] = R[1][0] = 0.0f;
  T[0] = T[1] = 0.0f;
  S[0] = S[1] = S_Override[0] = S_Override[1] = 1.0f;
  O[0] = O[1] = 0.0f;
  for ( int i = 0; i < 4; ++i )
  {
    mycolor[i].r = mycolor[i].g = mycolor[i].b = mycolor[i].a = 1.0f;
  }
  angle = 0.0f;
  linear_animation = true;
}

element::~element()
{
  if ( parent )
    parent->subelements.remove( this );
  else
    element_manager::inst()->remove_element( this );

  event_flush();
//  element_list_t::iterator eli; // unused -- remove me?
  while( subelements.size() )
    delete *subelements.begin();
}



// add NEW event to queue for this element; resolve overlap in time with other events of same type
void element::add_event( event* e )
{
  bool found_insert_pt = false;
  event_list_t::iterator it = event_run_list.begin(),
                         end_it = event_run_list.end(),
                         save_it = event_run_list.end();

  // process overlaps between events of same type, find insertion point
  for( ; it != end_it; ++it )
  {
    if ( !found_insert_pt && e->time_till_active() < (*it)->time_till_active() )
    {
      save_it = it;
      found_insert_pt = true;
    }
    if ( found_insert_pt )
    {
      // avoid overlap in time with following events of same type
      if ( (*it)->get_type() == e->get_type() && e->total_time_left() > (*it)->time_till_active() )
      {
         e->set_time_left( (*it)->time_till_active() );
      }
    }
    else
    {
      // avoid overlap in time with previous events of same type
      if ( (*it)->get_type() == e->get_type() && (*it)->total_time_left() > e->time_till_active() )
      {
         (*it)->set_time_left( e->time_till_active() );
      }
    }
  }
  if ( save_it == end_it )
  {
    // if not inserted, put at end of list
    event_run_list.push_back( e );
  }
  else
  {
    event_run_list.insert( save_it, e );
  }
}

// advance all the events queued for this element
void element::frame_advance( time_value_t time_inc )
{
  event_list_t::iterator it;
  element_list_t::iterator iu;

  for( it = event_run_list.begin(); it != event_run_list.end(); )
  {
    (*it)->frame_advance( time_inc );

    // complete? remove from list
    if ( (*it)->no_time_left() )
    {
      delete *it;
      it = event_run_list.erase( it );
    }
    else
    {
      ++it;
    }
  }

  for( iu = subelements.begin(); iu != subelements.end(); ++iu )
  {
    (*iu)->frame_advance( time_inc );
  }
}


void element::event_flush()
  {
  event_list_t::iterator   it;
  element_list_t::iterator iu;

  for( it = event_run_list.begin(); it != event_run_list.end(); ++it )
  {
    (*it)->do_event( 1.0f );
    delete *it;
  }
  event_run_list.erase(event_run_list.begin(), event_run_list.end());

  for( iu = subelements.begin(); iu != subelements.end(); ++iu )
  {
    (*iu)->event_flush();
  }
}



void element::offset(float v[2])
{
  // translate these to the origin
  v[0] -= O[0];
  v[1] -= O[1];
}

void element::transform(float v[2], color &c, int index)
{
  assert(index < 4);
  float xt, yt;

  // scale them
  v[0] *= S[0] * S_Override[0];
  v[1] *= S[1] * S_Override[1];
  // rotate them
  xt = v[0] * R[0][0] + v[1] * R[0][1];
  yt = v[0] * R[1][0] + v[1] * R[1][1];
  v[0] = xt;
  v[1] = yt;
  // translate them
  v[0] += T[0];
  v[1] += T[1];
  if(parent)
  {
    parent->transform(v, c, index);
  }
  c.r *= mycolor[index].r;
  c.g *= mycolor[index].g;
  c.b *= mycolor[index].b;
  c.a *= mycolor[index].a;
}

void element::ndc(float v[2])
{
  // if the buck stops here NDC them
  v[0] = (v[0] - 320.0f) / 320.0f;
  v[1] = (240.0f - v[1]) / 240.0f;
}



text_element::~text_element()
{
}

void text_element::render()
{
  color c;
  float health_xoff = 0.0f;
  float health_yoff = -4.0f;

  const font_char_info *ci = font->get_char_info('0');
  if( text.size() == 2 )
  {
    health_xoff = (ci->x1-ci->x0) * 0.5f;
  }
  if( text.size() == 1 )
  {
    health_xoff = (ci->x1-ci->x0) * 1.5f;
  }

  // PEH'S MISGUIDED ATTEMPT AT ENSURING THAT ALL INTERFACE TEXT OUTPUT
  // IS SET TO THE RIGHT SIZE
  S[0] = 1.0f;
  S[1] = 0.50f;

  float v[2] = { health_xoff, health_yoff };
  transform(v, c, 0);

  font->render(text, c.to_color32(), v[0],v[1], 0.1f, rhw_depth, 0.5f);

// DO NOT ADVANCE FOR TEXT OUTPUT
//  rhw_depth += 1.0f;

  element_list_t::iterator it;
  for(it = subelements.begin(); it != subelements.end(); ++it)
  {
    (*it)->render();
  }
}

void text_element::set_text(const char *fmt, ...)
{
  va_list     args;
  static char work[256];

  va_start(args, fmt);
  vsprintf(work, fmt, args);
  text = work;
  va_end(args);
}

// in pixels
int text_element::get_text_width()
{
  return font->text_width(text.c_str());
}



//------------------------------------------------------------------
void element::set_scale(float hs)
{
  S_Override[0] = hs;
  S_Override[1] = hs;
}


