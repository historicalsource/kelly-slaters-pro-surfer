#ifndef GAMEFILE_H
#define GAMEFILE_H


//P #include "osstorage.h"
#include "game.h"

#define MAX_GAMEFILE_SLOTS    8

#define MAX_ITEMS             16
#define MAX_ITEM_NAME_LEN     32

#define MAX_SUBLEVEL_LEN      24

#define DEF_GAMEFILE_VERSION_NUM 1001   // change when cur_data_t changes!!
#define GAMEFILE_BLOCKS  19             // change this as needed also

class gamefile_t
{
public:
  class cur_data_t
  {
  protected:
    short version_num;       // change DEF_GAMEFILE_VERSION_NUM every time cur_data_t's data format changes!!
    uint8 load_seq_index;    // in load sequence vector in game
    uint8 sublevel_index;    // in sublevel_index vector in game
    char heroname[MAX_PLAYERS][16];
    bool hero_data_valid;
    // hero soft attributes
    int hit_points;
    int armor_points;
    int max_ammo_points;
    int ammo_points;
    int nanotech_energy;
    int melee_damage;
    uint8 num_items;
    char item_names[MAX_ITEMS][MAX_ITEM_NAME_LEN];
    uint8 item_counts[MAX_ITEMS];
    int score;

  public:
    cur_data_t();
    ~cur_data_t() {}

    void init()
    {
      load_seq_index = 0;
      sublevel_index = 0;
      hero_data_valid = false;
    }

    void store_hero_to_cur_data();
    void restore_hero_from_cur_data();

    void set_heroname( int heronum, const char* name );
    const char* get_heroname(int heronum) const { return heroname[heronum]; }

    int get_version_num() const { return version_num; }

    uint8 get_load_seq_index() const { return load_seq_index; }
    void set_load_seq_index( uint8 _load_seq_index ) { load_seq_index = _load_seq_index; }
    uint8 get_sublevel_index() const { return sublevel_index; }
    void set_sublevel_index( uint8 _sublevel_index ) { sublevel_index = _sublevel_index; }
  };
  
  // info about files saved on current storage unit
  class file_info_t
  {
  public:
    file_info_t();
    ~file_info_t() {}

//P    const storage_time &get_time_stamp() const { return time_stamp; }
    const char *get_sublevel() const { return sublevel; }
    int get_slot_num() const { return slot_num; }

    friend class gamefile_t;

  private:
//P    storage_time time_stamp;
    char sublevel[MAX_SUBLEVEL_LEN];
    int slot_num;         // from which filename is built
  };

  gamefile_t();
  ~gamefile_t() {}

  bool save();
  bool load();

  void store_hero_to_cur_data() { cur_data.store_hero_to_cur_data(); }
  void restore_hero_from_cur_data() { cur_data.restore_hero_from_cur_data(); }
  void init_cur_data() { cur_data.init(); }

  void set_heroname(int heronum, const char* name ) { cur_data.set_heroname( heronum, name ); }
  const char* get_heroname(int heronum) const { return cur_data.get_heroname(heronum); }

  void get_saved_file_info();

  uint8 get_load_seq_index() const { return cur_data.get_load_seq_index(); }
  void set_load_seq_index( uint8 _load_seq_index ) { cur_data.set_load_seq_index( _load_seq_index ); }
  uint8 get_sublevel_index() const { return cur_data.get_sublevel_index(); }
  void set_sublevel_index( uint8 _sublevel_index ) { cur_data.set_sublevel_index( _sublevel_index ); }

  void set_next_save_slot();
  void set_next_load_slot( int slot ) { assert( slot > 0 && slot <= MAX_GAMEFILE_SLOTS ); next_load_slot = slot; }

  int get_num_saved_files() const;
  
  // access to saved file info; index is index into file_info array
//P  const storage_time &get_time_stamp( int index ) const;
  const char *get_sublevel( int index ) const;
  int get_slot_num( int index ) const;
  const stringx get_storage_time_text( int index ) const;

private:
//P  void update_saved_file_info( const storage_time &time_saved );  // updates catalogue with current gamefile and passed time
  void sort_file_info();

  cur_data_t cur_data;  // this is the object that actually gets saved

  int next_save_slot;  // set by get_saved_file_info (1 - MAX_GAMEFILE_SLOTS); keep hidden
  int next_load_slot;
  int num_saved_files;
  file_info_t file_info[MAX_GAMEFILE_SLOTS];  // time-sorted catalogue of game files on current storage unit
  bool file_info_valid;
};


#endif // GAMEFILE_H