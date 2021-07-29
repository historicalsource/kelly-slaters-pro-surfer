
#ifndef _TRIGGER_H
#define _TRIGGER_H

#include "signals.h"
#include "region_graph.h"

class entity;
class sector;
class region;
class portal;
//typedef set<region *> trig_region_pset;
//typedef graph<stringx,region*,portal*> region_graph;
//typedef region_graph::node region_node;

///////////// trigger base class /////////////
class trigger : public signaller
{
	friend class trigger_manager;

public:
	trigger( const stringx& _id );

  virtual void read(chunk_file &fs) {}
	virtual bool triggered(entity *) { return false; }

	void update();
	virtual void update_region() {}

	void force_region( stringx id );
	bool add_region( region* r );

  void set_use_any_char(bool u) { use_any_char = u; }

  virtual bool is_a_trigger() const { return(true); }

  virtual const vector3d& get_abs_position() const { return ZEROVEC; }

  bool is_active() const { return active; }
  void set_active( bool torf );

  entity *get_triggered_ent() const { return(whodunnit); }

	//// Signals ////
  enum signal_id_t
  {
    PARENT_SYNC_DUMMY = signaller::N_SIGNALS - 1,
    #define MAC(label,str)  label,
    #include "trigger_signals.h"
    #undef MAC
    N_SIGNALS
  };

  static void register_signals();

private:
  virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

	//// end of signals ////

protected:
  entity *whodunnit;
	stringx id;
	trigger *next;
	trig_region_pset in_regions;
	bool static_regions;
	bool active;
  bool occupied;
  bool use_any_char;
};

///////////// trigger manager /////////////
class trigger_manager : public singleton
{
public:
	DECLARE_SINGLETON(trigger_manager)
	trigger_manager() { list = NULL; }

	trigger *new_trigger(stringx id, stringx type, chunk_file &fs);
	trigger *find_instance(const stringx &id);

  // create a point-radius trigger
  trigger* new_point_trigger( vector3d p, rational_t r );
  // create an entity-radius trigger
  trigger* new_entity_trigger( entity* e, rational_t r );
  // create a convex box trigger
  trigger* new_box_trigger( entity* e );

	void update();
	void update_regions();
  void purge();

public:
  vector<region*> new_regions;  // permanent

protected:
  void add( trigger* t );
  void remove( trigger* t );

	trigger *list;
};

///////////// trigger within XZ cone /////////////
class point_trigger : public trigger
{
public:
	point_trigger( const stringx& _id );
  point_trigger( const stringx& _id, const vector3d& p, rational_t r );
	virtual void read(chunk_file &fs);
	virtual bool triggered(entity *e);
	virtual void update_region();

  virtual const vector3d& get_abs_position() const;

// INTERNAL
private:
  // add trigger to given region and recurse into any adjacent intersected regions
  void _intersect( region_node* r );
  void _update_regions();

protected:
	vector3d position;
	float radius;
};

///////////// trigger within convex box /////////////
#ifdef ECULL
class box_trigger : public trigger
{
public:
	box_trigger( const stringx& _id );
  box_trigger( const stringx& _id, entity* _box );
	virtual void read(chunk_file &fs);
	virtual bool triggered(entity *e);
	virtual void update_region();

  virtual const vector3d& get_abs_position() const;

// INTERNAL
private:
  // add trigger to given region and recurse into any adjacent intersected regions
  void _intersect( region_node* r );
  void _update_regions();

protected:
	entity* box;
};
#endif
///////////// trigger within region  /////////////
class region;

class region_trigger : public trigger
{
public:
	region_trigger( const stringx& _id );
	virtual void read(chunk_file &fs);
	virtual bool triggered(entity *e);
	virtual void update_region();
};

///////////// trigger via proximity to an entity /////////////

class entity_trigger : public trigger
{
public:
	entity_trigger( const stringx& _id );
  entity_trigger( const stringx& _id, entity* p, rational_t r );
	virtual void read(chunk_file &fs);
	virtual bool triggered(entity *e);
	virtual void update_region();

  virtual const vector3d& get_abs_position() const;

// INTERNAL
private:
  // add trigger to given region and recurse into any adjacent intersected regions
  void _intersect( region_node* r );
  void _update_regions();

protected:
	entity *ent;
	float radius;
  rational_t last_compute_sector_position_hash;
};

#endif

