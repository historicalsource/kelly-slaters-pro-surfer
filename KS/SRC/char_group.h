// char_group.h
#ifndef _CHAR_GROUP_H
#define _CHAR_GROUP_H


#include <map>

#include "algebra.h"
#include "singleton.h"
#include "chunkfile.h"
#include "signals.h"

//!class character;
class stringx;
class entity;


// CLASS char_group_id

// I'm just using strings for now but for efficiency we should
// probably do something like we did in DBTS 1.
class char_group_id
  {
  // Data
  protected:
    stringx val;

  // Constructors
  public:
    char_group_id() : val("UNREG") {}
    char_group_id(const stringx& name);

  // Methods
  public:
    bool operator==(const char_group_id& b) const { return val == b.val; }
    bool operator<(const char_group_id& b) const { return val < b.val; }
    const stringx& get_val() const {return val;}

  // Friends
  friend class char_group_manager;

  // IO
#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& io, const char_group_id* id);
#endif
#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& io, char_group_id* id);
#endif
  };

extern const char_group_id CG_ANONYMOUS;


// CLASS char_group
// This class supports grouping characters together for the purpose of executing
// character methods on the entire group.

class char_group : public signaller, public vector<character*>
{
// Types
protected:
  typedef vector<character*> _V;
  typedef vector<vector3d> relpos_list;

  friend class GroupDialog;

// Data
protected:
  char_group_id id;   // group ID
  relpos_list formation;
//  bool alerted;

// Constructors
public:
  char_group();
  char_group(const char_group_id& _id);

// Methods
public:
  const char_group_id& get_id() const { return id; }
  void clear() { _V::clear(); }
  void reserve(int n) { _V::reserve(n); }
  int size() const { return _V::size(); }
  void add(character* cp);
  const vector3d& get_abs_position() const;

//  void alert_me();
//  bool is_alerted() const { return alerted; }

  // return true if given character is a member of the group
  bool find( character* c ) const;

  void AI_action_go_to_range(const vector3d& v,entity* e,rational_t range) const;

  void suspend();
  void unsuspend();

  float min_distance2(const vector3d& pos) const;
  float min_distance(const vector3d& pos) const { return __fsqrt(min_distance2(pos)); }
  float max_distance(const vector3d& pos) const;
  int num_damaged() const;
  int num_alive() const;

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
    // replace "entity" with whatever is appropriate
  #include "char_group_signals.h"
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

  // Friends
  friend class char_group_manager;
#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& fs,const char_group* id);
#endif
#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& fs,char_group* id);
#endif
};


// SINGLETON CLASS char_group_manager
// This class manages a global list of char_groups.

class char_group_manager : public map<char_group_id,char_group*>, public singleton
{
  // Auto-singleton support:
  // Use slc_manager::inst()->function_name(parameters);
  public:
    DECLARE_SINGLETON(char_group_manager)

  // Types
  protected:
    typedef map<char_group_id,char_group*> _M;
  public:
    enum fail_t
    {
      FAIL_NOT_OK = 0,
      FAIL_OK
    };

  // Constructors (not public in a singleton)
  protected:
    char_group_manager();
    char_group_manager(const char_group_manager& b);
    char_group_manager& operator=(const char_group_manager& b);

    char_group* group_all;

  public:
    ~char_group_manager();

  // Methods
  public:
    // This is called by wds when creating the level to seed the manager with a group
    // called GROUP_ALL consisting of all non-hero characters
    void create_GROUP_ALL();

    char_group* get_group_all()   { return(group_all); }

    void add(char_group* cg);
    char_group* find(const char_group_id& id,fail_t fail=FAIL_NOT_OK) const;
    void purge();

    #if _ENABLE_WORLD_EDITOR
      void remove(char_group* cg);
    #endif
};


#endif  // _CHAR_GROUP_H
