#ifndef _BITPLANE_H
#define _BITPLANE_H


class bitplane
  {
  // Data
  private:
    unsigned short width;   // must be a multiple of 8
    unsigned short height;
    unsigned char* buf;

  // Methods
  public:
    bitplane( unsigned short _width, unsigned short _height );
    ~bitplane();

    unsigned short get_width() const { return width; }
    unsigned short get_height() const { return height; }

    void clear();

    unsigned char get_8bits( unsigned short x, unsigned short y ) const { return *get_bufptr(x,y); }

    bool is_set( unsigned short x, unsigned short y ) const { return get_8bits(x,y) & (1<<(7-(x%8))); }
    void set( unsigned short x, unsigned short y ) { *get_bufptr(x,y) |= (1<<(7-(x%8))); }
    void clear( unsigned short x, unsigned short y ) { *get_bufptr(x,y) &= ~(1<<(7-(x%8))); }

    void blit( int x, int y, const bitplane& b );
    void blit_or( int x, int y, const bitplane& b );
    void blit_and( int x, int y, const bitplane& b );

  private:
    unsigned int get_bufsize() const { return get_width()/8*get_height(); }

    unsigned char* get_bufptr( unsigned short x, unsigned short y ) const
      {
      return buf + (y*width+x)/8;
      }
  };


#endif  // _BITPLANE_H
