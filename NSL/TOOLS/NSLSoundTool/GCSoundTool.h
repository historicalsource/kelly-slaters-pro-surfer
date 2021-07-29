#ifndef _GC_SOUND_TOOL_H
#define _GC_SOUND_TOOL_H

struct gc_info_t 
{
  void clear (bool free_buffer = true)
  {
    // clear/init any member data here
  }
  void copy ( const gc_info_t &other )
  {
    // copy any member data here
  }
};

struct snd_info_t;

// called before any export occurs
int pre_export_gc( void );
// called after all per-snd exporting is finished
int post_export_gc( void );

// called before beginning to export per-file data for a level
int start_level_gc( const char* level );
// called for each file listed in an input SND file
int export_file_gc( snd_info_t* info );
// called after all files from an input SND have been exported
int end_level_gc( const char* level );

#endif