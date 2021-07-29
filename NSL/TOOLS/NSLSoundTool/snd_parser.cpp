#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#pragma warning(push)
#pragma warning( disable : 4786 )
#include <string>
#include <list>

#include "snd_parser.h"
#include "nslSoundTool.h"

vector<snd_info_t> snd_info_t::default_info;

struct alias_info_t
{
	char name[PATH_MAX];
  char alias[PATH_MAX];
  int id;
};

//typedef std::map<string, int> string2int_map;
//static string2int_map snd_ids_map;
//static snd_last_id;
typedef std::list<alias_info_t> snd_ids_list_t;
snd_ids_list_t snd_ids_list;

int generate_alias_id( char * alias, char * name )
{
  int id;

  id = snd_ids_list.size() + 1;
  snd_ids_list_t::iterator search;

  for ( search = snd_ids_list.begin(); search != snd_ids_list.end(); ++search )
  {
    if ( stricmp( (*search).alias, alias ) == 0 )
    {
      return (*search).id;
    }
  }

  alias_info_t alias_info;
  strcpy( alias_info.alias, alias );
  strcpy( alias_info.name, name );
  alias_info.id = id;
  snd_ids_list.insert( snd_ids_list.end(), alias_info );

  return id;
}

void export_alias_ids( char * output_dir )
{
  snd_ids_list_t::iterator iter;
  char fname[256];
  FILE * f;

  // export SoundIDs.h
  strcpy( fname, output_dir );
  if ( fname[strlen( fname ) - 1] != '\\' )
  {
    strcat( fname, "\\" );
  }
  strcat( fname, "SoundIDs.h" );

  f = fopen( fname, "wt" );

  for ( iter = snd_ids_list.begin(); iter != snd_ids_list.end(); iter++ )
  {
    fprintf( f, "#define %s %d\n", (*iter).alias, (*iter).id );
  }

  fclose( f );

  // export SoundDef.h
  strcpy( fname, output_dir );
  if ( fname[strlen( fname ) - 1] != '\\' )
  {
    strcat( fname, "\\" );
  }
  strcat( fname, "SoundDef.h" );

  f = fopen( fname, "wt" );

  fprintf( f, "#ifndef SoundDef_h\n" );
  fprintf( f, "#define SoundDef_h\n" );
  fprintf( f, "\n" );
  fprintf( f, "#include \"SoundIDs.h\"\n" );
  fprintf( f, "\n" );
  fprintf( f, "struct ASoundDef\n" );
  fprintf( f, "{\n" );
  fprintf( f, "  int soundId;\n" );
  fprintf( f, "  char soundName[64];\n" );
  fprintf( f, "};\n" );
  fprintf( f, "\n" );
  fprintf( f, "ASoundDef SoundDefs[] =\n" );
  fprintf( f, "{\n" );

  for ( iter = snd_ids_list.begin(); iter != snd_ids_list.end(); iter++ )
  {
    fprintf( f, "  { %s, \"%s\" },\n", (*iter).alias, (*iter).name );
  }

  fprintf( f, "};\n" );
  fprintf( f, "\n" );
  fprintf( f, "const int SoundDefsNum = sizeof(SoundDefs) / sizeof(ASoundDef);\n" );
  fprintf( f, "\n" );
  fprintf( f, "#endif // SoundDef_h\n" );

  fclose( f );
}

#pragma warning(pop)

void snd_info_t::clear(bool free_mem = true)
{
  path[0] = '\0';
  name[0] ='\0';
  alias[0] = '\0';
  volume_left = 1.0f;
  volume_right = 1.0f;
  pitch = 1.0f;
  loop = false;
  type = NSL_SOURCETYPE_SFX;
  cd = false;
  spu = false;
  ps2.clear(free_mem);
  xbox.clear(free_mem);
  gc.clear(free_mem);
  for (int j=0; j<NSL_LANGUAGE_Z; ++j)
  {
    languages[j] = false;
  }
}

void snd_info_t::copy( const snd_info_t &other )
{
  safe_strcpy( path, (char *)other.path );
  safe_strcpy( name, (char *)other.name );
  safe_strcpy( alias, (char *)other.alias );
  aliasID = other.aliasID;
  volume_left = other.volume_left;
  volume_right = other.volume_right;
  pitch = other.pitch;
  loop = other.loop;
  cd = other.cd;
  spu = other.spu;
  type = other.type;
// moved to ps2 specific portion  memcpy( realSize, other.realSize, sizeof(unsigned int)*NSL_LANGUAGE_Z );
  applyReverb = other.applyReverb;
  ps2.copy(other.ps2);
  xbox.copy(other.xbox);
  gc.copy(other.gc);
  
  for (int j=0; j<NSL_LANGUAGE_Z; ++j)
  {
    languages[j] = other.languages[j];
  }
}



void snd_info_t::set_name_with_path( char * full_name, bool set_name_flag, bool set_path_flag )
{
  char* ret = full_name;
  char* ret1 = strrchr( full_name, '\\' );
  char* ret2 = strrchr( full_name, '/' );
  if (ret1 && ret2)
  {
    if (ret1 > ret2)
      ret = ret1 + 1;
    else
      ret = ret2 + 1;
  }
  else if (ret1)
  {
    ret = ret1 + 1;
  }
  else if (ret2)
  {
    ret = ret2 + 1;
  }
  if (ret != full_name)
  {
    assert(ret > full_name);
    int path_length = ((int)ret - (int)full_name) + 1;
    assert(path_length < PATH_MAX);
    if (set_path_flag)
    {
      safe_strcpy(path, full_name, path_length);
    }
    if (set_name_flag)
    {
      safe_strcpy(name, ret);
    }
  }
  else
  {
    // no path
    if (set_path_flag)
    {
      path[0] = '\0';
    }
    if (set_name_flag)
    {
      safe_strcpy(name, full_name);
    }
  }

  // we force the names to lower-case for comparisons
  strlwr(name);
}

void snd_info_t::set_to_default(  )
{
  // brute force linear search... if we need to speed it up, we can.
  vector<snd_info_t>::const_iterator it;
  vector<snd_info_t>::const_iterator it_end = default_info.end();
  for ( it = default_info.begin(); it != it_end; ++it )
  {
    if ( strcmp(name, (*it).name) == 0)
    {
      // copy over its guts
      copy( (*it) );
      break;
    }
  }
}

void load_info_defaults( char * global_snd_filename )
{
  snd_info_t::default_info.clear();
  FILE *defaults = fopen( global_snd_filename, "rb" );
  if ( defaults == NULL )
  {
    // try prepending the input snd directory
    char full_path[1024];
  	_snprintf( full_path, sizeof( full_path ), "%s\\%s", opt.input_snd_dir, global_snd_filename );

    defaults = fopen( full_path, "rb" );
  }
  if ( defaults == NULL )
  {
    error( "could not find default snd info files %s\n", global_snd_filename );
  }
  else
  {
    // load in the defaults
    char line[1024];
	  while( !feof( defaults ) ) {
		  snd_info_t info;

		  // parse line
		  fgets( line, sizeof( line ), defaults );

	    if( line[0] == '\0' || ( line[0] == ';' ) || ( line[0] == '\r' ) || ( line[0] == '\n' ) )
		    continue;

		  build_info( line, &info, false );
      snd_info_t::default_info.push_back( info );
	  }

	  // close file
	  fclose( defaults );    
  }
}


const char* next_token( const char* src, char* dst, size_t n )
{
	unsigned int si = 0;
	unsigned int di = 0;

	// catch special case of an empty string
	if( ( src == NULL ) || ( src[0] == '\0' ) ) 
  {
		return NULL;
	}

  while ( src[si] == '\n' )
    ++si;

	while( 1 ) 
  {

		if( di >= n ) 
    {
			return &src[si];
		}

		if( src[si] == '\0' ) 
    {
			dst[di] = '\0';
			return &src[si];
		}
		
		if( src[si] == ' ' || src[si] == '\t' || src[si] == '\n' || src[si] == '\r' ) 
    {
			dst[di] = '\0';
			return &src[++si];
		}

		dst[di] = src[si];
		++si;
		++di;
	}

	// impossible state
	return NULL;
}

void build_info( const char* line, snd_info_t* info, bool consider_default )
{
	char token[PATH_MAX];
	const char* caret = next_token( line, token, sizeof( token ) );

  info->set_name_with_path( token );

  if ( consider_default )
    info->set_to_default( );

	info->applyReverb = false; // For this field the default is false.

  strcpy( info->alias, "SND_" );
  strcat( info->alias, info->name );

	while( ( caret = next_token( caret, token, sizeof( token ) ) ) ) 
  {
    strlwr(token);
	if( stricmp( token, "loop" ) == 0 ) 
    {
			info->loop = true;
	} 
    else if( stricmp( token, "cd" ) == 0 ) 
    {
			info->cd = true;
		}
    else if( stricmp( token, "spu" ) == 0 ) 
    {
      info->spu = true;
    }
    else if( stricmp( token, "alias" ) == 0 ) 
    {
      caret = next_token( caret, token, sizeof( token ) );
      safe_strcpy( info->alias, token );
    }
    else if( stricmp( token, "volume" ) == 0 ) 
    {
      caret = next_token( caret, token, sizeof( token ) );
      float vol = (float)atoi( token ) / 100.0f;
      info->volume_left = vol;
      info->volume_right = vol;
    }
    else if( stricmp( token, "voll" ) == 0 ) 
    {
      caret = next_token( caret, token, sizeof( token ) );
      float vol = (float)atoi( token ) / 100.0f;
      info->volume_left = vol;
    }
    else if( stricmp( token, "volr" ) == 0 ) 
    {
      caret = next_token( caret, token, sizeof( token ) );
      float vol = (float)atoi( token ) / 100.0f;
      info->volume_right = vol;
    }
    else if( stricmp( token, "pitch" ) == 0 ) 
    {
      caret = next_token( caret, token, sizeof( token ) );
      float pitch = atoi( token ) / 1000.0f;
      info->pitch = pitch;
    }
	else if (stricmp(token, "reverb") == 0)
	{
		info->applyReverb = true;
	}
    else if( stricmp(token, nslSourceTypesStr[NSL_SOURCETYPE_SFX] ) == 0)
    {
      info->type = NSL_SOURCETYPE_SFX;
    }
    else if( stricmp(token, nslSourceTypesStr[NSL_SOURCETYPE_MUSIC] ) == 0)
    {
      info->type = NSL_SOURCETYPE_MUSIC;
    }
    else if( stricmp(token, nslSourceTypesStr[NSL_SOURCETYPE_AMBIENT] ) == 0)
    {
      info->type = NSL_SOURCETYPE_AMBIENT;
    }
    else if( stricmp(token, nslSourceTypesStr[NSL_SOURCETYPE_VOICE] ) == 0)
    {
      info->type = NSL_SOURCETYPE_VOICE;
    }
    else if( stricmp(token, nslSourceTypesStr[NSL_SOURCETYPE_USER1] ) == 0)
    {
      info->type = NSL_SOURCETYPE_USER1;
    }
    else if( stricmp(token, nslSourceTypesStr[NSL_SOURCETYPE_USER2] ) == 0)
    {
      info->type = NSL_SOURCETYPE_USER2;
    }
    else if( stricmp(token, nslSourceTypesStr[NSL_SOURCETYPE_MOVIE] ) == 0)
    {
      info->type = NSL_SOURCETYPE_MOVIE;
    }
    else 
    {
			if( token[0] ) 
      {
				message( "build_info: unrecognized token '%s'\n", token );
			}
		}
	}

  strupr( info->alias );
  info->aliasID = generate_alias_id( info->alias, info->name );
}
