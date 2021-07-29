// lightmgr.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

#include "global.h"
#include "lightmgr.h"
#include "entity.h"
#include "light.h"
#include "terrain.h"
#include "pmesh.h" // for MAX_BONES, so we can extern bone_lights
#include "osdevopts.h"

#include "clipflags.h"
#include "game.h"

#include <vector>
#include <algorithm>

//#define ENABLE_AFFECT_TERRAIN_LIGHTS
//#define WADE_LIGHTING




static refptr<light_manager> static_light_set=NULL; //NEW light_manager(true);

void LIGHTMGR_StaticInitLightSet( void )
{
	static_light_set=NEW light_manager(true);
}



light_manager* light_manager::get_static_light_set()
{
  static_light_set->lights.resize(0);
  static_light_set->last_ambient=static_light_set->goal_ambient=color_white;
  assert(static_light_set->last_ambient.r==1.0f &&
         static_light_set->last_ambient.g==1.0f &&
         static_light_set->last_ambient.b==1.0f);
  static_light_set->max_lights=1;
  static_light_set->cur_max_lights=1;
  static_light_set->dynamic_only=true;
  static_light_set->allow_omni=true;
  return static_light_set;
}


class scan_light_context
{
public:
  struct light_rec
  {
    float brightness;
    light_source* light;
    light_rec() {}
    light_rec(float _influence, light_source* ls) : brightness(_influence), light(ls) {}
    inline bool operator <(const light_rec& r) const { return brightness > r.brightness; } // brightest lights go first
  };

  vector<light_rec> lights;

  color ambient_factor;
  color underwater_ambient_factor;

  scan_light_context()
  {
    lights.reserve(64);
  }

  const color &get_ambient_factor(region *reg, const int playerID);

  void reset_lights(region* my_region, const sphere& bound, bool dynamic_only, const int playerID);
};

static scan_light_context scanlites;


const color &scan_light_context::get_ambient_factor(region *reg, const int playerID )
{
	#ifdef PROJECT_KELLYSLATER
		// this is an ugly last minute hack to change the ambient
		// player lighting under water.  -EO
	if ( UNDERWATER_CameraUnderwater(playerID) )
	{
		int curbeach=g_game_ptr->get_beach_id();
		u_int underwater = BeachDataArray[curbeach].underwaterambient;
		underwater_ambient_factor.a=1.0f;
		underwater_ambient_factor.r=((float) ((underwater >> 16) & 0xFF )) / 255.0f;
		underwater_ambient_factor.g=((float) ((underwater >>  8) & 0xFF )) / 255.0f;
		underwater_ambient_factor.b=((float) ((underwater      ) & 0xFF )) / 255.0f;
#ifdef TARGET_XBOX
	// On Xbox, we override the lighting values from the scene.  The PS2 values aren't appropriate for 
	// Xbox, because of differences in gamma and saturation.  To be fixed in the exporter next year. 
	// (dc 07/11/02)
		float underwater_boost = BeachDataArray[curbeach].underwater_boost;
//		underwater_ambient_factor.a=1.0f;
		underwater_ambient_factor.r+=underwater_boost;
		underwater_ambient_factor.g+=underwater_boost;
		underwater_ambient_factor.b+=underwater_boost;
#endif
  	return underwater_ambient_factor;
	}
	#endif


  return reg->get_ambient();
}


void scan_light_context::reset_lights(region* my_region, const sphere& bound, bool dynamic_only, const int playerID )
{
  lights.resize(0);
  ambient_factor = color(0.125f,0.125f,0.125f,1.0f);

  if(my_region)
  {
    ambient_factor = get_ambient_factor(my_region,playerID);

    // I should check all nearby regions, not just the one I'm in.
    // However, directional lights can only come from the current region.
    const region::light_list& rlights = my_region->get_lights();
    for (region::light_list::const_iterator it=rlights.begin(); it!=rlights.end(); ++it)
    {
      light_source* ls = *it;
      if (ls)
      {
        assert(ls->has_properties());
        const light_properties* lprops = &ls->get_properties();
        color dc=lprops->get_color();
        color ac=lprops->get_additive_color();
        if (!dc.r && !dc.g && !dc.b &&
            !ac.r && !ac.g && !ac.b)
          continue; // light has no color; it's effectively "off"
        if (dynamic_only)
         #ifdef ENABLE_AFFECT_TERRAIN_LIGHTS
          if (!lprops->affects_terrain())
         #endif
            continue;
        float range = ls->get_dist(bound);
        float bright = lprops->get_brightness(range);
        if (bright>0)
          lights.push_back(light_rec(bright,ls));
      }
    }
  }

  // Sort omni lights by importance.  Brightest ones are at beginning of the list.
	#if !defined(PROJECT_KELLYSLATER)
  sort(lights.begin(), lights.end());
	#endif
}


#ifdef DEBUG
void light_manager::dump_debug_info() const
{
  debug_print("dumping light manager:  %d lights of max %d", lights.size(), max_lights);
  debug_print("  ambient current %f,%f,%f,%f goal %f,%f,%f,%f",
             last_ambient.r,last_ambient.g,last_ambient.b,last_ambient.a,
             goal_ambient.r,goal_ambient.g,goal_ambient.b,goal_ambient.a);
  debug_print("  allow omni %d, dynamic only %d",allow_omni,dynamic_only);
  for (int i=0; i<(int)lights.size(); ++i)
  {
    debug_print("  light %d: source=%p", i, lights[i].source);
    debug_print("    flavor=%s  vec=%f,%f,%f", (lights[i].props.get_flavor()==LIGHT_FLAVOR_POINT) ? "omni" : "dir",
       lights[i].dir_or_pos.x,lights[i].dir_or_pos.y,lights[i].dir_or_pos.z);
    debug_print("    intensity=%f  color=%f,%f,%f",
       lights[i].current_intensity,
       lights[i].props.get_color().r,lights[i].props.get_color().g,lights[i].props.get_color().b);
  }
}
#endif


int light_manager::compare_light(int sli)
{
  light_source* ls=scanlites.lights[sli].light;
  if (!ls)
    return -1;
  assert(ls->has_properties());
  int j;
  light_rec* lite;
  // if light is in the list already, re-use it
  int num_lights = lights.size();
  for (j=0; j<num_lights; ++j)
  {
    lite = &lights[j];
    if (ls == lite->source)
    {
      if (lite->props.get_flavor() == LIGHT_FLAVOR_POINT)
        lite->dir_or_pos = ls->get_abs_position();
      else
        lite->dir_or_pos = ls->get_abs_po().get_y_facing();
      return j;
    }
  }
  const light_properties& cur_lp = ls->get_properties(); // replace_light needs this set up!
  // try to find an empty slot and fill that
  for (j=0; j<num_lights; ++j)
  {
    lite = &lights[j];
    if (!lite->source && lite->current_intensity <= 0.1f)
    {
      //debug_print("replacing disabled light");
      goto replace_light;
    }
  }
  // is there room for a NEW light?
  if (num_lights < (int)cur_max_lights)
  {
    j = num_lights;
    lights.resize(j+1);
    //debug_print("inserting NEW light");
    goto replace_light;
  }

  {
    // get light brightness
    float l_bright=scanlites.lights[sli].brightness;
    //float l_bright=cur_lp.get_brightness(ls->get_dist(bound));
    // find the dimmest light dimmer than this one
    int dimmest=-1;
    float dimmest_brightness=l_bright;
    dimmest_brightness *= 0.985f; // kludge to keep lights from cycling as easily
    for (j=0; j<num_lights; ++j)
    {
      lite = &lights[j];
      if (lite->source)
      {
        float ex_bright = lite->props.get_brightness(lite->source->get_dist(bound));
        if (dimmest_brightness > ex_bright)
        {
          dimmest_brightness = ex_bright;
          dimmest = j;
        }
      }
    }
    if (dimmest>=0)
    {
      lite = &lights[dimmest];
      //debug_print("turning off dimmer light");
      lite->source = NULL;      // start turning it off.
      if (lite->current_intensity <= 0.01f)  // it just got added or was about to go out anyway
      {
        j = dimmest;
        //debug_print("replacing dimmer light");
        goto replace_light;
      }
    }
  }
  return -1;

replace_light: // replaces light j with ls
  lite = &lights[j];
  lite->source = ls;
  lite->props = cur_lp;
  // understanding directional light vectors 101:
  // the vector we need to dot product each normal with is the
  // vector facing the light from the vertex.  Happily, when you
  // make a directional light in MAX that's facing A, it returns
  // a po with a y-axis facing away from A.
  if ( cur_lp.get_flavor() == LIGHT_FLAVOR_POINT )
    lite->dir_or_pos = ls->get_abs_position();
  else
    lite->dir_or_pos = ls->get_abs_po().get_y_facing();

  assert(lite->dir_or_pos.is_valid());

  lite->current_intensity = 0.0001f;
  return j;
}


void light_manager::frame_advance(region* reg, time_value_t t, const int playerID)
{
  ectx e("light_manager::frame_advance");

  // get all lights that can possibly affect us, sorted by brightness
  scanlites.reset_lights(reg, bound, dynamic_only, playerID);

  goal_ambient = scanlites.ambient_factor; //*my_ambient;
  goal_ambient.clamp();

  int num_lites = scanlites.lights.size();

  cur_max_lights = max_lights;

  // this code causes flicker:  you light, your framerate drops, you don't light, your framerate goes up, etc.
  #if defined(TARGET_PC) && 0
  extern game* g_game_ptr;
  if (cur_max_lights > 1)
  {
    if (g_game_ptr->get_total_delta() > 1.0f/20) // skip this minor visual improvement if we're getting slow framerates
      cur_max_lights = 1;
  }
  else if (cur_max_lights > 2)
  {
    if (g_game_ptr->get_total_delta() > 1.0f/27) // skip this minor visual improvement if we're getting slow framerates
      cur_max_lights = 2;
  }
  #endif

  // If an old light doesn't exist in scanlites, zero its source pointer
  int i;
  for (i=lights.size(); --i>=0; )
  {
    light_rec* lite = &lights[i];
    if (!lite->source)
    {
      //if (lite->current_intensity<=0) // this is handled later
      //  lights.erase(lights.begin()+i);
      continue;
    }
    if (i>=(int)cur_max_lights)  // if code somehow decreased the number of max lights, kick extras out
    {
      //debug_print("removing extra light");
      lite->source = NULL;
    }
    int j;
    for (j=num_lites; --j>=0; )
    {
      light_source* ls = scanlites.lights[j].light;
      if (ls==lite->source)
        break;
    }
    if (j<0)
    {
      //debug_print("light no longer available");
      lite->source = NULL; // not found
    }
  }
  // allow stronger lights to replace weaker ones
  for ( i=0; i<num_lites; ++i )
  {
    if (compare_light(i) < 0) // couldn't place it.
      break; // Since strongest lights are first, there's no way any subsequent lights will be placed.
  }

  static const float fadefactor=3.0f; // 1/3 second
  //static const float fadefactor=0.5f; // 2 seconds

  float liteinc = t*fadefactor;

#ifndef PROJECT_KELLYSLATER	// this code causes the lights to shift at the beginning of the level (dc 06/26/02)
  color diff = goal_ambient-last_ambient;
  float dist2=sqr(diff.r)+sqr(diff.g)+sqr(diff.b);

  float maxallowdist = liteinc*0.6667f; // max color delta per second
  if (dist2>sqr(maxallowdist))
  {
    diff *= maxallowdist*fast_recip_sqrt(dist2);
    last_ambient += diff;
  } else
#endif
    last_ambient = goal_ambient;

  for (i=lights.size(); --i>=0; )
  {
    light_rec* lite = &lights[i];
    if (!lite->source)
    {
      lite->current_intensity -= liteinc;
      if (lite->current_intensity <= 0.0f)
      {
        //debug_print("light turned off");
        lights.erase(lights.begin()+i);
      }
    }
    else
    {
      lite->current_intensity += liteinc;
      if ( lite->current_intensity > 1.0f )
        lite->current_intensity = 1.0f;
    }
  }
}

void light_manager::prepare_for_rendering(use_light_context *lites)
{
  ectx e("light_manager::prepare_for_rendering");
  int point_light_iterator = 0;
  int dir_light_iterator = 0;
  lites->dir_lights.resize(max_lights);
  lites->point_light_dists.resize(max_lights);

  // this should really be based on how far the thing is from the light in relation to
  // its radius.
  bool convert_omni_to_dir = !allow_omni && os_developer_options::inst()->is_flagged(os_developer_options::FLAG_FAKE_POINT_LIGHTS);
  assert(lights.size()<=max_lights);
  for (int i=lights.size(); --i>=0; )
  {
    light_rec* lite = &lights[i];
    light_source* ls = lite->source;
    // no xforming of lights pos or dir is done, we leave it all in world coordinates for now.
    rational_t current_intensity = lite->current_intensity;
    if (current_intensity > 0.0f)
    {
      light_properties* props;
      if (lite->props.get_flavor() == LIGHT_FLAVOR_POINT)
      {
        if (!convert_omni_to_dir)
        {
          props = &lites->point_light_props[point_light_iterator];
          *props = lite->props;
          props->set_color(props->get_color() * current_intensity);
          lites->point_light_dists[point_light_iterator].second = point_light_iterator;
          lites->point_lights[point_light_iterator].pos = lite->dir_or_pos;
          lites->point_lights[point_light_iterator].light = ls;
          ++point_light_iterator;
        }
        else
        {
          props = &lites->dir_light_props[dir_light_iterator];
          *props = lite->props;
          float ldist = (lite->dir_or_pos - bound.get_center()).length()-bound.get_radius();
          float influence = props->get_influence(ldist); // must do this while the props are still point flavor!
          props->set_flavor(LIGHT_FLAVOR_DIRECTIONAL); // must happen after computing influence
          props->set_color(props->get_color() * (current_intensity * influence));
          lites->dir_lights[dir_light_iterator].dir = (lite->dir_or_pos-bound.get_center()).normalize();
          lites->dir_lights[dir_light_iterator].light = ls;
          ++dir_light_iterator;
        }
      }
      else
      {
        props = &lites->dir_light_props[dir_light_iterator];
        *props = lite->props;
        props->set_color(props->get_color() * current_intensity);
        lites->dir_lights[dir_light_iterator].dir = lite->dir_or_pos;
        lites->dir_lights[dir_light_iterator].light = ls;
        ++dir_light_iterator;
      }
    }
  }
  lites->dir_lights.resize( dir_light_iterator );
  lites->point_light_dists.resize( point_light_iterator );
  lites->ambient_factor = last_ambient;
}



extern vector3d bone_lights[MAX_BONES][ABSOLUTE_MAX_LIGHTS];


void use_light_context::xform_lights_to_local(const po& world2local, int num_bones, render_flavor_t render_flavor )  // <<<< add num_bones and integrate with render_skin
{
  // put lights into first bone:  one bone is all we need for non-skinned rendering
  assert(num_bones==1);
  int j;
  int num_point_lights = point_light_dists.size();
  int num_dir_lights = dir_lights.size();
  assert( (num_dir_lights + num_point_lights) <= ABSOLUTE_MAX_LIGHTS);
  for (j=0; j<num_dir_lights; ++j)
  {
    //bone_lights[0][j] = lites.dir_lights[j].dir;  // dir_lights already xformed by reset_lights or use_light_set
    bone_lights[0][j] = world2local.non_affine_slow_xform(dir_lights[j].dir);
  }
  for (j=0; j<num_point_lights; ++j)
  {
    int light_idx = point_light_dists[j].second;
    int jidx = j+num_dir_lights;
    bone_lights[0][jidx]=world2local.slow_xform(point_lights[light_idx].pos);
    dir_light_props[jidx] = point_light_props[light_idx];
    /* // this has already been done by now.  We're not likely to ever execute this code anyway.
    if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_FAKE_POINT_LIGHTS ) && (!(render_flavor&RENDER_REAL_POINT_LIGHTS)))
    {
      rational_t distance = bone_lights[0][jidx].length()+0.0001f;
      bone_lights[0][jidx] /= distance;  // normalized and inverted
      distance /= dir_light_props[jidx].get_near_range();
      if( distance<1 ) distance=1;
      float invdistance=1.0f/distance;
      dir_light_props[jidx].diffuse_color *= invdistance;  // attenuated
      dir_light_props[jidx].additive_color *= invdistance;
    }
    */
  }
}

// transform lights to bone space
void use_light_context::transform_lights_to_bone_space(const vector3d& pos,const matrix4x4* bones_world, int num_bones)
{
  int i,j;
  size_t num_dir_lights = dir_lights.size();
  size_t num_point_lights = min( ABSOLUTE_MAX_LIGHTS - num_dir_lights, point_light_dists.size() );
  assert( num_bones< MAX_BONES );
  assert( (num_dir_lights + num_point_lights) <= ABSOLUTE_MAX_LIGHTS);

  for(j=0;j<(int)num_dir_lights;++j)
  {
    for(i=0;i<num_bones;++i)
    {
      vector3d xformed_light = xform3d_0(bones_world[i], dir_lights[j].dir);
      light_matrices[MX_BONES+i][0][j+1] = xformed_light.x;
      light_matrices[MX_BONES+i][1][j+1] = xformed_light.y;
      light_matrices[MX_BONES+i][2][j+1] = xformed_light.z;
    }
  }
  int k;
  for(j=0,k=num_dir_lights;j<(int)num_point_lights;++j,++k)
  {
    int light_idx = point_light_dists[j].second;
    for(i=0;i<num_bones;++i)
    {
      vector3d xformed_light;
      xformed_light=xform3d_1(bones_world[i],point_lights[light_idx].pos);
      // turn into directional light
      // normalize light
      rational_t recip_distance = fast_recip_length( xformed_light.x, xformed_light.y, xformed_light.z );
      rational_t x1x = xformed_light.x * recip_distance;
      rational_t x1y = xformed_light.y * recip_distance;
      rational_t x1z = xformed_light.z * recip_distance;
      light_matrices[MX_BONES+i][0][k+1] = x1x;
      light_matrices[MX_BONES+i][1][k+1] = x1y;
      light_matrices[MX_BONES+i][2][k+1] = x1z;
    }  // <<<< IDEA!  scale light_matrices instead of dir_light_props for distance attenuation
    dir_light_props[ k ] = point_light_props[light_idx];
    /* // this has already been done by now.  We're not likely to ever execute this code anyway.
    if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_FAKE_POINT_LIGHTS ) )
    {
      rational_t distance2 = ( pos - point_lights[light_idx].pos ).length2();
      float near_range = dir_light_props[ k ].get_near_range();
      float recip_distance = 1.0f;
      if( sqr(near_range) < distance2 )
      {
        recip_distance = fast_recip_sqrt(distance2) * near_range;
      }
      dir_light_props[ num_dir_lights+j ].diffuse_color *= recip_distance;  // attenuated and inverted
    }
    */
  }
}

//////////////////////////////////



void use_light_context::clear_lights()
{
  dir_lights.resize(0);
  point_lights.resize(0);
  point_light_dists.resize(0);
  ambient_factor=color_white;
}


//////////////////////////////////


static light_types lt;

matrix4x4* light_matrices;

/*
void c_light(hw_rasta_vert* src_vert_list,
             int count,
             vector3d lighting_table[][ABSOLUTE_MAX_LIGHTS],
             color ambient_color,
             light_properties dir_light_props[ABSOLUTE_MAX_LIGHTS],
             int num_dir_lights,
             int num_point_lights,
             unsigned flags,
             use_light_context::light_dist* my_light_dists,
             hw_rasta_vert* dest_vert_list
             )
{
  if (num_point_lights||num_dir_lights||(flags&LIGHT_DIFFUSE))
  {
    const hw_rasta_vert* tvlit;
    hw_rasta_vert* dvlit;
    hw_rasta_vert* vert_list_end = src_vert_list+count;
    color color_ambient;
    if ( flags&LIGHT_DIFFUSE )
      color_ambient = ambient_color;
    else
      color_ambient = color_black;

    for (tvlit=src_vert_list,dvlit=dest_vert_list; tvlit!=vert_list_end; ++tvlit,++dvlit)
    {
    #if defined(TARGET_MKS)
      if ( !(tvlit->clip_flags & (FLAG_SELFLIT|CLIP_NOTINLOD) ) )
    #endif
      {
        int bonin = tvlit->specular.c.a;

        color diffuse(color_ambient), additive(color_black);
        vector3d vertex_normal = tvlit->get_normal();
        int j;
        for (j=num_dir_lights; --j>=0; )
        {
          rational_t lighting_factor = -dot(vertex_normal, bone_lights[bonin][j]); // the normal is going the wrong way!
          if(lighting_factor>0)
          {
            diffuse += lites.dir_lights[j].diffuse * lighting_factor;
            additive += lites.dir_lights[j].additive * lighting_factor;
          }
        }
        for(j=num_point_lights; --j>=0; )
        {
          int light_idx = my_light_dists[j].second;
          vector3d* lpl = &bone_lights[bonin][j+num_dir_lights];
          vector3d xyz = tvlit->xyz;
          vector3d light_vector = *lpl - xyz;   // needs transform
          rational_t distance2 = light_vector.length2();
          light_properties* props = &lites.point_light_props[ light_idx ];
          if( distance2 < sqr(props->get_cutoff_range()) )
          {
            rational_t lighting_factor;
            lighting_factor = dot(vertex_normal, light_vector);
            if(lighting_factor>0)
            {
              lighting_factor *= props->get_influence(__fsqrt(distance2));
              diffuse += props->get_color()*lighting_factor;
              additive += props->get_additive_color()*lighting_factor;
            }
          }
        }
        diffuse.clamp();
        additive.clamp();
        // mix vertex color into diffuse
        diffuse.r *= tvlit->diffuse.c.r * (1.0f/255);
        diffuse.g *= tvlit->diffuse.c.g * (1.0f/255);
        diffuse.b *= tvlit->diffuse.c.b * (1.0f/255);
        diffuse.a  = tvlit->diffuse.c.a * (1.0f/255);
        dvlit->diffuse = diffuse.to_color32();
        dvlit->specular = additive.to_color32();
      }
    }
  }
}
*/

// you might notice that while this is pretty it is in fact
// slower than c_light.  All this is is a template for a sega
// assembly function:  it should jamn on the sega.
void sweetlight(hw_rasta_vert* src_vert_list,
              int count,
              vector3d lighting_table[][ABSOLUTE_MAX_LIGHTS],
              light_properties dir_light_props[ABSOLUTE_MAX_LIGHTS],
              int num_dir_lights,
              int num_point_lights,        // dir_lights + point_lights
              unsigned flags,
              use_light_context::light_dist* my_light_dists,
              hw_rasta_vert_lit* dest_vert_list,
              int alpha,
              int num_bones
              )
{
  assert(src_vert_list);
  assert(count);

  if(( num_point_lights )||(num_dir_lights)||(flags&LIGHT_DIFFUSE))
  {
    // prepare color matrix, which was partially initialized at program start (mostly 0's, plus 1's for ambient light in bones)
    c_sweetlight_inner(src_vert_list,
                       dest_vert_list,
                       light_matrices,
                       count,
                       lighting_table,
                       dir_light_props,
                       lt,
                       flags
                       ); // where's the alpha parameter?
  }
  else
  { // we need to copy the src verts to the destination even though we're not lighting
    hw_rasta_vert* tvlit;
    hw_rasta_vert_lit* dest;
    // strip out normal as we copy
    if (alpha<255)
      for (tvlit=src_vert_list, dest=dest_vert_list; --count>=0; ++tvlit,++dest)
      {
        dest->set_xyz(tvlit->get_unxform());
        dest->tc[0] = tvlit->tc[0];
        dest->diffuse.i = tvlit->diffuse.i;
        dest->diffuse.c.a = (alpha*dest->diffuse.c.a)>>8;
      }
    else
      for (tvlit=src_vert_list, dest=dest_vert_list; --count>=0; ++tvlit,++dest)
      {
        dest->set_xyz(tvlit->get_unxform());
        dest->tc[0] = tvlit->tc[0];
        dest->diffuse.i = tvlit->diffuse.i;
      }
  }
}


void c_sweetlight_inner(hw_rasta_vert* src_vert_list,
                        hw_rasta_vert_lit* dest_vert_list,
                        matrix4x4* light_matrices,  // note:  the first lighting_matrix is the diffuse lighting table, the second is specular, and
                                                       // from then on you have the light vector matrices for each bone
                        int count,
                        vector3d lighting_table[][ABSOLUTE_MAX_LIGHTS],
                        light_properties dir_light_props[ABSOLUTE_MAX_LIGHTS],
                        light_types lt,
                        unsigned flags)

{
  vector4d light_intensities;
  light_intensities[0] = 1.0f;
  hw_rasta_vert* tvlit;
  hw_rasta_vert* vert_list_end = src_vert_list+count;
  hw_rasta_vert_lit* dvlit;
  int column;
  for (tvlit=src_vert_list,dvlit=dest_vert_list; tvlit!=vert_list_end; ++tvlit,++dvlit)
  {
    if( !(tvlit->clip_flags & FLAG_SELFLIT) )
    {
      if( !(tvlit->clip_flags & CLIP_NOTINLOD) )
      {
        column = 1;
        int bonin = tvlit->boneid();

        vector4d xnormal(tvlit->get_normal(),1.0f);

        float distance2s[4];
        float near_ranges[4];
        matrix4x4 copymat;
        int j;
        copymat = light_matrices[MX_BONES+bonin];
        for(j=1;j<4;++j)
        {
          if( lt.lt[j]==ltfNone )
          {
            break;
          }
          else if( lt.lt[j]==ltfDir )
          {
            // do nothing, we're a directional light and pre-prepared
          }
          else
          {
            assert(lt.lt[j]==ltfPoint);
            copymat[0][j] -= tvlit->xyz.x;
            copymat[1][j] -= tvlit->xyz.y;
            copymat[2][j] -= tvlit->xyz.z;

            distance2s[j] = copymat[0][j] * copymat[0][j] +
                            copymat[1][j] * copymat[1][j] +
                            copymat[2][j] * copymat[2][j];

            near_ranges[j] = dir_light_props[j-1].get_near_range();
          }
        }
        vector4d lighting_factors = xform4d( copymat, xnormal );
        for(j=1;j<4;++j)
        {
          if( lt.lt[j]==ltfPoint )
          {
            // 'normalizing' by squared distance instead of distance
            // means we're already factoring in distance attenuation
            lighting_factors[j] /= distance2s[j];
            lighting_factors[j] *= dir_light_props[j].get_near_range();
          }
          if(lighting_factors[j]<0)
            lighting_factors[j]=0;
        }
#ifdef WADE_LIGHTING
        dvlit->diffuse = tvlit->diffuse;
        for( int i=0; i<4; i++ )
        {
          vector4d thislight;
          thislight[0] = light_matrices[MX_COLORS][i][0];
          thislight[1] = light_matrices[MX_COLORS][i][1];
          thislight[2] = light_matrices[MX_COLORS][i][2];
          thislight[3] = 0.0f;
          thislight *= (lighting_factors[i]+1);
          dvlit->diffuse *= thislight;
        }
#else
        vector4d diffuse_result = xform4d( light_matrices[MX_COLORS], lighting_factors );
        //if( !(flags & LIGHT_ADDITIVE) )
        {
          diffuse_result[0] *= tvlit->diffuse.c.r * 2;
          diffuse_result[1] *= tvlit->diffuse.c.g * 2;
          diffuse_result[2] *= tvlit->diffuse.c.b * 2;
        }
        if( diffuse_result[0]>255) diffuse_result[0]=255;
        if( diffuse_result[1]>255) diffuse_result[1]=255;
        if( diffuse_result[2]>255) diffuse_result[2]=255;
        //if( flags & LIGHT_ADDITIVE )
        //{
        //  dvlit->specular.c.r = diffuse_result[0];
        //  dvlit->specular.c.g = diffuse_result[1];
        //  dvlit->specular.c.b = diffuse_result[2];
        //}
        //else
        {
          dvlit->diffuse.c.r = (uint8)diffuse_result[0];
          dvlit->diffuse.c.g = (uint8)diffuse_result[1];
          dvlit->diffuse.c.b = (uint8)diffuse_result[2];
        }
        dvlit->diffuse.c.a = tvlit->diffuse.c.a;
        #ifdef DEBUG
        //dvlit->specular.c.g = 64;
        #endif
#endif
      }
    }
  }
}


void c_onelight(hw_rasta_vert* src_vert_list,
                int count,
                light_properties* light_info,
                unsigned flags,
                hw_rasta_vert_lit* dest_vert_list,
                color32 ambientc)
{
  vector4d light_intensities;
  light_intensities[0] = 1.0f;
  hw_rasta_vert* tvlit;
  hw_rasta_vert* vert_list_end = src_vert_list+count;
  hw_rasta_vert_lit* dvlit;
  color32 lite_diffuse = light_info->get_color32();
//  color32 lite_additive = light_info->get_additive_color32(); // unused -- remove me?
  for (tvlit=src_vert_list,dvlit=dest_vert_list; tvlit!=vert_list_end; ++tvlit,++dvlit)
  {
    if( !(tvlit->clip_flags & FLAG_SELFLIT) )
    {
      if( !(tvlit->clip_flags & CLIP_NOTINLOD) )
      {
        int bonin = tvlit->boneid();
        matrix4x4& light_matrix = light_matrices[MX_BONES+bonin];

        vector3d local_normal;
        local_normal = tvlit->get_normal();
        float lighting_factor =
          light_matrix[0][1]*local_normal.x + light_matrix[1][1]*local_normal.y + light_matrix[2][1]*local_normal.z;
        /*
        if( flags & LIGHT_ADDITIVE )
        {
          if(lighting_factor>0)
          {
            int integral_lf = lighting_factor * 256;
            dvlit->specular.c.r = lite_additive.c.r * integral_lf >> 8;
            dvlit->specular.c.g = lite_additive.c.g * integral_lf >> 8;
            dvlit->specular.c.b = lite_additive.c.b * integral_lf >> 8;
          }
          else
          {
            dvlit->specular.c.r = 0;
            dvlit->specular.c.g = 0;
            dvlit->specular.c.b = 0;
          }
        }
        else
        */
#ifdef WADE_LIGHTING
        dvlit->diffuse = tvlit->diffuse * ambientc;
        dvlit->diffuse *= (lighting_factor+1);
        dvlit->diffuse *= lite_diffuse;
#else
        {
          if(lighting_factor>0)
          {
            int integral_lf = (int)(lighting_factor * 256);
            // it uses one-light-only for guys, therefore their diffuse is 0xff,
            // and we don't have to do the extra mul
            // why were we storing lighting in specular when we ignore diffuse anyhow?
            int temp;
            temp = ambientc.c.r + ( lite_diffuse.c.r * integral_lf >> 8 );
            if( temp > 255 )
              temp = 255;
            dvlit->diffuse.c.r = tvlit->diffuse.c.r * temp >> 8;

            temp = ambientc.c.g + ( lite_diffuse.c.g * integral_lf >> 8 );
            if( temp > 255 )
              temp = 255;
            dvlit->diffuse.c.g = tvlit->diffuse.c.g * temp >> 8;

            temp = ambientc.c.b + ( lite_diffuse.c.b * integral_lf >> 8 );
            if( temp > 255 )
              temp = 255;
            dvlit->diffuse.c.b = tvlit->diffuse.c.b * temp >> 8;
          }
          else
            dvlit->diffuse = ambientc;
        }
#endif
      }
    }
  }
}


void prepare_lighting_matrices( color ambient_color,
                                light_properties* light_props_table,
                                int num_dir_lights,
                                int num_point_lights,
                                unsigned flags,
                                int num_bones )

{
  assert(num_point_lights + num_dir_lights <= 3);
  if ((num_point_lights|num_dir_lights) || (flags&LIGHT_DIFFUSE))
  {
    // prepare color matrix, which was partially initialized at program start (mostly 0's, plus 1's for ambient light in bones)
    light_matrices[MX_COLORS][0][0] = ambient_color.r;
    light_matrices[MX_COLORS][0][1] = ambient_color.g;
    light_matrices[MX_COLORS][0][2] = ambient_color.b;
    int column=1;
    int i;
    for (i=0; i<num_dir_lights+num_point_lights; ++i)
    {
      if (flags&LIGHT_ADDITIVE) // specular
      {
        light_matrices[MX_COLORS][column][0] = light_props_table[i].get_additive_color().r * 255;
        light_matrices[MX_COLORS][column][1] = light_props_table[i].get_additive_color().g * 255;
        light_matrices[MX_COLORS][column][2] = light_props_table[i].get_additive_color().b * 255;
      }
      else
      { // the reason this doesn't have * 255 is that it gets multiplied by the vertex color,
        // which is already 0..255
        light_matrices[MX_COLORS][column][0] = light_props_table[i].get_color().r;
        light_matrices[MX_COLORS][column][1] = light_props_table[i].get_color().g;
        light_matrices[MX_COLORS][column][2] = light_props_table[i].get_color().b;
      }
      for (int j=0; j<num_bones; ++j)
      {
        light_matrices[MX_BONES+j][0][column] = bone_lights[j][i].x;
        light_matrices[MX_BONES+j][1][column] = bone_lights[j][i].y;
        light_matrices[MX_BONES+j][2][column] = bone_lights[j][i].z;
      }
      ++column;
    }
    for (i=column; i<4; ++i)
    {
      light_matrices[MX_COLORS][i][0] = 0;
      light_matrices[MX_COLORS][i][1] = 0;
      light_matrices[MX_COLORS][i][2] = 0;
      // light_vector_matrix values have to get set on the fly per vertex
    }
    lt.lt[0]=ltfNone;
    int lt_index = 1;
    for (i=num_dir_lights; --i>=0; )
      lt.lt[lt_index++]=ltfDir;
    for (i=num_point_lights; --i>=0; )
      lt.lt[lt_index++]=ltfPoint;
    while (lt_index < 4)
      lt.lt[lt_index++]=ltfNone;
  }
}


void initialize_lighting_matrices()
{
  light_matrices = (matrix4x4*)os_malloc((MAX_BONES+1)*sizeof(matrix4x4));
  memset(light_matrices,0,(MAX_BONES+1)*sizeof(matrix4x4) );
  for (int j=MAX_BONES; --j>=0; )
  {
    light_matrices[MX_BONES+j][3][0] = 1;  // guarantees a 1.0f multiplier on ambient no matter what direction vert is
  }
}
