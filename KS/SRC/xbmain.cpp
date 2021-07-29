//-----------------------------------------------------------------------------
// File: ps2main.cpp
//
//   This is it, ground zero for PS2 Spider-Man.
//   Entry-point for the program and so much more.
//
// Copyright (c) 2000 Treyarch LLC. All rights reserved.
//-----------------------------------------------------------------------------

#include "global.h"

#ifdef TARGET_XBOX

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
#include "game_info.h"
#include "semaphores.h"
extern "C" {
	  extern unsigned long end;
}

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
#include "hwosxb/xb_input.h"
#include "joystick.h"
#include "game.h"
#include "po.h"

extern game *g_game_ptr;

void init_fast_acos_table();
void init_fast_sin_table();


#ifdef BUILD_BOOTABLE
	#ifndef BOOTABLE_WITHOUT_CD
		#define IOPRP_IMG "cdrom0:\\"IOP_IMAGE_FILE";1"
		#define IOPRP_IMGB "host0:"IOP_IMAGE_FILE
	#else
		#define IOPRP_IMGB "cdrom0:\\"IOP_IMAGE_FILE";1"
		#define IOPRP_IMG "host0:"IOP_IMAGE_FILE
	#endif
#else
	#define IOPRP_IMG "host0:"IOP_IMAGE_FILE
	#define IOPRP_IMGB "cdrom0:\\"IOP_IMAGE_FILE";1"
#endif

void register_exception_handlers();

void KSCriticalError( const char* Text )
{
	error( Text );
  // Removed since we now let users try to button through these
  //for( ;; );
}
void KSHeapError( const char* Text )
{
	
	error( Text );
  // Removed since we now let users try to button through these
//  for( ;; );
}

void KSDebugPrint( const char* Text )	//--- can't use nglPrintf from here, because it causes an infinite loop.
{
  printf( Text );
}

void* KSMemAllocate( u_int Size, u_int Align, const char *file, int line )
{
  return arch_memalign(Align, Size, file, line);
}

void* KSMemAlloc( u_int Size, u_int Align )
{
  return arch_malloc(Size);
}

#undef BUILD_DEBUG

#ifdef BUILD_DEBUG
void* KSMemAllocNGL( u_int Size, u_int Align, const char *name )
#else
void* KSMemAllocNGL( u_int Size, u_int Align )
#endif
{
	#ifdef BUILD_DEBUG
	if ( name )
	{
		static char tmp[64];
		strcpy(tmp,"nglAlloc:");
		strcat(tmp,name);
  	return KSMemAllocate(Size,Align,tmp,0 );
	}
	#endif
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
  arch_free( Ptr);
}

bool KSReadFile( const char* FileName, nglFileBuf* File, u_int Align )
{
  bool was_locked = false;
  if(os_file::is_system_locked())
  {
    os_file::system_unlock();
    was_locked = true;
  }
  bool rv = world_dynamics_system::wds_readfile(FileName,&File->Buf,&File->Size,Align);

  if(was_locked)
    os_file::system_lock();

  return rv;
}

void KSReleaseFile( nglFileBuf* File )
{
	{
		world_dynamics_system::wds_releasefile(&File->Buf);
	}
  memset( File, 0, sizeof(nglFileBuf) );
}



nglSystemCallbackStruct nglhooks={
	&KSReadFile,
	&KSReleaseFile,
	&KSCriticalError,
	NULL,
	&KSMemAllocNGL,
	&KSMemFree
};



/*** main ***/
int main(int argc, char **argv)
{
  int checkpoint = mem_set_checkpoint();

  // NEW bootstrap code
  pstring spideyname = "Kelly Slater Pro Surfer";

  application_name = argc ? argv[0] : "ks.xbe";

  nglSetSystemCallbacks( &nglhooks );

  nglPrintf("%s: Welcome to the exciting world of %s!\n", application_name, spideyname.c_str());

  const bool nglUsingProView = false;

  os_alloc_init();
  os_developer_options::create_inst();
  debug_print("game.ini parsed");

  if (   !os_developer_options::inst()->is_flagged( os_developer_options::FLAG_NO_EXCEPTION_HANDLER )
      || nglUsingProView )
    register_exception_handlers();

  if (os_developer_options::inst()->is_flagged( os_developer_options::FLAG_CACHE_TO_DISK))
  {
    gDVDCache.InitDVDCache();
  }
#if 0
#ifdef TV_PAL
  nglSetTVMode(NGLTV_PAL);
#else
  nglSetTVMode(NGLTV_NTSC);
#endif
#endif
  nglInit();


  
  init_fast_acos_table();
  init_fast_sin_table();

  application_startup();

  low_level_console_init();
  
	g_debug.assert_screen = true;
  
	application_run();
  application_shutdown();

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

#ifndef NDEBUG
  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
  {
    files_used_log = NEW os_file(FILES_USED_LOG_NAME, os_file::FILE_WRITE);
    if (files_used_log->is_open())
    {
      stringx msg = "\\data\\m0_arena\\xb meshes\\sphere.xbmesh\r\n";
      files_used_log->write((void *)msg.c_str(), msg.length());
      msg = "\\data\\m0_arena\\xb meshes\\radius.xbmesh\r\n";
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
