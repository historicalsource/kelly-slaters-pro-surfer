
#ifndef _LIBSN_H
#define _LIBSN_H

/*  Version 1.2 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	UINT32
#define UINT32 unsigned int
#define NEEDUNDEFUINT32
#endif

/**************************/
/* SN debugger extensions */

/* Routine to install additional debugger supprt functions */

extern	int snDebugInit (void);

/*************************************/
/* SN Run time DLL handling routines */

/* call at the start of your code to initialise the dll system */

extern	int snInitDllSystem (void** index_pointer);

/* Call this routine after a dll has been loaded into memory to relocate and initialise it */

extern	int snDllLoaded (void* buffer, void** index_pointer);

/* Call this routine to close down a dll before freeing the memory it's in */

extern	int snDllUnload (void* buffer);

/* Call this routine to re-relocate a dll to a new address */
/* If your program has taken any pointers to the dll code or data your program will fail */
/* A dll cannot move itself or call a routine that moves it */

extern int snDllMove (void* destination, void* source, void** index_pointer);

#define SN_DLL_SUCCESS          0	/* operation succeeded */
#define SN_DLL_NOT_A_DLL        1	/* the buffer doesn't seem to contain a dll */
#define SN_DLL_BAD_VERSION      2	/* the dll version is not support by this code */
#define SN_DLL_INVALID          3	/* some data in the dll header was invalid */
#define SN_DLL_BAD_ALIGN        4	/* the dll is not aligned to the required boundary */
#define SN_DLL_NOT_LOADED       5	/* The dll has not been loaded so can't be unloaded */
#define SN_DLL_TOO_MANY_MODULES 6	/* Too many modules loaded */

/*============ PROFILER STUFF ====================*/

/* interval values to pass to snProfInit() and snProfSetInterval() */
#define	_1KHZ	300000
#define	_2KHZ	150000
#define	_4KHZ	 75000
#define	_10KHZ	 30000
#define	_20KHZ	 15000

struct _PROFHDR
{
	UINT32	interval;	/* interval count (cpu clocks) */
	UINT32	startpc;	/* profile lower limit for PC */
	UINT32	endpc;		/* profile upper limit for PC */
	UINT32	mask;		/* mask value for above flags */

	UINT32	flags;		/* flags */
	UINT32	buffptr;	/* ptr to two profile-sample-data buffers */
	UINT32	bufflen;	/* length of one-half sample buffer */
	UINT32	ptr;		/* actually an index but we just want to know if it is 0 or -1 */
/*	UINT32	buffnum;	not present on IOP side */
};
typedef struct _PROFHDR PROFHDR;

struct _PROFSAMPLE
{
	UINT32	pc;
/*	UINT32	flags; */
};
typedef struct _PROFSAMPLE PROFSAMPLE;

/* Export from profile.c */
int snProfInit(unsigned int rate, void* buffer, int buffersize );

/* and it's return values */
#define SN_PRF_SUCCESS          0	/* operation succeeded */
#define SN_PRF_TEQFAIL			1	/* IRQ hook failed to install */
#define SN_PRF_NOIOP			2	/* IOP sync failed */
#define SN_PRF_BUFFALIGN		3	/* The profile data buffer is not quadword aligned */
#define SN_PRF_BUFFSIZE			4	/* The buffer is too small or too large */
#define SN_PRF_RPCFAIL       	5   /* Cannot RPC the IOP module */

/* things in prof.s */
extern	int 	_snProfInit(unsigned int rate, void* buffer, int buffersize );
extern	void	snProfSetInterval(UINT32 interval);
extern	void	snProfSetRange(int profmask, void* startpc, void* endpc);
extern	int		snProfSetFlagValue(int value);
extern	int		snProfSetFlags(int flags);
extern	int		snProfClrFlags(int flags);

#ifdef NEEDUNDEFUINT32				/* only undefine it if *we* defined it */
#undef UINT32
#endif

#ifdef __cplusplus
}
#endif

#endif
