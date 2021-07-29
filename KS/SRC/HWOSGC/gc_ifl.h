#ifndef GC_IFL_H
#define GC_IFL_H

#include "global.h"
#include "ngl_gc.h"
#include "gc_arammgr.h"
#include "osassert.h"

class gc_ifl
{
  public:
    nglTexture *plain_ifl;
    nglTexture *mram_tex[2];
    unsigned char *mram_tex_buf[2];
    int current_frame;
    int active_tex;
    int num_frames;
    int common_size;
    uint32 *frame_offset;
    aram_id_t aram_id;
    
    bool initialized;
    
  public:
    gc_ifl()
    {
      initialized = false;
    }
    
    gc_ifl(char *ifl_name)
    {
      assert( init( ifl_name ) );
    }
    
    ~gc_ifl()
    {
      deinit();
    }
    
    bool init(char *ifl_name);
    void deinit();

    nglTexture *get_frame(int frame);
};

#endif
