#include "global.h"

#include "zip_filter.h"

#if defined(ENABLE_ZIPFILTER)

#include "osfile.h"
#include "ostimer.h"
#include "stringx.h"


#define MAX_INPUT_WINDOW_SIZE (128 * 1024 + 1)

#if defined(TARGET_XBOX)
__declspec(align(64))unsigned char zip_filter::input_window[MAX_INPUT_WINDOW_SIZE * 2];
#else
unsigned char zip_filter::input_window[MAX_INPUT_WINDOW_SIZE * 2] __attribute__((aligned (64)));
#endif

unsigned int zip_filter::readSize = 16*1024;

z_stream zip_filter::zip_stream;

void *zip_filter_alloc(void *unused, unsigned int items, unsigned int size)
{
  return malloc( items * size );
}

void zip_filter_free(void *unused, void *addr)
{
  free( addr );
}


#define NEXT_READ in_remaining -= ( this_read = ( ( in_remaining > (int) readSize ) ? readSize : in_remaining ) )
void zip_filter::init_async(os_file *io, void *data, int out_size)
{
  zip_stream.zfree = zip_filter_free;
  zip_stream.zalloc = zip_filter_alloc;
  
  zip_stream.next_out = (unsigned char*) data;
  zip_stream.avail_out = out_size;
  zip_stream.next_in = NULL;

  inflateInit( &zip_stream );
}

int zip_filter::shutdown_async()
{
  inflateEnd( &zip_stream );
  return zip_stream.total_out;
}
void zip_filter::set_read_size(unsigned int size)
{
  assert(size < MAX_INPUT_WINDOW_SIZE);
  readSize = size;

}
unsigned int zip_filter::get_read_size()
{
  return readSize;
}

void  zip_filter::async_filter(os_file *io, int in_remaining)
{
  int this_read = 0;
  unsigned char *input_window_1 = input_window;
  unsigned char *input_window_2 = input_window + ( MAX_INPUT_WINDOW_SIZE );
  int ret_val, actually_read;
  char error_txt[20];
  NEXT_READ;

  actually_read = io->read( input_window_1, this_read, false );
  sprintf(error_txt, "%d", actually_read);
  verify( actually_read == this_read);  

  while(1)
  {
    assert(zip_stream.avail_in == 0);
    zip_stream.next_in = input_window_1;
    zip_stream.avail_in = this_read;
    NEXT_READ;
    if( this_read )
    {
      actually_read = io->read( input_window_2, this_read, true );
      sprintf(error_txt, "%d", actually_read);
      verify(actually_read == this_read);
    }    
    else
    {
#ifdef TARGET_PS2
      sceCdSync(0);
#endif // TARGET_PS2
    }

    ret_val = inflate( &zip_stream, Z_SYNC_FLUSH );
    sprintf(error_txt, "%d", ret_val);
    assert(ret_val == Z_OK || ret_val == Z_STREAM_END);


    if( this_read )
    {
    assert(zip_stream.avail_in == 0);
      zip_stream.next_in = input_window_2;
      zip_stream.avail_in = this_read;
      NEXT_READ;
      if( this_read )
      {
	      
        actually_read = io->read( input_window_1, this_read, true );
        sprintf(error_txt, "%d", actually_read);
        verify(actually_read== this_read);
      }
      else
      {
#ifdef TARGET_PS2
        sceCdSync(0);
#endif // TARGET_PS2
      }

      ret_val = inflate( &zip_stream, Z_SYNC_FLUSH );
      sprintf(error_txt, "%d", ret_val);
      assert(ret_val == Z_OK || ret_val == Z_STREAM_END);


      if( !this_read )
        break;
    }
    else
      break;
  }
}
int zip_filter::filter(os_file *io, int in_remaining, void *data, int out_size)
{
  unsigned char *input_window_1 = input_window;
  unsigned char *input_window_2 = input_window + ( MAX_INPUT_WINDOW_SIZE );
  int ret_val;
  int this_read = 0;
  int actually_read;
  char error_txt[20];
  zip_stream.zfree = zip_filter_free;
  zip_stream.zalloc = zip_filter_alloc;
  
  zip_stream.next_out = (unsigned char*) data;
  zip_stream.avail_out = out_size;
  zip_stream.next_in = NULL;

  inflateInit( &zip_stream );
  
  NEXT_READ;
  actually_read = io->read( input_window_1, this_read, false );
  sprintf(error_txt, "%d", actually_read);
  verify(actually_read == this_read);
  while(1)
  {
    assert(zip_stream.avail_in == 0);
    zip_stream.next_in = input_window_1;
    zip_stream.avail_in = this_read;
    NEXT_READ;
    if( this_read )
    {
      actually_read = io->read( input_window_2, this_read, true );
      sprintf(error_txt, "%d", actually_read);
	    verify(actually_read == this_read);
    }
    else
    {
#ifdef TARGET_PS2
      sceCdSync(0);
#endif // TARGET_PS2
    }
    ret_val = inflate( &zip_stream, Z_SYNC_FLUSH );
    sprintf(error_txt, "%d", ret_val);
    assert(ret_val == Z_OK || ret_val == Z_STREAM_END);

    if( this_read )
    {
    assert(zip_stream.avail_in == 0);
      zip_stream.next_in = input_window_2;
      zip_stream.avail_in = this_read;
      NEXT_READ;
      if( this_read )
      {
        actually_read = io->read( input_window_1, this_read, true );
        sprintf(error_txt, "%d", actually_read);
		    verify(actually_read == this_read);
      } 
      else
      {
#ifdef TARGET_PS2
        sceCdSync(0);
#endif // TARGET_PS2
      }
      ret_val = inflate( &zip_stream, Z_SYNC_FLUSH );
      sprintf(error_txt, "%d", ret_val);
      assert(ret_val == Z_OK || ret_val == Z_STREAM_END);

      if( !this_read )
        break;
    }
    else
      break;
  }

  inflateEnd( &zip_stream );

  return zip_stream.total_out;
}


#endif //ENABLE_ZIPFILTER
