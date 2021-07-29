#include <machine/regdef.h>
// this function takes a 32 bit lightmap and seperates it into
// 8bit R, G, and B maps.  it ignores the alpha.
// NOTE: this function REQUIRES count to be a multiple of 16

	.set	noreorder

  .section .text
  .global calc_rgb_new

// a0 = pixels
// a1 = dest r
// a2 = dest g
// a3 = dest b
// t0 = count
// v0 = Return Result.

.align 8
calc_rgb_new:
loop:
	lq    t1, 0x00(a0)		                // load up the first four inputs
	lq    t2, 0x10(a0)
	lq    t3, 0x20(a0)
	lq    t4, 0x30(a0)
	ppacb t5, t2, t1											// now we have red/blue interleved
	ppacb t6, t4, t3											// now we have red/blue interleved
	ppacb	t7, t6, t5											// now red should be inline
	psrlh t5, t5, 8												// shift the blue to the low byte
	psrlh t6, t6, 8
	sq		t7,	0x00(a1)										// store red bytes
	ppacb	t7, t6, t5											// now blue should be inline
	sq		t7,	0x00(a3)										// store blue bytes
	psrlh t1, t1, 8												// shift all the inputs one byte right
	psrlh t2, t2, 8
	psrlh t3, t3, 8
	psrlh t4, t4, 8
	ppacb t5, t2, t1											// now we have green/alpha interleved
	ppacb t6, t4, t3											// now we have green/alpha interleved
	ppacb	t7, t6, t5											// now red should be inline
	sq		t7,	0x00(a2)										// store green bytes
	sub	  t0, 16													// decrease the count
	add 	a0, 0x40												// increase pointer
	add   a1, 0x10												// increase the r pointer
	add   a2, 0x10                        // increase the g pointer
	add   a3, 0x10                        // increase the b pointer
	bgtz	t0, loop
	nop
	j 	ra
	nop


