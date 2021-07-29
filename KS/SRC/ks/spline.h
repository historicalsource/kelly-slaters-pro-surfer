#ifndef SPLINE_H
#define SPLINE_H

void SPLINE_ComputeCoeffs(const float *x, const float *y, int n, float *a, float *b, float *c, float *d);

/*	Using the spline coefficients computed above, we compute an interpolated value valy for the 
	point valx.  First, we must determine the interval ( x[i],x[i+1] ] which contains valx.  Then, 
	we evaluate the function f[i](valx).  
*/
inline u_int SPLINE_BinarySearch(const float *x, u_int n, const float &valx)
{
	u_int lo = 0, hi = n - 1, mid;
	while (hi - lo > 1)
	{
		mid = (lo + hi) / 2;
		if (x[mid] < valx) lo = mid;
		else hi = mid;
	}

	return lo;
}

inline float SPLINE_Evaluate(const float *x, const float *a, const float *b, const float *c, const float *d, 
	u_int n, float valx)
{
	u_int lo = SPLINE_BinarySearch(x, n, valx);
	valx -= x[lo];
	return ((a[lo] * valx + b[lo]) * valx + c[lo]) * valx + d[lo];
}

/*	Compute the derivative of the interpolating function.
*/
inline float SPLINE_EvaluateD(const float *x, const float *a, const float *b, const float *c, const float *d, 
	u_int n, float valx)
{
	u_int lo = SPLINE_BinarySearch(x, n, valx);
	valx -= x[lo];
	return (3 * a[lo] * valx + 2 * b[lo]) * valx + c[lo];
}

/*	Compute the 2nd derivative of the interpolating function.
*/
inline float SPLINE_EvaluateDD(const float *x, const float *a, const float *b, const float *c, const float *d, 
	u_int n, float valx)
{
	u_int lo = SPLINE_BinarySearch(x, n, valx);
	valx -= x[lo];
	return 6 * a[lo] * valx + 2 * b[lo];
}

/*	Same as above, but the interval has been provided.
*/
inline float SPLINE_Evaluate(const float *x, const float *a, const float *b, const float *c, const float *d, 
	u_int n, float valx, u_int lo)
{
	valx -= x[lo];
	return ((a[lo] * valx + b[lo]) * valx + c[lo]) * valx + d[lo];
}

inline float SPLINE_EvaluateD(const float *x, const float *a, const float *b, const float *c, const float *d, 
	u_int n, float valx, u_int lo)
{
	valx -= x[lo];
	return (3 * a[lo] * valx + 2 * b[lo]) * valx + c[lo];
}

inline float SPLINE_EvaluateDD(const float *x, const float *a, const float *b, const float *c, const float *d, 
	u_int n, float valx, u_int lo)
{
	valx -= x[lo];
	return 6 * a[lo] * valx + 2 * b[lo];
}

/*	Modified version from Numerical Recipes.  The coefficient calculation runs around twice 
	as fast as ours, but the evaluation step is more than twice as slow, so it's slower overall.

inline float SPLINE_Evaluate(const float *x, const float *y, const float *ddy, u_int n, float valx, u_int lo)
{
	u_int up = lo + 1;
	float h = x[up] - x[lo];
//	assert(h);
	float a = (x[up] - valx) / h ;
	float b = (valx - x[lo]) / h ;
	return a*y[lo]+b*y[up]+((a*a*a-a)*ddy[lo]+(b*b*b-b)*ddy[up])*h*h/6.f;
}
*/

#endif