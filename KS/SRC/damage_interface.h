#ifndef _DAMAGE_INTERFACE_H_
#define _DAMAGE_INTERFACE_H_

#include "global.h"
#include "ostimer.h"
#include "entity_interface.h"
#include "attribute_template.h"

typedef enum
{
  DAMAGE_NONE,
  DAMAGE_DIRECT,
  DAMAGE_DIRECT_DIRECTIONAL,
  DAMAGE_MELEE,
  DAMAGE_NONBLOCK_MELEE,
  DAMAGE_GUN,
  DAMAGE_EXPLOSIVE,
  DAMAGE_KNOCKING_DOWN,
  DAMAGE_THROWN
} eDamageType;

enum
{
  _DF_KNOCKDOWN     = 0x00000001,
  _DF_DEATHBLOW     = 0x00000002,
  _DF_PRONEHIT      = 0x00000004,
  _DF_SYNC_WOUNDED  = 0x00000008,
  _DF_SYNC_DISTANCE = 0x00000010,
  _DF_SYNC_ANG      = 0x00000020,
  _DF_EXPLOSIVE     = 0x00000040,
  _DF_RHAND         = 0x00000080,
  _DF_LHAND         = 0x00000100,
  _DF_RFOOT         = 0x00000200,
  _DF_LFOOT         = 0x00000400,
	_DF_KNOCKDOWNFRONT = 0x00000800
};


class item;

class damage_info
{
public:
  int damage;
  vector3d loc;
  vector3d dir;

  eDamageType type;
  int flags;

  entity *attacker;
  item *attacker_itm;
  stringx wounded_anim;

  bool push_wounded;
  bool push_death;

  damage_info()
  {
    wounded_anim = empty_string;
  }

  void copy(const damage_info &b)
  {
    damage = b.damage;
    loc = b.loc;
    dir = b.dir;

    type = b.type;
    flags = b.flags;

    attacker = b.attacker;
    attacker_itm = b.attacker_itm;
    push_wounded = b.push_wounded;
    push_death = b.push_death;
    wounded_anim = b.wounded_anim;
  }

  damage_info(const damage_info &b)
  {
    copy(b);
  }

  damage_info& operator=(const damage_info &b) 
  {
		copy( b );
    return *this;
  }
};

class damage_interface : public entity_interface
{
protected:
  bounded_attribute<int> hit_points;
  bounded_attribute<int> armor_points;

  short flags;

  rational_t destroy_lifetime;
  stringx destroy_sound;
  stringx destroy_fx;
  stringx destroy_script;

//  int dread_net_cue;

  damage_info dmg_info;
  damage_info last_dmg_info;
  rational_t damage_mod;

public:
  enum eDamageInterfaceFlags
  {
    _DESTROY_FX         = 0x0002,
    _DESTROY_SCRIPT     = 0x0004,
    _DESTROY_SOUND      = 0x0008,
    _SINGLE_BLOW        = 0x0020,
    _REMAIN_VISIBLE     = 0x0040,
    _REMAIN_ACTIVE      = 0x0080,
    _NO_COLLISION       = 0x0400,
    _REMAIN_COLLISION   = 0x0800,

    _DAMAGED_LAST_FRAME   = 0x1000,
    _DAMAGED_THIS_FRAME   = 0x2000
  };

  damage_interface(entity *ent);
  virtual ~damage_interface();

  void copy(damage_interface *b);

  bool is_flagged(eDamageInterfaceFlags f) const          { return (flags & f) != 0; }
  void set_flag(eDamageInterfaceFlags f, bool val = true) { if(val) flags |= f; else flags &= ~f; }


  void set_hit_points(int val)                  { hit_points.set(val); }
  int get_hit_points()                          { return(hit_points.get()); }
  void set_max_hit_points(int max)              { hit_points.set_max(max);  if(hit_points > hit_points.get_max()) hit_points.set(hit_points.get_max()); }
  int get_max_hit_points()                      { return(hit_points.get_max()); }
                                                
  void set_armor_points(int val)                { armor_points.set(val); }
  int get_armor_points()                        { return(armor_points.get()); }
  void set_max_armor_points(int max)            { armor_points.set_max(max); if(armor_points > armor_points.get_max()) armor_points.set(armor_points.get_max()); }
  int get_max_armor_points()                    { return(armor_points.get_max()); }
                                                
  void inc_hit_points(int val)                  { hit_points += val; }
  void dec_hit_points(int val)                  { hit_points -= val; }
  void inc_armor_points(int val)                { armor_points += val; }
  void dec_armor_points(int val)                { armor_points -= val; }

  void set_damage_mod(rational_t val)           { damage_mod = val; }
  rational_t get_damage_mod()                   { return(damage_mod); }

  bool has_destroy_fx()        const            { return is_flagged(_DESTROY_FX); }
  bool has_destroy_script()    const            { return is_flagged(_DESTROY_SCRIPT); }
  bool has_destroy_sound()     const            { return is_flagged(_DESTROY_SOUND); }
  bool is_single_blow()        const            { return is_flagged(_SINGLE_BLOW); }
  bool remain_visible()        const            { return is_flagged(_REMAIN_VISIBLE); }
  bool no_collision()          const            { return is_flagged(_NO_COLLISION); }
  bool remain_collision()      const            { return is_flagged(_REMAIN_COLLISION); }
  bool remain_active()         const            { return is_flagged(_REMAIN_ACTIVE); }
  bool was_damaged()           const            { return is_flagged(_DAMAGED_LAST_FRAME); }

  rational_t get_destroy_lifetime() const       { return destroy_lifetime; }
  const stringx& get_destroy_sound() const      { return destroy_sound; }
  const stringx& get_destroy_fx() const         { return destroy_fx; }
  const stringx& get_destroy_script() const     { return destroy_script; }

  void set_single_blow(bool val)                { set_flag(_SINGLE_BLOW, val); }
  void set_remain_visible(bool val)             { set_flag(_REMAIN_VISIBLE, val); }
  void set_no_collision(bool val)               { set_flag(_NO_COLLISION, val); }
  void set_remain_collision(bool val)           { set_flag(_REMAIN_COLLISION, val); }
  void set_remain_active(bool val)              { set_flag(_REMAIN_ACTIVE, val); }
  void set_destroy_lifetime(rational_t val)     { destroy_lifetime = val; }
  void set_has_destroy_sound(bool val)          { set_flag(_DESTROY_SOUND, val); }
  void set_has_destroy_fx(bool val)             { set_flag(_DESTROY_FX, val); }
  void set_has_destroy_script(bool val)         { set_flag(_DESTROY_SCRIPT, val); }
  void set_destroy_sound(const stringx& val)    { if(val.length() > 0) { set_flag(_DESTROY_SOUND, true); destroy_sound = val; } else set_flag(_DESTROY_SOUND, false); }
  void set_destroy_fx(const stringx& val)       { if(val.length() > 0) { set_flag(_DESTROY_FX, true); destroy_fx = val; } else set_flag(_DESTROY_FX, false); }
  void set_destroy_script(const stringx& val)   { if(val.length() > 0) { set_flag(_DESTROY_SCRIPT, true); destroy_script = val; } else set_flag(_DESTROY_SCRIPT, false); }
  
  bool is_alive() const                         { return(hit_points.get() > 0); }

  eDamageType get_damage_type()                 { return(dmg_info.type); }
  int get_damage_amt() const                    { return(dmg_info.damage); }
  const vector3d& get_damage_loc() const        { return(dmg_info.loc); }
  const vector3d& get_damage_dir() const        { return(dmg_info.dir); }
  int get_damage_flags() const                  { return(dmg_info.flags); }
  entity* get_damage_attacker() const           { return(dmg_info.attacker); }
  item* get_damage_item() const                 { return(dmg_info.attacker_itm); }
  bool get_damage_push_wounded() const          { return(dmg_info.push_wounded); }
  bool get_damage_push_death() const            { return(dmg_info.push_death); }

  damage_info get_dmg_info() const              { return(dmg_info); }
  damage_info get_last_dmg_info() const         { return(last_dmg_info); }

  void set_damage_push_wounded(bool p)          { dmg_info.push_wounded = p; }
  void set_damage_push_death(bool p)            { dmg_info.push_death = p; }
  void set_damage_amt(int v)                    { dmg_info.damage = v; }

  int apply_damage(entity *attacker, int damage, eDamageType type, const vector3d &pos, const vector3d &dir, int flags = 0, const stringx &wounded_anim = empty_string);
  void apply_destruction_fx();
  void read_enx_data( chunk_file& fs, stringx& lstr );

  virtual void frame_advance(time_value_t t);

  virtual bool get_ifc_num(const pstring &att, rational_t &val);
  virtual bool set_ifc_num(const pstring &att, rational_t val);
};

#endif