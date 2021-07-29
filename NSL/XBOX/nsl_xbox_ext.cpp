#include "nsl_xbox.h"
#include "nsl_xbox_ext.h"
#include <math.h>

const char* nslGetDsMessageXbox( HRESULT hr )
{
  switch ( hr )
  {
	case DS_OK:                     return NULL;
	case DSERR_CONTROLUNAVAIL:      return "The control (volume, pan, and so forth) requested by the caller is not available.\n";                                              break;
	case DSERR_GENERIC:             return "An undetermined error occurred inside the DirectSound subsystem.\n";                                                               break;
	case DSERR_INVALIDCALL:         return "DirectSound: This function is not valid for the current state of this object.\n";                                                  break;
	case DSERR_NOAGGREGATION:       return "DirectSound: The object does not support aggregation.\n";                                                                          break;
	case DSERR_NODRIVER:            return "No sound driver is available for use.\n";                                                                                          break;
	case DSERR_OUTOFMEMORY:         return "The DirectSound subsystem could not allocate sufficient memory to complete the caller's request.\n";                               break;
	case DSERR_UNSUPPORTED:         return "DirectSound: The function called is not supported at this time.\n";                                                                break;
	default:                        return "Encountered unknown DirectSound result code.";
  }
}

void nslGetDsErrorMsgXbox(int line,HRESULT hr)
{
  const char* msg = nslGetDsMessageXbox(hr);
  if (!msg) return;
  char str[512];
	sprintf(str, "DirectSound failure: %s, %s line %d", msg, __FILE__, line);
  internalAssert(0 && str);
}

LPDIRECTSOUND nslGetDirectSoundObjXbox()
{
	return sndSystem.pDS;
}

bool nslLoadDSPImageXbox(const char* dspName, DWORD reverbIndex = DSFX_IMAGELOC_UNUSED, DWORD xtalkIndex = DSFX_IMAGELOC_UNUSED)
{
	nslFileBuf dspImage;
	if (!nslReadFile(dspName, &dspImage))
  {
		nslPrintf("NSL ERROR: Cannot load DSP program %s...\n", dspName);
    return false;
  }
	
	LPDSEFFECTIMAGEDESC pDesc;
	DSEFFECTIMAGELOC EffectLoc;	
	EffectLoc.dwI3DL2ReverbIndex = reverbIndex;
	EffectLoc.dwCrosstalkIndex = xtalkIndex;	
	DS_TRY( sndSystem.pDS->DownloadEffectsImage( dspImage.Buf, dspImage.Size, &EffectLoc, &pDesc ) );		

	nslReleaseFile(&dspImage);
  return true;
}

void nlVector3dAdd( nlVector3d &result, nlVector3d &v1, nlVector3d &v2 )
{
	result[0] = v1[0] + v2[0];
	result[1] = v1[1] + v2[1];
	result[2] = v1[2] + v2[2];
}

void nlVector3dSub( nlVector3d &result, nlVector3d &v1, nlVector3d &v2 )
{
	result[0] = v1[0] - v2[0];
	result[1] = v1[1] - v2[1];
	result[2] = v1[2] - v2[2];
}

float nlVector3dLength( nlVector3d &v )
{
	return( (float)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]) );
}

float nlVector3dLength2( nlVector3d &v )
{
	return( v[0]*v[0]+v[1]*v[1]+v[2]*v[2] );
}

