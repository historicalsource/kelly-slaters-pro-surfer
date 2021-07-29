// file_finder.cpp

#include "global.h"

#include "file_finder.h"
#include "osdevopts.h"

//#include <cstdarg>



file_finder::file_finder()
{
  init_path_list();
}


file_finder::~file_finder()
{
  clear_path_list();
}


stringx file_finder::find_file( const stringx &filename, const stringx &extension, bool fail_ok ) const
{
  assert ( !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY) );

	if ( os_file::file_exists( filename+extension ) )
		return filename + extension;

  filespec spec(filename);
	vector<stringx>::const_reverse_iterator i = path_list.rbegin();
	vector<stringx>::const_reverse_iterator i_end = path_list.rend();
	for ( ; i!=i_end; i++ )
  {
    const stringx& s = (*i);
    stringx first = s + spec.name + extension;

		if( os_file::file_exists( first ) )
			return first;

    stringx second = s + "entities\\" + spec.name + extension;

		if( os_file::file_exists( second ) )
			return second;

    stringx tdir = os_developer_options::inst( )->get_string( os_developer_options::STRING_TEXTURE_DIR );
    stringx third = s + tdir + "\\" + spec.name + extension;

		if( os_file::file_exists( third ) )
			return third;

    stringx fourth = s + spec.name + "\\" + spec.name + extension;

    if( os_file::file_exists( fourth ) )
			return fourth;

  }

  if(!fail_ok)
	  error( "Couldn't find \"" + spec.name + extension + "\"" );

	return stringx();
}


bool file_finder_exists(const stringx &filename, const stringx &extension, stringx* found_name)
{
	#ifdef EVAN
		char damnopaquestringclass[256];
		strcpy(damnopaquestringclass,filename.c_str());
	#endif

  if ( os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY) )
  {
    filespec spec(filename);
   	stringx checkname;
		if ( extension.length() )
    	checkname = spec.name + extension;
		else
    	checkname = spec.name + spec.ext;

		#ifdef EVAN
			strcpy(damnopaquestringclass,checkname.c_str());
		#endif
    if (!stash::file_exists(checkname) )
    {
			#if 0
				// just a bit too verbose
      	debug_print("File '%s' not found!", checkname.c_str());
			#endif
      return false;
    }
    if (found_name != NULL)
      *found_name = checkname;
  }
  else
  {
    if (g_file_finder==NULL)
		{
	    if (found_name != NULL)
			{
		    filespec spec(filename);
		   	stringx checkname;
				if ( extension.length() )
		    	checkname = spec.name + extension;
				else
		    	checkname = spec.name + spec.ext;
				#ifdef EVAN
					strcpy(damnopaquestringclass,checkname.c_str());
				#endif
	      *found_name = checkname;
			}
      return true;
		}
    else if ( !g_file_finder->exists(filename, extension, found_name) )
    {
			#if 0
				// just a bit too verbose
      	debug_print("File '%s' not found!", filename.c_str());
			#endif
      return false;
    }
  }
  return true;
}


bool file_finder::exists( const stringx& filename, const stringx& extension, stringx* found_name )
{
  assert ( !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY) );

  bool found = false;
  filespec spec(filename);
  stringx fname = spec.path + spec.name;
  stringx ext = extension.length() ? extension : spec.ext;
  fname += ext;

  if ( os_file::file_exists( fname ) )
    found = true;
  else
  {
    vector<stringx>::const_iterator i = path_list.begin();
    vector<stringx>::const_iterator i_end = path_list.end();
	  for ( ; i!=i_end; ++i )
    {
      fname = *i + spec.name + ext;
	  if ( os_file::file_exists( fname ) )
      {
        found = true;
        break;
	    }
/*      fname = *i + "entities\\" + spec.name + ext;
		  if ( os_file::file_exists( fname ) )
      {
        found = true;
        break;
	    }*/
      fname = *i + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\" + spec.name + ext;
		  if ( os_file::file_exists( fname ) )
      {
        found = true;
        break;
	    }
/*      fname = *i + spec.name + "\\" + spec.name + ext;
		  if ( os_file::file_exists( fname ) )
      {
        found = true;
        break;
	    }*/
    }
/*    if ( !found )
    {
      fname = "fx\\" + spec.name + ext;
		  if ( os_file::file_exists( fname ) )
        found = true;
    }*/
	}
  if ( found )
  {
    if ( found_name )
      *found_name = fname;
    return true;
  }
  return false;
}


void file_finder::init_path_list()
{
  path_list.resize(8);
  clear_path_list();
  path_list.push_back(stringx("items\\"));
  path_list.push_back(stringx("characters\\"));
//  path_list.push_back(stringx("projectiles\\"));
//  path_list.push_back(stringx("fx\\"));
//  path_list.push_back(stringx("frontend\\"));
//  path_list.push_back(stringx("interface\\"));
//  path_list.push_back(stringx("interface\\pda\\"));
//  path_list.push_back(stringx("MXCityscapes\\"));
//  path_list.push_back(os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + stringx("\\"));
}
