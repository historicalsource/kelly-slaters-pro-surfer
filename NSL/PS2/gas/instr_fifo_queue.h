#ifndef INSTRUCTION_FIFO_QUEUE_HEADER
#define INSTRUCTION_FIFO_QUEUE_HEADER

typedef struct
{
  short rpc_command;
  short short_arg;
  int   int_arg;
} GasInstruction;


/*** fifo_queue ***/
typedef struct
{
  // private: (that is, use the functions below, don't access these members directly)
  GasInstruction *queue;
  short queue_max;
  short start;
  short end;
  short count;
} fifo_instr_queue;

// public:
void            fifo_instr_init(fifo_instr_queue *fq, short size);
void            fifo_instr_clear(fifo_instr_queue *fq);
void            fifo_instr_free(fifo_instr_queue *fq);
short           fifo_instr_push(fifo_instr_queue *fq, GasInstruction *index_to_add);
short           fifo_instr_push_front(fifo_instr_queue *fq, GasInstruction *index_to_add);
GasInstruction  *fifo_instr_pop(fifo_instr_queue *fq);
short           fifo_instr_find(fifo_instr_queue *fq, GasInstruction *index_to_find, short remove_flag);
void            fifo_instr_print(fifo_instr_queue *fq);
// fq in these macros is of type 'fifo_instr_queue *'
#define         fifo_instr_size(fq)      ((fq)->count)
#define         fifo_instr_remaining(fq) ((fq)->queue_max - (fq)->count)

#endif