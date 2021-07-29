#ifndef SOFT_ATTRIBUTE_INTERFACE_CLASS_HEADER
#define SOFT_ATTRIBUTE_INTERFACE_CLASS_HEADER

#include "global.h"
#include "entity_interface.h"
#include "attribute_template.h"
#include "pstring.h"

class entity;

/*** generate the attribute classes ***/
#define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val)               \
    class code_name##_attrib : public atype##_attribute<##itype##>                                      \
    {                                                                                                   \
      protected:                                                                                        \
        static pstring attribute_name;                                                                  \
                                                                                                        \
      public:                                                                                           \
        code_name##_attrib()                                                                            \
          : atype##_attribute<##itype##>(itype_name, default_val, min_val, max_val)                     \
        {                                                                                               \
        }                                                                                               \
        const pstring &get_name() const { return attribute_name; }                                      \
    };

#include "entity_soft_attribs.h"
#include "character_soft_attribs.h"
#undef MAC


/*** soft attribute interface ***/
class soft_attrib_interface : public entity_interface
{
  protected:
    /*** soft attribute members ***/
    #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) code_name##_attrib code_name;
    #include "entity_soft_attribs.h"
    #undef MAC

  public:
    /*** constructors/destructors ***/
    soft_attrib_interface(entity *_my_entity)
      : entity_interface(_my_entity)
    {
    }
    virtual ~soft_attrib_interface() {}

    /*** get_soft_attrib_num ***/
    virtual const rational_t get_soft_attrib_num(pstring &by_name) const 
    {
      #define PROCESS_NUMBERS_ONLY
      #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) \
        if (by_name == code_name##.get_name()) { return (rational_t)##code_name##.get(); }

      #include "entity_soft_attribs.h"
      #undef MAC
      #undef PROCESS_NUMBERS_ONLY
      assert("Could not get a soft attribute number by name." == 0);
      return 0.0f;
    }

    /*** get_soft_attrib_str ***/
    virtual const pstring get_soft_attrib_str(pstring &by_name) const 
    {
      #define PROCESS_STRINGS_ONLY
      #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) \
        if (by_name == code_name##.get_name()) { return code_name##.get(); }

      #include "entity_soft_attribs.h"
      #undef MAC
      #undef PROCESS_STRINGS_ONLY
      assert("Could not get a soft attribute string by name." == 0);
      return 0;
    }

  /*** get attrib methods ***/
  #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) itype get_##code_name##() { return(##code_name##); }
  #include "entity_soft_attribs.h"
  #undef MAC

  /*** set attrib methods ***/
  #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) void set_##code_name##(const itype &value) { code_name.set(value); }
  #include "entity_soft_attribs.h"
  #undef MAC
};


/*** character soft attribute interface ***/
class character_soft_attrib_interface : public soft_attrib_interface
{
  private:
    /*** soft attribute members ***/
    #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) code_name##_attrib code_name;
    #include "character_soft_attribs.h"
    #undef MAC

  public:
    /*** constructors/destructors ***/
    character_soft_attrib_interface(entity *_my_entity)
      : soft_attrib_interface(_my_entity)
    {
    }
    virtual ~character_soft_attrib_interface() {}

    /*** get_soft_attrib_num ***/
    virtual const rational_t get_soft_attrib_num(pstring &by_name) const 
    {
      #define PROCESS_NUMBERS_ONLY
      #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) \
        if (by_name == code_name##.get_name()) { return (rational_t)##code_name##.get(); }

      #include "entity_soft_attribs.h"
      #include "character_soft_attribs.h"
      #undef MAC
      #undef PROCESS_NUMBERS_ONLY
      assert("Could not get a character soft attribute number by name." == 0);
      return 0.0f;
    }

    /*** get_soft_attrib_str ***/
    virtual const pstring get_soft_attrib_str(pstring &by_name) const 
    {
      #define PROCESS_STRINGS_ONLY
      #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) \
        if (by_name == code_name##.get_name()) { return code_name##.get(); }

      #include "entity_soft_attribs.h"
      #include "character_soft_attribs.h"
      #undef MAC
      #undef PROCESS_STRINGS_ONLY
      assert("Could not get a character soft attribute string by name." == 0);
      return 0;
    }

  /*** get attrib methods ***/
  #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) itype get_##code_name##() { return(##code_name##); }
  #include "character_soft_attribs.h"
  #undef MAC

  /*** set attrib methods ***/
  #define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) void set_##code_name##(const itype &value) { code_name.set(value); }
  #include "character_soft_attribs.h"
  #undef MAC
};


#endif // SOFT_ATTRIBUTE_INTERFACE_CLASS_HEADER
