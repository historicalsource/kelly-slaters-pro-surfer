// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#include "ngl.h"
#else
#ifdef DEBUG
#include "ngl_ps2.h"	// For nglPrintf
#endif
#endif /* TARGET_XBOX JIV DEBUG */

/*	SPLINE INTERPOLATION ROUTINES
*/

/*	Given a set of target points (x[i],y[i]), i = 0,...,n-1, we wish to define a set of cubic functions

		f[i](x) = a[i] * (x-x[i])^3 + b[i] * (x-x[i])^2 + c[i] * (x-x[i]) + d[i]

	i = 0,...,n-2 satisfying the following conditions:

		(1) f[i](x[i]) = y[i]				i = 0,...,n-2
		(2) f[i](x[i+1]) = y[i+1]			i = 0,...,n-2
		(3) f[i]'(x[i]) = f[i-1]'(x[i])		i = 1,...,n-2
		(4) f[i]''(x[i]) = f[i-1]''(x[i])	i = 1,...,n-2
		(5) f[0]''(x[0]) = 0
		(6) f[n-2]''(x[n-1]) = 0

	So the f[i]'s blend together to form a single, piecewise-cubic function which is everywhere C^2
	(continuous in value, derivative, and second derivative), and which passes through all the target
	points.

	It turns out that using conditions (1), (2), (4), we can solve for a[i], c[i], and d[i] in terms of
	b[i], x[i], and y[i], as follows:

	(*)
		a[i] = (b[i+1] - b[i]) / (3 * h[i])
		c[i] = (3 * k[i] - (b[i+1] + 2 * b[i]) * h[i] * h[i]) / (3 * h[i])
		d[i] = y[i]

	Here h[i] is shorthand for x[i+1] - x[i], and k[i] is shorthand for y[i+1] - y[i].  Note that b[n-1]
	is not yet defined, since i only goes up to n-2.

	Adding in conditions (5) and (6) we can derive:

		b[0] = 0
		b[n-1] = 0

	The remaining condition is (3).  This produces a tri-diagonal series of linear equations in b[i],
	h[i], and k[i] which can be written

	| q[1] p[2] 0    .  .  .   .    0      | | b[1]   |   | r[1]   |
	| p[2] q[2] p[3]                .      | | b[2]   |   | r[2]   |
	| 0    p[3] q[3]                .      | | .      |   | .      |
	| .              .              .      | | .      | = | .      |
	| .                 .           0      | | .      |   | .      |
	| .                    q[n-3]   p[n-2] | | .      |   | .      |
	| 0    .    .    .  0  p[n-2]   q[n-2] | | b[n-2] |   | r[n-2] |

	The coefficients are computed as follows:

		p[i] = h[i-1]
		q[i] = 2 * (h[i] + h[i-1])
		r[i] = 3 * (k[i] / h[i] - k[i-1] / h[i-1])

	Tridiagonal systems can be solved in linear time, unlike a general matrix system, which is order
	n^3 or slightly better.  The algorithm is to recursively compute the quantities:

		u[i] = p[i] / (q[i-1] - p[i-1] * u[i-1])
		v[i] = (r[i] - p[i] * v[i-1]) / (q[i] - p[i] * u[i])

	for i = 1,...,n-2 , starting with p[1] = v[0] = 0.  These values can be shown to satisfy the relation:

		b[i] = v[i] - u[i+1] * b[i+1]

	so we can compute b[i] recursively, starting from b[n-1] = 0 and working back down.  Then we can
	compute a[i], c[i], and d[i] from b[i] according to the equations marked (*) above.
*/

#ifdef DEBUG	// This code must run as fast as possible in Bootable.
#if defined(TARGET_XBOX) || defined(TARGET_GC)
#define SPLINEASSERTS
#endif
#endif

void SPLINE_ComputeCoeffs(const float *x, const float *y, int n, float *a, float *b, float *c, float *d)
{
	int i;
	float denom;
	// Save cache misses by reusing memory, but take care not to overwrite needed data
	float * const &h = c, * const &k_over_h = d, * const &u = a-1, * const &v = b;

//	START_PROF_TIMER(proftimer_spline_coeffs);	// affects frame rate to have this on
#ifdef DEBUG
	for (i = 1; i < n; ++i)
	{
		assert (x[i] > x[i-1]);
	}
#endif

	// Set up the symmetric, tridiagonal matrix.
	// And forward substitute.
	h[0] = x[1] - x[0];
	k_over_h[0] = (y[1] - y[0]) / h[0];
	h[1] = x[2] - x[1];
	k_over_h[1] = (y[2] - y[1]) / h[1];
	u[1] = 0;
	denom = 2 * (h[1] + h[0]);
	v[1] = (3 * (k_over_h[1] - k_over_h[0])) / denom;

#if defined(SPLINEASSERTS)
  assert( denom == denom );
  assert( h[0] == h[0] );
  assert( h[1] == h[1] );
  assert( k_over_h[0] == k_over_h[0] );
  assert( k_over_h[1] == k_over_h[1] );
  assert( u[1] == u[1] );
  assert( v[1] == v[1] );
#endif /* SPLINEASSERTS JIV DEBUG */

	for (i = 2; i < n-1; ++i)
	{
		h[i] = x[i+1] - x[i];
		k_over_h[i] = (y[i+1] - y[i]) / h[i];
		u[i] = h[i-1] / denom;
		denom = 2 * (h[i] + h[i-1]) - h[i-1] * u[i];
		v[i] = (3 * (k_over_h[i] - k_over_h[i-1]) - h[i-1] * v[i-1]) / denom;

#if defined(SPLINEASSERTS)
    assert( denom );
    assert( denom == denom );
    assert( h[i] == h[i] );
    assert( k_over_h[i] == k_over_h[i] );
    assert( v[i] == v[i] );
    assert( u[i] == u[i] );
#endif /* SPLINEASSERTS JIV DEBUG */

	}

#ifndef TARGET_PS2
  // NaN * 0 = NaN
	u[n-1] = 0.0f;
#else
	//u[n-1] = 0;	// not necessary, since we multiply it by 0
#endif /* SPLINEASSERTS JIV DEBUG */

	// Backsubstitute for b.
	// And solve for the other coefficients from b.
	b[n-1] = 0;
	for(i = n-2; i > 0; --i)
	{
#if defined(SPLINEASSERTS)
    assert( k_over_h[i] == k_over_h[i] );
    assert( h[i] == h[i] );
    assert( b[i+1] == b[i+1] );
    assert( b[i] == b[i] );
    assert( y[i] == y[i] );
#endif /* SPLINEASSERTS JIV DEBUG */

#if defined(SPLINEASSERTS)
    assert( v[i] == v[i] );
    assert( b[i+1] == b[i+1] );

    float oldval = v[i];
    float oldbp1 = b[i+1];

#endif /* SPLINEASSERTS JIV DEBUG */

		b[i] = v[i] - u[i+1] * b[i+1];

#if defined(SPLINEASSERTS)
    float oldb   = b[i];

    assert( b[i+1] - b[i] == b[i+1] - b[i] );
    assert( 3 * h[i] );
#endif /* SPLINEASSERTS JIV DEBUG */

		a[i] = (b[i+1] - b[i]) / (3 * h[i]);
		c[i] = k_over_h[i] - (b[i+1] + 2 * b[i]) * h[i] / 3;
		d[i] = y[i];

#if defined(SPLINEASSERTS)
    assert( k_over_h[i] == k_over_h[i] );
    assert( a[i] == a[i] );
    assert( b[i] == b[i] );
    assert( c[i] == c[i] );
    assert( d[i] == d[i] );
#endif /* SPLINEASSERTS JIV DEBUG */
	}
	b[0] = 0;
	a[0] = b[1] / (3 * h[0]);
	c[0] = k_over_h[0] - b[1] * h[0] / 3;
	d[0] = y[0];

#if defined(SPLINEASSERTS)
    assert( a[0] == a[0] );
    assert( b[0] == b[0] );
    assert( c[0] == c[0] );
    assert( d[0] == d[0] );
#endif /* SPLINEASSERTS JIV DEBUG */

//	STOP_PROF_TIMER(proftimer_spline_coeffs);	// affects frame rate to have this on
}

/*	More explicit, slower version.  

void SPLINE_ComputeCoeffs(const float *x, const float *y, int n, float *a, float *b, float *c, float *d)
{
	int i;
	float denom;
	float h[100], k[100], p[100], q[100], r[100], u[100], v[100];

#ifdef DEBUG
	for (i = 1; i < n; ++i)
	{
		assert (x[i] >= x[i-1])
	}
	assert (n <= 20);
#endif

	// Set up the symmetric, tridiagonal matrix.
	h[0] = x[1] - x[0];
	k[0] = y[1] - y[0];
	for (i = 1; i < n-1; ++i)
	{
		h[i] = x[i+1] - x[i];
		k[i] = y[i+1] - y[i];
		p[i] = h[i-1];
		q[i] = 2 * (h[i] + h[i-1]);
		r[i] = 3 * (k[i] / h[i] - k[i-1] / h[i-1]);
	}

	// Forward substitute.
	p[1] = 0;
	// v[0] = 0;	// not necessary, since we multiply it by 0
	denom = 1;	// default value, not used
	for (i = 1; i < n-1; ++i)
	{
		u[i] = p[i] / denom;
		denom = q[i] - p[i] * u[i];
		v[i] = (r[i] - p[i] * v[i-1]) / denom;
	}
	// u[n-1] = 0;	// not necessary, since we multiply it by 0

	// Backsubstitute for b.
	b[n-1] = 0;
	for(i = n-2; i >= 1; --i)
	{
		b[i] = v[i] - u[i+1] * b[i+1];
	}
	b[0] = 0;

	// Solve for the other coefficients from b.
	for(i = 0; i < n-1; ++i)
	{
		a[i] = (b[i+1] - b[i]) / (3 * h[i]);
		c[i] = (3 * k[i] - (b[i+1] + 2 * b[i]) * h[i] * h[i]) / (3 * h[i]);
		d[i] = y[i];
	}
}
*/

/*	Modified version from Numerical Recipes.  The coefficient calculation runs around twice 
	as fast as ours used to, but the evaluation step is more than twice as slow, so it's slower 
	overall.

void SPLINE_ComputeCoeffs(const float *x, const float *y, int n, float *ddy)
{
	int	i;
	float u[100], sig, p, qn, un;

	ddy[0]=0;
	u[0]=0;

	for (i = 1; i <n - 1; ++i)
	{
		sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p = sig * ddy[i-1]+2;
		ddy[i]=(sig-1.0f)/p;
		u[i]=(6.0f*((y[i+1]-y[i])/(x[i+1]-x[i])-(y[i]-y[i-1])/(x[i]-x[i-1]))
				/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}

	qn=0;
	un=0;

	ddy[n-1] = (un-qn*u[n-2])/(qn*ddy[n-2]+1.0f);
	for (i = n - 2; i > -1; --i)
	{
		ddy[i]=ddy[i]*ddy[i+1]+u[i];
	}
}*/
