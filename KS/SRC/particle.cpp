////////////////////////////////////////////////////////////////////////////////

// particle.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "particle.h"
#include "profiler.h"

#include "app.h"
#include "game.h"
#include "geomgr.h"
#include "iri.h"
#include "oserrmsg.h"
#include "vertwork.h"
#include "wds.h"
#include "terrain.h"
#include "osdevopts.h"
#include "collide.h"
#include "algebra.h"
#include "maxiri.h"
#include "forceflags.h"
#include "chunkfile.h"
#include "random.h"
#include "billboard.h"
#include "file_finder.h"
#include "ngl.h"
#include "ngl_support.h"


//#define PARTICLE_MIN_VISUAL_RADIUS  3.0f
#define PARTICLE_MIN_VISUAL_RADIUS  1.0f
//#define PARTICLE_BASE_RADIUS_MOD  3.0f
#define PARTICLE_BASE_RADIUS_MOD  1.5f

// features I need
//   scale : CHECK
//   grow for : CHECK
//   fade for : CHECK
//   off plane : CHECK
//   particle size variation : CHECK
//   object motion inheritance : CHECK
//   area of generation : CHECK
//   wind = gravity that goes a different direction and only is active in a given volume CHECK
//   force towards a point  CHECK
//   air resistance  CHECK
//   random/brownian something.
//   billboards CHECK
//   billboards size CHECK
//   billboards facing CHECK
//   billboards axis lock
//   speed up animating render sources CHECK
//   sorting everything
//   tangential force
//   read from disk  CHECK
//   particle rotation
//   tool
#include <algorithm>

// in maxiri.h
//#define MAX_INSTANCES (VIRTUAL_MAX_VERTS_PER_PRIMITIVE/3)




///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////

particle_generator::particle_generator( const entity_id& _id,
                                        unsigned int _flags )
  : entity( _id, ENTITY_PARTICLE_GENERATOR, _flags )
{
  initialize_variables();
}


particle_generator::particle_generator( const entity_id& _id,
                                        entity_flavor_t _flavor,
                                        unsigned int _flags )
  : entity( _id, _flavor, _flags )
{
  initialize_variables();
}

particle_generator::~particle_generator()
{
  if (particles != NULL)
  {
    delete[] particles;
    particles = NULL;
  }
}

void particle_generator::initialize_variables()
{
  disable_for_sony_booth = false;
  always_render = false;

  particles = NULL;
  start_particle = 0;
  end_particle = 0;
  max_particles = 0;
  particle_life_span = 0.0f;
  life_variation = 0.5f;
  birthrate = 0;
  rate_variation = 0.8f;
  base_speed = 1.0f;
  speed_variation = 0.8f;
  spread_off_axis = PI * 0.5f;
  time_to_next_particle = 0.0f;
  scale_variation = 0.5f;
  grow_for = 0.0f;
  shrink_for = 2.0f;
  fade_in = 0.0f;
  fade_out = 0.0f;
  spread_off_plane = PI * 0.125f;
  motion_inheritance = 1.0f;
  generation_radius = 2.0f;
  generation_height = 2.0f;
  rotation_period = 0.0f;
  recip_rotation_period = 0.0f;
  rotational_speed_variation = 0.0f;
  rotational_start_variation = 0.0f;
  on_for = LARGE_TIME;
  off_for = 0.0f;
  flags = 0;

  time_to_next_particle = 0.0f;

  rh_grow_for = 0.0f;
  rh_shrink_for = 0.0f;
  rh_fade_in = 0.0f;
  rh_fade_out = 0.0f;

  abs_visual_center = get_abs_position();
  visual_center = ZEROVEC;
  visual_radius = get_base_visual_radius();

  last_position = ZEROVEC;
  last_position_valid = false;

//#pragma fixme("Egregious sony booth hack.  jdf 4-2-01")
  if( strstr( filename.c_str(), "spraymsletrailsmoke" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraymsletrailglow" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraymsleblinkred" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmfire" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmflysmoke" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmglow" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmmember" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmrim" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmpreglow" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmpreline" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmressmoke" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpsmring" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdfirehead" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdfiretrail" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdflysmoke" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdfire" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdglow" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdmember" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdrim" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdring" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdsmoke" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdpreglow" ) )
    disable_for_sony_booth = false;
  if( strstr( filename.c_str(), "spraygenexpmdpreline" ) )
    disable_for_sony_booth = false;
}

///////////////////////////////////////////////////////////////////////////////
/*
void particle_generator::use_a_pool( int howmany )
{
  pool = NEW vector<particle_generator*>;
  for( int i = 0; i < howmany; ++i )
  {
    pool->push_back( NEW particle_generator( entity_id(ANONYMOUS), EFLAG_GRAPHICS_VISIBLE|EFLAG_MISC_NONSTATIC|EFLAG_MISC_REUSEME ) );
  }
}
*/


///////////////////////////////////////////////////////////////////////////////
// File I/O
///////////////////////////////////////////////////////////////////////////////
char g_debug_generator_filename[128] = "spraymsletrailsmoke";

particle_generator::particle_generator( const stringx& _filename,
                                        const entity_id& _id,
                                        entity_flavor_t _flavor,
                                        unsigned int _flags )
  : entity( _id, _flavor, _flags )
{
#ifdef DEBUG
  stringx tfilename = _filename;
  tfilename.to_lower();
  if( strstr( tfilename.c_str(), g_debug_generator_filename ) )
  {
    debug_print("loading particle generator %s",tfilename.c_str());
  }
#endif


  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
    filename = g_file_finder->find_file( _filename, ".ent" );
  else
    filename = _filename + ".ent";

  initialize_variables();
  load();

  if (particles==NULL)
  {
    particles = NEW particle[max_particles];
    assert( particles );
  }
}

void particle_generator::load()
{
  flags = 0;
  on_for = LARGE_TIME;
  off_for = 0;
  fade_in = 0.0f;
  fade_out = 0.0f;

  chunk_file fs;
  chunk_flavor cf;

  fs.open(filename);

  stringx flavor;
  serial_in( fs, &flavor );
  assert( flavor == "PARTICLE" );

  serial_in( fs, &cf );
  stringx visrep_name("");
  if ( cf == "visrep" )
    serial_in( fs, &visrep_name );
  else
  {
    // load the tbb data inline.
    assert( cf == "billboard" );
    my_visrep = NEW vr_billboard( fs, false );
  }

  serial_in( fs, &cf );
  assert( cf == "life" );
  serial_in( fs, &particle_life_span );

  serial_in( fs, &cf );
  assert( cf == "variance");
  serial_in( fs, &life_variation);

  serial_in( fs, &cf );
  assert( cf == "rate");
  serial_in( fs, &birthrate );

  serial_in( fs, &cf );
  assert( cf == "variance");
  serial_in( fs, &rate_variation );

  serial_in( fs, &cf );
  assert( cf == "speed");
  serial_in( fs, &base_speed );

  serial_in( fs, &cf );
  assert( cf == "variance");
  serial_in( fs, &speed_variation );

  serial_in( fs, &cf );
  assert( cf == "offaxis");
  serial_in( fs, &spread_off_axis );

  serial_in( fs, &cf );
  assert( cf == "offplane");
  serial_in( fs, &spread_off_plane );

  serial_in( fs, &cf );
  assert( cf == "offscale");
  serial_in( fs, &scale_variation );

  serial_in( fs, &cf );
  assert( cf == "growfor");
  serial_in( fs, &grow_for );

  serial_in( fs, &cf );
  assert( cf == "fadefor");
  serial_in( fs, &shrink_for );

  serial_in( fs, &cf );
  assert( cf == "motion");
  serial_in( fs, &motion_inheritance );

  serial_in( fs, &cf );
  assert( cf == "radius");
  serial_in( fs, &generation_radius );

  serial_in( fs, &cf );
  assert( cf == "height");
  serial_in( fs, &generation_height );

  force_list.resize(0);
  for(;;)
  {
    chunk_flavor cf;
    serial_in( fs, &cf );
    if( cf == "rotation")
    {
      serial_in( fs, &rotation_period );
			if ( rotation_period==0 )
      	recip_rotation_period = 10000;
			else
      	recip_rotation_period = 1/rotation_period;
    }
    else if( cf == "rotspdv" )
    {
      serial_in( fs, &rotational_speed_variation );
    }
    else if( cf == "rotstrtv" )
    {
      serial_in( fs, &rotational_start_variation );
    }
    else if( cf == "flags")
    {
      stringx flag_names;
      serial_in( fs, &flag_names );
      if (!flag_names.empty() && isdigit(flag_names[0]))
        flags = atoi(flag_names.c_str());
      else
      {
        if( flag_names.find( "GENERATOR_ORIENTED" ) != (int)string::npos )
          flags |= GENERATOR_ORIENTED;
        if( flag_names.find( "DIRECTION_ORIENTED" ) != (int)string::npos )
          flags |= DIRECTION_ORIENTED;
        if( flag_names.find( "ASYNC_ANIMS") != (int)string::npos )
          flags |= ASYNC_ANIMS;
        if( flag_names.find( "LIFETIME_ANIMS") != (int)string::npos )
          flags |= LIFETIME_ANIMS;
        if( flag_names.find( "LOCAL_SPACE" ) != (int)string::npos )
          flags |= LOCAL_SPACE;
        if( flag_names.find( "BSP_COLLISION" ) != (int)string::npos )
          flags |= PARTICLE_BSP_COLLISIONS;
      }
    }
    else if( cf == "onfor")
    {
      serial_in( fs, &on_for );
    }
    else if( cf == "offfor")
    {
      serial_in( fs, &off_for );
    }
    else if( cf == "fadeout")
    {
      serial_in( fs, &fade_out );
    }
    else if( cf== "fadein")
    {
      serial_in( fs, &fade_in );
    }
    else if( cf == "force")
    {
      stringx flavor;
      serial_in( fs, &flavor );
      particle_force::force_flavor_t fft = particle_force::force_flavor_t(-1);
      if(flavor=="constant")
      {
        fft = particle_force::CONSTANT;
      }
      else if(flavor=="topoint")
      {
        fft = particle_force::TOWARDS_POINT;
      }
      else if(flavor=="resist")
      {
        fft = particle_force::RESISTANCE;
      }
      else
      {
        error(stringx("Invalid force flavor in file ")+filename);
      }

      vector3d vec;
      serial_in( fs, &vec );
      rational_t delta, terminal;
      serial_in( fs, &delta );
      assert(delta > -1e15f && delta < 1e15f);
      serial_in( fs, &terminal );
      assert(terminal >= 0);
      force_list.push_back( particle_force( fft, vec, delta, terminal) );
    }
    else if( cf == CHUNK_END )
    {
      break;
    }
    else
    {
      stringx composite = stringx("Unknown chunk type in ")+filename;
      error(composite.c_str());
    }
  }

  rh_grow_for   = grow_for ? 1/grow_for : 1e25f;
  rh_shrink_for = shrink_for ? 1/shrink_for : 1e25f;
  rh_fade_in    = fade_in ? 1/fade_in : 1e25f;
  rh_fade_out   = fade_out ? 1/fade_out : 1e25f;

  stringx my_dirname = fs.get_dir();

#if _ENABLE_WORLD_EDITOR
  tool_visrep_name = visrep_name;
  dir_name = my_dirname;
#endif

  if ( visrep_name.length() )
  {
#ifdef NGL
    visrep_t vr_type = visual_rep_name_to_type( visrep_name );

    if( vr_type == VISREP_BILLBOARD )
    {
      stringx s = my_dirname + visrep_name;
      load_new_visual_rep( s, 0 );
      change_visrep( s );
    }
    else if( vr_type == VISREP_PMESH )
    {
      filespec spec( fs.get_name( ) );

      // set mesh path
      stringx mesh_path = spec.path;
      nglSetMeshPath( (char*) mesh_path.c_str( ) );

      // set texture path
      stringx texture_path = spec.path;
      texture_path.to_lower();

      if( texture_path.find( "entities" ) != stringx::npos )
      {
        stringx::size_type last_slash        = texture_path.rfind( '\\' );
        stringx::size_type penultimate_slash = texture_path.rfind( '\\', last_slash-1 );
        texture_path = texture_path.substr( 0, penultimate_slash+1 );
      }

      texture_path += os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR);
      texture_path += "\\";
      nglSetTexturePath( (char*) texture_path.c_str( ) );

      // load mesh
      filespec mesh_spec( visrep_name );

      if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_CHATTY_LOAD ) ) {
        nglPrintf( "Loading %s" PLATFORM_MESH_EXT ".\n", mesh_spec.name.c_str( ) );
      }

      my_mesh = nglGetMesh( (char*) mesh_spec.name.c_str( ) );
    }
#else
    //vr_pmesh::push_optimize_static_mesh_for_d3d_disallowed();
    visual_rep* v = load_new_visual_rep( my_dirname + visrep_name, 0 );
    if( v->get_type()==VISREP_PMESH )
      static_cast<vr_pmesh*>(v)->shrink_memory_footprint();
    change_visrep( my_dirname + visrep_name );
#endif
  }

  set_flag(EFLAG_GRAPHICS,true);
  time_to_next_particle = 0.0F; //get_age();
  // so we don't resize particles all the time
  max_particles = (int)(birthrate * particle_life_span + 2);  // plus one to make sure we round up, plus another one because looping nature
                                                       // of particle bank means we always have one less particle than we have size for
  //if(max_particles<2) max_particles = 2;
  assert(max_particles >= 2);
  start_particle = 0;
  end_particle = 0;
  // you'll never ever believe in me
  // i am your tourniquet
}


#if _ENABLE_WORLD_EDITOR
  void particle_generator::reset_sys(bool full)
  {
    rh_grow_for   = grow_for ? 1/grow_for : 1e25f;
    rh_shrink_for = shrink_for ? 1/shrink_for : 1e25f;
    rh_fade_in    = fade_in ? 1/fade_in : 1e25f;
    rh_fade_out   = fade_out ? 1/fade_out : 1e25f;

    if(full)
    {
      //vr_pmesh::push_optimize_static_mesh_for_d3d_disallowed();
      visual_rep* v = load_new_visual_rep( dir_name + tool_visrep_name, 0 );
      if( v->get_type()==VISREP_PMESH )
        static_cast<vr_pmesh*>(v)->shrink_memory_footprint();
      change_visrep( dir_name + tool_visrep_name );

      if ( particles != NULL )
      {
        destroy_particles();
      }
    }

    set_flag(EFLAG_GRAPHICS,true);
    time_to_next_particle = 0.0F; //get_age();
    // so we don't resize particles all the time

    int old_max = max_particles;
    max_particles = (int)(birthrate * particle_life_span + 2);  // plus one to make sure we round up, plus another one because looping nature
                                                       // of particle bank means we always have one less particle than we have size for
    if(particles != NULL)
    {
      if(max_particles > old_max)
      {
        particle* oldparticles = particles;
        particles = NEW particle[max_particles];
        memcpy(particles, oldparticles, old_max*sizeof(particle));
        delete[] oldparticles;
      }
      else if(max_particles < old_max)
      {
        particle* oldparticles = particles;
        particles = NEW particle[max_particles];
        memcpy(particles, oldparticles, max_particles*sizeof(particle));
        delete[] oldparticles;

        if(end_particle >= max_particles)
        {
          end_particle = max_particles - 1;
//          assert(start_particle < end_particle);
        }
        if(start_particle >= max_particles)
        {
          start_particle = max_particles - 1;
//          assert(end_particle < start_particle);
        }
      }
    }
    else
    {
      start_particle = 0;
      end_particle = 0;
    }

    //if(max_particles<2) max_particles = 2;
    assert(max_particles >= 2);
    // you'll never ever believe in me
    // i am your tourniquet
  }


  bool particle_generator::write(const stringx &file)
  {
    ofstream fout;
    stringx fname = os_file::get_root_dir() + file;
    fout.open(fname.c_str(), ios::out | ios::trunc);

    if(!fout.fail())
    {
      fout<<"PARTICLE\n";
      fout<<"visrep\n"<<tool_visrep_name.c_str()<<"\n";
      fout<<"life\n"<<particle_life_span<<"\n";
      fout<<"variance\n"<<life_variation<<"\n";
      fout<<"rate\n"<<birthrate<<"\n";
      fout<<"variance\n"<<rate_variation<<"\n";
      fout<<"speed\n"<<base_speed<<"\n";
      fout<<"variance\n"<<speed_variation<<"\n";
      fout<<"offaxis\n"<<spread_off_axis<<"\n";
      fout<<"offplane\n"<<spread_off_plane<<"\n";
      fout<<"offscale\n"<<scale_variation<<"\n";
      fout<<"growfor\n"<<grow_for<<"\n";
      fout<<"fadefor\n"<<shrink_for<<"\n";
      fout<<"motion\n"<<motion_inheritance<<"\n";
      fout<<"radius\n"<<generation_radius<<"\n";
      fout<<"height\n"<<generation_height<<"\n";
      fout<<"rotation\n"<<rotation_period<<"\n";
      fout<<"rotspdv\n"<<rotational_speed_variation<<endl;
      fout<<"rotstrtv\n"<<rotational_start_variation<<endl;
      fout<<"flags\n";

      bool first = true;
      if(flags & GENERATOR_ORIENTED)
      {
        if(!first)
          fout<<"+";
        fout<<"GENERATOR_ORIENTED";
        first = false;
      }
      if(flags & DIRECTION_ORIENTED)
      {
        if(!first)
          fout<<"+";
        fout<<"DIRECTION_ORIENTED";
        first = false;
      }
      if(flags & ASYNC_ANIMS)
      {
        if(!first)
          fout<<"+";
        fout<<"ASYNC_ANIMS";
        first = false;
      }
      if(flags & LIFETIME_ANIMS)
      {
        if(!first)
          fout<<"+";
        fout<<"LIFETIME_ANIMS";
        first = false;
      }
      if(flags & LOCAL_SPACE)
      {
        if(!first)
          fout<<"+";
        fout<<"LOCAL_SPACE";
        first = false;
      }
      if(flags & PARTICLE_BSP_COLLISIONS)
      {
        if(!first)
          fout<<"+";
        fout<<"BSP_COLLISION";
        first = false;
      }
      if(!first)
        fout<<"\n";

      fout<<"onfor\n"<<on_for<<"\n";
      fout<<"offfor\n"<<off_for<<"\n";
      fout<<"fadeout\n"<<fade_out<<"\n";
      fout<<"fadein\n"<<fade_in<<"\n";

      vector<particle_force>::iterator pfi;
      for(pfi = force_list.begin(); pfi != force_list.end() ; ++pfi)
      {
        fout<<"force\n";
        switch((*pfi).flavor)
        {
          case particle_force::CONSTANT:
            fout<<"constant\n";
            break;

          case particle_force::TOWARDS_POINT:
            fout<<"topoint\n";
            break;

          case particle_force::RESISTANCE:
            fout<<"resist\n";
            break;

          default:
            assert(0);
            break;
        }

        fout<<(*pfi).multipurpose.x<<" "<<(*pfi).multipurpose.y<<" "<<(*pfi).multipurpose.z<<"\n";
        fout<<(*pfi).delta_speed<<"\n";
        fout<<(*pfi).terminal_speed<<"\n";
      }

      fout<<"chunkend\n";
      fout.close();

      return(true);
    }
    else
    {
      warning(fname + " is write protected!");
      return(false);
    }
  }
#endif

///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* particle_generator::make_instance( const entity_id& _id,
                                           unsigned int _flags ) const
{
  // all particle generator instances must be nonstatic
  _flags |= EFLAG_MISC_NONSTATIC | EFLAG_GRAPHICS_VISIBLE;
  particle_generator* newpg = 0;

  if( !newpg )
  {
    newpg = NEW particle_generator( _id, _flags );
  }

  if( newpg )
  {
    newpg->copy_instance_data( *this );
  }

  return (entity*)newpg;
}


void particle_generator::copy_instance_data( const particle_generator& b )
{
//#pragma todo("sizeof assertion here  jdf 4/2/01")
  entity::copy_instance_data( b );

  disable_for_sony_booth = b.disable_for_sony_booth;
  filename = b.filename;
  particle_life_span = b.particle_life_span;
  life_variation = b.life_variation;
  birthrate = b.birthrate;
  rate_variation = b.rate_variation;
  base_speed = b.base_speed;
  speed_variation = b.speed_variation;
  spread_off_axis = b.spread_off_axis;
  spread_off_plane = b.spread_off_plane;
  scale_variation = b.scale_variation;
  grow_for = b.grow_for;
  shrink_for = b.shrink_for;
  motion_inheritance = b.motion_inheritance;
  generation_radius = b.generation_radius;
  generation_height = b.generation_height;
  rotation_period = b.rotation_period;
  recip_rotation_period = b.recip_rotation_period;
  rotational_speed_variation = b.rotational_speed_variation;
  rotational_start_variation = b.rotational_start_variation;
  flags = b.flags;
  on_for = b.on_for;
  off_for = b.off_for;
  fade_out = b.fade_out;
  fade_in = b.fade_in;
  rh_grow_for = b.rh_grow_for;
  rh_shrink_for = b.rh_shrink_for;
  rh_fade_in = b.rh_fade_in;
  rh_fade_out = b.rh_fade_out;
  max_particles = b.max_particles;

  if (particles==NULL)
  {
    particles = NEW particle[max_particles];
    assert( particles );
  }

  force_list = b.force_list;

  time_to_next_particle  = 0.0F; //b.time_to_next_particle; //get_age();
  start_particle = end_particle = 0;

#if _ENABLE_WORLD_EDITOR
  tool_visrep_name = b.tool_visrep_name;
  dir_name = b.dir_name;
#endif

  set_flag(EFLAG_GRAPHICS,true);
}


///////////////////////////////////////////////////////////////////////////////
// entity_maker caching interface

void particle_generator::acquire( unsigned int _flags )
{
  _flags |= EFLAG_MISC_NONSTATIC | EFLAG_GRAPHICS_VISIBLE;
  entity::acquire( _flags );
  time_to_next_particle  = 0.0F;
  start_particle = end_particle = 0;
  set_flag( EFLAG_GRAPHICS, true );
  last_position_valid = false;
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

// I hate globals --Sean
// I like globals --Jamie
// This is global only so that it will be 8 byte aligned, so we can
// get away with using fast_8byte_non_affine_xform.  However, this whole
// thing is crap anyway because we need to just leave the matrix loaded
// for all the particles, but right now it's not set up to do that. --Sean
po* generator_po = (po*)os_malloc32( sizeof(po) );


void particle_generator::po_changed()
{
  entity::po_changed();
  abs_visual_center = get_abs_po().slow_xform(visual_center);

  // ugh... I think this might be getting called each frame even when not moving.  Let's see.
  //visual_radius = get_base_visual_radius(); // because it's not centered anymore, set to max
}


extern profiler_timer proftimer_special_fx;
extern profiler_timer proftimer_advance_particles;


void particle_generator::frame_advance(time_value_t t)
{
/*
  int cur_detail = os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL);
  if (cur_detail < min_detail)
    return;
*/
  assert(t > 0.0F);

  if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_NO_PARTICLES ) ) {
    return;
  }

#ifdef NGL
  if( !my_visrep && !my_mesh ) {
    return;
  }
#endif

  proftimer_special_fx.start();
  proftimer_advance_particles.start();

  entity::frame_advance(t);

  *generator_po = get_abs_po();
  // remove scaling from generator po without affecting position
  generator_po->non_affine_normalize();

  if (!is_visible())
  {
    visual_center = ZEROVEC;
    abs_visual_center = get_abs_position();
  }
  else
  {
    assert (particles != NULL);
    assert(time_to_next_particle >= 0.0F);
    time_to_next_particle -= t;
    if( time_to_next_particle <= 0.0F )
    {
      // number of particles for interval
      // particles are born
      float recip_t = 1.0f / t;
#ifdef NGL
      visrep_t my_visrep_type = ( my_visrep ) ? my_visrep->get_type( ) : VISREP_KRMESH;
#else
      visrep_t my_visrep_type = my_visrep->get_type();
#endif
      float base_speed_times_speed_var        = base_speed * speed_variation;
      float particle_life_span_times_life_var = particle_life_span * life_variation;
      float birthrate_times_rate_var          = birthrate * rate_variation;
      vector3d frame_abs_delta_position = get_abs_position() - get_last_position();
      vector3d vel = frame_abs_delta_position * recip_t;//(1.0f / g_world_ptr->get_cur_time_inc());
      vector3d inverse_velocity;
      if ((flags & LOCAL_SPACE) && (motion_inheritance < 1.0f))
        inverse_velocity = generator_po->fast_non_affine_inverse_xform( vel ); // vel in parent space
      else
        inverse_velocity = ZEROVEC;
      do
      {
        if( is_on() )
        {
          assert( end_particle>=0 && end_particle<max_particles );
          particle* pptr = particles + end_particle;
          if (flags&LOCAL_SPACE)
          {
            pptr->my_pos = ZEROVEC;
          }
          else
          {
            float fraction; // intraframe offset (0.0f==previous frame, 1.0f==this frame)
            fraction = (time_to_next_particle+t) * recip_t;
            vector3d delta_p = frame_abs_delta_position * fraction;
            pptr->my_pos = get_last_position() + delta_p;
          }
          prefetch( ((char*)pptr)+32 );

          // give the particle a random position offset
          vector3d offset;
          offset.x = random(-generation_radius,generation_radius);
          offset.y = random(-generation_height,generation_height);
          offset.z = random(-generation_radius,generation_radius);
          if (!(flags&LOCAL_SPACE))
          {
            offset = generator_po->fast_8byte_non_affine_xform( offset );
          }
          pptr->my_pos += offset;

          // give the particle a random launch vector
          rational_t new_speed = base_speed - random(base_speed_times_speed_var);
          vector3d local_vel;
          float rotz = random(-spread_off_axis,spread_off_axis);
          float sinrot;
          float cosrot;
          fast_sin_cos_approx( rotz, &sinrot, &cosrot );
          local_vel.x = sinrot * new_speed;
          local_vel.y = cosrot * new_speed;
          local_vel.z = 0.0f;
          if( spread_off_plane )
          {
            po rotx;
            rotx.set_rotate_x( random(-spread_off_plane,spread_off_plane) );
            local_vel = rotx.slow_xform( local_vel );
          }

          if( flags&LOCAL_SPACE )
          {
            pptr->my_velocity = local_vel;
            if( motion_inheritance < 1.0f )
            {
              pptr->my_velocity -= inverse_velocity * (1.0f-motion_inheritance);
            }
          }
          else
          {
            pptr->my_velocity = generator_po->fast_8byte_non_affine_xform( local_vel );
            pptr->my_velocity += vel * motion_inheritance;
          }

          pptr->total_life = particle_life_span - random(particle_life_span_times_life_var);
          pptr->life_remaining = pptr->total_life + time_to_next_particle;
          pptr->scale = 1.0f - random( scale_variation );
          pptr->rotation = random( rotational_start_variation );
          pptr->rotation_speed = 1.0f - random( rotational_speed_variation );

          // create a random axis of rotation
          if( (my_visrep_type != VISREP_BILLBOARD ) && (rotation_period) )
          {
            //profcounter_particle_tpm_rotate.add_count(1);

            vector3d unnormalized_axis = vector3d( random(-1.0F,1.0F),random(-1.0F,1.0F),random(-1.0F,1.0F) );
            pptr->rot_axis = unnormalized_axis;
            pptr->rot_axis.normalize();
            assert(pptr->my_pos.is_valid());
          }
          else
          {
            pptr->rot_axis = YVEC;
          }
          //debug_print("particle %s born, lifetime %f",filename.c_str(), pptr->life_remaining);

          ++end_particle;
        }

        if( end_particle>=max_particles )
          end_particle=0;
        if( end_particle==start_particle )
        {
          //debug_print("particle %s kicked out, no room!! %f life left",filename.c_str(), particles[start_particle].life_remaining);
          ++start_particle;   // kick out dying particle
          if(start_particle>=max_particles)
            start_particle=0;
        }

        // putting this line here to help pipeline the div
        rational_t instantaneous_birthrate = birthrate - random(birthrate_times_rate_var);
//        instantaneous_birthrate *= framerate_compensator;    // see above not on compensator
        instantaneous_birthrate = 1.0f / instantaneous_birthrate;
        time_to_next_particle += instantaneous_birthrate;
      } while( time_to_next_particle <= 0.0F );
    }
  }

  if (!particles)
  {
    visual_center = ZEROVEC;
    abs_visual_center = get_abs_position();
  }
  else
  {
    // hope survives
    // animate particles
    // prepare forces for particle takeover
    vector<particle_force>::iterator pfi;
    vector<particle_force>::iterator pfi_end = force_list.end();
    for(pfi = force_list.begin(); pfi != pfi_end ; ++pfi)
    {
      if( (*pfi).flavor== particle_force::TOWARDS_POINT )
      {
        if( flags&particle_generator::LOCAL_SPACE )
          (*pfi).utility = (*pfi).multipurpose;
        else
          (*pfi).utility = generator_po->fast_8byte_xform((*pfi).multipurpose);
      }
    }

    //last_time = age;
    // particles die
    assert( start_particle>=0 && start_particle<max_particles );
    assert( end_particle>=0 && end_particle<max_particles );

    particle* start_particlep = particles + start_particle;
    particle* end_particlep   = particles + end_particle;
    particle* loop_particlep  = particles + max_particles;

    bool do_bsp = flags & PARTICLE_BSP_COLLISIONS;

    bounding_box box;
		box.vmin.x=	box.vmin.y=	box.vmin.z=	box.vmax.x=	box.vmax.y=	box.vmax.z= 0.0f;

    while (start_particlep!=end_particlep)
    {
      if ( (start_particlep->life_remaining -= t) <= 0.0F)
      {
        #ifdef DEBUG
        //if (start_particlep->life_remaining + t > 0.0F)
          //debug_print("particle %s dies!!",filename.c_str());
        #endif

        // 'deallocate' particles at beginning that are dead.
        ++start_particle;
        if( start_particle>=max_particles )
          start_particle=0;
      }
      else
      {
        if(do_bsp)
        {
          vector3d hit_loc, hit_normal;
          vector3d vel_inc = start_particlep->my_velocity * t;
          vector3d pos_plus_inc = start_particlep->my_pos+vel_inc;

#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
          bool bspspray_enabled = os_developer_options::inst()->is_flagged(os_developer_options::FLAG_BSP_SPRAY_PAINT);
          if ( !bspspray_enabled && g_world_ptr->get_the_terrain().find_intersection(start_particlep->my_pos, pos_plus_inc, hit_loc, hit_normal) )
#else
          if ( g_world_ptr->get_the_terrain().find_intersection(start_particlep->my_pos, pos_plus_inc, hit_loc, hit_normal) )
#endif
          {
            start_particlep->my_pos = hit_loc;
            vector3d vel = start_particlep->my_velocity;
            rational_t speed = vel.length2();
            if (speed>0.0f)
            {
              vel *= (1.0f / __fsqrt(speed));

              rational_t normal_speed = dot(vel, hit_normal);
              vel = vel-normal_speed*hit_normal*1.1f;
              vel += vel*(normal_speed/speed)*0.5f;
              start_particlep->my_velocity = vel;
            }
          }

#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
          if (bspspray_enabled)
          {
#if 0
            vector3d vel_inc = start_particlep->my_velocity * t;
#endif
            vector3d hit_loc, hit_normal;  // , dummy_hit_loc; // unused -- remove me?
        #if 0
            vector3d pog = start_particlep->my_pos;
            vector3d v0(0,2,0);
            vector3d v1(3,5,0);
            vector3d v2(3,2,0);
            hit_loc = start_particlep->my_pos;
            if ( dist_point_polygon(pog, v0, v1, v2)<0.1f)
        #endif

        #if 1
            if ( g_world_ptr->get_the_terrain().find_intersection(start_particlep->my_pos, pos_plus_inc, hit_loc, hit_normal) )
        #endif

        #if 0  //if SHOW_HERO_CAPSULE
            collision_capsule * cap = ((collision_capsule *) g_world_ptr->get_hero_ptr()->get_colgeom());
            if ((vel_inc.x || vel_inc.y || vel_inc.z) &&
              (collide_segment_capsule(start_particlep->my_pos, pos_plus_inc, *cap, hit_loc)) &&
              (collide_segment_capsule(start_particlep->my_pos, pos_plus_inc*.01f, *cap, dummy_hit_loc)) &&
              (!collide_segment_capsule(pos_plus_inc*.99f, pos_plus_inc, *cap, dummy_hit_loc)) )
        #endif

            {
              start_particlep->my_pos = hit_loc;
              start_particlep->my_velocity = ZEROVEC;
              //  what's going on here? -mkv
              //  this is code for the 'bsp spray', a debugging tool which sprays particles
              //  so you can see the bsp tree  -jdf
              start_particlep->scale *= 5.0f;
            }
          }
#endif
        }

        for(pfi = force_list.begin(); pfi != pfi_end ; ++pfi)
          start_particlep->my_velocity = (*pfi).get_new_v( *start_particlep, *this, t );

        start_particlep->my_pos += start_particlep->my_velocity * t;

        box.accumulate(start_particlep->my_pos);
      }

      ++start_particlep;
      if( start_particlep>=loop_particlep )
        start_particlep = particles;
    }

/*
    //////////////////////////////////////////////////////////////////////
    // Following code moved up to lower loop overhead (JDB 04-01-01)
    //////////////////////////////////////////////////////////////////////
    if (do_bsp)
    {
      assert( start_particle>=0 && start_particle<max_particles );
      start_particlep = particles+start_particle;
      assert( end_particle>=0 && end_particle<max_particles );
      end_particlep   = particles+end_particle;
      loop_particlep  = particles+max_particles;
      bool bspspray_enabled = os_developer_options::inst()->is_flagged(os_developer_options::FLAG_BSP_SPRAY_PAINT);
      while (start_particlep!=end_particlep)
      {
        vector3d hit_loc, hit_normal;
        vector3d vel_inc = start_particlep->my_velocity * t;
        if ( !bspspray_enabled && g_world_ptr->get_the_terrain().find_intersection(start_particlep->my_pos, start_particlep->my_pos+vel_inc, hit_loc, hit_normal) )
        {
          start_particlep->my_pos = hit_loc;
          vector3d vel = start_particlep->my_velocity;
          rational_t speed = vel.length();
          if (speed>0)
          {
            rational_t normal_speed = dot(vel, hit_normal);
            vel = vel-normal_speed*hit_normal*1.1f;
            vel += vel*(normal_speed/speed)*0.5f;
            start_particlep->my_velocity = vel;
          }
        }
        if (bspspray_enabled)
        {
          vector3d vel_inc = start_particlep->my_velocity * t;
          vector3d hit_loc, hit_normal;  // , dummy_hit_loc; // unused -- remove me?
      #if 0
          vector3d pog = start_particlep->my_pos;
          vector3d v0(0,2,0);
          vector3d v1(3,5,0);
          vector3d v2(3,2,0);
          hit_loc = start_particlep->my_pos;
          if ( dist_point_polygon(pog, v0, v1, v2)<0.1f)
      #endif
      #if 1
          if ( g_world_ptr->get_the_terrain().find_intersection(start_particlep->my_pos, start_particlep->my_pos+vel_inc, hit_loc, hit_normal) )
      #endif
      #if 0  //if SHOW_HERO_CAPSULE
          collision_capsule * cap = ((collision_capsule *) g_world_ptr->get_hero_ptr()->get_colgeom());
          if ((vel_inc.x || vel_inc.y || vel_inc.z) &&
            (collide_segment_capsule(start_particlep->my_pos, start_particlep->my_pos+vel_inc, *cap, hit_loc)) &&
            (collide_segment_capsule(start_particlep->my_pos, start_particlep->my_pos+vel_inc*.01f, *cap, dummy_hit_loc)) &&
            (!collide_segment_capsule(start_particlep->my_pos+vel_inc*.99f, start_particlep->my_pos+vel_inc, *cap, dummy_hit_loc)) )
      #endif
          //if ( !g_world_ptr->get_the_terrain().find_sector( start_particlep->my_pos ) )
    //        particles.erase( it );
          {
            start_particlep->my_pos = hit_loc;
            start_particlep->my_velocity = ZEROVEC;
            //  what's going on here? -mkv
            //  this is code for the 'bsp spray', a debugging tool which sprays particles
            //  so you can see the bsp tree  -jdf
            start_particlep->scale *= 5.0f;
          }
        }
        ++start_particlep;
        if( start_particlep>=loop_particlep )
          start_particlep = particles;
      }
    }

    assert( start_particle>=0 && start_particle<max_particles );
    start_particlep = particles + start_particle;
    assert( end_particle>=0 && end_particle<max_particles );
    end_particlep   = particles + end_particle;
    loop_particlep  = particles + max_particles;


    for( ; start_particlep != end_particlep ; )
    {
      if (start_particlep->life_remaining > 0.0F)
      {
        vector<particle_force>::iterator pfi;
        for(pfi = force_list.begin(); pfi != force_list.end() ; ++pfi)
        {
          start_particlep->my_velocity = (*pfi).get_new_v( *start_particlep, *this, t );
        }
        start_particlep->my_pos += start_particlep->my_velocity * t;

//        add_debug_sphere(start_particlep->my_pos, -1);

        box.accumulate(start_particlep->my_pos);
      }
      ++start_particlep;
      if( start_particlep>=loop_particlep )
        start_particlep = particles;
    }
*/


    if( flags&LOCAL_SPACE )
    {
      visual_center = box.center();
      abs_visual_center = generator_po->slow_xform(visual_center);
    }
    else
    {
      abs_visual_center = box.center();
      visual_center = generator_po->fast_inverse_xform(abs_visual_center);
    }

    visual_radius = box.radius()+PARTICLE_MIN_VISUAL_RADIUS;

#ifdef NGL
    if( my_visrep ) {
      visual_radius += my_visrep->get_radius( 0.0f );
    } else {
      visual_radius += my_mesh->SphereRadius;
    }
#else
    visual_radius += my_visrep->get_radius(0.0F);
#endif

    if ( start_particle==end_particle )
      if ( !is_visible() )
      {
        destroy_particles();
      }
  }

  last_position_valid = true;

  if(!is_still_visible())
  {
    entity::region_update_poss_active();
    entity::region_update_poss_render();
  }

  // cushion it in case the particles are quite large.  (need to add max radius of one particle, actually)
  //rational_t base_rad = get_base_visual_radius();
  //visual_radius += (base_rad > PARTICLE_MIN_VISUAL_RADIUS) ? base_rad : PARTICLE_MIN_VISUAL_RADIUS;

  // hope dies in the end, nothing is eternal
  proftimer_advance_particles.stop();
  proftimer_special_fx.stop();
}

render_flavor_t particle_generator::render_passes_needed() const
{
  //if (fade_in || fade_out)
    return RENDER_TRANSLUCENT_PORTION;
  //return my_visrep->render_passes_needed();
}

extern profiler_timer proftimer_render_particles;
extern profiler_timer proftimer_render_particles_bb;
extern profiler_timer proftimer_render_particles_mesh;

extern profiler_counter profcounter_sfx_tri_rend;
extern profiler_counter profcounter_particle_systems_rend;
extern profiler_counter profcounter_particle_rend;
extern profiler_counter profcounter_particle_rend_bb;
extern profiler_counter profcounter_particle_rend_mesh;

#ifdef NGL

static int _nglRenderInstanceInfo( nglMesh* mesh, instance_render_info* iri, int num, render_flavor_t flavor )
{
  assert( mesh );
  assert( iri );

#if defined NGL

  for( int i = 0; i < num; i++ ) {
    const po& p = iri[i].get_local_to_world( );
    const color32& c = iri[i].get_color_scale( );
    const float s = iri[i].particle_scale;

    nglRenderParams params;
    params.Flags = 0;

    if ( c.c.a != 255 )
    {
      params.Flags |= NGLP_TINT;
      params.TintColor[0] = 1.0f;
      params.TintColor[1] = 1.0f;
      params.TintColor[2] = 1.0f;
      params.TintColor[3] = ( ( (float) c.c.a ) / 255.0f );
    }

    if ( s != 1.0f )
    {
		params.Flags |= NGLP_SCALE;
		params.Scale[0] = s;
		params.Scale[1] = s;
		params.Scale[2] = s;
		if (s < 0)
		{
			params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// particle is rendered flipped (dc 04/30/02)
		}
    }


    // entity widget particle systems need to be drawn
    // with a full local->screen matrix
    if ( flavor & RENDER_ENTITY_WIDGET )
    {
      nglMatrix view_to_screen;
      nglGetMatrix( view_to_screen, NGLMTX_VIEW_TO_SCREEN );
//      params.Flags |= NGLP_FULL_MATRIX;	// Obsolete flag.  Was ignored previously.  (dc 05/30/02)
      matrix4x4 &fp = *(matrix4x4 *)&p;
      matrix4x4 vs = *(matrix4x4 *)&view_to_screen;
      matrix4x4 the_xform = fp * vs;
      START_PROF_TIMER( proftimer_render_add_mesh );
      nglListAddMesh( mesh, native_to_ngl( the_xform ), &params);
      STOP_PROF_TIMER( proftimer_render_add_mesh );
    }
    else // not an entity widget
    {
      START_PROF_TIMER( proftimer_render_add_mesh );
      nglListAddMesh( mesh, native_to_ngl( p.get_matrix( ) ), &params );
      STOP_PROF_TIMER( proftimer_render_add_mesh );
    }
  }

#endif

  return 0;
}

#endif // #ifdef NGL

void particle_generator::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
  if( disable_for_sony_booth )
    return;

  if (!is_visible () && !always_render)
    return;

  instance_render_info *viri = app::inst()->get_viri();

  profcounter_particle_systems_rend.add_count(1);

  proftimer_render_particles.start();

#ifndef BUILD_BOOTABLE
  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_PARTICLES))
  {
    proftimer_render_particles.stop();
    return;
  }
#endif

#ifdef DEBUG
  //debug_this = this;
#endif

#if defined(TARGET_PC) && defined(DEBUG)
  if ((flavor & render_passes_needed())==0)
  {
    proftimer_render_particles.stop();
    assert(false);
    return;
  }
#endif

#ifdef NGL
  if( !my_visrep && !my_mesh ) {
    return;
  }
#endif

  if(particles)
  {
    *generator_po = get_abs_po();
    // remove scaling from generator po without affecting position
    generator_po->non_affine_normalize();

    //time_value_t now = last_active_now; //game_clock::inst()->get_time();
    vector3d camera_pos = geometry_manager::inst()->get_camera_pos();
    vector3d camera_dir = geometry_manager::inst()->get_camera_dir();

#ifdef NGL
    visrep_t visrep_type = ( my_visrep ) ? my_visrep->get_type( ) : VISREP_KRMESH;
    int num_faces = ( my_visrep ) ? my_visrep->get_max_faces( ) : 0;
#else
    visrep_t visrep_type = my_visrep->get_type();
    int num_faces = my_visrep->get_max_faces();
#endif

    instance_render_info* virip = app::inst()->get_viri();
    assert( start_particle>=0 && start_particle<max_particles );
    particle* my_particle = particles + start_particle;
    assert( end_particle>=0 && end_particle<max_particles );
    particle* particle_end = particles + end_particle;
    particle* loop_particle = particles + max_particles;

    bool is_simple_billboard = (visrep_type==VISREP_BILLBOARD) && ((((vr_billboard*)my_visrep)->get_billboard_flags()&vr_billboard::AXIS_LOCK)==0);

    for( ;my_particle!=particle_end; my_particle = ((my_particle+1)==loop_particle)?(particles):(my_particle+1))
    {
      if( my_particle->life_remaining > 0.0F )
      {
        time_value_t age = my_particle->total_life-my_particle->life_remaining;
        if (age<0) age = 0;

        rational_t scale_fraction;
        rational_t life_expectancy = my_particle->life_remaining;
        if (life_expectancy<0) life_expectancy = 0;
        if( age < grow_for )
        {
          scale_fraction = age * rh_grow_for;
        }
        else if ( life_expectancy < shrink_for )
        {
          time_value_t backwards_age = life_expectancy;
          scale_fraction = backwards_age * rh_shrink_for;
        }
        else
          scale_fraction = 1;
        scale_fraction *= my_particle->scale;
        if( scale_fraction<=0)
          continue;

        rational_t opacity_fraction=1.0f;
        if( age  < fade_in )
        {
          opacity_fraction = age * rh_fade_in;
        }
        else if (fade_out)
          if( life_expectancy < fade_out )
          {
            opacity_fraction = life_expectancy * rh_fade_out;
          }
        if (opacity_fraction<0)
          continue;
          //opacity_fraction=0;

        // calculate rotation
        float rads;

        if(rotation_period) {
          rads = ( ( age * recip_rotation_period * 2 * PI ) + my_particle->rotation ) * my_particle->rotation_speed;
        } else {
          rads = 0;
        }

        vector3d cur_position;

        if ( flags&LOCAL_SPACE )
        {
          cur_position = generator_po->fast_8byte_xform(my_particle->my_pos);
        }
        else
        {
          cur_position = my_particle->my_pos;
        }

        // discard if on our side of front plane
        vector3d camera_to_particle = cur_position - camera_pos;
        rational_t distance_in_front_of_camera = dot( camera_to_particle, camera_dir );  // *

        if( distance_in_front_of_camera <= PROJ_NEAR_PLANE_D )
          continue;

        po scaled_po;

        // assign rotation to matrix if necessary
        if(!is_simple_billboard)  // not wasting time for billboards which don't use the info
        {
          // this section's a big fatass, but it only gets called for meshy particles and axis lock billboard particles
          if( flags & GENERATOR_ORIENTED )
          {
            scaled_po = *generator_po;                                                    // *
          }
          else if(flags & DIRECTION_ORIENTED )
          {
//            vector3d position = get_abs_position(); // unused -- remove me?
            vector3d v = my_particle->my_velocity;
            // find a transformation that takes us from [0,1,0] to v
            // this means rotating around a vector perpendicular to both of them that
            // many degrees.

            vector3d perpendicular( v.z, 0, -v.x );  // cross of vector3d(0,1,0) and v.
            float numerator = v.y;   // numerator of dot( vector3d(0,1,0), v );
            #if 0 // untested, but faster
            float invdenominator = fast_recip_length(v.x, v.y, v.z);  // norm(vector3d(0,1,0)*norm(v);
            rational_t angle = fast_acos( numerator*invdenominator );                        // * ouch
            #else
            float denominator = fast_length(v.x, v.y, v.z);  // norm(vector3d(0,1,0)*norm(v);
            if ( denominator == 0.0f )
              denominator = 0.001f;
			// fast_length(v.x, v.y, v.z) < v.y is possible for v.x ~ v.z ~ 0, small v.y (dc 07/12/02)
            if ( denominator > fabsf(numerator) )
				denominator = numerator;
            rational_t angle = fast_acos( numerator/denominator );                        // * ouch
            #endif
            scaled_po.set_rot( perpendicular, -angle );                               // *
            // does - change if we switch handedness of coordinate system?
          }
          else
            scaled_po = po_identity_matrix;                                           // *
          if (visrep_type != VISREP_BILLBOARD)
          { // billboards handle scale and rotation differently
            if (scale_fraction!=1.0f)
            {
#ifndef NGL
#pragma todo( "remove this guard when kRender supports scaling. -mkv 4/6/01" )
              scaled_po.scale(scale_fraction);
#endif
            }
            if (rads != 0.0f)
            {
              po rotation;
              rotation.set_rot( my_particle->rot_axis, rads );                            // *
              scaled_po = rotation * scaled_po;                                           // *
            }
          }
          scaled_po.set_position( cur_position );
        }

        // kludge to make duration of particle's animation last its lifetime:
        time_value_t fake_age = age;
        if(flags&LIFETIME_ANIMS)
        {
#ifdef NGL
          if( my_visrep ) {
            fake_age = my_visrep->get_ending_time() * age / my_particle->total_life;
          } else {
            fake_age = LARGE_TIME;
          }
#else
          fake_age = my_visrep->get_ending_time() * age / my_particle->total_life;
#endif
        }
        virip->number_of_faces_to_attempt = num_faces;
        if (is_simple_billboard)
        {
          // simple billboards don't use any of the po except the translation
          //virip->local_to_world = global_identity_po;
          //virip->local_to_world = *generator_po;
          virip->local_to_world.set_position( cur_position );
        }
        else
          virip->local_to_world = scaled_po;
        virip->force_flags                = FORCE_TRANSLUCENCY | FORCE_NO_LIGHT;
        virip->color_scale                = color32(255,255,255,255*opacity_fraction);
        virip->camera_relative_rotation   = rads;

        virip->set_age(fake_age);

        virip->my_region                  = get_region();
        virip->particle_scale             = scale_fraction;

        virip->set_ifl_frame_boost(0);

        virip->my_light_set               = NULL;
        virip->set_alt_materials( NULL );
        ++virip;
      }
    }
    if (visrep_type == VISREP_BILLBOARD)
      proftimer_render_particles_bb.start();
    else
      proftimer_render_particles_mesh.start();
    if(virip>viri)
    {
      int start_polys = hw_rasta::inst()->get_poly_count();
      int count = virip-viri;
      profcounter_particle_rend.add_count(count);
      if (visrep_type == VISREP_BILLBOARD)
        profcounter_particle_rend_bb.add_count(count);
      else
        profcounter_particle_rend_mesh.add_count(count);
#ifdef NGL
      if( visrep_type == VISREP_BILLBOARD ) {
        my_visrep->render_batch( flavor, viri, count );
      } else {
        _nglRenderInstanceInfo( my_mesh, viri, count, flavor );
      }
#else
      my_visrep->render_batch( flavor, viri, count );
#endif
      profcounter_sfx_tri_rend.add_count(hw_rasta::inst()->get_poly_count() - start_polys);
    }
    if (visrep_type == VISREP_BILLBOARD)
      proftimer_render_particles_bb.stop();
    else
      proftimer_render_particles_mesh.stop();
  }

  proftimer_render_particles.stop();
}


bool particle_generator::is_on() const
{
  time_value_t age = get_age();
  time_value_t slice = fmodf(age, (on_for+off_for));
  return slice < on_for;
}

void particle_generator::set_visible(bool a)
{
  bool changed = entity::is_visible() != a;

  if (a)
    time_to_next_particle = 0.0F;

  entity::set_visible(a);

  if(changed)
    entity::region_update_poss_active();
}

bool particle_generator::is_still_visible() const
{
  return is_visible() || start_particle!=end_particle;
}

vector3d particle_generator::get_visual_center() const
{
  return abs_visual_center;
}

rational_t particle_generator::get_base_visual_radius() const
{
  return(base_speed*(speed_variation+1)*particle_life_span
         +generation_radius
         +generation_height) * PARTICLE_BASE_RADIUS_MOD + PARTICLE_MIN_VISUAL_RADIUS;
}

rational_t particle_generator::get_visual_radius() const
{
  return visual_radius * 2; // frame_advance particles before you can see them, so they can spawn
}

rational_t particle_generator::get_visual_xz_radius_rel_center() const
{
  return get_visual_radius();
}

void particle_generator::set_created_entity_default_active_status()
{
  set_active(false);
}


void particle_generator::destroy_particles()
{
  // moved to destructor
}

////////////////////////////////////////////////////////////////////////////////

vector3d particle_force::get_new_v( const particle& _particle, particle_generator& pg, time_value_t delta_t )
{
  vector3d result;
  if (flavor==RESISTANCE)
  {
    rational_t norm_vel = fast_length2(_particle.my_velocity.x, _particle.my_velocity.y, _particle.my_velocity.z );
    assert(terminal_speed>=0);
    if( norm_vel > terminal_speed*terminal_speed)
    {
      norm_vel = __fsqrt(norm_vel);
      rational_t mul = -delta_speed * (norm_vel-terminal_speed) / norm_vel;
      mul *= delta_t;
      result = _particle.my_velocity * (mul + 1.0f);
    }
    else
      result = _particle.my_velocity;
  }
  else
  {
    if (flavor==CONSTANT)
    {
      result = _particle.my_velocity + multipurpose * delta_t;
    }
    else
    {
      vector3d orientation = utility - _particle.my_pos;
      rational_t recip_n = fast_recip_length(orientation.x, orientation.y, orientation.z);
      if (recip_n>100)
      {
        orientation = XVEC;
        recip_n = 1;
      }
      orientation *= delta_speed * recip_n;
      result = _particle.my_velocity + orientation * delta_t;
    }
    float recip_norm_result = fast_recip_length(result.x, result.y, result.z);
    if (recip_norm_result < recip_terminal_speed)
    {
      result = result * (terminal_speed * recip_norm_result);
    }
  }
  return result;
}

