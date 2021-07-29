// region.cpp
// Copyright (C) 2000 Treyarch LLC    ALL RIGHTS RESERVED

#include "global.h"
#include "region.h"
#include "colmesh.h"
#include "cface.h"
#include "entity.h"
#include "bound.h"
#include "light.h"
#include "vm_thread.h"
#include "profiler.h"

#include <algorithm>

// creates a sorting 1-d grid with a resolution REGION_SORT_RES meters per tick.
const rational_t REGION_SORT_RES = 1.0f;

const rational_t MIN_PATH_NORMAL_Y = 0.7f;


////////////////////////////////////////////////////////////////////////////////
// cface_replacement is what we use for terrain pathfinding now instead of cface
////////////////////////////////////////////////////////////////////////////////

cface_replacement::cface_replacement( const vr_pmesh *pPP, face_ref rFF )
:   pP( pPP ),
    rF( rFF )
{
  assert( pP && (rF>=0) );
  const vector3d& v0 = get_corner_point( 0 );
  const vector3d& v1 = get_corner_point( 1 );
  const vector3d& v2 = get_corner_point( 2 );
  // compute center
  center = (v0 + v1 + v2) * (1.0f / 3.0f);
  // compute radius
  rational_t r0 = ( v0 - center ).length2();
  rational_t r1 = ( v1 - center ).length2();
  rational_t r2 = ( v2 - center ).length2();
  radius = __fsqrt( max(r0,max(r1,r2)) );
  // compute normal
  normal = cross( v1-v0, v2-v0 );
  normal.normalize();
}

void cface_replacement::set( const vr_pmesh *pPP, face_ref rFF )
{
  pP = pPP;
  rF = rFF;

  const vector3d& v0 = get_corner_point( 0 );
  const vector3d& v1 = get_corner_point( 1 );
  const vector3d& v2 = get_corner_point( 2 );
  // compute center
  center = (v0 + v1 + v2) * (1.0f / 3.0f);
  // compute radius
  rational_t r0 = ( v0 - center ).length2();
  rational_t r1 = ( v1 - center ).length2();
  rational_t r2 = ( v2 - center ).length2();
  radius = __fsqrt( max(r0,max(r1,r2)) );
  // compute normal
  normal = cross( v1-v0, v2-v0 );
  normal.normalize();
}

void const cface_replacement::verify_integrity() const
{
  assert( pP && (rF>=0) );

  vector3d center2;
  rational_t radius2;
  vector3d normal2;

  const vector3d& v0 = get_corner_point( 0 );
  const vector3d& v1 = get_corner_point( 1 );
  const vector3d& v2 = get_corner_point( 2 );
  // compute center
  center2 = (v0 + v1 + v2) * (1.0f / 3.0f);
  // compute radius
  rational_t r0 = ( v0 - center2 ).length2();
  rational_t r1 = ( v1 - center2 ).length2();
  rational_t r2 = ( v2 - center2 ).length2();
  radius2 = __fsqrt( max(r0,max(r1,r2)) );
  // compute normal
  normal2 = cross( v1-v0, v2-v0 );
  normal2.normalize();

  assert( (normal2==normal) && (radius2==radius) && (center2==center) );
}



////////////////////////////////////////////////////////////////////////////////
// class region
////////////////////////////////////////////////////////////////////////////////


unsigned int region::visit_key = 0;

region::region( const stringx& region_name )
  : name( region_name ),
    ambient( 1.0f, 1.0f, 1.0f, 1.0f ),
    region_ambient_sound_volume( 1.0f )
{
  my_cg_mesh = NULL;
  flags = 0;
  set_active(false);

  num_affect_terrain_lights = 0;

  // stl prealloc-ish
  possible_render_ents.resize(64);
  possible_render_ents.resize(0);
  possible_active_ents.resize(64);
  possible_active_ents.resize(0);
}


region::~region()
{
  local_thread_list.resize(0);
#ifndef NGL
  for (VisRepList::iterator vri=visreps.begin();
       vri!=visreps.end();
       ++vri)
    if( (*vri) )
      vr_pmesh_bank.delete_instance((vr_pmesh*)(*vri));
#endif
}

#ifndef NGL
static stringx make_unique_label( const stringx& prototype )
{
  static unsigned int modifier = 1;
  return prototype + stringx( modifier++ );
}
#endif


extern int g_total_alloced;

void serial_in( chunk_file& fs, region* r, terrain* ter )
{
  chunk_flavor cf;

  // optional ambient color
  serial_in( fs, &cf );
  if ( cf == chunk_flavor("ambient") )
  {
    color ambient;
    serial_in( fs, &ambient );
#ifdef TARGET_XBOX
	// On Xbox, we override the lighting values from the scene.  The PS2 values aren't appropriate for 
	// Xbox, because of differences in gamma and saturation.  To be fixed in the exporter next year. 
	// (dc 07/11/02)
	float ambient_boost;
	if (!FEDone())
	{
		static float ambient_boost_fe = 10;
		ambient_boost = ambient_boost_fe / 256;
	}
	else 
	{
		int curbeach=g_game_ptr->get_beach_id();
		assert (curbeach >= 0 && curbeach < BEACH_LAST);
		ambient_boost = BeachDataArray[curbeach].ambient_boost / 256;
	}
//	ambient.a = 1.0f;
	ambient.r += ambient_boost;
	ambient.g += ambient_boost;
	ambient.b += ambient_boost;
#endif
    r->set_ambient( ambient );
    serial_in( fs, &cf );
  }

  // terrain mesh
  assert(cf==chunk_flavor("meshfile"));
  do
  {
    stringx meshname;
    serial_in( fs, &meshname );
#ifdef NGL
    filespec spec(fs.get_name());
    stringx mesh_path = spec.path;
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,mesh_path.c_str());
#endif
    //nglSetMeshPath( (char *)mesh_path.c_str() );
    stringx texture_path = spec.path + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
    //nglSetTexturePath( (char *)texture_path.c_str() );
    filespec mesh_spec(meshname);
    nglMesh* mesh = nglGetMesh( (char *)mesh_spec.name.c_str() );
    if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
      nglPrintf("Loading mesh %s - %p\n", mesh_spec.name.c_str(), mesh);
    r->ps2_meshes.push_back(mesh);
#else
    filespec spec(fs.get_name());
    stringx meshfilename = spec.path + meshname + stringx(".txtmesh");
    // read visual mesh (instanced by physent ID)
    chunk_file txtmeshfile;
    txtmeshfile.open( meshfilename );
    vr_pmesh* mesh = vr_pmesh_bank.new_instance( txtmeshfile, make_unique_label(fs.get_name()), vr_pmesh::NORMAL );
    r->visreps.push_back(mesh);
#endif

    serial_in( fs, &cf );
  } while (cf==chunk_flavor("meshfile"));

  assert(cf==CHUNK_END);

//  int before = g_total_alloced;
#if !(  defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC) )
  cg_mesh* colgeom = NEW cg_mesh(r->visreps, fs.get_name(), cg_mesh::ALLOW_WARNINGS);

  // terrain faces default to cosmetic until we find 'em in the BSP tree somewhere.
  for (int i=colgeom->get_num_cfaces(); --i>=0; )
  {
    // Kill the const: bad form! <<<<
    ((cface *) colgeom->get_cface_ptr(i))->set_flag(CFACE_FLAG_COSMETIC,true);
  }
  r->my_cg_mesh = colgeom;
  r->build_sorted();
  delete r->my_cg_mesh;  // why do we build this thing only to scrag it again?  oh, so it can manufacture 'sorted'
  r->my_cg_mesh=NULL;

  for (region::VisRepList::iterator vri=r->visreps.begin(); vri!=r->visreps.end(); ++vri)
  {
    vr_pmesh* mesh = (vr_pmesh*)(*vri);
    mesh->shrink_memory_footprint();
  }
#endif
}

// build sorted lists of entities based on:
//    bounding box info
//    visual_xz_radius_rel_center

void region::sort_entities()
{
  x_sort_entities_by_bounding_box_info();
}


void region::x_sort_entities_by_bounding_box_info()
{
  entity_list::iterator ei, ej;
  entity_list::const_iterator ei_end, ej_end;
  // build list
  int n = 0;
  ei_end = entities.end();
  for ( ei=entities.begin(); ei!=ei_end; ei++ )
  {
    entity* e = *ei;
    if ( e && e->has_bounding_box() )
      ++n;
  }
  x_sorted_entities.resize(0);
  x_sorted_ent_lookup_low.resize(0);
  x_sorted_ent_lookup_high.resize(0);
  x_sorted_ent_max = -FLT_MAX;
  if ( n )
  {
    if ( n > 0x00FF )
      error( get_name() + ": region has more than 255 static entities.  You must be insane!" );
    x_sorted_entities.reserve( n );
    for ( ei=entities.begin(); ei!=ei_end; ++ei )
    {
      entity* e = *ei;
      if ( e && e->has_bounding_box() )
      {
        x_sorted_entities.push_back( e );
        if ( e->get_bounding_box().vmax.x > x_sorted_ent_max )
          x_sorted_ent_max = e->get_bounding_box().vmax.x;
      }
    }
    // sort by min x
    ei_end = x_sorted_entities.end() - 1;
    ej_end = x_sorted_entities.end();
    for ( ei=x_sorted_entities.begin(); ei!=ei_end; ++ei )
    {
      entity* e1 = *ei;
      for ( ej=ei+1; ej!=ej_end; ++ej )
      {
        entity* e2 = *ej;
        if ( e2->get_bounding_box().vmin.x < e1->get_bounding_box().vmin.x )
        {
          *ei = e2;
          *ej = e1;
          e1 = e2;
        }
      }
    }
    x_sorted_ent_min = x_sorted_entities[0]->get_bounding_box().vmin.x;
    // build low lookup table (indexed by meters from minimum vmin.x value)
    int i, j;
    int i_end = (int)(x_sorted_ent_max - x_sorted_ent_min + 1);
    int j_end = x_sorted_entities.size();
    x_sorted_ent_lookup_low.reserve( i_end );
    x_sorted_ent_lookup_high.reserve( i_end );
    for ( i=j=0; i<i_end; ++i )
    {
      rational_t p = x_sorted_ent_min + i;
      while ( j<j_end && x_sorted_entities[j]->get_bounding_box().vmax.x<p )
        ++j;
      x_sorted_ent_lookup_low.push_back( j );
    }
    // build high lookup table (indexed by meters from maximum vmax.x value)
    j_end = -1;
    for ( i=0,j=x_sorted_entities.size()-1; i<i_end; ++i )
    {
      rational_t p = x_sorted_ent_max - i;
      while ( j>j_end && p<x_sorted_entities[j]->get_bounding_box().vmin.x )
        --j;
      x_sorted_ent_lookup_high.push_back( j );
    }
  }
}

// I'm scared of all this code.  It looks to me like it can return indices that point
// outside container.  --Sean

// BOUNDING BOX X sorted list

int region::get_low_xsorted_entity( rational_t x ) const
{
  if ( x_sorted_entities.empty() )
    return -1;
  int i = (int)(x - x_sorted_ent_min);
  if ( i < 0 )
    return x_sorted_ent_lookup_low[0];
  if ( i >= (int)x_sorted_ent_lookup_low.size() )
    return x_sorted_entities.size(); // is this right?
  return x_sorted_ent_lookup_low[i];
}

int region::get_high_xsorted_entity( rational_t x ) const
{
  if ( x_sorted_entities.empty() )
    return -1;
  int i = (int)(x_sorted_ent_max - x);
  if ( i < 0 )
    return x_sorted_ent_lookup_high[0];
  if ( i >= (int)x_sorted_ent_lookup_high.size() )
    return -1;                    // is this right?
  return x_sorted_ent_lookup_high[i];
}


static int compare_cface_replacement( const void* x1, const void* x2 )
{
  cface_replacement* cf1 = (cface_replacement*)x1;
  cface_replacement* cf2 = (cface_replacement*)x2;
  if( cf1->get_center().x < cf2->get_center().x )
    return -1;
  if( cf1->get_center().x > cf2->get_center().x )
    return 1;
  return 0;
}

void region::build_sorted()
{
  int i,j;
  assert(sorted.empty());

  rational_t solid_max;
  rational_t water_max;

  int nsolid = 0;
  int nwater = 0;
#if !( defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC) )
  VisRepList::const_iterator vri;
  for (vri = visreps.begin(); vri!=visreps.end(); ++vri)
  {
    vr_pmesh * cm = (vr_pmesh*)(*vri);

    // count polys of each type (solid and water only, for now)
    for ( i=0; i<cm->get_max_faces(); ++i )
    {
      const vector3d &v0 = cm->get_xvert_unxform(cm->get_wedge_ref( i, 0 ) );
      const vector3d &v1 = cm->get_xvert_unxform(cm->get_wedge_ref( i, 1 ) ) ;
      const vector3d &v2 = cm->get_xvert_unxform(cm->get_wedge_ref( i, 2 ) );

      vector3d favec = cross(v1-v0, v2-v0).normalize();
      if( dot(favec,YVEC) > MIN_PATH_NORMAL_Y && !cm->is_cosmetic( i ))
        ++nsolid;
      else if( cm->is_water( i ) )
        ++nwater;
    }
  }
#endif

  // create the lists
  if ( nsolid )
    sorted.reserve( nsolid );
  if ( nwater )
    sorted_water.reserve( nwater );

  solid_min =  FLT_MAX;
  solid_max = -FLT_MAX;
  water_min =  FLT_MAX;
  water_max = -FLT_MAX;

#if !( defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(TARGET_GC) )
  for (vri = visreps.begin(); vri!=visreps.end(); ++vri)
  {
    vr_pmesh * cm = (vr_pmesh*)(*vri);

    for ( i=0; i<cm->get_max_faces(); ++i )
    {

      const vector3d &v0 = cm->get_xvert_unxform(cm->get_wedge_ref( i, 0 ) );
      const vector3d &v1 = cm->get_xvert_unxform(cm->get_wedge_ref( i, 1 ) ) ;
      const vector3d &v2 = cm->get_xvert_unxform(cm->get_wedge_ref( i, 2 ) );

      vector3d favec = cross(v1-v0, v2-v0).normalize();

      bool is_walkable = ( dot(favec,YVEC) > MIN_PATH_NORMAL_Y && !cm->is_cosmetic( i ));

      bool is_water = cm->is_water( i );

      if ( is_walkable || is_water )
      {
        cface_replacement cf(cm,i);
        rational_t x = cf.get_center().x;
        rational_t r = cf.get_radius();
        if ( is_walkable )
        {
          sorted.push_back( cf );
          // track solid min/max
          if ( x-r < solid_min )
            solid_min = x - r;
          if ( x+r > solid_max )
            solid_max = x + r;
        }
        else
        {
          sorted_water.push_back( cf );
          // track water min/max
          if ( x-r < water_min )
            water_min = x - r;
          if ( x+r > water_max )
            water_max = x + r;
        }
      }
    }
  }
#endif

  if ( !sorted.empty() )
  {
    // sort was freaking out on us.  something to do with memory allocation and MEMTRACK, but who
    // knows what?
    qsort( &(*sorted.begin()), sorted.size(), sizeof(cface_replacement), compare_cface_replacement );

    // Build the lookup
    int lookup_size = (int)((solid_max - solid_min) / REGION_SORT_RES + 1);
    sorted_lookup_low .resize(lookup_size);
    sorted_lookup_high.resize(lookup_size);
    j = 0;
    for ( i=0; i<(int)sorted_lookup_low.size(); ++i )
    {
      rational_t val = solid_min + i*REGION_SORT_RES;
      while ( j<(int)sorted.size() && sorted[j].get_center().x + sorted[j].get_radius() < val )
        ++j;
      sorted_lookup_low[i] = j;
    }
    j = sorted.size()-1;
    for ( i=sorted_lookup_high.size(); --i>=0; )
    {
      rational_t val = solid_min + (i+1)*REGION_SORT_RES;
      while ( j>=0 && sorted[j].get_center().x - sorted[j].get_radius() > val )
        --j;
      sorted_lookup_high[i] = j;
    }
  }

  // build the water lookup
  if ( !sorted_water.empty() )
  {
    qsort( &(*sorted_water.begin()), sorted_water.size(), sizeof(cface_replacement), compare_cface_replacement );
    int water_lookup_size = (int)((water_max - water_min) / REGION_SORT_RES + 1);
    sorted_water_lookup_low .resize(water_lookup_size);
    sorted_water_lookup_high.resize(water_lookup_size);
    j = 0;
    for ( i=0; i<(int)sorted_water_lookup_low.size(); ++i )
    {
      rational_t val = water_min + i * REGION_SORT_RES;
      while ( j<(int)sorted_water.size() && sorted_water[j].get_center().x + sorted_water[j].get_radius() < val )
        ++j;
      sorted_water_lookup_low[i] = j;
    }
    j = sorted_water.size() - 1;
    for ( i=sorted_water_lookup_high.size(); --i>=0; )
    {
      rational_t val = water_min + (i+1) * REGION_SORT_RES;
      while ( j>=0 && sorted_water[j].get_center().x - sorted_water[j].get_radius() > val )
        --j;
      sorted_water_lookup_high[i] = j;
    }
  }
}


int region::get_low_index( rational_t val ) const
{
  if ( !sorted.empty() )
  {
    assert(!sorted_lookup_low.empty());
    int val_tick = (int)((val - solid_min) / REGION_SORT_RES);
    if ( val_tick < 0 )
      val_tick = 0;
    else if ( val_tick >= (int)sorted_lookup_low.size() )
      val_tick = sorted_lookup_low.size() - 1;
    return sorted_lookup_low[val_tick];
  }
  return -1;
}

int region::get_high_index( rational_t val ) const
{
  if ( !sorted.empty() )
  {
    assert(!sorted_lookup_high.empty());
    int val_tick = (int)((val - solid_min) / REGION_SORT_RES);
    if ( val_tick < 0 )
      val_tick = 0;
    else if ( val_tick >= (int)sorted_lookup_high.size() )
      val_tick = sorted_lookup_high.size() - 1;
    return sorted_lookup_high[val_tick];
  }
  return -1;
}


int region::get_low_water_index( rational_t val ) const
{
  if ( !sorted_water.empty() )
  {
    int val_tick = (int)((val - water_min) / REGION_SORT_RES);
    if ( val_tick < 0 )
      val_tick = 0;
    else if ( val_tick >= (int)sorted_water_lookup_low.size() )
      val_tick = sorted_water_lookup_low.size() - 1;
    return sorted_water_lookup_low[val_tick];
  }
  else
    return -1;
}

int region::get_high_water_index( rational_t val ) const
{
  if ( !sorted_water.empty() )
  {
    int val_tick = (int)((val - water_min) / REGION_SORT_RES);
    if ( val_tick < 0 )
      val_tick = 0;
    else if ( val_tick >= (int)sorted_water_lookup_high.size() )
      val_tick = sorted_water_lookup_high.size() - 1;
    return sorted_water_lookup_high[val_tick];
  }
  else
    return -1;
}

void region::add( entity* e )
{
  entity_list::iterator ei_begin = entities.begin();
  entity_list::iterator ei_end = entities.end();

  entity_list::iterator ei = find( ei_begin, ei_end, e );
  if ( ei == ei_end )
  {
    ei = find( ei_begin, ei_end, (entity*)NULL );
    if ( ei != ei_end )
      *ei = e;
    else
    {
      entities.push_back( e );
    }
    if ( e->is_beamable() || e->is_scannable() || (e->get_colgeom() && e->get_colgeom()->is_camera_collision()) )
      add_cam_coll_ent( e );
  }

  if(e->possibly_active() || e->possibly_aging())
  {
    ei_begin = possible_active_ents.begin();
    ei_end = possible_active_ents.end();

    entity_list::iterator ei = find( ei_begin, ei_end, e );
    if ( ei == ei_end )
    {
      ei = find( ei_begin, ei_end, (entity*)NULL );
      if ( ei != ei_end )
        *ei = e;
      else
      {
        possible_active_ents.push_back( e );
      }
    }
  }

#if USE_POSS_RENDER_LIST                                       // WTB - removed, isn't EFLAG_GRAPHICS enough here?
  if(e->is_still_visible() && e->is_flagged(EFLAG_GRAPHICS) /*&& (e->get_vrep() || e->has_mesh() || e->get_flavor() == ENTITY_POLYTUBE)*/)
  {
    entity_list::iterator ei_begin = possible_render_ents.begin();
    entity_list::iterator ei_end = possible_render_ents.end();

    entity_list::iterator ei = find( ei_begin, ei_end, e );
    if ( ei == ei_end )
    {
      ei = find( ei_begin, ei_end, (entity*)NULL );
      if ( ei != ei_end )
        *ei = e;
      else
      {
        possible_render_ents.push_back( e );
      }
    }
  }
#endif

  if(e->get_colgeom() && e->are_collisions_active())
  {
    entity_list::iterator ei_begin = possible_collide_ents.begin();
    entity_list::iterator ei_end = possible_collide_ents.end();

    entity_list::iterator ei = find( ei_begin, ei_end, e );
    if ( ei == ei_end )
    {
      ei = find( ei_begin, ei_end, (entity*)NULL );
      if ( ei != ei_end )
        *ei = e;
      else
      {
        possible_collide_ents.push_back( e );
      }
    }
  }
}

void region::remove( entity* e )
{
  entity_list::iterator ei_end = entities.end();
  entity_list::iterator ei = find( entities.begin(), ei_end, e );
  if ( ei != ei_end )
  {
    entities.erase(ei);
    *ei = NULL;
    if ( e->is_beamable() || e->is_scannable() || (e->get_colgeom() && e->get_colgeom()->is_camera_collision()) )
      remove_cam_coll_ent( e );
  }

  ei_end = possible_active_ents.end();
  ei = find( possible_active_ents.begin(), ei_end, e );
  if ( ei != ei_end )
    *ei = NULL;

#if USE_POSS_RENDER_LIST
  ei_end = possible_render_ents.end();
  ei = find( possible_render_ents.begin(), ei_end, e );
  if ( ei != ei_end )
    *ei = NULL;
#endif

  ei_end = possible_collide_ents.end();
  ei = find( possible_collide_ents.begin(), ei_end, e );
  if ( ei != ei_end )
    *ei = NULL;
}

void region::update_poss_active( entity* e )
{
  START_PROF_TIMER(proftimer_update_poss_active);
  if(e)
  {
    if(e->possibly_active() || e->possibly_aging())
    {
      entity_list::iterator ei_begin = possible_active_ents.begin();
      entity_list::iterator ei_end = possible_active_ents.end();

      entity_list::iterator ei = find( ei_begin, ei_end, e );
      if ( ei == ei_end )
      {
        ei = find( ei_begin, ei_end, (entity*)NULL );
        if ( ei != ei_end )
          *ei = e;
        else
        {
          possible_active_ents.push_back( e );
        }
      }
    }
    else
    {
      entity_list::iterator ei_end = possible_active_ents.end();
      entity_list::iterator ei = find( possible_active_ents.begin(), ei_end, e );
      if ( ei != ei_end )
        *ei = NULL;
    }
  }
  STOP_PROF_TIMER(proftimer_update_poss_active);
}


#if USE_POSS_RENDER_LIST
  void region::update_poss_render( entity* e )
  {
    START_PROF_TIMER(proftimer_update_poss_render);
    if(e)
    {                                                           // WTB - removed, isn't EFLAG_GRAPHICS enough here?
      if(e->is_still_visible() && e->is_flagged(EFLAG_GRAPHICS) /*&& (e->get_vrep() || e->has_mesh() || e->get_flavor() == ENTITY_POLYTUBE)*/)
      {
        entity_list::iterator ei_begin = possible_render_ents.begin();
        entity_list::iterator ei_end = possible_render_ents.end();

        entity_list::iterator ei = find( ei_begin, ei_end, e );
        if ( ei == ei_end )
        {
          ei = find( ei_begin, ei_end, (entity*)NULL );
          if ( ei != ei_end )
            *ei = e;
          else
          {
            possible_render_ents.push_back( e );
          }
        }
      }
      else
      {
        entity_list::iterator ei_end = possible_render_ents.end();
        entity_list::iterator ei = find( possible_render_ents.begin(), ei_end, e );
        if ( ei != ei_end )
          *ei = NULL;
      }
    }
    STOP_PROF_TIMER(proftimer_update_poss_render);
  }
#endif


void region::update_poss_collide( entity* e )
{
  START_PROF_TIMER(proftimer_update_poss_collide);
  if(e)
  {
    if(e->get_colgeom() && e->are_collisions_active())
    {
      entity_list::iterator ei_begin = possible_collide_ents.begin();
      entity_list::iterator ei_end = possible_collide_ents.end();

      entity_list::iterator ei = find( ei_begin, ei_end, e );
      if ( ei == ei_end )
      {
        ei = find( ei_begin, ei_end, (entity*)NULL );
        if ( ei != ei_end )
          *ei = e;
        else
        {
          possible_collide_ents.push_back( e );
        }
      }
    }
    else
    {
      entity_list::iterator ei_end = possible_collide_ents.end();
      entity_list::iterator ei = find( possible_collide_ents.begin(), ei_end, e );
      if ( ei != ei_end )
        *ei = NULL;
    }
  }
  STOP_PROF_TIMER(proftimer_update_poss_collide);
}


void region::add_cam_coll_ent( entity* e )
{
  entity_list::iterator ei_begin = cam_coll_ents.begin();
  entity_list::iterator ei_end = cam_coll_ents.end();
  entity_list::iterator ei = find( ei_begin, ei_end, (entity*)NULL );
  if ( ei != ei_end )
    *ei = e;
  else
  {
    cam_coll_ents.push_back( e );
  }
}

void region::remove_cam_coll_ent( entity* e )
{
  entity_list::iterator ei_begin = cam_coll_ents.begin();
  entity_list::iterator ei_end = cam_coll_ents.end();
  entity_list::iterator ei = find( ei_begin, ei_end, e );

  if( ei != ei_end )
  {
    *ei = NULL;
  }
}


void region::add( light_source* e )
{
  if(e->get_properties().affects_terrain())
    ++num_affect_terrain_lights;

  light_list::iterator ei_begin=lights.begin(), ei_end=lights.end();
  light_list::iterator ei = find( ei_begin, ei_end, e );
  if ( ei == ei_end )
  {
    for ( ei=ei_begin; ei!=ei_end; ++ei )
    {
      if ( !*ei )
      {
        *ei = e;
        return;
      }
    }
    lights.push_back( e );
  }
}

void region::remove( light_source* e )
{
  light_list::iterator ei_end = lights.end();
  light_list::iterator ei = find( lights.begin(), ei_end, e );
  if ( ei != ei_end )
  {
    if(e->get_properties().affects_terrain())
    {
      assert(num_affect_terrain_lights > 0);
      --num_affect_terrain_lights;
    }

    *ei = NULL;
  }
}

void region::add( trigger* e )
{
  trigger_list::iterator ei_begin=triggers.begin(), ei_end=triggers.end();
  trigger_list::iterator ei = find( ei_begin, ei_end, e );
  if ( ei == ei_end )
  {
    for ( ei=ei_begin; ei!=ei_end; ++ei )
    {
      if ( !*ei )
      {
        *ei = e;
        return;
      }
    }
    triggers.push_back( e );
  }
}

void region::remove( trigger* e )
{
  trigger_list::iterator ei_end = triggers.end();
  trigger_list::iterator ei = find( triggers.begin(), ei_end, e );
  if ( ei != ei_end )
    *ei = NULL;
}

void region::add( crawl_box* cb )
{
  // wds owns these for purposes of memory management
  crawls.push_back( cb );
}

void region::remove( crawl_box* cb )
{
  crawls.remove( cb );
}

void region::add( ai_polypath_cell* c )
{
  pathcell_list::iterator ci_begin=pathcells.begin(), ci_end=pathcells.end();
  pathcell_list::iterator ci = find( ci_begin, ci_end, c );
  if ( ci == ci_end )
  {
    for ( ci=ci_begin; ci!=ci_end; ++ci )
    {
      if ( !*ci )
      {
        *ci = c;
        return;
      }
    }
    pathcells.push_back( c );
  }
}

void region::remove( ai_polypath_cell* c )
{
  pathcell_list::iterator ci_end = pathcells.end();
  pathcell_list::iterator ci = find( pathcells.begin(), ci_end, c );
  if ( ci != ci_end )
    *ci = NULL;
}


void region::add_local_thread(vm_thread * thr)
{
  region * reg;
  if ((reg = thr->get_local_region())!=NULL)
  {
    reg->remove_local_thread(thr);
  }
  local_thread_list.push_back(thr);
  thr->set_local_region(this);
  thr->set_suspended(!is_active());
}


void region::remove_local_thread(vm_thread * thr)
{
  local_thread_list.remove(thr);
  thr->set_suspended(false);
}


void region::set_active( bool v )
{
  if (!is_locked()) flags = v ? (flags|ACTIVE) : (flags&~ACTIVE);
  list<vm_thread *>::iterator li,lb,le;
  lb = local_thread_list.begin();
  le = local_thread_list.end();
  for (li=lb;li!=le;++li)
  {
    (*li)->set_suspended(!v);
  }
}

void region::set_region_ambient_sound( stringx &sndname )
{
  if( region_ambient_sound_name.length() )
  {
    // kill old sound
  }
  region_ambient_sound_name = sndname;
}


enum
{
  LEDGE_NONE = 0,
  LEDGE_STAND,
  LEDGE_CRAWL,
  LEDGE_1M
};

inline static rational_t distance_point_to_line(const vector3d &line_start, const vector3d &line_dir, const vector3d &point)
{
  return cross((point - line_start), line_dir).length();
}

void region::optimize()
{
#ifdef TARGET_PC
  for (VisRepList::iterator vri=visreps.begin();
       vri!=visreps.end();
       ++vri)
  {
    visual_rep* vr = *vri;
    if (vr)
    {
      if (vr->get_type()==VISREP_PMESH)
      {
        vr_pmesh* mesh = (vr_pmesh*)vr;
        mesh->optimize_static_mesh_for_d3d();
      }
    }
  }
#endif
}

