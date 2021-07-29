// entity_maker.cpp
// Copyright (c) 2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
// now contains functions for creating entities
// since we now want to be able to have entities
// outside the wds

#include "global.h"

#include "entity.h"
#include "path.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "wds.h"
#include "profiler.h"
#include "marker.h"
#include "light.h"
#include "particle.h"
#include "item.h"
// BIGCULL #include "turret.h"
// BIGCULL #include "scanner.h"
#include "sky.h"

// BIGCULL #include "gun.h"
// BIGCULL #include "thrown_item.h"
// BIGCULL #include "melee_item.h"
#include "pmesh.h"
#include "file_finder.h"
#include "widget_entity.h"
#include "conglom.h"
#include "osdevopts.h"
// BIGCULL #include "manip_obj.h"
// BIGCULL#include "switch_obj.h"
#include "polytube.h"
#include "lensflare.h"
#include "FrontEndManager.h"


#include <cstdarg>


extern world_dynamics_system * g_world_ptr;
extern file_finder *g_file_finder;


entity_maker::entity_maker()
{
  owning_widget = NULL;
}


entity_maker::~entity_maker()
{
}



stringx entity_maker::open( chunk_file& fs, const stringx& filename, const stringx& extension, int io_flags )
{
  stringx fname;
  if ( file_finder_exists( filename, extension, &fname ) )
  {
    fs.open( fname, io_flags );
    return fname;
  }
  else
  {
    error( "Couldn't open \"" + filename + extension + "\"" );
    return stringx();
  }
}


/*
extern profiler_counter profcounter_total_blocks;
extern profiler_counter profcounter_alloced_mem;
extern profiler_counter profcounter_terrain_mem;
extern profiler_counter profcounter_entity_mem;
extern profiler_counter profcounter_physent_mem;
extern profiler_counter profcounter_actor_mem;
extern profiler_counter profcounter_character_mem;
extern profiler_counter profcounter_hero_mem;
extern profiler_counter profcounter_conglom_mem;
extern profiler_counter profcounter_item_mem;
extern profiler_counter profcounter_marker_mem;
extern profiler_counter profcounter_particle_mem;
extern profiler_counter profcounter_light_mem;
extern profiler_counter profcounter_ladder_mem;
extern profiler_counter profcounter_script_mem;
extern profiler_counter profcounter_texture_mem;
extern profiler_counter profcounter_audio_mem;
*/

// This function decides what sort of entity to create based on the first entry in the .ent file
entity* entity_maker::create_entity_or_subclass( const stringx& entity_name,
                   entity_id id,
                   po const & loc,
                   const stringx& scene_root,
                   unsigned int scene_flags,
                   const region_node_list *forced_regions)
//create_entity_or_subclass(const stringx& entity_name,
//                          entity_id id,
//                          po const & loc,
//              						const stringx& scene_root,
//                          unsigned int scene_flags,
//                          const list<region_node*>* forced_regions )
{
  assert( g_world_ptr || owning_widget );

  entity* e;
//  int diff=0;
//P  int outside_before = memtrack::get_total_alloced();
  {
    po my_po = loc;

    stringx temp;
    //list<region_node*>::const_iterator fri;
//    region_node_list::const_iterator fri;

    bool active     = ( (scene_flags & ACTIVE_FLAG)     != 0 );
    bool stationary = ( (scene_flags & STATIONARY_FLAG) != 0 );
    bool invisible  = ( (scene_flags & INVISIBLE_FLAG)  != 0 );
    bool cosmetic   = ( (scene_flags & COSMETIC_FLAG)   != 0 );
    bool walkable   = ( (scene_flags & WALKABLE_FLAG)   != 0 );
    bool repulsion  = ( (scene_flags & REPULSION_FLAG)  != 0 );
    bool nonstatic  = ( (scene_flags & NONSTATIC_FLAG) != 0 );
    bool nodistclip = ( (scene_flags & NO_DISTANCE_CLIP_FLAG) != 0 );


//#if defined( PROJECT_STEEL )
    // CTT 08/15/00:
    // In Max Steel, the notion of data people making things physical by
    // setting the ACTIVE and STATIONARY flags has pretty much been lost,
    // so now it is the code's responsibility to call set_stationary(false)
    // or, equivalently, set_flag(EFLAG_PHYSICS_MOVING,true) whenever
    // appropriate.
    // In my opinion, this is the way it should always have been.
    stationary = true;
/*
#else

    // Recall:  Stationary does NOT mean scripts can't move it, It just
    // means the object doesn't have velocity (or affected by forces)
    if (walkable || !active)
      stationary = TRUE;
#endif
*/
    // set up flags for entity constructors and make_instance() calls
    unsigned int ent_flags = 0;
    if ( active && !stationary )
      ent_flags |= EFLAG_ACTIVE;
    if ( !stationary )
      ent_flags |= EFLAG_PHYSICS_MOVING;
    if ( walkable )
      ent_flags |= EFLAG_PHYSICS_WALKABLE;
    if ( !invisible )
      ent_flags |= EFLAG_GRAPHICS_VISIBLE;
    if ( repulsion )
      ent_flags |= EFLAG_MISC_REPULSION;
    if ( nodistclip )
      ent_flags |= EFLAG_GRAPHICS_NO_DISTANCE_CLIP;
    if ( nonstatic )
    {
      ent_flags |= EFLAG_MISC_NONSTATIC;
      ent_flags |= EFLAG_MISC_RAW_NONSTATIC;
    }

    filespec entspec( entity_name );
    entspec.name.to_upper();
    entfile_map::const_iterator fi;
    if ( owning_widget == NULL )
    {
//#pragma todo("was a comment --> 'KILLME:  What the hell does this code do?'")
      fi = g_world_ptr->get_entfiles().find( entspec.name );
//      entity * fake = (*fi).second;
    }
    stringx ent_filename = empty_string;
    stringx ent_filename_low = empty_string;

// This makes a big mess of the path list.  We should get rid of it.  I added a pop below, in the meantime. (dc 01/16/02)
    if( !os_developer_options::inst( )->is_flagged( os_developer_options::FLAG_STASH_ONLY ) )
      g_file_finder->push_path_back( "characters\\" + entity_name + "\\" );

    if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
      ent_filename = g_file_finder->find_file( entity_name, ".ent", true );
    else
      ent_filename = entity_name + ".ent";
  
    if( !os_developer_options::inst( )->is_flagged( os_developer_options::FLAG_STASH_ONLY ) )
      g_file_finder->pop_path_back( );
    //fi = g_world_ptr->get_entfiles().end();

    filespec spec( ent_filename );

    if ( owning_widget == NULL && fi != g_world_ptr->get_entfiles().end() )
    {
      // file already loaded; copy from previous instance
      entity_flavor_t flav = (*fi).second->get_flavor();
      // make actors, characters, and markers nonstatic
      if ( flav == ENTITY_MARKER ||
           flav == ENTITY_BEAM ||
           flav == ENTITY_PARTICLE_GENERATOR ||
           flav == ENTITY_PHYSICAL ||
           flav == ENTITY_ITEM ||
           flav == ENTITY_SCANNER )
      {
        nonstatic = true;
        ent_flags |= EFLAG_MISC_NONSTATIC;
      }
      e = (*fi).second->make_instance( id, ent_flags );
      // add to appropriate list
      switch ( e->get_flavor() )
      {
        case ENTITY_ENTITY:
          create_entity( e );
          e->set_rel_po( my_po );
          break;

        case ENTITY_MARKER:
          g_world_ptr->add_marker( static_cast<marker*>(e) );
          break;

        case ENTITY_BEAM:
          g_world_ptr->add_beam( static_cast<beam*>(e) );
          break;

#if 0 // BIGCULL
        case ENTITY_MANIP:
          create_entity( e );
          break;

        case ENTITY_SWITCH:
          create_entity( e );
          break;
#endif // BIGCULL
        case ENTITY_LIGHT_SOURCE:
          g_world_ptr->add_light_source( static_cast<light_source*>(e) );
          break;

        case ENTITY_PARTICLE_GENERATOR:
          g_world_ptr->add_particle_generator( static_cast<particle_generator*>(e) );
          break;

        case ENTITY_LENSFLARE:
          g_world_ptr->add_lensflare( static_cast<lensflare*>(e) );
          break;

        case ENTITY_ITEM:
          g_world_ptr->add_item( static_cast<item*>(e) );
          break;

        case ENTITY_CONGLOMERATE:
          create_entity( e );
          break;

#if 0 // BIGCULL
        case ENTITY_TURRET:
          g_world_ptr->add_turret( static_cast<turret*>(e) );
          break;

        case ENTITY_SCANNER:
          g_world_ptr->add_scanner( static_cast<scanner*>(e) );
          break;
#endif // BIGCULL
        case ENTITY_SKY:
          g_world_ptr->add_sky( static_cast<sky*>(e) );
          e->set_rel_po( my_po );
          break;

        default:
          assert(0);
      }
      if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_LORES_MODELS) && (ent_filename_low = g_file_finder->find_file( entity_name, "_low.ent", true )) != empty_string)
      {
        e->fileName = spec.name;
      }
      else
      {
        e->fileName = spec.name;
      }

    }
    else
    {
      // THIS WAS CHANGED TO MAKE Metrowerks WORK ON DC, but caused
      // a bug because this function is recursive (characters add items)
      //static chunk_file fs;
      chunk_file fs;

      // load NEW file

#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,ent_filename.c_str());
#endif

      if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_LORES_MODELS) && (ent_filename_low = g_file_finder->find_file( entity_name, "_low.ent", true )) != empty_string)
      {
        fs.open( ent_filename_low );
      }
      else
      {
        fs.open( ent_filename );
	  }
      if (fs.at_eof())
	  {
		nglPrintf("WDS:\tCould not load entity %s.  Continuing...\n", ent_filename.c_str());
        return NULL;
	  }

     

      read_meshes( fs );
      
//
      stringx entity_flavor_name;
      serial_in(fs,&entity_flavor_name);

      if ( entity_flavor_name == "NEWENT" )
      {
        // NEW entity file format
        chunk_flavor cf;
        serial_in( fs, &cf );
        if ( cf != chunk_flavor("node") )
          error( fs.get_name() + ": expected \"node\" chunk" );
        stringx dummy;
        serial_in( fs, &dummy );
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
        if ( cf == chunk_flavor("po") )
        {
          // skip root po
          po _po;
          serial_in( fs, &_po );
          serial_in( fs, &cf );
        }
        // read according to type
//        int entflavor = -1;
        if ( cf == chunk_flavor("physical") )
        {
          if ( owning_widget )
            creating_widget_error( "physical entity" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          assert(0);
//          e = g_world_ptr->add_physent( fs, id, ent_flags );
//          e->set_rel_po( my_po );
        }
        else if ( cf == chunk_flavor("entity") )
        {
          e = create_entity( fs, id, ent_flags );
          e->set_rel_po( my_po );
        }
        else if ( cf == chunk_flavor("light") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "light source" );
          e = g_world_ptr->add_light_source( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("partsys") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "particle system" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          stringx fname;
          serial_in( fs, &fname );
          e = g_world_ptr->add_particle_generator( fname, id, ent_flags );
        }
        else if ( cf == chunk_flavor("lensflare") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "lensflare" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_lensflare( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("conglom") )
        {
          e = create_conglomerate( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("char") )
        {
          e = create_conglomerate( fs, id, ent_flags );
        }
#if 0 // BIGCULL
        else if ( cf == chunk_flavor("turret") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "turret" );
          e = g_world_ptr->add_turret( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("char") )
        {
          STUBBED(entity_maker_create_char, "character chunks");
        }
#endif // BIGCULL
        else if ( cf == chunk_flavor("item") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "item" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_item( fs, id, ent_flags );
        }
#if 0 // BIGCULL
        else if ( cf == chunk_flavor("gun") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "gun" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_gun( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("melee") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "melee" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_melee( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("thrown") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "thrown item" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_thrown_item( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("manip") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "manip object" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_manip_obj( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("switch") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "switch object" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_switch_obj( fs, id, ent_flags );
        }
#endif // BIGCULL
        else if ( cf == chunk_flavor("polytube") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "polytube" );
          ent_flags |= EFLAG_MISC_NONSTATIC;
          e = g_world_ptr->add_polytube( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("ladder") )
        {
STUBBED(entity_maker_create_ladder, "ladder chunks");
        }
#if 0 // BIGCULL
        else if ( cf == chunk_flavor("scanner") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "scanner" );
	        e = g_world_ptr->add_scanner( fs, id, ent_flags );
        }
        else if ( cf == chunk_flavor("crate") )
        {
          assert(0);
        }
#endif // BIGCULL
        else if ( cf == chunk_flavor("sky") )
        {
          if ( owning_widget != NULL )
            creating_widget_error( "sky" );
          e = g_world_ptr->add_sky( fs, id, ent_flags );
          e->set_rel_po( my_po );
        }
        else
          error( fs.get_name() + ": unknown chunk" );
        // read .enx file, if any
        fs.close();
        stringx stash_lookup(spec.name+".enx");


      if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_LORES_MODELS) && (ent_filename_low = g_file_finder->find_file( entity_name, "_low.ent", true )) != empty_string)
      {
        e->fileName = spec.name;
      }
      else
      {
        e->fileName = spec.name;
      }




// BETH
// these lines should not be executed while creating an entity in the front end
		if(FEDone())
		{

	        if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
		    {
	          if ( stash::file_exists( stash_lookup.c_str()))
		      {
			    fs.open( stash_lookup, os_file::FILE_READ|chunk_file::FILE_TEXT );
				e->read_enx( fs );
				fs.close();
			  }
			  else if (e->get_flavor()==ENTITY_ITEM)
			  {
			    stringx composite = "File "+spec.path + spec.name + ".enx not found, for an entity which requires it.";
				error(composite.c_str());
			  }
			}
			else if ( os_file::file_exists( spec.path + spec.name + ".enx" ) )
			{
			  open( fs, spec.path + spec.name, ".enx", os_file::FILE_READ|chunk_file::FILE_TEXT );
			  e->read_enx( fs );
			  fs.close();
			}
	        else if (e->get_flavor()==ENTITY_ITEM)
	        {
	          stringx composite = "File "+spec.path + spec.name + ".enx not found, for an entity which requires it.";
	          error(composite.c_str());
	        }
		}
// end BETH

		e->set_anim_id( ent_animid );
      }
      else if ( owning_widget == NULL )
      {
        // old entity file format
        fs.close();

        int i=0;
        while (!(entity_flavor_name==entity_flavor_names[i]) && i<NUM_ENTITY_FLAVORS) i++;
        if (i==NUM_ENTITY_FLAVORS)
        {
          stringx composite = stringx("Invalid entity type: ")+entity_flavor_name;
          error(composite.c_str());
        }

        #if defined(PROFILE_ENTITY_MEMORY) || defined(PROFILE_PHYSENT_MEMORY) || defined(PROFILE_CHARACTER_MEMORY)
        fstream eout, peout, chout;
        static int num=0;
        #endif

        // This is a hack to support old-style "physents that should really be plain old entities."
        if ((stationary || !active) && i==ENTITY_PHYSICAL)
          i = ENTITY_ENTITY;
        else if (i==ENTITY_MOBILE)
          i = ENTITY_PHYSICAL;

     	  stringx name(spec.path+spec.name);

        switch (i)
        {
          case ENTITY_ENTITY:
            {
            g_world_ptr->add_entity( name, active, stationary, invisible, cosmetic, walkable, repulsion, nonstatic, id);
            e = entity_manager::inst()->find_entity(id,IGNORE_FLAVOR);
            e->set_rel_po(my_po);
            #if defined(PROFILE_ENTITY_MEMORY)
            if (++num==stop_point)
              warning("stopped");
            stringx where=os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) + "\\eout.txt";
            eout.open(where.c_str(), ios::out | ios::app);
            eout << e->get_id().get_val().c_str() << "," << diff << endl;
            eout.close();
            #endif
            }
            break;

          case ENTITY_ITEM:
            nonstatic = true;

            assert(walkable==false);
            g_world_ptr->add_item( name, active, stationary, invisible, cosmetic, id);
            break;

          case ENTITY_MORPHABLE_ITEM:
            nonstatic = true;
            assert(walkable==false);
            g_world_ptr->add_morphable_item( name, active, stationary, invisible, cosmetic, id);
            break;

          case ENTITY_MARKER:
            g_world_ptr->add_marker( id );
            break;

          case ENTITY_BEAM:
          {
            beam *bm = g_world_ptr->add_beam( id, ent_flags );
            bm->set_active(active);
            bm->set_visible(!invisible);
          }
          break;

          case ENTITY_PARTICLE_GENERATOR:
            nonstatic = true;
            g_world_ptr->add_particle_generator( name, invisible, nonstatic, id);
            break;

          case ENTITY_LIGHT:
            g_world_ptr->add_light_source( name, invisible, nonstatic, id);
            break;
        }
        e = entity_manager::inst()->find_entity(id,IGNORE_FLAVOR);
      }

      // register entity file
      if ( owning_widget == NULL )
      {
        stringx strx( entspec.name );
        entfile_map::value_type v( strx, e );
        g_world_ptr->insert_entfile( v );
      }
    }

    // Newly created flags
    e->process_extra_scene_flags(scene_flags);

    e->set_rel_po( my_po );
    e->set_last_po( my_po );
    e->compute_visual_xz_radius_rel_center();

    // non-uniform scaling is not allowed;
    // uniform scaling is allowed only on entities that have no collision geometry
    check_po( e );

    // this needs to be called to set up last-frame info
    e->frame_done();

/* BETH	if ( owning_widget == NULL )
    {
      // we want each static entity to have an axis-aligned bounding box for collisions
      // (note that the bounding box will only be created if the entity has a cg_mesh)
      if ( e->is_statically_sortable() )
        e->compute_bounding_box();

      if ( forced_regions && !forced_regions->empty() )
      {
        // force entity to always be counted in given region(s)
        for ( fri=forced_regions->begin(); fri!=forced_regions->end(); fri++ )
          e->force_region( *fri );
      }
      else
      {
        // compute entity sector (and, thus, region)
        e->compute_sector( g_world_ptr->get_the_terrain(), g_world_ptr->is_loading_from_scn_file() );
      }
    }
*/

#if _ENABLE_WORLD_EDITOR
    e->ent_filename = entity_name;
    e->ent_filename.to_lower();
#endif

  g_world_ptr->add_entity_preload_script(e, entity_name);

  #if defined (DEBUG)
  //  e->debug_name = NEW stringx(entity_name);
  #endif
  }

  //if(e)
  //  e->add_signal_callbacks();

  if ( owning_widget != NULL )
    e->set_owning_widget( owning_widget );

  return e;
}



entity *entity_maker::create_entity( chunk_file& fs,
                                   const entity_id& id,
                                   unsigned int flags,
                                   bool add_bones )
{
  entity* ent = NEW entity( fs, id, ENTITY_ENTITY, flags );

  if(add_bones || ent->get_bone_idx() < 0)
    create_entity( ent );

  return ent;
}


void entity_maker::create_entity( entity* e )
{
  if ( owning_widget == NULL )
  {
    g_world_ptr->add_to_entities( e );
  }
  else
    e->set_owning_widget( owning_widget );

  // non-uniform scaling is not allowed;
  // uniform scaling is allowed only on entities that have no collision geometry
  check_po( e );

  // if you need the compute_sector call, use add_dynamic_instanced_entity instead
  // some flavors of entity are inactive by default
  switch ( e->get_flavor() )
  {
    case ENTITY_ENTITY:
    case ENTITY_MARKER:
    case ENTITY_BEAM:
    case ENTITY_MIC:
    case ENTITY_LIGHT_SOURCE:
    case ENTITY_CONGLOMERATE:
    {
      visual_rep *vrep = e->get_vrep();
      if( !(vrep && vrep->get_type()==VISREP_PMESH && ((vr_pmesh*)vrep)->is_uv_animated()) )
        e->set_flag( EFLAG_ACTIVE, false );
      break;
    }
    default:
      break;
  }
  // this needs to be called to set up last-frame info
  e->frame_done();
}

void entity_maker::read_meshes( chunk_file& fs ) // puts the meshes into the bank
{
  // FIXME: ngl doesn't support multi-mesh mesh files atm
  filespec txtmesh_spec( fs.get_name( ) );
  chunk_file txtmesh_fs;

  txtmesh_spec.ext = NGL_MESHFILE_EXT;

//  txtmesh_spec.path = file_manager::inst( )->hack_off_root_dir( txtmesh_spec.path );

  if ( !( world_dynamics_system::wds_exists ( txtmesh_spec.path + txtmesh_spec.name, txtmesh_spec.ext ) ) )
  {
    nglPrintf("The file %s is still using the obsolete multi-mesh format.\n", txtmesh_spec.fullname().c_str());
    /*
    static bool already_warned = false;
    if(!already_warned)
    {
      already_warned = true;
      warning("Some of the meshes still use the obsolete multi-mesh format. They will be listed in c:\\sm\\data\\obsolete_pc.txt.");
    }
#ifdef BUILD_DEBUG
    file_id_t obslog = file_manager::inst()->get_log_id("obsolete");
    if ( obslog == FILE_MANAGER_INVALID_FILE_ID )
      obslog = file_manager::inst()->acquire_log("obsolete");
    if( obslog != FILE_MANAGER_INVALID_FILE_ID )
	    file_manager::inst()->write_log( obslog, "%s\n", txtmesh_spec.fullname().c_str() );
#endif
      */
    return;
  }

#ifdef NGL

/*#if defined(TARGET_GC)
  nglSetMeshPath( txtmesh_spec.path.c_str() );
  // JIV FIXME
  // Well golly sarge why the hell am I setting the texture path with
  // to mesh path PYLE
//  	nglSetTexturePath( ( txtmesh_spec.path + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\" ).c_str() );
	nglLoadMesh(txtmesh_spec.name.c_str());
#else*/
	nglLoadMeshFile( txtmesh_spec.name.c_str() );
//#endif

#else

  txtmesh_spec.path = "";
  stringx txtmesh_file_name( txtmesh_spec.name + txtmesh_spec.ext);
  // txtmesh_spec.path = file_manager::inst( )->get_found_path(txtmesh_file_name );

  txtmesh_fs.open( txtmesh_file_name, os_file::FILE_READ );

  chunk_flavor cf;

  serial_in( txtmesh_fs, &cf );
  if ( cf != chunk_flavor( "nomeshes" ) )
  {
    txtmesh_fs.close( );
    return;
  }

  stringx mesh_name;
  int mesh_index, mesh_total;

  serial_in( txtmesh_fs, &mesh_total );
  assert( mesh_total > 0 );

  for ( mesh_index = 0; mesh_index < mesh_total; mesh_index++ )
  {
    serial_in( txtmesh_fs, &cf );
    if ( cf != chunk_flavor( "mesh" ) )
      error( "Expected 'mesh' chunk in entity_maker::read_meshes" );

    serial_in( txtmesh_fs, &cf );
    serial_in( txtmesh_fs, &mesh_name );

    if ( cf == chunk_flavor( "meshname" ) )
    {
      if( vr_pmesh_bank.find_instance( mesh_name ) )
      {
        // we've already got one of these meshes, ergo we've got all of them, ergo we might
        // as well leave and not waste any more time
        return;
      }

      if ( !( vr_pmesh_bank.new_instance( txtmesh_fs, mesh_name, vr_pmesh::ALLOW_SELF_LIT_MATERIALS ) ) )
        error( "Couldn't create new instance of meshname in entity_maker::read_meshes" );
    }
    else
      error( "Unexpected chunk in entity_maker::read_meshes" );

    serial_in( txtmesh_fs, &cf ); // 'chunkend' for mesh
  }

  txtmesh_fs.close( );
#endif file://NGL
}


conglomerate *entity_maker::create_conglomerate( chunk_file& fs,
                                         const entity_id& id,
                                         unsigned int flags )
{
  conglomerate* ent = NULL;

  stringx fname = fs.get_filename();
  fname.to_lower();

  ent = NEW conglomerate( fs, id, ENTITY_CONGLOMERATE, flags );
  create_entity( ent );
  return ent;
}




void entity_maker::creating_widget_error( const stringx &entity_kind ) const
{
  stringx msg = "Widgets cannot be a" + entity_kind;
  error( msg );
  assert( 0 );
}



void entity_maker::destroy_entity( entity* e )
{
  if ( g_world_ptr && e->get_owning_widget() == NULL )
  {
    g_world_ptr->destroy_entity( e );
  }
  else
    delete e;
}


///////////////////////////////////////////////////////////////////////////////
// entity caching interface

entity* entity_maker::acquire_entity( const stringx& name, unsigned int flags )
{
  return entity_cache.acquire( name, flags );
}

entity* entity_maker::acquire_beam( unsigned int flags )
{
  return entity_cache.acquire_beam( flags );
}

void entity_maker::release_entity( entity* e )
{
  if ( e->get_entity_pool() != NULL )
    e->get_entity_pool()->release( e );
}

void entity_maker::purge_entity_cache()
{
  entity_cache.purge();
}



///////////////////////////////////////////////////////////////////////////////
// support for entity caching system
///////////////////////////////////////////////////////////////////////////////

entity_pool::entity_pool()
: entities(),
  avail( 0 )
{
}

entity_pool::~entity_pool()
{
/*
  entity_list::const_iterator i = entities.begin();
  entity_list::const_iterator i_end = entities.end();
  for ( ; i!=i_end; ++i )
  {
    entity *e = *i;
    if ( e )
      e->set_entity_pool(NULL);
  }
*/
}

entity* entity_pool::acquire( unsigned int flags )
{
  entity* e;
  if ( avail > 0 )
  {
    // find available (cached) entity
    entity_list::const_iterator i = entities.begin();
    entity_list::const_iterator i_end = entities.end();
    for ( ; i!=i_end; ++i )
    {
      e = *i;
      if ( e->is_ext_flagged(EFLAG_EXT_UNUSED) )
      {
        // acquire this entity
        --avail;
        e->set_ext_flag( EFLAG_EXT_UNUSED, false );
        e->acquire( flags );
        if ( e->is_a_conglomerate() )
        {
          // initialize each member with po copied from corresponding "base" member
          assert( !entities.empty() );
          entity* base = *entities.begin();
          assert( base->is_a_conglomerate() );
          vector<entity*>::const_iterator i = ((conglomerate*)e)->get_members().begin();
          vector<entity*>::const_iterator i_end = ((conglomerate*)e)->get_members().end();
          vector<entity*>::const_iterator j = ((conglomerate*)base)->get_members().begin();
          for ( ; i!=i_end; ++i,++j )
          {
            entity* new_member = *i;
            entity* base_member = *j;
            new_member->set_rel_po( base_member->get_rel_po() );
          }
        }
        e->unforce_regions();
        return e;
      }
    }
  }
  // otherwise, make a NEW instance of the base entity
  assert( !entities.empty() );
  entity* base = *entities.begin();
  flags |= EFLAG_MISC_NONSTATIC;
  e = base->make_instance( entity_id::make_unique_id(), flags );
  add( e );
  if ( e->is_a_conglomerate() )
  {
    // add each member to the appropriate entity_pool (obtained from the base copy)
    assert( base->is_a_conglomerate() );
    vector<entity*>::const_iterator i = ((conglomerate*)e)->get_members().begin();
    vector<entity*>::const_iterator i_end = ((conglomerate*)e)->get_members().end();
    vector<entity*>::const_iterator j = ((conglomerate*)base)->get_members().begin();
    for ( ; i!=i_end; ++i,++j )
    {
      entity* new_member = *i;
      entity* base_member = *j;
      assert( base_member->get_entity_pool() != NULL );
      base_member->get_entity_pool()->add( new_member );
    }
  }
  // add NEW instance to world
  switch ( e->get_flavor() )
  {
  case ENTITY_MARKER:
    g_world_ptr->add_marker( (marker*)e );
    break;
  case ENTITY_BEAM:
    g_world_ptr->add_beam( (beam*)e );
    break;
  case ENTITY_LIGHT_SOURCE:
    g_world_ptr->add_light_source( (light_source*)e );
    break;
  case ENTITY_PARTICLE_GENERATOR:
    g_world_ptr->add_particle_generator( (particle_generator*)e );
    break;
  case ENTITY_PHYSICAL:
    assert(0);
//    g_world_ptr->add_physent( (physical_entity*)e );
    break;
  default:
    g_entity_maker->create_entity( e );
    break;
  }
  return e;
}

void entity_pool::add( entity* e )
{
  assert( !e->is_ext_flagged(EFLAG_EXT_UNUSED) );
  entities.push_back( e );
  e->set_entity_pool( this );
}

#include <algorithm>

void entity_pool::release( entity* e )
{
  assert( find(entities.begin(),entities.end(),e)!=entities.end() && !e->is_ext_flagged(EFLAG_EXT_UNUSED) );
  e->set_ext_flag( EFLAG_EXT_UNUSED, true );
  e->release();
  e->force_region( NULL );
  e->set_active( false );
  ++avail;
}


entity_pool_set::entity_pool_set()
: entity_pools(),
  aux_entity_pools()
{
}

entity_pool_set::~entity_pool_set()
{
  purge();
}

entity* entity_pool_set::acquire( const stringx& name, unsigned int flags )
{
  // find an entry corresponding to the given name
  filespec fspec( name );
  fspec.name.to_upper();
  typedef entity_pool* ep_ptr_t;
  ep_ptr_t& epool = entity_pools[fspec.name];
  if ( epool == NULL )
  {
    // need to construct the base entity
    if ( fspec.path.size() > 0 )
      g_file_finder->push_path_back( fspec.path );

    entity* base = g_entity_maker->create_entity_or_subclass( name, entity_id::make_unique_id(), po_identity_matrix, empty_string, flags, NULL );

    if ( fspec.path.size() > 0 )
      g_file_finder->pop_path_back();

    epool = NEW entity_pool();
    epool->add( base );
    if ( base->is_a_conglomerate() )
    {
      // create entity_pools for all members as well
      vector<entity*>::const_iterator i = ((conglomerate*)base)->get_members().begin();
      vector<entity*>::const_iterator i_end = ((conglomerate*)base)->get_members().end();
      for ( ; i!=i_end; ++i )
      {
        entity* e = *i;
        assert( e->get_entity_pool() == NULL );
        entity_pool* ep = NEW entity_pool();
        ep->add( e );
        aux_entity_pools.push_back( ep );
      }
    }
    return base;
  }
  else
  {
    // acquire an entity from the given pool
    return epool->acquire( flags );
  }
}

entity* entity_pool_set::acquire_beam( unsigned int flags )
{
  // find an entry corresponding to the given name
  typedef entity_pool* ep_ptr_t;
  ep_ptr_t& epool = entity_pools["_BEAM"];
  if ( epool == NULL )
  {
    // need to construct the base entity
    entity* base = g_world_ptr->add_beam( entity_id::make_unique_id(), flags );
    epool = NEW entity_pool();
    epool->add( base );
    return base;
  }
  else
  {
    // acquire an entity from the given pool
    return epool->acquire( flags );
  }
}

void entity_pool_set::purge()
{
  set_t::const_iterator i = entity_pools.begin();
  set_t::const_iterator i_end = entity_pools.end();
  for ( ; i!=i_end; ++i )
    delete (*i).second;
  entity_pools.clear();
  aux_t::const_iterator ai = aux_entity_pools.begin();
  aux_t::const_iterator ai_end = aux_entity_pools.end();
  for ( ; ai!=ai_end; ++ai )
    delete *ai;
  aux_entity_pools.resize(0);
}
