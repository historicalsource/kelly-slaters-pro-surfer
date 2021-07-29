//
//  portable types
//
#ifndef _XB_TYPES_H_
#define _XB_TYPES_H_

#ifndef TARGET_XBOX
#error "This file is for xbox only, sorry"
#endif /* TARGET_XBOX JIV DEBUG */


typedef unsigned long	u32;
typedef signed long		s32;

#define	PACKING(a)

// compile check for types here
typedef int check_u32[!((sizeof u32) - 4)];
typedef int check_s32[!((sizeof s32) - 4)];

#endif /* _XB_TYPES_H_ */