// script_lib_entity.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "camera.h"
#include "script_lib_signal.h"
#include "script_lib_entity.h"
#include "entity.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "wds.h"
//!#include "character.h"
//#include "brain.h"
#include "script_lib_sound_inst.h"
#include "script_lib_sound_stream.h"
//!#include "script_lib_character.h"
#include "script_lib_item.h"
//#include "fxman.h"
#include "app.h"
#include "game.h"
#include "entityflags.h"
#include "entity_maker.h"
#include "hwaudio.h"
#include "terrain.h"

#include "item.h"
#include "physical_interface.h"
// BIGCULL #include "damage_interface.h"
#include "time_interface.h"
#include "sound_interface.h"


// read an entity value (by id) from a stream
void slc_entity_t::read_value(chunk_file& fs,char* buf)
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find entity and write value to buffer
  id.to_upper();
  *(vm_entity_t*)buf = (vm_entity_t)find_instance(id);
}

// find named instance of entity
unsigned slc_entity_t::find_instance(const stringx& n) const
{
  if (n=="NULL") return (unsigned)0;
  const entity* r = entity_manager::inst()->find_entity(entity_id(n.c_str()),IGNORE_FLAVOR,FIND_ENTITY_UNKNOWN_OK);
  if (!r)
    {
    error( "entity " + n + " not found" );
    }
  return (unsigned)r;
}

// script library function:  vector3d entity::get_abs_position()
class slf_entity_get_position_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_position_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
//    #pragma todo("try to speed this up, as it needs to be correct, but could be called several times...")
    parms->me->update_abs_po_reverse();
    vector3d result = parms->me->get_abs_position();
    SLF_RETURN;
    SLF_DONE;
  }
};

// script library function:  vector3d entity::get_detonate_position()
class slf_entity_get_detonate_position_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_detonate_position_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me->get_detonate_position();
    SLF_RETURN;
    SLF_DONE;
  }
};

// script library function:  vector3d entity::get_facing()
class slf_entity_get_facing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_facing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result;
//    #pragma todo("try to speed this up, as it needs to be correct, but could be called several times...")
    parms->me->update_abs_po_reverse();

    if(parms->me->get_flavor() == ENTITY_PARTICLE_GENERATOR)
      result = parms->me->get_abs_po().get_y_facing();
    else
      result = parms->me->get_abs_po().get_facing();
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  vector3d entity::get_rel_position()
class slf_entity_get_rel_position_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_rel_position_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me->get_rel_position();
    SLF_RETURN;
    SLF_DONE;
  }
};




// script library function:  vector3d entity::rel_angle(vector3d loc)
class slf_entity_rel_angle_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_rel_angle_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d loc;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    //      vector3d rel_loc = parms->me->get_abs_po().inverse_xform(parms->loc);
    vector3d rel_loc = parms->me->get_abs_po().fast_inverse_xform(parms->loc);

    rational_t result = safe_atan2(-rel_loc.x,rel_loc.z);
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  entity::set_parent(vector3d)
class slf_entity_set_parent_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_parent_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t parent;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( parms->me != g_world_ptr->get_marky_cam_ptr()
      || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority()
      )
    {
      parms->me->check_nonstatic();
      if ( parms->parent == NULL )
        parms->me->link_ifc()->clear_parent ();
      //  parms->me->set_parent_rel( NULL );  // clear parent without affecting position in world
      else
      {
        parms->me->link_ifc()->set_parent( parms->parent );
        parms->me->update_abs_po();
        parms->me->compute_sector( g_world_ptr->get_the_terrain() );
      }
    }
    SLF_DONE;
  }
};

// script library function:  entity::set_parent_rel(vector3d)
class slf_entity_set_parent_rel_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_parent_rel_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t parent;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( parms->me != g_world_ptr->get_marky_cam_ptr()
      || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority()
      )
    {
      parms->me->check_nonstatic();
      parms->me->link_ifc()->set_parent (parms->parent);
//    parms->me->set_parent_rel( parms->parent );  // preserve position in world
    }
    SLF_DONE;
  }
};

// script library function:  entity::set_rel_position(vector3d)
class slf_entity_set_position_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_position_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
      parms->me->check_nonstatic();
      parms->me->set_rel_position(parms->p);
      parms->me->compute_sector(g_world_ptr->get_the_terrain());
#if 0
      if(parms->me->has_sound_ifc())
        parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
/*!      if (parms->me->get_flavor()==ENTITY_CHARACTER && parms->me!=g_world_ptr->get_hero_ptr())
        {
          //        ((character *) parms->me)->get_brain()->reset_rest_location();
        }
!*/
#endif
      }

    SLF_DONE;
  }
};

// script library function:  entity::set_velocity(vector3d)
class slf_entity_set_velocity_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_velocity_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
      parms->me->check_nonstatic();
      // set velocity using our coordinate system

      //      vector3d world_velocity = parms->me->get_abs_po().non_affine_slow_xform( parms->p );
      vector3d world_velocity = parms->me->get_abs_po().fast_8byte_non_affine_xform( parms->p );

      if(parms->me->has_physical_ifc())
        parms->me->physical_ifc()->set_velocity(world_velocity);

/*!      if ( parms->me->get_flavor() == ENTITY_CHARACTER )
        // disable collision checking for the frame
        ((character *)parms->me)->set_moved_last_frame( true );
!*/
      }
    SLF_DONE;
  }
};


// script library function:  entity::look_at(vector3d)
class slf_entity_look_at_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_look_at_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if ( parms->me != g_world_ptr->get_marky_cam_ptr()
       || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority()
       )
      {
      parms->me->check_nonstatic();
      parms->me->look_at( parms->p );
      }

    SLF_DONE;
  }
};


// script library function:  entity::set_xz_facing(vector3d)
class slf_entity_set_xz_facing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_xz_facing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

//#pragma fixme("Whoever wrote it, this needs a comment. -WTB")
    // I didn't do it.  -jdf
    if ( parms->me != g_world_ptr->get_marky_cam_ptr()
      || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority() )
    {
      parms->me->check_nonstatic();
      if ( parms->p.y < 0.95f )
      {
        // compute orientation matrix
        vector3d oz = parms->p;
        oz.y = 0;
        oz.normalize();
        vector3d ox = cross( YVEC, oz );
        ox.normalize();
        // set NEW orientation
        po the_po = po(ox,YVEC,oz,parms->me->get_rel_position());
        parms->me->set_rel_po( the_po );
      }
    }

    SLF_DONE;
  }
};

// script library function:  entity::set_facing(vector3d,vector3d)
class slf_entity_set_facing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_facing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d z;
    vector3d y;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->check_nonstatic();

    // compute orientation matrix
    vector3d oz = parms->z;
    oz.normalize();
    vector3d oy = parms->y;
    oy.normalize();
    vector3d ox = cross( oy, oz );
    ox.normalize();

    // set NEW orientation
    po the_po = po(ox,oy,oz,parms->me->get_rel_position());
    parms->me->set_rel_po( the_po );

    SLF_DONE;
  }
};

// script library function:  entity::set_abs_xz_facing(vector3d)
class slf_entity_set_abs_xz_facing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_abs_xz_facing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if ( parms->me != g_world_ptr->get_marky_cam_ptr()
      || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority() )
    {
      parms->me->check_nonstatic();
	  // is there a point to the following line?
      //if ( parms->p.y < 0.95f )
      {
        // compute orientation matrix
        vector3d oz = parms->p - parms->me->get_abs_position();
        oz.y = 0;
        oz.normalize();
        vector3d ox = cross( YVEC, oz );
        ox.normalize();
        po newpo( ox, YVEC, oz, parms->me->get_abs_position() );
        // transform to parent space, if applicable
        if ( parms->me->link_ifc()->get_parent() )
        {
          fast_po_mul(newpo, newpo, parms->me->link_ifc()->get_parent()->get_abs_po().inverse());
//          newpo = newpo * parms->me->link_ifc()->get_parent()->get_abs_po().inverse();
        }
        // set NEW orientation
        parms->me->set_rel_po( newpo );
      }
    }

    SLF_DONE;
  }
};

// script library function:  entity::set_po_facing(vector3d)
class slf_entity_set_po_facing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_po_facing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if ( parms->me != g_world_ptr->get_marky_cam_ptr()
      || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority() )
    {
      parms->me->check_nonstatic();

      vector3d face = parms->p;
      face.normalize();

      po face_po = po_identity_matrix;
      face_po.set_facing(face);
      face_po.set_position(parms->me->get_abs_position());

      if ( parms->me->link_ifc()->get_parent() )
      {
        fast_po_mul(face_po, face_po, parms->me->link_ifc()->get_parent()->get_abs_po().inverse());
//        face_po = face_po * parms->me->link_ifc()->get_parent()->get_abs_po().inverse();
      }

      parms->me->set_rel_po(face_po);
    }

    SLF_DONE;
  }
};


// script library function:  entity::wait_rotate(vector3d,num,num)
class slf_entity_wait_rotate_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_rotate_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d axis;
    vm_num_t rads;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    SLF_PARMS;
//#pragma fixme("Immoral E3 safety check  4-4-01  jdf (best keep it in braces, for more safety - JDB)")
    if( !(parms->me))
    {
      nglPrintf("BIG TIME E3 ERROR!  Jaime's Immorality Caught Us Again!\n");
      SLF_DONE;
    }
    else
    {
      // end immoral E3 safety check

      parms->me->check_nonstatic();
      if(parms->duration == 0.0f)
      {
        if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
        {
          po rot;
          rot.set_rot(parms->axis, parms->rads);
          parms->me->set_frame_delta(rot,1.0f);

          fast_po_mul(rot, rot, parms->me->get_rel_po());
          parms->me->set_rel_po(rot);
  //        parms->me->set_rel_po(rot * parms->me->get_rel_po());
        }

        SLF_DONE;
      }
      else if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
  //      sdata->clock.reset();
        SLF_RECALL;
      }
      else
      {
        // update timer
        time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
        time_value_t time_left = parms->duration - tdelta;
        if (time_left < 0)
        {
          tdelta += time_left;
          time_left = 0;
        }

        float inc_rads = 0.0f;

        if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
        {
          // compute incremental rotation
          inc_rads = parms->rads;
          inc_rads *= tdelta / parms->duration;
          po rot;
          rot.set_rot(parms->axis,inc_rads);
          // apply rotation
          parms->me->set_frame_delta(rot,tdelta); //update_moving_floor_adjustments(rot);

          fast_po_mul(rot, rot, parms->me->get_rel_po());
          parms->me->set_rel_po(rot);
  //        parms->me->set_rel_po(rot*parms->me->get_rel_po());
        }

        // repeat process until original duration elapses
        if (time_left)
        {
          // decrease duration to reflect incremental change
          parms->duration = time_left;
          // decrease rads by proportional amount; ratio rads/duration is preserved
          parms->rads -= inc_rads;
          // come back next frame
          SLF_RECALL;
        }
        else
        {
          // rotation completed; terminate library function
          SLF_DONE;
        }
      }
    }
  }
};



// script library function:  entity::wait_rotate_cosmetic(vector3d,num,num)
class slf_entity_wait_rotate_cosmetic_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_rotate_cosmetic_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d axis;
    vm_num_t rads;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    SLF_PARMS;
    if(parms->duration == 0.0f)
    {
      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        po rot;
        rot.set_rot(parms->axis, parms->rads);

        fast_po_mul(rot, rot, parms->me->get_rel_po());
        parms->me->set_rel_po(rot);
//        parms->me->set_rel_po(rot * parms->me->get_rel_po());
      }

      SLF_DONE;
    }
    else if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      // update timer
      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      float inc_rads = 0.0f;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental rotation
        inc_rads = parms->rads;
        inc_rads *= tdelta / parms->duration;
        po rot;
        rot.set_rot(parms->axis,inc_rads);
        // apply rotation

        fast_po_mul(rot, rot, parms->me->get_rel_po());
        parms->me->set_rel_po(rot);

//        parms->me->set_rel_po(rot*parms->me->get_rel_po());
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->rads -= inc_rads;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};





// script library function:  entity::wait_rotate_WCS(vector3d,vector3d,num,num)
// Rotate about a point expressed in world coordinate space (WCS).
class slf_entity_wait_rotate_WCS_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_rotate_WCS_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d center;
    vector3d axis;
    vm_num_t rads;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      parms->me->check_nonstatic();
      // update timer
      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      float inc_rads = 0.0f;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental rotation
        inc_rads = parms->rads;
        inc_rads *= tdelta / parms->duration;
        // translate entity
        parms->me->set_rel_position(parms->me->get_rel_position()-parms->center);
        // apply rotation
        po rot;
        rot.set_rot(parms->axis,inc_rads);
        parms->me->set_frame_delta(rot,tdelta); //update_moving_floor_adjustments(rot);

        fast_po_mul(rot, parms->me->get_rel_po(), rot );
        parms->me->set_rel_po(rot);
//        parms->me->set_rel_po(parms->me->get_rel_po()*rot);
        // translate entity back
        parms->me->set_rel_position(parms->me->get_rel_position()+parms->center);

#ifdef ECULL
        if(parms->me->has_sound_ifc())
          parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
#endif
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->rads -= inc_rads;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};



// script library function:  entity::wait_rotate_WCS_with_compute_sector(vector3d,vector3d,num,num)
// Rotate about a point expressed in world coordinate space (WCS).
class slf_entity_wait_rotate_WCS_with_compute_sector_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_rotate_WCS_with_compute_sector_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d center;
    vector3d axis;
    vm_num_t rads;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      parms->me->check_nonstatic();
      // update timer
      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      float inc_rads = 0.0f;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental rotation
        inc_rads = parms->rads;
        inc_rads *= tdelta / parms->duration;
        // translate entity
        parms->me->set_rel_position(parms->me->get_rel_position()-parms->center);
        // apply rotation
        po rot;
        rot.set_rot(parms->axis,inc_rads);
        parms->me->set_frame_delta(rot,tdelta); //update_moving_floor_adjustments(rot);
        fast_po_mul(rot, parms->me->get_rel_po(), rot );
        parms->me->set_rel_po(rot);
//        parms->me->set_rel_po(parms->me->get_rel_po()*rot);
        // translate entity back
        parms->me->set_rel_position(parms->me->get_rel_position()+parms->center);
        // recompute sector
        // NOTE: to be optimal, we should only perform compute_sector once per
        // frame for a given entity; currently, there is nothing to enforce this
        parms->me->compute_sector(g_world_ptr->get_the_terrain());
#ifdef GCCULL
        if(parms->me->has_sound_ifc())
          parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
#endif
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->rads -= inc_rads;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};





// script library function:  entity::wait_rotate_WCS_cosmetic(vector3d,vector3d,num,num)
// Rotate about a point expressed in world coordinate space (WCS).
class slf_entity_wait_rotate_WCS_cosmetic_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_rotate_WCS_cosmetic_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d center;
    vector3d axis;
    vm_num_t rads;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      // update timer
      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      float inc_rads = 0.0f;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental rotation
        inc_rads = parms->rads;
        inc_rads *= tdelta / parms->duration;
        // translate entity
        parms->me->set_rel_position(parms->me->get_rel_position()-parms->center);
        // apply rotation
        po rot;
        rot.set_rot(parms->axis,inc_rads);
        fast_po_mul(rot, parms->me->get_rel_po(), rot );
        parms->me->set_rel_po(rot);
//        parms->me->set_rel_po(parms->me->get_rel_po()*rot);
        // translate entity back
        parms->me->set_rel_position(parms->me->get_rel_position()+parms->center);
#ifdef GCCULL
        if(parms->me->has_sound_ifc())
          parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
#endif
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->rads -= inc_rads;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};





// script library function:  entity::wait_translate(vector3d,num)
class slf_entity_wait_translate_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_translate_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d translation;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      parms->me->check_nonstatic();
      // update timer
      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      vector3d inc_trans = ZEROVEC;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental translation
        inc_trans = parms->translation;
        inc_trans *= tdelta / parms->duration;
        po trans = po_identity_matrix;
        trans.set_translate( inc_trans );
        // apply translation
        parms->me->set_frame_delta(trans,tdelta); //update_moving_floor_adjustments(trans);
        fast_po_mul(trans, trans, parms->me->get_rel_po() );
        parms->me->set_rel_po(trans);
        parms->me->set_needs_compute_sector(true);
//        parms->me->set_rel_po(trans*parms->me->get_rel_po());

#ifdef GCCULL
        if(parms->me->has_sound_ifc())
          parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
#endif
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->translation -= inc_trans;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};


// script library function:  entity::wait_translate_with_compute_sector(vector3d,num)
class slf_entity_wait_translate_with_compute_sector_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_translate_with_compute_sector_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d translation;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      parms->me->check_nonstatic();
      // update timer

      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      vector3d inc_trans = ZEROVEC;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental translation
        inc_trans = parms->translation;
        inc_trans *= tdelta / parms->duration;
        po trans = po_identity_matrix;
        trans.set_translate( inc_trans );
        // apply translation
        parms->me->set_frame_delta(trans,tdelta); //update_moving_floor_adjustments(trans);
        fast_po_mul(trans, trans, parms->me->get_rel_po() );
        parms->me->set_rel_po(trans);
//        parms->me->set_rel_po(trans*parms->me->get_rel_po());
        // recompute sector
        // NOTE: to be optimal, we should only perform compute_sector once per
        // frame for a given entity; currently, there is nothing to enforce this
        parms->me->compute_sector(g_world_ptr->get_the_terrain());

//        if(parms->me->has_sound_ifc())
          //parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->translation -= inc_trans;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};




// script library function:  entity::wait_translate_cosmetic(vector3d,num)
class slf_entity_wait_translate_cosmetic_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_translate_cosmetic_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d translation;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      // update timer

      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      vector3d inc_trans = ZEROVEC;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental translation
        inc_trans = parms->translation;
        inc_trans *= tdelta / parms->duration;
        po trans = po_identity_matrix;
        trans.set_translate( inc_trans );
        // apply translation
        parms->me->set_frame_delta(trans,tdelta); //update_moving_floor_adjustments(trans);
        fast_po_mul(trans, trans, parms->me->get_rel_po() );
        parms->me->set_rel_po(trans);
        parms->me->set_needs_compute_sector(true);
//        parms->me->set_rel_po(trans*parms->me->get_rel_po());

        //if(parms->me->has_sound_ifc())
        //  parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->translation -= inc_trans;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};



// script library function:  entity::wait_translate_WCS(vector3d,num)
class slf_entity_wait_translate_WCS_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_translate_WCS_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d translation;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      parms->me->check_nonstatic();
      // update timer

      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      vector3d inc_trans = ZEROVEC;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental translation
        inc_trans = parms->translation;
        inc_trans *= tdelta / parms->duration;
        po trans = po_identity_matrix;
        trans.set_translate( inc_trans );
        // apply translation
        parms->me->set_frame_delta(trans,tdelta); //update_moving_floor_adjustments(trans);
        fast_po_mul(trans, parms->me->get_rel_po(), trans );
        parms->me->set_rel_po(trans);
        parms->me->set_needs_compute_sector(true);
//        parms->me->set_rel_po(parms->me->get_rel_po()*trans);

        //if(parms->me->has_sound_ifc())
        //  parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->translation -= inc_trans;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};


// script library function:  entity::wait_translate_WCS_with_compute_sector(vector3d,num)
class slf_entity_wait_translate_WCS_with_compute_sector_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_translate_WCS_with_compute_sector_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d translation;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      parms->me->check_nonstatic();
      // update timer

      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      vector3d inc_trans = ZEROVEC;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental translation
        inc_trans = parms->translation;
        inc_trans *= tdelta / parms->duration;
        po trans = po_identity_matrix;
        trans.set_translate( inc_trans );
        // apply translation
        parms->me->set_frame_delta(trans,tdelta); //update_moving_floor_adjustments(trans);
        fast_po_mul(trans, parms->me->get_rel_po(), trans );
        parms->me->set_rel_po(trans);
//        parms->me->set_rel_po(parms->me->get_rel_po()*trans);
        // recompute sector
        // NOTE: to be optimal, we should only perform compute_sector once per
        // frame for a given entity; currently, there is nothing to enforce this
        parms->me->compute_sector(g_world_ptr->get_the_terrain());

        //if(parms->me->has_sound_ifc())
        //  parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->translation -= inc_trans;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};



// script library function:  entity::wait_translate_WCS_cosmetic(vector3d,num)
class slf_entity_wait_translate_WCS_cosmetic_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_translate_WCS_cosmetic_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
/*
  struct sdata_t
  {
    game_clock_t clock;
  };
*/
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d translation;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    SLF_SDATA;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      SLF_RECALL;
    }
    else
    {
      SLF_PARMS;
      parms->me->check_nonstatic();
      // update timer

      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      vector3d inc_trans = ZEROVEC;

      if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      {
        // compute incremental translation
        inc_trans = parms->translation;
        inc_trans *= tdelta / parms->duration;
        po trans = po_identity_matrix;
        trans.set_translate( inc_trans );
        // apply translation
        fast_po_mul(trans, parms->me->get_rel_po(), trans );
        parms->me->set_rel_po(trans);
        parms->me->set_needs_compute_sector(true);
//        parms->me->set_rel_po(parms->me->get_rel_po()*trans);

        //if(parms->me->has_sound_ifc())
        //  parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
      }

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        parms->translation -= inc_trans;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};




// script library function:  vector3d entity::get_rel_velocity(entity e)
class slf_entity_get_rel_velocity_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_rel_velocity_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t e;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result;
    parms->me->get_velocity(&result);

    //      result = parms->e->get_abs_po().non_affine_slow_xform(result);
    result = parms->e->get_abs_po().fast_8byte_non_affine_xform(result);

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_rel_velocity_t slf_entity_get_rel_velocity(slc_entity,"get_rel_velocity(entity)");


// script library function:  vector3d entity::in_sector(vector3d origin, vector3d dir, num arc)
class slf_entity_in_sector_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_in_sector_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d origin;
    vector3d dir;
    rational_t arc;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    assert(__fabs(parms->dir.length2()-1.0f)<.01f);
    rational_t dotprod = dot(parms->me->get_abs_position()-parms->origin, parms->dir);

    rational_t result = fast_acos(dotprod>parms->arc);

    SLF_RETURN;
    SLF_DONE;
  }
};

extern stringx script_string_none;

// script library function: str entity::get_sector_name()
class slf_entity_get_sector_name_t: public script_library_class::function {
public:
  // constructor required
  slf_entity_get_sector_name_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    vm_str_t result = parms->me->get_region()? &(parms->me->get_region()->get_data()->get_name()) : &script_string_none;
    SLF_RETURN;
    SLF_DONE;
  }
};





// script library function: str entity::force_current_region()
class slf_entity_force_current_region_t: public script_library_class::function {
public:
  // constructor required
  slf_entity_force_current_region_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      parms->me->force_current_region();

    SLF_DONE;
  }
};




// script library function: str entity::unforce_regions()
class slf_entity_unforce_regions_t: public script_library_class::function {
public:
  // constructor required
  slf_entity_unforce_regions_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
    {
      parms->me->unforce_regions();
      parms->me->compute_sector(g_world_ptr->get_the_terrain());
    }

    SLF_DONE;
  }
};



// script library function:  num entity::operator==(entity)
class slf_entity_equal_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t b;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = (parms->me == parms->b);
    SLF_RETURN;
    SLF_DONE;
  }
};




// script library function:  num entity::operator!=(entity)
class slf_entity_not_equal_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_not_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t b;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = (parms->me != parms->b);
    SLF_RETURN;
    SLF_DONE;
  }
};



// script library function:  entity::set_invulnerable(num)
class slf_entity_set_invulnerable_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_invulnerable_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( parms->torf )
      parms->me->set_invulnerable( true );
    else
      parms->me->set_invulnerable( false );
    SLF_DONE;
  }
};




// script library function:  entity::set_targetting(num)
class slf_entity_set_targetting_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_targetting_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if ( parms->torf )
    {
      parms->me->set_combat_target( true );
      parms->me->set_beamable( true );
    }
    else
    {
      parms->me->set_combat_target( false );
      parms->me->set_beamable( false );
    }

    SLF_DONE;
  }
};




// script library function:  entity::set_visible(num)
class slf_entity_set_visible_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_visible_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    rational_t status;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->check_nonstatic();
    parms->me->set_visible(parms->status);
    SLF_DONE;
  }
};




// script library function:  entity::set_active(num)
class slf_entity_set_active_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_active_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    rational_t status;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    // not legal for scripts to use set_active() on actors or characters
//!    assert( parms->me->get_flavor()!=ENTITY_ACTOR && parms->me->get_flavor()!=ENTITY_CHARACTER );
    parms->me->set_active( parms->status );
    SLF_DONE;
  }
};


// script library function:  entity::set_physical(num)
class slf_entity_set_physical_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_physical_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    rational_t status;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->has_physical_ifc())
      parms->me->physical_ifc()->enable( parms->status );
    SLF_DONE;
  }
};



// script library function:  entity::raise_generic_signal(num)
class slf_entity_raise_generic_signal_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_raise_generic_signal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    rational_t signal_num;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    switch( (int)(parms->signal_num) )
    {
    case 1:
      parms->me->raise_signal( entity::GENERIC_SIGNAL_1 );
      break;
    case 2:
      parms->me->raise_signal( entity::GENERIC_SIGNAL_2 );
      break;
    case 3:
      parms->me->raise_signal( entity::GENERIC_SIGNAL_3 );
      break;
    case 4:
      parms->me->raise_signal( entity::GENERIC_SIGNAL_4 );
      break;
    default:
      stack.get_thread()->slf_error( "$" + parms->me->get_id().get_val() + ".raise_generic_signal(): " + stringx(parms->signal_num) + "not supported.  Use 1 to 4." );
    }
    SLF_DONE;
  }
};



// <<<< start & stop no longer functional
// script library function:  entity::start()
class slf_entity_start_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_start_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
    SLF_DONE;
  }
};



// script library function:  entity::stop()
class slf_entity_stop_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_stop_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
    SLF_DONE;
  }
};



// script library function:  entity::reset()
class slf_entity_reset_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_reset_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->rebirth();
    SLF_DONE;
  }
};


#if 0 // BIGCULL
// script library function:  entity::apply_damage(num)
class slf_entity_apply_damage_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_apply_damage_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t d;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->has_damage_ifc())
      parms->me->damage_ifc()->apply_damage(NULL, parms->d, DAMAGE_DIRECT, ZEROVEC, -parms->me->get_abs_po().get_facing());
    SLF_DONE;
  }
};



// script library function:  entity::apply_directed_damage(num, vector3d)
class slf_entity_apply_directed_damage_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_apply_directed_damage_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t d;
    vector3d dir;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->has_damage_ifc())
      parms->me->damage_ifc()->apply_damage(NULL, parms->d, DAMAGE_DIRECT_DIRECTIONAL, ZEROVEC, parms->dir);
    SLF_DONE;
  }
};

// script library function:  entity::apply_explosive_damage(num, vector3d)
class slf_entity_apply_explosive_damage_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_apply_explosive_damage_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t d;
    vector3d pos;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me->has_damage_ifc())
    {
      vector3d dir = parms->me->get_abs_position() - parms->pos;
      dir.normalize();
      parms->me->damage_ifc()->apply_damage(NULL, parms->d, DAMAGE_EXPLOSIVE, parms->pos, dir);
    }

    SLF_DONE;
  }
};

#endif // BIGCULL

// script library function:  entity::motion_blur_on()
class slf_entity_motion_blur_on_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_motion_blur_on_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->activate_motion_blur( 0, 196, 5, 0.6f );
    SLF_DONE;
  }
};


// script library function:  entity::motion_blur_off()
class slf_entity_motion_blur_off_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_motion_blur_off_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->deactivate_motion_blur();
    SLF_DONE;
  }
};


// script library function:  entity::motion_trail_on()
class slf_entity_motion_trail_on_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_motion_trail_on_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->activate_motion_trail( 4, color32(255,255,255,0), 0, 64, vector3d(0,1,0));
    SLF_DONE;
  }
};


// script library function:  entity::motion_trail_off()
class slf_entity_motion_trail_off_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_motion_trail_off_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->deactivate_motion_trail();
    SLF_DONE;
  }
};



// script library function:  bool entity::is_picked_up()
class slf_entity_is_picked_up_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_is_picked_up_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->is_picked_up();
    SLF_RETURN;
    SLF_DONE;
  }
};




// script library function:  entity::snap_to(entity)
class slf_entity_snap_to_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_snap_to_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t target;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->check_nonstatic();

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
    {
/*!      if (parms->me->get_flavor()==ENTITY_CHARACTER)
        ((character *) parms->me)->adjust_position(parms->target->get_abs_position());
!*/
      parms->me->set_rel_po(parms->target->get_abs_po());
      parms->me->compute_sector(g_world_ptr->get_the_terrain());

      //if(parms->me->has_sound_ifc())
      //  parms->me->sound_ifc()->get_emitter()->set_position( parms->me->get_abs_position() );
    }

    SLF_DONE;
  }
};




////////////////////////////////////////////////////////////////////////////////
//  camera extensions
////////////////////////////////////////////////////////////////////////////////

// script library function:  entity::camera_set_target(vector3d)
class slf_entity_camera_set_target_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_camera_set_target_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d new_pos;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      parms->me->camera_set_target(parms->new_pos);

    SLF_DONE;
  }
};


// script library function:  entity::camera_set_roll(num)
class slf_entity_camera_set_roll_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_camera_set_roll_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t new_roll;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      parms->me->camera_set_roll(parms->new_roll);

    SLF_DONE;
  }
};

// global script library function: wait_entity_camera_set_roll(num,num)
class slf_entity_wait_camera_set_roll_t : public script_library_class::function
{
  public:
    // constructor required
    slf_entity_wait_camera_set_roll_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
//      game_clock_t clock;
      vm_num_t     start_roll;
      vm_num_t time_elapsed;
    };

    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_entity_t me;
      vm_num_t dest_roll;
      vm_num_t duration;
    };

    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;

      assert(parms->me->is_a_marky_camera());

      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        if(parms->duration==0)
        {
          parms->me->camera_set_roll(parms->dest_roll);
          SLF_DONE;
        }
        else
        {
//          sdata->clock.reset();
          marky_camera *mk = (marky_camera *)(parms->me);
          sdata->start_roll = mk->get_roll();
          sdata->time_elapsed = 0.0f;
          SLF_RECALL;
        }
      }
      else
      {
        sdata->time_elapsed += CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
        float percentage_elapsed = sdata->time_elapsed / parms->duration;
        percentage_elapsed = min( percentage_elapsed, 1.0f );

        parms->me->camera_set_roll(((parms->dest_roll - sdata->start_roll) * percentage_elapsed) + sdata->start_roll);

        if( sdata->time_elapsed >= parms->duration )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_entity_wait_camera_set_roll_t slf_entity_wait_camera_set_roll("wait_camera_set_roll(num,num)");


// script library function:  entity::camera_set_collide_with_world(num)
class slf_entity_camera_set_collide_with_world_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_camera_set_collide_with_world_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t yesno;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      parms->me->camera_set_collide_with_world( (bool)parms->yesno );

    SLF_DONE;
  }
};


// script library function:  bool entity::camera_slide_to(vector3d, vector3d, num, num)
class slf_entity_camera_slide_to_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_camera_slide_to_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d new_pos;
    vector3d new_target;
    vm_num_t new_roll;
    vm_num_t speed;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    vm_num_t result = 0;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      result = parms->me->camera_slide_to(parms->new_pos, parms->new_target, parms->new_roll, parms->speed);

    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  bool entity::camera_slide_to_orbit(vector3d, num, num, num, num)
class slf_entity_camera_slide_to_orbit_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_camera_slide_to_orbit_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d center;
    vm_num_t range;
    vm_num_t theta;
    vm_num_t psi;
    vm_num_t speed;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    vm_num_t result = 0;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      result = parms->me->camera_slide_to_orbit(parms->center, parms->range, parms->theta, parms->psi, parms->speed);

    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  entity::camera_orbit(vector3d, num, num, num)
class slf_entity_camera_orbit_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_camera_orbit_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d center;
    vm_num_t range;
    vm_num_t theta;
    vm_num_t psi;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me != g_world_ptr->get_marky_cam_ptr() || stack.get_thread()->get_camera_priority() == g_world_ptr->get_marky_cam_ptr()->get_priority())
      parms->me->camera_orbit(parms->center, parms->range, parms->theta, parms->psi);

    SLF_DONE;
  }
};




////////////////////////////////////////////////////////////////////////////////
//  light extensions
////////////////////////////////////////////////////////////////////////////////

entity* entity_dammit;

class slf_entity_wait_change_color_t;
slf_entity_wait_change_color_t * g_this2;
vector3d g_color,g_old,g_new;
rational_t g_delta;

// script library function:  entity::wait_change_color(vector3d,num)
class slf_entity_wait_change_color_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_change_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
//    game_clock_t clock;
    vector3d old_color;
    vector3d old_additive_color;
    vm_num_t time_elapsed;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d new_color;
    vector3d new_additive_color;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      sdata->time_elapsed = 0.0f;
      entity_dammit = parms->me;
      color oc = entity_dammit->get_color();
      sdata->old_color = vector3d( oc.r, oc.g, oc.b );
      oc = entity_dammit->get_additive_color();
      sdata->old_additive_color = vector3d( oc.r, oc.g, oc.b );
      SLF_RECALL;
    }
    else
    {
      // update timer
      g_this2 = this;


      sdata->time_elapsed += CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t duration = parms->duration;
      if (sdata->time_elapsed > duration)
      {
        sdata->time_elapsed = duration;
      }
      // compute incremental translation

      vector3d cur_color = parms->new_color-sdata->old_color;
      cur_color *= sdata->time_elapsed / parms->duration;
      cur_color += sdata->old_color;
      g_color = cur_color;
      g_delta = sdata->time_elapsed;
      g_old = sdata->old_color;
      g_new = parms->new_color;
      parms->me->set_color( color( cur_color.x, cur_color.y, cur_color.z, 0 ) );

      cur_color = parms->new_additive_color - sdata->old_additive_color;
      cur_color *= sdata->time_elapsed / parms->duration;
      cur_color += sdata->old_additive_color;
      g_color = cur_color;
      parms->me->set_additive_color( color( cur_color.x, cur_color.y, cur_color.z, 0 ) );

      // repeat process until duration elapses
      if ( sdata->time_elapsed < duration)
      {
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};


// script library function:  entity::wait_change_range(vector3d,num)
class slf_entity_wait_change_range_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_change_range_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
//    game_clock_t clock;
    vm_num_t old_near_range;
    vm_num_t old_cutoff_range;
    vm_num_t time_elapsed;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vm_num_t new_near_range;
    vm_num_t new_cutoff_range;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      sdata->time_elapsed = 0.0f;
      sdata->old_near_range = parms->me->get_near_range();
      sdata->old_cutoff_range = parms->me->get_cutoff_range();
      SLF_RECALL;
    }
    else
    {
      // update timer

      sdata->time_elapsed += CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t duration = parms->duration;
      if (sdata->time_elapsed > duration)
      {
        sdata->time_elapsed = duration;
      }
      // compute incremental translation

      vm_num_t cur_range = parms->new_near_range - sdata->old_near_range;
      cur_range *= sdata->time_elapsed / parms->duration;
      cur_range += sdata->old_near_range;
      parms->me->set_near_range( cur_range );

      cur_range = parms->new_cutoff_range - sdata->old_cutoff_range;
      cur_range *= sdata->time_elapsed / parms->duration;
      cur_range += sdata->old_cutoff_range;
      //        parms->me->set_cutoff_range( cur_range );

      // repeat process until duration elapses
      if ( sdata->time_elapsed < duration)
      {
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};


// script library function:  entity::play_ent_sound(str)
#ifdef GCCULL
class slf_entity_play_ent_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_play_ent_sound_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
//    assert(0 && "This function (play_ent_sound) is out of style.  Fix your script");
    parms->me->get_emitter()->play_sound(*parms->n);
    SLF_DONE;
  }
};

// script library function:  num entity::play_3d_ent_sound(str,num,num)
class slf_entity_play_3d_ent_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_play_3d_ent_sound_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t name;
    vm_num_t volume;
    vm_num_t pitch;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if (!parms->me->has_sound_ifc())
      parms->me->create_sound_ifc();
    rational_t result = parms->me->sound_ifc()->play_3d_sound( *parms->name,
      parms->volume, parms->pitch );
    SLF_RETURN;
    SLF_DONE;
  }
};

// script library function:  entity::kill_ent_sound(num)
class slf_entity_kill_ent_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_kill_ent_sound_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t snd_id;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if (parms->me->has_sound_ifc())
      parms->me->sound_ifc()->kill_sound( parms->snd_id );
    SLF_DONE;
  }
};

// script library function:  entity::play_3d_ent_sound_group(str,num,num)
class slf_entity_play_3d_ent_sound_group_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_play_3d_ent_sound_group_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t name;
    vm_num_t volume;
    vm_num_t pitch;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    rational_t result = 0;
    if (parms->me->has_sound_ifc())
    {
      pstring pstr_name(*parms->name);
      sg_entry *my_entry = parms->me->sound_ifc()->play_3d_sound_grp( pstr_name,
        parms->volume, parms->pitch );
      if (my_entry != NULL)
        result = my_entry->last_id_played;
    }
    SLF_RETURN;
    SLF_DONE;
  }
};

// script library function:  entity::play_3d_ent_sound_group_looped(str,num,num)
class slf_entity_play_3d_ent_sound_group_looped_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_play_3d_ent_sound_group_looped_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t name;
    vm_num_t volume;
    vm_num_t pitch;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    rational_t result = 0;
    if (parms->me->has_sound_ifc())
    {
      pstring pstr_name(*parms->name);
      sg_entry *my_entry = parms->me->sound_ifc()->play_looping_3d_sound_grp( pstr_name,
        parms->volume, parms->pitch );
      if (my_entry != NULL)
        result = my_entry->last_id_played;
    }
    SLF_RETURN;
    SLF_DONE;
  }
};

// script library function:  sound_instance entity::create_ent_sound(str)
class slf_entity_create_ent_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_create_ent_sound_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
//    assert(0 && "This function (create_ent_sound) is out of style.  Fix your script");
    vm_sound_instance_t result = parms->me->get_emitter()->create_sound(*parms->n);
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  sound_stream entity::create_ent_stream(str)
class slf_entity_create_ent_stream_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_create_ent_stream_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
//    assert(0 && "This function (create_ent_stream) is out of style.  Fix your script");
    vm_sound_stream_t result = parms->me->get_emitter()->create_stream(*parms->n);
    g_stream_list.push_back(result);
    SLF_RETURN;
    SLF_DONE;
  }
};

#endif

// script library function:  entity::apply_blast( vector3d blast_center, num blast_radius, num blast_force )
class slf_entity_apply_blast_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_apply_blast_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d blast_center;
    vm_num_t blast_radius;
    vm_num_t blast_force;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->check_nonstatic();

    vector3d impulse = parms->me->get_abs_position() - parms->blast_center;

    rational_t dist = impulse.length();

    if(dist < parms->blast_radius)
    {
      // naturally give a little vertical oomph. (may need to be adjusted)
      impulse.y += 1.0f;
      impulse.normalize();

      impulse *= (1.0f - (dist / parms->blast_radius)) * parms->blast_force;

      // apply the impulse
      assert(0);
      //parms->me->apply_force_increment(impulse, entity::INSTANT);

      // apply a little extra shock if it is a character
/*!      if(parms->me->get_flavor() == ENTITY_CHARACTER)
      {
        if(((character *)parms->me)->has_limb_tree() && ((character *)parms->me)->limb_valid(WAIST))
        {
          assert(((character *)parms->me)->limb_ptr(WAIST));
          ((character *)parms->me)->limb_ptr(WAIST)->shock(impulse, ZEROVEC, g_world_ptr->get_cur_time_inc());
        }
      }
!*/
    }
    SLF_DONE;
  }
};


// script library function:  entity::portal_open()
class slf_entity_portal_open_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_portal_open_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( parms->me->is_door() )
      parms->me->set_door_closed( false );
    else
    {
      stack.get_thread()->slf_error( "$" + parms->me->get_id().get_val() + ".portal_open(): this entity is not a DOOR" );
    }
    SLF_DONE;
  }
};


// script library function:  entity::portal_close()
class slf_entity_portal_close_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_portal_close_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( parms->me->is_door() )
      parms->me->set_door_closed( true );
    else
    {
      stack.get_thread()->slf_error( "$" + parms->me->get_id().get_val() + ".portal_close(): this entity is not a DOOR" );
    }
    SLF_DONE;
  }
};


///////////////////////////////////////////////////////////////////////////////
//  entity script library functions for changing access
///////////////////////////////////////////////////////////////////////////////

// script library function:  entity entity::get_member( str )
class slf_entity_get_member_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_member_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t nodename;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result;
    if ( parms->me->is_a_conglomerate() )
    {
      conglomerate* c = static_cast<conglomerate*>( parms->me );
      result = c->get_member( *parms->nodename );
      if ( result == NULL )
        stack.get_thread()->slf_error( "conglomerate " + c->get_name() + " does not have a member " + *parms->nodename );
    }
    else
    {
      stack.get_thread()->slf_error( "get_member('" + *parms->nodename + "'): bad entity flavor; this function only works on conglomerates" );
    }
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  entity entity::get_nth_member( int )
class slf_entity_get_nth_member_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_nth_member_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t index;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result;
    if ( parms->me->is_a_conglomerate() )
    {
      conglomerate* c = static_cast<conglomerate*>( parms->me );
      const pentity_vector& members = c->get_members( );

      if( parms->index < members.size( ) )
      {
        int i = (int) parms->index;
        result = members[i];
      }
      else
      {
        stack.get_thread()->slf_error( "get_nth_member(): index out of range" );
      }

    }
    else
    {
      stack.get_thread()->slf_error( "get_nth_member(): bad entity flavor; this function only works on conglomerates" );
    }
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  num entity::get_nth_member( )
class slf_entity_get_num_members_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_num_members_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result;
    if ( parms->me->is_a_conglomerate() )
    {
      conglomerate* c = static_cast<conglomerate*>( parms->me );
      const pentity_vector& members = c->get_members( );
      result = members.size( );
    }
    else
    {
      stack.get_thread()->slf_error( "get_num_members(): bad entity flavor; this function only works on conglomerates" );
    }
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  entity entity::is_character()
class slf_entity_is_character_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_is_character_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
    vm_num_t result = 0;
//!    result = ( parms->me->get_flavor() == ENTITY_CHARACTER );
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  entity entity::entity_to_character( entity e )
class slf_entity_to_character_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_to_character_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t e;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
STUBBED(slf_entity_to_character_t, "script_lib slf_entity_to_character_t");
/*!    vm_character_t result;
    if ( parms->e->get_flavor() == ENTITY_CHARACTER )
      result = (static_cast<character*>(parms->e));
    else
    {
      stack.get_thread()->slf_error( "entity_to_character(): bad entity flavor; this function only works on characters" );
    }
!*/ vm_num_t result; //!
    SLF_RETURN;
    SLF_DONE;
  }
};


///////////////////////////////////////////////////////////////////////////////
// entity script library functions for animation
///////////////////////////////////////////////////////////////////////////////

// script library function:  void entity::load_anim(str)
class slf_entity_load_anim_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_load_anim_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t filename;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    parms->me->load_anim( *parms->filename );
    SLF_DONE;
  }
};


// script library function:  anim entity::play_anim(str)
class slf_entity_play_anim1_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_play_anim1_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t filename;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    entity_anim_tree* result = parms->me->play_anim( *parms->filename, 0.0f, ANIM_AUTOKILL );
    // CTT 04/04/00: TEMPORARY:
    // because the handle provided for anims is currently a pointer, it is not
    // safe to return a handle to an autokill animation; this will change when
    // we create an integer handle type for anims
    result = NULL;
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  anim entity::play_anim(str,num,num)
class slf_entity_play_anim3_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_play_anim3_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t filename;
    vm_num_t start_time;
    vm_num_t anim_flags;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    unsigned short anim_flags = (unsigned short)parms->anim_flags;
    // set anim to AUTOKILL if it is not a looping anim
    if ( !(anim_flags & ANIM_LOOPING) )
      anim_flags |= ANIM_AUTOKILL;
    else if ( anim_flags & ANIM_AUTOKILL )
    {
      stack.get_thread()->slf_error( "play_anim3(): cannot AUTOKILL a LOOPING anim" );
    }
    entity_anim_tree* result = parms->me->play_anim( *parms->filename, parms->start_time, (unsigned short)anim_flags );
    if ( anim_flags & ANIM_AUTOKILL )
    {
//#pragma fixme("CTT 04/04/00: TEMPORARY:")
      // because the handle provided for anims is currently a pointer, it is not
      // safe to return a handle to an autokill animation; this will change when
      // we create an integer handle type for anims
      result = NULL;
    }
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  num entity::wait_play_anim(str name)
class slf_entity_wait_play_anim1_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_play_anim1_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t filename;
  };
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    entity_anim_tree* my_anim;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_SDATA;
    SLF_PARMS;
    vm_num_t result = 0;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack
      sdata->my_anim = parms->me->play_anim( *parms->filename, 0.0f, (unsigned short)0 );
      if ( sdata->my_anim )
        SLF_RECALL;
      else
      {
        // anim didn't take
        SLF_RETURN;
        SLF_DONE;
      }
    }
    else
    {
      if ( sdata->my_anim->is_finished() )
      {
        // return amount by which end of animation was overshot;
        // useful if you want to chain another animation onto this one
        result = -sdata->my_anim->get_time_remaining();
        g_world_ptr->kill_anim( sdata->my_anim );
        SLF_RETURN;
        SLF_DONE;
      }
      else
      {
        // anim is still running
        SLF_RECALL;
      }
    }
  }
};


// script library function:  num entity::wait_play_anim(str name,num start_time,num anim_flags)
class slf_entity_wait_play_anim3_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_play_anim3_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t filename;
    vm_num_t start_time;
    vm_num_t anim_flags;
  };
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    entity_anim_tree* my_anim;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
    {
    SLF_SDATA;
    SLF_PARMS;
    vm_num_t result = 0;
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack
      unsigned short anim_flags = (unsigned short)parms->anim_flags;
      anim_flags &= ~ANIM_AUTOKILL;  // kill is handled locally
      if ( anim_flags & ANIM_LOOPING )
      {
        stack.get_thread()->slf_error( "wait_play_anim3(): use of ANIM_LOOPING in this function will create an infinite loop" );
      }
      sdata->my_anim = parms->me->play_anim( *parms->filename, parms->start_time, (unsigned short)anim_flags );
      if ( sdata->my_anim )
        SLF_RECALL;
      else
      {
        // anim didn't take
        SLF_RETURN;
        SLF_DONE;
      }
    }
    else
    {
      if ( sdata->my_anim->is_finished() )
      {
        // return amount by which end of animation was overshot;
        // useful if you want to chain another animation onto this one
        result = -sdata->my_anim->get_time_remaining();
        g_world_ptr->kill_anim( sdata->my_anim );
        SLF_RETURN;
        SLF_DONE;
      }
      else
      {
        // anim is still running
        SLF_RECALL;
      }
    }
  }
};



///////////////////////////////////////////////////////////////////////////////
// special effects
///////////////////////////////////////////////////////////////////////////////

// global script library function:  load_effect_visual(str)
class slf_load_effect_visual_t : public script_library_class::function
{
public:
  // constructor required
  slf_load_effect_visual_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
//    fx_manager::inst()->load_effect_visual(*parms->n);
    assert(0);
    SLF_DONE;
  }
};


////////////////////////////////////////////////////////////////////////////////
// global script library function:  entity play_effect( str effectname, vector3d where, num flavor )
class slf_play_effect_t : public script_library_class::function
{
public:
  // constructor required
  slf_play_effect_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t name;
    vector3d where;
    vm_num_t flavor;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    po my_po = po_identity_matrix;
    my_po.set_position( parms->where );
    vm_entity_t result = NULL;//fx_manager::inst()->play_effect(*parms->name,my_po,parms->flavor);
    assert(0);
    SLF_RETURN;
    SLF_DONE;
  }
};



////////////////////////////////////////////////////////////////////////////////
// global script library function:  entity play_effect( str effectname, vector3d where, num flavor )
class slf_create_light_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_light_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t name;
    vector3d where;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    entity_id temp_id;
    vm_entity_t result = app::inst()->get_game()->get_world()->add_light_source( *parms->name,
                                                                    true, true,
                                                                    temp_id );
    result->set_rel_position( parms->where );
    SLF_RETURN;
    SLF_DONE;
  }
};



////////////////////////////////////////////////////////////////////////////////
// global script library function:  entity play_effect( str effectname, vector3d where, num flavor )
class slf_kill_light_t : public script_library_class::function
{
public:
  // constructor required
  slf_kill_light_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t lite;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if( parms->lite->get_flavor() != ENTITY_LIGHT_SOURCE )
      stack.get_thread()->slf_error( "kill_light(): $" + parms->lite->get_id().get_val() + " is not a light source" );
    light_source* ls = static_cast<light_source*>( parms->lite );
    app::inst()->get_game()->get_world()->remove_light_source( ls );
    SLF_DONE;
  }
};



// global script library function: entity create_entity(str name)
class slf_create_entity_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_entity_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
		vm_str_t name;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result = g_entity_maker->acquire_entity( *parms->name, ACTIVE_FLAG|NONSTATIC_FLAG );
    result->set_created_entity_default_active_status();
    SLF_RETURN;
    SLF_DONE;
  }
};


// global script library function: entity create_entity_in_hero_region(str name)
class slf_create_entity_in_hero_region_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_entity_in_hero_region_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
		vm_str_t name;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result = g_entity_maker->acquire_entity( *parms->name, ACTIVE_FLAG|NONSTATIC_FLAG );
    result->set_created_entity_default_active_status();
    result->force_regions( g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player()) );
    SLF_RETURN;
    SLF_DONE;
  }
};



// PEH TEST
entity *efg;

// global script library function: entity create_preloaded_static_entity(str name)
class slf_create_preloaded_static_entity_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_preloaded_static_entity_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
	  vm_str_t name;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result = g_entity_maker->acquire_entity( *parms->name, (unsigned int)0 );
    if ( result->get_entity_pool()->size() > 1 )
    {
      // only the first instance needs to remain locked
      g_entity_maker->release_entity( result );
    }
    else
      g_world_ptr->remove_entity_from_world_processing( result );
    SLF_RETURN;
    SLF_DONE;
  }
};


// global script library function: entity cache_entity( str name, num quantity )
class slf_cache_entity_t : public script_library_class::function
{
public:
  // constructor required
  slf_cache_entity_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
		vm_str_t name;
    vm_num_t quantity;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    // acquire N copies of the given entity and then release them all back to the cache
    const int max_cache = 5;
    entity* cached_ents[max_cache];
    int i;
    for ( i=0; i<parms->quantity && i<max_cache; ++i )
      cached_ents[i] = g_entity_maker->acquire_entity( *parms->name, 0 );
    for ( i=0; i<parms->quantity && i<max_cache; ++i )
      g_entity_maker->release_entity( cached_ents[i] );
    SLF_DONE;
  }
};


// global script library function: destroy_entity(entity e)
class slf_destroy_entity_t : public script_library_class::function
{
public:
  // constructor required
  slf_destroy_entity_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t e;
  };
  // library function execution
  virtual bool operator()( vm_stack &stack, entry_t entry )
  {
    SLF_PARMS;
    #ifdef DEBUG
    error_context::inst()->push_context( stringx("(script) destroy_entity( id=")+parms->e->get_id().get_val()+", anim_id="+anim_id_manager::inst()->get_label(parms->e->get_anim_id())+" )" );
    #endif
    g_entity_maker->release_entity( parms->e );
    #ifdef DEBUG
    error_context::inst()->pop_context();
    #endif
    SLF_DONE;
  }
};

// global script library function: entity create_time_limited_entity(str name,num duration)
class slf_create_time_limited_entity_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_time_limited_entity_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t name;
    vm_num_t duration;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result = g_entity_maker->acquire_entity( *parms->name, (unsigned int)0 );
    result->set_created_entity_default_active_status();
    g_world_ptr->make_time_limited( result, parms->duration );
    SLF_RETURN;
    SLF_DONE;
  }
};



// script library function:  entity::add_item(entity e);
class slf_entity_add_item_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_add_item_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t it;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    assert(parms->it->get_flavor() == ENTITY_ITEM);

    if ( parms->me == g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player()) )
    {
      // add_item to hero means actually giving it to him
//!      ((item *)parms->it)->give_to_character( (character*)parms->me );
    }
    else
    {
      // add_item to anyone else means just treating them as a container
      parms->me->entity::add_item( (item *)parms->it );
    }
    SLF_DONE;
  }
};


// script library function:  num entity::get_num_items();
class slf_entity_get_num_items_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_num_items_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    rational_t result = (rational_t)parms->me->get_num_items();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_num_items_t slf_entity_get_num_items(slc_entity,"get_num_items()");

// script library function:  item entity::get_item(num n);
class slf_entity_get_item_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_item_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    rational_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_item_t result = ((int)parms->n<parms->me->get_num_items()) ? parms->me->get_item((int)parms->n) : NULL;
    if ( result && result->get_count()==0 )
      result = NULL;
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  item entity::get_item_by_name(str name);
class slf_entity_get_item_by_name_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_item_by_name_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_item_t result = parms->me->find_item_by_name(*parms->name);
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  num entity::get_item_quantity(num n);
class slf_entity_get_item_quantity_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_item_quantity_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    rational_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = 0;
    vm_item_t it = ((int)parms->n<parms->me->get_num_items()) ? parms->me->get_item((int)parms->n) : NULL;
    if ( it )
      result = it->get_count();
    SLF_RETURN;
    SLF_DONE;
  }
};


// script library function:  entity::disgorge_items();
class slf_entity_disgorge_items_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_disgorge_items_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->disgorge_items();
    SLF_DONE;
  }
};


// script library function:  entity::set_control_active(num v);
class slf_entity_set_control_active_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_set_control_active_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_t me;
      rational_t a;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;

/*
      if(parms->me->get_brain() != NULL)
      {
        // this is safer, as it avoids problems with animations and movement of brains
        if(parms->a)
          parms->me->get_brain()->set_active_true();
        else
          parms->me->get_brain()->set_active_false();
      }
      else
 */
      parms->me->set_control_active( parms->a );

      SLF_DONE;
      }
  };


// script library function:  entity::suspend();
class slf_entity_suspend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_suspend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->suspend();
      SLF_DONE;
      }
  };



// script library function:  entity::unsuspend();
class slf_entity_unsuspend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_unsuspend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->unsuspend();
      SLF_DONE;
      }
  };




///////////////////////////////////////////////////////////////////////////////
// Functions that suspend a thread waiting for a condition to be met
///////////////////////////////////////////////////////////////////////////////

// script library function:  entity::wait_for_pickup()
class slf_entity_wait_for_pickup_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_for_pickup_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( parms->me->is_picked_up() )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_prox( entity b, num r )
class slf_entity_wait_prox1_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_prox1_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t him;
    vm_num_t r;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( (parms->me->get_abs_position()-parms->him->get_abs_position()).length2() < parms->r*parms->r )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_prox( vector3d p, num r )
class slf_entity_wait_prox2_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_prox2_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
    vm_num_t r;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( (parms->me->get_abs_position()-parms->p).length2() < parms->r*parms->r )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_prox( vector3d p1, num r1, vector3d p2, num r2 )
class slf_entity_wait_prox3_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_prox3_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p1;
    vm_num_t r1;
    vector3d p2;
    vm_num_t r2;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    const vector3d& mypos = parms->me->get_abs_position();
    if ( (mypos-parms->p1).length2() < parms->r1*parms->r1 ||
         (mypos-parms->p2).length2() < parms->r2*parms->r2 )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_prox_minY( vector3d p, num r, num minY )
class slf_entity_wait_prox_minY_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_prox_minY_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
    vm_num_t r;
    vm_num_t minY;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    const vector3d& mypos = parms->me->get_abs_position();
    if ( (mypos-parms->p).length2() < parms->r*parms->r &&
         mypos.y > parms->minY )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_prox_maxY( vector3d p, num r, num maxY )
class slf_entity_wait_prox_maxY_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_prox_maxY_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
    vm_num_t r;
    vm_num_t maxY;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    const vector3d& mypos = parms->me->get_abs_position();
    if ( (mypos-parms->p).length2() < parms->r*parms->r &&
         mypos.y < parms->maxY )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_for_sector( str sector )
class slf_entity_wait_for_sector_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_for_sector_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t region_name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if (!parms->me->get_region())
      {
      stringx outbuf;
      outbuf = parms->me->get_id().get_val() +" is out of the world.";
      error(outbuf.c_str());
      }
    if ( parms->me->get_region()->get_data()->get_name() == *parms->region_name )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_for_not_sector( str sector )
class slf_entity_wait_for_not_sector_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_for_not_sector_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t region_name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( parms->me->get_region()->get_data()->get_name() != *parms->region_name )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};


// script library function:  entity::wait_prox_sector( vector3d p, num r, str sector )
class slf_entity_wait_prox_sector_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_prox_sector_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d p;
    vm_num_t r;
    vm_str_t region_name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if ( (parms->me->get_abs_position()-parms->p).length2() < parms->r*parms->r &&
         parms->me->get_region()->get_data()->get_name() == *parms->region_name )
      SLF_DONE;
    else
      SLF_RECALL;
  }
};



////////////////////////////////////////////////////////////////////////////////
class slf_remove_item_from_world_t : public script_library_class::function
{
public:
  // constructor required
  slf_remove_item_from_world_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t sid;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    app::inst()->get_game()->get_world()->remove_item( parms->sid );
    SLF_DONE;
  }
};


////////////////////////////////////////////////////////////////////////////////
class slf_remove_item_entity_from_world_t : public script_library_class::function
{
public:
  // constructor required
  slf_remove_item_entity_from_world_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t sid;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->sid->get_flavor() == ENTITY_ITEM)
      app::inst()->get_game()->get_world()->remove_item( (item *)parms->sid );
    SLF_DONE;
  }
};



///////////////////////////////////////////////////////////////////////////////
// entity script library functions for ifl locking when rendering entities
///////////////////////////////////////////////////////////////////////////////

// script library function:  void entity::lfl_lock(num)
class slf_entity_ifl_lock_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_ifl_lock_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t frame_locked;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    parms->me->ifl_lock(parms->frame_locked);
    SLF_DONE;
  }
};



// script library function:  void entity::lfl_ause()
class slf_entity_ifl_pause_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_ifl_pause_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    parms->me->ifl_pause();
    SLF_DONE;
  }
};





// script library function:  void entity::lfl_play()
class slf_entity_ifl_play_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_ifl_play_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    parms->me->ifl_play();
    SLF_DONE;
  }
};


// script library function:  entity entity::action_activated_by()
class slf_action_activated_by_t : public script_library_class::function
{
public:
  // constructor required
  slf_action_activated_by_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
    vm_entity_t result = NULL;//!parms->me->action_activated_by();
    SLF_RETURN;
    SLF_DONE;
  }
};




extern void register_entity2_lib(slc_entity_t* slc_entity);

void register_entity_lib()
{
	#ifdef GCCULL
  // pointer to single instance of library class
  slc_entity_t* slc_entity = NEW slc_entity_t("entity",4,"signaller");


  NEW slf_entity_get_position_t(slc_entity,"get_abs_position()");
  NEW slf_entity_get_detonate_position_t(slc_entity,"get_detonate_position()");
  NEW slf_entity_get_facing_t(slc_entity,"get_facing()");
  NEW slf_entity_get_rel_position_t(slc_entity,"get_rel_position()");
  NEW slf_entity_rel_angle_t(slc_entity,"rel_angle(vector3d)");
  NEW slf_entity_set_parent_t(slc_entity,"set_parent(entity)");
  NEW slf_entity_set_parent_rel_t(slc_entity,"set_parent_rel(entity)");
  NEW slf_entity_set_position_t(slc_entity,"set_rel_position(vector3d)");
  NEW slf_entity_set_velocity_t(slc_entity,"set_velocity(vector3d)");
  NEW slf_entity_look_at_t(slc_entity,"look_at(vector3d)");
  NEW slf_entity_set_facing_t(slc_entity,"set_facing(vector3d,vector3d)");
  NEW slf_entity_set_xz_facing_t(slc_entity,"set_xz_facing(vector3d)");
  NEW slf_entity_set_abs_xz_facing_t(slc_entity,"set_abs_xz_facing(vector3d)");
  NEW slf_entity_set_po_facing_t(slc_entity,"set_po_facing(vector3d)");
  NEW slf_entity_wait_rotate_t(slc_entity,"wait_rotate(vector3d,num,num)");
  NEW slf_entity_wait_rotate_cosmetic_t(slc_entity,"wait_rotate_cosmetic(vector3d,num,num)");
  NEW slf_entity_wait_rotate_WCS_t(slc_entity,"wait_rotate_WCS(vector3d,vector3d,num,num)");
  NEW slf_entity_wait_rotate_WCS_with_compute_sector_t(slc_entity,"wait_rotate_WCS_with_compute_sector(vector3d,vector3d,num,num)");
  NEW slf_entity_wait_rotate_WCS_cosmetic_t(slc_entity,"wait_rotate_WCS_cosmetic(vector3d,vector3d,num,num)");
  NEW slf_entity_wait_translate_t(slc_entity,"wait_translate(vector3d,num)");
  NEW slf_entity_wait_translate_with_compute_sector_t(slc_entity,"wait_translate_with_compute_sector(vector3d,num)");
  NEW slf_entity_wait_translate_cosmetic_t(slc_entity,"wait_translate_cosmetic(vector3d,num)");
  NEW slf_entity_wait_translate_WCS_t(slc_entity,"wait_translate_WCS(vector3d,num)");
  NEW slf_entity_wait_translate_WCS_with_compute_sector_t(slc_entity,"wait_translate_WCS_with_compute_sector(vector3d,num)");
  NEW slf_entity_wait_translate_WCS_cosmetic_t(slc_entity,"wait_translate_WCS_cosmetic(vector3d,num)");
  NEW slf_entity_get_rel_velocity_t(slc_entity,"get_rel_velocity(entity)");
  NEW slf_entity_in_sector_t(slc_entity,"in_sector(vector3d,vector3d,num)");
  NEW slf_entity_get_sector_name_t(slc_entity, "get_sector_name()");
  NEW slf_entity_force_current_region_t(slc_entity, "force_current_region()");
  NEW slf_entity_unforce_regions_t(slc_entity, "unforce_regions()");
  NEW slf_entity_equal_t(slc_entity,"operator==(entity)");
  NEW slf_entity_not_equal_t(slc_entity,"operator!=(entity)");
  NEW slf_entity_set_invulnerable_t(slc_entity,"set_invulnerable(num)");
  NEW slf_entity_set_targetting_t(slc_entity,"set_targetting(num)");
  NEW slf_entity_set_visible_t(slc_entity,"set_visible(num)");
  NEW slf_entity_set_active_t(slc_entity,"set_active(num)");
  NEW slf_entity_set_physical_t(slc_entity,"set_physical(num)");
  NEW slf_entity_raise_generic_signal_t(slc_entity,"raise_generic_signal(num)");
  NEW slf_entity_start_t(slc_entity,"start()");
  NEW slf_entity_stop_t(slc_entity,"stop()");
  NEW slf_entity_reset_t(slc_entity,"reset()");
//BIGCULL  NEW slf_entity_apply_damage_t(slc_entity,"apply_damage(num)");
//BIGCULL  NEW slf_entity_apply_directed_damage_t(slc_entity,"apply_directed_damage(num,vector3d)");
//BIGCULL  NEW slf_entity_apply_explosive_damage_t(slc_entity,"apply_explosive_damage(num,vector3d)");
  NEW slf_entity_motion_blur_on_t(slc_entity,"motion_blur_on()");
  NEW slf_entity_motion_blur_off_t(slc_entity,"motion_blur_off()");
  NEW slf_entity_motion_trail_on_t(slc_entity,"motion_trail_on()");
  NEW slf_entity_motion_trail_off_t(slc_entity,"motion_trail_off()");
  NEW slf_entity_is_picked_up_t(slc_entity,"is_picked_up()");
  NEW slf_entity_snap_to_t(slc_entity,"snap_to(entity)");
  NEW slf_entity_camera_set_target_t(slc_entity,"camera_set_target(vector3d)");

  NEW slf_entity_camera_set_roll_t(slc_entity,"camera_set_roll(num)");
  NEW slf_entity_wait_camera_set_roll_t(slc_entity,"wait_camera_set_roll(num,num)");

  NEW slf_entity_camera_set_collide_with_world_t(slc_entity,"camera_set_collide_with_world(num)");
  NEW slf_entity_camera_slide_to_t(slc_entity,"camera_slide_to(vector3d,vector3d,num,num)");
  NEW slf_entity_camera_slide_to_orbit_t(slc_entity,"camera_slide_to_orbit(vector3d,num,num,num,num)");
  NEW slf_entity_camera_orbit_t(slc_entity,"camera_orbit(vector3d,num,num,num)");
  NEW slf_entity_wait_change_color_t(slc_entity,"wait_change_color(vector3d,vector3d,num)");
  NEW slf_entity_wait_change_range_t(slc_entity,"wait_change_range(num,num,num)");
#ifdef GCCULL
  NEW slf_entity_play_ent_sound_t(slc_entity,"play_ent_sound(str)"); // depricated(sp?)
  NEW slf_entity_play_3d_ent_sound_t(slc_entity,"play_3d_ent_sound(str,num,num)");
  NEW slf_entity_play_3d_ent_sound_group_t(slc_entity,"play_3d_ent_sound_group(str,num,num)");
  NEW slf_entity_play_3d_ent_sound_group_looped_t(slc_entity,"play_3d_ent_sound_group_looped(str,num,num)");
  NEW slf_entity_kill_ent_sound_t(slc_entity,"kill_ent_sound(num)");
  NEW slf_entity_create_ent_sound_t(slc_entity,"create_ent_sound(str)");
  NEW slf_entity_create_ent_stream_t(slc_entity,"create_ent_stream(str)");
#endif
  NEW slf_entity_apply_blast_t(slc_entity,"apply_blast(vector3d,num,num)");
  NEW slf_entity_portal_open_t(slc_entity,"portal_open()");
  NEW slf_entity_portal_close_t(slc_entity,"portal_close()");
  NEW slf_entity_get_member_t(slc_entity,"get_member(str)");
  NEW slf_entity_get_nth_member_t(slc_entity,"get_member(num)");
  NEW slf_entity_get_num_members_t(slc_entity,"get_num_members()");
  NEW slf_entity_is_character_t(slc_entity,"is_character()");
  NEW slf_entity_to_character_t("entity_to_character(entity)");
  NEW slf_entity_load_anim_t(slc_entity,"load_anim(str)");
  NEW slf_entity_play_anim1_t(slc_entity,"play_anim(str)");
  NEW slf_entity_play_anim3_t(slc_entity,"play_anim(str,num,num)");
  NEW slf_entity_wait_play_anim1_t(slc_entity,"wait_play_anim(str)");
  NEW slf_entity_wait_play_anim3_t(slc_entity,"wait_play_anim(str,num,num)");
  NEW slf_load_effect_visual_t("load_effect_visual(str)");
  NEW slf_play_effect_t("play_effect(str,vector3d,num)");
  NEW slf_create_light_t("create_light(str,vector3d)");
  NEW slf_kill_light_t("kill_light(entity)");
  NEW slf_create_entity_t("create_entity(str)");
  NEW slf_create_entity_in_hero_region_t("create_entity_in_hero_region(str)");
  NEW slf_create_preloaded_static_entity_t("create_preloaded_static_entity(str)");
  NEW slf_cache_entity_t("cache_entity(str,num)");
  NEW slf_destroy_entity_t("destroy_entity(entity)");
  NEW slf_create_time_limited_entity_t("create_time_limited_entity(str,num)");
  NEW slf_entity_add_item_t(slc_entity,"add_item(entity)");
  NEW slf_entity_get_num_items_t(slc_entity,"get_num_items()");
  NEW slf_entity_get_item_t(slc_entity,"get_item(num)");
  NEW slf_entity_get_item_by_name_t(slc_entity,"get_item_by_name(str)");
  NEW slf_entity_get_item_quantity_t(slc_entity,"get_item_quantity(num)");
  NEW slf_entity_disgorge_items_t(slc_entity,"disgorge_items()");
  NEW slf_entity_set_control_active_t(slc_entity,"set_control_active(num)");
  NEW slf_entity_suspend_t(slc_entity,"suspend()");
  NEW slf_entity_unsuspend_t(slc_entity,"unsuspend()");
  NEW slf_entity_wait_for_pickup_t(slc_entity,"wait_for_pickup()");
  NEW slf_entity_wait_prox1_t(slc_entity,"wait_prox(entity,num)");
  NEW slf_entity_wait_prox2_t(slc_entity,"wait_prox(vector3d,num)");
  NEW slf_entity_wait_prox3_t(slc_entity,"wait_prox(vector3d,num,vector3d,num)");
  NEW slf_entity_wait_prox_minY_t(slc_entity,"wait_prox_minY(vector3d,num,num)");
  NEW slf_entity_wait_prox_maxY_t(slc_entity,"wait_prox_maxY(vector3d,num,num)");
  NEW slf_entity_wait_for_sector_t(slc_entity,"wait_for_sector(str)");
  NEW slf_entity_wait_for_not_sector_t(slc_entity,"wait_for_not_sector(str)");
  NEW slf_entity_wait_prox_sector_t(slc_entity,"wait_prox_sector(vector3d,num,str)");
  NEW slf_remove_item_from_world_t("remove_item_from_world(num)");
  NEW slf_remove_item_entity_from_world_t("remove_item_entity_from_world(entity)");
  NEW slf_entity_ifl_lock_t(slc_entity,"ifl_lock(num)");
  NEW slf_entity_ifl_pause_t(slc_entity,"ifl_pause()");
  NEW slf_entity_ifl_play_t(slc_entity,"ifl_play()");
  NEW slf_action_activated_by_t(slc_entity, "action_activated_by()");

  register_entity2_lib(slc_entity);
	#endif
}


