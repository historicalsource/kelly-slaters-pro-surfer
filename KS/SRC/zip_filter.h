#ifndef ZIPFILTER_H
#define ZIPFILTER_H

#include "global.h"

class os_file;
class stringx;

#if defined(TARGET_PS2) || defined(TARGET_XBOX)
#define ENABLE_ZIPFILTER
#endif

#if defined(ENABLE_ZIPFILTER)
#include "zlib/zlib.h"


class zip_filter
{
  public:
    static int filter(os_file *io, int in_size, void *out_buffer, int out_size);
    static void init_async(os_file *io, void *data, int out_size);
    static int shutdown_async();
    static void async_filter(os_file *io, int in_remaining);
    static void set_read_size(unsigned int size);
    static unsigned int get_read_size();

  private:
    static unsigned char input_window[];

    static z_stream zip_stream;
    static unsigned int readSize;
};            

#else

class zip_filter
{
  public:
    static int filter(os_file *io, int in_size, void *out_buffer, int out_size)
    {
      assert( "zip_filter not supported" && 0 );
      return -1;
    }
    static int shutdown_async()
    {
#ifndef USER_MKV
			debug_print( "faking zip_filter async shit" );
#endif
      return -1;
    }
    static void init_async(os_file *io, void *data, int out_size)
		{
#ifndef USER_MKV
			debug_print( "faking zip_filter async shit" );
#endif
		}
    static void async_filter(os_file *io, int in_remaining)
		{
			assert( "zip_filter not supported" && 0 );
		}
};

#endif // ENABLE_ZIPFILTER

#endif //ZIPFILTER_H
