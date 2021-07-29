// so_data_block.cpp
#include "global.h"

#include "so_data_block.h"
#include "vm_thread.h"
#include <memory>


// CLASS so_data_block

// Constructors

so_data_block::so_data_block(const so_data_block& b)
  {
  _init(b.blocksize);
  memcpy(buffer,b.buffer,blocksize);
  }

so_data_block::so_data_block(int sz)
  {
  _init(sz);
  }

so_data_block::~so_data_block()
  {
    _destroy();
  }


// Methods
void so_data_block::init(int sz)
  {
  _destroy();
  _init(sz);
  }

void so_data_block::clear()
  {
  _destroy();
  blocksize = 0;
  }


// Internal Methods

void so_data_block::_destroy()
  {
  if (buffer)
    delete[] buffer;
  }

void so_data_block::_init(int sz)
  {
  blocksize = sz;
  if (sz)
    {
    buffer = NEW char[sz];
#ifdef _DEBUG
    int stop = sz/4;
    assert (sz == stop*4);
    for (int i=0;i<stop;i++) ((unsigned int *)buffer)[i] = UNINITIALIZED_SCRIPT_PARM;
#endif
    }
  else
    buffer = NULL;
  }
