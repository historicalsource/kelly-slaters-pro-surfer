#ifndef NSL_XBOX_EXT_HEADER
#define NSL_XBOX_EXT_HEADER

#include <xtl.h>
#include "nsl.h"


/*-------------------------------------------------------------------------
  NSL PC Extended Functions
-------------------------------------------------------------------------*/
const char* nslGetDsMessageXbox( HRESULT hr );
LPDIRECTSOUND nslGetDirectSoundObjXbox();
void nslGetDsErrorMsgXbox(int line,HRESULT hr);
bool nslLoadDSPImageXbox(const char* dspName, DWORD reverbIndex, DWORD xtalkIndex);

void	nlVector3dAdd( nlVector3d &result, nlVector3d &v1, nlVector3d &v2 );
void	nlVector3dSub( nlVector3d &result, nlVector3d &v1, nlVector3d &v2 );
float nlVector3dLength( nlVector3d &v );
float nlVector3dLength2( nlVector3d &v );

#endif