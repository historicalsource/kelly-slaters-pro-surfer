#ifndef FILE_MANAGER_HEADER
#define FILE_MANAGER_HEADER
#include "avltree.h"



class file_info_node;
class path_info_node;

typedef unsigned int file_id_t;

#define MAX_DIRECTORY_LENGTH 64

class file_manager : public singleton
{
  public:
    file_manager();
    ~file_manager();

    DECLARE_SINGLETON( file_manager )

    // File Cache
    // Stash Index Tree (multiple-stash support)
    //

    // Memory image
    void * acquire_memory_image( const pstring &file_name, const char *file_path = NULL, int *buffer_size = NULL );
    void * acquire_memory_image( const filespec &spec, int *buffer_size = NULL );
    void * acquire_memory_image( const char *file_name, const char *file_path = NULL, int *buffer_size = NULL );

    // Standard File I/O - lies underneith chunkfile/textfile, but above os_file.
    // Do not use os_file directly, if you need a file, use this.
    // Memory images are, of course, preferred.
    file_id_t acquire_file( const pstring &file_name, const char *file_path = NULL );
    file_id_t acquire_file( const filespec &spec );
    file_id_t acquire_file( const char *file_name, const char *file_path = NULL );
    bool release_file( file_id_t file_id );

    int read_file( file_id_t file_id, unsigned char *buf, unsigned int size );

    // write-able files are currently just logs, which can be turned off en-masse
    file_id_t acquire_log();
    bool release_log();
    bool write_log( file_id_t log_file_id );

    // mass annihilation
    bool release_all_memory_images();
    bool release_all_files();
    bool release_all_logs();


    void init_level_context( const char *scene_name );      // initializes a NEW level context, asserts no level context already in place
    void clear_level_context();                             // releases the level context, writing fm info files and cleaning house
    bool level_context_is_set() { return level_context.is_set(); }

    void lock_file_system();
    void unlock_file_system();

    enum stash_setting {
      NO_STASH,
      STASH_SEARCH,
      NON_STASH_SEARCH,
    };
    void set_stash_priorities( file_manager::stash_setting first_search,
                               file_manager::stash_setting second_search )
    {
      first_search_stash = first_search;
      second_search_stash = second_search;
    }

    void set_stash_auto_build( bool setting ) { stash_auto_build = setting; }
    bool get_stash_auto_build() { return stash_auto_build; }

    void set_root_dir( const char *new_root_dir )
    {
      assert(strlen(new_root_dir) < MAX_DIRECTORY_LENGTH);
      strncpy(root_dir, new_root_dir, MAX_DIRECTORY_LENGTH);
      root_dir[MAX_DIRECTORY_LENGTH-1] = '\0';
    }
    // no get_root_dir on purpose, the app shouldn't bother with such things!

    void set_host_prefix( const char *new_host_prefix )
    {
      assert(strlen(new_host_prefix) < 8);
      strncpy(host_prefix, new_host_prefix, 8);
      host_prefix[7] = '\0';
    }
    // no get_host_prefix on purpose, the app shouldn't bother with such things!

    void set_search_paths(const char *path_filename);

    AvlTree<file_info_node> file_cache; // public for testing only
  private:
    stash my_stash;
    AvlTree<path_info_node> path_cache;

    char root_dir[MAX_DIRECTORY_LENGTH];
    char level_dir[MAX_DIRECTORY_LENGTH];
    char host_prefix[8];
    pstring level_context;

    stash_setting first_search_stash;
    stash_setting second_search_stash;

    bool stash_auto_build;
    bool file_system_locked;

    static file_id_t file_id_counter;

    bool read_cache();
    bool write_cache();

    bool write_level_cache(os_file &sfl);
    void write_level_cache_tree(os_file &fp, os_file &sfl, TreeNode<file_info_node> *curr);

  friend class file_info_node;
};


/*** path_info_node ***/
class path_info_node
{
  private:
    vector<stringx> paths;
    pstring ext;

  public:
    path_info_node() {}
    ~path_info_node() { paths.resize(0); }

    // for avl tree sorting
    int compare(path_info_node *data)
    {
      if (ext == data->ext)
        return 0;
      else if (ext < data->ext )
        return -1;
      else if (ext > data->ext)
        return 1;
      assert(false);
      return 0;
    }

    const pstring &get_extension() { return ext; }
    vector<stringx> &get_paths() { return paths; }

    void set_extension( const char *new_ext ) { assert(strlen(new_ext) < PSTRING_MAX_LENGTH); ext = new_ext; }
    void add_path( const stringx &new_path ) { paths.push_back(new_path); }
};


/*** file_info_node ***/
class file_info_node
{
  private:
    pstring name;
    char path[MAX_DIRECTORY_LENGTH];
    unsigned char ext[10];

    unsigned char *buffer;
    unsigned int buffer_size;

    unsigned short ref_count;

    struct
    {
      bool is_file:1;
      bool is_image:1;
      bool is_loaded:1;
      bool was_used:1;
      bool reserved:4;            // reserved for future use
    } flag;

    bool find_that_file( file_manager &fm, os_file &fp );

  public:
    file_info_node()
    {
      buffer = NULL;
      buffer_size = 0;
      ref_count = 0;
      ext[0] = '\0';
      flag.is_file = false;
      flag.is_image = false;
      flag.is_loaded = false;
      flag.was_used = true;   // for debugging only, normally should default to false
    }
    ~file_info_node()
    {
      assert(ref_count == 0);
      if (buffer)
        free(buffer);
      buffer = NULL;
      buffer_size = 0;
    }

    bool is_file() { return flag.is_file; }
    bool is_image() { return flag.is_image; }
    bool is_loaded() { return flag.is_loaded; }
    bool was_used() { return flag.was_used; }

    // for avl tree sorting
    int compare(file_info_node *data)
    {
      if (name == data->name)
        return 0;
      else if (name < data->name )
        return -1;
      else if (name > data->name)
        return 1;
      assert(false);
      return 0;
    }

    void set_name(const char *new_name) { assert(strlen(new_name) < PSTRING_MAX_LENGTH); name = new_name; }
    void set_name(const pstring& new_name) { name = new_name; }
    void set_path(const char *new_path)
    {
      assert(strlen(new_path) < MAX_DIRECTORY_LENGTH);
      strncpy(path, new_path, MAX_DIRECTORY_LENGTH);
      path[MAX_DIRECTORY_LENGTH-1] = '\0';
    }
    void set_ext(const char *new_ext)
    {
      assert(strlen(new_ext) < 10);
      strncpy((char *)ext, new_ext, 10);
      path[9] = '\0';
    }
    void set_name_ext_path( const filespec &spec )
    {
      assert(spec.path.length() < MAX_DIRECTORY_LENGTH);
      strncpy(path, spec.path.c_str(), MAX_DIRECTORY_LENGTH);
      path[MAX_DIRECTORY_LENGTH-1] = '\0';
      assert(spec.ext.length() < 10);
      strncpy((char *)ext, spec.ext.c_str(), 10);
      ext[9] = '\0';
      assert(spec.name.length() < PSTRING_MAX_LENGTH);
      name = spec.name.c_str();
    }
    bool load_file();
    bool load_image( file_manager &fm );

  friend class file_manager;
};


#endif
