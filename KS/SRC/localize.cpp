/*-------------------------------------------------------------------------------------------------------
  
  Localization support

-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "textfile.h"
#include "errorcontext.h"
#include "oserrmsg.h"
#include "game.h"
#include "localize.h"
#include "osdevopts.h"
//P #include "optionsfile.h"
#include <map>

const char* locales[] =
{
  "english",
  "french",
  "german",
  "spanish",
};
const int NUM_LOCALES = sizeof(locales) / sizeof(char*);

/*
map<stringx,stringx> localized_text_map[NUM_LOCALES];
stringx locale_dir;
*/
extern game *g_game_ptr;

void load_locales()
{
/*
  int i;
  for ( i=0; i<NUM_LOCALES; ++i )
  {
    text_file io;
    stringx token, value, last_valid_token = "...well, actually the first token's bad.";
    ectx( stringx("loading locale ") + locales[i] );

    locale_dir = stringx("locales\\") + locales[i];

    // the way this works is a little wierd, it's changed a little from the original method.
    // the filesystem scans for files in locale_dir automatically before scanning the real directory.
    stringx locale_str(locale_dir + stringx("\\") + "strings.txt");
    io.open( locale_str );

    localized_text_map[i].resize(0);

    for(;;)
    {
      // get the token.
      io.read( &token );

      if ( token == "eof" )
        break;
      if ( token[0] != '$' )
        error( "Invalid token in locale file after "+last_valid_token );

      // get the translated text.
      io.read( &value );
      // insert entry into map
      localized_text_map[i].insert( pair<const stringx,stringx>( token, value ) );
		  last_valid_token = token;
    }

    io.close();
  }
  locale_dir = stringx();  // FIXME: killed localization to speed load on PS2
*/
}


stringx localize_text( stringx src )
{
  if ( src[ 0 ] != '$' )
    return src;

/*P
  int cur_locale = 0;
  if ( g_game_ptr )
  {
    cur_locale = g_game_ptr->get_optionsfile()->get_option(GAME_OPT_LANGUAGE);
  }
  map<stringx,stringx>::iterator it = localized_text_map[cur_locale].find( src );

  if ( it == localized_text_map[cur_locale].end() )
    error( "Localized text not found for " + src + " in " + locale_dir );
P*/
  return src;
//P  return (*it).second;
}


// returns empty string if string not found in txt file
stringx localize_text_safe( stringx src )
{
  if ( src[ 0 ] != '$' )
    return src;
/*P
  int cur_locale = 0;
  if ( g_game_ptr )
  {
    cur_locale = g_game_ptr->get_optionsfile()->get_option(GAME_OPT_LANGUAGE);
  }

  map<stringx,stringx>::iterator it = localized_text_map[cur_locale].find( src );

  if ( it == localized_text_map[cur_locale].end() )
    return stringx();

  return (*it).second;
P*/
  return src;
}


// localize a VO stream filename
stringx localize_VO_stream( const stringx& filename )
{
/*P
  int cur_locale = 0;
  if ( g_game_ptr )
  {
    cur_locale = g_game_ptr->get_optionsfile()->get_option(GAME_OPT_LANGUAGE);
  }
  if ( cur_locale > 0 )
  {
    stringx new_filename = filename;
    new_filename.to_lower();
    filespec fspec( new_filename );
    // look for "_VO\\"
    int i;
    for ( i=0; i<fspec.path.size()-3; ++i )
    {
      if ( !strncmp( fspec.path.c_str()+i, "_vo\\", 4 ) )
      {
        new_filename = fspec.path + locales[cur_locale] + "\\" + fspec.name;
        #ifdef TARGET_MKS
        stringx fullname = os_file::get_pre_root_dir() + "dc_sounds\\"
                         + new_filename + ".str";
        #else
        stringx fullname = os_developer_options::inst()->get_string(os_developer_options::STRING_PC_SOUNDS_DIR)
                         + new_filename + ".wav";
        #endif
        if ( os_file::file_exists(fullname) )
        {
          return new_filename;
        }
        break;
      }
    }
  }
P*/
  return stringx( filename );
}
