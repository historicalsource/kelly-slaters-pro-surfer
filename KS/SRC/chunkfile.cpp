/*-------------------------------------------------------------------------------------------------------
  chunk_file implementation
-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "chunkfile.h"
#include "oserrmsg.h"
#include "debug.h"

#include "osdevopts.h"

/*-------------------------------------------------------------------------------------------------------
CF_WRITE_BINARY

If defined, opening a text file for reading will also cause a binary file to be opened for writing.
Subsequent read operations from the text file cause equivalent write operations to the binary file.
This allows the load process to act as a 'converter' from text to binary form.

This is just here so I can test the binary loading code.

Any time the text data or the loading code changes, you must execute "del \steel\data\*.b?? /s /q /f"
and build all the binaries again.
-------------------------------------------------------------------------------------------------------*/
#if defined( TARGET_PC ) && 0
#define CF_WRITE_BINARY
#endif

const chunk_flavor CHUNK_EOF    ("eof");
const chunk_flavor CHUNK_END    ("chunkend"); // only flexible chunks need the chunkend tag.
const chunk_flavor CHUNK_SPF    ("spf");      // seconds-per-frame for animatable files (.tam & .ban)
const chunk_flavor CHUNK_SCALE  ("scale");    // scale factor for meshes or whatever else.

const stringx chunkend_label( "chunkend" );

bool chunk_file::noquotes=false;

/*-------------------------------------------------------------------------------------------------------
  chunk_file implementation
-------------------------------------------------------------------------------------------------------*/
chunk_file::chunk_file() : use_stash(false), type(CFT_NONE) {}

#define FBIN_TAG_VALUE 0x4642494E // this actually should be reversed (right now it appears in the file backwards)

void chunk_file::open( const stringx& name, int flags )
{

  int theOldStash = the_stash.get_current_stash();
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,name.c_str());
#endif
  stringx temp_filename;
  int tag;
  int mode = flags & MODE_MASK;
  if ( mode==os_file::FILE_READ && !(flags&FILE_TEXT) )
  {
    if ( stash::is_stash_open() )
    {
      filespec spec(name);
      temp_filename = spec.name + spec.ext;
#ifdef EVAN
	strcpy(damnopaquestringclass,temp_filename.c_str());
#endif
      if ( temp_filename.length() > PSTRING_MAX_LENGTH )
			{
				 #ifdef EVAN
				 //warning("too long\n");
				 //asm("break");
				 #endif
			}
      if ( temp_filename.length() <= PSTRING_MAX_LENGTH &&
           the_stash.file_exists(temp_filename.c_str()) )
      {
        // get it from the stash
        use_stash = true;
        my_stash = the_stash.get_current_stash();
        the_stash.open(temp_filename.c_str());
				//int farggina=the_stash.get_fp();
        the_stash.read(&tag, sizeof(tag));
    		//if ( tag != FBIN_TAG_VALUE )
				//{
				//	the_stash.set_fp(farggina,stash::FP_BEGIN);
				//}
      }
			else
			{
				#ifdef EVAN
				warning("File not found : %s\n",temp_filename.c_str());
				//asm("break");
				#endif
			}
    }

    // check first four bytes for tag indicating a binary file
    type = CFT_BINARY;
    if (use_stash == false)
    {
      if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
        debug_print("Going to disk for file %s\n", name.c_str());
      if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
			{
				#ifdef EVAN
		    if ( stash::is_stash_open() )
				{
					warning("No stash is open\n");
				//	asm("break");
				}
				//asm("break");
				#endif
			}
      binary.open( name, mode );
      if (binary.is_open() ==false)
      { 
        return;
      }
			//int farggina=binary.get_fp();
      binary.read( &tag, sizeof(tag) );
    	//if ( tag != FBIN_TAG_VALUE )
			//{
			//	binary.set_fp(farggina,os_file::FP_BEGIN);
			//}
    }
		#if 1
    if ( tag != FBIN_TAG_VALUE )
    {

      if ( flags & FILE_BINARY )
        error( name + ": binary chunk_file tag not found" );
      if (use_stash == true)
      {
        the_stash.close();
        use_stash = false;
      }
      else
        binary.close();
      type = CFT_TEXT;
      text.open( name, mode );
      #if defined( CF_WRITE_BINARY )
      // if write binary is enabled, open the output file.
      stringx bname = name;
      bname.to_lower();
      filespec binfile( bname );
      stringx::size_type p = binfile.path.find( "data" );
      binfile.path = os_developer_options::inst()->get_string(os_developer_options::STRING_WRITE_BINARY_DIR) +
         ((p != stringx::npos) ? ("\\" + binfile.path.substr( p ))
                               : ("\\data\\" + binfile.path));
      bname = binfile.fullname();
      binary.open( bname, os_file::FILE_WRITE );
      if ( !binary.is_open() )
        warning( "Could not open binary file " + bname + " for output" );
      else
      {
        tag = FBIN_TAG_VALUE;
        binary.write( &tag, sizeof(tag) );
      }
      #endif
    }
		#endif
  }
  else if ( flags & FILE_BINARY )
  {
    binary.open( name, mode );
    if ( mode == os_file::FILE_WRITE )
    {
      tag = FBIN_TAG_VALUE;
      binary.write( &tag, sizeof(tag) );
    }
  }
  else
  {
    type = CFT_TEXT;
    text.open( name, mode );
  }

  the_stash.set_current_stash(theOldStash);
}

void chunk_file::close()
{
  if (type==CFT_BINARY)
  {
    if (use_stash == true)
    {
      use_stash = false;
      the_stash.close();
    }
    else
      binary.close();
  }
  else
  if (type==CFT_TEXT)
  {
    text.close();
#if defined( CF_WRITE_BINARY )
    if ( binary.is_open() )
      binary.close();
#endif
  }
  type=CFT_NONE;
}

bool chunk_file::operator!() const
{
  int theOldStash = the_stash.get_current_stash();
  the_stash.set_current_stash(my_stash);
  bool retval;
  switch ( type )
  {
    case CFT_BINARY:
      if (use_stash)
        retval = !the_stash.is_open();
      else
        retval = !binary;
    case CFT_TEXT:
      retval = !text;
    default:
      retval = true;
  }
  the_stash.set_current_stash(theOldStash);
  return retval;
}

void chunk_file::set_fp( unsigned int pos, os_file::filepos_t base )
{
  int theOldStash = the_stash.get_current_stash();
  the_stash.set_current_stash(my_stash);

  switch ( type )
  {
    case CFT_BINARY:
      if (use_stash)
        the_stash.set_fp( pos, (stash::filepos_t)base );
      else
        binary.set_fp( pos, base );
      break;
    case CFT_TEXT:
      text.set_fp( pos, base );
      break;
    default:
      break;
  }
  the_stash.set_current_stash(theOldStash);

}

bool chunk_file::at_eof()
{
  int theOldStash = the_stash.get_current_stash();
  bool retval=false;
  the_stash.set_current_stash(my_stash);

  if (type==CFT_BINARY)
  {
    if (use_stash)
      retval= the_stash.at_eof();
    else
      retval= binary.at_eof();
  }
  else
  if (type==CFT_TEXT)
    retval= text.at_eof();
  else
    assert(false);
  the_stash.set_current_stash(theOldStash);
  return retval;
}

int chunk_file::get_size(void)
{
  int theOldStash = the_stash.get_current_stash();
  bool retval=0;
  the_stash.set_current_stash(my_stash);

  if (type==CFT_BINARY)
  {
    if (use_stash)
      retval= the_stash.get_size();
    else
      retval= binary.get_size();
  }
  else if (type==CFT_TEXT)
    retval= 0; //text.get_size();
  else
    assert(false);

  the_stash.set_current_stash(theOldStash);
  return retval;
}

bool chunk_file::read( void *buf, int bytes )
{
  int theOldStash = the_stash.get_current_stash();
  bool retval=true;;
  the_stash.set_current_stash(my_stash);

  if (get_type()==chunk_file::CFT_TEXT)
  {
    retval= false; //io.get_text()->read( buf, chunk_flavor::CHUNK_FLAVOR_SIZE );
  }
  else
  {
    if (use_stash)
      the_stash.read(buf,bytes);
    else
      binary.read(buf,bytes);
  }
  the_stash.set_current_stash(theOldStash);

	return retval;
}

stringx chunk_file::get_name()
{
  int theOldStash = the_stash.get_current_stash();
  stringx retval="";
  the_stash.set_current_stash(my_stash);

  if (type==CFT_BINARY)
  {
    if (use_stash)
      retval= stringx(the_stash.get_name().c_str());
    else
      retval= binary.get_name();
  }
  else
  if (type==CFT_TEXT)
    retval= text.get_name();
  else
    retval= "File not open";
  the_stash.set_current_stash(theOldStash);
  return retval;
}

stringx chunk_file::get_dir()
{
  stringx name=get_name();
  int end = name.rfind('\\');
  if (end < 0)
    return stringx("");
  else
    return name.substr(0,end)+"\\";
}

stringx chunk_file::get_filename()
{
  stringx name=get_name();
	filespec spec=filespec(name);
	stringx fname=spec.name;
  return fname; //filespec(name).name;
}

#ifndef NO_SERIAL_IN
/*-------------------------------------------------------------------------------------------------------
  basic serial_in's
-------------------------------------------------------------------------------------------------------*/
// Macro to make implementing CF_WRITE_BINARY easier.
#if defined( CF_WRITE_BINARY )

#define CF_BINARY_OUT( var )                  \
  if ( io.get_type()==chunk_file::CFT_TEXT )  \
  {                                           \
    io.set_type( chunk_file::CFT_BINARY );    \
    if ( io.get_binary()->is_open() )         \
      serial_out( io, var );                  \
    io.set_type( chunk_file::CFT_TEXT );      \
  }
#else

#define CF_BINARY_OUT(var)

#endif

#define CF_SERIAL_IN(io, var)                 \
  if (io.get_type()==chunk_file::CFT_TEXT)    \
    io.get_text()->read(var);                 \
  else                                        \
{  if (io.use_stash)                          \
      io.get_stash()->read(var, sizeof(*var)); \
    else                                       \
      io.get_binary()->read(var,sizeof(*var)); \
}

void serial_in(chunk_file& io, chunk_flavor* d)
{
  if (io.get_type()==chunk_file::CFT_TEXT)
  {
    char cfname[chunk_flavor::CHUNK_FLAVOR_SIZE + 1];
    io.get_text()->read( cfname, chunk_flavor::CHUNK_FLAVOR_SIZE );
    *d = chunk_flavor( cfname );
  }
  else
  {
    if (io.use_stash)
      io.get_stash()->read(d,sizeof(chunk_flavor));
    else
      io.get_binary()->read(d,sizeof(chunk_flavor));
  }
  CF_BINARY_OUT(*d);


}

void serial_in(chunk_file& io, int* d)
{
  CF_SERIAL_IN(io,d);
  CF_BINARY_OUT(*d);
}

void serial_in(chunk_file& io, unsigned short* d)
{
  int i;
  serial_in(io,&i);
  *d=i;
}

// <<<< can't handle extended range of unsigned ints yet.  need to make unsigned int part of text_file.
void serial_in(chunk_file& io, unsigned int* d)
{
  int i;
  serial_in(io,&i);
//  assert(i>=0);
  *d=i;
}

// <<<< would be cool to support true/false for bool.
void serial_in(chunk_file& io, bool* d)
{
  int i;
  serial_in(io,&i);
  *d=(bool)i;
}

void serial_in(chunk_file& io, float* d)
{
  CF_SERIAL_IN(io,d);
  CF_BINARY_OUT(*d);
}

#if defined(TARGET_XBOX)
void serial_in(chunk_file& io, double* d)
{
  // JIV FIXME probably won't work
  CF_SERIAL_IN(io,d);
  CF_BINARY_OUT(*d);
}
#endif /* TARGET_XBOX JIV DEBUG */

void serial_in(chunk_file& io, stringx* d)
{
  if (io.get_type()==chunk_file::CFT_TEXT)
    io.get_text()->read(d);
  else
  {
    int len;
    char work[256];
    if (io.use_stash)
      io.get_stash()->read( &len, sizeof(int) );
    else
      io.get_binary()->read( &len, sizeof(int) );
    assert( len < (int)sizeof(work)-1 );
    if ( len > 0 )
    {
      if (io.use_stash)
        io.get_stash()->read( work, len );
      else
        io.get_binary()->read( work, len );
      work[len] = '\0';
      *d = work;
    }
    else
      *d = stringx();
  }
  CF_BINARY_OUT(*d);
}

#endif

#if !defined(NO_SERIAL_OUT)
/*-------------------------------------------------------------------------------------------------------
  basic serial_out's
-------------------------------------------------------------------------------------------------------*/
void serial_out(chunk_file& io, const chunk_flavor& d)
{


  if (io.get_type()==chunk_file::CFT_TEXT)
    io.get_text()->write( d.to_stringx() + sendl );
  else
    io.get_binary()->write((void*)&d,sizeof(chunk_flavor));

}

void serial_out(chunk_file& io,const int& d)
{
  if (io.get_type()==chunk_file::CFT_TEXT)
    io.get_text()->write(itos(d)+sendl);
  else
    io.get_binary()->write((void*)&d,sizeof(int));
  

}

void serial_out(chunk_file& io,const short& d)
{
  int i=d;
  serial_out(io,i);
}

void serial_out(chunk_file& io,const unsigned short& d)
{
  int i=d;
  serial_out(io,i);
}

void serial_out(chunk_file& io,const unsigned int& d)
{
  int i = d;
  serial_out( io, i );
}

void serial_out(chunk_file& io,const float& d)
{

  if (io.get_type()==chunk_file::CFT_TEXT)
    io.get_text()->write(ftos(d)+sendl);
  else
    io.get_binary()->write((void*)&d,sizeof(float));

}

#if defined(TARGET_XBOX)
void serial_out(chunk_file& io,const double& src)
{
  // JIV FIXME okay this is wrong
  const float d = static_cast<float>(src);

  if (io.get_type()==chunk_file::CFT_TEXT)
    io.get_text()->write(ftos(d)+sendl);
  else
    io.get_binary()->write((void*)&d,sizeof(float));

}
#endif /* TARGET_XBOX JIV DEBUG */


void serial_out(chunk_file& io,const stringx& d)
{

  if (io.get_type()==chunk_file::CFT_TEXT)
  {
    // PTA HACK TO AVOID HEADERS  FORGIVE ME PLEASE.  9/9/99
    // It's ok man... I made it a static member of chunk_file.  A tiny bit
    // better I guess.  You're forgiven.  ;)  --Sean
    if (chunk_file::noquotes)
      io.get_text()->write(d+sendl);
    else
      io.get_text()->write("\""+d+"\""+sendl);
  }
  else
  {
    int len=d.length();
    io.get_binary()->write((void*)&len,sizeof(int));
    if (len>0)
      io.get_binary()->write((void*)d.c_str(),len);
  }

}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  chunk_flavor members
  */
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* chunk_flavor::c_str() const
{
  return flavor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
stringx chunk_flavor::to_stringx() const
{
  return stringx( flavor );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool chunk_flavor::operator==( const chunk_flavor& cf ) const
{
  return (strcmp( flavor, cf.flavor )==0);
}

#ifndef TARGET_PS2

bool chunk_flavor::operator!=( const chunk_flavor& cf ) const
{
  return (strcmp( flavor, cf.flavor )!=0);
}

#endif
