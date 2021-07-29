/*-------------------------------------------------------------------------------------------------------
  app_file implementation
-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "file.h"
#include "oserrmsg.h"
#include "debug.h"

#include "osdevopts.h"

bool app_file::noquotes=false;

/*-------------------------------------------------------------------------------------------------------
  app_file implementation
-------------------------------------------------------------------------------------------------------*/
app_file::app_file() : use_stash(false), type(CFT_NONE) {}

#define FBIN_TAG_VALUE 0x4642494E // this actually should be reversed (right now it appears in the file backwards)

//void app_file::open( const stringx& name, int flags )
void app_file::open( const char * name, int flags )
{
  stringx temp_filename;
  //int tag;
  int mode = flags & MODE_MASK;
  if ( mode==os_file::FILE_READ && !(flags&FILE_TEXT) )
  {
    if ( stash::is_stash_open() )
    {
      filespec spec(name);
      temp_filename = spec.name + spec.ext;
			#ifdef EVAN
				char damnopaquestringclass[256];
				strcpy(damnopaquestringclass,temp_filename.c_str());
			#endif
      if ( temp_filename.length() > PSTRING_MAX_LENGTH )
			{
				 //printf("too long\n");
			}
      if ( temp_filename.length() <= PSTRING_MAX_LENGTH &&
           the_stash.file_exists(temp_filename.c_str()) )
      {
        // get it from the stash
        use_stash = true;
        the_stash.open(temp_filename.c_str());
        //the_stash.read(&tag, sizeof(tag));
      }
    }

    // check first four bytes for tag indicating a binary file
    type = CFT_BINARY;
    if (use_stash == false)
    {
      if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
        debug_print("Going to disk for file %s\n", name); //.c_str());
      assert(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY));
			#ifdef EVAN
				char damnopaquestringclass[256];
				strcpy(damnopaquestringclass,name);
			#endif
      binary.open( name, mode );
    }
  }
  else if ( flags & FILE_BINARY )
  {
    binary.open( name, mode );
    if ( mode == os_file::FILE_WRITE )
    {
      //tag = FBIN_TAG_VALUE;
      //binary.write( &tag, sizeof(tag) );
    }
  }
  else
  {
    type = CFT_TEXT;
    text.open( name, mode );
  }
}

void app_file::close()
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

bool app_file::operator!() const
{
  switch ( type )
  {
    case CFT_BINARY:
      if (use_stash)
        return !the_stash.is_open();
      else
        return !binary;
    case CFT_TEXT:
      return !text;
    default:
      return true;
  }
  return true;
}

void app_file::set_fp( unsigned int pos, os_file::filepos_t base )
{
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
}

bool app_file::at_eof()
{
  if (type==CFT_BINARY)
  {
    if (use_stash)
      return the_stash.at_eof();
    else
      return binary.at_eof();
  }
  else
  if (type==CFT_TEXT)
    return text.at_eof();
  else
    assert(false);
  return false;
}

int app_file::get_size(void)
{
  if (type==CFT_BINARY)
  {
    if (use_stash)
      return the_stash.get_size();
    else
      return binary.get_size();
  }
  else if (type==CFT_TEXT)
    return 0; //text.get_size();
  else
    assert(false);
  return 0;
}

bool app_file::read( void *buf, int bytes )
{
  if (get_type()==app_file::CFT_TEXT)
  {
    return false; //io.get_text()->read( buf, app_flavor::CHUNK_FLAVOR_SIZE );
  }
  else
  {
    if (use_stash)
		{
      the_stash.read(buf,bytes);
		}
    else
      binary.read(buf,bytes);
  }
	return true;
}

stringx app_file::get_name()
{
  if (type==CFT_BINARY)
  {
    if (use_stash)
      return stringx(the_stash.get_name().c_str());
    else
      return binary.get_name();
  }
  else
  if (type==CFT_TEXT)
    return text.get_name();
  else
    return "File not open";
}

stringx app_file::get_dir()
{
  stringx name=get_name();
  int end = name.rfind('\\');
  if (end < 0)
    return stringx("");
  else
    return name.substr(0,end)+"\\";
}

stringx app_file::get_filename()
{
  stringx name=get_name();
  return filespec(name).name;
}


void* KSMemAlloc( u_int Size, u_int Align );
void KSMemFree( void* Ptr );



//bool app_file::get_memory_image(const pstring& _name, unsigned char *&buf,
bool app_file::get_memory_image(const char * _name, unsigned char *&buf,
                                 unsigned int &buf_size, stash_index_entry *&hdr, int alignment)
{
  if ( stash::is_stash_open() )
  {
	  stringx temp_filename;
    filespec spec(_name); //.c_str());
    temp_filename = spec.name + spec.ext;
#ifdef EVAN
char damnopaquestringclass[256];
strcpy(damnopaquestringclass,temp_filename.c_str());
#endif
    if ( temp_filename.length() > PSTRING_MAX_LENGTH )
		{
				//printf("too long\n");
		}
    if ( temp_filename.length() <= PSTRING_MAX_LENGTH &&
          the_stash.file_exists(temp_filename.c_str()) )
    {
      // get it from the stash
      use_stash = true;
			int bsize;
      bool rv=the_stash.get_memory_image(temp_filename,buf,bsize,hdr);
			buf_size=bsize;
			return rv;
    }
		return false;
  }
	else
	{
		stringx thisislameness=_name; //.c_str();
		open(_name); //thisislameness);
		buf_size=get_size();

		if ( buf_size )
		{
			//
			// LEAK LEAK LEAK LEAK LEAK LEAK LEAK LEAK
			//

			buf=(unsigned char *) KSMemAlloc( buf_size, alignment );
			if ( buf==NULL )
				return false;
			return read( buf, buf_size );
		}
		close();
		return true;
	}

}


