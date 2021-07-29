#ifndef XB_PARTICLE_H
#define XB_PARTICLE_H

#ifdef TARGET_XBOX	// These are still in NGL for other platforms
#include "random.h"	// For reproducible random number streams

/*---------------------------------------------------------------------------------------------------------
Particle System API.

---------------------------------------------------------------------------------------------------------*/
// Particle system parameters.
struct ParticleSystem
{
	uint32 MaterialFlags;
	uint32 BlendMode;
	
	uint32 Num;                   // Number of Particles
	uint32 Scol;                  // Starting Color
	uint32 Rcol;                  // Random Starting Color
	uint32 Ecol;                  // Ending Color
	float Life;			          // Particle Lifetime
	float Rlife;		          // Random Particle Lifetime
	float Dura;			          // Duration for all particles to fire
	float Ctime;		          // Current Time
	float Ssize;                  // Starting Size
	float Rsize;		          // Random Starting Size Modifier
	float Esize;		          // Ending Size Modifier
	float MaxSize;	              // Maximum Size in Screen Space
	float Aspect;                 // Aspect Ratio.
	
	uint32 Seed;                  // Random Seed
	nglVector Spos;               // Starting Position
	nglVector Rpos1;              // Random Starting Line 1 Modifier
	nglVector Rpos2;              // Random Starting Line 2 Modifier
	nglVector Svel;	              // Starting Velocity
	nglVector Rvel1;	          // Random Starting Velocity Line 1 Modifier
	nglVector Rvel2;	          // Random Starting Velocity Line 2 Modifier
	nglVector Rvel3;	          // Random Starting Velocity Line 3 Modifier
	nglVector Force;	          // Constant force ( acceleration )
	
	nglTexture* Tex;              // Texture map.
};

void PARTICLE_Init();
void PARTICLE_Cleanup();
void PARTICLE_LockVertexBuffer();
void PARTICLE_ListAdd(ParticleSystem* Particle);
void PARTICLE_SetMax(uint16 Num);

typedef ParticleSystem nglParticleSystem;	// for compatibility with PS2
#define nglListAddParticle PARTICLE_ListAdd
#define nglSetMaxParticles PARTICLE_SetMax

#endif	// #ifdef TARGET_XBOX

#endif	// #ifndef XB_PARTICLE_H