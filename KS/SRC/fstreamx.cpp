////////////////////////////////////////////////////////////////////////////////
/*
  fstreamx.cpp

  fstream wrapper
*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#if !defined(NO_FSTREAM)

#include "fstreamx.h"
#include "oserrmsg.h"
#include <ctype.h>

////////////////////////////////////////////////////////////////////////////////
// static

stringx *fstreamx::proot_dir = 0;
stringx *fstreamx::pre_root_dir = 0;

////////////////////////////////////////////////////////////////////////////////
// fstreamx
fstreamx::fstreamx( const stringx& _fname, int mode ) :
    fstream( (proot_dir)?((*proot_dir + _fname).c_str()):(_fname.c_str()), mode ) , fname(_fname)
  {
  check_static_root();
  if (!is_open())
    {
    stringx composite = stringx("Unable to open ") + *proot_dir + _fname;
    if (mode & ios::in)
      composite += stringx(" for reading.");
    else
      composite += stringx(" for writing.");
    error( composite.c_str() );
    }
  }

void fstreamx::open( const stringx& _fname, int mode )
  {
  check_static_root();
  fstream::open( (*proot_dir + _fname).c_str(), mode );
  if (!is_open())
    {
    stringx composite = stringx("Unable to open ") + *proot_dir + _fname;
    if (mode & ios::in)
      composite += stringx(" for reading.");
    else
      composite += stringx(" for writing.");
    error( composite.c_str() );
    }
  fname = _fname;
  }

// Support functions

fstreamx& operator<<(fstreamx& fs,const stringx& s)
  {
  fs.write(s.c_str(),s.size());
  return fs;
  }

fstreamx& operator>>(fstreamx& fs,stringx& s)
  {
  char buf[1024];
  fs >> (char*)buf;
  s = buf;
  return fs;
  }

/*
fstreamx& operator<<(fstreamx& fs,const string& s)
  {
  fs.write(s.c_str(),s.size());
  return fs;
  }

fstreamx& operator>>(fstreamx& fs,string& s)
  {
  char buf[1024];
  fs >> (char*)buf;
  s = buf;
  return fs;
  }
*/

bool fstreamx::file_exists( const stringx& fname )
  {
  // Duplication of this functionality required
  // because this is a static member function.
  if(!proot_dir)
    {
    proot_dir = NEW stringx;
    }
  fstream fs;
  if (fname[1]!=':')
    fs.open((*proot_dir+fname).c_str(),ios::in);
  else
    fs.open(fname.c_str(),ios::in);
  if(fs.is_open())
    return true;
  return false;
  }

stringx fstreamx::get_dir() const
  {
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char my_fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  _splitpath( fname.c_str(), drive, dir, my_fname, ext );
  return stringx(dir);
  }

void fstreamx::set_root_dir( const stringx& _root_dir )
  {
  // Duplication of this functionality required
  // because this is a static member function.
  if(!proot_dir)
    {
    proot_dir = NEW stringx;
    }
  if(proot_dir) *proot_dir = _root_dir;
  }

void fstreamx::check_static_root( void )
  {
    if(!proot_dir)
      {
      proot_dir = NEW stringx;
      }
  }

void fstreamx::set_pre_root_dir( const stringx& _pre_root_dir )
  {
  // Duplication of this functionality required
  // because this is a static member function.
  if(!pre_root_dir)
    {
    pre_root_dir = NEW stringx;
    }
  if(pre_root_dir) *pre_root_dir = _pre_root_dir;
  }

#endif
