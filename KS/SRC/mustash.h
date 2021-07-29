/*-------------------------------------------------------------------------------------*
 * stash.h
 *
 * a 'wad'-file style file-io class which was created to reduce the number of seeks
 * when loading a level.  It organizes several files into one-big-file called a
 * 'stash'.  There can only be one stash file open in the system at a time, but multiple
 * files can be read out of that stash.  It is organized into two distinct buffers:
 * the 'temp' buffer, which contains entire files, and can be accessed thru an osfile-
 * like api.  This buffer is freed when the stash file is closed (close_stash()).  The
 * other buffer, 'stored', is not freed until free_stored is called.  The purpose of the
 * stored buffer is to allow memory images to be loaded from disk and used directly,
 * without any additional parsing or mashing.  Files in the stored buffer are not
 * accessed thru the osfile-style api, but instead use the function get_memory_image
 * to get a pointer to the memory image.
 *-------------------------------------------------------------------------------------*/

#ifndef MULTISTASH_CLASS_HEADER
#define MULTISTASH_CLASS_HEADER

//
//  publicly used stash calls
//    stash::using_stash()
//    stash::get_stash_filename()
//    stash::open_stash()
//    stash::close_stash()
//    stash::free_stored()
//    stash::is_stash_open()
//    stash::file_exists()
//    stash::is_system_stash_open()
//    stash::acquire_stash_bufferspace()
//    stash::release_stash_bufferspace()
//





#include "pstring.h"
#include "avltree.h"

#if  !defined(TARGET_GC)
#define ASYNC_PLAYER_LOAD
#define REAL_ASYNC_LOAD
#endif

#ifdef _MAX_PATH
#define MAX_STRING_LENGTH _MAX_PATH+1
#else
#define MAX_STRING_LENGTH 256
#endif

#if !defined(TARGET_GC) && !defined(TARGET_XBOX)
#include <eekernel.h>
#endif

#ifdef TARGET_GC
#include "gc_arammgr.h"
#endif

class stash_index_entry;

typedef TreeNode<stash_index_entry> AvlNode;

extern const int BIG_ASS_BUFFER_SIZE;

#define STASH_INDEX_COMP 0x00000001
#define STASH_STORED_COMP 0x00000002
#define STASH_TEMP_COMP 0x000000004

/*** stash_header ***/
struct stash_header
{
  // order matters!
  unsigned index_offset;
  unsigned short magic_number;
  unsigned short revision;
  unsigned index_entries;
  unsigned index_entry_size;
  unsigned stored_offset;
  unsigned stored_size;
  unsigned aram_offset;
  unsigned aram_size;

  unsigned temp_offset;
  unsigned temp_size;
  unsigned compression_flags;
  unsigned index_comp_size;
  unsigned stored_comp_size;
  unsigned temp_comp_size;
  unsigned foo4;
  unsigned bar4;

  // any future revisions add members to the header below
};

typedef int stash_id;
const stash_id system_stash=0;

#include "stashes.h"

#define STATIC static

//void shrink_string(char *in_str, pstring &out_str);

// this can increase, but should never decrease between file format revisions
#define STASH_INDEX_ENTRY_SIZE (16*sizeof(int))

/*** stash_index_entry ***/
class stash_index_entry
{
// each stash entry is currently up to 16 32-bit words.  This limit can be upped, but
// this would requite a new file format revision to be made.  But since this has been
// designed to be extensible, that should be ok to do.

	private:
		enum Flags
		{
			IS_VALID  = 0x1,
			IS_STORED = 0x2,
			WAS_USED  = 0x4,
			IN_ARAM   = 0x8,
			UNUSED2 = 0x10,
			UNUSED3 = 0x20,
			UNUSED4 = 0x40,
			UNUSED5 = 0x80
		};

	private:
    // order matters, add members at the end of this and even then do it with wisdom
    pstring name;                 // 8 words

    unsigned file_offset;         // 1 word

    unsigned entry_size;          // 1 word       // the 'true' (unpadded) size of the file

    unsigned char file_type;      // 1 word
		unsigned char flags;
    unsigned short padding1;      // reserved for future use

    unsigned char *raw_data;			// 1 word
    int raw_data_size;            // 1 word // the 'false' (aka padded to alignment) size of the file

		unsigned int padding2;        // 1 word
		unsigned int padding3;        // 1 word
		unsigned int padding4;        // 1 word
  
	public:
    stash_index_entry()
    {
      raw_data = NULL;
      raw_data_size = 0;
      flags = 0;
    }
    ~stash_index_entry()
    {
      if (raw_data)
        free(raw_data);
      raw_data = NULL;
    }

    // for stash-compatibility, keep these numbers the same, only add new ones to the end of the list
    enum index_entry_t {
      STASH_INDEX_ENTRY_UNUSED  = 0,
      STASH_INDEX_ENTRY_RAW     = 1,
      STASH_INDEX_ENTRY_PS2MESH = 2,
      STASH_INDEX_ENTRY_XBMESH  = 3,
      STASH_INDEX_ENTRY_GCMESH  = 4,
      STASH_INDEX_ENTRY_ANMX    = 5,
      STASH_INDEX_ENTRY_TEX     = 6,
      STASH_INDEX_ENTRY_SNMX    = 7,
      STASH_INDEX_ENTRY_MISC    = 8
    };

		bool is_stored() { return ( flags & IS_STORED ) == IS_STORED; }
    bool is_valid() { return ( flags & IS_VALID ) == IS_VALID; }
    bool was_used() { return ( flags & WAS_USED ) == WAS_USED; }
    bool in_aram() { return ( flags & IN_ARAM ) == IN_ARAM; }

    void set_stored(bool val)
		{
			if( val )
				flags |= IS_STORED;
			else
				flags &= ~IS_STORED;
		}

    void set_valid(bool val)
		{
			if( val )
				flags |= IS_VALID;
			else
				flags &= ~IS_VALID;
		}

    void set_used(bool val)
		{
			if( val )
				flags |= WAS_USED;
			else
				flags &= ~WAS_USED;
		}

    void set_aram(bool val)
		{
			if( val )
				flags |= IN_ARAM;
			else
				flags &= ~IN_ARAM;
		}

    // for avl tree sorting
    int compare(stash_index_entry *data)
    {
//nglPrintf("comparing %s and %s = %d\n", name.c_str(), data->name.c_str(), (name == data->name));
      if (name == data->name)
        return 0;
      else if (name < data->name )
        return -1;
      else if (name > data->name)
        return 1;
      assert(false);
      return 0;
    }

    void set_name(char *new_name) { name = new_name; }
	const char *get_name() {return name;}
    void set_name(const pstring& new_name) { name = new_name; }
    void set_type(index_entry_t type) { file_type = type; }
    void set_offset(unsigned new_offset) { file_offset = new_offset; }
    bool import_file (unsigned char *&file_data, unsigned &file_length);

    friend class stash;
};

typedef AvlTree<stash_index_entry> avl_index;
class stash;

enum stash_t
{
  PS2_STASH,
  PC_STASH
};



class multistash
{
	friend class stash;
  public: // static member functions
    bool open_stash(char *_stash_filename ); //bool is_system_stash = false);
    bool is_stash_open( void ) { return stash_file_is_open; }
    bool using_stash( void ) { return stash_file_is_open || stored_buf != NULL; }
    void close_stash( void );
    void free_stored( void );

    // stash creation api
    bool create_stash(stash_t _type, char *_list_filename = NULL, char *_stash_filename = NULL);
    bool read_list_file();
    void destash(); // not very well tested E3 2001 function, needs more testing/refinement before real use.

    // set filename functions supplied for stash creation
    void set_stash_filename(char * filename) { strcpy(stash_filename, filename); }
    const char *get_stash_filename() { return stash_filename; }
    void set_list_filename(char * filename) { strcpy(list_filename, filename); }
    const char *get_list_filename() { return list_filename; }

    bool file_exists(const pstring& _name);
    bool file_exists(const char * _name);

    // stash memory image functions
    bool get_memory_image(const pstring& _name, unsigned char *&buf,
                                 int &buf_size, stash_index_entry *&hdr);


  public: // non-STATIC member functions
    enum filepos_t
    {
      FP_BEGIN,
      FP_CURRENT,
      FP_END
    };

    // stash-io (one-big-file)
    bool open(const pstring & _name);
    bool open(const char * _name);
    unsigned read(void * data, int bytes);

    const unsigned char * get_ptr_temp_file();
    unsigned get_size();
    void set_fp( int pos, filepos_t base );
    unsigned get_fp(); // relative to beginning
    void close()
    {
      index = NULL;
      eof = false;
      opened = false;
      fp = 0;
      max_fp = 0;
    }

    // state queries
    const pstring & get_name() const;
    inline bool is_open() const { return opened; }           // returns true after a successful open call.
    inline bool at_eof() const { return eof; }               // check this after a read operation.

    // constructor/destructor
    multistash() : index_tree( 0 )
    {
      opened = false;
      pre_opened = false;
      async_current_stored_point = 0;
      async_current_temp_point = 0;
      async_current_aram_point = 0;
      
      eof = false;
      index = NULL;
      fp = 0;
      async_read_size = 128*1024;
      max_fp = 0;
    }
    ~multistash()
    {
    }

    void acquire_stash_bufferspace(int size);
    void release_stash_bufferspace();

    //AvlTree<stash_index_entry> &get_index_tree() { return index_tree; }
    avl_index &get_index_tree() { return index_tree; }


  private:
    char list_filename[MAX_STRING_LENGTH];
    char stash_filename[MAX_STRING_LENGTH];

    int async_current_stored_point;
    int async_current_temp_point;
    int async_current_aram_point;
    os_file async_stash_file;
    
    unsigned char *stored_buf;
    unsigned stored_buf_end;
    unsigned stored_buf_max;
    bool first_pass_temp;
    bool first_pass_stored;
    bool first_pass_aram;
    
    unsigned char *big_ass_buffer;
    unsigned big_ass_buffer_max;

    unsigned char *temp_buf;
    unsigned temp_buf_end;
    unsigned temp_buf_max;
    
    unsigned aram_buf_end;
    unsigned aram_buf_max;
#ifdef TARGET_GC
    aram_id_t aram_id;
#endif
    unsigned int async_read_size;
    stash_header header;
    bool stash_file_is_open;
    stash_t type;

    avl_index index_tree;
    //avl_index system_index_tree;

    // for write_tree
    enum stash_section_t {
      STASH_SECTION_STORED,
      STASH_SECTION_TEMP,
      STASH_SECTION_ARAM,
      STASH_SECTION_INDEX
    };

    void write_tree(os_file &the_file, AvlNode *curr, stash_section_t which_section);
    unsigned add_stored(os_file &the_file, unsigned char *raw_data, unsigned data_size);
    unsigned add_temp(os_file &the_file, unsigned char *raw_data, unsigned data_size);

    void destash_tree(AvlNode *curr);

  private:
    bool abort_stash_read;
    stash_index_entry *index;
    bool pre_opened;
    bool opened;
    bool eof;
    unsigned fp;
    unsigned max_fp;
		pstring fullname;
};



struct threadArgs 
{
  char name[30];
  void (*progressCallback)(stash_id stashid, void *, int);
  void *userData;
  stash_id stashid;
  KSHeapIDs heapid;
};

/*** stash ***/
class stash
{
  public: // static member functions
    STATIC bool open_stash(char *_stash_filename, stash_id stashid=DEFAULTSTASH ); //bool is_system_stash = false);

    // These next two are responisble for async loading of a stash.  First call pre_open_stash_for_async
    // Then call read_stash_async until it returns true
    STATIC int pre_open_stash_for_async( char *_stash_filename, stash_id stashid=DEFAULTSTASH );
    STATIC bool read_stash_async( stash_id stashid=DEFAULTSTASH );
    STATIC void set_async_read_size( stash_id stashid=DEFAULTSTASH, unsigned int read_size= 128*1024);
    STATIC float get_async_progress_percent( stash_id stashid=DEFAULTSTASH );
    STATIC int get_async_progress_bytes( stash_id stashid=DEFAULTSTASH );
    STATIC void close_stash_async( stash_id stashid=DEFAULTSTASH );

    STATIC void LoadStashAsync(char *_stash_filename, stash_id stashid, KSHeapIDs heapid, void (*progressCallback)(stash_id stashid, void *, int), void *userData);
    STATIC void LoadStashAsyncHelper(threadArgs *args);
    STATIC void AbortAsyncRead(stash_id stashid);
#ifdef REAL_ASYNC_LOAD
    STATIC void WaitForStashLoad();
#endif
    STATIC bool is_stash_open( ) { return substash[curstash].stash_file_is_open; }
    STATIC char *get_stash_name( stash_id stashid=DEFAULTSTASH ) { return substash[stashid].stash_filename; }
    STATIC bool is_stash_open( stash_id stashid ) { return substash[stashid].stash_file_is_open; }
    STATIC bool is_stash_preopened( stash_id stashid=DEFAULTSTASH  ) { return substash[stashid].pre_opened; }
    STATIC bool using_stash( stash_id stashid=DEFAULTSTASH  ) { return substash[curstash].stash_file_is_open || substash[curstash].stored_buf != NULL; }
    //STATIC bool is_system_stash_open() { return (substash[system_stash].stored_buf != NULL); }
    STATIC void close_stash( stash_id stashid=DEFAULTSTASH );
    STATIC void free_stored( stash_id stashid=DEFAULTSTASH );
    STATIC int  get_current_stash() {return curstash;}
    STATIC void set_current_stash(int cs) {curstash=cs;}
    // stash creation api
    STATIC bool create_stash(stash_t _type, char *_list_filename = NULL, char *_stash_filename = NULL);
    STATIC bool read_list_file();
    STATIC void destash(); // not very well tested E3 2001 function, needs more testing/refinement before real use.

    // set filename functions supplied for stash creation
    STATIC void set_stash_filename(char * filename) { strcpy(substash[curstash].stash_filename, filename); }
    STATIC const char *get_stash_filename() { return substash[curstash].stash_filename; }
    STATIC void set_list_filename(char * filename) { strcpy(substash[curstash].list_filename, filename); }
    STATIC const char *get_list_filename() { return substash[curstash].list_filename; }

    STATIC bool file_exists(const pstring& _name);
    STATIC bool file_exists(const char * _name);

    // stash memory image functions
    STATIC bool get_memory_image(const pstring& _name, unsigned char *&buf,
                                 int &buf_size, stash_index_entry *&hdr);

#ifdef TARGET_GC
    STATIC aram_id_t get_current_aram_id();
#endif

  public: // non-STATIC member functions
    enum filepos_t
    {
      FP_BEGIN,
      FP_CURRENT,
      FP_END
    };

    // stash-io (one-big-file)
    bool open(const pstring & _name);
    bool open(const char * _name);
    unsigned read(void * data, int bytes);

    const unsigned char * get_ptr_temp_file();
    unsigned get_size();
    void set_fp( int pos, filepos_t base );
    unsigned get_fp(); // relative to beginning
    void close()
    {
      index = NULL;
      eof = false;
      opened = false;
      fp = 0;
      max_fp = 0;
    }

    // state queries
    const pstring & get_name() const;
    inline bool is_open() const { return opened; }           // returns true after a successful open call.
    inline bool at_eof() const { return eof; }               // check this after a read operation.

    // constructor/destructor
    stash()
    {
      opened = false;
      eof = false;
      index = NULL;
      fp = 0;
      max_fp = 0;
      

    }
    ~stash()
    {
    }

    STATIC void acquire_stash_bufferspace(int size);
    STATIC void release_stash_bufferspace();

    STATIC AvlTree<stash_index_entry> &get_index_tree() { return substash[curstash].index_tree; }
    STATIC stash_id   curstash;
  private:

    

		STATIC multistash substash[STASH_LIMIT];
		
    STATIC unsigned int async_read_size;

    // for write_tree
    enum stash_section_t {
      STASH_SECTION_STORED,
      STASH_SECTION_TEMP,
      STASH_SECTION_ARAM,
      STASH_SECTION_INDEX
    };

    STATIC void write_tree(os_file &the_file, AvlNode *curr, stash_section_t which_section);
    STATIC unsigned add_stored(os_file &the_file, unsigned char *raw_data, unsigned data_size);
    STATIC unsigned add_temp(os_file &the_file, unsigned char *raw_data, unsigned data_size);

    STATIC void destash_tree(AvlNode *curr);

  private:
    stash_index_entry *index;
    bool opened;
    bool eof;
    unsigned fp;
    unsigned max_fp;
		pstring fullname;
};

#endif
