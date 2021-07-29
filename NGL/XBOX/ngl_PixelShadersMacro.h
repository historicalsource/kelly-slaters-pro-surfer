xps.1.1

#include "ngl_PixelShadersConst.h"

#define MAP_BM_CONST            c[PSC_MAP_BM_CONST].a
#define DETAILMAP_BM_CONST      c[PSC_DETAILMAP_BM_CONST].a
#define ENVIROMAP_BM_CONST      c[PSC_ENVIROMAP_BM_CONST].a
#define LIGHTMAP_BM_CONST       c[PSC_LIGHTMAP_BM_CONST].a

; xfc:
; Output.rgb = Src0 * Src1 + (1 - Src0) * Src2 + Src3
; Output.a = Src6.a
; prod = Src4 * Src5
; sum = r0 + v1

; When specular lighting will be implemented, set Src3 = sum.
#define XFC_SPECULAR            xfc fog.a, r0, fog.rgb, zero, zero, zero, r0.a
#ifdef PROJECT_KELLYSLATER
#define XFC_NO_SPECULAR         xfc 1-zero, r0, 1-zero, r0, zero, zero, r0.a	// double all rgb values (!)
#else
#define XFC_NO_SPECULAR         xfc fog.a, r0, fog.rgb, zero, zero, zero, r0.a
#endif
