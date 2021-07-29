#ifndef _HANDHELD_ITEM_H_
#define _HANDHELD_ITEM_H_

#include "item.h"
#include "simple_classes.h"
#include "matfac.h"

//!class character;

// low 2 nibbles for handheld items
// rest for subclasses
enum
{
  _HANDHELD_DRAWN               = 0x00000001,
  _HANDHELD_BRAIN_WEAPON        = 0x00000002,
  _HANDHELD_COMMON_BRAIN_WEAPON = 0x00000004,
  _HANDHELD_RADIO_DETONATOR     = 0x00000008,

  _HANDHELD_FLAGS_MASK          = 0x000000FF,

  _HANDHELD_SUBCLASS_FLAGS_MASK = 0xFFFFFF00
};




class handheld_item;
//class sound_instance;

class handheld_item_effect
{
public:
  rational_t lifetime;
  stringx static_fx;
  stringx script;
  pstring sound;

  sound_instance* loop_sound;
  entity *fx;

  handheld_item_effect()
  {
    lifetime = 1.0f;
    static_fx = empty_string;
    script = empty_string;

    loop_sound = NULL;
    fx = NULL;
  }

  ~handheld_item_effect()
  {
//      kill();
  }

  handheld_item_effect &operator=(const handheld_item_effect &b)
  {
    lifetime = b.lifetime;
    static_fx = b.static_fx;
    script = b.script;
    sound = b.sound;

    return *this;
  }

  handheld_item_effect(const handheld_item_effect &b)
  {
    loop_sound = NULL;
    fx = NULL;

    *this = b;
  }

  void set_script(const stringx &s)
  {
    script = s + stringx("(item,entity,vector3d,vector3d)");
  }

  inline bool has_script() const { return(!script.empty()); }
  inline bool has_static() const { return(!static_fx.empty()); }

  void spawn(bool remain, const vector3d &pos, const vector3d &face, handheld_item *owner, entity *script_ent = NULL, entity *attach_ent = NULL, const vector3d &offset = ZEROVEC);
  void update(entity *owner);
  void kill();
};



struct ai_item_info
{
protected:
  friend class handheld_item;

  rational_t min_dist;
  rational_t max_dist;
  int flags;

public:
  enum
  {
    _RIFLE        = 0x00000001,
    _SHOTGUN      = 0x00000002,
    _PISTOL       = 0x00000004,
    _LAUNCHER     = 0x00000008,
    _HAND_GRENADE = 0x00000010,
    _AUTOMATIC    = 0x00000020,
    _NEAR_RANGE   = 0x00000040,
    _MID_RANGE    = 0x00000080,
    _FAR_RANGE    = 0x00000100,
    _DUAL_WEAPON  = 0x00000200,
    _PUMP_ACTION  = 0x00000400
  };

  ai_item_info(rational_t min = 0.0f, rational_t max = 30.0f, int f = (_MID_RANGE|_PISTOL))
  {
    min_dist = min;
    max_dist = max;
    if(max_dist < min_dist)
    {
      rational_t b = min_dist;
      min_dist = max_dist;
      max_dist = b;
    }

    flags = f;
  }

  ai_item_info(const ai_item_info &b)
  {
    *this = b;
  }

  inline ai_item_info &operator=(const ai_item_info &b)
  {
    min_dist = b.min_dist;
    max_dist = b.max_dist;
    flags = b.flags;

    return(*this);
  }

  inline rational_t get_min_dist() const                            { return(min_dist); }
  inline rational_t get_max_dist() const                            { return(max_dist); }

  inline bool is_flagged(register unsigned int f) const             { return(flags & f); }
  inline void set_flag(register unsigned int f, register bool set)  { if(set) flags|=f; else flags&=~f; }

  inline bool is_rifle() const                                      { return(flags & _RIFLE); }
  inline bool is_shotgun() const                                    { return(flags & _SHOTGUN); }
  inline bool is_pistol() const                                     { return(flags & _PISTOL); }
  inline bool is_launcher() const                                   { return(flags & _LAUNCHER); }
  inline bool is_hand_grenade() const                               { return(flags & _HAND_GRENADE); }
  inline bool is_automatic() const                                  { return(flags & _AUTOMATIC); }
  inline bool allow_dual() const                                    { return(flags & _DUAL_WEAPON); }
  inline bool is_pump_action() const                                { return(flags & _PUMP_ACTION); }

  inline bool in_range(const vector3d &from, const vector3d &to) const
  {
    rational_t len = (to - from).length2();
    return(len >= (min_dist*min_dist) && len <= (max_dist*max_dist));
  }

  inline bool in_range(rational_t range) const
  {
    return(range <= max_dist && range >= min_dist);
  }

  inline bool too_close(rational_t range) const
  {
    return(range < min_dist);
  }

  inline bool too_far(rational_t range) const
  {
    return(range > max_dist);
  }

  inline rational_t optimal_range() const
  {
    if(flags & _NEAR_RANGE)
      return(min_dist + ((max_dist - min_dist) * 0.15f));
    else if(flags & _FAR_RANGE)
      return(min_dist + ((max_dist - min_dist) * 0.85f));
    else
      return(min_dist + ((max_dist - min_dist) * 0.5f));
  }
};



class handheld_item : public item
{
protected:
/*
  typedef enum
  {
    _X,
    _Y,
    _Z,
    _NEGX,
    _NEGY,
    _NEGZ
  } eAxis;
*/
  visual_item *vis_item;
//  eAxis vis_axis;
  entity *owner;

  stringx drawn_limb_name;
  stringx holster_limb_name;
  po drawn_offset;
  po holster_offset;

  stringx item_id;

  unsigned int handheld_flags;

  bool visible_on_character;

  virtual void init_defaults();

  virtual void copy_instance_data( handheld_item& b );

  void read_animation_chunk( chunk_file& fs );

  ai_item_info ai_info;

  vm_thread *exec_item_script_function( const stringx& function_name, const vector3d &pos, const vector3d &norm );
  friend class grenade;
  friend class rocket;

public:
  handheld_item( const entity_id& _id, unsigned int _flags );

  handheld_item( const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

  virtual ~handheld_item();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_handheld_item() const { return true; }

// NEWENT File I/O
public:
  handheld_item( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );


  virtual bool handle_enx_chunk( chunk_file& fs, stringx& lstr );

  // Old File I/O
  handheld_item( const stringx& item_type_filename,
        const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_PHYSICAL,
        bool _active = INACTIVE,
        bool _stationary = false );

  // Instancing
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  void read_orientation_chunk( chunk_file& fs );
  void read_ai_info_chunk( chunk_file& fs );

  void set_handheld_flag(unsigned int flag, bool on = true)
  {
    if(on)
      handheld_flags |= flag;
    else
      handheld_flags &= (~flag);
  }

  bool get_handheld_flag(unsigned int flag)  const
  {
    return (handheld_flags & flag)!=0;
  }

  virtual void frame_advance(time_value_t t);

  virtual bool is_brain_weapon() const;
  virtual bool is_common_brain_weapon() const;
  virtual bool is_a_radio_detonator() const;

  virtual visual_item* get_visual_item() const;
  virtual entity *get_owner() const;
  virtual void set_owner(entity *own);

  virtual void create_visual_item();
  virtual void holster(bool make_visible = true);
  virtual void draw(bool make_visible = true);
  virtual void hide();
  virtual void show();

  vector3d get_vis_facing();

  virtual void set_visibility(bool val);

  virtual void detach();

  const po &get_drawn_offset() const      { return(drawn_offset); }
  const po &get_holster_offset() const    { return(holster_offset); }
  void set_drawn_offset(const po &off)    { drawn_offset = off; }
  void set_holster_offset(const po &off)  { holster_offset = off; }

  const stringx &get_drawn_limb_name() const      { return(drawn_limb_name); }
  const stringx &get_holster_limb_name() const    { return(holster_limb_name); }
  void set_drawn_limb_name(const stringx &limb)   { drawn_limb_name = limb; }
  void set_holster_limb_name(const stringx &limb) { holster_limb_name = limb; }

  bool is_drawn() const { return get_handheld_flag( _HANDHELD_DRAWN );}

  const stringx &get_item_id() const { return(item_id); }
  void set_item_id(const stringx &id) { item_id = id; assert(item_id.length() > 0); }

  visual_item *get_vis_item() const { return(vis_item); }

  const ai_item_info &get_ai_info() const                               { return(ai_info); }
  inline rational_t get_min_dist() const                                { return(ai_info.get_min_dist()); }
  inline rational_t get_max_dist() const                                { return(ai_info.get_max_dist()); }
  inline bool is_rifle() const                                          { return(ai_info.is_rifle()); }
  inline bool is_shotgun() const                                        { return(ai_info.is_shotgun()); }
  inline bool is_pistol() const                                         { return(ai_info.is_pistol()); }
  inline bool is_automatic() const                                      { return(ai_info.is_automatic()); }
  inline bool allow_dual() const                                        { return(ai_info.allow_dual()); }
  inline bool is_pump_action() const                                    { return(ai_info.is_pump_action()); }
  inline bool in_range(const vector3d &from, const vector3d &to) const  { return(ai_info.in_range(from, to)); }
  inline bool in_range(rational_t range) const                          { return(ai_info.in_range(range)); }
  inline bool too_close(rational_t range) const                         { return(ai_info.too_close(range)); }
  inline bool too_far(rational_t range) const                           { return(ai_info.too_far(range)); }
  inline rational_t optimal_range() const                               { return(ai_info.optimal_range()); }


#if _VIS_ITEM_DEBUG_HELPER
  rational_t drawn_scale;
  vector3d drawn_rot;
  vector3d drawn_pos;

  rational_t holster_scale;
  vector3d holster_rot;
  vector3d holster_pos;

  bool render_axis;
#endif
};


#endif
