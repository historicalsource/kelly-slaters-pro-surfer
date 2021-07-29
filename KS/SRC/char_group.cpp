// char_group.cpp
#include "global.h"

//!#include "char_group.h"
//!#include "character.h"
#include "stringx.h"
#include "brain.h"
//!#include "rigid.h"
#include "wds.h"
//!#include "attrib.h"

DEFINE_SINGLETON(char_group_manager)

  // CLASS char_group_id

const char_group_id CG_ANONYMOUS;

// Constructors

char_group_id::char_group_id(const stringx& name)
{
  val = name;
}

// serial IO

void serial_in(chunk_file& io,char_group_id* id)
{
  stringx in;
  serial_in(io,&in);
  char_group_id id_in(in);
  *id = id_in;
}


// CLASS char_group

// Constructors

char_group::char_group()
//: alerted( false )
{
}

char_group::char_group(const char_group_id& _id)
: id( _id )
//  alerted( false )
{
}


// Methods

void char_group::add( character* cp )
{
  push_back(cp);
  // each character has a list of the char_groups to which he belongs
  cp->add_char_group( this );
  // store formation position (position relative to leader)
  if (size() == 1)
    formation.push_back(vector3d(0,0,0));
  else
  {
    // formation is specified in XZ plane
    vector3d relpos = cp->get_abs_position() - get_abs_position();
    relpos.y = 0;
    formation.push_back(relpos);
  }
}

const vector3d& char_group::get_abs_position() const
{
  // group position is position of leader (first character in group)
  if (size())
  {
	  // split this up to eliminate compiler crash.
	  character* ch=(*this)[0];
    return ch->get_abs_position();
  }
  return ZEROVEC;
}

/*
void char_group::alert_me()
{
  if ( !is_alerted() )
  {
    alerted = true;
    // alert all members of the group
    vector<character*>::const_iterator i = begin();
    vector<character*>::const_iterator i_end = end();
    for ( ; i!=i_end; ++i )
      (*i)->get_brain()->alert_me();
  }
}
*/

// script access

/*
void char_group::set_AI_state(const stringx& s) const
  {
  // set each character's brain state
  for (iterator i=(iterator)begin(); i!=end(); i++)
    (*i)->get_brain()->set_state(s);
  }

void char_group::set_AI_state_goto(const vector3d& v,entity* e) const
  {
  // send each character to indicated position, offset by formation vector
  // (position in formation relative to leader, computed when each character
  // is added to group; see add())
  for (int i=0; i<size(); i++)
    {
    (*this)[i]->get_brain()->set_state_goto(v+formation[i],e);
    }
  }
*/

void char_group::AI_action_go_to_range(const vector3d& v,entity* e,rational_t range) const
{
  // send each character to indicated position, offset by formation vector
  // (position in formation relative to leader, computed when each character
  // is added to group; see add())
  for (int i=0; i<size(); i++)
  {
    (*this)[i]->get_brain()->push_state_goto(v+formation[i],range,false);
  }
}

void char_group::suspend()
{
  for (int i=0; i<size(); i++)
  {
    (*this)[i]->suspend();
  }
}

void char_group::unsuspend()
{
  for (int i=0; i<size(); i++)
  {
    (*this)[i]->unsuspend();
  }
}

float char_group::min_distance2(const vector3d& pos) const
{
  float min2 = FLT_MAX;
  for (const_iterator i=begin(); i!=end(); ++i)
  {
    float d2 = ( (*i)->get_abs_position() - pos ).length2();
    if (d2 < min2)
      min2 = d2;
  }
  return min2;
}

float char_group::max_distance(const vector3d& pos) const
{
  float max2 = 0;
  for (const_iterator i=begin(); i!=end(); ++i)
  {
    float d2 = ((*i)->get_abs_position() - pos).length2();
    if (d2 > max2)
      max2 = d2;
  }
  return __fsqrt(max2);
}

int char_group::num_damaged() const
{
  int n = 0;
  for (const_iterator i=begin(); i!=end(); ++i)
  {
    if ((*i)->get_soft_attrib()->get_hit_points() < (*i)->get_full_hit_points())
      ++n;
  }
  return n;
}

int char_group::num_alive() const
{
  int n = 0;
  for (const_iterator i=begin(); i!=end(); ++i)
  {
    if ( (*i)->is_alive() )
      ++n;
  }
  return n;
}

//---------------------------------------------------------------

bool char_group::find( character* c ) const
{
// Damn compiler won't allow this! JDB 5/17/00
//  return(find(begin(), end(), c) != end());

  const_iterator i = begin();
  const_iterator i_end = end();

  while(i != i_end)
  {
    if(*i == c)
      return true;

    ++i;
  }

  return false;
}




/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

static const char* signal_names[] =
{
#define MAC(label,str)  str,
#include "char_group_signals.h"
#undef MAC
};

unsigned short char_group::get_signal_id( const char *name )
{
  int idx;

  for( idx = 0; idx < (sizeof(signal_names)/sizeof(char*)); idx++ )
  {
    int offset = strlen(signal_names[idx])-strlen(name);

    if( offset > strlen( signal_names[idx] ) )
      continue;

    if( !strcmp(name,&signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
  }

  // not found
  return signaller::get_signal_id( name );
}

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void char_group::register_signals()
{
#define MAC(label,str)  signal_manager::inst()->insert( str, label );
#include "char_group_signals.h"
#undef MAC
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* char_group::get_signal_name( unsigned short idx ) const
{
  assert( idx < N_SIGNALS );
  if ( idx <= PARENT_SYNC_DUMMY )
    return signaller::get_signal_name( idx );
  else
    return signal_names[idx-PARENT_SYNC_DUMMY-1];
}


// serial IO

void serial_in(chunk_file& fs,char_group* cg)
{
  // read nchars
  int n;
  serial_in(fs,&n);
  // build group
  cg->clear();
  cg->reserve(n);
  for (; n; n--)
  {
    // read character id
    stringx id;
    serial_in(fs,&id);
    // find character and add to group
    cg->add(find_character(id.c_str()));
  }
}



// SINGLETON CLASS char_group_manager

// Constructors (not public in a singleton)
char_group_manager::char_group_manager()
  :   _M()
{
  group_all = NULL;
}

// Manager is responsible for destroying managed groups.
char_group_manager::~char_group_manager()
{
  group_all = NULL;

  for (iterator i=begin(); i!=end(); i++)
    delete (*i).second;
}

void char_group_manager::purge()
{
  group_all = NULL;

  for (iterator i=begin(); i!=end(); i++)
    delete (*i).second;

  clear();
}

// Methods


void char_group_manager::create_GROUP_ALL()
{
  assert(group_all == NULL);

  if(group_all == NULL)
    group_all = NEW char_group(char_group_id("GROUP_ALL"));

  group_all->clear();

  int n = g_world_ptr->get_num_characters();
  group_all->reserve(n);
  for (int i=0;i<n;i++)
  {
    character * chr = g_world_ptr->get_character(i);
    if (!(chr->get_id()==entity_id("HERO")))
    {
      group_all->add(chr);
    }
  }
  add(group_all);
}

void char_group_manager::add(char_group* cg)
{
  if (!(cg->get_id()==CG_ANONYMOUS))
  {
    pair<iterator,bool> retval = insert(value_type(cg->get_id(),cg));
    if(retval.second==NULL)
    {
      stringx composite = stringx("Tried to add same char group twice: ")
        +cg->get_id().get_val();
      error( composite.c_str() );
    }
  }
}

char_group* char_group_manager::find(const char_group_id& id,fail_t fail) const
{
  _M::const_iterator it = _M::find(id);
  assert(fail==FAIL_OK || it!=end());
  if (it != end())
    return (*it).second;
  else
    return NULL;
}


#if _ENABLE_WORLD_EDITOR
  void char_group_manager::remove(char_group* cg)
  {
    iterator it = begin();
    while(it != end())
    {
      if((*it).second == cg)
      {
        delete (*it).second;
        erase(it);
      }

      ++it;
    }
  }
#endif
