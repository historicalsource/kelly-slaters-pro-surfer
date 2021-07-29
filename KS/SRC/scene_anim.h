#ifndef _SCENE_ANIM_H
#define _SCENE_ANIM_H

#include "entity_anim.h"

typedef unsigned int scene_anim_handle_t;  // scene animation handle

class scene_anims_info
  {
  public:
    entity* ent;
    vector3d entity_up_vec;
    scene_anim_handle_t handle;
    stringx name;
	//BETH
	entity_anim_tree* anim_tree;
  };

typedef vector<scene_anims_info> scene_anim_list_t;

class scene_anim
  {
  // Types
  public:
    typedef map<stringx,entity_track_tree*> track_tree_list_t;

  // Data
  private:
    track_tree_list_t track_tree_map;

  // Methods
  public:
    scene_anim() {};
    ~scene_anim();

    void load( const stringx& filename );
    void play( scene_anim_list_t &animlist, scene_anim_handle_t handle, bool reverse=false, float start_time = 0.0f );
    void kill();

  private:
    void text_load( const stringx& filename );
    void binary_load( const stringx& filename );
    void binary_load_from_os( const stringx& filename );
    void binary_load_from_stash( const stringx& filename );
  };

#endif  // _SCENE_ANIM_H