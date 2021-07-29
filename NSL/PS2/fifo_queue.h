#ifndef FIFO_QUEUE_HEADER
#define FIFO_QUEUE_HEADER


/*** fifo_queue ***/
template<class T> 
class fifo_queue
{
private: 
  T *queue;
  short queue_max;
  short start;
  short end;
  short count;

public:
  void           init(short size);
  void           clear();
  void           free();
  short          push(const T& index_to_add);
  short          push_front(const T& index_to_add);
  T              pop();
  short          find(const T& index_to_find, short remove_flag);
  void           print();
  int size()      { return count;}
  int remaining() { return queue_max - count; }

  // iterator functions, you supply the iterator variable (short)
  short          iterator_reset( short *it );
  T*             iterate( short *it );
};

#include "fifo_queue.cpp"
#endif