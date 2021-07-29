#ifndef VERTWORK_H
#define VERTWORK_H

// NOTE: if you change this number, or the size of the class hw_rasta_vert,
// make sure you update hwosmks\sy_alloc.cpp so that there is enough room
// for the global vert_workspace in main memory.

enum { VIRTUAL_MAX_VERTS_PER_PRIMITIVE=9000 };

#if defined(TARGET_XBOX)
extern vert_lit_buf vert_workspace;
#else

#ifdef _XBOX
#error "no, you shouldn't be here"
#endif /* _XBOX */

extern vert_buf vert_workspace;       // <= 4096 verts
#endif /* TARGET_XBOX JIV DEBUG */

#ifdef TARGET_PC
extern vert_buf vert_workspace_small; // <= 512 verts
extern vert_buf vert_workspace_quad;  // <= 4 verts
extern vert_buf_xformed vert_workspace_xformed;       // <= 4096 verts
extern vert_buf_xformed vert_workspace_xformed_small; // <= 512 verts
extern vert_buf_xformed vert_workspace_xformed_quad;  // <= 4 verts
#elif defined(TARGET_XBOX)
extern vert_lit_buf vert_workspace_small; // <= 512 verts
extern vert_lit_buf vert_workspace_quad;  // <= 4 verts
extern vert_buf_xformed vert_workspace_xformed;       // <= 4096 verts
extern vert_buf_xformed vert_workspace_xformed_small; // <= 512 verts
extern vert_buf_xformed vert_workspace_xformed_quad;  // <= 4 verts
#elif defined(TARGET_PS2)
extern vert_buf vert_workspace_quad;  // <= 4 verts
extern vert_buf_xformed vert_workspace_xformed;       // <= 4096 verts
extern vert_buf_xformed vert_workspace_xformed_quad;  // <= 4 verts
#endif

#endif // VERTWORK_H
