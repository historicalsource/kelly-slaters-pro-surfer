////////////////////////////////////////////////////////////////////////////////

// billboard.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// billboard sprites;  sprites that you put in the world

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "osalloc.h"

#include "hwrasterize.h"
#include "app.h"
#include "billboard.h"
#include "camera.h"
#include "game.h"
#include "geomgr.h"
#include "hwrasterize.h"
#include "iri.h"
#include "maxiri.h"
#include "oserrmsg.h"
#include "txtcoord.h"
#include "profiler.h"
#include "vertwork.h"
#include "renderflav.h"
#include "osdevopts.h"
#include "aggvertbuf.h"
#include "matfac.h"
#include "wds.h" // unfortunately.... sigh
#include "ngl_support.h"

#include <algorithm>

#ifdef NGL
rational_t g_NGL_inline_max_x;
rational_t g_NGL_inline_max_y;
rational_t g_NGL_inline_max_z;

rational_t g_NGL_inline_min_x;
rational_t g_NGL_inline_min_y;
rational_t g_NGL_inline_min_z;
#endif

//extern profiler_timer proftimer_render_billboards;
extern profiler_timer proftimer_render_setup_billboards;
extern profiler_timer proftimer_render_c_billboards;
extern profiler_timer proftimer_render_c_axis_billboards;
extern profiler_timer proftimer_render_sendctx_billboards;
extern profiler_timer proftimer_render_draw_billboards;

#ifdef TARGET_PC
extern int g_cur_player_render;
#endif


int g_debug_max_batches = 255;
int g_debug_num_bb_batches = 0;


////////////////////////////////////////////////////////////////////////////////////////////////////
void vr_billboard::flush()  // flush buffered rendering at end of frame
{
	g_world_ptr->get_matvertbufs().flush();
}


static po calc_axis_lock(const vector3d& lock_axis, // in world space, normalized
                         const vector3d& camera_dir, // in world space, normalized
                         const vector3d& point)
{
	vector3d cp = cross(lock_axis,camera_dir);
	float cpl = cp.length();
	if (!cpl)
	{
		po result = po_identity_matrix;
		result.set_position(point);
		return result;
	}
	cp /= cpl;

	return po(cp,lock_axis,cross(cp,lock_axis),point);
}


// include this instead
#include "maxiri.h"
//#define MAX_INSTANCES (VIRTUAL_MAX_VERTS_PER_PRIMITIVE/6)  // unfortunately, you must match this with


extern "C"
{
	instance_render_info * asm_prep_billboards( vr_billboard* me,
		instance_render_info* iri,
		instance_render_info* iri_end,
		hw_rasta_vert** vert_it,
		const matrix4x4& world_to_screen,
		const matrix4x4& start_po,
		float vp_width,
		float vp_height,
		render_flavor_t flavor,
		bool translucent_material);
}


static const texture_coord btc[4]=
{
	texture_coord(0.0f,1.0f),
		texture_coord(0.0f,0.0f),
		texture_coord(1.0f,1.0f),
		texture_coord(1.0f,0.0f)
};

static instance_render_info * c_prep_billboards( vr_billboard* me,
												instance_render_info* iri,
												instance_render_info* iri_end,
#ifdef NGL
												u_int scratch_mesh_id,
												int*  vert_idx,
#else
												hw_rasta_vert_xformed** vert_it_ptr,
#endif
												const matrix4x4& world_to_screen,
												const matrix4x4& start_po,
												float vp_width, // could get from me
												float vp_height, // could get from me
												render_flavor_t flavor,
												bool translucent_material,
												int flags)
{
	//#pragma todo("This is a huge mess.  Once PC is using NGL, get rid of the old stuff. -AC")

	proftimer_render_c_billboards.start();
	int last_frame = iri->frame;

#ifdef NGL
	matrix4x4 w2v;
	nglGetMatrix( native_to_ngl(w2v), NGLMTX_WORLD_TO_VIEW );

	int vertex_idx=*vert_idx;

#if defined(TARGET_PS2)
    // Load up matrix for massive muls
	asm __volatile__(
		"lqc2    vf9,0x0(%0) \n"
		"lqc2    vf10,0x10(%0) \n"
		"lqc2    vf11,0x20(%0) \n"
		"lqc2    vf12,0x30(%0) \n"
		: : "r" (((sceVu0FMATRIX)&w2v.x.x)));

    sceVu0FVECTOR center_pt;
#endif

#else
	rational_t min_z = geometry_manager::inst()->get_min_z();
	rational_t scl_z = geometry_manager::inst()->get_scale_z();

	hw_rasta_vert_xformed* vert_it = *vert_it_ptr;
	float z_proj;
	if (flags&vr_billboard::FORCE_NEAR)
		z_proj = PROJ_FAR_PLANE_D/(PROJ_FAR_PLANE_D-PROJ_NEAR_PLANE_D);
	else
		z_proj = 1.0f;
	float invzoom;
	if (flags&vr_billboard::DISABLE_PERSPECTIVE)
		invzoom = 1.0f;
	else
		invzoom = PROJ_RECIP_ZOOM;

	int total_quads=0;
#endif

	float me_width = me->get_width();
	float me_height = me->get_height();

	rational_t radius = __fsqrt((me_width*me_width) + (me_height*me_height));

	for(;(iri!=iri_end)&&(iri->frame==last_frame);++iri)
	{
#ifndef NGL
		total_quads++;
		if(total_quads>=aggregate_vert_buf::MAXQUADSPERBUF)
		{
			warning( "Particle system using texture %s overloaded 512 quad limit.",
				me->get_alterable_mat().get_material()->get_texture_filename().c_str() );
			break;
		}
		if(((flavor&RENDER_TRANSLUCENT_PORTION)!=0)==(translucent_material||iri->color_scale.c.a<255))
#endif
		{

#ifdef NGL
#if defined(TARGET_PS2)
			asm __volatile__(
				"lqc2    vf13,0x0(%0) \n"
				"vsub.w       vf13,vf13,vf13 \n"
				"vadd.w       vf13,vf13,vf0 \n"
				"vmulax.xyzw	ACC, vf9,vf13 \n"
				"vmadday.xyzw	ACC, vf10,vf13 \n"
				"vmaddaz.xyzw	ACC, vf11,vf13 \n"
				"vmaddw.xyzw	vf14, vf12,vf13 \n"
				"sqc2    vf14,0x0(%1) \n"
				: : "r" (((sceVu0FVECTOR)&iri->get_local_to_world().get_position().x)), "r" (center_pt));
#else
			vector3d cur_position( iri->get_local_to_world().get_position() );
			// make a 2-tri strip in world space
			vector3d center = xform3d_1(w2v, cur_position);
#endif
#else
			vector3d cur_position( iri->get_local_to_world().get_position() );
			// make a 2-tri strip in world space
			vector4d center_view = xform4d(world_to_screen, vector4d(cur_position,1.0f));

			if (center_view.w<=1e-5f) continue;
			center_view.homogenize();

			if (center_view.x<=-160.0f) continue;
			if (center_view.x>= 800.0f) continue;
			if (center_view.y<=-120.0f) continue;
			if (center_view.y>= 600.0f) continue;
#endif

			rational_t scale         = 0.5f * iri->particle_scale;

#ifdef NGL
			//			nglPrintf("WARNING: DISABLE_PERSPECTIVE is not supported (%s,%d)\n", __FILE__, __LINE__);
			// DISABLE_PERSPECTIVE is not supported.  It's probably not too noticeable anyway.
			// Will figure out a way to support it if it's absolutely necessary.  - AC
#else
			rational_t view_scaled_width  = vp_width  * me_width  * scale;
			rational_t view_scaled_height = vp_height * me_height * scale;

			if (!(flags&vr_billboard::DISABLE_PERSPECTIVE))
			{
				float zoomw = center_view.w*invzoom;
				view_scaled_width  *= zoomw;
				view_scaled_height *= zoomw;
			}
#endif

			float rads = iri->get_camera_relative_rotation();

#ifdef NGL
			// $jim - 0.75 makes billboards for PS2 match the size of them on the PC.
			// see me or James Chao or comment * 0.75 out if this makes any bad.
#ifdef TARGET_PS2
			float x_scale = me_width  * scale * 0.75;
			float y_scale = me_height * scale * 0.75;
#else
			float x_scale = me_width  * scale;
			float y_scale = me_height * scale;
#endif
#else
			float x_scale = view_scaled_width;
			float y_scale = view_scaled_height;
#endif

			vector3d xaxis(x_scale, 0.0f, 0.0f), yaxis(0.0f, -y_scale, 0.0f);

			if (rads)
			{
				rational_t fCos,fSin;

				// Bashes vf4-vf8
				fast_sin_cos_approx(rads,&fSin,&fCos);

				xaxis.x = fCos*x_scale;
				xaxis.y = -fSin*x_scale;

				yaxis.x = -fSin*y_scale;
				yaxis.y = -fCos*y_scale;
			}


#ifdef NGL
			//			nglPrintf("WARNING: FORCE_NEAR is not supported (%s,%d)\n", __FILE__, __LINE__);
			//#pragma todo("Once ngl supports Z push factor, add this feature back in. - AC")
#else
			if (flags&vr_billboard::FORCE_NEAR)
			{
				// This is really screwy because it's trying to adjust the observed
				// distance from camera, but doing so *after* the projection matrix
				// and also after the perspective divide.  Ugly.  Working in camera
				// space would make life so much simpler... --Sean
				// Zo/Wo = (near - Zi) * far / (Zi * (near-far)), taken from the projection matrix
				center_view.w = 1.0f/center_view.w; // damn this sux--2 div's!
				center_view.z *= center_view.w;
				center_view.w -= me->get_z_push_factor();
				if (center_view.w<PROJ_NEAR_PLANE_D+0.001f) center_view.w=PROJ_NEAR_PLANE_D+0.001f;
				center_view.z -= me->get_z_push_factor()*z_proj;
				if (center_view.z<0.001f) center_view.z=0.001f;
				center_view.w = 1.0f/center_view.w;
				center_view.z *= center_view.w;
			}

			center_view.z=scl_z*center_view.z+min_z;

			vector4d pp[4];
			pp[0] = center_view + vector4d(-xaxis-yaxis,0.0f);
			pp[1] = center_view + vector4d(-xaxis+yaxis,0.0f);
			pp[2] = center_view + vector4d( xaxis-yaxis,0.0f);
			pp[3] = center_view + vector4d( xaxis+yaxis,0.0f);
#endif

#ifdef NGL
			u_int ngl_color = NGL_RGBA32(iri->color_scale.get_red(),iri->color_scale.get_green(),iri->color_scale.get_blue(),iri->color_scale.get_alpha());

			nglMeshWriteStrip( 4 );

#ifdef TARGET_PS2
			nglMeshWriteVertexPCUV( center_pt[0] - xaxis.x - yaxis.x, center_pt[1] - xaxis.y - yaxis.y, center_pt[2], ngl_color, 0.0f, 0.0f );
			nglMeshWriteVertexPCUV( center_pt[0] + xaxis.x - yaxis.x, center_pt[1] + xaxis.y - yaxis.y, center_pt[2], ngl_color, 1.0f, 0.0f );
			nglMeshWriteVertexPCUV( center_pt[0] - xaxis.x + yaxis.x, center_pt[1] - xaxis.y + yaxis.y, center_pt[2], ngl_color, 0.0f, 1.0f );
			nglMeshWriteVertexPCUV( center_pt[0] + xaxis.x + yaxis.x, center_pt[1] + xaxis.y + yaxis.y, center_pt[2], ngl_color, 1.0f, 1.0f );

			START_PROF_TIMER( proftimer_render_billboard_sphere);
			rational_t scaled_rad = radius*scale;
			if ( (center_pt[0]-scaled_rad) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (center_pt[0]-scaled_rad);
			if ( (center_pt[0]+scaled_rad) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (center_pt[0]+scaled_rad);

			if ( (center_pt[1]-scaled_rad) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (center_pt[1]-scaled_rad);
			if ( (center_pt[1]+scaled_rad) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (center_pt[1]+scaled_rad);

			if ( (center_pt[2]-scaled_rad) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (center_pt[2]-scaled_rad);
			if ( (center_pt[2]+scaled_rad) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (center_pt[2]+scaled_rad);
			STOP_PROF_TIMER( proftimer_render_billboard_sphere);
#else
			nglMeshWriteVertexPCUV( center.x - xaxis.x - yaxis.x, center.y - xaxis.y - yaxis.y, center.z, ngl_color, 0.0f, 0.0f );
			nglMeshWriteVertexPCUV( center.x + xaxis.x - yaxis.x, center.y + xaxis.y - yaxis.y, center.z, ngl_color, 1.0f, 0.0f );
			nglMeshWriteVertexPCUV( center.x - xaxis.x + yaxis.x, center.y - xaxis.y + yaxis.y, center.z, ngl_color, 0.0f, 1.0f );
			nglMeshWriteVertexPCUV( center.x + xaxis.x + yaxis.x, center.y + xaxis.y + yaxis.y, center.z, ngl_color, 1.0f, 1.0f );

			START_PROF_TIMER( proftimer_render_billboard_sphere);
			rational_t scaled_rad = radius*scale;
			if ( (center.x-scaled_rad) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (center.x-scaled_rad);
			if ( (center.x+scaled_rad) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (center.x+scaled_rad);

			if ( (center.y-scaled_rad) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (center.y-scaled_rad);
			if ( (center.y+scaled_rad) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (center.y+scaled_rad);

			if ( (center.z-scaled_rad) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (center.z-scaled_rad);
			if ( (center.z+scaled_rad) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (center.z+scaled_rad);
			STOP_PROF_TIMER( proftimer_render_billboard_sphere);
#endif

			vertex_idx+=4;
#else
			color32 diffuse_color = iri->color_scale;

			// would be better to use indexed vertex lists to keep from duplicating work.
			// PC: 4 vertex indexable instead of 6 vertex list
			vert_it->reset();
			vert_it->set_xyz_rhw(pp[0]);
			vert_it->tc[0] = btc[0];
			vert_it->diffuse = diffuse_color;
			++vert_it;

			vert_it->reset();
			vert_it->set_xyz_rhw(pp[1]);
			vert_it->tc[0] = btc[1];
			vert_it->diffuse = diffuse_color;
			++vert_it;

			vert_it->reset();
			vert_it->set_xyz_rhw(pp[2]);
			vert_it->tc[0] = btc[2];
			vert_it->diffuse = diffuse_color;
			++vert_it;

			vert_it->reset();
			vert_it->set_xyz_rhw(pp[3]);
			vert_it->tc[0] = btc[3];
			vert_it->diffuse = diffuse_color;
			++vert_it;
#endif
	  }
  }
#ifndef NGL
  *vert_it_ptr = vert_it;
#else
  *vert_idx = vertex_idx;
#endif

  proftimer_render_c_billboards.stop();
  return iri;
}

static instance_render_info * c_prep_axis_billboards( vr_billboard* me,
													 instance_render_info* iri,
													 instance_render_info* iri_end,
#ifdef NGL
													 u_int scratch_mesh_id,
													 int* vert_idx,
#else
													 hw_rasta_vert_xformed** vert_it_ptr,
#endif
													 const matrix4x4& world_to_screen,
													 const matrix4x4& start_po,
													 float vp_width,
													 float vp_height,
													 render_flavor_t flavor,
													 unsigned int flags, // could get from me
													 const vector3d& axis_lock, // could get from me
													 bool translucent_material)
{
	proftimer_render_c_axis_billboards.start();
	int last_frame = iri->frame;

#ifdef NGL
	matrix4x4 w2v;
	nglGetMatrix( native_to_ngl(w2v), NGLMTX_WORLD_TO_VIEW );

	int vertex_idx = *vert_idx;
#else
	rational_t min_z=geometry_manager::inst()->get_min_z();
	rational_t scl_z=geometry_manager::inst()->get_scale_z();

	hw_rasta_vert_xformed* vert_it = *vert_it_ptr;
#endif

	vector3d axis = axis_lock;
	vector3d camdir = geometry_manager::inst()->get_camera_dir();
	float invzoom = PROJ_RECIP_ZOOM;

	for(;(iri!=iri_end)&&(iri->frame==last_frame);++iri)
	{
#if !defined(NGL)
		if(((flavor&RENDER_TRANSLUCENT_PORTION)!=0)==(translucent_material||iri->color_scale.c.a<255))
#endif
		{
			rational_t scale         = 0.5f * iri->particle_scale;
			rational_t view_scaled_width  = /*vp_width  * */me->get_width () * scale;
			rational_t view_scaled_height = /*vp_height * */me->get_height() * scale;

			// make a 2-tri strip
			vector4d pp[4];
			if (flags & vr_billboard::AXIS_LOCK_LOCAL)
				axis = iri->get_local_to_world().non_affine_slow_xform(axis_lock).normalize();

			if (flags & vr_billboard::AXIS_LOCK_PERSPECTIVE)
			{
				vector3d xaxis,yaxis;
				float rads = iri->get_camera_relative_rotation();
				if (rads)
				{
					rational_t fCos,fSin;
					fast_sin_cos_approx(rads,&fSin,&fCos);
					xaxis = vector3d(fCos,-fSin,0);
					yaxis = vector3d(fSin, fCos,0);
				}
				else
				{
					xaxis =  XVEC;
					yaxis =  YVEC;
				}

				xaxis *= view_scaled_width;
				yaxis *= view_scaled_height;

				if (flags & vr_billboard::AXIS_LOCK_HALFPOS)
					camdir = ((iri->get_local_to_world().get_position() - geometry_manager::inst()->get_camera_pos()).set_length(0.5f) + geometry_manager::inst()->get_camera_dir()).normalize();
				else if (flags & vr_billboard::AXIS_LOCK_POS)
					camdir = (iri->get_local_to_world().get_position() - geometry_manager::inst()->get_camera_pos()).normalize();

				matrix4x4 total_xform;

#ifdef NGL
				if (flags & (vr_billboard::AXIS_LOCK_POS|vr_billboard::AXIS_LOCK_HALFPOS|vr_billboard::AXIS_LOCK_LOCAL))
					total_xform = calc_axis_lock(axis, camdir, iri->get_local_to_world().get_position()).get_matrix() * w2v;
				else
					total_xform = start_po * iri->get_local_to_world().get_matrix() * w2v;
#else
				if (flags & (vr_billboard::AXIS_LOCK_POS|vr_billboard::AXIS_LOCK_HALFPOS|vr_billboard::AXIS_LOCK_LOCAL))
					total_xform = calc_axis_lock(axis,camdir,iri->get_local_to_world().get_position()).get_matrix() * world_to_screen;
				else
					total_xform = start_po * iri->get_local_to_world().get_matrix() * world_to_screen;
#endif

				pp[0]=xform4d(total_xform,vector4d(-xaxis-yaxis,1));
				pp[0].homogenize();
				pp[1]=xform4d(total_xform,vector4d(-xaxis+yaxis,1));
				pp[1].homogenize();
				pp[2]=xform4d(total_xform,vector4d( xaxis-yaxis,1));
				pp[2].homogenize();
				pp[3]=xform4d(total_xform,vector4d( xaxis+yaxis,1));
				pp[3].homogenize();
#ifndef NGL
				pp[0].z = scl_z*pp[0].z+min_z;
				pp[1].z = scl_z*pp[1].z+min_z;
				pp[2].z = scl_z*pp[2].z+min_z;
				pp[3].z = scl_z*pp[3].z+min_z;
#endif
			}
			else
			{
				vector4d spos = xform4d(world_to_screen,vector4d(iri->get_local_to_world().get_position(),1));
				if (spos.w<=1e-5f) continue;
				spos.homogenize();
#ifndef NGL
				spos.z = scl_z*spos.z+min_z;
#endif
				vector4d sxaxis,syaxis;
				{
					vector3d syaxis3 = xform3d_0(geometry_manager::inst()->get_world_to_screen(),axis); // screen axis
					vector2d syaxis2(syaxis3.x,-syaxis3.y);
					vector2d sxaxis2(-syaxis2.y,syaxis2.x);
					float zoomw = spos.w*invzoom;
					syaxis2.set_length(vp_height*view_scaled_height*zoomw);
					sxaxis2.set_length(vp_width *view_scaled_width *zoomw);
					syaxis=vector4d(syaxis2.x,syaxis2.y,0,0);
					sxaxis=vector4d(sxaxis2.x,sxaxis2.y,0,0);
				}

				pp[0]=spos-sxaxis-syaxis;
				pp[1]=spos-sxaxis+syaxis;
				pp[2]=spos+sxaxis-syaxis;
				pp[3]=spos+sxaxis+syaxis;
			}
			bool skip_this_particle = false;
			if( !skip_this_particle )
			{

				// I light all four corners of the billboard the same because
				// billboards usually represent spherical objects, and casting
				// light across them might look...weird...
				// And I don't use reflectivity, I only use distance to the light
				// source.

				// it would pay off to use an indexed triangle list!

				color32 diffuse_color = iri->color_scale;
				//static const color32 specular_blank = color32(0,0,0,0);

#ifdef NGL
				u_int ngl_color = NGL_RGBA32(diffuse_color.get_red(),diffuse_color.get_green(),diffuse_color.get_blue(),diffuse_color.get_alpha());
				nglMeshWriteStrip( 4 );
				nglMeshWriteVertexPCUV( pp[0].x, pp[0].y, pp[0].z, ngl_color, btc[0].get_u(), btc[0].get_v() );
				nglMeshWriteVertexPCUV( pp[1].x, pp[1].y, pp[1].z, ngl_color, btc[1].get_u(), btc[1].get_v() );
				nglMeshWriteVertexPCUV( pp[2].x, pp[2].y, pp[2].z, ngl_color, btc[2].get_u(), btc[2].get_v() );
				nglMeshWriteVertexPCUV( pp[3].x, pp[3].y, pp[3].z, ngl_color, btc[3].get_u(), btc[3].get_v() );

				START_PROF_TIMER( proftimer_render_billboard_sphere);
				if ( (pp[0].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[0].x);
				if ( (pp[0].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[0].x);
				if ( (pp[0].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[0].y);
				if ( (pp[0].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[0].y);
				if ( (pp[0].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[0].z);
				if ( (pp[0].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[0].z);

				if ( (pp[1].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[1].x);
				if ( (pp[1].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[1].x);
				if ( (pp[1].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[1].y);
				if ( (pp[1].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[1].y);
				if ( (pp[1].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[1].z);
				if ( (pp[1].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[1].z);

				if ( (pp[2].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[2].x);
				if ( (pp[2].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[2].x);
				if ( (pp[2].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[2].y);
				if ( (pp[2].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[2].y);
				if ( (pp[2].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[2].z);
				if ( (pp[2].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[2].z);

				if ( (pp[3].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[3].x);
				if ( (pp[3].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[3].x);
				if ( (pp[3].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[3].y);
				if ( (pp[3].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[3].y);
				if ( (pp[3].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[3].z);
				if ( (pp[3].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[3].z);
				STOP_PROF_TIMER( proftimer_render_billboard_sphere);

				vertex_idx+=4;
#else
				vert_it->set_xyz_rhw(pp[0]);
				vert_it->tc[0] = btc[0];
				vert_it->diffuse = diffuse_color;
				//vert_it->specular = specular_blank;
				++vert_it;

				vert_it->set_xyz_rhw(pp[1]);
				vert_it->tc[0] = btc[1];
				vert_it->diffuse = diffuse_color;
				//vert_it->specular = specular_blank;
				++vert_it;

				vert_it->set_xyz_rhw(pp[2]);
				vert_it->tc[0] = btc[2];
				vert_it->diffuse = diffuse_color;
				//vert_it->specular = specular_blank;
				++vert_it;

				vert_it->set_xyz_rhw(pp[2]);
				vert_it->tc[0] = btc[2];
				vert_it->diffuse = diffuse_color;
				//vert_it->specular = specular_blank;
				++vert_it;

				vert_it->set_xyz_rhw(pp[1]);
				vert_it->tc[0] = btc[1];
				vert_it->diffuse = diffuse_color;
				//vert_it->specular = specular_blank;
				++vert_it;

				vert_it->set_xyz_rhw(pp[3]);
				vert_it->tc[0] = btc[3];
				vert_it->diffuse = diffuse_color;
				//vert_it->specular = specular_blank;
				++vert_it;
#endif
			}
	  }
  }
#ifdef NGL
  *vert_idx = vertex_idx;
#else
  *vert_it_ptr = vert_it;
#endif
  proftimer_render_c_axis_billboards.stop();
  return iri;
}

#ifdef NGL_PS2

static instance_render_info * c_prep_billboards( vr_billboard* me,
												instance_render_info* iri,
												instance_render_info* iri_end,
												nglMesh *mesh,
												int*  vert_idx,
												const matrix4x4& world_to_screen,
												const matrix4x4& start_po,
												float vp_width, // could get from me
												float vp_height, // could get from me
												render_flavor_t flavor,
												bool translucent_material,
												int flags)
{
	proftimer_render_c_billboards.start();
	int last_frame = iri->frame;

	matrix4x4 w2v;
	nglGetMatrix( native_to_ngl(w2v), NGLMTX_WORLD_TO_VIEW );

#if defined(TARGET_PS2)
	// Load up matrix for massive muls
	asm __volatile__(
		"lqc2    vf9,0x0(%0) \n"
		"lqc2    vf10,0x10(%0) \n"
		"lqc2    vf11,0x20(%0) \n"
		"lqc2    vf12,0x30(%0) \n"
		: : "r" (((sceVu0FMATRIX)&w2v.x.x)));

	sceVu0FVECTOR center_pt;
#endif


	int vertex_idx=*vert_idx;


	float me_width = me->get_width();
	float me_height = me->get_height();

	rational_t radius = __fsqrt((me_width*me_width) + (me_height*me_height));

	for(;(iri!=iri_end)&&(iri->frame==last_frame);++iri)
	{
		{
#if defined(TARGET_PS2)
			asm __volatile__(
				"lqc2    vf13,0x0(%0) \n"
				"vsub.w       vf13,vf13,vf13 \n"
				"vadd.w       vf13,vf13,vf0 \n"
				"vmulax.xyzw	ACC, vf9,vf13 \n"
				"vmadday.xyzw	ACC, vf10,vf13 \n"
				"vmaddaz.xyzw	ACC, vf11,vf13 \n"
				"vmaddw.xyzw	vf14, vf12,vf13 \n"
				"sqc2    vf14,0x0(%1) \n"
				: : "r" (((sceVu0FVECTOR)&iri->get_local_to_world().get_position().x)), "r" (center_pt));
#else
			vector3d cur_position( iri->get_local_to_world().get_position() );
			// make a 2-tri strip in world space
			vector3d center = xform3d_1(w2v, cur_position);
#endif


			rational_t scale         = 0.5f * iri->particle_scale;
			float rads = iri->get_camera_relative_rotation();

			float x_scale = me_width * scale;
			float y_scale = me_height * scale;

			vector3d xaxis(x_scale, 0.0f, 0.0f), yaxis(0.0f, -y_scale, 0.0f);

			if (rads)
			{
				rational_t fCos,fSin;

				// Bashes vf4-vf8
				fast_sin_cos_approx(rads,&fSin,&fCos);

				xaxis.x = fCos*x_scale;
				xaxis.y = -fSin*x_scale;

				yaxis.x = -fSin*y_scale;
				yaxis.y = -fCos*y_scale;
			}

			//      vector3d p;
			u_int ngl_color = NGL_RGBA32(iri->color_scale.get_red(),iri->color_scale.get_green(),iri->color_scale.get_blue(),iri->color_scale.get_alpha());
			nglMeshWriteStrip( 4 );

#if defined(TARGET_PS2)
			nglMeshWriteVertexPCUV( center_pt[0] - xaxis.x - yaxis.x, center_pt[1] - xaxis.y - yaxis.y, center_pt[2], ngl_color, 0.0f, 0.0f );
			nglMeshWriteVertexPCUV( center_pt[0] + xaxis.x - yaxis.x, center_pt[1] + xaxis.y - yaxis.y, center_pt[2], ngl_color, 1.0f, 0.0f );
			nglMeshWriteVertexPCUV( center_pt[0] - xaxis.x + yaxis.x, center_pt[1] - xaxis.y + yaxis.y, center_pt[2], ngl_color, 0.0f, 1.0f );
			nglMeshWriteVertexPCUV( center_pt[0] + xaxis.x + yaxis.x, center_pt[1] + xaxis.y + yaxis.y, center_pt[2], ngl_color, 1.0f, 1.0f );

			START_PROF_TIMER( proftimer_render_billboard_sphere);
			rational_t scaled_rad = radius*scale;
			if ( (center_pt[0]-scaled_rad) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (center_pt[0]-scaled_rad);
			if ( (center_pt[0]+scaled_rad) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (center_pt[0]+scaled_rad);

			if ( (center_pt[1]-scaled_rad) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (center_pt[1]-scaled_rad);
			if ( (center_pt[1]+scaled_rad) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (center_pt[1]+scaled_rad);

			if ( (center_pt[2]-scaled_rad) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (center_pt[2]-scaled_rad);
			if ( (center_pt[2]+scaled_rad) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (center_pt[2]+scaled_rad);
			STOP_PROF_TIMER( proftimer_render_billboard_sphere);
#else
			//			p = center - xaxis - yaxis;
			nglMeshWriteVertexPCUV( center.x - xaxis.x - yaxis.x, center.y - xaxis.y - yaxis.y, center.z, ngl_color, 0.0f, 0.0f );
			//			p = center + xaxis - yaxis;
			nglMeshWriteVertexPCUV( center.x + xaxis.x - yaxis.x, center.y + xaxis.y - yaxis.y, center.z, ngl_color, 1.0f, 0.0f );
			//			p = center - xaxis + yaxis;
			nglMeshWriteVertexPCUV( center.x - xaxis.x + yaxis.x, center.y - xaxis.y + yaxis.y, center.z, ngl_color, 0.0f, 1.0f );
			//			p = center + xaxis + yaxis;
			nglMeshWriteVertexPCUV( center.x + xaxis.x + yaxis.x, center.y + xaxis.y + yaxis.y, center.z, ngl_color, 1.0f, 1.0f );

			START_PROF_TIMER( proftimer_render_billboard_sphere);
			rational_t scaled_rad = radius*scale;
			if ( (center.x-scaled_rad) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (center.x-scaled_rad);
			if ( (center.x+scaled_rad) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (center.x+scaled_rad);

			if ( (center.y-scaled_rad) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (center.y-scaled_rad);
			if ( (center.y+scaled_rad) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (center.y+scaled_rad);

			if ( (center.z-scaled_rad) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (center.z-scaled_rad);
			if ( (center.z+scaled_rad) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (center.z+scaled_rad);
			STOP_PROF_TIMER( proftimer_render_billboard_sphere);
#endif

			vertex_idx+=4;
		}
	}
	*vert_idx = vertex_idx;

	proftimer_render_c_billboards.stop();
	return iri;
}



static instance_render_info * c_prep_axis_billboards( vr_billboard* me,
													 instance_render_info* iri,
													 instance_render_info* iri_end,
													 nglMesh *mesh,
													 int* vert_idx,
													 const matrix4x4& world_to_screen,
													 const matrix4x4& start_po,
													 float vp_width,
													 float vp_height,
													 render_flavor_t flavor,
													 unsigned int flags, // could get from me
													 const vector3d& axis_lock, // could get from me
													 bool translucent_material)
{
	proftimer_render_c_axis_billboards.start();
	int last_frame = iri->frame;

	matrix4x4 w2v;
	nglGetMatrix( native_to_ngl(w2v), NGLMTX_WORLD_TO_VIEW );

	int vertex_idx = *vert_idx;

	vector3d axis = axis_lock;
	vector3d camdir = geometry_manager::inst()->get_camera_dir();
	float invzoom = PROJ_RECIP_ZOOM;

	for(;(iri!=iri_end)&&(iri->frame==last_frame);++iri)
	{
		{
			rational_t scale         = 0.5f * iri->particle_scale;
			rational_t view_scaled_width  = /*vp_width  * */me->get_width () * scale;
			rational_t view_scaled_height = /*vp_height * */me->get_height() * scale;

			// make a 2-tri strip
			vector4d pp[4];
			if (flags & vr_billboard::AXIS_LOCK_LOCAL)
				axis = iri->get_local_to_world().non_affine_slow_xform(axis_lock).normalize();

			if (flags & vr_billboard::AXIS_LOCK_PERSPECTIVE)
			{
				vector3d xaxis,yaxis;
				float rads = iri->get_camera_relative_rotation();
				if (rads)
				{
					rational_t fCos,fSin;
					fast_sin_cos_approx(rads,&fSin,&fCos);
					xaxis = vector3d(fCos,-fSin,0);
					yaxis = vector3d(fSin, fCos,0);
				}
				else
				{
					xaxis =  XVEC;
					yaxis =  YVEC;
				}

				xaxis *= view_scaled_width;
				yaxis *= view_scaled_height;

				if (flags & vr_billboard::AXIS_LOCK_HALFPOS)
					camdir = ((iri->get_local_to_world().get_position() - geometry_manager::inst()->get_camera_pos()).set_length(0.5f) + geometry_manager::inst()->get_camera_dir()).normalize();
				else if (flags & vr_billboard::AXIS_LOCK_POS)
					camdir = (iri->get_local_to_world().get_position() - geometry_manager::inst()->get_camera_pos()).normalize();

				matrix4x4 total_xform;

				if (flags & (vr_billboard::AXIS_LOCK_POS|vr_billboard::AXIS_LOCK_HALFPOS|vr_billboard::AXIS_LOCK_LOCAL))
					total_xform = calc_axis_lock(axis, camdir, iri->get_local_to_world().get_position()).get_matrix() * w2v;
				else
					total_xform = start_po * iri->get_local_to_world().get_matrix() * w2v;

				pp[0]=xform4d(total_xform,vector4d(-xaxis-yaxis,1));
				pp[0].homogenize();
				pp[1]=xform4d(total_xform,vector4d(-xaxis+yaxis,1));
				pp[1].homogenize();
				pp[2]=xform4d(total_xform,vector4d( xaxis-yaxis,1));
				pp[2].homogenize();
				pp[3]=xform4d(total_xform,vector4d( xaxis+yaxis,1));
				pp[3].homogenize();
			}
			else
			{
				vector4d spos = xform4d(world_to_screen,vector4d(iri->get_local_to_world().get_position(),1));
				if (spos.w<=1e-5f) continue;
				spos.homogenize();
				vector4d sxaxis,syaxis;
				{
					vector3d syaxis3 = xform3d_0(geometry_manager::inst()->get_world_to_screen(),axis); // screen axis
					vector2d syaxis2(syaxis3.x,-syaxis3.y);
					vector2d sxaxis2(-syaxis2.y,syaxis2.x);
					float zoomw = spos.w*invzoom;
					syaxis2.set_length(vp_height*view_scaled_height*zoomw);
					sxaxis2.set_length(vp_width *view_scaled_width *zoomw);
					syaxis=vector4d(syaxis2.x,syaxis2.y,0,0);
					sxaxis=vector4d(sxaxis2.x,sxaxis2.y,0,0);
				}

				pp[0]=spos-sxaxis-syaxis;
				pp[1]=spos-sxaxis+syaxis;
				pp[2]=spos+sxaxis-syaxis;
				pp[3]=spos+sxaxis+syaxis;
			}

			// These lines are nonsense!
			//      bool skip_this_particle = false;
			//      if( !skip_this_particle )
			{

				// I light all four corners of the billboard the same because
				// billboards usually represent spherical objects, and casting
				// light across them might look...weird...
				// And I don't use reflectivity, I only use distance to the light
				// source.

				// it would pay off to use an indexed triangle list!

				//static const color32 specular_blank = color32(0,0,0,0);

				u_int ngl_color = NGL_RGBA32(iri->color_scale.get_red(),iri->color_scale.get_green(),iri->color_scale.get_blue(),iri->color_scale.get_alpha());
				nglMeshWriteStrip( 4 );
				nglMeshWriteVertexPCUV( pp[0].x, pp[0].y, pp[0].z, ngl_color, btc[0].get_u(), btc[0].get_v() );
				nglMeshWriteVertexPCUV( pp[1].x, pp[1].y, pp[1].z, ngl_color, btc[1].get_u(), btc[1].get_v() );
				nglMeshWriteVertexPCUV( pp[2].x, pp[2].y, pp[2].z, ngl_color, btc[2].get_u(), btc[2].get_v() );
				nglMeshWriteVertexPCUV( pp[3].x, pp[3].y, pp[3].z, ngl_color, btc[3].get_u(), btc[3].get_v() );

				START_PROF_TIMER( proftimer_render_billboard_sphere);
				if ( (pp[0].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[0].x);
				if ( (pp[0].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[0].x);
				if ( (pp[0].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[0].y);
				if ( (pp[0].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[0].y);
				if ( (pp[0].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[0].z);
				if ( (pp[0].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[0].z);

				if ( (pp[1].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[1].x);
				if ( (pp[1].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[1].x);
				if ( (pp[1].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[1].y);
				if ( (pp[1].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[1].y);
				if ( (pp[1].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[1].z);
				if ( (pp[1].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[1].z);

				if ( (pp[2].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[2].x);
				if ( (pp[2].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[2].x);
				if ( (pp[2].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[2].y);
				if ( (pp[2].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[2].y);
				if ( (pp[2].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[2].z);
				if ( (pp[2].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[2].z);

				if ( (pp[3].x) < g_NGL_inline_min_x ) g_NGL_inline_min_x = (pp[3].x);
				if ( (pp[3].x) > g_NGL_inline_max_x ) g_NGL_inline_max_x = (pp[3].x);
				if ( (pp[3].y) < g_NGL_inline_min_y ) g_NGL_inline_min_y = (pp[3].y);
				if ( (pp[3].y) > g_NGL_inline_max_y ) g_NGL_inline_max_y = (pp[3].y);
				if ( (pp[3].z) < g_NGL_inline_min_z ) g_NGL_inline_min_z = (pp[3].z);
				if ( (pp[3].z) > g_NGL_inline_max_z ) g_NGL_inline_max_z = (pp[3].z);
				STOP_PROF_TIMER( proftimer_render_billboard_sphere);

				vertex_idx+=4;
			}
	  }
  }
  *vert_idx = vertex_idx;
  proftimer_render_c_axis_billboards.stop();
  return iri;
}
#endif
////////////////////////////////////////////////////////////////////////////////

instance_bank<vr_billboard> vr_billboard_bank;

////////////////////////////////////////////////////////////////////////////////

vr_billboard::vr_billboard() : visual_rep(VISREP_BILLBOARD)
{
	flags = 0;
	z_push_factor = 1.0f; // default 1 meter toward camera (if FORCE_NEAR flag is later enabled)
	axis_lock=YVEC;
}

vr_billboard::~vr_billboard()
{
	//hw_texture_mgr::inst()->delete_texture( &my_texture );
}

vr_billboard::vr_billboard(chunk_file& fs, bool _instanced )
: visual_rep( VISREP_BILLBOARD, _instanced )
{
	internal_serial_in( fs );
}

vr_billboard::vr_billboard(const stringx& fname, unsigned) : visual_rep(VISREP_BILLBOARD)
{
	chunk_file fs;
	fs.open(fname);
	internal_serial_in( fs );
}

void vr_billboard::internal_serial_in( chunk_file& fs )
{
	flags = 0;
	z_push_factor = 1.0f; // default 1 meter toward camera (if FORCE_NEAR flag is later enabled)
	width = height = 1.0f;
	axis_lock=YVEC;
	// ive bled just to have it such
	stringx texture_dir;

	// use one directory back
	stringx dir = fs.get_dir();
	int im_the_last_slash = dir.rfind('\\');
	int second_to_last_slash = dir.rfind('\\',im_the_last_slash-1);
	texture_dir = dir.substr(0,second_to_last_slash+1) + PLATFORM_TEXTURE_PATH;
	for(;;)
	{
		chunk_flavor cf;
		serial_in( fs, &cf );
		if(cf == chunk_flavor("material"))
		{
			serial_in( fs, &my_material, texture_dir, 0 );
		}
		else if(cf == chunk_flavor("width"))
			serial_in(fs, &width);
		else if(cf == chunk_flavor("height"))
			serial_in(fs, &height);
		else if(cf == chunk_flavor("radius"))
		{
			serial_in(fs, &width);
			height = width;
		}
		//else if(cf == chunk_flavor("push"))
		//  serial_in(fs, &push);
		//else if(cf == chunk_flavor("fade"))
		//  serial_in(fs, &fade);
		else if(cf == chunk_flavor("flags"))
		{
			stringx flag_names;
			serial_in( fs, &flag_names );
			if (!flag_names.empty() && isdigit(flag_names[0]))
				flags = atoi(flag_names.c_str());
			else
			{
				if( flag_names.find( "AXIS_LOCK" ) != (int)string::npos )
					flags |= AXIS_LOCK; // this is good because it's a substring of the other AXIS_LOCK flags, and we do need to set it if those are used.  ;)
				if( flag_names.find( "DISABLE_PERSPECTIVE" ) != (int)string::npos )
					flags |= DISABLE_PERSPECTIVE;
				if( flag_names.find( "FORCE_NEAR") != (int)string::npos )
					flags |= FORCE_NEAR;
				if( flag_names.find( "AXIS_LOCK_POS") != (int)string::npos )
					flags |= AXIS_LOCK_POS;
				if( flag_names.find( "AXIS_LOCK_HALFPOS" ) != (int)string::npos )
					flags |= AXIS_LOCK_HALFPOS;
				if( flag_names.find( "AXIS_LOCK_LOCAL" ) != (int)string::npos )
					flags |= AXIS_LOCK_LOCAL;
				if( flag_names.find( "AXIS_LOCK_PERSPECTIVE" ) != (int)string::npos )
					flags |= AXIS_LOCK_PERSPECTIVE;
			}
		}
		else if(cf == chunk_flavor("axis"))
		{
			serial_in( fs, &axis_lock );
			axis_lock.normalize();
			flags |= AXIS_LOCK;
		}
		else if(cf == chunk_flavor("z_push"))
		{
			serial_in( fs, &z_push_factor );
			assert(z_push_factor >= -100 && z_push_factor <= 100);
			if (z_push_factor==0.0f)
				flags &= ~FORCE_NEAR;
			else
				flags |= FORCE_NEAR;
		}
		else if(cf == CHUNK_EOF || cf == CHUNK_END )
			break;
		else
		{
			stringx composite = stringx("Unknown chunk type ") + cf.to_stringx() + stringx(" in file ")+fs.get_name();
			error( composite.c_str() );
		}
	}
}

int vr_billboard::get_anim_length() const
{
	return my_material.get_anim_length();
}

extern vector3d global_ZEROVEC;

const vector3d& vr_billboard::get_center( time_value_t delta_t ) const
{
	return global_ZEROVEC;
}

rational_t vr_billboard::get_radius(time_value_t delta_t) const
{
	return width+height;  // overestimate
}
rational_t vr_billboard::compute_xz_radius_rel_center( const po& xform )
{
	return width+height;  // overestimate
}

render_flavor_t vr_billboard::render_passes_needed() const
{
	return my_material.is_translucent() ? RENDER_TRANSLUCENT_PORTION : RENDER_OPAQUE_PORTION;
}

time_value_t vr_billboard::get_ending_time() const
{
	return my_material.get_anim_length()*(1/30.0f);
}

float vr_billboard::time_value_to_frame(time_value_t t)
{
	return t*30.0f;
}


#ifdef NGL_PS2
void vr_billboard::render_instance( nglMesh *mesh,
								   int *num_quads,
								   render_flavor_t flavor,
								   instance_render_info* iri,
								   short *ifl_lookup)
{
	// nasty kludge to get rid of lens flares in low detail setting
	// we should use a detail flag instead to choose which get shown
	//if (os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL)>0)
    render_batch(mesh,num_quads,flavor,iri,1,ifl_lookup);
}
#endif

void vr_billboard::render_instance(render_flavor_t flavor,
								   instance_render_info* iri,
								   short *ifl_lookup)
{
	// nasty kludge to get rid of lens flares in low detail setting
	// we should use a detail flag instead to choose which get shown
	//if (os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL)>0)
    render_batch(flavor,iri,1, ifl_lookup);
}

bool g_disable_axis_lock = false;

#ifdef TARGET_GC
static int iri_sort( const void* lhs, const void* rhs )
{
	const instance_render_info* iri_lhs = (const instance_render_info*) lhs;
	const instance_render_info* iri_rhs = (const instance_render_info*) rhs;

	if( (*iri_lhs) < (*iri_rhs) ) {
		return -1;
	} else if( (*iri_rhs) < (*iri_rhs) ) {
		return 1;
	} else {
		return 0;
	}

}
#endif

#ifdef NGL
#define MAX_IFL_FRAMES 192
int viri_ifl_num_quads[MAX_IFL_FRAMES];/* ={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 200 */

void vr_billboard::render_batch( render_flavor_t flavor,
								instance_render_info* viri,
								int num_instances,
								short *ifl_lookup)
{
	if( !my_material.has_texture() )
		return;

	if( g_disable_axis_lock )
		if( flags & AXIS_LOCK )
			return;
			/*
			if( g_debug_num_bb_batches >= g_debug_max_batches )
			return;
			g_debug_num_bb_batches++;
		*/
#ifndef BUILD_BOOTABLE
		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_BILLBOARDS))
			return;
#endif
		//  static po* start_po = new po;  // allocating rather than leaving on stack for 8-byte alignment
		static po start_po;

		//	proftimer_render_billboards.start();
		assert( num_instances <= MAX_INSTANCES );

		float vp_width = geometry_manager::inst()->get_vp_width();
		float vp_height = geometry_manager::inst()->get_vp_height();

		matrix4x4 world_to_screen;

		if (!(flavor & RENDER_ENTITY_WIDGET))
			nglGetMatrix( native_to_ngl(world_to_screen), NGLMTX_WORLD_TO_SCREEN );
		else
			nglGetMatrix( native_to_ngl(world_to_screen), NGLMTX_VIEW_TO_SCREEN );

#ifdef TARGET_PC
		camera* curcam = app::inst()->get_game()->get_current_view_camera(g_cur_player_render);
#else
		camera* curcam = app::inst()->get_game()->get_current_view_camera();
#endif

		po const & cam_po = curcam->get_abs_po();

		proftimer_render_setup_billboards.start();

		int my_anim_len = my_material.get_anim_length();

		int i;

		START_PROF_TIMER(proftimer_render_bb_calc_size);
		assert(my_anim_len <= (MAX_IFL_FRAMES-4) && "IFL contains too many frames!");
		memset(viri_ifl_num_quads, 0, MAX_IFL_FRAMES*sizeof(int));
		STOP_PROF_TIMER(proftimer_render_bb_calc_size);

		if (my_anim_len>1)
		{
			if(ifl_lookup != NULL)
			{
				for ( i=0; i<num_instances; ++i )
				{
					viri[i].frame = (int)(ifl_lookup[viri[i].time_to_frame_locked(my_anim_len)]);
					START_PROF_TIMER(proftimer_render_bb_calc_size);
					++viri_ifl_num_quads[viri[i].frame];
					STOP_PROF_TIMER(proftimer_render_bb_calc_size);
				}
			}
			else
			{
				for ( i=0; i<num_instances; ++i )
				{
					viri[i].frame = /*(int)*/ viri[i].time_to_frame_locked(my_anim_len);
					START_PROF_TIMER(proftimer_render_bb_calc_size);
					++viri_ifl_num_quads[viri[i].frame];
					STOP_PROF_TIMER(proftimer_render_bb_calc_size);
				}
			}
		}
		else
		{
			for ( i=0; i<num_instances; ++i )
				viri[i].frame = 0;

			viri_ifl_num_quads[0] = num_instances;
		}
		//   viri[i].frame = ((int)(time_value_to_frame( viri[i].age ) + viri[i].ifl_frame_boost)) % my_anim_len;

#ifdef TARGET_GC
#pragma FIXME( "temporary patch for sort() compile problem on CW, 1.3 beta fixes it. -mkv 10/9/01" )
		if (my_anim_len>1)
			qsort( viri, num_instances, sizeof( instance_render_info ), iri_sort );
#else
		// sorts based on IFL frame index (only necessary for IFL textures)
		if (my_anim_len>1)
			sort(viri, viri+num_instances);
#endif

		instance_render_info* iri = viri;
		instance_render_info* iri_end = viri+num_instances;
		int last_frame = viri[0].frame;

		if ((flags & AXIS_LOCK) && !(flags & AXIS_LOCK_LOCAL))
		{
			// prepare the po for when an axis is locked:  this is the same
			// so3 for all axis-locked billboards
			// it's facing 0,0,1 in world space.  we need to rotate it to face
			// the camera
			// so we need to know the angle from 0,0,1 to the camera around 0,1,0
			// which means we have to ignore the camera's y coordinate
			start_po = calc_axis_lock(axis_lock,
				geometry_manager::inst()->get_camera_dir(),
				//visual_center-geometry_manager::inst()->get_camera_pos()).normalize(),
				ZEROVEC);
		}
		else
		{
			start_po = cam_po;
			start_po.set_position(ZEROVEC);
		}

		uint32 forceflags=0;
		if (render_passes_needed()&RENDER_TRANSLUCENT_PORTION) forceflags |= FORCE_TRANSLUCENCY;
		forceflags |= iri->get_force_flags();

		proftimer_render_setup_billboards.stop();

		while (iri!=iri_end) // c_prep_billboards will return each time a material change is necessary
		{
			START_PROF_TIMER(proftimer_render_billboard_create_mesh);
			//    LOG_SCRATCH_MESH_ID("Billboard Batch");

			g_NGL_inline_min_x = 1.0e32f;
			g_NGL_inline_max_x = -1.0e32f;

			g_NGL_inline_min_y = 1.0e32f;
			g_NGL_inline_max_y = -1.0e32f;

			g_NGL_inline_min_z = 1.0e32f;
			g_NGL_inline_max_z = -1.0e32f;


			// calculate the exact number of needed quads
			START_PROF_TIMER(proftimer_render_bb_calc_size);
			int num_rendered_quads = viri_ifl_num_quads[iri->frame];

#ifdef DEBUG
			int test_rendered_quads = 0;
			instance_render_info* iri_tmp = iri;
			for(;(iri_tmp!=iri_end)&&((int)iri_tmp->frame==last_frame);++iri_tmp)
				++test_rendered_quads;

			if(test_rendered_quads != num_rendered_quads)
			{
				nglPrintf("Mismatch in rendered quads: Frame %d   (actual %d, calculated: %d)", iri->frame, test_rendered_quads, num_rendered_quads);
			}
#endif

			STOP_PROF_TIMER(proftimer_render_bb_calc_size);

#if NGL > 0x010700
			nglCreateMesh(( NGLMESH_TEMP | NGLMESH_PERFECT_TRICLIP ), 1 );
#else
			nglCreateMesh( 1 );
			nglSetMeshFlags( NGLMESH_TEMP | NGLMESH_PERFECT_TRICLIP );
#endif
			nglMeshAddSection((nglMaterial *)my_material.get_ngl_material(), num_rendered_quads*4, 1 );
			u_int scratch_mesh_id = (u_int)-1;


			ADD_PROF_COUNT(profcounter_particle_scratch_bb, 1);
			STOP_PROF_TIMER(proftimer_render_billboard_create_mesh);

			//    START_PROF_TIMER(proftimer_render_billboard_misc);
			int   vertex_idx = 0;
			bool xluc_mat = (forceflags&FORCE_TRANSLUCENCY) || my_material.is_translucent();
			// axis locked billboards get sent as two triangles which we front clip
			// the other kind get sent as a four-vertex strip which we don't
			unsigned send_flags;

			unsigned force_disable_perspective = (flavor & RENDER_ENTITY_WIDGET ? vr_billboard::DISABLE_PERSPECTIVE : 0);

			if (flags & AXIS_LOCK)
			{
				iri = c_prep_axis_billboards( this,
					iri,
					iri_end,
					scratch_mesh_id,
					&vertex_idx,
					world_to_screen,
					start_po.get_matrix(),
					vp_width,
					vp_height,
					flavor,
					flags | force_disable_perspective,
					axis_lock,
					my_material.is_translucent() );
				send_flags = hw_rasta::SEND_VERT_FRONT_CLIP;
			}
			else
			{
				iri = c_prep_billboards( this,
					iri,
					iri_end,
					scratch_mesh_id,
					&vertex_idx,
					world_to_screen,
					start_po.get_matrix(),
					vp_width,
					vp_height,
					flavor,
					xluc_mat,
					flags | force_disable_perspective);
				send_flags = 0;
			}
			//    STOP_PROF_TIMER(proftimer_render_billboard_misc);

			// we are only called if bounding sphere might be visible
			// but there may be some clipping needed especially for axis-aligned billboards
			if( vertex_idx > 0 )
			{
				assert( vertex_idx <= num_rendered_quads*4 );
				// set remaining verts to 0

				START_PROF_TIMER( proftimer_render_billboard_fill);
				for( ; vertex_idx < num_rendered_quads*4; vertex_idx += 4 )
				{
					nglMeshWriteStrip( 4 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
				}

				STOP_PROF_TIMER( proftimer_render_billboard_fill);

				START_PROF_TIMER( proftimer_render_billboard_sphere);
				//			nglMeshCalcSphere();

				nglVector Center;
				Center[0] = g_NGL_inline_min_x + ( g_NGL_inline_max_x - g_NGL_inline_min_x ) * 0.5f;
				Center[1] = g_NGL_inline_min_y + ( g_NGL_inline_max_y - g_NGL_inline_min_y ) * 0.5f;
				Center[2] = g_NGL_inline_min_z + ( g_NGL_inline_max_z - g_NGL_inline_min_z ) * 0.5f;
				Center[3] = 1.0f;

				// Calculate the radius from an arbitrary point on the box.
				float DistX, DistY, DistZ;
				DistX = g_NGL_inline_min_x - Center[0];
				DistY = g_NGL_inline_min_y - Center[1];
				DistZ = g_NGL_inline_min_z - Center[2];
				float Radius = __fsqrt( DistX * DistX + DistY * DistY + DistZ * DistZ );

				// Fill out the appropriate mesh entries.
				nglMeshSetSphere( Center, Radius );

				STOP_PROF_TIMER( proftimer_render_billboard_sphere);

				START_PROF_TIMER(proftimer_render_billboard_misc);
				nglMatrix view_to_world;
				nglGetMatrix( view_to_world, NGLMTX_VIEW_TO_WORLD );
				STOP_PROF_TIMER(proftimer_render_billboard_misc);

				START_PROF_TIMER( proftimer_render_add_mesh );

				START_PROF_TIMER( proftimer_render_billboard_add_mesh);

				nglRenderParams Params;
				memset( &Params, 0, sizeof( Params ) );

				Params.Flags |= NGLP_TEXTURE_FRAME;
				Params.TextureFrame = last_frame;

#if 0 //#ifdef TARGET_PS2
				// $jim - not finished fix at the moment.
				if( flags & vr_billboard::FORCE_NEAR )
				{
					Params.Flags |= NGLP_ZBIAS;
					Params.ZBias = get_z_push_factor() * 3750;
				}
#endif


				nglListAddMesh( nglCloseMesh(), view_to_world, &Params );

				STOP_PROF_TIMER( proftimer_render_billboard_add_mesh);

				STOP_PROF_TIMER( proftimer_render_add_mesh );
			}
			last_frame = iri->frame;
  }
  //	proftimer_render_billboards.stop();
}

#ifdef NGL_PS2
void vr_billboard::render_batch( nglMesh *mesh,
								int *num_quads,
								render_flavor_t flavor,
								instance_render_info* viri,
								int num_instances,
								short *ifl_lookup)
{
	if( !my_material.has_texture() )
		return;

	if( g_disable_axis_lock )
		if( flags & AXIS_LOCK )
			return;

		if( g_debug_num_bb_batches >= g_debug_max_batches )
			return;
		g_debug_num_bb_batches++;

#ifndef BUILD_BOOTABLE
		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_BILLBOARDS))
			return;
#endif
		//  static po* start_po = new po;  // allocating rather than leaving on stack for 8-byte alignment
		static po start_po;

		//	proftimer_render_billboards.start();
		assert( num_instances <= MAX_INSTANCES );

		float vp_width = geometry_manager::inst()->get_vp_width();
		float vp_height = geometry_manager::inst()->get_vp_height();

		matrix4x4 world_to_screen;

		if (!(flavor & RENDER_ENTITY_WIDGET))
			nglGetMatrix( native_to_ngl(world_to_screen), NGLMTX_WORLD_TO_SCREEN );
		else
			nglGetMatrix( native_to_ngl(world_to_screen), NGLMTX_VIEW_TO_SCREEN );

#ifdef TARGET_PC
		camera* curcam = app::inst()->get_game()->get_current_view_camera(g_cur_player_render);
#else
		camera* curcam = app::inst()->get_game()->get_current_view_camera();
#endif

		po const & cam_po = curcam->get_abs_po();

		proftimer_render_setup_billboards.start();

		int my_anim_len = my_material.get_anim_length();

		int i;
		if (my_anim_len>1)
		{
			for ( i=0; i<num_instances; ++i )
				viri[i].frame = /*(int)*/ viri[i].time_to_frame_locked(my_anim_len);
		}
		else
		{
			for ( i=0; i<num_instances; ++i )
				viri[i].frame = 0;
		}
		//   viri[i].frame = ((int)(time_value_to_frame( viri[i].age ) + viri[i].ifl_frame_boost)) % my_anim_len;

#ifdef TARGET_GC
		//#pragma FIXME( "temporary patch for sort() compile problem on CW. -mkv 10/9/01" )
		if (my_anim_len>1)
			qsort( viri, num_instances, sizeof( instance_render_info ), iri_sort );
#else
		// sorts based on IFL frame index (only necessary for IFL textures)
		if (my_anim_len>1)
			sort(viri, viri+num_instances);
#endif

		instance_render_info* iri = viri;
		instance_render_info* iri_end = viri+num_instances;
		int last_frame = viri[0].frame;

		if ((flags & AXIS_LOCK) && !(flags & AXIS_LOCK_LOCAL))
		{
			// prepare the po for when an axis is locked:  this is the same
			// so3 for all axis-locked billboards
			// it's facing 0,0,1 in world space.  we need to rotate it to face
			// the camera
			// so we need to know the angle from 0,0,1 to the camera around 0,1,0
			// which means we have to ignore the camera's y coordinate
			start_po = calc_axis_lock(axis_lock,
				geometry_manager::inst()->get_camera_dir(),
				//visual_center-geometry_manager::inst()->get_camera_pos()).normalize(),
				ZEROVEC);
		}
		else
		{
			start_po = cam_po;
			start_po.set_position(ZEROVEC);
		}

		uint32 forceflags=0;
		if (render_passes_needed()&RENDER_TRANSLUCENT_PORTION) forceflags |= FORCE_TRANSLUCENCY;
		forceflags |= iri->get_force_flags();

		proftimer_render_setup_billboards.stop();

		while (iri!=iri_end) // c_prep_billboards will return each time a material change is necessary
		{
			START_PROF_TIMER(proftimer_render_billboard_create_mesh);
			//    LOG_SCRATCH_MESH_ID("Billboard Batch");

			g_NGL_inline_min_x = 1.0e32f;
			g_NGL_inline_max_x = -1.0e32f;

			g_NGL_inline_min_y = 1.0e32f;
			g_NGL_inline_max_y = -1.0e32f;

			g_NGL_inline_min_z = 1.0e32f;
			g_NGL_inline_max_z = -1.0e32f;

			nglEditMesh(mesh);

			//    ADD_PROF_COUNT(profcounter_particle_scratch_bb, 1);
			STOP_PROF_TIMER(proftimer_render_billboard_create_mesh);

			//    START_PROF_TIMER(proftimer_render_billboard_misc);
			int   vertex_idx = 0;
			bool xluc_mat = (forceflags&FORCE_TRANSLUCENCY) || my_material.is_translucent();
			// axis locked billboards get sent as two triangles which we front clip
			// the other kind get sent as a four-vertex strip which we don't
			unsigned send_flags;

			unsigned force_disable_perspective = (flavor & RENDER_ENTITY_WIDGET ? vr_billboard::DISABLE_PERSPECTIVE : 0);

			if (flags & AXIS_LOCK)
			{
				iri = c_prep_axis_billboards( this,
					iri,
					iri_end,
					mesh,
					&vertex_idx,
					world_to_screen,
					start_po.get_matrix(),
					vp_width,
					vp_height,
					flavor,
					flags | force_disable_perspective,
					axis_lock,
					my_material.is_translucent() );
				send_flags = hw_rasta::SEND_VERT_FRONT_CLIP;
			}
			else
			{
				iri = c_prep_billboards( this,
					iri,
					iri_end,
					mesh,
					&vertex_idx,
					world_to_screen,
					start_po.get_matrix(),
					vp_width,
					vp_height,
					flavor,
					xluc_mat,
					flags | force_disable_perspective);
				send_flags = 0;
			}
			//    STOP_PROF_TIMER(proftimer_render_billboard_misc);

			// we are only called if bounding sphere might be visible
			// but there may be some clipping needed especially for axis-aligned billboards
			if( vertex_idx > 0 )
			{
				assert( vertex_idx <= num_instances*4 );
				// set remaining verts to 0

				START_PROF_TIMER( proftimer_render_billboard_sphere);
				//			nglMeshCalcSphere((u_int)vertex_idx);


				nglVector Center;
				Center[0] = g_NGL_inline_min_x + ( g_NGL_inline_max_x - g_NGL_inline_min_x ) * 0.5f;
				Center[1] = g_NGL_inline_min_y + ( g_NGL_inline_max_y - g_NGL_inline_min_y ) * 0.5f;
				Center[2] = g_NGL_inline_min_z + ( g_NGL_inline_max_z - g_NGL_inline_min_z ) * 0.5f;
				Center[3] = 1.0f;

				// Calculate the radius from an arbitrary point on the box.
				float DistX, DistY, DistZ;
				DistX = g_NGL_inline_min_x - Center[0];
				DistY = g_NGL_inline_min_y - Center[1];
				DistZ = g_NGL_inline_min_z - Center[2];
				float Radius = __fsqrt( DistX * DistX + DistY * DistY + DistZ * DistZ );

				// Fill out the appropriate mesh entries.
				nglMeshSetSphere( Center, Radius );



				STOP_PROF_TIMER( proftimer_render_billboard_sphere);

				START_PROF_TIMER( proftimer_render_billboard_fill);
				int fill_vertex_idx = (*num_quads)*4;
				for( ; vertex_idx < fill_vertex_idx; vertex_idx += 4 )
				{
					nglMeshWriteStrip( 4 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
					nglMeshWriteVertexPC( 0.0f, 0.0f, 0.0f, 0 );
				}
				*num_quads = num_instances;
				STOP_PROF_TIMER( proftimer_render_billboard_fill);

				START_PROF_TIMER(proftimer_render_billboard_misc);
				nglMatrix view_to_world;
				nglGetMatrix( view_to_world, NGLMTX_VIEW_TO_WORLD );
				STOP_PROF_TIMER(proftimer_render_billboard_misc);

				START_PROF_TIMER( proftimer_render_add_mesh );

				START_PROF_TIMER( proftimer_render_billboard_add_mesh);

				nglListAddMesh( mesh, view_to_world, NULL );

				STOP_PROF_TIMER( proftimer_render_billboard_add_mesh);

				STOP_PROF_TIMER( proftimer_render_add_mesh );
			}
			last_frame = iri->frame;
  }
  //	proftimer_render_billboards.stop();
}
#endif

#else // NGL

void vr_billboard::render_batch( render_flavor_t flavor,
								instance_render_info* viri,
								int num_instances,
								short *ifl_lookup)
{
	if( g_debug_num_bb_batches >= g_debug_max_batches )
		return;
	g_debug_num_bb_batches++;

#ifndef BUILD_BOOTABLE
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_BILLBOARDS))
		return;
#endif

	static po start_po;

	//	proftimer_render_billboards.start();
	assert( num_instances <= MAX_INSTANCES );

	//  rational_t proj_cosine = PROJ_COS_FOV;
	float vp_width = geometry_manager::inst()->get_vp_width();
	float vp_height = geometry_manager::inst()->get_vp_height();

	const matrix4x4& world_to_screen = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

#ifdef TARGET_PC
	camera* curcam = app::inst()->get_game()->get_current_view_camera(g_cur_player_render);
#else
	camera* curcam = app::inst()->get_game()->get_current_view_camera();
#endif

	po const & cam_po = curcam->get_abs_po();

	proftimer_render_setup_billboards.start();

	int my_anim_len = my_material.get_anim_length();

	int i;
	for ( i=0; i<num_instances; ++i )
		viri[i].frame = /*(int)*/ viri[i].time_to_frame_locked(my_anim_len);
	//   viri[i].frame = ((int)(time_value_to_frame( viri[i].age ) + viri[i].ifl_frame_boost)) % my_anim_len;

#if 1 // sorts based on IFL frame index (only necessary for IFL textures)
#ifndef TARGET_GC
	if (my_anim_len>1)
		sort(viri, viri+num_instances);
#endif
#endif

	instance_render_info* iri = viri;
	instance_render_info* iri_end = viri+num_instances;
	int last_frame = viri[0].frame;

	if ((flags & AXIS_LOCK) && !(flags & AXIS_LOCK_LOCAL))
	{
		// prepare the po for when an axis is locked:  this is the same
		// so3 for all axis-locked billboards
		// it's facing 0,0,1 in world space.  we need to rotate it to face
		// the camera
		// so we need to know the angle from 0,0,1 to the camera around 0,1,0
		// which means we have to ignore the camera's y coordinate
		start_po = calc_axis_lock(axis_lock,
			geometry_manager::inst()->get_camera_dir(),
			//visual_center-geometry_manager::inst()->get_camera_pos()).normalize(),
			ZEROVEC);
	}
	else
	{
		start_po = cam_po;
		start_po.set_position(ZEROVEC);
	}
	
	uint32 forceflags=0;
#ifndef TARGET_MKS
	if (render_passes_needed()&RENDER_TRANSLUCENT_PORTION) forceflags |= FORCE_TRANSLUCENCY;
#endif
	forceflags |= iri->get_force_flags();
	
	proftimer_render_setup_billboards.stop();
	
	
	while (iri!=iri_end) // c_prep_billboards will return each time a material change is necessary
	{
		vert_workspace_xformed.lock(num_instances*6);
		hw_rasta_vert_xformed* vert_it;
		hw_rasta_vert_xformed* vert_begin;
		vert_it = vert_begin = vert_workspace_xformed.begin();
		bool xluc_mat = (forceflags&FORCE_TRANSLUCENCY) || my_material.is_translucent();
		// axis locked billboards get sent as two triangles which we front clip
		// the other kind get sent as a four-vertex strip which we don't
		unsigned send_flags;
		unsigned num_verts=0;
		
		if (flags & AXIS_LOCK)
		{
			iri = c_prep_axis_billboards( this,
				iri, 
				iri_end, 
				&vert_it,
				world_to_screen,
				start_po.get_matrix(),
				vp_width,
				vp_height,
				flavor,
				flags,
				axis_lock,
				my_material.is_translucent() );
			send_flags = hw_rasta::SEND_VERT_FRONT_CLIP;
			num_verts = vert_it-vert_begin;
			assert(num_verts%3==0);
		}
		else
		{
			aggregate_vert_buf* matbuf = my_material.find_mat_buf( last_frame, forceflags );
			matbuf->lock();
			hw_rasta_vert_xformed* pverts = matbuf->get_quads(num_instances);
			hw_rasta_vert_xformed* pverts_begin = pverts;
			iri = c_prep_billboards( this,
				iri, 
				iri_end, 
				&pverts,
				world_to_screen,
				start_po.get_matrix(),
				vp_width,
				vp_height,
				flavor,
				xluc_mat,
				flags );
			num_verts = pverts-pverts_begin;
			matbuf->unget_quads(num_instances-(num_verts>>2));
			matbuf->unlock();
			assert((num_verts&3)==0);
			send_flags = 0;
		}
		vert_workspace_xformed.unlock();
		// we are only called if bounding sphere might be visible
		// but there may be some clipping needed especially for axis-aligned billboards
		if(num_verts)
		{
			// send it to the card
			proftimer_render_sendctx_billboards.start();
			my_material.send_context(last_frame, MAP_DIFFUSE, forceflags);
			proftimer_render_sendctx_billboards.stop();
			proftimer_render_draw_billboards.start();
#if defined(TARGET_MKS) || 0
			hw_rasta::inst()->send_vertex_list( vert_workspace_xformed, num_verts, send_flags );
#else
			if (send_flags & hw_rasta::SEND_VERT_FRONT_CLIP)
			{
				hw_rasta::inst()->send_vertex_list( vert_workspace_xformed, num_verts, send_flags );
			}
#endif
			proftimer_render_draw_billboards.stop();
		}
		last_frame = iri->frame;
	}
	//	proftimer_render_billboards.stop();
}

#endif  // !NGL
