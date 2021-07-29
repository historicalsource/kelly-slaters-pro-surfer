#include "global.h"

#include "project.h"
#include "marker.h"
//!#include "character.h"

entity* marker::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  marker* new_mk = NEW marker( _id, ENTITY_MARKER );
  new_mk->copy_instance_data( *((marker *)this) );
  return (entity*)new_mk;
}





bool rectangle_marker::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("rect_mkr") )
  {
    stringx cf;
    for ( serial_in(fs,&cf); cf!=chunkend_label; serial_in(fs,&cf) )
    {
      if ( cf == stringx("z_rad") )
      {
        serial_in( fs, &z_rad );
        z_rad = __fabs(z_rad);
      }
      else if ( cf == stringx("x_rad") )
      {
        serial_in( fs, &x_rad );
        x_rad = __fabs(x_rad);
      }
      else
      {
        error( get_id().get_val() + ": rectangle_marker::parse_instance(): bad keyword '" + cf.c_str() + "'");
        return(false);
      }
    }

    return(true);
  }
  else
    return(marker::parse_instance( pcf, fs ));
}


bool rectangle_marker::point_inside(const vector3d &pt)
{
  vector3d d = pt - get_abs_position();
  return(__fabs(dot(get_abs_po().get_x_facing(), d)) <= x_rad && __fabs(dot(get_abs_po().get_z_facing(), d)) <= z_rad);
}

entity* rectangle_marker::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  rectangle_marker* new_mk = NEW rectangle_marker( _id, ENTITY_MARKER );
  new_mk->copy_instance_data( *((rectangle_marker *)this) );
  return (entity*)new_mk;
}






entity* cube_marker::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  cube_marker* new_mk = NEW cube_marker( _id, ENTITY_MARKER );
  new_mk->copy_instance_data( *((cube_marker *)this) );
  return (entity*)new_mk;
}

bool cube_marker::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("cube_mkr") )
  {
    stringx cf;
    for ( serial_in(fs,&cf); cf!=chunkend_label; serial_in(fs,&cf) )
    {
      if ( cf == stringx("z_rad") )
      {
        serial_in( fs, &z_rad );
        z_rad = __fabs(z_rad);
      }
      else if ( cf == stringx("x_rad") )
      {
        serial_in( fs, &x_rad );
        x_rad = __fabs(x_rad);
      }
      else if ( cf == stringx("y_rad") )
      {
        serial_in( fs, &y_rad );
        y_rad = __fabs(y_rad);
      }
      else
      {
        error( get_id().get_val() + ": cube_marker::parse_instance(): bad keyword '" + cf.c_str() + "'");
        return(false);
      }
    }

    return(true);
  }
  else
    return(rectangle_marker::parse_instance( pcf, fs ));
}


bool cube_marker::point_inside(const vector3d &pt)
{
  vector3d d = pt - get_abs_position();
  return(__fabs(dot(get_abs_po().get_x_facing(), d)) <= x_rad && __fabs(dot(get_abs_po().get_z_facing(), d)) <= z_rad && __fabs(dot(get_abs_po().get_y_facing(), d)) <= y_rad);
}









/*!
entity* crawl_marker::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  crawl_marker* new_mk = NEW crawl_marker( _id, ENTITY_CRAWL_MARKER );
  new_mk->copy_instance_data( *((crawl_marker *)this) );
  return (entity*)new_mk;
}

#define CRAWL_MIN_X_RAD 0.65f
#define CRAWL_MIN_Z_RAD 0.55f
#define CRAWL_PRECENT_INC 0.1f

bool crawl_marker::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("crawl") )
  {
    stringx cf;
    for ( serial_in(fs,&cf); cf!=chunkend_label; serial_in(fs,&cf) )
    {
      if ( cf == stringx("z_rad") )
      {
        serial_in( fs, &z_rad );
        z_rad = __fabs(z_rad);
      }
      else if ( cf == stringx("x_rad") )
      {
        serial_in( fs, &x_rad );
        x_rad = __fabs(x_rad);
      }
      else if ( cf == stringx("pinpoint") )
      {
        set_ext_flag(EFLAG_EXT_CRAWL_PINPOINT, true);
      }
      else
      {
        error( get_id().get_val() + ": crawl_marker::parse_instance(): bad keyword '" + cf.c_str() + "'");
        return(false);
      }
    }

  #if !defined( PROJECT_STEEL )
    if(!is_ext_flagged(EFLAG_EXT_CRAWL_PINPOINT))
      warning("%s: Only pinpoint crawl markers are allowed in this project!", get_name().c_str())
  #endif

    if(x_rad < CRAWL_MIN_X_RAD)
      x_rad = CRAWL_MIN_X_RAD;

    if(z_rad < CRAWL_MIN_Z_RAD)
      z_rad = CRAWL_MIN_Z_RAD;

    return(true);
  }
  else
    return(rectangle_marker::parse_instance( pcf, fs ));

  return(true);
}


#define _COS_45_DEGREES 0.707106781186547524400844362104849f

bool is_crawling(character *chr);

bool crawl_marker::point_inside(const vector3d &pt)
{
  vector3d d = pt - get_abs_position();
  return(__fabs(dot(get_abs_po().get_x_facing(), d)) <= (x_rad + (x_rad * CRAWL_PRECENT_INC)) && __fabs(dot(get_abs_po().get_z_facing(), d)) <= (z_rad + (z_rad * CRAWL_PRECENT_INC)));
}

bool crawl_marker::allow_crawl(character *chr, rational_t dir)
{
  return(point_inside(chr->get_abs_position()) && crawl_or_stand(chr, dir) != is_crawling(chr));
}

bool crawl_marker::crawl_or_stand(character *chr, rational_t dir)
{
  // if we are moving backwards(back on stick), stop crawling...
  if(dir > 0.0f)
    return(false);
  else
  {
    vector3d face = chr->get_abs_po().get_facing();
    face.y = 0.0f;
    face.normalize();

    vector3d my_face = get_abs_po().get_facing();
    my_face.y = 0.0f;
    my_face.normalize();

    return(dot(face, my_face) > _COS_45_DEGREES);
  }
}



vector3d crawl_marker::get_crawl_pos(character *chr)
{
  vector3d pos = get_abs_position();

#if !defined( PROJECT_STEEL )
  if(!is_ext_flagged(EFLAG_EXT_CRAWL_PINPOINT))
  {
    vector3d v = get_abs_po().get_x_facing();
    vector3d w = chr->get_abs_position() - pos;

    vector3d delta = v*(dot(v, w));

    pos += delta;
    pos.y = chr->get_abs_position().y;
  }
#endif

  pos.y = chr->get_abs_position().y;

  return(pos);
}
!*/
