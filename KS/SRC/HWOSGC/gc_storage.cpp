// gc_storage.cpp  save game functions for GameCube

#include "global.h"

#include "osstorage.h"
//#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <card.h>
#include <texPalette.h>

//get_free_blocks() add sizeof(icon)
//get_last_error()  convert my errors in common errors 

static bool storage_init_called = false;   

DEFINE_SINGLETON(storage_mgr)
static storage_unit* g_callback_caller[2]= {NULL, NULL};

//---------------------------------------------------------------
void mc_attach_callback( s32 chan, s32 result )
{
	debug_print("mc_callback slot%i result=%i", chan, result);

	if( g_callback_caller[chan] )
  {
    //mounted= 1;
  	g_callback_caller[chan]->unmount_card();
  }
}

//---------------------------------------------------------------
s32 storage_unit::get_last_error()
{ 
  s32   error;

	do
  {
    error= CARDProbeEx(slot, &card_size, &sector_size);
  }
  while( error == CARD_RESULT_BUSY );
  if( error != CARD_RESULT_READY )
    last_error= error;

 	card_size= (card_size*1024*1024) / 8; //in megabits->bytes

  switch(last_error)
  {
    case CARD_RESULT_READY:
      return STORAGE_OK;

    case CARD_RESULT_NOCARD: //(no object is inserted) 
    case CARD_RESULT_WRONGDEVICE:// (found a non memory card device)
      return STORAGE_NO_MEDIA;

    case CARD_RESULT_BUSY:// (try later) 
      return STORAGE_BUSY;

    case CARD_RESULT_IOERROR://(need to be formatted?)
      return STORAGE_GENERIC;

    case CARD_RESULT_ENCODING://(need to be formatted) bad japanise format
      return STORAGE_UNFORMATTED;

    case CARD_RESULT_BROKEN://(need to be formatted) we tried to repare it before w/o luck.
      return STORAGE_UNFORMATTED;

    case CARD_RESULT_NOFILE:
      return STORAGE_NO_OBJECT_ACCESS;
    default:
      return STORAGE_GENERIC;

   
  }

  return STORAGE_OK;
}

//---------------------------------------------------------------
bool storage_unit::unmount_card()
{
  if(!mounted)
    return false;

  if(mounted==1)//do actual unmount
	  while( (last_error= CARDUnmount(slot)) == CARD_RESULT_BUSY ) continue;

    g_callback_caller[slot]= NULL;

	  mounted--;
    if(mounted<0)
      mounted= 0;
	  return (last_error==CARD_RESULT_READY);
}

//---------------------------------------------------------------
u8 mc_card_workarea [2][CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

bool storage_unit::mount_card()
{
  if(mounted)
    goto exit_mount;

	while( (last_error= CARDProbeEx(slot, &card_size, &sector_size)) == CARD_RESULT_BUSY ) continue;

	if( last_error != CARD_RESULT_READY )
		return(false);

	card_size= (card_size*1024*1024) / 8; //in megabits->bytes
	
  g_callback_caller[slot]= this;

	while( (last_error= CARDMount(slot, (void*)mc_card_workarea[slot], mc_attach_callback ))== CARD_RESULT_BUSY ) continue;

	switch( last_error )
	{ //this means mounted
		case CARD_RESULT_BROKEN:
		case CARD_RESULT_ENCODING:
		case CARD_RESULT_READY:
      break;

		default:
			return false;
	}
	//try to solve broken && encoding
	while( (last_error= CARDCheck(slot)) == CARD_RESULT_BUSY ) ;
 
	if( last_error != CARD_RESULT_READY )
		return(false);

exit_mount:
  mounted++;		
	return true;
}

/*
bool storage_unit::is_mounted()
{
  if(mounted) return true;
  return false;
}
*/

//---------------------------------------------------------------
s32 storage_unit::find_file(const char *file_name, CARDStat *f_stat)
{
  CARDStat 	stat;
  s32 			result;
  
  if(!mount_card())
  { 
    result= -1;
    goto exit_find;
  }

  for (int file_no = 0; file_no < CARD_MAX_FILE; file_no++)
  {
  	result = CARDGetStatus(slot, file_no, &stat);
	  if( result == CARD_RESULT_NOFILE )
	  {
	  	continue;
	  }
	  if( result != CARD_RESULT_READY)
	  {
	  	last_error= result;
      result= -1;
	  	goto exit_find;
	  }
	  if( !strcmp( stat.fileName, file_name ) )
	  {
	  	if(f_stat)
	  		memcpy(f_stat, &stat, sizeof(stat));
	  	result= file_no;
      goto exit_find;
	  }
  }

  unmount_card();
exit_find:
  return result;
}
//---------------------------------------------------------------
storage_unit::storage_unit(const char* apath)
  : path(apath)
{
	slot=         0;
	sector_size=  0;
  last_error=   CARD_RESULT_READY;
  mounted=      0;
 /* 
	if( get_last_error() != STORAGE_OK )
		return(false);*/
}
//---------------------------------------------------------------
storage_unit:: storage_unit(const int  n_slot)
{
	slot=         n_slot;
	sector_size=  0;
  last_error=   CARD_RESULT_READY;
  mounted=      0;
  /*
  mount_card();
  unmount_card();
  */
  /*
	if( get_last_error() != STORAGE_OK ) 
		return(false);*/
  
}
//---------------------------------------------------------------
storage_unit::~storage_unit() 
{
	unmount_card();
}

//---------------------------------------------------------------
uint32 storage_unit::get_block_size()
{
  if( get_last_error() != STORAGE_OK )
    return 0;
  else
    return sector_size;
}

//---------------------------------------------------------------
uint32 storage_unit::get_total_blocks() 
{
  u32 result= 0;
  if( get_last_error() != STORAGE_OK )
    return 0;

  if(sector_size)
    result= card_size/sector_size;

  return (result);
}

//---------------------------------------------------------------
uint32 storage_unit::get_free_blocks() 
{
  u32   result= 0, ico_size= 0;
  //is locked
  #ifdef USE_FILE_MANAGER
  	file_manager *fileman = file_manager::inst();
	  bool filelock = fileman->is_file_system_locked();
	  if ( filelock )
  	  fileman->unlock_file_system();
	#endif    
  
	os_file   file( stringx(ICON_NAME), os_file::FILE_READ);
	
	if( file.is_open() ) 
	{
  	ico_size= file.get_size();
	  file.close();
	}
	//is locked
  #ifdef USE_FILE_MANAGER
  if ( filelock )
    fileman->lock_file_system();
	#endif

  if(!mount_card())
    goto exit_get_free;
    
  s32       bytes_in_files= 0;
  CARDStat  stat;
  
  for (int file_no = 0; file_no < CARD_MAX_FILE; file_no++)
  {
  	while( (last_error = CARDGetStatus(slot, file_no, &stat)) == CARD_RESULT_BUSY ) continue;
  	
	  if( last_error == CARD_RESULT_NOFILE )
	  	continue;

	  if( last_error != CARD_RESULT_READY)
      goto exit_get_free;
	  	
	  bytes_in_files+= stat.length;
  }
  
  result= (card_size - bytes_in_files - ico_size)/sector_size;

  unmount_card();
exit_get_free:
  return result;

}


//---------------------------------------------------------------
bool storage_unit::file_exists(const char* filename)
{
  return ( find_file(filename)>=0 );
}

//---------------------------------------------------------------
char* mc_icon_load(char *f_name, s32 *tpl_size)
{
  char*   res= NULL;
  
  #ifdef USE_FILE_MANAGER
  file_manager *fileman = file_manager::inst();
  bool filelock = fileman->is_file_system_locked();
  if ( filelock )
    fileman->unlock_file_system();
	#endif
	
	os_file   file( stringx(f_name), os_file::FILE_READ);
	
	if(! file.is_open() )
    goto mc_icon_load_exit;
		
	*tpl_size= file.get_size();
	
	char *tpl_buf;
	res= tpl_buf= (char*) malloc( *tpl_size );
	
	if(!tpl_buf)
		goto mc_icon_load_exit;
		
    
 	s32	bytes_read= file.read((void*) tpl_buf, *tpl_size);
 	
 	file.close();

 	if( bytes_read != *tpl_size )
 	{
 		free(tpl_buf);
 		tpl_buf= res= NULL;
 		goto mc_icon_load_exit;
 	}
 	
mc_icon_load_exit: 	

  #ifdef USE_FILE_MANAGER
  if ( filelock ) 	
 	  fileman->lock_file_system();
 	#endif
 	
	return res;
}

//---------------------------------------------------------------
bool mc_comment_init(CARDStat *f_stat, char *buf)
{
	char 			cm[COMMENT_SIZE];
	memset(cm, 0, sizeof(cm));
	
	strncpy( cm, COMMENT_1, COMMENT_SIZE/2 - 1);
	strncpy( &cm[COMMENT_SIZE/2], COMMENT_2, COMMENT_SIZE/2 - 1);
	
	f_stat->commentAddr= 0;
	memcpy(buf, cm, COMMENT_SIZE);
	return true;
}		

#define normalize_ptr(start, x) { if((x)!=NULL)  ((char*)x)= (char*) ( ((u32)(x)) + (char*)(start) ); }
//---------------------------------------------------------------
char* mc_icon_init(CARDStat *f_stat, char* tpl_buf)
{
	s32 				i, j;
	bool				banner= false;
	s32					icon_num;
	char				*first_data= NULL;
	TEXHeader		*icon_tex;
	CLUTHeader	*icon_clut;
	
	///clean_up
	f_stat->iconFormat= 	0; //clean up format for all icons
	f_stat->bannerFormat=	0;
	f_stat->iconAddr=			0;
	f_stat->iconSpeed=		0;
	f_stat->commentAddr=  0;
	//
	TEXPalette	*buf;
	buf= (TEXPalette*)tpl_buf;
	icon_num= buf->numDescriptors;
	normalize_ptr(buf, buf->descriptorArray );
	TEXDescriptor* 	des_arr= buf->descriptorArray;
	CARDSetIconAddress(f_stat, COMMENT_SIZE );
	
	for(i=0, j=0; i<icon_num; i++)
	{
		//normalize_ptr( buf, des_arr[i]);
		icon_tex= 	des_arr[i].textureHeader;
		icon_clut= 	des_arr[i].CLUTHeader;
		normalize_ptr( buf, icon_tex);
		normalize_ptr( buf, icon_clut);
		if( !first_data )
		{
			normalize_ptr(buf, icon_tex->data);
			first_data= icon_tex->data;
		}
		//baner
		if( icon_tex->height==BANER_HSIZE  && icon_tex->width==BANER_WSIZE )
		{
			if(icon_clut)
				CARDSetBannerFormat(f_stat, CARD_STAT_BANNER_C8 );
			else
				CARDSetBannerFormat(f_stat, CARD_STAT_BANNER_RGB5A3 );
			banner= true;
		}
		//icon 
		if( icon_tex->height==ICON_SIZE  && icon_tex->width==ICON_SIZE )
		{
			if(icon_clut)
				CARDSetIconFormat(f_stat, j, CARD_STAT_ICON_C8);
			else
				CARDSetIconFormat(f_stat, j, CARD_STAT_ICON_RGB5A3);
			CARDSetIconSpeed(f_stat, j, ICON_SPEED);
			j++;
		}
	}
	
	if(!banner)
		CARDSetBannerFormat(f_stat, CARD_STAT_BANNER_NONE );

	mc_comment_init(f_stat, first_data - COMMENT_SIZE);
	return (first_data - COMMENT_SIZE);
}

//---------------------------------------------------------------
void mc_copy_stats(CARDStat *dst, CARDStat *src)
{ 
	//clean up format for all icons
	dst->iconFormat= 		src->iconFormat;
	dst->bannerFormat=	src->bannerFormat;
	dst->iconAddr=			src->iconAddr;
	dst->iconSpeed=			src->iconSpeed;
	dst->commentAddr=		src->commentAddr;
}

//---------------------------------------------------------------
u32 mc_get_control_sum(char *data, s32 size)
{
	u32 	*p=(u32*)data,
				res= 0, i;
	for(i=0; i<size/4; i++, p++)
		res+= *p;
	return res;
}

//---------------------------------------------------------------
bool storage_unit::save_file(const char *f_name,
                  const void *data, uint32 data_size,
                  const char* shortdesc, const char* longdesc,
                  const storage_icon* icon,
                  storage_progress_callback* callback,  // for progress bar
                  storage_time *time_saved,
                  bool do_beep, bool do_lcd)
{	
	CARDStat			f_stat, temp_stat;
	CARDFileInfo	file_info;
	char          file_name[CARD_FILENAME_MAX];
	
	memset(&temp_stat, 0, sizeof(temp_stat));
	//mount & check card
  if(!mount_card())
     return false;    
  strncpy(file_name, f_name, CARD_FILENAME_MAX-1);
  file_name[ CARD_FILENAME_MAX-1 ]= 0;
				
	s32 file_no= find_file( file_name );
	//if file found delete it 
	if( file_no >= 0)
	{	
  	while ( (last_error= CARDFastDelete(slot, file_no)) == CARD_RESULT_BUSY ) continue;
		if(last_error<0)
      goto exit_save_file;
	}
	//get size of save buffer aligned to 8k; 64bytes for comment is in tpl file
	s32		tpl_size;
	char 	*tpl_file= mc_icon_load(ICON_NAME, &tpl_size);
	assert(tpl_file);
	char 	*tpl_data= mc_icon_init(&temp_stat, tpl_file);
	tpl_size= tpl_size- (tpl_data - tpl_file);
	
	u32 	aligned_size=( data_size + tpl_size );
	aligned_size= (aligned_size/sector_size + ((aligned_size%sector_size)!=0) ) * sector_size;
	
	while( (last_error= CARDCreate(slot, file_name, aligned_size, &file_info)) == CARD_RESULT_BUSY ) ;
	if(last_error!=CARD_RESULT_READY)
    goto exit_save_file;
		
	file_no= file_info.fileNo;
	
	while ( (last_error= CARDGetStatus(slot, file_no, &f_stat)) == CARD_RESULT_BUSY ) continue;
  if(last_error!=CARD_RESULT_READY)
    goto exit_save_file;
	
	mc_copy_stats(&f_stat, &temp_stat);
	
	char *save_buf= (char*)malloc(aligned_size);
	memset( save_buf, 0, aligned_size);
	//copy actual data
	memcpy( save_buf, tpl_data, tpl_size );
	memcpy( &save_buf[tpl_size], data, data_size );
	//save control summ
	*((u32*)( &save_buf[aligned_size - sizeof(u32)] ))= mc_get_control_sum( save_buf, aligned_size - sizeof(u32) );
	free( tpl_file );	

	while ( (last_error= CARDWrite(&file_info, save_buf, aligned_size, 0)==CARD_RESULT_BUSY ) ) continue;
 	free( save_buf );
  if( last_error != CARD_RESULT_READY)
     goto exit_save_file;
 	  
	while ( (last_error= CARDSetStatus(slot, file_no, &f_stat)) == CARD_RESULT_BUSY ) continue;
	if( last_error!=CARD_RESULT_READY )
    goto exit_save_file;
		
	last_error= CARDClose(&file_info);

exit_save_file:
  unmount_card();

  if(last_error!=CARD_RESULT_READY)
	  return false;
  else
    return true;
}
//---------------------------------------------------------------
// returns file size if successful, or 0 if error
// call with NULL for data to retrieve max file size, 
// then alloc buffer and call again
//---------------------------------------------------------------
uint32   storage_unit::load_file(const char *file_name, void *data, uint32 data_size,
                         storage_progress_callback* callback,
                         bool do_beep, bool do_lcd) 
{
	CARDFileInfo	file_info;
  u32           res= 0;
	
  if(!mount_card())
    return res;
    
	CARDStat 	stat;
	s32 file_no= find_file( file_name, &stat );
	
	if( file_no <0 )
		goto exit_load_file;
		
	if(data==NULL)
	  return( stat.length - stat.offsetData );
	
 	while( (last_error= CARDFastOpen(slot, file_no, &file_info)) == CARD_RESULT_BUSY ) ;
 	if( last_error<0 )
 		goto exit_load_file;
 		
 	char *read_buf= (char*) malloc( stat.length );
 	
 	while( (last_error= CARDRead(&file_info, read_buf, stat.length, 0)) == CARD_RESULT_BUSY) continue;
 	if( last_error<0 )
 	{
 		free(read_buf);
 		goto exit_load_file;
 	}

	if( *((u32*)( &read_buf[stat.length - sizeof(u32)] )) != mc_get_control_sum(read_buf, stat.length-sizeof(u32)) )
	{
 		free(read_buf);
	  last_error= CARD_RESULT_IOERROR;//control summ is wrong
	  goto exit_load_file;
	}
	
 	memcpy(data, &read_buf[stat.offsetData], data_size);
 	free(read_buf);
	last_error= CARDClose(&file_info);
	if( last_error<0 )
	  goto exit_load_file;

  res= data_size;

exit_load_file:
	unmount_card();
	return(res);
}

bool storage_unit::format()
{
  //i dont know should it be mounted before format 
  last_error= CARDFormat(slot);
  return last_error>=0;
}

//---------------------------------------------------------------
bool storage_unit::get_file_name_list(vector<stringx> * namelist, vector<storage_time> * timelist, char *filter)
{
  return false;
}
//---------------------------------------------------------------
bool storage_unit::get_file_info(vector<stringx> * namelist, vector<void*> * datalist,
                                 vector<storage_time> * timelist, char *filter,
                                 bool do_beep, bool do_lcd )
{
  return false;
}
//---------------------------------------------------------------
bool storage_unit::get_file_time( const char *filename, storage_time *time )
{
  return false;
}

storage_unit_changed_callback* storage_unit_changed_callback::callback_list;

storage_unit_changed_callback::storage_unit_changed_callback()
{
}

storage_unit_changed_callback::~storage_unit_changed_callback()
{
}

void storage_unit_changed_callback::notify_storage_unit_changed(uint32 drive)
{
}
//---------------------------------------------------------------
storage_mgr::storage_mgr()
{
  assert(!storage_init_called); //  card already initialized
  //debug_print("Initializing backup subsystem");

  storage_init_called=true;
}
//---------------------------------------------------------------
storage_mgr::~storage_mgr() 
{
  assert(storage_init_called);
  debug_print("Shutting down backup subsystem");
  storage_init_called = false;
}

//---------------------------------------------------------------
uint32 storage_mgr::get_max_units() 
{
  return 2;
}

//---------------------------------------------------------------
storage_unit* storage_mgr::get_unit(uint32 slot) 
{ 
  if(slot >= get_max_units()) //why dont use it outside
    return NULL;
    
  storage_unit *res= new storage_unit(slot);
  if(res && (res->get_last_error()<0) )
  {
    delete res;
    res= NULL;
  }
  return res;
}

//---------------------------------------------------------------
void storage_mgr::release_unit( storage_unit *unit ) 
{
  if(unit)
    delete unit;
}

//---------------------------------------------------------------
storage_unit* storage_mgr::get_first_valid_unit() 
{
  storage_unit *res= NULL;
  for(s32 i=0; i<get_max_units(); i++)
    if( (res= get_unit(i)) )
      return res;
      
  return 0;
}

//---------------------------------------------------------------
storage_unit* storage_mgr::get_first_valid_unit_with_free_bytes(uint32 bytes) 
{
  return 0;
}

//---------------------------------------------------------------
bool storage_mgr::is_operation_in_progress() const 
{
  return false;
}

