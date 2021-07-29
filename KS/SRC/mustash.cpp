/* mustash.cpp
 *
 * a 'wad'-style library to store multiple files in a single big file
 */

// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)

#include "global.h"
#include "users.h"
#include "osdevopts.h"
#include "osfile.h"
#include "semaphores.h"
#include "stringx.h"
#include "zip_filter.h"

#include "mustash.h"
#include "stash_support.h"
#include "filespec.h"
#include "avltree.h"

#ifdef TARGET_GC
#include "gc_arammgr.h"
#endif

#if defined(TARGET_XBOX)
#include "ngl.h" // nglPrintf.  Hey guys, check that return type
#endif /* TARGET_XBOX JIV DEBUG */

//  Some file io sutff.
#include "osfile.h"

#define STASHSEARCHREVERSED

//#define STASH_CHATTY_OPEN

#define STASH_MAGIC_NUMBER 0x5AFE
#define STASH_CURRENT_REVISION 4

const int BIG_ASS_BUFFER_SIZE = (11000000);

multistash stash::substash[STASH_LIMIT];
stash_id   stash::curstash=DEFAULTSTASH;

#if 0
/*** Initialization of static class members ***/
char stash::list_filename[MAX_STRING_LENGTH] = { '\0' };
char stash::substash.stash_filename[MAX_STRING_LENGTH] = { '\0' };

unsigned char *stash::substash.system_stored_buf = NULL;
unsigned stash::substash.system_stored_buf_end = 0;
unsigned stash::system_stored_buf_max = 0;

unsigned char *stash::substash.stored_buf = NULL;
unsigned stash::substash.stored_buf_end = 0;
unsigned stash::stored_buf_max = 0;

unsigned char *stash::substash.big_ass_buffer = NULL;
unsigned stash::substash.big_ass_buffer_max = 0;

unsigned char *stash::substash.temp_buf = NULL;
unsigned stash::substash.temp_buf_end = 0;
unsigned stash::temp_buf_max = 0;

bool stash::substash.stash_file_is_open = false;
stash::stash_t stash::type = PS2_STASH;

stash_header stash::substash.header;
AvlTree<stash_index_entry> stash::substash.index_tree;
AvlTree<stash_index_entry> stash::system_index_tree;
#endif


#ifdef REAL_ASYNC_LOAD
// Arguments to pass to the thread

threadArgs my_thread_args;


// A stack for the loading thread
#ifdef TARGET_XBOX
char da_stash[0x10000];
#else 
char da_stack[0x10000] __attribute__((aligned(16)));
#endif

#endif // REAL_ASYNC_LOAD


// pre-packed pstring-ifications for the various types
const pstring ps2mesh_type("ps2mesh"), xbmesh_type("xbmesh"), gcmesh_type("gcmesh");
const pstring unused_type("unused"), raw_type("raw"), anmx_type("anmx");
const pstring tex_type("tex"), snmx_type("snmx"), misc_type("misc"), unknown_type("unknown???");



/***************************** stash file api ********************************/

//////////////////////////////   open_stash   /////////////////////////////////
//
// Loads in a stash file from disk into memory
bool stash::open_stash(char *_stash_filename, stash_id stashid )
{
#ifndef USER_MKV
  debug_print("Opening stash %d: %s",stashid,_stash_filename);
#endif


//	bool is_system_stash= (stashid==system_stash);
  assert(substash[stashid].stored_buf == NULL);
  //assert((is_system_stash == false) || (substash[system_stash].stored_buf == NULL));
  assert(substash[stashid].stash_file_is_open == false);
  assert(_stash_filename != NULL);
  bool preserve_old_stored_buf = false;

  if ( strcmp(substash[stashid].stash_filename, _stash_filename) == 0 )
  {
    // preserve the substash[stashid].stored_buf (ie, don't reload it)
    preserve_old_stored_buf = true;
  }
  else
    strcpy(substash[stashid].stash_filename, _stash_filename);
  preserve_old_stored_buf = false;

  // read in stash file
  os_file stash_file;
  stringx stash_filename_str(substash[stashid].stash_filename);

  stash_file.open(stash_filename_str, os_file::FILE_READ);

  if (stash_file.is_open() == false)
    return false;
  int hsize = sizeof(substash[stashid].header); 
  stash_file.read(&substash[stashid].header, hsize);

  // check version
  if (substash[stashid].header.magic_number != STASH_MAGIC_NUMBER)
  {
    debug_print("Not a valid stash file.");
    stash_file.close();
    return false;
  }

  substash[stashid].acquire_stash_bufferspace(substash[stashid].header.stored_size);

  if (substash[stashid].header.revision != STASH_CURRENT_REVISION)
  {
    debug_print("warning, stash file %s is of version %d, and the current version is %d",
           substash[stashid].stash_filename, substash[stashid].header.revision, STASH_CURRENT_REVISION);
    debug_print("this should not cause any problems, but you should try to keep them synced.");
  }

#ifndef USER_MKV
  nglPrintf("\nLoading from stash file %s\n", substash[stashid].stash_filename);
  nglPrintf("\tmagic #    %11x\t\trevision  %12u\n", substash[stashid].header.magic_number, substash[stashid].header.revision);
  nglPrintf("\theader size    %7u\t\tindex entries %8u\n", sizeof(substash[stashid].header), substash[stashid].header.index_entries);
  nglPrintf("\tindex entry size    %u\t\tindex offset %9u\n", substash[stashid].header.index_entry_size, substash[stashid].header.index_offset);
  nglPrintf("\tstored offset %8u\t\tstored size %10u\n", substash[stashid].header.stored_offset, substash[stashid].header.stored_size);
  nglPrintf("\ttemp offset %10u\t\ttemp size %12u\n", substash[stashid].header.temp_offset, substash[stashid].header.temp_size);
  nglPrintf("\taram offset %10u\t\taram size %12u\n", substash[stashid].header.aram_offset, substash[stashid].header.aram_size);
#endif

  // read in the index list
//debug_print("\tindex seeking to %u, reading %u\n", substash[stashid].header.index_offset, substash[stashid].header.index_entry_size * substash[stashid].header.index_entries);
  stash_file.set_fp(substash[stashid].header.index_offset, os_file::FP_BEGIN);

// Begin index reading
// =================================

//low_level_console_print("%s - index", substash[stashid].stash_filename); llc_memory_log();
  unsigned buf_size = substash[stashid].header.index_entry_size * substash[stashid].header.index_entries;
  unsigned char *buf = (unsigned char *)memalign(16, buf_size);
  if (buf == NULL)
  {
    nglPrintf("Error allocating memory for index buffer (size %d)\n", buf_size);
  }

  unsigned index_read_size = substash[stashid].header.index_entry_size * substash[stashid].header.index_entries;
  unsigned ret = 0;
  if( substash[stashid].header.compression_flags & STASH_INDEX_COMP )
    ret = zip_filter::filter( &stash_file, substash[stashid].header.index_comp_size, buf, index_read_size );
  else
    ret = stash_file.read(buf, index_read_size);
  if( ret != index_read_size )
  {
    debug_print("Error reading index block, perhaps the file is bad?");
    free(buf);
    stash_file.close();
  }
  
  unsigned stash_index_copy_size = STASH_INDEX_ENTRY_SIZE;
  if (substash[stashid].header.index_entry_size < stash_index_copy_size)
  {
    assert(false);
    stash_index_copy_size = substash[stashid].header.index_entry_size;
  }

  for (unsigned i=0; i<substash[stashid].header.index_entries; ++i)
  {
    stash_index_entry *new_guy = NEW stash_index_entry;
    unsigned char *copy_from = (buf + (i*substash[stashid].header.index_entry_size));
    memcpy(new_guy, copy_from, stash_index_copy_size);
		new_guy->raw_data = NULL;
    if (substash[stashid].index_tree.find(new_guy))
	{
		stringx diagnosis(stringx::fmt, "new_guy = %s", new_guy->get_name());
		assertmsg(false, diagnosis.c_str());
	}
    //assert(new_guy->flag.is_stored == (new_guy->flag.file_type == stash_index_entry::STASH_INDEX_ENTRY_RAW ? false : true));
		#if 0
    if (is_system_stash)
      substash[stashid].system_index_tree.add(new_guy);
    else
		#endif
      substash[stashid].index_tree.add(new_guy);

#ifdef STASH_CHATTY_OPEN
    pstring the_type;
    switch (new_guy->file_type)
    {
      case stash_index_entry::STASH_INDEX_ENTRY_PS2MESH: the_type = "ps2mesh";    break;
      case stash_index_entry::STASH_INDEX_ENTRY_XBMESH:  the_type = "xbmesh";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_GCMESH:  the_type = "gcmesh";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_UNUSED:  the_type = "unused";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_RAW:     the_type = "raw";        break;
      case stash_index_entry::STASH_INDEX_ENTRY_ANMX:    the_type = "anmx";       break;
      case stash_index_entry::STASH_INDEX_ENTRY_TEX:     the_type = "tex";        break;
      case stash_index_entry::STASH_INDEX_ENTRY_SNMX:    the_type = "snmx";       break;
      case stash_index_entry::STASH_INDEX_ENTRY_MISC:    the_type = "misc";       break;
      default:                                           the_type = "unknown???"; break;
    }
    debug_print("\nHere's the substash[stashid].header for %s", new_guy->name.c_str());
    debug_print("\tfile offset %u\t\t\tis_stored 0x%x", new_guy->file_offset, new_guy->is_stored() );
    debug_print("\tentry size %u\t\t\ttype %s", new_guy->entry_size, the_type.c_str());
#endif
  }

  /*******************************
 *  END INDEX READ
 *******************************/
  
// low_level_console_print("%s - stored", substash[stashid].stash_filename); llc_memory_log();
  // read in the stored buffer
  if (substash[stashid].header.stored_size)
  {
    {
      // regular stored portion
      substash[stashid].stored_buf_end = substash[stashid].header.stored_size;

      assert(substash[stashid].big_ass_buffer != NULL);
      assert(substash[stashid].stored_buf_end <= substash[stashid].big_ass_buffer_max);
      substash[stashid].stored_buf = substash[stashid].big_ass_buffer;
      if ( preserve_old_stored_buf == false )
      {
        if (substash[stashid].stored_buf == NULL)
        {
          nglPrintf("Error allocating memory for stored buffer (size %d)\n", substash[stashid].stored_buf_end);
        }

        stash_file.set_fp(substash[stashid].header.stored_offset, os_file::FP_BEGIN);
        unsigned ret = 0;
        if( substash[stashid].header.compression_flags & STASH_STORED_COMP )
          ret = zip_filter::filter( &stash_file, substash[stashid].header.stored_comp_size,
			substash[stashid].stored_buf, substash[stashid].stored_buf_end );
        else
          ret = stash_file.read(substash[stashid].stored_buf, substash[stashid].stored_buf_end);
        if ((unsigned) ret != substash[stashid].stored_buf_end)
        {
          debug_print("error reading stored portion of the buffer");
          substash[stashid].stored_buf = NULL;
          substash[stashid].stored_buf_end = 0;
        }
      }
    }
//debug_print("\tstored seeking to %u, reading %u\n", substash[stashid].header.stored_offset, substash[stashid].stored_buf_end);
  }
  
//low_level_console_print("%s - temp", substash[stashid].stash_filename); llc_memory_log();
  // read in the temp buffer
  if (substash[stashid].header.temp_size)
  {
    {
      substash[stashid].temp_buf_end = substash[stashid].header.temp_size;
  //debug_print("\ttemp seeking to %u, reading %u", substash[stashid].header.temp_offset, substash[stashid].temp_buf_end);
      substash[stashid].temp_buf = (unsigned char *)malloc(substash[stashid].temp_buf_end);
      if (substash[stashid].temp_buf == NULL)
      {
        nglPrintf("Error allocating memory for temp buffer (size %d)\n", substash[stashid].temp_buf_end);
      }

      stash_file.set_fp(substash[stashid].header.temp_offset, os_file::FP_BEGIN);
      unsigned ret = 0;
      if( substash[stashid].header.compression_flags & STASH_TEMP_COMP )
        ret = zip_filter::filter( &stash_file, substash[stashid].header.temp_comp_size,
			substash[stashid].temp_buf, substash[stashid].temp_buf_end );
      else
        ret = stash_file.read(substash[stashid].temp_buf, substash[stashid].temp_buf_end);

      if( ret != substash[stashid].temp_buf_end )
      {
        free(substash[stashid].temp_buf);
				//memset(substash[stashid].temp_buf,0,substash[stashid].temp_buf_end);
        substash[stashid].temp_buf = NULL;
        substash[stashid].temp_buf_end = 0;
      }
    }
  }

  if( substash[stashid].header.aram_size )
  {
#ifndef TARGET_GC
    assert( 0 );
#else
    substash[stashid].aram_buf_end = substash[stashid].header.aram_size;
    substash[stashid].aram_id = aram_mgr::allocate( substash[stashid].aram_buf_end );
    if( substash[stashid].aram_id == aram_id_t( INVALID_ARAM_ID ) )
    {
      nglPrintf( "Error allocating aram memory for aram buffer (size %d)\n", substash[stashid].aram_buf_end );
    }
    stash_file.set_fp( substash[stashid].header.aram_offset, os_file::FP_BEGIN );
    bool ret = aram_mgr::aram_write( substash[stashid].aram_id, 0, &stash_file, substash[stashid].aram_buf_end );
    if( !ret )
    {
      aram_mgr::deallocate( substash[stashid].aram_id );
      substash[stashid].aram_id = INVALID_ARAM_ID;
      substash[stashid].aram_buf_end = 0;
    }
#endif
  }
  free( buf );

  stash_file.close();
#if 0
  if (is_system_stash == false)
#endif
    substash[stashid].stash_file_is_open = true;

//low_level_console_print("%s - done", substash[stashid].stash_filename); llc_memory_log();

#ifndef USER_MKV
  debug_print("Fin.");
#endif
	curstash=stashid;

  assert(substash[stashid].header.stored_size == 0 || substash[stashid].stored_buf != NULL );
  assert(substash[stashid].header.temp_size == 0 || substash[stashid].temp_buf != NULL);

  return true;
}

#ifdef TARGET_PS2
void Print_Stash_Node( os_file &output_file, AvlNode *curr)
{

	//  Check to see if this was used.  If not, then record it.
	if (!curr->data()->was_used())
	{
		output_file.write((void *)curr->data()->get_name(), strlen((char *)curr->data()->get_name()));
		output_file.write((void *)"\n",1);
	}

	//  Now check its children.
	if (curr->left() != NULL)
		Print_Stash_Node(output_file, curr->left());

	if (curr->right() != NULL)
		Print_Stash_Node(output_file, curr->right());
}

//  This is a tool for stash clean up.  Enable them, then run every level in the game once.
//  You'll get a file [stash filename].xtr (as in "extra") in your (or diskimg*) directory with a
//  list of everything in the stash that is not actually used.
void Write_Extra_Stash(multistash *the_stash)
{
	// Open the file and write to it, appending to the end of what was there before.
	os_file stash_extra_file;
	char file_name[100];

	if (!the_stash->is_stash_open())
		return;

	strcpy(file_name, (char *)the_stash->get_stash_filename());
	int last_char = strlen(file_name) - 1;

	if (last_char == -1)  //  no name
		return;

	//  change the extention.
	file_name[last_char - 2] = 'x';
	file_name[last_char - 1] = 't';
	file_name[last_char] = 'r';

	//  add something onto the front so that it goes to the your diskimg directory.
	while (strchr(file_name, '\\'))
	{
		strcpy(file_name, strchr(file_name, '\\') + 1);
	}

	stash_extra_file.open(file_name, os_file::FILE_WRITE);

	//  Name of the file at the top.
	stash_extra_file.write((void *)file_name, strlen(file_name));
	stash_extra_file.write((void *)"\n\n",2);

	//  Now look at everything file listed in the stash.  If it was not loaded, then list it.
	Print_Stash_Node(stash_extra_file, the_stash->get_index_tree().root());
	stash_extra_file.write((void *)"\n\n\n\n",4);

	stash_extra_file.close();
}
#endif // #ifdef TARGET_PS2



//////////////////////////////  LoadStashAsync   /////////////////////////////////
//
// This is launches a separate thread.  it takes as parameters:
//  _stash_filename   - the stash to open
//  stashid           - which stash to use
//  progressCallback  - a callback which is passed some userdata and an int
//                      the int is -1 for error, 100 for success, percent done otherwise
//  userData          - the data passed to the callback
//


void stash::LoadStashAsync(char *stashName, stash_id stashid, KSHeapIDs heap, void (*progressCallback)(stash_id stashid, void *userData, int), void *userData)
{
#if defined(REAL_ASYNC_LOAD) && !defined(TARGET_GC)


  GETSEMA(LoadNewStashSema);

#ifdef TARGET_PS2
  ThreadParam th_param;
    // Set thread properties
  th_param.entry = stash::LoadStashAsyncHelper;
  th_param.stack = da_stack;
  th_param.stackSize = 0x10000;
  th_param.initPriority = 1;
  th_param.gpReg = &_gp;
  th_param.option = 0;

  // Set thread arguments

  int dathread;
#elif defined(TARGET_XBOX)
  HANDLE dathread;  
  int dwThrdParam = 1;
  int threadId;
#else

#endif

  my_thread_args.stashid           = stashid;
  my_thread_args.userData          = userData;
  my_thread_args.heapid            = heap;
  my_thread_args.progressCallback  = progressCallback;
  strcpy(my_thread_args.name, stashName);



  // Create the thread
#ifdef TARGET_XBOX
  dathread = CreateThread(
    NULL,
    0,
    (LPTHREAD_START_ROUTINE)stash::LoadStashAsyncHelper,
    &my_thread_args,
    0,
    (LPDWORD)&threadId);

#elif defined(TARGET_PS2)
  dathread = CreateThread(&th_param);
#endif


  if (dathread < 0)
  {
	  progressCallback(stashid, userData, -1);

    RELEASESEMA(LoadNewStashSema);

    return;
  }
#ifdef TARGET_PS2
  // Start the thread
  if (StartThread(dathread, &my_thread_args) < 0)
  {
	  progressCallback(stashid, userData, -1);

    RELEASESEMA(LoadNewStashSema);

    return;
  }
#endif

#endif
}
//////////////////////////////  LoadStashAsyncHelper   /////////////////////////////////
//
// This is designed to run as a separate thread.  it takes as a parameter:
//  args - the arguements passed to LoadStashAsync packaged up
// This thread is expecting to run at the same priority as the main thread.

#if !defined(TARGET_GC)

void stash::LoadStashAsyncHelper(threadArgs *args)
{


  char *_stash_filename = args->name;
  stash_id stashid = args->stashid;
  int heapid       = args->heapid;
  void (*progressCallback)(stash_id stashid, void *userData, int) = args->progressCallback;
  void *userData = args->userData;

#ifdef TARGET_PS2
  ThreadParam p;
  ReferThreadStatus(GetThreadId(), &p);
  int priority = p.currentPriority;
#endif
  int oldReadSize = zip_filter::get_read_size();


  zip_filter::set_read_size(substash[stashid].async_read_size);



  unsigned stash_index_copy_size = STASH_INDEX_ENTRY_SIZE;

  stash::close_stash_async(stashid);
  stash::free_stored(stashid);
  
  PASSTHREADCONTROL(priority);


//	bool is_system_stash= (stashid==system_stash);
  assert(substash[stashid].stored_buf == NULL);
  //assert((is_system_stash == false) || (substash[system_stash].stored_buf == NULL));
  assert(substash[stashid].stash_file_is_open == false);
  assert(_stash_filename != NULL);
  bool preserve_old_stored_buf = false;

  strcpy(substash[stashid].stash_filename, _stash_filename);
  preserve_old_stored_buf = false;

  os_file stash_file;
  stringx stash_filename_str(substash[stashid].stash_filename);
  substash[stashid].async_stash_file.open(stash_filename_str, os_file::FILE_READ);
  if (substash[stashid].async_stash_file.is_open() == false)
  {
    

    close_stash_async(stashid);
    zip_filter::set_read_size(oldReadSize);

    RELEASESEMA(LoadNewStashSema);
    progressCallback(stashid, userData, -1);
    EXITTHREAD;
    return;
  }

  int hsize = sizeof(substash[stashid].header);
  substash[stashid].async_stash_file.read(&substash[stashid].header, hsize);
  if (substash[stashid].abort_stash_read || (substash[stashid].header.magic_number != STASH_MAGIC_NUMBER))
  {
    

    close_stash_async(stashid);
    zip_filter::set_read_size(oldReadSize);

    RELEASESEMA(LoadNewStashSema);

    progressCallback(stashid, userData, -1);

    EXITTHREAD;
    return;
  }
  mem_push_current_heap(heapid);
  substash[stashid].acquire_stash_bufferspace(substash[stashid].header.stored_size);
  mem_pop_current_heap();

  if (substash[stashid].big_ass_buffer == NULL || substash[stashid].abort_stash_read)
  {
    
    close_stash_async(stashid);
    zip_filter::set_read_size(oldReadSize);
    RELEASESEMA(LoadNewStashSema);
    progressCallback(stashid, userData, -1);

    EXITTHREAD;
    return;
  }


  
  PASSTHREADCONTROL(priority);
  


  nglPrintf("\nLoading from stash file %s\n", substash[stashid].stash_filename);
  nglPrintf("\tmagic #    %11x\t\trevision  %12u\n", substash[stashid].header.magic_number, substash[stashid].header.revision);
  nglPrintf("\theader size    %7u\t\tindex entries %8u\n", sizeof(substash[stashid].header), substash[stashid].header.index_entries);
  nglPrintf("\tindex entry size    %u\t\tindex offset %9u\n", substash[stashid].header.index_entry_size, substash[stashid].header.index_offset);
  nglPrintf("\tstored offset %8u\t\tstored size %10u\n", substash[stashid].header.stored_offset, substash[stashid].header.stored_size);
  nglPrintf("\ttemp offset %10u\t\ttemp size %12u\n", substash[stashid].header.temp_offset, substash[stashid].header.temp_size);
  nglPrintf("\taram offset %10u\t\taram size %12u\n", substash[stashid].header.aram_offset, substash[stashid].header.aram_size);

  substash[stashid].async_stash_file.set_fp(substash[stashid].header.index_offset, os_file::FP_BEGIN);
  if (substash[stashid].abort_stash_read)
  {
    
    close_stash_async(stashid);
    zip_filter::set_read_size(oldReadSize);
    RELEASESEMA(LoadNewStashSema);
    progressCallback(stashid, userData,-1);

    EXITTHREAD;
    return;
  }
  unsigned buf_size = substash[stashid].header.index_entry_size * substash[stashid].header.index_entries;
  mem_push_current_heap(heapid);
  unsigned char *buf = (unsigned char *)malloc(buf_size);
  mem_pop_current_heap();
  if (buf == NULL || substash[stashid].abort_stash_read)
  {
    if (buf)
      free(buf);
    
    close_stash_async(stashid);
    zip_filter::set_read_size(oldReadSize);
    RELEASESEMA(LoadNewStashSema);
    progressCallback(stashid, userData, -1);

    EXITTHREAD;
    return;
  }

  unsigned index_read_size = substash[stashid].header.index_entry_size * substash[stashid].header.index_entries;
  unsigned ret = 0;

  mem_push_current_heap(heapid);
  if( substash[stashid].header.compression_flags & STASH_INDEX_COMP )
    ret = zip_filter::filter( &substash[stashid].async_stash_file, substash[stashid].header.index_comp_size, buf, index_read_size );
  else
    ret = substash[stashid].async_stash_file.read(buf, index_read_size);
  mem_pop_current_heap();

  if( ret != index_read_size || substash[stashid].abort_stash_read)
  {
    debug_print("Error reading index block, perhaps the file is bad?");
    free(buf);

    
    close_stash_async(stashid);
    zip_filter::set_read_size(oldReadSize);
    RELEASESEMA(LoadNewStashSema);
    progressCallback(stashid, userData, -1);

    EXITTHREAD;
    return;
  }


  
  PASSTHREADCONTROL(priority);
  


  for (unsigned i=0; i<substash[stashid].header.index_entries; ++i)
  {
    mem_push_current_heap(heapid);
    stash_index_entry *new_guy = NEW stash_index_entry;
    mem_pop_current_heap();
    unsigned char *copy_from = (buf + (i*substash[stashid].header.index_entry_size));
    memcpy(new_guy, copy_from, stash_index_copy_size);
    new_guy->raw_data = NULL;
    if (substash[stashid].index_tree.find(new_guy))
	{
		stringx diagnosis(stringx::fmt, "new_guy = %s", new_guy->get_name());
		assertmsg(false, diagnosis.c_str());
	}
    //assert(new_guy->flag.is_stored == (new_guy->file_type == stash_index_entry::STASH_INDEX_ENTRY_RAW ? false : true));
		#if 0
    if (is_system_stash)
      substash[stashid].system_index_tree.add(new_guy);
    else
		#endif
      mem_push_current_heap(heapid);
      substash[stashid].index_tree.add(new_guy);
      mem_pop_current_heap();
#ifdef STASH_CHATTY_OPEN
    pstring the_type;
    switch (new_guy->file_type)
    { 
      case stash_index_entry::STASH_INDEX_ENTRY_PS2MESH: the_type = "ps2mesh";    break;
      case stash_index_entry::STASH_INDEX_ENTRY_XBMESH:  the_type = "xbmesh";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_GCMESH:  the_type = "gcmesh";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_UNUSED:  the_type = "unused";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_RAW:     the_type = "raw";        break;
      case stash_index_entry::STASH_INDEX_ENTRY_ANMX:    the_type = "anmx";       break;
      case stash_index_entry::STASH_INDEX_ENTRY_TEX:     the_type = "tex";        break;
      case stash_index_entry::STASH_INDEX_ENTRY_SNMX:    the_type = "snmx";       break;
      case stash_index_entry::STASH_INDEX_ENTRY_MISC:    the_type = "misc";       break;
      default:                                           the_type = "unknown???"; break;
    }
    debug_print("\nHere's the substash[stashid].header for %s", new_guy->name.c_str());
    debug_print("\tfile offset %u\t\t\tis_stored 0x%x", new_guy->file_offset, new_guy->is_stored());
    debug_print("\tentry size %u\t\t\ttype %s", new_guy->entry_size, the_type.c_str());
#endif
    if (i%5 == 0)
    {

      
  PASSTHREADCONTROL(priority);      


      if( substash[stashid].abort_stash_read )
      {
        free(buf);

        

        close_stash_async(stashid);
        zip_filter::set_read_size(oldReadSize);
        RELEASESEMA(LoadNewStashSema);
        progressCallback(stashid, userData, -1);

        EXITTHREAD;
        return;
      }
    }
  }


  
  PASSTHREADCONTROL(priority);
  




  if (substash[stashid].header.stored_size)
  {
    // regular stored portion
    substash[stashid].stored_buf_end = substash[stashid].header.stored_size;

    assert(substash[stashid].big_ass_buffer != NULL);
    assert(substash[stashid].stored_buf_end <= substash[stashid].big_ass_buffer_max);
    substash[stashid].stored_buf = substash[stashid].big_ass_buffer;
    if ( preserve_old_stored_buf == false )
    {
      if (substash[stashid].stored_buf == NULL || substash[stashid].abort_stash_read)
      {
        nglPrintf("Error allocating memory for stored buffer (size %d)\n", substash[stashid].stored_buf_end);
        
        free(buf);
        close_stash_async(stashid);
        zip_filter::set_read_size(oldReadSize);
        RELEASESEMA(LoadNewStashSema);
        progressCallback(stashid, userData, -1);

        EXITTHREAD;
        return;
      }
    }
  }

  substash[stashid].async_current_stored_point = 0;
  substash[stashid].async_current_temp_point  = 0;

  if (substash[stashid].header.temp_size)
  {
    substash[stashid].temp_buf_end = substash[stashid].header.temp_size;
//debug_print("\ttemp seeking to %u, reading %u", substash[stashid].header.temp_offset, substash[stashid].temp_buf_end);
    mem_push_current_heap(heapid);
    substash[stashid].temp_buf = (unsigned char *)malloc(substash[stashid].temp_buf_end);
    mem_pop_current_heap();
    if (substash[stashid].temp_buf == NULL || substash[stashid].abort_stash_read)
    {
      nglPrintf("Error allocating memory for stored buffer (size %d)\n", substash[stashid].stored_buf_end);
      
      free(buf);
      close_stash_async(stashid);
      zip_filter::set_read_size(oldReadSize);
      progressCallback(stashid, userData, -1);
      RELEASESEMA(LoadNewStashSema);
      EXITTHREAD;
      return;
    }
  }


 
  PASSTHREADCONTROL(priority);
  




  substash[stashid].async_stash_file.set_fp(substash[stashid].header.stored_offset, os_file::FP_BEGIN);
  free( buf );
  substash[stashid].pre_opened  = true;
  substash[stashid].opened = false;
  substash[stashid].first_pass_temp = true;
  substash[stashid].first_pass_stored = true;
  //set_async_read_size(stashid, 256*1024);
  mem_push_current_heap(heapid);
  bool retval = read_stash_async(stashid);
  mem_pop_current_heap();
  while(!retval)
  {

    
  PASSTHREADCONTROL(priority);
    

    if (substash[stashid].abort_stash_read)
    {
      close_stash_async(stashid);
      
      zip_filter::set_read_size(oldReadSize);
  
      RELEASESEMA(LoadNewStashSema);
      progressCallback(stashid, userData, -1);

      EXITTHREAD;
      return;
    }
    mem_push_current_heap(heapid);
    retval = read_stash_async(stashid);
    mem_pop_current_heap();
  }
  substash[stashid].abort_stash_read = false;
  
  zip_filter::set_read_size(oldReadSize);

  curstash = stashid;

  progressCallback(stashid, userData, 100);
  RELEASESEMA(LoadNewStashSema);


  EXITTHREAD;
  return;
}

#else  // Xbox or
void stash::LoadStashAsyncHelper(threadArgs *args)
{

}
#endif


#ifdef REAL_ASYNC_LOAD
//////////////////////////////  WaitForStashLoad   /////////////////////////////////
//  Simply waits for the stash to load.

void stash::WaitForStashLoad()
{

  GETSEMA(LoadNewStashSema);

  RELEASESEMA(LoadNewStashSema);
}

//////////////////////////////  AbortAsyncRead   /////////////////////////////////
//  aborts call to LoadStashAsync.  The Semaphore stuff ensures this does not exit
//  until loading is truly aborted
//  This is called from the main thread

void stash::AbortAsyncRead(stash_id stashid)
{
  substash[stashid].abort_stash_read = true;

  // If the sema is locked
  if (!POLLSEMA(LoadNewStashSema))
  {
    // Wait til we abort

    GETSEMA(LoadNewStashSema);

    RELEASESEMA(LoadNewStashSema);

    PASSTHREADCONTROL(1);
  }
  else
  {
    RELEASESEMA(LoadNewStashSema);
  }
  substash[stashid].abort_stash_read = false;

}

#endif


//////////////////////////////  close_stash   /////////////////////////////////
//
// Free's up the temp and index portions of the stash.  The stored portion is left in-tact until
// free_stored is called.
void stash::close_stash( stash_id stashid )
{
#ifdef TARGET_PS2
	//  Debugging thing.
  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_EXTRA_STASH_OUTPUT))
    Write_Extra_Stash(&substash[stashid]);
#endif // #ifdef TARGET_PS2

#ifndef USER_MKV
  debug_print("Closing stash %d",stashid);
#endif
	
  substash[stashid].close_stash();

	#if 0
  if (substash[stashid].temp_buf != NULL)
    free(substash[stashid].temp_buf);
  substash[stashid].temp_buf = NULL;
	//substash[stashid].release_stash_bufferspace();

  substash[stashid].stash_file_is_open = false;
	#endif
}

/////////////////////////  pre_open_stash_for_async   /////////////////////////
//
// Does the one time opening parts of reading a stash.
// Things like allocating memory, reading the header, etc.
// returns the size of the stash
int stash::pre_open_stash_for_async( char *_stash_filename, stash_id stashid )
{
#ifndef USER_MKV
  debug_print("Opening stash %d: %s",stashid,_stash_filename);
#endif



//	bool is_system_stash= (stashid==system_stash);
  assert(substash[stashid].stored_buf == NULL);
  //assert((is_system_stash == false) || (substash[system_stash].stored_buf == NULL));
  assert(substash[stashid].stash_file_is_open == false);
  assert(_stash_filename != NULL);
  bool preserve_old_stored_buf = false;

  preserve_old_stored_buf = false;

  strcpy(substash[stashid].stash_filename, _stash_filename);

  // read in stash file
  stringx stash_filename_str(substash[stashid].stash_filename);
  substash[stashid].async_stash_file.open(stash_filename_str, os_file::FILE_READ);
  if (substash[stashid].async_stash_file.is_open() == 0)
    return 0;
  int hsize = sizeof(substash[stashid].header);
  substash[stashid].async_stash_file.read(&substash[stashid].header, hsize);

  // check version
  if (substash[stashid].header.magic_number != STASH_MAGIC_NUMBER)
  {
    debug_print("Not a valid stash file.");
    substash[stashid].async_stash_file.close();
    return 0;
  }
  
  
  substash[stashid].acquire_stash_bufferspace(substash[stashid].header.stored_size);
  
  if (substash[stashid].header.revision != STASH_CURRENT_REVISION)
  {
    debug_print("warning, stash file %s is of version %d, and the current version is %d",
           substash[stashid].stash_filename, substash[stashid].header.revision, STASH_CURRENT_REVISION);
    debug_print("this should not cause any problems, but you should try to keep them synced.");
  }

  nglPrintf("\nLoading from stash file %s\n", substash[stashid].stash_filename);
  nglPrintf("\tmagic #    %11x\t\trevision  %12u\n", substash[stashid].header.magic_number, substash[stashid].header.revision);
  nglPrintf("\theader size    %7u\t\tindex entries %8u\n", sizeof(substash[stashid].header), substash[stashid].header.index_entries);
  nglPrintf("\tindex entry size    %u\t\tindex offset %9u\n", substash[stashid].header.index_entry_size, substash[stashid].header.index_offset);
  nglPrintf("\tstored offset %8u\t\tstored size %10u\n", substash[stashid].header.stored_offset, substash[stashid].header.stored_size);
  nglPrintf("\ttemp offset %10u\t\ttemp size %12u\n", substash[stashid].header.temp_offset, substash[stashid].header.temp_size);
  nglPrintf("\taram offset %10u\t\taram size %12u\n", substash[stashid].header.aram_offset, substash[stashid].header.aram_size);

  // read in the index list
//debug_print("\tindex seeking to %u, reading %u\n", substash[stashid].header.index_offset, substash[stashid].header.index_entry_size * substash[stashid].header.index_entries);
  substash[stashid].async_stash_file.set_fp(substash[stashid].header.index_offset, os_file::FP_BEGIN);

//low_level_console_print("%s - index", substash[stashid].stash_filename); llc_memory_log();
  unsigned buf_size = substash[stashid].header.index_entry_size * substash[stashid].header.index_entries;
  unsigned char *buf = (unsigned char *)malloc(buf_size);
  if (buf == NULL)
  {
    nglPrintf("Error allocating memory for index buffer (size %d)\n", buf_size);
  }
  unsigned index_read_size = substash[stashid].header.index_entry_size * substash[stashid].header.index_entries;
  unsigned ret = 0;


  
  if( substash[stashid].header.compression_flags & STASH_INDEX_COMP )
    ret = zip_filter::filter( &substash[stashid].async_stash_file, substash[stashid].header.index_comp_size, buf, index_read_size );
  else
    ret = substash[stashid].async_stash_file.read(buf, index_read_size);
  


  if( ret != index_read_size )
  {
    debug_print("Error reading index block, perhaps the file is bad?");
    free(buf);
    substash[stashid].async_stash_file.close();
  }

  unsigned stash_index_copy_size = STASH_INDEX_ENTRY_SIZE;
  if (substash[stashid].header.index_entry_size < stash_index_copy_size)
  {
    assert(false);
    stash_index_copy_size = substash[stashid].header.index_entry_size;
  }

//  int counter = 0;
  for (unsigned i=0; i<substash[stashid].header.index_entries; ++i)
  {
    
    stash_index_entry *new_guy = NEW stash_index_entry;
    
    unsigned char *copy_from = (buf + (i*substash[stashid].header.index_entry_size));
    memcpy(new_guy, copy_from, stash_index_copy_size);
    new_guy->raw_data = NULL;
    if (substash[stashid].index_tree.find(new_guy))
	{
		stringx diagnosis(stringx::fmt, "new_guy = %s", new_guy->get_name());
		assertmsg(false, diagnosis.c_str());
	}
    //assert(new_guy->flag.is_stored == (new_guy->flag.file_type == stash_index_entry::STASH_INDEX_ENTRY_RAW ? false : true));
		#if 0
    if (is_system_stash)
      substash[stashid].system_index_tree.add(new_guy);
    else
		#endif
      substash[stashid].index_tree.add(new_guy);
#ifdef STASH_CHATTY_OPEN
    pstring the_type;
    switch (new_guy->file_type)
    {
      case stash_index_entry::STASH_INDEX_ENTRY_PS2MESH: the_type = "ps2mesh";    break;
      case stash_index_entry::STASH_INDEX_ENTRY_XBMESH:  the_type = "xbmesh";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_GCMESH:  the_type = "gcmesh";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_UNUSED:  the_type = "unused";     break;
      case stash_index_entry::STASH_INDEX_ENTRY_RAW:     the_type = "raw";        break;
      case stash_index_entry::STASH_INDEX_ENTRY_ANMX:    the_type = "anmx";       break;
      case stash_index_entry::STASH_INDEX_ENTRY_TEX:     the_type = "tex";        break;
      case stash_index_entry::STASH_INDEX_ENTRY_SNMX:    the_type = "snmx";       break;
      case stash_index_entry::STASH_INDEX_ENTRY_MISC:    the_type = "misc";       break;
      default:                                           the_type = "unknown???"; break;
    }
    debug_print("\nHere's the substash[stashid].header for %s", new_guy->name.c_str());
    debug_print("\tfile offset %u\t\t\tis_stored 0x%x", new_guy->file_offset, new_guy->is_stored());
    debug_print("\tentry size %u\t\t\ttype %s", new_guy->entry_size, the_type.c_str());
#endif

  }
  if (substash[stashid].header.stored_size)
  {
    // regular stored portion
    substash[stashid].stored_buf_end = substash[stashid].header.stored_size;

    assert(substash[stashid].big_ass_buffer != NULL);
    assert(substash[stashid].stored_buf_end <= substash[stashid].big_ass_buffer_max);
    substash[stashid].stored_buf = substash[stashid].big_ass_buffer;
    if ( preserve_old_stored_buf == false )
    {
      if (substash[stashid].stored_buf == NULL)
      {
        nglPrintf("Error allocating memory for stored buffer (size %d)\n", substash[stashid].stored_buf_end);
      }
    }
  }

  substash[stashid].async_current_stored_point = 0;
  substash[stashid].async_current_temp_point  = 0;
  substash[stashid].async_current_aram_point  = 0;
  if (substash[stashid].header.temp_size)
  {
    substash[stashid].temp_buf_end = substash[stashid].header.temp_size;
//debug_print("\ttemp seeking to %u, reading %u", substash[stashid].header.temp_offset, substash[stashid].temp_buf_end);
    substash[stashid].temp_buf = (unsigned char *)malloc(substash[stashid].temp_buf_end);
    if (substash[stashid].temp_buf == NULL)
    {
      nglPrintf("Error allocating memory for temp buffer (size %d)\n", substash[stashid].temp_buf_end);
    }
  }

  if( substash[stashid].header.aram_size )
  {
#ifndef TARGET_GC
    assert( "You shouldn't have an aram buffer!" && 0 );
#else
    substash[stashid].aram_buf_end = substash[stashid].header.aram_size;
    substash[stashid].aram_id = aram_mgr::allocate( substash[stashid].aram_buf_end );
    if( substash[stashid].aram_id == aram_id_t( INVALID_ARAM_ID ) )
    {
      nglPrintf( "Error allocating memory for aram buffer (size %d)\n", substash[stashid].aram_buf_end );
    }
#endif
  }
  substash[stashid].async_stash_file.set_fp(substash[stashid].header.stored_offset, os_file::FP_BEGIN);
  free( buf );
  substash[stashid].pre_opened  = true;
  substash[stashid].opened = false;
  substash[stashid].first_pass_temp = true;
  substash[stashid].first_pass_stored = true;
  substash[stashid].first_pass_aram = true;
	curstash=stashid;
//  return substash[stashid].header.stored_comp_size;
  if (substash[stashid].header.compression_flags & STASH_STORED_COMP)
    return substash[stashid].header.stored_comp_size + substash[stashid].header.temp_comp_size;
  else
    return substash[stashid].header.stored_size + substash[stashid].header.temp_size;
}


float stash::get_async_progress_percent( stash_id stashid )
{
  return ((float)(substash[stashid].async_current_stored_point + substash[stashid].async_current_stored_point))/(float)(substash[stashid].header.stored_comp_size);
}

int stash::get_async_progress_bytes( stash_id stashid )
{
  return (substash[stashid].async_current_stored_point + substash[stashid].async_current_stored_point);
}
/////////////////////////  read_stash_async   /////////////////////////
//
// Reads async_read_size bytes of the stash.
// returns true when done.
static int current_temp_buff_prog = 0;
static int current_stored_buff_prog = 0;
static int current_aram_buff_prog = 0;

bool stash::read_stash_async( stash_id stashid )
{
  int this_read_size;
  bool preserve_old_stored_buf = false;
  if (substash[stashid].stash_file_is_open)
    return true;

  // The stashes have 2 parts - temp and stored.  We read the stored buffer first, then the temp.
  preserve_old_stored_buf = false;
  assert( substash[stashid].pre_opened && "MUST PREOPEN STASHES BEFORE ASYNC READING" );

  // Is it compressed?
  // Pick the read size accordingly

  if( substash[stashid].header.compression_flags & STASH_STORED_COMP )
  {
    if (substash[stashid].async_read_size < (substash[stashid].header.stored_comp_size - substash[stashid].async_current_stored_point))
      this_read_size = substash[stashid].async_read_size;
    else
      this_read_size = substash[stashid].header.stored_comp_size - substash[stashid].async_current_stored_point;
  }
  else
  {
    if (substash[stashid].async_read_size < (substash[stashid].header.stored_size - substash[stashid].async_current_stored_point))
      this_read_size = substash[stashid].async_read_size;
    else
      this_read_size = substash[stashid].header.stored_size - substash[stashid].async_current_stored_point;
  }

  // If this_read_size > 0, then we go ahead and read in some of the
  // data from the stored portion of the stash.
  if (this_read_size > 0)
  {
    // If its the first pass, we do a little initing
    if (substash[stashid].first_pass_stored)
    {
      // Setup the zip_filter
      zip_filter::init_async(&substash[stashid].async_stash_file,
		                         substash[stashid].stored_buf,
                             substash[stashid].stored_buf_end);
      current_stored_buff_prog = 0;
      // Set the file pointer
      substash[stashid].async_stash_file.set_fp(substash[stashid].header.stored_offset, os_file::FP_BEGIN);

      substash[stashid].first_pass_stored = false;
    }

    if (substash[stashid].header.stored_size)
    {
      if ( preserve_old_stored_buf == false )
      {
        // Actually read in this_read_size bytes
        unsigned ret = 0;
        if( substash[stashid].header.compression_flags & STASH_STORED_COMP )
          zip_filter::async_filter( &substash[stashid].async_stash_file, this_read_size);
        else
          ret = substash[stashid].async_stash_file.read(substash[stashid].stored_buf + current_stored_buff_prog, this_read_size);

      }

    }
    // Update the current amount
    substash[stashid].async_current_stored_point += this_read_size;
    current_stored_buff_prog += this_read_size;
  }
  else if (this_read_size == 0) // No more to read from stored
  {
    // if its the first pass
    if (substash[stashid].first_pass_temp)
    {
      // If we had a first run before
      // shutdown that zip_filter run
      if (!substash[stashid].first_pass_stored)
        zip_filter::shutdown_async();

      // Move the fp
      substash[stashid].async_stash_file.set_fp(substash[stashid].header.temp_offset, os_file::FP_BEGIN);
      
      // Setup the zip_filter
      zip_filter::init_async(&substash[stashid].async_stash_file,
		                         substash[stashid].temp_buf, substash[stashid].temp_buf_end);
      substash[stashid].first_pass_temp = false;
      current_temp_buff_prog = 0;
    }

    // Is it compressed?
    // Pick the read size accordingly
    if (substash[stashid].header.compression_flags & STASH_TEMP_COMP )
    {
      if (substash[stashid].async_read_size < (substash[stashid].header.temp_comp_size - substash[stashid].async_current_temp_point))
        this_read_size = substash[stashid].async_read_size;
      else
        this_read_size = substash[stashid].header.temp_comp_size - substash[stashid].async_current_temp_point;
    }
    else
    {
      if (substash[stashid].async_read_size < (substash[stashid].header.temp_size - substash[stashid].async_current_temp_point))
        this_read_size = substash[stashid].async_read_size;
      else
        this_read_size = substash[stashid].header.temp_size - substash[stashid].async_current_temp_point;
    }

    // If we have data to read
    if (this_read_size > 0)
    {

      if (substash[stashid].header.temp_size)
      {
        unsigned ret = 0;
        if( substash[stashid].header.compression_flags & STASH_TEMP_COMP )
          zip_filter::async_filter( &substash[stashid].async_stash_file, this_read_size );
        else
          ret = substash[stashid].async_stash_file.read(substash[stashid].temp_buf + current_temp_buff_prog, this_read_size);
      }
      current_temp_buff_prog += this_read_size;
      substash[stashid].async_current_temp_point += this_read_size;
    }
    else if (this_read_size == 0) // No more to read
    {
      // lets go deeper just for the fun of it
      // reading aram buffer now
      if( substash[stashid].first_pass_aram )
      {
        substash[stashid].async_stash_file.set_fp( substash[stashid].header.aram_offset, os_file::FP_BEGIN );
        substash[stashid].first_pass_aram = false;
        current_aram_buff_prog = 0;
      }
      
      if (substash[stashid].async_read_size < (substash[stashid].header.aram_size - substash[stashid].async_current_aram_point))
        this_read_size = substash[stashid].async_read_size;
      else
        this_read_size = substash[stashid].header.aram_size - substash[stashid].async_current_aram_point;
      
      if( this_read_size > 0 )
      {
#ifdef TARGET_GC
        bool b = aram_mgr::aram_write( substash[stashid].aram_id, substash[stashid].async_current_aram_point, &substash[stashid].async_stash_file, this_read_size );
        if( !b )
        {
          assert( "read failure!" && 0 );
        }
        current_aram_buff_prog += this_read_size;
        substash[stashid].async_current_aram_point += this_read_size;
#else
        assert( "We shouldn't have an aram buffer!" && 0 );
#endif //TARGET_GC
      }
      else if( this_read_size == 0 )
      {
        substash[stashid].opened = true;
      }
    }
    else
    {
      assert (this_read_size < 0 && "READING < 0 bytes!");
    }
  }
  else // this_read_size < 0
  {
    assert (this_read_size  < 0 && "READING < 0 bytes!");
  }


  if (substash[stashid].opened) // if we're done
  {
    // shutdown the zip_filter & close the file
    if (!substash[stashid].first_pass_stored || !substash[stashid].first_pass_temp)
      zip_filter::shutdown_async();

    substash[stashid].async_stash_file.close();
  
    // Set this flag
    substash[stashid].stash_file_is_open = true;
#ifndef USER_MKV
    debug_print("Fin.");
#endif
    // return true meaning we have NO more to read
    return true;
  }

  // return false meaning we have more to read
  return false;


}

/////////////////////////  set_async_read_size   /////////////////////////
//
// Sets how much we read per frame for a stash
void stash::set_async_read_size( stash_id stashid, unsigned int read_size)
{
  substash[stashid].async_read_size = read_size;
}

void stash::close_stash_async( stash_id stashid )
{

  substash[stashid].first_pass_temp = true;
  substash[stashid].first_pass_stored = true;
  substash[stashid].abort_stash_read = false;

  if (!substash[stashid].stash_file_is_open)
  {
    zip_filter::shutdown_async();
  }
  if (substash[stashid].async_stash_file.is_open())
    substash[stashid].async_stash_file.close();

  close_stash(stashid);
  free_stored(stashid);

}



void multistash::close_stash( void )
{
  if (temp_buf != NULL)
    free(temp_buf);
  temp_buf = NULL;
	//substash[stashid].release_stash_bufferspace();
  pre_opened = false;
  stash_file_is_open = false;
}

/*** free_stored ***/
// Free's up the stored portion of the stash.  Should be called before opening a NEW stash, but
// not while there are still users of the stored portion.
void stash::free_stored( stash_id stashid )
{
	//for ( int i=STASH_SYSTEM; i<STASH_LIMIT; i++ )
	{
		substash[stashid].free_stored();
	}
	#if 0
	substash[stashid].release_stash_bufferspace();
  if (substash[stashid].index_tree.size() > 0)
    substash[stashid].index_tree.dispose();
  substash[stashid].stored_buf = NULL;
  substash[stashid].stored_buf_end = 0;
  substash[stashid].stash_file_is_open = false;
	#endif
}

void multistash::free_stored( void )
{
	close_stash();
	release_stash_bufferspace();
  if (index_tree.size() > 0)
    index_tree.dispose();
  stored_buf = NULL;
  stored_buf_end = 0;
#ifdef TARGET_GC
  if( aram_id != INVALID_ARAM_ID )
  {
    aram_mgr::deallocate( aram_id );
    aram_id = INVALID_ARAM_ID;
  }
#endif
  stash_file_is_open = false;
  memset(&header, 0, sizeof(header));
}

////////////////////////// acquire_stash_bufferspace /////////////////////////////
//
void stash::acquire_stash_bufferspace(int size)
{
	for ( int i=STASH_SYSTEM; i<STASH_LIMIT; i++ )
	{
		//substash[i].acquire_stash_bufferspace(size);
		#if 0
	  assert(substash[curstash].big_ass_buffer == NULL);
	  assert(substash[curstash].big_ass_buffer_max == 0);

	  substash[curstash].big_ass_buffer = (unsigned char *)memalign(128, size);
	#ifdef DEBUG
	  memset(substash[curstash].big_ass_buffer, 0x77, size);
	#endif
	  substash[curstash].big_ass_buffer_max = size;
		#endif
	}

}


////////////////////////// acquire_stash_bufferspace /////////////////////////////
//
void multistash::acquire_stash_bufferspace(int size)
{
	assert(big_ass_buffer == NULL);
	assert(big_ass_buffer_max == 0);
#ifdef TARGET_GC
	//We allocate a small buffer anyway, just so various asserts don't cause
	//problems
	if( size == 0 )
		size = 32;
#endif
	
#ifdef TARGET_XBOX
  big_ass_buffer = (unsigned char *)memalign(4096, size);
#else
	big_ass_buffer = (unsigned char *)memalign(128, size);
#endif
#ifdef DEBUG
	memset(big_ass_buffer, 0x77, size);
#endif
	big_ass_buffer_max = size;
}


////////////////////////// release_stash_bufferspace /////////////////////////////
//
void stash::release_stash_bufferspace()
{
	for ( int i=STASH_SYSTEM; i<STASH_LIMIT; i++ )
	{
		substash[i].release_stash_bufferspace();
	}
}

void multistash::release_stash_bufferspace()
{
//  assert( big_ass_buffer != NULL );
  free( big_ass_buffer );
	//memset(big_ass_buffer,0,big_ass_buffer_max);
  big_ass_buffer = NULL;
  big_ass_buffer_max = 0;
}



/***************************** stash creation api ********************************/

//////////////////////////////  create_stash  /////////////////////////////////
//
// imports all of the files listed in the file_list file and creates a stash file from them
bool stash::create_stash(stash_t _type, char *_list_filename, char *_stash_filename)
{
  // process the list file
  substash[curstash].index_tree.dispose();
  if (_list_filename != NULL)
    strcpy(substash[curstash].list_filename, _list_filename);

  if (_stash_filename != NULL)
    strcpy(substash[curstash].stash_filename, _stash_filename);
  substash[curstash].type = _type;

  substash[curstash].header.temp_size = 0;
  substash[curstash].header.stored_size = 0;

  if (read_list_file() == false)
    return false;

  // prepare substash[curstash].header info
  substash[curstash].header.magic_number = STASH_MAGIC_NUMBER;
  substash[curstash].header.revision = STASH_CURRENT_REVISION;

  // Revision 1 moved index from the first to the last in the file, for ProView memory concerns
  substash[curstash].header.stored_offset = sizeof (substash[curstash].header);
  substash[curstash].header.temp_offset = substash[curstash].header.stored_offset + substash[curstash].header.stored_size;
  substash[curstash].header.index_offset = substash[curstash].header.temp_offset + substash[curstash].header.temp_size;
  substash[curstash].header.index_entries = substash[curstash].index_tree.size();
  substash[curstash].header.index_entry_size = STASH_INDEX_ENTRY_SIZE;


  // write out stash file
  os_file stash_file;
  stringx stash_filename_str(substash[curstash].stash_filename);
  stash_file.open(stash_filename_str, os_file::FILE_WRITE);
  stash_file.read(&substash[curstash].header, sizeof(substash[curstash].header));

  substash[curstash].header.aram_offset = 0;
  substash[curstash].header.aram_size = 0;
/*	We've started using these fields (dc 11/01/01)
  substash[curstash].header.foo2 = 0xdeadf00d;
  substash[curstash].header.bar2 = 0xdeadf00d;
  substash[curstash].header.foo3 = 0xdeadf00d;
  substash[curstash].header.bar3 = 0xdeadf00d;
*/
  substash[curstash].header.compression_flags = 0;
  substash[curstash].header.index_comp_size = 0;
  substash[curstash].header.stored_comp_size = 0;
  substash[curstash].header.temp_comp_size = 0;
  substash[curstash].header.foo4 = 0xdeadf00d;
  substash[curstash].header.bar4 = 0xdeadf00d;

  // write out substash[curstash].header
  stash_file.write(&substash[curstash].header, sizeof(substash[curstash].header));

  unsigned old_stored_buf_end = substash[curstash].stored_buf_end;
  substash[curstash].stored_buf_end = 0;
  unsigned old_temp_buf_end = substash[curstash].temp_buf_end;
  substash[curstash].temp_buf_end = 0;

  // write out the sections of the stash file.  For memory purposes (exporting stashes on
  // ProView (32mb max)), we are now doing three walks of the tree, not optimal speed-wise, sigh.
//debug_print("writing index buf at %u\n", ftell(stash_file));
  write_tree(stash_file, substash[curstash].index_tree.root(), STASH_SECTION_STORED);
  assert (substash[curstash].stored_buf_end == substash[curstash].header.stored_size);

  write_tree(stash_file, substash[curstash].index_tree.root(), STASH_SECTION_TEMP);
  debug_print("substash[curstash].temp_buf_end %d == substash[curstash].header.temp_size %d", substash[curstash].temp_buf_end, substash[curstash].header.temp_size);
  assert (substash[curstash].temp_buf_end == substash[curstash].header.temp_size);

  write_tree(stash_file, substash[curstash].index_tree.root(), STASH_SECTION_INDEX);

  // restore the old values (not that it matters much)
  substash[curstash].stored_buf_end = old_stored_buf_end;
  substash[curstash].temp_buf_end = old_temp_buf_end;

  // free the index tree
  substash[curstash].index_tree.dispose();

  stash_file.close();

  debug_print("\nLoading from stash file %s", substash[curstash].stash_filename);
  debug_print("\tmagic #    %11x\t\trevision  %12u", substash[curstash].header.magic_number, substash[curstash].header.revision);
  debug_print("\theader size    %7u\t\tindex entries %8u", sizeof(substash[curstash].header), substash[curstash].header.index_entries);
  debug_print("\tindex entry size    %u\t\tindex offset %9u", substash[curstash].header.index_entry_size, substash[curstash].header.index_offset);
  debug_print("\tstored offset %8u\t\tstored size %10u", substash[curstash].header.stored_offset, substash[curstash].header.stored_size);
  debug_print("\ttemp offset %10u\t\ttemp size %12u", substash[curstash].header.temp_offset, substash[curstash].header.temp_size);
  debug_print("\taram offset %10u\t\taram size %12u", substash[curstash].header.aram_offset, substash[curstash].header.aram_size);
  debug_print("Fin.\n");

  return true;
}

//////////////////////////////   add_stored   /////////////////////////////////
//
unsigned stash::add_stored(os_file &the_file, unsigned char *raw_data, unsigned data_size)
{
//debug_print("writing stored buf at %u %u\n", ftell(the_file), substash[curstash].stored_buf_end);
  the_file.write(raw_data, data_size);
  unsigned ret_val = substash[curstash].stored_buf_end;
  substash[curstash].stored_buf_end += data_size;

  return ret_val;
}

//////////////////////////////    add_temp    /////////////////////////////////
//
unsigned stash::add_temp(os_file &the_file, unsigned char *raw_data, unsigned data_size)
{
  the_file.write(raw_data, data_size);
  unsigned ret_val = substash[curstash].temp_buf_end;
  substash[curstash].temp_buf_end += data_size;

  return ret_val;
}

//////////////////////////////   write_tree   /////////////////////////////////
//
// a utility function for import, which writes out the contents of the index tree to the disk
void stash::write_tree(os_file &the_file, AvlNode *curr, stash_section_t which_section)
{
  assert(the_file.is_open());
  if (curr == NULL)
    return;

  if (which_section == STASH_SECTION_STORED)
  {
    // write out stored section block
    if (curr->data()->is_stored() == true && curr->data()->is_valid())
    {
      assert( curr->data()->file_type != stash_index_entry::STASH_INDEX_ENTRY_RAW );
      curr->data()->set_offset(add_stored(the_file, curr->data()->raw_data, curr->data()->raw_data_size));
    }
  }
  else if (which_section == STASH_SECTION_TEMP)
  {
    // write out temp section block
    if (curr->data()->is_stored() == false && curr->data()->is_valid())
    {
      assert( curr->data()->file_type == stash_index_entry::STASH_INDEX_ENTRY_RAW );
      curr->data()->set_offset(add_temp(the_file, curr->data()->raw_data, curr->data()->raw_data_size));
    }
  }
  else if (which_section == STASH_SECTION_INDEX)
  {
    // write out index section block
    pstring the_type;

    switch (curr->data()->file_type)
    {
      case stash_index_entry::STASH_INDEX_ENTRY_PS2MESH: the_type = ps2mesh_type; break;
      case stash_index_entry::STASH_INDEX_ENTRY_XBMESH:  the_type = xbmesh_type;  break;
      case stash_index_entry::STASH_INDEX_ENTRY_GCMESH:  the_type = gcmesh_type;  break;
      case stash_index_entry::STASH_INDEX_ENTRY_UNUSED:  the_type = unused_type;  break;
      case stash_index_entry::STASH_INDEX_ENTRY_RAW:     the_type = raw_type;     break;
      case stash_index_entry::STASH_INDEX_ENTRY_ANMX:    the_type = anmx_type;    break;
      case stash_index_entry::STASH_INDEX_ENTRY_TEX:     the_type = tex_type;     break;
      case stash_index_entry::STASH_INDEX_ENTRY_SNMX:    the_type = snmx_type;    break;
      case stash_index_entry::STASH_INDEX_ENTRY_MISC:    the_type = misc_type;    break;
      default:                                           the_type = unknown_type; break;
    }
    debug_print("\nHere's the substash[curstash].header for %s", curr->data()->name.c_str());
    debug_print("\tfile offset %u\t\t\tis_stored 0x%x", curr->data()->file_offset, curr->data()->is_stored());
    debug_print("\tentry size %u\t\t\ttype %s", curr->data()->entry_size, the_type.c_str());

    // write myself out
    the_file.write(curr->data(), STASH_INDEX_ENTRY_SIZE);
  }
  else
    assert(0);

  // and now for my children
  if (curr->left())
    write_tree(the_file, curr->left(), which_section);
  if (curr->right())
    write_tree(the_file, curr->right(), which_section);
}


bool read_file(os_file &the_file, unsigned char *&buf, unsigned &buf_size);


////////////////////////////// read_list_file /////////////////////////////////
//
// A utility function for import, reads in the list file into memory,
// and goes on to load in each of the files in the list file
bool stash::read_list_file()
{
    os_file list_file;
    unsigned char *buf = NULL;
    unsigned file_size;
    char buf2[1024];

    stringx list_filename_str(substash[curstash].list_filename);
    list_file.open(list_filename_str, os_file::FILE_READ);

  // read in the list file, if everything goes ok then process it
    if (list_file.is_open() && read_file(list_file, buf, file_size))
  {
    unsigned last_pos = 0;

    // process all of the entries in the list file
    for (unsigned pos = 0; pos < file_size; ++pos)
    {
      if (buf[pos] != '\n')
        continue;

      // we have a full line of text, copy the line to buf2
      int amount = pos - last_pos;
      assert(amount < 1024);
      strncpy((char *)buf2, (char *)(buf + last_pos), amount);
      buf2[amount] = '\0';
      last_pos = pos + 1;

      filespec spec((char *)buf2);

      // try to read the file, create a stash index entry for this file
      stash_index_entry * new_guy = NEW stash_index_entry;
      new_guy->name = spec.name.c_str();
      new_guy->name.concatinate(spec.ext.c_str());

    debug_print("Processing %s", new_guy->name.c_str());

      // determine the substash[curstash].type of file we're dealing with
      pstring ext(spec.ext.c_str());
      if (ext == ".ps2mesh" )
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_PS2MESH);
        new_guy->set_stored( true );
      }
      else if (ext == ".xbmesh")
      {
          new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_XBMESH);
          new_guy->set_stored( true );
      }
      else if (ext == ".gcmesh")
      {
          new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_GCMESH);
          new_guy->set_stored( true );
      }
      else if (ext == ".tm2" || ext == ".dds" || ext == ".gct" )
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_TEX);
        new_guy->set_stored( true );
      }
			else if (ext == ".tga")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_TEX);
        new_guy->set_stored( true );
      }
			else if (ext == ".ifl")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_TEX);
        new_guy->set_stored( true );
      }
			else if (ext == ".ate")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_TEX);
        new_guy->set_stored( true );
      }
      else if( ext == ".snmx" )
      {
        new_guy->set_type( stash_index_entry::STASH_INDEX_ENTRY_SNMX );
        new_guy->set_stored( true );
      }
      else
			if (ext == ".wave")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".beach")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".csv")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".cam")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".txt")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".dat")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".fon")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".panel")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".anim")
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_MISC);
        new_guy->set_stored( true );
      }
      else
			if (ext == ".ANMX" )
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_ANMX);
        new_guy->set_stored( true );
      }
			if (ext == ".ANMB" )
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_ANMX);
        new_guy->set_stored( true );
      }
      else
      {
        new_guy->set_type(stash_index_entry::STASH_INDEX_ENTRY_RAW);
        new_guy->set_stored( true );
      }

      if (new_guy->file_type != stash_index_entry::STASH_INDEX_ENTRY_UNUSED)
      {
        // read in the data for the file
        os_file the_file;
        the_file.open(spec.fullname(), os_file::FILE_READ);
        if (the_file.is_open())
        {
          unsigned char *buf3 = NULL;
          unsigned buf3_size = 0;
          read_file(the_file, buf3, buf3_size);

          if (substash[curstash].index_tree.find(new_guy) == NULL)
          {
            if (new_guy->import_file(buf3, buf3_size))
            {
              // if the file imported ok, add it to the index tree
              if (new_guy->is_valid())
              {
                if (new_guy->is_stored())
                  substash[curstash].header.stored_size += buf3_size;
                else
                  substash[curstash].header.temp_size += buf3_size;
                substash[curstash].index_tree.add(new_guy);
              }
            }
            else
            {
              // clean up the bad entry but don't exit (we want to keep parsing)
              delete new_guy;
            }
          }
          else
          {
            debug_print("warning, duplicate file %s skipped", spec.fullname().c_str());
            delete new_guy;
          }
        }
        else
        {
          debug_print("Error.  Could not read file %s", spec.fullname().c_str());
          delete new_guy;
        }
      }
      else
      {
        if (stash::substash[curstash].type == PS2_STASH)
        {
          debug_print("File %s not included in the ps2 version of the stash.", spec.fullname().c_str());
        }
        else if (stash::substash[curstash].type == PC_STASH)
        {
          debug_print("File %s not included in the pc version of the stash.", spec.fullname().c_str());
        }
        delete new_guy;
      }
    }
    // free the list file buffer before returning
    free(buf);
    return true;
  }

  return false;
}


//////////////////////////////   read_file    /////////////////////////////////
//
// special use (ie not for the general public) function - reads in
// an already opened file into buf, where buf is a NULL pointer.
// after this function buf is allocated with malloc, to the size
// recorded in file_size, and it contains the contents of the file.
bool read_file(os_file &the_file, unsigned char *&buf, unsigned &file_size)
{
  assert (the_file.is_open() && buf == NULL);

  file_size = the_file.get_size();
  the_file.set_fp(0, os_file::FP_BEGIN);
  buf = (unsigned char *)malloc(file_size);

  bool ret_val = the_file.read(buf, file_size);
  the_file.close();

  return ret_val;
}

////////////////////////// destash /////////////////////////////
//
void stash::destash()
{
  destash_tree(substash[curstash].index_tree.root());
}


////////////////////////// destash_tree /////////////////////////////
//
void stash::destash_tree(AvlNode *curr)
{
}


/***************************** stored buffer api ********************************/

///////////////////////////// get_memory_image ////////////////////////////////
//
bool stash::get_memory_image(const pstring& _name, unsigned char *&buf,
                             int &buf_size, stash_index_entry *&hdr)
{
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,_name.c_str());
#endif
  assert(substash[curstash].stash_file_is_open);
#if !defined(TARGET_XBOX) && !defined(TARGET_GC) && defined(DEBUG)
  assert(substash[curstash].stored_buf != NULL);
#endif /* TARGET_XBOX JIV DEBUG */

  stash_index_entry find_me;
  find_me.set_name(_name);
  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
    debug_print("trying to find image %s in the stash", _name.c_str());
  AvlNode *the_node = NULL;

	#ifdef STASHSEARCHREVERSED
	for ( int i=STASH_LIMIT-1; i>=STASH_SYSTEM; i-- )
	#else
	for ( int i=STASH_SYSTEM; i<STASH_LIMIT; i++ )
	#endif
	{
		the_node=substash[i].index_tree.find(&find_me);
		if ( the_node )
		{
//    		debug_print("found %s in stash %d", _name.c_str(),i);	// too much output (dc 05/05/02)
			curstash=i;
			break;
		}
	}

  if (!the_node || !the_node->data())
  {
    return false;
  }


  unsigned char* base = substash[curstash].stored_buf;

  if( the_node->data()->in_aram() )
    base = NULL;
  
  if (!the_node->data()->is_stored())
    base = substash[curstash].temp_buf;

  the_node->data()->set_used( true );
  buf = base + the_node->data()->file_offset;
  buf_size = the_node->data()->entry_size;
  hdr = the_node->data();
  //debug_print("Refering to file %s in the stash", the_node->data()->name.c_str());
  return true;
}

#ifdef TARGET_GC
aram_id_t stash::get_current_aram_id()
{
  return substash[curstash].aram_id;
}
#endif

/****************************** temp buffer api *********************************/

//////////////////////////// stash::file_exists ////////////////////////////////
//
// checks for file existence within the stash temp buffer
// similar to osfile exists, except that it takes in a name and a seperate extension,
// both are pstrings
bool stash::file_exists(const char * _name)
{
  filespec spec(_name);
  pstring pname(spec.name.c_str());
  pname.concatinate(spec.ext.c_str());
  return file_exists(pname);
}

bool stash::file_exists(const pstring& _name)
{
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,_name.c_str());
#endif

  //if (substash[curstash].stash_file_is_open == false)
  //  return false;

  stash_index_entry find_me;

//debug_print("This is the existence check %s", _name.c_str());

  find_me.set_name(_name);

  AvlNode *the_node = NULL; //substash[curstash].index_tree.find(&find_me);
	#ifdef STASHSEARCHREVERSED
	for ( int i=STASH_LIMIT-1; i>=STASH_SYSTEM; i-- )
	#else
	for ( int i=STASH_SYSTEM; i<STASH_LIMIT; i++ )
	#endif
	{
  	if (substash[i].stash_file_is_open)
			the_node=substash[i].index_tree.find(&find_me);
		if ( the_node )
		{
//    		debug_print("found %s in stash %d", _name.c_str(),i);	// too much output (dc 05/05/02)
			curstash=i;
			break;
		}
	}

	#if 0
  if (the_node == NULL)
  {
    // look in the system stash for it
    //the_node = substash[curstash].system_index_tree.find(&find_me);
    the_node = substash[system_stash].index_tree.find(&find_me);
  }
	#endif

  if (the_node && the_node->data())
    return true;
  else
	{
#ifndef USER_MKV
   	debug_print("%s not found in any stash file", _name.c_str());
#endif
    return false;
	}
}


/////////////////////////////    stash::open   ////////////////////////////////
//
// opens a file within the stash temp buffer
bool stash::open(const char * _name)
{
  pstring pname(_name);
  return open(pname);
}

bool stash::open(const pstring & _name)
{
  assert(!opened);
  assert(substash[curstash].stash_file_is_open == true);

  if (index == NULL || index->name != _name)
  {
    stash_index_entry find_me;

    find_me.name = _name;
    AvlNode *the_node = NULL; //substash[curstash].index_tree.find(&find_me);
		#ifdef STASHSEARCHREVERSED
		for ( int i=STASH_LIMIT-1; i>=STASH_SYSTEM; i-- )
		#else
		for ( int i=STASH_SYSTEM; i<STASH_LIMIT; i++ )
		#endif
		{
			the_node=substash[i].index_tree.find(&find_me);
			if ( the_node )
			{
//    			debug_print("found %s in stash %d", _name.c_str(),i);	// too much output (dc 05/05/02)
				curstash=i;
				break;
			}
		}
    if (the_node == NULL)
    {
//      debug_print("could not open file %s%s in stash %s\n", _name.c_str(), _ext.c_str(), substash[curstash].stash_filename);
      opened = false;
      eof = false;
      return false;
    }
    stash_index_entry *_index = the_node->data();
    index = _index;
  }
//  debug_print("Reading file %s from the stash", index->name.c_str());
/*
    debug_print("Here's the substash[curstash].header for %s%s\n", index->name.c_str(), index->ext.c_str());
    debug_print("\tfile offset %u\n", index->file_offset);
    debug_print("\tentry size %u\n", index->entry_size);
    debug_print("\tis_stored 0x%x\n", index->flag.is_stored);
    debug_print("\tflags 0x%x\n", index->flag.reserved);
    debug_print("\treserved 0x%x\n", index->reserved);
*/
  // stored file io should use get_memory_image instead of this api
  assert(index->file_type == stash_index_entry::STASH_INDEX_ENTRY_RAW);

  // now set the fp to the location in the temp buffer that this file is at
  fp = index->file_offset;
  max_fp = fp + index->entry_size;
  opened = true;
  eof = false;
	fullname=_name;

  return true;
}

/////////////////////////////    stash::read   ////////////////////////////////
//
// reads data from a file within the stash temp buffer
unsigned stash::read(void * data, int bytes)
{
  assert(opened);
  assert(substash[curstash].stash_file_is_open == true);
  index->set_used( true );

  if (fp >= max_fp)
  {
    eof = true;
    return 0;
  }
  unsigned read_size = bytes;
  if (fp+read_size >= max_fp)
  {
    eof = true;
    read_size = max_fp - fp;
  }
  unsigned char *read_from = substash[curstash].temp_buf + fp;
  memcpy(data, read_from, read_size);

  fp += read_size;
  return read_size;
}

///////////////////////////// stash::get_name ////////////////////////////////
//
// gets the name of a file within a stash temp buffer
const pstring & stash::get_name() const
{
  assert(opened);
  assert(index);
  assert(substash[curstash].stash_file_is_open == true);

	return fullname;
  //return index->name;
}

///////////////////////////// stash::set_fp ////////////////////////////////
//
// sets the 'file pointer' of a file within a stash temp buffer
void stash::set_fp( int pos, filepos_t base )
{
  assert(opened);
  assert(index);
  assert(substash[curstash].stash_file_is_open == true);

  switch (base)
  {
    case FP_BEGIN:
      if (pos >= (int)index->entry_size)
      {
        eof = true;
        fp = max_fp;
      }
      else
      {
        eof = false;
        fp = index->file_offset + pos;
      }
      break;

    case FP_CURRENT:
      if (fp + pos >= max_fp)
      {
        eof = true;
        fp = max_fp;
      }
      else if (fp + pos < 0)
      {
        fp = 0;
        eof = false;
      }
      else
      {
        eof = false;
        fp += pos;
      }
      break;

    case FP_END:
      if (fp + pos >= max_fp)
      {
        eof = true;
        fp = max_fp;
      }
      else if (fp + pos < 0)
      {
        fp = 0;
        eof = false;
      }
      else
      {
        eof = false;
        fp = max_fp + pos;
      }
      break;
  }
}

///////////////////////////// stash::get_fp ////////////////////////////////
//
// gets the 'file pointer' within a stash temp buffer
unsigned stash::get_fp()
{
  assert(opened);
  assert(substash[curstash].stash_file_is_open == true);

  return fp - index->file_offset;
}

///////////////////////////// stash::get_size ////////////////////////////////
//
// returns the size of the 'file' stored within the temp buffer
unsigned stash::get_size()
{
  assert(opened);
  assert(substash[curstash].stash_file_is_open == true);

  return index->entry_size;
}

////////////////////////// stash::get_ptr_to_temp_file /////////////////////////////
//
// used for temp buffers that we want to read out of, but don't want to have to allocate
// another buffer to copy that into, only to throw it away again.  A special use function
// use only if you know what you're doing.
const unsigned char * stash::get_ptr_temp_file()
{
  assert(opened);
  assert(substash[curstash].stash_file_is_open == true);
  assert(substash[curstash].temp_buf);
  assert(!index->is_stored());
  index->set_used( true );

  return substash[curstash].temp_buf + index->file_offset;
}




/****************************** stash_index_entry *********************************/

///////////////////////////// import_file ////////////////////////////////
//
// imports a single file corresponding to a file list entry.  If the contents of
// the file meet one of our special 'memory image' file types (the types of files
// we wish to convert into memory images that can be loaded directly into memory
// when the game loads and used).  If it doesn't meet one of these, it is read
// in as raw, that is, the file-io system layer will read out of this
// buffer just like it was reading from a file, and when the system stops loading
// a level, the raw entries are free'd from memory (the assumption being that
// the relevant data has either been used or copied to other locations in memory).
bool stash_index_entry::import_file (unsigned char *&file_data, unsigned &file_length)
{
  bool ret_val = true;
  int align_to;

  switch(file_type)
  {
    case STASH_INDEX_ENTRY_PS2MESH:
    case STASH_INDEX_ENTRY_XBMESH:
    case STASH_INDEX_ENTRY_GCMESH:
    case STASH_INDEX_ENTRY_TEX:
    case STASH_INDEX_ENTRY_ANMX:
    case STASH_INDEX_ENTRY_SNMX:
    case STASH_INDEX_ENTRY_MISC:
      ret_val = true;
      set_stored( true );
      set_valid( true );
      break;

    case STASH_INDEX_ENTRY_RAW:
      set_stored( false );
      set_valid( true );
      break;
  }
  set_used( false );

  // pad the entry to 128 (stored) or 4 (temp) bytes, if req'd
  if (file_type == STASH_INDEX_ENTRY_XBMESH)
    align_to = 4096;
  else if (is_stored())
    align_to = 128;
  else
    align_to = 4;

  entry_size = file_length;
  if (file_length % align_to != 0)
  {
    raw_data_size = file_length + align_to - (file_length % align_to);
    raw_data = (unsigned char *)malloc(raw_data_size);
    memset(raw_data, 0, raw_data_size);
    memcpy(raw_data, file_data, entry_size);
    free(file_data);
    file_data = raw_data;
    file_length = raw_data_size;
  }
  else
  {
    raw_data = file_data;
    raw_data_size = file_length;
  }

  return ret_val;
}


#if 0
int main()
{
  // de-stashifier test app
  stash::acquire_stash_bufferspace(12*1024*1024);
  stash::open_stash("..\\jood\\newyork2.st2");
  stash::destash();
  stash::release_stash_bufferspace();

  return 0;
}
#endif



