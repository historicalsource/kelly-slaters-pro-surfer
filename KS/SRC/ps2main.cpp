//-----------------------------------------------------------------------------
// File: ps2main.cpp
//
//   This is it, ground zero for PS2 Kelly Slater.
//   Entry-point for the program and so much more.
//
// Copyright (c) 2000 Treyarch LLC. All rights reserved.
//-----------------------------------------------------------------------------

#include "global.h"

#ifdef TARGET_PS2

#include <stdio.h>
#include <sntty.h>
#include <sifcmd.h>
#include "pstring.h"
#include "game.h"
#include "random.h"
#include "osdevopts.h"
#include "osalloc.h"
#include "ksnsl.h"
#include "app.h"
#include "wds.h"
#include "profiler.h"
#include "ksnsl.h"
#include "ksnvl.h"
#include "semaphores.h"
#ifdef TARGET_PS2
#include "ps2\nsl_ps2.h"
#endif
extern "C" {
	  extern unsigned long end;
}



int g_alloc_size_megs = 96;

// turn this off to let the debugger handle hardware exceptions
#if !defined(USER_JDF)
//#define ENABLE_EXCEPTION_HANDLER
#endif

bool g_app_active = true;
const char *application_name = NULL;

void application_startup();
void application_shutdown();
void application_run();
void system_idle();

// for testing
#include "osfile.h"
#include "ostimer.h"
#include "hwosps2/ps2_input.h"
#include "joystick.h"
#include "game.h"
#include "po.h"

extern game *g_game_ptr;

void init_fast_acos_table();
void init_fast_sin_table();

#define IOPRP_IMGCD "cdrom0:\\"IOP_IMAGE_FILE";1"
#define IOPRP_IMGHOST "host0:"IOP_IMAGE_FILE

const char *g_thps4_path = "cdrom0:\\THPS4\\SLUS_205.04;1";
char *g_ksps_path;

void register_exception_handlers();

void KSHeapError( const char* Text )
{
	onscreenerror( Text );
	error( Text );
  // Removed since we now let users try to button through these
  for( ;; );
}

void KSCriticalError( const char* Text )
{
	onscreenerror( Text );
	error( Text );
  // Removed since we now let users try to button through these
  //for( ;; );
}

void KSDebugPrint( const char* Text )	//--- can't use nglPrintf from here, because it causes an infinite loop.
{
  if ( nglUsingProView )
    snputs( Text );
  else
    scePrintf( Text );
}

void* KSMemAllocate( u_int Size, u_int Align, const char *file, int line )
{
  return arch_memalign( Align, Size,file,line );
}

void* KSMemAlloc( u_int Size, u_int Align )
{
  return KSMemAllocate(Size,Align,"KSMemAlloc",0 );
}

void* KSMemAllocNGL( u_int Size, u_int Align )
{
	Align = Align > 16 ? Align : 16;
	if ( Size==sizeof(nglMesh) ) // bigger
	{
  	return KSMemAllocate(Size,Align,"KSMemAllocNGL - nglMesh?",0 );
	}
	if ( Size==sizeof(nglTexture) ) // 128
	{
  	return KSMemAllocate(Size,Align,"KSMemAllocNGL - nglTexture?",0 );
	}
  return KSMemAllocate(Size,Align,"KSMemAllocNGL",0 );
}

void* KSMemAllocNSL( u_int Size, u_int Align )
{
  return KSMemAllocate(Size,Align,"KSMemAllocNSL",0 );
}
void* KSMemAllocNVL( u_int Align, u_int Size )
{
  return KSMemAllocate(Size,Align,"KSMemAllocNVL",0 );
}
void KSMemFree( void* Ptr )
{
  free( Ptr );
}

bool KSReadFile( const char* FileName, nglFileBuf* File, u_int Align )
{
	//world_dynamics_system *wds = g_world_ptr; //app::inst();
	//if ( wds )
	//{
		bool was_locked = false;
		if(os_file::is_system_locked())
		{
			os_file::system_unlock();
			was_locked = true;
		}
		bool rv=world_dynamics_system::wds_readfile(FileName,&File->Buf,&File->Size,Align);
		//=wds->wds_readfile(FileName,&File->Buf,&File->Size,Align);
		if(was_locked) os_file::system_lock();
		return rv;
	//}
	#if 0
  static char Work[256];

  if ( 1 ) //strncmp( FileName, nglHostPrefix, strlen(nglHostPrefix) ) == 0 )
    strcpy( Work, "" );
  else
    strcpy( Work, nglHostPrefix );
  strcat( Work, FileName );

  int fd = sceOpen( Work, SCE_RDONLY );
  if ( fd < 0 )
    return false;

  File->Size = sceLseek( fd, 0, SCE_SEEK_END );
  sceLseek( fd, 0, SCE_SEEK_SET );

  // ps2 only reads in 32-byte chunks, rounding down, so we need to read in the extra on our own.
  File->Buf = (u_char*)KSMemAlloc( ( File->Size + 31 ) & ~31, 128 );
  sceRead( fd, File->Buf, ( File->Size + 31 ) & ~31 );
  sceClose( fd );

  return true;
	#endif
}

void KSReleaseFile( nglFileBuf* File )
{
	//world_dynamics_system *wds = g_world_ptr; //app::inst();
	//if ( wds )
	{
		//wds->wds_releasefile(&File->Buf);
		world_dynamics_system::wds_releasefile(&File->Buf);
	}
	//else
  //	KSMemFree( File->Buf );
  memset( File, 0, sizeof(nglFileBuf) );
}



nglSystemCallbackStruct nglhooks={
	&KSReadFile,
	&KSReleaseFile,
	&KSCriticalError,
	&KSDebugPrint,
	&KSMemAllocNGL,
	&KSMemFree
};

//nvlSystemCallbackStruct nvlhooks={
//	&KSCriticalError,
//	&KSDebugPrint,
//	&KSMemAllocNGL,
//	&KSMemFree
//};

//#define	T10K

#ifdef	T10K
#define MEM_SIZE	0x08000000	/* T10000 memory size */
#else
#define MEM_SIZE	0x02000000	/* PS2 memory size */
#endif
#define	DUMMY_PAGE	0x1000		/* 4kb dummy page  */
#define	HEADER_SIZE	0x10		/* malloc header  */

#define	printf		scePrintf	/* scePrintf does not call
					   malloc */


// As per sony recommendations, we do a full free and alloc of all
// available memory so the heap grows correctly.-mjd
unsigned int g_full_alloc_size = 0;

int full_alloc_and_free()
{
//  void *p;
  int heap_size, end, stack_size;
  extern void *_end, *_stack_size;

  end = (int)&_end;
  stack_size = (int)&_stack_size + 0x1000;
  /* default settings needs more 4kb memory */

  heap_size = MEM_SIZE - (end + stack_size + DUMMY_PAGE);

  // use std malloc and free
#if 0
#undef malloc
#undef free
  mem_unbreak_malloc();
  while( ( p = malloc( heap_size - HEADER_SIZE ) ) == NULL )
  {
    heap_size -= 16;
  }
  free(p);
  mem_break_malloc();
#define malloc   arch_malloc
#define free     arch_free
#endif
  return heap_size - HEADER_SIZE;
}


#if 0
// We preallocate a hell of a lot of STL small objects here
// so no persistant stl allocations occur in the middle of
// the heap -mjd
#define STL_MAX_ALLOCS ( 32 * 1024 )
#define STL_MAX_BYTES 128
#define STL_ALIGN 8
#define STL_ALLOC_COUNTS_LINE __LINE__
                                                         /*    8     16     24     32     40     48     56     64 */
static int stl_alloc_counts[STL_MAX_BYTES / STL_ALIGN] = {  2048,  4096, 32768,  4096,  4096,   256,   512,   256,
                                                         /*   72     80     88     96    104    112    120    128 */
                                                              64,   512,    32,    64,    64,    128,    64,   256 };

unsigned int cast_stl_magic()
{
  void **ptr_to_alloc;
  __STL_DEFAULT_ALLOCATOR(void) stl_allocator;
  int i,j;
  unsigned int total_bytes = 0;
  ptr_to_alloc = new void* [STL_MAX_ALLOCS];

  for( i = 0; i < STL_MAX_BYTES / STL_ALIGN; i++ )
  {
    int count;
#ifdef __STL_USING_COUNT_ALLOCS
    count = stl_allocator.__alloc_counts[i];
#else
    count = 0;
#endif
    for( j = 0; j < stl_alloc_counts[i] - count; j++ )
    {
      ptr_to_alloc[j] = stl_allocator.allocate( ( i + 1 ) * STL_ALIGN );
      total_bytes += ( ( i + 1 ) * STL_ALIGN );
    }

    for( j = 0; j < stl_alloc_counts[i] - count; j++ )
      stl_allocator.deallocate( ptr_to_alloc[j], ( i + 1 ) * STL_ALIGN );
  }

  delete [] ptr_to_alloc;

  return total_bytes;
}


void check_stl_magic()
{
#ifdef __STL_USING_COUNT_ALLOCS
  __STL_DEFAULT_ALLOCATOR(void) stl_allocator;
  int i;
  for( i = 0; i < STL_MAX_BYTES / STL_ALIGN; i++ )
  {
    if( stl_allocator.__alloc_peaks[i] > stl_alloc_counts[i] )
      break;
  }

  if( i < STL_MAX_BYTES / STL_ALIGN )
  {
    nglPrintf( "STL Preallocation Overflow\nThe values set in ps2main.cpp line %d are no longer sufficent\n", STL_ALLOC_COUNTS_LINE );
    nglPrintf( "The following values are required to preallocate the correct amount of STL memory\n" );
    nglPrintf( "{ " );
    int j;
    for( j = 0; j < ( STL_MAX_BYTES / STL_ALIGN ) - 1; j++ )
    {
      nglPrintf( "%d, ", stl_allocator.__alloc_peaks[j] );
    }
    nglPrintf( "%d }\n", stl_allocator.__alloc_peaks[j] );
  }
#endif
}
#endif

#if !defined(JASON)
extern "C"
{


typedef double FLO_type;
unsigned int fptoui(FLO_type arg_a)
{
#if defined(DEBUG) || defined(AUTOBUILD)
  assert(0 && "We don't use this anymore.  Please use FTOI");
#endif
  return FTOI(arg_a);
}

}
#endif




static bool load_iop_from_host = false;

/*** main ***/
int main(int argc, char **argv)
{
  int checkpoint;
	mem_check_heap_init();

  // NEW bootstrap code
  if ( argc > 1 && strcmp(argv[1], "proview") == 0)
  {
		checkpoint = mem_init_checkpoint(false);
    nglSetProViewPS2( true );
    nslPreInitProviewModePS2(true);
  }
  else
  {
		checkpoint = mem_init_checkpoint(true);
    nslPreInitProviewModePS2(false);
  }
//  do_balloc_memory_hack();	// Is this important? It allocates a lot of memory!  (dc 01/11/02)

#ifndef BUILD_FINAL
  load_iop_from_host = (CheckMemorySizeMB() == 128);	// load from host if we're on a dev kit. (dc 01/07/02)
#endif

  pstring spideyname = "Kelly Slater Pro Surfer";
  application_name = argv[0];
  g_ksps_path = argv[0];

  // Reboot IOP with NEW IMG flash
  sceSifInitRpc(0);

  nglPrintf("%s: Welcome to the exciting world of %s!\n", application_name, spideyname.c_str());

	// Some versions of printf allocate memory the first time they need scratch space.
  // They don't free this memory, so it can cause fragmentation.  Better get it out
  // of the way now. (dc 02/01/02)
extern void UseStaticBallocMem(void);
	UseStaticBallocMem();

  int DiskType;
  if (nglUsingProView == false)
  {
    /* wait for DiskReady */
    sceCdInit(SCECdINIT);
#ifdef BUILD_FINAL
	DiskType = SCECdPS2DVD;
#else
	DiskType = sceCdGetDiskType();
#endif
	sceCdMmode(DiskType == SCECdPS2DVD ? SCECdDVD : SCECdCD);

    /* reboot IOP, replace default modules */
	while( !sceSifRebootIop(load_iop_from_host ? IOPRP_IMGHOST : IOPRP_IMGCD) );
    while( !sceSifSyncIop() );
    sceSifInitRpc(0);
  }
  
  /* reinitialize */
  sceCdInit(SCECdINIT);
#ifdef BUILD_FINAL
  DiskType = SCECdPS2DVD;
#else
  DiskType = sceCdGetDiskType();
#endif
  sceCdMmode(DiskType == SCECdPS2DVD ? SCECdDVD : SCECdCD);
  sceFsReset();

  //assert (DiskType == sceCdGetDiskType());	// has the answer changed since IOP reboot?

#ifdef TARGET_PS2
  nslPreInitCdDvdModePS2(DiskType == SCECdPS2DVD ? NSL_PS2_DVD_MODE : NSL_PS2_CD_MODE);
#endif

#if !defined(BUILD_BOOTABLE) && !defined(KEVIN) && !defined(TOBY)
  if (nglUsingProView)
#endif
  {
		g_debug.assert_screen = true;
  }
  int mainThreadId = GetThreadId(); 
  ChangeThreadPriority(mainThreadId, 1);
  stash::acquire_stash_bufferspace(BIG_ASS_BUFFER_SIZE);

  os_alloc_init();
  os_developer_options::create_inst();
  debug_print("game.ini parsed");

#ifdef BUILD_BOOTABLE
  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_PRESS_BUILD))
  {
	  g_debug.halt_on_asserts = false;
  }
#endif

  // Must come after game.ini is parsed! (dc 01/11/07)
  nglSetSystemCallbacks( &nglhooks );	// Must come before nglInit!!! (dc 01/11/02)
#ifndef USE_DEFAULT_NGL_BUFFER_SIZES
  // Must come before nglInit!!! (dc 01/11/02)
  nglSetBufferSize( NGLBUF_VIF1_PACKET_WORK,
	  os_developer_options::inst()->get_int(os_developer_options::INT_NGL_VIF1_PACKET_BUFFER) * 1024 );
  nglSetBufferSize( NGLBUF_GIF_PACKET_WORK,
	  os_developer_options::inst()->get_int(os_developer_options::INT_NGL_GIF_PACKET_BUFFER) * 1024 );
  nglSetBufferSize( NGLBUF_SCRATCH_MESH_WORK,
	  os_developer_options::inst()->get_int(os_developer_options::INT_NGL_SCRATCH_MESH_BUFFER) * 1024 );
  nglSetBufferSize( NGLBUF_LIST_WORK,
	  os_developer_options::inst()->get_int(os_developer_options::INT_NGL_LIST_BUFFER) * 1024 );
#endif

#ifdef TV_PAL
  nglSetTVMode(NGLTV_PAL);
#else
  nglSetTVMode(NGLTV_NTSC);
#endif

  nglInit();
  nglSetIFLSpeed(60.f);

  //nvlSetSystemCallbacks( &nvlhooks );

  if (   !os_developer_options::inst()->is_flagged( os_developer_options::FLAG_NO_EXCEPTION_HANDLER )
      || nglUsingProView )
    register_exception_handlers();

  low_level_console_init();

  low_level_console_print( "Kelly Slater PS2 " BUILD_NAME );
  llc_memory_log();

  init_fast_acos_table();
  init_fast_sin_table();


  application_startup();

  application_run();
  application_shutdown();   // If it ever gets here we're in deep doo doo

  stash::release_stash_bufferspace();

  mem_check_leaks_since_checkpoint( checkpoint );
  return 0;
}

extern os_file* files_used_log;
extern const char* FILES_USED_LOG_NAME;

rational_t timer_1;
rational_t timer_2;

void application_startup()
{
  nglPrintf("%s: application is starting up.\n", application_name);

  master_clock::create_inst();
  g_master_clock_is_up = true;

#ifdef PROFILING_ON
  profiler::create_inst();
#endif




#if 0
  hires_clock_t timer_test;

  po po1;
  char test1;
  po po2;
  char test2;
  po po3;
  char test3;
  po po4;
  char test4;
  po po5;
  char test5;

  vector3d t = vector3d(1,2,3);
//  po1.set_translate(t);
//  po2 = po_identity_matrix;

  vector3d dir(PLUS_MINUS_ONE, PLUS_MINUS_ONE, PLUS_MINUS_ONE);
  dir.normalize();
  po1.set_rot(dir, PLUS_MINUS_ONE*PI);

  vector3d dir2(PLUS_MINUS_ONE*10.0f, PLUS_MINUS_ONE*10.0f, PLUS_MINUS_ONE*10.0f);
//  dir2.normalize();
  po2.set_translate(dir2);

  int test_loop = 0;


  timer_test.reset();
  for(test_loop=0; test_loop<100000; ++test_loop)
    po3 = po2*po1;
  timer_1 = timer_test.elapsed_and_reset();

  timer_test.reset();
  for(test_loop=0; test_loop<100000; ++test_loop)
    fast_po_mul(po4, po2, po1);
  timer_2 = timer_test.elapsed_and_reset();


  float sin1, cos1, sin2, cos2, sin3, cos3;

  float multiplier = PI*12.27f;

  timer_test.reset();
  for(test_loop=0; test_loop<100000; ++test_loop)
    sin1 = sin(0.5f);
  timer_1 = timer_test.elapsed_and_reset();

  timer_test.reset();
  for(test_loop=0; test_loop<100000; ++test_loop)
    fast_sin_cos_approx(0.5f, &sin2, &cos2 );
  timer_2 = timer_test.elapsed_and_reset();

  nglPrintf("%f(C) vs. %f(ASM)\n", timer_1, timer_2);

  multiplier = PI*3.0f;
  for(test_loop=0; test_loop<100000; ++test_loop)
  {
    rational_t t = (((float)test_loop/100000.0f)*2.0f*PI);//PLUS_MINUS_ONE*multiplier;
    fast_sin_cos_approx(t, &sin2, &cos2 );
    sin1 = sin(t);
    cos1 = cos(t);
    sin3 = fast_cos(t-(PI*0.5f));
    cos3 = fast_cos((PI*0.5f)-t);

    if(__fabs(sin1-sin2) >= 0.000001f)
    {
      nglPrintf("sin(%f) = %f(C) vs. %f(ASM) diff: %f\n", t, sin1, sin2, __fabs(sin1-sin2));
    }

    if(__fabs(cos1-cos2) >= 0.000001f)
    {
      nglPrintf("cos(%f) = %f(C) vs. %f(ASM) diff: %f\n", t, cos1, cos2, __fabs(cos1-cos2));
    }

    t = 0.0f;
  }

#endif

//  os_alloc_init();

//  os_developer_options::create_inst();

#ifdef FILES_USED_LOG
  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
  {
    files_used_log = NEW os_file(FILES_USED_LOG_NAME, os_file::FILE_WRITE);
    if (files_used_log->is_open())
    {
      stringx msg = "\\data\\m0_arena\\ps2 meshes\\sphere.ps2mesh\r\n";
      files_used_log->write((void *)msg.c_str(), msg.length());
      msg = "\\data\\m0_arena\\ps2 meshes\\radius.ps2mesh\r\n";
      files_used_log->write((void *)msg.c_str(), msg.length());
      files_used_log->close();
    }
  }
#endif

  app::create_inst();
}

void application_shutdown()
{
  nglPrintf("%s: application is shutting down.\n", application_name);

  app::delete_inst();
  os_developer_options::delete_inst();

  os_alloc_shutdown();
}

void application_run()
{
  nglPrintf("%s: application is entering run loop.\n", application_name);

  while (!g_game_ptr->get_i_quit())
  {
    app* theapp = app::inst();
    assert(theapp);

    if ( g_app_active )
    {
      master_clock::inst()->tick();

      #ifdef PROFILING_ON
//      START_PROF_TIMER( proftimer_profiler );
      profiler::inst()->start_frame();
//      STOP_PROF_TIMER( proftimer_profiler );
      START_PROF_TIMER( proftimer_frame_total );
      #endif

      theapp->tick();

      STOP_PROF_TIMER( proftimer_frame_total );
    }
  }
  nglPrintf("%s: application is leaving run loop.\n", application_name);
}

void system_idle() // do system message-processing so the app doesn't appear hung to the OS
{
}

#ifdef ENABLE_EXCEPTION_HANDLER
void print_exception_info(const char *debug_msg, u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  nglPrintf("Exception Type:        %s\n", debug_msg);
  nglPrintf("Program Counter:       0x%08x(%u)\n", epc, epc);
  nglPrintf("COP0 Status Register:  0x%08x(%u)\n", stat, stat);
  nglPrintf("COP0 Cause Register:   0x%08x(%u)\n", cause, cause);
  nglPrintf("Bad address:           0x%08x(%u) (when a memory access or branch is invalid)\n", bva, bva);
  nglPrintf("Bus error address:     0x%08x(%u)\n", bpa, bpa);
  nglPrintf("General Purpose Register dump:\n");
  for (int i=0; i<32; ++i)
  {
    int *gpr_int = (int *)&gpr[i];
    nglPrintf("gpr[%d] = %08x %08x %08x %08x\n", i, gpr_int[0], gpr_int[1], gpr_int[2], gpr_int[3] );
  }
  nglPrintf("Now halting in a while(1) loop.\n");
  if (low_level_console_is_available())
  {
    low_level_console_print("%s", debug_msg);
    low_level_console_print("PC 0x%08x", epc);
    llc_memory_log();
  }
  while (1);
}

void TLB_change_exception_handler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  print_exception_info("TLB change exception", stat, cause, epc, bva, bpa, gpr);
}

void TLB_mismatch_load_exception_handler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  print_exception_info("TLB mismatch (load)", stat, cause, epc, bva, bpa, gpr);
}

void TLB_mismatch_store_exception_handler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  print_exception_info("TLB mismatch (store)", stat, cause, epc, bva, bpa, gpr);
}

void address_error_load_exception_handler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  print_exception_info("Address error (load)", stat, cause, epc, bva, bpa, gpr);
}

void address_error_store_exception_handler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  print_exception_info("Address error (store)", stat, cause, epc, bva, bpa, gpr);
}

void bus_error_fetch_exception_handler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  print_exception_info("Bus error (fetch)", stat, cause, epc, bva, bpa, gpr);
}

void bus_error_data_exception_handler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
  print_exception_info("Bus error (load/store)", stat, cause, epc, bva, bpa, gpr);
}

void register_exception_handlers()
{
  void *old;
  old = SetDebugHandler(1, TLB_change_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(2, TLB_mismatch_load_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(3, TLB_mismatch_store_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(4, address_error_load_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(5, address_error_store_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(6, bus_error_fetch_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(7, bus_error_data_exception_handler);
  assert(old == NULL);
  /* these two hose the debugger, I don't know why --gt
  old = SetDebugHandler(8, system_call_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(10, reserved_instruction_exception_handler);
  assert(old == NULL);
  old = SetDebugHandler(12, calculation_overflow_exception_handler);
  assert(old == NULL);*/
}
#else
void register_exception_handlers()
{
  return;
}
#endif

#endif

