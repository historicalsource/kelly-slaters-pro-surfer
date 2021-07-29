////////////////////////////////////////////////////////////////////////////////

// terrain.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// This is the home of various classes which define the terrain.  It includes the
// portals, regions, and the BSP tree->

////////////////////////////////////////////////////////////////////////////////

#include "global.h"

#include "terrain.h"
#include "debug.h"
#include "colmesh.h"
#include "wds.h"
#include "collide.h"
//P #include "memorycontext.h"
#include "osdevopts.h"
#include "profiler.h"
#include "oserrmsg.h"
#include <vector>
#include <algorithm>
#include "bsp_tree.h"
#include "osalloc.h"
#include "iri.h"
#include "bsp_tree.h"

partition3 *global_plane;


////////////////////////////////////////////////////////////////////////////////
// CLASS partition3
////////////////////////////////////////////////////////////////////////////////

// Constructors

partition3::partition3()
  : _H(),
    my_sector( NULL )
{
}

partition3::partition3( const partition3& b )
  : _H( b ),
    my_sector( b.my_sector )
{
}

partition3::partition3( const vector3d& c, const vector3d& n, sector* s)
  : _H( c, n ),
    my_sector( s )
{
}


// SUBCLASS terrain::tree_t

// Constructors

tree_t::tree_t()
  : _Tr()
{
}


// Methods

// return sector corresponding to given world-space coordinate position;
// return NULL if outside legal world space
sector* tree_t::find_sector(const vector3d& pos) const
{
  branch br = find_leaf(pos);
  if (br.child == branch::LEFT)
    return NULL;
  assert(br.child==branch::RIGHT && br.parent._Mynode());
  return br.parent->get_sector();
}


////////////////////////////////////////////////////////////////////////////////
// CLASS terrain
////////////////////////////////////////////////////////////////////////////////

// Statics

entity * terrain::last_elevation_entity;

// Constructors

terrain::terrain()
{
}

terrain::terrain(const stringx& filename)
{
  chunk_file fs;
  fs.open(filename);
  tree = NEW tree_t;
  serial_in(fs,this);
}

terrain::~terrain()
{
  tree->clear();
  delete tree;
  for (int i=regions.size(); --i>=0; )
  {
    delete regions[i];
  }
  regions.resize(0);
  sectors.resize(0);
  portal_list::iterator pli;
  for( pli = portals.begin(); pli != portals.end(); ++pli )
    delete *pli;
}

// Internal Methods

void terrain::_load_tree(chunk_file& fs)
{
#ifndef TARGET_XBOX	// XBox stl clears the reserved memory as well as the data! (dc 02/06/02)
  sectors.resize(0);
  tree->clear();
#endif
  // set up stack for "recursion"
  branch_vector recursion_stack;
  recursion_stack.reserve( 500 );
  recursion_stack.push_back( tree_t::branch() );
  // perform recursive load
  _recursive_load_tree( fs, recursion_stack );
}


// find an intersection by BSP tree solid geometry and shooting a ray from p0 to p1 and
// return the intersection pi and the normal n
bool terrain::find_intersection(const vector3d &p0, const vector3d &p1, vector3d &pi, vector3d &n)
{
	return tree->find_intersection(p0, p1, pi, n);
}

bool terrain::in_world(const vector3d &p, const rational_t r, const vector3d &v, vector3d &n, vector3d &pi)
{
	return tree->in_world(p, r, v, n, pi);
}



// this creates sorted lists of entities in each region according to their
// bounding box info and
// visual_xz_radius_rel_center
void terrain::sort_entities_within_each_region()
{
  region_list::iterator ri;
  region_list::const_iterator ri_end = regions.end();
  for ( ri=regions.begin(); ri!=ri_end; ++ri )
    (*ri)->sort_entities();
}


vector3d terrain::get_gradient( const vector3d& normal, const vector3d& direction )
{
  vector3d outvec = cross(normal,cross(direction,normal).normalize());
  return outvec;
}


////////////////////////////////////////////////////////////////////////////////
// serial IO
////////////////////////////////////////////////////////////////////////////////

void serial_in( chunk_file& fs, terrain* ter )
{
  chunk_flavor cf;
  bool no_BSP = false;
  chunk_file txtmesh_fs;
  stringx txtmesh_file_name;
  filespec txtmesh_spec( fs.get_name( ) );

  txtmesh_spec.name += "R";

#if defined(TARGET_XBOX)
  txtmesh_spec.ext = ".xbmesh";
#elif defined(TARGET_PS2)
  txtmesh_spec.ext = NGL_MESHFILE_EXT;
#else
  txtmesh_spec.ext = ".txtmesh";
#endif

  bool single_region_txtmesh = false;

  if ( world_dynamics_system::wds_exists ( txtmesh_spec.path + txtmesh_spec.name, txtmesh_spec.ext ) )
  {
    single_region_txtmesh = true;
#ifdef NGL

#if defined(TARGET_XBOX) || defined(TARGET_GC)
    nglSetMeshPath( txtmesh_spec.path.c_str() );
		nglSetTexturePath( ( txtmesh_spec.path + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\" ).c_str() );
#else
		nglLoadMeshFile( txtmesh_spec.name.c_str() );
#endif // TARGET_XBOX

#else
    txtmesh_file_name = txtmesh_spec.name + txtmesh_spec.ext;
    txtmesh_fs.open( txtmesh_file_name, os_file::FILE_READ );

    debug_print( "Reading the region file: " + txtmesh_file_name );

    serial_in( txtmesh_fs, &cf ); // 'nomeshes'
    int mesh_total;
    serial_in( txtmesh_fs, &mesh_total );

    // read in all regions now
    for (int index = 0; index < mesh_total; index++)
    {
      chunk_flavor txtmesh_cf;
      serial_in( txtmesh_fs, &txtmesh_cf ); // 'mesh'
      assert( txtmesh_cf == chunk_flavor( "mesh" ) );
      serial_in( txtmesh_fs, &txtmesh_cf ); // 'meshname'
      assert( txtmesh_cf == chunk_flavor( "meshname" ) );
      stringx mesh_name;
      serial_in( txtmesh_fs, &mesh_name );

      debug_print( "  Reading region txtmesh: " + mesh_name );

      // read the txtmesh into vr_pmesh_bank
      if ( !( vr_pmesh_bank.new_instance( txtmesh_fs, mesh_name,vr_pmesh::NORMAL ) ) )
        error( "Couldn't create new instance of meshname in serial_in(fs,*ter)" );

      serial_in( txtmesh_fs, &txtmesh_cf ); // 'chunkend'
    }

    txtmesh_fs.close( );
#endif
  }

  // read list of regions
  serial_in(fs,&cf);
  assert (cf==CHUNK_REGIONS);
  for (serial_in(fs,&cf); cf==CHUNK_REGION; serial_in(fs,&cf))
  {
    // read region name
    stringx name;
    serial_in(fs,&name);
    // insert NEW region into list and add node to graph
    region * reg = NEW region(name);
    ter->regions.push_back(reg);
    region* r = ter->regions.back();
    ter->regions_graph.insert_node(name,r);
    // read region data
    error_context::inst()->push_context(name);
    serial_in( fs, r, ter );
    error_context::inst()->pop_context();

    // BUILT REGION PATH
  }
  assert(cf==CHUNK_END);

  // read list of portals
  serial_in(fs,&cf);
  assert(cf==CHUNK_PORTALS);
  for (serial_in(fs,&cf); cf==CHUNK_PORTAL; )
  {
    // read front region name and find associated node
    stringx region_name;
    serial_in(fs,&region_name);
    region_iterator rgi = ter->regions_graph.find(region_name);
    assert(rgi!=ter->regions_graph.end());
    region_node& front_node = (*rgi).second;
    // read back region name and find associated node
    serial_in(fs,&region_name);
    rgi = ter->regions_graph.find(region_name);
    if (rgi==ter->regions_graph.end())
    {
      stringx composite;
      composite = fs.get_name() + ": Couldn't find region " + region_name;
      error(composite);
    }
    region_node& back_node = (*rgi).second;
    // create portal
    ter->portals.push_back(NEW portal(&front_node,&back_node));
    portal* p = ter->portals.back();
    // read portal mesh
    serial_in(fs,&cf);
    assert(cf==chunk_flavor("meshfile"));
    stringx meshname;
    serial_in( fs, &meshname );
    filespec spec(fs.get_name());
    stringx meshfilename = spec.path + meshname + stringx(".txtmesh");
    // read visual mesh (instanced by physent ID)
    chunk_file txtmeshfile;
    txtmeshfile.open( meshfilename );

    error_context::inst()->push_context("NOWARN");
    serial_in(txtmeshfile,p);
    error_context::inst()->pop_context();
    // insert NEW edge into region graph
    ter->regions_graph.insert_edge(p,front_node,back_node);

    // get next chunk
    serial_in(fs,&cf);
  }
  assert(cf==CHUNK_END);

  serial_in(fs,&cf);
  if (cf == CHUNK_BSP_NODES)
  {
    int n;
    serial_in(fs,&n);
    ter->tree->reserve(n);
    serial_in(fs,&cf);
  }
  else
  {
    warning("bsp_node chunk missing from TER file.\nRe-export terrain to remove this warning.");
    ter->tree->reserve(40000);
    no_BSP = true;
  }

  if (cf == CHUNK_BSP_SECTORS)
  {
    int n;
    serial_in(fs,&n);
    ter->sectors.reserve(n);
    serial_in(fs,&cf);
  }
  else
  {
    warning("bsp_sect chunk missing from TER file.\nRe-export terrain to remove this warning.");
    ter->sectors.reserve(20000);
    no_BSP = true;
  }

  if (cf == CHUNK_BSP_TREE)
  {
    // read bsp tree
    ter->_load_tree(fs);
    serial_in(fs,&cf);
  }
  assert(cf==CHUNK_END);
}

void terrain::_recursive_load_tree( chunk_file& fs, branch_vector & recursion_stack )
{
  chunk_flavor cf;

  while ( !recursion_stack.empty() )
  {
    tree_t::branch br = recursion_stack.back();
    recursion_stack.pop_back();

   // read hyperplane
    vector3d center;
    serial_in(fs,&center);
    vector3d normal;
    serial_in(fs,&normal);
    // read terrain face references, if any
    partition3::faceref_list new_facerefs;
    serial_in( fs, &cf );
    tree_t::branch newbr;
    if ( cf == CHUNK_FACEREFS )
    {
      unsigned short n;
      serial_in( fs, &n );
      for (int k=n; --k>=0; )
      {
        // read mesh and face ids
        unsigned short meshidx, faceidx;
        serial_in( fs, &meshidx );
        serial_in( fs, &faceidx );
        // obtain mesh pointer
        // region_list::iterator rli;
        // for ( rli=regions.begin(); meshidx; rli++,meshidx-- );
        //assert( rli != regions.end() );
        // assert( faceidx < (*rli).get_cg_mesh()->get_num_cfaces() );
        assert(meshidx<regions.size());

        // add face reference to partition
        new_facerefs.push_back( partition3::faceref(meshidx, faceidx)); //  (*rli).get_cg_mesh(), faceidx ) );
      }
      serial_in(fs,&cf);
    }
    // read sector value
    sector* psec;
    if (cf == CHUNK_SECTOR)
    {
      stringx region_name;
      serial_in(fs,&region_name);
      region_iterator rgi = regions_graph.find(region_name);
		  if ( rgi == regions_graph.end() )
			  error( ("Unable to locate region " + region_name).c_str() );
      region_node& rnode = (*rgi).second;
      // make sure we don't overflow the sectors array.
      assert(sectors.size()<sectors.capacity());
      sectors.push_back(sector(&rnode));
      sector& s = sectors.back();
      psec = &s;
      serial_in(fs,&cf);
    }
    else
      psec = NULL;

    // insert partition into tree
    tree->insert(br,partition3(center,normal, psec));

    if (tree->size() == 1)
    {
      // we just inserted the root node; initialize the branch value to the
      // root of the tree
      newbr = tree_t::branch(tree->begin());
    }
    else
      newbr = br;

    // determine presence of right subtree
    chunk_flavor cfr;
    serial_in(fs,&cfr);
    if (cfr == CHUNK_RIGHT)
      recursion_stack.push_back( tree_t::branch(*newbr,tree_t::branch::RIGHT) );
    else
      assert(cfr==CHUNK_NULL);

    // determine presence of left subtree
    if (cf == CHUNK_LEFT)
      recursion_stack.push_back( tree_t::branch(*newbr,tree_t::branch::LEFT) );
    else
      assert(cf==CHUNK_NULL);
  }
}


const rational_t MIN_GET_ELEVATION_NORMAL_Y = 0.7f;

extern profiler_timer proftimer_get_elevation;

rational_t terrain::get_elevation( const vector3d& p,
                                   vector3d& normal,
                                   region_node *default_region,
                                   unsigned char* hit_surface_type )
{
  proftimer_get_elevation.start();

  vector3d pos(p);
  // familiar kludge to avoid out of world problems.
  pos.y += 0.1f;
  rational_t outval = BOTTOM_OF_WORLD;
  normal = YVEC;
  vector3d ground_loc, ground_normal;
  vector3d low_pos = pos - YVEC*6; // DEEP;  New theory says 4 meters is enough.  We shall see.

  sector* s = g_world_ptr->get_the_terrain().find_sector(pos);
  region_node* r;
  if ( s )
    r = s->get_region();
  else
    r = default_region;

  vector3d local_entity_ground_loc, local_entity_ground_normal;
  vector3d entity_ground_loc, entity_ground_normal;
  int search_direction = 1;
//  bool done = false;
  last_elevation_entity = NULL;

  if ( r )
  {
    const region* rg = r->get_data();
    const vector3d vdown(0,-1,0);
    bool hit;
    if ( hit_surface_type )
    {
      const vr_pmesh* pM = NULL;
      face_ref rF;
      hit = collide_segment_region_with_poly_data( pos, low_pos, rg, ground_loc, ground_normal, PP_REAR_CULL, vector3d(0,-1,0), &pM, &rF );
      if ( hit )
        *hit_surface_type = pM->get_surface_type( rF );
    }
    else
      hit = collide_segment_region( pos, low_pos, rg, ground_loc, ground_normal, PP_REAR_CULL, vector3d(0,-1,0) );
    if ( hit )
    {
      if ( ground_normal.y < MIN_GET_ELEVATION_NORMAL_Y )
        hit = false;
      else
      {
        outval = ground_loc.y;
        normal = ground_normal;
        low_pos = ground_loc;
      }
    }
    //profcounter_get_elevation_a.add_count(1);

    // let's just check the entities in the current region;
    // this might produce anomalies near portals that are not perfectly vertical,
    // but there should be precious few of those anyway

    // first check moving walkable entities
    entity *ent;

//    region::entity_list::const_iterator ei = rg->get_entities().begin();
//    region::entity_list::const_iterator ei_end = rg->get_entities().end();
    region::entity_list::const_iterator ei = rg->get_possible_collide_entities().begin();
    region::entity_list::const_iterator ei_end = rg->get_possible_collide_entities().end();
    for ( ; ei!=ei_end; ++ei )
    {
      ent = *ei;
      if ( ent && ent->get_colgeom() && ent->is_walkable() && ent->is_flagged(EFLAG_MISC_NONSTATIC) )
      {
        assert( ent->is_stationary() );
        vector3d diff = ent->get_abs_position() - pos;
        rational_t rad = ent->get_radius();
        if ( diff.xz_length2() < rad*rad )
        {
          bool hit;
          ent->get_colgeom()->set_owner( ent );
          g_world_ptr->get_origin_entity()->set_radius( ent->get_radius() );

          vector3d local_pos = ent->get_abs_po().fast_inverse_xform(pos);
          vector3d local_low_pos = ent->get_abs_po().fast_inverse_xform(low_pos);

          vector3d local_vdown = ent->get_abs_po().fast_non_affine_inverse_xform(vdown);

          hit = collide_segment_mesh( local_pos, local_low_pos, (cg_mesh*)ent->get_updated_colgeom(), local_entity_ground_loc, local_entity_ground_normal, PP_REAR_CULL, local_vdown );
          if ( hit )
          {
            entity_ground_loc = ent->get_abs_po().fast_8byte_xform( local_entity_ground_loc );
            entity_ground_normal = ent->get_abs_po().fast_8byte_non_affine_xform( local_entity_ground_normal );
            if ( hit_surface_type )
            {
              // CTT 04/18/00: TODO: obtain surface type info from entity and fill hit_surface_type
            }
          }

          if ( hit )
          {
            if ( entity_ground_normal.y < MIN_GET_ELEVATION_NORMAL_Y )
              hit = false;
            else if ( (entity_ground_loc.y-outval)*search_direction > 0 )
            {
              outval = entity_ground_loc.y;
              normal = entity_ground_normal;
              last_elevation_entity = ent;
              low_pos = entity_ground_loc;
            }
          }
        }
      }
    }

    // now check static walkable entities
    int low = rg->get_low_xsorted_entity( pos.x );
    if ( low >= 0 )
    {
      int high = rg->get_high_xsorted_entity( pos.x );
      if ( high >= low )
      {
        ei = rg->get_x_sorted_entities().begin() + low;
        ei_end = rg->get_x_sorted_entities().begin() + high;
        for ( ; ei<=ei_end; ++ei )
        {
          ent = *ei;
          if ( ent->is_walkable() )
          {
            assert( ent->is_stationary() );
            if ( ent->get_colgeom() && ent->get_bounding_box().xz_intersect( pos ) )
            {
              bool hit;
              ent->get_colgeom()->set_owner( ent );
              g_world_ptr->get_origin_entity()->set_radius( ent->get_radius() );

              vector3d local_pos = ent->get_abs_po().fast_inverse_xform(pos);
              vector3d local_low_pos = ent->get_abs_po().fast_inverse_xform(low_pos);

              vector3d local_vdown = ent->get_abs_po().fast_non_affine_inverse_xform(vdown);

              hit = collide_segment_mesh( local_pos, local_low_pos, (cg_mesh*)ent->get_updated_colgeom(), local_entity_ground_loc, local_entity_ground_normal, PP_REAR_CULL, local_vdown );
              if ( hit )
              {
                entity_ground_loc = ent->get_abs_po().fast_8byte_xform( local_entity_ground_loc );
                entity_ground_normal = ent->get_abs_po().fast_8byte_non_affine_xform( local_entity_ground_normal );
                if ( hit_surface_type )
                {
                  // CTT 04/18/00: TODO: obtain surface type info from entity and fill hit_surface_type
                }
              }

              if ( hit )
              {
                if ( entity_ground_normal.y < MIN_GET_ELEVATION_NORMAL_Y )
                  hit = false;
                else if ( (entity_ground_loc.y-outval)*search_direction > 0 )
                {
                  outval = entity_ground_loc.y;
                  normal = entity_ground_normal;
                  last_elevation_entity = ent;
                  low_pos = entity_ground_loc;
                }
              }
            }
          }
        }
      }
    }
  }

  proftimer_get_elevation.stop();

  return outval;
}


// Return vertical distance (negative below, positive above) of nearest surface
// of given type within given region; optionally, output surface normal.
rational_t terrain::get_vertical_distance( const vector3d& p,
                                           region_node* r,
                                           surface_type_t surface_type,
                                           vector3d* normal )
{
  rational_t dist = -10000;
  if ( !r )
  {
    // need a region to search
    sector* s = find_sector( p );
    if ( s )
      r = s->get_region();
    else
      return dist;
  }

  // first check region's terrain mesh
  region* rg = r->get_data();
  vector3d p1 = p + vector3d(0,1000,0);
  vector3d p2 = p - vector3d(0,1000,0);

  // for now, only supports WATER
  if ( surface_type == WATER )
  {
    int low_idx = rg->get_low_water_index( p.x );
    if ( low_idx >= 0 )
    {
      int high_idx = rg->get_high_water_index( p.x );
      if ( low_idx <= high_idx )
      {
        vector<cface_replacement>::const_iterator cfi = rg->get_sorted_water().begin() + low_idx;
        vector<cface_replacement>::const_iterator cfi_end = rg->get_sorted_water().begin() + high_idx;
        for ( ; cfi<=cfi_end; ++cfi )
        {
          const cface_replacement& f = *cfi;
          rational_t fr = f.get_radius();
          vector3d xz_diff = p - f.get_center();
          if ( xz_diff.x*xz_diff.x + xz_diff.z*xz_diff.z < fr*fr )
          {
            // within XZ radius of a cface
            vector3d hit_loc;
            if ( collide_polygon_segment( f.rF, f.pP, p1, p2, hit_loc ) )
            {
              rational_t d = hit_loc.y - p.y;
              if ( __fabs(d) < __fabs(dist) )
              {
                dist = d;
                if ( normal )
                  *normal = f.get_normal();
              }
            }
          }
        }
      }
    }
  }

  return dist;
}



sector* terrain::find_sector(const vector3d& pos) const
{
  return tree->find_sector(pos);
}

void terrain::optimize()
{
  for (int i=regions.size(); --i>=0; )
  {
    regions[i]->optimize();
  }

  // Causes some bp_tree memory to be preallocated.  (dc 03/27/02)
  vector3d dummy;
    // PLEASE don't use nan!
  dummy.x=0.0f;
  dummy.y=0.0f;
  dummy.z=0.0f;
  tree->in_world(dummy, 0, dummy, dummy, dummy);
}
