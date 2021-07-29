#include "global.h"

#include "bitplane.h"


bitplane::bitplane( unsigned short _width, unsigned short _height )
:   width( _width ),
    height( _height )
  {
  // width must be a multiple of 8
  assert( (width%8) == 0 );
  // allocate buffer
  buf = NEW unsigned char[get_bufsize()];
  }


bitplane::~bitplane()
  {
  delete[] buf;
  }


void bitplane::clear()
  {
  memset( buf, 0, get_bufsize() );
  }


static unsigned char bytemask_left[9] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };


void bitplane::blit( int x, int y, const bitplane& b )
  {
  }


void bitplane::blit_or( int x, int y, const bitplane& b )
  {
  // clip source rectangle to destination limits
  int bx = 0;
  int by = 0;
  if ( x < 0 )
    {
    bx -= x;
    x = 0;
    }
  if ( y < 0 )
    {
    by -= y;
    y = 0;
    }
  int bw = b.get_width();
  int bh = b.get_height();
  if ( x+bw-bx > get_width() )
    bw = get_width() - (x-bx);
  if ( y+bh-by > get_height() )
    bh = get_height() - (y-by);
  if ( bw<=0 || bh<=0 )
    return;

  // binary-or source into destination
  unsigned short sx, sy, dx, dy;
  sy = by;
  dy = y;
  while ( sy < bh )
    {
    sx = bx;
    dx = x;
    while ( sx < bw )
      {
      int sa = sx % 8;
      int da = dx % 8;
      unsigned char v = b.get_8bits(sx,sy) << sa;
      int r = bw - sx;
      if ( r > 8-sa )
        r = 8 - sa;
      *get_bufptr(dx,dy) |= ((v & bytemask_left[r]) >> da);
      if ( da > sa )
        r = 8 - da;
      sx += r;
      dx += r;
      }
    sy++;
    dy++;
    }
  }


void bitplane::blit_and( int x, int y, const bitplane& b )
  {
  }
