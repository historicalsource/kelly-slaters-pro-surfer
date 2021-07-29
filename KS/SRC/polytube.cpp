#include "global.h"

#include "polytube.h"
#include "profiler.h"
#include "game.h"
#include "geomgr.h"
#include "b_spline.h"
#include "matfac.h"
#include "material.h"
#include "vertwork.h"
#include "ksngl.h"	// For KSNGL_CreateScratchMesh (dc 06/06/02)
#ifdef TARGET_GC
#include "ngl.h"
#endif

#include "ngl_support.h"

#ifdef NGL

  bool g_render_polytubes = true;

  void render_polytube(const vector<vector3d> &pts, rational_t radius, int num_sides, const color32 &col, mat_fac *the_material, rational_t tiles_per_meter, const matrix4x4 &the_matrix, rational_t max_length)
  {
    #define NUM_VERTS_PER_SEG           6
    #define MAX_POLYTUBE_SIDES          60

    if ( !g_render_polytubes )
      return;

    int num_points = pts.size();
    if(num_points < 2)
      return;

    if(num_sides < 3)
      num_sides = 3;
    if(num_sides > MAX_POLYTUBE_SIDES)
      num_sides = MAX_POLYTUBE_SIDES;

    int num_segments = (num_points - 1);
    int verts_per_seg = NUM_VERTS_PER_SEG*num_sides;

    static rational_t sinx[60], cosx[60];
    rational_t inc = (2.0f*PI)/(rational_t)num_sides;
    for(int i=0; i<num_sides; ++i)
      fast_sin_cos_approx((inc*i), &sinx[i], &cosx[i]);

    u_int newcol = color32( col.get_red() / 2, col.get_green() / 2, col.get_blue() / 2, col.get_alpha() / 2 ).to_ulong();
    int total_verts = num_segments*verts_per_seg;
	assert(false);	// If we use ScratchMesh here, we should make it locked (dc 08/23/01)
    KSNGL_CreateScratchMesh( total_verts, the_material->get_ngl_material(), false );

    rational_t lastv = 0.0f;
    rational_t ulen = 1.0f / num_sides;

    vector<vector3d>::const_iterator pti_end = pts.end();
    vector<vector3d>::const_iterator pti = pts.begin();
    vector<vector3d>::const_iterator pti2 = pti;
    ++pti2;

    rational_t total_length = 0.0f;
    int vertex = 0;
    for( ; pti2 < pti_end && (max_length < 0.0f || total_length < max_length); ++pti, ++pti2)
    {
      vector3d seg_pt0 = *pti;
      vector3d seg_pt1 = *pti2;

      vector3d dir = seg_pt1 - seg_pt0;
      rational_t length = dir.length();

      if(length > 0.0f)
      {
        dir /= length;

        vector3d fake_forward = ZVEC;
        if(__fabs(dir.z) >= 0.9f)
          fake_forward = YVEC;

        vector3d right = cross(dir, fake_forward);
        right.normalize();

        vector3d forward = cross(right, dir);
        forward.normalize();

        rational_t rad_capper = 0.1f;
        length += radius*2.0f*rad_capper;

        if(max_length >= 0.0f && (length+total_length) > max_length)
          length = max_length - total_length;

        rational_t lastu = 0.0f;

        seg_pt0 -= (dir*(rad_capper*radius));
        seg_pt1 = seg_pt0 + (dir*length);
        rational_t vlen = length*tiles_per_meter;

        for(int i=0; i<num_sides; ++i)
        {
          int j = i+1;
          if(j >= num_sides)
            j = 0;

          vector3d norm1 = (cosx[i]*forward + sinx[i]*right);
          norm1.normalize();

          vector3d norm2 = (cosx[j]*forward + sinx[j]*right);
          norm2.normalize();

          vector3d pt1 = seg_pt0 + (norm1 * radius);
          vector3d pt2 = seg_pt0 + (norm2 * radius);
          vector3d pt3 = seg_pt1 + (norm1 * radius);
          vector3d pt4 = seg_pt1 + (norm2 * radius);

          nglMeshWriteStrip( 3 );
          nglMeshWriteVertexPNCUV( pt1.x, pt1.y, pt1.z, norm1.x, norm1.y, norm1.z, newcol, lastu, lastv );
          nglMeshWriteVertexPNCUV( pt2.x, pt2.y, pt2.z, norm2.x, norm2.y, norm2.z, newcol, lastu+ulen, lastv );
          nglMeshWriteVertexPNCUV( pt3.x, pt3.y, pt3.z, norm1.x, norm1.y, norm1.z, newcol, lastu, lastv+vlen );

          nglMeshWriteStrip( 3 );
          nglMeshWriteVertexPNCUV( pt2.x, pt2.y, pt2.z, norm2.x, norm2.y, norm2.z, newcol, lastu+ulen, lastv );
          nglMeshWriteVertexPNCUV( pt4.x, pt4.y, pt4.z, norm2.x, norm2.y, norm2.z, newcol, lastu+ulen, lastv+vlen );
          nglMeshWriteVertexPNCUV( pt3.x, pt3.y, pt3.z, norm1.x, norm1.y, norm1.z, newcol, lastu, lastv+vlen );

          vertex += 6;

          lastu += ulen;
        }

        total_length += length;

        lastv += vlen;
      }
    }

//    #pragma todo("Remove this and replace with num verts used (when Wade finishes it)")
    vector3d garbage_pt = *pti;
    while(vertex < total_verts)
    {
      nglMeshWriteStrip( 3 );
      nglMeshWriteVertexPC( garbage_pt.x, garbage_pt.y, garbage_pt.z, 0 );
      nglMeshWriteVertexPC( garbage_pt.x, garbage_pt.y, garbage_pt.z, 0 );
      nglMeshWriteVertexPC( garbage_pt.x, garbage_pt.y, garbage_pt.z, 0 );
      vertex += 3;
    }

//#pragma todo("This won't work universally, only for lines:  jdf 4-2-01")
    // use begin and end verts for sphere.
    vector3d center = *(pts.begin());
    center += *(pts.end()-1);
    center *= 0.5f;
    vector3d radline = *(pts.begin()) - center;
    float nglradius = radline.length();
    nglVector nglcenter;
    nglcenter[0] = center.x;
    nglcenter[1] = center.y;
    nglcenter[2] = center.z;
    nglcenter[3] = 1.0f;
    nglMeshSetSphere( nglcenter, nglradius );
    // send model
    START_PROF_TIMER( proftimer_render_add_mesh );
    nglListAddMesh( nglCloseMesh(), native_to_ngl( the_matrix ), NULL);
    STOP_PROF_TIMER( proftimer_render_add_mesh );

    #undef NUM_STATIC_RENDER_VERTICES
    #undef NUM_VERTS_PER_SEG
    #undef MAX_POLYTUBE_SIDES
  }

#else

  void render_polytube(const vector<vector3d> &pts, rational_t radius, int num_sides, const color32 &col, mat_fac *the_material, rational_t tiles_per_meter, const matrix4x4 &the_matrix, rational_t max_length)
  {
    #define NUM_STATIC_RENDER_VERTICES  4096
    #define NUM_VERTS_PER_SEG           6
    #define MAX_POLYTUBE_SIDES          (NUM_STATIC_RENDER_VERTICES/NUM_VERTS_PER_SEG)

    static bool render_indices_initted = false;
    static uint16 render_indices[NUM_STATIC_RENDER_VERTICES];
    if(!render_indices_initted)
    {
      render_indices_initted = true;
      for(int i=0; i<NUM_STATIC_RENDER_VERTICES; ++i)
        render_indices[i] = i;
    }

    int num_points = pts.size();
    if(num_points < 2)
      return;

    if(num_sides < 3)
      num_sides = 3;
    if(num_sides > MAX_POLYTUBE_SIDES)
      num_sides = MAX_POLYTUBE_SIDES;

    int num_segments = (num_points - 1);
    int verts_per_seg = NUM_VERTS_PER_SEG*num_sides;
    int segs_per_flush = NUM_STATIC_RENDER_VERTICES / verts_per_seg;

    assert(segs_per_flush > 0);

    static rational_t sinx[60], cosx[60];
    rational_t inc = (2.0f*PI)/(rational_t)num_sides;
    for(int i=0; i<num_sides; ++i)
      fast_sin_cos_approx((inc*i), &sinx[i], &cosx[i]);

    matrix4x4 oldmatrix = geometry_manager::inst()->xforms[geometry_manager::XFORM_LOCAL_TO_WORLD];
    geometry_manager::inst()->set_local_to_world(the_matrix);

    if(the_material == NULL)
      g_game_ptr->get_blank_material()->send_context(0, MAP_DIFFUSE);
    else
      the_material->send_context(0);

    vert_workspace.lock(segs_per_flush*verts_per_seg);
    hw_rasta_vert_lit* vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();

    rational_t lastv = 0.0f;
    rational_t ulen = 1.0f / num_sides;

    vector<vector3d>::const_iterator pti_end = pts.end();
    vector<vector3d>::const_iterator pti = pts.begin();
    vector<vector3d>::const_iterator pti2 = pti;
    ++pti2;

    int seg_count = 0;
    rational_t total_length = 0.0f;
    for( ; pti2 < pti_end && (max_length < 0.0f || total_length < max_length); ++pti, ++pti2)
    {
      vector3d seg_pt0 = *pti;
      vector3d seg_pt1 = *pti2;

      vector3d dir = seg_pt1 - seg_pt0;
      rational_t length = dir.length();

      if(length > 0.0f)
      {
        dir /= length;

        vector3d fake_forward = ZVEC;
        if(__fabs(dir.z) >= 0.9f)
          fake_forward = YVEC;

        vector3d right = cross(dir, fake_forward);
        right.normalize();

        vector3d forward = cross(right, dir);
        forward.normalize();

        rational_t rad_capper = 0.1f;
        length += radius*2.0f*rad_capper;

        if(max_length >= 0.0f && (length+total_length) > max_length)
          length = max_length - total_length;

        rational_t lastu = 0.0f;

        seg_pt0 -= (dir*(rad_capper*radius));
        seg_pt1 = seg_pt0 + (dir*length);
        rational_t vlen = length*tiles_per_meter;

        for(int i=0; i<num_sides; ++i)
        {
          int j = i+1;
          if(j >= num_sides)
            j = 0;

          vector3d pt1 = seg_pt0 + ((cosx[i]*forward + sinx[i]*right) * radius);
          vector3d pt2 = seg_pt0 + ((cosx[j]*forward + sinx[j]*right) * radius);
          vector3d pt3 = seg_pt1 + ((cosx[i]*forward + sinx[i]*right) * radius);
          vector3d pt4 = seg_pt1 + ((cosx[j]*forward + sinx[j]*right) * radius);

          vert_it->set_xyz(pt1);
          vert_it->diffuse = col;
          vert_it->tc[0] = texture_coord(lastu,lastv);
          ++vert_it;
          vert_it->set_xyz(pt2);
          vert_it->diffuse = col;
          vert_it->tc[0] = texture_coord(lastu+ulen,lastv);
          ++vert_it;
          vert_it->set_xyz(pt3);
          vert_it->diffuse = col;
          vert_it->tc[0] = texture_coord(lastu,lastv+vlen);
          ++vert_it;
          vert_it->set_xyz(pt2);
          vert_it->diffuse = col;
          vert_it->tc[0] = texture_coord(lastu+ulen,lastv);
          ++vert_it;
          vert_it->set_xyz(pt4);
          vert_it->diffuse = col;
          vert_it->tc[0] = texture_coord(lastu+ulen,lastv+vlen);
          ++vert_it;
          vert_it->set_xyz(pt3);
          vert_it->diffuse = col;
          vert_it->tc[0] = texture_coord(lastu,lastv+vlen);
          ++vert_it;

          lastu += ulen;
        }

        total_length += length;

        ++seg_count;
        if(seg_count >= segs_per_flush)
        {
          vert_workspace.unlock();

          hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, seg_count*verts_per_seg, render_indices, seg_count*verts_per_seg);

          vert_workspace.lock(-1);
          vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();
          seg_count = 0;
        }


        lastv += vlen;
      }
    }

    vert_workspace.unlock();

    if(seg_count > 0)
      hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, seg_count*verts_per_seg, render_indices, seg_count*verts_per_seg);

    geometry_manager::inst()->set_local_to_world(oldmatrix);

    #undef NUM_STATIC_RENDER_VERTICES
    #undef NUM_VERTS_PER_SEG
    #undef MAX_POLYTUBE_SIDES
  }

#endif






















polytube::polytube( const entity_id& _id, unsigned int _flags )
  : entity( _id, _flags )
{
  init();
  flavor = ENTITY_POLYTUBE;
}

polytube::polytube( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor,
        unsigned int _flags )
  : entity(fs, _id, _flavor, _flags)
{
  init();
}


polytube::~polytube()
{
  if ( my_material != NULL )
  {
    delete my_material;
    my_material = NULL;
  }
}

void polytube::init()
{
  my_material = NULL;

  tiles_per_meter = 1.0f;
  num_sides = 5;
  tube_radius = 0.025f;
  use_spline = true;
  entity::set_flag(EFLAG_GRAPHICS, true);
  max_length = -1.0f;
}



entity* polytube::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  polytube* newit = NEW polytube( _id, _flags );
  newit->copy_instance_data( *((polytube *)this) );
  return (entity*)newit;
}

void polytube::copy_instance_data( const polytube& b )
{
  set_material(b.get_material());

  spline = b.spline;
  tiles_per_meter = b.tiles_per_meter;
  num_sides = b.num_sides;
  tube_radius = b.radius;
  use_spline = b.use_spline;
  max_length = b.max_length;

  entity::copy_instance_data(b);
}


render_flavor_t polytube::render_passes_needed() const
{
  render_flavor_t passes = 0;

  if ( (my_material && my_material->is_translucent()) || render_color.get_alpha() < 0xFF)
    passes |= RENDER_TRANSLUCENT_PORTION;
  else
    passes |= RENDER_OPAQUE_PORTION;

  return passes;
}

void polytube::set_material( const stringx& file )
{
  if(my_material == NULL)
    my_material = NEW mat_fac();

  my_material->load_material(file);
}

void polytube::set_material(mat_fac *mat)
{
  if(mat != NULL)
  {
    if(my_material == NULL)
      my_material = NEW mat_fac(*mat);
    else
      *my_material = *mat;
  }
  else
  {
    if(my_material != NULL)
    {
      delete my_material;
      my_material = NULL;
    }
  }
}



void polytube::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
#ifdef NGL
  my_material->get_ngl_material()->MapBlendMode = NGLBM_BLEND;
  my_material->get_ngl_material()->Flags |= NGLMAT_ALPHA;
  my_material->get_ngl_material()->Flags |= NGLMAT_ANTIALIAS;
#endif

  render_polytube(use_spline ? spline.get_curve_pts() : spline.get_control_pts(), tube_radius, num_sides, render_color, my_material, tiles_per_meter, get_abs_po().get_matrix(), max_length);
}






