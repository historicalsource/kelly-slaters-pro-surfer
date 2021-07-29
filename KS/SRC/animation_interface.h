#ifndef _ANIMATION_INTERFACE_H_
#define _ANIMATION_INTERFACE_H_

#include "global.h"
#include "entity_interface.h"

class animation_interface : public entity_interface
{
protected:
  unsigned int flags;

public:
  enum
  {
    ANIM_IFC_REVERSE = 0x00000001
  };

  animation_interface(entity *ent);
  virtual ~animation_interface();

  void copy(animation_interface *b);

  bool is_flagged(unsigned int f) const          { return (flags & f) != 0; }
  void set_flag(unsigned int f, bool val = true) { if(val) flags |= f; else flags &= ~f; }

/////////////////////////////////////////////
// animation interface (moved from old brain)
/////////////////////////////////////////////
protected:
  struct s_t_s_t_r
  {
    bool operator()( const stringx &s1, const stringx &s2 ) const { return strcmp(s1.c_str(), s2.c_str()) < 0;}
  };

  struct anim_info
  {
    rational_t  percent;
    stringx     anim;
    int         damage;
    rational_t  recover_time;
    rational_t  recover_var;
    int         flags;
  
    anim_info();
    anim_info( rational_t _per, const stringx &_anim, int _dam, rational_t _rec, rational_t _rec_var, int _flags );
  };

  map<stringx,vector< anim_info >,s_t_s_t_r> anim_info_id_map;

  void add_anim_info_id_map_entry( const stringx& id, rational_t _per, const stringx &_anim, int _dam, rational_t _rec, rational_t _rec_var, int _flags );

  void read_animations_chunk(chunk_file &fs, const stringx &anims_directory);
  void read_anim_sub_chunk(chunk_file &fs, const stringx &anim_id, const stringx &anims_directory);
  void read_animations(chunk_file &f, const stringx& anims_directory);

public:
  typedef map<stringx,vector< anim_info >,s_t_s_t_r>::iterator anim_info_id_map_iterator;
  typedef vector< anim_info >::iterator anim_info_id_map_anims_iterator;
  typedef map<stringx,vector< anim_info >,s_t_s_t_r>::const_iterator anim_info_id_map_const_iterator;
  typedef vector< anim_info >::const_iterator anim_info_id_map_anims_const_iterator;

  const map<stringx,vector< anim_info >,s_t_s_t_r> &get_anim_info_id_map() const { return(anim_info_id_map); }

  void read_anim_info_file( const stringx& anims_filename );

  const stringx& extract_random_anim_info_id_map_anim( const stringx& id, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL );
  const stringx& extract_given_anim_info_id_map_anim( const stringx& id, int idx, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL );
  const stringx& extract_given_anim_info_id_map_anim_by_number( const stringx& id, int idx, int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL );
};

#endif