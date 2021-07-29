#ifndef MOBJECT_H
#define MOBJECT_H

class motion_object
{
public:
  motion_object()
  {
    active = true;
    locked = false;
  }
  virtual ~motion_object() {}

  virtual bool is_active() const { return active; }
  virtual void set_active(bool yorn)
  {
    active = yorn;
  }

  void set_locked( bool _locked) { locked = _locked; }
  bool is_locked() const { return locked; }
protected:
  bool active;
  bool locked;
};

#endif
