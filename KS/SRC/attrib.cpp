#if 0



/*

      attrib.cpp

      Set of attributes of a character.  Includes subclasses for SOFT and HARD, HARD_LEG, HARD_ARM, HARD_WING, and HARD_UPGRADE character attributes

*/
#include "global.h"

#include "osalloc.h"

//!#include "attrib.h"
#include "hwaudio.h"

////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////
//! attribute_registry g_char_hard_att_reg(character_hard_attributes());
//! attribute_registry g_char_soft_att_reg(character_soft_attributes());

//! instance_bank<character_hard_attributes> character_hard_attributes_bank;


////////////////////////////////////////////////////////////////////////
// attribute_registry
////////////////////////////////////////////////////////////////////////

// General functions for registry management

attribute_registry::attribute_registry( character_attributes& attrib )
  {
  attrib.build_registry_entries( entries );
  }


int attribute_registry::get_idx( const stringx& att_name )
  {
  int idx = 0;
  vector<reg_entry>::iterator ent;
  for (ent=entries.begin();ent<entries.end();ent++)
    {
    if ( att_name == ent->get_name() )
      break;
    idx++;
    }
  if (ent==entries.end())
    error( "Invalid attribute name: " + att_name );
  return idx;
  }

////////////////////////////////////////////////////////////////////////
// char_attributes
////////////////////////////////////////////////////////////////////////

character_attributes::character_attributes()
  {
  }


void character_attributes::load_attributes(const char * att_filename)
  {
  chunk_file fs;
  stringx att_filename_str(att_filename);
  fs.open( att_filename_str, os_file::FILE_READ|chunk_file::FILE_TEXT );

  stringx name,equals,input;
  int int_val;
  rational_t rational_t_val;
  vector3d vector3d_val;
  stringx string_val;

  serial_in(fs,&name);
  while (name!=stringx("EOF"))
    {
    serial_in(fs,&equals);
    int idx = get_idx(name);
    switch (get_type(idx))
      {
      case ATYPE_INT:
        serial_in(fs,&int_val);
        set_int(idx,int_val);
        break;
      case ATYPE_RATIONAL_T:
        serial_in(fs,&rational_t_val);
        set_rational_t(idx,rational_t_val);
        break;
      case ATYPE_VECTOR3D:
        serial_in(fs,&vector3d_val);
        set_vector3d(idx,vector3d_val);
        break;
      case ATYPE_STRING:
        serial_in(fs,&string_val);
        set_string(idx,string_val);
        break;
      }
    serial_in(fs,&name);
    }
  }


void * character_attributes::pointer(int idx)
  {
  return (void *) (((char *)this)+get_reg()->get_offset(idx));
  }


////////////////////////////////////////////////////////////////////////
// character_hard_attributes
////////////////////////////////////////////////////////////////////////

// Sets up all hard character stats.
// These are the data loaded from disk about the character.

character_hard_attributes::character_hard_attributes(const char * att_filename, unsigned flags)
  {
// Attribute default values, of the form:
//    death_scream_name = "kob_dth0";
#define MAC(x,y,v,d) v = d;
#include "hard_attribs.h"
#undef MAC

  load_attributes(att_filename); //character_attributes::character_attributes(att_filename);

  }


void character_hard_attributes::build_registry_entries(vector<reg_entry> & re)
  {
  char * base = (char *) this;

// Atribute registry entries, of the form:
//  re.push_back(reg_entry("CROUCH_HEIGHT", ATYPE_RATIONAL_T, (char *) &crouch_height - base));
#define MAC(x,y,v,d) re.push_back(reg_entry(x,y,(char *) &v - base));
#include "hard_attribs.h"
#undef MAC
    }


attribute_registry * character_hard_attributes::get_reg()
  {
  return &g_char_hard_att_reg;
  }


////////////////////////////////////////////////////////////////////////
// character_soft_attributes
////////////////////////////////////////////////////////////////////////

// Sets up all soft character stats.
// These are the saved data about the character.


character_soft_attributes::character_soft_attributes(const char * att_filename)
  {

#define MAC(x,y,v,d) v = d;
#include "soft_attribs.h"
#undef MAC

  build_defaults();
  load_attributes(att_filename); //character_attributes::character_attributes(att_filename);
  }


character_soft_attributes::character_soft_attributes(character_hard_attributes * _hard_att)
  {

#define MAC(x,y,v,d) v = d;
#include "soft_attribs.h"
#undef MAC

  build_defaults();
  hard_att = _hard_att;
  full_hit_points = hard_att->get_full_hit_points();
  }


void character_soft_attributes::build_defaults()
  {
  // default values
  }


void character_soft_attributes::build_registry_entries(vector<reg_entry> & re)
  {
  char * base = (char *) this;
// Atribute registry entries, of the form:
//  re.push_back(reg_entry("CROUCH_HEIGHT", ATYPE_RATIONAL_T, (char *) &crouch_height - base));
#define MAC(x,y,v,d) re.push_back(reg_entry(x,y,(char *) &v - base));
#include "soft_attribs.h"
#undef MAC

  // these have special get-set functions, so they are done manually
  re.push_back(reg_entry("HIT_POINTS", ATYPE_INT, (char *) &hit_points - base));
  }


attribute_registry * character_soft_attributes::get_reg()
  {
  return &g_char_soft_att_reg;
  }


void character_soft_attributes::set_hit_points(int v)
  {
  rational_t min = 0;
  rational_t max = 100000;
  //hard_att->get_full_hit_points();
  if (v<min) v=min;
  else if (v>max) v=max;
  hit_points = v;
  }
#endif