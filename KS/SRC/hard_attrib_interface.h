#ifndef HARD_ATTRIBUTE_INTERFACE_CLASS_HEADER
#define HARD_ATTRIBUTE_INTERFACE_CLASS_HEADER

#include "global.h"
#include "entity_interface.h"
#include "attribute_template.h"
#include "pstring.h"

extern bool g_frame_advance_called_this_game; // false when we are loading a level, true when playing a level

class entity;

/*** generate the attribute classes ***/
#define MAC(itype, itype_name, code_name, str_name, default_val)    \
    class code_name##_attrib : public basic_attribute<##itype##>    \
    {                                                               \
      protected:                                                    \
        static pstring attribute_name;                              \
                                                                    \
      public:                                                       \
        code_name##_attrib()                                        \
          : basic_attribute<##itype##>(itype_name, default_val)     \
        {                                                           \
        }                                                           \
        const pstring &get_name() const { return attribute_name; }  \
    };

#include "entity_hard_attribs.h"
#include "character_hard_attribs.h"

#undef MAC


/*** hard attribute interface ***/
class hard_attrib_interface : public entity_interface
{
  protected:
    /*** hard attribute members ***/
    #define MAC(itype, itype_name, code_name, str_name, default_val) code_name##_attrib code_name;
    #include "entity_hard_attribs.h"
    #undef MAC

  public:
    /*** constructors/destructors ***/
    hard_attrib_interface(entity *_my_entity)
      : entity_interface(_my_entity)
    {
    }
    virtual ~hard_attrib_interface() {}

    virtual void copy(hard_attrib_interface *b);

    /*** get_hard_attrib_num ***/
    virtual const rational_t get_hard_attrib_num(pstring &by_name) const 
    {
      #define PROCESS_NUMBERS_ONLY
      #define MAC(itype, itype_name, code_name, str_name, default_val) \
        if (by_name == code_name##.get_name()) { return (rational_t)##code_name##.get(); }

      #include "entity_hard_attribs.h"
      #undef MAC
      #undef PROCESS_NUMBERS_ONLY
      assert("Could not get a hard attribute number by name." == 0);
      return 0.0f;
    }

    /*** get_hard_attrib_str ***/
    virtual const pstring get_hard_attrib_str(pstring &by_name) const 
    {
      #define PROCESS_STRINGS_ONLY
      #define MAC(itype, itype_name, code_name, str_name, default_val) \
        if (by_name == code_name##.get_name()) { return code_name##.get(); }

      #include "entity_hard_attribs.h"
      #undef MAC
      #undef PROCESS_STRINGS_ONLY
      assert("Could not get a hard attribute string by name." == 0);
      return 0;
    }

  /*** get attrib methods ***/
  #define MAC(itype, itype_name, code_name, str_name, default_val) itype get_##code_name##() { return(##code_name##); }
  #include "entity_hard_attribs.h"
  #undef MAC

  /*** set attrib methods ***/
  #define MAC(itype, itype_name, code_name, str_name, default_val) void set_##code_name##(const itype &value) { code_name##.set(value); }
  #include "entity_hard_attribs.h"
  #undef MAC
};


/*** character hard attribute interface ***/
class character_hard_attrib_interface : public hard_attrib_interface
{
  private:
    /*** hard attribute members ***/
    #define MAC(itype, itype_name, code_name, str_name, default_val) code_name##_attrib code_name;
    #include "character_hard_attribs.h"
    #undef MAC

  public:
    /*** constructors/destructors ***/
    character_hard_attrib_interface(entity *_my_entity)
      : hard_attrib_interface(_my_entity)
    {
    }
    virtual ~character_hard_attrib_interface() {}

    /*** get_hard_attrib_num ***/
    virtual const rational_t get_hard_attrib_num(pstring &by_name) const 
    {
      #define PROCESS_NUMBERS_ONLY
      #define MAC(itype, itype_name, code_name, str_name, default_val) \
        if (by_name == code_name##.get_name()) { return (rational_t)##code_name##.get(); }

      #include "entity_hard_attribs.h"
      #include "character_hard_attribs.h"
      #undef MAC
      #undef PROCESS_NUMBERS_ONLY
      assert("Could not get a character hard attribute number by name." == 0);
      return 0.0f;
    }

    /*** get_hard_attrib_str ***/
    virtual const pstring get_hard_attrib_str(pstring &by_name) const 
    {
      #define PROCESS_STRINGS_ONLY
      #define MAC(itype, itype_name, code_name, str_name, default_val) \
        if (by_name == code_name##.get_name()) { return code_name##.get(); }

      #include "entity_hard_attribs.h"
      #include "character_hard_attribs.h"
      #undef MAC
      #undef PROCESS_STRINGS_ONLY
      assert("Could not get a character hard attribute string by name." == 0);
      return 0;
    }
    
  /*** get attrib methods ***/
  #define MAC(itype, itype_name, code_name, str_name, default_val) itype get_##code_name##() { return(##code_name##); }
  #include "character_hard_attribs.h"
  #undef MAC

  /*** set attrib methods ***/
  #define MAC(itype, itype_name, code_name, str_name, default_val) void set_##code_name##(const itype &value) \
    {                                                     \
      assert (g_frame_advance_called_this_game == false);   \
      code_name.set(value);                               \
    }
  #include "character_hard_attribs.h"
  #undef MAC
};


#endif // HARD_ATTRIBUTE_INTERFACE_CLASS_HEADER
