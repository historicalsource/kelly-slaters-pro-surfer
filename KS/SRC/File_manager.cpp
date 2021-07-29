#include "global.h"

//#include <vector>
//using namespace std;
#include "pstring.h"
#include "stringx.h"
#include "filespec.h"
#include "mustash.h"
#include "singleton.h"
#include "file_manager.h"
//#include <ctype.h>
#ifndef krPrintf
#define krPrintf printf
#endif

#include "entity.h"
#include "path.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "wds.h"
#include "profiler.h"
#include "marker.h"
#include "light.h"
#include "particle.h"
#include "item.h"
// BIGCULL #include "turret.h"
// BIGCULL #include "scanner.h"
#include "sky.h"

// BIGCULL #include "gun.h"
// BIGCULL #include "thrown_item.h"
// BIGCULL #include "melee_item.h"
#include "pmesh.h"
#include "file_finder.h"
#include "widget_entity.h"
#include "conglom.h"
#include "osdevopts.h"
// BIGCULL #include "manip_obj.h"
// BIGCULL#include "switch_obj.h"
#include "polytube.h"
#include "lensflare.h"
#include "FrontEndManager.h"


#ifdef TARGET_PC
#define STASH_EXTENSION ".pcstash"
#define STASH_FILE_LIST_EXTENSION ".pcsfl"
#define FILE_MANAGER_CACHE_EXTENSION ".pcfmc"
#else
#define STASH_EXTENSION ".st2"
#define STASH_FILE_LIST_EXTENSION ".ps2sfl"
#define FILE_MANAGER_CACHE_EXTENSION ".ps2fmc"
#endif

file_id_t file_manager::file_id_counter = 0;

DEFINE_SINGLETON(file_manager)

/////////////////////////////////////////////////////////////////////////////////////
// C O N S T R U C T O R / D E S T R U C T O R
//
file_manager::file_manager()
{
  root_dir[0] = '\0';
  host_prefix[0] = '\0';
  level_dir[0] = '\0';
  stash_auto_build = false;
  file_system_locked = false;
  file_cache.setDisposeElements(true);
  path_cache.setDisposeElements(true);

  // default search priorities
  first_search_stash = STASH_SEARCH;
  second_search_stash = NON_STASH_SEARCH;
}

file_manager::~file_manager()
{
  if (level_context.is_set())
    clear_level_context();
}

/////////////////////////////////////////////////////////////////////////////////////
// A C Q U I R E   M E M O R Y   I M A G E
//
void * file_manager::acquire_memory_image( const pstring &file_name, const char *file_path, int *buffer_size )
{
  unsigned char *buf;
  int buf_size;
  bool found_it = false;

  // search search locations based on priority
  if (first_search_stash == STASH_SEARCH)
  {
    stash_index_entry *hdr = NULL;
    my_stash.get_memory_image(file_name, buf, buf_size, hdr);
    if (buf != NULL)
    {
      if (buffer_size != NULL)
        *buffer_size = buf_size;
      found_it = true;
    }
  }
  else
  {
    assert(first_search_stash == NON_STASH_SEARCH);
    file_info_node find_me;
    find_me.set_name(file_name);
    file_info_node *found = file_cache.findData(&find_me);
    if (found != NULL)
    {
      if (found->is_loaded() == false)
      {
        // load it
        found->load_image(*this);
      }
      assert(found->is_image() && found->is_loaded());
      buf = found->buffer;
      if (buffer_size != NULL)
        *buffer_size = found->buffer_size;
      found_it = true;
    }
  }

  return buf;
}

void * file_manager::acquire_memory_image( const filespec &spec, int *buffer_size )
{
  pstring file_name(spec.name.c_str());
  return acquire_memory_image(file_name, spec.path.c_str(), buffer_size);
}

void * file_manager::acquire_memory_image( const char *file_name, const char *file_path, int *buffer_size )
{
  pstring file_name_str(file_name);
  return acquire_memory_image(file_name_str, file_path, buffer_size);
}


/////////////////////////////////////////////////////////////////////////////////////
// A C Q U I R E   F I L E
//
file_id_t file_manager::acquire_file( const pstring &file_name, const char *file_path )
{
  return 0;
}

file_id_t file_manager::acquire_file( const filespec &spec )
{
  return 0;
}

file_id_t file_manager::acquire_file( const char *file_name, const char *file_path
                                     )
{
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// R E L E A S E   F I L E
//
bool file_manager::release_file( file_id_t file_id )
{
  return false;
}

int file_manager::read_file( file_id_t file_id, unsigned char *buf, unsigned int size )
{
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// A C Q U I R E   L O G
//
// write-able files are currently just logs, which can be turned off en-masse
file_id_t file_manager::acquire_log()
{
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// R E L E A S E   L O G
//
bool file_manager::release_log()
{
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// W R I T E   L O G
//
bool file_manager::write_log( file_id_t log_file_id )
{
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// R E L E A S E   A L L   M E M O R Y   I M A G E S
//
// mass annihilation
bool file_manager::release_all_memory_images()
{
  // ish
  file_cache.dispose();
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// R E L E A S E   A L L   F I L E S
//
bool file_manager::release_all_files()
{
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// R E L E A S E   A L L   L O G S
//
bool file_manager::release_all_logs()
{
  return false;
}


/////////////////////////////////////////////////////////////////////////////////////
// R E A D   C A C H E
//
bool file_manager::read_cache()
{
  os_file fp;

  stringx cache_filename(root_dir);
  cache_filename += level_dir;
  cache_filename += level_context.c_str();
  cache_filename += FILE_MANAGER_CACHE_EXTENSION;
  fp.open( cache_filename , os_file::FILE_READ );
  if (fp.is_open())
  {
    int buf_size = fp.get_size();
    char *buf = (char *)malloc(buf_size);
    assert(buf != NULL);
    fp.read(buf, buf_size);
    fp.close();

    char buf2[1024];
    unsigned last_pos = 0;

    // process all of the entries in the list file
    for (int pos = 0; pos < buf_size; ++pos)
    {
      if (buf[pos] != '\n')
        continue;

      // we have a full line of text, copy the line to buf2
      int amount = pos - last_pos;
      assert(amount < 1024);
      strncpy((char *)buf2, (char *)(buf + last_pos), amount);
      buf2[amount] = '\0';
      last_pos = pos + 1;

      file_info_node *add_me = NEW file_info_node;
      filespec spec((char *)buf2);
      add_me->set_name_ext_path(spec);
      file_cache.add(add_me);
    }

    // free the list file buffer before returning
    free(buf);
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////////////////
// W R I T E   C A C H E
//
bool file_manager::write_cache()
{
  os_file fp;
  bool ret_val = false;

  stringx cache_filename(root_dir);
  cache_filename += level_dir;
  cache_filename += level_context.c_str();
  cache_filename += STASH_FILE_LIST_EXTENSION;
  fp.open( cache_filename, os_file::FILE_READ );
  if (fp.is_open() == false)
  {
    // no existing sfl, so just create a NEW one
    fp.open( cache_filename, os_file::FILE_WRITE );
    assert(fp.is_open());
  }
  else
  {
    int buf_size = fp.get_size();
    char *buf = (char *)malloc(buf_size);
    assert(buf != NULL);
    fp.read(buf, buf_size);
    fp.close();
    cache_filename += ".out";  // for debugging
    fp.open( cache_filename, os_file::FILE_WRITE );

    char buf2[1024];
    unsigned last_pos = 0;

    // process all of the entries in the list file
    for (int pos = 0; pos < buf_size; ++pos)
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

      stash_index_entry find_me;
      pstring pname(spec.name.c_str());
      pname.concatinate(spec.ext.c_str());
      find_me.set_name(pname);

//debug_print("Processing %s ", pname.c_str());

      // determine if this file was used
      stash_index_entry *found = my_stash.get_index_tree().findData(&find_me);
      if (found != NULL)
      {
        if (found->was_used())
        {
          // write it out to the NEW sfl file
          strcat(buf2, "\r\n");
          fp.write(buf2, strlen(buf2));
        }
      }
    }
    // free the list file buffer before returning
    free(buf);
  }
  write_level_cache(fp);
  fp.close();
  ret_val = true;

  return ret_val;
}


/////////////////////////////////////////////////////////////////////////////////////
// W R I T E   L E V E L   C A C H E
//
bool file_manager::write_level_cache(os_file &sfl)
{
  os_file fp;
  bool ret_val = false;

  stringx cache_filename(root_dir);
  cache_filename += level_dir;
  cache_filename += level_context.c_str();
  cache_filename += FILE_MANAGER_CACHE_EXTENSION;
  fp.open( cache_filename, os_file::FILE_WRITE );
  if (fp.is_open() == false)
    error("Could not write level files used cache");
  else
  {
    TreeNode<file_info_node> *root = file_cache.root();
    write_level_cache_tree(fp, sfl, root);
    fp.close();
    ret_val = true;
  }

  return ret_val;
}

void file_manager::write_level_cache_tree(os_file &fp, os_file &sfl, TreeNode<file_info_node> *curr)
{
  if (curr == NULL)
    return;
  file_info_node *me = curr->data();
  if (me->was_used())
  {
    // write this entry out
    stringx file_entry(me->path);
    file_entry += me->name.c_str();
    file_entry += (char *)me->ext;
    file_entry += "\r\n";
    fp.write((void *)file_entry.c_str(), file_entry.length());
    if (sfl.is_open())
    {
      stringx sfl_entry;
      if (me->path[0] != '\\' && me->path[0] != '\0')
      {
        sfl_entry = level_dir;
      }
      sfl_entry += file_entry;
      sfl.write((void *)sfl_entry.c_str(), sfl_entry.length());
    }
  }

  if (curr->left() != NULL)
    write_level_cache_tree(fp, sfl, curr->left());
  if (curr->right() != NULL)
    write_level_cache_tree(fp, sfl, curr->right());
}


/////////////////////////////////////////////////////////////////////////////////////
// I N I T   L E V E L   C O N T E X T
//
void file_manager::init_level_context( const char *scene_name )
{
  assert(level_context.is_set() == false);
  assert(file_system_locked == false);
  filespec spec(scene_name);
  level_context = spec.name.c_str();
//  level_dir[0] = '\\';
//  level_dir[1] = '\0';
  strcat(level_dir, spec.path.c_str());

  // open up the stash for this level
  assert(my_stash.using_stash() == false);
  stringx stash_filename(root_dir);
  stash_filename += "data\\";
  stash_filename += level_context.c_str();
  stash_filename += STASH_EXTENSION;
printf("Attempting to open stash file %s\n", stash_filename.c_str());
  my_stash.open_stash((char *)stash_filename.c_str());

  // read in file manager cache for this level
  read_cache();
}


/////////////////////////////////////////////////////////////////////////////////////
// L O C K   F I L E   S Y S T E M
//
void file_manager::lock_file_system()
{
  assert(file_system_locked == false);
  file_system_locked = true;
  my_stash.close_stash();
}

/////////////////////////////////////////////////////////////////////////////////////
// U N L O C K   F I L E   S Y S T E M
//
void file_manager::unlock_file_system()
{
  assert(file_system_locked == true);
  file_system_locked = false;
}

/////////////////////////////////////////////////////////////////////////////////////
// C L E A R   L E V E L   C O N T E X T
//
void file_manager::clear_level_context()
{
  assert(level_context.is_set());

  write_cache();
//#pragma todo("create stash for level here")

  level_context = "";
  level_dir[0] = '\0';
  my_stash.close_stash();
  my_stash.free_stored();

  release_all_memory_images();
  release_all_files();
  release_all_logs();
}


enum TokenType {
  NO_TOKEN = 0,
  TOKEN_EXTENSION,
  TOKEN_PATH
};

void new_line(char *line);
int get_token(char **token, int *token_type, int *num_value);

/////////////////////////////////////////////////////////////////////////////////////
// S E T   S E A R C H   P A T H S
//
void file_manager::set_search_paths( const char *path_filename )
{
  assert(path_filename != NULL);
  os_file path_file;
  path_file.open(path_filename, os_file::FILE_READ);

  if (path_file.is_open())
  {
    unsigned int buf_size = path_file.get_size();
    unsigned char *buf = (unsigned char *)malloc(buf_size+1);

    assert(buf != NULL);
    path_file.read(buf, buf_size);
    buf[buf_size] = '\0';
    path_file.close();

    unsigned int pos = 0;

    char *token = NULL;
    int token_type;
    int token_num;
    path_info_node *curr_path = NULL;

    // parse out our file lists
    new_line((char *)&buf[pos]);

    while (get_token(&token, &token_type, &token_num) != NO_TOKEN)
    {
      if (token_type == TOKEN_EXTENSION)
      {
        // NEW extension
        curr_path = NEW path_info_node();

        token[strlen(token) - 2] = '\0';
        curr_path->set_extension(&token[1]);
        path_cache.add(curr_path);
      }
      else if (token_type == TOKEN_PATH)
      {
        // add path
        if (curr_path == NULL)
          error("Error reading path file, no extension found\n");
        stringx token_str(token);
        curr_path->add_path(token_str);
      }
    }

    free(buf);

    // print out the tree for debugging purposes
/*  pstring pstr(".tm2");
    path_info_node findme;
    findme.set_extension(pstr);
    path_info_node *curr = path_cache.findData(&findme);
    if (curr != NULL)
    {
      printf("Extension %s:\n", curr->get_extension().c_str());
      vector<stringx>::iterator it = curr->get_paths().begin();
      while (it != curr->get_paths().end())
      {
        printf("%s\n", (*it).c_str());
        ++it;
      }
    }
*/
  }
  else
    error("Error.  Could not open path file %s\n", path_filename);
}



/////////////////////////////////////////////////////////////////////////////////////
// F I N D   T H A T   F I L E
//
bool file_info_node::find_that_file( file_manager &fm, os_file &fp )
{
  stringx image_filename;

  // look in the file's path location first
  if (path[0] != '\0')
  {
    image_filename = fm.root_dir;
    if (path[0] != '\\')
      image_filename += fm.level_dir;
    image_filename += path;
    image_filename += name.c_str();
    image_filename += (char *)ext;
printf("Attempting to open file %s\n", image_filename.c_str());
    fp.open(image_filename, os_file::FILE_READ);
    if (fp.is_open())
      return true;
  }

  path_info_node find_me;
  find_me.set_extension((char *)ext);
  path_info_node *paths = fm.path_cache.findData(&find_me);
  if (paths == NULL)
    return false;
  vector<stringx> &path_list = paths->get_paths();
  vector<stringx>::iterator it = path_list.begin();
  vector<stringx>::iterator it_end = path_list.end();

  while (it != it_end)
  {
    const char *c_str = (*it).c_str();
    image_filename = fm.root_dir;

    if (c_str[0] != '\\')
    {
      // relative to level directory
      image_filename += fm.level_dir;
    }

    image_filename += c_str;
    image_filename += name.c_str();
    image_filename += (char *)ext;
printf("Attempting to open file %s\n", image_filename.c_str());
    fp.open(image_filename, os_file::FILE_READ);
    if (fp.is_open())
    {
      // update the level_dir to contain the path where we found it
      strcpy(fm.level_dir, c_str);
      return true;
    }
    ++it;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// L O A D   I M A G E
//
bool file_info_node::load_image( file_manager &fm )
{
  os_file fp;
  if (find_that_file( fm, fp ) == true)
  {
    int the_size = fp.get_size();
    buffer = (unsigned char *)memalign(128, the_size);
    assert(buffer != NULL);
    fp.read(buffer, the_size);
    buffer_size = the_size;
    fp.close();
    flag.is_loaded = true;
    flag.was_used = true;
    flag.is_image = true;
  }
  return flag.is_loaded;
}


/////////////////////////////////////////////////////////////////////////////////////
// P A T H   F I L E   P A R S I N G   R O U T I N E S
//
int gScanPos;
char *gLine = NULL;
char gToken[MAX_DIRECTORY_LENGTH];
int gStoredToken = 0;
int gStoredType = 0;
int gStoredNum = 0;

int build_token(char *gLine, char *gToken);
//void strupr(char *lift_me_up);

/*** get_token ***/
// token is a pointer which we change to point to the token to use
int get_token(char **token, int *token_type, int *num_value)
{
  int ret;
  if (gStoredToken)
  {
    *token = gToken;
    *token_type = gStoredType;
    *num_value = gStoredNum;

    gStoredToken = 0;
    return *token_type;
  }

  gToken[0] = '\0';
  while(isspace(gLine[gScanPos]) && (gLine[gScanPos] != '\0'))
    ++gScanPos;

  ret = build_token(&gLine[gScanPos], gToken);
  gScanPos += ret;

  if (ret == 0)
  {
    *token = NULL;
    return NO_TOKEN;
  }

  strupr(gToken);
  *token = gToken;

  if (gToken[0] == '[')
    *token_type = TOKEN_EXTENSION;
  else if (strlen(gToken) > 0)
    *token_type = TOKEN_PATH;
  else
    *token_type = NO_TOKEN;
  gStoredNum = *num_value;
  gStoredType = *token_type;

  return *token_type;
}

int build_token(char *line, char *token)
{
  int i=0;
  while ( (line[i] != '\n') && (line[i] != '\0') )
  {
    token[i] = line[i];
    ++i;
  }
  token[i] = '\0';
  return i;
}

/*void strupr(char *lift_me_up)
{
  int i=0;
  while ( lift_me_up[i] != '\0' )
  {
    lift_me_up[i] = toupper(lift_me_up[i]);
    ++i;
  }
}
*/
/*** unget_token ***/
// pushes back the last token, only works for one-token's worth
void unget_token()
{
  gStoredToken = 1;
}

/*** new_line ***/
// passes in a NEW line to get tokens from
void new_line(char *line)
{
  gLine = line;
  gScanPos = 0;
}


#ifdef TEST_FILE_MANAGER

/////////////////////////////////////////////////////////////////////////////////////
// M A I N
//
int main( int argc, char *argv[] )
{
  file_manager fm;
  fm.set_host_prefix("host0:");
  fm.set_root_dir("c:\\ks\\bin\\data");
  fm.set_search_paths("ks_paths.txt");

  fm.init_level_context("test_levels\\newyork\\newyork2");

  // fill the file_cache up with dummy files
  file_info_node *foo = NEW file_info_node;
  {
    filespec spec("\\spidey.snd");
    foo->set_name_ext_path(spec);
  }
  fm.file_cache.add(foo);
  foo = NEW file_info_node;
  {
    filespec spec("\\ps2 textures\\alpha.tm2");
    foo->set_name_ext_path(spec);
  }
  fm.file_cache.add(foo);

  // at the end of level load do this:
  fm.lock_file_system();

  // at level unload, do this:
  fm.clear_level_context();

  return 0;
}
#endif
