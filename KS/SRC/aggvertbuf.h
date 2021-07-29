#ifndef aggvertbuf_h
#define aggvertbuf_h

#include "material.h"
#include "hwrasterize.h"

class aggregate_vert_buf // this could be a generic class, not really specific to particles, anything with quads works
{
public:
  enum { MAXQUADSPERBUF=512, MAXVERTS=MAXQUADSPERBUF*4, MAXIDXS=MAXQUADSPERBUF*6 };

  material* mat;
  unsigned frame;
  unsigned force_flags;
  unsigned nquads;
  refptr<vert_buf_xformed> vertbuf;
  unsigned send_flags;
  bool locked;

  aggregate_vert_buf();

  aggregate_vert_buf(material* amaterial, unsigned aframe, unsigned aforceflags);

  ~aggregate_vert_buf();

  inline bool operator ==(const aggregate_vert_buf& r) const 
  { 
    return frame == r.frame && mat == r.mat; 
  }
  inline bool operator < (const aggregate_vert_buf& r) const 
  { 
    if (mat == r.mat) 
      return frame < r.frame;
    return mat <  r.mat; 
  }

  void make_vertbuf();

  void lock();
  
  hw_rasta_vert_xformed* get_quads(unsigned how_many);

  void unget_quads(unsigned how_many);

  void unlock();
  
  void flush();
};

class aggregate_vert_buf_list 
{
  enum { nslots = 16 };
  aggregate_vert_buf slots[nslots];

public:
  void init();

  aggregate_vert_buf* find(material* mat, unsigned frame, unsigned force_flags);

  void flush();
};

//now a member of wds
//extern aggregate_vert_buf_list matvertbufs;

#endif // aggvertbuf_h