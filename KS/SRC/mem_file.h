#ifndef MEM_FILE_H
#define MEM_FILE_H
/*-----------------------------------------------------------------
  MEM_FILE.H - class that emulates an os_file, but it works on a
  plain pointer to an arbitrary block of memory.
  Same api for reads/writes.  Initialize a lot differently.
  Can get a pointer to the current file pointer memory.
  Be careful... if you pass in read-only memory, don't try to write
  to it!  Use flags appropriate to the memory.
-------------------------------------------------------------------*/
//#include "stringx.h"

union multiptr
{
  unsigned char* c;
  unsigned short* s;
  unsigned int* i;
  float* f;
  void** p;
};

#if !defined(_M_IX86)
  #define FAULT_ON_UNALIGNED_ACCESS
#endif

class mem_file
{
  private:
    multiptr p, // current file pointer
             s, // pointer to start of file
             e; // pointer to end of file
    // common to all os_file implementations
    int   flags;

  public:
    enum mode_flags
    {
      FILE_READ   = 1,
      FILE_WRITE  = 2,
      FILE_MODIFY = 3,    // opens the file for writing, w/o deleting existing contents. (how to set NEW EOF? huh? --Sean)
    };

    // ctors & dtor.
    mem_file()
    {
      p.c=s.c=e.c=NULL;
      flags=0;
    }
    mem_file(const void* _data, int _size, int _flags=FILE_READ)
    {
      open(const_cast<void*>(_data),_size,_flags);
    }
    mem_file(void* _data, int _size, int _flags)
    {
      open(_data,_size,_flags);
    }
    ~mem_file()
    {
      //close(); // nothing to do
    }

    void open(void* _data, int _size, int _flags)
    {
      assert(_data && _size && _flags);
      p.c=s.c=(unsigned char*)_data;
      e.c=s.c+_size;
      flags=_flags;
    }
    void close()
    {
      p.c=s.c=e.c=NULL;
      flags=0;
    }

    // read/write return number of bytes read/written.
    int read(void * data, int bytes)
    {
      assert(flags&FILE_READ);
      int bytesleft=e.c-p.c;
      if (bytesleft<0)
        bytesleft=0;
      if (bytesleft<bytes)
        bytes=bytesleft;
      memcpy(data,p.c,bytes);
      p.c+=bytes;
      return bytes;
    }
    int write(const void * data, int bytes)
    {
      assert(flags&FILE_WRITE);
      int bytesleft=e.c-p.c;
      if (bytesleft<0)
        bytesleft=0;
      if (bytesleft<bytes)
        bytes=bytesleft;
      memcpy(p.c,data,bytes);
      p.c+=bytes;
      return bytes;
    }

    unsigned char read_char()
    {
      assert(p.c<e.c);
      return *p.c++;
    }
    void write_char(unsigned char c)
    {
      assert(p.c<e.c);
      *p.c++ = c;
    }

    // Be careful, on some processors, accessing unaligned data
    // can produce faults!  Check alignment.

    unsigned short read_short()
    {
      assert(p.c<=e.c-sizeof(unsigned short));
      #ifdef FAULT_ON_UNALIGNED_ACCESS
      if ((reinterpret_cast<uint32>(p.c)&(sizeof(unsigned short)-1))!=0)
      {
        unsigned short temp=p.c[0]|(p.c[1]<<8);
        ++p.s;
        return temp;
      }
      #endif
      return *p.s++;
    }
    void write_short(unsigned short s)
    {
      assert(p.c<=e.c-sizeof(unsigned short));
      #ifdef FAULT_ON_UNALIGNED_ACCESS
      if ((reinterpret_cast<uint32>(p.c)&(sizeof(unsigned short)-1))!=0)
      {
        p.c[0]=s;
        p.c[1]=s>>8;
        ++p.s;
        return;
      }
      #endif
      *p.s++ = s;
    }

    unsigned int read_long()
    {
      assert(p.c<=e.c-sizeof(unsigned int));
      #ifdef FAULT_ON_UNALIGNED_ACCESS
      if ((reinterpret_cast<uint32>(p.c)&(sizeof(unsigned int)-1))!=0) {
        unsigned short temp=p.c[0]|(p.c[1]<<8)|(p.c[2]<<16)|(p.c[3]<<24);
        ++p.i;
        return temp;
      }
      #endif
      return *p.i++;
    }
    void write_long(unsigned int i)
    {
      assert(p.c<=e.c-sizeof(unsigned int));
      #ifdef FAULT_ON_UNALIGNED_ACCESS
      if ((reinterpret_cast<uint32>(p.c)&(sizeof(unsigned int)-1))!=0)
      {
        p.c[0]=i;
        p.c[1]=i>>8;
        p.c[2]=i>>16;
        p.c[3]=i>>24;
        ++p.i;
        return;
      }
      #endif
      *p.i++ = i;
    }

    unsigned int read_float()
    {
      assert(p.c<=e.c-sizeof(float));
      #ifdef FAULT_ON_UNALIGNED_ACCESS
      if ((reinterpret_cast<uint32>(p.c)&(sizeof(float)-1))!=0)
      {
        unsigned short temp=p.c[0]|(p.c[1]<<8)|(p.c[2]<<16)|(p.c[3]<<24);
        ++p.i;
        return temp;
      }
      #endif
      return *p.f++;
    }
    void write_float(float f)
    {
      assert(p.c<=e.c-sizeof(float));
      #ifdef FAULT_ON_UNALIGNED_ACCESS
      if ((reinterpret_cast<uint32>(p.c)&(sizeof(float)-1))!=0)
      {
        unsigned int i=*(unsigned int*)&f;
        p.c[0]=i;
        p.c[1]=i>>8;
        p.c[2]=i>>16;
        p.c[3]=i>>24;
        ++p.f;
        return;
      }
      #endif
      *p.f++ = f;
    }

    // returns file size
    int size() const
    {
      return e.c-s.c;
    }

    // returns bytes remaining in the file (after current position)
    int avail() const
    {
      return e.c-p.c;
    }

    // returns ptr to beginning of file
    inline const void* get_data() const { return s.c; }
    inline       void* get_data()       { return s.c; }

    // returns ptr to current offset in file
    inline const void* get_fileptr() const { return p.c; }
    inline       void* get_fileptr()       { return p.c; }

    bool acquire_data_ptr(const void*& data, int& size)
    {
      data = p.c; size=e.c-s.c;
      return data!=0;
    }
    bool release_data_ptr(const void* data)
    {
      return true;
    }

    enum filepos_t
    {
      FP_BEGIN,
      FP_CURRENT,
      FP_END
    };
    // set file pointer
    void set_fp( unsigned int pos, filepos_t base )
    {
      switch (base)
      {
        case FP_BEGIN:
          assert(pos<=e.c-s.c);
          p.c = s.c+pos;
          break;
        case FP_CURRENT:
          assert(pos<=e.c-p.c);
          p.c += pos;
          break;
        case FP_END:
          assert(pos<=e.c-s.c);
          p.c = e.c-pos;
          break;
      };
    }
    unsigned int get_fp() // relative to beginning
    {
      return p.c-s.c;
    }

    inline bool is_open() const { return p.c!=NULL; } // returns true after a successful open call.
    inline bool at_eof() const { return p.c==e.c; } // check this after a read operation.

    bool operator!() const { return p.c!=NULL; }
};


#undef FAULT_ON_UNALIGNED_ACCESS


#endif // MEM_FILE_H
