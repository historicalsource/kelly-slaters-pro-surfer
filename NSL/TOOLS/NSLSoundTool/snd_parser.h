#ifndef SND_FILE_PARSER_HEADER
#define SND_FILE_PARSER_HEADER

#include <vector>
using namespace std;

#ifndef PATH_MAX
#define PATH_MAX 260
#endif


#include "PS2SoundTool.h"
#include "XBoxSoundTool.h"
#include "GCSoundTool.h"

// 
// Per-Sound information structure
//
struct snd_info_t 
{
  // anything that can be parsed from an input-snd file goes here
	char                path[PATH_MAX]; // path to the sound
	char                name[PATH_MAX]; // name of the sound (no path, or extension information)
  char                alias[PATH_MAX];// another 'name' for the sound that can be converted into an
                          // ... enumerated type (NSL 2.0, for Asen & Malkia)
  int                 aliasID;
  float               volume_left;    // 0 to 1
  float               volume_right;   // 0 to 1
  float               pitch;          // 1.0 is base pitch
  bool                loop;           // looping sound
  bool                cd;             // force CD streamed
	bool                spu;            // [PS2 specific] force SPU resident on ps2
  nslSourceTypeEnum   type;           // Sound type (ie music, 
  bool                languages[NSL_LANGUAGE_Z];
// moved to ps2 specific section  unsigned int        realSize[NSL_LANGUAGE_Z];;       // The unpadded size.  Each platform may pad this out
	bool applyReverb;


  // platform specific info-structs
  ps2_info_t  ps2;
  xbox_info_t xbox;
  gc_info_t   gc;

  static vector<snd_info_t> default_info;

  // member functions
  snd_info_t()
  {
    clear(false);
  }
  snd_info_t( const snd_info_t &other )
  {
    copy(other);
  }
  void merge_with( const snd_info_t *other )
  {
    // merge the export flags
    for (int i=0; i<NSL_LANGUAGE_Z; ++i)
    {
      languages[i] = languages[i] || other->languages[i];
    }
  }
  snd_info_t& operator = (const snd_info_t& new_guy)
  {
    if (&new_guy == this) 
      return *this;
    copy(new_guy);
    return *this;
  }
    
  bool operator == (const snd_info_t &check_me) const
  {
    return (strcmp(name, check_me.name) == 0);
  }

  void clear(bool free_mem);
  void copy( const snd_info_t &other );
  void set_to_default( );

  void set_name_with_path( char * full_name, bool set_name_flag = true, bool set_path_flag = true );
  void set_name( char *full_name ) { set_name_with_path( full_name, true, false ); }
  void set_path( char *full_name ) { set_name_with_path( full_name, false, true ); }
};

const char* next_token( const char* src, char* dst, size_t n );
void        build_info( const char* line, snd_info_t* info, bool consider_default = true );
void        load_info_defaults( char * global_snd_filename );


///////////////////////
// Sound IDs support

int generate_alias_id( char * alias );
void export_alias_ids( char * output_dir );

#endif
