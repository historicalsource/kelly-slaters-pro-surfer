// script_lib_beam.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_beam.h"
#include "beam.h"
#include "script_lib_entity.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "wds.h"
#include "script_lib_vector3d.h"
#include "entity_maker.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: beam
///////////////////////////////////////////////////////////////////////////////

// pointer to single instance of library class
// read a beam value (by id) from a stream
void slc_beam_t::read_value( chunk_file& fs, char* buf )
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find beam and write value to buffer
  *(vm_beam_t*)buf = (vm_beam_t)find_instance(id);
}

// find named instance of beam
unsigned slc_beam_t::find_instance( const stringx& n ) const
{
  if ( n == "NULL" )
    return (unsigned)0;
  const entity* r = entity_manager::inst()->find_entity( entity_id(n.c_str()), IGNORE_FLAVOR, FIND_ENTITY_UNKNOWN_OK );
  if ( !r )
    error( "beam " + n + " not found\n" );
  else if ( r->get_flavor() != ENTITY_BEAM )
    error( "entity " + n + " is not a beam\n" );
  return (unsigned)r;
}


// script library function:  num beam::hit_world();
class slf_beam_hit_world_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_hit_world_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    vm_num_t result = parms->me->hit_world();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_beam_hit_world_t slf_beam_hit_world(slc_beam,"hit_world()");

// script library function:  num beam::hit_hero();
class slf_beam_hit_hero_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_hit_hero_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    vm_num_t result = parms->me->hit_hero();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_beam_hit_hero_t slf_beam_hit_hero(slc_beam,"hit_hero()");

// script library function:  vector3d beam::get_impact_point();
class slf_beam_get_impact_point_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_get_impact_point_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    vm_vector3d_t result = parms->me->get_impact_point();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_beam_get_impact_point_t slf_beam_get_impact_point(slc_beam,"get_impact_point()");

// script library function:  vector3d beam::get_impact_normal();
class slf_beam_get_impact_normal_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_get_impact_normal_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    vm_vector3d_t result = parms->me->get_impact_normal();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_beam_get_impact_normal_t slf_beam_get_impact_normal(slc_beam,"get_impact_normal()");

// script library function:  beam::set_thickness(num);
class slf_beam_set_thickness_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_thickness_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t thickness;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_thickness( parms->thickness );
    SLF_DONE;
  }
};
//slf_beam_set_thickness_t slf_beam_set_thickness(slc_beam,"set_thickness(num)");

// script library function:  beam::set_max_length(num);
class slf_beam_set_max_length_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_max_length_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t len;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_max_length( parms->len );
    SLF_DONE;
  }
};
//slf_beam_set_max_length_t slf_beam_set_max_length(slc_beam,"set_max_length(num)");

// script library function:  beam::set_color(num,num,num,num);
class slf_beam_set_color_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_color_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t r;
    vm_num_t g;
    vm_num_t b;
    vm_num_t a;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_beam_color( color32((uint8)parms->r,(uint8)parms->g,(uint8)parms->b,(uint8)parms->a) );
    SLF_DONE;
  }
};
//slf_beam_set_color_t slf_beam_set_color(slc_beam,"set_color(num,num,num,num)");


// script library function:  beam::set_detect_stealth(num);
class slf_beam_set_detect_stealth_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_detect_stealth_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    if ( parms->torf == 0.0f )
      parms->me->clear_beam_flag( beam::DETECTS_STEALTH );
    else
      parms->me->set_beam_flag( beam::DETECTS_STEALTH );
    SLF_DONE;
  }
};
//slf_beam_set_detect_stealth_t slf_beam_set_detect_stealth(slc_beam,"set_detect_stealth(num)");

// script library function:  beam::set_collide_hero(num);
class slf_beam_set_collide_hero_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_collide_hero_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    if ( parms->torf != 0.0f )
      parms->me->clear_beam_flag( beam::NO_CLIP_TO_HERO );
    else
      parms->me->set_beam_flag( beam::NO_CLIP_TO_HERO );
    SLF_DONE;
  }
};
//slf_beam_set_collide_hero_t slf_beam_set_collide_hero(slc_beam,"set_collide_hero(num)");

// script library function:  beam::set_collide_world(num);
class slf_beam_set_collide_world_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_collide_world_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    if ( parms->torf != 0.0f )
      parms->me->clear_beam_flag( beam::NO_CLIP_TO_WORLD );
    else
      parms->me->set_beam_flag( beam::NO_CLIP_TO_WORLD );
    SLF_DONE;
  }
};
//slf_beam_set_collide_world_t slf_beam_set_collide_world(slc_beam,"set_collide_world(num)");

// script library function:  beam::set_collide_beamable(num);
class slf_beam_set_collide_beamable_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_collide_beamable_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    if ( parms->torf != 0.0f )
      parms->me->clear_beam_flag( beam::NO_CLIP_TO_BEAMABLE );
    else
      parms->me->set_beam_flag( beam::NO_CLIP_TO_BEAMABLE );
    SLF_DONE;
  }
};
//slf_beam_set_collide_beamable_t slf_beam_set_collide_beamable(slc_beam,"set_collide_beamable(num)");

// script library function:  beam::set_no_collision();
class slf_beam_set_no_collision_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_no_collision_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_beam_flag( beam::NO_CLIPPING );
    SLF_DONE;
  }
};
//slf_beam_set_no_collision_t slf_beam_set_no_collision(slc_beam,"set_no_collision()");

// script library function:  beam::set_additive(num);
class slf_beam_set_additive_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_additive_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_additive(parms->torf != 0.0f);
    SLF_DONE;
  }
};
//slf_beam_set_additive_t slf_beam_set_additive(slc_beam,"set_additive(num)");

// script library function:  beam::set_point_to_point(vector3d,vector3d);
class slf_beam_set_point_to_point_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_point_to_point_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_vector3d_t start;
    vm_vector3d_t end;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_point_to_point( parms->start, parms->end );
    SLF_DONE;
  }
};

//slf_beam_set_point_to_point_t slf_beam_set_point_to_point(slc_beam,"set_point_to_point(vector3d,vector3d)");



// script library function:  beam::set_uv_anim(num,num);
class slf_beam_set_uv_anim_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_uv_anim_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t u;
    vm_num_t v;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;

    vector2d uv( parms->u, parms->v );
    parms->me->set_uv_anim(uv);

    SLF_DONE;
  }
};
//slf_beam_set_uv_anim_t slf_beam_set_uv_anim(slc_beam,"set_uv_anim(num,num)");



// script library function:  beam::set_tiles_per_meter(num);
class slf_beam_set_tiles_per_meter_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_tiles_per_meter_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_num_t tpm;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;

    parms->me->set_tiles_per_meter(parms->tpm);

    SLF_DONE;
  }
};
//slf_beam_set_tiles_per_meter_t slf_beam_set_tiles_per_meter(slc_beam,"set_tiles_per_meter(num)");



// script library function:  beam::set_material(str);
class slf_beam_set_material_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_set_material_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;
    vm_str_t mat;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;

    parms->me->set_texture(*parms->mat);

    SLF_DONE;
  }
};
//slf_beam_set_material_t slf_beam_set_material(slc_beam,"set_material(str)");



// script library function:  beam::add_color_effect(num,num,num,num,num,num,num,num,num,num);
class slf_beam_add_color_effect_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_add_color_effect_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;

    vm_num_t start_r;
    vm_num_t start_g;
    vm_num_t start_b;

    vm_num_t end_r;
    vm_num_t end_g;
    vm_num_t end_b;

    vm_num_t duration;
    vm_num_t delay;
    vm_num_t loop_delay;
    vm_num_t invert;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    rational_t result = parms->me->add_color_effect(color32((uint8)parms->start_r,(uint8)parms->start_g,(uint8)parms->start_b,(uint8)0), color32((uint8)parms->end_r,(uint8)parms->end_g,(uint8)parms->end_b,(uint8)0), parms->duration, parms->delay, parms->loop_delay, (bool)parms->invert);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_beam_add_color_effect_t slf_beam_add_color_effect(slc_beam,"add_color_effect(num,num,num,num,num,num,num,num,num,num)");

// script library function:  beam::add_width_effect(num,num,num,num,num,num);
class slf_beam_add_width_effect_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_add_width_effect_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;

    vm_num_t start_w;
    vm_num_t end_w;

    vm_num_t duration;
    vm_num_t delay;
    vm_num_t loop_delay;
    vm_num_t invert;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    rational_t result = parms->me->add_width_effect(parms->start_w, parms->end_w, parms->duration, parms->delay, parms->loop_delay, (bool)parms->invert);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_beam_add_width_effect_t slf_beam_add_width_effect(slc_beam,"add_width_effect(num,num,num,num,num,num)");

// script library function:  beam::add_alpha_effect(num,num,num,num,num,num);
class slf_beam_add_alpha_effect_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_add_alpha_effect_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;

    vm_num_t start_a;
    vm_num_t end_a;

    vm_num_t duration;
    vm_num_t delay;
    vm_num_t loop_delay;
    vm_num_t invert;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    rational_t result = parms->me->add_alpha_effect((uint8)parms->start_a, (uint8)parms->end_a, parms->duration, parms->delay, parms->loop_delay, (bool)parms->invert);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_beam_add_alpha_effect_t slf_beam_add_alpha_effect(slc_beam,"add_alpha_effect(num,num,num,num,num,num)");

// script library function:  beam::kill_effect(num,num);
class slf_beam_kill_effect_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_kill_effect_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;

    vm_num_t id;
    vm_num_t apply_target_vals;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->kill_effect((unsigned short)parms->id, (bool)parms->apply_target_vals);
    SLF_DONE;
  }
};
//slf_beam_kill_effect_t slf_beam_kill_effect(slc_beam,"kill_effect(num,num)");

// script library function:  beam::kill_all_effects(num);
class slf_beam_kill_all_effects_t : public script_library_class::function
{
public:
  // constructor required
  slf_beam_kill_all_effects_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_beam_t me;

    vm_num_t apply_target_vals;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->kill_all_effects((bool)parms->apply_target_vals);
    SLF_DONE;
  }
};
//slf_beam_kill_all_effects_t slf_beam_kill_all_effects(slc_beam,"kill_all_effects(num)");


///////////////////////////////////////////////////////////////////////////////
// global support for script library class beam
///////////////////////////////////////////////////////////////////////////////

// global script library function: beam create_beam()
class slf_create_beam_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_beam_t( const char* n )
    :   script_library_class::function( n )
  {
  }
  // library function execution
  virtual bool operator()( vm_stack &stack, entry_t entry )
  {
    vm_beam_t result;
    result = (vm_beam_t)g_entity_maker->acquire_beam( (unsigned int)(EFLAG_ACTIVE|EFLAG_MISC_NONSTATIC) );
    result->set_created_entity_default_active_status();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_beam_t slf_create_beam("create_beam()");


// global script library function: beam to_beam(entity)
class slf_to_beam_t : public script_library_class::function
{
public:
  // constructor required
  slf_to_beam_t( const char* n )
    :   script_library_class::function( n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t e;
  };
  // library function execution
  virtual bool operator()( vm_stack &stack, entry_t entry )
  {
    SLF_PARMS;
    vm_beam_t result = NULL;
    if ( parms->e->is_a_beam() )
      result = static_cast<vm_beam_t>( parms->e );
    else
    {
      stack.get_thread()->slf_error( "to_beam(): " + parms->e->get_id().get_val() + " is not a beam" );
    }
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_to_beam_t slf_to_beam("to_beam(entity)");


void register_beam_lib()
{
  slc_beam_t* slc_beam = NEW slc_beam_t("beam",4,"entity");

  NEW slf_beam_hit_world_t(slc_beam,"hit_world()");
  NEW slf_beam_hit_hero_t(slc_beam,"hit_hero()");
  NEW slf_beam_get_impact_point_t(slc_beam,"get_impact_point()");
  NEW slf_beam_get_impact_normal_t(slc_beam,"get_impact_normal()");
  NEW slf_beam_set_thickness_t(slc_beam,"set_thickness(num)");
  NEW slf_beam_set_max_length_t(slc_beam,"set_max_length(num)");
  NEW slf_beam_set_color_t(slc_beam,"set_color(num,num,num,num)");
  NEW slf_beam_set_detect_stealth_t(slc_beam,"set_detect_stealth(num)");
  NEW slf_beam_set_collide_hero_t(slc_beam,"set_collide_hero(num)");
  NEW slf_beam_set_collide_world_t(slc_beam,"set_collide_world(num)");
  NEW slf_beam_set_collide_beamable_t(slc_beam,"set_collide_beamable(num)");
  NEW slf_beam_set_no_collision_t(slc_beam,"set_no_collision()");
  NEW slf_beam_set_additive_t(slc_beam,"set_additive(num)");
  NEW slf_beam_set_point_to_point_t(slc_beam,"set_point_to_point(vector3d,vector3d)");

  NEW slf_beam_set_uv_anim_t(slc_beam,"set_uv_anim(num,num)");
  NEW slf_beam_set_tiles_per_meter_t(slc_beam,"set_tiles_per_meter(num)");
  NEW slf_beam_set_material_t(slc_beam,"set_material(str)");


  NEW slf_beam_add_color_effect_t(slc_beam,"add_color_effect(num,num,num,num,num,num,num,num,num,num)");
  NEW slf_beam_add_width_effect_t(slc_beam,"add_width_effect(num,num,num,num,num,num)");
  NEW slf_beam_add_alpha_effect_t(slc_beam,"add_alpha_effect(num,num,num,num,num,num)");
  NEW slf_beam_kill_effect_t(slc_beam,"kill_effect(num,num)");
  NEW slf_beam_kill_all_effects_t(slc_beam,"kill_all_effects(num)");
  NEW slf_create_beam_t("create_beam()");
  NEW slf_to_beam_t("to_beam(entity)");
}
