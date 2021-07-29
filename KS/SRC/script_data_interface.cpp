#include "global.h"

#include "script_data_interface.h"
#include "pstring.h"

script_data_interface::script_data_interface(entity *ent)
  : entity_interface(ent)
{
  num_1 = 0.0f;
  num_2 = 0.0f;
  num_3 = 0.0f;
  num_4 = 0.0f;
  num_5 = 0.0f;
  num_6 = 0.0f;
  num_7 = 0.0f;
  num_8 = 0.0f;

  str_1 = empty_string;
  str_2 = empty_string;
  str_3 = empty_string;
  str_4 = empty_string;

  vec_1 = ZEROVEC;
  vec_2 = ZEROVEC;
}

script_data_interface::~script_data_interface()
{
}


bool script_data_interface::get_ifc_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("SD_NUM_1", num_1);
  IFC_INTERNAL_GET_MACRO("SD_NUM_2", num_2);
  IFC_INTERNAL_GET_MACRO("SD_NUM_3", num_3);
  IFC_INTERNAL_GET_MACRO("SD_NUM_4", num_4);
  IFC_INTERNAL_GET_MACRO("SD_NUM_5", num_5);
  IFC_INTERNAL_GET_MACRO("SD_NUM_6", num_6);
  IFC_INTERNAL_GET_MACRO("SD_NUM_7", num_7);
  IFC_INTERNAL_GET_MACRO("SD_NUM_8", num_8);

  return(false);
}

bool script_data_interface::set_ifc_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("SD_NUM_1", num_1);
  IFC_INTERNAL_SET_MACRO("SD_NUM_2", num_2);
  IFC_INTERNAL_SET_MACRO("SD_NUM_3", num_3);
  IFC_INTERNAL_SET_MACRO("SD_NUM_4", num_4);
  IFC_INTERNAL_SET_MACRO("SD_NUM_5", num_5);
  IFC_INTERNAL_SET_MACRO("SD_NUM_6", num_6);
  IFC_INTERNAL_SET_MACRO("SD_NUM_7", num_7);
  IFC_INTERNAL_SET_MACRO("SD_NUM_8", num_8);

  return(false);
}

bool script_data_interface::get_ifc_str(const pstring &att, stringx &val)
{
  IFC_INTERNAL_GET_MACRO("SD_STR_1", str_1);
  IFC_INTERNAL_GET_MACRO("SD_STR_2", str_2);
  IFC_INTERNAL_GET_MACRO("SD_STR_3", str_3);
  IFC_INTERNAL_GET_MACRO("SD_STR_4", str_4);

  return(false);
}

bool script_data_interface::set_ifc_str(const pstring &att, const stringx &val)
{
  IFC_INTERNAL_SET_MACRO("SD_STR_1", str_1);
  IFC_INTERNAL_SET_MACRO("SD_STR_2", str_2);
  IFC_INTERNAL_SET_MACRO("SD_STR_3", str_3);
  IFC_INTERNAL_SET_MACRO("SD_STR_4", str_4);

  return(false);
}

bool script_data_interface::get_ifc_vec(const pstring &att, vector3d &val)
{
  IFC_INTERNAL_GET_MACRO("SD_VEC_1", vec_1);
  IFC_INTERNAL_GET_MACRO("SD_VEC_2", vec_2);
  
  return(false);
}

bool script_data_interface::set_ifc_vec(const pstring &att, const vector3d &val)
{
  IFC_INTERNAL_SET_MACRO("SD_VEC_1", vec_1);
  IFC_INTERNAL_SET_MACRO("SD_VEC_2", vec_2);

  return(false);
}
