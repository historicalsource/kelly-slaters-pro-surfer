////////////////////////////////////////////////////////////////////////
// Colgeom.cpp
//
//  Collision geometry routines.
////////////////////////////////////////////////////////////////////////

#include "global.h"

#include "osalloc.h"

//!#include "character.h"
#include "colgeom.h"
#include "pmesh.h"
//#include "colcap.h"
#include "collide.h"
#include "colmesh.h"
#include "face.h"
#include "oserrmsg.h"
#include "hwmath.h"
//!#include "rigid.h"
#include "profiler.h"
#include "terrain.h"
//#include "fxman.h"
#include "capsule.h"
#include "vsplit.h"
#include "osdevopts.h"

#include "physical_interface.h"

////////////////////////////////////////////////////////////////////////
//  Globals
////////////////////////////////////////////////////////////////////////
vectorvector hit_list;
vectorvector normal_list1;
vectorvector normal_list2;

instance_bank<cg_mesh> cg_mesh_bank;


void COLGEOM_stl_prealloc( void )
{
	vector3d dummy;
	for ( int i=0; i<256; i++ )
	{
    hit_list.push_back( dummy );
    normal_list1.push_back( dummy );
    normal_list2.push_back( dummy );
	}
  hit_list.resize(0);
  normal_list1.resize(0);
  normal_list2.resize(0);
}





////////////////////////////////////////////////////////////////////////
//
//  collsion_geometry
//
////////////////////////////////////////////////////////////////////////

collision_geometry::collision_geometry()
{
  valid = false;
}

collision_geometry::collision_geometry(const collision_geometry &b)
{
  owner = b.owner;
  valid = b.valid;
}


collision_geometry::~collision_geometry()
{
}


bool collision_geometry::collides( collision_geometry* g1,
                                   collision_geometry* g2,
                                   vectorvector* hit_list,
                                   vectorvector* normal_list1,
                                   vectorvector* normal_list2,
                                   unsigned int ct,
                                   const vector3d& rel_vel,
								   cface * hitFace)
{
  bool hit = false;
  rational_t dist;

  if ( g1 && g2 )
  {
    assert ( g1->valid && g2->valid );
    rational_t diff = g1->get_radius() + g2->get_radius();
    if ( (g1->get_abs_position()-g2->get_abs_position()).length2() < diff*diff )
    {
      int t1 = g1->get_type();
      int t2 = g2->get_type();

      if ( t1!=NONE && t2!=NONE )
      {
        switch ( t1 | (t2<<8) )
        {
          case CAPSULE | (CAPSULE<<8):
            hit = collide_capsule_capsule(((collision_capsule *)g1)->get_abs_capsule(),
                                          ((collision_capsule *)g2)->get_abs_capsule(),
                                          hit_list, normal_list1, normal_list2, dist, ct, rel_vel);
            break;
          case CAPSULE | (MESH<<8):
            if ((ct&PP_FULL_MESH)!=0)
              hit = collide_capsule_full_mesh((collision_capsule *) g1,(cg_mesh *) g2,
                                              hit_list, normal_list1, normal_list2, ct, rel_vel);
            else
              hit = collide_capsule_mesh((collision_capsule *) g1,(cg_mesh *) g2,
                                         hit_list, normal_list1, normal_list2, ct, rel_vel);
            break;
          case MESH | (CAPSULE<<8):
            if ((ct&PP_FULL_MESH)!=0)
              hit = collide_capsule_full_mesh((collision_capsule *) g2, (cg_mesh *) g1,
                                              hit_list, normal_list2, normal_list1, ct, -rel_vel);
            else
              hit = collide_capsule_mesh((collision_capsule *) g2, (cg_mesh *) g1,
                                         hit_list, normal_list2, normal_list1, ct, -rel_vel);
            break;
          case MESH | (MESH<<8):
            hit = collide_mesh_mesh((cg_mesh *) g1,(cg_mesh *) g2, hit_list, normal_list1, normal_list2, ct, rel_vel, hitFace);
            break;
        }
      }
    }
  }
  return hit;
}


rational_t collision_geometry::distance_between_cores(const collision_geometry* g1,
                                                      const collision_geometry* g2, vector3d * core_diff)
{
  vectorvector dummy_hit_list;
  vectorvector dummy_normal_list1;
  vectorvector dummy_normal_list2;

  bool hit = false;
  rational_t distance = 9999.0f;
  unsigned int ct = 0;
  vector3d rel_vel = ZEROVEC;

  if ( g1 && g2 )
  {
    assert ( g1->valid && g2->valid );
    rational_t diff = g1->get_radius() + g2->get_radius();
    if (core_diff)
      *core_diff = g1->get_abs_position()-g2->get_abs_position();
    distance = (g1->get_abs_position()-g2->get_abs_position()).length();
    if ( distance < diff )
    {
      int t1 = g1->get_type();
      int t2 = g2->get_type();

      if ( t1!=NONE && t2!=NONE )
      {
        switch ( t1 | (t2<<8) )
        {
          case CAPSULE | (CAPSULE<<8):
            // this function replaces distance with a more sophisticated computation.
            hit = collide_capsule_capsule(((collision_capsule *)g1)->get_abs_capsule(),((collision_capsule *)g2)->get_abs_capsule(), &dummy_hit_list, &dummy_normal_list1, &dummy_normal_list2, distance, ct, rel_vel, core_diff);
            break;
          case CAPSULE | (MESH<<8):
          case MESH | (CAPSULE<<8):
          case MESH | (MESH<<8):
            break;
        }
      }
    }
  }
  return distance;
}


void collision_geometry::xform(po const & the_po)
{
  valid = true;
}


/*
////////////////////////////////////////////////////////////////////////
//
//  cg_capsule
//
////////////////////////////////////////////////////////////////////////

cg_capsule::cg_capsule() : collision_geometry()
{
}


cg_capsule::~cg_capsule()
{
}


collision_geometry* cg_capsule::make_instance( entity* _owner ) const
{
return NEW cg_capsule( _owner );
}


unsigned int cg_capsule::get_type()
{
return CAPSULE;
}


void cg_capsule::xform(po & the_po)
{
collision_geometry::xform(the_po);
// not implemenmted yet...
assert(0);
}


vector3d cg_capsule::get_closest_point_along_dir( const vector3d& axis ) const
{
// Not implemented yet <<<<
assert(0);
return ZEROVEC;
}

*/

////////////////////////////////////////////////////////////////////////
//
//  cg_mesh
//
////////////////////////////////////////////////////////////////////////

cg_mesh::cg_mesh() : collision_geometry()
{
  flags = FLAG_ENTITY_COLLISION;
  radius = 0;
}

cg_mesh::~cg_mesh()
{
  for (int i=0;i<(int)materials.size();++i)
  {
    material_bank.delete_instance( materials[i] );
  }
}

// for reading from NEWENT files
cg_mesh::cg_mesh( chunk_file& fs, bool allow_warnings )
  :   collision_geometry()
{
  read_from_file( fs, allow_warnings );
}


#ifdef TARGET_PS2
typedef vector<wedge,malloc_alloc> wedgevector;
typedef vector<vert_ref,malloc_alloc> vert_refvector;
typedef vector<face,malloc_alloc> facevector;
#else
typedef vector<wedge> wedgevector;
typedef vector<vert_ref> vert_refvector;
typedef vector<face> facevector;
#endif



void cg_mesh::read_from_file( chunk_file& fs, bool allow_warnings )
{
  // <<<< most of the asserts in this file should give warnings about
  //      where file is corrupt to user, that is if this remains a text file
  //      that people touch

  // we load the wedges temporarily so we can hook faces to vertices
  wedgevector local_wedges;
  vert_refvector local_vert_refs_for_wedge_ref;
  facevector local_faces;

  stringx next_string;
  //  char   next_char;
  int vert_count=0;
  int wedge_count=0;
  int face_count=0;
//  int vsplit_count=0;
  flags = FLAG_ENTITY_COLLISION;

  for(;;)
  {
    chunk_flavor flavor_flav;
    serial_in( fs, &flavor_flav );
    // TODO: determine whether NEWENT cg_mesh chunk uses CHUNK_PIVOT
    //if ( CHUNK_PIVOT == flavor_flav )
    //  {
    //  vector3d local_pivot;
    //  serial_in( fs, &local_pivot );
    //  pivot = local_pivot;
    //  pivot_valid = true;
    //  }  else
    if ( CHUNK_NUM_VERTS == flavor_flav )
    {
      int nverts;
      serial_in( fs, &nverts );
      verts = vert_vect(nverts);
      raw_verts = vert_vect(nverts);
    }
    else if ( CHUNK_VERT == flavor_flav )
    {
      vert vert_chunk( fs );
      if (is_pivot_valid())
      {
        vert_chunk = vert(vert_chunk.get_point()-pivot);
      }
      raw_verts[vert_count] = verts[vert_count] = vert_chunk;
      ++vert_count;
    }
    else if ( CHUNK_NUM_WEDGES == flavor_flav )
    {
      int nwedges;
      serial_in( fs, &nwedges );
      local_wedges = wedgevector(nwedges);
      local_vert_refs_for_wedge_ref = vert_refvector(nwedges);
    }
    else if ( CHUNK_WEDGE == flavor_flav )
    {
      wedge wedge_chunk;
      vert_ref mvr;
      serial_in( fs, &wedge_chunk, &mvr );
      local_wedges[wedge_count] = wedge_chunk;
      local_vert_refs_for_wedge_ref[wedge_count] = mvr;
      ++wedge_count;
    }
    else if ( CHUNK_NUM_FACES == flavor_flav )
    {
      int nfaces;
      serial_in( fs, &nfaces );
      local_faces = facevector(nfaces);
      cfaces = cface_vect(nfaces);
    }
    else if ( flavor_flav == CHUNK_FACE ||
              flavor_flav == CHUNK_FACE_WITH_FLAGS ||
              flavor_flav == CHUNK_PROGRESSIVE_FACE )
    {
      face face_chunk( fs, flavor_flav );
      local_faces[face_count] = face_chunk;
      ++face_count;
    }
    else if ( CHUNK_MATERIAL == flavor_flav )
    {
      skip_material_chunk( fs );
    }
    else if( CHUNK_VSPLIT == flavor_flav )
    {
      // skip vsplit
      vsplit garbage;
      serial_in( fs, &garbage );
    }
    else if( CHUNK_MESH_FLAGS == flavor_flav )
    {
      unsigned int flags = 0;
      serial_in( fs, &flags );
      // ignore these
    }
    else if ( CHUNK_END == flavor_flav )
    {
      break;
    }
  	else if( chunk_flavor("version") == flavor_flav )
	  {
		  // ignore for the engine, used for bulk exporters
		  int version;
		  serial_in( fs, &version );
	  }
    else if( chunk_flavor("nomeshes") == flavor_flav )
    {
      int num;
      serial_in( fs, &num);
      assert( num == 1 );
      serial_in( fs, &flavor_flav ); // 'mesh'
      serial_in( fs, &flavor_flav ); // 'meshname'
      stringx name;
      serial_in( fs, &name ); // the meshes name
    }
    else
    {
      error( fs.get_name() + ": unknown chunk '" +
             flavor_flav.to_stringx() + "' in collision mesh" );
    }
  }

  int i;
  for(i=0;i<(int)local_faces.size();++i)
  {
    for (int j=0;j<3;++j)
      cfaces[i].vert_refs[j] = local_vert_refs_for_wedge_ref[local_faces[i].get_wedge_ref(j)];
    cfaces[i].my_material = local_faces[i].my_material;
  }

  // Build the normals
  build_raw_cface_data(fs.get_name(), allow_warnings);

  compute_radius();
}


cg_mesh::cg_mesh(const char * cg_mesh_fname, bool allow_warnings ) : collision_geometry()
{
  chunk_file fs;
  fs.open(cg_mesh_fname);

  read_from_file( fs, allow_warnings );

  fs.close();
}


cg_mesh::cg_mesh(vr_pmesh * my_pmesh, const stringx & mesh_filename, bool allow_warnings ) : collision_geometry()
{
  int i;

  int wedge_count;
  int face_count;
  int vert_count;

  flags = FLAG_ENTITY_COLLISION;

  wedge_count = my_pmesh->get_num_wedges();
  vector<wedge> local_wedges = vector<wedge>(wedge_count);
  vector<vert_ref> local_vert_refs_for_wedge_ref = vector<vert_ref>(wedge_count);
  for (i=0;i<wedge_count;++i)
  {
    local_wedges[i] = my_pmesh->get_wedge(i);
    local_vert_refs_for_wedge_ref[i] = my_pmesh->vert_refs_for_wedge_ref[i];
  }

  face_count = my_pmesh->get_max_faces();
  vector<face> local_faces(face_count);
  cfaces.resize(face_count);
  for (i=0; i<face_count; ++i )
    /*  old interface:
    local_faces[i] = my_pmesh->get_face(i);*/
    local_faces[i] = face( my_pmesh->get_wedge_ref( i, 0 ),
                           my_pmesh->get_wedge_ref( i, 1 ),
                           my_pmesh->get_wedge_ref( i, 2 ),
                           my_pmesh->get_material_ref( i ),
                           my_pmesh->get_face_flags( i ) );

  vert_count = my_pmesh->get_num_verts();
  verts.resize(vert_count);
  raw_verts.resize(vert_count);
  for (i=0;i<vert_count;++i)
  {
    raw_verts[i] = verts[i] = my_pmesh->get_vert(i);
  }


  for(i=0; i<(int)local_faces.size(); ++i)
  {
    for (int j=0;j<3;++j)
      cfaces[i].vert_refs[j] = local_vert_refs_for_wedge_ref[local_faces[i].get_wedge_ref(j)];
    cfaces[i].my_material = local_faces[i].my_material;
    if ( local_faces[i].is_cosmetic() )
      cfaces[i].set_flag( CFACE_FLAG_COSMETIC, true );
    if ( local_faces[i].is_water() )
      cfaces[i].set_flag( CFACE_FLAG_WATER, true );
    if ( local_faces[i].is_vegetation() )
      cfaces[i].set_flag( CFACE_FLAG_VEGETATION, true );
  }

  // Build the normals
  // We intentionally skip the bad normal reporting in this case
  // to avoid flagging things thatren't really used as collision
  // geometry
  build_raw_cface_data(mesh_filename, allow_warnings);

  compute_radius();
}



cg_mesh::cg_mesh(const VisRepList& visreps, const stringx & mesh_filename, bool allow_warnings ) : collision_geometry()
{
  int i;

  flags = FLAG_ENTITY_COLLISION;

  int totfaces=0;
  int totverts=0;

  VisRepList::const_iterator vri;
  for (vri=visreps.begin(); vri!=visreps.end(); ++vri)
  {
    vr_pmesh* my_pmesh = (vr_pmesh*)(*vri);
    int face_count = my_pmesh->get_max_faces();
    totfaces+=face_count;
    int vert_count = my_pmesh->get_num_verts();
    totverts+=vert_count;
  }

  cfaces.resize(totfaces);
  verts.resize(totverts);
  raw_verts.resize(totverts);

  int tf=0; // total faces so far
  int tv=0; // total verts so far

  for (vri=visreps.begin(); vri!=visreps.end(); ++vri)
  {
    vr_pmesh* my_pmesh = (vr_pmesh*)(*vri);

    int wedge_count;
    int face_count;
    int vert_count;

    wedge_count = my_pmesh->get_num_wedges();
    vector<wedge> local_wedges(wedge_count);
    vector<vert_ref> local_vert_refs_for_wedge_ref(wedge_count);
    for (i=0;i<wedge_count;++i)
    {
      local_wedges[i] = my_pmesh->get_wedge(i);
      local_vert_refs_for_wedge_ref[i] = my_pmesh->vert_refs_for_wedge_ref[i];
    }

    face_count = my_pmesh->get_max_faces();
    vector<face> local_faces(face_count);
    for (i=0; i<face_count; ++i )
      local_faces[i] = face( my_pmesh->get_wedge_ref( i, 0 ),
                             my_pmesh->get_wedge_ref( i, 1 ),
                             my_pmesh->get_wedge_ref( i, 2 ),
                             my_pmesh->get_material_ref( i ),
                             my_pmesh->get_face_flags( i ) );

    vert_count = my_pmesh->get_num_verts();
    for (i=0;i<vert_count;++i)
    {
      raw_verts[tv+i] = verts[tv+i] = my_pmesh->get_vert(i);
    }

    for(i=0; i<face_count; ++i)
    {
      for (int j=0;j<3;++j)
        cfaces[tf+i].vert_refs[j] = tv+local_vert_refs_for_wedge_ref[local_faces[i].get_wedge_ref(j)];
      cfaces[i].my_material = local_faces[i].my_material;
      if ( local_faces[i].is_cosmetic() )
        cfaces[tf+i].set_flag( CFACE_FLAG_COSMETIC, true );
      if ( local_faces[i].is_water() )
        cfaces[tf+i].set_flag( CFACE_FLAG_WATER, true );
      if ( local_faces[i].is_vegetation() )
        cfaces[tf+i].set_flag( CFACE_FLAG_VEGETATION, true );
    }
    tf+=face_count;
    tv+=vert_count;
  }

  // Build the normals
  // We intentionally skip the bad normal reporting in this case
  // to avoid flagging things that aren't really used as collision
  // geometry
  build_raw_cface_data(mesh_filename, allow_warnings);

  compute_radius();
}


collision_geometry* cg_mesh::make_instance( entity* ent ) const
{
  cg_mesh* newcgmesh = NEW cg_mesh;
  newcgmesh->owner = ent;
  newcgmesh->valid = valid;
  newcgmesh->raw_verts = raw_verts;
  newcgmesh->verts = verts;
  newcgmesh->cfaces = cfaces;
  newcgmesh->cface_xmin = cface_xmin;
  newcgmesh->lookup_low = lookup_low;
  newcgmesh->lookup_high = lookup_high;
  for (int i=0; i<(int)materials.size(); ++i)
  {
    newcgmesh->materials.push_back(material_bank.new_instance(get_material(i)));
  }
  newcgmesh->pivot = pivot;
  newcgmesh->radius = radius;
  newcgmesh->set_flag(FLAG_PIVOT_VALID,is_pivot_valid());
  return newcgmesh;
}


unsigned int cg_mesh::get_type() const
{
  return MESH;
}


void cg_mesh::xform(po const & the_po)
{
  int i;
  collision_geometry::xform(the_po);
  for (i=0; i<get_num_verts(); ++i)
  {
    vert v(the_po.slow_xform(get_raw_vert(i).get_point()));
    set_vert(i,v);
  }
  for (i=0; i<get_num_cfaces(); ++i)
  {
    vector3d v = the_po.non_affine_slow_xform(get_cface(i).get_raw_normal());
    cfaces[i].set_normal(v);
    v = the_po.slow_xform(get_cface(i).get_raw_center());
    cfaces[i].set_center(v);
  }
}


void cg_mesh::split_xform(po const & po1, po const & po2, int second_po_start_idx)
{
  int i;
  collision_geometry::xform(po1);
  for (i=0; i<second_po_start_idx; ++i)
  {
    vert v(po1.slow_xform(get_raw_vert(i).get_point()));
    set_vert(i,v);
  }
  for (i=second_po_start_idx; i<get_num_verts(); ++i)
  {
    vert v(po2.slow_xform(get_raw_vert(i).get_point()));
    set_vert(i,v);
  }
  // Normals just get po2's orientation
  for (i=0; i<get_num_cfaces(); ++i)
  {
    vector3d v = po2.non_affine_slow_xform(get_cface(i).get_raw_normal());
    cfaces[i].set_normal(v);
    v = po2.slow_xform(get_cface(i).get_raw_center());
    cfaces[i].set_center(v);
  }
}



void cg_mesh::split_xform(po const & po1, po const & po2, po const & po3, int second_po_start_idx, int third_po_start_idx)
{
  int i;
  collision_geometry::xform(po1);
  for (i=0;i<second_po_start_idx;i++)
  {
    vert v(po1.slow_xform(get_raw_vert(i).get_point()));
    set_vert(i,v);
  }
  for (i=second_po_start_idx; i<third_po_start_idx; ++i)
  {
    vert v(po2.slow_xform(get_raw_vert(i).get_point()));
    set_vert(i,v);
  }
  for (i=third_po_start_idx; i<get_num_verts(); ++i)
  {
    vert v(po3.slow_xform(get_raw_vert(i).get_point()));
    set_vert(i,v);
  }
  // Normals just get po2's orientation
  for (i=0; i<get_num_cfaces(); ++i)
  {
    vector3d v = po2.non_affine_slow_xform(get_cface(i).get_raw_normal());
    cfaces[i].set_normal(v);
    v = po2.slow_xform(get_cface(i).get_raw_center());
    cfaces[i].set_center(v);
  }
}


// axis must be normalized.  Checks against transformed verts.
void cg_mesh::get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const
{
  assert (__fabs(axis.length2()-1.0f)<.015f);
  rational_t min_dist = LARGE_DIST;
  int min_idx = -1;

  for (int i=0;i<get_num_verts();++i)
  {
    const vector3d& p = get_vert(i).get_point();
    rational_t dist = dot(p,axis);
    if (dist<min_dist)
    {
      min_idx = i;
      min_dist = dist;
    }
  }
  assert (min_idx!=-1);
  *target = get_vert(min_idx).get_point();
}


void cg_mesh::get_min_extent( vector3d* v ) const
{
  *v = vector3d( FLT_MAX, FLT_MAX, FLT_MAX );
  int i;
  for ( i=0; i<get_num_verts(); ++i )
  {
    const vector3d& p = get_vert(i).get_point();
    if ( p.x < v->x )
      v->x = p.x;
    if ( p.y < v->y )
      v->y = p.y;
    if ( p.z < v->z )
      v->z = p.z;
  }
}

void cg_mesh::get_max_extent( vector3d* v ) const
{
  *v = vector3d( FLT_MIN, FLT_MIN, FLT_MIN );
  int i;
  for ( i=0; i<get_num_verts(); ++i )
  {
    const vector3d& p = get_vert(i).get_point();
    if ( p.x > v->x )
      v->x = p.x;
    if ( p.y > v->y )
      v->y = p.y;
    if ( p.z > v->z )
      v->z = p.z;
  }
}



// computes the normal to each face, putting results into the vector 'normals'
// also computes center and radius
void cg_mesh::build_raw_cface_data(const stringx & meshname, bool allow_warnings)
{
  for (int i=0;i<get_num_cfaces();++i)
  {
    const cface& f1 = get_cface(i);
    const vector3d& v0 = (get_raw_vert(f1.get_vert_ref(0))).get_point();
    const vector3d& v1 = (get_raw_vert(f1.get_vert_ref(1))).get_point();
    const vector3d& v2 = (get_raw_vert(f1.get_vert_ref(2))).get_point();
    vector3d c = (v0+v1+v2)/3;
    rational_t r0 = (c-v0).length2();
    rational_t r1 = (c-v1).length2();
    rational_t r2 = (c-v2).length2();
    rational_t r;

    // r2 = maximum radius squared
    if (r1>r0)
      if (r2>r1)
        r = r2;
      else
        r = r1;
    else
      if (r2>r0)
        r = r2;
      else
        r = r0;
    r = __fsqrt(r);

    vector3d n = cross(v1-v0,v2-v0).normalize();
    #ifdef DEBUG
    if (n.length2()<0.96f && !(meshname==empty_string) && !get_flag(FLAG_GEOM_WARNING_GIVEN) &&
        !os_developer_options::inst()->get_no_warnings() && allow_warnings)
    {
      stringx composite = "Degenerate tri in " + meshname;
      warning(composite);
      set_flag(FLAG_GEOM_WARNING_GIVEN,true);
    }
    #endif
    cfaces[i].set_normal(n);
    cfaces[i].set_raw_normal(n);
    cfaces[i].set_center(c);
    cfaces[i].set_raw_center(c);
    cfaces[i].set_radius(r);
  }
  // now sort the faces along the local X-axis and set up the lookup tables
  sort_cfaces();
}


void cg_mesh::compute_radius()
{
  rational_t radius2 = 0;
  int i;
  for (i=0;i<get_num_verts();++i)
  {
    const vector3d& diff = get_raw_vert(i).get_point();
    rational_t r2 = diff.length2();
    if (r2>radius2)
      radius2 = r2;
  }
  radius = __fsqrt( radius2 );
}


// estimate_physical_properties:
//
//    Computes the mass, volume, center_of_mass, and moment of inertia of the mesh.
//    The algorithm assumes the mesh forms the boundry of a closed region of space.
// Not all meshes have this property, so they may require additional hacks to keep
// from generating wild misestimates.
//
// Uses the collision_geometry, which had better be a mesh.
void cg_mesh::estimate_physical_properties(entity * rb, rational_t material_density)
{
  int i;

  rational_t I_x=0, I_y=0, I_z=0;
  rational_t volume = 0, mass = 0;
  vector3d c_o_m = ZEROVEC;

  vector3d * partial_c_o_m_list = NEW vector3d[get_num_cfaces()];
  rational_t * partial_volume_list = NEW rational_t[get_num_cfaces()];

  // first compute the c_o_m and volume, while recording the elements that make
  // them up.  They will be needed below for computation of I (momemnt of Inertia).
  for (i=0;i<get_num_cfaces();++i)
  {
    rational_t partial_volume = 0;
    vector3d partial_c_o_m(0,0,0);

    const cface * f1 = get_cface_ptr(i);
    const vector3d& v0 = get_raw_vert(f1->get_vert_ref(0)).get_point();
    const vector3d& v1 = get_raw_vert(f1->get_vert_ref(1)).get_point();
    const vector3d& v2 = get_raw_vert(f1->get_vert_ref(2)).get_point();
    vector3d e1 = v1-v0;
    vector3d e2 = v2-v0;

    rational_t edge_length = e1.length();

    if (edge_length>0)
    {
      vector3d normalized_edge = e1*(1/edge_length);
      vector3d edge_perp = e2 - dot(e2,normalized_edge)*normalized_edge;
      rational_t base_area = edge_perp.length()*edge_length*0.5F;
      if (base_area>0)
      {
        vector3d base_perp = cross(e2,e1);
        base_perp.normalize();
        rational_t height = dot(-v0, base_perp);
        partial_volume = height*base_area*(1.0F/3.0F);
      }
    }
    // Center of mass for the terahedron.  I believe this is incorrect, but it's not a
    // terrible approximation (PTA)
    partial_c_o_m = (v0+v1+v2)*(partial_volume*(1.0F/3.0F));

    volume += partial_volume;
    c_o_m += partial_c_o_m;
    partial_volume_list[i] = partial_volume;
    partial_c_o_m_list[i] = partial_c_o_m;
  }

  // rescale
  if (volume==0)
  {
    error(stringx("No mass in collision geometry of file ") + rb->get_filename());
  }
  c_o_m /= volume;
  for (i=0;i<get_num_cfaces();++i)
  {
    partial_c_o_m_list[i] = partial_c_o_m_list[i]/volume;
  }

  // To approximate moments of inertia, here is the twofold strategy
  //    a) approximate the mesh by replacing each terahedron by a sphere of the same
  //    volume at the same center of mass.
  //    b) Compute an I about each principal axis through the c_o_m (I_x, I_y, I_z) and
  //    then for a given axis v (v_x, v_y, v_z), estimate:
  //        I_v = v_x^2*I_x + v_y^2*I_y + v_z^2*I_z
  //
  //  Note:  Volume = mass until we rescale below

  for (i=0;i<get_num_cfaces();++i)
  {
    rational_t partial_volume = partial_volume_list[i];
    vector3d partial_c_o_m = partial_c_o_m_list[i];

    // let sphere_rad be = ((partial_volume*3)/(4*PI)) ^ (1/3)
    int vol_sign = sgn(partial_volume);
    partial_volume = __fabs(partial_volume);
    rational_t sphere_rad = pow( (partial_volume*3.0f)/(4.0f*PI), 1.0f/3.0f );
    vector3d r = partial_c_o_m-c_o_m;

    rational_t partial_I = (2.0f/5.0f)*partial_volume*sphere_rad*sphere_rad;
    rational_t partial_I_x = partial_I + partial_volume*(r.y*r.y + r.z*r.z);
    rational_t partial_I_y = partial_I + partial_volume*(r.z*r.z + r.x*r.x);
    rational_t partial_I_z = partial_I + partial_volume*(r.x*r.x + r.y*r.y);

    I_x += partial_I_x*vol_sign;
    I_y += partial_I_y*vol_sign;
    I_z += partial_I_z*vol_sign;
  }

  // treated material density as 1 (i.e. mass = volume) up
  // 'till now, so we now rescale properly.
  mass = volume * material_density;
  I_x *= material_density;
  I_y *= material_density;
  I_z *= material_density;

  if(rb->has_physical_ifc())
  {
    rb->physical_ifc()->set_mass(mass);
    rb->physical_ifc()->set_I_vector(vector3d(I_x,I_y,I_z));
    rb->physical_ifc()->set_volume(volume);
    rb->physical_ifc()->set_c_o_m(c_o_m);
  }

  rb->set_radius(radius);

  delete[] partial_volume_list;
  delete[] partial_c_o_m_list;
}


// creates a sorting 1-d grid with a resolution SORT_RES meters per tick.
//#pragma fixme("If this number is ever changed to a non-one value, remove below optimizations (search for //OPT//) JDB 05-08-01")
const rational_t SORT_RES = 1.0f;
const rational_t INV_SORT_RES = 1.0f / SORT_RES;


static int compare_cface( const void* x1, const void* x2 )
{
  cface* cf1 = (cface*)x1;
  cface* cf2 = (cface*)x2;
  if( cf1->get_raw_center().x < cf2->get_raw_center().x )
    return -1;
  if( cf1->get_raw_center().x > cf2->get_raw_center().x )
    return 1;
  return 0;
}


void cg_mesh::sort_cfaces()
{
  int i,j;
  assert ( cfaces.size() );

  cface_xmin = FLT_MAX;
  rational_t cface_xmax = -FLT_MAX;

  cface_vect::const_iterator cfi = cfaces.begin();
  cface_vect::const_iterator cfi_end = cfaces.end();
  for ( ; cfi!=cfi_end; ++cfi )
  {
    const cface& cf = *cfi;
    rational_t x = cf.get_raw_center().x;
    rational_t r = cf.get_radius();
    if ( x-r < cface_xmin )
      cface_xmin = x - r;
    if ( x+r > cface_xmax )
      cface_xmax = x + r;
  }

  // sort was freaking out on us.  something to do with memory allocation and MEMTRACK, but who
  // knows what?
  qsort( &(*cfaces.begin()), cfaces.size(), sizeof(cface), compare_cface );

  // Build the lookup
//OPT//  int lookup_size = (int)((cface_xmax - cface_xmin) * INV_SORT_RES + 1);
  int arr_size = (int)((cface_xmax - cface_xmin) + 1);
  lookup_low = intvector(arr_size);
  lookup_high = intvector(arr_size);
  j = 0;
  for ( i=0; i<(int)lookup_low.size(); ++i )
  {
//OPT//    rational_t val = cface_xmin + i*SORT_RES;
    rational_t val = cface_xmin + (rational_t)i;
    while ( j<(int)cfaces.size() && cfaces[j].get_raw_center().x + cfaces[j].get_radius() < val )
      ++j;
    lookup_low[i] = j;
  }
  j = cfaces.size()-1;
  for ( i=lookup_high.size()-1; i>=0; --i )
  {
//OPT//    rational_t val = cface_xmin + (i+1)*SORT_RES;
    rational_t val = cface_xmin + (rational_t)(i+1);
    while ( j>-1 && cfaces[j].get_raw_center().x - cfaces[j].get_radius() > val )
      --j;
    lookup_high[i] = j;
  }
  lookup_size = lookup_low.size()-1;
}


// val is x-coordinate in local space
int cg_mesh::get_low_index( rational_t val ) const
{
  assert( lookup_low.size( ) > 0 );
  assert( (int) (lookup_low.size( )-1) == lookup_size);

//OPT//  int val_tick = (int)((val - cface_xmin) * INV_SORT_RES);
  int val_tick = (int)(val - cface_xmin);

  if ( val_tick < 0 )
    val_tick = 0;
  else if ( val_tick > lookup_size )
    val_tick = lookup_size;

  return lookup_low[val_tick];
}

// val is x-coordinate in local space
int cg_mesh::get_high_index( rational_t val ) const
{
  assert( (int) (lookup_high.size( )-1) == lookup_size);

//OPT//  int val_tick = (int)((val - cface_xmin) * INV_SORT_RES);
  int val_tick = (int)(val - cface_xmin);

  if ( val_tick < 0 )
    val_tick = 0;
  else if ( val_tick > lookup_size )
    val_tick = lookup_size;

  return lookup_high[val_tick];
}


////////////////////////////////////////////////////////////////////////
//
//  cg_none
//
////////////////////////////////////////////////////////////////////////

cg_none::cg_none() : collision_geometry()
{
}


cg_none::~cg_none()
{
}


unsigned int cg_none::get_type() const
{
  return NONE;
}


void cg_none::xform(po const & the_po)
{
  collision_geometry::xform(the_po);
}



