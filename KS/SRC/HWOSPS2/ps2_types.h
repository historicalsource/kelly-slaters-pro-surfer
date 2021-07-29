//
//  portable types
//

#ifdef WIN32
	typedef unsigned long	u32;
	typedef signed long		s32;

	#define	PACKING(a)

  #error This file should not be compiled under Win32.
#endif

#ifdef __GNUC__
	typedef unsigned int	u32;
	typedef signed int		s32;

	#define	PACKING(x)	__attribute__((aligned (x)))

	// defines for dot and cross product
	#define sceVu0DotProduct(a,b)		sceVu0InnerProduct(a,b)
	#define sceVu0CrossProduct(a,b,c)	sceVu0OuterProduct(a,b,c)
#endif
