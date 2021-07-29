// filespec.cpp
#include "global.h"

#include "filespec.h"


filespec::filespec(const stringx& s)
  {
  _extract(s);
  }

void filespec::_extract(const stringx & src)
  {
  int cp1=src.rfind('\\');
  if (cp1!=stringx::npos)
    {
    cp1++;
    path = src.substr(0,cp1);
    }
  else
    {
    path = empty_string;
    cp1=0;
    }

  int cp2 = src.rfind('.');
  if (cp2!=stringx::npos)
    {
    name = src.substr(cp1,cp2-cp1);
    ext = src.substr(cp2,src.length()-cp2);
    }
  else
    {
    name = src.substr(cp1,src.length()-cp1+1);
    ext = empty_string;
    }
  }
