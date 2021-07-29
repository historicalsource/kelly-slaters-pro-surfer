#ifndef COLOR_H
#define COLOR_H
////////////////////////////////////////////////////////////////////////////////
/*
  color.h

   a four-byte color structure:  red, green, blue, and alpha

*/
////////////////////////////////////////////////////////////////////////////////
//#include "algebra.h"
#include "chunkfile.h"


////////////////////////////////////////////////////////////////////////////////

// 0.0f-1.0f to 0-255
inline uint8 float_to_byte( float f )
{
  return static_cast<uint8>( f * 255.0f );
}

class color;
class vector4d;

#ifndef __GNUC__
#pragma pack(push,1)
#endif
class color32
{
public:
  union
  {
    struct
    {
      uint8 b,g,r,a;
    } c;
    uint32 i;
  };

  // From DWORD.
  color32( uint32 ic=0 ) : i(ic) {}

  // Members
  color32( uint8 _r, uint8 _g, uint8 _b, uint8 _a=255 )
  {
    c.b = _b;
    c.g = _g;
    c.r = _r;
    c.a = _a;
  }

  uint8 get_red  () const { return c.r; }
  uint8 get_green() const { return c.g; }
  uint8 get_blue () const { return c.b; }
  uint8 get_alpha() const { return c.a; }
  void set_red  ( uint8 arg ) { c.r = arg; }
  void set_green( uint8 arg ) { c.g = arg; }
  void set_blue ( uint8 arg ) { c.b = arg; }
  void set_alpha( uint8 arg ) { c.a = arg; }

  uint32 to_ulong() const {
    return i;
  }

  color32 operator*(const color32& d) const
  {
    color32 retval;
    retval.c.b = c.b * d.c.b / 255;  // because thing max at 255 I wanted *255 = unchanged
    retval.c.g = c.g * d.c.g / 255;  // that's why i didnt use 256
    retval.c.r = c.r * d.c.r / 255;
    retval.c.a = c.a * d.c.a / 255;
    return retval;
  }

  color32 operator*=(const color32& d)
  {
    c.b = c.b * d.c.b / 255;
    c.g = c.g * d.c.g / 255;
    c.r = c.r * d.c.r / 255;
    c.a = c.a * d.c.a / 255;
    return *this;
  }

  // moved this to the .cpp file to avoid pulling in a header
  // and to alleviate a circular reference of
  // vector -> profiler -> color -> vector
  color32 operator*=( const vector4d& rgbaf );

  bool operator==(const color32& d) const
  {
    return i==d.i;
  }

#ifndef TARGET_PS2
  bool operator!=(const color32& d) const
  {
    return i!=d.i;
  }
#endif

  const color32& operator=(const color32& d)
  {
    i = d.i;
    return *this;
  }

  color to_color();
};

// basic colors
// all have alpha of 255
/*
const color32 color32_white;
const color32 color32_black;
const color32 color32_grey;
#define color32_gray color32_grey
const color32 color32_red;
const color32 color32_green;
const color32 color32_blue;
const color32 color32_cyan;
const color32 color32_magenta;
const color32 color32_yellow;
*/
#define color32_white   color32( 255,  255,  255,  255 )
#define color32_black   color32( 0,    0,    0,    255 )
#define color32_grey    color32( 127,  127,  127,  255 )
#define color32_red     color32( 255,  0,    0,    255 )
#define color32_green   color32( 0,    255,  0,    255 )
#define color32_blue    color32( 0,    0,    255,  255 )
#define color32_cyan    color32( 0,    255,  255,  255 )
#define color32_magenta color32( 255,  0,    255,  255 )
#define color32_yellow  color32( 255,  255,  0,    255 )
#define color32_orange  color32( 255,  127,  0,    255 )
#define color32_purple  color32( 127,  0,    127,  255 )
#define color32_gray    color32_grey


#ifndef __GNUC__
#pragma pack(pop)
#endif

#if !defined(NO_SERIAL_IN)
inline void serial_in( chunk_file& io, color32* c )
{
  serial_in( io, (unsigned int*)c );
}
#endif

color32 saturated_add(color32 c1,color32 c2);  //


class color
{
public:
  rational_t r, g, b, a;

  // uninitialized color
  color() {}

  // r,g,b,a color specified as 0.0f to 1.0f inclusive
  color(float _r,float _g,float _b,float _a=1.0F)
    : r( _r ), g( _g ), b( _b ), a( _a )
  {
  }

  color( const color& c ) : r(c.r), g(c.g), b(c.b), a(c.a) {}
  color& operator=( const color& c ) { r = c.r; g = c.g; b = c.b; a = c.a; return *this;}
  bool   operator==( const color& c ) const { return ( r==c.r ) && ( g==c.g ) && ( b==c.b ) && ( a==c.a); }

  float get_red  () const { return r; }
  float get_green() const { return g; }
  float get_blue () const { return b; }
  float get_alpha() const { return a; }

  float length2() const { return sqr(r)+sqr(g)+sqr(b)+sqr(a); } // for brightness, or distance in rgba color space
  float length() const { return __fsqrt(sqr(r)+sqr(g)+sqr(b)+sqr(a)); } // for brightness, or distance in rgba color space

  color32 to_color32() const
  {
    assert(r>=0.0f && r<=1.0f);
    assert(g>=0.0f && g<=1.0f);
    assert(b>=0.0f && b<=1.0f);
    assert(a>=0.0f && a<=1.0f);
    return color32( r * 255, g * 255, b * 255, a * 255 );
  }

  void clamp()
  {
    if (r<0.0f) r=0.0f; else if (r>1.0f) r=1.0f;
    if (g<0.0f) g=0.0f; else if (g>1.0f) g=1.0f;
    if (b<0.0f) b=0.0f; else if (b>1.0f) b=1.0f;
    if (a<0.0f) a=0.0f; else if (a>1.0f) a=1.0f;
  }

  void rescale()  // preserves hue at the expense of brightness
  {
    if (r<0.0f) r=0.0f;
    if (g<0.0f) g=0.0f;
    if (b<0.0f) b=0.0f;
    if (a<0.0f) a=0.0f;
    if (r>1.0f || g>1.0f || b>1.0f || a>1.0f)
    {
      float l = fast_recip_sqrt(length2());
      r *= l; g *= l; b *= l; a *= l;
    }
  }

  friend class hw_rasta;
#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& io, color* c);
#endif

  /*const color& operator=( const color& c )
  {
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
    return *this;
  }*/
  color operator+(const color& c) const
  {
    return color( r + c.r,
                  g + c.g,
                  b + c.b,
                  a + c.a );
  }
  color operator-(const color& c) const
  {
    return color( r - c.r,
                  g - c.g,
                  b - c.b,
                  a - c.a );
  }
  color operator*(float d) const
  {
    return color( r * d,
                  g * d,
                  b * d,
                  a * d );
  }
  color operator*(const color& c) const
  {
    return color( r * c.r,
                  g * c.g,
                  b * c.b,
                  a * c.a );
  }
  color operator/(float d) const
  {
    return *this * (1.0F/d);
  }
  color& operator+=(const color& c)
  {
    r += c.r; 
    g += c.g;
    b += c.b; 
    a += c.a;
    return *this;
  }
  color& operator-=(const color& c)
  {
    r -= c.r; 
    g -= c.g;
    b -= c.b; 
    a -= c.a;
    return *this;
  }
  color& operator*=(float d)
  {
    r *= d;
    g *= d;
    b *= d;
    a *= d;
    return *this;
  }
  color& operator/=(float d)
  {
    return *this *= 1.0F/d;
  }
};

#define color_white   color( 1,  1,  1,  1 )
#define color_black   color( 0,  0,  0,  1 )
#define color_grey    color( 0.5f, 0.5f, 0.5f, 1 )
#define color_red     color( 1,  0,  0,  1)
#define color_green   color( 0,  1,  0,  1)
#define color_blue    color( 0,  0,  1,  1)

// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out(chunk_file& io, const color& c ) 
{ 
  if (io.get_type()==chunk_file::CFT_TEXT)
  {
    io.get_text()->write(ftos(c.get_red  ())+sendl);
    io.get_text()->write(ftos(c.get_green())+sendl);
    io.get_text()->write(ftos(c.get_blue ())+sendl);
    io.get_text()->write(ftos(c.get_alpha())+sendl);
  }
  else
    io.get_binary()->write((void*)&c,sizeof(color));
}
#endif


#if !defined(NO_SERIAL_IN)
inline void serial_in(chunk_file& io, color* c) 
{
  serial_in(io,&c->r);
  serial_in(io,&c->g);
  serial_in(io,&c->b);
  serial_in(io,&c->a);
}
#endif

const chunk_flavor CHUNK_COLOR("color");


// so we can do integer math without capping
class color128
{
public:
  int r;
  int g;
  int b;
  int a;
  color128() {}
  color128(int _r,int _g,int _b,int _a) : r(_r), g(_g), b(_b), a(_a) {}
  color128(const color& c) : r(((int)c.r)*255), g(((int)c.g)*255), b(((int)c.b)*255), a(((int)c.a)*255) {}
  color128(const color32& c) : r(c.c.r), g(c.c.g), b(c.c.b), a(c.c.a) {}
  
  operator color() const { return color(r*(1/255.0f),g*(1/255.0f),b*(1/255.0f),a*(1/255.0f)); }
  operator color32() const 
  { 
    assert(r>=0 && r<=255 && g>=0 && g<=255 &&
           b>=0 && b<=255 && a>=0 && a<=255);
    return color32(r,g,b,a); 
  }

  color128 operator+(const color128& c) const
  {
    return color128( r + c.r,
                     g + c.g,
                     b + c.b,
                     a + c.a );
  }
  color128 operator-(const color128& c) const
  {
    return color128( r - c.r,
                     g - c.g,
                     b - c.b,
                     a - c.a );
  }
  color128 operator*(int d) const
  {
    return color128( r * d,
                     g * d,
                     b * d,
                     a * d );
  }
  color128 operator/(int d) const
  {
    return color128( r / d,
                     g / d,
                     b / d,
                     a / d );
  }
  color128& operator+=(const color128& c)
  {
    r += c.r; 
    g += c.g;
    b += c.b; 
    a += c.a;
    return *this;
  }
  color128& operator-=(const color128& c)
  {
    r -= c.r; 
    g -= c.g;
    b -= c.b; 
    a -= c.a;
    return *this;
  }
};

inline color color32::to_color()  
{
  color retval( c.r/255.0f, c.g/255.0f, c.b/255.0f, c.a/255.0f );
  return retval;
}


#endif
