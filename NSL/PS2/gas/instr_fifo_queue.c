#include "instr_fifo_queue.h"
#include <kernel.h>

//===================================================================================
// fifo_instr_queue functions

/*** fifo_instr_init ***/
// serves as an initializer (constructor)
void fifo_instr_init(fifo_instr_queue *fq, short size)
{
  fq->queue = (GasInstruction *)AllocSysMemory(0, size * sizeof(GasInstruction), NULL);
  fq->queue_max = size;

  fifo_instr_clear(fq);
}

/*** fifo_instr_free ***/
// serves as an destructor
void fifo_instr_free(fifo_instr_queue *fq)
{
  if (fq->queue != NULL)
    FreeSysMemory(fq->queue);
  fq->queue = NULL;
  fq->queue_max = 0;

  fifo_instr_clear(fq);
}

/*** fifo_instr_clear ***/
// clears the fifo_instr_queue, also serves as an initializer (constructor)
void fifo_instr_clear(fifo_instr_queue *fq)
{
  int i;
  fq->start = 0;
  fq->end = -1;
  fq->count = 0;
  for (i=0; i<fq->queue_max; ++i)
  {
    memset(&(fq->queue[i]), 0, sizeof(GasInstruction));
  }
}

/*** fifo_instr_push ***/
// pushes a new index into the queue at the end (fifo)
// returns new queue size on success, 0 on error (no room)
short fifo_instr_push(fifo_instr_queue *fq, GasInstruction *index_to_add)
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
  memcpy(&fq->queue[(int)fq->end], index_to_add, sizeof(GasInstruction));
  ++fq->count;
  return fq->count;
}

/*** fifo_instr_push_front ***/
// pushes a new index into the queue at the front (lifo)
// returns new queue size on success, 0 on error (no room)

// GAS MODULE SPECIFIC NOTES:
// should only be used for stuff that absolutely needs to be refreshed asap,
// and can't go to the end of the queue.  Mis-using this function could 
// lead to some bad behaviour because the code that uses this queue
// relies on a generally fifo-like behaviour for good results.
short fifo_instr_push_front(fifo_instr_queue *fq, GasInstruction *index_to_add)
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
  memcpy(&fq->queue[(int)fq->start], index_to_add, sizeof(GasInstruction));
  ++fq->count;
  return fq->count;
}

/*** fifo_instr_pop ***/
// returns the front element in the queue, will return NULL on error
// but don't use that, instead, check size before poping or simply avoid
// poping off an empty queue.

// watch your back, sucka
GasInstruction * fifo_instr_pop(fifo_instr_queue *fq)
{
  GasInstruction * ret = &fq->queue[(int)fq->start];

  if (fq->count == 0)
    return NULL;

  ++fq->start;
  if (fq->start >= fq->queue_max)
  {
    fq->start = 0;
  }
  --fq->count;

  return ret;
}

/*** fifo_instr_find ***/
// returns 1 if the element was found, 0 otherwise, 
// if the remove_flag parameter is zero, the element will not be removed from the queue,
// otherwise it will remove it.
// the removal can mess up the ordering in the queue (it replaces the removed element with
// the element from the end of the queue) if you need to preserve order in the queue,
// do not use this fn to remove an element -- write one to do the proper removal.
short fifo_instr_find(fifo_instr_queue *fq, GasInstruction * index_to_find, short remove_flag)
{
  short curr = fq->start;

  if (fq->count <= 0)
    return 0;
  if (memcmp(&fq->queue[curr], index_to_find, sizeof(GasInstruction)) == 0)
  {
    if (remove_flag)
    {
      fifo_instr_pop(fq);
    }
    return 1;
  }
  while (curr != fq->end)
  {
    ++curr;
    if (curr >= fq->queue_max)
      curr = 0;
    if (memcmp(&fq->queue[curr], index_to_find, sizeof(GasInstruction)) == 0)
    {
      if (remove_flag)
      {
        memcpy(&fq->queue[curr], &fq->queue[fq->end], sizeof(GasInstruction));
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
