
#ifndef XBOXSOUNDTOOL_H
#define XBOXSOUNDTOOL_H

#include <windows.h>
// moved snd header and structs to this file so we don't have multiple copies
#include "../../xbox/nsl_xbox_source.h"

// ExportLevel - Exports a level specific sound bank
// input - the input .SND file
// outputHeader - the output XBox specific header file
// outputBank - the output XBox specific bank file
BOOL ExportLevel(const char * input, const char * inputDir, const char * outputHeader, const char * outputBank);

// ExportStreams - Exports the streams for ALL levels
// During exporting levels all streaming sounds are collected
// This function MUST be called at the end to export the streamed sounds
// outputHeader - the output XBox specific header file
// outputBank - the output XBox specific bank file
BOOL ExportStreams(const char * outputHeader, const char * outputBank);

struct xbox_info_t 
{
  void clear (bool free_buffer = true)
  {
    // clear/init any member data here
  }
  void copy ( const xbox_info_t &other )
  {
    // copy any member data here
  }
};

struct snd_info_t;


// called before any export occurs
int pre_export_xbox( void );
// called after all per-snd exporting is finished
int post_export_xbox( void );

// called before beginning to export per-file data for a level
int start_level_xbox( const char* level );
// called for each file listed in an input SND file
int export_file_xbox( snd_info_t* info );
// called after all files from an input SND have been exported
int end_level_xbox( const char* level );

#endif
