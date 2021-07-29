// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "career.h"

#include "crc32.h"

#include "gc_GameSaver.h"
#include <dolphin/card.h>
#include <charPipeline/texPalette.h>
//                 12345678901234567890123456789012

#define BLOCK_SIZE 8192
#define PADDING 16384
const int GenericGameSaver::saved_game_size = 
	((sizeof(*g_career) + sizeof(GlobalDataClass) + PADDING + BLOCK_SIZE - 1) / BLOCK_SIZE);

#define COMMENT_1 "Kelly Slater Pro Surfer         "
#define COMMENT_SIZE 32

#define GC_SYSTEM_FILE "!!Global!!" // Bangs(!) are important, the user can't save files like this.
#define ICON_FILE "ksico.tpl"
#define ICON_SIZE 32
#define BANNER_HSIZE 32
#define BANNER_WSIZE 96
#define ICON_SPEED CARD_STAT_SPEED_SLOW

#define GCGS_STACK_SIZE 8 * 1024
#define GCGS_THREAD_PRIO 8

#define GC_BUSY_WHILE(r) while( ( r ) == CARD_RESULT_BUSY ){}

saveInfo currentGame;
int savePort=INVALID_CARD_VALUE, saveSlot=INVALID_CARD_VALUE;


static u8 gs_work_area[CARD_WORKAREA_SIZE] __attribute__((aligned(32)));
static u8 gs_small_read_buffer[CARD_READ_SIZE] __attribute__((aligned(32)));


void GenericGameSaver::ENTER_CRITICAL()
{
	OSWaitSemaphore( &thread_sema );
}

void GenericGameSaver::EXIT_CRITICAL()
{
	OSSignalSemaphore( &thread_sema );
}

void *GenericGameSaver::gs_dispatch(void *param)
{
	GenericGameSaver *inst = (GenericGameSaver *) param;
	inst->dispatch();
	return NULL;
}


void GenericGameSaver::dispatch()
{
	while( 1 )
	{
		switch( dispatch_data.type )
		{
			case DISPATCH_SAVE:
				saveData( dispatch_data.slot, dispatch_data.save_info,
						dispatch_data.buffer, dispatch_data.size, dispatch_data.over_write,
						dispatch_data.progress_callback, dispatch_data.user_callback_data );
				break;

			case DISPATCH_READ:
				readData( dispatch_data.slot, dispatch_data.save_info,
						dispatch_data.buffer, dispatch_data.size,
						dispatch_data.progress_callback, dispatch_data.user_callback_data );
				break;

			default:
				OSReport( "Unknown dispatch type!\n" );
				break;
		}

		OSSuspendThread( OSGetCurrentThread() );
	}
}
	
DEFINE_SINGLETON(GenericGameSaver)

int GenericGameSaver::mount( int slot )
{
	s32 card_size, sector_size;
  int ret;
	
  if( slot < 0 )
    return GSErrorNoMedia;
  
  GC_BUSY_WHILE( ret = CARDProbeEx( slot, &card_size, &sector_size ) );
	
	ret = CARDToGSError( ret );
	if( ret != GSOk )
		return ret;

	if( sector_size != BLOCK_SIZE )
		return GSErrorIncompatible;
	
	GC_BUSY_WHILE( ret = CARDMount( slot, gs_work_area, NULL ) );
	if( ret != CARD_RESULT_BROKEN && ret != CARD_RESULT_READY )
		return CARDToGSError( ret );

	GC_BUSY_WHILE( ret = CARDCheck( slot ) );

  ret = CARDToGSError( ret );

	return ret;
}

int GenericGameSaver::unmount( int slot )
{
  if( slot < 0 )
    return GSErrorNoMedia;

  int ret;
	
	GC_BUSY_WHILE( ret = CARDUnmount( slot ) );
  
	return CARDToGSError( ret );
}

int  GenericGameSaver::savePhoto (int slot, int _unused, saveInfo *game, int beach, CompressedPhoto* data )
{
	assert( 0 );
	return GSErrorOther;
}
int  GenericGameSaver::loadPhoto (int slot, int _unused, saveInfo *game, int beach, CompressedPhoto* data )
{
	assert( 0 );
	return GSErrorOther;
}

bool GenericGameSaver::photoExists (int slot, int _unused, saveInfo *game, int beach)
{
	assert( 0 );
	return GSErrorOther;
}

bool GenericGameSaver::photoExists(const int beachIdx)
{
	assert( 0 );
	return GSErrorOther;
}

/////////////////////////////////////////////////////////////////////////////////////////
int GenericGameSaver::saveSystemFile(int slot, int _unused, GlobalDataClass *data, void(*progressCallback)(void *, int), void *userCallbackData)
{	
	saveInfo s;
	strcpy(s.shortname, GC_SYSTEM_FILE );
	strcpy(s.desc, "Global Settings");
	s.version = CAREER_DATA_VERSION;
	setFileInfo(s);

	saveFile(slot, _unused, (void *)data, sizeof(GlobalDataClass), true, progressCallback, userCallbackData);
	return GSOk;
}

int  GenericGameSaver::readSystemFile( int slot, int _unused, GlobalDataClass *data, void(*progressCallback)(void *, int), void *userCallbackData)
{
	saveInfo s;
	strcpy(s.shortname, GC_SYSTEM_FILE );
	s.version = CAREER_DATA_VERSION;
	setFileInfo(s);

	readFile(slot, _unused, (void *)data, sizeof(GlobalDataClass), progressCallback, userCallbackData);
	return GSOk;
}

bool GenericGameSaver::hasSystemFile( int slot, int _unused )
{	
	return ( isFileValid( slot, _unused, GC_SYSTEM_FILE ) == GSOk );
}

/////////////////////////////////////////////////////////////////////////////////////////
int GenericGameSaver::deleteFile(int slot, int _unused, saveInfo s)
{
  int ret;
  ENTER_CRITICAL();
  ret = mount( slot );
  if( ret != GSOk )
  {
    EXIT_CRITICAL();
    return ret;
  }
  GC_BUSY_WHILE( ret = CARDDelete( slot, s.shortname ) );
  unmount( slot );
  EXIT_CRITICAL();
  return CARDToGSError( ret );
}

/////////////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::getInfo(int slot, int _unused, int *type, int *free, int *formatted)
{
  int ret;
  long _dummy;
  long _free;
	u32 sector_size;
  ENTER_CRITICAL();

  ret = mount( slot );
  if( ret != GSOk && ret != GSErrorUnformatted )
  {
    EXIT_CRITICAL();
    return ret;
  }

  if( ret == GSErrorUnformatted )
  {
    *free = 0;
    *formatted = 0;
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  else if( ret != GSOk )
  {
    *free = 0;
    *formatted = 0;
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }

	GC_BUSY_WHILE( ret = CARDGetSectorSize( (long)slot, &sector_size ) );
  ret = CARDToGSError( ret );
  if( ret != GSOk )
  {
    *free = 0;
		*formatted = 0;
		unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
 
  GC_BUSY_WHILE( ret = CARDFreeBlocks( (long)slot, &_free, &_dummy ) );
  unmount( slot );
  EXIT_CRITICAL();
 
  ret = CARDToGSError( ret );
  if( ret == GSOk )
  {
    *formatted = 1;
    *free = (int)_free / sector_size;
  }
  else
  {
    *free = 0;
    *formatted = 0;
  }

  return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::init()
{
  CARDInit();
  
	thread_stack = ( char * ) malloc( GCGS_STACK_SIZE );
	if( !OSCreateThread( &thread_data, gs_dispatch, this, thread_stack + GCGS_STACK_SIZE, GCGS_STACK_SIZE, GCGS_THREAD_PRIO, 0 ) )
	{
		OSReport( "Could not create gamesaver thread!\n" );
		return GSErrorOther;
	}

	*thread_data.stackEnd = OS_THREAD_STACK_MAGIC;

	OSInitSemaphore( &thread_sema, 1 );
	
	return GSOk;
}


/////////////////////////////////////////////////////////////////////////////////////////
stringx GenericGameSaver::getSlotString(int port, int slot)
{
	char txt[200];
	sprintf( txt, ksGlobalTextArray[GT_MEMORY_SLOT_GC].c_str(), 'A' + port );
	return stringx( txt );
}

stringx GenericGameSaver::getShortCardString(int port, int slot)
{
	return getCardString(port, slot);
}

stringx GenericGameSaver::getCardString(int port, int slot)
{
	char errortxt[100];
	sprintf(errortxt, ksGlobalTextArray[GT_MEMORY_CARD_GC].c_str(), 'A' + port );
	return stringx(errortxt);
}
/////////////////////////////////////////////////////////////////////////////////////////


int GenericGameSaver::format(int slot, int _unused)
{
  int ret;
  ENTER_CRITICAL();
  
  ret = mount( slot );
  if( ret != GSOk && ret != GSErrorUnformatted && ret != GSErrorWrongRegion )
  {
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }

  GC_BUSY_WHILE( ret = CARDFormat( slot ) );

  unmount( slot );
  EXIT_CRITICAL();
  
  return CARDToGSError( ret );
}

/////////////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::getFileListing(int slot, int _unused, saveInfo *info, void(*progressCallback)(void *, float), void *userData)
{
  saveInfo *save_info;
  CARDFileInfo file_info;
  CARDStat file_stat;
  
  int ret;
  int file_no;
  int num_info = 0;
  
  DVDDiskID *disk_id = DVDGetCurrentDiskID();

  ENTER_CRITICAL();

  ret = mount( slot );
  if( ret != GSOk )
  {
    if( progressCallback )
      progressCallback( userData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return 0;
  }

  
  for( file_no = 0; file_no < 127; file_no ++ )
  {
    if( progressCallback )
      progressCallback( userData, ( file_no / 128.0f ) * 99.0f );
    GC_BUSY_WHILE( ret = CARDGetStatus( slot, file_no, &file_stat ) );
		if( ret != CARD_RESULT_READY
        || memcmp( file_stat.gameName, disk_id->gameName, sizeof( disk_id->gameName ) ) != 0
        || memcmp( file_stat.company, disk_id->company, sizeof( disk_id->company ) ) != 0 )
      continue;
    
		
    GC_BUSY_WHILE( ret = CARDFastOpen( slot, file_no, &file_info ) );
		if( ret != CARD_RESULT_READY )
      continue;

		
    GC_BUSY_WHILE( ret = CARDRead( &file_info, gs_small_read_buffer, CARD_READ_SIZE, 0 ) );
		if( ret != CARD_RESULT_READY )
    {
      GC_BUSY_WHILE( CARDClose( &file_info ) );
      continue;
    }

    GC_BUSY_WHILE( CARDClose( &file_info ) );
    
    save_info = (saveInfo *) gs_small_read_buffer;

    if( save_info->version == CAREER_DATA_VERSION
				&& strcmp( save_info->shortname, GC_SYSTEM_FILE ) )
    {
      if( info )
			{
				memcpy( &info[num_info], save_info, sizeof( saveInfo ) );
      	info[num_info].valid = GSOk;
			}
      num_info++;
    }
    
  }
    
  unmount( slot );
  
  EXIT_CRITICAL();
  
  if( progressCallback )
    progressCallback( userData, 100 );
      
  return num_info;
}

/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::readFile( int slot, int _unused, void *buffer, int maxSize,
                                 void (*progressCallback)(void *, int),
                                 void *userCallbackData)
{
  ENTER_CRITICAL();
	dispatch_data.type = DISPATCH_READ;
	dispatch_data.slot = slot;
	dispatch_data.buffer = buffer;
	dispatch_data.size = maxSize;
	dispatch_data.progress_callback = progressCallback;
	dispatch_data.user_callback_data = userCallbackData;
	dispatch_data.save_info = &fInfo;
	OSResumeThread( &thread_data );
	EXIT_CRITICAL();
}

int GenericGameSaver::readData( int slot, saveInfo *sInfo, void *buffer, int size,
                                void (*progressCallback)(void *, int), void *userCallbackData )
{
  int ret;
  unsigned long sector_size;
  
  CARDFileInfo file_info;
  saveInfo *save_info;
  CARDStat file_stat;
  char *read_buffer = NULL;
  
  ENTER_CRITICAL();

  ret = mount( slot );
  if( ret != GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  GC_BUSY_WHILE( ret = CARDGetSectorSize( (long)slot, &sector_size ) );
  ret = CARDToGSError( ret );
  if( ret != GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
        
  
  GC_BUSY_WHILE( ret = CARDOpen( slot, sInfo->shortname, &file_info ) );
  ret = CARDToGSError( ret );
  
  if( ret == GSOk )
  {
    if( progressCallback )
        progressCallback( userCallbackData, 20 );
  }
  else
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  GC_BUSY_WHILE( ret = CARDGetStatus( slot, CARDGetFileNo( &file_info ), &file_stat ) );
  ret = CARDToGSError( ret );
  if( ret != GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  read_buffer = (char *)memalign( 32, file_stat.length );
  
  GC_BUSY_WHILE( ret = CARDRead( &file_info, read_buffer, file_stat.length, 0 ) );
  ret = CARDToGSError( ret );

  if( ret == GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, 50 );
  }
  else
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    GC_BUSY_WHILE( CARDClose( &file_info ) );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }

  save_info = (saveInfo *)read_buffer;
  
  int info_crc = save_info->info_crc;
	save_info->info_crc = 0xffffffff;
	bool bad_crc = false;
	
	bad_crc = ( info_crc != gen_crc32( 0, save_info, sizeof( saveInfo ) ) );
	if( !bad_crc )
	{
		bad_crc = ( save_info->data_crc != gen_crc32( 0, &read_buffer[ file_stat.offsetData ], size ) );
	}

	if( bad_crc )
	{
		if( progressCallback )
			progressCallback( userCallbackData, GSErrorBadCRC );
    GC_BUSY_WHILE( CARDClose( &file_info ) );
    unmount( slot );
    free( read_buffer );
    EXIT_CRITICAL();
    return GSErrorBadCRC;
	}
	
	if( save_info->version != CAREER_DATA_VERSION )
  {
    if( progressCallback )
      progressCallback( userCallbackData, GSErrorBadVersion );
    GC_BUSY_WHILE( CARDClose( &file_info ) );
    unmount( slot );
    free( read_buffer );
    EXIT_CRITICAL();
    return GSErrorBadVersion;
  }
    
  memcpy( buffer, &read_buffer[ file_stat.offsetData ], size );
  
  free( read_buffer );
  
	GC_BUSY_WHILE( CARDClose( &file_info ) );
	
  ret = unmount( slot );
  if( ret != GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    EXIT_CRITICAL();
    return ret;
  }

  EXIT_CRITICAL();
  if( progressCallback )
    progressCallback( userCallbackData, 100 );

  return GSOk;
}


/////////////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::isFileValid( int slot, int _unused, char *shortname )
{
  int ret;
  
  CARDFileInfo file_info;
  saveInfo *save_info;
  
  ENTER_CRITICAL();

  ret = mount( slot );
  if( ret != GSOk )
  {
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  GC_BUSY_WHILE( ret = CARDOpen( slot, shortname, &file_info ) );
  ret = CARDToGSError( ret );
  
  if( ret != GSOk )
  {
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  GC_BUSY_WHILE( ret = CARDRead( &file_info, gs_small_read_buffer, CARD_READ_SIZE, 0 ) );
  ret = CARDToGSError( ret );
  
	GC_BUSY_WHILE( CARDClose( &file_info ) );

  if( ret != GSOk )
  {
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }

  save_info = (saveInfo *)gs_small_read_buffer;
  
  if( save_info->version != CAREER_DATA_VERSION )
  {
    unmount( slot );
    EXIT_CRITICAL();
    return GSErrorBadVersion;
  }
    
  ret = unmount( slot );
  if( ret != GSOk )
  {
    EXIT_CRITICAL();
    return ret;
  }

  EXIT_CRITICAL();
  
	return GSOk;
}

/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::releaseIconData()
{
}

int GenericGameSaver::getIconData()
{
  return GSOk;
}

/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::saveFile( int slot, int _unused,
                                 void *buffer, int size,
                                 bool overWrite,
                                 void (*progressCallback)(void *, int),
                                 void *userCallbackData)
{
  ENTER_CRITICAL();
	dispatch_data.type = DISPATCH_SAVE;
	dispatch_data.slot = slot;
	dispatch_data.buffer = buffer;
	dispatch_data.size = size;
	dispatch_data.over_write = overWrite;
	dispatch_data.progress_callback = progressCallback;
	dispatch_data.user_callback_data = userCallbackData;
	dispatch_data.save_info = &fInfo;
	OSResumeThread( &thread_data );
	EXIT_CRITICAL();
}

int GenericGameSaver::saveData( int slot, saveInfo *sInfo, void *buffer, int size, bool overWrite,
    void (*progressCallback)(void *, int),
    void *userCallbackData )
{
  int ret;
  CARDFileInfo file_info;
  CARDStat file_stat;
  CARDStat icon_stat;
  nglFileBuf icon_file;
  char *icon_base;
  int icon_size;
  unsigned long sector_size;
  char *write_buffer;
  
  ENTER_CRITICAL();

  ret = mount( slot );
  if( ret != GSOk )
  {
    progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }

  icon_stat.commentAddr = sizeof( saveInfo );
  GC_BUSY_WHILE( ret = CARDGetSectorSize( (long)slot, &sector_size ) );
  ret = CARDToGSError( ret );
  if( ret != GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  if( !KSReadFile( ICON_FILE, &icon_file, 0 ) )
  {
    if( progressCallback )
      progressCallback( userCallbackData, GSErrorOther );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  icon_base = iconInit( &icon_stat, &icon_file, &icon_size );
  
  int total_size = sizeof( saveInfo ) + ( COMMENT_SIZE * 2 ) + icon_size + size;
  int aligned_size = ( ( total_size + sector_size - 1 ) / sector_size ) * sector_size;


  GC_BUSY_WHILE( ret = CARDCreate( slot, sInfo->shortname, aligned_size, &file_info ) );
  ret = CARDToGSError( ret );

  sInfo->version = (float) CAREER_DATA_VERSION;
 	sInfo->timestamp = OSTicksToSeconds( OSGetTime() );
  
  if( ret == GSErrorFileExists )
  {
    if( overWrite )
    {
      GC_BUSY_WHILE( ret = CARDDelete( slot, sInfo->shortname ) );
      ret = CARDToGSError( ret );
      if( ret == GSOk )
      {
        GC_BUSY_WHILE( ret = CARDCreate( slot, sInfo->shortname, aligned_size, &file_info ) );
        ret = CARDToGSError( ret );
      }
    }
  }
  
  if( ret == GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, 20 );
  }
  else
  {
    KSReleaseFile( &icon_file );
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
  
  ret = getStatus( slot, CARDGetFileNo( &file_info ), &file_stat );
  if( ret != GSOk )
  {
    KSReleaseFile( &icon_file );
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }

  file_stat.bannerFormat = icon_stat.bannerFormat;
  file_stat.iconAddr = icon_stat.iconAddr;
  file_stat.commentAddr = icon_stat.commentAddr;
  file_stat.iconFormat = icon_stat.iconFormat;
  file_stat.iconSpeed = icon_stat.iconSpeed;

  ret = setStatus( slot, CARDGetFileNo( &file_info ), &file_stat );
  if( ret != GSOk )
  {
    KSReleaseFile( &icon_file );
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }

  write_buffer = (char *)memalign( 32, aligned_size );
 
  sInfo->data_crc = gen_crc32( 0, buffer, size );
	sInfo->info_crc = 0xffffffff;
	int info_crc = gen_crc32( 0, sInfo, sizeof( saveInfo ) );
	sInfo->info_crc = info_crc;
	char *ptr = write_buffer;
  memcpy( ptr, sInfo, sizeof( saveInfo ) );
  ptr += sizeof( saveInfo );
  memcpy( ptr, COMMENT_1, COMMENT_SIZE );
  ptr += COMMENT_SIZE;
  memset( ptr, ' ', COMMENT_SIZE );
	sInfo->desc[COMMENT_SIZE] = '\0';
	memcpy( ptr, sInfo->desc, strlen( sInfo->desc ) );
  ptr += COMMENT_SIZE;
  memcpy( ptr, icon_base, icon_size );
  ptr += icon_size;
  memcpy( ptr, buffer, size );
  
  KSReleaseFile( &icon_file );

  GC_BUSY_WHILE( ret = CARDWrite( &file_info, write_buffer, aligned_size, 0 ) );
  ret = CARDToGSError( ret );

  if( ret != GSOk )
  {
    free( write_buffer );
    KSReleaseFile( &icon_file );
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    unmount( slot );
    EXIT_CRITICAL();
    return ret;
  }
   
  free( write_buffer );
	
	GC_BUSY_WHILE( CARDClose( &file_info ) );
	
  ret = unmount( slot );
  if( ret != GSOk )
  {
    if( progressCallback )
      progressCallback( userCallbackData, ret );
    EXIT_CRITICAL();
    return ret;
  }

  EXIT_CRITICAL();
  if( progressCallback )
    progressCallback( userCallbackData, 100 );
  
  return GSOk;
}


/////////////////////////////////////////////////////////////////////////////////////////


void GenericGameSaver::setFileInfo(saveInfo s)
{
  fInfo = s;
};

int GenericGameSaver::getStatus(int slot, int file_no, CARDStat *fstat)
{
  int ret;

  GC_BUSY_WHILE( ret = CARDGetStatus( slot, file_no, fstat ) );

  return CARDToGSError( ret );
}

int GenericGameSaver::setStatus(int slot, int file_no, CARDStat *fstat)
{
  int ret;

  GC_BUSY_WHILE( ret = CARDSetStatus( slot, file_no, fstat ) );
  
  return CARDToGSError( ret );
}

int GenericGameSaver::CARDToGSError(int error)
{
  switch( error )
  {
    case CARD_RESULT_FATAL_ERROR:
    case CARD_RESULT_IOERROR:
    	return GSErrorDamaged;
			break;
			
		case CARD_RESULT_NOPERM:
    case CARD_RESULT_BUSY:
    case CARD_RESULT_LIMIT:
    case CARD_RESULT_NAMETOOLONG:
    case CARD_RESULT_CANCELED:
      return GSErrorOther;
      break;

    case CARD_RESULT_NOFILE:
      return GSErrorDoesNotExist;
      break;

    case CARD_RESULT_EXIST:
      return GSErrorFileExists;
      break;

    case CARD_RESULT_INSSPACE:
    case CARD_RESULT_NOENT:
      return GSErrorNotEnoughSpace;
      break;
        
    case CARD_RESULT_NOCARD:
      return GSErrorNoMedia;
      break;
    
    case CARD_RESULT_WRONGDEVICE:
      return GSErrorUnknownMedia;
      break;

    case CARD_RESULT_BROKEN:
      return GSErrorUnformatted;
      break;
			
    case CARD_RESULT_ENCODING:
    	return GSErrorWrongRegion;
			break;

    case CARD_RESULT_READY:
      return GSOk;
      break;

    default:
      assert( "Unknown error value" && 0 );
      break;
  }

  assert( 0 );

  return GSErrorOther;
}

#define NORMALIZED(base,ofs) ( ((char *)(base)) + ((int)(ofs)) )
char *GenericGameSaver::iconInit(CARDStat *f_stat, nglFileBuf *icon_file, int *size)
{
  TEXHeader *icon_tex;
  CLUTHeader *icon_clut;
  TEXPalette *icon_pal;
  bool banner = false;
  char *base = NULL;
  
  *size = 0;
  
  f_stat->iconFormat = 0;
  f_stat->bannerFormat = 0;
  f_stat->iconAddr = 0xffffffff;
  f_stat->iconSpeed = 0;
  
  CARDSetIconAddress( f_stat, f_stat->commentAddr + ( COMMENT_SIZE * 2 ) );

  icon_pal = (TEXPalette *)icon_file->Buf;
  int num_icons = icon_pal->numDescriptors;
  TEXDescriptor *des_array = (TEXDescriptor *) NORMALIZED( icon_pal, icon_pal->descriptorArray );

  for( int i = 0, j = 0; i < num_icons; i++ )
  {
    icon_tex = (TEXHeader *) NORMALIZED( icon_pal, des_array[i].textureHeader );
    icon_clut = (CLUTHeader *) NORMALIZED( icon_pal, des_array[i].CLUTHeader );
    
    if( base == NULL )
    {
      base = (char *) NORMALIZED( icon_pal, icon_tex->data );
    }

  	if( icon_tex->height == BANNER_HSIZE  && icon_tex->width == BANNER_WSIZE )
		{
			if( icon_clut != (CLUTHeader *)icon_pal )
      {
				CARDSetBannerFormat( f_stat, CARD_STAT_BANNER_C8 );
        *size += BANNER_HSIZE * BANNER_WSIZE + ( 2 * 256 );
      }
			else
      {
        CARDSetBannerFormat( f_stat, CARD_STAT_BANNER_RGB5A3 );
        *size += BANNER_HSIZE * BANNER_WSIZE * 2;
      }
			banner = true;
		}
		//icon 
		if( icon_tex->height == ICON_SIZE  && icon_tex->width == ICON_SIZE )
		{
			if( icon_clut != (CLUTHeader *)icon_pal )
      {
				CARDSetIconFormat( f_stat, j, CARD_STAT_ICON_C8 );
        *size += ICON_SIZE * ICON_SIZE + ( 2 * 256 );
      }
			else
      {
        CARDSetIconFormat( f_stat, j, CARD_STAT_ICON_RGB5A3 );
        *size += ICON_SIZE * ICON_SIZE * 2;
      }
			CARDSetIconSpeed( f_stat, j, ICON_SPEED );
			j++;
		}
  }
 
  assert( base );
  
  return base;
}
  
stringx GenericGameSaver::getErrorString(int port, int slot, int err)
{
	char errorText[256];
	
	switch( err )
	{
		case GSErrorUnformatted:
			sprintf( errorText, ksGlobalTextArray[ GT_MC_CORRUPT_GC ].c_str(), ( 'A' + port ) );
			break;
		case GSErrorUnknownMedia:
			sprintf( errorText, ksGlobalTextArray[ GT_MC_WRONG_GC ].c_str(), ( 'A' + port ) );
			break;
		case GSErrorNoMedia:
			sprintf( errorText, ksGlobalTextArray[ GT_MC_NO_CARD_GC ].c_str(), ( 'A' + port ) );
			break;
		case GSErrorNotEnoughSpace:
			sprintf( errorText, ksGlobalTextArray[ GT_MC_NOSPACE_GC ].c_str(), ( 'A' + port ), getSavedGameSize() );
			break;
		case GSErrorIncompatible:
			sprintf( errorText, ksGlobalTextArray[ GT_MC_COMPAT_GC ].c_str(), ( 'A' + port ) );
			break;
		case GSErrorWrongRegion:
			sprintf( errorText, ksGlobalTextArray[ GT_MC_REGION_GC ].c_str(), ( 'A' + port ) );
			break;
		case GSErrorDamaged:
			sprintf( errorText, ksGlobalTextArray[ GT_MC_DAMAGED_GC ].c_str(), ( 'A' + port ) );
			break;
			
		default:
			errorText[0] = '\0';
			break;
	}

	return stringx( errorText );
}

stringx GenericGameSaver::getSavingString(int port, int slot, stringx saveWhat)
{
	char error[200];
	sprintf(error, ksGlobalTextArray[GT_MC_SAVING_PS2].c_str(), saveWhat.c_str(),getCardString(port, slot).c_str(),getCardString(port, slot).c_str());
	return stringx(error);
}
stringx GenericGameSaver::getLoadingString(int port, int slot, stringx saveWhat)
{
	char error[200];
	sprintf(error, ksGlobalTextArray[GT_MC_LOADING_PS2].c_str(), saveWhat.c_str(),getCardString(port, slot).c_str(),getCardString(port, slot).c_str());
	return stringx(error);
}
stringx GenericGameSaver::getDeletingString(int port, int slot, stringx saveWhat)
{
	char error[200];
	sprintf(error, ksGlobalTextArray[GT_MC_DELETING_PS2].c_str(), saveWhat.c_str(), getCardString(port, slot).c_str(),getCardString(port, slot).c_str());
	return stringx(error);
}

stringx GenericGameSaver::getNotEnoughRoomString(int port, int slot)
{
	stringx retval(stringx::fmt, ksGlobalTextArray[GT_FE_MENU_NO_SPACE].c_str(), inst()->getCardString(port, slot).c_str(), 
		getSavedGameSize(), ksGlobalTextArray[GT_MC_BLOCKS].c_str());
	retval.to_upper();

	return retval;
}

stringx GenericGameSaver::getInsertCardString(int port, int slot)
{
	return ksGlobalTextArray[GT_FE_MENU_NO_CARD]+" "+getCardString(port, slot);
}

stringx GenericGameSaver::getUnavailableCardString(int port, int slot)
{
	assertmsg(false, "Function unimplemented for this platform.");
	return stringx("");
}

stringx GenericGameSaver::getOverwriteString(int port, int slot)
{
	return ksGlobalTextArray[GT_FE_MENU_OVERWRITE].c_str();
}
stringx GenericGameSaver::getFormattingString(int port, int slot)
{
	char sentance2[255];
	sprintf(sentance2, ksGlobalTextArray[GT_FE_MENU_FORMATTING].c_str(), getCardString(port, slot).c_str(), getCardString(port, slot).c_str() );
	return stringx(sentance2);
}
