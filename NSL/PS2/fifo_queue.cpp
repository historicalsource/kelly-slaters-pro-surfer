
#include <malloc.h>
#include <stdio.h>

//===================================================================================
// fifo_queue functions


/*** fifo_queue_init ***/
// serves as an initializer (constructor)
template<class T>
void fifo_queue<T>::init(short size)
{
  queue = (T *)new T[size]; 
  queue_max = size;

  clear();
}

/*** fifo_queue_free ***/
// serves as an destructor
template<class T>
void fifo_queue<T>::free()
{
  if (queue != NULL)
    delete[] queue;

  queue = NULL;
  queue_max = 0;

  clear();
}

/*** fifo_queue_clear ***/
// clears the fifo_queue, also serves as an initializer (constructor)
template<class T>
void fifo_queue<T>::clear()
{
  int i;
  start = 0;
  end = -1;
  count = 0;
  for (i=0; i<queue_max; ++i)
  {
    queue[i] = 0;
  }
}

/*** fifo_queue_push ***/
// pushes a new index into the queue at the end (fifo)
// returns new queue size on success, 0 on error (no room)
template<class T>
short fifo_queue<T>::push(const T& index_to_add)
{
  if (count >= queue_max)
  {
    return 0;
  }
  ++end;
  if (end >= queue_max)
  {
    end = 0;
  }
  queue[(int)end] = index_to_add;
  ++count;
  return count;
}

/*** fifo_queue_push_front ***/
// pushes a new index into the queue at the front (lifo)
// returns new queue size on success, 0 on error (no room)

// GAS MODULE SPECIFIC NOTES:
// should only be used for stuff that absolutely needs to be refreshed asap,
// and can't go to the end of the queue.  Mis-using this function could 
// lead to some bad behaviour because the code that uses this queue
// relies on a generally fifo-like behaviour for good results.
template<class T>
short fifo_queue<T>::push_front(const T& index_to_add)
{
  if (count >= queue_max)
  {
    return 0;
  }
  --start;
  if (start < 0)
  {
    start = queue_max-1;
  }
  queue[(int)start] = index_to_add;
  ++count;
  return count;
}

/*** fifo_queue_pop ***/
// returns the front element in the queue, will return 255 on error (-1)
// but don't use that, instead, check size before poping or simply avoid
// poping off an empty queue.
template<class T>
T fifo_queue<T>::pop()
{
  T ret = queue[(int)start];

  if (count == 0)
    return (T)-1;

  ++start;
  if (start >= queue_max)
  {
    start = 0;
  }
  --count;

  return ret;
}

/*** fifo_queue_find ***/
// returns 1 if the element was found, 0 otherwise, 
// if the remove_flag parameter is zero, the element will not be removed from the queue,
// otherwise it will remove it.
// the removal can mess up the ordering in the queue (it replaces the removed element with
// the element from the end of the queue) if you need to preserve order in the queue,
// do not use this fn to remove an element -- write one to do the proper removal.
template<class T>
short fifo_queue<T>::find(const T& index_to_find, short remove_flag)
{
  short curr = start;

  if (count <= 0)
    return 0;
  if (queue[curr] == index_to_find)
  {
    if (remove_flag)
    {
      pop();
    }
    return 1;
  }
  while (curr != end)
  {
    ++curr;
    if (curr >= queue_max)
      curr = 0;
    if (queue[curr] == index_to_find)
    {
      if (remove_flag)
      {
        queue[curr] = queue[end];
        --count;
        --end;
        if (end < 0)
          end = queue_max - 1;
      }
      return 1;
    }
  }
  return 0;
}

// iterator functions, you supply the iterator variable (short)
template<class T>
short fifo_queue<T>::iterator_reset( short *it )
{
  if (count > 0)
    (*it) = start;
  else
    (*it) = -1;
  return count;
}

// the pointer to element returned, null if we've reached The End, post-increment position
template<class T>
T * fifo_queue<T>::iterate( short *it )
{
  // catch those pesky bad itertators
  T* ret = NULL;
  if ((*it) < 0 || (*it) >= queue_max)
  {
    return ret;
  }

  // fetch their value
  ret = &(queue[(*it)]);

  if ((*it) == end)
  {
    // mark done with iteration
    (*it) = -1;
  }
  else
  {
    // iterate
    ++(*it);
    if ((*it) >= queue_max)
      (*it) = 0;
  }
  return ret;
}

#ifdef UNIT_TEST_FIFO_QUEUE // test main
#include <stdio.h>

int main()
{
  fifo_queue fq;
  int i;
  fifo_queue_init(&fq, 5);

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  fifo_queue_push(&fq, 3);

  gas_printf("Queue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");
  fifo_queue_push(&fq, 5);
  fifo_queue_push(&fq, 4);
  fifo_queue_push(&fq, 1);

  gas_printf("Queue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  fifo_queue_push(&fq, 6);
  gas_printf("Queue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  gas_printf("popped element %d\n", fifo_queue_pop(&fq));
  gas_printf("Queue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  gas_printf("popped element %d\n", fifo_queue_pop(&fq));
  fifo_queue_push(&fq, 9);
  gas_printf("popped element %d\n", fifo_queue_pop(&fq));
  gas_printf("Queue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  fifo_queue_push(&fq, 8);
  fifo_queue_push(&fq, 7);

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  gas_printf("popped element %d\n", fifo_queue_pop(&fq));
  gas_printf("popped element %d\n", fifo_queue_pop(&fq));
  gas_printf("Queue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  fifo_queue_push(&fq, 4);
  fifo_queue_pop(&fq);
  fifo_queue_push(&fq, 6);
  fifo_queue_push(&fq, 2);

  gas_printf("\nQueue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  gas_printf("Finding %d %d %d %d %d %d %d %d %d %d\n", fifo_queue_find(&fq, 0, 0),
    fifo_queue_find(&fq, 1, 0),fifo_queue_find(&fq, 2, 1),fifo_queue_find(&fq, 3, 0),
    fifo_queue_find(&fq, 4, 0),fifo_queue_find(&fq, 5, 0),fifo_queue_find(&fq, 6, 0),
    fifo_queue_find(&fq, 7, 0),fifo_queue_find(&fq, 8, 0),fifo_queue_find(&fq, 9, 0));

  gas_printf("\nQueue contents s(%d) e(%d): ", fq.start, fq.end);
  for (i=0; i<fq.queue_max; ++i)
  {
    gas_printf("%d ", fq.queue[i]);
  }
  gas_printf("\n");

  gas_printf("fq size %d remaining %d\n", fifo_queue_size(&fq), 
    fifo_queue_remaining(&fq));

  fifo_queue_free(&fq);

  return 0;
}

#endif