// terrain.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

#ifndef TERRAIN_H
#define TERRAIN_H

#include "pmesh.h"
#include "cface.h"
#include "graph.h"
#include "algebra.h"
#include "light.h"
#include "hwmath.h"
#include "capsule.h"
#include <set>
#include <stack>
#include "colmesh.h"
#include "wds.h"
#include "bound.h"
#include "sphere.h"
#include "hyperplane.h"
#include "hull.h"
#include "portal.h"
#include "region.h"

class automap;
class tree_t;
class branch_vector;



// default return value from get_elevation
const rational_t BOTTOM_OF_WORLD = -10000.0F;

// depth it will check for elevation
const rational_t DEEP = 300.0F;


typedef region_graph::iterator region_iterator;
typedef region_graph::_Pairib region_Pairib;
typedef region_node::iterator edge_iterator;




class sector
{
private:
  region_node* my_region;
public:
  sector() : my_region(NULL) {}
  sector(region_node* n) : my_region(n) {}
  region_node* get_region() const { return my_region; }
};

typedef vector<sector> sector_list;
typedef vector<sector> sector_vector;


// This class partitions R3 into two halfspaces, and is used by the bsp_tree
// template in a hierarchy which recursively divides world space into convex
// subspaces.  (There is a more complex version of class partition3 used in the
// MAX tools; see the TreyarchMods project.)
//
// In a tree which represents solid geometry, any right-hand leaf subspace is
// guaranteed to represent a minimal convex subspace contained by the world
// geometry (and thus, legal space for an entity to inhabit).  We call such
// subspaces "sectors," and provide a sector data pointer for associating
// specific information with sectors in the world.
//
// Note that this class is NOT responsible for allocating or deleting the
// sector data pointer.

enum
{
  FIRST_FACEREF_BITS = 20
};
const unsigned int first_faceref_mask = ((0xffffffff)>>FIRST_FACEREF_BITS);
const unsigned int faceref_count_mask = first_faceref_mask^0xffffffff;

class partition3 : public hyperplane<vector3d>
{
// Types
public:
  typedef hyperplane<vector3d> _H;

  class faceref
  {
  // Data
  private:
    // One mesh per region
    unsigned short region_idx;
    unsigned short face_idx;

  // Constructors
  public:
    faceref()
    {
      region_idx = (unsigned short)-1;
    }

    faceref( unsigned short _mesh_idx,  unsigned short _face_idx)
    {
      region_idx = _mesh_idx;
      face_idx = _face_idx;
    }

  // Methods
  public:
    bool valid() const { return region_idx!=(unsigned short)-1; } 
    face_ref get_face_ref() const { return face_idx; } 
  };
  typedef vector<faceref> faceref_list;

// Data
private:
  sector* my_sector;

// Constructors
public:
  partition3();
  partition3( const partition3& b );
  partition3( const vector3d& c, const vector3d& n, sector* s = NULL );

// Methods
public:
  sector* get_sector() const { return my_sector; }
  //const vector<faceref>& get_facerefs() const { return facerefs; }
};


class terrain
{
// Types
public:
  enum surface_type_t
  {
    WATER,
    VEGETATION
  };

  // this is a list of all the facerefs in the world, pointed to by partition3's/
  vector<partition3::faceref> const & get_facerefs(){return facerefs;}
  void add_to_facerefs(vector<partition3::faceref> _facerefs)
  {
    for (int i=0;i<(int)_facerefs.size();++i)
    {
      facerefs.push_back(_facerefs[i]);
    }
  }
// Data
private:
  vector<partition3::faceref> facerefs;
  region_graph regions_graph;
  region_list regions;
  portal_list portals;
  sector_list sectors;
  tree_t * tree;

// Constructors
  static entity * last_elevation_entity;


public:
  terrain();
  terrain(const stringx& filename);
  ~terrain();

// Methods
public:
  sector* find_sector(const vector3d& pos) const;

  // this creates sorted lists of entities in each region according to their
  // bounding box info
  void sort_entities_within_each_region();

  rational_t get_elevation( const vector3d& p,
                            vector3d& normal,
                            region_node *default_region = NULL,
                            unsigned char* hit_surface_type = NULL );

  // Return vertical distance (negative below, positive above) of nearest surface
  // of given type within given region; optionally, output surface normal.
  rational_t get_vertical_distance( const vector3d& p,
                                    region_node* r,
                                    surface_type_t surface_type,
                                    vector3d* normal = NULL );

  bool find_intersection( const vector3d& p0, const vector3d& p1, vector3d& pi, vector3d& n);
  bool in_world( const vector3d& p, const rational_t r, const vector3d& v, vector3d& n, vector3d& pi);

  void optimize(); // optimizes visreps of all regions (only call after building colgeoms and such)

  region_node* find_region( const stringx& name )
  {
    region_iterator rgi = regions_graph.find( name );
		if ( rgi == regions_graph.end() )
      return NULL;
    return &(*rgi).second;
  }

  const region_list& get_regions() const { return regions; }
  region& get_region(int idx) { return *regions[idx]; }
  int get_num_regions() const { return regions.size(); }

  portal_list& get_portals() { return portals; }

  // treats direction as an XZ 2d vector
  static vector3d get_gradient( const vector3d& normal, const vector3d& direction );

  // returns the ground entity found by the last get_elevation call.  Returns NULL if the elevation
  // was from the terrain.
  static entity * get_last_elevation_entity()
  {
    return last_elevation_entity;
  }

  void deactivate_all_regions()
  {
    for ( region_list::iterator i=regions.begin(); i!=regions.end(); ++i )
      (*i)->set_active( false );
  }
    
// Internal Methods
private:
  void _load_tree(chunk_file& fs);
  void _recursive_load_tree( chunk_file& fs, branch_vector & recursion_stack );

  // serial IO
  friend void serial_in(chunk_file& fs,terrain* ter);
  friend void serial_in( chunk_file& fs, region * r, terrain* ter );
};


const chunk_flavor CHUNK_REGIONS      ("regions");
const chunk_flavor CHUNK_REGION       ("region");
const chunk_flavor CHUNK_PORTALS      ("portals");
const chunk_flavor CHUNK_PORTAL       ("portal");
const chunk_flavor CHUNK_BSP_TREE     ("bsp_tree");
const chunk_flavor CHUNK_BSP_NODES    ("bsp_node");
const chunk_flavor CHUNK_BSP_SECTORS  ("bsp_sect");
const chunk_flavor CHUNK_LEFT         ("left");
const chunk_flavor CHUNK_RIGHT        ("right");
const chunk_flavor CHUNK_NULL         ("null");
const chunk_flavor CHUNK_SECTOR       ("sector");
const chunk_flavor CHUNK_FACEREFS     ("facerefs");


#endif
