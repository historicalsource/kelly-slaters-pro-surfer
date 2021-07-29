#ifndef OSPARTICLE_H
#define OSPARTICLE_H
/*-------------------------------------------------------------------------------------------------------
  PARTICLE.H - Include file redirect for wave render module.
-------------------------------------------------------------------------------------------------------*/

#if defined(TARGET_PC)
//#include "hwospc/w32_particle.h"
#elif defined(TARGET_MKS)
//#include "hwosmks/sy_particle.h"
#elif defined(TARGET_PS2)
struct nglParticleSystem;
//#include "hwosps2/ps2_particle.h"
#elif defined(TARGET_NULL)
struct nglParticleSystem;
//#include "hwosnull/null_particle.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_particle.h"
#elif defined(TARGET_GC)
//#include "hwosgc\gc_particle.h"
#endif

#endif
