#include "global.h"

#include "algebra.h"
#include "lensflare.h"
#include "profiler.h"
#include "game.h"
#include "geomgr.h"
#include "vertwork.h"
#include "matfac.h"
//#include "material.h"
#include "debug_render.h"
#include "osdevopts.h"
#include "collide.h"
#include "random.h"
#include "blendmodes.h"
#include "forceflags.h"
#include "beachdata.h"
#if defined(TARGET_XBOX)
#include "wds.h"
#include "conglom.h"
#endif /* TARGET_XBOX JIV DEBUG */
#include "kellyslater_controller.h"
#include "wave.h"
#include "ksngl.h"	// For KSNGL_CreateScratchMesh

void lensflare::init()
{
  amount = 0.0f;
  target = 0.0f;

  nflares = 0;

  // start off lensflares randomly offset from time 0.0, this should
  // evenly spread their line of sight checks.
  los_freq = 0.2f;
  los_time = random( los_freq );

  entity::set_flag(EFLAG_GRAPHICS, true);
}

// TODO: Check which constructors are actually required.
lensflare::lensflare( const entity_id& _id, unsigned int _flags )
: entity( _id, ENTITY_LENSFLARE, _flags )
{
  init();
}

lensflare::lensflare( const lensflare& b )
: entity( b )
{
  init();
  copy_instance_data( b );
}

lensflare::lensflare(const entity_id& _id, entity_flavor_t _flavor, unsigned int _flags,
                     float _speed, float _losfreq, float _farrange,
                     int _nflares, stringx *textures, color *theColors, float *offsets, float *sizes)
: entity(_id, _flavor, _flags)
{
  init();

  speed = _speed;
  los_freq = _losfreq;
  farrange = _farrange;
  nflares = _nflares;
  if ( nflares > 8 )
    error( "maximum of 8 flares per lensflare." );

  for ( int i = 0; i < nflares; i++ )
  {

    flares[i].texture = NEW mat_fac();
    flares[i].texture->load_material(textures[i]);
    flares[i].texture->set_blend_mode( NGLBM_ADDITIVE, MAP_DIFFUSE );

    flares[i].col = theColors[i];
    flares[i].offset = offsets[i];
    flares[i].size = sizes[i];

  }

  los_time = random( los_freq );
}

lensflare::lensflare( chunk_file& fs, const entity_id& _id, entity_flavor_t _flavor, unsigned int _flags )
  : entity(_id, _flavor, _flags)
{
  init();

  for ( ;; )
  {
    chunk_flavor ch;
    serial_in( fs, &ch );

    if ( ch == CHUNK_END )
      break;
    else if ( ch == "speed" )
      serial_in( fs, &speed );
    else if ( ch == "losfreq" )
      serial_in( fs, &los_freq );
    else if ( ch == "farrange" )
      serial_in( fs, &farrange );
    else if ( ch == "nflares" )
    {
      serial_in( fs, &nflares );
      if ( nflares > 8 )
        error( "maximum of 8 flares per lensflare." );
      for ( int i = 0; i < nflares; i++ )
      {
        serial_in( fs, &ch );
        assert( ch == "flare" );

        for( ;; )
        {
          serial_in( fs, &ch );
          if ( ch == CHUNK_END )
            break;
          else if ( ch == "texture" )
          {
            stringx name;
            serial_in( fs, &name );
            flares[i].texture = NEW mat_fac();
            flares[i].texture->load_material(
				"levels\\" +
				stringx(BeachDataArray[g_game_ptr->get_beach_id()].folder) + "\\" +
				os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\" +
				name
			);
            flares[i].texture->set_blend_mode( NGLBM_ADDITIVE, MAP_DIFFUSE );
          }
          else if ( ch == "color" )
          {
            serial_in( fs, &flares[i].col.r );
            serial_in( fs, &flares[i].col.g );
            serial_in( fs, &flares[i].col.b );
          }
          else if ( ch == "offset" )
            serial_in( fs, &flares[i].offset );
          else if ( ch == "size" )
            serial_in( fs, &flares[i].size );
          else
            error( "invalid chunk in lens flare." );
        }
      }
    }
    else if (ch == "lensflare")
    {}
    else if (ch == "node")
    {}
  }

  los_time = random( los_freq );
}

lensflare::~lensflare()
{
  for ( int i = 0; i < nflares; i++ )
    if ( flares[i].texture != NULL )
    {
      delete flares[i].texture;
      flares[i].texture = NULL;
    }
}

entity* lensflare::make_instance( const entity_id& _id, unsigned int _flags ) const
{
  lensflare* newit = NEW lensflare( _id, _flags );
  newit->copy_instance_data( *((lensflare *)this) );
  return (entity*)newit;
}

void lensflare::copy_instance_data( const lensflare& b )
{
  speed = b.speed;
  amount = b.amount;
  target = b.target;
  farrange = b.farrange;

  nflares = b.nflares;
  for ( int i = 0; i < nflares; i++ )
  {
    flares[i].texture = NEW mat_fac( *b.flares[i].texture );
    flares[i].offset = b.flares[i].offset;
    flares[i].size = b.flares[i].size;
    flares[i].col = b.flares[i].col;
  }

  los_freq = b.los_freq;
  los_time = random( los_freq );

  entity::copy_instance_data(b);
}

#define INBETWEEN(a,x,y) (x > y?((a > y) && (a < x)):((a < y) && (a > x)))
bool blocked_by_wave()
{
	vector3d cameraPos	= g_game_ptr->get_player_camera(g_game_ptr->get_active_player())->get_rel_po().get_position();
  vector3d sunPos;
	vector3d side0 = *WAVE_GetMarker(WAVE_MarkerProfileSide0);
	vector3d side1 = *WAVE_GetMarker(WAVE_MarkerProfileSide1);
	vector3d top = *WAVE_GetMarker(WAVE_MarkerProfileTop);
  vector3d dir;
	float t, x, y;
	
	sunPos.x = BeachDataArray[g_game_ptr->get_beach_id()].lensflarex;
  sunPos.y = BeachDataArray[g_game_ptr->get_beach_id()].lensflarey;
  sunPos.z = BeachDataArray[g_game_ptr->get_beach_id()].lensflarez;
	
  if (((sunPos.x == 0.0f) && (sunPos.y == 0.0f) && (sunPos.z == 0.0f)))
  {
		// No lensflare
		return true;
	}
	dir = sunPos - cameraPos;
	dir.normalize();

	// A line is A+Bt
	// So the intersection Point P 
	// satisfies A+Bt for some value of t
	// which means it satisfies it for z
	// We know z, so calc t, then x and y
	// t = (P.z -A.z)/B.z
	t = (top.z - cameraPos.z)/dir.z;
	x = cameraPos.x + dir.x*t;
	y = cameraPos.y + dir.y*t;


	if ((y < top.y) && INBETWEEN(x, side0.x, side1.z))
	{
		return true;
	}


	return false;
}
void lensflare::frame_advance( time_value_t t )
{
  float d = fmodf( t, los_freq );
  los_time += d;
  if ( los_time >= los_freq )
  {
	// This code appears designed to fade the lensflare in and out when the sun is occluded by entities.
	// That's not working right now, and it's causing asserts in the collision code, so I'm taking it out.
	// (dc 07/11/02)
/*
    vector3d hit_loc, hit_norm;
    region_node* hit_reg;
    entity* hit_ent;
    bool v = find_intersection(
      get_abs_position(),
      geometry_manager::inst()->get_camera_pos(),
      g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->get_region(),
      FI_COLLIDE_ENTITY,
      &hit_loc, &hit_norm, &hit_reg, &hit_ent );
*/
	bool v = false;	// not blocked
    
    //  v = true;
    if (blocked_by_wave() || g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_board_controller().GetRegion() == WAVE_REGIONTUBE) // lensflare::frame_advance(): lensflare currently only works for 1 player. (multiplayer fixme?)
      v = true;
    target = v ? 0.0f : 1.0f;
    //target = 1;
    los_time -= los_freq;
  }

  // lerp the current amount towards the target based on time.
  float diff = target - amount;
  float dmax = t * speed;
  diff = range(diff,-dmax,dmax);
  amount += diff;

  set_active( true );
}

// basic billboard structure
struct billboard
{
  vector3d pos;     // world space position
  float angle;      // 2d rotation angle
  color32 c;        // color

  float w, h;       // horizontal and vertical size (world units)

//  float u0, v0;     // top left tex coords
//  float u1, v1;     // bottom right tex coords
};

// Creates a view space scratch mesh containing the billboards.
void render_billboards( int num, billboard* bb, mat_fac* mat )
{
  assert( num >= 0 );

  //assert(false);	// If we use ScratchMesh here, we should make it locked (dc 08/23/01)
#ifdef NGL
  KSNGL_CreateScratchMesh( num * 4, mat->get_ngl_material(), false );

  nglMatrix nglWorldToView;
  nglGetMatrix (nglWorldToView, NGLMTX_WORLD_TO_VIEW);
  matrix4x4* w2v = (matrix4x4*)&nglWorldToView;
#else
  matrix4x4* w2v = &geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW];
  geometry_manager::inst()->set_local_to_world(geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_WORLD]);

#define BB_MAX_QUADS 128
#define BB_MAX_VERTS (BB_MAX_QUADS*4)
#define BB_MAX_INDICES (BB_MAX_QUADS*6)

  if ( num >= BB_MAX_QUADS )
    error( "too many billboards rendered at once.\n" );

  vert_workspace.lock(num * 4);
  hw_rasta_vert_lit* vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();

  static bool render_indices_initted = false;
  static uint16 render_indices[BB_MAX_INDICES];
  if(!render_indices_initted)
  {
    render_indices_initted = true;
    for(int i=0; i<BB_MAX_QUADS; ++i)
    {
      render_indices[i * 6 + 0] = i * 4 + 0;
      render_indices[i * 6 + 1] = i * 4 + 1;
      render_indices[i * 6 + 2] = i * 4 + 2;
      render_indices[i * 6 + 3] = i * 4 + 2;
      render_indices[i * 6 + 4] = i * 4 + 1;
      render_indices[i * 6 + 5] = i * 4 + 3;
    }
  }
#endif

  for ( int i = 0; i < num; i++ )
  {
    // find the center in screen space
    vector3d center = xform3d_1( *w2v, bb->pos );

    vector3d xaxis,yaxis;
    if ( bb->angle )
    {
#ifdef TARGET_PS2
      float fSin = fast_sin_lookup( bb->angle );
      float fCos = fast_cos_lookup( bb->angle );
#else
      float fSin, fCos;
      fast_sin_cos_approx( bb->angle, &fSin, &fCos );
#endif
      xaxis = vector3d(  fCos, -fSin, 0 );
      yaxis = vector3d( -fSin, -fCos, 0 );
      xaxis *= bb->w;
      yaxis *= bb->h;
    }
    else
    {
      xaxis = vector3d( bb->w, 0, 0 );
      yaxis = vector3d( 0, -bb->h, 0 );
    }

//    unsigned int c = bb->c.to_ulong();	// Old, platform-dependent way. (dc 07/06/02)
    unsigned int c = NGL_RGBA32(
		2 * bb->c.get_blue(), 
		2 * bb->c.get_green(), 
		2 * bb->c.get_red(), 
		2 * bb->c.get_alpha()
	);	// The 2's and the blue/red switch compensate for the (wrong) way we used to do this.  (dc 07/06/02)

#ifdef NGL
    vector3d p;
    nglMeshWriteStrip( 4 );

    p = center - xaxis - yaxis;
    nglMeshWriteVertexPCUV( p.x, p.y, p.z, c, 0, 1 );

    p = center - xaxis + yaxis;
    nglMeshWriteVertexPCUV( p.x, p.y, p.z, c, 0, 0 );

    p = center + xaxis - yaxis;
    nglMeshWriteVertexPCUV( p.x, p.y, p.z, c, 1, 1 );

    p = center + xaxis + yaxis;
    nglMeshWriteVertexPCUV( p.x, p.y, p.z, c, 1, 0 );
#else
    vert_it->set_xyz(center - xaxis - yaxis);
    vert_it->diffuse = c;
    vert_it->tc[0] = texture_coord(0, 0);
    ++vert_it;
    vert_it->set_xyz(center + xaxis - yaxis);
    vert_it->diffuse = c;
    vert_it->tc[0] = texture_coord(1, 0);
    ++vert_it;
    vert_it->set_xyz(center - xaxis + yaxis);
    vert_it->diffuse = c;
    vert_it->tc[0] = texture_coord(0, 1);
    ++vert_it;
    vert_it->set_xyz(center + xaxis + yaxis);
    vert_it->diffuse = c;
    vert_it->tc[0] = texture_coord(1, 1);
    ++vert_it;
#endif

    bb++;
  }

#ifdef NGL
  // I think this is probably faster than calculating while we build, as long as we don't exceed the cache size.
#ifdef TARGET_XBOX
  nglMeshSetSphere(nglVector(0,0,0,0), 0);	// replace by nglMeshCalcSphere, when that's implemented (dc 06/11/02)
#else
  nglMeshCalcSphere();
#endif
	// Material must be specified at mesh creation time now.  (dc 06/03/02)
//  if( mat ) KSNGL_ScratchSetMaterial( mat->get_ngl_material() );

  nglMatrix nglViewToWorld;
  nglGetMatrix (nglViewToWorld, NGLMTX_VIEW_TO_WORLD);
  START_PROF_TIMER( proftimer_render_add_mesh );
  nglListAddMesh( nglCloseMesh(), nglViewToWorld, NULL );
  STOP_PROF_TIMER( proftimer_render_add_mesh );
#else
  if ( mat )
    mat->send_context( 0, MAP_DIFFUSE, FORCE_ADDITIVE_BLENDING );
  else
    g_game_ptr->get_blank_material()->send_context(0, MAP_DIFFUSE);

  vert_workspace.unlock();
  hw_rasta::inst()->send_indexed_vertex_list( vert_workspace, num * 4, render_indices, num * 6 );
#endif
}

billboard lensflare_bbs[8];

void lensflare::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
//  print_3d_text( get_abs_position(), color32_white, "flare: %f - %f", amount, target );
  if (( amount <= 0.0f)
//	  || WAVE_CameraUnderwater() // should no longer be necessary. (dc 07/06/02)
	  )
    return;

  vector3d eyep = geometry_manager::inst()->get_camera_pos();
  vector3d eyen = geometry_manager::inst()->get_camera_dir();
  vector3d fpos = get_abs_position();
  vector3d f2eye = fpos - eyep;

  float angledot=dot(f2eye, eyen);
  angledot/=eyen.length();
  angledot/=(fpos-eyep).length();
  float myscale = 0;
  if (angledot > .5f)
    myscale = (powf(11.0f, (angledot-.5f)/.5f) - 1)/5;

  float d = dot(eyen, f2eye);
  if (d <= PROJ_NEAR_PLANE_D )
    return;
  // could instead use the viewer pos to compute influence, then multiply by 2.0!
  //float atten = 1.0f-d/min(3.0F*brightness,PROJ_FAR_PLANE_D+1.0f);
  float atten = 1.0f-sqr(d)/sqr(farrange+1.0f);
  if (atten <= 0)
    return;
  atten *= amount;//*0.3f;

//  float ang=PI*0.5F*(eyen.x+eyen.y);


  float sclz=(PROJ_NEAR_PLANE_D+1e-3f)/d;

  for ( int i = 0; i < nflares; i++ )
  {
    billboard* bb = &lensflare_bbs[i];

/*	This doesn't seem to be doing what's intended.  (dc 10/17/01)
    color fade = flares[i].col*atten*farrange;
    fade.rescale();
    bb->c = fade.to_color32();
*/
    vector3d flarepos=lerp(fpos,eyep+eyen*d,flares[i].offset);
    flarepos=lerp(eyep,flarepos,sclz); // move to front plane
    bb->pos = flarepos;

    //float scale=min(atten*brightness*brightness*0.009F,3.0F*nearz);
    bb->w = bb->h = flares[i].size * sclz * myscale*(flares[i].offset + 1);

    bb->angle = cosf(angledot);

	// The regular attentuation didn't work right.  The flares would not become 
	// translucent even when they became very small.  Replaced with simpler 
	// system.  (dc 07/11/02)
	static float lensflare_attenfactor = 4;
	atten = 1 - lensflare_attenfactor * (1 - angledot);
	if (atten < 0) atten = 0;
    color fade = flares[i].col * atten;
    bb->c = fade.to_color32();

    render_billboards( 1, bb, flares[i].texture );
  }
}
