//SNDEntry.cpp


#include "stdafx.h"
#include "SNDEntry.h"


SNDEntry::SNDEntry()
{
  name = NULL;
  snd_entry = NULL;
  used = false;
}

SNDEntry::SNDEntry(const SNDEntry& c)
{
  if( c.name )
  {
    name = new char[strlen(c.name) + 1];
    strcpy( name, c.name );
  }
  else
    name = NULL;

  if( c.snd_entry )
  {
    snd_entry = new char[strlen(c.snd_entry) + 1];
    strcpy( snd_entry, c.snd_entry );
  }
  else
    snd_entry = NULL;

  used = false;
}

SNDEntry::~SNDEntry()
{
  if( name )
    delete [] name;

  if( snd_entry )
    delete [] snd_entry;
}


bool SNDEntry::parse_line( const char *line )
{
  const char *whitespace, *end_of_valid;
  
  whitespace = find_whitespace( line );
  end_of_valid = find_end_of_valid( line );

  if( end_of_valid > whitespace )
    end_of_valid = whitespace;

  if( end_of_valid == line )
    return false;

  const char *end_of_path = strrchr( line, '\\' );

  if( end_of_path == NULL || end_of_path > end_of_valid )
    end_of_path = line;
  else
    end_of_path++;

  if( end_of_path == end_of_valid )
    return false;

  int len = end_of_valid - end_of_path;

  name = new char[len + 1];

  strncpy( name, end_of_path, len );
  name[ len ] = '\0';

  end_of_valid = find_end_of_valid( line );

  len = end_of_valid - line;

  snd_entry = new char [len + 1];

  strncpy( snd_entry, line, len );
  
  snd_entry[ len ] = '\0';

  return true;
}

  
const char *SNDEntry::find_end_of_valid( const char *line )
{
  const char *ptr = line;

  while( *ptr != ';' && *ptr != '\0' && *ptr != '\n' )
    ptr++;

  return ptr;
}

const char *SNDEntry::find_whitespace( const char *line )
{
  const char *ptr = line;

  while( *ptr != ' ' && *ptr != '\t' )
    *ptr++;

  return ptr;
}

