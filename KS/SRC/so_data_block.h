// so_data_block.h
#ifndef _SO_DATA_BLOCK_H
#define _SO_DATA_BLOCK_H


#include <stddef.h>


class so_data_block
  {
  // Data
  protected:
    int blocksize;
    char* buffer;

  // Constructors
  public:
    so_data_block() : blocksize(0),buffer(NULL) {}
    so_data_block(const so_data_block& b);
    so_data_block(int sz);
    ~so_data_block();

  // Methods
  public:
    void init(int sz);
    void clear();
    int size() const         { return blocksize; }
    char* get_buffer() const { return buffer; }

  // Internal Methods
  protected:
    void _destroy();
    void _init(int sz);
  };


#endif  // _SO_DATA_BLOCK_H
