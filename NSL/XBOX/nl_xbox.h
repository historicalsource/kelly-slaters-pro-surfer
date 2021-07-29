#ifndef NL_XBOX_HEADER
#define NL_XBOX_HEADER

typedef unsigned char   nlUchar;
typedef unsigned short  nlUshort;
typedef unsigned int    nlUint;
typedef unsigned long   nlUlong;
typedef unsigned char   nlUint8;
typedef unsigned short  nlUint16;
typedef unsigned int    nlUint32;
typedef unsigned long   nlUint64;
typedef          char   nlint8;
typedef          short  nlint16;
typedef          int    nlint32;
typedef          long   nlint64;

typedef float nlMatrix4x4[4][4];
typedef float nlVector4d[4];
typedef float nlVector3d[3];

#define TO_NLVECTOR3D(x) (*( (nlVector3d *) &(x) ) ) 
#define NL2XGVEC3(x) (&(XGVECTOR3&)x)

#endif