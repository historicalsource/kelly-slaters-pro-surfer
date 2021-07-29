#ifndef GENERATOR_H
#define GENERATOR_H

#include "algebra.h"
#include "mobject.h"
#include "ostimer.h"

class joint;
class entity;
class character_hard_attributes;


class force_generator : public motion_object
  {
  public:
    force_generator() : motion_object() {};
    virtual ~force_generator() {}

    virtual void frame_advance(time_value_t time_inc)=0;
  };

class gravity_generator: public force_generator
  {
  public:
    gravity_generator()
      {
      set_active(true);
      }
    virtual ~gravity_generator() {}

    virtual void frame_advance(time_value_t time_inc);
  protected:
  };

class angular_force_generator: public force_generator
  {
  public:
    angular_force_generator() : force_generator() {};
    virtual ~angular_force_generator() {}

    virtual void frame_advance(time_value_t time_inc)=0;
  private:
    joint * j;
  };

class linear_force_generator: public force_generator
  {
  public:
    linear_force_generator(entity * _ent,vector3d _dir);
    virtual ~linear_force_generator() {}

    virtual void frame_advance(time_value_t time_inc)=0;
    entity * get_entity() const {return ent;}

    void set_dir(vector3d new_dir) {dir = new_dir;}
  protected:
    vector3d dir;
    entity * ent;
  };


class launcher: public linear_force_generator
  {
  public:
    launcher( entity* _ent, const vector3d& _dir, character_hard_attributes* _att, rational_t (*_att_get_launch_height)(character_hard_attributes*) ):
        linear_force_generator(_ent,_dir)
      {
      att_get_launch_height = _att_get_launch_height;
      att = _att;
      doit = false;
      }
    virtual ~launcher() {}

    virtual void frame_advance(time_value_t time_inc);

    void fcs_set_doit(bool go)
      {
      doit = go;
      }
  protected:
    bool doit;
    character_hard_attributes * att;

    rational_t (*att_get_launch_height)(character_hard_attributes *);
  };


#endif