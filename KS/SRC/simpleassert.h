#ifndef __SimpleAssert_H__
#define __SimpleAssert_H__

//============================================================================
// An extended assert macro.  This code is based on the ideas and code
// from Game Programming Gems.
//=============================================================

#ifdef _DEBUG

  bool _sAssertDlg_(const char*, const char*, 
                    const char*, int, bool*);

  // sAssert
  #define sAssert(exp) { \
    static bool _ignoreAlways_ = false; \
    if (!_ignoreAlways_ && !(exp)) { \
      if (_sAssertDlg_(#exp, NULL, __FILE__, __LINE__, &_ignoreAlways_)) \
        { _asm { int 3 } } \
    } \
  }

  // sAssertM
  #define sAssertM(exp, msg) { \
    static bool _ignoreAlways_ = false; \
    if (!_ignoreAlways_ && !(exp)) { \
      if (_sAssertDlg_(#exp, msg, __FILE__, __LINE__, &_ignoreAlways_)) \
        { _asm { int 3 } } \
    } \
  }

#else

  #define sAssert(exp)
  #define sAssertM(exp, msg)
#endif


#endif // __SimpleAssert_H__
