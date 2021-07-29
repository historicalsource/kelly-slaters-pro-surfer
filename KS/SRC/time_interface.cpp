#include "global.h"

#include "time_interface.h"
#include "pstring.h"

rational_t g_time_dilation = 1.0f;

bool time_interface::get_ifc_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("TIME_DILATION", time_dilation);
  IFC_INTERNAL_GET_MACRO("TIME_MODE", time_mode);

  return(false);
}

bool time_interface::set_ifc_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("TIME_DILATION", time_dilation);

  static pstring check_mode("TIME_MODE");
  if(att == check_mode)
  {
    set_mode((eTimeMode)((int)val));
    return(true);
  }



  return(false);
}
