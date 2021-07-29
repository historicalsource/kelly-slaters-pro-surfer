// filespec.h
#ifndef _FILESPEC_H
#define _FILESPEC_H


#include "stringx.h"


class filespec
  {
  public:
    stringx path;
    stringx name;
    stringx ext;
    filespec() {}
    filespec(const filespec& src) : path(src.path), name(src.name), ext(src.ext) {}
    filespec(const stringx& p,const stringx& n,const stringx& e) : path(p),name(n),ext(e) {}
    explicit filespec(const stringx& s);
    stringx fullname() { return path+name+ext; }
    const filespec& operator=(const filespec& rhs) { path=rhs.path; name=rhs.name; ext=rhs.ext; return *this; }
  protected:
    void _extract(const stringx & src);
  };


#endif  //  _FILESPEC_H
