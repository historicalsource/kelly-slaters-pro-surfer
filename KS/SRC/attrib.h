#if 0

#ifndef ATTRIB_H
#define ATTRIB_H

#include "algebra.h"
#include "instance.h"
#include "oserrmsg.h"
#include "stringx.h"
//!class character_attributes;

//  The attribute system is a "Pull" system for attribute access.
//  This is done so that we can dynamically reloadthe attribute file and have its effect be correct.
//  This means that attribute values should never be used to construct anything which has permanence.


enum attribute_type
{
  ATYPE_INT,
  ATYPE_RATIONAL_T,
  ATYPE_VECTOR3D,
  ATYPE_STRING,
  ATYPE_COUNT
};

// Defines munged versions of the attribute_type names for use in the macros included thgrough such files
// as "hard_attrib.h"

// for use where the variables are defined
#define def_type_ATYPE_INT int
#define def_type_ATYPE_RATIONAL_T rational_t
#define def_type_ATYPE_VECTOR3D vector3d
#define def_type_ATYPE_STRING stringx

// for use where the access functions are defined
#define getset_type_ATYPE_INT int
#define getset_type_ATYPE_RATIONAL_T rational_t
#define getset_type_ATYPE_VECTOR3D const vector3d&
#define getset_type_ATYPE_STRING const stringx&

enum att_set_t
{
  HARD_ATTRIB,
  SOFT_ATTRIB,
};

const rational_t ATT_DEFAULT_VAL = -9999;

class reg_entry
{

  public:
    enum
    {
      FLAG_NON_DEFAULT=1
    };

    reg_entry() { modified = NULL; }
    reg_entry( const stringx& _name, attribute_type _type, int _offset )
    {
      name = _name;
      type = _type;
      offset = _offset;
      modified = NULL;
    }

    const stringx& get_name() { return name; }
    void set_name( const stringx& _name ) { name = _name; }

    attribute_type get_type(){return type;}
    void set_type(attribute_type _type) {type = _type;}

    int get_offset() {return offset;}
    void set_offset(int _offset) {offset = _offset;}
    
    void set_modified(void *ptr) { modified = ptr; }
    void *is_modified()          { return(modified); }

  private:
    stringx name;
    attribute_type type;
    int offset;
    void *modified;
};


class attribute_registry
{
  public:
    attribute_registry( character_attributes& attrib );

    int get_idx( const stringx& att_name );

    const stringx& get_name( int idx )
    {
      return entries[idx].get_name();
    }
    attribute_type get_type(int idx)
    {
      return entries[idx].get_type();
    }
    int get_offset(int idx)
    {
      return entries[idx].get_offset();
    }
    void *is_modified(int idx)
    {
      return entries[idx].is_modified();
    }
    void set_modified(int idx, void *ptr)
    {
      entries[idx].set_modified(ptr);
    }
  private:
    vector <reg_entry> entries;
};


//!class character;

class character_attributes
{
  public:
    character_attributes();
    virtual ~character_attributes() {}

    virtual void build_registry_entries(vector<reg_entry> & re)=0;
    virtual attribute_registry * get_reg()=0;

    void load_attributes(const char * att_filename);

    void * pointer(int idx);

    int get_idx( const stringx& name ) { return get_reg()->get_idx(name); }
    int get_type(int idx) {return get_reg()->get_type(idx);}

    character *is_modified(int idx)                        { return (character *)get_reg()->is_modified(idx);  }
    character *is_modified(const stringx& name)            { return (character *)is_modified(get_idx( name )); }
    void set_modified(int idx, character *chr)             { get_reg()->set_modified(idx, chr);   }
    void set_modified(const stringx& name, character *chr) { set_modified(get_idx( name ), chr);  }

    // Note:  I could not figure out how to make a registry_index class, with a constructor which
    // would convert strings to registry_indices (analogue used for entity_id's in DBTS), because
    // of the dependece on the pure virtual get_reg() for the conversion.  That explains why there
    // are two versions of each, instead of the aforementioned approach.  A little uglier, but it
    // works the same.
    int get_int(int idx)
    {
      assert(get_reg()->get_type(idx) == ATYPE_INT);
      return *((int *) pointer(idx));
    }
    int get_int( const stringx& name )
    {
      return get_int( get_idx( name ) );
    }

    rational_t get_rational_t(int idx)
    {
      assert(get_reg()->get_type(idx) == ATYPE_RATIONAL_T);
      return *((rational_t *) pointer(idx));
    }
    rational_t get_rational_t( const stringx& name )
    {
      return get_rational_t( get_idx( name ) );
    }

    const vector3d& get_vector3d( int idx )
    {
      assert(get_reg()->get_type(idx) == ATYPE_VECTOR3D);
      return *((vector3d *) pointer(idx));
    }
    const vector3d& get_vector3d( const stringx& name )
    {
      return get_vector3d( get_idx( name ) );
    }

    const stringx& get_string( int idx )
    {
      assert(get_reg()->get_type(idx) == ATYPE_STRING);
      return *((stringx *) pointer(idx));
    }
    const stringx& get_string( const stringx& name )
    {
      return get_string( get_idx(name) );
    }

    void set_int(int idx, int val)
    {
      if (get_reg()->get_type(idx) != ATYPE_INT)
        error( get_reg()->get_name(idx) + " is not of type 'int'." );
      *((int *) pointer(idx)) = val;
    }
    void set_int( const stringx& name, int val )
    {
      set_int(get_idx(name), val);
    }

    void set_rational_t(int idx, rational_t val)
    {
      if (get_reg()->get_type(idx) != ATYPE_RATIONAL_T)
        error( get_reg()->get_name(idx) + " is not of type 'rational_t'." );
      *((rational_t *) pointer(idx)) = val;
    }
    void set_rational_t( const stringx& name, rational_t val )
    {
      set_rational_t(get_idx(name), val);
    }

    void set_vector3d( int idx, const vector3d& val )
    {
      if (get_reg()->get_type(idx) != ATYPE_VECTOR3D)
        error( get_reg()->get_name(idx) + " is not of type 'vector3d'." );
      *((vector3d *) pointer(idx)) = val;
    }
    void set_vector3d( const stringx& name, const vector3d& val )
    {
      set_vector3d( get_idx(name), val );
    }

    void set_string( int idx, const stringx& val )
    {
      if (get_reg()->get_type(idx) != ATYPE_STRING)
        error( get_reg()->get_name(idx) + " is not of type 'stringx'." );
      *((stringx *) pointer(idx)) = val;
    }
    void set_string( const stringx& name, const stringx& val )
    {
      set_string( get_idx(name), val );
    }
};


// These are the attributes loaded from disk when the character is created.  They should be instanced.
class character_hard_attributes : public character_attributes
{
  public:
    // I had to add the second constructor for the instance template to compile.  It wasn't correctly
    // finding the (const char * att_filename) constructor from character_attributes.  That required
    // adding the first as well.  If I am concealing an error here, I hope someone will catch it eventually.
    // PTA 9/27/98
    // (SEE ALSO soft_attributes BELOW)
    character_hard_attributes() : character_attributes(){}
    character_hard_attributes(const char * att_filename, unsigned flags);
    ~character_hard_attributes() {}

    virtual void build_registry_entries(vector<reg_entry> & re);
    virtual attribute_registry * get_reg();

// Get-Set pairs of the form:
//    rational_t get_crouch_height() const {return crouch_height;}
//    void set_crouch_height(rational_t v){crouch_height = v;}
#define MAC(x,y,v,d) \
    getset_type_##y get_##v() const {return v;}  \
    void set_##v(getset_type_##y in){v = in;}
#include "hard_attribs.h"
#undef MAC

    static att_set_t get_att_set(){return HARD_ATTRIB;}

// C-style access functions, for being passed to objects, of the form:
//    static rational_t get_max_speed_forward(character_hard_attributes * q)
//      {return q->get_max_speed_forward();}
#define MAC(x,y,v,d)                  \
    static getset_type_##y static_get_##v(character_hard_attributes * q) \
      {return q->get_##v();}
#include "hard_attribs.h"
#undef MAC

  private:
    static attribute_registry registry;

// Attribute variable definitions, of the form:
//    rational_t crouch_height;
#define MAC(x,y,v,d) def_type_##y v;
#include "hard_attribs.h"
#undef MAC


    // Derivitive attributes;
};



// These are the saved data about the character.
class character_soft_attributes : public character_attributes
{
  public:
    enum
    {
      NEUTRAL_TEAM=-2,
      NO_TEAM=-1,
      HERO_TEAM=0,
      MONSTER_TEAM_1=1
    };

    // SEE COMMENT ON hard_attributes ABOVE
    character_soft_attributes() : character_attributes(){}
    character_soft_attributes(const char * att_filename);
    character_soft_attributes(character_hard_attributes * _hard_att);
    virtual ~character_soft_attributes() {}

    virtual void build_registry_entries(vector<reg_entry> & rer);
    virtual attribute_registry * get_reg();



// Get-Set pairs of the form:
//    rational_t get_crouch_height() const {return crouch_height;}
//    void set_crouch_height(rational_t v){crouch_height = v;}
#define MAC(x,y,v,d) \
    getset_type_##y get_##v() const {return v;}  \
    void set_##v(getset_type_##y in){v = in;}
#include "soft_attribs.h"
#undef MAC

// C-style access functions, for being passed to objects, of the form:
//    static rational_t get_max_speed_forward(character_hard_attributes * q)
//      {return q->get_max_speed_forward();}
#define MAC(x,y,v,d)                  \
    static getset_type_##y static_get_##v(character_soft_attributes * q) \
      {return q->get_##v();}
#include "soft_attribs.h"
#undef MAC

    int get_hit_points() const {return hit_points;}
    void set_hit_points(int h);
    int is_alive(){return alive;}
    void set_alive(int a){alive = a;}

    static att_set_t get_att_set(){return SOFT_ATTRIB;}

  private:
    void build_defaults();

    static attribute_registry registry;

// Attribute variable definitions, of the form:
//    rational_t crouch_height;
#define MAC(x,y,v,d) def_type_##y v;
#include "soft_attribs.h"
#undef MAC

    int hit_points;
    bool alive;

    character_hard_attributes * hard_att;
};

extern instance_bank<character_hard_attributes> character_hard_attributes_bank;


#endif


#endif