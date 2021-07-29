#include "iop_fifo_queue.h"
#ifdef GAS_MODULE
#include <kernel.h>
#else
#include <malloc.h>
#include <stdio.h>
#endif

//===================================================================================
// fifo_queue functions

/*** fifo_queue_init ***/
// serves as an initializer (constructor)
void fifo_queue_init(fifo_queue *fq, short size)
{
#ifdef GAS_MODULE
  fq->queue = (unsigned short *)AllocSysMemory(0, size * sizeof(unsigned short), NULL);
#else
  fq->queue = (unsigned short *)malloc(size * sizeof(unsigned short));
#endif
  fq->queue_max = size;

  fifo_queue_clear(fq);
}

/*** fifo_queue_free ***/
// serves as an destructor
void fifo_queue_free(fifo_queue *fq)
{
  if (fq->queue != NULL)
#ifdef GAS_MODULE
    FreeSysMemory(fq->queue);
#else
    free(fq->queue);
#endif
  fq->queue = NULL;
  fq->queue_max = 0;

  fifo_queue_clear(fq);
}

/*** fifo_queue_clear ***/
// clears the fifo_queue, also serves as an initializer (constructor)
void fifo_queue_clear(fifo_queue *fq)
{
  int i;
  fq->start = 0;
  fq->end = -1;
  fq->count = 0;
  for (i=0; i<fq->queue_max; ++i)
  {
    fq->queue[i] = 0;
  }
}

/*** fifo_queue_push ***/
// pushes a new index into the queue at the end (fifo)
// returns new queue size on success, 0 on error (no room)
short fifo_queue_push(fifo_queue *fq, unsigned short index_to_add)
{
  if (fq->count >= fq->queue_max)
  {
    return 0;
  }
  ++fq->end;
  if (fq->end >= fq->queue_max)
  {
    fq->end = 0;
  }
  fq->queue[(int)fq->end] = index_to_add;
  ++fq->count;
  return fq->count;
}

/*** fifo_queue_push_front ***/
// pushes a new index into the queue at the front (lifo)
// returns new queue size on success, 0 on error (no room)

// GAS MODULE SPECIFIC NOTES:
// should only be used for stuff that absolutely needs to be refreshed asap,
// and can't go to the end of the queue.  Mis-using this function could 
// lead to some bad behaviour because the code that uses this queue
// relies on a generally fifo-like behaviour for good results.
short fifo_queue_push_front(fifo_queue *fq, unsigned short index_to_add)
{
  if (fq->count >= fq->queue_max)
  {
    return 0;
  }
  --fq->start;
  if (fq->start < 0)
  {
    fq->start = fq->queue_max-1;
  }
  fq->queue[(int)fq->start] = index_to_add;
  ++fq->count;
  return fq->count;
}

/*** fifo_queue_pop ***/
// returns the front element in the queue, will return 255 on error (-1)
// but don't use that, instead, check size before poping or simply avoid
// poping off an empty queue.
unsigned short fifo_queue_pop(fifo_queue *fq)
{
  unsigned short ret = fq->queue[(int)fq->start];

  if (fq->count == 0)
    return (unsigned short)-1;

  ++fq->start;
  if (fq->start >= fq->queue_max)
  {
    fq->start = 0;
  }
  --fq->count;

  return ret;
}

/*** fifo_queue_find ***/
// returns 1 if the element was found, 0 otherwise, 
// if the remove_flag parameter is zero, the element will not be removed from the queue,
// otherwise it will remove it.
// the removal can mess up the ordering in the queue (it replaces the removed element with
// the element from the end of the queue) if you need to preserve order in the queue,
// do not use this fn to remove an element -- write one to do the proper removal.
short fifo_queue_find(fifo_queue *fq, unsigned short index_to_find, short remove_flag)
{
  short curr = fq->start;

  if (fq->count <= 0)
    return 0;
  if (fq->queue[curr] == index_to_find)
  {
    if (remove_flag)
    {
      fifo_queue_pop(fq);
    }
    return 1;
  }
  while (curr != fq->end)
  {
    ++curr;
    if (curr >= fq->queue_max)
      curr = 0;
    if (fq->queue[curr] == index_to_find)
    {
      if (remove_flag)
      {
        fq->queue[curr] = fq->queue[fq->end];
        --fq->count;
        --fq->end;
        if (fq->end < 0)
          fq->end = fq->queue_max - 1;
      }
      return 1;
    }
  }
  return 0;
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