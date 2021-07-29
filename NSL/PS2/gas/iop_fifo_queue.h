#ifndef IOP_FIFO_QUEUE_HEADER
#define IOP_FIFO_QUEUE_HEADER

#ifdef GAS_MODULE
#include <thread.h>
#endif

/*** fifo_queue ***/
typedef struct
{
  // private: (that is, use the functions below, don't access these members directly)
  unsigned short *queue;
  short queue_max;
  short start;
  short end;
  short count;
} fifo_queue;

// public:
void           fifo_queue_init(fifo_queue *fq, short size);
void           fifo_queue_clear(fifo_queue *fq);
void           fifo_queue_free(fifo_queue *fq);
short          fifo_queue_push(fifo_queue *fq, unsigned short index_to_add);
short          fifo_queue_push_front(fifo_queue *fq, unsigned short index_to_add);
unsigned short fifo_queue_pop(fifo_queue *fq);
short          fifo_queue_find(fifo_queue *fq, unsigned short index_to_find, short remove_flag);
void           fifo_queue_print(fifo_queue *fq);
// fq in these macros is of type 'fifo_queue *'
#define        fifo_queue_size(fq)      ((fq)->count)
#define        fifo_queue_remaining(fq) ((fq)->queue_max - (fq)->count)


/*** iop_clock_t ***/
#define iop_clock_t struct SysClock

// prototypes for the macros (what to send them)
// void iop_clock_reset  ( iop_clock_t *the_clock );
// void iop_clock_elapsed( iop_clock_t *the_clock, int *secs, int *usecs );

#define iop_clock_reset GetSystemTime
#define iop_clock_elapsed( the_clock, secs, usecs ) \
  { iop_clock_t new_time; \
    int new_secs, new_usecs; \
    GetSystemTime(&new_time); \
    SysClock2USec(&new_time, &new_secs, &new_usecs); \
    SysClock2USec((the_clock), (secs), (usecs)); \
    *(secs) = new_secs - *(secs); \
    *(usecs) = new_usecs - *(usecs); }

#endif