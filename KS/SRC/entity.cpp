//--------------------------------------------------
// ENTITY.CPP
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
//
// Home of entity and entity_id
//--------------------------------------------------
#include "global.h"

#include "anim_maker.h"
#include "app.h"
#include "bound.h"
#include "capsule.h"
#include "collide.h"
#include "controller.h"
#include "debug_render.h"
#include "dropshadow.h"
#include "element.h"
#include "entity.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "entityid.h"
#include "fogmgr.h"
#include "game.h"
#include "geomgr.h"
// BIGCULL #include "handheld_item.h"
#include "hwaudio.h"
#include "iri.h"
#include "item.h"
#include "lightmgr.h"
#include "mbi.h"
#include "msgboard.h"
#include "osdevopts.h"
#include "oserrmsg.h"
#include "profiler.h"
#include "renderflav.h"
// BIGCULL #include "spiderman_controller.h"
#include "terrain.h"
// BIGCULL #include "thrown_item.h"
#include "vm_thread.h"
#include "wds.h"
#include "widget_entity.h"
//#include "brain.h"
//#include "brain_turret.h"
//#include "dread_net.h"
//P #include "memorycontext.h"

#include "ai_interface.h"
#include "animation_interface.h"
// BIGCULL #include "damage_interface.h"
#include "hard_attrib_interface.h"
#include "owner_interface.h"
#include "physical_interface.h"
#include "render_interface.h"
#include "script_data_interface.h"
#include "skeleton_interface.h"
#include "slave_interface.h"
#include "soft_attrib_interface.h"
#include "sound_interface.h"
#include "time_interface.h"

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#include "ngl.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "ngl_support.h"

ENTITY_INTERFACE_CPP(entity, ai)
ENTITY_INTERFACE_CPP(entity, animation)
// BIGCULL ENTITY_INTERFACE_CPP(entity, damage)
ENTITY_INTERFACE_CPP(entity, hard_attrib)
ENTITY_INTERFACE_CPP(entity, owner)
ENTITY_INTERFACE_CPP(entity, physical)
ENTITY_INTERFACE_CPP(entity, render)
#ifdef ECULL
ENTITY_INTERFACE_CPP(entity, script_data)
#endif
ENTITY_INTERFACE_CPP(entity, skeleton)
ENTITY_INTERFACE_CPP(entity, slave)
ENTITY_INTERFACE_CPP(entity, soft_attrib)
#ifdef ECULL
ENTITY_INTERFACE_CPP(entity, sound)
#endif
ENTITY_INTERFACE_CPP(entity, time)
#ifdef ECULL
ENTITY_INTERFACE_CPP(entity, box_trigger)
#endif

DEFINE_SINGLETON(anim_id_manager)

DEFINE_SINGLETON(entity_manager)

chunk_file * entity::my_fs = NULL;

#ifdef DEBUG
int g_getpo_counter;
int g_buildpo_counter;
bool g_entity_posn_checker = false;
#endif

static rational_t MAX_VISUALLY_SORTED_RADIUS = 20;

STATICALLOCCLASSINSTANCE(entity::movement_info,16);

////////////////////////////////////////////////////////////////////////////////
// utility macro for drawing motion blur
#define RHW_XFORM( sv, mat, dv, rhw )  \
  { rational_t w = mat[0][3]*sv.x+mat[1][3]*sv.y+mat[2][3]*sv.z+mat[3][3];  \
    rhw = 1/w; \
    dv.x  = mat[0][0]*sv.x+mat[1][0]*sv.y+mat[2][0]*sv.z+mat[3][0];  \
    dv.y  = mat[0][1]*sv.x+mat[1][1]*sv.y+mat[2][1]*sv.z+mat[3][1];  \
   dv.z  = mat[0][2]*sv.x+mat[1][2]*sv.y+mat[2][2]*sv.z+mat[3][2]; }

// Hash function for recording last frame's motion to see if we need to recompute sector.
// Designed for 'a' being a vector3d.
#define POSHASH(a) ((a).x+2.7f*(a).y+5.92f*(a).z)

///////////////////////////////////////////////////////////////////////////////
//  anim_id_t
///////////////////////////////////////////////////////////////////////////////

void anim_id_manager::stl_prealloc(void)
{
  anim_id_manager::create_inst();
	static anim_id_manager *aman=anim_id_manager::inst();
	char tmp[32];

	for ( int i=0; i<1024; i++ )
	{
		sprintf(tmp,"STL_PRE_%4.4u",i);
		/*anim_id_t stanim=*/aman->anim_id(tmp);
	}
	aman->purge();

	//static anim_id_t stanim=aman->anim_id("STL_INIT");
	//static entity tentity=entity(teid,0);
	//static entity *e=&tentity;
	//static entity_manager *eman=entity_manager::inst();
	//eman->register_entity(e);
}


anim_id_manager::anim_id_manager() : NO_ID_label( "NO_ID" )
//  : labels()
{
  // pre-defined anim ids
  #define MAC(a) anim_id( #a );
  #include "anim_ids.h"
  #undef MAC
  // NEW biped anim ids
  #define MAC(a,b) anim_id( #a, b );
  #include "biped_ids.h"
  #undef MAC
}

anim_id_t anim_id_manager::anim_id( const char* _label )
{
	stringx _l=_label;
  return anim_id( _l );
}

anim_id_t anim_id_manager::anim_id( const stringx& _label )
{
  label_map_t::iterator lm = label_map.find(_label);
  if (lm==label_map.end())
  {
    labels.push_back( _label );
    label_map.insert(label_map_t::value_type(_label,labels.size() - 1));
    return labels.size() - 1;
  }
  else
    return (*lm).second;
}

anim_id_t anim_id_manager::anim_id( const char* _label, short id )
{
	stringx _l=_label;
  return anim_id( _l, id );
}

anim_id_t anim_id_manager::anim_id( const stringx& _label, short id )
{
  label_map_t::iterator lm = label_map.find(_label);
  if (lm==label_map.end())
  {
    label_map.insert( label_map_t::value_type(_label,id) );
    return (anim_id_t)id;
  }
  else
    return (*lm).second;
}

anim_id_t anim_id_manager::find_id( const stringx& _label )
{
  label_map_t::iterator lm = label_map.find(_label);
  if (lm==label_map.end())
    return NO_ID;
  else
    return (*lm).second;
}

void anim_id_manager::stl_dealloc()
{
  labels.resize(0);
  label_map.clear();
}

void anim_id_manager::purge()
{
  labels.resize(0);
  label_map.clear();
  // pre-defined anim ids
  #define MAC(a) anim_id( #a );
  #include "anim_ids.h"
  #undef MAC
  // NEW biped anim ids
  #define MAC(a,b) anim_id( #a, b );
  #include "biped_ids.h"
  #undef MAC
}


////////////////////////////////////////////////////////////////////////////////
//  Globals
////////////////////////////////////////////////////////////////////////////////

// These strings must be maintained in a 1-1 correspondence with entity_flavors.
// This is somewhat automatically enforced by the entity_manager constructor, using
// the designated stringx "null" at the end of the list.
const char* entity_flavor_names[NUM_ENTITY_FLAVORS+1] =
{
  "CAMERA",
  "ENTITY",
  "MARKER",
  "MIC",
  "LIGHT_SOURCE",
  "PARTICLE",
  "PHYSICAL",
  "ITEM",
  "LIGHT",
  "MOBILE",
  "CONGLOMERATE",
  "TURRET",
  "BEAM",
  "SCANNER",
  "MORPHABLE_ITEM",
  "SKY",
  "MANIP",
  "SWITCH",
  "BOX_TRIGGER",
  "POLYTUBE",
  "LENSFLARE",
  "null"
};

// return entity_flavor_t corresponding to given string (NUM_ENTITY_FLAVORS if not found)
entity_flavor_t to_entity_flavor_t( const stringx& s )
{
  int i;
  for ( i=0; i<NUM_ENTITY_FLAVORS; ++i )
  {
    if ( s == entity_flavor_names[i] )
      break;
  }
  return (entity_flavor_t)i;
}


////////////////////////////////////////////////////////////////////////////////
//  entity_id
////////////////////////////////////////////////////////////////////////////////
entity_id::entity_id(const char* name)
{
  set_entity_id(name);
}

void entity_id::set_entity_id(const char* name)
{
  // from now on, entity_id's must be uppercase.  tools are responsible
  // for converting.
#ifndef NDEBUG
  for(unsigned i=0;i<strlen(name);++i)
  {
    if(islower(name[i]))
    {
      stringx warnmsg = stringx(name) + " is not all uppercase.";
      warning( warnmsg );
      break;  // so we can ignore and find more
    }
  }
#endif
  name_to_number_map& name_to_number = entity_manager::inst()->name_to_number;
  name_to_number_map::iterator it =
    name_to_number.find( charstarwrap( const_cast<char*>(name) ) );
  if( it == name_to_number.end() )
  {
    char* newname = strdupcpp( name );
/*    char* newname = NEW char[strlen(name)+1];
    memcpy(newname,name,strlen(name));
    newname[strlen(name)]=0;*/
    pair<name_to_number_map::iterator,bool> result = name_to_number.insert(
      name_to_number_map::value_type( charstarwrap( newname ), entity_manager::inst()->number_server++ ) );
    // will only insert if not in there already
    val = (*result.first).second;
  }
  else
    val = (*it).second;
}


void entity_id::delete_entity_id(entity_id id)
{
  name_to_number_map::iterator found;

  name_to_number_map& name_to_number = entity_manager::inst()->name_to_number;
  for (found = name_to_number.begin(); found != name_to_number.end(); ++found)
  {
    if ((*found).second == id.val)
    {
      delete[] (*found).first.str;
      name_to_number.erase(found);
      return;
    }
  }
  error("Tried to delete unrecognized entity_id: " + itos(id.val)); // obviously it isn't in the list, so we can't get the name
  //error("Tried to delete unrecognized entity_id: " + id.get_val());
  assert(false);
}


stringx entity_id::get_val() const
{
  name_to_number_map::iterator found;

  name_to_number_map& name_to_number = entity_manager::inst()->name_to_number;
  for( found = name_to_number.begin(); found != name_to_number.end(); ++found)
  {
    if( (*found).second == val )
    {
      return stringx((*found).first.str);
    }
  }
  assert(false);
  return stringx("garbage");
}


#define UNIQUE_ENTITY_ID_BASE "_ENTID_"
static int unique_entity_id_idx=0;
#ifdef DEBUG
//int agorra;
#endif
entity_id &entity_id::make_unique_id()
{
  static entity_id ret;
  stringx name( UNIQUE_ENTITY_ID_BASE );
#ifdef DEBUG
  //if (unique_entity_id_idx==258 || unique_entity_id_idx==271)
    //agorra = 1;
#endif
  name += itos( unique_entity_id_idx++ ); //name_to_number.size() );
  ret.set_entity_id(name.c_str());
  return ret;
}


void serial_in(chunk_file& io,entity_id* eid)
{
  stringx in;
  serial_in(io,&in);
  in.to_upper();
  entity_id id_in(in.c_str());
  *eid = id_in;
}



////////////////////////////////////////////////////////////////////////////////
//  entity
////////////////////////////////////////////////////////////////////////////////
unsigned int entity::visit_key = 0;
unsigned int entity::visit_key2 = 0;

entity::entity( const entity_id& _id, unsigned int _flags )
  : bone()
{
  _construct( _id, ENTITY_ENTITY, NO_ID, _flags );
}

entity::entity( const entity_id& _id, unsigned int _flags, const po & last_po_init_val )
  : bone()
{
  _construct( _id, ENTITY_ENTITY, NO_ID, _flags );
  last_po = NEW po;
  *last_po = last_po_init_val;
}


entity::entity( const entity_id& _id,
                entity_flavor_t _flavor,
                unsigned int _flags )
  : bone()
{
  _construct( _id, _flavor, NO_ID, _flags );
}


entity::entity( const entity_id& _id,
                entity_flavor_t _flavor,
                anim_id_t _anim_id )
  : bone()
{
  _construct( _id, _flavor, _anim_id );
}


entity::entity( const entity_id& _id,
                entity_flavor_t _flavor,
                const char* _anim_id )
  : bone()
{
  if ( _anim_id )
    _construct( _id, _flavor, anim_id_manager::inst()->anim_id( _anim_id ) );
  else
    _construct( _id, _flavor, NO_ID );
}


entity::~entity()
{
  destruct();
}

void entity::destruct()
{
  delete last_po;
  last_po=NULL;
//  emitter = NULL;
  delete mbi;
  mbi = NULL;
  delete mi;
  mi = NULL;
  if ( get_anim() )
    get_anim()->detach();
  delete_visrep();
  remove_from_terrain();
  delete bbi;

#ifdef NGL
	if ( my_mesh )
	{
#if defined(TARGET_GC)
    nglReleaseMesh( my_mesh );
#else // defined(TARGET_GC)
    nglReleaseMeshFile( my_mesh->Name );
#endif // defined(TARGET_XBOX)
		my_mesh=NULL;
	}
	if ( lores_mesh )
	{
#if defined(TARGET_GC)
    nglReleaseMesh( lores_mesh );
#else // defined(TARGET_GC)
    nglReleaseMeshFile( lores_mesh->Name );
#endif // defined(TARGET_XBOX)
		lores_mesh=NULL;
	}
	if ( shadow_mesh )
	{
#if defined(TARGET_GC)
    nglReleaseMesh( shadow_mesh );
#else // defined(TARGET_XBOX)
    nglReleaseMeshFile( shadow_mesh->Name );
#endif // defined(TARGET_XBOX)
		shadow_mesh=NULL;
	}

#endif

  if ( coninfo )
  {
    item_list_t::const_iterator i = coninfo->items.begin();
    item_list_t::const_iterator i_end = coninfo->items.end();
    for ( ; i!=i_end; ++i )
    {
      if ( g_world_ptr && g_world_ptr->remove_entity( *i ) )
        delete *i;
    }
    delete coninfo;
  }

  if ( colgeom )
  {
    if ( is_flagged(EFLAG_COLGEOM_INSTANCED) )
      cg_mesh_bank.delete_instance( (cg_mesh *)colgeom );
    else
      delete colgeom;

    colgeom=NULL;
  }

//  if ( anim_trees != NULL )
  {
    int i;
    for ( i=0; i<MAX_ANIM_SLOTS; ++i )
    {
      if ( anim_trees[i] )
        delete anim_trees[i];
      anim_trees[i]=NULL;
    }
//    delete[] anim_trees;
  }

  if(my_light_mgr)
  {
    delete my_light_mgr;
    my_light_mgr = NULL;
  }


#if 0 // BIGCULL
  if(has_damage_ifc())
    destroy_damage_ifc();
#endif // BIGCULL

  if(has_ai_ifc())
    destroy_ai_ifc();

  if(has_animation_ifc())
    destroy_animation_ifc();

  if(has_hard_attrib_ifc())
    destroy_hard_attrib_ifc();

  if(has_owner_ifc())
    destroy_owner_ifc();

  if(has_physical_ifc())
    destroy_physical_ifc();

  if(has_render_ifc())
    destroy_render_ifc();

#ifdef ECULL
  if(has_script_data_ifc())
    destroy_script_data_ifc();
#endif

  if(has_skeleton_ifc())
    destroy_skeleton_ifc();

  if(has_slave_ifc())
    destroy_slave_ifc();

  if(has_soft_attrib_ifc())
    destroy_soft_attrib_ifc();

#ifdef ECULL
  if(has_sound_ifc())
    destroy_sound_ifc();
#endif

  if(has_time_ifc())
    destroy_time_ifc();

#ifdef ECULL
  if(has_box_trigger_ifc())
    destroy_box_trigger_ifc();
#endif

  entity_manager::inst()->deregister_entity( this );
}


void entity::_construct( const entity_id& _id,
                         entity_flavor_t _flavor,
                         anim_id_t _anim_id,
                         unsigned int _flags,
                         const po& _my_po )
{
	which_hero=-1;
  my_ai_interface = NULL;
  my_animation_interface = NULL;
// BIGCULL   my_damage_interface = NULL;
  my_hard_attrib_interface = NULL;
  my_owner_interface = NULL;
  my_physical_interface = NULL;
  my_render_interface = NULL;
#ifdef ECULL
  my_script_data_interface = NULL;
#endif
  my_skeleton_interface = NULL;
  my_slave_interface = NULL;
  my_soft_attrib_interface = NULL;
#ifdef ECULL
	my_sound_interface = NULL;
#endif
	my_time_interface = NULL;
#ifdef ECULL
  my_box_trigger_interface = NULL;
#endif

  min_detail = 0;
  id = _id;
  flavor = _flavor;
  anim_id = _anim_id;
  flags = _flags;
  bone_idx = (unsigned short)-1;
  mi = NULL;
  programmed_cell_death = LARGE_TIME;
  set_rel_po( _my_po );
  mbi = NULL;
  mi = NULL;
//  emitter = NULL;
  radius = 0;
  my_sector = NULL;
  center_region = NULL;
  my_visrep = NULL;
  vis_xz_rad_rel_center = 0;
  colgeom = NULL;

  MaterialMask = 0;
  TextureFrame = -1;
  use_uv_scrolling = false;
  cull_entity = true;

#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
  my_mesh = NULL;
  lores_mesh = NULL;
  shadow_mesh = NULL;
  force_hi_res = false;
	usezbias=false;
	zbias=0.0f;
#endif
//  colgeom_instanced = false;
//  age = 0;


  frame_time_info.set_ifl_frame_boost( get_random_ifl_frame_boost() );

  current_anim = NULL;
  coninfo = NULL;
  bbi = NULL;
  // register
  entity_manager::inst()->register_entity( this );
  set_in_use(true);
  last_compute_sector_position_hash = FLT_MAX;
  last_po = NULL;
	door_portal = NULL;

  ext_flags = 0;

  destroy_info = NULL;
  //set_gravity(is_physical());

  character_action_anim = stringx();
  action_character = NULL;

  alternative_materials = NULL;
  render_color = color32_white;
  render_scale = vector3d( 1, 1, 1 );

  target_timer = 0.0f;
  damage_resist_modifier = 1.0f;

  // list of hierarchical animations attached to this entity;
  // allocated by entity::play_anim() (see entity2.cpp)
//  anim_trees = NULL;
  for ( int i=0; i<MAX_ANIM_SLOTS; ++i )	// was being used uninitialized (dc 01/16/02)
  {
    anim_trees[i]=NULL;
  }

  owning_widget = NULL;
  my_entity_pool = NULL;

  current_target = NULL;
  current_target_pos = ZEROVEC;
  current_target_norm = YVEC;

  last_item_used = NULL;

  suspended = false;
  suspended_active_status=0;
#if _ENABLE_WORLD_EDITOR
  scene_flags = 0;
#endif

  my_controller = NULL;
  max_lights = 1;

  my_light_mgr = NULL;

  for(int i = 0; i <  MAX_ANIM_SLOTS; i++)
     anim_trees[i] = NULL;

	in_underwater_scene=FALSE;

}

int entity::get_hero_id( void )
{
	if ( which_hero!=-1 )
	{
		return which_hero;
	}
  return g_game_ptr->get_active_player();
}


void FixupEntityMesh( nglMesh *Mesh, int lit=0 )
{
	if ( Mesh )
	{
		if ( lit==-1 )
		{
			Mesh->Flags &=~ NGL_LIGHTCAT_MASK;
		}
		else if ( lit==1 )
		{
			if ( 0 == (Mesh->Flags & NGL_LIGHTCAT_MASK) )
				Mesh->Flags |= NGLMESH_LIGHTCAT_1;
		}
		for ( u_int i = 0; i < Mesh->NSections; i++ )
		{
			nglMeshSection* Section = &Mesh->Sections[i];
			nglMaterial* Material = Section->Material;

			if (Material->Flags & NGLMAT_BACKFACE_CULL)
			{
#ifndef USER_MKV
				nglPrintf(PRINT_RED "Mesh %s has backface culling set.  Changing to backface default.\n" PRINT_BLACK, Mesh->Name.c_str());
#endif
				Material->Flags &= ~NGLMAT_BACKFACE_CULL;
			}
			else if (!(Material->Flags & NGLMAT_BACKFACE_DEFAULT))
			{
				nglPrintf(PRINT_RED "Mesh %s has culling set to none.  Changing to backface default.\n" PRINT_BLACK, Mesh->Name.c_str());
			}
#ifdef DEBUG
			if (os_developer_options::inst()->get_string (os_developer_options::STRING_BACKFACE_CULL) == "ALL")
			{
				Material->Flags &= ~NGLMAT_BACKFACE_DEFAULT;
				Material->Flags |= NGLMAT_BACKFACE_CULL;
			}
			else if (os_developer_options::inst()->get_string (os_developer_options::STRING_BACKFACE_CULL) == "NONE")
			{
				Material->Flags &= ~NGLMAT_BACKFACE_DEFAULT;
				Material->Flags &= ~NGLMAT_BACKFACE_CULL;
			}
			else 
#endif
			{
				Material->Flags |= NGLMAT_BACKFACE_DEFAULT;
				Material->Flags &= ~NGLMAT_BACKFACE_CULL;
			}
			if (Material->Flags & NGLMAT_FOG)
			{
#ifndef USER_MKV
				nglPrintf(PRINT_RED "Mesh %s has fog set.  Turning it off.\n" PRINT_BLACK, Mesh->Name.c_str());
#endif
			}
			Material->Flags &= ~NGLMAT_FOG;
			if ( lit==-1 )
			{
				Material->Flags &= ~NGLMAT_LIGHT;
#ifdef TARGET_PS2
				Material->Flags |= NGLMAT_FORCE_Z_WRITE ;
#endif
			}
			else if ( lit==1 )
			{
				Material->Flags |= NGLMAT_LIGHT;
			}
			else if ( Mesh->Flags & NGL_LIGHTCAT_MASK )
			{
				if (! ( Material->Flags & NGLMAT_LIGHT ))
				{
					Material->Flags |= NGLMAT_LIGHT;
					nglPrintf(PRINT_RED "Mesh %s has light cat, but material had lighting turned off.  Turning it on.\n" PRINT_BLACK, Mesh->Name.c_str());
				}
			}
			else
			{
				Material->Flags &= ~NGLMAT_LIGHT;
			}
		}
	}
}



void entity::load_lores_mesh( const char *name )
{
/*	#ifdef TARGET_GC
  lores_mesh=nglLoadMesh( name );
	FixupEntityMesh( lores_mesh );
	#else*/
  if (nglLoadMeshFile( name ))
  {
    lores_mesh=nglGetFirstMeshInFile( name );
		FixupEntityMesh( lores_mesh );
  }
//	#endif
}

void entity::load_shadow_mesh( const char *name )
{
/*	#ifdef TARGET_GC
  shadow_mesh=nglLoadMesh( name );
	FixupEntityMesh( lores_mesh );
	#else*/
  if (nglLoadMeshFile( name ))
  {
    shadow_mesh=nglGetFirstMeshInFile(name);
		FixupEntityMesh( shadow_mesh );
  }
//	#endif
}

void entity::initialize()
{
  if (is_door()) // should put this stuff in DOOR, not here!
  {
    // find the closest portal and link to it
    portal *closest = NULL;
    rational_t closest_dist = 1e28f;
    terrain& wt=g_world_ptr->get_the_terrain();
    for (portal_list::iterator pi = wt.get_portals().begin(); pi != wt.get_portals().end(); ++pi)
    {
      portal *p = *pi;
      vector3d diff = p->get_effective_center() - get_visual_center();
      rational_t radius = p->get_effective_radius()*1.25f;
      rational_t dist = diff.length2();
      if (dist < radius*radius)
      {
        dist = __fabs(dot(p->get_cylinder_normal(),diff));
        if (dist < p->get_cylinder_depth() + 0.2f*radius)
        {
          if (dist < closest_dist)
          {
            closest = p;
            closest_dist = dist;
          }
        }
      }
    }

    if (!closest)
      warning("'is_door' flag specified for entity %s but couldn't find a suitable portal for it",entity::get_name().c_str());
    else
    {
      door_portal = closest;
      door_portal->set_active(!is_door_closed());
    }
  }
}


// this function will be called when an entity is acquired from the entity_maker cache
void entity::acquire( unsigned int _flags )
{
  unsigned int old_flags = flags;
  flags = _flags;

  programmed_cell_death = LARGE_TIME;
  frame_time_info = frame_info();
  frame_time_info.set_ifl_frame_boost( get_random_ifl_frame_boost() );
  set_in_use( true );
  last_compute_sector_position_hash = FLT_MAX;
//  set_gravity( is_physical() );
  character_action_anim = stringx();
  action_character = NULL;
  render_color = color32_white;
  render_scale = vector3d( 1, 1, 1 );
  target_timer = 0.0f;
  damage_resist_modifier = 1.0f;
  current_target = NULL;
  current_target_pos = ZEROVEC;
  current_target_norm = YVEC;
  last_item_used = NULL;
  suspended = false;
  suspended_active_status = 0;
#if _ENABLE_WORLD_EDITOR
  scene_flags = 0;
#endif
  max_lights = 1;

  flags |= (old_flags & EFLAG_COPY_MASK);
  flags &= (old_flags | ~EFLAG_COPY_MASK);

  TextureFrame = -1;
  MaterialMask = 0;
  cull_entity = true;

  // visual
  // this isn't necessarily valid.  Just because I happen to have a
  // visrep doesn't mean I necessarily want to be visible
  if ( my_visrep || has_mesh() )
  {
    set_flag( EFLAG_GRAPHICS, true );
//#pragma fixme( "jamie sez 'fix this when uv animation works on the ps2'. -mkv 4/6/01" )
    if ( my_visrep && ( my_visrep->get_type() == VISREP_PMESH ) && ((vr_pmesh*)my_visrep)->is_uv_animated() )
    {
      // make active if uv animated
      set_flag( EFLAG_ACTIVE, true );
    }
    set_visible( true );
  }

  // collision
  // this isn't necessarily valid.  Just because I happen to have a
  // colgeom doesn't mean I necessarily want to be collidable
  if ( colgeom )
  {
    set_flag( EFLAG_PHYSICS, true );
    if (min_detail <= os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL))
      set_collisions_active( true );
  }

  if ( mi )
    mi->frame_delta_valid = mi->last_frame_delta_valid = false;

  set_clone( true );
}

// this function will be called when the entity is released to the entity_maker cache
void entity::release()
{
  if ( get_anim() )
    get_anim()->detach();
  remove_from_terrain();
/*!  clear_parent();
  if ( ci )
  {
    child_list::const_iterator i = ci->children.begin();
    child_list::const_iterator i_end = ci->children.end();
    for ( ; i!=i_end; ++i )
    {
      entity* e = *i;
      assert( e->pi );
      // clear parent without affecting position in world
      e->pi->parent = NULL;
      e->my_po = e->pi->my_abs_po;
      e->update_abs_po();
    }
    ci->children.resize(0);
  }
!*/
//  if ( anim_trees != NULL )
  {
    int i;
    for ( i=0; i<MAX_ANIM_SLOTS; ++i )
      kill_anim( i );
  }
}


///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

entity::entity( chunk_file& fs,
                const entity_id& _id,
                entity_flavor_t _flavor,
                unsigned int _flags )
  :   bone()
{
  _construct( _id, _flavor, NO_ID, _flags );

  chunk_flavor cf;
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( cf == chunk_flavor("meshfile") )
    {
      stringx meshname;
      serial_in( fs, &meshname );
      assert( meshname.length() );
#ifdef NGL
	#if 0
    //      serial_in( fs, &cf );
      filespec spec(fs.get_name());
      stringx mesh_path = spec.path;
      nglSetMeshPath( (char *)mesh_path.c_str() );
      stringx texture_path = spec.path;
      // trim "entities" off of path
      texture_path.to_lower();
      if( texture_path.find( "entities" ) != stringx::npos )
      {
        stringx::size_type last_slash        = texture_path.rfind( '\\' );
        stringx::size_type penultimate_slash = texture_path.rfind( '\\', last_slash-1 );
        texture_path = texture_path.substr( 0, penultimate_slash+1 );
      }
      texture_path += os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR);
      texture_path += "\\";
      nglSetTexturePath( (char *)texture_path.c_str() );
	#endif
      if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
        nglPrintf("Loading %s" PLATFORM_MESH_EXT ".\n", meshname.c_str());
      my_mesh = nglGetMesh( (char *)meshname.c_str() );

//		  nglMesh *Mesh = my_mesh;
			FixupEntityMesh( my_mesh );

			lores_mesh=NULL;
			shadow_mesh=NULL;

			char entityname[64];
			strcpy(entityname,meshname.c_str());
			if ( strlen(entityname)>3 )
			{
				entityname[strlen(entityname)-3]=0;
			}
			strcat(entityname,"_lo");
      stringx mesh_path = nglGetMeshPath();
			stringx loresname=entityname; //meshname + "_lo";
			if (
				//os_file::file_exists( local ) )
				world_dynamics_system::wds_exists(mesh_path+loresname, PLATFORM_MESH_EXT ) )
			{
				load_lores_mesh( loresname.c_str() );
			}
			else
			{
				//nglPrintf("No lo res mesh found: %s\n",loresname.c_str());
			}



#else
      // read visual mesh (instanced by physent ID)
      filespec spec( fs.get_name( ) );
#pragma fixme( "we should have a function that returns hokey in-source constants like this. -mkv 4/6/01" )
      stringx meshfilename = spec.path + meshname + ".txtmesh";
      chunk_file txtmeshfile;
      txtmeshfile.open( meshfilename );
      my_visrep = load_new_visual_rep( txtmeshfile, id.get_val(), VISREP_PMESH, vr_pmesh::ALLOW_SELF_LIT_MATERIALS  );
#pragma todo("is_uv_animated will go away once we have texture coordinate transforms:  (JDF 2-23-01)")
      // make active if uv animated
      if( my_visrep->get_type()==VISREP_PMESH && ((vr_pmesh*)my_visrep)->is_uv_animated() )
        set_flag( EFLAG_ACTIVE, true );
#endif
      set_flag( EFLAG_GRAPHICS, true );
    }
    else if ( cf == chunk_flavor("vrextern") )
    {
      // read any other variety of visrep from named external file (extension expected)
      stringx vrname;
      serial_in( fs, &vrname );
      my_visrep = load_new_visual_rep( fs.get_dir() + vrname, vr_pmesh::ALLOW_SELF_LIT_MATERIALS );
      set_flag( EFLAG_GRAPHICS, true );
      serial_in( fs, &cf );
      assert( cf == CHUNK_END );
    }
    else if ( cf == chunk_flavor("bone") )
    {
      // bone index (used for rendering skin)
      serial_in( fs, &bone_idx );
      serial_in( fs, &cf );
      assert( cf == CHUNK_END );
    }
    else if ( cf == chunk_flavor("collide") )
    {
      // read collision geometry
      serial_in( fs, &cf );
      if ( cf == chunk_flavor("meshfile") )
      {
        stringx meshname;
        serial_in( fs, &meshname );
        filespec spec(fs.get_name());
        stringx meshfilename = spec.path + meshname + stringx(".txtmesh");
        // read visual mesh (instanced by physent ID)
        chunk_file txtmeshfile;
        txtmeshfile.open( meshfilename );

        if ( is_stationary() )
        {
          // stationary colgeoms are instanced by physent ID
          colgeom = cg_mesh_bank.new_instance( txtmeshfile, id.get_val(), 0 );
          set_flag(EFLAG_COLGEOM_INSTANCED,true);
        }
        else
          colgeom = NEW cg_mesh( txtmeshfile, cg_mesh::ALLOW_WARNINGS );
        colgeom->set_owner( this );
      }
      else if ( cf == chunk_flavor("visrep") )
      {
        error( fs.get_name() + ": entities can no longer use visreps for collision.");
/*
        // use the entity's own visrep to build a collision mesh
        assert( my_visrep && my_visrep->get_type()==VISREP_PMESH );
        colgeom = NEW cg_mesh( static_cast<vr_pmesh*>(my_visrep), id.get_val(), cg_mesh::DISALLOW_WARNINGS );
        if ( is_stationary() )
        {
          // stationary colgeoms are instanced by physent ID
          cg_mesh_bank.insert_new_object( static_cast<cg_mesh*>(colgeom), id.get_val() );
          set_flag(EFLAG_COLGEOM_INSTANCED,true);
        }
        colgeom->set_owner(this);
        */
      }
      else if ( cf == chunk_flavor("capsule") )
      {
        // no data required for now
				if ( colgeom==NULL )
        	colgeom = NEW collision_capsule( this );
				if ( last_po==NULL )
        	last_po = NEW po;
      }
      else
        error( fs.get_name() + ": unknown chunk found in collide data" );
      serial_in( fs, &cf );
      if( cf==chunk_flavor("ps2mesh") )
      {
				// platform specific stuff
				assert(0);
        stringx ps2meshname;
        serial_in( fs, &ps2meshname );
        serial_in( fs, &cf );
      }
      assert( cf == CHUNK_END );
      if (min_detail <= os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL))
        set_collisions_active( true );
    }
    else
      error( fs.get_name() + ": unknown chunk found in entity" );
  }

  if (my_visrep)
  {
    set_radius(my_visrep->get_radius(0));
  }

  if (colgeom)
    set_flag(EFLAG_PHYSICS, true);
}


void entity::optimize()
{
  if (my_visrep)
  {
    if (my_visrep->get_type()==VISREP_PMESH)
    {
      vr_pmesh* mesh = static_cast<vr_pmesh*>(my_visrep);
      mesh->shrink_memory_footprint();
    #ifdef TARGET_PC
      if (//is_flagged(EFLAG_GRAPHICS) &&
          !is_ext_flagged(EFLAG_EXT_DONT_OPTIMIZE) &&
          !is_flagged(EFLAG_MISC_MEMBER_OF_VARIANT) &&
          !get_light_set() &&
//!          !is_a_character() &&
          !is_a_conglomerate() &&
          !is_a_sky() &&
          !is_a_beam() &&
          !is_a_particle_generator() &&
          !is_a_scanner() &&
          !is_a_visual_item() &&
          /*
          !is_a_handheld_item() &&
          !is_a_gun() &&
          !is_a_thrown_item() &&
          !is_a_morphable_item() &&
          !is_a_projectile() &&
          !is_a_grenade() &&
          !is_a_rocket() &&
          */
          !is_an_item())
        mesh->optimize_static_mesh_for_d3d();
    #endif
    }
  }
}

void entity::read_enx(chunk_file& fs)
{
  stringx label;
  serial_in(fs, &label);
//  read_enx(fs, label);

  // read all the labels...
  while (label.length() > 0)
  {
    if (!handle_enx_chunk(fs, label))
		{
#ifdef WEENIEASSERT  // this triggers every time KSPS starts so it's gon
      error( fs.get_filename() + ": unknown chunk '" + label +"' for entity " + get_name() + "(" + entity_flavor_names[get_flavor()] + ")");
#endif
		}

    serial_in(fs, &label);
  }
}


/*
void entity::read_enx( chunk_file& fs, stringx& lstr )
{
  // read all the labels...
  while( lstr.length() > 0 )
  {
    if(!handle_enx_chunk( fs, lstr ))
      error( fs.get_filename() + ": unknown chunk '" + lstr +"' for entity " + get_name() + "(" + entity_flavor_names[get_flavor()] + ")");

    serial_in( fs, &lstr );
  }
}
*/

bool entity::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  // optional collision flags
  if ( label == "collision_flags:" )
  {
    // at the moment, these flags only apply to cg_mesh
    if ( !get_colgeom() )
    {
      if(is_a_conglomerate())
        error( get_id().get_val() + ": error reading " + fs.get_name() + ": cannot apply collision flags to an entity without a collision mesh\n\nThis entity is a conglomerate. It needs a collision root node." );
      else
        error( get_id().get_val() + ": error reading " + fs.get_name() + ": cannot apply collision flags to an entity without a collision mesh" );
    }

    if(get_colgeom()->get_type()!=collision_geometry::MESH)
    {
      error( get_id().get_val() + ": error reading " + fs.get_name() + ": cannot apply collision flags to an entity whose collision is not a mesh" );
    }

    cg_mesh* m = static_cast<cg_mesh*>( get_colgeom() );
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="camera_collision" || label=="CAMERA_COLLISION" )
        m->set_flag( cg_mesh::FLAG_CAMERA_COLLISION, true );
      else if ( label=="no_entity_collision" || label=="NO_ENTITY_COLLISION" )
        m->set_flag( cg_mesh::FLAG_ENTITY_COLLISION, false );
      else
        error("Bad keyword %s in collision_flags section", label.c_str());
    }

    return true;
  }
  else if ( label == "targeting_flags:" )
  {
    error(fs.get_filename() + ": targeting_flags no longer supported. Move data into destroy_info.");

    return true;
  }
#if 0 // BIGCULL
  else if ( label == "destroy_info:" )
  {
    error(fs.get_filename() + ": 'destroy_info:' No longer supported. Move data into damage_interface.");
/*
    if(!destroy_info)
      destroy_info = NEW destroyable_info(this);

    destroy_info->read_enx_data(fs, label);
*/

    return true;
  }
#endif // BIGCULL
  else if ( label == "action_flags:" )
  {
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="actionable" || label=="ACTIONABLE" )
      {
        set_actionable( true );
      }
      else if ( label=="character_anim" || label=="CHARACTER_ANIM" )
      {
        serial_in( fs, &character_action_anim);
        load_anim( character_action_anim );
      }
      else if ( label=="use_facing" || label=="USE_FACING" )
      {
        set_action_uses_facing( true );
      }
      else
        error("Bad keyword %s in action_flags section", label.c_str());
    }

    return true;
  }
  else if ( label == "entity_flags:" )
  {
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      label.to_lower();

      if ( label=="walkable" )
      {
        set_walkable( true );
        set_ext_flag(EFLAG_EXT_ENX_WALKABLE, true);
      }
      else if ( label=="noncrawl" )
      {
        set_crawlable( false );
      }
      else if ( label=="smallcrawl" )
      {
        set_smallcrawl( true );
      }
      else if ( label=="mustcrawl" )
      {
        set_mustcrawl( true );
      }
      else if ( label=="cover" )
      {
        set_ai_cover( true );
      }
      else if ( label=="ai_see_thru" || label=="non_block_los")
      {
        set_ai_los_block( false );
      }
      else if ( label=="dont_optimize" )
      {
        set_ext_flag_recursive(EFLAG_EXT_DONT_OPTIMIZE, true);
//        bool flagged = is_ext_flagged(EFLAG_EXT_DONT_OPTIMIZE);
      }
      else
        error("Bad keyword %s in entity_flags section", label.c_str());
    }

    return true;
  }
  else if ( label == "scan_flags:" )
  {
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="scanable" || label=="SCANABLE" )
      {
        if ( has_mesh() )
          set_scannable( true );
        else
        {
          if(is_a_conglomerate())
            warning( fs.get_name() + ": " + get_id().get_val() + ": entity must have mesh visrep to be set as SCANABLE\n\nThis entity is a conglomerate. It needs a visrep for the root node." );
          else
            warning( fs.get_name() + ": " + get_id().get_val() + ": entity must have mesh visrep to be set as SCANABLE" );
        }
      }
      else if ( label=="beamable" || label=="BEAMABLE" )
      {
        set_beamable( true );
      }
      else
        error("Bad keyword %s in scan_flags section", label.c_str());
    }

    return true;
  }
  else if ( label == "door_flags:" )
  {
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="is_door" || label=="IS_DOOR" )
        set_door( true );
      else if ( label=="open" || label=="OPEN" )
        set_door_closed( false );
      else
        error("Bad keyword %s in door_flags section", label.c_str());
    }

    return true;
  }
  else if ( label == "min_detail:" )
  {
    serial_in( fs, &min_detail );
    if (min_detail<0 || min_detail>2)
      error("Bad min_detail value");

    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      error("Bad keyword %s in min_detail section", label.c_str());
    }

    return true;
  }
  else if ( label == "script_objects:" )
  {
    error( fs.get_filename() + ": script object attach is no longer supported" );

    return true;
  }
/*
  else if ( label == "turret_ai:" )
  {
    // only valid for non-characters that do not have brains yet
    assert(my_controller == NULL);

    turret_brain *brn = NEW turret_brain(this);
    brn->read_data(fs);

    g_world_ptr->add_controller(brn);

    set_controller(brn);

    return true;
  }
*/
  else if ( label == "ai:" )
  {
    if(!has_ai_ifc())
    {
      create_ai_ifc();

      ai_ifc()->read_data(fs);
      ai_ifc()->set_active(true);
    }

    return true;
  }
  else if ( label == "animation:" )
  {
    if(!has_animation_ifc())
      create_animation_ifc();

    stringx label;
    for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
    {
      if(label == "ani_file")
      {
        serial_in(fs,&label);
        animation_ifc()->read_anim_info_file(label);
      }
    }

    return true;
  }
  else if ( label == "lighting:" )
  {
    //assert(get_flavor() == ENTITY_CHARACTER || get_flavor() == ENTITY_CONGLOMERATE);

    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="max_lights" )
      {
        serial_in(fs,&max_lights);
        if (get_light_set())
          get_light_set()->max_lights = max_lights;
      }
      else
        error("Bad keyword %s in lighting section", label.c_str());
    }
    return true;
  }
  else if(label == "hard_attrib:")
  {
    if(!has_hard_attrib_ifc())
      create_hard_attrib_ifc();

    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      #define PROCESS_NUMBERS_ONLY
      #define MAC(itype, itype_name, code_name, str_name, default_val) if(label == ##str_name##) { itype val; serial_in(fs, &val); hard_attrib_ifc()->set_##code_name##(val); continue; }
      #include "entity_hard_attribs.h"
      #undef MAC
      #undef PROCESS_NUMBERS_ONLY

      warning("bad hard attrib found: '%s' in %s", label.c_str(), fs.get_filename().c_str());
    }

    return(true);
  }
#if 0 // BIGCULL
  else if(label == "damage_interface:")
  {
    if(!has_damage_ifc())
      create_damage_ifc();

    damage_ifc()->read_enx_data(fs, label);
    return(true);
  }
#endif // BIGCULL
  else if(label == "sound_interface:")
  {
    return(true); // Remove old sound stuff
#ifdef ECULL
    if(!has_sound_ifc())
      create_sound_ifc();

    sound_ifc()->read_enx_data(fs, label);
#endif
    return(true);
  }
  else if ( label == "items:" )  // contained items
  {
    int i = 0;
    for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label),++i )
    {
      stringx idstr = "INV_" + get_name() + itos(i);
      entity_id id( idstr.c_str() );
      entity* e = g_entity_maker->create_entity_or_subclass( label, id, po_identity_matrix, "items\\", INACTIVE );
      if ( !e->is_an_item() )
        error( fs.get_name() + ": items: '" + label + "' is not an item" );
      entity::add_item( (item*)e );
    }
    return true;
  }
/*
  else if ( label == "weapons:" )  // brain weapons
  {
    for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
    {
      entity* itm;
//      warning(label);
      stringx entity_name = get_fname_wo_ext( label );
      entity_name.to_upper();
      stringx entity_dir = get_dir( label );
      itm = g_entity_maker->create_entity_or_subclass( entity_name,
                                                    entity_id::make_unique_id(),
                                                    po_identity_matrix,
                                                    entity_dir,
                                                    ACTIVE_FLAG | NONSTATIC_FLAG );

      if ( itm->get_flavor() != ENTITY_ITEM )
      {
        error( "Entity weapon: entity " + entity_name + " is not an item" );
      }
      else
      {
        if ( !itm->is_a_handheld_item() )
        {
          error( "Entity weapon: entity " + entity_name + " is not an handheld item" );
        }
        else
        {
          itm->set_created_entity_default_active_status();
          ((handheld_item*)itm)->set_handheld_flag( _HANDHELD_BRAIN_WEAPON, true );
          ((handheld_item*)itm)->set_handheld_flag( _HANDHELD_COMMON_BRAIN_WEAPON, true );
          bool added;

          {
            added = add_item( (item*)itm );
            // this entity needs to frame_advance() even when ineligible by the usual rules;
            // normally, this is handled by item::give_to_character(), but as you can see that
            // cannot be called in this case
            if(itm->is_a_gun())
              g_world_ptr->guarantee_active( itm );
          }

          if ( !added )
            warning( fs.get_name() + ": brain weapon '" + entity_name + "' added more than once to entity '" + get_name() + "'" );
          else
          {
            ((handheld_item*)itm)->draw();
            ((handheld_item*)itm)->hide();
          }
        }
      }
    }

    return true;
  }
*/
  return false;
}

/*
void entity::read_enx( chunk_file& fs, stringx& lstr )
{
  stringx label = lstr;

  // optional collision flags
  if ( label == "collision_flags:" )
  {
    // at the moment, these flags only apply to cg_mesh
    if ( !get_colgeom() || get_colgeom()->get_type()!=collision_geometry::MESH )
      error( get_id().get_val() + ": error reading " + fs.get_name() + ": cannot apply collision flags to an entity without a collision mesh" );
    cg_mesh* m = static_cast<cg_mesh*>( get_colgeom() );
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="camera_collision" || label=="CAMERA_COLLISION" )
        m->set_flag( cg_mesh::FLAG_CAMERA_COLLISION, true );
      else if ( label=="no_entity_collision" || label=="NO_ENTITY_COLLISION" )
        m->set_flag( cg_mesh::FLAG_ENTITY_COLLISION, false );
    }
    serial_in( fs, &label );
  }

  // optional collision flags
  if ( label == "targeting_flags:" )
  {
    error(fs.get_filename() + ": targeting_flags no longer supported. Move data into destroy_info.");
  }

  if ( label == "destroy_info:" )
  {
    if(!destroy_info)
      destroy_info = NEW destroyable_info(this);

    destroy_info->read_enx(fs, label);
    serial_in( fs, &label );
  }

  if ( label == "action_flags:" )
  {
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="actionable" || label=="ACTIONABLE" )
      {
//        bool actionable;
//        serial_in( fs, &actionable );
        set_actionable( true );
      }
      else if ( label=="character_anim" || label=="CHARACTER_ANIM" )
      {
        serial_in( fs, &character_action_anim);
        load_anim( character_action_anim );
      }
      else if ( label=="use_facing" || label=="USE_FACING" )
      {
//        bool use_facing;
//        serial_in( fs, &use_facing );
        set_action_uses_facing( true );
      }
    }
    serial_in( fs, &label );
  }

  if ( label == "scan_flags:" )
  {
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="scanable" || label=="SCANABLE" )
      {
//        bool scanable;
//        serial_in( fs, &scanable );
        set_scanable( true );
      }
      else if ( label=="beamable" || label=="BEAMABLE" )
      {
//        bool beamable;
//        serial_in( fs, &beamable );
        set_beamable( true );
      }
    }
    serial_in( fs, &label );
  }

  if ( label == "door_flags:" )
  {
    serial_in( fs, &label );
    for ( ; label!=chunkend_label; serial_in(fs,&label) )
    {
      if ( label=="is_door" || label=="IS_DOOR" )
        set_door( true );
      else if ( label=="open" || label=="OPEN" )
        set_door_closed( false );
    }
    serial_in( fs, &label );
  }

  if ( label == "script_objects:" )
    error( fs.get_filename() + ": script object attach is no longer supported" );
}
*/

///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

entity::entity( const stringx& entity_fname,
                const entity_id& _id,
                entity_flavor_t _flavor,
                bool delete_stream )
  : bone()
{
  _construct( _id, _flavor, NO_ID );

  // scan for and load the visrep
  stringx vmesh_fname, dummy;
  // Though new'd here, this will be deleted by the externally called constructor.
  assert (my_fs == NULL);
  my_fs = NEW chunk_file();
  stringx fname_str(entity_fname+".ent");
  my_fs->open(fname_str);

  // Strip Entity Flavor
  serial_in(*my_fs, &dummy);

  // Read Visual Mesh
  serial_in(*my_fs, &dummy);
  serial_in(*my_fs, &vmesh_fname);

  assert(dummy=="mesh:");
  my_visrep = load_new_visual_rep( my_fs->get_dir()+vmesh_fname, vr_pmesh::ALLOW_SELF_LIT_MATERIALS );
  if( my_visrep->get_type()==VISREP_PMESH )
    static_cast<vr_pmesh*>(my_visrep)->shrink_memory_footprint();
  set_flag(EFLAG_GRAPHICS,true);

  stringx label;
  serial_in(*my_fs, &label);
  if(label=="detail:")
  {
    rational_t detail_dist;
    serial_in(*my_fs, &detail_dist);
    if ( my_visrep )
    {
      assert( my_visrep->get_type()==VISREP_PMESH );
      my_visrep->set_min_detail_distance( detail_dist );
    }
    serial_in(*my_fs, &detail_dist);
    if ( my_visrep )
    {
      my_visrep->set_max_detail_distance( detail_dist );
    }
    serial_in(*my_fs, &label);
    assert(label==chunkend_label);
    serial_in(*my_fs, &label);
  }
  // Read Collision Geometry
  stringx colmesh_fname;
  assert(label=="collision:");
  serial_in(*my_fs, &colmesh_fname);

  if (colmesh_fname=="capsule")
  {
    // colgeom is a capsule.
    colgeom = NEW collision_capsule(this);
    last_po = NEW po;
  }
  else if (colmesh_fname=="none")
  {
    colgeom = NULL;
  }
  else
  {
    // colgeom is a mesh.
    stringx full_fname = my_fs->get_dir() + colmesh_fname;
    if (is_stationary())
    {
      colgeom = cg_mesh_bank.new_instance( full_fname, 0 );
      set_flag(EFLAG_COLGEOM_INSTANCED,true);
    }
    else
      colgeom = NEW cg_mesh(full_fname.c_str(), cg_mesh::ALLOW_WARNINGS);
    colgeom->set_owner(this);
  }

  if ( delete_stream )
  {
    assert( my_fs );
    delete my_fs;
    my_fs = NULL;
  }

  if ( my_visrep )
    set_radius( my_visrep->get_radius(0) );

  if ( colgeom )
  {
    set_flag( EFLAG_PHYSICS, true );
    if (min_detail <= os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL))
      set_collisions_active( true );
  }
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* entity::make_instance( const entity_id& _id,
                               unsigned int _flags ) const
{
//  if(_id.get_val() == "FORGE01_FORGE")
//    int b = 1;

  entity* ent = NEW entity( _id, _flags );
  ent->copy_instance_data( *this );

  return ent;
}

void entity::copy_instance_data( const entity& b )
{
  min_detail = b.min_detail;
  flavor = b.flavor;
  anim_id = b.anim_id;
  //my_po = b.my_po;
  radius = b.radius;
  vis_xz_rad_rel_center = b.vis_xz_rad_rel_center;
  bone_idx = b.bone_idx;

  // copy flags that are not per-instance
  copy_flags( b );

  TextureFrame = b.GetTextureFrame();
  MaterialMask = b.GetMaterialMask();
  cull_entity = true;

  // visual
  if ( b.my_visrep )
  {
    if ( b.my_visrep->is_instanced() )
      my_visrep = new_visrep_instance( b.my_visrep );
    else
      my_visrep = new_visrep_copy( b.my_visrep );

    // make active if uv animated
    if( my_visrep->get_type()==VISREP_PMESH && ((vr_pmesh*)my_visrep)->is_uv_animated() )
      set_flag( EFLAG_ACTIVE, true );

    set_flag(EFLAG_GRAPHICS,true);
  }

//#pragma todo("A better fix in colgeom constructor, maybe?  jdf 4/5/01")
  // Colgeom constructor expects dude at the origin
  po old_po = get_rel_po();
  set_rel_po_no_children(po_identity_matrix);

  // collision
  if ( b.colgeom )
  {
    if ( is_flagged(EFLAG_COLGEOM_INSTANCED) )
    {
      // use existing collision geometry
      colgeom = cg_mesh_bank.new_instance( (cg_mesh*)b.colgeom );
    }
    else
      colgeom = b.colgeom->make_instance( this );
    set_flag(EFLAG_PHYSICS,true);
    if (min_detail <= os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL))
      set_collisions_active( true );
  }

  // restore it
  set_rel_po_no_children(old_po);

  if(b.destroy_info)
    destroy_info = b.destroy_info->make_instance(this);
  else
    destroy_info = NULL;

  // copy any common brain weapons (from enx)
  int num_items = b.get_num_items();
  for(int i = 0; i<num_items; ++i)
  {
    item *itm = b.get_item(i);

    if(is_an_item())
    {
      if ( itm->is_linked() )
      {
        item* new_itm = (item*)itm->make_instance( entity_id::make_unique_id(), 0 );
        g_world_ptr->add_dynamic_instanced_entity(new_itm);
        add_item( new_itm );
        assert(new_itm->is_linked());
      }
    }
    else
    {
#if 0 // BIGCULL
      if(itm->is_a_handheld_item())
      {
        handheld_item *h_itm = (handheld_item *)itm;
        if ( h_itm->is_brain_weapon() && h_itm->is_common_brain_weapon() )
        {
          handheld_item* new_itm = (handheld_item*)h_itm->make_instance( entity_id::make_unique_id(), 0 );
          g_world_ptr->add_dynamic_instanced_entity(new_itm);
          bool added;

/*!          if ( is_a_character() )
            added = new_itm->give_to_character( (character*)this );
          else
!*/
            added = add_item( new_itm );

          assert( added );

          if(h_itm->is_drawn())
            new_itm->draw();
          else
            new_itm->holster();
//          new_itm->hide();

          if(new_itm->is_a_gun())
            g_world_ptr->guarantee_active( new_itm );

          assert(new_itm->is_brain_weapon() && new_itm->is_common_brain_weapon());
        }
      }
#endif // BIGCULL
    }
  }

  // characters should never copy brains (only turrets, etc...)
  if(b.my_controller != NULL /*! && !b.is_a_character() !*/)
  {
    assert(my_controller == NULL);

    entity_controller *c = b.my_controller->make_instance( this );
    g_world_ptr->add_controller(c);
    set_controller(c);
  }

  if(b.has_ai_ifc())
  {
    if(!has_ai_ifc())
      create_ai_ifc();

    ai_ifc()->copy(b.ai_ifc());
    ai_ifc()->set_active(true);
  }

  if(b.has_animation_ifc())
  {
    if(!has_animation_ifc())
      create_animation_ifc();

    animation_ifc()->copy(b.animation_ifc());
  }

#ifdef ECULL
  if(b.has_sound_ifc())
  {
    if(!has_sound_ifc())
      create_sound_ifc();

    sound_ifc()->copy(b.sound_ifc());
  }
#endif

  if(b.has_hard_attrib_ifc())
  {
    if(!has_hard_attrib_ifc())
      create_hard_attrib_ifc();

    hard_attrib_ifc()->copy(b.hard_attrib_ifc());
  }


#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)

#if defined(TARGET_XBOX)
  if(b.my_mesh)
  {
    const char *mesh_name = b.my_mesh->Name.c_str();

    my_mesh = nglGetMesh(mesh_name);

/*	Not what we want on Xbox. (dc 04/30/02)
    nglMesh *Mesh = my_mesh;
    if ( Mesh )
    {
      for ( u_int i = 0; i < Mesh->NSections; i++ )
      {
        nglMeshSection* Section = &Mesh->Sections[i];
        nglMaterial* Material = Section->Material;

        if (Material->Flags & NGLMAT_BACKFACE_CULL)
        {
          nglPrintf(PRINT_RED "Mesh %s has backface culling set.  Turning it off.\n" PRINT_BLACK, mesh_name);
        }
        Material->Flags &= ~NGLMAT_BACKFACE_CULL;
      }
    }
*/
  }
  else
  {
    assert(!my_mesh);
    my_mesh = 0;
  }
#else
  my_mesh = b.my_mesh;
#endif /* TARGET_XBOX JIV DEBUG */

  lores_mesh = b.lores_mesh;
  shadow_mesh = b.shadow_mesh;
	//set_mesh_flags(NGLMESH_LIGHTCAT_1);
#endif

#if _ENABLE_WORLD_EDITOR
  scene_flags = b.scene_flags;
  ent_filename = b.ent_filename;
#endif

  set_clone(true);
}


///////////////////////////////////////////////////////////////////////////////
// Interfaces
///////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

static const char* entity_signal_names[] =
{
  #define MAC(label,str)  str,
  #include "entity_signals.h"
  #undef MAC
};

unsigned short entity::get_signal_id( const char *name )
{
  unsigned idx;

  for( idx = 0; idx < (sizeof(entity_signal_names)/sizeof(char*)); ++idx )
  {
    // compare with the end of entity_signal_name string
    // so you don't need to add the "entity::" part if you don't want
    unsigned offset = strlen(entity_signal_names[idx])-strlen(name);

    if( offset > strlen( entity_signal_names[idx] ) )
      continue;

    if( !strcmp(name,&entity_signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
  }

  // not found
  return signaller::get_signal_id( name );
}

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void entity::register_signals()
{
  // for descendant class, replace "entity" with appropriate string
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "entity_signals.h"
  #undef MAC
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* entity::get_signal_name( unsigned short idx ) const
{
  assert( idx < N_SIGNALS );
  if ( idx <= (unsigned short)PARENT_SYNC_DUMMY )
    return signaller::get_signal_name( idx );
  else
    return entity_signal_names[idx-PARENT_SYNC_DUMMY-1];
}


///////////////////////////////////////////////////////////////////////////////
// Po stuff
///////////////////////////////////////////////////////////////////////////////


void entity::get_direction(vector3d* target) const
{
  *target = get_abs_po().get_facing();
}


void entity::get_velocity(vector3d* target) const
{
  if (has_parent())
  {
    const bone * next_parent = link_ifc()->get_parent();
    po relative_to_parent = get_rel_po();
    while( next_parent->has_parent() )
    {
      relative_to_parent = relative_to_parent * next_parent->get_abs_po();
      next_parent = next_parent->link_ifc()->get_parent();
    }
    vector3d vel;
    ((entity *)next_parent)->get_velocity(&vel);
    vector3d angvel;
    ((entity *)next_parent)->get_angular_velocity(&angvel);
    *target=vel + cross( relative_to_parent.get_position(),angvel);
  }
  else
  {
    if (mi && mi->frame_delta_valid)
    {
      *target = mi->frame_delta.get_position()*(1/mi->frame_time);
    }
    else
      *target=vector3d(0,0,0);
  }
}

void entity::get_angular_velocity(vector3d* target) const
{
  *target=vector3d(0,0,0);
}


// local workspace for adding entity to regions
static vector<region_node*> new_regions(32);  // permanent


void entity::compute_sector( terrain& ter, bool use_high_res_intersect )
{
#ifndef REGIONCULL
  START_PROF_TIMER(proftimer_compute_sector);

  set_needs_compute_sector(false);

  // while forced to region(s), sector is irrelevant;
  // also, we're not allowing limbs to compute their sector

  if ( !is_flagged(EFLAG_REGION_FORCED) && !is_a_limb_body() )
  {
    vector3d curpos = get_abs_position();
    rational_t poshash = POSHASH(curpos);
    if ( poshash != last_compute_sector_position_hash )
    {
      last_compute_sector_position_hash = poshash;
      sector* sec = ter.find_sector( curpos ); //terrain_position() );
      if (sec)
      {
        assert( sec->get_region() );
        if ( terrain_radius() > 0 )
        {
          // store region of origin
          center_region = sec->get_region();
          // entity obviously intersects the region he currently belongs to;
          // adjacent regions will be checked recursively
          // NOTE: new_regions is a local workspace for listing the regions he intersects
          new_regions.resize(0);
          region::prepare_for_visiting();
          _intersect( sec->get_region(), use_high_res_intersect );
          // now remove entity from regions he has departed and add him to regions he has entered
          _update_regions();
        }
        else
        {
          // entity with zero radius
          if ( !in_regions.empty() )
          {
            if ( sec->get_region() != *in_regions.begin() )
            {
              // entity moved from one region to another
              remove_from_regions();
              center_region = sec->get_region();
              add_region( sec->get_region() );
            }
          }
          else
          {
            center_region = sec->get_region();
            add_region( sec->get_region() );
          }
        }
      }
//      set_flag(EFLAG_MISC_COMP_SECT_THIS_FRAME,true);
      my_sector = sec;
    }
  }

  ADD_PROF_COUNT(profcounter_compute_sector, 1);
  STOP_PROF_TIMER(proftimer_compute_sector);
#endif
}


// add given region to list and add entity to region
bool entity::add_region( region_node* r )
{
#ifndef REGIONCULL
  if ( r && in_regions.insert( r ).second )
  {
    // this virtual function allows descendant types to be recognized when
    // adding them to regions, so that the region class can maintain lists of
    // different entity types as desired
#endif
    add_me_to_region( r->get_data() );
    return true;
#ifndef REGIONCULL
  }
  return false;
#endif
}


// these virtual functions allow types descended from entity to be
// recognized when adding them to regions, so that the region class can
// maintain lists of different entity types as desired
void entity::add_me_to_region( region* r )
{
  r->add( this );
}

void entity::remove_me_from_region( region* r )
{
  r->remove( this );
}

// remove entity from all regions in which he is currently listed
void entity::remove_from_regions()
{
#ifndef REGIONCULL
	for ( region_node_pset::iterator i=in_regions.begin(); i!=in_regions.end(); i++ )
    remove_me_from_region( (*i)->get_data() );
  in_regions.clear();
#endif
}

void entity::remove_from_terrain()
{
  remove_from_regions();
  my_sector = NULL;
  center_region = NULL;
}


// remove entity from regions no longer inhabited (according to NEW list), and
// add entity to regions newly inhabited
void entity::_update_regions()
{
#ifndef REGIONCULL
  region_node_pset::iterator i,j;
  for ( i=in_regions.begin(); i!=in_regions.end(); )
  {
    region* r = (*i)->get_data();
    if ( !r->already_visited() )
    {
      remove_me_from_region( r );
      j = i;
      ++j;
      in_regions.erase( i );
      i = j;
    }
    else
      ++i;
  }
  vector<region_node*>::iterator k;
  for ( k=new_regions.begin(); k!=new_regions.end(); k++ )
    add_region( *k );
#endif
}

// INTERNAL
// add entity to given region and recurse into any adjacent intersected regions
void entity::_intersect( region_node* r, bool use_high_res_intersect )
{
  // add region to list
  r->get_data()->visit();
  new_regions.push_back( r );
  // check for intersection with portals leading from this region
  edge_iterator tei = r->begin();
  edge_iterator tei_end = r->end();
  for ( ; tei!=tei_end; ++tei )
  {
    // don't bother with regions we've already visited
    region_node* dest = (*tei).get_dest();
    if ( !dest->get_data()->already_visited() )
    {
      portal* port = (*tei).get_data();
      // don't need to recurse across inactive portals, unless you're a door or doorframe!
      if ( is_door() ||
           flavor==ENTITY_ENTITY ||
           (flavor==ENTITY_CONGLOMERATE && (my_visrep || colgeom)) ||
           port->is_active() )
      {
        // intersection of entity sphere and portal cylinder
        if ( port->touches_sphere(sphere(terrain_position(),terrain_radius())) )
        {
          if ( use_high_res_intersect && get_vrep() && get_vrep()->get_type()==VISREP_PMESH
              #ifdef TARGET_PC
               && !((vr_pmesh*)get_vrep())->is_optimized_static_mesh_for_d3d()
              #endif
              )
          {
//#pragma todo("This needs PS2 equivalent  jdf 3/21/01")
            // intersection of entity mesh and portal mesh
            vr_pmesh* m = static_cast<vr_pmesh*>( get_vrep() );
            int i, j;
            const vector3d& v0 = terrain_position();
            for ( i=0; i<m->get_num_wedges(); ++i )
            {
              vector3d v1 = get_abs_po().fast_8byte_xform( m->get_xvert_unxform( i ) );
              for ( j=0; j<port->get_max_faces(); ++j )
              {
                vector3d hit_loc;
                if ( collide_polygon_segment( j, port, v0, v1, hit_loc ) )
                {
                  _intersect( dest, use_high_res_intersect );
                  i = m->get_num_wedges();
                  break;
                }
              }
            }
          }
          else
            _intersect( dest, use_high_res_intersect );
        }
      }
    }
  }
}

// force entity to belong to given region until un-forced (see below)
void entity::force_region( region_node* r )
{
  if ( !is_flagged(EFLAG_REGION_FORCED) )
  {
    // first forced region for this entity;
    // remove from previous regions (if any)
    remove_from_regions();
  }
  _set_region_forced_status();
  // add to region list and add entity to region
  add_region( r );
}

void entity::change_visrep( const stringx& new_visrep_name )
{
  unload_visual_rep( my_visrep );
  my_visrep = find_visual_rep( new_visrep_name );
}


// Position history stuff

// Rel pos is guarenteed to be ZEROVEC when first_time = true;
vector3d entity::get_frame_abs_delta_position( bool first_time, const vector3d& rel_delta_pos ) const
{
  vector3d deltapos=ZEROVEC;
  if (!first_time)
  {
    deltapos = get_rel_po().non_affine_inverse_scaled_xform(rel_delta_pos);
  }
  if (mi && mi->frame_delta_valid)
    deltapos += mi->frame_delta.get_position();
  else
    deltapos += ZEROVEC;
  if (has_parent())
  {
    deltapos = ((entity *)link_ifc()->get_parent())->get_frame_abs_delta_position(false,deltapos);
  }
  return deltapos;
}

void entity::invalidate_frame_delta()
{
  if (mi)
  {
    mi->last_frame_delta_valid = mi->frame_delta_valid;
    mi->frame_delta_valid = false;
    mi->frame_delta=po_identity_matrix;
  }
}



vector3d entity::get_last_position() const
{
  return get_abs_position()-get_frame_abs_delta_position();
}


void entity::set_created_entity_default_active_status()
{
  switch ( flavor )
  {
    case ENTITY_ENTITY:
    case ENTITY_MARKER:
    case ENTITY_MIC:
    case ENTITY_LIGHT_SOURCE:
    case ENTITY_CONGLOMERATE:
      set_active( false );
      break;
    default:
      set_active( true );
      break;
  }
}


void entity::set_last_po( const po& the_po )
{
  if (last_po)
    *last_po = the_po; //get_colgeom_root_po();
}


const po& entity::get_last_po()
{
  if (last_po)
    return *last_po;
  else
    return get_abs_po();
}

void entity::set_family_visible( bool _vis, bool _cur_variant_only )
{
  set_visible( _vis );
/*!  if( has_children() )
  {
    list<entity*>::iterator ch;
    for( ch = ci->children.begin(); ch != ci->children.end(); ++ch )
    {
      if( (!_cur_variant_only) || (*ch)->is_flagged( EFLAG_MISC_MEMBER_OF_VARIANT ) )
      {
        (*ch)->set_family_visible( _vis, _cur_variant_only );
      }
    }
  }
!*/
}

////////////////////////////////////////////////////////////////////////////////
char* strdupcpp(const char* str)
{
  char* retstr;
  retstr = NEW char[ strlen(str)+1 ];
  strcpy( retstr, str );
  return retstr;
}


light_manager* entity::get_light_set() const
{
  if (!my_light_mgr && has_parent())
    return ((entity *)link_ifc()->get_parent())->get_light_set();

  return my_light_mgr;
}

void entity::create_light_set()
{
  if(!my_light_mgr)
    my_light_mgr = NEW light_manager();
}


// Lock material frames

void entity::ifl_lock(int frame_index)
{
  if (my_visrep)
  {
    if ((frame_index >= 0) && (frame_index<my_visrep->get_anim_length()))
      frame_time_info.set_ifl_frame_locked(frame_index);
  }
 // else warning("No visual representation is available on the entity.");
}

void entity::ifl_pause()
{
  if (my_visrep)
  {
    int frame_locked = frame_time_info.get_ifl_frame_locked();
    if (frame_locked < 0) // No lock or pause on entity. Do nothing if entity is locked or paused.
    {
      int period = my_visrep->get_anim_length();
      if (period < 0)
        warning("The animation length of each material should be the same.");
      else
      {
        int current_frame = frame_time_info.time_to_frame(period);
        ifl_lock(current_frame); // Pause calls lock
      }
    }
  }
 // else
 //   warning("No visual representation is available on the entity.");
}

void entity::ifl_play()
{
  if (my_visrep)
  {
    int period = my_visrep->get_anim_length();
    if (period < 0)
      warning("The animation length of each material should be the same.");
    else
    {
      /*
      if (period > 1)
        nglPrintf("aaaaaaaaaaaaaaaaa\n");
     */
      frame_time_info.compute_boost_for_play(period);
      frame_time_info.set_ifl_frame_locked(-1);
    }
  }
 // else
 //   warning("No visual representation is available on the entity.");
}



bool entity::is_statically_visually_sortable()
{
  return( ( (!is_flagged(EFLAG_MISC_NONSTATIC)) || ((flavor==ENTITY_PARTICLE_GENERATOR) && !is_flagged(EFLAG_MISC_RAW_NONSTATIC)) ) &&
            get_visual_xz_radius_rel_center()<MAX_VISUALLY_SORTED_RADIUS);
}

/*!
entity* entity::get_flavor_parent(entity_flavor_t flav)
{
  entity *ent = get_parent();
  while(ent != NULL && ent->get_flavor() != flav)
    ent = ent->get_parent();

  return(ent);
}
!*/
void entity::set_door( bool d )
{
  if ( d )
  {
    ext_flags |= EFLAG_EXT_IS_DOOR;
    set_door_closed( true );
  }
  else
    ext_flags &=~ EFLAG_EXT_IS_DOOR;
}

void entity::set_door_closed(bool d)
{
  bool oldstate = is_door_closed();

  if (is_door() && d)
    ext_flags |= EFLAG_EXT_DOOR_CLOSED;
  else
    ext_flags &=~ EFLAG_EXT_DOOR_CLOSED;

  if (oldstate != is_door_closed())
    if (door_portal)
      door_portal->set_active(!is_door_closed());
}


void entity::set_alternative_materials( material_set* arg )
{
//#pragma todo("Needs ps2 equivalent -jdf 3-21-01")
  if ( arg && get_vrep() )
  {
    switch ( get_vrep()->get_type() )
    {
    case VISREP_PMESH:
      // verify that material set matches mesh (numerically)
      if ( static_cast<vr_pmesh*>(get_vrep())->get_num_materials() != (int)arg->data->size() )
        warning( get_id().get_val() + ": set_alternative_materials('" + *arg->name + "'): number of materials in given material set does not match that expected in visrep" );
      break;
    default:
      warning( get_id().get_val() + ": set_alternative_materials('" + *arg->name + "'): this entity does not have a mesh visrep; alternative material is not applicable" );
      arg = NULL;
      break;
    }
  }
  alternative_materials = arg;
}

void entity::set_alternative_materials( const stringx& alt_mat_name )
{
  if ( alt_mat_name.size()==0 || alt_mat_name=="none" )
    set_alternative_materials( NULL );
  else
    set_alternative_materials( g_world_ptr->get_material_set(alt_mat_name) );
}

bool entity::get_distance_fade_ok() const
{
  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISTANCE_FADING))
    return false;
  if (!my_visrep)
    return false;
  return my_visrep->get_distance_fade_ok();
}

void entity::copy_flags( const entity& b )
{
  flags |= (b.flags & EFLAG_COPY_MASK);
  flags &= (b.flags | ~EFLAG_COPY_MASK);
  ext_flags |= (b.ext_flags & EFLAG_EXT_COPY_MASK);
  ext_flags &= (b.ext_flags | ~EFLAG_EXT_COPY_MASK);

  if(is_ext_flagged(EFLAG_EXT_ENX_WALKABLE))
    set_walkable( true );
}

#ifdef DEBUG
stringx g_current_entity_name;
stringx g_debug_entity_name="#*!@$"; // Try not to check in as anything but this!
bool    g_use_debug_entity_name=false;
//stringx g_debug_entity_name="NIGHTSKY"; // in .SCN file
#endif


// in entity.cpps
extern void delete_entity_id(entity_id id);

// motion blur stuff
enum { MAX_TRAIL_LENGTH=16 };

// hack to offset ifl displays on entities
int random_ifl_frame_boost_table[256];

////////////////////////////////////////////////////////////////////////////////
// utility macro for drawing motion blur
#define RHW_XFORM( sv, mat, dv, rhw )  \
  { rational_t w = mat[0][3]*sv.x+mat[1][3]*sv.y+mat[2][3]*sv.z+mat[3][3];  \
    rhw = 1/w; \
    dv.x  = mat[0][0]*sv.x+mat[1][0]*sv.y+mat[2][0]*sv.z+mat[3][0];  \
    dv.y  = mat[0][1]*sv.x+mat[1][1]*sv.y+mat[2][1]*sv.z+mat[3][1];  \
    dv.z  = mat[0][2]*sv.x+mat[1][2]*sv.y+mat[2][2]*sv.z+mat[3][2]; }


vector3d entity::get_visual_center() const
{
  if (!my_visrep)
    return get_abs_position();
//  return get_abs_po().slow_xform(my_visrep->get_center(get_age()));
  //return get_abs_po().fast_8byte_xform(my_visrep->get_center(get_age()));
  vector3d ctr=my_visrep->get_center(get_age());
  assert(ctr.y>-1e9F && ctr.y<1e9F);
  return get_abs_po().fast_8byte_xform(ctr);
}

rational_t entity::get_visual_radius() const
{
  return my_visrep ? my_visrep->get_radius(get_age()) : 0;
}

void entity::compute_visual_xz_radius_rel_center()
{
  if ( is_flagged( EFLAG_MISC_NONSTATIC ) )
    vis_xz_rad_rel_center = get_visual_radius();
  else if ( my_visrep )
    vis_xz_rad_rel_center = my_visrep->compute_xz_radius_rel_center( get_abs_po() );
}


// set entity orientation such that it is facing the given world-coordinate point
void entity::look_at( const vector3d& abs_pos )
{
  // compute orientation matrix;
  // z-basis is normalized vector from my position to dest position
  vector3d bz = abs_pos - get_abs_position();
  rational_t bzl = bz.length();
  if ( bzl > 0.001f )
    bz *= 1.0f / bzl;
  else
    bz = ZVEC;
  // x-basis is always on the right
  vector3d bx( bz.z, 0, -bz.x );
  rational_t bxl = bx.length();
  if ( bxl > 0.00001f )
    bx /= bxl;
  else
    bx = XVEC;
  // y-basis is z cross x
  vector3d by = cross( bz, bx );
  // build NEW po
  po newpo( bx, by, bz, ZEROVEC );
//  entity* parent = get_parent();
  if ( has_parent() )
  {
    fast_po_mul(newpo, newpo, link_ifc()->get_parent()->get_abs_po().inverse());
//    newpo = newpo * link_ifc()->get_parent()->get_abs_po().inverse();
  }
  newpo.fixup();
  newpo.set_position( get_rel_position() );
  // set NEW po
  set_rel_po( newpo );
}


// force entity to belong to current region(s) until un-forced (see below)
void entity::force_current_region()
{
  _set_region_forced_status();
}

// INTERNAL
void entity::_set_region_forced_status()
{
  set_flag(EFLAG_REGION_FORCED,true);
  // while forced, the entity has no sector and will not compute one!
  my_sector = NULL;
  center_region = NULL;
}

// un-force forced region(s)
// NOTE: this does not perform an update, so I will have no locale until
// compute_sector is called
void entity::unforce_regions()
{
  if ( is_flagged(EFLAG_REGION_FORCED) )
  {
    remove_from_regions();
    set_flag(EFLAG_REGION_FORCED,false);
  }
}


// put me into the same region(s) as the given entity
void entity::force_regions( entity* e )
{
#ifndef REGIONCULL
  region_node_pset::const_iterator i;
  for ( i=e->get_regions().begin(); i!=e->get_regions().end(); i++ )
    force_region( *i );
#endif
}


int entity::is_in_active_region()
{
#ifndef REGIONCULL
  if(my_sector)
    return(my_sector->get_region()->get_data()->is_active());
  else
  {
    region_node_pset::const_iterator i;
    for ( i=in_regions.begin(); i!=in_regions.end(); ++i )
    {
      if((*i)->get_data()->is_active())
#endif
        return 1;
#ifndef REGIONCULL
    }
  }

  return 0;
#endif
}

bool entity::has_entity_collision() const
{
  return ( colgeom && are_collisions_active() && colgeom->is_entity_collision() );
}

bool entity::has_camera_collision() const
{
  return ( colgeom && are_collisions_active() && colgeom->is_camera_collision() );
}


collision_geometry* entity::get_updated_colgeom(po * replacement_po, rational_t radius_scale)
{
  if (get_colgeom() && !get_colgeom()->is_valid() && !is_stationary())
  {
    update_colgeom(replacement_po);
  }
  if (get_colgeom())
    get_colgeom()->apply_radius_scale(radius_scale);
  return get_colgeom();
}


void entity::update_colgeom(po * replacement_po)
{
  if (get_colgeom())
  {
    collision_geometry * cg = get_colgeom();
    if (replacement_po)
      cg->xform(*replacement_po);
    else
    {
      if (0 && cg->get_type()==collision_geometry::CAPSULE)
      {
        cg->split_xform(get_colgeom_root_po(), get_last_po(), 0);
        #if defined (DEBUG)
        collision_capsule * cc = (collision_capsule *) cg;
        add_capsule_history(*cc);
        #endif
      }
      else
        cg->xform(get_colgeom_root_po());
    }
  }
}


void entity::delete_colgeom()
{
  if(colgeom)
  {
    if (is_flagged(EFLAG_COLGEOM_INSTANCED))
      cg_mesh_bank.delete_instance((cg_mesh *)colgeom);
    else
      delete colgeom;
    colgeom=NULL;
  }
}


void entity::delete_visrep()
{
  if (my_visrep)
  {
    unload_visual_rep(my_visrep);
    my_visrep = NULL;
  }
}

extern profiler_timer proftimer_adv_visrep;

void entity::advance_age( time_value_t t )
{
//  age += t;
  set_age(get_age()+t);
  START_PROF_TIMER(proftimer_adv_visrep);
  if( my_visrep && my_visrep->get_type()==VISREP_PMESH )
    ((vr_pmesh*)my_visrep)->anim_uvs( t );
  STOP_PROF_TIMER(proftimer_adv_visrep);
}

extern profiler_timer proftimer_IFC_sound;
extern profiler_timer proftimer_IFC_damage;
extern profiler_timer proftimer_IFC_physical;
extern profiler_timer proftimer_adv_light_mgr;

void entity::frame_advance( time_value_t t )
{
#if defined(TARGET_XBOX)
  assert( t > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

  #ifdef DEBUG
  if( g_use_debug_entity_name )
  {
    g_current_entity_name=get_name();
    if (g_current_entity_name==g_debug_entity_name)
    { // you can put a breakpoint here
      debug_print("frame_advancing "+g_debug_entity_name);

      //get_light_set()->dump_debug_info();
    }
  }
  #endif

#ifdef ECULL
  START_PROF_TIMER(proftimer_IFC_sound);
  // Sound emitter processing
  if (has_sound_ifc())
    sound_ifc()->frame_advance(t);
  STOP_PROF_TIMER(proftimer_IFC_sound);
#endif

#if 0 // BIGCULL
  START_PROF_TIMER(proftimer_IFC_damage)
  if (has_damage_ifc())
    damage_ifc()->frame_advance(t);
  STOP_PROF_TIMER(proftimer_IFC_damage);
#endif // BIGCULL

  START_PROF_TIMER(proftimer_IFC_physical);
  if(has_physical_ifc())
  {
    if ( physical_ifc()->is_enabled() && !physical_ifc()->is_suspended())
        physical_ifc()->frame_advance(t);
  }
  STOP_PROF_TIMER(proftimer_IFC_physical);

  START_PROF_TIMER(proftimer_adv_light_mgr);
  int heroNum = 0;
  heroNum = get_hero_id();
	updatelighting(t,heroNum);
  STOP_PROF_TIMER(proftimer_adv_light_mgr);

//!!  record_motion();
}

void entity::updatelighting( time_value_t t, const int playerID )
{

  region_node *node = get_primary_region();

  if(my_light_mgr && node)
    my_light_mgr->frame_advance(node->get_data(), t, playerID);
}


extern profiler_timer proftimer_draw_shadow;
extern profiler_timer proftimer_motion_trail;
extern profiler_timer proftimer_instance_render;
extern profiler_timer proftimer_visrep_rend_inst;

extern bool loresmodelbydefault;

void entity::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
#ifdef NGL
	float camdist=0.0f;
	if ( camera_link )
	{
		vector3d rel_pos=get_abs_position();
		vector3d cam_pos=camera_link->get_abs_position();
		vector3d rel_cam_pos=rel_pos-cam_pos; //get_rel_position() - camera_link->get_rel_position();
		camdist=rel_cam_pos.length();
	}

	nglMesh* mesh=NULL;
	if (flavor & RENDER_SHADOW_MODEL)
		mesh = get_shadow_mesh();
	if (g_game_ptr->is_splitscreen() ||
	    !force_hi_res && (camdist> 3.0f ||
			loresmodelbydefault ||
			(flavor & RENDER_LORES_MODEL)))
		mesh = get_lores_mesh();
	if ( mesh==NULL )
	  mesh = get_mesh();
	if ( !mesh )
	  return;

  nglRenderParams params;
	memset(&params,0,sizeof(params));
  params.Flags = 0;

	if ( usezbias )
	{
		params.Flags |= NGLP_ZBIAS;
		params.ZBias = zbias;
	}

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

  if (use_uv_scrolling)
  {
    params.Flags |= NGLP_TEXTURE_SCROLL;
    params.ScrollU = scroll_u;
    params.ScrollV = scroll_v;
  }


  // added to be same as conglom render
  if(my_mesh->Flags & NGL_LIGHTCAT_MASK && get_light_set())
  {
//	  if(!get_light_set()) create_light_set();
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
  }
  else
  {
	  // enable tint if neccessary.  duplicated in entity::render.
	  color32 c = get_render_color();
	  if ( c != color32_white )
	  {
	    params.Flags |= NGLP_TINT;
	    float i = 1.0f / 255.0f;
	    params.TintColor[0] = c.get_red() * i;
	    params.TintColor[1] = c.get_green() * i;
	    params.TintColor[2] = c.get_blue() * i;
	    params.TintColor[3] = c.get_alpha() * i;
	  }
  }

  vector3d s = get_render_scale();
  // Without NGLP_SCALE, transform will be normalized, which removes the flip for lefties.  (dc 05/31/02)
//  if ( s != vector3d( 1, 1, 1 ) )
  {
    params.Flags |= NGLP_SCALE;
    params.Scale[0] = s.x;
    params.Scale[1] = s.y;
    params.Scale[2] = s.z;
    params.Scale[3] = 1.0f;
	if (s.x * s.y * s.z < 0)
	{
		params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// entity is rendered flipped (dc 04/30/02)
	}
  }

  po render_po;
  if ((flip_axis >= 0) && (flip_axis <= 2))
  {
	  render_po = get_handed_abs_po();
	  params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// entity is rendered flipped (dc 04/30/02)
  }
  else
	  render_po = get_abs_po();

  params.Flags |= NGLP_LIGHT_CONTEXT;
  params.LightContext = g_world_ptr->get_current_light_context();
  assert(params.LightContext != NULL);

  START_PROF_TIMER( proftimer_render_add_mesh );
	  nglListAddMesh( mesh, native_to_ngl( render_po ), &params );
  STOP_PROF_TIMER( proftimer_render_add_mesh );
#else
  #ifdef DEBUG
  if( g_use_debug_entity_name )
  {
    g_current_entity_name=get_name();
    if (g_current_entity_name==g_debug_entity_name)
    { // you can put a breakpoint here
      debug_print("rendering %s, flavor 0x%x",g_debug_entity_name.c_str(),flavor);
    }
  }
  #endif
	//nglPrintf("rendering %s, flavor 0x%x\n", get_name().c_str(), flavor);

  light_manager* lm = get_light_set();
  render_heart( detail, flavor, lm, 0, entity_translucency_pct );
#endif
}


	// 0 = dark shadow, 1 = player reflection
extern float shadow_reflective_value;


void entity::rendershadow( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct, rational_t scale )
{
#ifdef NGL
  nglMesh* mesh=NULL;
  if (flavor & RENDER_SHADOW_MODEL)
	  mesh = get_shadow_mesh();
  if (flavor & RENDER_LORES_MODEL)
	  mesh = get_lores_mesh();
	if ( mesh==NULL )
  	mesh = get_mesh();
  if ( !mesh )
    return;

  nglRenderParams params;
	memset(&params,0,sizeof(params));
  params.Flags = 0;

	if ( usezbias )
	{
		params.Flags |= NGLP_ZBIAS;
		params.ZBias = zbias;
	}

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
#if 0
  else
  {
    // workaround for some ifls not being updated correctly.
    extern int KS_TextureAnimFrame;
	  params.Flags |= NGLP_TEXTURE_FRAME;
	  params.TextureFrame = KS_TextureAnimFrame;
  }
#endif

  if (use_uv_scrolling)
  {
    params.Flags |= NGLP_TEXTURE_SCROLL;
    params.ScrollU = scroll_u;
    params.ScrollV = scroll_v;
  }

  // enable tint if neccessary.  duplicated in entity::render.
  params.Flags |= NGLP_TINT;
  params.TintColor[0] = shadow_reflective_value; //c.get_red() * i;
  params.TintColor[1] = shadow_reflective_value; //c.get_green() * i;
  params.TintColor[2] = shadow_reflective_value; //c.get_blue() * i;
  params.TintColor[3] = entity_translucency_pct; //c.get_alpha() * i;

//  vector3d s = get_render_scale();
// Without NGLP_SCALE, transform will be normalized, which removes the flip for lefties.  (dc 05/31/02)
//  if ( scale != 1.0f ) //s != vector3d( 1, 1, 1 ) )
  {
    params.Flags |= NGLP_SCALE;
    params.Scale[0] = scale;
    params.Scale[1] = scale;
    params.Scale[2] = scale;
    params.Scale[3] = 1.0f;
	if (scale < 0)
	{
		params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// entity is rendered flipped (dc 04/30/02)
	}
  }
	params.Flags |= NGLP_NO_CULLING;
	params.Flags |= NGLP_NO_LIGHTING;
	#if !defined (TARGET_GC) && !defined(TARGET_XBOX)
	params.Flags |= NGLP_WRITE_FB_ALPHA;
	#endif

  po render_po;
  if ((flip_axis >= 0) && (flip_axis <= 2))
  {
	  render_po = get_handed_abs_po();
	  params.Flags ^= NGLP_REVERSE_BACKFACECULL;	// entity is rendered flipped (dc 04/30/02)
  }
  else
	  render_po = get_abs_po();

  START_PROF_TIMER( proftimer_render_add_mesh );
  nglListAddMesh( mesh, native_to_ngl( render_po ), &params );
  STOP_PROF_TIMER( proftimer_render_add_mesh );
#else
  #ifdef DEBUG
  if( g_use_debug_entity_name )
  {
    g_current_entity_name=get_name();
    if (g_current_entity_name==g_debug_entity_name)
    { // you can put a breakpoint here
      debug_print("rendering %s, flavor 0x%x",g_debug_entity_name.c_str(),flavor);
    }
  }
  #endif
//nglPrintf("rendering %s, flavor 0x%x\n", get_name().c_str(), flavor);

  light_manager* lm = get_light_set();
  render_heart( detail, flavor, lm, 0, entity_translucency_pct );
#endif

}

// permanent:
/*
#ifndef PROJECT_STEEL
//static vert_buf_xformed motiontrailverts; // I'd rather just use vert_workspace_xformed
#define motiontrailverts vert_workspace_xformed
#endif
*/
void entity::render_heart( rational_t detail, render_flavor_t flavor, light_manager* light_set, unsigned force_flags, rational_t entity_translucency_pct )
{
#ifdef TARGET_PC
/*
#ifndef PROJECT_STEEL
  int                          i, j;
  rational_t                   lerp;
  hw_rasta_vert_xformed       *vert_it;
  qt                           qt0, qt1;
  quaternion                   q;
  vector3d                     t;
  matrix4x4                    m;
  if(is_motion_trailed()||is_motion_blurred())
  {
    record_motion();
  }
#endif
*/
  instance_render_info *viri = app::inst()->get_viri();
  if (is_flagged(EFLAG_GRAPHICS) && is_visible())
  {
    assert( my_visrep );

    // draw shadow
/*P
    if( is_flagged(EFLAG_MISC_CAST_SHADOW) )
  #if !defined( TARGET_MKS )
    if( flavor & RENDER_TRANSLUCENT_PORTION )
  #endif
    {
      proftimer_draw_shadow.start();
      po shadow_po = po_identity_matrix;
      vector3d spos, ground_normal = vector3d( 0, 1, 0 );
      instance_render_info iri;

      spos = get_abs_position();
      spos.y = g_world_ptr->get_the_terrain().get_elevation( spos, ground_normal, get_region() ) + 0.1f;
      shadow_po.set_position( spos );
      shadow_po.set_facing( spos + ground_normal );
      po shadow_scale;
      rational_t scale = get_visual_radius()*0.5F;
      shadow_scale.set_scale( vector3d( scale, scale, scale) );
      shadow_po = shadow_scale * shadow_po;
      iri = instance_render_info( vr_dropshadow::inst()->get_max_faces(), shadow_po, 0, get_region() );
      vr_dropshadow::inst()->render_instance( flavor, &iri);
      proftimer_draw_shadow.stop();
    }
P*/

/*
    #ifndef PROJECT_STEEL
    if(is_motion_trailed())
    {
      proftimer_motion_trail.start();

    #if !defined(TARGET_MKS)
      if(flavor & RENDER_TRANSLUCENT_PORTION)
    #endif
      {
        assert(mbi);
        const matrix4x4& w2vp = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

        // motion blur trail
        // generate the front faces
        j = 0;
        motiontrailverts.lock(mbi->motion_trail_count*2);
        vert_it = motiontrailverts.begin();

        for(i = 0; i < mbi->motion_trail_count; ++i)
        {
          if( mbi->motion_trail_count>=2 )
            lerp = float(i) / float(mbi->motion_trail_count-1);
          else
            lerp = 0;
          mbi->motion_trail_buffer[(mbi->motion_trail_start + i) % mbi->motion_trail_length].q.to_matrix( &m );
          t = mbi->motion_trail_buffer[(mbi->motion_trail_start + i) % mbi->motion_trail_length].t;

          vector3d v1,v2;
          vector3d v1vp,v2vp;
          float rhw1,rhw2;
          v1 = po(m).slow_xform(mbi->motion_trail_head)+t;  // world space
          RHW_XFORM( v1, w2vp, v1vp, rhw1 );  // viewport now in v1vp, rhw1
          v1vp *= rhw1;
          vector3d new_tail = lerp * (mbi->motion_trail_tail - mbi->motion_trail_head) + mbi->motion_trail_head;
          v2 = po(m).slow_xform(new_tail)+t;
          RHW_XFORM( v2, w2vp, v2vp, rhw2 );
          v2vp *= rhw2;
          color32 this_segment_color = mbi->trail_color;
          this_segment_color.c.a = mbi->trail_min_alpha + (mbi->trail_max_alpha - mbi->trail_min_alpha) * lerp;
          vert_it[j++] = hw_rasta_vert_xformed(v1vp, rhw1, texture_coord(0.0f, 0.0f), this_segment_color);
          this_segment_color.c.a >>= 1;

          vert_it[j++] = hw_rasta_vert_xformed(v2vp, rhw2, texture_coord(0.0f, 1.0f), this_segment_color);
        }
        assert(j==mbi->motion_trail_count*2);
        motiontrailverts.unlock();
        if(j > 2)
        {
          // need to make a permanent material to use for this effect instead!
          vertex_context vc;
          vc.set_translucent(true);
          // we use the white texture because NULL texture is undefined
          // color on the sega side, or seems to be, anyway.
          vc.set_texture( hw_texture_mgr::inst()->get_white_texture() );
          vc.set_src_alpha_mode( vertex_context::SRCALPHA );
          vc.set_dst_alpha_mode( vertex_context::ONE );
          vc.set_fog( false );
          vc.set_cull_mode( vertex_context::CNONE );
          hw_rasta::inst()->send_context(vc);
          material::flush_last_context();
          hw_rasta::inst()->send_vertex_strip(motiontrailverts, j);
        }
      }

      proftimer_motion_trail.stop();
    }
  #endif // !PROJECT_STEEL
*/
  }

  proftimer_instance_render.start();
// this statement is broken up to get around the CodeWarrior long branch bug
  if (is_flagged(EFLAG_GRAPHICS) && is_visible())
  {
//!!    bool needs_lighting = is_flagged(EFLAG_GRAPHICS_FORCE_LIGHT);  // FORCE_LIGHT is stored with the mesh now.
/*
  #ifndef PROJECT_STEEL
    if ( is_motion_blurred() )
    {
//      if(flavor & RENDER_TRANSLUCENT_PORTION)
      {
        assert(mbi);
        const matrix4x4& w2vp = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

        // render the motion blurred copies
        qt0 = mbi->motion_trail_buffer[(mbi->motion_trail_start + mbi->motion_trail_count - 1) % mbi->motion_trail_length];
        if(mbi->motion_trail_count > 1)
          qt1 = mbi->motion_trail_buffer[(mbi->motion_trail_start + mbi->motion_trail_count - 2) % mbi->motion_trail_length];
        else
          qt1 = mbi->motion_trail_buffer[(mbi->motion_trail_start + mbi->motion_trail_count - 1) % mbi->motion_trail_length];

        float recip_num_images = 1.0f / mbi->num_blur_images;
        for(i = 0; i < mbi->num_blur_images; ++i)
        {
          float step = (float)(i+1)*recip_num_images;
          lerp = step * mbi->blur_spread;
          q = slerp(qt0.q, qt1.q, lerp);
          t = qt0.t + lerp * (qt1.t - qt0.t);
          q.to_matrix( &m );
          m[3][0] = t.x;
          m[3][1] = t.y;
          m[3][2] = t.z;
          int alpha = mbi->blur_min_alpha + (mbi->blur_max_alpha-mbi->blur_min_alpha)*(1.0f-step);
          viri[0] = instance_render_info(detail, m, frame_time_info //,get_age()
            , get_region()//, random_ifl_frame_boost
            , color32(255, 255, 255, alpha), FORCE_TRANSLUCENCY, 0, 1, NULL
            , get_alternative_materials() );
          my_visrep->render_instance(flavor, viri);
        }
      }
    }
  #endif // !PROJECT_STEEL
*/
  /*
    if( my_visrep->get_type()==VISREP_PMESH )
    {
      vr_pmesh* casted = (vr_pmesh*)my_visrep;
    #if defined(DEBUG) && 0
      if( casted->get_num_materials()==0)
      {
        warning(stringx("No materials in ")+get_id().get_val());
      }
    #endif
    }
  */

    rational_t old_clip_dist =fog_manager::inst()->get_fog_end_distance();
    if ( is_flagged( EFLAG_GRAPHICS_NO_DISTANCE_CLIP ) ) // for sky?
      geometry_manager::inst()->set_clip_distance( 10000.0f );

    if (is_a_sky())
      force_flags |= FORCE_NO_LIGHT;

    color32 act_color = render_color;
    act_color.set_alpha( uint8(entity_translucency_pct * (rational_t)render_color.get_alpha()) );
    if ( act_color.get_alpha() < 0xFF )
      force_flags |= FORCE_TRANSLUCENCY;

    viri[0] = instance_render_info(detail, get_abs_po(), frame_time_info
      , get_primary_region()  // get_primary_region accounts for force_region'd entities
      , act_color, force_flags|((flavor&RENDER_CLIPPED_FULL_DETAIL)?0:FORCE_SKIP_CLIP)
      , 0, 1.0f, light_set, get_alternative_materials() );

    proftimer_visrep_rend_inst.start();
    my_visrep->render_instance(flavor, viri);
    proftimer_visrep_rend_inst.stop();

    if ( is_flagged( EFLAG_GRAPHICS_NO_DISTANCE_CLIP ) )
      geometry_manager::inst()->set_clip_distance( old_clip_dist );
  }
  proftimer_instance_render.stop();
#endif // TARGET_PC
}


int entity::get_max_polys() const
{
  assert( my_visrep );
  return my_visrep->get_max_faces( get_age() );
}

int entity::get_min_polys() const
{
  assert( my_visrep );
  return my_visrep->get_min_faces( get_age() );
}

// Motion Blur support
extern rational_t g_level_time;

void entity::record_motion()
{
  // save the motion blur rotation as quaternions as we're going to want to
  // interpolate between records
  if(is_motion_blurred() || is_motion_trailed())
  {
    assert(mbi);
    if( mbi->last_motion_recording != g_level_time )
    {
//      matrix4x4 m = get_abs_po().get_matrix(); // unused, remove me?
      mbi->motion_trail_buffer[mbi->motion_trail_end].q = quaternion(get_abs_po().get_matrix());
      matrix4x4 n;
      mbi->motion_trail_buffer[mbi->motion_trail_end].q.to_matrix( &n );
      mbi->motion_trail_buffer[mbi->motion_trail_end].t = get_abs_po().get_position();
      mbi->motion_trail_end++;
      if(mbi->motion_trail_end >= mbi->motion_trail_length)
        mbi->motion_trail_end = 0;
      if(mbi->motion_trail_count < mbi->motion_trail_length)
      {
        ++mbi->motion_trail_count;
      }
      else
      {
        // whee, the circle is complete
        mbi->motion_trail_start = mbi->motion_trail_end;
      }
      mbi->last_motion_recording = g_level_time;
    }
  }
}


// turn on or off the motion blur effect
void entity::allocate_motion_info()
{
  assert( mbi == NULL );

  // CTT 07/22/00: for the Max Steel project, the NONSTATIC flag only matters
  // for walkable entities
  // guard against dynamic allocation in static entity
  assert( !is_walkable() || is_flagged(EFLAG_MISC_NONSTATIC) );

  // allocate the mbi the first time the user turns on motion blur or trail
  mbi = NEW motion_blur_info(MAX_TRAIL_LENGTH);
  // need to be frame advancing henceforth
//!  if ( flavor!=ENTITY_ACTOR && flavor!=ENTITY_CHARACTER &&
//!  if (flavor!=ENTITY_LIMB_BODY)
    set_flag( EFLAG_ACTIVE, true );
}

void entity::activate_motion_blur(int _blur_min_alpha,
                                  int _blur_max_alpha,
                                  int _num_blur_images,
                                  float _blur_spread)
{
  set_flag(EFLAG_GRAPHICS_MOTION_BLUR, true);
  if ( is_motion_blurred() )
  {
    assert( mbi );
    // note that we have no initial motion blurred copies
    mbi->motion_trail_start = 0;
    mbi->motion_trail_end   = 0;
    mbi->motion_trail_count = 0;
    mbi->blur_min_alpha = _blur_min_alpha;
    mbi->blur_max_alpha = _blur_max_alpha;

/*!    if(g_world_ptr->get_num_active_characters() > 2)
      mbi->num_blur_images = (int) ( (rational_t)_num_blur_images / (rational_t)(g_world_ptr->get_num_active_characters() - 1) + 0.5f );
    else
!*/
      mbi->num_blur_images = _num_blur_images;

    mbi->blur_spread = _blur_spread;
  }
}

void entity::deactivate_motion_blur()
{
  set_flag(EFLAG_GRAPHICS_MOTION_BLUR,false);
}

#ifdef ECULL
sound_emitter* entity::get_emitter()
{
/*
  if ( !emitter )
  {
    // dynamic allocation allowed even on static entities because we make a
    // special effort to save these pointers as necessary in save_dynamic_heap();
    // see dynamic save/restore support section, below, and app.cpp
    emitter = sound_device::inst()->create_emitter();
    // need to be frame advancing henceforth
//!    if ( flavor!=ENTITY_ACTOR && flavor!=ENTITY_CHARACTER && flavor!=ENTITY_LIMB_BODY)
      set_flag( EFLAG_ACTIVE, true );
    get_emitter()->set_position( get_abs_position() );
  }
  return emitter;
*/

  if(!has_sound_ifc())
    create_sound_ifc();

  return(sound_ifc()->get_emitter());
}
#endif



void entity::set_frame_delta(po const & bob, time_value_t t)
{
//  bool locked;
  if(t > 0.0f)
  {
    if ( !mi )
    {
      // CTT 07/22/00: for the Max Steel project, the NONSTATIC flag only matters
      // for walkable entities
  //    #ifdef PROJECT_STEEL
      // guard against dynamic allocation in static entity
      if ( is_walkable() && !is_flagged(EFLAG_MISC_NONSTATIC) )
      {
        error( "Tried to manipulate static walkable object, ID: " + id.get_val() );
      }

      mi = NEW movement_info;
      invalidate_frame_delta();
    }

    if(mi->frame_delta_valid)
    {
      fast_po_mul(mi->frame_delta, mi->frame_delta, bob);
  //    mi->frame_delta = mi->frame_delta*bob;
    }
    else
      mi->frame_delta = bob;

    mi->frame_time = t;
    mi->frame_delta_valid = true;
  }

  assert(!mi || !mi->frame_delta_valid || mi->frame_time > 0.0f);

//  assert(!mi || !mi->frame_delta_valid || mi->frame_delta.get_position().length() < 50.0f);
}

void entity::set_frame_delta_trans(const vector3d &bob, time_value_t t)
{
  if(t > 0.0f)
  {
    if ( !mi )
    {
      // CTT 07/22/00: for the Max Steel project, the NONSTATIC flag only matters
      // for walkable entities
  //    #ifdef PROJECT_STEEL
      // guard against dynamic allocation in static entity
      if ( is_walkable() && !is_flagged(EFLAG_MISC_NONSTATIC) )
      {
        error( "Tried to manipulate static walkable object, ID: " + id.get_val() );
      }

      mi = NEW movement_info;
      invalidate_frame_delta();
    }

    if(mi->frame_delta_valid)
    {
      static po trans;
      trans.set_translate(bob);
      fast_po_mul(mi->frame_delta, mi->frame_delta, trans);
  //    mi->frame_delta = mi->frame_delta*trans;
    }
    else
    {
      mi->frame_delta.set_position(bob);
    }

    mi->frame_time = t;
    mi->frame_delta_valid = true;
  }

  assert(!mi || !mi->frame_delta_valid || mi->frame_time > 0.0f);

//  assert(!mi || !mi->frame_delta_valid || mi->frame_delta.get_position().length() < 50.0f);
}


// turn on or off the motion trail effect
void entity::activate_motion_trail( int _trail_length,
                                    color32 _trail_color,
                                    int _trail_min_alpha,
                                    int _trail_max_alpha,
                                    const vector3d& tip
                                    )
{
  set_flag(EFLAG_GRAPHICS_MOTION_TRAIL, true);
  assert( mbi );
  mbi->motion_trail_length = _trail_length;
  assert( mbi->motion_trail_length <= mbi->buffer_size );
  mbi->trail_color = _trail_color;
  mbi->trail_min_alpha = _trail_min_alpha;
  mbi->trail_max_alpha = _trail_max_alpha;
  // note that we have no initial motion trail copies
  mbi->motion_trail_start = 0;
  mbi->motion_trail_end   = 0;
  mbi->motion_trail_count = 0;
  mbi->motion_trail_head = tip;
}

void entity::deactivate_motion_trail()
{
  set_flag(EFLAG_GRAPHICS_MOTION_TRAIL,false);
}


////////////////////////////////////////////////////////////////////////////////
// entity:  Physical Representation Methods
////////////////////////////////////////////////////////////////////////////////


// For use during setup, because updating the colgeom is bad before the game is
// running.  Or if you know the colgeom is updated (a *little* faster).
void entity::get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const
{
  get_colgeom()->get_closest_point_along_dir( target, axis );
}

// For use once game is active.
vector3d entity::get_updated_closest_point_along_dir( const vector3d& axis )
{
  vector3d cpad;
  get_updated_colgeom()->get_closest_point_along_dir( &cpad, axis );
  return cpad;
}


void entity::invalidate_colgeom()
{
  if (get_colgeom())
  {
    get_colgeom()->invalidate();
  }
}

time_value_t entity::get_age() const
{
//  return age;
  return frame_time_info.get_age();
}

void entity::set_age(time_value_t new_age)
{
  frame_time_info.set_age(new_age);
}

void entity::rebirth()
{
//  age = 0;
  frame_time_info.set_age(0.0f);
}


////////////////////////////////////////////////////////////////////////////////
// Render optimization support
// /////////////////////////////////////////////////////////////////////////////


render_flavor_t entity::render_passes_needed() const
{
  if ( !my_visrep )
    return 0;

  render_flavor_t passes=my_visrep->render_passes_needed();
  if ( my_visrep->get_type() == VISREP_PMESH )
  {
    if ( render_color.get_alpha() < 0xFF )
      passes = RENDER_TRANSLUCENT_PORTION;
    else if ( is_motion_blurred() || is_motion_trailed() )
      passes |= RENDER_TRANSLUCENT_PORTION;
  }
  return passes;
}

////////////////////////////////////////////////////////////////////////////////
//   dummy light stubs
////////////////////////////////////////////////////////////////////////////////

color evil_color(0,0,0,0);

const color& entity::get_color() const
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
  return evil_color;
}

void   entity::set_color(const color&)
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
}

const color& entity::get_additive_color() const
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
  return evil_color;
}

void   entity::set_additive_color(const color&)
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
}

rational_t entity::get_near_range() const
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
  return 0;
}

void       entity::set_near_range(rational_t)
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
}

rational_t entity::get_cutoff_range() const
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
  return 0;
}

void       entity::set_cutoff_range(rational_t)
{
  stringx composite = id.get_val() + " is not a light source.";
  error(composite.c_str());
}


/////////////////////////////////////////////////////////////////////////////
// Animation interface
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////
// hierarchical animation interface

// maximum number of applied hierachical animations
// const int MAX_ANIM_SLOTS = 4;

// this function loads the named hierarchical animation data (must be performed
// before the animation can be played)
void entity::load_anim( const stringx& filename ) const
{
  if ( has_dirname() )
  {
    stringx local = get_dirname() + filename;
    if ( os_file::file_exists( local ) )
    {
      g_world_ptr->get_ett_manager()->load( local );
      return;
    }
  }
  g_world_ptr->get_ett_manager()->load( filename );
}

void entity::unload_anim( const stringx& filename ) const
{
  g_world_ptr->get_ett_manager()->unload( filename );
}
// play a hierarchical animation in the given slot
entity_anim_tree* entity::play_anim( int slot,
                                     const stringx& filename,
                                     time_value_t start_time,
                                     unsigned short anim_flags,
                                     short loop )
{
  assert( slot>=0 && slot<MAX_ANIM_SLOTS );
  entity_track_tree* track = NULL;
    // look for matching filename in entity's own data path
  track = g_world_ptr->get_ett_manager()->acquire( filename );

  if ( track == NULL )
  {
    nglPrintf( "%s: play_anim('%s'): must load anim before playing", get_name().c_str(), filename.c_str() );
	return NULL;
  }

  return play_anim( slot, filename, *track, start_time, anim_flags, loop );
}

// play a hierarchical animation in the given slot
entity_anim_tree* entity::play_anim( int slot,
                                     const stringx& filenamea,
                                     const stringx& filenameb,
                                     rational_t blenda,
                                     rational_t blendb,
                                     time_value_t start_time,
                                     unsigned short anim_flags,
                                     short loop )
{
  assert( slot>=0 && slot<MAX_ANIM_SLOTS );
  entity_track_tree* tracka = NULL;
  entity_track_tree* trackb = NULL;

  tracka = g_world_ptr->get_ett_manager()->acquire( filenamea );
  trackb = g_world_ptr->get_ett_manager()->acquire( filenameb );

  if ( tracka == NULL)
    error( "%s: play_anim('%s'): must load anim before playing", get_name().c_str(), filenamea.c_str() );
  if ( trackb == NULL)
    error( "%s: play_anim('%s'): must load anim before playing", get_name().c_str(), filenameb.c_str() );

  stringx name = stringx("(") + filenamea + "+" + filenameb + stringx(")");
  return play_anim( slot, name, *tracka, *trackb, blenda, blendb, start_time, anim_flags, loop );
}

// play a hierarchical animation in slot 0
entity_anim_tree* entity::play_anim( const stringx& filename,
                                     time_value_t start_time,
                                     unsigned short anim_flags,
                                     short loop )
{
  return play_anim( 0, filename, start_time, anim_flags, loop );
}

entity_anim_tree* entity::play_loop_anim( const stringx& filename, unsigned short anim_flags, short loop )
{
  return play_anim( filename, -0.25f, anim_flags, loop );
}


entity_anim_tree* entity::play_anim( int slot,
                                     const stringx& _name,
                                     const entity_track_tree& track,
                                     time_value_t start_time,
                                     unsigned short anim_flags,
                                     short loop )
{
//  assert(slot==ANIM_PRIMARY);

  START_PROF_TIMER( proftimer_play_anim1 );
	make_animateable(true);
  //if ( anim_trees == NULL )
  //{
  //  // this is the first time a hierarchical animation has been requested for
  //  // this entity, so we must construct the hierarchical animation list
  //  anim_trees = NEW entity_anim_tree*[MAX_ANIM_SLOTS];
  //  memset( anim_trees, 0, sizeof(entity_anim_tree*)*MAX_ANIM_SLOTS );
  //}

  entity_anim_tree* prev = anim_trees[slot];
  if ( prev )
  {
  	START_PROF_TIMER( proftimer_play_anim1old );
    if ( !prev->is_attached()
      || prev->get_name() != _name
      || prev->is_finished() )
    {
      if ( prev->is_attached() )
      {
        if ( owning_widget )
        {
          owning_widget->kill_anim( prev );
        }
        else
        {
          g_world_ptr->kill_anim( prev );
        }
      }
      // create NEW anim with cached anim tree in this slot
      g_anim_maker->create_anim( anim_trees[slot], _name, track, anim_flags, start_time, slot, loop, owning_widget );
    }
    else
    {
      // apply any flags that are appropriate
      prev->set_flag( ANIM_LOOPING, (anim_flags&ANIM_LOOPING)!=0 );
      prev->set_flag( ANIM_AUTOKILL, (anim_flags&ANIM_AUTOKILL)!=0 );
      if ( (prev->get_flags()&ANIM_REVERSE) != (anim_flags&ANIM_REVERSE) )
      {
        // change reverse play status
        prev->set_flag( ANIM_REVERSE, (anim_flags&ANIM_REVERSE)!=0 );
        // reset keys and root start position values
        prev->set_time( prev->get_time() );
        prev->reset_root_position();
      }
    }
  	STOP_PROF_TIMER( proftimer_play_anim1old );
  }
  else
  {
  	START_PROF_TIMER( proftimer_play_anim1new );
    // construct a NEW anim tree for this slot
    anim_trees[slot] = g_anim_maker->create_anim( _name, track, this, anim_flags, start_time, slot, loop, owning_widget );
  	STOP_PROF_TIMER( proftimer_play_anim1new );
  }

  STOP_PROF_TIMER( proftimer_play_anim1 );
  return anim_trees[slot];
}


void entity::make_animateable( bool onOff )
{
	if ( onOff && anim_trees==NULL )
	{
    // this is the first time a hierarchical animation has been requested for
    // this entity, so we must construct the hierarchical animation list
//    anim_trees = NEW entity_anim_tree*[MAX_ANIM_SLOTS];
    memset( anim_trees, 0, sizeof(entity_anim_tree*)*MAX_ANIM_SLOTS );
	}
  /*
	else
	if ( !onOff && anim_trees!=NULL )
	{
		delete [] anim_trees;
	}
  */
}

entity_anim_tree* entity::play_anim( int slot,
                                     const stringx& _name,
                                     const entity_track_tree& tracka,
                                     const entity_track_tree& trackb,
                                     rational_t blenda,
                                     rational_t blendb,
                                     time_value_t start_time,
                                     unsigned short anim_flags,
                                     short loop )
{
  assert(slot==ANIM_PRIMARY);
  START_PROF_TIMER( proftimer_play_anim2 );
	make_animateable(true);
  //if ( anim_trees == NULL )
  //{
  //  // this is the first time a hierarchical animation has been requested for
  //  // this entity, so we must construct the hierarchical animation list
  //  anim_trees = NEW entity_anim_tree*[MAX_ANIM_SLOTS];
  //  memset( anim_trees, 0, sizeof(entity_anim_tree*)*MAX_ANIM_SLOTS );
  //}

  entity_anim_tree* prev = anim_trees[slot];
  if ( prev )
  {
    if ( !prev->is_attached()
      || prev->get_name() != _name
      || prev->is_finished() )
    {
      if ( prev->is_attached() )
      {
        if ( owning_widget )
        {
          owning_widget->kill_anim( prev );
        }
        else
        {
          g_world_ptr->kill_anim( prev );
        }
      }
      // create NEW anim with cached anim tree in this slot
      g_anim_maker->create_anim( anim_trees[slot], _name, tracka, trackb, blenda, blendb, anim_flags, start_time, slot, loop, owning_widget );
    }
    else
    {
      // apply any flags that are appropriate
      prev->set_flag( ANIM_LOOPING, (anim_flags&ANIM_LOOPING)!=0 );
      prev->set_flag( ANIM_AUTOKILL, (anim_flags&ANIM_AUTOKILL)!=0 );
      if ( (prev->get_flags()&ANIM_REVERSE) != (anim_flags&ANIM_REVERSE) )
      {
        // change reverse play status
        prev->set_flag( ANIM_REVERSE, (anim_flags&ANIM_REVERSE)!=0 );
        // reset keys and root start position values
        prev->set_time( prev->get_time() );
        prev->reset_root_position();
      }
    }
  }
  else
  {
    // construct a NEW anim tree for this slot
    anim_trees[slot] = g_anim_maker->create_anim( _name, tracka, trackb, blenda, blendb, this, anim_flags, start_time, slot, loop, owning_widget );
  }

  STOP_PROF_TIMER( proftimer_play_anim2 );
  return anim_trees[slot];
}


entity_anim_tree* entity::get_anim_tree( int slot ) const
{
  //assert(slot==ANIM_PRIMARY);
  assert( slot>=0 && slot<MAX_ANIM_SLOTS );
//  if ( anim_trees == NULL )
//    return NULL;
//  else
	if ( anim_trees )
  {
    entity_anim_tree* a = anim_trees[slot];
    if ( a && a->is_attached() )
      return a;
    else
      return NULL;
  }
	else
	  return NULL;
}


void entity::kill_anim( int slot )
{
  //assert(slot==ANIM_PRIMARY);
  assert( slot>=0 && slot<MAX_ANIM_SLOTS );
//  if ( anim_trees )
  {
    // wds::kill_anim() destroys the animation; the animation destructor in turn
    // calls entity::clear_anim(), so to do so here would be redundant
    entity_anim_tree* a = get_anim_tree( slot );
    if ( a )
      g_world_ptr->kill_anim( a );
  }
}

bool entity::anim_finished( int slot ) const
{
 // assert(slot==ANIM_PRIMARY);
  entity_anim_tree* a = get_anim_tree( slot );
  return ( a==NULL || a->is_finished() );
}

// This function should ONLY be called when the world kills an entity_anim_tree.
void entity::clear_anim( entity_anim_tree* a )
{
//  assert( anim_trees != NULL );
  int slot;
  for ( slot=0; slot<MAX_ANIM_SLOTS; ++slot )
  {
    entity_anim_tree* local = get_anim_tree( slot );
    if ( a == local )
    {
      // deconstruct and detach this animation (effectively caches the memory used for any later anim in the same slot)
      local->deconstruct();
      local->detach();
      // now re-attach all lower-priority animations
      int i;
      for ( i=0; i<slot; i++ )
      {
        entity_anim_tree* a = get_anim_tree( i );
        if ( a && a->is_valid() )
          a->attach();
      }
      return;
    }
  }
  assert( 0 );  // this should never happen
}

// this function causes the internal animation nodes of each attached hierarchical
// animation to be destroyed (made necessary by the actor limb_tree_pool system)
void entity::deconstruct_anim_trees()
{
  if ( has_anim_trees() )
  {
    int i;
    for ( i=0; i<MAX_ANIM_SLOTS; i++ )
    {
      entity_anim_tree* a = get_anim_tree( i );
      if ( a )
        a->deconstruct();
    }
  }
}

// this function reattaches all hierarchical animations
void entity::reconstruct_anim_trees()
{
  if ( has_anim_trees() )
  {
    int i;
    for ( i=0; i<MAX_ANIM_SLOTS; i++ )
    {
      entity_anim_tree* a = get_anim_tree( i );
      if ( a )
      {
        a->reconstruct( i );
      }
    }
  }
}


///////////////////////////////////////
// entity_anim interface

// attach given animation
// NOTE: this causes the removal of any previously attached animation

bool entity::attach_anim( entity_anim* new_anim )
{
  assert( new_anim );
  if ( current_anim==NULL || new_anim->get_priority()>=current_anim->get_priority() )
  {
    if ( current_anim && new_anim!=current_anim )
    {
      // another animation is currently attached;
      // detach it
      current_anim->detach();
    }
    current_anim = new_anim;
    return true;
  }
  else
    return false;
}

// detach given animation if it matches current_anim; returns true if successful
void entity::detach_anim()
{
  current_anim = NULL;
}


////////////////////////////////////////////////////////////////////////////////
//  bounding box
////////////////////////////////////////////////////////////////////////////////

void entity::compute_bounding_box()
{
  if ( get_colgeom() && get_colgeom()->get_type()==collision_geometry::MESH )
  {
    if ( !has_bounding_box() )
      bbi = NEW bounding_box;
    cg_mesh* m = static_cast<cg_mesh*>( get_colgeom() );
    int i;
    for ( i=0; i<m->get_num_verts(); ++i )
    {
//      vector3d v = get_abs_po().slow_xform( m->get_vert_ptr(i)->get_point() );
      vector3d v = get_abs_po().fast_8byte_xform( m->get_vert_ptr(i)->get_point() );

      if ( v.x < bbi->vmin.x )
        bbi->vmin.x = v.x;
      if ( v.x > bbi->vmax.x )
        bbi->vmax.x = v.x;
      if ( v.y < bbi->vmin.y )
        bbi->vmin.y = v.y;
      if ( v.y > bbi->vmax.y )
        bbi->vmax.y = v.y;
      if ( v.z < bbi->vmin.z )
        bbi->vmin.z = v.z;
      if ( v.z > bbi->vmax.z )
        bbi->vmax.z = v.z;
    }
  }
}


void entity::check_nonstatic()
{
  // CTT 07/22/00: for the Max Steel project, the NONSTATIC flag only matters
  // for walkable entities
  #ifndef BUILD_BOOTABLE
//  #ifdef PROJECT_STEEL
  if ( is_walkable() && !is_flagged(EFLAG_MISC_NONSTATIC) )
  {
    warning( get_id().get_val() + ": walkable entity is static; change to NONSTATIC if you wish to modify this entity at run-time." );
//P    add_onscreen_error(get_id().get_val() + " is static and walkable, change to NONSTATIC");
    set_flag( EFLAG_MISC_NONSTATIC, true );  // do this to avoid further warnings
  }
/*
  #else
  if ( !is_flagged( EFLAG_MISC_NONSTATIC ) )
  {
    warning( get_id().get_val() + ": entity is static; change to NONSTATIC if you wish to modify this entity at run-time." );
//P    add_onscreen_error(get_id().get_val() + " is static, change to NONSTATIC");
    set_flag( EFLAG_MISC_NONSTATIC, true );  // do this to avoid further warnings
  }
  #endif
*/
  #endif // !BUILD_BOOTABLE
}


int g_iflrand_counter=0;
int entity::get_random_ifl_frame_boost() const
{
  g_iflrand_counter++;
  return random_ifl_frame_boost_table[0xff&(get_id().get_numerical_val()*3)];
}

void init_random_ifl_frame_boost_table()
{
  for (int i=0;i<64;++i) // notice that this doesn't match the table size.  Dunno why not.  --Sean
  {
    random_ifl_frame_boost_table[i] = g_random_ptr->rand() & 63;
  }
}


region_node * entity::get_primary_region() const
{
#ifndef REGIONCULL
  if (flags&EFLAG_REGION_FORCED)
    return (in_regions.empty() ? NULL : *in_regions.begin());
  else
#endif
    return center_region;
}

// maintains parented entities region info
region_node * entity::update_region(bool parent_computed)
{
#ifndef REGIONCULL
  set_needs_compute_sector(false);

  if (flags&EFLAG_REGION_FORCED)
  {
    return (in_regions.empty() ? NULL : *in_regions.begin());
  }
  if (has_parent())// /*! pi && pi->parent !*/ && flags&EFLAG_MISC_NONSTATIC)
  {
    region_node * parents_region;
/*!    if (get_flavor()==ENTITY_LIMB_BODY)
      parents_region = ((limb_body *) this)->get_my_limb()->get_my_actor()->update_region();
    else
!*/
    parents_region = parent_computed ? ((entity *)link_ifc()->get_parent())->get_primary_region() : ((entity *)link_ifc()->get_parent())->update_region();
    if (get_primary_region()!=parents_region /*! && get_flavor()!=ENTITY_LIMB_BODY !*/)
      compute_sector(g_world_ptr->get_the_terrain());
  }
#endif
  return get_primary_region();
}


////////////////////////////////////////////////////////////////////////////////
//  entity_manager
////////////////////////////////////////////////////////////////////////////////

entity_manager::entity_manager() : number_server( 0 )
{
#if defined(TARGET_XBOX)
  assert( !strcmp(entity_flavor_names[NUM_ENTITY_FLAVORS], "null") );
#else
  assert( entity_flavor_names[NUM_ENTITY_FLAVORS]=="null" );
#endif /* TARGET_XBOX JIV DEBUG */

  name_to_number.clear();
  number_server = 0;
  name_to_number.insert( name_to_number_map::value_type( strdupcpp("UNREG"), -1) );
}

entity_manager::~entity_manager()
{
  name_to_number_map::iterator mi;
  mi = name_to_number.begin();
  for(; mi != name_to_number.end(); ++mi )
  {
    delete[] (*mi).first.str;
  }
  erase( begin(), end() );
  name_to_number = name_to_number_map();
}

entity* entity_manager::find_entity(const entity_id& target_entity,
                                    entity_flavor_t flavor, bool unknown_ok)
{
  entity * outval;

  iterator it = find( target_entity );
  if( ( unknown_ok != FIND_ENTITY_UNKNOWN_OK ) && (it==end() ) )
  {
    stringx composite = stringx("Unable to find entity ") + target_entity.get_val();
    error( composite.c_str() );
  }

  if (it!= end())
  {
    if (flavor!=IGNORE_FLAVOR)
    {
      if((*it).second->get_flavor() != flavor )
      {
        stringx composite = stringx("Entity ") + target_entity.get_val() + " is not a " + entity_flavor_names[flavor];
        error( composite.c_str() );
      }
    }
    outval = (*it).second;
  }
  else
    outval = NULL;
  return outval;
}

#ifdef SPIDEY_SIM
entity* entity_manager::find_nearest_entity_to_line(const vector3d& pos1, const vector3d& pos2, entity_flavor_t flavor )
{
  entity * outval;

  iterator it = begin();
  float best = 0;
  iterator winner = end();
  vector3d unit_ray = pos2 - pos1;
  unit_ray.normalize();
  for( ; it != end(); it++ )
  {
    if( (*it).second->get_flavor() == flavor )
    {
      vector3d entpos = (*it).second->get_abs_position();
      // is in front of spidey?
      float dotty = dot( (entpos-pos1), unit_ray );
      if( dotty > 0 )
      {
        // project entpos onto ray
        vector3d projection = pos1 + dotty * unit_ray;
        // find distance to line
        vector3d ent_to_proj = entpos - projection;
        float distance = ent_to_proj.length();
        // find distance to spidey
        vector3d proj_to_pos1 = projection - pos1;
        float distance2 = proj_to_pos1.length();
        // disqualify anything ridiculous
        if( distance2<50 )  // this will actually vary from level to level
        {
          // quality is length over width
          float quality = distance2 / distance;
          if( quality > best )
          {
            best = quality;
            winner = it;
          }
        }
      }
    }
  }
  if( winner != end() )
    return (*winner).second;
  else
    return NULL;
}

#endif /* SPIDEY_SIM */
void entity_manager::purge()
{
  name_to_number_map::iterator mi;
  mi = name_to_number.begin();
  for(; mi != name_to_number.end(); ++mi )
  {
    delete[] (*mi).first.str;
  }
  erase( begin(), end() );
  name_to_number = name_to_number_map();
  number_server = 0;
  // *weird:  this causes allocations at the end of a heap we're going to delete most of
//  pair<name_to_number_map::iterator,bool> result =  // unused, remove me?
	//nglPrintf("Weird 6 byte leak\n");
  //name_to_number.insert( name_to_number_map::value_type( strdupcpp("UNREG"), -1) );
}


void entity_manager::stl_prealloc(void)
{
  entity_manager::create_inst();
	static entity_id teid=entity_id("STL_INIT");
	static entity tentity=entity(teid,0);
	static entity *e=&tentity;

	pentity_vector evect;
	for ( int i=0; i<1024; i++ )
	{
		evect.push_back( e );
	}
	evect.resize(0);

}


void entity_manager::register_entity(entity* e)
{
  if( !(e->get_id() == ANONYMOUS) )
  {
    pair<iterator,bool> retval = insert( value_type(e->get_id(), e) );
    if(!retval.second)
    {
      stringx composite = stringx("Same entity name appears twice: ")+ e->get_id().get_val();
      error( composite.c_str() );
    }
  }
}

void entity_manager::deregister_entity(entity* e)
{
  if( !(e->get_id() == ANONYMOUS) )
  {
    iterator it = find( e->get_id() );
    entity_id::delete_entity_id(e->get_id());
    erase(it);
  }
}

////////////////////////////////////////////////////////////////////////////////
//  motion_blur stuff
motion_blur_info::motion_blur_info( int max_trail_length )
  : trail_min_alpha(0),
    trail_max_alpha(0),
    blur_min_alpha(0),
    blur_max_alpha(0),
    num_blur_images(0),
    blur_spread(0.0f)
{
  motion_trail_start = 0;
  motion_trail_end = 0;
  motion_trail_count = 0;
  motion_trail_length = 0;
  motion_trail_buffer = NEW qt[max_trail_length];
  buffer_size = max_trail_length;
  motion_trail_head = vector3d(0.0f, 0.0f, 1.1f);
  motion_trail_tail = vector3d(0.0f, 0.0f, 0.0f);
}

motion_blur_info::~motion_blur_info()
{ delete[] motion_trail_buffer; }



destroyable_info* destroyable_info::make_instance(entity *ent)
{
  destroyable_info* info = NEW destroyable_info(ent);
  info->copy_instance_data(this);
  return info;
}

void destroyable_info::copy_instance_data(destroyable_info* data)
{
  flags = data->flags;
  destroy_lifetime = data->destroy_lifetime;
#ifdef ECULL
  destroy_sound = data->destroy_sound;
#endif
  destroy_fx = data->destroy_fx;
  destroy_script = data->destroy_script;
  preload_script = data->preload_script;
  destroyed_visrep = data->destroyed_visrep;

  hit_points = data->hit_points;

//  dread_net_cue = data->dread_net_cue;

  if(data->destroyed_mesh)
    destroyed_mesh = new_visrep_instance(data->destroyed_mesh);
  else
    destroyed_mesh = NULL;
}

destroyable_info::destroyable_info(entity *ent)
{
  flags = 0;
  destroy_lifetime = 1.0f;
#ifdef ECULL
  destroy_sound = empty_string;
#endif
  destroy_fx = empty_string;
  destroy_script = empty_string;
  preload_script = empty_string;
  destroyed_visrep = empty_string;

  destroyed_mesh = NULL;

  hit_points = 0;

//  dread_net_cue = dread_net::UNDEFINED_AV_CUE;

  owner = ent;
  assert(owner);
}

destroyable_info::~destroyable_info()
{
  if(destroyed_mesh)
  {
    unload_visual_rep(destroyed_mesh);
    destroyed_mesh = NULL;
  }
}

void destroyable_info::read_enx_data( chunk_file& fs, stringx& lstr )
{
//  assert(lstr == "destroy_info:");

  // defaults (if from entity enx, or brand NEW read)
  owner->set_ext_flag(EFLAG_EXT_TARGETABLE, false );
  owner->set_target_type(TARGET_TYPE_BIO);

  serial_in( fs, &lstr );

  while(!(lstr == chunkend_label))
  {
    if ( lstr=="auto_aim" || lstr=="AUTO_AIM" )
    {
      owner->set_ext_flag(EFLAG_EXT_TARGETABLE, true );
    }
    else if ( lstr=="target_type" || lstr=="TARGET_TYPE" )
    {
      serial_in( fs, &lstr );

      if(lstr == "bio" || lstr == "BIO")
        owner->set_target_type(TARGET_TYPE_BIO);
      else if(lstr == "mechanical" || lstr == "MECHANICAL" || lstr == "mech" || lstr == "MECH")
        owner->set_target_type(TARGET_TYPE_MECHANICAL);
      else
        error(fs.get_filename() + ": Unknown target_type '" + lstr + "' found");
    }
    else if(lstr == "hit_points")
    {
      set_flag(_HIT_POINTS, true);
      serial_in( fs, &hit_points );
    }
    else if(lstr == "effect")
    {
      set_flag(_DESTROY_FX, true);
      serial_in( fs, &destroy_fx );
    }
    else if(lstr == "lifetime")
    {
      serial_in( fs, &destroy_lifetime );
    }
    else if(lstr == "script")
    {
      set_flag(_DESTROY_SCRIPT, true);
      serial_in( fs, &destroy_script );
    }
    else if(lstr == "preload_script")
    {
      set_flag(_PRELOAD_SCRIPT, true);
      set_flag(_PRELOAD_SCRIPT_RAN, false);
      serial_in( fs, &preload_script );
    }
    else if(lstr == "remain_active")
    {
      set_flag(_REMAIN_ACTIVE, true);
    }
    else if(lstr == "remain_visible")
    {
      set_flag(_REMAIN_VISIBLE, true);
    }
    else if(lstr == "no_collision")
    {
      set_flag(_NO_COLLISION, true);
    }
    else if(lstr == "remain_collision")
    {
      set_flag(_REMAIN_COLLISION, true);
    }
    else if(lstr == "single_blow")
    {
      set_flag(_SINGLE_BLOW, true);
    }
/*
    else if(lstr == "dread_net_cue")
    {
      stringx cue;
      serial_in(fs, &cue);
      cue.to_upper();
      dread_net_cue = dread_net::get_cue_type(cue);
    }
*/
    else if(lstr == "sound")
    {
	    #ifdef ECULL
      set_flag(_DESTROY_SOUND, true);
      serial_in( fs, &destroy_sound );
      #endif

//      sound_device::inst()->load_sound( destroy_sound );
    }
    else if(lstr == "visrep")
    {
      set_flag(_DESTROYED_VISREP, true);
      serial_in( fs, &destroyed_visrep );

      if(destroyed_mesh != NULL)
      {
        unload_visual_rep(destroyed_mesh);
        destroyed_mesh = NULL;
      }

      destroyed_mesh = load_new_visual_rep( destroyed_visrep, vr_pmesh::ALLOW_SELF_LIT_MATERIALS  );

      if(destroyed_mesh && destroyed_mesh->get_type()==VISREP_PMESH)
        (static_cast<vr_pmesh*>(destroyed_mesh))->shrink_memory_footprint();
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + lstr + "' in destroy_info section" );
    }

    serial_in( fs, &lstr );
  }

  if ( is_flagged(_DESTROY_FX) )
    g_world_ptr->create_preloaded_entity_or_subclass( destroy_fx.c_str(),  empty_string );
}

void destroyable_info::reset()
{
  // reset the destroy info structure
  set_hit_points(0);
  set_has_hit_points(false);
  set_destroy_lifetime(1.0f);
  set_has_destroy_fx(false);
  set_has_destroy_script(false);
  set_has_preload_script(false);
  set_has_preload_script_run(false);
#ifdef ECULL
  set_has_destroy_sound(false);
#endif
  set_has_destroyed_visrep(false);
  set_single_blow(false);
  set_remain_visible(false);
  set_remain_active(false);
  set_no_collision(false);
  set_remain_collision(false);
//  dread_net_cue = dread_net::UNDEFINED_AV_CUE;
}


int destroyable_info::apply_damage(int damage, const vector3d &pos, const vector3d &norm)
{
  if ( has_hit_points() )
  {
    if ( !is_single_blow() || damage>=hit_points )
      hit_points -= damage;
    if ( hit_points <= 0 )
      hit_points = 0;
    return hit_points;
  }
  else
    return 1;
}

void destroyable_info::apply_destruction_fx()
{
  assert(owner != NULL);

//  if(dread_net_cue != dread_net::UNDEFINED_AV_CUE)
//    g_world_ptr->get_dread_net()->add_cue((dread_net::eAVCueType)dread_net_cue, owner);

  if ( has_destroy_fx() )
  {
    po the_po = po_identity_matrix;
    the_po.set_position(owner->get_abs_position());
//    entity *fx =  // unused, remove me?
    g_world_ptr->add_time_limited_effect( destroy_fx.c_str(), the_po, destroy_lifetime );
  }

  if ( has_destroy_script() )
  {
    owner->spawn_entity_script_function( get_destroy_script() );
  }

  if ( has_destroyed_visrep() )
  {
    owner->change_visrep(destroyed_visrep);
    unload_visual_rep(destroyed_mesh);
    destroyed_mesh = NULL;
  }
  else if ( !remain_visible() )
  {
    owner->set_visible( false );

    if( !remain_collision() )
    {
      owner->set_collisions_active( false );
      owner->set_walkable( false );
      owner->set_repulsion( false );
    }
  }

  if( no_collision() )
  {
    owner->set_collisions_active( false );
    owner->set_walkable( false );
    owner->set_repulsion( false );
  }

  if ( !remain_active() )
  {
    owner->set_active( false );
    owner->set_actionable( false );
  }

#ifdef ECULL
  if ( has_destroy_sound() )
  {
    if ( owner->get_emitter() )
      owner->get_emitter()->play_sound( destroy_sound );
    else
    {
      assert(0);
 //     sound_device::inst()->play_sound( destroy_sound );
    }
  }
#endif
}




void destroyable_info::preload()
{
  if(!has_preload_script_run())
  {
    set_has_preload_script_run(true);
    entity::exec_preload_function(get_preload_script());
  }
}


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


void entity::apply_damage(int damage, const vector3d &pos, const vector3d &norm, int _damage_type, entity *attacker, int dmg_flags)
{
  assert(0);
/*
  if ( is_destroyable() )
  {
    if(attacker)
    {
      if(attacker->is_a_handheld_item())
      {
        dmg_info.attacker = ((handheld_item *)attacker)->get_owner();
        dmg_info.attacker_itm = (item *)attacker;
      }
      else
      {
        dmg_info.attacker = attacker;
        dmg_info.attacker_itm = NULL;
      }
    }
    else
    {
      dmg_info.attacker = NULL;
      dmg_info.attacker_itm = NULL;
    }

    // make sure members of a conglomerate cannot hurt itself... (for tanks and turrets)
    if(is_a_conglomerate() && ((conglomerate *)this)->has_member(dmg_info.attacker))
      return;

    dmg_info.damage = damage;
    dmg_info.dir = -norm;
    dmg_info.loc = pos;
    dmg_info.type = _damage_type;
    dmg_info.push_wounded = true;
    dmg_info.push_death = true;
    dmg_info.flags = dmg_flags;

    raise_signal( DAMAGED );

    damage = (int)((((rational_t)damage) * damage_resist_modifier) + 0.5f);

    #ifdef TARGET_PC
      char    outbuf[100];
      sprintf( outbuf, "%s takes %d damage", id.get_val().c_str(), damage );
      app::inst()->get_game()->get_message_board()->post( stringx(outbuf), 2.0F );
    #endif

    if ( !is_invulnerable() && !destroy_info->apply_damage(damage, pos, norm) )
    {
      set_auto_targetable(false);
      apply_destruction_fx();
      target_timer = 0.0f;
      raise_signal( DESTROYED );
    }
  }
*/
}


void entity::apply_destruction_fx()
{
  if ( destroy_info )
  {
    destroy_info->apply_destruction_fx();
    // disgorge any items I may be carrying
    disgorge_items();
  }
  else
    set_active( false );
}

bool entity::is_destroyable() const
{
  return destroy_info != NULL && (!destroy_info->has_hit_points() || destroy_info->get_hit_points()>0);
}

// add an item to this container;
// returns true if no like item already in list; otherwise, adds to existing item's count
bool entity::add_item( item* it )
{
  if ( !is_container() )
    coninfo = NEW container_info;

  item* lit = it->is_brain_weapon() ? NULL : find_like_item( it );
  if ( lit )
  {
    lit->set_count( lit->get_number() + it->get_number() );

    #if _ENABLE_WORLD_EDITOR
      lit->set_original_count(lit->get_original_count() + it->get_number());
    #endif

    it->set_count( 0 );
    return false;
  }
  else
  {
    coninfo->items.push_back( it );
    it->set_visible( false );
//!    it->set_parent( this );

    #if _ENABLE_WORLD_EDITOR
      it->set_original_count(it->get_number());
    #endif


#if 0 // BIGCULL
      if(it->is_a_handheld_item())
    {
      handheld_item *h_item = (handheld_item *)it;

      if(h_item->get_owner() == NULL)
        h_item->set_owner(this);
    }
#endif // BIGCULL

    // automatically spawn and run the special X_callbacks(item) script function
    if ( g_world_ptr->get_current_level_global_script_object() )
      it->spawn_item_script();
//    else
//      it->need_to_initialize = true;

    return true;
  }
}


// returns NULL if index is out-of=range
item* entity::get_item( unsigned int n ) const
{
  if ( is_container() && n<(unsigned)get_num_items() )
    return coninfo->items[n];
  else
    return NULL;
}

// returns null pointer if no like item found
item* entity::find_like_item( item* it ) const
{
  if ( is_container() )
  {
    item_list_t::const_iterator i = coninfo->items.begin();
    item_list_t::const_iterator i_end = coninfo->items.end();
    for ( ; i!=i_end; ++i )
    {
      item* lit = *i;
      if ( lit && lit->is_same_item( *it ) )
        return lit;
    }
  }
  return NULL;
}

// returns null pointer if no like item found
item* entity::find_item_by_name( const stringx &name ) const
{
  if ( is_container() )
  {
    item_list_t::const_iterator i = coninfo->items.begin();
    item_list_t::const_iterator i_end = coninfo->items.end();
    for ( ; i!=i_end; ++i )
    {
      item* lit = *i;
      if ( lit && lit->get_name() == name )
        return lit;
    }
  }
  return NULL;
}

#if 0 // BIGCULL
handheld_item* entity::find_item_by_id( const stringx &id ) const
{
  if ( is_container() )
  {
    item_list_t::const_iterator i = coninfo->items.begin();
    item_list_t::const_iterator i_end = coninfo->items.end();
    for ( ; i!=i_end; ++i )
    {
      item* lit = *i;
      if ( lit && lit->is_a_handheld_item() && ((handheld_item *)lit)->get_item_id() == id )
        return((handheld_item *)lit);
    }
  }
  return NULL;
}
#endif // BIGCULL

// returns -1 if item is not found in list
int entity::get_item_index( item* it ) const
{
  if ( is_container() )
  {
    item_list_t::const_iterator i = coninfo->items.begin();
    item_list_t::const_iterator i_end = coninfo->items.end();
    int index = 0;
    for ( ; i!=i_end; ++i,++index )
    {
      if ( *i == it )
        return index;
    }
  }
  return -1;
}


// returns the next item in the list after the given one (wraps around);
// returns NULL if given item is not found
item* entity::get_next_item( item* itm ) const
{
  int n = get_num_items();
  if ( n )
  {
    int base_cur_item = get_item_index( itm );

    if(base_cur_item == -1)
      base_cur_item = 0;

    int c = base_cur_item;
    do
    {
      ++c;
      if ( c >= n )
        c = 0;
    } while ( (!get_item(c) || get_item(c)->get_number()==0) && c!=base_cur_item );

    item* newitm = get_item( c );
    if ( newitm && newitm->get_number()>0 )
      return newitm;
  }
  return NULL;
}

// returns the previous item in the list before the given one (wraps around);
// returns NULL if given item is not found
item* entity::get_prev_item( item* itm ) const
{
  int n = get_num_items();
  if ( n )
  {
    int base_cur_item = get_item_index( itm );

    if(base_cur_item == -1)
      base_cur_item = 0;

    int c = base_cur_item;
    do
    {
      --c;
      if ( c < 0 )
        c = n - 1;
    } while ( (!get_item(c) || get_item(c)->get_number()==0) && c!=base_cur_item );

    item* newitm = get_item( c );
    if ( newitm && newitm->get_number()>0 )
      return newitm;
  }
  return NULL;
}


// disgorge any items I may be carrying
void entity::disgorge_items(entity *target)
{
error("Disgorge_items not supported in KS.");
#if 0  //BIGCULL
  if ( is_container() )
  {
    item_list_t::const_iterator i = coninfo->items.begin();
    item_list_t::const_iterator i_end = coninfo->items.end();
    for ( ; i!=i_end; ++i )
    {
      item* it = *i;

      if(it && (it->get_number() > 0 || (it->is_a_thrown_item() && ((thrown_item *)it)->is_a_radio_detonator())) && !it->is_brain_weapon())
      {
        if(target == NULL)
        {
          it->set_visible( true );
          vector3d newpos( (((float)(random(100)))*.03f)-1.5f, .5f, (((float)(random(100)))*.03f)-1.5f );
          newpos *= 0.25f;
          newpos += get_abs_position();
          po newpo = po_identity_matrix;
          newpo.set_position( newpos );
//!          it->set_parent( NULL );
          it->set_rel_po( newpo );
          it->set_pickup_timer(0.5f);
          it->compute_sector( g_world_ptr->get_the_terrain() );
          it->raise_signal( item::SCHWING );
        }
        else
        {
/*!          if(target->is_a_character())
            it->give_to_character((character *)target);
          else
!*/
            target->add_item(it);
        }
      }
    }

    coninfo->items.resize(0);
  }
#endif //BIGCULL
}

void entity::use_item(item *it)
{
  if(it != NULL)
  {
    last_item_used = it;

    it->apply_effects( this );

    raise_signal(entity::USE_ITEM);
  }
}

void entity::copy_visrep(entity *ent)
{
#ifdef NGL

  my_mesh = ent->get_mesh();
  lores_mesh = ent->get_lores_mesh();
  shadow_mesh = ent->get_shadow_mesh();

  if(my_mesh)
    set_flag( EFLAG_GRAPHICS, true );
  else
    set_flag( EFLAG_GRAPHICS, false );

#else

  unload_visual_rep( my_visrep );

  if(ent->get_vrep())
  {
    if ( ent->get_vrep()->is_instanced() )
      my_visrep = new_visrep_instance( ent->get_vrep() );
    else
      my_visrep = new_visrep_copy( ent->get_vrep() );
  }
  else
    my_visrep = NULL;

  if(my_visrep)
    set_flag( EFLAG_GRAPHICS, true );
  else
    set_flag( EFLAG_GRAPHICS, false );

#endif
}

/*!
void entity::activate_by_character(character *chr)
{
  action_character = chr;

  raise_signal(ACTIVATED_BY_CHARACTER);
}
!*/


bool entity::allow_targeting() const
{
  return 1;
#if 0 // BIGCULL
  if (!has_damage_ifc())
    return false;

  return (damage_ifc()->is_alive() && is_combat_target());
#endif // BIGCULL
}

bool entity::test_combat_target( const vector3d& p0, const vector3d& p1,
                                 vector3d* impact_pos, vector3d* impact_normal,
                                 rational_t default_radius, bool rear_cull ) const
{
  return(collide_segment_entity(p0, p1, this, impact_pos, impact_normal, default_radius, rear_cull));
}

void entity::process_extra_scene_flags(unsigned int scn_flags)
{
  if(scn_flags & BEAMABLE_FLAG)
  {
    if(!is_beamable())
      set_beamable(true);
  }
  else if(scn_flags & NO_BEAMABLE_FLAG)
  {
    if(is_beamable())
      set_beamable(false);
  }

  if ( scn_flags & SCANABLE_FLAG )
  {
    if ( !is_scannable() )
    {
      if ( has_mesh() )
      {
        set_scannable( true );
      }
      else
        warning( get_id().get_val() + ": entity must have mesh visrep to be set as SCANABLE" );
    }
  }
  else if ( scn_flags & NO_SCANABLE_FLAG )
  {
    if ( is_scannable() )
      set_scannable(false);
  }

  if ( get_colgeom() && get_colgeom()->get_type() == collision_geometry::MESH )
  {
    cg_mesh* m = static_cast<cg_mesh*>( get_colgeom() );

    if(scn_flags & CAMERA_COLL_FLAG)
    {
      if(!m->is_camera_collision())
        m->set_flag( cg_mesh::FLAG_CAMERA_COLLISION, true );
    }
    else if(scn_flags & NO_CAMERA_COLL_FLAG)
    {
      if(m->is_camera_collision())
        m->set_flag( cg_mesh::FLAG_CAMERA_COLLISION, false );
    }

    if(scn_flags & ENTITY_COLL_FLAG)
    {
      if(!m->is_entity_collision())
        m->set_flag( cg_mesh::FLAG_ENTITY_COLLISION, true );
    }
    else if(scn_flags & NO_ENTITY_COLL_FLAG)
    {
      if(m->is_entity_collision())
        m->set_flag( cg_mesh::FLAG_ENTITY_COLLISION, false );
    }
  }

  if(scn_flags & ACTIONABLE_FLAG)
  {
    if(!is_actionable())
      set_actionable(true);
  }
  else if(scn_flags & NO_ACTIONABLE_FLAG)
  {
    if(is_actionable())
      set_actionable(false);
  }

  if(scn_flags & ACTION_FACING_FLAG)
  {
    if(!action_uses_facing())
      set_action_uses_facing(true);
  }
  else if(scn_flags & NO_ACTION_FACING_FLAG)
  {
    if(action_uses_facing())
      set_action_uses_facing(false);
  }

  if(scn_flags & IS_DOOR_FLAG)
  {
    if(!is_door())
      set_door(true);
  }
  else if(scn_flags & NO_IS_DOOR_FLAG)
  {
    if(is_door())
      set_door(false);
  }

  if(is_door())
  {
    if(scn_flags & DOOR_OPEN_FLAG)
    {
      if(is_door_closed())
        set_door_closed(false);
    }
    else if(scn_flags & DOOR_CLOSED_FLAG)
    {
      if(!is_door_closed())
        set_door_closed(true);
    }
  }

#if _ENABLE_WORLD_EDITOR
  scene_flags = scn_flags & OVERIDE_MASK_FLAG;

  if(is_beamable())
    scene_flags |= BEAMABLE_FLAG;

  if(is_scannable())
    scene_flags |= SCANABLE_FLAG;


  if ( get_colgeom() && get_colgeom()->get_type() == collision_geometry::MESH )
  {
    cg_mesh* m = static_cast<cg_mesh*>( get_colgeom() );

    if(m->is_camera_collision())
      scene_flags |= CAMERA_COLL_FLAG;

    if(m->is_entity_collision())
      scene_flags |= ENTITY_COLL_FLAG;
  }

  if(is_actionable())
    scene_flags |= ACTIONABLE_FLAG;

  if(action_uses_facing())
    scene_flags |= ACTION_FACING_FLAG;

  if(is_door())
  {
    scene_flags |= IS_DOOR_FLAG;

    if(!is_door_closed())
      scene_flags |= DOOR_OPEN_FLAG;
  }
#endif
}

bool entity::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("entity") )
  {
    stringx cf;
    for ( serial_in(fs,&cf); cf!=chunkend_label; serial_in(fs,&cf) )
    {
      if ( cf == stringx("status") )
      {
        unsigned int scn_flags;
        serial_in( fs, &scn_flags );
        process_extra_scene_flags(scn_flags);
        set_needs_export(true);
      }
      else if ( cf == stringx("invuln") )
      {
        set_invulnerable(true);
      }
      else if ( cf == stringx("items") )
      {
/*!        if ( is_a_character() )
          error( fs.get_name() + ": entity_info: " + get_name() + ": 'items' chunk is for adding brain_weapons to a non-character; use 'brain weapons' instead" );
!*/
        error( "Entity item: Items not supported in KS." );
#if 0 //BIGCULL
        stringx label;
        for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
        {
          entity* itm;
          stringx entity_name = get_fname_wo_ext( label );
          entity_name.to_upper();
          stringx entity_dir = get_dir( label );
          itm = g_entity_maker->create_entity_or_subclass( entity_name,
                                                     entity_id::make_unique_id(),
                                                     po_identity_matrix,
                                                     entity_dir,
                                                     ACTIVE_FLAG | NONSTATIC_FLAG );
          if ( !itm->is_an_item() )
          {
            error( "Entity item: entity " + entity_name + " is not an item" );
          }
          else
          {
            if ( !itm->is_a_handheld_item() )
            {
              error( "Entity item: entity " + entity_name + " is not an handheld item" );
            }
            else
            {
              itm->set_created_entity_default_active_status();
              ((handheld_item*)itm)->set_handheld_flag( _HANDHELD_BRAIN_WEAPON, true );
              bool added = add_item( (item*)itm );

              if ( !added )
                warning( fs.get_name() + ": brain weapon '" + entity_name + "' added more than once to entity '" + get_name() + "'" );
              else
              {
                ((handheld_item*)itm)->draw();
                ((handheld_item*)itm)->hide();
                // this entity needs to frame_advance() even when ineligible by the usual rules;
                // normally, this is handled by item::give_to_character(), but as you can see that
                // cannot be called in this case
                if(itm->is_a_gun())
                  g_world_ptr->guarantee_active( itm );
              }
            }
          }
        }
#endif //BIGCULL
      }
      else if ( cf == stringx("destroy") )
      {
        stringx label = "destroy";

        if(!destroy_info)
        {
          destroy_info = NEW destroyable_info(this);
          label = "destroy_info:";
        }
        else
        {
          // reset the destroy info structure
          destroy_info->reset();
        }

        destroy_info->read_enx_data(fs, label);
        set_needs_export(true);
      }
      else if ( cf == stringx("lighting:" ))
      {
        //assert(get_flavor() == ENTITY_CHARACTER || get_flavor() == ENTITY_CONGLOMERATE);

        stringx label;
        for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
        {
          if ( label=="max_lights" )
          {
            serial_in(fs,&max_lights);
            light_manager* lm = get_light_set();
            if (lm)
              lm->max_lights = max_lights;
          }
          else
            error("Bad keyword %s in lighting section", label.c_str());
        }
        return true;
      }
      else
        return false;
    }

    return true;
  }
  else if ( pcf == stringx("ai:") )
  {
    assert( !(get_name() == "HERO") );
    if ( !has_ai_ifc() )
    {
      create_ai_ifc();
      ai_ifc()->set_active(true);
    }

    ai_ifc()->read_data( fs );

    return true;
  }
#if 0 // BIGCULL
  else if ( pcf == stringx("damage_interface:") )
  {
    assert( !(get_name() == "HERO") );
    if(!has_damage_ifc())
      create_damage_ifc();

    stringx label;
    damage_ifc()->read_enx_data(fs, label);

    return true;
  }
#endif

  return false;
}

void entity::create_destroy_info()
{
  if(destroy_info == NULL)
    destroy_info = NEW destroyable_info(this);
}

void entity::suspend()
{
  if (!suspended)
  {
    suspended = true;

    if(has_ai_ifc())
      ai_ifc()->push_disable();

    if ( my_controller != NULL )
    {
      suspended_active_status = my_controller->is_active();
//      if ( my_controller->is_a_brain() )
//      {
//        if ( get_brain()->brain_active() )
//          get_brain()->push_inactive();
//      }
//      else
        my_controller->set_active( false );
    }
  }
}


void entity::unsuspend()
{
  if (suspended)
  {
    suspended = false;

    if(has_ai_ifc())
      ai_ifc()->pop_disable();

    if ( my_controller != NULL )
    {
//      if ( my_controller->is_a_brain() )
//      {
//        if ( !get_brain()->brain_active() )
//          get_brain()->pop_inactive();
//      }
//      else
        my_controller->set_active( suspended_active_status );
    }
  }
}


void entity::set_controller(entity_controller * c)
{
  assert(my_controller == NULL);

  my_controller = c;

  if ( c )
    c->set_active( is_active()/* || c->is_a_brain()*/ );
}


/*
brain * entity::get_brain()
{
//  assert (this!=g_world_ptr->get_hero_ptr() && my_controller->is_a_brain());
  if(my_controller && my_controller->is_a_brain())
    return (brain *)my_controller;
  else
    return(NULL);
}
*/
void entity::set_control_active( bool a )
{
  if ( my_controller )
    my_controller->set_active( a );
}

bool entity::is_alive() const
{
// BIGCULL return(!has_damage_ifc() || damage_ifc()->is_alive());
return true;
}

bool entity::is_dying() const
{
  return(false);
}

bool entity::is_alive_or_dying() const
{
  return(is_alive() || is_dying());
}

bool entity::is_hero() const
{
	for (int i = 0; i < g_game_ptr->get_num_players(); i++)
	{
		if (this == g_world_ptr->get_hero_ptr(i))
			return true;
	}

	return false;
}


bool entity::possibly_active() const
{
  return !is_conglom_member()
    &&  (  /*is_active() ||*/
           ( get_light_set() && is_flagged(EFLAG_GRAPHICS) && is_visible())
        || (has_physical_ifc() || has_ai_ifc()
#ifdef ECULL
				|| has_sound_ifc()
#endif
				));
}


bool entity::possibly_aging() const
{
  return(my_visrep != NULL && ( ( my_visrep->get_anim_length() > 1)||( my_visrep->is_uv_animated() ) ));
}


void entity::set_active( bool a )
{
  if(entity::is_active() != a)
  {
    if ( a )
      flags|=EFLAG_ACTIVE;
    else
      flags&=~EFLAG_ACTIVE;

//    region_update_poss_active();

    if ( my_controller )
      set_control_active( a );
  }
}

void entity::set_visible( bool a )
{
	if(entity::is_visible() != a)
	{
		if( a )
		{
			if (!is_ext_flagged(EFLAG_EXT_WAS_VISIBLE))	// we turned it off in the draw menu
			{
				flags|=EFLAG_GRAPHICS_VISIBLE;
			}
		}
		else
		{
			flags&=~EFLAG_GRAPHICS_VISIBLE;
		}

		region_update_poss_render();
	}
}

void entity::set_collisions_active( bool a, bool update_reg )
{
  if(entity::are_collisions_active() != a)
  {
    if(a)
    {
      flags|=EFLAG_PHYSICS_COLLISIONS_ACTIVE;
    }
    else
    {
      flags&=~EFLAG_PHYSICS_COLLISIONS_ACTIVE;
    }

    if(update_reg)
      region_update_poss_collide();
  }
}

void entity::region_update_poss_active()
{
#ifndef REGIONCULL
  region_node_pset::iterator i; //,j;
  for ( i=in_regions.begin(); i!=in_regions.end(); ++i)
  {
    region* r = (*i)->get_data();
    if(r)
      r->update_poss_active(this);
  }
#endif
}

void entity::region_update_poss_render()
{
#ifndef REGIONCULL
  region_node_pset::iterator i; //,j;
  for ( i=in_regions.begin(); i!=in_regions.end(); ++i)
  {
    region* r = (*i)->get_data();
    if(r)
      r->update_poss_render(this);
  }
#endif
}

void entity::region_update_poss_collide()
{
#ifndef REGIONCULL
  region_node_pset::iterator i; //,j;
  for ( i=in_regions.begin(); i!=in_regions.end(); ++i)
  {
    region* r = (*i)->get_data();
    if(r)
      r->update_poss_collide(this);
  }
#endif
}


void entity::preload()
{
  if(!was_preloaded())
  {
    set_preloaded(true);

    if(destroy_info != NULL)
      destroy_info->preload();

//    if(get_brain() != NULL)
//      get_brain()->preload();
  }
}

void entity::set_min_detail(int md)
{
  assert(md>=0 && md<=2); min_detail = md;
  if (min_detail > os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL))
  {
    set_visible(false);
    set_collisions_active(false);
    set_combat_target(false);
    /*set_auto_targetable(false);
    set_beamable(false);
    set_visible_to_AI(false);*/
    delete_colgeom(); // I'd love to not have to do this, but someone keeps turning collisions back on if we have one!!
  }
}


void entity::clear_all_raised_signals()
{
  signals_raised[0] = 0;
  signals_raised[1] = 0;
}

void entity::clear_signal_raised(unsigned short sig_id)
{
  assert(sig_id > 0 && sig_id < N_SIGNALS && sig_id < 64);
  signals_raised[(sig_id < 32 ? 1 : 0)] &= ~(0x80000000 >> (sig_id < 32 ? sig_id : (sig_id - 32)));
}

bool entity::signal_raised(unsigned short sig_id)
{
  assert(sig_id > 0 && sig_id < N_SIGNALS && sig_id < 64);
  return((signals_raised[(sig_id < 32 ? 1 : 0)] & (0x80000000 >> (sig_id < 32 ? sig_id : (sig_id - 32)))) != 0);
}

static void entity_signal_callback_footstep(signaller* sig, const char*pccdata)
{
#ifdef ECULL
  entity *whoami = (entity*)sig;
  if(whoami->has_sound_ifc())
  {
    static pstring footstep("FOOTSTEP");
    whoami->sound_ifc()->play_3d_sound_grp(footstep);
  }
#endif
}

static void entity_signal_callback_attack(signaller* sig, const char*pccdata)
{
#if 0 // BIGCULL
  entity *whoami = (entity*)sig;

  if ( whoami->is_hero() )
  {
    g_spiderman_controller_ptr->apply_attack(g_spiderman_controller_ptr->get_combo_move(), g_spiderman_controller_ptr->get_combo_damage(), g_spiderman_controller_ptr->get_combo_flags(), g_spiderman_controller_ptr->get_combo_wounded_anim());
  }
  else if(whoami->has_ai_ifc() && whoami->ai_ifc()->get_target())// || whoami->get_brain()->get_anim_attack_type() != DAMAGE_NONE)
  {
    // This has been moved into the AI (ai_actions.cpp, class: attack_ai_action), where it should be (JDB 4-10-01)....
/*
    entity *pc = whoami->ai_ifc()->get_target();//g_world_ptr->get_hero_ptr();
    vector3d mypos = whoami->get_abs_position();
    vector3d pcpos = pc->get_abs_position();
    vector3d vec = mypos - pcpos;
    vector3d invvec = pcpos - mypos;
    rational_t len = vec.length();
    int attack_type = DAMAGE_MELEE;//whoami->get_brain() ? whoami->get_brain()->get_anim_attack_type() : DAMAGE_DIRECT;

    assert(pc->has_damage_ifc());

//    int damage = whoami->get_brain() ? whoami->get_brain()->get_anim_damage() : 10;
    int damage = 10;

    // THE multiply THING IS A FUDGE FACTOR
    // TO MAKE THE SPHERE MORE OF AN OBLATE SPHEROID
    // SINCE I'M NOT SUCKING THE CAPSULE
    rational_t rad = (whoami->get_radius()+pc->get_radius());
    if( len*1.0f < rad )
    {
      rational_t ab = angle_between( invvec, whoami->get_abs_po().get_facing() );
      // GUESSTIMATE FOR 90deg EXPRESSED IN RADS, GOUACHE AND MIXED MEDIA ON CANVAS
      if( ab <= 1.570795f )
      {
        pc->damage_ifc()->apply_damage( whoami, damage, (eDamageType)attack_type, mypos, invvec );
        if(whoami->has_sound_ifc())
        {
          pstring p("IMPACT");
          whoami->sound_ifc()->play_3d_sound_grp( p );
        }
      }
    }
*/
  }
#endif // BIGCULL
}

void entity_signal_callback_raiser(signaller* sig, const char* sig_id)
{
  assert(sig->is_an_entity());
  unsigned short id = (unsigned short)sig_id;
  ((entity *)sig)->signals_raised[(id < 32 ? 1 : 0)] |= (0x80000000 >> (id < 32 ? id : (id - 32)));
}

void entity::add_signal_callbacks()
{
  signal_ptr( ATTACK )->add_callback( entity_signal_callback_attack, NULL );

  signal_ptr( FOOTSTEP_L )->add_callback( entity_signal_callback_footstep, NULL );
  signal_ptr( FOOTSTEP_R )->add_callback( entity_signal_callback_footstep, NULL );

  for(unsigned short i=0; i<N_SIGNALS; ++i)
    signal_ptr( i )->add_callback( entity_signal_callback_raiser, (char *)i );
}


bool entity::get_ifc_num(const pstring &att, rational_t &val)
{
//BIGCULL   IFC_DATA_GET_NUM_MACRO(damage);
#ifdef ECULL
  IFC_DATA_GET_NUM_MACRO(script_data);
#endif
  IFC_DATA_GET_NUM_MACRO(ai);
  IFC_DATA_GET_NUM_MACRO(physical);
  IFC_DATA_GET_NUM_MACRO(soft_attrib);
  IFC_DATA_GET_NUM_MACRO(hard_attrib);
  IFC_DATA_GET_NUM_MACRO(time);

  return(false);
}

bool entity::set_ifc_num(const pstring &att, rational_t val)
{
//BIGCULL   IFC_DATA_SET_NUM_MACRO(damage);
#ifdef ECULL
  IFC_DATA_SET_NUM_MACRO(script_data);
#endif
  IFC_DATA_SET_NUM_MACRO(ai);
  IFC_DATA_SET_NUM_MACRO(physical);
  IFC_DATA_SET_NUM_MACRO(soft_attrib);
  IFC_DATA_SET_NUM_MACRO(time);

  return(false);
}

bool entity::get_ifc_vec(const pstring &att, vector3d &val)
{
//BIGCULL   IFC_DATA_GET_VEC_MACRO(damage);
#ifdef ECULL
  IFC_DATA_GET_VEC_MACRO(script_data);
#endif
  IFC_DATA_GET_VEC_MACRO(ai);
  IFC_DATA_GET_VEC_MACRO(physical);
  IFC_DATA_GET_VEC_MACRO(soft_attrib);
  IFC_DATA_GET_VEC_MACRO(hard_attrib);

  return(false);
}

bool entity::set_ifc_vec(const pstring &att, const vector3d &val)
{
//BIGCULL   IFC_DATA_SET_VEC_MACRO(damage);
#ifdef ECULL
	IFC_DATA_SET_VEC_MACRO(script_data);
#endif
  IFC_DATA_SET_VEC_MACRO(ai);
  IFC_DATA_SET_VEC_MACRO(physical);
  IFC_DATA_SET_VEC_MACRO(soft_attrib);

  return(false);
}

bool entity::get_ifc_str(const pstring &att, stringx &val)
{
// BIGCULL   IFC_DATA_GET_STR_MACRO(damage);
#ifdef ECULL
  IFC_DATA_GET_STR_MACRO(script_data);
#endif
  IFC_DATA_GET_STR_MACRO(ai);
  IFC_DATA_GET_STR_MACRO(physical);
  IFC_DATA_GET_STR_MACRO(soft_attrib);
  IFC_DATA_GET_STR_MACRO(hard_attrib);

  return(false);
}

bool entity::set_ifc_str(const pstring &att, const stringx &val)
{
// BIGCULL   IFC_DATA_SET_STR_MACRO(damage);
#ifdef ECULL
  IFC_DATA_SET_STR_MACRO(script_data);
#endif
  IFC_DATA_SET_STR_MACRO(ai);
  IFC_DATA_SET_STR_MACRO(physical);
  IFC_DATA_SET_STR_MACRO(soft_attrib);

  return(false);
}

entity::entity_search_list entity::found_entities;

int entity::find_entities(int flags, const vector3d &pos, rational_t radius, region_node *reg, bool only_active_portals)
{
  found_entities.resize(0);

  if(reg == NULL)
  {
    sector *sect = g_world_ptr->get_the_terrain().find_sector(pos);
    if(sect)
      reg = sect->get_region();
  }

  if(reg != NULL)
  {
    static vector<region_node *> search_regs;
    search_regs.resize(0);
    rational_t radsqr = radius*radius;

    build_region_list_radius(&search_regs, reg, pos, radius, only_active_portals);

    entity::prepare_for_visiting2();

    vector<entity*>::const_iterator e;
    vector<entity*>::const_iterator e_end;
    vector<region_node*>::iterator rn = search_regs.begin();
    vector<region_node*>::iterator rn_end = search_regs.end();
    for ( ; rn!=rn_end; ++rn )
    {
      region* r = (*rn)->get_data();
      e = r->get_entities().begin();
      e_end = r->get_entities().end();

	    while( e != e_end )
	    {
        entity *ent = (*e);
        ++e;

        if(ent && !ent->already_visited2() && ent->match_search_flags(flags) && (ent->get_abs_position() - pos).length2() <= radsqr)
        {
          found_entities.push_back(ent);
          ent->visit2();
        }
      }
    }
  }

  return(found_entities.size());
}

int entity::find_entities(int flags)
{
  found_entities.resize(0);

  vector<entity*>::const_iterator e = g_world_ptr->get_entities().begin();
  vector<entity*>::const_iterator e_end = g_world_ptr->get_entities().end();

	while( e != e_end )
	{
    entity *ent = (*e);
    ++e;

    if(ent && ent->match_search_flags(flags))
      found_entities.push_back(ent);
  }

  return(found_entities.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool entity::has_mesh()
{
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
  if( get_mesh() )
    return true;
  else
    return false;
#else
  if( get_vrep() )
    if( get_vrep()->get_type()==VISREP_PMESH )
      return true;
  return false;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int entity::num_mesh_bones()
{
  if( has_mesh() )
  {
#if defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC)
    return get_mesh()->NBones;
#else
    return ((vr_pmesh*)get_vrep())->get_num_bones();
#endif
  }
  return 0;
}

void entity::set_max_lights( unsigned int ml )
{
  max_lights=min( ml, (unsigned int)ABSOLUTE_MAX_LIGHTS );
  light_manager* lm = get_light_set();
  if( lm )
  {
    lm->max_lights = max_lights;
  }
}

void entity::set_mesh_distance( nglVector &Center, float Radius, float forcedist )
{
	if(!my_mesh)
		return;

	my_mesh->SphereCenter=Center;
	if ( lores_mesh )
		lores_mesh->SphereCenter=Center;
	if ( shadow_mesh )
		shadow_mesh->SphereCenter=Center;
	my_mesh->SphereRadius=Radius;	// no real need for this sphere, since we will never cull this mesh
	if ( lores_mesh )
		lores_mesh->SphereRadius=Radius;	// no real need for this sphere, since we will never cull this mesh
	if ( shadow_mesh )
		shadow_mesh->SphereRadius=Radius;	// no real need for this sphere, since we will never cull this mesh
/*	This code does nothing.  (dc 01/21/02)
	if ( forcedist>=0.0f )
	{
		my_mesh->Sections[0].Material->ForcedSortDistance;
		if ( lores_mesh )
			lores_mesh->Sections[0].Material->ForcedSortDistance;
		if ( shadow_mesh )
			shadow_mesh->Sections[0].Material->ForcedSortDistance;
	}
*/
}

u_int entity::get_mesh_matflags( void )
{
	if(!my_mesh)
		return 0;

	return my_mesh->Sections->Material->Flags;
}


void SetEntityMeshFlags( nglMesh *Mesh, int flag )
{
  if ( Mesh )
  {
    for ( u_int i = 0; i < Mesh->NSections; i++ )
    {
      nglMeshSection* Section = &Mesh->Sections[i];
      nglMaterial* Material = Section->Material;
      Material->Flags |= flag;
    }
  }
}

void ClearEntityMeshFlags( nglMesh *Mesh, int flag )
{
  if ( Mesh )
  {
    for ( u_int i = 0; i < Mesh->NSections; i++ )
    {
      nglMeshSection* Section = &Mesh->Sections[i];
      nglMaterial* Material = Section->Material;
      Material->Flags &= ~flag;
    }
  }
}

void SetEntityMeshSpecLev( nglMesh *Mesh, float splev )
{
#ifdef TARGET_PS2
  if ( Mesh )
  {
    for ( u_int i = 0; i < Mesh->NSections; i++ )
    {
      nglMeshSection* Section = &Mesh->Sections[i];
      nglMaterial* Material = Section->Material;
      Material->SpecularIntensity = splev;
    }
  }
#endif
}




void entity::set_mesh_matflags( u_int flags )
{
	ClearEntityMeshFlags( my_mesh, 0xFFFFFFFF );
	SetEntityMeshFlags( my_mesh, flags );
	ClearEntityMeshFlags( lores_mesh, 0xFFFFFFFF );
	SetEntityMeshFlags( lores_mesh, flags );
}

#ifndef TARGET_XBOX
void entity::set_specular_env_level( float splev )
{
	if ( splev>0.001 )
	{
		set_mesh_matflagbits(NGLMAT_ENV_SPECULAR);
	}
	else
	{
		clear_mesh_matflagbits(NGLMAT_ENV_SPECULAR);
	}
	SetEntityMeshSpecLev(my_mesh,splev);
	SetEntityMeshSpecLev(lores_mesh,splev);
}
#endif

void entity::set_mesh_matflagbits( u_int flags )
{
	SetEntityMeshFlags( my_mesh, flags );
	SetEntityMeshFlags( lores_mesh, flags );
}

void entity::clear_mesh_matflagbits( u_int flags )
{
	ClearEntityMeshFlags( my_mesh, flags );
	ClearEntityMeshFlags( lores_mesh, flags );
}

void entity::set_mesh_texture( nglTexture *tex )
{
	if(my_mesh)
		my_mesh->Sections[0].Material->Map=tex;
}

void entity::set_zbias( int newz )
{
	usezbias=(newz!=0);
	zbias=(float) newz;
}

u_int entity::get_mesh_flags( void )
{
  if(my_mesh)
    return my_mesh->Flags;

  return 0;
}

void entity::set_mesh_flags( u_int flags )
{
  if (my_mesh)
  	my_mesh->Flags=flags;
}









