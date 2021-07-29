//
// Generic text parsing class
//
// Author: Leonardo Zide
//

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if _XBOX
#include "stringx.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "text_parser.h"
#include "wds.h"

text_parser::text_parser ()
{
  buffer = NULL;
  script_ptr = NULL;
}

text_parser::~text_parser ()
{
  cleanup ();
}

void text_parser::cleanup ()
{
	#if 1
  // This is a hack!
  if (last_val_pos)
    *last_val_pos = last_val;
  g_world_ptr->wds_releasefile( (unsigned char **) &buffer);
	#else
  free (buffer);
  buffer = NULL;
	#endif
  script_ptr = NULL;
}

bool text_parser::load_file (stringx& filename)
{
  last_val_pos = 0;

  cleanup ();
  script_line = 1;

	#if 1

  unsigned int file_size;
  if (g_world_ptr->wds_readfile( filename.c_str(), (unsigned char **) &buffer, &file_size, 1))
	{
		script_ptr = buffer;
    // Grab that last val to restore later.
    // this is a hack for Milestone 2
    last_val = buffer[file_size];
    last_val_pos = &buffer[file_size];
    // This writes outside the bounds of the array
    buffer[file_size] = '\0';
		return true;
	}
  cleanup ();
	return FALSE;


	#else
  os_file file;
  file.open (filename, os_file::FILE_READ);

  if (file.is_open ())
  {

    file.set_fp (0, os_file::FP_BEGIN);
    script_ptr = buffer = (char *)malloc (file_size + 1);
    buffer[file_size] = '\0';

    if (file.read (buffer, file_size))
      return true;
  }
  cleanup ();
  return false;
	#endif

}

bool text_parser::get_token (bool crossline, bool optional)
{
  char *token_p;

skipspace:
  while (*script_ptr <= 32)
  {
    // EOF 
    if (*script_ptr == 26)
    {
      if (!crossline && !optional)
      {
      	debug_print ("text_parser: Script line %i is incomplete\n", script_line);
        assert (false);
      }
      return false;

    }
    if (!*script_ptr)
    {
      if (!crossline && !optional)
      {
      	debug_print ("text_parser: Script line %i is incomplete\n", script_line);
        assert (false);
      }
      return false;
    }

    if (*script_ptr++ == '\n')
    {
      if (!crossline)
      {
        if (optional)
          return false;
        else
        {
          debug_print ("text_parser: Script line %i is incomplete\n", script_line);
          assert (false);
        }
      }
      script_line++;
    }
  }

  // comment field
  if (script_ptr[0] == '/' && script_ptr[1] == '/')
  {
    if (!crossline && !optional)
    {
      if (optional)
        return false;
      else
      {
        debug_print ("text_parser: Script line %i is incomplete\n", script_line);
        assert (false);
      }
    }

    while ((*script_ptr != '\n') && ( *script_ptr != 0x26 )) 
      if (!*(++script_ptr))
      {
        if (!crossline && !optional)
        {
          debug_print ("text_parser: Script line %i is incomplete\n", script_line);
          assert (false);
        }
        return false;
      }
      goto skipspace;
  }

  // copy token
  token_p = token;

  if (*script_ptr == '"')
  {
    script_ptr++;
    while (*script_ptr != '"')
    {
      if (!*script_ptr)
      {
        debug_print ("text_parser: EOF inside quoted token\n");
        assert (false);
      }
      *token_p++ = *script_ptr++;
      if (token_p == &token[MAXTOKEN])
      {
        debug_print ("text_parser: Token too large on line %i\n",script_line);
        assert (false);
      }
    }
    script_ptr++;
  }
  else while (*script_ptr > 32)
  {
    *token_p++ = *script_ptr++;
    if (token_p == &token[MAXTOKEN])
    {
      debug_print ("text_parser: Token too large on line %i\n",script_line);
      assert (false);
    }
  }

  *token_p = 0;

  return true;
}
