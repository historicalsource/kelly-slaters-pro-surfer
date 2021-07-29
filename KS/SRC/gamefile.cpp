#include "global.h"

#include "osdevopts.h"
#include "gamefile.h"
#include "game.h"
#include "wds.h"
#include "controller.h"
#include "interface.h"
#include "entity_maker.h"
#include "entityflags.h"
//P #include "optionsfile.h"
//P #include "warning.h"
#ifdef TARGET_MKS
//#include "vmu_beep.h"
#endif

#define GAMEFILE_PREFIX   "MAXSTEEL.00"


extern game *g_game_ptr;



//--------------------------------------------------------------

gamefile_t::cur_data_t::cur_data_t()
{
  version_num = DEF_GAMEFILE_VERSION_NUM;
  heroname[0][0] = '\0';
  heroname[1][0] = '\0';
  init();
}


//--------------------------------------------------------------

void gamefile_t::cur_data_t::store_hero_to_cur_data()
{
  assert( g_game_ptr->get_world()->get_hero_ptr(g_game_ptr->get_active_player()) );
  STUBBED(store_hero_to_cur_data, "gamefile_t::cur_data_t::store_hero_to_cur_data");

  // mark valid data
  hero_data_valid = true;
}


//--------------------------------------------------------------

void gamefile_t::cur_data_t::restore_hero_from_cur_data()
{
  assert( g_game_ptr->get_world()->get_hero_ptr(g_game_ptr->get_active_player()) );
  STUBBED(restore_hero_to_cur_data, "gamefile_t::cur_data_t::restore_hero_to_cur_data");
}


//--------------------------------------------------------------

void gamefile_t::cur_data_t::set_heroname(int heronum, const char* name )
{
  assert( strlen(name) < 16 );
  assert( heronum<MAX_PLAYERS );
  strcpy( heroname[heronum], name );
}



//------------------------------------------------------------------------

gamefile_t::file_info_t::file_info_t()
{
  sublevel[0] = 0;
  slot_num = -1;
}


//------------------------------------------------------------------------


static int compare_gamefile_infos( const void* arg1, const void* arg2 )
{
/*P
  storage_time stamp1 = ((gamefile_t::file_info_t *)arg1)->get_time_stamp();
  storage_time stamp2 = ((gamefile_t::file_info_t *)arg2)->get_time_stamp();

  if ( stamp1 < stamp2 )
    return -1;
  if ( stamp2 < stamp1 )
    return 1;
P*/
  return 0;
}



//------------------------------------------------------------------------

gamefile_t::gamefile_t()
{
  next_save_slot = -1;
  next_load_slot = -1;
  num_saved_files = -1;
  file_info_valid = false;
}


//------------------------------------------------------------------------

bool gamefile_t::save()
{
  bool err = false;
/*P
  if ( syvars->storage_unit_exists && !g_game_ptr->get_vmu_checker()->do_skip_save() && !g_game_ptr->get_vmu_checker()->cur_vmu_is_full() )
  {
    if ( g_game_ptr->get_vmu_checker()->cur_vmu_is_different() ||
         g_game_ptr->get_vmu_checker()->cur_vmu_was_removed() )
    {
      err = true;
    }
    else
    {
      if ( !file_info_valid )
      {
        error( "Tried to use invalid value for next_save_slot" );
        assert( 0 );
      }
    
      // build file name from next avail slot
      assert( next_save_slot > 0 && next_save_slot < 10 ); // must be one digit
      stringx filename = stringx( GAMEFILE_PREFIX ) + stringx( next_save_slot );


      // build comments
      stringx shortdesc = os_developer_options::inst()->get_string(os_developer_options::STRING_GAME_TITLE);
      stringx longtitle = os_developer_options::inst()->get_string(os_developer_options::STRING_GAME_LONG_TITLE);

      const char *s = g_game_ptr->get_sublevel( cur_data.get_load_seq_index(), cur_data.get_sublevel_index() ).c_str();
      assert( strlen(s) < MAX_SUBLEVEL_LEN );
      stringx longdesc = longtitle + " " + stringx(s);

      // save that puppy
      storage_unit *sup = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );
      storage_time time_saved;
      if ( sup )
      {
        err = !sup->save_file( filename.c_str(), (void*)&cur_data, sizeof(gamefile_t::cur_data_t), shortdesc.c_str(), longdesc.c_str(), g_game_ptr->get_icon_data(), 0, &time_saved,
          (g_game_ptr->get_optionsfile()->get_option(GAME_OPT_VMU_AUDIO) == 0) );
      }
      else
        err = true;
      storage_mgr::inst()->release_unit( sup );

      // update file_info array
      if ( !err )
        update_saved_file_info( time_saved );
    }
  }
  g_game_ptr->get_vmu_checker()->set_skip_save(false);
P*/
  return err;
}

//------------------------------------------------------------------------

bool gamefile_t::load()
{
  uint32 filesize = 1;
/*P
  if ( syvars->storage_unit_exists && !g_game_ptr->get_vmu_checker()->cur_vmu_was_removed() )
  {    
    if ( !file_info_valid )
    {
      error( "Tried to use invalid file info preparing for load" );
      assert( 0 );
    }

    // build file name from next slot to be loaded
    assert( next_load_slot > 0 && next_load_slot < 10 ); // must be one digit
    stringx filename = stringx( GAMEFILE_PREFIX ) + stringx( next_load_slot );

    // load that puppy
    storage_unit *sup = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );
    filesize = sup->load_file( filename.c_str(), (void*)&cur_data, sizeof(gamefile_t::cur_data_t), 0,
      (g_game_ptr->get_optionsfile()->get_option(GAME_OPT_VMU_AUDIO) == 0) );
    storage_mgr::inst()->release_unit( sup );
    if ( filesize )
    {
      g_level_prefix = g_game_ptr->get_level_prefix();
    }
  }
P*/
  return ( filesize == 0 );
}



//------------------------------------------------------------------------

// fills file_info array, sorts it by time, and sets num_saved_files and next_save_slot
void gamefile_t::get_saved_file_info()
{
/*P
  assert( MAX_GAMEFILE_SLOTS < 10 );  // we assume below that slot nums are one digit

  next_save_slot = -1;
  num_saved_files = -1;
  file_info_valid = false;

  vector<stringx> filenames;
  vector<void *> data;
//P  vector<storage_time> timestamps;

  if ( syvars->storage_unit_exists && !g_game_ptr->get_vmu_checker()->cur_vmu_was_removed() )
  {
    storage_unit *sup = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );
    bool res = sup->get_file_info( &filenames, &data, &timestamps, GAMEFILE_PREFIX,
      false );  // no beep since we haven't loaded options yet and don't know whether vmu audio should be on
    storage_mgr::inst()->release_unit( sup );
    if ( !res )
    {
      // SET num_saved_files
      num_saved_files = filenames.size();

      // FILL data in file_info array
      vector<stringx>::iterator filenames_vit = filenames.begin();
      vector<void *>::iterator data_vit = data.begin();
      vector<storage_time>::iterator timestamps_vit = timestamps.begin();
      int i;

      for ( i = 0; i < num_saved_files; ++i, ++filenames_vit, ++data_vit, ++timestamps_vit )
      {
        // check version num on gamefile
        cur_data_t *load_data = (cur_data_t*)(*data_vit);
        if ( load_data->get_version_num() != DEF_GAMEFILE_VERSION_NUM )
        {
          g_game_ptr->do_warning_process( "Obsolete gamefile version detected.  Please delete your Max Steel gamefiles.", 24.0f*3600.0f );
          return;
        }

        // get slot num
        file_info[i].slot_num = atoi( ((*filenames_vit).substr(11)).c_str() );

        // get sublevel
        uint8 level_index = load_data->get_load_seq_index();
        uint8 sublevel_index = load_data->get_sublevel_index();
        const char *s = g_game_ptr->get_sublevel( level_index, sublevel_index ).c_str();
        assert( strlen(s) < MAX_SUBLEVEL_LEN );
        strcpy( file_info[i].sublevel, s );
        
        // get time stamp
        file_info[i].time_stamp = *timestamps_vit;
      }

      sort_file_info();
      set_next_save_slot();

      // our data is valid now
      file_info_valid = true;
    }
    else
    {
#ifdef TARGET_MKS
      vmu_sad_beep( syvars->cur_storage_unit_slot, BEEP_LENGTH*2 );
#else
      error( "Error updating file info." );
#endif
    }
  }

  vector<void *>::iterator vit = data.begin();
  vector<void *>::iterator vit_end = data.end();
  for( ; vit != vit_end; ++vit )
  {
    delete[] *vit;
  }
  P*/
}




//------------------------------------------------------------------------

// updates same vars as get_saved_file_info(), but adds current gamefile to 
// the catalogue instead of reading VMU for a full update
/*P
void gamefile_t::update_saved_file_info( const storage_time &time_saved )
{
  assert( MAX_GAMEFILE_SLOTS < 10 );  // we assume below that slot nums are one digit

  int just_saved_slot = next_save_slot;

  // look for this slot num among already saved files
  int new_index = num_saved_files;
  for ( int i = 0; i < num_saved_files; ++i )
  {
    if ( file_info[i].slot_num == just_saved_slot )
    {
      new_index = i;  // we will be replacing info in slot i
      break;
    }
  }
  
  // if not replacing, add to num_saved_files
  if ( new_index == num_saved_files )
  {
    ++num_saved_files;
    assert( num_saved_files <= MAX_GAMEFILE_SLOTS );
  }

  // put info into file_info array
  //    time
  file_info[new_index].time_stamp = time_saved;
  //    sublevel
  uint8 level_index = cur_data.get_load_seq_index();
  uint8 sublevel_index = cur_data.get_sublevel_index();
  const char *s = g_game_ptr->get_sublevel( level_index, sublevel_index ).c_str();
  assert( strlen(s) < MAX_SUBLEVEL_LEN );
  strcpy( file_info[new_index].sublevel, s );
  //    slot num
  file_info[new_index].slot_num = just_saved_slot;

  sort_file_info();
  set_next_save_slot();
}
P*/


//------------------------------------------------------------------------

// SORT file_info array by time stamp
void gamefile_t::sort_file_info()
{
  qsort( file_info, num_saved_files, sizeof(file_info_t), compare_gamefile_infos );
}


// next_save_slot will be set to next one available in allowable range or,
// if all slots are filled, to that of the oldest file
void gamefile_t::set_next_save_slot()
{
/*P
  next_save_slot = -1;
  int oldest_file_slot = 0;
  storage_time earliest_time;
  bool slot_filled[MAX_GAMEFILE_SLOTS];
  int i;

  for ( i = 0; i < MAX_GAMEFILE_SLOTS; ++i )
  {
    slot_filled[i] = false;
  }

  for ( i = 0; i < num_saved_files; ++i )
  {
    slot_filled[file_info[i].slot_num-1] = true;

    // is this the oldest file?
    if ( i == 0 || file_info[i].time_stamp < earliest_time )
    {
       earliest_time = file_info[i].time_stamp;
       oldest_file_slot = file_info[i].slot_num;
    }
  }

  // find avail slot; if none, use oldest file's slot
  bool slot_found = false;
  for ( i = 0; i < syvars->max_gamefile_slots; ++i )
  {
    if ( !slot_filled[i] )
    {
      next_save_slot = i+1;
      slot_found = true;
      break;
    }
  }
  if ( !slot_found )
  {
    next_save_slot = oldest_file_slot;
  }
P*/
}

//------------------------------------------------------------------------


int gamefile_t::get_num_saved_files() const
{
/*P
  if ( !syvars->storage_unit_exists || g_game_ptr->get_vmu_checker()->cur_vmu_was_removed() )
    return 0;

  if ( !file_info_valid )
  {
    error( "Tried to access invalid gamefile info in get_num_saved_files()." );
    assert( 0 );
  }
P*/
  return num_saved_files;
}
  

/*P
const storage_time &gamefile_t::get_time_stamp( int index ) const
{  
  if ( !file_info_valid && syvars->storage_unit_exists )
  {
    error( "Tried to access invalid gamefile info in get_time_stamp()." );
    assert( 0 );
  }

  assert( index >= 0 && index < num_saved_files );
  return file_info[index].time_stamp;
}
P*/
const char *gamefile_t::get_sublevel( int index ) const
{
  if ( !file_info_valid /*H && syvars->storage_unit_exists H*/ )
  {
    error( "Tried to access invalid gamefile info in get_sublevel()." );
    assert( 0 );
  }

  assert( index >= 0 && index < num_saved_files );
  return file_info[index].sublevel;
}


int gamefile_t::get_slot_num( int index ) const
{
  if ( !file_info_valid /*H && syvars->storage_unit_exists H*/ )
  {
    error( "Tried to access invalid gamefile info in get_slot_num()." );
    assert( 0 );
  }

  assert( index >= 0 && index < num_saved_files );
  return file_info[index].slot_num;
}


const stringx gamefile_t::get_storage_time_text( int index ) const
{
/*P
  if ( !syvars->storage_unit_exists || g_game_ptr->get_vmu_checker()->cur_vmu_was_removed() )
    return empty_string;
P*/
  if ( !file_info_valid )
  {
    error( "Tried to access invalid gamefile info in get_storage_time_text()." );
    assert( 0 );
  }

  assert( index >= 0 && index < num_saved_files );

  stringx time = stringx();
#ifdef TARGET_MKS
  char buf[4];
  storage_time time_stamp = file_info[index].get_time_stamp();
  sprintf( buf, time_stamp.minute > 9 ? "%i" : "0%i", time_stamp.minute );
  time = stringx(time_stamp.month) + stringx("-") + stringx(time_stamp.day) + stringx("-") + stringx(time_stamp.year)
         + stringx("  ") + stringx(time_stamp.hour) + stringx(":") + stringx(buf);
#endif
  return time;
}



//------------------------------------------------------------------------
