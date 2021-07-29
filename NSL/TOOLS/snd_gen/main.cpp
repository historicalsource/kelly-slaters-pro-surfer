//main.cpp

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include "SNDEntry.h"

#include <iostream>
#include <vector>
#include <list>

using namespace std;

#define MAX_FILENAME 256

struct include_file_t
{
  char include_name[MAX_FILENAME];
  char language_name[MAX_FILENAME];
  include_file_t()
  {
    include_name[0] = '\0';
    language_name[0] = '\0';
  }
  include_file_t( const include_file_t & other)
  {
    strcpy( include_name, other.include_name );
    strcpy( language_name, other.language_name );
  }
};

class Options
{
  public:
    Options()
    {
      dry_run = false;
      strcpy( global_snd_name, "global.snd" ); 
      include_file.clear();
    };

    bool dry_run;
    char global_snd_name[MAX_FILENAME];
    vector<include_file_t> include_file;
};




bool parse_global_snd(Options& options, vector<SNDEntry>& entry_list)
{
  FILE *fp;
  char line[1024];

  fp = fopen( options.global_snd_name, "r" );

  if( fp == NULL )
  {
    printf( "Could not open global SND file: %s\n", options.global_snd_name );
    return false;
  }

  while( fgets( line, 1023, fp ) )
  {
    SNDEntry entry;

    if( entry.parse_line( line ) )
    {
      entry_list.push_back( entry );
    }
  }

  fclose( fp );

  return true;
}


int main( int argc, char *argv[] )
{ 
  Options options;

  int curr_arg = 1;
  bool in_args = true;
  vector<SNDEntry> global_entry_list;

  char input_file[256];
  char output_file[256];

  while( curr_arg < argc && in_args )
  {
    if( argv[curr_arg][0] != '-' )
    {
      in_args = false;
      continue;
    }

    switch( argv[curr_arg][1] )
    {
      case 'D':
      case 'd':
        options.dry_run = true;
        curr_arg++;
        break;
      
      case 'G':
      case 'g':
        curr_arg++;
        strcpy( options.global_snd_name, argv[curr_arg] );
        curr_arg++;
        break;

      case 'I':
      case 'i': // specify include file
        {
          curr_arg++;
          include_file_t new_guy;
          strcpy( new_guy.include_name, argv[curr_arg] );
          curr_arg++;
          strcpy( new_guy.language_name, argv[curr_arg] );
          options.include_file.push_back( new_guy );
          curr_arg++;
        }
        break;

      default:
        printf( "Unknown option \'%s\'\n", argv[curr_arg] );
        curr_arg++;
    }
  }

  if( in_args )
  {
    printf( "You must specify audio files\n\n" );
    return -1;
  }

  if( !parse_global_snd(options, global_entry_list) )
  {
    printf( "Could not parse global snd file: %s\n", options.global_snd_name );
    return -3;
  }

  while( curr_arg < argc )
  {
    strcpy( input_file, argv[curr_arg] );
    strcat( input_file, ".lst" );

    strcpy( output_file, argv[curr_arg] );
    strcat( output_file, ".snd" );
    
    FILE *fp_in, *fp_out;

    fp_in = fopen( input_file, "r" );

    if( fp_in == NULL )
    {
      printf( "Could not open %s for reading\n", input_file );
      curr_arg++;
      continue;
    }

    fp_out = fopen( output_file, "w" );

    if( fp_out == NULL )
    {
      printf( "Could not open %s for writing\n", output_file );
      fclose( fp_in );
      curr_arg++;
      continue;
    }

    // write out the include directives
    vector<include_file_t>::iterator it = options.include_file.begin();
    vector<include_file_t>::iterator it_end = options.include_file.end();
    while (it != it_end )
    {
      strupr((*it).language_name);
      fprintf( fp_out, "#include <%s> %s\n", (*it).include_name, (*it).language_name );
      ++it;
    }

    char line[1024];

    vector<SNDEntry>::iterator iter = global_entry_list.begin();

    for( ; iter < global_entry_list.end(); iter++ )
    {
      (*iter).clear_used();
    }

    while( fgets( line, 1023, fp_in ) )
    {
      char *line_ptr = line;

      while( *line_ptr != '\n' && *line_ptr != ' ' && *line_ptr != '\0' )
        line_ptr++;

      *line_ptr = '\0';
      
      line_ptr = strrchr( line, '\\' );
      if( line_ptr == NULL )
        line_ptr = strrchr( line, '/' );
      
      if( line_ptr ) 
        line_ptr++;
      else
        line_ptr = line;

      if( !line_ptr || strlen( line_ptr ) == 0 )
        continue;

      vector<SNDEntry>::iterator it = global_entry_list.begin();

      for( ; it < global_entry_list.end(); it++ )
      {
        if( *it == line_ptr )
          break;
      }

      if( it == global_entry_list.end() )
      {
        printf( "Could not find sound: %s\n", line_ptr );
      }
      else
      {
        if( !(*it).is_used() )
        {
          fprintf( fp_out, "%s\n", (*it).get_snd_entry() );
          (*it).mark_used();
        }
      }
    }

    fclose( fp_out );
    fclose( fp_in );
    
    curr_arg++;
  }
  return 0;
}

