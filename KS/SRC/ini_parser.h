#ifndef INI_FILE_PARSER_HEADER
#define INI_FILE_PARSER_HEADER

class os_developer_options : public singleton
{
public:
  // construct and read the ini file
  os_developer_options();

  enum flags_t
  {
#define MAC(x,y,d) x,
#include "devoptflags.h"
#undef MAC
    NUM_FLAGS
  };

  enum strings_t
  {
#define MAC(x,y,d) x,
#include "devoptstrs.h"
#undef MAC
    NUM_STRINGS
  };
    
  enum ints_t
  {
#define MAC(x,y,d) x,
#include "devoptints.h"
#undef MAC
    NUM_INTS
  };

  bool is_flagged( flags_t _flag ) const { return flags[_flag]; }
  bool get_flag( flags_t _flag ) const { return flags[_flag]; }
  void set_flag( flags_t _flag, bool v ) { flags[_flag] = v; }
  void toggle_flag( flags_t _flag ) { flags[_flag] = 1-flags[_flag]; }
  const stringx& get_string( strings_t _string ) const { return strings[_string]; }
  int get_int( ints_t _int ) const { return ints[_int]; }
  void set_int( ints_t _int, int a ) { ints[_int] = a; }

  //  get scene name
  const stringx& get_scene_name() const   { return get_string( STRING_SCENE_NAME ); }

  // get stash_name
  const stringx& get_stash_name() const   { return get_string( STRING_STASH_NAME ); }

  //  get hero name
  const stringx& get_hero_name(int hero_num) const
      {
        assert(STRING_HERO_NAME_0 + hero_num < STRING_END_HERO_NAMES);
        return get_string( (strings_t) (STRING_HERO_NAME_0 + hero_num) );
      }

  const stringx& get_song_name() const    { return get_string( STRING_SONG_NAME ); }

  const stringx& get_exe_name() const     { return exe_name; }

  // temporary

  int get_camera_state() const { return get_int( INT_CAMERA_STATE ); }
  void set_camera_state(int i) { ints[INT_CAMERA_STATE]=i; }

  bool get_gravity_enabled() const { return is_flagged( FLAG_GRAVITY ); }
  void set_gravity_enabled(bool i) { flags[FLAG_GRAVITY]=i; }
  void toggle_gravity_enabled() { toggle_flag( FLAG_GRAVITY ); }

  bool get_no_warnings() const { return is_flagged( FLAG_NO_WARNINGS ); }
  bool get_no_lighting_warnings() const { return is_flagged( FLAG_NO_LIGHTING_WARNINGS ); }

  bool get_lock_step() const { return is_flagged( FLAG_LOCK_STEP ); }
  void set_lock_step(bool i) { flags[FLAG_LOCK_STEP]=i; }
  void toggle_lock_step() { toggle_flag( FLAG_LOCK_STEP ); }

  bool is_texture_dump_requested() const { return is_flagged( FLAG_TEXTURE_DUMP ); }

  bool is_locked_hero() const { return is_flagged( FLAG_LOCKED_HERO ); }

  bool is_move_editor() const { return is_flagged( FLAG_MOVE_EDITOR ); }

//  const stringx& get_herostate_file() const { return get_string( STRING_HERO_STATE_FILE ); }

  //void set_debugaipath(bool on) { flags[FLAG_AI_PATH_DEBUG]=on; }

  // the instance of the command line
  DECLARE_SINGLETON(os_developer_options)

private:
  bool    flags[NUM_FLAGS];
  stringx strings[NUM_STRINGS];
  int     ints[NUM_INTS];
  stringx exe_name;

  friend class ini_parser;
};

#define MAX_FILENAME_LENGTH 256

class ini_parser
{
  private:
    enum TokenType {
      NO_TOKEN = 0,
      TOKEN_GROUP,
      TOKEN_STRING,
      TOKEN_EQUALS
    };

    enum GroupType {
      NO_GROUP = 0,
      FLAGS_GROUP,
      INTS_GROUP,
      STRINGS_GROUP
    };

    char filename[MAX_FILENAME_LENGTH];
    char token[MAX_FILENAME_LENGTH];
    int scan_pos;
    char *line;

    char stored_token;
    char stored_type;
    char stored_num;

    int build_token(char *line, char *the_token);
    int get_token(char **curr_token, int *token_type, int *num_value);
    void unget_token();
    void new_line(char *curr_line);
    bool parse(os_developer_options *opts);
    void despacify_token(char *curr_token);

  public:
    ini_parser(const char *ini_filename, os_developer_options *opts)
    {
      assert(ini_filename != NULL);
      strncpy(filename, ini_filename, MAX_FILENAME_LENGTH);
      filename[MAX_FILENAME_LENGTH-1] = '\0';
      scan_pos = 0;
      line = NULL;
      token[0] = '\0';
      stored_token = 0;
      stored_type = 0;
      stored_num = 0;
      parse(opts);
    }
    ~ini_parser() {}
};



#endif
