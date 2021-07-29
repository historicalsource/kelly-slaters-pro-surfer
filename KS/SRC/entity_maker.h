#ifndef ENTITY_MAKER_H
#define ENTITY_MAKER_H

#include "global.h"
#include "entity.h"
#include "path.h"
#include "region_graph.h"

class conglomerate;
class entity_widget;


// support for entity caching (a system used mostly just during gameplay to avoid excessive memory allocation)

class entity_pool
{
public:
  typedef list<entity*> entity_list;
private:
  entity_list entities;
  unsigned int avail;
public:
  entity_pool();
  ~entity_pool();
  int size() const { return entities.size(); }
  void add( entity* e );
  entity* acquire( unsigned int flags );
  void release( entity* e );
};

class entity_pool_set
{
public:
  typedef map<stringx,entity_pool*> set_t;
  typedef list<entity_pool*> aux_t;
private:
  set_t entity_pools;
  aux_t aux_entity_pools;  // entity pools for conglomerate members (and other non-searchable pools)
public:
  entity_pool_set();
  ~entity_pool_set();
  entity* acquire( const stringx& name, unsigned int flags );
  entity* acquire_beam( unsigned int flags );
  void purge();
};


// entity_maker is a partial attempt to rationalize the system for entity creation;
// much of the system is still buried in wds, and will need to be cleaned up in
// the future, perhaps on another project if this engine gets used again

//typedef list< region_node*
//	,malloc_alloc
//  > region_node_list;



class entity_maker
{
public:
  entity_maker();
  virtual ~entity_maker();

  stringx open( chunk_file& fs, const stringx& filename, const stringx& extension, int io_flags );

  entity* create_entity_or_subclass( const stringx& entity_name,
                   entity_id id,
                   po const & loc,
                   const stringx& scene_root,
                   unsigned int scene_flags = entity::ACTIVE,
                   const region_node_list *forced_regions = NULL );

  entity* create_entity( chunk_file& fs,
                      const entity_id& id,
                      unsigned int flags,
                      bool add_bones = true );
  void create_entity( entity* e );

  // remove and delete entity if owned by world
  void destroy_entity( entity* e );

  // this function should only be called when entity_widget is being
  // created, and then twice, once to set the owning widget before
  // entity creation, and once to clear the owning widget after entity
  // creation (so that further entities created are added correctly to
  // the world).
  // owning_widget specifies the entity_widget that will be responsible
  // for updating and rendering this entity.
  // if NULL, the entity will be added to the world as is customary.
  void set_owning_widget( entity_widget *_owning_widget ) { owning_widget = _owning_widget; }
  const entity_widget* get_owning_widget() const { return owning_widget; }

private:
  void read_meshes( chunk_file& fs ); // puts the meshes into the bank
  conglomerate* create_conglomerate( chunk_file& fs,
                                  const entity_id& id,
                                  unsigned int flags );

  void creating_widget_error( const stringx &entity_kind ) const;
  entity_widget *owning_widget; // if NULL, entity will be added to the world

///////////////////////////////////////////////////////////////////////////////
// entity caching interface
private:
  entity_pool_set entity_cache;
public:
  entity* acquire_entity( const stringx& name, unsigned int flags );
  entity* acquire_beam( unsigned int flags );
  void release_entity( entity* e );
  void purge_entity_cache();
};


extern entity_maker *g_entity_maker;


#endif // ENTITY_MAKER_H
