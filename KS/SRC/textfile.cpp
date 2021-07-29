/*-------------------------------------------------------------------------------------------------------

  TEXTFILE.CPP - Text file streaming layer on top of binary file I/O

-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include <math.h>
#include "osfile.h"
#include "textfile.h"
#include "oserrmsg.h"
#include "osdevopts.h"
#include "ngl.h"	// for nglPrintf()

const int BUFFER_SIZE = 2048;

#if defined(TARGET_XBOX)
#include "ngl.h"
#endif /* TARGET_XBOX JIV DEBUG */

//#include <ctype.h>

#if defined(UNICODE)
#define ISDIGIT iswdigit
#define ISSPACE iswspace
#else
#define ISDIGIT isdigit
#define ISSPACE isspace
#endif

#if defined(TARGET_MKS)
MSIPLSTD::_MSLstring::_MSLstring(const string& value)
{
    char* data = NEW char [value.size()+1];
	strcpy(data, value.c_str());
	data_ = data;
}
#endif


//--------------------------------------------------------------
text_file::text_file()
  {
  buf=(char*)os_malloc32(BUFFER_SIZE);
  bufpos=0;
  bufamt=0;
  pushbackdata = -1;
  use_stash = false;
  }

//--------------------------------------------------------------
text_file::~text_file()
  {
  os_free32(buf);
  }

//--------------------------------------------------------------
void text_file::open( const stringx & name, int mode )
{
  // Fix to support reading from multiple stashes
  int oldStash = the_stash.get_current_stash();

  filespec spec(name);
  stringx temp_filename = spec.name + spec.ext;
  if ( (temp_filename != "GAME.INI" && temp_filename != "tricks.dat") &&
       stash::is_stash_open() && mode == os_file::FILE_READ )
  {
//debug_print("This is the filename before sending it in %s", temp_filename.c_str());
    if ( temp_filename.length() <= PSTRING_MAX_LENGTH &&
         the_stash.file_exists(temp_filename.c_str()) )
    {
      // get it from the stash
      use_stash = true;
      my_stash = the_stash.get_current_stash();
      the_stash.open(temp_filename.c_str());
    }
  }
  if (use_stash == false)
  {
//debug_print("This is the culprit %s", temp_filename.c_str());
    if (temp_filename != "GAME.INI" && temp_filename != "tricks.dat")
      assert(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY));
    io.open( name, mode );
  }
  if ( (!!io || the_stash.is_open()) && mode==os_file::FILE_READ )
    refill_buf();

  the_stash.set_current_stash(oldStash);
}

//--------------------------------------------------------------
void text_file::close()
{
  if (use_stash)
    the_stash.close();
  else
    io.close();
}

bool text_file::text_file_exists(const stringx& name)
{
  filespec spec(name);
  stringx temp_filename = spec.name + spec.ext;
  if ( stash::file_exists(temp_filename.c_str()) )
  {
    return true;
  }
  else
  {
    if (temp_filename != "GAME.INI" && temp_filename != "tricks.dat")
      assert(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY));
    return os_file::file_exists(name);
  }
}

//--------------------------------------------------------------
void text_file::read(int* i)
{
  int ret=0;
  bool negative;
  bool found=false;

  eat_whitespace();

  negative=false;
  if (peek_char()=='-') { negative=true; read_char(); }
  else if (peek_char()=='+') { negative=false; read_char(); }

  while (ISDIGIT(peek_char()) && !at_eof())
  {
    found=true;
    ret=ret*10+(read_char()-'0');
  }

  if (!found)
    error("Integer value expected.");

  *i=negative ? -ret : ret;
}

//inv_pow_table[i]=pow(10,-1-i);
static float inv_pow_table[8] =
  {
  0.1f,
  0.01f,
  0.001f,
  0.0001f,
  0.00001f,
  0.000001f,
  0.0000001f,
  0.00000001f,
  };

//--------------------------------------------------------------
void text_file::read(float * f)
{
	char *bufbak=buf;
  float ret=0.0;
  bool negative;
  int exponent;
  bool found=false;

  eat_whitespace();

  negative=false;
  if (peek_char()=='-') { negative=true; read_char(); }
  else if (peek_char()=='+') { negative=false; read_char(); }

  // integer portion (optional)
  while (ISDIGIT(peek_char()) && !at_eof())
    {
    found=true;
    ret=(ret*10)+(read_char()-'0');
    }

  // decimal portion (optional)
  if (peek_char()=='.')
    {
    read_char();

    exponent=0;
    while (ISDIGIT(peek_char()) && !at_eof())
      {
      found=true;
      if (exponent<8)
        ret+=(read_char()-'0')*inv_pow_table[exponent++];
      else
        ret+=(read_char()-'0')*pow(10.0f,-1-(exponent++));
      }
    }

  // apply negative
  if (negative)
    ret*=-1;

  // exponent
  if (toupper(peek_char())=='E')
    {
    read_char();

    negative=false;
    if (peek_char()=='-') { negative=true; read_char(); }
    else if (peek_char()=='+') { negative=false; read_char(); }

    exponent=0;
    while (ISDIGIT(peek_char()) && !at_eof())
      exponent=(exponent*10)+(read_char()-'0');

    // apply exponent negative
    if (negative)
      exponent*=-1;
    ret*=pow(10.0f,exponent);
    }

  if (!found)
    {
		nglPrintf("The damn string is %s\n",bufbak);
    stringx composite = error_context::inst()->get_context()+"Floating point value expected. ";
    error(composite.c_str());
    }

  *f=ret;
}

#if defined(TARGET_XBOX)
// JIV FIXME double check this
//--------------------------------------------------------------
void text_file::read(double * f)
{
	char *bufbak=buf;
  float ret=0.0;
  bool negative;
  int exponent;
  bool found=false;

  eat_whitespace();

  negative=false;
  if (peek_char()=='-') { negative=true; read_char(); }
  else if (peek_char()=='+') { negative=false; read_char(); }

  // integer portion (optional)
  while (ISDIGIT(peek_char()) && !at_eof())
    {
    found=true;
    ret=(ret*10)+(read_char()-'0');
    }

  // decimal portion (optional)
  if (peek_char()=='.')
    {
    read_char();

    exponent=0;
    while (ISDIGIT(peek_char()) && !at_eof())
      {
      found=true;
      if (exponent<8)
        ret+=(read_char()-'0')*inv_pow_table[exponent++];
      else
        ret+=(read_char()-'0')*pow(10.0f,-1-(exponent++));
      }
    }

  // apply negative
  if (negative)
    ret*=-1;

  // exponent
  if (toupper(peek_char())=='E')
    {
    read_char();

    negative=false;
    if (peek_char()=='-') { negative=true; read_char(); }
    else if (peek_char()=='+') { negative=false; read_char(); }

    exponent=0;
    while (ISDIGIT(peek_char()) && !at_eof())
      exponent=(exponent*10)+(read_char()-'0');

    // apply exponent negative
    if (negative)
      exponent*=-1;
    ret*=pow(10.0f,exponent);
    }

  if (!found)
    {
		nglPrintf("The damn string is %s\n",bufbak);
    stringx composite = error_context::inst()->get_context()+"Floating point value expected. ";
    error(composite.c_str());
    }

  *f=ret;
}
#endif /* TARGET_XBOX JIV DEBUG */

char work[4096];
//--------------------------------------------------------------
void text_file::read(stringx * s)
  {
  read( work, sizeof(work)-1 );
  *s = work;
  }

bool stashcheck =false;
// read a string
void text_file::read( char* s, int maxlen )
  {
  int i = 0;
  char* cp = s;
  if (stashcheck)
    assert(stash::curstash == 3);
  eat_whitespace();
  // quoted string special case.
  if ( peek_char() == '\"' )
    {
    read_char(); // eat first quote
    while ( peek_char()!='\"' && !at_eof() )
      {
      if ( i >= maxlen )
        {
        *cp = 0;
        error( get_name() + ": string too long for buffer: " + s );
        }
      *cp++ = read_char();
      ++i;
      }
    read_char(); // eat end quote
    }
  else
    {
    while ( !ISSPACE(peek_char()) && !at_eof() )
      {
      if ( i >= maxlen )
        {
        *cp = 0;
        error( get_name() + ": string too long for buffer: " + s );
        }
      *cp++ = read_char();
      ++i;
      }
    }
  *cp = 0;
  }

/************************************************************************
Reads up to maxlen characters from current position till the end of line
or delimiter simbol, including whitespaces. Returns number of characters
actually read, which may be less than maxlen if an error occurs or if the
end of the file is encountered before reaching maxlen.
*************************************************************************/
int text_file::readln( char* s, unsigned int maxlen, char delimiter, bool* hitDelimiter )
	{
	unsigned		i = 0;
	char	*cp = s;

  do {
		*cp = nextchar();
		if ( *cp==delimiter || (delimiter=='\n' && *cp==0x0d) ) break;
		cp++;
		i++;
	} while( !at_eof() && i < maxlen );
	if( hitDelimiter != NULL ) *hitDelimiter = *cp == delimiter;
	*cp = '\x0';
	return i;
	}

//--------------------------------------------------------------
void text_file::read(char *c)
  {
  eat_whitespace();

  *c = read_char();
  }

//--------------------------------------------------------------
void text_file::eat_whitespace()
{
  while(ISSPACE(peek_char()) && !at_eof())
    read_char();

	// C++ style comments.
  while ( peek_char() == '/' && !at_eof())
  {
    read_char();
    if ( peek_char() == '*' )
    {
      while ( !( read_char() == '*' && peek_char() == '/' ) && !at_eof() ) {}
      read_char();
    }
    else
    if ( peek_char() == '/' )
    {
	    while ( peek_char() == '/' && !at_eof())
	    {
		    // Skip to the end of the line.
		    while (peek_char()!='\n' && !at_eof())
			    read_char();
	    }
    }
    else
    {
      error( "Invalid comment." );
    }

    while(ISSPACE(peek_char()) && !at_eof())
      read_char();
  }
}

//--------------------------------------------------------------
void text_file::refill_buf()
{
  int theOldStash = the_stash.get_current_stash();
  the_stash.set_current_stash(my_stash);
  if (use_stash)
    bufamt=the_stash.read(buf,BUFFER_SIZE);
  else
    bufamt=io.read(buf,BUFFER_SIZE);
  bufpos=0;
  the_stash.set_current_stash(theOldStash);
}

//--------------------------------------------------------------
void text_file::write(int i)
{
  write(itos(i));
}

//--------------------------------------------------------------
void text_file::write(float f)
  {
  write(ftos(f));
  }

//--------------------------------------------------------------
void text_file::write(char c)
  {
  io.write((void*)&c,1);
  }

//--------------------------------------------------------------
void text_file::write(const stringx & s)
  {
  io.write((void*)s.c_str(),s.length());
  }

//--------------------------------------------------------------
char text_file::peek_char()
  {
  char ch;

  if( pushbackdata >= 0 )
    {
    ch = pushbackdata;
    }
  else
    {
    ch = buf[bufpos];
    }
  return ch;
  }

//--------------------------------------------------------------
char text_file::read_char()
  {
  char ch;

  if( pushbackdata >= 0 )
    {
    ch = pushbackdata;
    pushbackdata = -1;
    }
  else
    {
    ch = buf[bufpos];
    if (++bufpos>=bufamt)
      refill_buf();
    }

//   char * txt = &(buf[bufpos]); // is this used for anything?
  return( ch );
  }

//--------------------------------------------------------------
// Integrating PFE parsing functionality
//--------------------------------------------------------------
int text_file::nextchar()
  {
  unsigned char c;

  if( pushbackdata >= 0 )
    {
    c = pushbackdata;
    pushbackdata = -1;
    }
  else
    {
    c = read_char();

    if( at_eof() )
      return( -1 );
    }

  return( c );
  }

//--------------------------------------------------------------
int text_file::nextnonwhite()
  {
  int c;
  do
    {
    c = nextchar();
    } while( ISSPACE(c) && (c!=-1) );

  return( c );
  }

//--------------------------------------------------------------
int text_file::skipuntilthischar(char chin)
  {
  int c;
  do
    {
    c = nextchar();
    } while( (c!=chin) && (c!=-1) );

  return( c );
  }

//--------------------------------------------------------------
int text_file::readuntilthischar(char chin,char *buf,int buflen)
  {
  int c;
  int i = 0;
  do
    {
    c = nextchar();
    if( (c!=chin) && (c!=-1) )
      {
      ++i;
      *buf++ = c;
      }
    } while( (c!=chin) && (c!=-1) && (i<buflen-1) );

  *buf++ = 0;
  return( c );
  }

//--------------------------------------------------------------
int text_file::readuntilwhite(char *buf,int buflen)
  {
  int c;
  int i = 0;
  do
    {
    c = nextchar();
    if( (!ISSPACE(c)) && (c!=-1) )
      {
      ++i;
      *buf++ = c;
      }
    } while( (!ISSPACE(c)) && (c!=-1) && (i<buflen-1) );

  *buf++ = 0;
  return( c );
  }

//--------------------------------------------------------------
int text_file::readuntilnotdigit(char *buf,int buflen)
  {
  int c;
  int i = 0;
  do
    {
    c = nextchar();
    if( (ISDIGIT(c)) && (c!=-1) )
      {
      ++i;
      *buf++ = c;
      }
    } while( (ISDIGIT(c)||(c=='.')) && (c!=-1) && (i<buflen-1) );

  *buf++ = 0;
  return( c );
  }

//--------------------------------------------------------------
int text_file::skipuntildigit()
  {
  int c;
  do
    {
    c = nextchar();
    } while( (!ISDIGIT(c)) && (c!=-1) );

  keypushback(c);
  return( c );
  }

//--------------------------------------------------------------
void text_file::keypushback(int c)
  {
  pushbackdata = c;
  }

#if defined(TARGET_MKS)
void my_memcpy(void* dst, void* src, int bytes);
void my_memcpy(void* dst, void* src, int bytes)
	{
	if ( bytes > 4096 )
		error( "memcpy'd too much." );
	#undef memcpy
	memcpy(src,dst,bytes);
	}
#endif

