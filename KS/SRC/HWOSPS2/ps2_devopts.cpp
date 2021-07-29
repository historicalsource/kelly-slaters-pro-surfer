/*
ps2_devopts.cpp
*/

#include "global.h"

#include "ps2_devopts.h"

#include "debug.h"
#include "projconst.h"
#include "oserrmsg.h"
#include "osalloc.h"
#include <algorithm>

//#define DONT_READ_INI_FILE 1

DEFINE_SINGLETON(os_developer_options)

char* flag_names[] =
{
#define MAC(x,y,d) y,
#include "devoptflags.h"
#undef MAC
};

int flag_defaults[] =
{
#define MAC(x,y,d) d,
#include "devoptflags.h"
#undef MAC
};

char* string_names[] =
{
#define MAC(x,y,d) y,
#include "devoptstrs.h"
#undef MAC
};

char *string_defaults[] =
{
#define MAC(x,y,d) d,
#include "devoptstrs.h"
#undef MAC
};

char* int_names[] =
{
#define MAC(x,y,d) y,
#include "devoptints.h"
#undef MAC
};

int int_defaults[] =
{
#define MAC(x,y,d) d,
#include "devoptints.h"
#undef MAC
};


#define MAX_STR_LEN         256
#define APP_SECT_DELIMITER  ' '   // Delimits section names in return string
#define APP_KEY_DELIMITER   ' '   // Delimits key names in return string

typedef const char * LPCTSTR;
typedef char * LPTSTR;

// To avoid multiple open and close
static stringx    name = empty_string;
static text_file  inifile;
static char buf[MAX_STR_LEN];     // Should never be used in few functions simultaneously


/*******************************************************************
By some strange reason this function exists in mks win library only.
*******************************************************************/
// this is declared in global.h
int stricmp( const char *s1, const char *s2 )
{
  const char  *p1 = s1, *p2 = s2;
  int         rv = 0;

  assert( p1 != NULL && p2 != NULL );
  do
  {
    rv = toupper(*p1) - toupper(*p2);
    if( *p1++ == '\x0' || *p2++ == '\x0' )
        break;
  } while( rv == 0 );
  return rv;
}

/*******************************************************************
Returns pointer to the key value substring if str contains key,
NULL otherwise.
*******************************************************************/
static char *getKeyValPos( char *str, const char *key, bool stpOnFst = false )
{
  char  *cp, *ep, sv;

  for( cp = str; !isalnum(*cp) && *cp != '_'; ++cp )
    if( *cp == '\x0' || *cp == ';' || (*cp == '/' && *(cp + 1) == '/') )
      return NULL;
  for( ep = cp; isalnum(*ep) || *ep == '_'; ++ep )
    ;
  sv = *ep;
  *ep = '\x0';
  if( stricmp(cp, key) != 0 )
  {
    *ep = sv;
    return NULL;
  }
  *ep = sv;
  while( *ep != '\x0' && isspace(*ep) )
  {
    if( *ep == ';' || (*ep == '/' && *(ep + 1) == '/') )
      return NULL;
    ++ep;
  }
  if( *ep == '\x0' || stpOnFst )
    return ep;
  assert( *ep == '=' );   // check syntax
  ++ep;
  while( *ep != '\x0' && isspace(*ep) )
  {
    if( *ep == ';' || (*ep == '/' && *(ep + 1) == '/') )
      break;
    ++ep;
  }
  return ep;
}

/**********************************************************************
Brings current position in inifile to the next section name, skipping
commented out stuff.
**********************************************************************/
static int nextSect()
{
  int c;
  do
  {
    c = inifile.nextchar();
    if( c == ';' || (c == '/' && (c = inifile.peek_char()) == '/') )
    {
      do
      {
        c = inifile.nextchar();
      } while( c >= 0 && c != '\n' );
    }
  } while( c >= 0 && c != '[' );
  return c;
}

/**********************************************************************
if file with name referenced by pFileName doesn't exist function
returns -1

pSectName != NULL :

Sets current position in .ini file with name, referenced by pFileName,
to the very first line of the section with name referenced by pSectName.
Return value is 0 if section with required name found. If section with
pSectName not found, function returns negative value and current
position in .ini file is undefined.

pSectName == NULL :

First call of the function sets current position in .ini file with name,
referenced by pFileName, to the very first character of the first
section name. Next calls of the function set current positions in the
file to the first character of next section name. Return value is
positive if sect. name found, negative otherwise.

**********************************************************************/
static int GetSection( const char *pFileName, const char *pSectName = NULL )
{
  char  *cp;
  if ( inifile.is_open() && name==pFileName )
    inifile.set_fp( 0, os_file::FP_BEGIN );
  else
  {
    name = pFileName;
    if ( !inifile.text_file_exists(name) )
      return -1;
    if ( inifile.is_open() )
      inifile.set_fp( 0, os_file::FP_BEGIN );
    else
      inifile.open( name );
  }
  if( pSectName == NULL )
    return nextSect();
  do
  {
    if ( nextSect() < 0 )
      return -1;
    inifile.readln( buf, sizeof(buf) );
    cp = getKeyValPos( buf, pSectName, true );
  } while( cp == NULL );
  return 0;
}

static void CloseSection( void )
{
  inifile.close( );
}

/*******************************************************************
This function substitutes corresponding WIN16 function. See VC
library manual for detailes.
*******************************************************************/
static uint32 GetPrivateProfileInt( const char* lpAppName, const char* lpKeyName, int nDefault, const char* lpFileName )
{
  uint32    rv = nDefault, rvf;
  char    *cp;

  assert( lpAppName != NULL && lpKeyName != NULL && lpFileName != NULL );
  if( GetSection(lpFileName, lpAppName) != 0 ) return rv;
  do
  {
    if( inifile.readln(buf, sizeof(buf)) != 0 )
    {
      if( (cp = getKeyValPos( buf, lpKeyName )) != NULL )
      {
        if( sscanf(cp, "%d", &rvf) == 1 )         // Correct value in rv
        {
          rv = rvf;
          break;
        }
        else
        {                                 // Key found, but value is incorrect, what to do?
        }                                         // - just ignore the key for now
      }
    }
  } while( !inifile.at_eof() && strchr(buf, '[') == NULL );
  CloseSection( );
  return rv;
}

/*******************************************************************
Just shortens source and object code.
*******************************************************************/
static uint32 returnDefault( char *cp, int delta, const char* lpDefault, char* lpReturnedString, uint32 nSize )
{
  unsigned int   nchr;

  nchr = strlen( strncpy(lpReturnedString, lpDefault, nSize) );
  if( nchr < strlen(lpDefault) )
  {
    if( delta > 1 )
      *( cp + nchr - 1 ) = '\x0';
    return nSize - delta;
  }
  else
    return nchr;
}

/*******************************************************************
This function substitutes corresponding WIN16 function. See VC
library manual for detailes. The function appears to be too big and
doing meaningless things - that's because it implements ALL WIN16
GetPrivateProfileString's features.
*******************************************************************/
static uint32 GetPrivateProfileString( const char* lpAppName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, uint32 nSize, const char* lpFileName )
{
  int nchr;
  uint32 rv = 0;
  bool expend = false;
  char* cp = lpReturnedString;
  char* bp;

  assert( lpReturnedString!=NULL && lpDefault!=NULL && lpFileName!=NULL );
  if ( lpAppName == NULL )
  {
    while ( rv<nSize-1 && GetSection(lpFileName)>0 )
    {
      nchr = inifile.readln( cp, nSize-rv-1, ']', &expend );
      while ( !isalnum(cp[nchr-1]) && cp[nchr-1]!='_' )
        --nchr;
      rv += nchr;
      cp += nchr;
      if ( rv < nSize-1 )
      {
        *cp++ = APP_SECT_DELIMITER;
        ++rv;
      }
    }
    if ( rv )
    {
      *cp = '\x0';
      if ( !expend )
        {
        rv = nSize - 2;
        *(cp-1) = '\x0';
      }
    }
    else
      rv = returnDefault( cp, 2, lpDefault, lpReturnedString, nSize );
  }
  else
  {
    if ( GetSection(lpFileName,lpAppName) != 0 )
    {
      CloseSection( );
      return returnDefault( cp, (lpKeyName==NULL)?2:1, lpDefault, lpReturnedString, nSize );
    }
    if ( lpKeyName == NULL )
    {
      do
      {
        if ( inifile.readln( buf, sizeof(buf), '\n', &expend ) )
        {
          assert( expend );
          for ( bp = buf; !isspace(*bp) && *bp != '=' && cp < lpReturnedString + nSize; rv++ )
            *cp++ = *bp++;
          if ( cp == lpReturnedString + nSize )
          {
            *(--cp) = '\x0', *(--cp) = '\x0';
            CloseSection( );
            return nSize - 2;
          }
          else
          {
            *cp++ = APP_KEY_DELIMITER;
            ++rv;
          }
        }
      } while ( !inifile.at_eof() && strchr(buf,'[')==NULL );
      *cp = '\x0';
    }
    else
    {
      do
      {
        if ( inifile.readln( buf, sizeof(buf), '\n', &expend ) )
        {
//          assert( expend );     // we ran out of buf space - it needs to be increased
          bp = getKeyValPos( buf, lpKeyName );
          if ( bp != NULL )
          {
            strncpy( cp, bp, nSize );
            rv = strlen( bp );
            if ( rv > nSize )
              rv = nSize - 1;
            break;
          }
        }
      } while ( !inifile.at_eof() && strchr(buf,'[')==NULL );
      if ( rv == 0 )
        rv = returnDefault( cp, 1, lpDefault, lpReturnedString, nSize );
    }
  }
  CloseSection( );
  return rv;
}

#undef MAX_STR_LEN
#undef APP_SECT_DELIMITER
#undef APP_KEY_DELIMITER

os_developer_options::os_developer_options()
{
  parse(empty_string);
}

void os_developer_options::parse( stringx command_line )
{
  // Needs some mechanism to switch
#ifdef TARGET_PS2
  stringx ini_pathname( "GAME.INI" );
#else
  stringx ini_pathname( "..\\GAME.INI" );
#endif

  // open ini file and get stuff
  int i;
  for(i=0;i<NUM_FLAGS;++i)
  {
#ifdef DONT_READ_INI_FILE
    flags[i] = flag_defaults[i];
#else
    flags[i] = GetPrivateProfileInt( "Flags", flag_names[i], flag_defaults[i], ini_pathname.c_str() );
#endif
  }
  flags[FLAG_TEXTURES_LORES] = true;  // currently necessary on Dreamcast (we tend to overflow texture memory otherwise)

  for(i=0;i<NUM_STRINGS;++i)
  {
#ifdef DONT_READ_INI_FILE
    strings[i] = string_defaults[i];
#else
    char dest_buffer[256];
    GetPrivateProfileString("Strings", string_names[i], string_defaults[i], dest_buffer, 256, ini_pathname.c_str() );
    strings[i] = stringx(dest_buffer);
#endif
  }

  for(i=0;i<NUM_INTS;++i)
  {
#ifdef DONT_READ_INI_FILE
    ints[i] = int_defaults[i];
#else
    ints[i] = GetPrivateProfileInt( "Ints", int_names[i], int_defaults[i], ini_pathname.c_str() );
#endif
  }

  // make sure the root dir has a \.
  stringx root_dir = get_string( STRING_ROOT_DIR );
  if ( root_dir[root_dir.length()-1]!='\\' )
    root_dir += stringx( "\\" );
  strings[ STRING_ROOT_DIR ] = root_dir;

  #ifndef BUILD_BOOTABLE
  debug_print(root_dir);
  #endif

  stringx pre_root_dir = get_string( STRING_PRE_ROOT_DIR );
  if ( pre_root_dir[pre_root_dir.length()-1]!='\\' )
    pre_root_dir += stringx( "\\" );
  strings[ STRING_PRE_ROOT_DIR ] = pre_root_dir;

  #ifndef BUILD_BOOTABLE
  debug_print(pre_root_dir);
  #endif

  os_file::set_root_dir( root_dir );
  os_file::set_pre_root_dir( pre_root_dir );
}
