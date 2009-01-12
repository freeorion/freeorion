/*
---------------------------------------------------------------------------------

	TerrainRayTest.h

	Project(s):
	TerrainRayTest Project

	Author:
	Paul Edmondson

	Description:
	Interface for the RayTester class

	Notes:
	None at this time.

	Known Issues:
	None at this time.

---------------------------------------------------------------------------------
*/

#ifndef __TERRAINRAYTEST__
#define __TERRAINRAYTEST__

#if _MSC_VER > 1000
#pragma once
#endif


// This is so that everything can be changed to double precision if needed:

#define TRT_DOUBLE_PRECISION


// This controls whether or not the data set is transformed or just the query values are transformed
//	into the local coordinate system. This should probably be left commented out unless the entire
//	pipeline is double precision

//#define TRT_TRANSFORM_DATA


// You may want to get better performance by precomputing normals, at the expense of a little
//	extra computation time at load and more memory consumption. If you want the normals pre-computed,
//	define the following:

#define TRT_PRECOMPUTE_NORMALS


// If you do not precompute the normals, you may not want the tester to normalize the collision
//	normals if you have to rescale them later. This will save you a little computation for every
//	collision. If you want the ray test normals pre-normalized, define the following:

//#define TRT_NORMALIZE


// Set up the typedef for floating point values
#include <float.h>
#ifdef TRT_DOUBLE_PRECISION
	typedef double TRTScalar;
	#define	TRT_INFINITY	DBL_MAX
#else
	typedef float TRTScalar;
	#define	TRT_INFINITY	FLT_MAX
#endif


// The structure for raytest results
struct RayTestInfo {
	bool hitOccurred;				// Infinite ray test collission
	TRTScalar t;					// Normal scale to intersect point
	TRTScalar pos[3];				// Intersect point
	TRTScalar norm[3];
};


// The ray tester object
class RayTester{

public:

	RayTester();						// simple constructor
	~RayTester();						// destructor

	void LoadData( char *fname,	TRTScalar xMin=0, TRTScalar xMax=0,
								TRTScalar yMin=0, TRTScalar yMax=0,
								TRTScalar zMin=0, TRTScalar zMax=0 );

	void RayCast( RayTestInfo &results, const TRTScalar *eyePos, const TRTScalar *viewNorm, TRTScalar maxt=TRT_INFINITY ) const;

private:

	int width, height;

	struct GridCell {
		TRTScalar maxy;
		TRTScalar pos[3];

		#ifdef TRT_PRECOMPUTE_NORMALS
			TRTScalar upLeftNorm[3];
			TRTScalar lowRightNorm[3];
		#endif
	};

	GridCell *data;

	bool transformData;

	TRTScalar minx,maxx,xrange,xstep;
	TRTScalar miny,maxy,yrange;
	TRTScalar minz,maxz,zrange,zstep;

	#ifndef TRT_TRANSFORM_DATA
		TRTScalar _xMin,_xRange;
		TRTScalar _yMin,_yRange;
		TRTScalar _zMin,_zRange;
	#endif

	void RayCastTriangle( RayTestInfo &results, const TRTScalar *eyePos, const TRTScalar *viewNorm, 
							const TRTScalar *vert0, const TRTScalar *vert1, const TRTScalar *vert2 ) const;

	void RectifyResults( RayTestInfo &results ) const;

	void GetNormal( TRTScalar *r, const TRTScalar *u, const TRTScalar *v, const TRTScalar *w ) const;
	void Normalize( TRTScalar *v ) const;

};



#endif
