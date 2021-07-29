// widget_entity.cpp

#include "global.h"

#include "app.h"
#include "game.h"
#include "projconst.h"
#include "camera.h"
#include "iri.h"
#include "widget.h"
#include "widget_entity.h"
#include "conglom.h"
#include "entityflags.h"
#include "entity_maker.h"
#include "anim_maker.h"
#include "wds.h"
#include "pmesh.h"
#include "geomgr.h"
#include "time_interface.h"

#include <cstdarg>
#include <algorithm>



//------------------------------------------------------------------



entity_widget::entity_widget( const char *_widget_name, widget *_parent, short _x, short _y, const char *ent_file )
:   widget( _widget_name, _parent, _x, _y )
{
  type = WTYPE_Entity;

  rhw_half_range = (rhw_layer_ranges[rhw_3d_layer][1]-rhw_layer_ranges[rhw_3d_layer][0])/2;
  rhw_midpoint = rhw_layer_ranges[rhw_3d_layer][0] + rhw_half_range;
  z = get_pc_z( rhw_midpoint );

  axis = vector3d( 0.0f, -1.0f, 0.0f );
  ax = ay = az = 0.0f;
  rps = 0.0f;

  if ( ent_file )
  {
    stringx entity_name = get_fname_wo_ext( stringx(ent_file) );
    entity_name.to_upper();
    stringx entity_dir = get_dir( stringx(ent_file) );
//    entity_id eid = entity_id( entity_name.c_str() ); // unused -- remove me?

    g_entity_maker->set_owning_widget( this );
    ent = g_entity_maker->create_entity_or_subclass( entity_name,
                                             entity_id::make_unique_id(),
                                             po_identity_matrix,
                                             entity_dir,
                                             ACTIVE_FLAG | NONSTATIC_FLAG );
    g_entity_maker->set_owning_widget( NULL );

    set_rotation( 90.0f * PI / 180.0f, 180.0f * PI / 180.0f, 0 );  // rotate to face the camera
  }
  else
  {
    ent = NULL;
  }
}


void entity_widget::set_layer( rhw_layer_e rhw_layer )
{
  set_rhw_3d_layer(rhw_layer);

  rhw_half_range = (rhw_layer_ranges[rhw_3d_layer][1]-rhw_layer_ranges[rhw_3d_layer][0])/2;
  rhw_midpoint = rhw_layer_ranges[rhw_3d_layer][0] + rhw_half_range;
  z = get_pc_z( rhw_midpoint );

  restore_last_rhw_3d_layer();
}

entity_widget::~entity_widget()
{
  anims.resize(0);
  if ( ent )
  {
    g_entity_maker->destroy_entity( ent );
    ent = NULL;
  }
}


void entity_widget::show()
{
  widget::show();
  update_rot();
}



// The entity_widget assumes responsibility for deleting anims that get added here.
// NOTE: this function assumes the given anim has not previously been added
void entity_widget::add_anim( entity_anim_tree* new_anim )
{
  pentity_anim_tree_vector::iterator i = find( anims.begin(), anims.end(), (entity_anim_tree*)NULL );

  if ( i == anims.end() )
    anims.push_back( new_anim );
  else
    (*i) = new_anim;
}


// This deconstructs the given anim.
void entity_widget::kill_anim( entity_anim_tree* the_anim )
{
  pentity_anim_tree_vector::iterator i = find( anims.begin(), anims.end(), the_anim );
  if ( i != anims.end() )
  {
    entity_anim_tree* a = *i;
    a->get_entity()->clear_anim( a );
    *i = NULL;
  }
}


void entity_widget::frame_advance_entity( entity *e, time_value_t time_inc )
{
  if ( e )
  {
    e->frame_advance( CALC_ENTITY_TIME_DILATION(time_inc, e) );

    // advance playing ifls
    e->advance_age( CALC_ENTITY_TIME_DILATION(time_inc, e) );

    // now recurse through any children
    if ( e->is_a_conglomerate() )
    {
      pentity_vector children = ((conglomerate*)e)->get_members();
      pentity_vector::iterator vi;
      pentity_vector::iterator vi_end = children.end();
      for ( vi = children.begin(); vi != vi_end; ++vi )
      {
        frame_advance_entity( (*vi), CALC_ENTITY_TIME_DILATION(time_inc, (*vi)) );
      }
    }
  }
}



void entity_widget::frame_advance( time_value_t time_inc )
{
  // update any rotation
  if ( rps )
  {
    angle += rps * time_inc;
    angle = fmodf(angle,2.0f*PI);
    update_entity_po();
  }

  frame_advance_entity( ent, time_inc );

  // frame advance animations for this entity
  pentity_anim_tree_vector::iterator ani = anims.begin();
  pentity_anim_tree_vector::iterator ani_end = anims.end();
  for ( ; ani!=ani_end; ++ani )
  {
    entity_anim_tree* anm = *ani;
    if ( anm )
    {
      if ( !anm->is_suspended() && !anm->is_finished() )
      {
        if ( anm->is_relative_to_start() )
          anm->reset_root_position();

        anm->frame_advance( CALC_ENTITY_TIME_DILATION(time_inc, anm->get_entity()) );

        update_entity_po();
      }

      if ( anm->is_autokill() && anm->is_finished() )
        kill_anim( anm );
    }
  }
  widget::frame_advance( time_inc );

}


void entity_widget::render_entity( entity *e, camera* camera_link )
{
  if ( e )
  {
    visual_rep *vrep = e->get_vrep();
    if ( vrep )
    {
      float prevcopx,prevcopy,prevminz,prevmaxz;
      geometry_manager::inst()->get_cop(&prevcopx,&prevcopy,&prevminz,&prevmaxz);

      color cl = color(1.0f, 1.0f, 1.0f, 1.0f);
      rational_t v[2] = { 0.0f, 0.0f };
      transform(v, cl, 0);

#ifdef TARGET_MKS
      cur_special_w_xform = calc_special_w_xform( e->get_abs_po().get_matrix(), v[0], v[1] );
#endif

      geometry_manager::inst()->set_cop(v[0], v[1], VREPWIDGET_MIN_Z, VREPWIDGET_MAX_Z);

      render_flavor_t flavor = 0;
#ifdef TARGET_PC
      if ( abs_col[0].a == 1.0f )
      {
        flavor = RENDER_ENTITY_WIDGET|RENDER_OPAQUE_PORTION;
        e->render( camera_link, vrep->get_max_faces(0), flavor, cl.a );
      }
      flavor = RENDER_ENTITY_WIDGET|RENDER_TRANSLUCENT_PORTION;
      e->render( camera_link, vrep->get_max_faces(0), flavor, cl.a );
#else
      flavor = RENDER_ENTITY_WIDGET|RENDER_TRANSLUCENT_PORTION|RENDER_OPAQUE_PORTION;
      e->render( camera_link, vrep->get_max_faces(0), flavor, cl.a );
#endif

      geometry_manager::inst()->set_cop(prevcopx,prevcopy,prevminz,prevmaxz);
    }

    // recurse through children
    if ( e->is_a_conglomerate() )
    {
      pentity_vector children = ((conglomerate*)e)->get_members();
      pentity_vector::iterator vi;
      pentity_vector::iterator vi_end = children.end();
      for ( vi = children.begin(); vi != vi_end; ++vi )
      {
        render_entity( *vi, camera_link );
      }
    }
  }
}


void entity_widget::render( camera* camera_link )
{
  if ( is_shown() && abs_col[0].a > 0.0f )
  {
    cur_rhw_midpoint = rhw_midpoint;
    cur_rhw_half_range = rhw_half_range;
    cur_largest_z = largest_z;

//    hw_rasta::inst()->set_zbuffering( true, true );
    render_entity( ent, camera_link );
//    hw_rasta::inst()->set_zbuffering( true, false );

    widget::render();
  }
}



void entity_widget::update_pos()
{
  widget::update_pos();
  update_entity_po();
}


void entity_widget::update_scale()
{
  widget::update_scale();
  update_entity_po();
}


void entity_widget::update_rot()
{
  widget::update_rot();

  matrix4x4 R, Rx, Ry, Rz;
  R.make_rotate( axis, angle );
  Rx.make_rotate( vector3d( 1, 0, 0 ), ax );
  Ry.make_rotate( vector3d( 0, 1, 0 ), ay );
  Rz.make_rotate( vector3d( 0, 0, 1 ), az );
  rot_matrix = R * Rx * Ry * Rz;

  update_entity_po();
}


void entity_widget::set_rotation( rational_t _ax, rational_t _ay, rational_t _az )
{
  ax = _ax;
  ay = _ay;
  az = _az;
  update_rot();
}

void entity_widget::update_entity_po()
{
  if ( ent )
  {
    matrix4x4 mat, S, T1; // , T0; // unused -- remove me?

    S.make_scale( vector3d(abs_S[0], abs_S[0], abs_S[0]) );
#ifdef TARGET_PC
    T1.make_translate( vector3d(0.0f, 0.0f, z ) );
#else
    T1.make_translate( vector3d(0.0f, 0.0f, 0.1f - ent->get_radius()*abs_S[0]*0.7f ) );
#endif
    mat = S * rot_matrix * T1;
    ent->set_rel_po( po(mat) );

#ifdef TARGET_MKS
    calc_largest_z( ent );
#endif
  }
}


void entity_widget::set_ent( entity *_ent )
{
  ent = _ent;
  update_rot();
}


#ifdef TARGET_MKS
// this is done for congloms, since we need a largest z for whole thing
void entity_widget::calc_largest_z( entity *e )
{
  if ( e )
  {
    if ( e == ent )
      largest_z = 0.0f;

    visual_rep *vrep = e->get_vrep();
    if ( vrep && vrep->get_type()==VISREP_PMESH )
    {
      color cl = color(1.0f, 1.0f, 1.0f, 1.0f);
      rational_t v[2] = { 0.0f, 0.0f };
      transform(v, cl, 0);
      cur_special_w_xform = calc_special_w_xform( e->get_abs_po().get_matrix(), v[0], v[1] );
      vr_pmesh *mesh = (vr_pmesh*)vrep;
      int i = 0;
      int num_verts = mesh->get_num_wedges();
      for ( ; i < num_verts; ++i )
      {
        vector3d xyz = mesh->get_xvert_unxform_pos(i);
        // apply rotation to z
        rational_t abs_z = abs(calc_z(mesh->get_xvert_unxform_pos(i)));
        if ( abs_z > largest_z )
          largest_z = abs_z;
      }
    }

    // recurse through children
    if ( e->is_a_conglomerate() )
    {
      vector<entity*> children = ((conglomerate*)e)->get_members();
      vector<entity*>::iterator vi;
      vector<entity*>::iterator vi_end = children.end();
      for ( vi = children.begin(); vi != vi_end; ++vi )
      {
        calc_largest_z( *vi );
      }
    }
  }
}
#endif

