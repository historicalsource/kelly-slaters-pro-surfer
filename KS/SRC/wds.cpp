//--------------------------------------------------
// WDS.CPP
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
//
// Home of the world_dynamics_system
//--------------------------------------------------

#include "global.h"

#include "project.h"
#include "wds.h"

#include "particle.h"
// BIGCULL #include "ai_cue.h"
#include "ai_interface.h"
#include "anim_maker.h"
#include "app.h"
#include "beam.h"
#include "bsp_collide.h"
#include "camera.h"
#include "capsule.h"
#include "colgeom.h"
#include "collide.h"
#include "commands.h"
#include "controller.h"
// BIGCULL #include "damage_interface.h"
#include "debug_render.h"
#include "element.h" // PEH BETA LOCK
#include "entity_maker.h"
#include "entityflags.h"
#include "fcs.h"
#include "file.h"
#include "file_finder.h"
#include "game.h"
#include "game_info.h"
#include "generator.h"
#include "geomgr.h"
#include "frustum.h"
// BIGCULL #include "gun.h"
#include "hwmath.h"
#include "hwrasterize.h"
#include "inputmgr.h"
#include "iri.h"
#include "item.h"
#include "light.h"
#include "lightmgr.h"
// BIGCULL #include "manip_obj.h"
#include "marker.h"
#include "mcs.h"
#include "mic.h"
#include "osdevopts.h"
#include "oserrmsg.h"
#include "particlecleaner.h"
#include "physical_interface.h"
#include "profiler.h"
#include "projconst.h"
#include "rect.h"
#include "render_data.h"
#include "renderflav.h"
// BIGCULL #include "scanner.h"
#include "script_lib_controller.h"
#include "signals.h"
#include "sky.h"
#include "sound_group.h"
#include "sphere.h"
// BIGCULL #include "spiderman_camera.h"
// BIGCULL #include "spiderman_controller.h"
#include "stringx.h"
#include "switch_obj.h"
#include "polytube.h"
#include "lensflare.h"
#include "terrain.h"
// BIGCULL #include "thrown_item.h"
// BIGCULL #include "melee_item.h"
#include "time_interface.h"
#include "trigger.h"
// BIGCULL #include "turret.h"
#include "vm_thread.h"
#include "box_trigger_interface.h"
#include "script_access.h"
#include "crawl_box.h"

#include "ai_polypath.h"
#include "kellyslater_main.h"

#if defined(TARGET_XBOX)
#include "wave.h"
#include "timer.h"
#include "ks/kellyslater_controller.h"
#include "FrontEndManager.h"
#endif /* TARGET_XBOX JIV DEBUG */

#ifdef TARGET_PS2
#include "archalloc.h"	// for FreeBallocMem (dc 02/05/02)
#endif

#include "ngl_support.h"

#include "ksfx.h"

#include "camera_tool.h"
#include "unlock_manager.h"

#include <math.h>
#include <numeric>
#include <string.h>
#include <algorithm>
//#include "brain.h"
//#include "dread_net.h"
//P #include "membudget.h"
//P #include "memorycontext.h"

#ifdef PROJECT_KELLYSLATER
#include "ks_camera.h"
#endif // PROJECT_KELLYSLATER


#if _CONSOLE_ENABLE
#include "console.h"
#endif
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
// BIGCULL #include "spiderman_common.h"
#endif

#ifdef TARGET_GC
#ifdef START_PROF_TIMER
#undef START_PROF_TIMER
#endif
#ifdef STOP_PROF_TIMER
#undef STOP_PROF_TIMER
#endif
#define START_PROF_TIMER(s) ((void)0)
#define STOP_PROF_TIMER(s) ((void)0)
#endif



//H extern FILE * debug_log_file;


////////////////////////////////////////////////////////////////////////////////
//  Globals
////////////////////////////////////////////////////////////////////////////////
world_dynamics_system * g_world_ptr = NULL;
vector3d up_vector(0,1,0);
bool g_frame_advance_called_this_game = false;
mouselook_controller * g_mouselook_controller;

#if defined(PROJECT_KELLYSLATER)
joypad_usercam_controller * g_ps2usercam_controller;
#endif

theta_and_psi_mcs* g_theta_and_psi_mcs;

extern profiler_timer proftimer_file_exists;
extern profiler_timer proftimer_file_open;
extern profiler_timer proftimer_file_read;

extern profiler_counter profcounter_ents;
extern profiler_counter profcounter_regions;
extern profiler_counter profcounter_active_ents;
extern profiler_counter profcounter_poss_active_ents;
extern profiler_counter profcounter_anims;

extern instance_bank<entity_track_tree> entity_track_bank;

bool g_debug_slow_ass           = false;
char g_debug_entity_id_name[32] = "GCS_WALLBREAKA";
int  g_debug_entity_id          = 3202;
int g_render_cube_map = 0;

////////////////////////////////////////////////////////////////////////////////
// File reading classes
////////////////////////////////////////////////////////////////////////////////
rational_t g_level_time;
////////////////////////////////////////////////////////////////////////////////
world_dynamics_system::world_dynamics_system()
{
	path_graph_system_construct();

	loading_from_scn_file = false;

	t_inc = T_INC_INVALID;
	cur_time_inc = T_INC_INVALID;

	for(int i=0;i<MAX_PLAYERS;i++)
	{
		hero_ptr[i] = NULL;
		ks_controller[i] = NULL;
	}

	world_path = NULL;

	usercam = NULL;
	usercam_orient_mcs = NULL;
	usercam_move_mcs = NULL;
	usercam_controller = NULL;


	scene_analyzer_move_mcs = NULL;
	scene_analyzer_orient_mcs = NULL;
	scene_analyzer_controller = NULL;
	scene_analyzer_cam = NULL;

	current_malor_marker = -1;

	origin_entity = NEW entity;
	origin_entity->set_rel_po(po_identity_matrix);

	setup_time = 4;
	g_level_time = 0;
	the_terrain = 0;

	marky_cam = 0;
	marky_cam_enabled = false;

	cur_global_so = NULL;
	cur_global_so_inst = NULL;

	last_snm_handle = 0;

	collision_dummy=NEW entity(ANONYMOUS,EFLAG_PHYSICS_MOVING|EFLAG_PHYSICS,po_identity_matrix);

	//  dread_network = NULL;
	// BIGCULL   ai_cue_mgr = NULL;

	/*
	m_pLipSync = NULL;
	m_pLipSync = NEW LipSyncMngr;
	*/


	// THE STL SUCKS FIX
	// This is to ensure that STL will free these
	// since STL doesn't free anything < 128 bytes
	collision_entities.reserve(100);
	collision_entities.resize(0);
	active_entities.reserve( 100 );
	active_entities.resize(0);

	parented_entities.reserve( 100 );
	parented_entities.resize(0);

	dead_ents.reserve( 100 );
	dead_ents.resize(0);
	ett_mgr = NEW ett_manager();

	current_light_context = NULL;
}


extern void destroy_script_lists();

world_dynamics_system::~world_dynamics_system()
{
	debug_render_done();

	// BETH: for front end purposes
	//if(FEDone())
	//{
	// unload scene animations
	scene_anim_map_t::iterator angel_of_death;
	for( angel_of_death = scene_anim_map.begin(); angel_of_death != scene_anim_map.end(); angel_of_death++ )
	{
		scene_anim* falls_under_the_scythe = (*angel_of_death).second;
		delete falls_under_the_scythe;
	}
	scene_anim_map.clear();
	//}


	//  script_manager* scriptman = get_script_manager();
	//  scriptman->check_all_objects();

	{
		vector<controller*>::const_iterator i = controllers.begin();
		vector<controller*>::const_iterator i_end = controllers.end();
		for ( ; i!=i_end; ++i )
			delete *i;
		controllers.resize(0);
	}

	{
		vector<force_generator*>::const_iterator i = generators.begin();
		vector<force_generator*>::const_iterator i_end = generators.end();
		for ( ; i!=i_end; ++i )
			delete *i;
		generators.resize(0);
	}

	{
		vector<force_control_system*>::const_iterator i = fcs_list.begin();
		vector<force_control_system*>::const_iterator i_end = fcs_list.end();
		for ( ; i!=i_end; ++i )
			delete *i;
		fcs_list.resize(0);
	}

	{
		vector<motion_control_system*>::const_iterator i = mcs_list.begin();
		vector<motion_control_system*>::const_iterator i_end = mcs_list.end();
		for ( ; i!=i_end; ++i )
			delete *i;
		mcs_list.resize(0);
	}

	{
		surfaceinfo_list_t::const_iterator i = surfaceinfo_list.begin();
		surfaceinfo_list_t::const_iterator i_end = surfaceinfo_list.end();
		for ( ; i!=i_end; ++i )
			delete (*i).second;
		surfaceinfo_list.clear();
	}

	// destroy conglomerates first!
	{
		entity_list::const_iterator i = entities.begin();
		entity_list::const_iterator i_end = entities.end();
		for ( ; i!=i_end; )
		{
			entity* e = *i;
			if ( e && e->is_a_conglomerate() && remove_entity(e) )
			{
				delete e;
				i = entities.begin();
				i_end = entities.end();
			}
			else
			{
				++i;
			}
		}
	}
	//  scriptman->check_all_objects();


	{
		entity_list::const_iterator i = entities.begin();
		entity_list::const_iterator i_end = entities.end();
		for ( ; i!=i_end; ++i )
		{
			entity* e = *i;
			if ( e )
			{
				// if this assertion fails then the entity has already been deleted
				assert(e->get_flavor() != 0x13131313);
				if ( e->is_an_item() || e->is_a_limb_body() )
					continue;
				else if ( e->is_a_light_source() )
				{
					remove_light_source( static_cast<light_source*>(e) );
				}
				else if ( remove_entity(e) )
				{
					delete e;
				}
			}
		}
		entities.resize(0);
	}

	{
		vector<item*>::const_iterator i = items.begin();
		vector<item*>::const_iterator i_end = items.end();
		for ( ; i!=i_end; ++i )
			delete *i;
		items.resize(0);
	}

	{
		crawl_box_list::const_iterator i = crawl_boxes.begin( );
		crawl_box_list::const_iterator i_end = crawl_boxes.end( );

		for( ; i != i_end; i++ ) {
			delete *i;
		}

		crawl_boxes.clear( );
	}
	//  scriptman->check_all_objects();

	// terrain must be deleted after ents and items, since they remove
	// themselves from their associated region when destroyed
	if ( the_terrain )
	{
		delete the_terrain;
	}
	delete origin_entity;

	// these lists contain duplicate pointers, owned elsewhere
	lights.resize(0);

	active_entities.clear();
	entfiles.clear();

	// clear all the path_graphs
	{
		vector<path_graph*>::iterator i = path_graph_list.begin();
		for ( ; i!=path_graph_list.end(); )
		{
			path_graph* pg = *i;
			if ( pg != NULL )
			{
				i = path_graph_list.erase( i );
				delete pg;
			}
			else
				++i;
		}
		path_graph_list.resize(0);
	}
	//  scriptman->check_all_objects();

	/*
	if ( dread_network )
	{
    delete dread_network;
    dread_network = NULL;
	}
	*/

	if (collision_dummy != NULL)
	{
		delete collision_dummy;
		collision_dummy=NULL;
	}

	entity_track_bank.purge(); // PEH BETA (including variants. sequence below)

#if 0 // BIGCULL
	if ( ai_cue_mgr )
	{
		delete ai_cue_mgr;
		ai_cue_mgr = NULL;
	}
#endif

	while( material_sets.size() )
		delete_material_set( *material_sets.begin() );

	material_sets.resize(0);
	// I put this in to fix a memory leaking hero, but it crashes....hmmm.  what to do GT-4/17/01
	// if (hero_ptr != NULL)
	//    delete hero_ptr;

	// <<<< shouldn't there be a delete hero entities stuff here?  -DL
	for(int i=0;i<MAX_PLAYERS;i++)
		hero_ptr[i] = NULL;

	// BIGCULL   g_spiderman_ptr = NULL;

	//  element_manager::inst()->purge(); // PEH BETA LOCK

	// BIGCULL  g_spiderman_ptr = NULL;
	// BIGCULL  g_spiderman_controller_ptr = NULL;
	// BIGCULL   g_spiderman_camera_ptr = NULL;

	if(world_path)
	{
		delete world_path;
		world_path = NULL;
	}

	//  scriptman->check_all_objects();
	script::destroy();

#ifdef KSCULL
	destroy_script_lists();
#endif

	/*
	if ( m_pLipSync )
	delete m_pLipSync;
	*/

	path_graph_system_destruct();

#ifdef TARGET_PS2
	// No longer needed now that we have the heap system
	// This actually causes problems with sprintf
	//
	//FreeBallocMem();
#endif

  if( ett_mgr )
  {
    delete ett_mgr;
    ett_mgr = NULL;
  }
}

void system_idle(); // in w32_main or sy_main

void world_dynamics_system::parse_scene(chunk_file& fs, const stringx& scene_root, bool loading_sin )
{
	// entities (optional)
	stringx entity_id_name, entity_name;
	stringx id, id2;
	serial_in(fs,&id);
	error_context::inst()->push_context(fs.get_name());
	error_context::inst()->push_context(id);

	// MAKE SURE TO KEEP THE ENUM, NAME ARRAY AND CHUNK READ ORDER IN SYNC !!!
	enum
	{
		MATERIAL_GROUPS_TAG = 0,
			SURFACE_TYPES_TAG,
			VARIANTS_TAG,
			ENTITIES_TAG,
			ENTITY_INFO_TAG,
			ENTITY_DETAIL_TAG,
			MAPS_TAG,
			CHAR_GROUPS_TAG,
			TRIGGERS_TAG,
			SCRIPT_INSTANCES_TAG,
			PATHS_TAG,
			//    DREAD_NET_TAG,
			REGION_AMBIENT_SOUND_TAG,
			CRAWL_BOXES_TAG,

			CHUNK_TAG_COUNT
	};

	char *chunk_name_array[] =
	{
		"material_groups:",
			"surface_types:",
			"variants:",
			"entities:",
			"entity_info:",
			"entity_detail:",
			"maps:",
			"char_groups:",
			"triggers:",
			"script_instances:",
			"paths:",
			//    "dread_net:",
			"region_ambient_sounds:",
			"crawl_boxes:",
	};

	// make sure dreadnet is created and loaded
	/*
	if(!dread_network)
	{
    dread_network = NEW dread_net();
    dread_network->read_cue_file("av_cues");
	}
	*/

	// make sure dreadnet is created and loaded
#if 0 // BIGCULL
	if(!ai_cue_mgr)
	{
		ai_cue_mgr = NEW ai_cue_manager();
	}
#endif // BIGCULL

	// material groups
	if( id == chunk_name_array[ MATERIAL_GROUPS_TAG ] )
	{
		for( serial_in(fs, &id); id != chunkend_label; serial_in(fs, &id) )
		{
			add_material_set( id );
		}
		serial_in(fs,&id);
	}

	system_idle();

	if( id== chunk_name_array[ SURFACE_TYPES_TAG ] )
	{
		for( serial_in( fs, &id ); id!=chunkend_label && id!=chunkend_label; serial_in( fs, &id ) )
		{
			surface_type_info *typeinfopt;
			int type;
			stringx readstring;

			type = atoi( id.c_str() );

			if( type < 0 || type > 7 )
			{
				error("surface_type %d out of range (0 to 7)", type);
			}

			typeinfopt = surfaceinfo_list[ type ];

			if( !typeinfopt )
			{
				typeinfopt = NEW surface_type_info;
				surfaceinfo_list[ type ] = typeinfopt;
			}

			typeinfopt->sound_name = empty_string;
			typeinfopt->effect_name = empty_string;
			typeinfopt->effect_duration = 0.0f;

			serial_in( fs, &typeinfopt->sound_name );

			serial_in( fs, &readstring );
#ifdef GCCULL
			if ( readstring == "sound_groups:" )
				serial_in( fs, typeinfopt->sound_groups );
#endif

			serial_in( fs, &readstring );

			if( readstring == chunkend_label || readstring == chunkend_label )
				continue;

			typeinfopt->effect_name = readstring;

			// preload effects
			create_preloaded_entity_or_subclass(  typeinfopt->effect_name.c_str(),  "fx\\" );

			serial_in( fs, &typeinfopt->effect_duration );
			serial_in( fs, &readstring );

			assert( readstring == chunkend_label || readstring == chunkend_label );
		}

		serial_in(fs,&id);
	}

	system_idle();

	// variants
	if( id == chunk_name_array[ VARIANTS_TAG ] )
	{
		STUBBED(wds_parse_scene_VARIANTS_TAG, "wds::parse_scene VARIANTS_TAG");
		bool doit = true;
		while(doit)
		{
			serial_in( fs, &id );

			for(int i=0; doit && i<CHUNK_TAG_COUNT; ++i)
			{
				if(id.length() <= 0 || id == chunk_name_array[i])
					doit = false;
			}
		}
	}
	system_idle();

	if (id==chunk_name_array[ENTITIES_TAG])
	{
		// NEW chunky-style entities:
		chunk_flavor cf;
		for(;;)
		{
			serial_in( fs, &cf );
			if( cf == chunk_flavor("entity") )
			{
				color32   in_color( color32_white );
				stringx   in_material_filename;
				bool      set_material_flag = false;
				serial_in(fs,&entity_id_name);
				entity_id_name.to_upper();
#ifdef DEBUG
				if( entity_id_name == stringx( g_debug_entity_id_name ) )
				{
					debug_print( "Loaded entity %s", g_debug_entity_id_name );
				}
#endif
				serial_in(fs,&entity_name);
				entity_name.to_upper();
				error_context::inst()->pop_context();
				error_context::inst()->push_context(entity_id_name);
				// look for reserved name identifying special entity
				bool is_light = false;
				if ( entity_name == "_OMNI_LIGHT" ||
					entity_name == "_DIR_LIGHT" )
					is_light = true;
				bool is_trigger = false;
				if( entity_name == "BOX_TRIGGER" ) {
					is_trigger = true;
				}
				// chunks within chunks
				po loc;
				light_properties lprops;
				convex_box binfo;
				int mindetaillvl=-1;
				int active_status;
				//list<region_node*> forced_regions;
				region_node_list forced_regions;
				entity* me = NULL;
				chunk_flavor cf;
				for(;;)
				{
					serial_in( fs, &cf );
					if( cf == chunk_flavor("pos") )
					{
						vector3d pos;
						serial_in( fs, &pos );
						loc = po_identity_matrix;
						loc.set_position( pos );
					}
					else if ( cf == chunk_flavor("lookat") )
					{
						// read point for entity to rotate facing toward;
						// NOTE: this only works correctly if the "pos" chunk came first
						vector3d lookat;
						serial_in( fs, &lookat );
						loc.set_facing( lookat );
					}
					else if ( cf == chunk_flavor("po") )
					{
						serial_in( fs, &loc );
					}
					else if ( cf == chunk_flavor("light") )
					{
						serial_in( fs, &lprops );
#ifdef TARGET_XBOX
// On Xbox, we override the lighting values from the scene.  The PS2 values aren't appropriate for
// Xbox, because of differences in gamma and saturation.  To be fixed in the exporter next year.
// (dc 07/11/02)
						if (lprops.get_flavor() == LIGHT_FLAVOR_DIRECTIONAL)
						{
							float dir_reduction;
							if (!FEDone())
							{
								float dir_reduction_fe = .75f;
								dir_reduction = dir_reduction_fe;
							}
							else
							{
								int curbeach=g_game_ptr->get_beach_id();
								assert (curbeach >= 0 && curbeach < BEACH_LAST);
								dir_reduction = BeachDataArray[curbeach].dir_reduction;
							}
							color dir_reduced(
								lprops.get_color().r * dir_reduction,
								lprops.get_color().g * dir_reduction,
								lprops.get_color().b * dir_reduction,
								1
							);
							lprops.set_color(dir_reduced);
						}
#endif
					}
					else if ( cf == chunk_flavor("status") )
					{
						serial_in( fs, &active_status );
					}
					else if ( cf == chunk_flavor("forcergn") )
					{
						// get region name)
						stringx region_name;
						serial_in( fs, &region_name );
						// find region
						region_node* fr = the_terrain->find_region( region_name );
						if ( !fr )
							error( ("Entity force_region: unknown region: " + region_name).data() );
						// add to forced-region list
						forced_regions.push_back( fr );
					}
					else if ( cf == chunk_flavor("mndetail") )
					{
						serial_in( fs, &mindetaillvl );
					}
					else if( cf == chunk_flavor("color") )
					{
						int itmp[4];
						for( int i = 0; i < 4; i++ ) serial_in( fs, &itmp[i] );
						in_color = color32( itmp[0], itmp[1], itmp[2], itmp[3] );
					}
					else if( cf == chunk_flavor("material") )
					{
						serial_in( fs, &in_material_filename );
						add_material_set( in_material_filename );
						set_material_flag = true;
					}
					else if( cf == chunk_flavor( "trigger" ) )
					{

						//#pragma fixme( "replace this with a serial_in( fs, trigger_props* ) or some such. -mkv 4/6/01" )
						for( serial_in( fs, &cf ); cf != CHUNK_END; serial_in( fs, &cf ) )
						{

							if( cf == chunk_flavor( "convxbox" ) )
							{
								serial_in( fs, &binfo );
							}
							else
							{
								// parse more later
							}

						}

					}
					else if ( cf == CHUNK_END )
					{
						// break out on ending chunk signifies that instance parsing is completed
						break;
					}
					else
					{
						// break out on non-ending chunk signifies that instance parsing is not yet completed
						break;
					}
				}
				if ( is_light )
				{
					me = add_light_source( entity_id::make_entity_id(entity_id_name.c_str()), loc, lprops, &forced_regions, active_status );
				}
				else if( is_trigger )
				{
					me = add_box_trigger( entity_id::make_entity_id( entity_id_name.c_str( ) ), loc, binfo, &forced_regions, active_status );
				}
				else
				{
					const char *s = entity_id_name.c_str();
					entity_id ei( s );
					//P          int alloc0 = memtrack::get_total_alloced();
					//P          int script_mem = membudget()->get_usage( membudget_t::SCRIPTS );
					//P          int anim_mem = membudget()->get_usage( membudget_t::ANIMS );
					//P          g_memory_context.push_context( "ENTS" );

					//stringx mesh_path = scene_root + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
					stringx mesh_path = scene_root + "entities\\";
					nglSetMeshPath( (char *)mesh_path.c_str() );
					stringx texture_path = scene_root + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
					nglSetTexturePath( (char *)texture_path.c_str() );


					me = g_entity_maker->create_entity_or_subclass( entity_name, ei, loc, scene_root, active_status, &forced_regions );

					if( me && me->get_mesh() && me->get_mesh()->Flags & NGL_LIGHTCAT_MASK) me->create_light_set();

					if( me && !FEDone()) me->compute_sector(*the_terrain);

					// track entity memory
					//P          g_memory_context.pop_context();
					//P          script_mem = membudget()->get_usage( membudget_t::SCRIPTS ) - script_mem;
					//P          anim_mem = membudget()->get_usage( membudget_t::ANIMS ) - anim_mem;
					//P          int non_entity_mem = script_mem + anim_mem;

					//P          membudget()->use( membudget_t::SCENE, memtrack::get_total_alloced()-alloc0-non_entity_mem );

					if ( me && mindetaillvl>=0)
						me->set_min_detail(mindetaillvl);
				}
				// now that the entity has been constructed from source file or base copy,
				// continue instance parsing if necessary
				stringx cfs = cf.c_str();
				for ( ; cfs!=chunkend_label; serial_in(fs,&cf) )
				{
					if ( !me->parse_instance( cf.to_stringx(), fs ) )
						error( stringx(entity_id_name) + ": unrecognized instance chunk: " + cf.to_stringx() );
				}

				if (me)
				{
					me->set_from_sin_file(loading_sin);
					if( set_material_flag ) me->set_alternative_materials( in_material_filename );
					me->set_render_color( in_color );
				}
      }
      else if( cf == CHUNK_END )
      {
		  break;
      }
      else
      {
		  stringx composite = stringx("Unknown chunk ")+cf.c_str()
			  +(" in entity list.");
		  error(composite.c_str() );
      }
    }
    serial_in(fs,&id);
    error_context::inst()->pop_context();
    error_context::inst()->pop_context();
  }

  system_idle();

  if ( id == chunk_name_array[ENTITY_INFO_TAG]  )
  {
	  // read maps
	  stringx label;
	  for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
	  {
		  entity *ent = get_entity( label );

		  if ( ent )
		  {
			  stringx cf;
			  for ( serial_in(fs,&cf); cf!=chunkend_label; serial_in(fs,&cf) )
			  {
				  if(!ent->parse_instance(cf, fs))
					  error( fs.get_name() + ": unknown instance chunk '"+stringx(cf.c_str())+"' for entity '"+label+"'" );
			  }

			  ent->compute_sector( *the_terrain, loading_sin );
		  }
		  else
			  error( fs.get_name() + ": unknown entity '"+label+"' in entity_specs section" );
	  }

	  serial_in( fs, &id );
  }

  if ( id == chunk_name_array[ENTITY_DETAIL_TAG]  )
  {
	  // read maps
	  stringx label;
	  for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
	  {
		  if (label != stringx("detail"))
			  error("unknown chunk found in entity_detail section");
		  int mindetaillvl=0;
		  serial_in(fs,&mindetaillvl);
		  assert(mindetaillvl>=0 && mindetaillvl<=2);
		  for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
		  {
			  entity *ent = get_entity( label );

			  if ( ent )
			  {
				  ent->set_min_detail(mindetaillvl);
			  }
			  else
				  error( fs.get_name() + ": unknown entity '"+label+"' in entity_detail section" );
		  }
	  }

	  serial_in( fs, &id );
  }

  system_idle();

  // data for auto-mapping (optional)
  if ( id == chunk_name_array[ MAPS_TAG ] )
  {
	  STUBBED(wds_parse_scene_MAPS_TAG, "wds::parse_scene MAPS_TAG");
	  bool doit = true;
	  while(doit)
	  {
		  serial_in( fs, &id );

		  for(int i=0; doit && i<CHUNK_TAG_COUNT; ++i)
		  {
			  if(id.length() <= 0 || id == chunk_name_array[i])
				  doit = false;
		  }
	  }
  }

  // character groups (optional)
  if (id == chunk_name_array[ CHAR_GROUPS_TAG ] )
  {
	  STUBBED(wds_parse_scene_CHAR_GROUPS_TAG, "wds::parse_scene CHAR_GROUPS_TAG");
	  bool doit = true;
	  while(doit)
	  {
		  serial_in( fs, &id );

		  for(int i=0; doit && i<CHUNK_TAG_COUNT; ++i)
		  {
			  if(id.length() <= 0 || id == chunk_name_array[i])
				  doit = false;
		  }
	  }
  }

  system_idle();

  // triggers (optional)
  if (id == chunk_name_array[ TRIGGERS_TAG ] )
  {
	  stringx str, type;
	  serial_in(fs, &str);
	  while ( str != chunkend_label )
	  {
		  serial_in(fs, &type);
		  trigger *t = trigger_manager::inst()->new_trigger(str, type, fs);
		  serial_in(fs, &str);
		  // possibility of a forcergn chunk here
		  if (str == "forcergn")
		  {
			  do
			  {
				  serial_in(fs, &str);
				  if ( str == chunkend_label ) break;
				  t->force_region( str );
			  } while (1);

			  serial_in(fs, &str);
		  }
		  if (str == "use_any_char")
		  {
			  t->set_use_any_char(true);
			  serial_in(fs, &str);
		  }
	  }

	  serial_in(fs,&id);
  }

  system_idle();

  // script object instances (optional)
  if ( id == chunk_name_array[ SCRIPT_INSTANCES_TAG ] )
  {
	  for ( serial_in( fs, &id );
	  id!=chunkend_label && id!=chunkend_label;
	  serial_in( fs, &id ) )
	  {
		  // read script object type name
		  stringx type;
		  serial_in(fs,&type);
		  // find script object
		  script_object* so = scriptman.find_object(type);
		  if (!so)
		  {
			  stringx err;
			  err="script object type "+type+" not found\n";
			  error(err.c_str());
		  }
		  // create script object instance
		  //P      int before = memtrack::get_total_alloced();
		  //P      g_memory_context.push_context( "SCRIPTS" );
		  script_object::instance* inst = so->add_instance(id, &fs);
		  inst->get_name(); // phony fn call to get gnu to stop warning about this being unused.
		  //P      g_memory_context.pop_context();
		  //P     membudget()->use( membudget_t::SCRIPTS, memtrack::get_total_alloced()-before );
	  }
	  serial_in(fs,&id);
  }

  system_idle();

  // AI paths
  if( id == chunk_name_array[ PATHS_TAG ] )
  {
	  for( serial_in(fs, &id); id != chunkend_label; serial_in(fs, &id) )
	  {
		  if(id == "polypath" || id == "POLYPATH")
		  {
			  chunk_file polyfile;
			  stringx name;
			  serial_in(fs, &name);

			  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
				  name = g_file_finder->find_file( name, ".path" );
			  else
				  name = name + ".path";

			  if(world_path != NULL)
				  error("Only one polypath allowed per level!");

			  world_path = NEW ai_polypath();
			  polyfile.open(name);
			  serial_in(polyfile, world_path);
			  polyfile.close();

			  world_path->optimize();
		  }
		  else if(id == "assign:" || id == "ASSIGN:")
		  {
			  for( serial_in(fs, &id); id != chunkend_label; serial_in(fs, &id) )
			  {
				  id.to_upper();

				  entity *ent = get_entity(id);
				  if(ent)
				  {
					  if(ent != get_hero_ptr(g_game_ptr->get_active_player()))
					  {
						  if(ent->has_ai_ifc())
						  {
							  serial_in(fs, &id);
							  id.to_upper();

							  path_graph *graph = get_path_graph(id);
							  if(graph)
							  {
								  ent->ai_ifc()->set_current_path_graph(graph);
							  }
							  else
								  error("Path '%s' does not exist!", id.c_str());
						  }
						  else
							  error("Character '%s' does not have an AI!", id.c_str());
					  }
					  else
						  error("Cannot assign paths to the hero!");
				  }
				  else
					  error("Entity '%s' does not exist!", id.c_str());
			  }
		  }
		  else
		  {
			  path_graph *graph = NEW path_graph();
			  graph->read_data(fs, id);
			  add_path_graph(graph);
		  }
	  }
	  serial_in(fs,&id);
  }

  system_idle();

  /*
  if( id == chunk_name_array[ DREAD_NET_TAG ])
  {
  dread_network->read_data(fs, id);
  serial_in(fs,&id);
  }
  */
  // region-based ambient sound streams
  if( id == chunk_name_array[ REGION_AMBIENT_SOUND_TAG ] )
  {
	  for( serial_in(fs, &id); id != chunkend_label; serial_in(fs, &id) )
	  {
		  rational_t rt;
		  serial_in(fs, &id2);
		  serial_in( fs, &rt );
		  add_region_ambient_sound( id, id2, rt );
	  }
	  serial_in(fs,&id);
  }

  system_idle();

  if( id == chunk_name_array[ CRAWL_BOXES_TAG ] ) {
	  chunk_flavor cb;

	  for( serial_in( fs, &cb ); cb != CHUNK_END; serial_in( fs, &cb ) ) {

		  if( cb != chunk_flavor( "crawlbox" ) ) {
			  error( "unknown chunk '" + cb.to_stringx() + "' while parsing scene" );
		  }

		  chunk_flavor bf;
		  convex_box binfo;
		  int type = 0;
		  vector3d where;
		  bool forced = false;

		  for( serial_in( fs, &bf ); bf != CHUNK_END; serial_in( fs, &bf ) ) {

			  if( bf == chunk_flavor( "convxbox" ) ) {
				  serial_in( fs, &binfo );
			  } else if( bf == chunk_flavor( "type" ) ) {
				  serial_in( fs, &type );
			  } else if( bf == chunk_flavor( "pos" ) ) {
				  serial_in( fs, &where );
			  } else if( bf == chunk_flavor( "forced" ) ) {
				  serial_in( fs, &forced );
			  } else {
				  error( "unknown chunk '" + cb.to_stringx() + "' while parsing scene" );
			  }

		  }

		  add_crawl_box( type, forced, where, binfo );
	  }

	  serial_in( fs, &id );
  }

  if( loading_sin && id.size() )
  {
	  while( id == chunkend_label )
		  serial_in(fs,&id);

	  if( id.size() )
	  {
		  for( int cn = 0; cn < CHUNK_TAG_COUNT; ++cn )
		  {
			  if( id == chunk_name_array[cn] )
			  {

				  stringx msg = "Finished reading SIN file. Skipping chunk '" + id + "' and everything thereafter.";

				  msg += "\n\nExpected chunk order is:\n";
				  for( int cn1 = 0; cn1 < CHUNK_TAG_COUNT; ++cn1 )
				  {
					  msg += "   ";
					  msg += chunk_name_array[cn1];
					  msg += "\n";
				  }

				  warning( msg );
			  }
		  }
	  }
  }
}

#if defined(TARGET_PS2)
#include <sifdev.h>
#endif /* TARGET_PS2 JIV DEBUG */

void* KSMemAllocate( u_int Size, u_int Align, const char *file, int line );

bool world_dynamics_system::wds_readfile( const char *name, unsigned char **buf, unsigned int *len, int alignment, int extra_bytes )
{
	static char Work[256];

	strcpy( Work, "");
	strcat( Work, name );

	stringx work=Work;
	stringx ext="";

	if ( wds_exists(work,ext) )
	{
		app_file ChunkU;
		stash_index_entry *hdr;

		if ( !stash::is_stash_open() || !ChunkU.get_memory_image( Work, *buf, *len, hdr, alignment ) )
		{
			app_file ChunkU;

			wds_open(ChunkU,work,ext);

			*len = ChunkU.get_size();
			if ( *len )
			{
#ifdef TARGET_PS2
				// ps2 only reads in 32-byte chunks, rounding down, so we need to read in the extra on our own.
				*buf = (u_char*)KSMemAllocate( ( *len + 31 + extra_bytes ) & ~31, alignment, name, 99999  );
				ChunkU.read( *buf, ( *len + 31 ) & ~31 );
#else
				*buf = (u_char*)KSMemAllocate( *len + extra_bytes, alignment, name, 99999  );
				ChunkU.read( *buf, *len );
#endif
			}
			ChunkU.close();
		}

		return (*len)>0;
	}
	return false;
}

// This is designed to use a preallocated buffer.
bool world_dynamics_system::wds_readfile_prealloc( const char *name, unsigned char **buf, unsigned int *len, int alignment, int extra_bytes, int max_len )
{
	static char Work[256];

#if defined(TARGET_XBOX)
	strcpy( Work, "");
#else

	//  if ( 1 ) //strncmp( name, nglHostPrefix, strlen(nglHostPrefix) ) == 0 )
    strcpy( Work, "" );
	//  else
	//    strcpy( Work, nglHostPrefix );
#endif /* TARGET_XBOX JIV DEBUG */
	strcat( Work, name );

#if 1
	stringx work=Work;
	stringx ext="";

	if ( wds_exists(work,ext) )
	{
#if 1
		int size;
		char tmpbuf[32];

		app_file ChunkU;
		stash_index_entry *hdr;

		if ( !stash::is_stash_open() )
		{
			app_file ChunkU;

			wds_open(ChunkU,work,ext);

			*len = ChunkU.get_size();

			if ( *len )
			{
				if (*len > (u_int) max_len)
					return false;
				size = (*len / 32)*32;
				ChunkU.read( *buf, size );

				assert ((*len - size) < 32);
				ChunkU.read( tmpbuf, 32 );
				memcpy(*buf + size, tmpbuf, *len - size);			}
			ChunkU.close();
		}
		else if ( !ChunkU.get_memory_image( Work, *buf, *len, hdr, alignment ))
		{
			app_file ChunkU;
			int size;
			char tmpbuf[32];
			wds_open(ChunkU,work,ext);

			*len = ChunkU.get_size();

			if ( *len )
			{
				// ps2 only reads in 32-byte chunks, rounding down, so we need to read in the extra on our own.
				if (*len > (u_int) max_len)
					return false;
				size = (*len / 32)*32;
				ChunkU.read( *buf, size );

				assert ((*len - size) < 32);
				ChunkU.read( tmpbuf, 32 );
				memcpy(*buf + size, tmpbuf, *len - size);
			}
			ChunkU.close();
		}

		return (*len)>0;
#else
		app_file ChunkU;
		int size;
		char tmpbuf[32];

		wds_open(ChunkU,work,ext);

		*len = ChunkU.get_size();
		if ( *len )
		{
			// ps2 only reads in 32-byte chunks, rounding down, so we need to read in the extra on our own.
			if (*len > max_len)
				return false;
			size = (*len / 32)*32;
			ChunkU.read( *buf, size );

			assert ((*len - size) < 32);
			ChunkU.read( tmpbuf, 32 );
			memcpy(*buf + size, tmpbuf, *len - size);
		}
		ChunkU.close();

		return (*len)>0;
#endif
	}
	return false;

#else
	int size;
	char tmpbuf[32];

	int fd = sceOpen( Work, SCE_RDONLY );
	if ( fd < 0 )
		return false;

	*len = sceLseek( fd, 0, SCE_SEEK_END );
	sceLseek( fd, 0, SCE_SEEK_SET );

	// ps2 only reads in 32-byte chunks, rounding down, so we need to read in the extra on our own.
	if (*len > max_len)
		return false;
	size = (*len / 32)*32;
	sceRead( fd, *buf, size );

	assert ((*len - size) < 32);
	sceRead( fd, tmpbuf, 32 );
	memcpy(*buf + size, tmpbuf, *len - size);

	sceClose( fd );
#endif

	return true;

}

void KSMemFree( void* Ptr );

bool world_dynamics_system::wds_releasefile( unsigned char **buf )
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
		KSMemFree(*buf);
	*buf=NULL;

	return true;
}


stringx world_dynamics_system::wds_open( chunk_file& fs, const stringx& filename, const stringx& extension, int io_flags )
{
	stringx fname;
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,(filename+extension).c_str());
#endif
	if ( file_finder_exists( filename, extension, &fname ) )
		//if ( wds_exists( filename, extension ) )
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

stringx world_dynamics_system::wds_open( app_file& fs, const stringx& filename, const stringx& extension, int io_flags )
{
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,(filename+extension).c_str());
#endif
	stringx fname;
	if ( file_finder_exists( filename, extension, &fname ) )
		//if ( wds_exists( filename, extension ) )
	{
		fs.open( fname.c_str(), io_flags );
		return fname;
	}
	else
	{
		error( "Couldn't open \"" + filename + extension + "\"" );
		return stringx();
	}
}

bool world_dynamics_system::wds_exists( const stringx& filename, const stringx& extension, int io_flags )
{
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,(filename+extension).c_str());
	//strcpy(damnopaquestringclass,filename.c_str());
#endif
	stringx fname;
	if ( file_finder_exists( filename, extension, &fname ) )
	{
		return true;
	}
	else
	{
#ifdef EVAN
		nglPrintf("unable to open %s\n",damnopaquestringclass);
#endif
		return false;
	}
}


int g_debug_num_loads = 0;
int g_debug_stop_on_load = 16;

//extern rational_t g_max_character_cam_dist;
void world_dynamics_system::load_scene( const stringx& scene_path, const stringx& hero_filename )
{
	switch(g_game_ptr->current_loading_state)
	{
	case game::LOADING_SCENE:
		{

			g_debug_num_loads++;
			if( g_debug_num_loads==g_debug_stop_on_load )
			{
				debug_print("Rendering entity");
			}

			//P  int before;
			chunk_file fs;
			g_frame_advance_called_this_game = false;

			// Max Steel version supports level subdirs
			stringx scene_root;
			stringx scene_filename;

			// expect single load from subdirectory of "levels" directory (dc 06/21/01)
			scene_root = scene_path + "\\";
			scene_filename = scene_path;

			// check the scene path before the global paths

			stringx beach_name = g_game_ptr->get_beach_name();
			stringx scene_fname="levels\\"+beach_name+"\\"+beach_name;
			loading_from_scn_file = true;
			wds_open( fs, scene_fname, stringx(".scn") );

			stringx id,terrain_fname;
			stringx where;
			g_file_finder->push_path_front( scene_root );

#if defined(PROFILE_ENTITY_MEMORY)
			where=os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) + "\\eout.txt";
			eout.open(where.c_str(), ios::out | ios::app);
			eout << "--------" << endl;
			eout.close();
#endif
#if defined(PROFILE_CHARACTER_MEMORY)
			where=os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) + "\\chout.txt";
			chout.open(where.c_str(), ios::out | ios::app);
			chout << "--------" << endl;
			chout.close();
#endif
#if defined(PROFILE_PHYSENT_MEMORY)
			where=os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) + "\\peout.txt";
			peout.open(where.c_str(), ios::out | ios::app);
			peout << "--------" << endl;
			peout.close();
#endif

			serial_in(fs,&id);
			assert(id=="terrain:");

			serial_in(fs,&terrain_fname);
			//P  before = memtrack::get_total_alloced();

			system_idle();


			nglSetMeshPath( (char *)scene_root.c_str() );
			stringx texture_path = scene_root + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
			nglSetTexturePath( (char *)texture_path.c_str() );
			//P  g_memory_context.push_context( "TERRAIN" );
			the_terrain = NEW terrain(scene_root + terrain_fname);
			/*P
			g_memory_context.pop_context();
			membudget()->use( membudget_t::TERRAIN, memtrack::get_total_alloced()-before );
			P*/

			gravity_generator * f = NEW gravity_generator();
			add_generator(f);

			// script executable file (optional)
			scriptman.clear();  // make sure we start fresh for NEW scene

			//P  before = memtrack::get_total_alloced();
			//P  g_memory_context.push_context( "SCRIPTS" );
			serial_in(fs,&id);
			g_sxfname = "no script file";
			if (id == "script_file:")
			{
				serial_in(fs,&g_sxfname);

				// NOTE: this merely loads the script executable file;
				// link must be performed before any threads are executed (see below)
				scriptman.load( (scene_root + g_sxfname).c_str());
			}

			//P  g_memory_context.pop_context();
			//P  membudget()->use( membudget_t::SCRIPTS, memtrack::get_total_alloced()-before );

			parse_scene( fs, scene_root, false );

			loading_from_scn_file = false;

			system_idle();

			// setup normal camera
			marky_camera* new_camera = NEW marky_camera(entity_id::make_entity_id("MARKY_CAM"));
			new_camera->set_flag( EFLAG_MISC_NONSTATIC, true );
			marky_cam = new_camera;
			add_camera( new_camera );

			chunk_file sin;
			//stringx scene_fname="levels\\"+level_name+"\\"+level_name;
			if ( file_finder_exists( scene_fname, stringx(".sin") ) )
			{
				wds_open( sin, scene_fname, ".sin", os_file::FILE_READ|chunk_file::FILE_TEXT );
				parse_scene( sin, scene_root, true );
			}
			// this creates sorted lists of entities in each region according to their
			// bounding box info
			get_the_terrain().sort_entities_within_each_region();

			system_idle();

	} // loading state LOADING_SCENE
	break;

	case game::LOADING_HERO_1_STASH:
	case game::LOADING_HERO_1_AUX_STASH:
	case game::LOADING_HERO_1_REST:
		{
			// Load player avatars.
			load_hero( (device_id_t)( (int)JOYSTICK1_DEVICE + g_game_ptr->get_player_device( 0 ) ), 0);     // hero 0
		}
		break;

	case game::LOADING_HERO_2_STASH:
	case game::LOADING_HERO_2_AUX_STASH:
	case game::LOADING_HERO_2_REST:
		{
			if (g_game_ptr->get_num_players() == 2)
			{
				load_hero( (device_id_t)( (int)JOYSTICK1_DEVICE + g_game_ptr->get_player_device( 1 ) ), 1);     // hero 1
				/*
				if (g_game_ptr->get_num_active_players() == 1)
				ks_controller[1]->set_active(false);
				else
				ks_controller[1]->set_active(true);
				*/
			}
		}
		break;

		/*if (g_game_ptr->get_num_ai_players())
		{
		int hero = g_game_ptr->GetSurferIdx(0);
		int my_index = random(9);
		while((my_index == hero) || (my_index == 2))
		my_index = random(9);
		load_ai_hero(AI_JOYSTICK, my_index,0);
		}
		*/

	case game::LOADING_SCENE_END:
		{
			if (g_game_ptr->get_num_active_players() == 1)
				g_game_ptr->set_active_player(0);

			setup_cameras();

			debug_render_init();

			nglPrintf("PS2 Load scene end\n");

			// file profiling
#ifdef TARGET_PC
			if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
			{
				host_system_file_handle outfile = host_fopen( "startupprof.txt", HOST_WRITE );
				host_fprintf( outfile, "File Exists:  %6.2f\n", proftimer_file_exists.get_running_total() );
				host_fprintf( outfile, "File Open:    %6.2f\n", proftimer_file_open.get_running_total() );
				host_fprintf( outfile, "File Read:    %6.2f\n", proftimer_file_read.get_running_total() );

				host_fclose( outfile );
			}
#endif

		} // loading state LOADING_SCENE_END
		break;
	}
}

// BETH
void world_dynamics_system::load_fe_scene(const stringx& scene_path, const stringx& filename)
{
	g_debug_num_loads++;
	if( g_debug_num_loads==g_debug_stop_on_load )
	{
		debug_print("Rendering entity");
	}

	//P  int before;
	chunk_file fs;

	// Max Steel version supports level subdirs
	stringx scene_root;
	//  stringx scene_filename;

	// expect single load from subdirectory of "levels" directory (dc 06/21/01)
	scene_root = scene_path + "\\";
	// scene_filename = scene_path;

	// check the scene path before the global paths

	//  stringx level_name = g_game_ptr->get_level_name();
	stringx scene_fname="levels\\frontend\\"+filename;
	loading_from_scn_file = true;
	wds_open( fs, scene_fname, stringx(".scn") );

	stringx id,terrain_fname;
	stringx where;
	g_file_finder->push_path_front( scene_root );

#if defined(PROFILE_ENTITY_MEMORY)
	where=os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) + "\\eout.txt";
	eout.open(where.c_str(), ios::out | ios::app);
	eout << "--------" << endl;
	eout.close();
#endif
#if defined(PROFILE_CHARACTER_MEMORY)
	where=os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) + "\\chout.txt";
	chout.open(where.c_str(), ios::out | ios::app);
	chout << "--------" << endl;
	chout.close();
#endif
#if defined(PROFILE_PHYSENT_MEMORY)
	where=os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) + "\\peout.txt";
	peout.open(where.c_str(), ios::out | ios::app);
	peout << "--------" << endl;
	peout.close();
#endif

	serial_in(fs,&id);
	assert(id=="terrain:");
	serial_in(fs,&terrain_fname);

	system_idle();


	nglSetMeshPath( (char *)scene_root.c_str() );
	stringx texture_path = scene_root + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath( (char *)texture_path.c_str() );

	the_terrain = NEW terrain(scene_root + terrain_fname);

	serial_in(fs,&id);
	g_sxfname = "no script file";
	if (id == "script_file:")
	{
		serial_in(fs,&g_sxfname);

		// NOTE: this merely loads the script executable file;
		// link must be performed before any threads are executed (see below)
		scriptman.load( (scene_root + g_sxfname).c_str());
	}

	parse_scene( fs, scene_root, false );

	loading_from_scn_file = false;

	system_idle();

	chunk_file sin;
	//stringx scene_fname="levels\\"+level_name+"\\"+level_name;
	if ( file_finder_exists( scene_fname, stringx(".sin") ) )
	{
		wds_open( sin, scene_fname, ".sin", os_file::FILE_READ|chunk_file::FILE_TEXT );
		parse_scene( sin, scene_root, true );
	}
	get_the_terrain().sort_entities_within_each_region();

	system_idle();

	// setup normal camera
	marky_camera* new_camera = NEW marky_camera(entity_id::make_entity_id("MARKY_CAM"));
	new_camera->set_flag( EFLAG_MISC_NONSTATIC, true );
	marky_cam = new_camera;
	add_camera( new_camera );

	// setup user camera
	{
		camera* new_camera = NEW camera(NULL,entity_id::make_entity_id("USER_CAM"));
		new_camera->set_flag( EFLAG_MISC_NONSTATIC, true );
		new_camera->set_rel_position(vector3d(0, 0, 0));
		add_camera( new_camera );
		theta_and_psi_mcs* xmcs = NEW theta_and_psi_mcs( new_camera, 0, 0 );
		g_theta_and_psi_mcs= xmcs;
		add_mcs( xmcs );
		dolly_and_strafe_mcs* dasmcs = NEW dolly_and_strafe_mcs( new_camera );
		add_mcs( dasmcs );

		joypad_usercam_controller * jc = NEW joypad_usercam_controller(dasmcs,xmcs);
		g_ps2usercam_controller = jc;
		add_controller( jc );
		usercam_controller = jc;

		usercam_move_mcs = xmcs;
		usercam_orient_mcs = dasmcs;
		usercam = new_camera;
	}

	nglPrintf("PS2 Load FE scene end\n");

	// file profiling
#ifdef TARGET_PC
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
	{
		host_system_file_handle outfile = host_fopen( "startupprof.txt", HOST_WRITE );
		host_fprintf( outfile, "File Exists:  %6.2f\n", proftimer_file_exists.get_running_total() );
		host_fprintf( outfile, "File Open:    %6.2f\n", proftimer_file_open.get_running_total() );
		host_fprintf( outfile, "File Read:    %6.2f\n", proftimer_file_read.get_running_total() );

		host_fclose( outfile );
	}
#endif
}

void world_dynamics_system::start_usercam()
{
/*	Set usercam equal to current cam.  The usercam orientation gets reset by usercam_move_mcs,
so we have to mess with the state of that object.  (dc 04/02/02)
	*/
	camera *prevcam = g_game_ptr->get_player_camera(0);
	usercam->set_rel_po(prevcam->get_rel_po());
	const po usercam_po = usercam->get_rel_po();
	float theta = fast_acos(usercam_po.get_matrix()[0][0]);
	if (usercam_po.get_matrix()[0][2] < 0) theta = -theta;
	float psi = fast_acos(usercam_po.get_matrix()[1][1]);
	if (usercam_po.get_matrix()[2][1] < 0) psi = -psi;
	usercam_move_mcs->set_pan_for_next_frame(theta);
	usercam_move_mcs->set_tilt_for_next_frame(psi);
	usercam_move_mcs->frame_advance(0);
	usercam_move_mcs->set_pan_for_next_frame(0);
	usercam_move_mcs->set_tilt_for_next_frame(0);
}


extern struct part_debug PartDebug;

// This gets called after the primary level data has already been loaded;
// this division is necessary for the ability to load_entire_state();
// see game.cpp and app.cpp

//================================================================================
// BIG CHANGES
// This procedure is now called many times to support "async" loading
// g_game_ptr->current_loading_state        The state
// g_game_ptr->current_loading_sub_state    The substate.. -1 when done with a state, >=0 else.
//                                          it is automatically incemented elsewhere.  == 0 on first run
//
// We try to find the stashes already open.. which is why this gets soo messy
//  --KES 02/25/02
//================================================================================

void world_dynamics_system::load_hero(device_id_t joystick_num, const int hero_num)
{
	int i,j;
	char hero_sname[64];
	char board_stash_name[100];

	//strcpy(hero_sname,hero_filename.c_str());

	if (g_game_ptr->GetUsingPersonalitySuit(hero_num))
	{
		strcpy(hero_sname, "PERSONALITY");
		strcat(hero_sname, SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr);
	}
	else
		strcpy(hero_sname,SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name);

	stringx stash_filename = "";
	stringx stash_filename_aux = "";

	i=0; j=0;
	while ( hero_sname[i] && j<8 )
	{
		if ( hero_sname[i]!=' ' )
		{
			char tstr[2];
			tstr[0] = hero_sname[i];
			tstr[1] = '\0';
			stash_filename += tstr;
			j++;
		}
		i++;
	}
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		/*if ((g_game_ptr->GetBoardIdx(hero_num) >= MAX_BOARDS) &&
				!g_career->locations[g_game_ptr->GetBoardIdx(hero_num) - MAX_BOARDS].IsBoardUnlocked())
				g_game_ptr->SetBoardIdx(hero_num, 0);*/
		if ((g_game_ptr->GetBoardIdx(hero_num) >= MAX_BOARDS) &&
				!unlockManager.isLocationBoardUnlocked(g_game_ptr->GetBoardIdx(hero_num) - MAX_BOARDS))
				g_game_ptr->SetBoardIdx(hero_num, 0);

	}
	else
	{
		/*if ((g_game_ptr->GetBoardIdx(hero_num) >= MAX_BOARDS) &&
				!globalCareerData.isLocationBoardUnlocked(g_game_ptr->GetBoardIdx(hero_num) - MAX_BOARDS))
				g_game_ptr->SetBoardIdx(hero_num, 0);*/
		if ((g_game_ptr->GetBoardIdx(hero_num) >= MAX_BOARDS) &&
				!unlockManager.isLocationBoardUnlocked(g_game_ptr->GetBoardIdx(hero_num) - MAX_BOARDS))
				g_game_ptr->SetBoardIdx(hero_num, 0);

	}
	if (g_game_ptr->GetBoardIdx(hero_num) >= MAX_BOARDS)
	{
		char shortbeachname[10];

		strcpy(shortbeachname, g_game_ptr->get_beach_board_name(g_game_ptr->GetBoardIdx(hero_num) - MAX_BOARDS).c_str());

		shortbeachname[4] = '\0';
		stringx daname = stringx("SURFERS\\BCHBRD\\") + stringx(shortbeachname) + stringx("_BRD.ST2");
		strcpy(board_stash_name, daname.c_str());
	}
	else if (g_game_ptr->GetUsingPersonalitySuit(hero_num))
	{

		stringx daname = stringx("SURFERS\\AUXSTASH\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile) + stringx("\\");
		daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr) + stringx("P") + stringx("_BRD.ST2");
		strcpy(board_stash_name, daname.c_str());
	}
	else
	{
		stringx daname = stringx("SURFERS\\AUXSTASH\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile) + stringx("\\");
		daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr) + stringx("_") + stringx(g_game_ptr->GetBoardIdx(hero_num)) + stringx("_BRD.ST2");
		strcpy(board_stash_name, daname.c_str());
	}


	if (g_game_ptr->GetUsingPersonalitySuit(hero_num))
	{
		stash_filename_aux = stringx("SURFERS\\AUXSTASH\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile + stringx("\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr + stringx("P_AUX.ST2");
		stash_filename= stringx("SURFERS\\") + "PERSON" + stringx( SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr);
	}
	else
	{
		stash_filename_aux = stringx("SURFERS\\AUXSTASH\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile + stringx("\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr + stringx("_AUX.ST2");
		stash_filename= stringx("SURFERS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile);
	}

	stash_filename += ".st2";

	if(g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH ||
		g_game_ptr->current_loading_state == game::LOADING_HERO_2_STASH ||
		g_game_ptr->current_loading_state == game::LOADING_HERO_1_AUX_STASH ||
		g_game_ptr->current_loading_state == game::LOADING_HERO_2_AUX_STASH)
	{
		if(g_game_ptr->current_loading_sub_state == 0)
		{
			//stringx hero_name (os_developer_options::inst()->get_string(os_developer_options::STRING_HERO_NAME));

			assert(g_game_ptr);
			assert(hero_num < MAX_PLAYERS);

			stash a_stash;

			if(!stash::using_stash()) g_game_ptr->current_loading_sub_state = -1;

			if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
			{
				switch (g_game_ptr->current_loading_state)
				{
				case game::LOADING_HERO_1_STASH:
				case game::LOADING_HERO_2_STASH:

					// Is the stash already open in STASH_SURFER??
					if (stash::is_stash_open(STASH_SURFER) && (stricmp(stash::get_stash_name(STASH_SURFER), stash_filename.c_str()) == 0))
					{
						// Is the aux stash already open in STASH_SURFER_AUX??
						if (stash::is_stash_open(STASH_SURFER_AUX) && (stricmp(stash::get_stash_name(STASH_SURFER_AUX), stash_filename_aux.c_str()) == 0))
						{
							// Set the state ahead...
							if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH)
								g_game_ptr->current_loading_state = game::LOADING_HERO_1_AUX_STASH;
							else
								g_game_ptr->current_loading_state = game::LOADING_HERO_2_AUX_STASH;
							// And mark it (state LOADING_HERO_1_AUX_STASH) as done
							g_game_ptr->current_loading_sub_state = -1;
						}
						else
						{
							// This tells the updater we're done with this step
							g_game_ptr->current_loading_sub_state = -1;
						}
					}
					// Is the stash already open in STASH_SURFER_2??
					else if (stash::is_stash_open(STASH_SURFER_2) && (stricmp(stash::get_stash_name(STASH_SURFER_2), stash_filename.c_str()) == 0))
					{
						// Is the aux stash already open in STASH_SURFER_2_AUX??
						if (stash::is_stash_open(STASH_SURFER_2_AUX) && (stricmp(stash::get_stash_name(STASH_SURFER_2_AUX), stash_filename_aux.c_str()) == 0))
						{
							// Set the state ahead...
							if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH)
								g_game_ptr->current_loading_state = game::LOADING_HERO_1_AUX_STASH;
							else
								g_game_ptr->current_loading_state = game::LOADING_HERO_2_AUX_STASH;

							// And mark it (state LOADING_HERO_1_AUX_STASH) as done
							g_game_ptr->current_loading_sub_state = -1;

						}
						else
						{
							// This tells the updater we're done with this step
							g_game_ptr->current_loading_sub_state = -1;
						}
					}
					// Ok, the STASH_SURFER is open.. use it
					else if (!stash::is_stash_open(STASH_SURFER) && mem_get_total_avail(SURFER_HEAP))
					{

						mem_push_current_heap(SURFER_HEAP);
						int stash_size = stash::pre_open_stash_for_async((char *)stash_filename.c_str(),STASH_SURFER);
						assert(stash_size != 0)
						stash::set_async_read_size(STASH_SURFER);
						if(stash_size == 0) g_game_ptr->current_loading_sub_state = -1;
						if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH)
							g_game_ptr->SetStashSize(game::LOADING_HERO_1_STASH, stash_size);
						else
							g_game_ptr->SetStashSize(game::LOADING_HERO_2_STASH, stash_size);
						mem_pop_current_heap();
					}
					// Ok, the STASH_SURFER_2 is open.. use it
					else if (!stash::is_stash_open(STASH_SURFER_2) /*&& mem_get_total_avail(SURFER_HEAP2)*/)
					{
						//mem_push_current_heap(SURFER_HEAP2);
						int stash_size = stash::pre_open_stash_for_async((char *)stash_filename.c_str(),STASH_SURFER_2);
						assert(stash_size != 0)
						stash::set_async_read_size(STASH_SURFER_2);
						if(stash_size == 0) g_game_ptr->current_loading_sub_state = -1;
						if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH)
							g_game_ptr->SetStashSize(game::LOADING_HERO_1_STASH, stash_size);
						else
							g_game_ptr->SetStashSize(game::LOADING_HERO_2_STASH, stash_size);
						//mem_pop_current_heap();
					}
					else  // Not loaded and no stashes free
					{
						// Two players?
						if (g_game_ptr->get_num_players() + g_game_ptr->get_num_ai_players() == 2)
						{
							// Find the one to close
							stringx other_hero_name = stringx("SURFERS\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num?0:1)].stashfile + stringx(".ST2");
							if (stash::is_stash_open(STASH_SURFER) && stricmp(stash::get_stash_name(STASH_SURFER), other_hero_name.c_str()) == 0)
							{

								// Close the other one
								stash::close_stash(STASH_SURFER_2);
								stash::free_stored(STASH_SURFER_2);
								stash::close_stash(STASH_SURFER_2_AUX);
								stash::free_stored(STASH_SURFER_2_AUX);
								// Load into it

								//mem_push_current_heap(SURFER_HEAP2);
								int stash_size = stash::pre_open_stash_for_async((char *)stash_filename.c_str(),STASH_SURFER_2);
								assert(stash_size != 0)
								stash::set_async_read_size(STASH_SURFER_2);
								if(stash_size == 0) g_game_ptr->current_loading_sub_state = -1;
								if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH)
									g_game_ptr->SetStashSize(game::LOADING_HERO_1_STASH, stash_size);
								else
									g_game_ptr->SetStashSize(game::LOADING_HERO_2_STASH, stash_size);
								//mem_pop_current_heap();
							}
							else if (stash::is_stash_open(STASH_SURFER_2) && stricmp(stash::get_stash_name(STASH_SURFER_2), other_hero_name.c_str()) == 0)
							{
								// Close the other one
								stash::close_stash(STASH_SURFER);
								stash::free_stored(STASH_SURFER);
								stash::close_stash(STASH_SURFER_AUX);
								stash::free_stored(STASH_SURFER_AUX);

								// Load into it

								mem_push_current_heap(SURFER_HEAP);
								int stash_size = stash::pre_open_stash_for_async((char *)stash_filename.c_str(),STASH_SURFER);
								assert(stash_size != 0)
								stash::set_async_read_size(STASH_SURFER);
								if(stash_size == 0) g_game_ptr->current_loading_sub_state = -1;
								if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH)
									g_game_ptr->SetStashSize(game::LOADING_HERO_1_STASH, stash_size);
								else
									g_game_ptr->SetStashSize(game::LOADING_HERO_2_STASH, stash_size);
								mem_pop_current_heap();
							}
						}
						else // One player, neither stash is the right one??
						{
							stash::close_stash(STASH_SURFER);
							stash::free_stored(STASH_SURFER);
							stash::close_stash(STASH_SURFER_AUX);
							stash::free_stored(STASH_SURFER_AUX);
							mem_push_current_heap(SURFER_HEAP);
							int stash_size = stash::pre_open_stash_for_async((char *)stash_filename.c_str(),STASH_SURFER);
							assert(stash_size != 0)
							stash::set_async_read_size(STASH_SURFER);
							if(stash_size == 0) g_game_ptr->current_loading_sub_state = -1;
							if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_STASH)
								g_game_ptr->SetStashSize(game::LOADING_HERO_1_STASH, stash_size);
							else
								g_game_ptr->SetStashSize(game::LOADING_HERO_2_STASH, stash_size);
							mem_pop_current_heap();
						}
					}
					break;
      case game::LOADING_HERO_1_AUX_STASH:
      case game::LOADING_HERO_2_AUX_STASH:
		  if (stash::is_stash_open(STASH_SURFER_AUX) && (stricmp(stash::get_stash_name(STASH_SURFER_AUX), stash_filename_aux.c_str()) == 0))
		  {
			  g_game_ptr->current_loading_sub_state = -1;
		  }
		  else if (stash::is_stash_open(STASH_SURFER_2_AUX) && (stricmp(stash::get_stash_name(STASH_SURFER_2_AUX), stash_filename_aux.c_str()) == 0))
		  {
			  g_game_ptr->current_loading_sub_state = -1;
		  }
		  else
		  {
			  if (stash::is_stash_open(STASH_SURFER) && stricmp(stash::get_stash_name(STASH_SURFER), stash_filename.c_str()) == 0)
			  {
				  stash::close_stash(STASH_SURFER_AUX);
				  stash::free_stored(STASH_SURFER_AUX);
					stash::close_stash(STASH_SURFER_BOARD);
					stash::free_stored(STASH_SURFER_BOARD);
				  mem_push_current_heap(SURFER_HEAP);
					stash::open_stash(board_stash_name, STASH_SURFER_BOARD);
				  int stash_size = stash::pre_open_stash_for_async((char *)stash_filename_aux.c_str(),STASH_SURFER_AUX);
					assert(stash_size != 0)
				  stash::set_async_read_size(STASH_SURFER_AUX);
				  if(stash_size == 0) g_game_ptr->current_loading_sub_state = -1;
				  if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_AUX_STASH)
					  g_game_ptr->SetStashSize(game::LOADING_HERO_1_AUX_STASH, stash_size);
				  else
					  g_game_ptr->SetStashSize(game::LOADING_HERO_2_AUX_STASH, stash_size);
				  mem_pop_current_heap();
			  }
			  else if (stash::is_stash_open(STASH_SURFER_2) && stricmp(stash::get_stash_name(STASH_SURFER_2), stash_filename.c_str()) == 0)
			  {
				  stash::close_stash(STASH_SURFER_2_AUX);
				  stash::free_stored(STASH_SURFER_2_AUX);
					stash::close_stash(STASH_SURFER_2_BOARD);
				  stash::free_stored(STASH_SURFER_2_BOARD);
					stash::open_stash(board_stash_name, STASH_SURFER_2_BOARD);
				  //mem_push_current_heap(SURFER_HEAP2);
				  int stash_size = stash::pre_open_stash_for_async((char *)stash_filename_aux.c_str(),STASH_SURFER_2_AUX);
					assert(stash_size != 0)
				  stash::set_async_read_size(STASH_SURFER_2_AUX);
				  if(stash_size == 0) g_game_ptr->current_loading_sub_state = -1;
				  if (g_game_ptr->current_loading_state == game::LOADING_HERO_1_AUX_STASH)
					  g_game_ptr->SetStashSize(game::LOADING_HERO_1_AUX_STASH, stash_size);
				  else
					  g_game_ptr->SetStashSize(game::LOADING_HERO_2_AUX_STASH, stash_size);
				  //mem_pop_current_heap();
			  }
			  else
			  {
				  stringx diagnostic(stringx::fmt,
					  "stash::is_stash_open(STASH_SURFER) = %s,  stash::get_stash_name(STASH_SURFER) = %s,  stash_filename.c_str() = %s.",
					  stash::is_stash_open(STASH_SURFER) ? "true" : "false",
					  stash::get_stash_name(STASH_SURFER),
					  stash_filename.c_str()
				  );
				  assertmsg(0, diagnostic.c_str());
			  }

		  }
		  break;
    }

	}
#if 0
	else if (
#ifdef TARGET_PC
		os_file::file_exists((char *)stash_filename.c_str()) &&
#endif
		stash::open_stash((char *)stash_filename.c_str()) == false)
        debug_print("could not open stash file %s\n", stash_filename.c_str());
    }
    // else do nothing, we're ready to fly
  }
  else
  {
	  // for insurance
	  stash::close_stash();
	  stash::free_stored();
	  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
		  stash::open_stash((char *)stash_filename.c_str());
	  else if (
#ifdef TARGET_PC
		  os_file::file_exists((char *)stash_filename.c_str()) &&
#endif
		  stash::open_stash((char *)stash_filename.c_str()) == false)
		  debug_print("could not open stash file %s\n", stash_filename.c_str());
  }
#endif
	}	// g_game_ptr->current_loading_sub_state == 0

	bool done;

	if ((stash::using_stash() == true) && g_game_ptr->current_loading_sub_state != -1)
	{
		switch(g_game_ptr->current_loading_state)
		{
		case game::LOADING_HERO_1_STASH:
		case game::LOADING_HERO_2_STASH:
			if (stash::is_stash_preopened(STASH_SURFER) && (stricmp(stash::get_stash_name(STASH_SURFER), stash_filename.c_str()) == 0))
			{
				mem_push_current_heap(SURFER_HEAP);
				done = stash::read_stash_async(STASH_SURFER);
				if(done) g_game_ptr->current_loading_sub_state = -1;
				mem_pop_current_heap();
			}
			else
			{
				//mem_push_current_heap(SURFER_HEAP2);
				done = stash::read_stash_async(STASH_SURFER_2);
				if(done) g_game_ptr->current_loading_sub_state = -1;
				//mem_pop_current_heap();
			}
			break;
		case game::LOADING_HERO_1_AUX_STASH:
		case game::LOADING_HERO_2_AUX_STASH:
			if (stash::is_stash_preopened(STASH_SURFER_AUX) && (stricmp(stash::get_stash_name(STASH_SURFER_AUX), stash_filename_aux.c_str()) == 0))
			{
				mem_push_current_heap(SURFER_HEAP);
				done = stash::read_stash_async(STASH_SURFER_AUX);
				if(done) g_game_ptr->current_loading_sub_state = -1;
				mem_pop_current_heap();
			}
			else
			{
				//mem_push_current_heap(SURFER_HEAP2);
				done = stash::read_stash_async(STASH_SURFER_2_AUX);
				if(done) g_game_ptr->current_loading_sub_state = -1;
				//mem_pop_current_heap();
			}
			break;
		}
	}
  }		// g_game_ptr->current_loading_state == LOADING_HERO_1_STASH or 1_AUX_STASH or 2_STASH or 2_AUX_STASH

  else if(g_game_ptr->current_loading_state == game::LOADING_HERO_1_REST ||
	  g_game_ptr->current_loading_state == game::LOADING_HERO_2_REST)
  {
  /*P
  int before = memtrack::get_total_alloced();
  int script_mem = membudget()->get_usage( membudget_t::SCRIPTS );
  int anim_mem = membudget()->get_usage( membudget_t::ANIMS );
  int sound_mem = membudget()->get_usage( membudget_t::SOUNDS );
  g_memory_context.push_context( "HERO" );
	  P*/
	  // BIGCULL  assert(g_spiderman_ptr == NULL);

	  /*
	  {
	  stringx test = g_game_ptr->getHeroname(0);

		test.to_upper();
		//BIGCULL    extern bool spidey_or_spiderman;
		//BIGCULL    spidey_or_spiderman = (test == stringx("SPIDEY"));
		}
	  */

	  // <<<<	stringx mesh_path = "characters\\" + hero_filename + "\\entities\\";
	  stringx mesh_path = "characters\\" + stringx(hero_sname) + "\\entities\\";
	  nglSetMeshPath( (char *)mesh_path.c_str() );
	  // <<<<	stringx texture_path = "characters\\" + hero_filename + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	  stringx texture_path = "characters\\" + stringx(hero_sname) + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	  nglSetTexturePath( (char *)texture_path.c_str() );


	  stringx	entityName("HERO" + stringx(hero_num));
	  entity*	newhero = g_entity_maker->create_entity_or_subclass(stringx("characters\\") + stringx(hero_sname) + stringx("\\entities\\") + stringx(hero_sname), entity_id::make_entity_id(entityName.c_str()), po_identity_matrix, "characters\\", ACTIVE_FLAG|NONSTATIC_FLAG);
	  set_hero_ptr(hero_num, newhero);

	  kellyslater_controller *ksc_ptr=NEW kellyslater_controller(hero_num, newhero);
	  set_ks_controller(hero_num, ksc_ptr);
	  ksc_ptr->set_joystick_num( joystick_num );
	  g_world_ptr->guarantee_active(ksc_ptr->GetBoardMember());
	  //if(hero_num) return;

/*	// Should be superseded by the code in entity.cpp, FixupEntityMesh().  (dc 04/30/02)
	  // Temporary hack to make sure we don't waste time on backface culling. (dc 12/11/01)
	  nglMesh *Mesh = get_hero_ptr(hero_num)->get_mesh ();
	  if (Mesh)
	  {
		  for ( u_int i = 0; i < Mesh->NSections; i++ )
		  {
			  nglMeshSection* Section = &Mesh->Sections[i];
			  nglMaterial* Material = Section->Material;

			  if (Material->Flags & NGLMAT_BACKFACE_CULL)
			  {
				  nglPrintf(PRINT_RED "Mesh %s has backface culling set.  Turning it off.\n" PRINT_BLACK, Mesh->Name.c_str());
			  }
			  Material->Flags &= ~NGLMAT_BACKFACE_CULL;
		  }
	  }
*/


	  //hero_ptr->set_rel_position(hero_ptr->get_rel_position() + YVEC);
	  get_hero_ptr(hero_num)->set_max_lights( ABSOLUTE_MAX_LIGHTS );
	  //  vector3d v = g_world_ptr->get_hero_ptr()->get_abs_position();
	  /*
	  // If possible, delay until wave has been loaded (dc 01/24/02)
	  ksc_ptr->get_board_controller ().ResetPhysics ();
	  ksc_ptr->Reset();
	  */
	  ksc_ptr->get_my_scoreManager().Reset();

	  assert(get_hero_ptr(hero_num) && get_hero_ptr(hero_num)->get_flavor() == ENTITY_CONGLOMERATE);
	  conglomerate *surferman_ptr = (conglomerate *)get_hero_ptr(hero_num);
	  // <<<< I dont really understand the conglomerate vs. entity casting for hero_ptr.. just going with it for now.  -DL
	  ksc_ptr->GetBoardMember()->set_max_lights( ABSOLUTE_MAX_LIGHTS );

	  surferman_ptr->create_light_set();
	  ksc_ptr->GetBoardMember()->create_light_set();
	  ksc_ptr->GetBoardMember()->set_active(true);

	  surferman_ptr->make_animateable(true);
	  entity *ent = ksc_ptr->get_owner();
	  // <<<<	mesh_path = "characters\\" + hero_filename + "\\entities\\";
	  mesh_path = "characters\\" + stringx(hero_sname) + "\\entities\\";
	  nglSetMeshPath( (char *)mesh_path.c_str() );
	  // <<<<	texture_path = "characters\\" + hero_filename + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	  texture_path = "characters\\" + stringx(hero_sname) + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	  nglSetTexturePath( (char *)texture_path.c_str() );

	  if ( wds_exists(mesh_path+"shadow",PLATFORM_MESH_EXT) )
	  {
		  ent->load_shadow_mesh("shadow");
	  }

#ifdef EVAN
#ifndef TARGET_XBOX
	  // Player specular reflection
	  //ent->set_specular_env_level(0.6);
#endif
#endif

	  assert(surferman_ptr->has_ai_ifc());

	  guarantee_active(surferman_ptr);

	  surferman_ptr->ai_ifc()->set_team(ai_interface::_TEAM_HERO);
	  surferman_ptr->ai_ifc()->set_enemy_team(ai_interface::_TEAM_EVIL);
	  surferman_ptr->ai_ifc()->set_active(true);

	  // Choose surfer's outfit.
	  int outfit = os_developer_options::inst()->get_int(os_developer_options::INT_OUTFIT);
	  // These need to go in the beach database! (dc 01/29/02)
	  if (outfit == -1)
	  {
	  /*  switch (g_game_ptr->get_beach_id())
	  {
	  case BEACH_ANTARCTICA : outfit = 1; break;
	  case BEACH_BELLS : outfit = 1; break;
	  case BEACH_CORTESBANK : outfit = 1; break;
	  case BEACH_GLAND : outfit = 0; break;
	  case BEACH_JAWS : outfit = 0; break;
	  case BEACH_JEFFERSONBAY : outfit = 1; break;
	  case BEACH_MAVERICKS : outfit = 1; break;
	  case BEACH_MUNDAKA : outfit = 1; break;
	  case BEACH_PIPELINE : outfit = 0; break;
	  case BEACH_SEBASTIAN : outfit = 0; break;
	  case BEACH_TEAHUPOO : outfit = 0; break;
	  case BEACH_TRESTLES : outfit = 0; break;
	  case BEACH_INDOOR : outfit = 0; break;
	  case BEACH_COSMOS : outfit = 1; break;
	  default : outfit = 0;
	  }*/
		  outfit = BeachDataArray[g_game_ptr->get_beach_id()].use_wetsuit;
	  }

	  int id;
	  /*if (surferman_ptr->get_mesh())
	  {
		  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 96)) != 0) //  leash strap
		  {
			  int mask = (1 << id);
			  surferman_ptr->SetMaterialMask(mask);
		  }
	  }*/

	  if (outfit == 0)	// wearing shorts / sunglasses
	  {
		  if (surferman_ptr->get_mesh())
		  {
			  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 92)) != 0)	// right pelvis
			  {
				  int mask = (1 << id);
				  surferman_ptr->SetMaterialMask(mask);
			  }
			  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 93)) != 0)	// left pelvis
			  {
				  int mask = (1 << id);
				  surferman_ptr->SetMaterialMask(mask);
			  }
		  }

			// Let KS head and Surfreak's eyes animate...
		  if (!g_game_ptr->GetUsingPersonalitySuit(hero_num) && SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].initially_unlocked)
				surferman_ptr->SetTextureFrame(0);
	  }
	  else if (outfit == 1)	// wearing wet suit
	  {
		  if (surferman_ptr->get_mesh())
		  {
			  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 99)) != 0)	// sunglasses
			  {
				  int mask = (1 << id);
				  surferman_ptr->SetMaterialMask(mask);
			  }
			  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 95)) != 0)	// right shorts
			  {
				  int mask = (1 << id);
				  surferman_ptr->SetMaterialMask(mask);
			  }
			  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 94)) != 0)	// left shorts
			  {
				  int mask = (1 << id);
				  surferman_ptr->SetMaterialMask(mask);
			  }
		  }

			// Let KS head and Surfreak's eyes animate...
		  if (!g_game_ptr->GetUsingPersonalitySuit(hero_num) && SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].initially_unlocked)
				surferman_ptr->SetTextureFrame(1);
	  }

		/*P  g_memory_context.pop_context();
	  script_mem = membudget()->get_usage( membudget_t::SCRIPTS ) - script_mem;
	  anim_mem = membudget()->get_usage( membudget_t::ANIMS ) - anim_mem;
	  sound_mem = membudget()->get_usage( membudget_t::SOUNDS ) - sound_mem;
	  int non_entity_mem = script_mem + anim_mem + sound_mem;
	  membudget()->use( membudget_t::HERO, memtrack::get_total_alloced()-before-non_entity_mem );
	  P*/
	  system_idle();
  }		// g_game_ptr->current_loading_state == LOADING_HERO_1_REST or LOADING_HERO_2_REST
}

void world_dynamics_system::load_ai_hero(device_id_t joystick_num, int surfer_index, bool pers)
{
	//stringx hero_name (os_developer_options::inst()->get_string(os_developer_options::STRING_HERO_NAME));
	int hero_num = 1;
	stringx stash_filename = "";
	stringx stash_filename_aux = "";
	char board_stash_name[100];
	stringx temp;
	assert(g_game_ptr);
	stringx hero_name;

	//assert(g_game_ptr->get_num_players() == 1);
	//strcpy(hero_sname,hero_filename.c_str());
		g_game_ptr->SetSurferIdx(1, surfer_index);
	g_game_ptr->SetUsingPersonalitySuit(1, pers);
	//g_game_ptr->set_num_ai_players(1);
	temp = SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name;
	g_game_ptr->setHeroname(hero_num, temp);

	if (g_game_ptr->GetBoardIdx(hero_num) >= MAX_BOARDS)
	{
		char shortbeachname[10];

		strcpy(shortbeachname, g_game_ptr->get_beach_board_name(g_game_ptr->GetBoardIdx(hero_num) - MAX_BOARDS).c_str());
		shortbeachname[4] = '\0';
		stringx daname = stringx("SURFERS\\BCHBRD\\") + stringx(shortbeachname) + stringx("_BRD.ST2");
		strcpy(board_stash_name, daname.c_str());
	}
	else if (g_game_ptr->GetUsingPersonalitySuit(hero_num))
	{

		stringx daname = stringx("SURFERS\\AUXSTASH\\PERSON") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr) + stringx("\\");
		daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr) + stringx("P") + stringx("_BRD.ST2");
		strcpy(board_stash_name, daname.c_str());
	}
	else
	{
		stringx daname = stringx("SURFERS\\AUXSTASH\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile) + stringx("\\");
		daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr) + stringx("_") + stringx(g_game_ptr->GetBoardIdx(hero_num)) + stringx("_BRD.ST2");
		strcpy(board_stash_name, daname.c_str());
	}


	stringx hero_dir;
	if (g_game_ptr->GetUsingPersonalitySuit(hero_num))
	{
		stash_filename_aux = stringx("SURFERS\\AUXSTASH\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile + stringx("\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr + stringx("_P_AUX.ST2");
		stash_filename= stringx("SURFERS\\") + "PERSON" + stringx( SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr);
		hero_name = stringx("PERSONALITY") + stringx( SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr);

	}
	else
	{
		stash_filename_aux = stringx("SURFERS\\AUXSTASH\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile + stringx("\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr + stringx("_AUX.ST2");
		stash_filename= stringx("SURFERS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].stashfile);
		hero_name  = SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name;

	}

	stash_filename += ".st2";

	stash a_stash;
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
	{
		debug_print("opening surfer stash %s\n", stash_filename.c_str());

		if (stash::is_stash_open(STASH_SURFER) && (stricmp(stash::get_stash_name(STASH_SURFER), stash_filename.c_str()) == 0))
		{
			debug_print("%s is already open (STASH_SURFER)", stash_filename.c_str());
		}
		else if (stash::is_stash_open(STASH_SURFER_2) && (stricmp(stash::get_stash_name(STASH_SURFER_2), stash_filename.c_str()) == 0))
		{
			debug_print("%s is already open (STASH_SURFER_2)", stash_filename.c_str());
		}
		else
		{
			if (!stash::is_stash_open(STASH_SURFER))
				stash::open_stash(  (char *)stash_filename.c_str(),STASH_SURFER);
			else if (!stash::is_stash_open(STASH_SURFER_2))
				stash::open_stash(  (char *)stash_filename.c_str(),STASH_SURFER_2);
			else
				assert(0 && "CAN'T FIND FREE SURFER STASH!");
		}
		debug_print("opening surfer stash %s\n", stash_filename.c_str());



		if (stash::is_stash_open(STASH_SURFER_AUX) && (stricmp(stash::get_stash_name(STASH_SURFER_AUX), stash_filename_aux.c_str()) == 0))
		{
			debug_print("%s is already open (STASH_SURFER)", stash_filename_aux.c_str());
		}
		else if (stash::is_stash_open(STASH_SURFER_2_AUX) && (stricmp(stash::get_stash_name(STASH_SURFER_2_AUX), stash_filename_aux.c_str()) == 0))
		{
			debug_print("%s is already open (STASH_SURFER_2)", stash_filename_aux.c_str());
		}
		else
		{
			if (!stash::is_stash_open(STASH_SURFER_AUX))
				stash::open_stash(  (char *)stash_filename_aux.c_str(),STASH_SURFER_AUX);
			else if (!stash::is_stash_open(STASH_SURFER_2_AUX))
				stash::open_stash(  (char *)stash_filename_aux.c_str(),STASH_SURFER_2_AUX);
			else
				assert(0 && "CAN'T FIND FREE SURFER AUX STASH!");
		}


		if (stash::is_stash_open(STASH_SURFER_BOARD) && (stricmp(stash::get_stash_name(STASH_SURFER_BOARD), stash_filename_aux.c_str()) == 0))
		{
			debug_print("%s is already open (STASH_SURFER_BOARD)", stash_filename_aux.c_str());
		}
		else if (stash::is_stash_open(STASH_SURFER_2_BOARD) && (stricmp(stash::get_stash_name(STASH_SURFER_2_BOARD), stash_filename_aux.c_str()) == 0))
		{
			debug_print("%s is already open (STASH_SURFER_2_BOARD)", stash_filename_aux.c_str());
		}
		else
		{
			if (!stash::is_stash_open(STASH_SURFER_BOARD))
				stash::open_stash(board_stash_name, STASH_SURFER_BOARD);
			else if (!stash::is_stash_open(STASH_SURFER_2_BOARD))
				stash::open_stash(board_stash_name, STASH_SURFER_2_BOARD);
			else
				assert(0 && "CAN'T FIND FREE STASH_SURFER_BOARD!");
		}

	}
#if 0
	else if (
#ifdef TARGET_PC
		os_file::file_exists((char *)stash_filename.c_str()) &&
#endif
		stash::open_stash((char *)stash_filename.c_str()) == false)
        debug_print("could not open stash file %s\n", stash_filename.c_str());
}
// else do nothing, we're ready to fly
  }
  else
  {
	  // for insurance
	  stash::close_stash();
	  stash::free_stored();
	  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
		  stash::open_stash((char *)stash_filename.c_str());
	  else if (
#ifdef TARGET_PC
		  os_file::file_exists((char *)stash_filename.c_str()) &&
#endif
		  stash::open_stash((char *)stash_filename.c_str()) == false)
		  debug_print("could not open stash file %s\n", stash_filename.c_str());
  }
#endif


  // <<<<	stringx mesh_path = "characters\\" + hero_filename + "\\entities\\";
  stringx mesh_path = "characters\\" + stringx(hero_name) + "\\entities\\";
  nglSetMeshPath( (char *)mesh_path.c_str() );
  // <<<<	stringx texture_path = "characters\\" + hero_filename + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
  stringx texture_path = "characters\\" + stringx(hero_name) + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
  nglSetTexturePath( (char *)texture_path.c_str() );


  stringx	entityName("HERO" + stringx(hero_num));
  entity*	newhero = g_entity_maker->create_entity_or_subclass(stringx("characters\\") + hero_name + stringx("\\entities\\") + hero_name, entity_id::make_entity_id(entityName.c_str()), po_identity_matrix, "characters\\", ACTIVE_FLAG|NONSTATIC_FLAG);
  set_hero_ptr(hero_num, newhero);

  kellyslater_controller *ksc_ptr=NEW kellyslater_controller(hero_num, newhero);
  set_ks_controller(hero_num, ksc_ptr);
  ksc_ptr->set_joystick_num( joystick_num );
  g_world_ptr->guarantee_active(ksc_ptr->GetBoardMember());
  //if(hero_num) return;

/*	// Should be superseded by the code in entity.cpp, FixupEntityMesh().  (dc 04/30/02)
  // Temporary hack to make sure we don't waste time on backface culling. (dc 12/11/01)
  nglMesh *Mesh = get_hero_ptr(hero_num)->get_mesh ();
  if (Mesh)
  {
	  for ( u_int i = 0; i < Mesh->NSections; i++ )
	  {
		  nglMeshSection* Section = &Mesh->Sections[i];
		  nglMaterial* Material = Section->Material;

		  if (Material->Flags & NGLMAT_BACKFACE_CULL)
		  {
			  nglPrintf(PRINT_RED "Mesh %s has backface culling set.  Turning it off.\n" PRINT_BLACK, Mesh->Name.c_str());
		  }
		  Material->Flags &= ~NGLMAT_BACKFACE_CULL;
	  }
  }
*/


  //hero_ptr->set_rel_position(hero_ptr->get_rel_position() + YVEC);
  get_hero_ptr(hero_num)->set_max_lights( ABSOLUTE_MAX_LIGHTS );
  //  vector3d v = g_world_ptr->get_hero_ptr()->get_abs_position();
  /*
  // If possible, delay until wave has been loaded (dc 01/24/02)
  ksc_ptr->get_board_controller ().ResetPhysics ();
  ksc_ptr->Reset();
  */
  ksc_ptr->get_my_scoreManager().Reset();

  assert(get_hero_ptr(hero_num) && get_hero_ptr(hero_num)->get_flavor() == ENTITY_CONGLOMERATE);
  conglomerate *surferman_ptr = (conglomerate *)get_hero_ptr(hero_num);
  // <<<< I dont really understand the conglomerate vs. entity casting for hero_ptr.. just going with it for now.  -DL

  surferman_ptr->create_light_set();
  ksc_ptr->GetBoardMember()->create_light_set();
	ksc_ptr->GetBoardMember()->set_active(true);
	surferman_ptr->make_animateable(true);
#if 1
#endif
  entity *ent = ksc_ptr->get_owner();
  // <<<<	mesh_path = "characters\\" + hero_filename + "\\entities\\";
  mesh_path = "characters\\" + stringx(hero_name) + "\\entities\\";
  nglSetMeshPath( (char *)mesh_path.c_str() );
  // <<<<	texture_path = "characters\\" + hero_filename + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
  texture_path = "characters\\" + stringx(hero_name) + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
  nglSetTexturePath( (char *)texture_path.c_str() );

  if ( wds_exists(mesh_path+"shadow", PLATFORM_MESH_EXT) )
  {
	  ent->load_shadow_mesh("shadow");
  }

#ifdef EVAN
#ifndef TARGET_XBOX
		// Player specular reflection
  //ent->set_specular_env_level(0.6);
#endif
#endif

  assert(surferman_ptr->has_ai_ifc());

  guarantee_active(surferman_ptr);

  surferman_ptr->ai_ifc()->set_team(ai_interface::_TEAM_HERO);
  surferman_ptr->ai_ifc()->set_enemy_team(ai_interface::_TEAM_EVIL);
  surferman_ptr->ai_ifc()->set_active(true);

  // Choose surfer's outfit.
  int outfit = os_developer_options::inst()->get_int(os_developer_options::INT_OUTFIT);
  // These need to go in the beach database! (dc 01/29/02)
  if (outfit == -1)
  {
  /*switch (g_game_ptr->get_beach_id())
  {
		case BEACH_ANTARCTICA : outfit = 1; break;
		case BEACH_BELLS : outfit = 1; break;
		case BEACH_CORTESBANK : outfit = 1; break;
		case BEACH_GLAND : outfit = 0; break;
		case BEACH_JAWS : outfit = 0; break;
		case BEACH_JEFFERSONBAY : outfit = 1; break;
		case BEACH_MAVERICKS : outfit = 1; break;
		case BEACH_MUNDAKA : outfit = 1; break;
		case BEACH_PIPELINE : outfit = 0; break;
		case BEACH_SEBASTIAN : outfit = 0; break;
		case BEACH_TEAHUPOO : outfit = 0; break;
		case BEACH_TRESTLES : outfit = 0; break;
		case BEACH_INDOOR : outfit = 0; break;
		case BEACH_COSMOS : outfit = 1; break;
		default : outfit = 0;
  }*/
	  outfit = BeachDataArray[g_game_ptr->get_beach_id()].use_wetsuit;
  }
  if (outfit == 0)	// wearing shorts / sunglasses
  {
	  if (surferman_ptr->get_mesh())
	  {
		  int id;
		  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 92)) != 0)	// right pelvis
		  {
			  int mask = (1 << id);
			  surferman_ptr->SetMaterialMask(mask);
		  }
		  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 93)) != 0)	// left pelvis
		  {
			  int mask = (1 << id);
			  surferman_ptr->SetMaterialMask(mask);
		  }
	  }
	  surferman_ptr->SetTextureFrame(0);
  }
  else if (outfit == 1)	// wearing wet suit
  {
	  if (surferman_ptr->get_mesh())
	  {
		  int id;
		  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 99)) != 0)	// sunglasses
		  {
			  int mask = (1 << id);
			  surferman_ptr->SetMaterialMask(mask);
		  }
		  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 95)) != 0)	// right shorts
		  {
			  int mask = (1 << id);
			  surferman_ptr->SetMaterialMask(mask);
		  }
		  if ((id = nglGetMaterialIdx(surferman_ptr->get_mesh(), 94)) != 0)	// left shorts
		  {
			  int mask = (1 << id);
			  surferman_ptr->SetMaterialMask(mask);
		  }
	  }
	  surferman_ptr->SetTextureFrame(1);
  }


  /*P  g_memory_context.pop_context();
  script_mem = membudget()->get_usage( membudget_t::SCRIPTS ) - script_mem;
  anim_mem = membudget()->get_usage( membudget_t::ANIMS ) - anim_mem;
  sound_mem = membudget()->get_usage( membudget_t::SOUNDS ) - sound_mem;
  int non_entity_mem = script_mem + anim_mem + sound_mem;
  membudget()->use( membudget_t::HERO, memtrack::get_total_alloced()-before-non_entity_mem );
  P*/
  system_idle();
}
/* formerly the "_and_so_forth" of load_hero_and_so_forth() */
void world_dynamics_system::setup_cameras()
{
	// user camera
	{
		camera* new_camera = NEW camera(NULL,entity_id::make_entity_id("USER_CAM"));
		new_camera->set_flag( EFLAG_MISC_NONSTATIC, true );
		new_camera->set_rel_position( get_hero_ptr(g_game_ptr->get_active_player())->get_abs_position() );
		add_camera( new_camera );
		theta_and_psi_mcs* xmcs = NEW theta_and_psi_mcs( new_camera, 0, 0 );
		g_theta_and_psi_mcs= xmcs;
		add_mcs( xmcs );
		dolly_and_strafe_mcs* dasmcs = NEW dolly_and_strafe_mcs( new_camera );
		add_mcs( dasmcs );

		joypad_usercam_controller * jc = NEW joypad_usercam_controller(dasmcs,xmcs);
		g_ps2usercam_controller = jc;
		add_controller( jc );
		usercam_controller = jc;

		usercam_move_mcs = xmcs;
		usercam_orient_mcs = dasmcs;
		usercam = new_camera;

	}

	replay_cam_ptr = NEW replay_camera(entity_id::make_entity_id("REPLAY_CAM"), get_hero_ptr(g_game_ptr->get_active_player()));
	replay_cam_ptr->set_ks_controller(ks_controller[0]);
	replay_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	add_camera(replay_cam_ptr);

	system_idle();

#ifdef DEBUG
	{
		{
			add_capsule_history(collision_capsule(get_hero_ptr(g_game_ptr->get_active_player())));	// wds::setup_cameras(): wtf is this?  (multiplayer fixme?)
		}
	}
#endif

	// boom mic:  a mic on the character
	//add_mic( NEW mic( get_hero_ptr(g_game_ptr->get_active_player()), entity_id::make_entity_id("BOOM_MIC") ));  // wds::setup_cameras(): wtf is this?  (multiplayer fixme)

#if 0	// seems to be purely Spiderman stuff (dc 04/02/02)
	// marky cam
	{
		marky_camera* new_camera = NEW marky_camera(entity_id::make_entity_id("MARKY_CAM"));
		new_camera->set_flag( EFLAG_MISC_NONSTATIC, true );
		marky_cam = new_camera;
		add_camera( new_camera );
	}

	// scene analyzer - secret camera to help test culling.
	{
		camera* new_camera = NEW camera(NULL,entity_id::make_entity_id("SCENE_ANALYZER_CAM"));
		new_camera->set_flag( EFLAG_MISC_NONSTATIC, true );
		new_camera->set_rel_position( get_hero_ptr(g_game_ptr->get_active_player())->get_abs_position() );
		add_camera( new_camera );
		theta_and_psi_mcs* xmcs = NEW theta_and_psi_mcs( new_camera, 0, 0 );
		add_mcs( xmcs );
		dolly_and_strafe_mcs* dasmcs = NEW dolly_and_strafe_mcs( new_camera );
		add_mcs( dasmcs );
		mouselook_controller* mc = NEW mouselook_controller(dasmcs,xmcs);
		add_controller( mc );

		scene_analyzer_cam = new_camera;
		scene_analyzer_move_mcs = xmcs;
		scene_analyzer_orient_mcs = dasmcs;
		scene_analyzer_controller = mc;
	}
#endif


	//  g_spiderman_ptr->prep();
	//  g_spiderman_camera_ptr->prep();




	// Perform run-time link of all script functions.
	// NOTE: We are forced to split load and link into two separate steps in
	// order to allow linking to script object instances.
	scriptman.link();

	char gsoi_name[200];
	strcpy(gsoi_name, g_sxfname.c_str());

	// strip off the .SX
	gsoi_name[strlen(gsoi_name)-3] = 0;
	stringx global_object_name = "_"+stringx(gsoi_name);
	global_object_name.to_lower();
	cur_global_so = scriptman.find_object( global_object_name );
	if ( !cur_global_so )
		error( g_sxfname + ": global script object not found" );
	cur_global_so_inst = cur_global_so->find_instance( "__auto" );
	assert( cur_global_so_inst );

	script::init(cur_global_so, cur_global_so_inst);


	system_idle();

	// allow entities to do post-load, pre-optimize initialization
	entity_list::const_iterator ei = entities.begin();
	entity_list::const_iterator ei_end = entities.end();
	for ( ; ei!=ei_end; ++ei )
	{
		entity *e = *ei;
		e->initialize();
	}

	system_idle();

	// allow entities to optimize their internal representations
	for (ei = entities.begin(); ei!=ei_end; ++ei )
	{
		entity *e = *ei;
		e->optimize();
		if(e->has_ai_ifc() && !e->ai_ifc()->cosmetic())
			guarantee_active(e);
	}

	the_terrain->optimize();

	system_idle();

	vector<item*>::const_iterator ii = items.begin();
	vector<item*>::const_iterator ii_end = items.end();
	for ( ; ii!=ii_end; ++ii )
	{
		item* it = *ii;
		it->initialize();
	}

	ks_fx_init();

	system_idle();

	Read_Camera_List();
	CreateDebugMenuTools();
}

void world_dynamics_system::unload_scene( void )
{
#if 0
	entity_list::const_iterator ei = entities.begin();
	entity_list::const_iterator ei_end = entities.end();
	while ( ei!=ei_end )
	{
		entity *e = *ei;
		//destroy_entity( e );
		g_entity_maker->destroy_entity( e );
		ei = entities.begin();
		ei_end = entities.end();
	}
#endif

	// >>>> Shouldn't the hero entities be deleted here (destroy_entity'd) here
	// and the hero_ptr[]'s nulled out?  -DL

	for(int i=0;i<MAX_PLAYERS;i++)
	{
		if(ks_controller[i])
		{
			delete ks_controller[i];
			ks_controller[i]=NULL;
		}
	}

}

bool world_dynamics_system::entity_is_preloaded(const stringx& entity_name)
{
	filespec entspec( entity_name );
	entspec.name.to_upper();
	entfile_map::iterator fi = entfiles.find( entspec.name );
	return( fi != entfiles.end() );
}


void world_dynamics_system::remove_entity_from_world_processing( entity *e )
{
	e->remove_from_terrain();
	e->force_region( NULL );  // this effectively removes the entity from the world
	e->set_active( false );
	e->set_rel_position( vector3d(-9000.0f,-9000.0f,-9000.0f) );
}

entity* world_dynamics_system::add_box_trigger( entity_id id,
											   const po& loc,
											   const convex_box& binfo,
											   const region_node_list* forced_regions,
											   unsigned int scene_flags )
{
#ifdef ECULL
	entity* e = NEW entity( id, ENTITY_BOX_TRIGGER );

	if( !( scene_flags & NONSTATIC_FLAG ) ) {
		e->set_flag( EFLAG_MISC_NONSTATIC, true );
	}

	if(!e->has_box_trigger_ifc())
		e->create_box_trigger_ifc();

	box_trigger_interface* bti = e->box_trigger_ifc( );
	bti->set_box_info( binfo );

	add_box_trigger( e );

	e->set_rel_po( loc );

	if( forced_regions && !forced_regions->empty( ) ) {
		//list<region_node*>::const_iterator fri;
		region_node_list::const_iterator fri;

		for( fri = forced_regions->begin( ); fri != forced_regions->end( ); fri++ ) {
			e->force_region( *fri );
		}

	} else {
		e->compute_sector( *the_terrain );
	}

	e->frame_done( );

	return e;
#else
	return NULL;
#endif
}

void world_dynamics_system::add_box_trigger( entity* e )
{
	g_entity_maker->create_entity( e );
}

light_source* world_dynamics_system::add_light_source( entity_id id,
													  po const & loc,
													  const light_properties& props,
													  //const list<region_node*>* forced_regions,
													  const region_node_list* forced_regions,
													  unsigned int scene_flags )
{
	// construct light_source entity
	//  int before = memtrack::get_total_alloced(); // unused -- remove me?
	light_source* new_ls = NEW light_source( props, NULL, id );
	if ( !(scene_flags & NONSTATIC_FLAG) )
		new_ls->set_flag( EFLAG_MISC_NONSTATIC, true );
	add_light_source( new_ls );

	// assign position and orientation
	new_ls->set_rel_po( loc );

	// determine terrain locale
	if ( forced_regions && forced_regions->size() )
	{
		// force entity to always be counted in given region(s)
		//list<region_node*>::const_iterator fri;
		region_node_list::const_iterator fri;
		for ( fri=forced_regions->begin(); fri!=forced_regions->end(); ++fri )
			new_ls->force_region( *fri );
	}
	else
	{
		// compute entity sector (and, thus, region)
		new_ls->compute_sector( *the_terrain );
	}
	// this needs to be called to set up last-frame info
	new_ls->frame_done();
	return new_ls;
}


//DEBUG <<<<<
unsigned int * g_ptr1;


void world_dynamics_system::add_entity(const stringx& entity_name,
									   bool a, bool stationary, bool invisible, bool cosmetic, bool walkable,
									   bool repulsion, bool nonstatic,
									   entity_id & _id, entity_flavor_t flavor )
{
	entity * ent = NEW entity(entity_name, _id, flavor,DELETE_STREAM);
	if ( nonstatic )
		ent->set_flag( EFLAG_MISC_NONSTATIC, true );

	g_entity_maker->create_entity(ent);
	if (a && !stationary)
	{
		stringx composite;
		composite = stringx("Pure entity ") + entity_name  + stringx(" is active and not stationary.");
		warning (composite.c_str());
	}
	ent->set_active(a);
	ent->set_stationary(stationary);
	ent->set_walkable(walkable);
	ent->set_repulsion(repulsion);
	if (invisible)
		ent->set_visible(false);
	else
		ent->set_visible(true);
	assert(!cosmetic);
}

void world_dynamics_system::add_item( const stringx& entity_name,
									 bool a, bool stationary, bool invisible, bool cosmetic,
									 entity_id & _id, entity_flavor_t flavor )
{
	stringx ent_filename = entity_name;
	item * new_item = NEW item(ent_filename, _id, flavor, a, stationary);
	if (invisible)
		new_item->set_visible(false);
	else
		new_item->set_visible(true);
	new_item->set_flag( EFLAG_MISC_NONSTATIC, true );
	if (cosmetic && new_item->has_physical_ifc())
		new_item->physical_ifc()->set_collision_flags(0x00000000);
	add_item( new_item );
}


void world_dynamics_system::add_morphable_item( const stringx& entity_name,
											   bool a, bool stationary, bool invisible, bool cosmetic,
											   entity_id & _id, entity_flavor_t flavor )
{
	stringx ent_filename = entity_name;
	morphable_item * new_item = NEW morphable_item(ent_filename, _id, flavor, a, stationary);

	if (invisible)
		new_item->set_visible(false);
	else
		new_item->set_visible(true);

	new_item->set_flag( EFLAG_MISC_NONSTATIC, true );

	if (cosmetic && new_item->has_physical_ifc())
		new_item->physical_ifc()->set_collision_flags(0x00000000);

	add_item( new_item );
}

particle_generator* world_dynamics_system::add_particle_generator( const stringx& filename,
																  bool invisible, bool nonstatic,
																  entity_id &_id )
{
	//#pragma fixme( "this filename may need to be munged for the NEW locating system. -mkv 4/6/01" )
	particle_generator* new_pg = NEW particle_generator( filename, _id );
	new_pg->set_flag( EFLAG_MISC_NONSTATIC, true );
	new_pg->set_flag( EFLAG_MISC_RAW_NONSTATIC, true );

	new_pg->set_visible(!invisible);
	add_particle_generator(new_pg);
	return new_pg;
}


light_source *world_dynamics_system::add_light_source( const stringx& filename,
													  bool invisible, bool nonstatic,
													  entity_id &_id )
{
	light_source* new_ls = NEW light_source( filename, _id );
	if ( nonstatic )
		new_ls->set_flag( EFLAG_MISC_NONSTATIC, true );
	new_ls->set_visible(!invisible);
	add_light_source(new_ls);
	return new_ls;
}


#if defined(TARGET_PC)
#define TWO_PASS_RENDER
#endif

#if defined(TARGET_PC) && _ENABLE_WORLD_EDITOR
extern void frame_advance_all_dialogs(time_value_t t);
#endif

//int g_visible_check_count[2];
entity * g_entity;
//H extern FILE * debug_log_file;

extern rational_t g_level_time;

#ifndef BUILD_BOOTABLE
//H extern bool g_dump_frame_info;
//extern bool g_dump_pete_stuff;

bool g_render_capsule_history = false;
//H extern bool g_render_spheres;
void render_spheres();

bool g_render_frustum_pmesh=false;
bool g_render_lights=false;
bool g_render_box_triggers = false;
#endif


int rc_count=0;


extern rational_t ACTIVATION_RADIUS;
bool g_camera_out_of_world=true;


// generate error message if entity has invalid po
void check_po( entity* e )
{
	if ( e->get_abs_po().has_nonuniform_scaling() )
		warning( e->get_id().get_val() + ": non-uniform scaling is not supported" );
	else if ( e->get_colgeom() )
	{
		rational_t s = e->get_abs_po().get_scale();
		if ( s<0.99f || s>1.01f )
			warning( e->get_id().get_val() + ": scaling is not supported on entities with collision geometry" );
	}
}


bool visibility_check( const vector3d& p1, const vector3d& p2, entity *ent )
{
	// cast a ray from p1 to p2 and see if it hits the world
	vector3d hitp, hitn;
	entity *hit_entity = NULL;

	//  if ( g_world_ptr->get_the_terrain().find_intersection( p1, p2, hitp, hitn ) )

	// takes into account the terrain and beamable entities
	bool ret = find_intersection( p1, p2,
		g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->get_region(),  // visibility_check(): debug render vis check only works for 1 player (multiplayer fixme?)
		FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
		&hitp, &hitn,
		NULL, &hit_entity);
	if(ret && (ent == NULL || ent != hit_entity))
		return false;
	else
		return true;
}


//!extern int num_chars_on_screen;
extern bool g_frame_advance_called_this_game;

char g_dump_activity_info = 0;
char g_dump_render_info = 0;
char g_dump_collide_info = 0;
char g_dump_profile_info = 0;

bool g_render_markers = false;
char g_render_paths = 0;
char g_render_brains = 0;
char g_render_anim_info = 0;
char g_render_scene_anim_info = 0;
char g_render_targeting = 0;
char g_render_vis_spheres = 0;
char g_render_portals = 0;
char g_render_ladders = 0;
char g_render_collisions = 0;
//char g_render_dread_net = 0;
char g_brains_enabled = 1;

#if _ENABLE_WORLD_EDITOR
void tool_rendering();
#endif


#if defined(DEBUG)
//vector<entity *> pass0_entities;
//vector<entity *> pass1_entities;
#endif

bool g_disable_render_ents = false;
bool g_disable_lights = false;
#ifdef PROJECT_KELLYSLATER
bool g_disable_render_rgn  = true;
#else
bool g_disable_render_rgn  = false;
#endif

rational_t g_trans_max_test = 0.07f;
rational_t g_trans_min_test = 0.0f;

#if _ENABLE_WORLD_EDITOR
extern matrix4x4 dlg_w2v;
extern matrix4x4 dlg_v2vp;
#endif

extern profiler_counter profcounter_rgn_tri_rend;

struct TranslucentObj
{
	union
	{
		struct
		{
			float detail;
			entity* ent;
		} ent;
		struct
		{
			visual_rep* vrep;
			region_node* node;
		} reg;
	};
	enum
	{
		kNone,
			kEntity,
			kRegion,
	} kind;
	TranslucentObj() : kind(kNone) {}
	TranslucentObj(float detail,entity* e) : kind(kEntity) { ent.detail=detail; ent.ent=e; }
	TranslucentObj(visual_rep* r,region_node* n) : kind(kRegion) { reg.vrep=r; reg.node=n; }
	void render( camera* camera_link, render_flavor_t flavor, const vector3d& camera_pos, time_value_t world_age = 0.0f ) const
	{
		switch (kind)
		{
		case kEntity:
			{
				float entity_translucency_pct=1.0f;
				if (flavor&RENDER_TRANSLUCENT_PORTION)
				{
					if (ent.ent->get_distance_fade_ok())
					{
						rational_t d = (ent.ent->get_visual_center() - camera_pos).xz_length();
						rational_t diff =  PROJ_FAR_PLANE_D - d;
						visual_rep * visrep = ent.ent->get_vrep();
						if (diff > PROJ_FAR_PLANE_D*visrep->get_distance_fade_min_pct())
						{
							rational_t entity_translucency_pct = diff/(PROJ_FAR_PLANE_D*visrep->get_distance_fade_max_pct());
							if (entity_translucency_pct>1.0f)
								entity_translucency_pct=1.0f;
						}
					}
				}
				if ((ent.ent->get_flags()&EFLAG_MISC_NO_CLIP_NEEDED)==0)
					flavor|=RENDER_CLIPPED_FULL_DETAIL;
				ent.ent->render(camera_link, ent.detail, flavor, entity_translucency_pct);
				break;
			}
		case kRegion:
			{
				int start_polys = hw_rasta::inst()->get_poly_count();

				instance_render_info iri( reg.vrep->get_max_faces(),
					po_identity_matrix,
					world_age,
					reg.node );

				reg.vrep->render_instance(flavor |
					RENDER_CLIPPED_FULL_DETAIL |
					RENDER_REAL_POINT_LIGHTS,
					&iri);

				profcounter_rgn_tri_rend.add_count(hw_rasta::inst()->get_poly_count() - start_polys);
				break;
			}
		default:
			assert(false);
		}
	}
};

//extern profiler_timer proftimer_render_world;
extern profiler_timer proftimer_build_data;
extern profiler_timer proftimer_opaque_rgn;
extern profiler_timer proftimer_opaque_ent;
extern profiler_timer proftimer_trans_rgn;
extern profiler_timer proftimer_trans_ent;
extern profiler_timer proftimer_render_debug;

#ifdef DEBUG
bool dump_portals_to_console=false;
#endif

#include <algorithm>

float g_xpos = 0.0f;
float g_ypos = 0.0f;
float g_zpos = 20.0f;
int g_use_static_cam = 0;
float g_xup = 0.0f;
float g_yup = 1.0f;
float g_zup = 0.0f;


#ifdef NGL

#if 0
//defined(DEBUG)
static void draw_light_frustum(nglProjectorLightInfo *pProjectorLightInfo)
{
	debug_ngl_render_object.begin();
	debug_ngl_render_object.render_sphere(pProjectorLightInfo->Pos, 0.5f); // the pos of the light
	for(int i = 0; i < 8; i++)
	{
		debug_ngl_render_object.render_sphere(pProjectorLightInfo->Frustum.wp[i], 0.25f); // the 8 frustum points
	}
	debug_ngl_render_object.end();
}
#endif


// next time you change a big header file, fix this.
//   uh... what's wrong with it?
camera* g_camera_link;

nglTexture *WAVETEX_TestScreenTex( void );


void world_dynamics_system::render_debug( camera *camera_link )
{
#if !defined(BUILD_BOOTABLE)
	proftimer_render_debug.start();
	if (!g_disable_render_ents && g_render_frustum_pmesh)
	{
		render_frustum(*g_camera_link,color32(255,255,255,128));
	}

	if (g_render_lights)
	{
		vector3d cp = geometry_manager::inst()->get_camera_pos();
		vector3d cd = geometry_manager::inst()->get_camera_dir();
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{

			if ((*ri).reg->get_data())
			{
				vector<light_source*> const& lights=(*ri).reg->get_data()->get_lights();
				for (vector<light_source*>::const_iterator li=lights.begin();
				li!=lights.end();
				++li)
				{
					light_source* lp = (*li);
					if (!lp) continue;
					if (lp->has_properties())
					{





						color tmpclr(lp->get_color());
						tmpclr.clamp();
						color32 lcolor = tmpclr.to_color32();
						if (lp->get_properties().get_flavor()==LIGHT_FLAVOR_POINT)
						{
							vector3d pos = lp->get_abs_position();
							lcolor.c.a=160;
							render_sphere(pos,0.33f,lcolor);
							lcolor.c.a=100;
							render_sphere(pos,lp->get_near_range(),lcolor);
							lcolor.c.a=70;
							render_sphere(pos,lp->get_cutoff_range(),lcolor);
							if (lp->get_properties().affects_terrain())
								print_3d_text(pos, color32_white, "terrain");
							if (lp->get_properties().get_additive_color32().c.r!=0 &&
								lp->get_properties().get_additive_color32().c.g!=0 &&
								lp->get_properties().get_additive_color32().c.b!=0)
								print_3d_text(pos+vector3d(0.0f,0.5f,0.0f), color32_white, "additive");
						}
						else if (ri == to_render.regions.begin())
						{ // should only draw directional lights that are in the current camera region!
							vector3d dir = -lp->get_abs_po().get_y_facing();
							rational_t dp = -dot(dir,cd);
							if (dp>-0.5f)
							{
								if (dp<0) dp=0;
								lcolor.c.a=uint8(96*dp+32);
								render_plane(plane(cp-dir*2.0f,dir),
									lcolor);
							}
						}
					}
				}
			}
			// render small quad of current region's ambient color if you look straight down
			if (ri == to_render.regions.begin())
			{
				color tmpclr((*ri).reg->get_data()->get_ambient());
				tmpclr.clamp();
				color32 lcolor = tmpclr.to_color32();
				lcolor.c.a = 216;
				if (dot(cd,vector3d(0,-1,0))>0.9f)
					render_quad(cp+vector3d(-0.1f,-0.6f,-0.1f),
					cp+vector3d(-0.1f,-0.6f, 0.1f),
					cp+vector3d( 0.1f,-0.6f, 0.1f),
					cp+vector3d( 0.1f,-0.6f,-0.1f),
					lcolor, false);
			}
		}
	}

	if(g_render_markers)
	{

		vector3d vec;
		camera* cam = app::inst()->get_game()->get_current_view_camera();
		vector3d cam_face = cam->get_abs_po().get_facing();
		vector3d cam_pos = cam->get_abs_position();
		bool vis_check = g_camera_out_of_world;

		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			if ((*ri).reg->get_data())
			{
				vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				vector<entity*>::const_iterator i = entlist.begin();

				while (i != entlist.end())
				{
					entity *mark = (*i);
					++i;

					if(mark && (mark->get_flavor() == ENTITY_MARKER /*! || mark->get_flavor() == ENTITY_CRAWL_MARKER !*/))
					{
						vec = mark->get_abs_position();

						vector3d dir = (vec - cam_pos);
						if(dir != ZEROVEC)
							dir.normalize();
						else
							dir = cam_face;

						if(dot(cam_face, dir) > 0.0f && (vis_check || visibility_check(cam_pos, vec, mark)))
						{
							vec = xform3d_1(geometry_manager::inst()->xforms[geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW], vec);

							if(vec.z > 0.0f)
							{
								vec = xform3d_1_homog(geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_SCREEN], vec);

								render_marker(camera_link, mark->get_abs_position(), color32(255, 255, 255, 192), 0.5f);
								render_beam( mark->get_abs_position(), mark->get_abs_position() + (mark->get_abs_po().get_facing() * 0.25f), color32(0, 255, 0, 192), 0.05f);

								hw_rasta::inst()->print( mark->get_name(), vector2di(vec.x, vec.y) );
							}
						}
					}
				}
			}
		}
	}

	if(g_render_paths)
	{
		vector<path_graph *>::iterator i = path_graph_list.begin();
		while(i != path_graph_list.end())
		{
			if(*i != NULL)
				(*i)->render(camera_link,color32(0, 255, 0, 128), 0.05f, g_render_paths);

			++i;
		}

		if(world_path)
			world_path->render();
	}

	if(g_render_targeting)
	{
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			if ((*ri).reg->get_data())
			{
				vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				vector<entity*>::const_iterator i = entlist.begin();

				while (i != entlist.end())
				{
					entity *ent = (*i);
					++i;

					if(ent && ent->allow_targeting())
					{
						collision_geometry *cg = ent->get_colgeom();

						if(cg)
						{
							render_colgeom(ent, color32(255, 0, 255, 128));
						}
						else
							render_sphere(ent->get_abs_position(), ent->get_radius() > 0.0f ? ent->get_radius() : 1.0f, color32(255, 0, 255, 128));
					}
				}
			}
		}

		if(get_hero_ptr(g_game_ptr->get_active_player())->get_current_target() != NULL)
			print_3d_text(get_hero_ptr(g_game_ptr->get_active_player())->get_current_target_pos(), color32(255, 0, 0, 128), get_hero_ptr(g_game_ptr->get_active_player())->get_current_target()->get_name().c_str());

		render_sphere(get_hero_ptr(g_game_ptr->get_active_player())->get_current_target_pos(), 0.1f, color32(0, 255, 0, 128));
	}


	if(g_render_vis_spheres)
	{
		//for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri==to_render.regions.begin(); ++ri) // just current region
		{
			region_node* node = (*ri).reg;
			region* reg = node->get_data();
			if (reg)
			{
				// to cut down on overlap madness, we only render spheres that are within x units of the camera
				float nearby = 10.0f;
				if (g_render_vis_spheres & 15)
				{
					vector<entity*> const& entlist=reg->get_entities();
					vector<entity*>::const_iterator i = entlist.begin();

					while (i != entlist.end())
					{
						entity *ent = (*i);
						++i;

						if(ent && ent->is_still_visible() && ent->is_flagged(EFLAG_MISC_IN_VISIBLE_REGION))
						{
							color32 color;
							if (ent->is_a_particle_generator())
							{
								if (!(g_render_vis_spheres & 4)) continue;
								color=color32(40, 200, 40, 96);
							}
							else if (ent->is_a_light_source())
							{
								if (!(g_render_vis_spheres & 8)) continue;
								color=color32(200, 200, 200, 96);
							}
							else
							{
								if (!(g_render_vis_spheres & 1)) continue;
								color=color32(161, 0, 236, 96);
							}
							if ((ent->get_visual_center() - geometry_manager::inst()->get_camera_pos()).length() < nearby + ent->get_visual_radius())
								render_sphere(ent->get_visual_center(), ( ent->get_visual_radius() > 0.0f ) ? ent->get_visual_radius() : 1.0f, color);
						}
					}
				}
				if (g_render_vis_spheres & 16)
				{
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
					// something here?
#else
					for (int j=0; j<reg->get_num_visreps(); ++j)
					{
						visual_rep* vrep = reg->get_visrep(j);

						if ((vrep->get_center() - geometry_manager::inst()->get_camera_pos()).length() < nearby + vrep->get_radius())
							render_sphere(vrep->get_center(), ( vrep->get_radius() > 0.0f ) ? vrep->get_radius() : 1.0f, color32(236, 161, 0, 96));
					}
#endif
				}
			}
		}
	}

	if(g_render_portals)
	{
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
			//for (render_data::region_list::iterator ri=to_render.regions.begin(); ri==to_render.regions.begin(); ++ri) // just current region
		{
			region_node* node = (*ri).reg;
			region* reg = node->get_data();
			if (reg)
			{
				if (reg->is_active())
				{
					// to cut down on overlap madness, we only render spheres that are within x units of the camera
					for (region_node::iterator rni=node->begin(); rni!=node->end(); ++rni)
					{
						portal* port = (*rni).get_data();

						if (port->is_active()) // don't show closed portals
						{
							rational_t radius = port->get_effective_radius();
							vector3d cylvec = port->get_cylinder_normal()*(port->get_cylinder_depth()+0.01f);

							if ((g_render_portals & 1)!=0)
								render_cylinder(port->get_effective_center()-cylvec,
                                port->get_effective_center()+cylvec,
                                radius, color32(161, 236, 0, 96));
							if ((g_render_portals & 4)!=0)
							{
								vector<material*> altmat;
								extern game* g_game_ptr;
								altmat.push_back(g_game_ptr->get_blank_material());
								instance_render_info iri(port->get_max_faces(),
									identity_matrix,
									0,
									app::inst()->get_game()->get_current_view_camera()->get_region(),
									0,
									color32(236, 161, 0, 96),
									FORCE_TRANSLUCENCY,
									0,
									1.0f,
									NULL,
									-1,
									&altmat);
								port->render_instance(RENDER_TRANSLUCENT_PORTION, &iri);
							}
							if ((g_render_portals & 2)!=0)
								render_sphere(port->get_effective_center(), radius, color32(161, 236, 0, 56));
							if ((g_render_portals & 8)!=0)
								print_3d_text(port->get_effective_center(), color32(161, 255, 80, 192),
								"%s <-> %s", port->get_front()->get_data()->get_name().c_str(),
								port->get_back ()->get_data()->get_name().c_str());
						}
					}
				}
			}
		}
	}

	if(g_render_collisions)
	{
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			if ((*ri).reg->get_data())
			{
				vector<entity*> const& entlist=(*ri).reg->get_data()->get_possible_collide_entities();
				vector<entity*>::const_iterator i = entlist.begin();

				while (i != entlist.end())
				{
					entity *ent = (*i);
					++i;

					if ( ent && ent->are_collisions_active() )
					{
						bool scannable = ( ent->is_scannable() && ent->is_visible() && !ent->is_destroyable() && !ent->is_a_crate() );
						if ( g_render_collisions == 5
							|| (ent->has_camera_collision() && (g_render_collisions == 2 || g_render_collisions == 1))
							|| (ent->is_beamable() && (g_render_collisions == 3 || g_render_collisions == 1))
							|| (scannable && (g_render_collisions == 4 || g_render_collisions == 1))
							)
						{
							int r=0, g=0, b=0;
							if ( g_render_collisions == 5 )
							{
								r = ent->has_entity_collision() ? 255 : 0;
								g = 255;
								b = 255;
							}
							else
							{
								if ( ent->has_camera_collision() && (g_render_collisions==1 || g_render_collisions==2) )
									r = 255;
								if ( ent->is_beamable() && (g_render_collisions==1 || g_render_collisions==3) )
									g = 255;
								if ( scannable && (g_render_collisions==1 || g_render_collisions==4) )
									b = 255;
							}

							color32 col = color32(r,g,b,96);
							render_colgeom( ent, col );

							stringx flags = empty_string;

							if ( g_render_collisions <= 5 )
							{
								if ( ent->has_camera_collision() )
									flags += "C";
								if ( ent->is_scannable() )
									flags += "S";
								if ( ent->is_beamable() )
									flags += "B";
								if( ent->has_entity_collision() )
									flags += "E";
							}

							print_3d_text(ent->get_abs_position(), col, "%s (%s)", ent->get_name().c_str(), flags.c_str());
						}
					}
				}
			}
		}
	}

	if ( g_render_brains )
	{
		ai_interface::render_ai(g_render_brains);
	}

#if _VIS_ITEM_DEBUG_HELPER
	extern void render_vis_item_debug_info();
	if(g_render_vis_item_debug_info)
		render_vis_item_debug_info();
#endif

	if ( g_render_anim_info )
	{
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			if ((*ri).reg->get_data())
			{
				vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				vector<entity*>::const_iterator i = entlist.begin();

				while (i != entlist.end())
				{
					entity *ent = (*i);
					++i;

					if(ent && ent->is_visible())
					{
						if(g_render_anim_info == 1)
						{
							if(ent->is_hero() || !ent->has_ai_ifc())
								continue;
						}
						else if(g_render_anim_info == 2)
						{
							if(!ent->has_ai_ifc())
								continue;
						}

						stringx misc_str = empty_string;

						if(!ent->anim_finished(ANIM_PRIMARY))
							misc_str += stringx(stringx::fmt, "\nANIM_PRIMARY: %s (%.2f)", ent->get_anim_tree(ANIM_PRIMARY)->get_name().c_str(), ent->get_anim_tree(ANIM_PRIMARY)->get_floor_offset());
						if(!ent->anim_finished(ANIM_SECONDARY_A))
							misc_str += stringx(stringx::fmt, "\nANIM_SECONDARY_A: %s (%.2f)", ent->get_anim_tree(ANIM_SECONDARY_A)->get_name().c_str(), ent->get_anim_tree(ANIM_SECONDARY_A)->get_floor_offset());
						if(!ent->anim_finished(ANIM_SECONDARY_B))
							misc_str += stringx(stringx::fmt, "\nANIM_SECONDARY_B: %s (%.2f)", ent->get_anim_tree(ANIM_SECONDARY_B)->get_name().c_str(), ent->get_anim_tree(ANIM_SECONDARY_B)->get_floor_offset());
						if(!ent->anim_finished(ANIM_SECONDARY_C))
							misc_str += stringx(stringx::fmt, "\nANIM_SECONDARY_C: %s (%.2f)", ent->get_anim_tree(ANIM_SECONDARY_C)->get_name().c_str(), ent->get_anim_tree(ANIM_SECONDARY_C)->get_floor_offset());
						if(!ent->anim_finished(ANIM_TERTIARY_A))
							misc_str += stringx(stringx::fmt, "\nANIM_TERTIARY_A: %s (%.2f)", ent->get_anim_tree(ANIM_TERTIARY_A)->get_name().c_str(), ent->get_anim_tree(ANIM_TERTIARY_A)->get_floor_offset());
						if(!ent->anim_finished(ANIM_TERTIARY_B))
							misc_str += stringx(stringx::fmt, "\nANIM_TERTIARY_B: %s (%.2f)", ent->get_anim_tree(ANIM_TERTIARY_B)->get_name().c_str(), ent->get_anim_tree(ANIM_TERTIARY_B)->get_floor_offset());
						if(!ent->anim_finished(ANIM_TERTIARY_C))
							misc_str += stringx(stringx::fmt, "\nANIM_TERTIARY_C: %s (%.2f)", ent->get_anim_tree(ANIM_TERTIARY_C)->get_name().c_str(), ent->get_anim_tree(ANIM_TERTIARY_C)->get_floor_offset());
						if(!ent->anim_finished(ANIM_SCENE))
							misc_str += stringx(stringx::fmt, "\nANIM_SCENE: %s", ent->get_anim_tree(ANIM_SCENE)->get_name().c_str());

						if(ent->has_physical_ifc())
						{
							rational_t floor_off = ent->physical_ifc()->get_floor_offset();

							misc_str += stringx(stringx::fmt, "\nCurrent Floor: %.2f", floor_off);

							rational_t y = ent->get_abs_position().y - floor_off + 0.001f;
							vector3d ul = ent->get_abs_position() - (ent->get_abs_po().get_x_facing()*0.5f) - (ent->get_abs_po().get_z_facing()*0.5f);
							vector3d ur = ent->get_abs_position() + (ent->get_abs_po().get_x_facing()*0.5f) - (ent->get_abs_po().get_z_facing()*0.5f);
							vector3d ll = ent->get_abs_position() - (ent->get_abs_po().get_x_facing()*0.5f) + (ent->get_abs_po().get_z_facing()*0.5f);
							vector3d lr = ent->get_abs_position() + (ent->get_abs_po().get_x_facing()*0.5f) + (ent->get_abs_po().get_z_facing()*0.5f);

							ul.y = y;
							ur.y = y;
							ll.y = y;
							lr.y = y;

							render_triangle(ul, ur, lr, color32(0, 255, 0, 128), true);
							render_triangle(ul, ll, lr, color32(0, 255, 0, 128), true);
						}

						print_3d_text(ent->get_abs_position()+YVEC, color32_blue, "%s%s", ent->get_name().c_str(), misc_str.c_str());
					}
				}
			}
		}
	}

	if ( g_render_scene_anim_info )
	{
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			if ((*ri).reg->get_data())
			{
				vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				vector<entity*>::const_iterator i = entlist.begin();

				while (i != entlist.end())
				{
					entity *ent = (*i);
					++i;

					if ( ent && ent->playing_scene_anim() )
					{
						render_beam(ent->get_abs_position(), ent->get_abs_position() + (ent->get_abs_po().get_x_facing() * 2.0f), color32(255, 0, 0, 192), 0.05);
						render_beam(ent->get_abs_position(), ent->get_abs_position() + (ent->get_abs_po().get_y_facing() * 2.0f), color32(0, 255, 0, 192), 0.05);
						render_beam(ent->get_abs_position(), ent->get_abs_position() + (ent->get_abs_po().get_z_facing() * 2.0f), color32(0, 0, 255, 192), 0.05);
					}
				}
			}
		}
	}

	//  render_debug_text();

	proftimer_render_debug.stop();
#endif // !BUILD_BOOTABLE

#if defined (DEBUG)
	if (g_render_capsule_history)
		render_capsule_history();
	if (g_debug.render_spheres)
		render_debug_spheres();

#if 0 //BIGCULL
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG) && defined(TARGET_PS2)
	extern new_LineInfo g_lineinfo[4];

	render_sphere(g_lineinfo[0].StartCoords, 0.1f, color32(255, 0, 0, 128));
	render_sphere(g_lineinfo[0].EndCoords, 0.1f, color32(0, 0, 255, 128));
	render_beam(g_lineinfo[0].StartCoords, g_lineinfo[0].EndCoords, color32(0, 255, 0, 128), 0.05f);
	if(g_lineinfo[0].collision)
	{
		render_sphere(g_lineinfo[0].hit_pos, 0.1f, color32(255, 255, 255, 128));
		render_beam(g_lineinfo[0].hit_pos, g_lineinfo[0].hit_pos+g_lineinfo[0].hit_norm, color32(0, 0, 0, 128), 0.05f);
	}

	render_sphere(g_lineinfo[1].StartCoords, 0.1f, color32(255, 0, 0, 128));
	render_sphere(g_lineinfo[1].EndCoords, 0.1f, color32(0, 0, 255, 128));
	render_beam(g_lineinfo[1].StartCoords, g_lineinfo[1].EndCoords, color32(255, 255, 0, 128), 0.05f);
	if(g_lineinfo[1].collision)
	{
		render_sphere(g_lineinfo[1].hit_pos, 0.1f, color32(255, 255, 255, 128));
		render_beam(g_lineinfo[1].hit_pos, g_lineinfo[1].hit_pos+g_lineinfo[1].hit_norm, color32(0, 0, 0, 128), 0.05f);
	}

	render_sphere(g_lineinfo[2].StartCoords, 0.1f, color32(255, 0, 0, 128));
	render_sphere(g_lineinfo[2].EndCoords, 0.1f, color32(0, 0, 255, 128));
	render_beam(g_lineinfo[2].StartCoords, g_lineinfo[2].EndCoords, color32(0, 255, 255, 128), 0.05f);
	if(g_lineinfo[2].collision)
	{
		render_sphere(g_lineinfo[2].hit_pos, 0.1f, color32(255, 255, 255, 128));
		render_beam(g_lineinfo[2].hit_pos, g_lineinfo[2].hit_pos+g_lineinfo[2].hit_norm, color32(0, 0, 0, 128), 0.05f);
	}

	render_sphere(g_lineinfo[3].StartCoords, 0.1f, color32(255, 0, 0, 128));
	render_sphere(g_lineinfo[3].EndCoords, 0.1f, color32(0, 0, 255, 128));
	render_beam(g_lineinfo[3].StartCoords, g_lineinfo[3].EndCoords, color32(255, 255, 255, 128), 0.05f);
	if(g_lineinfo[3].collision)
	{
		render_sphere(g_lineinfo[3].hit_pos, 0.1f, color32(255, 255, 255, 128));
		render_beam(g_lineinfo[3].hit_pos, g_lineinfo[3].hit_pos+g_lineinfo[3].hit_norm, color32(0, 0, 0, 128), 0.05f);
	}

	// BIGCULL g_spiderman_controller_ptr->render_lookaround_reticle();
#endif
#endif// BIGCULL

#endif
}

//nglVector &WAVETEX_GetSunPos(void);
//void WAVE_SaveTex( camera &cam );

//float WDS_lightscale=3.75;
//extern matrix4x4 WAVETEX_suntolit;

bool g_dump_rendered_ents = false;
//bool WAVETEX_ProjectThisLight( light_source *lp );
void WAVETEX_ProjectLight( light_source *lp );
//nglVector WDS_projScale;

void world_dynamics_system::render(camera* camera_link, const int heroIdx ) //, const bool underwaterflag )
{
	extern bool g_environment_maps_enabled;
	extern bool g_decal_maps_enabled;
	extern bool g_detail_maps_enabled;

	g_environment_maps_enabled = !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_ENV_PASS);
	g_decal_maps_enabled = !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_DECAL_PASS);
	g_detail_maps_enabled = !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_DETAIL_PASS);

	g_camera_link = camera_link;

	START_PROF_TIMER( proftimer_build_data );
	START_PROF_TIMER( proftimer_build_data_clear );

	// clear the previous frame's rendered entities list.
	render_data::region_list::iterator ri;
	for (ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
	{
		region_node* rn = (*ri).reg;

		const region::entity_list * ralist;
		region::entity_list::const_iterator rali, rali_begin, rali_end;

		ralist = &rn->get_data()->get_entities();
		rali_begin = ralist->begin();
		rali_end = ralist->end();

		for ( rali=rali_begin; rali!=rali_end; ++rali )
		{
			entity* ent = *rali;
			if(ent)
				ent->set_flag( EFLAG_MISC_IN_VISIBLE_REGION, false );
		}
	}

	to_render.clear();


	STOP_PROF_TIMER( proftimer_build_data_clear );
	START_PROF_TIMER( proftimer_det_vis_rgn );

	// build rendering lists starting in region containing camera

	// ***temporary solution; this should really be handled at the level of
	// class entity, and accessed through the camera entity instead of the
	// geometry_manager
	camera_link->compute_sector(*the_terrain);
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_BSP_SPRAY_PAINT) )
	{
		entity * sf = entity_manager::inst()->find_entity("SPACE_FILLER", IGNORE_FLAVOR);
		sf->set_rel_position(get_hero_ptr(g_game_ptr->get_active_player())->get_abs_position());	// wds::render(): wtf is this?  (multiplayer fixme)
		sf->compute_sector(*the_terrain);
		sf->set_active(true);
		sf->set_visible(true);
	}

	region_node* camera_region = camera_link->get_region();
	if ( camera_region == NULL )
		camera_region = get_hero_ptr(heroIdx)->get_region();	// wds::render(): worthless BSP region code?  (multiplayer fixme)
	if ( !camera_region )
		return;

	if (g_use_static_cam)
	{
		vector3d lookfrom(g_xpos, g_ypos, g_zpos);
		vector3d lookto(0.0f, 0.0f, 0.0f);
		vector3d upvector(g_xup, g_yup, g_zup);
		geometry_manager::inst()->set_view( lookfrom, lookto, upvector );
	}

#if defined(DEBUG) && defined(TARGET_XBOX)
static bool old_render_cube_map = false;
extern void nglInitShaders();
	if (!g_render_cube_map && old_render_cube_map)
	{
		nglInitShaders();	// reset the view to screen transform --- must wait one frame to change this back
	}
	old_render_cube_map = (g_render_cube_map != 0);

	if (g_render_cube_map)
	{
		extern float nglAspectRatio;
		static float RealAspectRatio;
		extern int nglScreenWidthTV, nglScreenHeightTV;
		extern int nglScreenXOffsetTV, nglScreenYOffsetTV;
		static int RealWidthTV, RealHeightTV;
		static int RealXOffsetTV, RealYOffsetTV;
		if (g_render_cube_map == 1)
		{
			RealAspectRatio = nglAspectRatio;
			RealWidthTV = nglScreenWidthTV;
			RealHeightTV = nglScreenHeightTV;
			RealXOffsetTV = nglScreenXOffsetTV;
			RealYOffsetTV = nglScreenYOffsetTV;

			// remove screen buffer margins
			nglScreenWidthTV = nglGetScreenWidth();
			nglScreenHeightTV = nglGetScreenHeight();
			nglScreenXOffsetTV = 0;
			nglScreenYOffsetTV = 0;
			nglInitShaders();	// reset the view to screen transform

			// make the screen square
			nglAspectRatio = (float)nglGetScreenHeight() / (float)nglGetScreenWidth();
		}
		ksnglSetPerspectiveMatrix(90, nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 65536.0f);
		static nglVector Offset(0, -5, 0, 1);
		static char *TexNames[6] = {	// Max naming convention (dc 04/19/02)
			"CubeEnv_RT",
			"CubeEnv_LF",
			"CubeEnv_UP",
			"CubeEnv_DN",
			"CubeEnv_BK",
			"CubeEnv_FR",
		};
		static nglMatrix CubeMatrix[6] = {
			nglMatrix(
				// right face; XP
				nglVector(0, 0, 1, 0),
				nglVector(0, 1, 0, 0),
				nglVector(-1, 0, 0, 0),
				nglVector(0, 0, 0, 1)
			),
			nglMatrix(
				// left face; XN
				nglVector(0, 0, -1, 0),
				nglVector(0, 1, 0, 0),
				nglVector(1, 0, 0, 0),
				nglVector(0, 0, 0, 1)
			),
			nglMatrix(
				// top face; YP
				nglVector(1, 0, 0, 0),
				nglVector(0, 0, 1, 0),
				nglVector(0, -1, 0, 0),
				nglVector(0, 0, 0, 1)
			),
			nglMatrix(
				// bottom face; YN
				nglVector(1, 0, 0, 0),
				nglVector(0, 0, -1, 0),
				nglVector(0, 1, 0, 0),
				nglVector(0, 0, 0, 1)
			),
			nglMatrix(
				// back face; ZP
				nglVector(1, 0, 0, 0),
				nglVector(0, 1, 0, 0),
				nglVector(0, 0, 1, 0),
				nglVector(0, 0, 0, 1)
			),
			nglMatrix(
				// front face; ZN
				nglVector(-1, 0, 0, 0),
				nglVector(0, 1, 0, 0),
				nglVector(0, 0, -1, 0),
				nglVector(0, 0, 0, 1)
			),
		};
		nglMatrix Identity, Trans;
		nglIdentityMatrix(Identity);
		KSNGL_TranslateMatrix(Trans, Identity, Offset);
		nglMatrix WorldToView;
		nglMulMatrix(WorldToView, CubeMatrix[g_render_cube_map - 1], Trans);
		nglSetWorldToViewMatrix(WorldToView);
		g_screenshot = true;
		g_screenshot_filename = TexNames[g_render_cube_map - 1];
		g_render_cube_map = (g_render_cube_map + 1) % 7;
		if (g_render_cube_map == 0)
		{
			nglAspectRatio = RealAspectRatio;
			nglScreenWidthTV = RealWidthTV;
			nglScreenHeightTV = RealHeightTV;
			nglScreenXOffsetTV = RealXOffsetTV;
			nglScreenYOffsetTV = RealYOffsetTV;
		}
	}
	else
#endif	// #if defined(DEBUG) && defined(TARGET_XBOX)
	{
		if(!g_game_ptr->is_splitscreen())
			ksnglSetPerspectiveMatrix( proj_field_of_view_in_degrees(), nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 65536.0f );
		nglSetWorldToViewMatrix(
			native_to_ngl( geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW] ) );
	}

	the_terrain->deactivate_all_regions();

	region::prepare_for_visiting();
	rectf screen_rect( -1, -1, 1, 1 );
	_build_render_data_regions( to_render, camera_region, screen_rect, *camera_link );

#ifdef DEBUG
    dump_portals_to_console=false;
#endif

	//    _build_render_data_ents( to_render );

	// Setting up stuff for use by render batch
	// this viewport matrix isn't the same as the one in the d3d docs
	// but it actually works and makes sense
	//w2v  = geometry_manager::inst()->xforms[geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW];
	//v2vp = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_VIEWPORT];

#if _ENABLE_WORLD_EDITOR
	dlg_w2v  = geometry_manager::inst()->xforms[geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW];
	dlg_v2vp = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_SCREEN];
#endif

	vector3d campos = camera_link->get_abs_position();

	vector3d hit_loc;
	g_camera_out_of_world = !in_world(campos, 0.25f, ZEROVEC, camera_link->get_region(), hit_loc);

	STOP_PROF_TIMER( proftimer_det_vis_rgn );
	STOP_PROF_TIMER( proftimer_build_data );

		// this is an extra pass to add the projector lights to the
	  //    root light context so the shadow shows up on the wave -EO
	if (!g_disable_lights && FEDone())
	{

		// Add projector lights to the render list.
		START_PROF_TIMER( proftimer_ent_lights );
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			if (!(*ri).reg->get_data())
				continue;
			vector<light_source*> const& lights=(*ri).reg->get_data()->get_lights();
			vector<light_source*>::const_iterator li=lights.begin();
			for ( ; li!=lights.end(); ++li )
			{
				light_source* lp = (*li);
				if (!lp)
					continue;
				if (!lp->has_properties())
					continue;

				WAVETEX_ProjectLight( lp );
			}
    }
    STOP_PROF_TIMER( proftimer_ent_lights );
  }

	// Lights must now come before entities, since we need to create the nglLightContexts before using
	// them.
	if (!g_disable_lights)// && FEDone())
	{
		// Apply these lights only to this viewport. (dc 07/12/02)
		current_light_context = nglCreateLightContext();

		// Add lights to the render list.
		START_PROF_TIMER( proftimer_ent_lights );
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			if (!(*ri).reg->get_data())
				continue;
			vector<light_source*> const& lights=(*ri).reg->get_data()->get_lights();
			vector<light_source*>::const_iterator li=lights.begin();
			for ( ; li!=lights.end(); ++li )
			{
				light_source* lp = (*li);
				if (!lp)
					continue;
				if (!lp->has_properties())
					continue;

					// only the wave has shadows
					//   don't bother adding projector lights here
				//if(FEDone()) WAVETEX_ProjectLight( lp );
				// there's something amiss in the light properties
				u_int lightcat;
				nglVector lightcolor;
				if (lp->get_properties().get_flavor()==LIGHT_FLAVOR_POINT)
					lightcat = NGLLIGHT_LIGHTCAT_3;
				else
					lightcat = NGLLIGHT_LIGHTCAT_1;
				//linfo.Flags = lp->get_lightcat();
				float DistFalloffStart = lp->get_near_range();
				float DistFalloffEnd = lp->get_cutoff_range();
				color add = lp->get_properties().get_additive_color();
				lightcolor[0] = add.r;
				lightcolor[1] = add.g;
				lightcolor[2] = add.b;
				lightcolor[3] = add.a;
				/*	Mult color no longer supported in NGL (dc 12/06/01)
				Temporary hack to simulate.
				*/
				if(FEDone())
				{
					color mul = lp->get_properties().get_color();
					color amb = get_hero_ptr(heroIdx)->get_light_set()->last_ambient;
					amb = amb * get_hero_ptr(heroIdx)->get_render_color().to_color();
					lightcolor[0] += (1 + add.r) * mul.r * amb.r;
					lightcolor[1] += (1 + add.g) * mul.g * amb.g;
					lightcolor[2] += (1 + add.b) * mul.b * amb.b;
					lightcolor[3] += (1 + add.a) * mul.a * amb.a;
				}
				else
				{
					color mul = lp->get_properties().get_color();
					color amb = color(1, 1, 1, 1);
					lightcolor[0] += (1 + add.r) * mul.r * amb.r;
					lightcolor[1] += (1 + add.g) * mul.g * amb.g;
					lightcolor[2] += (1 + add.b) * mul.b * amb.b;
					lightcolor[3] += (1 + add.a) * mul.a * amb.a;
				}


				nglVector LightPos(0, 0, 0, 0);
				nglVector LightDir(0, 0, 0, 0);
				if (lp->get_properties().get_flavor()==LIGHT_FLAVOR_POINT)
				{
					vector3d pos = lp->get_abs_position();
					LightPos[0] = pos.x;
					LightPos[1] = pos.y;
					LightPos[2] = pos.z;
					LightPos[3] = 1.0f;
					nglListAddFakePointLight( lightcat, LightPos, DistFalloffStart, DistFalloffEnd, lightcolor );
				}
				else if (ri == to_render.regions.begin())
				{
					// should add draw directional lights that are in the current camera region!
					vector3d dir = -lp->get_abs_po().get_y_facing();
					LightDir[0] = dir.x;
					LightDir[1] = dir.y;
					LightDir[2] = dir.z;
					LightDir[3] = 1.0f;
					nglListAddDirLight( lightcat, LightDir, lightcolor );
				}
			}
    }
    STOP_PROF_TIMER( proftimer_ent_lights );
  }

	START_PROF_TIMER( proftimer_opaque_rgn );
	if( !g_disable_render_rgn )
	{
		render_data::region_list::iterator ri_end = to_render.regions.end();
		for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=ri_end; ++ri)
		{
			region* reg=(*ri).reg->get_data();

			for (int i=reg->get_num_meshes(); --i>=0; )
			{
				nglMesh* mesh = reg->get_mesh(i);
				if ( mesh )
				{
					START_PROF_TIMER( proftimer_render_add_mesh );
					nglListAddMesh(mesh, native_to_ngl( identity_matrix ), NULL);
					STOP_PROF_TIMER( proftimer_render_add_mesh );
				}
			}
		}
	}
	STOP_PROF_TIMER( proftimer_opaque_rgn );

	if (!g_disable_render_ents)
	{
		START_PROF_TIMER( proftimer_opaque_ent );
		//      static rational_t max_dist = 200.0f;
		//      vector3d cam_face = camera_link.get_abs_po().get_facing();
		static rational_t cos_60 = cos(DEG_TO_RAD(60.0f));

		render_data::region_list::iterator ri;
		entity::prepare_for_visiting();
		for (ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		{
			region_node* rn = (*ri).reg;

			const region::entity_list * ralist;
			region::entity_list::const_iterator rali, rali_begin, rali_end;

			ralist = &rn->get_data()->get_possible_render_entities();
			rali_begin = ralist->begin();
			rali_end = ralist->end();

			ADD_PROF_COUNT(profcounter_poss_render_ents, ralist->size());

			if ( g_dump_rendered_ents )
				debug_print( "region: %s", rn->get_data()->get_name().c_str() );

			bool underflag=UNDERWATER_CameraUnderwater(heroIdx);

			if ( !underflag )
			{
				nglListBeginScene();
				ksnglSceneParamsFromParent();
			}
			for ( rali=rali_begin; rali!=rali_end; ++rali )
			{
				entity* ent = *rali;
				if (ent &&
					!ent->already_visited() &&
					ent->is_still_visible() &&
					underflag != ent->is_in_underwater_scene() &&
					ent->is_flagged(EFLAG_GRAPHICS) &&
					( ent->get_vrep() || ent->has_mesh() || ent->get_flavor() == ENTITY_POLYTUBE || ent->get_flavor() == ENTITY_LENSFLARE)
					)
				{
					ent->visit();

					// Splitscreen: only draw 1 player per side.
					assert(MAX_PLAYERS == 2);
					if (g_game_ptr->is_splitscreen())
					{
						if (heroIdx == 0 && (ent == hero_ptr[1] || ent == ks_controller[1]->GetBoardMember())) continue;
						if (heroIdx == 1 && (ent == hero_ptr[0] || ent == ks_controller[0]->GetBoardMember())) continue;
					}

					ent->set_flag( EFLAG_MISC_IN_VISIBLE_REGION, true );

					if ( g_dump_rendered_ents )
						debug_print( "ent: %s", ent->get_id().get_val().c_str() );

					START_PROF_TIMER( proftimer_render_entity );
					ent->render(camera_link, 1.0f, RENDER_OPAQUE_PORTION | RENDER_TRANSLUCENT_PORTION, 1.0f);
					STOP_PROF_TIMER( proftimer_render_entity );

					ADD_PROF_COUNT(profcounter_rendered_ents, 1);
				}
			}
			if ( !underflag )
			{
				nglListEndScene();
				nglSetClearFlags(0);
			}
			for ( rali=rali_begin; rali!=rali_end; ++rali )
			{
				entity* ent = *rali;
				if (ent &&
					!ent->already_visited() &&
					ent->is_still_visible() &&
					underflag == ent->is_in_underwater_scene() &&
					ent->is_flagged(EFLAG_GRAPHICS) &&
					( ent->get_vrep() || ent->has_mesh() || ent->get_flavor() == ENTITY_POLYTUBE || ent->get_flavor() == ENTITY_LENSFLARE)
					)
				{
					ent->visit();

					// Splitscreen: only draw 1 player per side.
					assert(MAX_PLAYERS == 2);
					if (g_game_ptr->is_splitscreen())
					{
						if (heroIdx == 0 && (ent == hero_ptr[1] || ent == ks_controller[1]->GetBoardMember())) continue;
						if (heroIdx == 1 && (ent == hero_ptr[0] || ent == ks_controller[0]->GetBoardMember())) continue;
					}

					ent->set_flag( EFLAG_MISC_IN_VISIBLE_REGION, true );

					if ( g_dump_rendered_ents )
						debug_print( "ent: %s", ent->get_id().get_val().c_str() );

					START_PROF_TIMER( proftimer_render_entity );
					ent->render(camera_link, 1.0f, RENDER_OPAQUE_PORTION | RENDER_TRANSLUCENT_PORTION, 1.0f);
					STOP_PROF_TIMER( proftimer_render_entity );

					ADD_PROF_COUNT(profcounter_rendered_ents, 1);
				}
			}
		}
		STOP_PROF_TIMER( proftimer_opaque_ent );

		// BIGCULL g_spiderman_controller_ptr->update_lookaround_reticle();
	}

	current_light_context = NULL;	// Make sure the stale value doesn't carry over. (dc 07/12/02)

  // render debugging info.
  render_debug(camera_link);

#if _ENABLE_WORLD_EDITOR
  tool_rendering();
#endif

  START_PROF_TIMER( proftimer_kelly_slater );
  if(FEDone()) render_kelly_slater_stuff(heroIdx);
  STOP_PROF_TIMER( proftimer_kelly_slater );


#ifndef BUILD_BOOTABLE
  if (g_debug.dump_frame_info)
  {
	  dump_frame_info();
  }
#endif
  //WAVE_SaveTex(camera_link);

}

nglLightContext *world_dynamics_system::set_current_light_context(nglLightContext *new_lc)
{
	nglLightContext *old_lc = current_light_context;
	current_light_context = new_lc;
	return old_lc;
}
#else


//
//
//  DEAD CODE?
//
//

void world_dynamics_system::render(camera* camera_link)
{
	extern bool g_environment_maps_enabled;
	extern bool g_decal_maps_enabled;
	extern bool g_detail_maps_enabled;

	g_environment_maps_enabled = !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_ENV_PASS);
	g_decal_maps_enabled = !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_DECAL_PASS);
	g_detail_maps_enabled = !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_DETAIL_PASS);

	to_render.resize(0);

	// build rendering lists starting in region containing camera

	// ***temporary solution; this should really be handled at the level of
	// class entity, and accessed through the camera entity instead of the
	// geometry_manager
	camera_link.compute_sector(*the_terrain);
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_BSP_SPRAY_PAINT) )
	{
		entity * sf = entity_manager::inst()->find_entity("SPACE_FILLER", IGNORE_FLAVOR);
		sf->set_rel_position(get_hero_ptr()->get_abs_position());
		sf->compute_sector(*the_terrain);
		sf->set_active(true);
		sf->set_visible(true);
	}
	region_node* camera_region = camera_link.get_region();
	if ( camera_region == NULL )
		camera_region = get_hero_ptr()->get_region();

	if ( camera_region )
	{
		proftimer_build_data.start();

		the_terrain->deactivate_all_regions();
		region::prepare_for_visiting();

		// clear all entities' IN_VISIBLE_REGION flag
		entity_list::iterator ei = entities.begin();
		entity_list::iterator ei_end = entities.end();
		for ( ; ei!=ei_end; ++ei)
		{
			if (*ei)
				(*ei)->set_flag( EFLAG_MISC_IN_VISIBLE_REGION, false );
		}
		vector<item*>::iterator ii = items.begin();
		vector<item*>::iterator ii_end = items.end();
		for ( ; ii!=ii_end; ++ii)
		{
			if (*ii)
				(*ii)->set_flag( EFLAG_MISC_IN_VISIBLE_REGION, false );
		}

		/*
		entity_list::iterator si = special_fx.begin();
		entity_list::iterator si_end = special_fx.end();
		for ( ; si!=si_end; ++si)
		{
		assert(*si);
		if (*si)
        (*si)->set_flag( EFLAG_MISC_IN_VISIBLE_REGION, false );
		}
		*/

		rectf screen_rect( -1, -1, 1, 1 );

		/*
		if(g_spiderman_ptr->is_ceiling_camera())
		{
		render_data::region_info temp_ri;
		temp_ri.reg = g_spiderman_ptr->get_primary_region();

		  render_data::region_list::iterator rii = find( to_render.regions.begin(), to_render.regions.end(), temp_ri );
		  if ( rii == to_render.regions.end() ) // first time seeing this region
		  {
		  // add NEW region to list (along with associated screen rect)
		  render_data::region_info ris;
		  ris.reg=temp_ri.reg;
		  ris.screen_rect = screen_rect;
		  to_render.regions.push_back(ris);
		  }
		  }
		  else
		*/
		_build_render_data_regions( to_render, camera_region, screen_rect, camera_link );

#ifdef DEBUG
		dump_portals_to_console=false;
#endif

		_build_render_data_ents( to_render );

		proftimer_build_data.stop();

		// render all opaque polygons first
		hw_rasta::inst()->send_start( hw_rasta::PT_OPAQUE_POLYS );

		// Setting up stuff for use by render batch
		// this viewport matrix isn't the same as the one in the d3d docs
		// but it actually works and makes sense
		//w2v  = geometry_manager::inst()->xforms[geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW];
		//v2vp = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_VIEWPORT];

#if _ENABLE_WORLD_EDITOR
		dlg_w2v  = geometry_manager::inst()->xforms[geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW];
		dlg_v2vp = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_SCREEN];
#endif

#ifdef TWO_PASS_RENDER
		typedef multimap<float,TranslucentObj> TranslucentMap;
		TranslucentMap translucents; // used to sort translucent entities and terrain back to front
#endif

		vector3d campos = camera_link.get_abs_position();
		vector3d cam2hero = get_hero_ptr()->get_abs_position() - campos;
		rational_t cam2hero_len2 = cam2hero.length2();
		cam2hero.normalize();

		vector3d hit_loc;
		g_camera_out_of_world = !in_world(campos, 0.25f, ZEROVEC, camera_link.get_region(), hit_loc);

		proftimer_opaque_rgn.start();
		if( !g_disable_render_rgn )
		{
			for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
			{
				region_node* regnode=(*ri).reg;
				region* reg=regnode->get_data();

				for (int i=reg->get_num_visreps(); --i>=0; )
				{
					visual_rep* vr = reg->get_visrep(i);
					// This is ok because the terrain is all already in world coordinates.  ;)
					if (/*!g_spiderman_ptr->is_ceiling_camera() && */!reg->frustum().includes(sphere(vr->get_center(),vr->get_radius())))
						continue;

					TranslucentObj tobj(vr,regnode);
#ifdef TWO_PASS_RENDER
					render_flavor_t passes=vr->render_passes_needed();
					if (passes&RENDER_TRANSLUCENT_PORTION)
					{
						// we need to sort these translucent regions by distance to camera squared.
						vector3d ctr=vr->get_center(0.0F);
						float dist2=(ctr-campos).length2();
						translucents.insert(TranslucentMap::value_type(-dist2,tobj));
					}
					if (passes&RENDER_OPAQUE_PORTION)
#endif
						tobj.render(RENDER_OPAQUE_PORTION
#ifndef TWO_PASS_RENDER
						| RENDER_TRANSLUCENT_PORTION
#endif
						, campos, world_clock.elapsed() );
				}
			}
		}

		proftimer_opaque_rgn.stop();
		if (!g_disable_render_ents)
		{
			proftimer_opaque_ent.start();
			render_data::entity_list::iterator pei = to_render.entities.begin();
			render_data::entity_list::iterator pei_end = to_render.entities.end();

			//      int sz = 0;
			for ( ; pei!=pei_end; ++pei)
			{
				entity* ent = (*pei).ent;
				if(ent)
				{
#ifdef DEBUG
					if( g_debug_slow_ass )
					{
						if ( stricmp( ent->get_id().get_val().c_str(), g_debug_entity_id_name )==0 )
						{
							debug_print("Rendering entity");
						}
						if ( ent->get_id().get_numerical_val()==g_debug_entity_id )
						{
							debug_print("Rendering entity");
						}
					}
#endif

					//          ++sz;
					rational_t detail = (*pei).extent;

					if ( ent->get_vrep() )
						detail = min(detail,(float)ent->get_max_polys());
					else
						detail = 1.0f;

#if 0  // this code fades conglomerates between the hero and the camera, causing sorting problems - LZ 7/19/01
					if(ent->is_alive() && ent->has_ai_ifc())
					{
						color32 new_color = ent->get_render_color();

						if(!ent->is_hero() && !ent->playing_scene_anim() && !marky_cam_enabled && os_developer_options::inst()->get_camera_state() == 0)
						{
							vector3d cam2ent = get_hero_ptr()->get_abs_position() - ent->get_abs_position();
							rational_t len2 = cam2ent.length2();

							if(len2 > 0.1f)// && len2 < cam2hero_len2)
							{
								len2 = __fsqrt(len2);
								cam2ent /= len2;

								rational_t dotp = dot(cam2ent, cam2hero);
								if(dotp > 0.75f)
								{
									float mod = 1.75f - dotp;
									mod *= mod;
									new_color.set_alpha((unsigned char)((255.0f*mod)+0.5f));
								}
								else
									new_color.set_alpha(255);
							}
							else
								new_color.set_alpha(255);
						}
						else // if (!ent->is_hero())
							new_color.set_alpha(255);

						ent->set_render_color(new_color);
					}
#endif

#ifdef TWO_PASS_RENDER
					render_flavor_t passes=ent->render_passes_needed();
					vector3d vectocamera = ent->get_visual_center() - campos;

					if(ent->get_flavor() != ENTITY_POLYTUBE)
					{
						visual_rep * my_visrep = ent->get_vrep();
						// I'd like to put this stuff into render_passes_needed.  --Sean
						// Well, then you should have joined the Spider-Man team.  --jdf
						if (ent->get_distance_fade_ok())
						{
							rational_t d = vectocamera.xz_length();
							rational_t diff =  PROJ_FAR_PLANE_D - d;
							if (diff > PROJ_FAR_PLANE_D*my_visrep->get_distance_fade_min_pct())
								passes|=RENDER_TRANSLUCENT_PORTION;
						}
					}

					if (passes&RENDER_TRANSLUCENT_PORTION)
						translucents.insert(TranslucentMap::value_type(-(vectocamera.length2()/*-sqr(ent->get_radius())*/), TranslucentObj(detail,ent)));
					if (passes&RENDER_OPAQUE_PORTION)
#endif
					{
						render_flavor_t flavor=0;

						if ((ent->get_flags()&EFLAG_MISC_NO_CLIP_NEEDED)==0)
							flavor|=RENDER_CLIPPED_FULL_DETAIL;

						ent->render(detail,
							flavor | RENDER_OPAQUE_PORTION
#ifndef TWO_PASS_RENDER
							| RENDER_TRANSLUCENT_PORTION
#endif
							, 1.0f);
					}
				}
			}

			proftimer_opaque_ent.stop();

#ifdef TWO_PASS_RENDER
			hw_rasta::inst()->send_start( hw_rasta::PT_TRANS_POLYS );

			// now render translucent stuff
			//proftimer_trans_rgn.start();
			proftimer_trans_ent.start();

			for (TranslucentMap::iterator ti=translucents.begin();
			ti!=translucents.end();
			++ti)
			{
				const TranslucentObj& to=(*ti).second;
#ifdef DEBUG
				/*        if( g_debug_slow_ass )
				if ( stricmp( to.ent.ent->get_id().get_val().c_str(), g_debug_entity_id_name )==0 )
				{
				debug_print("Rendering entity");
				}
				*/
#endif
				to.render(RENDER_TRANSLUCENT_PORTION,campos);
			}

			proftimer_trans_ent.stop();

#endif // TWO_PASS_RENDER

			// BIGCULL g_spiderman_controller_ptr->update_lookaround_reticle();


#if !defined(BUILD_BOOTABLE)
			proftimer_render_debug.start();
			if (!g_disable_render_ents && g_render_frustum_pmesh)
			{
				render_frustum(camera_link,color32(255,255,255,128));
			}

			if (g_render_lights)
			{
				vector3d cp = geometry_manager::inst()->get_camera_pos();
				vector3d cd = geometry_manager::inst()->get_camera_dir();
				for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
				{
					if ((*ri).reg->get_data())
					{
						vector<light_source*> const& lights=(*ri).reg->get_data()->get_lights();
						for (vector<light_source*>::const_iterator li=lights.begin();
						li!=lights.end();
						++li)
						{
							light_source* lp = (*li);
							if (!lp) continue;
							if (lp->has_properties())
							{
								// begin stolen verbatim from MR
								nglProjectorLightInfo plinfo;

								if (1) //lp->get_properties().get_flavor() == LIGHT_FLAVOR_PROJECTED_DIRECTIONAL)
								{
									plinfo.Type = NGLLIGHT_PROJECTED_DIRECTIONAL;

									//#pragma fixme("***** Streaming textures for projected lights needs to be fixed ASAP Wade!!! *****")

									//nglSetTexturePath( "textures\\ps2\\" );

									plinfo.Texture = WAVETEX_TestScreenTex();  //nglGetTextureA(name);
#if 0
									vector3d pos = lp->get_abs_position();

									LightPos[0] = pos.x;
									LightPos[1] = pos.y;
									LightPos[2] = pos.z;
									LightPos[3] = 1.0f;

									vector3d z_axis = lp->get_properties().get_z_axis();//lp->get_abs_po().get_z_facing();

									LightDir[0] = z_axis.x;
									LightDir[1] = z_axis.y;
									LightDir[2] = z_axis.z;
									LightDir[3] = 0.0f;

									vector3d x_axis = lp->get_properties().get_x_axis();

									linfo.Xaxis[0] = x_axis.x;
									linfo.Xaxis[1] = x_axis.y;
									linfo.Xaxis[2] = x_axis.z;
									linfo.Xaxis[3] = 0.0f;

									vector3d y_axis = lp->get_properties().get_y_axis();

									linfo.Yaxis[0] = y_axis.x;
									linfo.Yaxis[1] = y_axis.y;
									linfo.Yaxis[2] = y_axis.z;
									linfo.Yaxis[3] = 0.0f;

									linfo.Zaxis[0] = z_axis.x;
									linfo.Zaxis[1] = z_axis.y;
									linfo.Zaxis[2] = z_axis.z;
									linfo.Zaxis[3] = 0.0f;
#endif
									vector3d scale = vector3d(1,1,1); //lp->get_properties().get_scale();

									plinfo.Scale[0] = scale.x;
									plinfo.Scale[1] = scale.y;
									plinfo.Scale[2] = scale.z;
									plinfo.Scale[3] = 1.0f;

									nglListAddProjectorLight( &plinfo, (nglMatrix)&lp->get_abs_po_ptr()->get_matrix() );

#if defined(DEBUG)
									//draw_light_frustum(&plinfo);
#endif
								}

								// end stolen verbatim from MR
								color tmpclr(lp->get_color());
								tmpclr.clamp();
								color32 lcolor = tmpclr.to_color32();
								if (lp->get_properties().get_flavor()==LIGHT_FLAVOR_POINT)
								{
									vector3d pos = lp->get_abs_position();
									lcolor.c.a=160;
									render_sphere(pos,0.33f,lcolor);
									lcolor.c.a=100;
									render_sphere(pos,lp->get_near_range(),lcolor);
									lcolor.c.a=70;
									render_sphere(pos,lp->get_cutoff_range(),lcolor);
									if (lp->get_properties().affects_terrain())
										print_3d_text(pos, color32_white, "terrain");
									if (lp->get_properties().get_additive_color32().c.r!=0 &&
										lp->get_properties().get_additive_color32().c.g!=0 &&
										lp->get_properties().get_additive_color32().c.b!=0)
										print_3d_text(pos+vector3d(0.0f,0.5f,0.0f), color32_white, "additive");
								}
								else if (ri == to_render.regions.begin())
								{ // should only draw directional lights that are in the current camera region!
									vector3d dir = -lp->get_abs_po().get_y_facing();
									rational_t dp = -dot(dir,cd);
									if (dp>-0.5f)
									{
										if (dp<0) dp=0;
										lcolor.c.a=uint8(96*dp+32);
										render_plane(plane(cp-dir*2.0f,dir),
											lcolor);
									}
								}
              }
            }
          }
          // render small quad of current region's ambient color if you look straight down
          if (ri == to_render.regions.begin())
          {
			  color tmpclr((*ri).reg->get_data()->get_ambient());
			  tmpclr.clamp();
			  color32 lcolor = tmpclr.to_color32();
			  lcolor.c.a = 216;
			  if (dot(cd,vector3d(0,-1,0))>0.9f)
				  render_quad(cp+vector3d(-0.1f,-0.6f,-0.1f),
				  cp+vector3d(-0.1f,-0.6f, 0.1f),
				  cp+vector3d( 0.1f,-0.6f, 0.1f),
				  cp+vector3d( 0.1f,-0.6f,-0.1f),
				  lcolor, false);
          }
        }
      }

      if( g_render_box_triggers ) {
		  // for now, render boxes
		  render_data::region_list::iterator ri;

		  for( ri = to_render.regions.begin( ); ri != to_render.regions.end( ); ri++ ) {
			  // yoiks, this syntax is goofy
			  region* reg = (*ri).reg->get_data( );
			  const region::entity_list& ents = reg->get_entities( );
			  region::entity_list::const_iterator ei;

			  for( ei = ents.begin( ); ei != ents.end( ); ei++ ) {
				  entity* e = (*ei);

				  if( !e ) {
					  continue;
				  }

				  if( e->has_box_trigger_ifc( ) ) {
					  // render_beam_box
					  box_trigger_interface* bti = e->box_trigger_ifc( );
					  convex_box& box = bti->get_box_info( );
					  vector3d min = box.bbox.vmin;
					  vector3d max = box.bbox.vmax;
					  vector3d pos = e->get_abs_position( );
					  min += pos;
					  max += pos;
					  render_beam_box( min, max, color32_white, 0.1f );
				  }

			  }

		  }

      }

      if(g_render_markers)
      {

		  vector3d vec;
		  camera* cam = app::inst()->get_game()->get_current_view_camera();
		  vector3d cam_face = cam->get_abs_po().get_facing();
		  vector3d cam_pos = cam->get_abs_position();
		  bool vis_check = g_camera_out_of_world;

		  for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		  {
			  if ((*ri).reg->get_data())
			  {
				  vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				  vector<entity*>::const_iterator i = entlist.begin();

				  while (i != entlist.end())
				  {
					  entity *mark = (*i);
					  ++i;

					  if(mark && (mark->get_flavor() == ENTITY_MARKER /*! || mark->get_flavor() == ENTITY_CRAWL_MARKER !*/))
					  {
						  vec = mark->get_abs_position();

						  vector3d dir = (vec - cam_pos);
						  if(dir != ZEROVEC)
							  dir.normalize();
						  else
							  dir = cam_face;

						  if(dot(cam_face, dir) > 0.0f && (vis_check || visibility_check(cam_pos, vec, mark)))
						  {
							  vec = xform3d_1(geometry_manager::inst()->xforms[geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW], vec);

							  if(vec.z > 0.0f)
							  {
								  vec = xform3d_1_homog(geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_SCREEN], vec);

								  render_marker(mark->get_abs_position(), color32(255, 255, 255, 192), 0.5f);
								  render_beam(mark->get_abs_position(), mark->get_abs_position() + (mark->get_abs_po().get_facing() * 0.25f), color32(0, 255, 0, 192), 0.05f);

								  hw_rasta::inst()->print( mark->get_name(), vector2di(vec.x, vec.y) );
							  }
						  }
					  }
				  }
			  }
		  }
      }

      if(g_render_paths)
      {
		  vector<path_graph *>::iterator i = path_graph_list.begin();
		  while(i != path_graph_list.end())
		  {
			  if(*i != NULL)
				  (*i)->render(color32(0, 255, 0, 128), 0.05f, g_render_paths);

			  ++i;
		  }

		  if(world_path)
			  world_path->render();
      }

      if(g_render_targeting)
      {
		  for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		  {
			  if ((*ri).reg->get_data())
			  {
				  vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				  vector<entity*>::const_iterator i = entlist.begin();

				  while (i != entlist.end())
				  {
					  entity *ent = (*i);
					  ++i;

					  if(ent && ent->allow_targeting())
					  {
						  collision_geometry *cg = ent->get_colgeom();

						  if(cg)
						  {
							  render_colgeom(ent, color32(255, 0, 255, 128));
						  }
						  else
							  render_sphere(ent->get_abs_position(), ent->get_radius() > 0.0f ? ent->get_radius() : 1.0f, color32(255, 0, 255, 128));
					  }
				  }
			  }
		  }

		  if(get_hero_ptr()->get_current_target() != NULL)
			  print_3d_text(get_hero_ptr()->get_current_target_pos(), color32(255, 0, 0, 128), hero_ptr->get_current_target()->get_name().c_str());

		  render_sphere(get_hero_ptr()->get_current_target_pos(), 0.1f, color32(0, 255, 0, 128));
      }


      if(g_render_vis_spheres)
      {
		  //for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		  for (render_data::region_list::iterator ri=to_render.regions.begin(); ri==to_render.regions.begin(); ++ri) // just current region
		  {
			  region_node* node = (*ri).reg;
			  region* reg = node->get_data();
			  if (reg)
			  {
				  // to cut down on overlap madness, we only render spheres that are within x units of the camera
				  float nearby = 10.0f;
				  if (g_render_vis_spheres & 15)
				  {
					  vector<entity*> const& entlist=reg->get_entities();
					  vector<entity*>::const_iterator i = entlist.begin();

					  while (i != entlist.end())
					  {
						  entity *ent = (*i);
						  ++i;

						  if(ent && ent->is_still_visible() && ent->is_flagged(EFLAG_MISC_IN_VISIBLE_REGION))
						  {
							  color32 color;
							  if (ent->is_a_particle_generator())
							  {
								  if (!(g_render_vis_spheres & 4)) continue;
								  color=color32(40, 200, 40, 96);
							  }
							  else if (ent->is_a_light_source())
							  {
								  if (!(g_render_vis_spheres & 8)) continue;
								  color=color32(200, 200, 200, 96);
							  }
							  else
							  {
								  if (!(g_render_vis_spheres & 1)) continue;
								  color=color32(161, 0, 236, 96);
							  }
							  if ((ent->get_visual_center() - geometry_manager::inst()->get_camera_pos()).length() < nearby + ent->get_visual_radius())
								  render_sphere(ent->get_visual_center(), ( ent->get_visual_radius() > 0.0f ) ? ent->get_visual_radius() : 1.0f, color);
						  }
					  }
				  }
				  if (g_render_vis_spheres & 16)
				  {
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
					  // do something here?
#else
					  for (int j=0; j<reg->get_num_visreps(); ++j)
					  {
						  visual_rep* vrep = reg->get_visrep(j);

						  if ((vrep->get_center() - geometry_manager::inst()->get_camera_pos()).length() < nearby + vrep->get_radius())
							  render_sphere(vrep->get_center(), ( vrep->get_radius() > 0.0f ) ? vrep->get_radius() : 1.0f, color32(236, 161, 0, 96));
					  }
#endif
				  }
			  }
		  }
      }

      if(g_render_portals)
      {
		  for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
			  //for (render_data::region_list::iterator ri=to_render.regions.begin(); ri==to_render.regions.begin(); ++ri) // just current region
		  {
			  region_node* node = (*ri).reg;
			  region* reg = node->get_data();
			  if (reg)
			  {
				  if (reg->is_active())
				  {
					  // to cut down on overlap madness, we only render spheres that are within x units of the camera
					  for (region_node::iterator rni=node->begin(); rni!=node->end(); ++rni)
					  {
						  portal* port = (*rni).get_data();

						  if (port->is_active()) // don't show closed portals
						  {
							  rational_t radius = port->get_effective_radius();
							  vector3d cylvec = port->get_cylinder_normal()*(port->get_cylinder_depth()+0.01f);

							  if ((g_render_portals & 1)!=0)
								  render_cylinder(port->get_effective_center()-cylvec,
								  port->get_effective_center()+cylvec,
								  radius, color32(161, 236, 0, 96));
							  if ((g_render_portals & 4)!=0)
							  {
								  vector<material*> altmat;
								  extern game* g_game_ptr;
								  altmat.push_back(g_game_ptr->get_blank_material());
								  instance_render_info iri(port->get_max_faces(),
									  identity_matrix,
									  0,
									  app::inst()->get_game()->get_current_view_camera()->get_region(),
									  0,
									  color32(236, 161, 0, 96),
									  FORCE_TRANSLUCENCY,
									  0,
									  1.0f,
									  NULL,
									  -1,
									  &altmat);
								  port->render_instance(RENDER_TRANSLUCENT_PORTION, &iri);
							  }
							  if ((g_render_portals & 2)!=0)
								  render_sphere(port->get_effective_center(), radius, color32(161, 236, 0, 56));
							  if ((g_render_portals & 8)!=0)
								  print_3d_text(port->get_effective_center(), color32(161, 255, 80, 192),
								  "%s <-> %s", port->get_front()->get_data()->get_name().c_str(),
								  port->get_back ()->get_data()->get_name().c_str());
						  }
					  }
				  }
			  }
		  }
      }

      if(g_render_collisions)
      {
		  for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		  {
			  if ((*ri).reg->get_data())
			  {
				  vector<entity*> const& entlist=(*ri).reg->get_data()->get_possible_collide_entities();
				  vector<entity*>::const_iterator i = entlist.begin();

				  while (i != entlist.end())
				  {
					  entity *ent = (*i);
					  ++i;

					  if ( ent && ent->are_collisions_active() )
					  {
						  bool scannable = ( ent->is_scannable() && ent->is_visible() && !ent->is_destroyable() && !ent->is_a_crate() );
						  if ( g_render_collisions == 5
							  || (ent->has_camera_collision() && (g_render_collisions == 2 || g_render_collisions == 1))
							  || (ent->is_beamable() && (g_render_collisions == 3 || g_render_collisions == 1))
							  || (scannable && (g_render_collisions == 4 || g_render_collisions == 1))
							  )
						  {
							  int r=0, g=0, b=0;
							  if ( g_render_collisions == 5 )
							  {
								  r = ent->has_entity_collision() ? 255 : 0;
								  g = 255;
								  b = 255;
							  }
							  else
							  {
								  if ( ent->has_camera_collision() && (g_render_collisions==1 || g_render_collisions==2) )
									  r = 255;
								  if ( ent->is_beamable() && (g_render_collisions==1 || g_render_collisions==3) )
									  g = 255;
								  if ( scannable && (g_render_collisions==1 || g_render_collisions==4) )
									  b = 255;
							  }

							  color32 col = color32(r,g,b,96);
							  render_colgeom( ent, col );

							  stringx flags = empty_string;

							  if ( g_render_collisions <= 5 )
							  {
								  if ( ent->has_camera_collision() )
									  flags += "C";
								  if ( ent->is_scannable() )
									  flags += "S";
								  if ( ent->is_beamable() )
									  flags += "B";
								  if( ent->has_entity_collision() )
									  flags += "E";
							  }

							  print_3d_text(ent->get_abs_position(), col, "%s (%s)", ent->get_name().c_str(), flags.c_str());
						  }
					  }
				  }
			  }
		  }
      }

      if ( g_render_brains )
      {
		  ai_interface::render_ai(g_render_brains);
      }

#if _VIS_ITEM_DEBUG_HELPER
	  extern void render_vis_item_debug_info();
	  if(g_render_vis_item_debug_info)
		  render_vis_item_debug_info();
#endif

      if ( g_render_anim_info )
      {
		  for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		  {
			  if ((*ri).reg->get_data())
			  {
				  vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				  vector<entity*>::const_iterator i = entlist.begin();

				  while (i != entlist.end())
				  {
					  entity *ent = (*i);
					  ++i;

					  if(ent && ent->is_visible())
					  {
						  if(g_render_anim_info == 1)
						  {
							  if(ent->is_hero() || !ent->has_ai_ifc())
								  continue;
						  }
						  else if(g_render_anim_info == 2)
						  {
							  if(!ent->has_ai_ifc())
								  continue;
						  }

						  stringx misc_str = empty_string;

						  if(!ent->anim_finished(ANIM_PRIMARY))
							  misc_str += stringx(stringx::fmt, "\nANIM_PRIMARY: %s (%.2f)", ent->get_anim_tree(ANIM_PRIMARY)->get_name().c_str(), ent->get_anim_tree(ANIM_PRIMARY)->get_floor_offset());
						  if(!ent->anim_finished(ANIM_SECONDARY_A))
							  misc_str += stringx(stringx::fmt, "\nANIM_SECONDARY_A: %s (%.2f)", ent->get_anim_tree(ANIM_SECONDARY_A)->get_name().c_str(), ent->get_anim_tree(ANIM_SECONDARY_A)->get_floor_offset());
						  if(!ent->anim_finished(ANIM_SECONDARY_B))
							  misc_str += stringx(stringx::fmt, "\nANIM_SECONDARY_B: %s (%.2f)", ent->get_anim_tree(ANIM_SECONDARY_B)->get_name().c_str(), ent->get_anim_tree(ANIM_SECONDARY_B)->get_floor_offset());
						  if(!ent->anim_finished(ANIM_SECONDARY_C))
							  misc_str += stringx(stringx::fmt, "\nANIM_SECONDARY_C: %s (%.2f)", ent->get_anim_tree(ANIM_SECONDARY_C)->get_name().c_str(), ent->get_anim_tree(ANIM_SECONDARY_C)->get_floor_offset());
						  if(!ent->anim_finished(ANIM_TERTIARY_A))
							  misc_str += stringx(stringx::fmt, "\nANIM_TERTIARY_A: %s (%.2f)", ent->get_anim_tree(ANIM_TERTIARY_A)->get_name().c_str(), ent->get_anim_tree(ANIM_TERTIARY_A)->get_floor_offset());
						  if(!ent->anim_finished(ANIM_TERTIARY_B))
							  misc_str += stringx(stringx::fmt, "\nANIM_TERTIARY_B: %s (%.2f)", ent->get_anim_tree(ANIM_TERTIARY_B)->get_name().c_str(), ent->get_anim_tree(ANIM_TERTIARY_B)->get_floor_offset());
						  if(!ent->anim_finished(ANIM_TERTIARY_C))
							  misc_str += stringx(stringx::fmt, "\nANIM_TERTIARY_C: %s (%.2f)", ent->get_anim_tree(ANIM_TERTIARY_C)->get_name().c_str(), ent->get_anim_tree(ANIM_TERTIARY_C)->get_floor_offset());
						  if(!ent->anim_finished(ANIM_SCENE))
							  misc_str += stringx(stringx::fmt, "\nANIM_SCENE: %s", ent->get_anim_tree(ANIM_SCENE)->get_name().c_str());

						  if(ent->has_physical_ifc())
						  {
							  rational_t floor_off = ent->physical_ifc()->get_floor_offset();

							  misc_str += stringx(stringx::fmt, "\nCurrent Floor: %.2f", floor_off);

							  rational_t y = ent->get_abs_position().y - floor_off + 0.001f;
							  vector3d xface = ent->get_abs_po().get_x_facing();
							  vector3d zface = ent->get_abs_po().get_z_facing();
							  vector3d ul = ent->get_abs_position() - (ent->get_abs_po().get_x_facing()*0.5f) - (ent->get_abs_po().get_z_facing()*0.5f);
							  vector3d ur = ent->get_abs_position() + (ent->get_abs_po().get_x_facing()*0.5f) - (ent->get_abs_po().get_z_facing()*0.5f);
							  vector3d ll = ent->get_abs_position() - (ent->get_abs_po().get_x_facing()*0.5f) + (ent->get_abs_po().get_z_facing()*0.5f);
							  vector3d lr = ent->get_abs_position() + (ent->get_abs_po().get_x_facing()*0.5f) + (ent->get_abs_po().get_z_facing()*0.5f);

							  ul.y = y;
							  ur.y = y;
							  ll.y = y;
							  lr.y = y;

							  render_triangle(ul, ur, lr, color32(0, 255, 0, 128), true);
							  render_triangle(ul, ll, lr, color32(0, 255, 0, 128), true);
						  }

						  print_3d_text(ent->get_abs_position()+YVEC, color32_blue, "%s%s", ent->get_name().c_str(), misc_str.c_str());
					  }
				  }
			  }
		  }
      }

      if ( g_render_scene_anim_info )
      {
		  for (render_data::region_list::iterator ri=to_render.regions.begin(); ri!=to_render.regions.end(); ++ri)
		  {
			  if ((*ri).reg->get_data())
			  {
				  vector<entity*> const& entlist=(*ri).reg->get_data()->get_entities();
				  vector<entity*>::const_iterator i = entlist.begin();

				  while (i != entlist.end())
				  {
					  entity *ent = (*i);
					  ++i;

					  if ( ent && ent->playing_scene_anim() )
					  {
						  render_beam(ent->get_abs_position(), ent->get_abs_position() + (ent->get_abs_po().get_x_facing() * 2.0f), color32(255, 0, 0, 192), 0.05);
						  render_beam(ent->get_abs_position(), ent->get_abs_position() + (ent->get_abs_po().get_y_facing() * 2.0f), color32(0, 255, 0, 192), 0.05);
						  render_beam(ent->get_abs_position(), ent->get_abs_position() + (ent->get_abs_po().get_z_facing() * 2.0f), color32(0, 0, 255, 192), 0.05);
					  }
				  }
			  }
		  }
      }

	  /*
      if(g_render_dread_net && dread_network)
      {
	  dread_network->render(color32(0, 0, 255, 127), g_render_dread_net);
      }
	  */

	  //      render_debug_text();

      proftimer_render_debug.stop();
#endif // !BUILD_BOOTABLE

#if _ENABLE_WORLD_EDITOR
      tool_rendering();
#endif

    }
  }
#if defined (DEBUG)
  if (g_render_capsule_history)
	  render_capsule_history();
  if (g_debug.render_spheres)
	  render_debug_spheres();

#if 0 // BIGCULL
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
  extern new_LineInfo g_lineinfo[4];

  render_sphere(g_lineinfo[0].StartCoords, 0.1f, color32(255, 0, 0, 128));
  render_sphere(g_lineinfo[0].EndCoords, 0.1f, color32(0, 0, 255, 128));
  render_beam(g_lineinfo[0].StartCoords, g_lineinfo[0].EndCoords, color32(0, 255, 0, 128), 0.05f);
  if(g_lineinfo[0].collision)
  {
	  render_sphere(g_lineinfo[0].hit_pos, 0.1f, color32(255, 255, 255, 128));
	  render_beam(g_lineinfo[0].hit_pos, g_lineinfo[0].hit_pos+g_lineinfo[0].hit_norm, color32(0, 0, 0, 128), 0.05f);
  }

  render_sphere(g_lineinfo[1].StartCoords, 0.1f, color32(255, 0, 0, 128));
  render_sphere(g_lineinfo[1].EndCoords, 0.1f, color32(0, 0, 255, 128));
  render_beam(g_lineinfo[1].StartCoords, g_lineinfo[1].EndCoords, color32(255, 255, 0, 128), 0.05f);
  if(g_lineinfo[1].collision)
  {
	  render_sphere(g_lineinfo[1].hit_pos, 0.1f, color32(255, 255, 255, 128));
	  render_beam(g_lineinfo[1].hit_pos, g_lineinfo[1].hit_pos+g_lineinfo[1].hit_norm, color32(0, 0, 0, 128), 0.05f);
  }

  render_sphere(g_lineinfo[2].StartCoords, 0.1f, color32(255, 0, 0, 128));
  render_sphere(g_lineinfo[2].EndCoords, 0.1f, color32(0, 0, 255, 128));
  render_beam(g_lineinfo[2].StartCoords, g_lineinfo[2].EndCoords, color32(0, 255, 255, 128), 0.05f);
  if(g_lineinfo[2].collision)
  {
	  render_sphere(g_lineinfo[2].hit_pos, 0.1f, color32(255, 255, 255, 128));
	  render_beam(g_lineinfo[2].hit_pos, g_lineinfo[2].hit_pos+g_lineinfo[2].hit_norm, color32(0, 0, 0, 128), 0.05f);
  }

  render_sphere(g_lineinfo[3].StartCoords, 0.1f, color32(255, 0, 0, 128));
  render_sphere(g_lineinfo[3].EndCoords, 0.1f, color32(0, 0, 255, 128));
  render_beam(g_lineinfo[3].StartCoords, g_lineinfo[3].EndCoords, color32(255, 255, 255, 128), 0.05f);
  if(g_lineinfo[3].collision)
  {
	  render_sphere(g_lineinfo[3].hit_pos, 0.1f, color32(255, 255, 255, 128));
	  render_beam(g_lineinfo[3].hit_pos, g_lineinfo[3].hit_pos+g_lineinfo[3].hit_norm, color32(0, 0, 0, 128), 0.05f);
  }

  // BIGCULL g_spiderman_controller_ptr->render_lookaround_reticle();
#endif
#endif// BIGCULL

#endif

#ifndef BUILD_BOOTABLE
  if (g_debug.dump_frame_info)
  {
	  dump_frame_info();
  }
#endif
}

#endif

void world_dynamics_system::usercam_frame_advance(time_value_t t)
{
	usercam_controller->frame_advance(t);
	usercam->frame_advance(t);
	usercam_move_mcs->frame_advance(t);
	usercam_orient_mcs->frame_advance(t);
}

void world_dynamics_system::scene_analyzer_frame_advance(time_value_t t)
{
	if ( geometry_manager::inst()->is_scene_analyzer_enabled() )
	{
		scene_analyzer_controller->frame_advance(t);
		scene_analyzer_cam->frame_advance(t);
		scene_analyzer_move_mcs->frame_advance(t);
		scene_analyzer_orient_mcs->frame_advance(t);
	}
}




#define _TARGETING_NEAR_THRESHOLD       3.0f
#define _TARGETING_FAR_THETA            0.98f // ~11.5 degrees
#define _TARGETING_NEAR_THETA           0.6f  // ~53.1 degrees
#define _TARGETING_FAR_STICKY_THETA     0.8f  // ~36.8 degrees
#define _TARGETING_NEAR_STICKY_THETA    0.4f  // ~66.4 degrees
#define _TARGETING_STICKY_DISTANCE_MOD  0.75f
#define _TARGETING_HEIGHT_TOLERANCE     0.0f


void build_region_list_radius( vector<region_node*> *regs, region_node* rn, const vector3d& pos, rational_t rad, bool only_active_portals, vector<region_node*> *append )
{
	if( rn && regs && find(regs->begin(), regs->end(), rn) == regs->end() )
	{
		regs->push_back( rn );

		if(append && find(append->begin(), append->end(), rn) == append->end())
			append->push_back(rn);

		// check for intersection with portals leading from this region
		edge_iterator tei = rn->begin();
		edge_iterator tei_end = rn->end();

		for ( ; tei!=tei_end; ++tei )
		{
			// don't bother with regions we've already visited
			region_node* dest = (*tei).get_dest();

			if( find(regs->begin(), regs->end(), dest) == regs->end() )
			{
				portal* port = (*tei).get_data();

				if (!only_active_portals || port->is_active())
				{
					// intersection of activation sphere and portal cylinder
					rational_t r = rad + port->get_effective_radius();
					vector3d v = pos - port->get_effective_center();

					if ( v.length2() < r*r )
					{
						rational_t d = dot( v, port->get_cylinder_normal() );
						rational_t ad = __fabs( d );

						if ( ad < rad+port->get_cylinder_depth() )
						{
							// compute closest point on portal cylinder
							vector3d dpos = pos - port->get_cylinder_normal() * d;
							dpos -= port->get_effective_center();
							d = dpos.length2();

							if ( d > port->get_effective_radius()*port->get_effective_radius() )
							{
								d = __fsqrt( d );
								dpos = dpos * (port->get_effective_radius() / d);
								rational_t dd = d - port->get_effective_radius();
								ad = __fsqrt( ad*ad + dd*dd );
							}

							dpos += port->get_effective_center();
							rational_t drad = rad + port->get_cylinder_depth() - ad;

							if ( drad > 0 )
							{
								// recurse into dest region using NEW sphere
								build_region_list_radius( regs, dest, dpos, drad, only_active_portals, append );
							}
						}
					}
				}
			}
		}
	}
}


void build_region_list( vector<region_node*> *regs, region_node *r, const vector3d& o, const vector3d& d, vector<region_node*> *append )
{
	if( find(regs->begin(), regs->end(), r) == regs->end() )
	{
		if (!r) return; // laser grenades can end up here with a null primary region
		regs->push_back( r );

		if(append && find(append->begin(), append->end(), r) == append->end())
			append->push_back(r);

		for( edge_iterator tei = r->begin(); tei != r->end(); ++tei )
		{
			region_node* dest = (*tei).get_dest();

			if( find(regs->begin(), regs->end(), dest) == regs->end() )
			{
				rational_t  rad, depth;
				vector3d    hit_point, norm, cent;
				portal      *port = (*tei).get_data();
				if (port->is_active())
				{
					rad = port->get_effective_radius();
					depth = port->get_cylinder_depth();
					cent = port->get_effective_center();
					norm = port->get_cylinder_normal();

					if( collide_segment_cylinder( o, d, cent, norm, rad, depth, hit_point ) )
					{
						build_region_list( regs, dest, hit_point, (o + d) - hit_point, append );
					}
				}
			}
		}
	}
}


#define MAX_AUTO_AIM_DIST 15.0f




#ifdef DEBUG
rational_t debug_delay = 0.0f;

//static int g_debug;

#endif

float g_radius = 0;
float g_frame_by_frame = 0.0f;

extern int g_iflrand_counter;
int global_frame_counter = 0;

extern profiler_timer proftimer_physics;
extern profiler_timer proftimer_adv_controllers;
extern profiler_timer proftimer_adv_player_controller;
extern profiler_timer proftimer_adv_fcs;
extern profiler_timer proftimer_adv_generators;
extern profiler_timer proftimer_adv_entities;
extern profiler_timer proftimer_adv_mcs;
extern profiler_timer proftimer_phys_render;
extern profiler_timer proftimer_adv_anims;
extern profiler_timer proftimer_adv_anims_ents;
extern profiler_timer proftimer_get_elevation;
extern profiler_timer proftimer_adv_ent_timed;
extern profiler_timer proftimer_adv_scripts;
extern profiler_timer proftimer_adv_ent_setup;

extern profiler_timer proftimer_adv_AI;
extern profiler_timer proftimer_adv_AI_cue_mgr;

#include <algorithm>

#if defined(PROJECT_KELLYSLATER) && defined(DEBUG)
entity * gde = NULL;
char gde_name[256];
#endif


void world_dynamics_system::frame_advance( time_value_t t )
{
#if defined(TARGET_XBOX)
	assert( t > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

#if _VIS_ITEM_DEBUG_HELPER
    extern void do_the_vis_item_helper();
    if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, ITEM_SWITCH_DEBUG_ON ) == AXIS_MAX )
    {
		g_render_vis_item_debug_info = !g_render_vis_item_debug_info;
    }

    if(g_render_vis_item_debug_info)
		do_the_vis_item_helper();
#endif

		/*
		// notify the LipSync
		if ( m_pLipSync )
		m_pLipSync->frame_advance(t);
	*/

	++global_frame_counter;
#ifdef DEBUG
	g_getpo_counter = 0;
	g_buildpo_counter= 0;
#endif

	proftimer_adv_ent_timed.start();

	cur_time_inc = t;
	g_level_time += t;

#if defined(TARGET_XBOX)
	assert( t > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

	g_iflrand_counter=0;
	g_frame_advance_called_this_game = true;

	// destroy any entities on the time-limited ents list that have expired
	vector<ent_time_limit>::iterator etli;
	for ( etli=time_limited_entities.begin(); etli!=time_limited_entities.end(); )
	{
		if ( (*etli).duration <= 0 )
		{
			// due to the nature of particle generators, we must tell the entity to
			// become invisible and wait for it to actually achieve that state before
			// we destroy it
			entity* e = (*etli).ent;

			e->set_visible( false );
			if ( !e->is_still_visible() )
			{
#ifdef DEBUG
				error_context::inst()->push_context( stringx("(script) destroy_entity( id=")+e->get_id().get_val()+", anim_id="+anim_id_manager::inst()->get_label(e->get_anim_id())+" )" );
#endif
				if ( e->get_entity_pool() == NULL )
					destroy_entity( e );
				else
					g_entity_maker->release_entity( e );
#ifdef DEBUG
				error_context::inst()->pop_context();
#endif
				etli = time_limited_entities.erase( etli );
			}
			else
				++etli;
		}
		else
		{
			(*etli).duration -= CALC_ENTITY_TIME_DILATION(t, (*etli).ent);
			++etli;
		}
	}

	proftimer_adv_ent_timed.stop();

	// scripting was moved lower (JDB), but caused problems with parented entities, and destroyed ents.
	// so moved back here.
	proftimer_adv_scripts.start();
	// execute all script threads
	scriptman.run(t);
	proftimer_adv_scripts.stop();

#ifndef BUILD_BOOTABLE
	if ( g_debug.dump_threads )
	{
		scriptman.dump_threads();
		g_debug.dump_threads = false;
	}
#endif

	//  Process the placeholder character_activation commands
	proftimer_adv_ent_setup.start();
	// build list of active entities
	collision_entities.reserve(entities.size());
	collision_entities.resize(0);
	active_entities.reserve( entities.size() );
	active_entities.resize(0);

	parented_entities.reserve( entities.size() );
	parented_entities.resize(0);

	dead_ents.reserve( 8 );
	dead_ents.resize(0);
	//  moved_ents.resize(0);

#if (defined(BUILD_DEBUG) || defined(BUILD_FAST_DEBUG)) && defined (TARGET_PC)
	if(g_dump_activity_info)
	{
		host_system_file_handle outfile = host_fopen( "activity.txt", HOST_WRITE );
		host_fprintf( outfile, "Activity Data\n\n" );

		vector<entity*>::const_iterator cei, ei_end;

		host_fprintf( outfile, "guaranteed_active_entities (%d)\n---------------\n", guaranteed_active_entities.size() );
		cei = guaranteed_active_entities.begin();
		ei_end = guaranteed_active_entities.end();
		for ( ; cei!=ei_end; ++cei )
		{
			entity* e = *cei;
			if ( e!=NULL )
				host_fprintf( outfile, "%s (%s::%s)\n", e->get_name().c_str(),
				entity_flavor_names[e->get_flavor()], e->ent_filename.c_str() );
		}

		host_fprintf( outfile, "Regions (%d)\n---------------\n", the_terrain->get_regions().size() );

		region_list::const_iterator ri, ri_end=the_terrain->get_regions().end();
		region* r;
		for ( ri=the_terrain->get_regions().begin(); ri!=ri_end; ++ri )
		{
			r = *ri;
			host_fprintf( outfile, "%s\n", r->get_name().c_str());
			host_fprintf( outfile, "     Entities (%d)\n", r->get_possible_active_entities().size() );

			ei_end = r->get_possible_active_entities().end();
			for ( cei=r->get_possible_active_entities().begin(); cei!=ei_end; ++cei )
			{
				entity* e = *cei;
				if ( e!=NULL )
					host_fprintf( outfile, "          %s (%s::%s)\n", e->get_name().c_str(),
					entity_flavor_names[e->get_flavor()], e->ent_filename.c_str() );
			}
		}

		host_fprintf( outfile, "\n----------------------------------------------------\n" );

		//    profiler::inst()->write_to_host_file(outfile);

		host_fclose( outfile );

		g_dump_activity_info = false;
	}

	if(g_dump_render_info)
	{
		host_system_file_handle outfile = host_fopen( "render.txt", HOST_WRITE );
		host_fprintf( outfile, "Render Data\n\n" );

		vector<entity*>::const_iterator cei, ei_end;

		host_fprintf( outfile, "Regions (%d)\n---------------\n", the_terrain->get_regions().size() );

		region_list::const_iterator ri, ri_end=the_terrain->get_regions().end();
		region* r;
		for ( ri=the_terrain->get_regions().begin(); ri!=ri_end; ++ri )
		{
			r = *ri;
			host_fprintf( outfile, "%s\n", r->get_name().c_str());
			host_fprintf( outfile, "     Entities (%d)\n", r->get_possible_render_entities().size() );

			ei_end = r->get_possible_render_entities().end();
			for ( cei=r->get_possible_render_entities().begin(); cei!=ei_end; ++cei )
			{
				entity* e = *cei;
				if ( e!=NULL )
					host_fprintf( outfile, "          %s (%s::%s)\n", e->get_name().c_str(),
					entity_flavor_names[e->get_flavor()], e->ent_filename.c_str() );
			}
		}

		host_fprintf( outfile, "\n----------------------------------------------------\n" );

		//    profiler::inst()->write_to_host_file(outfile);

		host_fclose( outfile );

		g_dump_render_info = false;
	}

#if defined(TARGET_XBOX)
	assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

	if(g_dump_collide_info)
	{
		host_system_file_handle outfile = host_fopen( "collide.txt", HOST_WRITE );
		host_fprintf( outfile, "Collide Data\n\n" );

		vector<entity*>::const_iterator cei, ei_end;

		host_fprintf( outfile, "Regions (%d)\n---------------\n", the_terrain->get_regions().size() );

		region_list::const_iterator ri, ri_end=the_terrain->get_regions().end();
		region* r;
		for ( ri=the_terrain->get_regions().begin(); ri!=ri_end; ++ri )
		{
			r = *ri;
			host_fprintf( outfile, "%s\n", r->get_name().c_str());
			host_fprintf( outfile, "     Entities (%d)\n", r->get_possible_collide_entities().size() );

			ei_end = r->get_possible_collide_entities().end();
			for ( cei=r->get_possible_collide_entities().begin(); cei!=ei_end; ++cei )
			{
				entity* e = *cei;
				if ( e!=NULL )
					host_fprintf( outfile, "          %s (%s::%s)\n", e->get_name().c_str(),
					entity_flavor_names[e->get_flavor()], e->ent_filename.c_str() );
			}
		}

		host_fprintf( outfile, "\n----------------------------------------------------\n" );

		//    profiler::inst()->write_to_host_file(outfile);

		host_fclose( outfile );

		g_dump_collide_info = false;
	}
#endif
#ifdef PROFILING_ON
	if(g_dump_profile_info)
	{
		stringx profdumpfile = os_developer_options::inst()->get_string (os_developer_options::STRING_PROFILE_DUMP_FILE);
		host_system_file_handle outfile = host_fopen( profdumpfile.c_str(), HOST_WRITE );
		host_fprintf( outfile, "Profile Data\n\n" );

		profiler::inst()->write_to_host_file(outfile);

		host_fclose( outfile );

		g_dump_profile_info = false;
	}
#endif

	vector<entity*>::iterator ei, ei1;
	vector<entity*>::const_iterator cei, ei_end, ei_end1;

	{
		region_list::const_iterator ri, ri_end;
		region* r;

		// BETH
		//if(FEDone())
		//{
		ri_end = the_terrain->get_regions().end();
		//}

		/*
		region_list::const_iterator ri, ri_end=the_terrain->get_regions().end();
		region* r;
		*/

		entity::prepare_for_visiting();

		// there are some entities that need to be active even when not in an active region
		cei = guaranteed_active_entities.begin();
		ei_end = guaranteed_active_entities.end();
		for ( ; cei!=ei_end; ++cei )
		{
			entity* e = *cei;
			if ( e!=NULL )// && e->is_active() )
			{
				e->visit();

				if (e->has_parent())// && !e->is_conglom_member())
					parented_entities.push_back(e);

				e->set_active(true);

				active_entities.push_back( e );

				if (e->is_frame_delta_valid() || e->is_last_frame_delta_valid())
					e->invalidate_frame_delta();
			}
		}

		// determine other active entities
		entity* e;

		//BETH
		//if(FEDone())
		//{
		for ( ri=the_terrain->get_regions().begin(); ri!=ri_end; ++ri )
		{
			r = *ri;
			// We only have one region, so this doesn't really matter.  The test below caused
			// bugs, because the determination of which regions are active happens on the previous
			// frame's render.  If the camera changed between frames, the calculation became
			// invalid.  (dc 03/27/02)
			//      if ( r->is_active() )
			{
				ADD_PROF_COUNT(profcounter_regions, 1);
				ADD_PROF_COUNT(profcounter_ents, r->get_entities().size());
				ADD_PROF_COUNT(profcounter_poss_active_ents, r->get_possible_active_entities().size());
				ADD_PROF_COUNT(profcounter_poss_collide_ents, r->get_possible_collide_entities().size());

				ei_end = r->get_possible_active_entities().end();
				for ( cei=r->get_possible_active_entities().begin(); cei!=ei_end; ++cei )
				{
					e = *cei;
					if ( e && !e->already_visited() )
					{
						e->visit();
						if (e->has_parent())// && !e->is_conglom_member())
						{
							parented_entities.push_back(e);
						}

						if( !e->is_flagged(EFLAG_MISC_IN_VISIBLE_REGION) )
						{
							if( e->get_flavor() != ENTITY_PARTICLE_GENERATOR &&
								e->get_flavor() != ENTITY_CONGLOMERATE &&
								(e->get_flavor() == ENTITY_PHYSICAL && !e->is_active() && e->is_stationary()) &&
								e->is_visible() &&  // this covers use of invisible dummy objects used for moving stuff around
								e->get_programmed_cell_death() >= LARGE_TIME*0.1F)  // fixes text HP numbers above heads
							{
								continue;
							}
						}

						if (e->is_frame_delta_valid() || e->is_last_frame_delta_valid())
						{
							//              moved_ents.push_back(e);
							e->invalidate_frame_delta();
						}

						// don't include static (immobile) entities (flavor ENTITY)
						if ( e->has_bounding_box() && !(e->has_physical_ifc() && e->physical_ifc()->is_enabled()) )
						{
							// this allows animating textures, etc. to run on static entities
							e->advance_age( t );
						}
						else
						{
							bool active = false;
							bool aging = true;
							switch ( e->get_flavor() )
							{
							case ENTITY_ENTITY:
							case ENTITY_MANIP:
							case ENTITY_SWITCH:
							case ENTITY_CONGLOMERATE:
								active = !e->is_conglom_member() && (e->is_active() || e->get_light_set() || (e->has_physical_ifc() && e->physical_ifc()->is_enabled()) || (e->has_ai_ifc() && e->is_alive() && (!e->ai_ifc()->cosmetic() || e->is_flagged(EFLAG_MISC_IN_VISIBLE_REGION)) ));
								e->set_active(active);
								break;

							case ENTITY_MARKER:
							case ENTITY_ITEM:
							case ENTITY_SKY:
								active = e->is_active() || (e->has_physical_ifc() && e->physical_ifc()->is_enabled()) || (e->has_ai_ifc());
								break;

							case ENTITY_BEAM:
							case ENTITY_SCANNER:
							case ENTITY_LENSFLARE:
								active = e->is_visible();
								break;

							case ENTITY_PHYSICAL:
								active = ( e->is_active() && !e->is_stationary() );
								assert(0);
								break;

							case ENTITY_PARTICLE_GENERATOR:
								{
									static rational_t largetimer = LARGE_TIME*0.1F;
									if ( e->get_programmed_cell_death() < largetimer ||
										(e->is_active() && e->is_flagged(EFLAG_MISC_IN_VISIBLE_REGION)) ||
										(!e->is_visible() && e->is_still_visible())
										)
										active = true;
									else
										aging = false;

									break;
								}
							default:
								break;
							}
							if ( active )
							{
								active_entities.push_back( e );

								//                e->set_flag( EFLAG_MISC_COMP_SECT_THIS_FRAME, false );
							}
							if ( aging )
							{
								e->advance_age( CALC_ENTITY_TIME_DILATION(t, e) );
								if ( e->get_age() >= e->get_programmed_cell_death() )
									dead_ents.push_back( e );
							}
						}
					}
				}
      }
    }

	//	}//BETH
  }

  /*
  ei_end = moved_ents.end();
  for ( ei=moved_ents.begin(); ei<ei_end; ++ei )
  {
  entity* e = (*ei);
  if ( e )
  {
  e->invalidate_frame_delta();
  }
  }
  */
  if (setup_time)
	  --setup_time;
  ei = dead_ents.begin();
  ei_end = dead_ents.end();
  for ( ; ei!=ei_end; ++ei )
  {
	  entity* e = *ei;
	  remove_entity( e );

	  // Smoke it from the parented_entities list
	  ei_end1 = parented_entities.end();
	  ei1 = find( parented_entities.begin(), parented_entities.end(), e );
	  if ( ei1 != ei_end1 )
		  *ei1 = NULL;

	  if( !e->is_flagged(EFLAG_MISC_NOKILLME) )
	  {
		  delete e;
	  }
	  else
	  {
		  e->destruct();
		  e->set_flag( EFLAG_MISC_REUSEME, true );
	  }
  }

  t_inc = t;

  proftimer_adv_ent_setup.stop();
  proftimer_physics.start();

  /*
  // moved lower to reduce loop overhead
  for ( ei=active_entities.begin(); ei<active_entities.end(); ++ei )
  {
  if ( *ei )
  {
  (*ei)->invalidate_colgeom();
  }
  }
  */

  vector<controller *>::iterator cti;
  proftimer_adv_controllers.start();
  for (cti=controllers.begin();cti!=controllers.end();++cti)
  {
	  if ((*cti)->is_active() ) // BIGCULL && (*cti) != g_spiderman_controller_ptr)
		  (*cti)->frame_advance(t);
  }

  // Update all ACTIVE kellyslater controllers.
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
	  if (ks_controller[i] && ks_controller[i]->is_active())
		  ks_controller[i]->frame_advance(t);
  }

  if (g_beach_ptr != NULL)
	  g_beach_ptr->update(t);

  //  if(dread_network && dread_network->is_active())
  //    dread_network->frame_advance(t);

  // The player controller is handled specially.  This is for move editing purposes, to make sure it goes last.
  // (it is not added to the main list).  Since the Hero is added last, the move editor is currently
  // the only way that additional controllers get added after the hero's player_controller.
  /*
  proftimer_adv_player_controller.start();
  if ( g_spiderman_controller_ptr->is_active() )
  g_spiderman_controller_ptr->frame_advance(CALC_ENTITY_TIME_DILATION(t, g_spiderman_controller_ptr->get_owner()));
  proftimer_adv_player_controller.stop();
  */
  proftimer_adv_controllers.stop();

  // update entity animations
  proftimer_adv_anims.start();
  proftimer_adv_anims_ents.start();
  ADD_PROF_COUNT(profcounter_anims, anims.size());
  pentity_anim_tree_vector::iterator ani = anims.begin();
  pentity_anim_tree_vector::iterator ani_end = anims.end();
  for ( ; ani!=ani_end; ++ani )
  {

	  entity_anim_tree* anm = *ani;
	  float t_time = t;

	  if ( anm )
	  {
#ifdef DEBUG
		  if (g_frame_by_frame)
		  {
			  input_mgr* inputmgr = input_mgr::inst();
			  bool psx_circ = inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_CIRCLE);
			  bool psx_sq = inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_SQUARE);
			  if (!(psx_circ || psx_sq))
				  break;
			  else
			  {
				  if (psx_sq)
				  {
					  // fixme  ...  need to make playable in reverse
					  //anm->set_flag(ANIM_REVERSE);
					  //t_time = -0.01f;
					  t_time = 0.01f;
				  }
				  else
				  {
					  anm->clear_flag(ANIM_REVERSE);
					  t_time = 0.01f;
				  }
			  }
		  }
#endif
		  if ( eligible_for_frame_advance(anm) )
		  {
			  //profcounter_anim_resets.add_count(1);
			  if ( anm->is_valid() && anm->is_relative_to_start() )
				  anm->reset_root_position();

			  anm->frame_advance( CALC_ENTITY_TIME_DILATION(t_time, anm->get_entity()) );
		  }
		  if ( anm->is_autokill() && anm->is_finished() )
			  kill_anim( anm );
	  }
  }
  proftimer_adv_anims_ents.stop();

  proftimer_adv_anims.stop();

  // Moved here to quickly respond to signals brought up by anims and controllers
  proftimer_adv_AI.start();
  ai_interface::frame_advance_ai_interfaces(t);
  proftimer_adv_AI.stop();

#if 0 // BIGCULL
  proftimer_adv_AI_cue_mgr.start();
  if(ai_cue_mgr)
	  ai_cue_mgr->frame_advance(CALC_GLOBAL_TIME_DILATION(t));
  proftimer_adv_AI_cue_mgr.stop();
#endif

  proftimer_adv_anims.start();
  // update scene_anims
  if(!scene_anims.empty())
  {
	  bool playing = false;

	  scene_anim_list_t::iterator sani = scene_anims.begin();
	  scene_anim_list_t::iterator sani_end = scene_anims.end();
	  for ( ; sani!=sani_end; ++sani )
	  {
		  entity* e = (*sani).ent;
		  if(e) e->invalidate_frame_delta();

		  if ( e && (e->anim_finished(ANIM_SCENE)))// || (*sani).name != e->get_anim_tree(ANIM_SCENE)->get_name()) )
		  {
			  e->set_playing_scene_anim(false);
			  e->raise_signal(entity::SCENE_ANIM_FINISHED);
			  if(e->has_physical_ifc())
			  {
				  e->physical_ifc()->unsuspend();

				  e->physical_ifc()->set_velocity(ZEROVEC);
				  e->physical_ifc()->set_acceleration_factor(ZEROVEC);
				  e->physical_ifc()->set_acceleration_correction_factor(ZEROVEC);
				  e->physical_ifc()->set_last_acceleration_correction_factor(ZEROVEC);
			  }

			  // hack to rotate characters into our skeleton system after they've beeen
			  // brutally managed by the a MAX scene animation skeleton system
			  if(e != get_marky_cam_ptr() && e->has_skeleton_ifc() )
			  {
				  vector3d right;
				  vector3d front = e->get_abs_po().get_z_facing();
				  front.y = 0.0f;
				  assert(front.xz_length2() > 0.1f);
				  front.normalize();
				  vector3d pos = e->get_rel_position();

				  // Cross product of normal and desired front gives right vector.
				  // (*sani).entity_up_vec
				  right = cross(YVEC, front);
				  right.normalize();

				  // Cross product of normal and right gives desired front.
				  front = cross(right, YVEC);
				  front.normalize();

				  po po2(right, YVEC, front, ZEROVEC);
				  if(e->has_parent())
					  fast_po_mul(po2, po2, e->link_ifc()->get_parent()->get_abs_po().inverse());
				  po2.set_position(pos);
				  e->set_rel_po(po2);

				  rational_t ang = DEG_TO_RAD(135.0f);
				  po rot_po;
				  rot_po.set_rotate_y(ang);

				  po e_po = e->get_abs_po();
				  pos = e->get_rel_position();

				  e_po.set_position(ZEROVEC);
				  fast_po_mul(e_po, e_po, rot_po);


				  if(e->has_parent())
					  fast_po_mul(e_po, e_po, e->link_ifc()->get_parent()->get_abs_po().inverse());

				  e_po.set_position(pos);

				  e->set_rel_po_no_children(e_po);

				  if(e->has_children())
				  {
					  entity *child = (entity *)e->link_ifc()->get_first_child();
					  child->update_abs_po_reverse();

					  rot_po = po_identity_matrix;
					  rot_po.set_rotate_y(-ang);

					  e_po = child->get_abs_po();
					  pos = child->get_rel_position();

					  e_po.set_position(ZEROVEC);
					  fast_po_mul(e_po, e_po, rot_po);
					  fast_po_mul(e_po, e_po, e->get_abs_po().inverse());
					  e_po.set_position(pos);

					  child->set_rel_po_no_children(e_po);
				  }

				  e->update_abs_po();
			  }

			  (*sani).ent = NULL;

			  if ( e == get_marky_cam_ptr() )
			  {
				  app::inst()->get_game()->enable_marky_cam( false );
				  get_marky_cam_ptr()->set_externally_controlled( false );

				  // BIGCULL chase_cam_ptr->set_rel_position(marky_cam->get_rel_position());

				  // BIGCULL          g_spiderman_camera_ptr->SetStartPosition();
			  }

			  // check if all animations ended
			  scene_anim_list_t::iterator si;
			  for( si = scene_anims.begin(); si != scene_anims.end(); ++si )
				  if( (*si).ent )
					  break;

				  if ( si == scene_anims.end() )
				  {
					  // enable brains
					  ai_interface::pop_disable_all();
				  }
      }
      else if(e)
      {
		  sani->anim_tree->frame_advance(t);
		  //		  sani->name.
		  //		if(!FEDone()) e->frame_advance(t);
		  e->compute_sector(*the_terrain);
		  playing = true;
      }
    }

    if(!playing)
		scene_anims.resize(0);
  }
  proftimer_adv_anims.stop();


  proftimer_adv_fcs.start();
  vector<force_control_system *>::iterator fcs;
  for (fcs = fcs_list.begin();fcs<fcs_list.end();++fcs)
  {
	  if ((*fcs)->is_active())
		  (*fcs)->frame_advance(t);
  }
  proftimer_adv_fcs.stop();

  proftimer_adv_generators.start();
  vector<force_generator *>::iterator fg;
  for (fg = generators.begin();fg<generators.end();++fg)
  {
	  if ((*fg)->is_active())
		  (*fg)->frame_advance(t);
  }
  proftimer_adv_generators.stop();

  proftimer_adv_entities.start();
  ei_end = active_entities.end();
  ADD_PROF_COUNT(profcounter_active_ents, active_entities.size());
  for ( ei=active_entities.begin(); ei!=ei_end; ++ei )
  {
	  entity* e = (*ei);
	  if ( e )
	  {
		  e->invalidate_colgeom();

		  // clear the collided flags (so they work properly) (JDB 9/7/00) (moved from do_all_collisions (JDB 9/8/00))
		  if(e->has_physical_ifc())
		  {
			  e->physical_ifc()->set_collided_last_frame( false );
			  e->physical_ifc()->set_ext_collided_last_frame( false );
		  }

		  rational_t e_time_inc = CALC_ENTITY_TIME_DILATION(t, e);
		  e->frame_advance( e_time_inc );

		  // parented and conglom members compute sectors elsewhere
		  if(e->needs_compute_sector() && (!e->has_parent() || e->has_ai_ifc()))
			  e->compute_sector( *the_terrain );

		  if(e->has_physical_ifc())
			  e->physical_ifc()->update_unused_velocity( e_time_inc );

		  if(e->get_colgeom() && e->are_collisions_active() && e->has_entity_collision() && !e->playing_scene_anim() && (e->get_colgeom()->get_type() != collision_geometry::CAPSULE || e->is_visible()))
			  collision_entities.push_back( e );
	  }
  }
  proftimer_adv_entities.stop();

  proftimer_adv_mcs.start();
  vector<motion_control_system *>::iterator mcs;
  for (mcs = mcs_list.begin();mcs<mcs_list.end();++mcs)
  {
	  if ((*mcs)->is_active())
		  (*mcs)->frame_advance(t);
  }
  proftimer_adv_mcs.stop();

  // we handle collisions separately on KS. - LZ (8/3/01)
  //  do_all_collisions(t);

  if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_ALLOW_ITEM_PICKUP))
  {
	  vector<item*>::iterator it;
	  vector<item*>::const_iterator it_end = items.end();
	  item* itp;
	  for ( it=items.begin(); it<it_end; ++it )
	  {
		  itp = *it;
		  if ( itp && itp->is_visible() && itp->is_in_active_region() && !itp->is_brain_weapon() )
		  {
			  if ( itp->check_for_pickup() )
			  {
				  // if we transferred the item to a character, remove it from out inventory
				  *it = NULL;
			  }
		  }
	  }
  }

  t_inc = T_INC_INVALID;

  proftimer_physics.stop();

  ei_end = active_entities.end();
  for ( ei=active_entities.begin(); ei<ei_end; ++ei )
  {
	  entity* e = *ei;
	  if ( e )
	  {
		  // CTT 03/31/00: TEMPORARY:
		  // it may or may not be adequate to do this only once per frame (I certainly hope it is!);
		  // for now, we're doing it in entity_anim_tree::frame_advance() as well as here, until
		  // we figure out exactly what the correct order of operations is, etc.
		  //BIGCULLif(!e->is_a_handheld_item() || ((handheld_item *)e)->get_owner() == NULL)
		  e->update_abs_po();

		  e->frame_done();
	  }
  }

  // recompute which regions each trigger is in
  trigger_manager::inst()->update_regions();

  // update only triggers that reside in active regions

  //BETH
  if(FEDone())
  {
	  region_list::const_iterator ri;
	  for (ri = the_terrain->get_regions().begin(); ri != the_terrain->get_regions().end(); ++ri)
	  {
		  region *r = *ri;
		  if (r->is_active())
		  {
			  region::trigger_list triggers = r->get_triggers();
			  region::trigger_list::const_iterator ti;
			  for (ti = triggers.begin(); ti != triggers.end(); ++ti)
			  {
				  trigger *t = *ti;
				  if ( t )
					  t->update();
			  }
		  }
	  }
  }//BETH

  ei_end = parented_entities.end();
  for ( ei=parented_entities.begin(); ei<ei_end; ++ei )
  {
	  entity * e = *ei;
	  if (e)
		  e->update_region();
  }


  g_particle_cleaner->clean_up_particles();

#if defined(TARGET_PC) && _ENABLE_WORLD_EDITOR
  frame_advance_all_dialogs(t);
#endif

#ifndef BUILD_BOOTABLE
  frame_advance_markers(t);
  frame_advance_debug_text(t);
#endif

#ifdef PROJECT_KELLYSLATER

  process_kelly_slater_stuff();

#endif // PROJECT_KELLYSLATER

#if defined(PROJECT_KELLYSLATER) && defined(DEBUG)
  if (gde) strcpy(gde_name, gde->get_id().get_val().c_str());
#endif
}

extern bool loading_from_scn_file;  // see wds.cpp for comments
#ifdef DEBUG
//H extern bool g_dump_frame_info;
#endif

void world_dynamics_system::add_marker( entity_id& _id )
{
	marker* marker_ptr = NEW marker(_id);
	marker_ptr->set_flag( EFLAG_MISC_NONSTATIC, true );
	add_marker( marker_ptr );
}

beam* world_dynamics_system::add_beam( const entity_id& _id, unsigned int _flags )
{
	beam* b = NEW beam(_id,_flags);
	add_beam( b );
	return b;
}

entity* world_dynamics_system::create_preloaded_entity_or_subclass( const stringx& entity_name,
																   const stringx& entity_dir )
{
	entity* e;
	filespec spec( entity_name );
	spec.name.to_upper();
	entfile_map::const_iterator fi = get_entfiles().find( spec.name );

	po loc( po_identity_matrix );
	loc.set_translate( vector3d(-9000,-9000,-9000) );

	if ( fi == get_entfiles().end() )
	{
		if ( entity_dir.size() > 0 )
			g_file_finder->push_path_back( entity_dir );
			/*P
			int alloc0 = memtrack::get_total_alloced();
			int script_mem = membudget()->get_usage( membudget_t::SCRIPTS );
			int anim_mem = membudget()->get_usage( membudget_t::ANIMS );
			int sound_mem = membudget()->get_usage( membudget_t::SOUNDS );
			g_memory_context.push_context( "ENTPRELOADS" );
		P*/
		e = g_entity_maker->create_entity_or_subclass( entity_name,  entity_id::make_unique_id(), loc, empty_string, 0 );
		/*P
		g_memory_context.pop_context();
		script_mem = membudget()->get_usage( membudget_t::SCRIPTS ) - script_mem;
		anim_mem = membudget()->get_usage( membudget_t::ANIMS ) - anim_mem;
		sound_mem = membudget()->get_usage( membudget_t::SOUNDS ) - sound_mem;
		int non_entity_mem = script_mem + anim_mem + sound_mem;
		membudget()->use( membudget_t::ENTPRELOADS, memtrack::get_total_alloced()-alloc0-non_entity_mem );
		P*/
		remove_entity_from_world_processing( e );
		if ( entity_dir.size() > 0 )
			g_file_finder->pop_path_back();
	}
	else
		e = (*fi).second;
	return e;
}

float world_dynamics_system::get_surface_effect_duration( int surf_index )
{
	surfaceinfo_list_t::value_type::second_type v = surfaceinfo_list[ surf_index ];
	return v ? v->effect_duration : 0.0f;
}

stringx world_dynamics_system::get_surface_sound_name( int surf_index )
{
	surfaceinfo_list_t::value_type::second_type v = surfaceinfo_list[ surf_index ];
	return v ? v->sound_name : stringx();
}

stringx world_dynamics_system::get_surface_effect_name( int surf_index )
{
	surfaceinfo_list_t::value_type::second_type v = surfaceinfo_list[ surf_index ];
	return v ? v->effect_name : stringx();
}

void world_dynamics_system::load_scene_anim( const stringx &filename )
{
	//P  int alloc0 = memtrack::get_total_alloced();
	//P  g_memory_context.push_context( "ANIMS" );

	scene_anim* snm_pt = NEW scene_anim;
	snm_pt->load( filename );
	scene_anim_map[ filename ] = snm_pt;

	/*P
	g_memory_context.pop_context();
	membudget()->use( membudget_t::ANIMS, memtrack::get_total_alloced()-alloc0 );
	P*/
}


void world_dynamics_system::add_light_source( light_source* ls )
{
	g_entity_maker->create_entity( ls );
	lights.push_back( ls );
}

void world_dynamics_system::remove_light_source( light_source* ls )
{
	vector<light_source*>::iterator lit;
	lit = find( lights.begin(), lights.end(), ls );
	if (lit!=lights.end())
		lights.erase( lit );
	if ( remove_entity( ls ) )
		delete ls;
}


void world_dynamics_system::add_marker( marker* e )
{
	g_entity_maker->create_entity( e );
}

void world_dynamics_system::add_beam( beam* e )
{
	g_entity_maker->create_entity( e );
}

void world_dynamics_system::add_camera( camera* e )
{
	g_entity_maker->create_entity( e );
}

void world_dynamics_system::add_mic( mic* e )
{
	g_entity_maker->create_entity( e );
}

void world_dynamics_system::add_particle_generator( particle_generator* pg )
{
	g_entity_maker->create_entity( pg );
}

void world_dynamics_system::add_lensflare( lensflare* e )
{
	g_entity_maker->create_entity( e );
}

void world_dynamics_system::add_item( item* it )
{
	items.push_back( it );
	// this needs to be called to set up last-frame info
	it->frame_done();
}

#if 0 //BIGCULL
void world_dynamics_system::add_turret( turret* cg )
{
	g_entity_maker->create_entity( cg );
}
#endif

// This creates an instance of the named effect (assumed to be an .ent file
// located in the fx\ directory) and adds it to the list of effects to be
// destroyed after expiration.
entity* world_dynamics_system::add_time_limited_effect( const char* name,
													   const po& loc,
													   time_value_t duration )
{
	entity* e = g_entity_maker->create_entity_or_subclass( name, entity_id::make_unique_id(), loc, "fx\\", ACTIVE_FLAG|NONSTATIC_FLAG );

	assert(e != NULL);

	time_limited_entities.push_back( ent_time_limit(e,duration) );
	e->set_time_limited(true);

	// this needs to be called to set up last-frame info
	e->frame_done();
	return e;
}

void world_dynamics_system::make_time_limited( entity* e, time_value_t duration )
{
	assert(e != NULL);

	time_limited_entities.push_back( ent_time_limit(e,duration) );
	e->set_time_limited(true);
}


// The world assumes responsibility for deleting anims that get added here.
// NOTE: this function assumes the given anim has not previously been added
void world_dynamics_system::add_anim( entity_anim_tree* new_anim )
{
	pentity_anim_tree_vector::iterator i = find( anims.begin(), anims.end(), (entity_anim_tree*)NULL );

	if ( i == anims.end() )
		anims.push_back( new_anim );
	else
		(*i) = new_anim;
}

// This deconstructs the given anim.
void world_dynamics_system::kill_anim( entity_anim_tree* the_anim )
{
	pentity_anim_tree_vector::iterator i = find( anims.begin(), anims.end(), the_anim );
	if ( i != anims.end() )
	{
		entity_anim_tree* a = *i;
		a->get_entity()->clear_anim( a );
		*i = NULL;
	}
}

ett_manager *world_dynamics_system::get_ett_manager()
{
  return ett_mgr;
}

bool world_dynamics_system::eligible_for_frame_advance( entity_anim_tree* anm ) const
{
	if ( anm->is_suspended() || (anm->is_finished() && anm->is_done_tween()) )  // don't advance suspended or finished anim
		return false;

	entity* e = anm->get_entity();

	if(e == NULL || !e->is_visible())	// don't frame_advance our despawned water objects (dc 10/15/01)
		return(false);

	return (
        ( !anm->is_relative_to_start()
		|| anm->is_noncosmetic()                          // always advance NONCOSMETIC anims
		|| e->is_flagged( EFLAG_MISC_IN_VISIBLE_REGION )  // or if entity is currently visible
		|| (e->has_ai_ifc() && e->ai_ifc()->is_active() && !e->ai_ifc()->cosmetic() && !e->ai_ifc()->is_disabled())
		|| (!e->is_flagged(EFLAG_GRAPHICS) && e->is_in_active_region())  // or, if non-graphical, is in an active region
		|| !anm->is_looping()                             // or anim is not looping
		|| anm->get_time() == 0.0f                        // or at start of looping anim
		|| !anm->is_done_tween()
		)
        );
}


// these are created elsewhere.  Will need to become like add_joint if
// we add a remove_controller.  Also in frame_advance.
int world_dynamics_system::add_controller(controller * new_controller)
{
	controllers.push_back(new_controller);
	return controllers.size();
}


void world_dynamics_system::set_ks_controller(int hero_num, kellyslater_controller * new_controller)
{
	assert(ks_controller[hero_num] == NULL);      // should only be setting these once
	ks_controller[hero_num] = new_controller;
	ks_controller[hero_num]->set_player_num(hero_num);
}

// these are created elsewhere.  Will need to become like add_joint if
// we add a remove_mcs.  Also in frame_advance.
int world_dynamics_system::add_mcs(motion_control_system * new_mcs)
{
	mcs_list.push_back(new_mcs);
	return mcs_list.size();
}


// these are created elsewhere.  Will need to become like add_joint if
// we add a remove_fcs.  Also in frame_advance.
int world_dynamics_system::add_fcs(force_control_system * new_fcs)
{
	fcs_list.push_back(new_fcs);
	return fcs_list.size();
}


// these are created elsewhere.  Will need to become like add_joint if
// we add a remove_generator.  Also in frame_advance.
int world_dynamics_system::add_generator(force_generator * new_generator)
{
	generators.push_back(new_generator);
	return generators.size();
}


void world_dynamics_system::enable_marky_cam( bool enable, rational_t priority )
{
	assert(marky_cam);

	if((enable && priority >= marky_cam->get_priority()) || (!enable && priority == marky_cam->get_priority()))
	{
		marky_cam_enabled = enable;
		marky_cam->camera_set_roll(0.0f);

		if(enable)
		{
			marky_cam->set_priority(priority);
		}
		else
		{
			marky_cam->reset_priority();
		}
	}
}


void world_dynamics_system::add_to_entities( entity *e )
{
	vector<entity*>::iterator it;
	assert(find( entities.begin(), entities.end(), e) == entities.end());	// don't add the same entity twice
	it = find( entities.begin(), entities.end(), (entity*)NULL);
	if ( it == entities.end() )
	{
		assert(e->get_bone_idx() < 0);
		entities.push_back( e );
	}
	else
		*it = e;
}


void world_dynamics_system::add_dynamic_instanced_entity( entity* e )
{
	vector<entity*>::iterator it;
	assert(find( entities.begin(), entities.end(), e) == entities.end());	// don't add the same entity twice
	it = find( entities.begin(), entities.end(), (entity*)NULL);
	if ( it == entities.end() )
	{
		assert(e->get_bone_idx() < 0);
		entities.push_back( e );
	}
	else
		*it = e;
	// non-uniform scaling is not allowed;
	// uniform scaling is allowed only on entities that have no collision geometry
	check_po( e );
	e->compute_sector( *the_terrain, loading_from_scn_file );
	// some flavors of entity are inactive by default
	switch ( e->get_flavor() )
	{
    case ENTITY_ENTITY:
    case ENTITY_MARKER:
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

bool world_dynamics_system::remove_entity( unsigned int i )
{
	// remove it from file
	assert( i < entities.size() );
	return remove_entity(entities[i]);
}


bool world_dynamics_system::remove_entity( entity *e )
{
	vector<entity*>::iterator it;
	bool success = false;
	it = find( entities.begin(), entities.end(), e );
	if ( it != entities.end() )
	{
		*it = NULL;
		success = true;
	}
	it = find( active_entities.begin(), active_entities.end(), e );
	if ( it != active_entities.end() )
		*it = NULL;
	it = find( guaranteed_active_entities.begin(), guaranteed_active_entities.end(), e );
	if ( it != guaranteed_active_entities.end() )
		*it = NULL;

	// remove it from file list...this is weird, because you could
	// add item a to the list, add item b, and then remove item a,
	// and the game would load the file rather than instance it.
	// generally, we shouldn't be loading/unloading stuff on the fly,
	// but that's currently the way spells are designed
	entfile_map::iterator fi;
	for( fi = entfiles.begin(); fi != entfiles.end(); ++fi )
	{
		if( (*fi).second == e )
		{
			entfiles.erase( fi );
			break;
		}
	}
	return success;
}


bool world_dynamics_system::remove_item( item * iptr )
{
	vector<item *>::iterator it;
	bool outval = false;
	int i;
	for (it = items.begin(), i=0; it!=items.end(); ++it, ++i)
	{
		if (iptr==(*it))
		{
			remove_item(i);
			outval = true;
			break;
		}
	}
	return outval;
}


bool world_dynamics_system::remove_item( unsigned int i )
{
	items[i] = NULL;
	return true;
}


// remove and delete entity if present; note that some flavors are disallowed
void world_dynamics_system::destroy_entity( entity* e )
{
	switch ( e->get_flavor() )
	{
    case ENTITY_ITEM:
		error( "Unsupported flavor for destruction: %s", entity_flavor_names[e->get_flavor()] );

    case ENTITY_LIGHT_SOURCE:
		remove_light_source( static_cast<light_source*>(e) );
		break;

    default:
		if ( remove_entity( e ) )
			delete e;
		break;
	}
}


entity *world_dynamics_system::get_entity(const stringx &name)
{
	return(entity_manager::inst()->find_entity(entity_id::make_entity_id(name.c_str()),IGNORE_FLAVOR,FIND_ENTITY_UNKNOWN_OK));
}


#ifndef BUILD_BOOTABLE
// for debugging purposes; dump information on all threads to a file
void world_dynamics_system::dump_frame_info()
{
	static unsigned dump_frame_info_counter = ~0;
	static hires_clock_t xTPS;

	if(dump_frame_info_counter == (unsigned)~0)
	{
		dump_frame_info_counter = 0;
		xTPS.reset();
	}
	else
	{
		time_value_t delta = xTPS.elapsed_and_reset();
		dump_frame_info_counter += (unsigned)(delta*1000.0f);
	}

	if(dump_frame_info_counter > 3000)
	{
		unsigned i;
		host_system_file_handle outfile = host_fopen( "frameinfo.txt", HOST_WRITE );
		host_fprintf( outfile, "Render Data\n" );

		host_fprintf( outfile, "  Regions\n" );
		for (i=0;i<to_render.regions.size();++i)
		{
			host_fprintf( outfile, "    %s\n", to_render.regions[i].reg->get_data()->get_name().c_str());
		}

		host_fprintf( outfile, "  Entities\n" );
		for (i=0;i<to_render.entities.size();++i)
		{
			entity * e = to_render.entities[i].ent;
			host_fprintf( outfile, "    (%s) %s : %s\n",
				e->get_region()?e->get_region()->get_data()->get_name().c_str():
			"",entity_flavor_names[e->get_flavor()], e->get_id().get_val().c_str() );
		}
		host_fprintf( outfile, "\n" );

		host_fprintf( outfile, "Active entities\n" );
		vector<entity *>::const_iterator en;
		for (en = (get_active_entities()).begin(); en!=(get_active_entities()).end(); ++en)
		{
			entity * e  = (*en);
			host_fprintf( outfile, "  %s : %s \n", entity_flavor_names[e->get_flavor()],
				e->get_id().get_val().c_str());
		}

		host_fprintf( outfile, "\nActive animations\n" );

		pentity_anim_tree_vector::iterator ani;
		for ( ani=anims.begin(); ani!=anims.end(); ++ani )
		{
			if ( (*ani) )
				if( (!(*ani)->get_entity()->has_valid_sector()) || (*ani)->get_entity()->is_in_active_region() ||
					((*ani)->get_flags()&ANIM_COMPUTE_SECTOR))
					if (!(*ani)->is_suspended() && !(*ani)->is_finished() )
					{
						entity * e = (*ani)->get_entity();
						host_fprintf( outfile, "  Anim : %s \n",  e->get_id().get_val().c_str()); //(*ani)->get_name().c_str(),
					}
		}

		host_fprintf( outfile, "\n" );

#ifdef PROFILING_ON
		profiler::inst()->write_to_host_file(outfile);
#endif

		host_fclose( outfile );

		g_debug.dump_frame_info = false;
		dump_frame_info_counter = ~0;
	}
}
#endif

path_graph *world_dynamics_system::get_path_graph(stringx id)
{
	vector<path_graph *>::iterator i = path_graph_list.begin();
	while(i != path_graph_list.end())
	{
		if(*i != NULL && (*i)->id == id)
			return((*i));

		++i;
	}

	return(NULL);
}

void world_dynamics_system::add_path_graph(path_graph *pg)
{
	if(pg && get_path_graph(pg->id) == NULL)
		path_graph_list.push_back(pg);
}

void world_dynamics_system::remove_path_graph(path_graph *pg)
{
	vector<path_graph *>::iterator i = path_graph_list.begin();
	while(i != path_graph_list.end())
	{
		if(*i != NULL && *i == pg)
		{
			path_graph *pg_del = (*i);
			i = path_graph_list.erase(i);
			delete pg_del;
		}
		else
			++i;
	}
}

void world_dynamics_system::add_material_set( material_set *mset )
{
	material_set_list::iterator msi;
	for( msi = material_sets.begin(); msi != material_sets.end(); ++msi )
		if( *msi == mset )
			return;
		material_sets.push_back( mset );
}


void world_dynamics_system::add_material_set( const stringx& fname )
{
	chunk_file      fs;
	stringx         instr;
	material_set    *ms;
	material        *mt;
	stringx         *st;
	unsigned        add_flgs = 0;
	material_list   *ml;

	wds_open( fs, fname, ".mts", os_file::FILE_READ | chunk_file::FILE_TEXT );

	serial_in( fs, &instr );
	if( !(instr == "mat_set") ) //error( "Invalid material file " + scene_root + fname + ".mts" );
	{
		fs.close();
		return;
	}
	st = NEW stringx;
	serial_in( fs, st );
	if( get_material_set( *st ) != NULL )
	{
		delete st;
		fs.close();
		return;
	}

	filespec fspec( fname );
	stringx texdir = fspec.path + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";

	ml = NEW material_list;
	for( serial_in(fs, &instr); instr != chunkend_label; serial_in(fs, &instr) )
	{
		if( instr == "additional_flags" )
		{
			serial_in( fs, &add_flgs );
			serial_in( fs, &instr );
		}
		else if( instr == "material" )
		{
			mt = material_bank.new_instance( material( fs, texdir, add_flgs, 0 ) );  // FIXME:  is the final '0' correct?
			(*ml).push_back( mt );
		}
	}
	ms = NEW material_set;
	ms->name = st;
	ms->data = ml;
	material_sets.push_back( ms );

	fs.close();
}


void world_dynamics_system::delete_material_set( material_set *mset )
{
	material_set_list::iterator msi;
	for( msi = material_sets.begin(); msi != material_sets.end(); ++msi )
		if( *msi == mset ) break;
		if( msi != material_sets.end() ) material_sets.erase( msi );
}


void world_dynamics_system::delete_material_set( stringx& set_name )
{
	material_set_list::iterator msi;
	for( msi = material_sets.begin(); msi != material_sets.end(); ++msi )
		if( *((*msi)->name) == set_name ) break;
		if( msi != material_sets.end() ) material_sets.erase( msi );
}


material_set *world_dynamics_system::get_material_set( const stringx& set_name )
{
	material_set_list::iterator msi;
	for( msi = material_sets.begin(); msi != material_sets.end(); ++msi )
		if( *((*msi)->name) == set_name ) break;
		if( msi != material_sets.end() ) return *msi;
		return NULL;
}


extern profiler_timer proftimer_collide;
extern profiler_timer proftimer_collide_entity_entity;
extern profiler_timer proftimer_collide_entity_entity_int;

void world_dynamics_system::do_all_collisions(time_value_t t)
{
	proftimer_collide.start();

	// must go back and recalculate with NEW position
	// This code is only being done on characters, so why not just use the 'active_characters' list? (JDB 9/7/00)
	vector<entity *>::const_iterator ei = active_entities.begin();
	vector<entity *>::const_iterator ei_end = active_entities.end();
	for ( ; ei!=ei_end; ++ei )
	{
		if((*ei) && (*ei)->get_colgeom() && (*ei)->get_colgeom()->get_type()==collision_geometry::CAPSULE)
		{
			(*ei)->update_colgeom();

			if((*ei)->has_physical_ifc())
			{
				if(!(*ei)->playing_scene_anim() && (*ei)->physical_ifc()->is_enabled() && !(*ei)->physical_ifc()->is_suspended()/* && !(*ei)->is_hero()*/)
					(*ei)->physical_ifc()->manage_standing();

					/*
					if((*ei)->physical_ifc()->is_standing())
					{
					// cancel xz vel (manage standing used to do this when standing)
					vector3d vel = (*ei)->physical_ifc()->get_velocity();
					vel.x = vel.z = 0.0f;
					(*ei)->physical_ifc()->set_velocity(vel);
					}
					else
					{
					//          if((*ei)->physical_ifc()->get_collided_last_frame())
					//            (*ei)->physical_ifc()->set_standing(true);
					}
				*/
			}
		}
	}

	do_entity_to_entity_collisions(t);

	// make sure characters don't tunnel through the world
	do_entity_to_bsp_collisions(t);

	//*
	ei = active_entities.begin();
	for ( ; ei!=ei_end; ++ei )
	{
		if((*ei) && (*ei)->get_colgeom() && (*ei)->get_colgeom()->get_type()==collision_geometry::CAPSULE && (*ei)->has_physical_ifc() && !(*ei)->playing_scene_anim() && (*ei)->physical_ifc()->is_enabled() && !(*ei)->physical_ifc()->is_suspended())
			(*ei)->physical_ifc()->manage_standing((*ei)->physical_ifc()->is_effectively_standing());
	}
	//*/

	proftimer_collide.stop();
}


void world_dynamics_system::do_entity_to_bsp_collisions(time_value_t t)
{
	vector<entity *>::const_iterator ei = collision_entities.begin();
	vector<entity *>::const_iterator ei_end = collision_entities.end();
	for ( ; ei != ei_end; ++ei )
	{
		entity *ent = *ei;

		assert(ent && ent->is_active() && ent->are_collisions_active() && ent->has_entity_collision());
		//    if ( !ent ) continue;
		if ( !ent->has_physical_ifc() ) continue;
		//    if ( !ent->is_flagged(EFLAG_PHYSICS) ) continue;
		//    if ( !ent->is_active() ) continue;
		//    if ( !ent->are_collisions_active() ) continue;
		//    if ( !ent->has_entity_collision() ) continue;
		if ( ent->is_stationary() ) continue;

		vector3d pi, n;
		bool bad_collision = false;
		vector3d collision_delta = ZEROVEC;
		if ( ent->is_hero() )
		{
			// hero does a more rigorous check that includes collidable entities
			vector3d oldp = ent->get_last_position();
			vector3d newp = ent->get_abs_position();
			newp -= oldp;
			rational_t d = newp.length2();
			if ( d > 0.000001f )
			{
				d = __fsqrt( d );
				newp *= (d + ent->get_colgeom()->get_core_radius()) / d;
			}
			newp += oldp;
			bool coll_active = ent->are_collisions_active();
			ent->set_collisions_active(false, false);
			if ( find_intersection( oldp, newp,
				ent->get_primary_region(),
				FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
				&pi, &n ) )
			{
				bad_collision = true;
				collision_delta = newp;
			}
			ent->set_collisions_active(coll_active, false);
		}
		else if ( get_the_terrain().find_intersection( ent->get_last_position(), ent->get_abs_position(), pi, n) )
		{
			collision_delta = ent->get_abs_position() - ent->get_last_position();
			collision_delta.normalize();
			bad_collision = true;
		}

		if ( bad_collision && n.length2() > 0.0f )
		{
			//      collision_delta
			po newpo = ent->get_abs_po();
			newpo.set_position( ent->get_last_position() );

			if(ent->has_parent())
				fast_po_mul(newpo, newpo, ent->link_ifc()->get_parent()->get_abs_po().inverse());

			ent->set_rel_po( newpo );
			if ( ent->has_physical_ifc() )
			{
				ent->physical_ifc()->set_velocity( ZEROVEC );
				ent->physical_ifc()->set_collided_last_frame( true );
				ent->physical_ifc()->set_last_collision_normal( n );
				ent->physical_ifc()->set_ext_collided_last_frame( true );
			}
		}
	}
}


//#define COLLISION_NAMES
#if defined(BUILD_DEBUG) && defined(COLLISION_NAMES)
stringx ent_i_name;
stringx ent_j_name;
#endif

void world_dynamics_system::do_entity_to_entity_collisions(time_value_t t)
{
	proftimer_collide_entity_entity.start();

	// Two passes, first char's against each other, then chars against environment
	int pass;
	for (pass = 0;pass<2;++pass)
	{
		entity::prepare_for_visiting();

		vector<entity *>::const_iterator ei = collision_entities.begin();
		vector<entity *>::const_iterator ei_end = collision_entities.end();

		for ( ; ei!=ei_end; ++ei )
		{
			entity* ent_i = (*ei);
			assert(ent_i && ent_i->is_active() && ent_i->are_collisions_active() && ent_i->has_entity_collision());
#if defined(BUILD_DEBUG) && defined(COLLISION_NAMES)
			ent_i_name = ent_i->get_name();
#endif

			if (  //ent_i
				//&& ent_i->is_flagged( EFLAG_PHYSICS )
				//            && ent_i->is_active()
				//            && ent_i->are_collisions_active()
				//            && ent_i->has_entity_collision()
				!ent_i->is_stationary()
				&& ent_i->get_colgeom()->get_type() == collision_geometry::CAPSULE
				)
			{
				// This was moved to a loop in do all collisions because this would sometimes clear the collision flag on a collided entity inadvertently.
				// Could cause some problems

				if ( 1 /*(ent_i->get_collision_flags() & actor::COLLIDE_WITH_ACTORS)*/ )
				{
					rational_t r_i = ent_i->get_radius();
					vector3d vel;
					ent_i->get_velocity( &vel );

					if ( ent_i->has_parent() && ent_i->is_frame_delta_valid() )
					{
						// adjust for non-physical motion (e.g., animation)
						vel += ent_i->get_frame_delta().get_position() * (1.0f / cur_time_inc);
					}

					r_i += vel.length()*cur_time_inc;
					//          rational_t r_i_sq = r_i * r_i;
					//          rational_t r_i_x2 = 2.0f * r_i;

					//          const vector3d& pos_i = ent_i->get_abs_position();
					// mark the current source entity as visited
					ent_i->visit();

#if 0 //ndef REGIONCULL
					//DAJ I'm initializing "hit" here because it isn't initialized anywhere
					// else and it was getting used without initialization
					bool hit;

					//          int count = ent_i == get_hero_ptr(g_game_ptr->get_active_player()) ? 20 : 5;
					entity *hit_ent = NULL;
					do
					{
						hit = false;
						// for all regions occupied by this entity
						entity::prepare_for_visiting2();
						region_node_pset::const_iterator rni;
						for ( rni=ent_i->get_regions().begin(); rni!=ent_i->get_regions().end(); rni++ )
						{
							assert( *rni );
							region* rg = (*rni)->get_data();
							// first collide with x_sorted entities (these are entities with
							// world-space bounding boxes; they are for the moment all static
							// and flavor ENTITY_ENTITY)
							region::entity_list::const_iterator ej;
							region::entity_list::const_iterator ej_end;
							if (pass==1)
							{
								int low = rg->get_low_xsorted_entity( pos_i.x - r_i );
								if ( low >= 0 )
								{
									int high = rg->get_high_xsorted_entity( pos_i.x + r_i );
									if ( high >= low )
									{
										ej = rg->get_x_sorted_entities().begin() + low;
										ej_end = rg->get_x_sorted_entities().begin() + high;
										for ( ; ej<=ej_end; ++ej )
										{
											entity* ent_j = *ej;
											if ( ent_j
												&& ent_j->are_collisions_active()
												&& !ent_j->already_visited()
												&& !ent_j->already_visited2()
												)
											{
#if defined(BUILD_DEBUG) && defined(COLLISION_NAMES)
												ent_j_name = ent_j->get_name();
#endif
												ent_j->visit2();
												if ( ent_j->get_bounding_box().intersect( ent_i->get_abs_position(), r_i ) )
												{
													if ( entity_entity_collision(ent_i, ent_j, t) )
													{
														hit = true;
														hit_ent = ent_j;
													}
												} // bounding box intersects
											} // not already visited
										} // each x-sorted entity
									} // high >= low
								} // low >= 0
							} // pass==1
							// now collide with other entities
							if (pass==0)
							{
								//                ej = rg->get_entities().begin();
								//                ej_end = rg->get_entities().end();
								ej = rg->get_possible_collide_entities().begin();
								ej_end = rg->get_possible_collide_entities().end();

								for ( ; ej!=ej_end; ++ej )
								{
									entity* ent_j = *ej;

									if (  ent_j
										&& ent_j->are_collisions_active()
										&& (ent_j->from_sin_file() || !ent_j->has_bounding_box())
										&& !ent_j->already_visited()
										&& !ent_j->already_visited2()
										&& (ent_j->get_colgeom() && (ent_j->get_colgeom()->get_type() != collision_geometry::CAPSULE || ent_j->is_visible()))
										)
									{
#if defined(BUILD_DEBUG) && defined(COLLISION_NAMES)
										ent_j_name = ent_j->get_name();
#endif

										ent_j->visit2();
										if (  //ent_j->is_flagged( EFLAG_PHYSICS )
											ent_j->has_entity_collision()
											&& ent_j->are_collisions_active()
											)
										{
											rational_t r_j = ent_j->get_radius();
											if ( (pos_i - ent_j->get_abs_position()).length2() < r_i_sq+r_j*r_j+r_i_x2*r_j )
											{
												if ( entity_entity_collision(ent_i, ent_j, t) )
													if ( ent_j->get_colgeom()->get_type() == collision_geometry::CAPSULE )
													{
														hit = true; // don't do multiple collisions on character-character collisions
														hit_ent = ent_j;
													}
											}
										}
									}
								} // each entity in region
							} // pass==0
						} // each region ent_i is in
					} while ( --count >= 0 && hit );
					if ( hit )
					{
						po newpo = ent_i->get_abs_po();
						newpo.set_position( ent_i->get_last_position() );
						vector3d pre_hit_pos = ent_i->get_abs_position();

						if(ent_i->has_parent())
							fast_po_mul(newpo, newpo, ent_i->link_ifc()->get_parent()->get_abs_po().inverse());

						ent_i->set_rel_po( newpo );

						if( ent_i->has_physical_ifc() )
						{
							ent_i->physical_ifc()->set_velocity( ZEROVEC );
							ent_i->physical_ifc()->set_collided_last_frame( true );

							vector3d norm = (newpo.get_position() - pre_hit_pos);
							norm.normalize();

							if(norm.length2() <= 0.1f)
							{
								if(hit_ent)
								{
									norm = ent_i->get_abs_position() - hit_ent->get_abs_position();
									norm.normalize();

									if(norm.length2() <= 0.1f)
										norm = -ent_i->get_abs_po().get_facing();
								}
								else
									norm = -ent_i->get_abs_po().get_facing();
							}

							ent_i->physical_ifc()->set_last_collision_normal( norm );
							ent_i->physical_ifc()->set_ext_collided_last_frame( true );
						}
					}
#endif
        } // if ent_i collides with actors
      } // if ent_i satisfies a bunch of conditions
    } // each active entity
  } // pass

  proftimer_collide_entity_entity.stop();
}

// *permanent:

#ifdef _DEBUG
stringx g_colla1, g_colla2;
bool g_stationary_a1, g_stationary_a2;
#endif


// shared data for the entity_entity_collision_check and the entity_entity_collision fn's
static bool terrain_collision = false;
static vector3d abs_base_point;


// does the checking portion of an ent/ent collision, but no resolving
bool world_dynamics_system::entity_entity_collision_check(entity * a1, entity * a2, time_value_t time, cface * hitFace)
{
	// following should be removed when fixed up. will slow things down badly.....
	terrain_collision = false;
	entity * b1, * b2;

	if (a1->is_active() && a1->is_sticky() && !a2->is_active())
	{
		terrain_collision = true;
	}

	po old_a1_rel_po = a1->get_rel_po();
	po old_a2_rel_po = a2->get_rel_po();

	entity * old_a1_parent = (entity *)a1->link_ifc()->get_parent();
	entity * old_a2_parent = (entity *)a2->link_ifc()->get_parent();

	// If either a1 or a2 is stationary, resolve the collision in the local space of that entity.
	// To facilitate this,  we create a fake entity for the non-stationary entity and mangle its data instead,
	// of corrupting the actual entity.

	b1 = a1;
	b2 = a2;

	bool character_collision_hack = false;
	if (!b1->is_stationary() && !b2->is_stationary())
	{
		// This flag will cause this routing to use the damage capsules rather than the normal collision
		// geometries for two colliding entities.  It assumes that any two such have valid damage_capsules,
		// i.e. are characters.  This causes them to have longer capsules (covering the
		// characters' feet) so that characters will better retard each other.
		character_collision_hack = true;
	}

	if (b1->is_stationary())
	{
		if ( b1->get_colgeom() )
		{
			// b1 has instanced collision geometry, so we need to set its owner to myself for now.
			b1->get_colgeom()->set_owner(b1);
			b1->get_colgeom()->validate();
			origin_entity->set_radius(b1->get_radius());

			// Now we move b2 into b1's local space.  We do this by creating a temporary entity with the
			// same collision_geometry, position and radius.  This gets around a sticky situation created
			// when we attached character's colgeom's to theiw waist instead of to the character itself.
			// (see character::get_colgeom_root_po())
			po b2_to_b1,last_b2_to_b1;
			fast_po_mul(b2_to_b1, b2->get_colgeom_root_po(), b1->get_abs_po().inverse());
			fast_po_mul(last_b2_to_b1, b2->get_last_po(), b1->get_abs_po().inverse());

			collision_dummy->set_rel_po(b2_to_b1);
			collision_dummy->set_last_po(last_b2_to_b1);
			collision_dummy->set_colgeom(b2->get_colgeom());
			collision_dummy->invalidate_colgeom();
			collision_dummy->set_radius(b2->get_radius());
			b2 = collision_dummy;
			b2->get_colgeom()->set_owner(b2);
			b2->invalidate_colgeom();
			b1->link_ifc()->clear_parent();
			b1->set_rel_po(po_identity_matrix);
		}
		else
		{
			nglPrintf("Crash averted but problem not fixed\n");
			return false;
		}
	}

	// mirror of above for b1, b2 role reversal...
	if (b2->is_stationary())
	{
		b2->get_colgeom()->set_owner(b2);
		b2->get_colgeom()->validate();
		origin_entity->set_radius(b2->get_radius());

		po b1_to_b2, last_b1_to_b2;
		fast_po_mul(b1_to_b2, b1->get_colgeom_root_po(), b2->get_abs_po().inverse());
		fast_po_mul(last_b1_to_b2, b1->get_last_po(), b2->get_abs_po().inverse());

		collision_dummy->set_rel_po(b1_to_b2);
		collision_dummy->set_last_po(last_b1_to_b2);
		collision_dummy->set_colgeom(b1->get_colgeom());
		collision_dummy->invalidate_colgeom();
		collision_dummy->set_radius(b1->get_radius());
		b1 = collision_dummy;
		b1->get_colgeom()->set_owner(b1);
		b1->invalidate_colgeom();
		b2->link_ifc()->clear_parent();
		b2->set_rel_po(po_identity_matrix);
	}

	rational_t radius1 = a1->get_colgeom()->get_core_radius(), radius2 = a2->get_colgeom()->get_core_radius();
	rational_t scale1 = 1.0f, scale2 = 1.0f;

	vector3d a1vel, a2vel;
	a1->get_velocity(&a1vel);
	if (time != 0.0f)
	{
		if ( a1->has_parent() && a1->is_frame_delta_valid() )
		{
			// adjust for non-physical motion (e.g., animation)
			a1vel += a1->get_frame_delta().get_position() * (1.0f / time);
		}
		rational_t delta = time * a1vel.length();
		if ( delta > 2.0f * radius1 )
			scale1 = (delta - radius1) / radius1;

		a2->get_velocity(&a2vel);
		if ( a2->has_parent() && a2->is_frame_delta_valid() )
		{
			// adjust for non-physical motion (e.g., animation)
			a2vel += a2->get_frame_delta().get_position() * (1.0f / time);
		}
		delta = time * a2vel.length();
		if ( delta > 2.0f * radius2 )
			scale2 = (delta - radius2) / radius2;
	}
	if ( character_collision_hack )
	{
		scale1 = a1->get_inter_capsule_radius_scale();
		scale2 = a2->get_inter_capsule_radius_scale();
	}

#if defined(VERBOSE_COLLISIONS)
	debug_print("cg1: scale: " + ftos(scale1) );
#endif
	collision_geometry *cg1 = b1->get_updated_colgeom( NULL, scale1 );
#if defined(VERBOSE_COLLISIONS)
	debug_print("cg2: scale: " + ftos(scale2) );
#endif
	collision_geometry *cg2 = b2->get_updated_colgeom( NULL, scale2 );

	unsigned int col_flags = 0;
	if ( !b2->is_stationary() )
		col_flags |= PP_FULL_MESH;
	if ( terrain_collision )
		col_flags |= ONE_HIT_PER_M2_POLY | PP_REAR_CULL;

	// We aren't using the massive list of collision vectors - this flag
	//   turns the check into a boolean check.
	col_flags |= ONE_HIT_PER_MESH;

	bool hit = (collision_geometry::collides(cg1,cg2,&hit_list,&normal_list1,&normal_list2,col_flags,a1vel-a2vel, hitFace));

	/*
	b1->get_updated_colgeom( NULL, (1.0f / scale1) );
	b2->get_updated_colgeom( NULL, (1.0f / scale2) );
	*/
	/*
	// default to ent 1's collision capsule (if both are non stationary)
	if (a1->get_colgeom() && a1->get_colgeom()->get_type()==collision_geometry::CAPSULE)
    abs_base_point = ((collision_capsule *)a1->get_colgeom())->get_base();
	else if (a2->get_colgeom() && a2->get_colgeom()->get_type()==collision_geometry::CAPSULE)
    abs_base_point = ((collision_capsule *)a2->get_colgeom())->get_base();
	else
    assert(0);
	*/
	// Restore positions afterwards...
	if (a2->is_stationary())
	{
		a2->link_ifc()->set_parent(old_a2_parent);
		a2->set_rel_po(old_a2_rel_po);
		a1->get_colgeom()->set_owner(a1);
		a1->invalidate_colgeom();
		po last_po = a1->get_last_po();
		a1->get_updated_colgeom( &last_po, scale1 );
		abs_base_point = ((collision_capsule *)a1->get_colgeom())->get_base();
		a1->invalidate_colgeom();
		last_po = a1->get_colgeom_root_po();
		a1->get_updated_colgeom( &last_po, scale1 );
	}

	if (a1->is_stationary())
	{
		a1->link_ifc()->set_parent(old_a1_parent);
		a1->set_rel_po(old_a1_rel_po);
		a2->get_colgeom()->set_owner(a2);
		a2->invalidate_colgeom();
		po last_po = a2->get_last_po();
		a2->get_updated_colgeom( &last_po, scale2 );
		abs_base_point = ((collision_capsule *)a2->get_colgeom())->get_base();
		a2->invalidate_colgeom();
		last_po = a2->get_colgeom_root_po();
		a2->get_updated_colgeom( &last_po, scale2 );
	}

	collision_dummy->set_colgeom(NULL);

	// This list seemed to never get cleared before, and eventually it started eating up memory.
	// Hope this is the right place! (dc 01/28/02)
	hit_list.resize(0);
	normal_list1.resize(0);
	normal_list2.resize(0);

	return hit;
}


// does the resolving portion of an ent/ent collision and calls the check fn (above) to do the
// checking itself
bool world_dynamics_system::entity_entity_collision(entity * a1, entity * a2, time_value_t time)
{
	proftimer_collide_entity_entity_int.start();

	bool hit = entity_entity_collision_check(a1, a2, time);

	//  if (collision_geometry::collides(cg1,cg2,&hit_list,&normal_list1,&normal_list2,terrain_collision?ONE_HIT_PER_M2_POLY|PP_REAR_CULL:0,a1->get_velocity()-a2->get_velocity()))
	if (hit)
	{
		vector3d old_a1_pos = a1->get_last_position();
		vector3d old_a2_pos = a2->get_last_position();
		// Treat collisions with static objects like terrain collisions
#if defined(VERBOSE_COLLISIONS)
		debug_print("collision detected: " + v3tos(a1->get_abs_position()) );
#endif
		if ((a1->is_active()==false || a1->get_flavor()==ENTITY_ENTITY || a1->is_stationary()) &&
			a1->get_colgeom()->get_type()==collision_geometry::MESH)
		{
			for (unsigned i=0;i<normal_list1.size();++i)
			{
				vector3d swap = normal_list2[i];
				normal_list2[i] = normal_list1[i];
				normal_list1[i] = swap;
			}

			if(a1->is_stationary())
				hit = physical_interface::resolve_collision_with_terrain(a2, time, a1->get_abs_po(), abs_base_point);
			else
				hit = physical_interface::resolve_collision_with_terrain(a2, time, po_identity_matrix, abs_base_point);
		}
		else if ((a2->is_active()==false || a2->get_flavor()==ENTITY_ENTITY || a2->is_stationary()) &&
			a2->get_colgeom()->get_type()==collision_geometry::MESH)
		{
			if(a2->is_stationary())
				hit = physical_interface::resolve_collision_with_terrain(a1, time, a2->get_abs_po(), abs_base_point);
			else
				hit = physical_interface::resolve_collision_with_terrain(a1, time, po_identity_matrix, abs_base_point);
		}
		else
			physical_interface::resolve_collision(a1, a2, time, terrain_collision);
		//      debug_print( "after collision response: " + v3tos(a1->get_abs_position()) );

		vector3d dir = a2->get_abs_position() - a1->get_abs_position();
		dir.normalize();

		if(a1->has_physical_ifc())
		{
			a1->physical_ifc()->set_collided_last_frame(true);
			a1->physical_ifc()->set_ext_collided_last_frame( true );
			a1->physical_ifc()->set_last_collision_normal( -dir );
		}

		//*
		if(a1->is_frame_delta_valid())
		{
			vector3d delta = (a1->get_abs_position() - old_a1_pos);
			a1->get_movement_info()->frame_delta.set_position(delta);
		}
		//*/
		if(a2->has_physical_ifc())
		{
			a2->physical_ifc()->set_collided_last_frame(true);
			a2->physical_ifc()->set_ext_collided_last_frame( true );
			a2->physical_ifc()->set_last_collision_normal( dir );
		}
		//*
		if(a2->is_frame_delta_valid())
		{
			vector3d delta = (a2->get_abs_position() - old_a2_pos);
			a2->get_movement_info()->frame_delta.set_position(delta);
		}
		//*/

		hit_list.resize(0);
		normal_list1.resize(0);
		normal_list2.resize(0);
	}

	proftimer_collide_entity_entity_int.stop();
	return hit;
}



// add given entity to the list of entities that are guaranteed to be active
// independent of visibility, region status, etc.
void world_dynamics_system::guarantee_active( entity* e )
{
	vector<entity*>::iterator i = find( guaranteed_active_entities.begin(), guaranteed_active_entities.end(), e );
	if ( i == guaranteed_active_entities.end() )
	{
		vector<entity*>::iterator i = find( guaranteed_active_entities.begin(), guaranteed_active_entities.end(), (entity*)NULL );
		if ( i == guaranteed_active_entities.end() )
		{
			guaranteed_active_entities.push_back( e );
		}
		else
			*i = e;
	}
}



///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

// TODO: check all NEWENT stuff against g_entity_maker->create_entity_or_subclass()


light_source*
world_dynamics_system::add_light_source( chunk_file& fs,
										const entity_id& id,
										unsigned int flags )
{
	light_source* ent = NEW light_source( fs, id, ENTITY_LIGHT_SOURCE, flags );
	add_light_source( ent );
	return ent;
}


particle_generator*
world_dynamics_system::add_particle_generator( const stringx& fname,
											  const entity_id& id,
											  unsigned int flags )
{
	filespec spec(fname);
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
	{
		stringx actual_fname (g_file_finder->find_file( fname, ".ent" ));
		spec = filespec(actual_fname);
	}

	// all particle generators added to world must be nonstatic
	flags |= EFLAG_MISC_NONSTATIC;
	particle_generator* ent = NEW particle_generator( spec.name, id, ENTITY_PARTICLE_GENERATOR, flags );

	add_particle_generator( ent );
	return ent;
}


item*
world_dynamics_system::add_item( chunk_file& fs,
								const entity_id& id,
								unsigned int flags )
{
	item* ent = NEW item( fs, id, ENTITY_ITEM, flags );
	add_item( ent );
	return ent;
}

polytube*
world_dynamics_system::add_polytube( chunk_file& fs,
                                    const entity_id& id,
                                    unsigned int flags )
{
	polytube* ent = NEW polytube( fs, id, ENTITY_POLYTUBE, flags );
	add_to_entities( ent );
	return ent;
}

lensflare*
world_dynamics_system::add_lensflare( chunk_file& fs,
									 const entity_id& id,
									 unsigned int flags )
{
	lensflare* ent = NEW lensflare( fs, id, ENTITY_LENSFLARE, flags );
	add_to_entities( ent );
	return ent;
}

#if 0 //BIGCULL
gun*
world_dynamics_system::add_gun( chunk_file& fs,
							   const entity_id& id,
							   unsigned int flags )
{
	gun* ent = NEW gun( fs, id, ENTITY_ITEM, flags );
	add_item( ent );
	return ent;
}

melee_item*
world_dynamics_system::add_melee( chunk_file& fs,
								 const entity_id& id,
								 unsigned int flags )
{
	melee_item* ent = NEW melee_item( fs, id, ENTITY_ITEM, flags );
	add_item( ent );
	return ent;
}

thrown_item*
world_dynamics_system::add_thrown_item( chunk_file& fs,
									   const entity_id& id,
									   unsigned int flags )
{
	thrown_item* ent = NEW thrown_item( fs, id, ENTITY_ITEM, flags );
	add_item( ent );
	return ent;
}

manip_obj*
world_dynamics_system::add_manip_obj( chunk_file& fs,
									 const entity_id& id,
									 unsigned int flags )
{
	manip_obj* ent = NEW manip_obj( fs, id, ENTITY_MANIP, flags );
	add_to_entities( ent );
	return ent;
}

switch_obj*
world_dynamics_system::add_switch_obj( chunk_file& fs,
									  const entity_id& id,
									  unsigned int flags )
{
	switch_obj* ent = NEW switch_obj( fs, id, ENTITY_SWITCH, flags );
	add_to_entities( ent );
	return ent;
}

turret*
world_dynamics_system::add_turret( chunk_file& fs,
								  const entity_id& id,
								  unsigned int flags )
{
	turret* ent = NEW turret( fs, id, ENTITY_TURRET, flags );

	add_turret( ent );

	return ent;
}

scanner* world_dynamics_system::add_scanner( chunk_file& fs,
											const entity_id& id,
											unsigned int flags )
{
	scanner* e = NEW scanner( fs, id, ENTITY_SCANNER, flags );
	add_scanner( e );
	return e;
}

void world_dynamics_system::add_scanner( scanner* e )
{
	g_entity_maker->create_entity( e );
}

#endif //BIGCULL
sky* world_dynamics_system::add_sky( chunk_file& fs, const entity_id& id, unsigned int flags )
{
	sky* ent = NEW sky( fs, id, ENTITY_SKY, flags );
	ent->set_flag( EFLAG_MISC_NONSTATIC, true );
	add_sky( ent );
	return ent;
}

void world_dynamics_system::add_sky( sky *e )
{
	g_entity_maker->create_entity( e );
}

// ok, I'm cleaning this portal stuff up.  Things are getting a lot simpler around here.

// Portals, once loaded, are always converted into convex quads in a counterclockwise
// orientation with the top/bottom edges aligned with the XZ plane and the side edges
// essentially vertical.
// This is so we won't have lots of vertices to deal with later on.
// If the original portal was non-planar, we will use artificially inflate the portal quad.

// Now that we have nice simple portals, all we do when we check portals for visibility
// is this:

//  A) Check portal bounding sphere against frustum planes.  If completely outside, skip it
//  B) IF   the portal cylinder touches the near plane rectangle
//     THEN consider portal too complicated to clip the frustum properly at this time
//          so we just leave the frustum as-is
//     ELSE we can build a screen-space bounding rectangle from the portal mesh
//          and use that to clip down the view frustum,
//  C) if view frustum is nonempty, recurse into the portal dest region

//  --Sean

#ifdef DEBUG
extern bool dump_portals_to_console;
#endif

#include <algorithm>

void world_dynamics_system::_build_render_data_regions( render_data& rd, region_node* rn, const rectf& screen_rect, camera& camera_link )
{
	region* rg = rn->get_data();
	// mark region as visited; this avoids re-processing the region within the
	// current recursion branch, but we free it up for any sibling recursion
	// branch to process later (see below)
	rg->visit();

	// mark region as active (note that this can fail if the region is locked)
	rg->set_active( true );

	// The rectangles we're dealing with here are actually in clip space, not screen space.
	// That is, after the projection matrix but before the perspective divide.
	// That makes it run from (-1,-1) to (1,1) but we invert Y compared to how the clip space is oriented.
	if ( rg->is_active() )
	{
		float aspectcorrection = -1.0f;//*PROJ_ASPECT;

		render_data::region_info temp_ri;
		temp_ri.reg = rn;
		render_data::region_list::iterator rii = find( rd.regions.begin(), rd.regions.end(), temp_ri );
		if ( rii == rd.regions.end() ) // first time seeing this region
		{
			// add NEW region to list (along with associated screen rect)
			render_data::region_info ris;
			ris.reg=rn;
			ris.screen_rect = screen_rect;
			rd.regions.push_back(ris);
			rii = rd.regions.end()-1;
		}
		else // already saw this region (perhaps through a different portal)
		{
			// merge NEW screen rectangle with previous
			(*rii).screen_rect += screen_rect;
		}
		const rectf& scrnrect = (*rii).screen_rect;
#ifdef DEBUG
		if (dump_portals_to_console)
			console_log("region %s screen rect %1.2f,%1.2f %1.2f,%1.2f",rg->get_name().c_str(),
			scrnrect.get_left(),scrnrect.get_top(),scrnrect.get_right(),scrnrect.get_bottom());
#endif

		hull& fi=rg->frustum();

		const matrix4x4& world2view  = geometry_manager::inst()->xforms[ geometry_manager::XFORM_WORLD_TO_VIEW ];
		const po& view2world = camera_link.get_abs_po();

		{
			// We're going to build a frustum from the camera projection parameters and the old clipspace rect.
			vector3d p0,p1,p2,p3;
			float zf=PROJ_FAR_PLANE_D;
			float xf=PROJ_ZOOM*zf;
			float yf=-PROJ_ASPECT*xf;
			// compute farplane rectangle in view space
			p0 = vector3d( scrnrect.get_left ()*xf, scrnrect.get_top   ()*yf, zf );
			p1 = vector3d( scrnrect.get_right()*xf, scrnrect.get_bottom()*yf, zf );
			p2 = vector3d( scrnrect.get_right()*xf, scrnrect.get_top   ()*yf, zf );
			p3 = vector3d( scrnrect.get_left ()*xf, scrnrect.get_bottom()*yf, zf );
			// transform farplane rectangle into world space
			p0 = view2world.non_affine_slow_xform( p0 );
			p1 = view2world.non_affine_slow_xform( p1 );
			p2 = view2world.non_affine_slow_xform( p2 );
			p3 = view2world.non_affine_slow_xform( p3 );

			// compute view frustum extrema
			vector3d p4 = view2world.get_position();
			vector3d p5 = view2world.get_z_facing();

			// compute frustum planes in world space
			fi.resize(0);
			fi.add_face(plane(p4, cross( p0, p3 ))); // left
			fi.add_face(plane(p4, cross( p1, p2 ))); // right
			fi.add_face(plane(p4 + p0, -p5 )); // back
			fi.add_face(plane(p4, cross( p2, p0 ))); // top
			fi.add_face(plane(p4, cross( p3, p1 ))); // bottom
			//fi.add_face(plane(p4 + p5*nearplane_d, p5)); // front

			//fi.bound_center = p4 + p5*(farplane_d*0.5f);
			//fi.bound_radius = (fi.bound_center-(p2+p4)).length(); // overestimates a little
		}

		// for each adjacent region connected by unused portals, compute a NEW view
		// frustum and, if nonempty, recurse
		for ( edge_iterator tei=rn->begin(); tei!=rn->end(); ++tei )
		{
			region_node* dest = (*tei).get_dest();
			portal* port = (*tei).get_data();

#ifdef DEBUG
			if (dump_portals_to_console)
				console_log("considering portal %s->%s",rg->get_name().c_str(),dest->get_data()->get_name().c_str());
#endif
			if (port->is_active() && !dest->get_data()->already_visited() )
			{
				// Region past portal has not been previously processed in my recursion branch.
				// Determine visibility
				const sphere& portal_bound = port->get_bound_sphere(); // in world space
				if (!fi.includes(portal_bound))
				{
#ifdef DEBUG
					if (dump_portals_to_console)
						console_log("  portal bound not in frustum");
#endif
					continue;
				}

				// check portal average normal facing (make sure we're looking through the right way for this edge)
				vector3d portal_normal = port->get_normal(port->get_back() != dest);
				vector3d cam2portal = view2world.get_position()-portal_bound.get_center();

				rational_t dp;
				dp = dot( view2world.get_z_facing(), portal_normal );
				if (dp < -0.8f)
				{
#ifdef DEBUG
					if (dump_portals_to_console)
						console_log("  not even remotely facing");
#endif
					continue;
				}

				vector3d portal_view_pos = xform3d_1(world2view, portal_bound.get_center());

				// the near plane check is more important for portals, because it can potentially
				// cull lots of other stuff in the tiny chance it rejects a portal
				if (portal_view_pos.z+portal_bound.get_radius() < PROJ_NEAR_PLANE_D)
				{
#ifdef DEBUG
					if (dump_portals_to_console)
						console_log("  trivial reject");
#endif
					continue; // behind near plane, no way to see it
				}

				// reduce frustum and screen rect by intersecting with portal
				{
					// do a more precise check to see if we're looking at the portal
					dp = dot( cam2portal, portal_normal );
					rational_t non_planar_fudge_factor = port->get_non_planar_fudge_factor(); // account for non-planar portals
					if ( dp*dp < non_planar_fudge_factor * cam2portal.length2() )
					{
#ifdef DEBUG
						if (dump_portals_to_console)
							console_log("  not looking at it");
#endif
						continue;
					}

					// kludge for trivial acceptance, camera inside portal bound cylinder
					if (port->touches_sphere(sphere(view2world.get_position()+view2world.get_z_facing()*(PROJ_NEAR_PLANE_D*0.75f),PROJ_NEAR_PLANE_D*1.5f)))
					{
#ifdef DEBUG
						if (dump_portals_to_console)
							console_log("  trivial accept");
#endif
						_build_render_data_regions( rd, dest, scrnrect, camera_link );
					}
					else
					{
						vector3d vbuf[2];
						vector3d const* poly[5];

						int i;
						for (i=port->get_num_verts(); --i>=0; )
						{
							poly[i]=&port->get_xvert_unxform(i);
						}
						for (i=port->get_num_verts(); i<5; ++i)
						{
							poly[i]=NULL;
						}

						plane nearplane(view2world.get_z_facing()*PROJ_NEAR_PLANE_D + view2world.get_position(),
                            view2world.get_z_facing());
						ClipQuadToPlane(nearplane, poly, vbuf);

						if (!poly[0])
						{
#ifdef DEBUG
							if (dump_portals_to_console)
								console_log("  entirely clipped by near plane");
#endif
							continue;
						}
						// check to see if any portal face can possibly be visible
						const matrix4x4& world2screen = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_PROJECTION];

						// could load the matrix here and do all the points without unloading
						// but portal transforms are definitely not our bottleneck
						vector3d p = xform3d_1_homog(world2screen, *(poly[0]));
#ifdef DEBUG
						if (dump_portals_to_console)
							console_log(stringx(stringx::fmt,"  clipped vert %d at (%1.3f,%1.3f)",0,p.x,p.y).c_str());
#endif

						rectf new_screen_rect = rectf(p.x,p.y*aspectcorrection,p.x,p.y*aspectcorrection);

						i=1;
						for (; i < 5 && poly[i]; ++i)
						{
							p = xform3d_1_homog(world2screen, *(poly[i]));
#ifdef DEBUG
							if (dump_portals_to_console)
								console_log(stringx(stringx::fmt,"  clipped vert %d at (%1.3f,%1.3f)",i,p.x,p.y).c_str());
#endif

							new_screen_rect.accumulate(vector2d(p.x,p.y*aspectcorrection));
						}

						new_screen_rect.intersect(scrnrect);

						if (new_screen_rect.empty())
						{
#ifdef DEBUG
							if (dump_portals_to_console)
								console_log("  screen rect is empty");
#endif
						}
						else
						{
							// recurse with NEW screen rectangle
#ifdef DEBUG
							if (dump_portals_to_console)
								console_log("  recursing into its region");
#endif
							_build_render_data_regions( rd, dest, new_screen_rect, camera_link );
						}
					}
				}
      }
#ifdef DEBUG
      else
      {
		  if (dump_portals_to_console)
			  if (!port->is_active())
				  console_log("  not active");
			  else
				  console_log("  already visited");
      }
#endif
    }

    // clear visitation flag for this region; region is now free to be processed
    // in any sibling recursion branch
    rg->unvisit();
  }
}



int g_rendered_actors_last_frame = 0;

#define _JDB_ENHANCED_LOD_METHOD 1

void world_dynamics_system::_build_render_data_ents( render_data& rd )
{
	const matrix4x4& world2view = geometry_manager::inst()->xforms[ geometry_manager::XFORM_WORLD_TO_VIEW ];
	po view2world(world2view);
	view2world.invert();

	const po& campo = app::inst()->get_game()->get_current_view_camera()->get_abs_po();   // Changed from get_current_game_cam()
	// because I'm eliminating that variable, not used in KSPS. 10/22/01 -DL
	rd.cam = campo.get_position();

	render_data::region_list::iterator ri;
	render_data::region_list::iterator ri_end = rd.regions.end();
	for (ri=rd.regions.begin(); ri!=ri_end; ++ri)
	{
		region_node* rn = (*ri).reg;

		_determine_visible_entities(rd, rn, campo);
	}

	g_rendered_actors_last_frame = 0;
}


extern profiler_timer proftimer_det_vis_ents;

void world_dynamics_system::_determine_visible_entities( render_data& rd, region_node* rn,
														po const & campo)
{
	const rational_t PROJ_NEAR_PLANE_D_2x = PROJ_NEAR_PLANE_D*2.0f;
	proftimer_det_vis_ents.start();

	const hull& fi=rn->get_data()->frustum();

#if _JDB_ENHANCED_LOD_METHOD
    //calculate an offset of number rendered characters for LOD
    vector3d camera_pos = campo.get_position();
#endif

	// Determine visibility of entities in this region.

	const region::entity_list * ralist;
	region::entity_list::const_iterator rali, rali_begin, rali_end;

	//  ralist = &rn->get_data()->get_entities();
	ralist = &rn->get_data()->get_possible_render_entities();
	rali_begin = ralist->begin();
	rali_end = ralist->end();

	ADD_PROF_COUNT(profcounter_poss_render_ents, ralist->size());

#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
	// do something here?
#else
	int cur_detail = os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL);
#endif

	for ( rali=rali_begin; rali!=rali_end; ++rali )
	{
		entity* ent = *rali;

		// if it's hidden, or doesn't have a graphical representation,
		// or already processed, ignore it
		if (ent &&
			ent->is_still_visible() &&
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
			// do something here?
#else
			cur_detail >= ent->get_min_detail() &&
#endif                                    // WTB - commented out, isn't EFLAG_GRAPHICS enough here?
			ent->is_flagged(EFLAG_GRAPHICS) /*&& ( ent->get_vrep() || ent->has_mesh() || ent->get_flavor() == ENTITY_POLYTUBE)*/ &&
			!ent->is_flagged(EFLAG_MISC_IN_VISIBLE_REGION))
		{
			bool accept = false;
			rational_t extent = 1.0f;
			bool clip_needed = true;

			// TEMPORARY: need to check actual intersection of beam/scanner with view frustum (easily done with line/triangle)
			if ( ent->get_flavor()==ENTITY_BEAM
				|| ent->get_flavor()==ENTITY_SCANNER
				|| ent->get_flavor()==ENTITY_POLYTUBE
				|| ent->is_flagged(EFLAG_GRAPHICS_NO_DISTANCE_CLIP))
			{
				// TODO: determine visibility of beam / scanner
				accept = true;
			}
			else
			{
#ifdef DEBUG
				if(g_debug_slow_ass)
				{
					if( stricmp( ent->get_id().get_val().c_str(), g_debug_entity_id_name )==0 )
					{
						debug_print( "Checking entity %s visibility", g_debug_entity_id_name );
					}
					if ( (int) ent->get_id().get_numerical_val()==g_debug_entity_id )
					{
						debug_print("Checking entity visibility");
					}
				}
#endif
				sphere bound;
				bound.set_r(ent->get_visual_radius() * ent->get_abs_po().get_scale());
				bound.set_origin(ent->get_visual_center()); // in world space

				accept = fi.includes(bound);

				if (accept)
				{
					// check if the object subtends a sufficient angle in camera space to be visible as
					// more than a pixel.
					vector3d dottmp = bound.get_origin() - campo.get_position();
					rational_t view_dist = dot(campo.get_z_facing(), dottmp);


					// I commented this out because it caused a lot of distant things in spiderman to just disappear... (JDB 11/06/00)
					//            accept = bound.get_radius() >= view_dist*PROJ_ZOOM*0.008f; // .0022 == tan(PI/(6*240))

					if (accept)
					{
						if ( ent->get_vrep() )
						{
							clip_needed = (view_dist-bound.get_r() < PROJ_NEAR_PLANE_D_2x); // overestimates a tad to be safe

							// compute reasonable value for extent
							// calc LOD detail level
							rational_t far_detail_dist = ent->get_vrep()->get_min_detail_distance();
							rational_t close_detail_dist = ent->get_vrep()->get_max_detail_distance();

							if( view_dist > far_detail_dist )
								extent = 0.0f;
							else if( view_dist < close_detail_dist || ent->is_flagged(EFLAG_MISC_NO_LOD))
								extent = 1.0f;
							else
							{

#if _JDB_ENHANCED_LOD_METHOD
								vector3d my_position = ent->get_abs_position();
								float distance_to_cam = (my_position-camera_pos).length();

								float lod_mod = 0.0f;

								lod_mod = 0.06f;


								if(lod_mod > 0.0f && g_rendered_actors_last_frame > 2)
									view_dist = pow( view_dist, (1.0f + ((rational_t)(g_rendered_actors_last_frame - 1.0f)*lod_mod) ) );
#endif


								extent = 1.0f - ( close_detail_dist - view_dist ) /
									( close_detail_dist - far_detail_dist );

								if(extent <= 0.0f)
									extent = 0.0f;
								else
								{
									if(extent > 1.0f)
										extent = 1.0f;

#if _JDB_ENHANCED_LOD_METHOD

									lod_mod = 0.7f;

									static const rational_t max_character_cam_dist = 30.0f;
									if(g_rendered_actors_last_frame > 2)
										lod_mod *= ((distance_to_cam*g_rendered_actors_last_frame) / max_character_cam_dist);
									else
										lod_mod *= ((distance_to_cam*2) / max_character_cam_dist);

									if(lod_mod > 0.0f && g_rendered_actors_last_frame > 2)
										extent = pow(extent, (( (rational_t)g_rendered_actors_last_frame - 1.0f ) * lod_mod) );

									extern game* g_game_ptr;
									rational_t framerate_compensator = 1.0f/(g_game_ptr->get_total_delta()*30.0f);
									if (framerate_compensator < 0.2f) framerate_compensator = 0.2f;
									else if (framerate_compensator > 1.0f) framerate_compensator = 1.0f;

									extent *= framerate_compensator;

#else
									extent *= extent;
#endif
								}
							}
						}
					}
				}
      }

      if ( accept )
      {
		  ent->set_flag( EFLAG_MISC_IN_VISIBLE_REGION, true );
		  ent->set_flag( EFLAG_MISC_NO_CLIP_NEEDED, !clip_needed);
		  render_data::entity_info e;
		  e.ent = ent;
		  if ( ent->get_vrep() )
		  {
			  rational_t detail = extent * (ent->get_flavor() != ENTITY_POLYTUBE ? ent->get_max_polys() : 1);
			  e.extent = detail;
		  }
		  else
			  e.extent = extent;

		  // add entity to render data
		  rd.entities.push_back( e );

		  ADD_PROF_COUNT(profcounter_rendered_ents, 1);
      }
    }
  }
  proftimer_det_vis_ents.stop();
}




// teleport hero to next malor marker (if any)
void world_dynamics_system::malor_next()
{
	// find next malor marker
	int n = entities.size();
	unsigned idx = current_malor_marker;
	while (n)
	{
		--n;
		++idx;
		if (idx == entities.size())
			idx = 0;
		if (entities[idx])
			if (stringx("MALOR")==entities[idx]->get_id().get_val().substr(0,5))
			{
				current_malor_marker = idx;
				break;
			}
	}
	// perform malor
	if (current_malor_marker != -1)
	{
		if (get_hero_ptr(g_game_ptr->get_active_player()))
		{
#if _CONSOLE_ENABLE
			console_log("Maloring to: %s", entities[current_malor_marker]->get_name().c_str());
#endif

			po the_po = get_hero_ptr(g_game_ptr->get_active_player())->get_abs_po();
			the_po.set_position(entities[current_malor_marker]->get_abs_position());
			if(get_hero_ptr(g_game_ptr->get_active_player())->has_parent())
				fast_po_mul(the_po, the_po, get_hero_ptr(g_game_ptr->get_active_player())->link_ifc()->get_parent()->get_abs_po().inverse());
			get_hero_ptr(g_game_ptr->get_active_player())->set_rel_po(the_po);

			// BIGCULL chase_cam_ptr->set_rel_position(hero_ptr->get_abs_po().fast_8byte_xform(vector3d(0,1,-2)));
			// BIGCULL chase_cam_ptr->invalidate();
		}
	}
}

// teleport hero to next malor marker (if any)
void world_dynamics_system::malor_prev()
{
	// find next malor marker
	int n = entities.size();
	int idx = current_malor_marker;
	while (n)
	{
		--n;
		--idx;
		if (idx < 0)
			idx = entities.size()-1;

		if (entities[idx])
			if (stringx("MALOR")==entities[idx]->get_id().get_val().substr(0,5))
			{
				current_malor_marker = idx;
				break;
			}
	}

	// perform malor
	if (current_malor_marker != -1)
	{
		if (get_hero_ptr(g_game_ptr->get_active_player()))
		{
#if _CONSOLE_ENABLE
			console_log("Maloring to: %s", entities[current_malor_marker]->get_name().c_str());
#endif

			po the_po = get_hero_ptr(g_game_ptr->get_active_player())->get_abs_po();
			the_po.set_position(entities[current_malor_marker]->get_abs_position());
			if(get_hero_ptr(g_game_ptr->get_active_player())->has_parent())
				fast_po_mul(the_po, the_po, get_hero_ptr(g_game_ptr->get_active_player())->link_ifc()->get_parent()->get_abs_po().inverse());
			get_hero_ptr(g_game_ptr->get_active_player())->set_rel_po(the_po);

			// BIGCULL chase_cam_ptr->set_rel_position(hero_ptr->get_abs_po().fast_8byte_xform(vector3d(0,1,-2)));
			// BIGCULL chase_cam_ptr->invalidate();
		}
	}
}

bool world_dynamics_system::malor_to(stringx point)
{
	// find next malor marker
	vector<entity*>::iterator i = entities.begin();
	vector<entity*>::iterator i_end = entities.end();

	int old = current_malor_marker;

	for( current_malor_marker = 0; i != i_end; ++i, ++current_malor_marker)
	{
		entity *ent = (*i);
		if (ent && stringx("MALOR") == ent->get_name().substr(0,5) && (point == ent->get_name() || (stringx("MALOR_")+point) == ent->get_name() || (stringx("MALOR")+point) == ent->get_name()))
		{
			if (get_hero_ptr(g_game_ptr->get_active_player()))
			{
#if _CONSOLE_ENABLE
				console_log("Maloring to: %s", ent->get_name().c_str());
#endif

				po the_po = get_hero_ptr(g_game_ptr->get_active_player())->get_abs_po();
				the_po.set_position(ent->get_abs_position());
				if(get_hero_ptr(g_game_ptr->get_active_player())->has_parent())
					fast_po_mul(the_po, the_po, get_hero_ptr(g_game_ptr->get_active_player())->link_ifc()->get_parent()->get_abs_po().inverse());
				get_hero_ptr(g_game_ptr->get_active_player())->set_rel_po(the_po);

				// BIGCULL chase_cam_ptr->set_rel_position(hero_ptr->get_abs_po().fast_8byte_xform(vector3d(0,1,-2)));
				// BIGCULL chase_cam_ptr->invalidate();

				return true;
			}
		}
	}

	current_malor_marker = old;

	return(false);
}


void world_dynamics_system::apply_radius_damage(vector3d center, rational_t radius, int bio_damage, int mechanical_damage)
{
	error ("Apply damage radius not supported in KS.");
#if 0 //BIGCULL
	if(bio_damage != 0 || mechanical_damage != 0)
	{
		// apply the damage to everyone nearby.
		vector<entity*> ents = get_entities();
		vector<entity*>::const_iterator i = ents.begin();
		vector<entity*>::const_iterator i_end = ents.end();

		for ( ; i<i_end; ++i )
		{
			entity* e = *i;

			if ( e && (e->is_hero() || (e->is_visible() && e->allow_targeting())) )
			{
				vector3d vec = center - e->get_abs_position();
				rational_t len = vec.length();

				if(len <= radius)
				{
					switch ( e->get_target_type() )
					{
					case TARGET_TYPE_BIO:
						if(bio_damage != 0)
							e->apply_damage( bio_damage, center, vec, DAMAGE_EXPLOSIVE, NULL );
						break;

					case TARGET_TYPE_MECHANICAL:
						if(mechanical_damage != 0)
							e->apply_damage( mechanical_damage, center, vec, DAMAGE_EXPLOSIVE, NULL );
						break;

					default:
						assert(0);
						break;
					}
				}
			}
		}
	}
#endif //BIGCULL
}


void world_dynamics_system::reload_particle_generators()
{
	unsigned i;
	for(i=0;i<entities.size();++i)
	{
		if( entities[i]->get_flavor() == ENTITY_PARTICLE_GENERATOR )
			(static_cast<particle_generator*>(entities[i]))->load();
	}

}

/*
vm_thread* entity::spawn_entity_script_function( const stringx& function_name ) const
{
vm_thread* nt = NULL;

  if ( function_name.length() > 0 )
  {
  stringx actual_name = function_name + "(entity)";
  actual_name.to_lower();

    script_object* so = g_world_ptr->get_current_level_global_script_object();
    script_object::instance* inst = g_world_ptr->get_current_level_global_script_object_instance();

	  if ( so!=NULL && inst!=NULL )
	  {
      int fidx = so->find_func( actual_name );
      if ( fidx >= 0 )
      {
	  nt = inst->add_thread( &so->get_func(fidx) );
	  // push entity parameter
	  const entity* e = this;
	  nt->get_data_stack().push( (char*)&e, 4 );
      }
      else
	  warning( get_name() + ": script function '" + actual_name + "' not found" );
	  }
	  }

		return nt;
		}

		  void entity::exec_preload_function(const stringx &preload_func)
		  {
		  if(preload_func.length() > 0)
		  {
		  // there might be a script function for preloading additional assets needed by item
		  stringx preload_func_name = preload_func + "()";
		  preload_func_name.to_lower();

			script_object* gso = g_world_ptr->get_current_level_global_script_object();
			assert( gso );
			int fidx = gso->find_func( preload_func_name );
			if ( fidx >= 0 )
			{
			script_object::instance* gsoi = g_world_ptr->get_current_level_global_script_object_instance();
			assert( gsoi );
			// spawn thread for function
			vm_thread* newt = gsoi->add_thread( &gso->get_func(fidx) );
			// run the NEW thread immediately
			gsoi->run_single_thread( newt, false );
			}
			}
			}
*/

void world_dynamics_system::add_entity_preload_script(entity *e, const stringx &entity_name)
{
	switch(e->get_flavor())
	{
    case ENTITY_CAMERA:
    case ENTITY_MARKER:
    case ENTITY_MIC:
    case ENTITY_LIGHT_SOURCE:
    case ENTITY_PARTICLE_GENERATOR:
    case ENTITY_LIGHT:
		//    case ENTITY_BEAM:
		//    case ENTITY_SCANNER:
    case ENTITY_BOX_TRIGGER:
		break;

    default:
		{
			stringx name = entity_name;
			name.to_lower();
			filespec spec(name);

			entity_preloads.push_back(entity_preload_pair(e, spec.name));
		}
		break;
	}
}

// this function is called immediately after a level load, to run any required
// item preload scripts
void world_dynamics_system::spawn_misc_preload_scripts()
{
	// because it is possible for an entity/item preload script to load a NEW entity/item
	// (for example, the detpack item), we must traverse the entities/items list by index
	// rather than iterator

	/*
	// there might be a script function for preloading additional assets needed by item
	stringx preload_func_name = "preload_max_inventory()";
	script_object* gso = get_current_level_global_script_object();
	assert( gso );
	int fidx = gso->find_func( preload_func_name );
	if ( fidx >= 0 )
	{
    script_object::instance* gsoi = get_current_level_global_script_object_instance();
    assert( gsoi );
    // spawn thread for function
    vm_thread* newt = gsoi->add_thread( &gso->get_func(fidx) );
    // run the NEW thread immediately
    gsoi->run_single_thread( newt, false );
	}
	*/

	script_object* gso = get_current_level_global_script_object();
	script_object::instance* gsoi = get_current_level_global_script_object_instance();
	assert( gso && gsoi );

	while(!entity_preloads.empty())
	{
		vector<entity_preload_pair>::iterator epi = entity_preloads.begin();
		stringx preload_func_name = (*epi).name + "_auto_preload(entity)";
		const entity* e = (*epi).ent;
		entity_preloads.erase(epi);

		int fidx = gso->find_func( preload_func_name );
		if ( fidx >= 0 )
		{
			vm_thread *nt = gsoi->add_thread( &gso->get_func(fidx) );
			nt->get_data_stack().push( (char*)&e, 4 );
			gsoi->run_single_thread( nt, false );
		}
	}

	entity_preloads.clear();

	/*
	unsigned i = 0;

	  for ( i=0; i<items.size(); ++i )
	  {
	  if(items[i] != NULL)
      items[i]->preload();
	  }

		for ( i=0; i<entities.size(); ++i )
		{
		if(entities[i] != NULL && !entities[i]->is_an_item())
		entities[i]->preload();
		}
	*/
}


rational_t world_dynamics_system::compute_total_energy()
{
	rational_t total_kinetic_energy = 0;

	int entities_size = entities.size();
	for (int i=0;i<entities_size;++i)
	{
		if ( entities[i] && entities[i]->has_physical_ifc() && !entities[i]->is_stationary() )
			total_kinetic_energy += entities[i]->physical_ifc()->compute_energy();
	}
	return total_kinetic_energy;
}


bool world_dynamics_system::is_entity_valid(entity *ent)
{
	vector<entity*>::iterator it;
	it = find( entities.begin(), entities.end(), ent );

	return( it != entities.end() );
}


scene_anim_handle_t world_dynamics_system::play_scene_anim( const stringx &filename, bool reverse, float start_time )
{
	scene_anim_map_t::iterator si = scene_anim_map.find( filename );

	if( si != scene_anim_map.end() )
	{
		last_snm_handle++;
		(*si).second->play( scene_anims, last_snm_handle, reverse, start_time );
	}
	else
		error( "world_dynamics_system::play_scene_anim() : Invalid filename" );

	return last_snm_handle;
}

float world_dynamics_system::get_scene_anim_time(scene_anim_handle_t handle)
{
	scene_anim_list_t::iterator i = scene_anims.begin();

	for ( ; i!=scene_anims.end(); i++ )
	{
		if ( (*i).handle == handle && (*i).ent != NULL)
		{
			entity_anim_tree *tree = (*i).ent->get_anim_tree(ANIM_SCENE);
			return tree->get_time();
		}
	}
	return -1;
}

entity_anim_tree* world_dynamics_system::get_scene_anim_tree(scene_anim_handle_t handle)
{
	scene_anim_list_t::iterator i = scene_anims.begin();

	for ( ; i!=scene_anims.end(); i++ )
	{
		if ( (*i).handle == handle && (*i).ent != NULL)
			return (*i).ent->get_anim_tree(ANIM_SCENE);
	}
	return NULL;
}

void world_dynamics_system::kill_scene_anim( scene_anim_handle_t handle )
{
	scene_anim_list_t::iterator i = scene_anims.begin();

	for ( ; i!=scene_anims.end(); ++i )
	{
		if ( (*i).handle == handle && (*i).ent != NULL)
		{
			entity_anim_tree *tree = (*i).ent->get_anim_tree(ANIM_SCENE);
			tree->set_time(tree->get_duration() - 0.10f);
			//      tree->frame_advance(0.0f);
		}
	}
}

void world_dynamics_system::add_region_ambient_sound( stringx &id, stringx &id2, rational_t volume )
{
	region_node* fr = the_terrain->find_region( id );
	if( fr )
	{
		(*fr).get_data()->set_region_ambient_sound( id2 );
		(*fr).get_data()->set_region_ambient_sound_volume( volume );
	}
}


void world_dynamics_system::add_crawl_box( int type, bool forced, const vector3d& where, const convex_box& binfo )
{
	crawl_box* crawl = NEW crawl_box( type, forced, where, binfo );

	// wds 'owns' all crawl boxes for memory tracking purposes
	crawl_boxes.push_back( crawl );

	//#pragma fixme( "crawl box has volume, so it can straddle regions. -mkv 4/6/01" )
	sector* sector = the_terrain->find_sector( where );
	region_node* rnode = sector != NULL ? sector->get_region() : NULL;
	region* reg = rnode != NULL ? rnode->get_data() : NULL;

	if(reg != NULL)
		reg->add( crawl );
	else
		warning("Crawl box placed outside world at <%.3f, %.3f, %.3f>", where.x, where.y, where.z);
}


void world_dynamics_system::recompute_all_sectors() // calls compute_sector for each entity
{
	for (entity_list::iterator i = entities.begin();
	i != entities.end();
	++i)
	{
		if (*i != NULL) // We do this since destroying an entity leaves this pointer null
		{
			(*i)->last_compute_sector_position_hash*=2.0f; // should make this a member of entity, such as invalidate_last_sector_position_hash()
			(*i)->compute_sector(*the_terrain,true);
		}
	}
}


bool world_dynamics_system::is_loading_from_scn_file() const
{
	return loading_from_scn_file;
}

