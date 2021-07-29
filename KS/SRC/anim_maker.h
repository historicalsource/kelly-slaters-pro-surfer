#ifndef ANIM_MAKER_H
#define ANIM_MAKER_H

#include "ostimer.h"

class entity;
class entity_anim_tree;
class entity_track_tree;
class entity_widget;

// this class is a kludge required to allow anims outside the world;
// a more elegant solution can come whenever we redesign the engine

class anim_maker
{
public:
  anim_maker();
  virtual ~anim_maker();

  entity_anim_tree* create_anim( const stringx& filename,
                              entity* ent,
                              unsigned short flags = 0,
                              time_value_t start_time = 0,
                              int priority = 0,
                              short loop = -1,
                              entity_widget *owning_widget = NULL );

  entity_anim_tree* create_anim( const stringx& name,
                                 const entity_track_tree& track,
                                 entity* ent,
                                 unsigned short flags = 0,
                                 time_value_t start_time = 0,
                                 int priority = 0,
                                 short loop = -1,
                                 entity_widget *owning_widget = NULL );

  void create_anim( entity_anim_tree* cached_anim_tree,
                    const stringx& name,
                    const entity_track_tree& track,
                    unsigned short flags = 0,
                    time_value_t start_time = 0,
                    int priority = 0,
                    short loop = -1,
                    entity_widget *owning_widget = NULL );

  entity_anim_tree* create_anim( const stringx& name,
                                 const entity_track_tree& _tracka,
                                 const entity_track_tree& _trackb,
                                 rational_t blenda, 
                                 rational_t blendb, 
                                 entity* ent,
                                 unsigned short flags = 0,
                                 time_value_t start_time = 0,
                                 int priority = 0,
                                 short loop = -1,
                                 entity_widget *owning_widget = NULL );

  void create_anim( entity_anim_tree* cached_anim_tree,
                    const stringx& name,
                    const entity_track_tree& _tracka,
                    const entity_track_tree& _trackb,
                    rational_t blenda, 
                    rational_t blendb, 
                    unsigned short flags = 0,
                    time_value_t start_time = 0,
                    int priority = 0,
                    short loop = -1,
                    entity_widget *owning_widget = NULL );

private:
};



extern anim_maker *g_anim_maker;


#endif // ANIM_MAKER_H