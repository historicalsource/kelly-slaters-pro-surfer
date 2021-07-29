// debug_render.h
// Copyright (c) 2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
#ifndef DEBUG_RENDER_H
#define DEBUG_RENDER_H

#include "algebra.h"
#include "color.h"
#include "capsule.h"
#include "ostimer.h"

class plane;
class collision_capsule;
//class po;
class text_font;

void render_triangle(const vector3d& pt1, const vector3d& pt2, const vector3d& pt3,
                     color32 col = color32(0, 255, 0, 96), bool double_sided=false);

void render_quad(const vector3d& pt1, const vector3d& pt2, const vector3d& pt3, const vector3d& pt4,
                 color32 col = color32(0, 255, 0, 96), bool double_sided=false);

void render_quad_2d(rational_t l,rational_t t,rational_t r,rational_t b, rational_t z=0.0f,
                 color32 col = color32(0, 255, 0, 96));

void render_quad_2d(const vector2d& pos, const vector2d& size, rational_t z=0.0f,
                 color32 col = color32(0, 255, 0, 96));

void render_plane(const plane& pln, color32 front_color = color32(255, 128, 0, 96), color32 back_color=color32(0,0,0,0), rational_t size=2e3f );

void render_sphere(const vector3d &pos, rational_t radius, color32 col = color32(0, 0, 255, 96));

void render_capsule(const vector3d &base, const vector3d &end, rational_t radius, color32 col = color32(255, 0, 255, 96));

inline void render_capsule(const capsule &cap, color32 col = color32(255, 0, 255, 96))
{
  render_capsule(cap.base, cap.end, cap.radius, col);
}
inline void render_capsule(const collision_capsule &cap, color32 col = color32(255, 0, 255, 96))
{
  render_capsule(cap.get_abs_capsule(), col);
}

void render_cylinder(const vector3d &base, const vector3d &end, rational_t radius, color32 col = color32(0, 128, 255, 96));

void render_box(const vector3d &pos,
                const vector3d& halfsizex, const vector3d& halfsizey, const vector3d& halfsizez,
                color32 col = color32(0, 255, 128, 96));

// beams

void render_beam( const vector3d &pt1, const vector3d &pt2, color32 col, rational_t thickness = 0.05f );
void render_beam_box( const vector3d &tl, const vector3d &br, color32 col, rational_t thickness = 0.05f );
void render_beam_cube( const vector3d &pt, rational_t radius, color32 col, rational_t thickness = 0.05f );
//void render_oriented_beam_box( const vector3d &pt, const vector3d &br, const po &orient_po, color32 col, rational_t thickness = 0.05f );
//void render_oriented_beam_cube( const vector3d &pt, rational_t radius, const po &orient_po, color32 col, rational_t thickness = 0.05f );

inline void render_line( const vector3d &pt1, const vector3d &pt2, color32 col, rational_t thickness = 0.05f )
{
  render_beam(pt1,pt2,col,thickness);
}


#ifndef BUILD_BOOTABLE

class camera;
class collision_geometry;
class entity;
class partition3;

// frustum

void render_frustum(const camera& cam, const color32& col = color32(255, 255, 255, 128));

// markers

void frame_advance_markers(time_value_t);
void render_marker(  camera *camera_link, vector3d pos, color32 col = color32_white, rational_t scale = 1.0f);

// collision geometry

void render_colgeom(const collision_geometry *colgeom, color32 col = color32(255, 255, 0, 96), const entity *ent=NULL);

inline void render_colgeom(const entity *ent, color32 col = color32(255, 255, 0, 96))
{
  if(ent) render_colgeom(ent->get_colgeom(), col, ent);
}

// debug spheres

void clear_debug_spheres();
void add_debug_sphere(vector3d pos,rational_t radius);
void render_debug_spheres();

// capsule history

void clear_capsule_history();
void add_capsule_history(const collision_capsule& cap);
void render_capsule_history();

void add_3d_text(const vector3d& pos, color32 col, time_value_t dur, const stringx &str);
void add_2d_text(const vector3d& pos, color32 col, time_value_t dur, const stringx &str);
void frame_advance_debug_text(time_value_t t);
//void render_debug_text();




// plane debugging

#ifdef TARGET_PS2
enum { GLOBAL_PMESH_COUNT=0 };  // this takes like 10 seconds to load...
#else
enum { GLOBAL_PMESH_COUNT=50 };
#endif

extern int LAST_SHOW_PLANE;
extern int FIRST_SHOW_PLANE;
void render_debug_planes(vector<partition3 *> plane_list, vector<int> hit_dir_list, vector3d p);
void clear_planes();

#endif // !BUILD_BOOTABLE

// text

void render_text(const stringx& str, const vector2di& pos, color32 col=color32_white, float depth=0.0f, float size=1.0f, text_font* f=NULL);

void print_3d_text(const vector3d& pos, color32 col, const char *format, ...);


bool debug_render_init(); // initialize debug render subsystem
bool debug_render_done(); // shutdown debug render subsystem


#endif //DEBUG_RENDER_H
