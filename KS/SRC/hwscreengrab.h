// hw_screengrab.h

#ifndef HW_SCREENGRAB_H
#define HW_SCREENGRAB_H

#if !defined(BUILD_BOOTABLE) && defined(TARGET_MKS) || 0
//#define ENABLE_SCREEN_GRAB  // it screws up debug and fastdebug versions as well,
                              // so, please enable this macro on your local machines only.
                              // Thank you,
                              // Slava.
#endif

void SaveFrontBufferAsBMP( const char * fname );

#endif