#ifndef VARIANTS_H
#define VARIANTS_H

//#include "actor.h" // no.. it includes us!
#include "entity.h"
//#include "brain_enums.h"


class variant;
//!class actor;


typedef vector<variant*>  variant_list;


enum
{
  _BRAIN_ANIM_UNBLOCKABLE = 0x00000001,
  _BRAIN_ANIM_STUN        = 0x00000002,
  _BRAIN_ANIM_KNOCKDOWN   = 0x00000004
};

class attachment_info
{
public:
  enum
  {
    USE_CHARACTER_MATERIAL    = 0x00000001,
    USE_CHARACTER_COLOR       = 0x00000002
  };

private:
  entity        *base_copy;
  anim_id_t     parent_id;
  po            rel_po;
  unsigned int  flags;

public:
  attachment_info( entity* _base_copy, anim_id_t _parent_id, const po& _rel_po, const unsigned int _flags = 0 )
  : base_copy( _base_copy ),
    parent_id( _parent_id ),
    rel_po( _rel_po ),
    flags( _flags )
  {
    _base_copy->set_flag(EFLAG_MISC_MEMBER_OF_VARIANT, true);
  }

  entity* get_base_copy() const { return base_copy; }
  anim_id_t get_parent_id() const { return parent_id; }
  const po& get_rel_po() const { return rel_po; }

  void set_flag( unsigned int flg ) { flags |= flg;}
  void clear_flag( unsigned int flg ) { flags &= ~flg;}
  bool is_flagged( unsigned int flg ) const { return flags & flg;}
};

typedef vector<attachment_info*> attachment_list;

class variant_descriptor
{
private:
  stringx         name;
  stringx         char_name;
  stringx         char_material_name;
  color32         char_color;
  variant_list    pool;
  int             pool_size;
  int             pool_capacity;
  eAIState        AIStates[_BRAIN_MAX_REACTION_STATES];
  rational_t      default_reload_timer;
  rational_t      reload_timer_variance;
  bool            use_alt_material, use_alt_color;
  int             full_hit_points;
  attachment_list attachments;
public:
  variant_descriptor( const stringx& _name );
  ~variant_descriptor();

  const eAIState get_ai_state(eReactionState state) const { return AIStates[state]; }
  void set_ai_states(eReactionState react, eAIState ai)   { AIStates[react] = ai; }

  rational_t get_reload_timer() const           { return(default_reload_timer); }
  rational_t get_reload_timer_variance() const  { return(reload_timer_variance); }

  void set_reload_timer(rational_t t = 1.0f)           { default_reload_timer = t; }
  void set_reload_timer_variance(rational_t v = 0.5f)  { reload_timer_variance = v; }

  int get_full_hit_points() const { return full_hit_points; }
  void set_full_hit_points(int hp) { full_hit_points = hp; }

  const stringx& get_name() const { return name; }

  void set_character( const stringx& _name );

  variant* acquire( actor* act );

  bool full() const { return pool_size == pool_capacity; }
  int get_num_free() const { return pool_capacity - pool_size; }

  const color32& get_character_color() const { return char_color;}
  const stringx& get_character_material_name() const { return char_material_name;}
  bool use_alternative_material() const { return use_alt_material;}
  bool use_alternative_color() const { return use_alt_color;}
  void set_use_alternative_material( bool torf ) { use_alt_material = torf;}
  void set_use_alternative_color( bool torf ) { use_alt_color = torf;}

  const attachment_list& get_attachments() const { return attachments; }
  void add_attachment( const stringx& ent_name, anim_id_t parent_id, const po& rel_po, const unsigned int flags = 0 );

  struct s_t_s_t_r
  {
    bool operator()( const stringx s1, const stringx s2 ) const { return strcmp(s1.c_str(), s2.c_str()) < 0;}
  };

  struct brain_anim
  {
    rational_t  percent;
    stringx     anims[_BRAIN_MAX_REACTION_STATES - 1];
    int         damage;
    rational_t  recover_time;
    rational_t  recover_var;
    int         flags;
    
    brain_anim();
    brain_anim( rational_t _per, const stringx _anim[_BRAIN_MAX_REACTION_STATES - 1], int _dam, rational_t _rec, rational_t _rec_var, int _flags );
  };

  map<stringx,vector< brain_anim >,s_t_s_t_r> brain_id_map;
  typedef map<stringx,vector< brain_anim >,s_t_s_t_r>::iterator brain_id_map_iterator;
  typedef vector< brain_anim >::iterator brain_id_map_anims_iterator;

  void add_brain_id_map_entry( const stringx& id, rational_t _per, const stringx _anim[_BRAIN_MAX_REACTION_STATES - 1], int _dam, rational_t _rec, rational_t _rec_var, int _flags );

  void read_animations_chunk(chunk_file &fs, const stringx &anims_directory);
  void read_preloads_chunk(chunk_file &fs, const stringx &anims_directory);
  void read_anim_sub_chunk(chunk_file &fs, const stringx &anim_id, const stringx &anims_directory);
  void read_brain_anim_stuff( const stringx& anims_filename );

  virtual const stringx& extract_random_brain_id_map_anim( stringx& id, eReactionState reaction_level = BRAIN_REACT_IDLE, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL );
  virtual const stringx& extract_given_brain_id_map_anim( stringx& id, int idx, eReactionState reaction_level = BRAIN_REACT_IDLE, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL );
  virtual const stringx& extract_given_brain_id_map_anim_by_number( stringx& id, int idx, eReactionState reaction_level = BRAIN_REACT_IDLE, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL );
};


class variant
{
private:
  variant_descriptor* my_descriptor;
  vector<entity*>     entities;
  bool                locked;
  actor*              my_actor;
public:
  variant( variant_descriptor* dsc );
  ~variant(){}

  const stringx& get_variant_name() const { return my_descriptor->get_name(); }
  const vector<entity*>& get_entities() const { return entities; }

  const variant_descriptor *get_descriptor() const { return my_descriptor; }

  bool acquire( actor* act );
  void release();

  void set_locked( bool torf ) { locked = torf; }
  bool is_locked() const { return locked; }

  void set_visible( bool torf );

  bool use_alternative_material() const { return my_descriptor->use_alternative_material();}
  bool use_alternative_color() const { return my_descriptor->use_alternative_color();}

  const color32& get_my_color() const { return my_descriptor->get_character_color();}
  const stringx& get_my_material_name() const { return my_descriptor->get_character_material_name();}

  const stringx& extract_random_brain_id_map_anim( stringx& id, eReactionState reaction_level = BRAIN_REACT_IDLE, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL ) const
  {
    return my_descriptor->extract_random_brain_id_map_anim( id, reaction_level, damage_value, recover, recover_var, flags );
  }
  const stringx& extract_given_brain_id_map_anim( stringx& id, int idx, eReactionState reaction_level = BRAIN_REACT_IDLE, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL ) const
  {
    return my_descriptor->extract_given_brain_id_map_anim( id, idx, reaction_level, damage_value, recover, recover_var, flags );
  }
  const stringx& extract_given_brain_id_map_anim_by_number( stringx& id, int idx, eReactionState reaction_level = BRAIN_REACT_IDLE, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL ) const
  {
    return my_descriptor->extract_given_brain_id_map_anim_by_number( id, idx, reaction_level, damage_value, recover, recover_var, flags );
  }
};



#endif // VARIANTS_H
