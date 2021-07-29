#ifndef _SCRIPT_DATA_INTERFACE_H_
#define _SCRIPT_DATA_INTERFACE_H_

#include "global.h"
#include "entity_interface.h"

class script_data_interface : public entity_interface
{
protected:
  rational_t num_1;
  rational_t num_2;
  rational_t num_3;
  rational_t num_4;
  rational_t num_5;
  rational_t num_6;
  rational_t num_7;
  rational_t num_8;

  stringx str_1;
  stringx str_2;
  stringx str_3;
  stringx str_4;

  vector3d vec_1;
  vector3d vec_2;

public:
  script_data_interface(entity *ent);
  virtual ~script_data_interface();

  virtual bool get_ifc_num(const pstring &att, rational_t &val);
  virtual bool set_ifc_num(const pstring &att, rational_t val);
  virtual bool get_ifc_str(const pstring &att, stringx &val);
  virtual bool set_ifc_str(const pstring &att, const stringx &val);
  virtual bool get_ifc_vec(const pstring &att, vector3d &val);
  virtual bool set_ifc_vec(const pstring &att, const vector3d &val);
};

#endif