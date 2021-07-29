#include "global.h"

#include "soft_attrib_interface.h"

#define MAC(atype, itype, itype_name, code_name, str_name, default_val, min_val, max_val) \
    pstring code_name##_attrib::attribute_name = str_name##;

#include "entity_soft_attribs.h"
#include "character_soft_attribs.h"
#undef MAC