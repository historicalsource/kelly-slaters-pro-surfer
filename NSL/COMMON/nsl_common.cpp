/* This file should contain common stuff to NSL
 * that isn't needed by the PS2's IOP processor, 
 * which is most of the common stuff.  The rest,
 * goes in nsl.cpp.
 */

#include "nsl.h"

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#include <string.h>
#include <ctype.h>
#if defined (NSL_PC) || defined (NSL_SOUND_TOOL)
#include <sys/types.h>
#include <sys/stat.h>
#endif

static char* _strupr( char* s )
{
	char* sp = s;

	while( *sp ) {
		*sp = toupper( *sp );
		++sp;
	}

	return s;
}

nslSourceTypeEnum nslSourceTypeStringToEnum( const char *stringToLookup )
{
  char *lookAtMe = new char[strlen(stringToLookup)+1];
  strcpy(lookAtMe, stringToLookup);
  _strupr(lookAtMe);
  int i;

  for (i=0; i<(int)NSL_SOURCETYPE_Z; ++i)
  {
    if (strcmp(lookAtMe, nslSourceTypesStr[i]) == 0)
    {
      break;
    }
  }
  delete []lookAtMe;

  return (nslSourceTypeEnum)i;
}

nslLanguageEnum nslLanguageStringToEnum( const char *stringToLookup )
{
  char *lookAtMe = new char[strlen(stringToLookup)+1];
  strcpy(lookAtMe, stringToLookup);
  _strupr(lookAtMe);
  int i;

  for (i=0; i<(int)NSL_LANGUAGE_Z; ++i)
  {
    if (strcmp(lookAtMe, nslLanguageStr[i]) == 0)
    {
      break;
    }
  }
  delete []lookAtMe;

  return (nslLanguageEnum)i;
}


#if defined (NSL_PC) || defined (NSL_SOUND_TOOL)

// FIXME: make localized_fopen use this function
int _nslLocalizedPath( const char* file_path,
                       const char* file_name,
                       nslLanguageEnum lang,
                       nslPlatformEnum platform,
                       char* found_filename,
                       char* path_addendum, 
                       bool *used_lang )
{
  char check_name[PATH_MAX];
	int ret_val = 0;
	struct _stat buf;

  // check the localized platform directory first (blah/english/ps2/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, nslLanguageStr[lang]);
  strcat(check_name, "\\");
  strcat(check_name, nslPlatformStr[platform]);
  strcat(check_name, "\\");
  strcat(check_name, file_name);
  ret_val = _stat(check_name, &buf);

  if (ret_val == 0)
  {
    if (found_filename)
    {
			strcpy( found_filename, check_name );
    }
    if (path_addendum)
    {
      strcpy( path_addendum, nslLanguageStr[lang] );
      strcat( path_addendum, "\\" );
      strcat( path_addendum, nslPlatformStr[platform] );
      strcat( path_addendum, "\\" );
    }
    if (used_lang)
    {
      *used_lang = true;
    }

    return 0;
  }
  
  // now try the platform dir (blah/ps2/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, nslPlatformStr[platform]);
  strcat(check_name, "\\");
  strcat(check_name, file_name);
  ret_val = _stat(check_name, &buf);

  if (ret_val == 0)
  {
    if (found_filename)
    {
			strcpy( found_filename, check_name );
    }
    if (path_addendum)
    {
      strcpy( path_addendum, nslPlatformStr[platform] );
      strcat( path_addendum, "\\" );
    }
    if (used_lang)
    {
      *used_lang = false;
    }
    return 0;
  }

  // check the localized directory (blah/english/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, nslLanguageStr[lang]);
  strcat(check_name, "\\");
  strcat(check_name, file_name);
  ret_val = _stat(check_name, &buf);

  if (ret_val == 0)
  {
    if (found_filename)
    {
      strcpy(found_filename, check_name);
    }
    if (path_addendum)
    {
      strcpy(path_addendum, nslLanguageStr[lang]);
      strcat(path_addendum, "\\");
    }
    if (used_lang)
    {
      *used_lang = true;
    }
    return 0;
  }

  // now try the root dir (blah/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, file_name);
  ret_val = _stat(check_name, &buf);

  if (ret_val == 0)
  {
    if (found_filename)
    {
      strcpy(found_filename, check_name);
    }
    if (path_addendum)
    {
      strcpy(path_addendum, "");
    }
    if (used_lang)
    {
      *used_lang = false;
    }
    return 0;
  }

  return -1;
}
#endif