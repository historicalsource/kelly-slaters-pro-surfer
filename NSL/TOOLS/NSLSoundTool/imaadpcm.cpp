/***************************************************************************
 *
 *  Copyright (C) 2001 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       imaadpcm.cpp
 *  Content:    IMA ADPCM CODEC.
 *  History:
 *   Date       By      Reason
 *   ====       ==      ======
 *  04/29/01    dereks  Created.
 *  06/12/01    jharding Adapted for command-line encode
 *
 ****************************************************************************/

#include "imaadpcm.h"

/****************************************************************************
 *
 *  CImaAdpcmCodec
 *
 *  Description:
 *      Object constructor.
 *
 *  Arguments:
 *      (void)
 *
 *  Returns:  
 *      (void)
 *
 ****************************************************************************/

//
// This array is used by NextStepIndex to determine the next step index to use.  
// The step index is an index to the m_asStep[] array, below.
//

const short CImaAdpcmCodec::m_asNextStep[16] =
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

//
// This array contains the array of step sizes used to encode the ADPCM
// samples.  The step index in each ADPCM block is an index to this array.
//

const short CImaAdpcmCodec::m_asStep[89] =
{
        7,     8,     9,    10,    11,    12,    13,
       14,    16,    17,    19,    21,    23,    25,
       28,    31,    34,    37,    41,    45,    50,
       55,    60,    66,    73,    80,    88,    97,
      107,   118,   130,   143,   157,   173,   190,
      209,   230,   253,   279,   307,   337,   371,
      408,   449,   494,   544,   598,   658,   724,
      796,   876,   963,  1060,  1166,  1282,  1411,
     1552,  1707,  1878,  2066,  2272,  2499,  2749,
     3024,  3327,  3660,  4026,  4428,  4871,  5358,
     5894,  6484,  7132,  7845,  8630,  9493, 10442,
    11487, 12635, 13899, 15289, 16818, 18500, 20350,
    22385, 24623, 27086, 29794, 32767
};

CImaAdpcmCodec::CImaAdpcmCodec
(
    void
)
{
}


/****************************************************************************
 *
 *  ~CImaAdpcmCodec
 *
 *  Description:
 *      Object destructor.
 *
 *  Arguments:
 *      (void)
 *
 *  Returns:  
 *      (void)
 *
 ****************************************************************************/

CImaAdpcmCodec::~CImaAdpcmCodec
(
    void
)
{
}


/****************************************************************************
 *
 *  Initialize
 *
 *  Description:
 *      Initializes the object.
 *
 *  Arguments:
 *      LPCIMAADPCMWAVEFORMAT [in]: encoded data format.
 *      BOOL [in]: TRUE to initialize the object as an encoder.
 *
 *  Returns:  
 *      BOOL: TRUE on success.
 *
 ****************************************************************************/

BOOL
CImaAdpcmCodec::Initialize
(
    LPCIMAADPCMWAVEFORMAT               pwfxEncode, 
    BOOL                                fEncoder
)
{
    static const LPFNIMAADPCMCONVERT    apfnConvert[2][2] = 
    { 
        {
            DecodeM16,
            DecodeS16 
        },
        {
            EncodeM16,
            EncodeS16 
        }
    };
    
    if(!IsValidImaAdpcmFormat(pwfxEncode))
    {
        return FALSE;
    }

    //
    // Save the format data
    //

    m_wfxEncode = *pwfxEncode;
    m_fEncoder = !!fEncoder;

    //
    // Set up the conversion function
    //

    m_pfnConvert = apfnConvert[m_fEncoder][m_wfxEncode.wfx.nChannels - 1];

    //
    // Initialize the stepping indeces
    //

    m_nStepIndexL = m_nStepIndexR = 0;

    return TRUE;
}


/****************************************************************************
 *
 *  Convert
 *
 *  Description:
 *      Converts data from the source to destination format.
 *
 *  Arguments:
 *      LPCVOID [in]: source buffer.
 *      LPVOID [out]: destination buffer.
 *      UINT [in]: block count.
 *
 *  Returns:  
 *      BOOL: TRUE on success.
 *
 ****************************************************************************/

BOOL
CImaAdpcmCodec::Convert
(
    LPCVOID                 pvSrc,
    LPVOID                  pvDst,
    UINT                    cBlocks
)
{
    return m_pfnConvert((LPBYTE)pvSrc, (LPBYTE)pvDst, cBlocks, m_wfxEncode.wfx.nBlockAlign, m_wfxEncode.wSamplesPerBlock, &m_nStepIndexL, &m_nStepIndexR);
}


/****************************************************************************
 *
 *  Reset
 *
 *  Description:
 *      Resets the conversion operation.
 *
 *  Arguments:
 *      (void)
 *
 *  Returns:  
 *      (void)
 *
 ****************************************************************************/

void
CImaAdpcmCodec::Reset
(
    void
)
{
    //
    // Reset the stepping indeces
    //

    m_nStepIndexL = m_nStepIndexR = 0;
}


/****************************************************************************
 *
 *  GetEncodeAlignment
 *
 *  Description:
 *      Gets the alignment of an encoded buffer.
 *
 *  Arguments:
 *      (void)
 *
 *  Returns:  
 *      WORD: alignment, in bytes.
 *
 ****************************************************************************/

WORD
CImaAdpcmCodec::GetEncodeAlignment
(
    void
)
{
    return m_wfxEncode.wfx.nBlockAlign;
}


/****************************************************************************
 *
 *  GetDecodeAlignment
 *
 *  Description:
 *      Gets the alignment of a decoded buffer.
 *
 *  Arguments:
 *      (void)
 *
 *  Returns:  
 *      DWORD: alignment, in bytes.
 *
 ****************************************************************************/

WORD
CImaAdpcmCodec::GetDecodeAlignment
(
    void
)
{
    return m_wfxEncode.wSamplesPerBlock * m_wfxEncode.wfx.nChannels * IMAADPCM_PCM_BITS_PER_SAMPLE / 8;
}


/****************************************************************************
 *
 *  CalculateEncodeAlignment
 *
 *  Description:
 *      Calculates an encoded data block alignment based on a PCM sample
 *      count and an alignment multiplier.
 *
 *  Arguments:
 *      WORD [in]: channel count.
 *      WORD [in]: PCM samples per block.
 *
 *  Returns:  
 *      WORD: alignment, in bytes.
 *
 ****************************************************************************/

WORD
CImaAdpcmCodec::CalculateEncodeAlignment
(
    WORD                    nChannels,
    WORD                    nSamplesPerBlock
)
{
    const WORD              nEncodedSampleBits  = nChannels * IMAADPCM_BITS_PER_SAMPLE;
    const WORD              nHeaderBytes        = nChannels * IMAADPCM_HEADER_LENGTH;
    WORD                    nBlockAlign;

    //
    // Calculate the raw block alignment that nSamplesPerBlock dictates.  This
    // value may include a partial encoded sample, so be sure to round up.
    //
    // Start with the samples-per-block, minus 1.  The first sample is actually
    // stored in the header.
    //

    nBlockAlign = nSamplesPerBlock - 1;

    //
    // Convert to encoded sample size
    //

    nBlockAlign *= nEncodedSampleBits;
    nBlockAlign += 7;
    nBlockAlign /= 8;

    //
    // The stereo encoder requires that there be at least two DWORDs to process
    //

    nBlockAlign += 7;
    nBlockAlign /= 8;
    nBlockAlign *= 8;

    //
    // Add the header
    //

    nBlockAlign += nHeaderBytes;

    return nBlockAlign;
}


/****************************************************************************
 *
 *  CreatePcmFormat
 *
 *  Description:
 *      Creates a PCM format descriptor.
 *
 *  Arguments:
 *      WORD [in]: channel count.
 *      DWORD [in]: sampling rate.
 *      LPWAVEFORMATEX [out]: format descriptor.
 *
 *  Returns:  
 *      (void)
 *
 ****************************************************************************/

void
CImaAdpcmCodec::CreatePcmFormat
(
    WORD                    nChannels, 
    DWORD                   nSamplesPerSec, 
    LPWAVEFORMATEX          pwfx
)
{
    pwfx->wFormatTag = WAVE_FORMAT_PCM;
    pwfx->nChannels = nChannels;
    pwfx->nSamplesPerSec = nSamplesPerSec;
    pwfx->nBlockAlign = nChannels * IMAADPCM_PCM_BITS_PER_SAMPLE / 8;
    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
    pwfx->wBitsPerSample = IMAADPCM_PCM_BITS_PER_SAMPLE;
}


/****************************************************************************
 *
 *  CreateImaAdpcmFormat
 *
 *  Description:
 *      Creates an IMA ADPCM format descriptor.
 *
 *  Arguments:
 *      WORD [in]: channel count.
 *      DWORD [in]: sampling rate.
 *      LPIMAADPCMWAVEFORMAT [out]: format descriptor.
 *
 *  Returns:  
 *      (void)
 *
 ****************************************************************************/

void
CImaAdpcmCodec::CreateImaAdpcmFormat
(
    WORD                    nChannels, 
    DWORD                   nSamplesPerSec, 
    WORD                    nSamplesPerBlock,
    LPIMAADPCMWAVEFORMAT    pwfx
)
{
    pwfx->wfx.wFormatTag = WAVE_FORMAT_XBOX_ADPCM;
    pwfx->wfx.nChannels = nChannels;
    pwfx->wfx.nSamplesPerSec = nSamplesPerSec;
    pwfx->wfx.nBlockAlign = CalculateEncodeAlignment(nChannels, nSamplesPerBlock);
    pwfx->wfx.nAvgBytesPerSec = nSamplesPerSec * pwfx->wfx.nBlockAlign / nSamplesPerBlock;
    pwfx->wfx.wBitsPerSample = IMAADPCM_BITS_PER_SAMPLE;
    pwfx->wfx.cbSize = sizeof(*pwfx) - sizeof(pwfx->wfx);
    pwfx->wSamplesPerBlock = nSamplesPerBlock;
}


/****************************************************************************
 *
 *  IsValidPcmFormat
 *
 *  Description:
 *      Validates a format structure.
 *
 *  Arguments:
 *      LPCWAVEFORMATEX [in]: format.
 *
 *  Returns:  
 *      BOOL: TRUE on success.
 *
 ****************************************************************************/

BOOL 
CImaAdpcmCodec::IsValidPcmFormat
(
    LPCWAVEFORMATEX         pwfx
)
{
    if(WAVE_FORMAT_PCM != pwfx->wFormatTag)
    {
        return FALSE;
    }
    
    if((pwfx->nChannels < 1) || (pwfx->nChannels > IMAADPCM_MAX_CHANNELS))
    {
        return FALSE;
    }

    if(IMAADPCM_PCM_BITS_PER_SAMPLE != pwfx->wBitsPerSample)
    {
        return FALSE;
    }

    if(pwfx->nChannels * pwfx->wBitsPerSample / 8 != pwfx->nBlockAlign)
    {
        return FALSE;
    }

    if(pwfx->nBlockAlign * pwfx->nSamplesPerSec != pwfx->nAvgBytesPerSec)
    {
        return FALSE;
    }

    return TRUE;
}


/****************************************************************************
 *
 *  IsValidXboxAdpcmFormat
 *
 *  Description:
 *      Validates a format structure.
 *
 *  Arguments:
 *      LPCIMAADPCMWAVEFORMAT [in]: format.
 *
 *  Returns:  
 *      BOOL: TRUE on success.
 *
 ****************************************************************************/

BOOL 
CImaAdpcmCodec::IsValidImaAdpcmFormat
(
    LPCIMAADPCMWAVEFORMAT   pwfx
)
{
    if(WAVE_FORMAT_XBOX_ADPCM != pwfx->wfx.wFormatTag)
    {
        return FALSE;
    }

    if(sizeof(*pwfx) - sizeof(pwfx->wfx) != pwfx->wfx.cbSize)
    {
        return FALSE;
    }
    
    if((pwfx->wfx.nChannels < 1) || (pwfx->wfx.nChannels > IMAADPCM_MAX_CHANNELS))
    {
        return FALSE;
    }

    if(IMAADPCM_BITS_PER_SAMPLE != pwfx->wfx.wBitsPerSample)
    {
        return FALSE;
    }

    if(CalculateEncodeAlignment(pwfx->wfx.nChannels, pwfx->wSamplesPerBlock) != pwfx->wfx.nBlockAlign)
    {
        return FALSE;
    }

    return TRUE;
}


/****************************************************************************
 *
 *  EncodeSample
 *
 *  Description:
 *      Encodes a sample.
 *
 *  Arguments:
 *      int [in]: the sample to be encoded.
 *      LPINT [in/out]: the predicted value of the sample.
 *      int [in]: the quantization step size used to encode the sample.
 *
 *  Returns:  
 *      int: the encoded ADPCM sample.
 *
 ****************************************************************************/

int
CImaAdpcmCodec::EncodeSample
(
    int                 nInputSample,
    LPINT               pnPredictedSample,
    int                 nStepSize
)
{
    int                 nPredictedSample;
    LONG                lDifference;
    int                 nEncodedSample;
    
    nPredictedSample = *pnPredictedSample;

    lDifference = nInputSample - nPredictedSample;
    nEncodedSample = 0;

    if(lDifference < 0) 
    {
        nEncodedSample = 8;
        lDifference = -lDifference;
    }

    if(lDifference >= nStepSize)
    {
        nEncodedSample |= 4;
        lDifference -= nStepSize;
    }

    nStepSize >>= 1;

    if(lDifference >= nStepSize)
    {
        nEncodedSample |= 2;
        lDifference -= nStepSize;
    }

    nStepSize >>= 1;

    if(lDifference >= nStepSize)
    {
        nEncodedSample |= 1;
        lDifference -= nStepSize;
    }

    if(nEncodedSample & 8)
    {
        nPredictedSample = nInputSample + lDifference - (nStepSize >> 1);
    }
    else
    {
        nPredictedSample = nInputSample - lDifference + (nStepSize >> 1);
    }

    if(nPredictedSample > 32767)
    {
        nPredictedSample = 32767;
    }
    else if(nPredictedSample < -32768)
    {
        nPredictedSample = -32768;
    }

    *pnPredictedSample = nPredictedSample;
    
    return nEncodedSample;
}


/****************************************************************************
 *
 *  DecodeSample
 *
 *  Description:
 *      Decodes an encoded sample.
 *
 *  Arguments:
 *      int [in]: the sample to be decoded.
 *      int [in]: the predicted value of the sample.
 *      int [i]: the quantization step size used to encode the sample.
 *
 *  Returns:  
 *      int: the decoded PCM sample.
 *
 ****************************************************************************/

int
CImaAdpcmCodec::DecodeSample
(
    int                 nEncodedSample,
    int                 nPredictedSample,
    int                 nStepSize
)
{
    LONG                lDifference;
    LONG                lNewSample;

    lDifference = nStepSize >> 3;

    if(nEncodedSample & 4) 
    {
        lDifference += nStepSize;
    }

    if(nEncodedSample & 2) 
    {
        lDifference += nStepSize >> 1;
    }

    if(nEncodedSample & 1) 
    {
        lDifference += nStepSize >> 2;
    }

    if(nEncodedSample & 8)
    {
        lDifference = -lDifference;
    }

    lNewSample = nPredictedSample + lDifference;

    if((LONG)(short)lNewSample != lNewSample)
    {
        if(lNewSample < -32768)
        {
            lNewSample = -32768;
        }
        else
        {
            lNewSample = 32767;
        }
    }

    return (int)lNewSample;
}


/****************************************************************************
 *
 *  Conversion Routines
 *
 *  Description:
 *      Converts a PCM buffer to ADPCM, or the reverse.
 *
 *  Arguments:
 *      LPBYTE [in]: source buffer.
 *      LPBYTE [out]: destination buffer.
 *      UINT [in]: block count.
 *      UINT [in]: block alignment of the ADPCM data, in bytes.
 *      UINT [in]: the number of samples in each ADPCM block (not used in
 *                 decoding).
 *      LPINT [in/out]: left-channel stepping index.
 *      LPINT [in/out]: right-channel stepping index.
 *
 *  Returns:  
 *      BOOL: TRUE on success.
 *
 ****************************************************************************/

BOOL
CImaAdpcmCodec::EncodeM16
(
    LPBYTE                  pbSrc,
    LPBYTE                  pbDst,
    UINT                    cBlocks,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    LPINT                   pnStepIndexL,
    LPINT                   pnStepIndexR
)
{
    LPBYTE                  pbBlock;
    UINT                    cSamples;
    int                     nSample;
    int                     nStepSize;
    int                     nEncSample1;
    int                     nEncSample2;
    int                     nPredSample;
    int                     nStepIndex;

    //
    // Save a local copy of the step index so we're not constantly 
    // dereferencing a pointer.
    //
    
    nStepIndex = *pnStepIndexL;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbDst;
        cSamples = cSamplesPerBlock - 1;

        //
        // Block header
        //

        nPredSample = *(short *)pbSrc;
        pbSrc += sizeof(short);

        *(LONG *)pbBlock = MAKELONG(nPredSample, nStepIndex);
        pbBlock += sizeof(LONG);

        //
        // We have written the header for this block--now write the data
        // chunk (which consists of a bunch of encoded nibbles).  Note
        // that if we don't have enough data to fill a complete byte, then
        // we add a 0 nibble on the end.
        //

        while(cSamples)
        {
            //
            // Sample 1
            //

            nSample = *(short *)pbSrc;
            pbSrc += sizeof(short);
            cSamples--;

            nStepSize = m_asStep[nStepIndex];
            nEncSample1 = EncodeSample(nSample, &nPredSample, nStepSize);
            nStepIndex = NextStepIndex(nEncSample1, nStepIndex);

            //
            // Sample 2
            //

            if(cSamples)
            {
                nSample = *(short *)pbSrc;
                pbSrc += sizeof(short);
                cSamples--;

                nStepSize = m_asStep[nStepIndex];
                nEncSample2 = EncodeSample(nSample, &nPredSample, nStepSize);
                nStepIndex = NextStepIndex(nEncSample2, nStepIndex);
            }
            else
            {
                nEncSample2 = 0;
            }

            //
            // Write out encoded byte.
            //

            *pbBlock++ = (BYTE)(nEncSample1 | (nEncSample2 << 4));
        }

        //
        // Skip padding
        //

        pbDst += nBlockAlignment;
    }

    //
    // Restore the value of the step index to be used on the next buffer.
    //

    *pnStepIndexL = nStepIndex;

    return TRUE;
}


BOOL
CImaAdpcmCodec::EncodeS16
(
    LPBYTE                  pbSrc,
    LPBYTE                  pbDst,
    UINT                    cBlocks,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    LPINT                   pnStepIndexL,
    LPINT                   pnStepIndexR
)
{
    LPBYTE                  pbBlock;
    UINT                    cSamples;
    UINT                    cSubSamples;
    int                     nSample;
    int                     nStepSize;
    DWORD                   dwLeft;
    DWORD                   dwRight;
    int                     nEncSampleL;
    int                     nPredSampleL;
    int                     nStepIndexL;
    int                     nEncSampleR;
    int                     nPredSampleR;
    int                     nStepIndexR;
    UINT                    i;

    //
    // Save a local copy of the step indeces so we're not constantly 
    // dereferencing a pointer.
    //
    
    nStepIndexL = *pnStepIndexL;
    nStepIndexR = *pnStepIndexR;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbDst;
        cSamples = cSamplesPerBlock - 1;

        //
        // LEFT channel block header
        //

        nPredSampleL = *(short *)pbSrc;
        pbSrc += sizeof(short);

        *(LONG *)pbBlock = MAKELONG(nPredSampleL, nStepIndexL);
        pbBlock += sizeof(LONG);

        //
        // RIGHT channel block header
        //

        nPredSampleR = *(short *)pbSrc;
        pbSrc += sizeof(short);

        *(LONG *)pbBlock = MAKELONG(nPredSampleR, nStepIndexR);
        pbBlock += sizeof(LONG);

        //
        // We have written the header for this block--now write the data
        // chunk.  This consists of 8 left samples (one DWORD of output)
        // followed by 8 right samples (also one DWORD).  Since the input
        // samples are interleaved, we create the left and right DWORDs
        // sample by sample, and then write them both out.
        //

        while(cSamples)
        {
            dwLeft = 0;
            dwRight = 0;

            cSubSamples = min(cSamples, 8);

            for(i = 0; i < cSubSamples; i++)
            {
                //
                // LEFT channel
                //

                nSample = *(short *)pbSrc;
                pbSrc += sizeof(short);

                nStepSize = m_asStep[nStepIndexL];
                
                nEncSampleL = EncodeSample(nSample, &nPredSampleL, nStepSize);

                nStepIndexL = NextStepIndex(nEncSampleL, nStepIndexL);
                dwLeft |= (DWORD)nEncSampleL << (4 * i);

                //
                // RIGHT channel
                //

                nSample = *(short *)pbSrc;
                pbSrc += sizeof(short);

                nStepSize = m_asStep[nStepIndexR];
                
                nEncSampleR = EncodeSample(nSample, &nPredSampleR, nStepSize);

                nStepIndexR = NextStepIndex(nEncSampleR, nStepIndexR);
                dwRight |= (DWORD)nEncSampleR << (4 * i);
            }

            //
            // Write out encoded DWORDs.
            //

            *(LPDWORD)pbBlock = dwLeft;
            pbBlock += sizeof(DWORD);

            *(LPDWORD)pbBlock = dwRight;
            pbBlock += sizeof(DWORD);

            cSamples -= cSubSamples;
        }

        //
        // Skip padding
        //

        pbDst += nBlockAlignment;
    }

    //
    // Restore the value of the step index to be used on the next buffer.
    //
    
    *pnStepIndexL = nStepIndexL;
    *pnStepIndexR = nStepIndexR;

    return TRUE;

}


BOOL
CImaAdpcmCodec::DecodeM16   
(
    LPBYTE                  pbSrc,
    LPBYTE                  pbDst,
    UINT                    cBlocks,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    LPINT                   pnStepIndexL,
    LPINT                   pnStepIndexR
)
{
    BOOL                    fSuccess    = TRUE;
    LPBYTE                  pbBlock;
    UINT                    cSamples;
    BYTE                    bSample;
    int                     nStepSize;
    int                     nEncSample;
    int                     nPredSample;
    int                     nStepIndex;
    DWORD                   dwHeader;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbSrc;
        cSamples = cSamplesPerBlock - 1;
        
        //
        // Block header
        //

        dwHeader = *(LPDWORD)pbBlock;
        pbBlock += sizeof(DWORD);

        nPredSample = (int)(short)LOWORD(dwHeader);
        nStepIndex = (int)(BYTE)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndex))
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero bytes converted.
            //

            fSuccess = FALSE;
            break;
        }
        
        //
        // Write out first sample
        //

        *(short *)pbDst = (short)nPredSample;
        pbDst += sizeof(short);

        //
        // Enter the block loop
        //

        while(cSamples)
        {
            bSample = *pbBlock++;

            //
            // Sample 1
            //

            nEncSample = (bSample & (BYTE)0x0F);
            nStepSize = m_asStep[nStepIndex];
            nPredSample = DecodeSample(nEncSample, nPredSample, nStepSize);
            nStepIndex = NextStepIndex(nEncSample, nStepIndex);

            *(short *)pbDst = (short)nPredSample;
            pbDst += sizeof(short);

            cSamples--;

            //
            // Sample 2
            //

            if(cSamples)
            {
                nEncSample = (bSample >> 4);
                nStepSize = m_asStep[nStepIndex];
                nPredSample = DecodeSample(nEncSample, nPredSample, nStepSize);
                nStepIndex = NextStepIndex(nEncSample, nStepIndex);

                *(short *)pbDst = (short)nPredSample;
                pbDst += sizeof(short);

                cSamples--;
            }
        }

        //
        // Skip padding
        //

        pbSrc += nBlockAlignment;
    }

    return fSuccess;
}


BOOL
CImaAdpcmCodec::DecodeS16
(
    LPBYTE                  pbSrc,
    LPBYTE                  pbDst,
    UINT                    cBlocks,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    LPINT                   pnStepIndexL,
    LPINT                   pnStepIndexR
)
{
    BOOL                    fSuccess    = TRUE;
    LPBYTE                  pbBlock;
    UINT                    cSamples;
    UINT                    cSubSamples;
    int                     nStepSize;
    DWORD                   dwHeader;
    DWORD                   dwLeft;
    DWORD                   dwRight;
    int                     nEncSampleL;
    int                     nPredSampleL;
    int                     nStepIndexL;
    int                     nEncSampleR;
    int                     nPredSampleR;
    int                     nStepIndexR;
    UINT                    i;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbSrc;
        cSamples = cSamplesPerBlock - 1;

        //
        // LEFT channel header
        //

        dwHeader = *(LPDWORD)pbBlock;
        pbBlock += sizeof(DWORD);
        
        nPredSampleL = (int)(short)LOWORD(dwHeader);
        nStepIndexL = (int)(BYTE)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndexL)) 
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero bytes converted.
            //

            fSuccess = FALSE;
            break;
        }
        
        //
        // RIGHT channel header
        //

        dwHeader = *(LPDWORD)pbBlock;
        pbBlock += sizeof(DWORD);
        
        nPredSampleR = (int)(short)LOWORD(dwHeader);
        nStepIndexR = (int)(BYTE)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndexR))
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero bytes converted.
            //

            fSuccess = FALSE;
            break;
        }

        //
        // Write out first sample
        //

        *(LPDWORD)pbDst = MAKELONG(nPredSampleL, nPredSampleR);
        pbDst += sizeof(DWORD);

        //
        // The first DWORD contains 4 left samples, the second DWORD
        // contains 4 right samples.  We process the source in 8-byte
        // chunks to make it easy to interleave the output correctly.
        //

        while(cSamples)
        {
            dwLeft = *(LPDWORD)pbBlock;
            pbBlock += sizeof(DWORD);
            dwRight = *(LPDWORD)pbBlock;
            pbBlock += sizeof(DWORD);

            cSubSamples = min(cSamples, 8);
            
            for(i = 0; i < cSubSamples; i++)
            {
                //
                // LEFT channel
                //

                nEncSampleL = (dwLeft & 0x0F);
                nStepSize = m_asStep[nStepIndexL];
                nPredSampleL = DecodeSample(nEncSampleL, nPredSampleL, nStepSize);
                nStepIndexL = NextStepIndex(nEncSampleL, nStepIndexL);

                //
                // RIGHT channel
                //

                nEncSampleR = (dwRight & 0x0F);
                nStepSize = m_asStep[nStepIndexR];
                nPredSampleR = DecodeSample(nEncSampleR, nPredSampleR, nStepSize);
                nStepIndexR = NextStepIndex(nEncSampleR, nStepIndexR);

                //
                // Write out sample
                //

                *(LPDWORD)pbDst = MAKELONG(nPredSampleL, nPredSampleR);
                pbDst += sizeof(DWORD);

                //
                // Shift the next input sample into the low-order 4 bits.
                //

                dwLeft >>= 4;
                dwRight >>= 4;
            }

            cSamples -= cSubSamples;
        }

        //
        // Skip padding
        //

        pbSrc += nBlockAlignment;
    }

    return fSuccess;
}


