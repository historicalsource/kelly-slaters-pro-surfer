////////////////////////////////////////////////////////////////////////////////
/*
  floatingtext

  floating text usually attached to some other object
*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "osalloc.h"

#include "hwrasterize.h"
#include "app.h"
#include "floatingtext.h"
#include "camera.h"
#include "game.h"
#include "geomgr.h"
#include "hwrasterize.h"
#include "iri.h"
//#include "maxiri.h"
#include "oserrmsg.h"
#include "txtcoord.h"
#include "profiler.h"
#include "element.h"
#include "vertwork.h"
#include "renderflav.h"

#include <algorithm>

float g_dist_cap_const = 15;    // this value keeps text from getting too small


#define LERP(s, d, f) ((s) + ((d) - (s)) * (f))

// permanent.  This is a waste.
static vert_buf transformed_verts(300, vert_buf::TRANSFORMED);


vr_floating_text::vr_floating_text( int v, material* mat, bool _instanced )
{
  text = itos(v); my_material = material_bank.new_instance( mat );
  set_instanced(_instanced);
  diffuse_color = color32(255,255,255,255);
}

vr_floating_text::vr_floating_text( const stringx& s, material* mat, bool _instanced )
{ 
  text = s; 
  my_material = material_bank.new_instance( mat ); 
  set_instanced(_instanced);
  diffuse_color = color32(255,255,255,255);
}


void vr_floating_text::render_batch(render_flavor_t flavor, instance_render_info* viri, int num_instances) 
{
	hw_rasta_vert  *vert_it = transformed_verts.begin();
	hw_rasta_vert  *vert_begin = vert_it;
	int vp_height = hw_rasta::inst()->get_screen_height();
	int vp_width = hw_rasta::inst()->get_screen_width();
	// this viewport matrix isn't the same as the one in the d3d docs
	// but it actually works and makes sense
	matrix4x4       viewport(vp_width / 2,              0, 0, 0,
		                                  0, -vp_height / 2, 0, 0,
                                      0,              0, 1, 0,
                           vp_width / 2,  vp_height / 2, 0, 1);
  matrix4x4       m = geometry_manager::inst()->get_world_to_screen() * viewport;
	int             i, j;
	float           s, e = 0.0f, l=0.0f;
	char_info      *ci;

	assert(num_instances < 10);

  for ( i=0; i<num_instances; ++i )
    viri[i].frame = (int) viri[i].time_to_frame_locked(my_material->diffuse_map.get_anim_length());
//    viri[i].frame = time_value_to_frame(viri[i].age) + viri[i].ifl_frame_boost;
  sort(viri, viri+num_instances);

  uint32 forceflags = 0;
  #ifndef TARGET_MKS
  if (render_passes_needed()&RENDER_TRANSLUCENT_PORTION) forceflags |= FORCE_TRANSLUCENCY;
  #endif

#if !defined(TARGET_MKS)
  //if(flavor & RENDER_TRANSLUCENT_PORTION)
#endif    
  {
	  int last_frame = viri[0].frame;

		for(i = 0; i < num_instances; ++i)
    {
      const instance_render_info *iri = &viri[i];
      if((int)iri->frame != last_frame)
      {
        transformed_verts.unlock();

				// no need to clip;  we are only called if bounding sphere might be visible
				if(vert_it > vert_begin)
        {
					int num_verts = vert_it - vert_begin;

					my_material->send_context(last_frame, DIFFUSE, forceflags);
					geometry_manager::inst()->draw_primitive(PRIMITIVE_TRILIST, transformed_verts, num_verts);
				}
				vert_it = vert_begin = transformed_verts.begin();
			}
			last_frame = iri->frame;

			po my_po = po_identity_matrix;
			camera* curcam = app::inst()->get_game()->get_current_view_camera();
			po const & cam_po = curcam->get_po_inline();
			po xform_po = po( iri->get_local_to_world() );
			vector3d cur_position = xform_po.get_abs_position();

			po spin = po_identity_matrix;
			spin.set_rot( vector3d(0,0,1), iri->get_camera_relative_rotation() );

			if(flags&AXIS_LOCK)
      {
				// it's facing 0,0,1 in world space.  we need to rotate it to face
				// the camera
				// so we need to know the angle from 0,0,1 to the camera around 0,1,0
				// which means we have to ignore the camera's y coordinate
				vector3d to_cam = -geometry_manager::inst()->get_camera_dir();
				rational_t angle = safe_atan2( to_cam.x, to_cam.z );
				my_po.set_rot( vector3d(0,1,0), -angle + PI );
			}
			else
      {
				// what transform will make our manually created two-tri mesh face the
				// camera?  the inverse of the camera po
				my_po = cam_po;
				my_po.set_rel_position(vector3d(0,0,0));

				// my_po.invert();

				// the way the thing is facing with no transform is 0,0,1
				// vector3d cur_facing = vector3d(0,0,1);
			}

			my_po = spin * my_po;

			// cap the distance so that the text is always readable
			// modify the distance at which this is done by changing g_dist_cap_const
			vector3d to_cam;
			to_cam = cam_po.get_abs_position() - my_po.get_abs_position();
			float dist = to_cam.length();
			if (dist > g_dist_cap_const)
			{
				to_cam *= (dist - g_dist_cap_const) / dist;
				to_cam += my_po.get_abs_position();
				my_po.set_rel_position(to_cam);
			}

			// its fairly ridiculous, but in rendering a particle floatingtext,
			// we convert scale to a matrix, send it to here, convert it back
			// to a float.  (We could convert it back to a matrix instead of
			// modifying radius...wouldn't that be special?)
			rational_t scale         = xform_po.get_scale();

			// the way we want it to face is camera - position
			my_po.set_rel_position( cur_position );
			// hope that we die holding hands

			// two transforms is faster than a matrix multiply and a transform
			//matrix4x4 total_xform = my_po.get_matrix() * m;

			vector4d sctr = xform4d(my_po.get_matrix(),vector4d(0,0,0,1));
			sctr = xform4d(m,sctr);
			if (sctr.w<=0) 
			  continue;
			float invw=1.0f/sctr.w;
			vector3d ctr(sctr.x*invw,sctr.y*invw,sctr.z*invw);
			float scrnwidth =0.5f * width  * scale * invw;
			float scrnheight=0.5f * height * scale * invw;
			
			// make a 2-tri strip in world space
			vector3d p[4];
	
			p[0] = ctr+vector3d(-scrnwidth,-scrnheight,0);
			p[1] = ctr+vector3d(-scrnwidth, scrnheight,0);
			p[2] = ctr+vector3d( scrnwidth,-scrnheight,0);
			p[3] = ctr+vector3d( scrnwidth, scrnheight,0);

      hw_texture* tex = hw_texture_mgr::inst()->get_texture( my_material->get_texture_id( last_frame ) );
      int tw = tex->get_width ();
      int th = tex->get_height();

      int textsize=text.size();
			l = 0.0f;
			for(j = 0; j < textsize; ++j)
      {
				ci = font->get_char_info(text[j]);
				if(ci)
					l += (float)(ci->x1 - ci->x0);
			}
			e = 0.0f;
			for(j = 0; j < textsize; ++j)
      {
				ci = font->get_char_info(text[j]);
				if(ci)
        {
					s = e;
					e += (float)(ci->x1 - ci->x0) / l;
					// some of these LERP's can be factored out
					*vert_it++ = hw_rasta_vert(LERP(p[0], p[2], s), invw, texture_coord((float)ci->x0/tw, (float)ci->y1/th), diffuse_color, color32(0, 0, 0, 0));
					*vert_it++ = hw_rasta_vert(LERP(p[1], p[3], s), invw, texture_coord((float)ci->x0/tw, (float)ci->y0/th), diffuse_color, color32(0, 0, 0, 0));
					*vert_it++ = hw_rasta_vert(LERP(p[0], p[2], e), invw, texture_coord((float)ci->x1/tw, (float)ci->y1/th), diffuse_color, color32(0, 0, 0, 0));
					*vert_it++ = hw_rasta_vert(LERP(p[0], p[2], e), invw, texture_coord((float)ci->x1/tw, (float)ci->y1/th), diffuse_color, color32(0, 0, 0, 0));
					*vert_it++ = hw_rasta_vert(LERP(p[1], p[3], s), invw, texture_coord((float)ci->x0/tw, (float)ci->y0/th), diffuse_color, color32(0, 0, 0, 0));
					*vert_it++ = hw_rasta_vert(LERP(p[1], p[3], e), invw, texture_coord((float)ci->x1/tw, (float)ci->y0/th), diffuse_color, color32(0, 0, 0, 0));
				}
			}
		}

		transformed_verts.unlock();

		if(vert_it > vert_begin)
    {
			int num_verts = vert_it-vert_begin;
			// send it to the card
			my_material->send_context(last_frame, DIFFUSE, forceflags);
			geometry_manager::inst()->draw_primitive(PRIMITIVE_TRILIST, transformed_verts, num_verts);
		}
	}
}
