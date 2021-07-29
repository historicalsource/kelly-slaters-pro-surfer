////////////////////////////////////////////////////////////////////////////////
/*
  ladder.cpp

  Class ladder is derived from entity, and allows vertical movement of characters.

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!include "character.h"
//!#include "ladder.h"
#include "terrain.h"
#include "osdevopts.h"


ladder::ladder( const entity_id& _id, unsigned int _flags )
:   entity( _id, ENTITY_LADDER, _flags )
{
  initialize_variables();
}


ladder::ladder( const entity_id& _id,
                entity_flavor_t _flavor,
                unsigned int _flags )
:   entity( _id, _flavor, _flags )
{
  initialize_variables();
}


ladder::ladder(const entity_id & eid, rational_t _height, entity_flavor_t _flavor)
    : entity( eid, _flavor )
{
  assert(0);
  initialize_variables();
}


void ladder::initialize_variables()
{
  lad_type = UNDEFINED_LADDER;

  top_mount_offset = ZEROVEC;
  bottom_mount_offset = ZEROVEC;

  top_dismount_offset = ZEROVEC;
  bottom_dismount_offset = ZEROVEC;

  climb_up_r_anim = stringx();
  climb_up_l_anim = stringx();

  climb_down_r_anim = stringx();
  climb_down_l_anim = stringx();

  mount_top_anim = stringx();
  mount_bottom_anim = stringx();

  dismount_top_l_anim = stringx();
  dismount_top_r_anim = stringx();

  dismount_bottom_l_anim = stringx();
  dismount_bottom_r_anim = stringx();

  gravity_booster = 100.0f;
}


///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

ladder::ladder( chunk_file& fs,
                const entity_id& _id,
                entity_flavor_t _flavor,
                unsigned int _flags )
:   entity( fs, _id, _flavor, _flags )
{
  initialize_variables();
}


bool ladder::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  if(label == "ladder:")
  {
    serial_in(fs,&label);

    while(label != chunkend_label)
    {
      if(label == "type")
      {
        stringx type;

        serial_in(fs,&type);

        if(type == "vertical")
          lad_type = VERTICAL_LADDER;
        else if(type == "horizontal")
          lad_type = HORIZONTAL_LADDER;
        else if(type == "zip_line")
          lad_type = ZIP_LINE;
        else
          error( fs.get_filename() + ": Bad ladder type '" + type + "'" );
      }
      else if(label == "gravity_booster")
      {
        serial_in(fs, &gravity_booster);
      }
      else if(label == "top_mount_offset")
      {
        serial_in(fs, &top_mount_offset);
      }
      else if(label == "bottom_mount_offset")
      {
        serial_in(fs, &bottom_mount_offset);
      }
      else if(label == "top_dismount_offset")
      {
        serial_in(fs, &top_dismount_offset);
      }
      else if(label == "bottom_dismount_offset")
      {
        serial_in(fs, &bottom_dismount_offset);
      }
      else if(label == "climb_anim")
      {
        serial_in(fs, &climb_up_r_anim);
        climb_up_l_anim = climb_up_r_anim;
        climb_down_l_anim = climb_up_r_anim;
        climb_down_r_anim = climb_up_r_anim;
      }
      else if(label == "climb_up_anim")
      {
        serial_in(fs, &climb_up_r_anim);
        climb_up_l_anim = climb_up_r_anim;
      }
      else if(label == "climb_up_r_anim")
      {
        serial_in(fs, &climb_up_r_anim);
      }
      else if(label == "climb_up_l_anim")
      {
        serial_in(fs, &climb_up_l_anim);
      }
      else if(label == "climb_down_anim")
      {
        serial_in(fs, &climb_down_r_anim);
        climb_down_l_anim = climb_down_r_anim;
      }
      else if(label == "climb_down_r_anim")
      {
        serial_in(fs, &climb_down_r_anim);
      }
      else if(label == "climb_down_l_anim")
      {
        serial_in(fs, &climb_down_l_anim);
      }
      else if(label == "mount_anim")
      {
        serial_in(fs, &mount_top_anim);
        mount_bottom_anim = mount_top_anim;
      }
      else if(label == "mount_top_anim")
      {
        serial_in(fs, &mount_top_anim);
      }
      else if(label == "mount_bottom_anim")
      {
        serial_in(fs, &mount_bottom_anim);
      }
      else if(label == "dismount_anim")
      {
        serial_in(fs, &dismount_top_l_anim);
        dismount_top_r_anim = dismount_top_l_anim;
        dismount_bottom_l_anim = dismount_top_l_anim;
        dismount_bottom_r_anim = dismount_top_l_anim;
      }
      else if(label == "dismount_top_anim")
      {
        serial_in(fs, &dismount_top_l_anim);
        dismount_top_r_anim = dismount_top_l_anim;
      }
      else if(label == "dismount_top_l_anim")
      {
        serial_in(fs, &dismount_top_l_anim);
      }
      else if(label == "dismount_top_r_anim")
      {
        serial_in(fs, &dismount_top_r_anim);
      }
      else if(label == "dismount_bottom_anim")
      {
        serial_in(fs, &dismount_bottom_l_anim);
        dismount_bottom_r_anim = dismount_bottom_l_anim;
      }
      else if(label == "dismount_bottom_l_anim")
      {
        serial_in(fs, &dismount_bottom_l_anim);
      }
      else if(label == "dismount_bottom_r_anim")
      {
        serial_in(fs, &dismount_bottom_r_anim);
      }
      else
        error( fs.get_filename() + ": Unknown keyword '" + label + "' in ladder section" );

      serial_in(fs,&label);
    }

//    assert(g_world_ptr->get_hero_ptr());
//!    load_anims(g_world_ptr->get_hero_ptr());

    return(true);
  }
  else
    return(entity::handle_enx_chunk(fs, label));
}

///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

ladder::ladder( const stringx& entity_fname,
                     const entity_id& _id ,
                     entity_flavor_t _flavor )
:   entity( entity_fname, _id, _flavor, SKIP_DELETE_STREAM )
{
  assert( my_fs );
  entity::read_enx(*my_fs);
  delete my_fs;
  my_fs = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////
entity* ladder::make_instance( const entity_id& _id,
                               unsigned int _flags ) const
{
  ladder* newld = NEW ladder( _id, _flags );
  newld->copy_instance_data( *this );
  return newld;
}


void ladder::copy_instance_data( const ladder& b )
{
  entity::copy_instance_data( b );

  lad_type = b.lad_type;

  top_mount_offset = b.top_mount_offset;
  bottom_mount_offset = b.bottom_mount_offset;

  top_dismount_offset = b.top_dismount_offset;
  bottom_dismount_offset = b.bottom_dismount_offset;

  climb_up_r_anim = b.climb_up_r_anim;
  climb_up_l_anim = b.climb_up_l_anim;

  climb_down_r_anim = b.climb_down_r_anim;
  climb_down_l_anim = b.climb_down_l_anim;

  mount_top_anim = b.mount_top_anim;
  mount_bottom_anim = b.mount_bottom_anim;

  dismount_top_l_anim = b.dismount_top_l_anim;
  dismount_top_r_anim = b.dismount_top_r_anim;

  dismount_bottom_l_anim = b.dismount_bottom_l_anim;
  dismount_bottom_r_anim = b.dismount_bottom_r_anim;

  gravity_booster = b.gravity_booster;
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

// these virtual functions allow types descended from entity to be
// recognized when adding them to regions, so that the region class can
// maintain lists of different entity types as desired
void ladder::add_me_to_region( region* r )
{
  r->add( this );
}

void ladder::remove_me_from_region( region* r )
{
  r->remove( this );
}


vector3d ladder::get_top_mount_position()
{
  return(get_abs_position() + my_po.fast_8byte_non_affine_xform(top_mount_offset) );
}

vector3d ladder::get_bottom_mount_position()
{
  return(get_abs_position() + my_po.fast_8byte_non_affine_xform(bottom_mount_offset) );
}

vector3d ladder::get_top_dismount_position()
{
  return(get_abs_position() + my_po.fast_8byte_non_affine_xform(top_dismount_offset) );
}

vector3d ladder::get_bottom_dismount_position()
{
  return(get_abs_position() + my_po.fast_8byte_non_affine_xform(bottom_dismount_offset) );
}


#define _LADDER_FACE_THETA 0.5f

// temp_testing
//beam *zip_beamer = NULL;
//bool test_zip_beamer = false;

bool ladder::allow_mount(entity *ent)
{
  switch(lad_type)
  {
    case VERTICAL_LADDER:
    case HORIZONTAL_LADDER:
    {
      if ( (ent->get_abs_position() - get_top_mount_position()).length2() < 0.35f )
      {
        vector3d face = get_abs_po().get_facing();
        face.y = 0.0f;

        assert( face.length() );

        face.normalize();

        vector3d ent_face = ent->get_abs_po().get_facing();
        ent_face.y = 0.0f;

        assert( ent_face.length() );

        ent_face.normalize();

        if ( dot(face, ent_face) > _LADDER_FACE_THETA )
          return true;
      }
      else if((ent->get_abs_position() - get_bottom_mount_position()).length2() < 0.25f)
      {
        vector3d face = get_abs_po().get_facing() * -1.0f;
        face.y = 0.0f;

        assert(face.length());

        face.normalize();

        vector3d ent_face = ent->get_abs_po().get_facing();
        ent_face.y = 0.0f;

        assert(ent_face.length());

        ent_face.normalize();

        if ( dot(face, ent_face) > _LADDER_FACE_THETA )
          return true;
      }
    }
    break;

    case ZIP_LINE:
    {
      if((ent->get_abs_position() - get_top_mount_position()).length2() < 0.25f)
      {
/*
        if(!zip_beamer && test_zip_beamer)
        {
          zip_beamer = g_world_ptr->add_beam( entity_id::make_unique_id(), (unsigned int)(EFLAG_ACTIVE|EFLAG_MISC_NONSTATIC) );
          zip_beamer->set_color(color32_blue);
          zip_beamer->set_point_to_point(get_top_mount_position()+(YVEC*1.25f), get_bottom_dismount_position()+(YVEC*1.25f));
          zip_beamer->compute_sector(g_world_ptr->get_the_terrain());
        }
*/

        return true;
      }
    }
    break;

    default:
      assert(0);
      break;
  }

  return false;
}


void ladder::load_anims(character *chr)
{
// BETH  stringx dir_name = stringx("characters\\")+g_game_ptr->get_game_info().get_hero_name()+stringx("\\");
	stringx dir_name = stringx("characters\\")+g_game_ptr->getHeroname()+stringx("\\");

  if(chr && chr->has_dirname())
  {
    dir_name = chr->get_dirname();
  }

  if(!(climb_up_r_anim == empty_string))
    load_entity_track(dir_name+climb_up_r_anim);
  if(!(climb_up_l_anim == empty_string))
    load_entity_track(dir_name+climb_up_l_anim);

  if(!(climb_down_r_anim == empty_string))
    load_entity_track(dir_name+climb_down_r_anim);
  if(!(climb_down_l_anim == empty_string))
    load_entity_track(dir_name+climb_down_l_anim);

  if(!(mount_top_anim == empty_string))
    load_entity_track(dir_name+mount_top_anim);
  if(!(mount_bottom_anim == empty_string))
    load_entity_track(dir_name+mount_bottom_anim);

  if(!(dismount_top_l_anim == empty_string))
    load_entity_track(dir_name+dismount_top_l_anim);
  if(!(dismount_top_r_anim == empty_string))
    load_entity_track(dir_name+dismount_top_r_anim);

  if(!(dismount_bottom_l_anim == empty_string))
    load_entity_track(dir_name+dismount_bottom_l_anim);
  if(!(dismount_bottom_r_anim == empty_string))
    load_entity_track(dir_name+dismount_bottom_r_anim);
}

