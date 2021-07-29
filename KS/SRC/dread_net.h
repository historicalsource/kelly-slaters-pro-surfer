#if 0

#ifndef _DREAD_NET_H_
#define _DREAD_NET_H_

#include "global.h"

#include "controller.h"
#include "signals.h"

#if _ENABLE_WORLD_EDITOR
  #include <fstream.h>
#endif

class dread_net;
class dread_net_link;
class av_cue;
class brain;
//!class char_group;
//!class character;
class entity;


enum
{
  _DREAD_NET_LINK_ACTIVATION  = 0x01,
  _DREAD_NET_LINK_DAMAGED     = 0x02,
  _DREAD_NET_LINK_DESTRUCTION = 0x04,
  _DREAD_NET_LINK_DETECTION   = 0x08,
  _DREAD_NET_LINK_ALL         = 0xff
};

// Network of evil dudes and machinery
// Master AI, like HAL, SkyNet, BigBrother, etc...
class dread_net : public controller, public signaller
{
private:
  static int num_cues;

public:
/*
  typedef enum
  {
  #define MAC(a, b, c, d, e, f, g, h, i) a,
  #include "av_cues.h"
  #undef MAC
    UNDEFINED_AV_CUE
  } eAVCueType;
*/

typedef int eAVCueType;

enum
{
  UNDEFINED_AV_CUE = -1
};


protected:
  static vector<stringx> av_cue_name;
  static vector<rational_t> av_cue_power;
  static vector<rational_t> av_cue_stealth_mod;
  static vector<rational_t> av_cue_turbo_mod;
  static vector<rational_t> av_cue_radius;
  static vector<bool> av_cue_investigate;
  static vector<bool> av_cue_search;
  static vector<bool> av_cue_run;

  list<dread_net_link *> links;
  vector<region_node *> regions;

  virtual void add_cue_recurse(const av_cue &cue, const vector3d &cur_pos, region_node* rgn, rational_t dist);
  virtual void add_cue_helper(eAVCueType type, const vector3d &pos, region_node* rgn, const entity *ent);

  virtual void add_cue(brain *brn, const av_cue &cue);

public:
  dread_net();
  virtual ~dread_net();

  static eAVCueType get_cue_type(const stringx &cue);

  virtual bool is_dread_net() const { return(true); }

  static stringx& get_cue_name(eAVCueType type);
  inline static rational_t get_cue_power(eAVCueType type)                     { if(type == UNDEFINED_AV_CUE) return(0.0f); else return(av_cue_power[type]); }
  inline static rational_t get_cue_stealth_mod(eAVCueType type)               { if(type == UNDEFINED_AV_CUE) return(1.0f); else return(av_cue_stealth_mod[type]); }
  inline static rational_t get_cue_turbo_mod(eAVCueType type)                 { if(type == UNDEFINED_AV_CUE) return(1.0f); else return(av_cue_turbo_mod[type]); }
  inline static rational_t get_cue_radius(eAVCueType type)                    { if(type == UNDEFINED_AV_CUE) return(0.0f); else return(av_cue_radius[type]); }
  inline static rational_t get_cue_radius_square(eAVCueType type)             { rational_t r = get_cue_radius(type); return(r*r); }
  inline static bool get_cue_investigate(eAVCueType type)                     { if(type == UNDEFINED_AV_CUE) return(true); else return(av_cue_investigate[type]); }
  inline static bool get_cue_search(eAVCueType type)                          { if(type == UNDEFINED_AV_CUE) return(true); else return(av_cue_search[type]); }
  inline static bool get_cue_run(eAVCueType type)                             { if(type == UNDEFINED_AV_CUE) return(true); else return(av_cue_run[type]); }
  static rational_t get_cue_mod_power(eAVCueType type, const entity *ent);

  virtual void add_cue(eAVCueType type, const vector3d &pos, region_node* rgn, const entity *ent)       { add_cue_helper(type, pos, rgn, ent); }
  virtual void add_cue(const stringx &type, const vector3d &pos, region_node* rgn, const entity *ent)   { add_cue_helper(get_cue_type(type), pos, rgn, ent); }
  virtual void add_cue(eAVCueType type, const entity *ent)                                              { if(ent != NULL) add_cue_helper(type, ent->get_abs_position(), ent->get_primary_region(), ent); }
  virtual void add_cue(const stringx &type, const entity *ent)                                          { if(ent != NULL) add_cue_helper(get_cue_type(type), ent->get_abs_position(), ent->get_primary_region(), ent); }
  virtual void add_cue(eAVCueType type, const vector3d &pos);
  virtual void add_cue(const stringx &type, const vector3d &pos)                                        { add_cue(get_cue_type(type), pos); }

  virtual void send_cue(entity *ent, const av_cue &cue);
  virtual void send_cue(entity *ent, eAVCueType type, const vector3d &pos);
  virtual void send_cue(entity *ent, const stringx &type, const vector3d &pos)            { send_cue(ent, get_cue_type(type), pos); }
/*!
  virtual void send_cue(char_group *grp, const av_cue &cue);
  virtual void send_cue(char_group *grp, eAVCueType type, const vector3d &pos);
  virtual void send_cue(char_group *grp, const stringx &type, const vector3d &pos)        { send_cue(grp, get_cue_type(type), pos); }
!*/
  virtual void spotted_hero(entity *ent);
  virtual void was_damaged(entity *ent);
  virtual void was_killed(entity *ent);

  virtual void initialize();
  virtual void clear();
  virtual void frame_advance(time_value_t time_inc);

  virtual void add_signaller(signaller *sig, int event_flags = _DREAD_NET_LINK_ALL);
  virtual void remove_signaller(signaller *sig);

//!  virtual void link(signaller *sig, char_group *grp);
  virtual void link(signaller *sig, entity *ent);
//!  virtual void unlink(signaller *sig, char_group *grp);
  virtual void unlink(signaller *sig, entity *ent);

  virtual dread_net_link *get_link(signaller *sig);
  virtual bool signaller_linked(signaller *sig)   { return(get_link(sig) != NULL); }

  virtual void read_data(chunk_file &fs, stringx &label);

#if _ENABLE_WORLD_EDITOR
  void write_data(ofstream &out);
#endif

  virtual void render(color32 col, char level = 1);

  void read_cue_file( const stringx& filename );
  void read_cue_file(chunk_file &fs);
  void read_cue(chunk_file &fs, stringx &label);


/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////
public:
  // enum of local signal ids (for coding convenience and readability)
  enum signal_id_t
  {
    // a descendant class uses the following line to append its local signal ids after the parent's
    PARENT_SYNC_DUMMY = signaller::N_SIGNALS - 1,
#define MAC(label,str)  label,
#include "dread_net_signals.h"
#undef MAC
    N_SIGNALS
  };

  // This static function must be implemented by every class which can generate
  // signals, and is called once only by the application for each such class;
  // the effect is to register the name and local id of each signal with the
  // signal_manager.  This call must be performed before any signal objects are
  // actually created for this class (via signaller::signal_ptr(); see signal.h).
  static void register_signals();

  static unsigned short get_signal_id( const char *name );

private:
  // Every descendant of signaller that expects to generate signals and has
  // defined its own local list of signal ids should implement this virtual
  // function for the construction of the signal list, so that it will reserve
  // exactly the number of signal pointers required, on demand.
  virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

protected:
  // This virtual function, used only for debugging purposes, returns the
  // name of the given local signal
  virtual const char* get_signal_name( unsigned short idx ) const;
};


class dread_net_link
{
protected:
  char flags;
  signaller *sig;
//!  list<char_group *> groups;
  list<entity *> entities;

  bool hero_detected;

  friend class dread_net;

public:
  dread_net_link(signaller *s, int f);
  virtual ~dread_net_link();

  virtual void process_activation(dread_net *net);
  virtual void process_destruction(dread_net *net);
  virtual void process_damaged(dread_net *net);
  virtual void process_hero_detected(dread_net *net);
  virtual void process_hero_undetected(dread_net *net);

  virtual bool linked(entity *ent);
//!  virtual bool linked(char_group *grp);
  virtual void link(entity *ent);
//!  virtual void link(char_group *grp);
  virtual void unlink(entity *ent);
//!  virtual void unlink(char_group *grp);

  virtual void frame_advance(dread_net *net, time_value_t time_inc);
  virtual void send_cue(dread_net *net, dread_net::eAVCueType type, const vector3d &pos);

  virtual void render(color32 col, char level = 1);
};


class av_cue
{
public:
  dread_net::eAVCueType type;
  const entity *owner;
  region_node *region;
  rational_t power;
  vector3d pos;
  bool reached;
  bool investigate;
  bool search;
  bool run;
  rational_t delay;

  av_cue();
  av_cue(dread_net::eAVCueType t, const vector3d &p, region_node *rgn = NULL, const entity *ent = NULL);

  virtual ~av_cue();

  void copy(const av_cue &b);

  inline void operator=(const av_cue &b)   { copy( b ); }

  void set_reached(bool r);
  const vector3d& get_owner_position() const { if(owner != NULL) return(owner->get_abs_position()); else return(pos); }
  region_node *get_region() const { return(region); }

  virtual void frame_advance(time_value_t time_inc);
};



#endif

#endif
