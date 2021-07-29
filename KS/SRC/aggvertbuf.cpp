// aggvertbuf.cpp

#include "global.h"

#include "aggvertbuf.h"
#include "profiler.h"



aggregate_vert_buf::aggregate_vert_buf()
  : mat(NULL), frame(0), force_flags(0), nquads(0)
  , send_flags(0), locked(false)
{
}

aggregate_vert_buf::aggregate_vert_buf(material* amaterial, unsigned aframe, unsigned aforceflags)
  : mat(amaterial), frame(aframe), force_flags(aforceflags)
  , nquads(0), vertbuf(), send_flags(0), locked(false)
{
}

aggregate_vert_buf::~aggregate_vert_buf()
{
  if (locked)
    unlock();
  flush();
  if (!vertbuf)
    delete vertbuf;
  vertbuf = NULL;
}

void aggregate_vert_buf::make_vertbuf()
{
  if (!vertbuf)
    vertbuf = NEW vert_buf_xformed(MAXVERTS);
}

void aggregate_vert_buf::lock()
{
  assert(!locked);
  lock_type_t type = (nquads == 0) ? LOCK_CLEAR : LOCK_NOOVERWRITE;
  vertbuf->lock(-1,type);
  locked=true;
}

hw_rasta_vert_xformed* aggregate_vert_buf::get_quads(unsigned how_many)
{
  if (how_many+nquads > MAXQUADSPERBUF)
    flush(); // this can generate the exceptional state of needing to draw while the vertbuf is locked
             // but we handle this case gracefully, it is not an error.
  assert(how_many+nquads <= MAXQUADSPERBUF);
  hw_rasta_vert_xformed* ptr = vertbuf->begin()+nquads*4;
  nquads+=how_many;
  return ptr;
}

void aggregate_vert_buf::unget_quads(unsigned how_many)
{
  assert(nquads >= how_many);
  nquads-=how_many;
}

void aggregate_vert_buf::unlock()
{
  assert(locked);
  vertbuf->unlock();
  locked=false;
}

// this isn't entirely accurate as this may be used for the interface as well
extern profiler_timer proftimer_render_sendctx_billboards;
extern profiler_timer proftimer_render_draw_billboards;

void aggregate_vert_buf::flush()
{
#ifdef TARGET_PC
  // leaves lock state same as when function was called (but may need to temporarily unlock
  // in order to draw)
  if (!nquads) return;
  if (locked) vertbuf->unlock();
  proftimer_render_sendctx_billboards.start();
  mat->send_context(frame, MAP_DIFFUSE, force_flags);
  proftimer_render_sendctx_billboards.stop();
  proftimer_render_draw_billboards.start();
  // have to convert from quads to triangle list
  unsigned short agg_indices[MAXIDXS];
  unsigned num_verts=nquads*4;
  unsigned num_indices=nquads*6;
  uint16* ip=&agg_indices[num_indices];
  for (unsigned n=num_verts; ip>agg_indices; )
  {
    ip -= 6;
    n -= 4;
    ip[0]= n+0; ip[1]= n+1; ip[2]= n+2;
    ip[3]= n+2; ip[4]= n+1; ip[5]= n+3;
  }
  hw_rasta::inst()->send_indexed_vertex_list( *vertbuf, num_verts,
              agg_indices, num_indices,
              send_flags/* | hw_rasta::SEND_VERT_SKIP_CLIP*/ );

  proftimer_render_draw_billboards.stop();
  nquads=0;
  if (locked) vertbuf->lock(-1);
#endif // TARGET_PC
}


void aggregate_vert_buf_list::init()
{
  for (aggregate_vert_buf* slot = &slots[nslots]; --slot>=slots; )
    slot->make_vertbuf();
}

aggregate_vert_buf* aggregate_vert_buf_list::find(material* mat, unsigned frame, unsigned force_flags)
{
  // is one already in the list?  While looking, note the slot that has the most entries
  // in case we have to spill one to make room.
  aggregate_vert_buf* fullest=slots;
  unsigned fullest_nquads = 0;
  for (aggregate_vert_buf* slot = &slots[nslots]; --slot>=slots; )
  {
    slot->make_vertbuf();
    if (slot->mat == mat &&
        slot->frame == frame &&
        slot->force_flags == force_flags)
    {
      return slot;
    }
    if (slot->nquads > fullest_nquads)
    {
      fullest_nquads = slot->nquads;
      fullest = slot;
    }
  }
  fullest->flush();
  fullest->mat = mat;
  fullest->frame = frame;
  fullest->force_flags = force_flags;
  return fullest;
}

void aggregate_vert_buf_list::flush()
{
  for (aggregate_vert_buf* slot = &slots[nslots]; --slot>=slots; )
    slot->flush();
}
