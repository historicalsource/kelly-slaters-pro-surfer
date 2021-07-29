// entity_anim.cpp

// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "project.h"
#include "algebra.h"

#include "osalloc.h"

#include "entity_anim.h"
#include "oserrmsg.h"
//!#include "character.h"
#include "wds.h"
#include "signal_anim.h"

//P #include "membudget.h"
//P #include "memorycontext.h"
#include "osdevopts.h"
#include "profiler.h"
#include "pstring.h"
#include "chunkfile.h"

//#define ENTITY_ANIM_STD_ALLOC

//const int MAX_CONCURRENT_ANIM=384;
const int MAX_CONCURRENT_ANIM=1000;

STATICALLOCCLASSINSTANCE(entity_anim,MAX_CONCURRENT_ANIM);
STATICALLOCCLASSINSTANCE(entity_anim_tree,MAX_CONCURRENT_ANIM);


#ifdef TARGET_GC

// most of this big ugly hack is in gc_algebra.h

inline void fixupprs( PRS_track * p )
{
	p->endian_fixup_hack();
}

#define fixupsig(p) ((void)0)

#else

#define fixupint(p) ((void)0)
#define fixupuint(p) ((void)0)
#define fixupuint32(p) ((void)0)
#define fixupint32(p) ((void)0)
#define fixupfloat(p) ((void)0)
#define fixupptr(p) ((void)0)
#define fixupprs(p) ((void)0)
#define fixupsig(p) ((void)0)

#endif



// CLASS entity_anim

bool entity_anim::attach( const anim_control_t& ac )
{
  if ( !is_attached() && ent->attach_anim( this ) )
  {
    if(ac.is_tween() && has_po_anim() && po_anim_ptr->has_R() && ent->is_conglom_member())
    {
//      #pragma todo("This attach function is pretty costly. Causes spikes every time a NEW anim is played. Probably the fixup, and the quaternion conversion. (JDB 02-12-01)")

      po the_po = ent->get_rel_po();
	  rel_pos = ent->get_rel_position();
      the_po.set_position(ZEROVEC);
      the_po.fixup();
      tween_quat = quaternion(the_po.get_matrix());

      //tween_duration = 0.3f;

      /*if(!ac.is_looping())
      {
        rational_t dur = ac.get_duration() - ac.get_time();
        if(dur < 0.0f)
          dur = 0.0f;

		if(dur < 0.2f)
          tween_duration = dur;
        else if(dur < 0.6f)
          set_tween_duration = dur*0.5f;
        else if(dur < 1.2f)
          set_tween_duration = dur*0.25f;
      }*/

      //tween_timer = 0.0f;
	  //ac.set_tween_timer(0.0f);
    }
    /*else
    {
      //tween_timer = tween_duration = 0.0f;
		tween_timer = 0.0f;
		//ac.set_tween_timer(0.0f);
    }*/

    set_flag( ANIM_ATTACHED );
    return true;
  }
  return false;
}

void entity_anim::detach()
{
  if ( is_attached() )
  {
    clear_flag( ANIM_ATTACHED );
    if (ent)
    {
    assert( ent!=NULL && ent->get_anim() == this );
    ent->detach_anim();
    }

    //tween_timer = tween_duration = 0.0f;
	//tween_timer = 0.0f;
  }
}


void entity_anim::frame_advance( const anim_control_t& ac )
{
  START_PROF_TIMER( proftimer_anim_adv );

/*
	#ifdef DEBUG
	if (ent->get_id().get_val() == (stringx ("NYPDCHOPPER_LIGHT05_CHOPPERLITEA")) )
	{
		debug_print("found chopper anim");
	}
	#endif
*/

  // po animation
	if ( ent && has_po_anim() )
	{
	  po newpo;

		float tween_duration = ac.get_tween_duration();
		float tween_timer = ac.get_tween_timer() - ac.get_time_delta();
	  if(ent && ac.is_tween() && (tween_timer < tween_duration) && ent->is_conglom_member())
	  {
	    tween_timer += ac.get_time_delta();

	    if(tween_timer < tween_duration)
	    {
	      vector3d p;
	      quaternion r;
	      rational_t s;

	      po_anim_ptr->frame_advance( ac, p, r, s );

				float ratio = tween_timer / tween_duration;
				p = rel_pos + (p - rel_pos)*ratio;
	      r = slerp(tween_quat, r, ratio);

	      newpo = po( p, r, s );
	    }
	    else
	      po_anim_ptr->frame_advance( ac, &newpo );
	  }
	  else
	    po_anim_ptr->frame_advance( ac, &newpo );

	  if(ac.is_po_fixup())
	    newpo.fixup();

	  ent->set_rel_po_no_children( newpo );

	  // This is a hodge-podge of conditions to ensure we don't create motion  info (mi) for anything
	  // that's actually static.  The last condition is kind of a catch all.  The reason this is necessary
	  // is that we sometimes modify static_heap data with animation when we don't care what state they're in
	  // when they restore, so we don't want them creating motion info's dynaimcally--- leads to a crash on restore mark if they do.
	  if ( ent->get_bone_idx() < 0 &&
	        ent->is_flagged( EFLAG_MISC_NONSTATIC ) )
	  {
	    const po& oldpo = ent->get_abs_po();
	    static po fdpo;
	    static vector3d pos;

	    if(ent->has_parent())
	      fast_po_mul(newpo, newpo, ent->link_ifc()->get_parent()->get_abs_po());

	    fast_po_mul(fdpo, newpo, oldpo.inverse());

	    pos = (newpo.get_position() - oldpo.get_position());
	    fdpo.set_position( pos );

	    ent->set_frame_delta( fdpo, ac.get_time_delta() );
	  }

	  ent->set_needs_compute_sector(true);

	  if ( !ent->has_parent() && ac.is_compute_sector() )
	    ent->compute_sector( g_world_ptr->get_the_terrain() );
	}
  // signal animation
  if ( 0 ) //has_signal_anim() )
  {
    static vector<signal_id_t> sigs;
    signal_anim_ptr->frame_advance( ac, &sigs );
    if ( !sigs.empty() )
    {
      entity *sig_entity = ent;
      assert(sig_entity);

      // This makes it so that if a signal is raised on his hand, it will raise it on the character himself
      // Useful for signals in partial animations.
      while(sig_entity->get_bone_idx() >= 0 && sig_entity->has_parent())
        sig_entity = (entity *)sig_entity->link_ifc()->get_parent();

      vector<signal_id_t>::const_iterator i = sigs.begin();
      vector<signal_id_t>::const_iterator i_end = sigs.end();
      for ( ; i!=i_end; ++i )
      {
        // Raise signal here !!!
        sig_entity->raise_signal( *i );
      }
    }
  }

  STOP_PROF_TIMER( proftimer_anim_adv );
}

void entity_anim::reset_start( const anim_control_t& ac )
{
  if ( has_po_anim() )
    po_anim_ptr->reset_start( ac, ent->get_rel_po() );
}

void entity_anim::set_po_anim( po_anim* pp )
{
  delete po_anim_ptr;
  po_anim_ptr = pp;
}

///////////////////////////////////////////////////////////////////////////////
// CLASS entity_track_node
///////////////////////////////////////////////////////////////////////////////

entity_track_node::entity_track_node()
: id( NO_ID ),
  m_child(NULL),
  m_sibling(NULL),
  m_prs_track(NULL),
  m_signal_track(NULL)
{
  owner = OWNS_DATA;
}


entity_track_node::~entity_track_node()
{
  if( owner==OWNS_DATA )
  {
    // delete my tracks
    delete m_prs_track;
    delete m_signal_track;

    // delete my children
    const entity_track_node* ci;
    for ( ci=get_first_child(); ci!=NULL; )
    {
      const entity_track_node* next_sib = ci->get_next_sibling();
      delete ci;
      ci = next_sib;
    }
  }
}


time_value_t entity_track_node::compute_duration() const
{
  time_value_t duration = 0;
  if( m_prs_track )
    duration = m_prs_track->get_duration();
  if( m_signal_track )
    duration = max( duration, m_signal_track->get_duration() );

  return duration;
}


void entity_track_node::add_child( entity_track_node* good_kid )
{
  assert( good_kid );
  assert( good_kid->m_sibling==NULL );
  // this isn't elegant because we have to put the child on the end of the sibling list
  // to maintain the correct order
  if( m_child )
  {
    entity_track_node* nodep = m_child;
    for( ;nodep->m_sibling!=NULL; )
    {
      nodep = nodep->m_sibling;
    }
    nodep->m_sibling = good_kid;
  }
  else
  {
    m_child = good_kid;
  }
}


#if !defined(NO_SERIAL_IN)
void serial_in( chunk_file& fs, entity_track_node* node, entity_track_tree* tree )
  {
  bool first_subtree = true;
  chunk_flavor cf;

  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
    {
    if( cf == chunk_flavor("subtree") )
      {
      //MessageBox( NULL, "subtree chunk", "Msg", MB_OK );
      entity_track_node *newnode = tree->get_root();

      if( !first_subtree )  // if first root was just added bt the tree chunk, don't add a NEW one yet
        newnode = tree->insert_root();

      serial_in( fs, newnode, tree );

      first_subtree = false;
      continue;
      }

    if ( cf == chunk_flavor("flavor") )
      {
      // IGNORE entity flavor!
      stringx str;
      serial_in( fs, &str );
      }
    else if ( cf == chunk_flavor("animid") )
      {
      // read anim id
      stringx idstr;
      serial_in( fs, &idstr );
      node->id = anim_id_manager::inst()->anim_id( idstr );
      }
    else if ( cf == chunk_flavor("tracks") )
      {
      // read list of tracks
      unsigned short ntracks;
      serial_in( fs, &ntracks );
      assert( ntracks<=2 );
      node->m_prs_track = NULL;
      node->m_signal_track = NULL;
      for( int i=0; i<ntracks; i++ )
        {
        anim_track_flavor_t flavor;
        serial_in( fs, &flavor );
        switch ( flavor )
          {
          case PRS_TRACK:
            assert( node->m_prs_track == NULL );
            node->m_prs_track = NEW PRS_track;
            node->m_prs_track->internal_serial_in( fs );
            break;

          case SIGNAL_TRACK:
            assert( node->m_signal_track == NULL );
            node->m_signal_track = NEW signal_track;
            node->m_signal_track->internal_serial_in( fs );
            break;

          default:
            error( fs.get_name() + ": bad anim track flavor in entity_track_tree node" );
          }
        }
      }
    else if ( cf == chunk_flavor("children") )
      {
      unsigned short nchildren;
      serial_in( fs, &nchildren );
      for( int i=0; i<nchildren; i++ )
      {
        entity_track_node* new_child = NEW entity_track_node;
        serial_in( fs, new_child, tree );
        node->add_child( new_child );
      }
/*
      unsigned short nchildren;
      serial_in( fs, &nchildren );
      node->children.resize( nchildren );
      node_list::iterator i;
      for ( i=node->children.begin(); i!=node->children.end(); ++i )
        {
        entity_track_node* node = NEW entity_track_node;
        *i = node;
        serial_in( fs, node, tree );
        }*/
      }
    }
  }
#endif

/*
#if !defined(NO_SERIAL_OUT)
void serial_out( chunk_file& fs, const entity_track_node& node )
  {
  if ( node.id != NO_ID )
    {
    // anim id
    serial_out( fs, chunk_flavor("animid") );
    serial_out( fs, anim_id_manager::inst()->get_label( node.id ) );
    }
  if ( node.tracks.size() )
    {
    // tracks
    serial_out( fs, chunk_flavor("tracks") );
    serial_out( fs, (unsigned short)(node.tracks.size()) );
    entity_track_tree::track_list::const_iterator ti;
    for ( ti=node.tracks.begin(); ti!=node.tracks.end(); ++ti )
      {
      serial_out( fs, (*ti)->get_flavor() );
      (*ti)->internal_serial_out( fs );
      }
    }
  if ( node.children.size() )
    {
    // children
    serial_out( fs, chunk_flavor("children") );
    serial_out( fs, (unsigned short)(node.children.size()) );
    entity_track_tree::node_list::const_iterator ci;
    for ( ci=node.children.begin(); ci!=node.children.end(); ++ci )
      serial_out( fs, **ci );
    }
  // end of entity_track_node
  serial_out( fs, CHUNK_END );
  }
#endif
*/

///////////////////////////////////////////////////////////////////////////////
// CLASS entity_track_tree
///////////////////////////////////////////////////////////////////////////////

entity_track_tree::~entity_track_tree()
{
}


entity_track_node* entity_track_tree::insert_root()
{
  // shift roots
  num_root_nodes++;
  int i;
  for( i=num_root_nodes-1; i>0; i-- )
    root_nodes[i] = root_nodes[i-1];
  root_nodes[0] = entity_track_node();
  return root_nodes;
}


entity_track_node* entity_track_tree::get_root()
{
  return root_nodes;
}


void entity_track_tree::_compute_duration()
{
  duration = 0;
  _recursive_compute_duration( get_root(), duration );
}

void entity_track_tree::_recursive_compute_duration( const entity_track_node* node, time_value_t& d ) const
{
  if ( node )
  {
    time_value_t nd = node->compute_duration();
    if ( nd > d )
      d = nd;
    const entity_track_node* node_it = node->get_first_child();
    for ( ; node_it!=NULL; )
    {
      _recursive_compute_duration( node_it, d );
      node_it = node_it->get_next_sibling();
    }
/*    node_list::const_iterator i = node->get_children().begin();
    node_list::const_iterator i_end = node->get_children().end();
    for ( ; i!=i_end; ++i )
      _recursive_compute_duration( *i, d );*/
  }
}



#if !defined(NO_SERIAL_IN)
void serial_in( chunk_file& fs, entity_track_tree* tree )
{
  chunk_flavor cf;
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( cf == chunk_flavor("anim") )
    {
      // CTT 02/28/00: TEMPORARY METHOD:
      // the chunk label "anim" actually signifies the entire entity_track_tree
      // object and not a sub-chunk as the current placement implies; it has
      // been put here merely to provide temporary backward-compatibility
    }
    else if ( cf == chunk_flavor("floor") )
    {
      serial_in( fs, &tree->floor_offset );
    }
    else if ( cf == chunk_flavor("tree") )
    {
      serial_in( fs, tree->insert_root(), tree );
    }
    else
      error( fs.get_name() + ": unknown chunk found in entity_track_tree" );
  }
  tree->_compute_duration();
}
#endif

void entity_track_tree_from_binary( entity_track_tree * ett, const char* filename, unsigned int ett_size )
{
  // start fixing up
  uint8* animbuf = (uint8*)ett;

	#ifdef EVAN
	//warning("Loading animation %s",filename);
	#endif

  // check correct filetype
  if( memcmp( ett->file_header, "ANMX", 4) != 0 )
  {
    if( memcmp( ett->file_header, "ZNMX", 4) == 0 )
    {
      // already fixed up.  I should have put a flag in the export format.
			warning("Revisited animation %s",filename);
      return;
    }
    else
    {
      error( "Not an .anmx file.", filename );
    }
  }

  ett->file_header[0] = 'Z';

	#ifdef TARGET_GC
		fixupuint32(&(ett->version));
		fixupint32(&(ett->num_root_nodes));
		fixup((unsigned char *)&(ett->floor_offset),sizeof(rational_t));
		fixup((unsigned char *)&(ett->duration),sizeof(time_value_t));
		fixupint32(&(ett->total_nodes));
	#endif
  //  entity_track_node  root_nodes[MAX_ROOT_NODES];  // this is somewhat of a misnomer;

  // check version number
  if( ett->version != 0x0100 )
	{
   	error( "Incorrect version number.  Expected 0x0100 but got %x in %s", ett->version, filename );
	}

  // fix up nodes
  int i;
  for( i=0; i<ett->total_nodes; i++ )
  {
		#ifdef TARGET_GC
			unsigned char *idptr=(unsigned char *) &(ett->root_nodes[i].id);
			for ( int j=0; j<(sizeof(pstring)/sizeof(int64)); j++ )
			{
				fixup(idptr,sizeof(int64));
				idptr += sizeof(int64);
			}

			//fixupint32(&(ett->root_nodes[i].id));
			//fixupint32(&(ett->root_nodes[i].owner));
			fixupint(&(ett->root_nodes[i].m_child));
			fixupint(&(ett->root_nodes[i].m_sibling));
			fixupint(&(ett->root_nodes[i].m_prs_track));
			fixupint(&(ett->root_nodes[i].m_signal_track));
		#endif
		int64 iid=0;
		memcpy(&iid, &ett->root_nodes[i].id, 4);

    // anim id from string to int
    pstring* anim_id_string = (pstring*)&(ett->root_nodes[i].id);
//#pragma todo("This will be faster when anim_id_manager uses pstrings (JDF 1-26-01)")
    int numeric_id;
    const char *c_str = anim_id_string->c_str();
    if( c_str[0] )
    {
			#ifdef EVAN
				//warning("  ID: %s", c_str);
			#endif
      #if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
        if(i==0 && !strnicmp(c_str, "FAKE", 4))
          warning("Animation '%s' has an anim_id on the fake_root (%s)!", filename, c_str);
      #endif

      numeric_id = anim_id_manager::inst()->anim_id( c_str );
    }
    else
      numeric_id = -1;

    ett->root_nodes[i].id = numeric_id;
    ett->root_nodes[i].owner = BORROWS_DATA;

    // fixup children and siblings
    unsigned child_idx = (int)ett->root_nodes[i].m_child;
    assert( (int)child_idx < ett->total_nodes );
    if( child_idx )
      ett->root_nodes[i].m_child = &ett->root_nodes[child_idx];
    unsigned sibling_idx = (int)ett->root_nodes[i].m_sibling;
    assert( (int)sibling_idx < ett->total_nodes );
    if( sibling_idx )
      ett->root_nodes[i].m_sibling = &ett->root_nodes[sibling_idx];


    PRS_track* prs_track = ett->root_nodes[i].m_prs_track;  // not yet valid
    if( ett->root_nodes[i].m_prs_track )
    {
//      fixupint(&(prs_track));
      prs_track = (PRS_track*)((int)prs_track + animbuf);  // fixup
			fixupprs(prs_track);

      ett->root_nodes[i].m_prs_track = prs_track;
      if( prs_track->P )
      {
        fixupint(&(prs_track->P));
        assert( (unsigned int)prs_track->P < ett_size );
				prs_track->P = (linear_track<vector3d>*)((int)(prs_track->P) + animbuf );
        assert( prs_track->P->m_keys == NULL );
        prs_track->P->m_keys = ((linear_key<vector3d>*)((char*)(&(prs_track->P->m_keys))+4));
				#ifdef TARGET_GC
				prs_track->endian_fixup_hack_P();
				#endif
      }
      if( prs_track->R )
      {
        fixupint(&(prs_track->R));
        assert( (unsigned int)prs_track->R < ett_size );
        prs_track->R = (linear_track<quaternion>*)((int)(prs_track->R) + animbuf );
        assert( prs_track->R->m_keys == NULL );
        prs_track->R->m_keys = ((linear_key<quaternion>*)((char*)(&(prs_track->R->m_keys))+4));
				#ifdef TARGET_GC
				prs_track->endian_fixup_hack_R();
				#endif
      }
      if( prs_track->S )
      {
        fixupint(&(prs_track->S));
        assert( (unsigned int)prs_track->S < ett_size );
        prs_track->S = (linear_track<float>*)((int)(prs_track->S) + animbuf );
        assert( prs_track->S->m_keys == NULL );
        prs_track->S->m_keys = ((linear_key<float>*)((char*)(&(prs_track->S->m_keys))+4));
				#ifdef TARGET_GC
				prs_track->endian_fixup_hack_S();
				#endif
      }
    }
    if( ett->root_nodes[i].m_signal_track )
    {
			assert(0);
      ett->root_nodes[i].m_signal_track = (signal_track*)((int)(ett->root_nodes[i].m_signal_track) + animbuf);
			fixupsig(ett->root_nodes[i].m_signal_track);
      assert( ett->root_nodes[i].m_signal_track->signals == NULL );
      ett->root_nodes[i].m_signal_track->signals = (signal_key*)(((char*)(&(ett->root_nodes[i].m_signal_track->signals)))+4);
    }
  }
}


void* entity_track_tree::operator new( size_t size )
{
  return malloc( size );
}

void entity_track_tree::operator delete( void* buf )
{
  if (!stash::using_stash())
    free( buf );
}

const char* entity_track_tree::binary_extension( void )
{
  return "." PLATFORM_ANIM_NAME ;
}

const char* entity_track_tree::text_extension( void )
{
  return ".anm";
}

const char* entity_track_tree::extension( void )
{
  return binary_extension( );
}

/*
#if !defined(NO_SERIAL_OUT)
void serial_out( chunk_file& fs, const entity_track_tree& tree )
{
  if ( tree.root_list.size() )
  {
    serial_out( fs, chunk_flavor("tree") );
    serial_out( fs, *(tree.get_root()) );
  }
  serial_out( fs, CHUNK_END );
}
#endif
*/

///////////////////////////////////////////////////////////////////////////////
// entity track management
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CLASS ett_manager
///////////////////////////////////////////////////////////////////////////////

#define ETT_BIN1_SIZE ( 8 * 1024 )
#define ETT_BIN1_COUNT 12
#define ETT_BIN2_SIZE ( 16 * 1024 )
#define ETT_BIN2_COUNT  8
//#define ETT_BOOKKEEPING 1

ett_manager::ett_manager()
{
  resident_anms = 0;
  resident_alloc_size = 0;
  max_resident_anms = 0;
  max_resident_alloc_size = 0;
#ifdef TARGET_GC  
  bin1_addr = (char *)malloc( ( ETT_BIN1_SIZE * ETT_BIN1_COUNT ) + ( ETT_BIN2_SIZE * ETT_BIN2_COUNT ) );
  bin2_addr = bin1_addr + ( ETT_BIN1_SIZE * ETT_BIN1_COUNT );
#endif //TARGET_GC
};

ett_manager::~ett_manager()
{
  ett_map_itr_t itr = ett_map.begin();
  for( ; itr != ett_map.end(); itr++ )
  {
    stringx name = (*itr).first;
    ett_node& node = (*itr).second;
#if defined(ETT_BOOKKEEPING)
    if( node.ref > 0 )
      debug_print( "ett_mgr: entity_track_tree \'%s\' has ref count of %d", name.c_str(), node.ref );
#endif
    if( !node.stored && node.ett != NULL )
      ett_free( node.ett );
  }
  
  ett_map.clear();
#ifdef TARGET_GC
  free( bin1_addr );
#endif

#if defined(ETT_BOOKKEEPING)
  debug_print( "ett_mgr: Max resident anms: %d. Max resident allocs: %dbytes", max_resident_anms,
                                                                               max_resident_alloc_size );
#endif
};

entity_track_tree *ett_manager::acquire(const stringx &name)
{
  filespec fname( name );
  fname.name.to_upper();
  
  
  ett_map_itr_t itr = ett_map.find( fname.name );
  
  if( itr != ett_map.end() )
  {
    ett_node& node = (*itr).second;
    
    if( node.stored ) // image is resident in the stored buffer
    {
      assert( node.ett != NULL );
      node.ref++;
      return node.ett;
    }
    
    if( node.ett ) // image is in memory
    {
      node.ref++;
      return node.ett;
    }
   
#if defined(TARGET_GC)
    assert( node.ref == 0 );
    assert( node.stored == false );

#if defined(ETT_BOOKKEEPING)
    resident_anms++;
    resident_alloc_size += node.size;
    if( resident_anms > max_resident_anms )
      max_resident_anms = resident_anms;
    if( resident_alloc_size > max_resident_alloc_size )
      max_resident_alloc_size = resident_alloc_size;
    debug_print( "ett_mgr: acquired \'%s\' (RA:%d, RAS:%d)", (*itr).first.c_str()
                                                           , resident_anms
                                                           , resident_alloc_size );
#endif //ETT_BOOKKEEPING
    
    node.ett = (entity_track_tree *)ett_malloc( node.size );
    aram_mgr::aram_read( node.aram_id, node.offset, (void *)node.ett, node.size );
    entity_track_tree_from_binary( node.ett, (*itr).first.c_str() );
    
    node.ref++;
    return node.ett;
#else
	assert( "this makes no sense. The node should have met one of the above conditions" && 0 );
#endif //TARGET_GC

  }
//  assert( 0 );
  return NULL;
}

void ett_manager::release(const entity_track_tree *ett)
{
  ett_map_itr_t itr = ett_map.begin();
  
  for( ; itr != ett_map.end(); itr++ )
  {
    if( (*itr).second.ett == ett )
    {
      ett_node& node = (*itr).second;
      
      //assert( node.ref > 0 );
      node.ref--;
      if( node.stored )
        return;
        
#ifdef TARGET_GC
		if( node.aram_id == aram_id_t( INVALID_ARAM_ID ) )
			return;
#else
		return;
#endif

      if( node.ref == 0 )
      {
        assert( !node.stored );
        ett_free( node.ett );
        node.ett = NULL;
#if defined(ETT_BOOKKEEPING)
        resident_anms--;
        resident_alloc_size -= node.size;
        debug_print( "ett_mgr: freeing \'%s\' (RA:%d, RAS:%d)", (*itr).first.c_str()
                                                              , resident_anms
                                                              , resident_alloc_size );
#endif //ETT_BOOKKEEPING
      }
      return;
    }
  }
  
  debug_print( "Could not find entity_track_tree \'0x%08X\'. Probably part of a scene anim" );
}

bool ett_manager::unload(const stringx& filename)
{
  filespec spec( filename );
  spec.name.to_upper();
 
  ett_map_itr_t itr = ett_map.find( spec.name );
  
  if( itr == ett_map.end() )
  {
    debug_print( "Couldn't unload anim %s", spec.name.c_str() );
    return false;
  }

  ett_node& node = (*itr).second;

  if( node.ett )
  {
    node.ref = 1;
    release( node.ett );

    if( !node.stored && node.ett )
      free( node.ett );
  }

  ett_map.erase( itr );

  return true;
}

 
bool ett_manager::load(const stringx& filename)
{
  filespec spec( filename );
  spec.ext = entity_track_tree::binary_extension();
  spec.name.to_upper();
   
  ett_map_itr_t itr = ett_map.find( spec.name );
  
  if( itr != ett_map.end() )
    return true;
    

  ett_node node;

  node.ref = 0;
  node.ett = NULL;  

  if( stash::using_stash() )
  {
    stash anim_file;

    pstring pname( spec.name + spec.ext );
    stash_index_entry *anim_header = NULL;

    unsigned char *temp_ptr;
    int size;
    bool b = anim_file.get_memory_image( pname, temp_ptr, size, anim_header );
    node.ett = (entity_track_tree *) temp_ptr;
    node.size = size;

    if( b )
    {
#ifdef TARGET_GC
      if( anim_header->in_aram() )
      {
        node.offset = (unsigned int) node.ett;
        node.aram_id = anim_file.get_current_aram_id();
        node.ett = NULL;
        node.stored = false;
      }
      else
#endif //TARGET_GC
      {
        node.stored = true;
        node.offset = 0;
        node.aram_id = INVALID_ARAM_ID;
        entity_track_tree_from_binary( node.ett, spec.name.c_str() );
      }
      ett_map.insert( ett_map_t::value_type( spec.name, node ) );
#ifdef ETT_BOOKKEEPING
      debug_print( "Loaded anim %s. offs=0x%08X, id = %d, ett = 0x%08X, stored = %d",
          spec.name.c_str(), node.offset, (uint32)node.aram_id, node.ett, (int)node.stored );
#endif
      return true;
    }
  }
  else
  {
    stringx name = spec.fullname();
    if( os_file::file_exists( name ) )
		{
      os_file anim_file( name, os_file::FILE_READ );
      int size = anim_file.get_size( );
      ett_node node;
      
      node.ett = (entity_track_tree*) malloc( size );

	  assert( node.ett != NULL );

      if( anim_file.read( (unsigned char*) node.ett, size ) != size )
			{
        error( "ANMX read failed for '%s'", name.c_str( ) );
        free( node.ett );
        return false;
      }

      entity_track_tree_from_binary( node.ett, name.c_str( ) );
      node.ref = 0;
      node.stored = false;
      node.offset = 0;
      node.aram_id = INVALID_ARAM_ID;

      ett_map.insert( ett_map_t::value_type( spec.name, node ) );
      return true;
    }
  }
#ifndef USER_MKV
  debug_print( "couldn't get memory image for animation file '%s'", spec.name.c_str() );
#endif
  return false;
}
  
bool ett_manager::search(const stringx &name)
{
  filespec fname( name );
  fname.name.to_upper();
  
  ett_map_itr_t itr = ett_map.find( fname.name );
  
  if( itr != ett_map.end() )
    return true;
  
  return false;
}

entity_track_tree *ett_manager::ett_malloc(int size)
{
#ifdef TARGET_GC
  assert( size > 0 );
  
  //Too big for bins? Allocate it
  if( size > ETT_BIN2_SIZE )
  {
    return (entity_track_tree *)malloc( size );
  }
  
  // Try in BIN1 if possible
  if( size <= ETT_BIN1_SIZE )
  {
    char *base_addr = bin1_addr;
    
    int i;
    
    for( i = 0; i < ETT_BIN1_COUNT; i++ )
    {
      if( ( *(unsigned int *)base_addr ) == 0x00000000 )
      {
        *base_addr = 0x01;
        return (entity_track_tree *)base_addr;
      }
      
      base_addr += ETT_BIN1_SIZE;
    }
  }
  
  assert( size <= ETT_BIN2_SIZE );
  
  // Fall through to BIN2 if needed
  char *base_addr = bin2_addr;
  
  int i;
  
  for( i = 0; i < ETT_BIN2_COUNT; i++ )
  {
    if( ( *(unsigned int *)base_addr ) == 0x00000000 )
    {
      *base_addr = 0x01;
      return (entity_track_tree *)base_addr;
    }
    
    base_addr += ETT_BIN2_SIZE;
  }
  
#endif //TARGET_GC  

  debug_print( "Alloced %dbytes from heap", size );
  //No free slots or non gamecube (why are you here?)
  return (entity_track_tree *)malloc( size );
}

void ett_manager::ett_free(void *addr)
{
#ifdef TARGET_GC
  if( addr >= bin1_addr
      && addr < ( bin2_addr + ( ETT_BIN2_COUNT * ETT_BIN2_SIZE ) ) )
  {
      unsigned int *temp = (unsigned int *) addr;
      
      *temp = 0x00000000;
      return;
  }
#endif//TARGET_GC
  free( addr );
}

instance_bank<entity_track_tree> entity_track_bank;

#if 0
static entity_track_tree* load_entity_track_binary( const stringx& filename )
{
  entity_track_tree* et = NULL;

  filespec spec( filename );
  stringx name( spec.path + spec.name + entity_track_tree::binary_extension( ) );
  stringx instance_name( spec.name );
  instance_name.to_upper();

  if( stash::using_stash( ) )
	{
    stash anim_file;

    pstring pname( spec.name.c_str() );
    if (strlen(spec.ext.c_str()) > 0)
    	pname.concatinate( spec.ext.c_str() );
		else
    	pname.concatinate( "." PLATFORM_ANIM_NAME  );

#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,pname.c_str());
#endif
    if( pname.length( ) < PSTRING_MAX_LENGTH ) {
      unsigned char* et_buf = NULL;
      int anim_size = 0;
      stash_index_entry* anim_header = NULL;

      bool b = anim_file.get_memory_image( pname, et_buf, anim_size, anim_header );

      if( b )
			{
        et = (entity_track_tree*) et_buf;
        entity_track_tree_from_binary( et, name.c_str( ) );
        entity_track_bank.insert_new_object( et, instance_name );
        return et;
      }
			else
			{
#ifndef USER_MKV
        debug_print( "couldn't get memory image for animation file '%s'", name.c_str( ) );
#endif
        return NULL;
      } // get_memory_image

    }
		else
		{
      debug_print( "animation file '%s' has name that is too long", name.c_str( ) );
      return NULL;
    } // length < MAX

  }
	else
	{ // using_stash

    if( os_developer_options::inst( )->is_flagged( os_developer_options::FLAG_STASH_ONLY ) )
		{
      debug_print( "stash load failed for '%s', but FLAG_STASH_ONLY is set", name.c_str( ) );
      return NULL;
    }

    if( os_file::file_exists( name ) )
		{
      os_file anim_file( name, os_file::FILE_READ );
      int size = anim_file.get_size( );

      et = (entity_track_tree*) malloc( size );

      if( anim_file.read( (unsigned char*) et, size ) != size )
			{
        error( "ANMX read failed for '%s'", name.c_str( ) );
        free( et );
        return NULL;
      }

      entity_track_tree_from_binary( et, name.c_str( ) );
      entity_track_bank.insert_new_object( et, instance_name );
      return et;
    }
		else
		{
      debug_print( "couldn't find '%s', will try text", name.c_str( ) );
      return NULL;
    } // file exists

  } // using_stash

  return NULL;
}

static entity_track_tree* load_entity_track_text( const stringx& filename )
{
  entity_track_tree* et = NULL;

  filespec spec( filename );
  stringx name( spec.path + spec.name + entity_track_tree::text_extension( ) );
  stringx instance_name( spec.name );
  instance_name.to_upper();

  chunk_file fs;

  if( os_developer_options::inst( )->is_flagged( os_developer_options::FLAG_CHATTY_LOAD ) ) {
    debug_print( "missing .ANMX file: '%s'", name.c_str( ) );
  }

  fs.open( name );
  et = new entity_track_tree( );
  serial_in( fs, et );
  entity_track_bank.insert_new_object( et, instance_name);

  return et;
}
#endif //0

void load_entity_track( const stringx& filename )
{
  assert( 0 );
#if 0
  if( find_entity_track( filename ) ) {
    return;
  }

  entity_track_tree* e = NULL;

  e = load_entity_track_binary( filename );

  if( !e ) {
    e = load_entity_track_text( filename );
  }
#endif
}

void debug_compare_nodes( entity_track_node* node1, entity_track_node* node2 )
{
#ifdef DEBUG
  assert( node1->id == node2->id );
  // PRS tracks
  PRS_track* prstrack1 = node1->m_prs_track;
  PRS_track* prstrack2 = node2->m_prs_track;
  if( prstrack1 )
  {
    assert( prstrack2 );
    // compare PRS tracks
    assert( prstrack1->get_duration() == prstrack2->get_duration() );
    assert( prstrack1->get_flags() == prstrack2->get_flags() );
    if( prstrack1->get_P() )
    {
      assert( prstrack2->get_P() );
      assert( prstrack1->get_P()->num_keys == prstrack2->get_P()->num_keys );
      for( int i=0; i<prstrack1->get_P()->num_keys; i++ )
      {
        assert( prstrack1->get_P()->m_keys[i].get_time() == prstrack2->get_P()->m_keys[i].get_time() );
        assert( prstrack1->get_P()->m_keys[i].get_value() == prstrack2->get_P()->m_keys[i].get_value() );
      }
    }
    else
    {
      assert( prstrack2->get_P() == NULL );
    }
    if( prstrack1->get_R() )
    {
      assert( prstrack2->get_R() );
      assert( prstrack1->get_R()->num_keys == prstrack2->get_R()->num_keys );
      for( int i=0; i<prstrack1->get_R()->num_keys; i++ )
      {
        assert( prstrack1->get_R()->m_keys[i].get_time() == prstrack2->get_R()->m_keys[i].get_time() );
        assert( prstrack1->get_R()->m_keys[i].get_value() == prstrack2->get_R()->m_keys[i].get_value() );
      }
    }
    else
    {
      assert( prstrack2->get_R() == NULL );
    }
    if( prstrack1->get_S() )
    {
      assert( prstrack2->get_S() );
      assert( prstrack1->get_S()->num_keys == prstrack2->get_S()->num_keys );
      for( int i=0; i<prstrack1->get_S()->num_keys; i++ )
      {
        assert( prstrack1->get_S()->m_keys[i].get_time() == prstrack2->get_S()->m_keys[i].get_time() );
        assert( prstrack1->get_S()->m_keys[i].get_value() == prstrack2->get_S()->m_keys[i].get_value() );
      }
    }
    else
    {
      assert( prstrack2->get_S() == NULL );
    }
  }
  else
  {
    assert( prstrack2==NULL );
  }
  // signal track
  signal_track* sigtrack1 = node1->m_signal_track;
  signal_track* sigtrack2 = node2->m_signal_track;
  if( sigtrack1 )
  {
    assert( sigtrack2 );
    assert( sigtrack1->duration == sigtrack2->duration );
    assert( sigtrack1->num_signals == sigtrack2->num_signals );
    for( int i=0; i<sigtrack1->num_signals; i++ )
    {
      assert( sigtrack1->signals[i].get_time() == sigtrack2->signals[i].get_time() );
      assert( sigtrack1->signals[i].get_value() == sigtrack2->signals[i].get_value() );
    }
  }
  else
  {
    assert( sigtrack2 == NULL );
  }
  // recurse
  if( node1->m_child )
  {
    assert( node2->m_child );
    debug_compare_nodes( node1->m_child, node2->m_child );
  }
  else
  {
    assert( node2->m_child==NULL );
  }
  if( node1->m_sibling )
  {
    assert( node2->m_sibling );
    debug_compare_nodes( node1->m_sibling, node2->m_sibling );
  }
  else
  {
    assert( node2->m_sibling==NULL );
  }
#endif
}


void unload_entity_track( const stringx& filename )
{
  assert( 0 );
#if 0
  entity_track_tree* ptr = find_entity_track( filename );
  if ( ptr )
    entity_track_bank.delete_instance( ptr );
#endif
}


entity_track_tree* find_entity_track( const stringx& filename )
{
  assert( 0 );
  return NULL;	// avoid compiler error (dc 04/29/02)
#if 0
  filespec brokenname( filename );
  stringx name = brokenname.name;
  name.to_upper();
  return entity_track_bank.find_instance( name );
#endif
}


///////////////////////////////////////////////////////////////////////////////
// CLASS entity_anim_tree
///////////////////////////////////////////////////////////////////////////////

#define ENITITY_ANIM_TREE_PREALLOC 64

//entity_anim_tree::pentity_anim_vector entity_anim_tree_anims[MAX_CONCURRENT_ANIM];
pentity_anim_vector *entity_anim_tree_anims;//[MAX_CONCURRENT_ANIM];

#ifndef USINGSTATICSTLALLOCATIONS

static char entity_anim_tree_membuffer[MAX_CONCURRENT_ANIM*sizeof(entity_anim_tree)];

#endif


void entity_anim_tree_stl_prealloc ()
{
	#ifndef USINGSTATICSTLALLOCATIONS
		memset(entity_anim_tree_membuffer,0,MAX_CONCURRENT_ANIM*sizeof(entity_anim_tree));
	#endif

    if (!entity_anim_tree_anims)
      entity_anim_tree_anims = NEW pentity_anim_vector[MAX_CONCURRENT_ANIM];

  for (int i = 0; i < MAX_CONCURRENT_ANIM; i++)
  {
		#ifdef USINGSTATICSTLALLOCATIONS
//    entity_anim_tree *rv=(entity_anim_tree*)( ((char *)entity_anim_tree::membuffer)+(i*sizeof(entity_anim_tree)) );
		#else
//    entity_anim_tree *rv=(entity_anim_tree*)( ((char *)entity_anim_tree_membuffer)+(i*sizeof(entity_anim_tree)) );
		#endif
/*
    entity_anim anim_pt;
    rv->anims.clear ();
    for (int j = 0; j < ENITITY_ANIM_TREE_PREALLOC; j++)
      rv->anims.push_back (&anim_pt);
    rv->anims.clear ();
*/
    entity_anim anim_pt;
    entity_anim_tree_anims[i].clear ();
    for (int j = 0; j < ENITITY_ANIM_TREE_PREALLOC; j++)
      entity_anim_tree_anims[i].push_back (&anim_pt);
    entity_anim_tree_anims[i].clear ();

//    rv->anims = &entity_anim_tree_anims[i];
  }

}

void entity_anim_tree_stl_dealloc ()
{
  for (int i = 0; i < MAX_CONCURRENT_ANIM; i++)
    entity_anim_tree_anims[i].clear ();

  delete [] entity_anim_tree_anims;
  entity_anim_tree_anims = NULL;
}

entity_anim_tree::entity_anim_tree( const stringx& _name,
                                    entity* _ent,
                                    const entity_track_tree& _track,
                                    unsigned short anim_flags,
                                    time_value_t start_time,
                                    int _priority,
                                    short loop )
: entity_anim( _ent )
#ifdef USINGSTATICSTLALLOCATIONS
, anims ( entity_anim_tree_anims[current_allocation] )
#endif
{
  construct( _name, _track, anim_flags, start_time, _priority, loop );
}


entity_anim_tree::entity_anim_tree( const stringx& _name,
                                    entity* _ent,
                                    const entity_track_tree& _tracka,
                                    const entity_track_tree& _trackb,
                                    rational_t blenda,
                                    rational_t blendb,
                                    unsigned short anim_flags,
                                    time_value_t start_time,
                                    int _priority,
                                    short loop )
: entity_anim( _ent )
#ifdef USINGSTATICSTLALLOCATIONS
, anims ( entity_anim_tree_anims[current_allocation] )
#endif
{
  construct( _name, _tracka, _trackb, blenda, blendb, anim_flags, start_time, _priority, loop );
}


//static entity_anim_tree::pentity_anim_vector::const_iterator construction_iterator;
static pentity_anim_vector::const_iterator construction_iterator;

bool anims_b_used = false;

void entity_anim_tree::construct( const stringx& _name,
                                  const entity_track_tree& _track,
                                  unsigned short anim_flags,
                                  time_value_t start_time,
                                  int _priority,
                                  short loop )
{
START_PROF_TIMER( proftimer_anim_const1 );
START_PROF_TIMER( proftimer_anim_const1pre );
  entity_anim::construct( ent, anim_flags );
	deconstruct();

  name = _name;
  track = &_track;
  control = anim_control_t( start_time, _track.get_duration(), 1.0f, anim_flags, loop );
  if(control.is_tween())
  {
      if(!control.is_looping())
      {
        rational_t dur = control.get_duration() - control.get_time();
        if(dur < 0.0f)
          dur = 0.0f;

        if(dur < 0.2f)
          control.set_tween_duration(dur);
        else if(dur < 0.6f)
          control.set_tween_duration(dur*0.5f);
        else if(dur < 1.2f)
          control.set_tween_duration(dur*0.25f);
      }
	  control.set_tween_timer(0.0f);
  }
  else
  {
	  control.set_tween_duration(0.0f);
	  control.set_tween_timer(0.0f);
  }
  control_b = control;

  set_valid( true );
  entity::prepare_for_visiting();
  // add NEW root node for each seperate hierarchy in the track and construct NEW tree on top of NEW root

//	if (anims==NULL) return;
  //clear_anims();
  construction_iterator = anims.begin();
  const entity_track_node* i = _track.get_root_nodes();
  const entity_track_node* i_end = i + _track.get_num_root_nodes();
  anims_b_used = false;
STOP_PROF_TIMER( proftimer_anim_const1pre );
START_PROF_TIMER( proftimer_anim_const1rec );
  for ( ; i!=i_end; ++i )
  {
    if ( !_recursive_construct( get_entity(), i ) )
		{
			#ifdef TARGET_GC
      warning( name + ": anim_id mismatch; could not construct animation for entity " + get_entity()->get_id().get_val() );
			return;
			#else
      error( name + ": anim_id mismatch; could not construct animation for entity " + get_entity()->get_id().get_val() );
			#endif
		}
  }
STOP_PROF_TIMER( proftimer_anim_const1rec );
START_PROF_TIMER( proftimer_anim_const1post );
  // set anim tree flagged as relative to start if root anim is such
  if( !(  anim_flags & ANIM_FORCE_ABSOLUTE ) )
    if ( _track.get_num_root_nodes()==1 && !anims.empty() && (*anims.begin())->is_relative_to_start() )
		set_flag( ANIM_RELATIVE_TO_START );
  // set the priority of all nodes
  set_priority( _priority );
  // attach the animation
  attach();
  // make sure the po_anim start values (for start-relative animation) sync with
  // the current state of the entity
  reset_root_position();
  // set up floor_offset
  floor_offset = _track.get_floor_offset();
  (*anims.begin())->floor_offset = floor_offset;

  blend_a = 1.0f;
  blend_b = 0.0f;
  trackb = NULL;
STOP_PROF_TIMER( proftimer_anim_const1post );
STOP_PROF_TIMER( proftimer_anim_const1 );
}


void entity_anim_tree::construct( const stringx& _name,
                                  const entity_track_tree& _tracka,
                                  const entity_track_tree& _trackb,
                                  rational_t blenda,
                                  rational_t blendb,
                                  unsigned short anim_flags,
                                  time_value_t start_time,
                                  int _priority,
                                  short loop )
{

  START_PROF_TIMER( proftimer_anim_const2 );
  entity_anim::construct( ent, anim_flags );
	deconstruct();

  set_blend(blenda, blendb);

  name = _name;
  track = &_tracka;
  trackb = &_trackb;
  control = anim_control_t( start_time, _tracka.get_duration(), 1.0f, anim_flags, loop );
  control_b = anim_control_t( start_time, _trackb.get_duration(), 1.0f, anim_flags, loop );

  set_valid( true );
  entity::prepare_for_visiting();

  // add NEW root node for each seperate hierarchy in the track and construct NEW tree on top of NEW root
//	if (anims==NULL) return;
  //clear_anims();
  construction_iterator = anims.begin();
  const entity_track_node* i = _tracka.get_root_nodes();
  const entity_track_node* i_end = i + _tracka.get_num_root_nodes();
  anims_b_used = false;
  for ( ; i!=i_end; ++i )
  {
    if ( !_recursive_construct( get_entity(), i ) )
      error( name + ": anim_id mismatch; could not construct animation A for entity " + get_entity()->get_id().get_val() );
  }

  entity::prepare_for_visiting();

  //clear_anims_b();
  construction_iterator = anims_b.begin();
  i = _trackb.get_root_nodes();
  i_end = i + _trackb.get_num_root_nodes();
  anims_b_used = true;
  for ( ; i!=i_end; ++i )
  {
    if ( !_recursive_construct( get_entity(), i ) )
      error( name + ": anim_id mismatch; could not construct animation B for entity " + get_entity()->get_id().get_val() );
  }

  if(anims.size() != anims_b.size())
    error("Blended animation '%s' does not contain the same number of nodes in each animation: %d vs %d", name.c_str(), anims.size(), anims_b.size());

  // set anim tree flagged as relative to start if root anim is such
  if ( _tracka.get_num_root_nodes()==1 && !anims.empty() && (*anims.begin())->is_relative_to_start() )
    set_flag( ANIM_RELATIVE_TO_START );
  // set the priority of all nodes
  set_priority( _priority );
  // attach the animation
  attach();
  // make sure the po_anim start values (for start-relative animation) sync with
  // the current state of the entity
  reset_root_position();
  // set up floor_offset
  floor_offset = _tracka.get_floor_offset();
  (*anims.begin())->floor_offset = floor_offset;
  STOP_PROF_TIMER( proftimer_anim_const2 );
}


static int s_debug;

bool entity_anim_tree::_recursive_construct( entity* _ent,
                                             const entity_track_node* node )
{
	// check for errors
	START_PROF_TIMER( proftimer_anim_recursekids );
	int32 nodeid=node->get_id();
	int32 entid=_ent->get_anim_id();
	if ( nodeid != NO_ID )
	{
		if ( nodeid != entid )
		{
			// if node id is unexpected, just keep skipping stuff, until we hit one that is correct
			// this is to support animation of subtrees of a full hierarchy
			//
			//assert(0);
			warning("Animation ID mismatch %08X vs %08X",nodeid,entid);
			bool found = false;
			if ( _ent->has_children() )
			{
				const bone *child = _ent->link_ifc()->get_first_child();
				
				while (child != NULL)
				{
					STOP_PROF_TIMER( proftimer_anim_recursekids );
					if (_recursive_construct( (entity *)child, node ) )
					{
						START_PROF_TIMER( proftimer_anim_recursekids );
						found = true;
						break;
					}
					START_PROF_TIMER( proftimer_anim_recursekids );
					child = child->link_ifc()->get_next_sibling();
				}
/*!				entity::child_list::const_iterator ci = _ent->get_children().begin();
				entity::child_list::const_iterator ci_end = _ent->get_children().end();
				for ( ; ci!=ci_end; ++ci )
				{
					if ( _recursive_construct( *ci, node ) )
					found = true;
				}
!*/
			}
			if( found==false )
			{
				s_debug++;
			}
			STOP_PROF_TIMER( proftimer_anim_recursekids );
			return found;
		}
	}
	STOP_PROF_TIMER( proftimer_anim_recursekids );

  START_PROF_TIMER( proftimer_anim_recursemake );
  // create animation(s) for this entity
  entity_anim* anim_pt;
//	if (anims==NULL) return false;
  if ( construction_iterator != (anims_b_used ? anims_b.end() : anims.end()) )
  {
    anim_pt = *construction_iterator;
    ++construction_iterator;
  }
  else
  {
    anim_pt = NEW entity_anim;
    if(anims_b_used)
    {
      anims_b.push_back( anim_pt );
      construction_iterator = anims_b.end();
    }
    else
    {
      anims.push_back( anim_pt );
      construction_iterator = anims.end();
    }
  }
  anim_pt->construct( _ent, get_flags()&FLAGS_PASSED_TO_SUBANIMS );
  STOP_PROF_TIMER( proftimer_anim_recursemake );

  START_PROF_TIMER( proftimer_anim_recursecrap );
  // invalidate po and signal anims
  po_anim* pa = anim_pt->get_po_anim();
  if ( pa != NULL )
    pa->set_valid( false );
  signal_anim* sa = NULL; //anim_pt->get_signal_anim();
  if ( sa != NULL )
    sa->set_valid( false );

  if( node->get_prs_track() )
  {
    const PRS_track& track = *(node->get_prs_track());
    if ( pa == NULL )
    {
      // create NEW po_anim
      pa = NEW po_anim;
      anim_pt->set_po_anim( pa );
    }
    // (re)construct po_anim
    po ent_po = _ent->get_rel_po();
    if(track.get_S() == NULL)
      ent_po.fixup();

    pa->construct( ent_po,
                   track,
                   get_flags() & FLAGS_PASSED_TO_SUBANIMS );
    if ( pa && pa->is_valid() && pa->is_relative_to_start() )
      anim_pt->set_flag( ANIM_RELATIVE_TO_START );
  }

#if 0
  if( node->get_signal_track() )
  {
    const signal_track& track = *(node->get_signal_track() );
    if ( sa == NULL )
    {
      // create NEW signal anim
      sa = NEW signal_anim;
      anim_pt->set_signal_anim( sa );
    }
    // (re)construct signal anim
    sa->construct( track,
                   get_flags() & FLAGS_PASSED_TO_SUBANIMS );
    if ( pa && pa->is_valid() && pa->is_relative_to_start() )
      anim_pt->set_flag( ANIM_RELATIVE_TO_START );
  }
#endif
  STOP_PROF_TIMER( proftimer_anim_recursecrap );
/*
  for ( ti=node->get_tracks().begin(); ti!=node->get_tracks().end(); ++ti )
  {
    switch ( (*ti)->get_flavor() )
    {
    case PRS_TRACK:
      {
        const PRS_track& track = *(PRS_track*)(anim_track*)(*ti);
        if ( pa == NULL )
        {
          // create NEW po_anim
          pa = NEW po_anim;
          anim_pt->set_po_anim( pa );
        }
        // (re)construct po_anim
        pa->construct( _ent->get_rel_po(),
                       track,
                       get_flags() & FLAGS_PASSED_TO_SUBANIMS );
      }
      break;

    case SIGNAL_TRACK:
      {
        const signal_track& track = *(signal_track*)(anim_track*)(*ti);
        if ( sa == NULL )
        {
          // create NEW signal anim
          sa = NEW signal_anim;
          anim_pt->set_signal_anim( sa );
        }
        // (re)construct signal anim
        sa->construct( track,
                       get_flags() & FLAGS_PASSED_TO_SUBANIMS );
      }
      break;

    default:
      break;
    }

    if ( pa && pa->is_valid() && pa->is_relative_to_start() )
      anim_pt->set_flag( ANIM_RELATIVE_TO_START );
  }
*/

  START_PROF_TIMER( proftimer_anim_scanner );
#if 1

  if ( _ent->has_children() )
  {
	  const entity_track_node* ni;
    const bone *child = _ent->link_ifc()->get_first_child();
	  for ( ni=node->get_first_child(); ni!=NULL; ni = ni->get_next_sibling() )
	  {
	    const entity_track_node* cnode = ni;
	    bool match = false;
	    bool first = true;
	    const bone *child0 = child;


	    while (child != NULL && (child!=child0||first) ) //&& !match)
	    {
				first=false;
	      entity *ent_child = (entity *)child;
	      if ( !ent_child->already_visited()
	        && ( cnode->get_id()==NO_ID || cnode->get_id()== ent_child->get_anim_id() )
	        )
	      {
	        // match found
	        match = true;
	        ent_child->visit();
	        // recurse on this child
  				STOP_PROF_TIMER( proftimer_anim_scanner );
	        _recursive_construct( ent_child, cnode );
  				START_PROF_TIMER( proftimer_anim_scanner );
	        break;
	      }
		    child = child->link_ifc()->get_next_sibling();
		    if (child == NULL )
	    		child = _ent->link_ifc()->get_first_child();
	    }
    }
    //if ( !match && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_ANIM_WARNINGS) )
    //  warning( name + ": no match found for track " + anim_id_manager::inst()->get_label(cnode->get_id()) + " in entity " + ent->get_id().get_val() );
  }
#else

  // iterate through node's children looking for matches on which to recurse
  const entity_track_node* ni;
  for ( ni=node->get_first_child(); ni!=NULL; ni = ni->get_next_sibling() )
  {
    const entity_track_node* cnode = ni;
    bool match = false;
    if ( _ent->has_children() )
    {
      const bone *child = _ent->link_ifc()->get_first_child();

      while (child != NULL)
      {
        entity *ent_child = (entity *)child;
        if ( !ent_child->already_visited()
          && ( cnode->get_id()==NO_ID || cnode->get_id()== ent_child->get_anim_id() )
          )
        {
          // match found
          match = true;
          ent_child->visit();
          // recurse on this child
  				STOP_PROF_TIMER( proftimer_anim_scanner );
	        _recursive_construct( ent_child, cnode );
  				START_PROF_TIMER( proftimer_anim_scanner );
          break;
        }
        child = child->link_ifc()->get_next_sibling();
      }
/*!
      entity::child_list::const_iterator ci = _ent->get_children().begin();
      entity::child_list::const_iterator ci_end = _ent->get_children().end();
      for ( ; ci!=ci_end; ++ci )
      {
        entity* child = *ci;
        if ( !child->already_visited()
          && ( cnode->get_id()==NO_ID || cnode->get_id()==child->get_anim_id() )
          )
        {
          // match found
          match = true;
          child->visit();
          // recurse on this child
          _recursive_construct( child, cnode );
          break;
        }
      }
!*/
    }
    if ( !match && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_ANIM_WARNINGS) )
      warning( name + ": no match found for track " + anim_id_manager::inst()->get_label(cnode->get_id()) + " in entity " + ent->get_id().get_val() );
  }
#endif
  STOP_PROF_TIMER( proftimer_anim_scanner );

  return true;
}

entity_anim_tree::~entity_anim_tree()
{
  clear_anims();

  clear_anims_b();

  detach();

  if( track )
  {
    g_world_ptr->get_ett_manager()->release( track );
    track = NULL;
  }

  if( trackb )
  {
    g_world_ptr->get_ett_manager()->release( trackb );
    trackb = NULL;
  }
}

void entity_anim_tree::clear_anims()
{
//	if ( anims )
	{
	  pentity_anim_vector::const_iterator i = anims.begin();
	  pentity_anim_vector::const_iterator i_end = anims.end();
	  for ( ; i!=i_end; ++i )
		{
    	delete *i;
		}
	  anims.resize(0);	// reuse the memory (dc 04/25/02)
	}
}

void entity_anim_tree::clear_anims_b()
{
  pentity_anim_vector::const_iterator i = anims_b.begin();
  pentity_anim_vector::const_iterator i_end = anims_b.end();
  for ( ; i!=i_end; ++i )
	{
    delete *i;
	}
  anims_b.resize(0);	// reuse the memory (dc 04/25/02)
}

// force current_time to given value
void entity_anim_tree::set_time( time_value_t t )
{
  control.set_time( t );
  control_b.set_time( t );

  pentity_anim_vector::iterator i;
  for ( i=anims.begin(); i!=anims.end(); ++i )
  {
    entity_anim* a = *i;
    if ( a->is_valid() )
      a->set_time( t );
  }

  if(trackb)
  {
    for ( i=anims_b.begin(); i!=anims_b.end(); ++i )
    {
      entity_anim* b = *i;
      if ( b->is_valid() )
        b->set_time( t );
    }
  }
}


// This is a container class, descended from entity_anim for convenience, but
// differing from the usual entity_anim in one respect: it does not attach
// directly to the entity with which it is associated; that is, the entity
// doesn't point back to this object, but instead will point to whatever anim
// (if any) is currently responsible for performing animation on that root
// entity node.
void entity_anim_tree::attach()
{
  set_flag( ANIM_ATTACHED, true );
  pentity_anim_vector::iterator i = anims.begin();
  pentity_anim_vector::iterator i_end = anims.end();
  pentity_anim_vector::iterator bi = anims_b.begin();
  pentity_anim_vector::iterator bi_end = anims_b.end();
//  bool tweener = control.is_tween();
//  bool root = tweener;
  for ( ; i!=i_end; ++i )
  {
    entity_anim* a = *i;
    entity_anim* b = (bi != bi_end) ? *bi : NULL;
    if(bi != bi_end)
      ++bi;

    if ( a->is_valid() )
    {
      // We do not tween the root. Causes problems...
//      if(root && a->ent)
//        control.set_flag(ANIM_TWEEN, false);

      if ( a->attach(control) )
      {
        a->set_time( control.get_time() );  // make sure every node is updated to the current time
        if(b)
          b->set_time( control_b.get_time() );
      }

/*
      if(root)
      {
        control.set_flag(ANIM_TWEEN, tweener);
        root = false;
      }
*/
    }
  }
}

// for an entity_anim_tree, the notion of being attached is different than for
// a normal entity_anim, in that a tree does not call entity::attach_anim() for
// itself; the concept of tree attachment is used to distinguish between
// animation data that is actually being used at the moment versus data that is
// merely cached for later use (see, for example, entity::clear_anim())
void entity_anim_tree::detach()
{
  set_flag( ANIM_ATTACHED, false );
}

// this suspends me and invalidates all internal entity animations, but leaves
// the allocated memory intact for later use
void entity_anim_tree::deconstruct()
{
  pentity_anim_vector::iterator i;
  for ( i=anims.begin(); i!=anims.end(); ++i )
  {
    entity_anim* a = *i;
    if ( a->is_valid() )
    {
      a->detach();
      a->set_valid( false );
    }
  }
  for ( i=anims_b.begin(); i!=anims_b.end(); ++i )
  {
    entity_anim* a = *i;
    if ( a->is_valid() )
    {
      a->detach();
      a->set_valid( false );
    }
  }
  set_valid( false );

  if( track )
  {
    g_world_ptr->get_ett_manager()->release( track );
    track = NULL;
  }

  if( trackb )
  {
    g_world_ptr->get_ett_manager()->release( trackb );
    trackb = NULL;
  }

}

// this reconstructs the animation from the track data (used after a previous deconstruct call)
void entity_anim_tree::reconstruct( int _priority )
{
  if ( is_valid() )
    deconstruct();  // make sure the anim list is clear before reconstruction

  if(trackb)
    construct( stringx(name), *track, *trackb, blend_a, blend_b, flags, control.get_time(), _priority, control.get_loop_count() );
  else
    construct( stringx(name), *track, flags, control.get_time(), _priority, control.get_loop_count() );
}


// the animation timescale defaults to 1.0f; setting it otherwise changes the
// effective playback rate of the animation
void entity_anim_tree::set_timescale_factor( rational_t _timescale_factor )
{
  control.set_timescale_factor( _timescale_factor );
  control_b.set_timescale_factor( _timescale_factor );
}


void entity_anim_tree::frame_advance( time_value_t t )
{
  control.frame_advance( t );
  if(trackb)
    control_b.frame_advance( t );

  if ( is_valid() )//&& anims )
  {
    if(trackb)
    {
      pentity_anim_vector::iterator i = anims.begin();
      pentity_anim_vector::iterator i_end = anims.end();
      pentity_anim_vector::iterator bi = anims_b.begin();
      pentity_anim_vector::iterator bi_end = anims_b.end();
      for ( ; i!=i_end; ++i )
      {
        entity_anim* a = *i;
        entity_anim* b = (bi != bi_end) ? *bi : NULL;
        if(bi != bi_end)
          ++bi;

        if ( a->is_valid() && a->is_attached() )
        {
          if(b && blend_b > 0.0f)
          {
            vector3d bp;
            quaternion br;
            rational_t bs;

            START_PROF_TIMER( proftimer_anim_adv );

            // po animation
            const po& oldpo = a->ent->get_abs_po();
            vector3d oldpos = oldpo.get_position();

            if ( b->has_po_anim() )
            {
              b->po_anim_ptr->frame_advance( control_b, bp, br, bs );
            }

            vector3d np = bp;
            quaternion nr = br;
            rational_t ns = bs;

            if(blend_a > 0.0f && a->has_po_anim())
            {
              vector3d ap;
              quaternion ar;
              rational_t as;

              a->po_anim_ptr->frame_advance( control, ap, ar, as );

              np = oldpos + ((ap - oldpos)*blend_a) + ((bp - oldpos)*blend_b);
              nr = slerp(ar, br, (1.0f - blend_a));
              ns = as;
            }

			float tween_duration = control.get_tween_duration();
			float tween_timer = control.get_tween_timer() - control.get_time_delta();
            if(control.is_tween() && tween_timer < tween_duration)
            {
              tween_timer += control.get_time_delta();

              if(tween_timer < tween_duration)
                nr = slerp(a->tween_quat, nr, tween_timer / tween_duration);
            }

            po newpo( np, nr, ns );
            if(control.is_po_fixup())
              newpo.fixup();

            a->ent->set_rel_po_no_children( newpo );

            if ( a->ent->get_bone_idx() < 0 &&
                 a->ent->is_flagged( EFLAG_MISC_NONSTATIC ) )
            {
              static po fdpo;
              static vector3d pos;

              if(ent->has_parent())
                fast_po_mul(newpo, newpo, ent->link_ifc()->get_parent()->get_abs_po());

              fast_po_mul(fdpo, newpo, oldpo.inverse());

              pos = (newpo.get_position() - oldpo.get_position());
              fdpo.set_position( pos );

              ent->set_frame_delta( fdpo, control.get_time_delta() );
            }

            if ( !a->ent->has_parent() && control.is_compute_sector() )
              a->ent->compute_sector( g_world_ptr->get_the_terrain() );

            // signal animation
            if ( 0 ) //a->has_signal_anim() )
            {
              static vector<signal_id_t> sigs;
              a->signal_anim_ptr->frame_advance( control, &sigs );
              if ( !sigs.empty() )
              {
                vector<signal_id_t>::const_iterator i = sigs.begin();
                vector<signal_id_t>::const_iterator i_end = sigs.end();
                for ( ; i!=i_end; ++i )
                {
                  // Raise signal here !!!
                  a->get_entity()->raise_signal( *i );
                }
              }
            }

            // signal animation
            if ( 0 ) //b->has_signal_anim() )
            {
              static vector<signal_id_t> sigs;
              b->signal_anim_ptr->frame_advance( control_b, &sigs );
              if ( !sigs.empty() )
              {
                vector<signal_id_t>::const_iterator i = sigs.begin();
                vector<signal_id_t>::const_iterator i_end = sigs.end();
                for ( ; i!=i_end; ++i )
                {
                  // Raise signal here !!!
                  b->get_entity()->raise_signal( *i );
                }
              }
            }

            STOP_PROF_TIMER( proftimer_anim_adv );
          }
          else
            a->frame_advance( control );
        }
      }
    }
    else
    {
      pentity_anim_vector::iterator i = anims.begin();
      pentity_anim_vector::iterator i_end = anims.end();
      for ( ; i!=i_end; ++i )
      {
        entity_anim* a = *i;
        if ( a->is_valid() && a->is_attached() )
          a->frame_advance( control );
      }
    }

    // CTT 03/24/00: TEMPORARY:
    // we really only want to do this update at the end of wds frame_advance, but
    // that'll have to wait until the entity animation system becomes more rational
/*!    if ( ent->get_flavor() == ENTITY_CHARACTER )
      ent->update_abs_po_including_limbs();
    else
!*/
    // anything animatable should have a link_interface
    ent->update_abs_po();
  }
}


void entity_anim_tree::set_priority( int _priority )
{
  entity_anim::set_priority( _priority );
  pentity_anim_vector::iterator i;
  for ( i=anims.begin(); i!=anims.end(); ++i )
  {
    entity_anim* a = *i;
    if ( a->is_valid() )
      a->set_priority( _priority );
  }
}


// this function adjusts the start position of the root node based on the
// current actual position of the entity and the current state of the root anim
void entity_anim_tree::reset_root_position()
{
  if ( !anims.empty() )
  {
    // we expect one unique start-relative root animation when we're doing this
    entity_anim* a = *anims.begin();
    if ( a->is_valid() && a->is_relative_to_start() && a->is_attached() )
    {
      a->reset_start( control );

      if ( trackb && !anims_b.empty() )
      {
        // we expect one unique start-relative root animation when we're doing this
        entity_anim* b = *anims_b.begin();
        if ( b->is_valid() && b->is_relative_to_start() )
          b->reset_start( control_b );
      }
    }
  }
}


// this function returns the unadjusted (relative) value of the root position anim
// at the current time
void entity_anim_tree::get_current_root_relpos( vector3d* destP ) const
{
  if ( !anims.empty() )
  {
    entity_anim* a = *anims.begin();
    if ( a->is_valid() && a->is_relative_to_start() && a->has_po_anim() )
    {
      if ( a->is_attached() )
      {
        // animation is currently attached, so we can assume the control time
        // matches the current state of the animation (the easy way)
        a->get_po_anim()->get_unadjusted_value( control, destP );
      }
      else
      {
        // animation is not currently attached, so we must get the value that
        // corresponds to the current time (the hard way)
        a->get_po_anim()->get_unadjusted_value( control.get_time(), destP );
      }
    }
  }
}

bool entity_anim_tree::is_root(entity *ent) const
{
  return(!anims.empty() && (*anims.begin())->ent == ent);
}

void entity_anim_tree::get_current_po( po* dest ) const
{
  if ( !anims.empty() )
  {
    entity_anim* a = *anims.begin();
    if ( a->is_valid() && a->has_po_anim() )
    {
      if ( a->is_attached() )
      {
        // animation is currently attached, so we can obtain the current value
        // the easy way, using the current control data and assuming the
        // animation key iterator will be in sync
        a->get_po_anim()->get_value( control, dest );
      }
      else
      {
        // animation is not attached, so we must get the value the hard way,
        // by computing it at the correct time but without assuming that the
        // key iterator is in sync with the control data
        a->get_po_anim()->get_value( control.get_time(), dest );
      }
    }
  }
}

void entity_anim_tree::set_blend(rational_t a, rational_t b)
{
  assert((a+b) > 0.0f);

  rational_t d = 1.0f / (a+b);
  blend_a = a * d;
  blend_b = b * d;
}

void entity_anim_tree::debug_print_PRS_to_file(void)
{
	host_system_file_handle outfile = host_fopen( "animation.txt", HOST_WRITE );
    host_fprintf( outfile, "Animation Data\n\n" );
	if ( is_valid() )
	{
		pentity_anim_vector::iterator i = anims.begin();
		pentity_anim_vector::iterator i_end = anims.end();
		for ( ; i!=i_end; ++i )
		{
			entity_anim* a = *i;
			if ( a->is_valid() && a->is_attached() )
			{
				rational_t start_scale, scale = 1.0f;
				vector3d start_pos, pos = ZEROVEC;
				quaternion start_quat, quat;
				po value(po_identity_matrix);
				if (a->has_po_anim())
				{
					entity_id eid = a->get_entity()->get_id();
					stringx cname = eid.get_val();
					po_anim *po_anim_ptr = a->get_po_anim();
					/*if (!strcmp(eid.get_val().c_str(), "HERO") || !strcmp(eid.get_val().c_str(), "HERO_BIP01 PELVIS"))
						int f = 0;
					else
						po_anim_ptr->reset_start_vals();*/

					/*po_anim_ptr->get_unadjusted_value(control, &scale);
					po_anim_ptr->get_unadjusted_value(control, &pos);
					po_anim_ptr->get_unadjusted_value(control, &quat);*/
					po_anim_ptr->get_value(control, &value);
					quat = value.get_quaternion();
					start_quat = po_anim_ptr->get_R_start();
					start_pos = po_anim_ptr->get_P_start();
					start_scale = po_anim_ptr->get_S_start();
					pos = value.get_position();
					scale = value.get_scale();
					host_fprintf( outfile, "entity_id: %s\n----------------\n", eid.get_val().c_str());
					host_fprintf( outfile, "quaternion: %f %f %f %f\n---------------\n", quat.a, quat.b, quat.c, quat.d );
					host_fprintf( outfile, "start_quaternion: %f %f %f %f\n---------------\n", start_quat.a, start_quat.b, start_quat.c, start_quat.d );
					host_fprintf( outfile, "position: %f %f %f\n---------------\n", pos.x, pos.y, pos.z );
					host_fprintf( outfile, "start_position: %f %f %f\n---------------\n", start_pos.x, start_pos.y, start_pos.z );
					host_fprintf( outfile, "scale: %f\n---------------\n", scale );
					host_fprintf( outfile, "start_scale: %f\n---------------\n", start_scale );
				}
			}
		}
	}

	host_fclose( outfile );
}
