#include "global.h"

#include "script_library_class.h"

#define MAC(a) extern void register_##a##_lib();
#include "script_register.h"
#undef MAC

void register_script_libs()
{
  slc_global = NEW script_library_class("_global_slc",0);
  slc_num = NEW slc_num_t("num",4);
  slc_str = NEW slc_str_t("str",4);

#define MAC(a) register_##a##_lib();
#include "script_register.h"
#undef MAC
}
