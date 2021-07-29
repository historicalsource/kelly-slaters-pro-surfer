
typedef enum _DSP_IMAGE_test2_FX_INDICES {
    Graph0_I3DL2Reverb24K = 0,
    Graph0_XTalk = 1
} DSP_IMAGE_test2_FX_INDICES;

typedef struct _Graph0_FX0_I3DL2Reverb24K_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[2];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[35];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph0_FX0_I3DL2Reverb24K_STATE, *LPGraph0_FX0_I3DL2Reverb24K_STATE;

typedef const Graph0_FX0_I3DL2Reverb24K_STATE *LPCGraph0_FX0_I3DL2Reverb24K_STATE;

typedef struct _Graph0_FX1_XTalk_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[4];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[4];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph0_FX1_XTalk_STATE, *LPGraph0_FX1_XTalk_STATE;

typedef const Graph0_FX1_XTalk_STATE *LPCGraph0_FX1_XTalk_STATE;
