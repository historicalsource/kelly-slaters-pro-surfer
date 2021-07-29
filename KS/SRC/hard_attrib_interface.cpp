#include "global.h"

#include "hard_attrib_interface.h"

#define MAC(itype, itype_name, code_name, str_name, default_val)    \
  pstring code_name##_attrib::attribute_name = str_name##; 

#include "entity_hard_attribs.h"
#include "character_hard_attribs.h"
#undef MAC


void hard_attrib_interface::copy(hard_attrib_interface *b)
{
  #define MAC(itype, itype_name, code_name, str_name, default_val) code_name = b->##code_name##;
  #include "entity_hard_attribs.h"
  #undef MAC
}
