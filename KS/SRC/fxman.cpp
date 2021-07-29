////////////////////////////////////////////////////////////////////////////////
/*
  fxman

  Special FX manager
*/
////////////////////////////////////////////////////////////////////////////////
#if 0

#include "global.h"

#include "fxman.h"
#include "app.h"
#include "entity.h"
#include "game.h"
#include "visrep.h"
#include "wds.h"
#include "oserrmsg.h"
#include "pmesh.h"

DEFINE_SINGLETON(fx_manager)

fx_manager::fx_manager()
{
//  put global game effects that are always loaded here:
//  load_effect_visual( stringx( "effectsflippycube.tam" ) );
  minimum_size = preloaded_effect_visuals.size();
}

fx_manager::~fx_manager()
{
  // unload everything
  int i;
  for( i=0;i<preloaded_effect_visuals.size();++i)
    unload_visual_rep( preloaded_effect_visuals[i] );
}

void fx_manager::load_effect_visual( const stringx& s )
{
  visual_rep* v = load_new_visual_rep( stringx("fx\\") + s, 0 );
  if( v->get_type()==VISREP_PMESH )
    static_cast<vr_pmesh*>(v)->shrink_memory_footprint();
  preloaded_effect_visuals.push_back( v );
}

void fx_manager::reset()
{
  int i;
  for( i=0;i<preloaded_effect_visuals.size();++i)
  {
    unload_visual_rep( preloaded_effect_visuals[i] );
  }

  preloaded_effect_visuals.clear();
}


entity* fx_manager::play_effect( const stringx& visrep_name,
                                 const po& where,
                                 unsigned flavor )
{
  stringx modified_name = stringx("fx\\")+visrep_name;
  world_dynamics_system* wds = app::inst()->get_game()->get_world();
  int i;

  // these loops have to use MAX_NUM_SPECIAL_FX constant instead
  // of special_fx.size() because particle_generators get dropped
  // on the end of the special_fx pile, and badness can ensue. 
  // this is what you call bad design.  my bad.  -jamie
  for(i=0;i<MAX_NUM_SPECIAL_FX;i++)
  {
    if( !(wds->special_fx[i]->get_in_use() ) )
      break;
  }
  if(i>=MAX_NUM_SPECIAL_FX)
  {
#ifdef DEBUG
    warning("Out of special_fx slots.");
#endif
    return NULL;  // no effect
  }
  entity* ve = wds->special_fx[i];
  assert( ve->get_flavor()==ENTITY_PHYSICAL );
  ve->change_visrep( modified_name );
  ve->rebirth();
  if(!( flavor&LOOPING ))
    ve->set_programmed_cell_death( ve->get_visrep_ending_time() );
  ve->set_rel_po( where );
  ve->set_visible( true );
  ve->compute_sector(g_world_ptr->get_the_terrain());
  wds->special_fx[i]->set_in_use(true);
  return ve;
}


#endif