/*
---------------------------------------------------------------------------------

	TerrainRayTest.cpp

	Project(s):
	TerrainRayTest Project

	Author:
	Paul Edmondson

	Description:
	Implementation for the RayTester class

	Notes:
	None at this time.

	Known Issues:
	None at this time.

---------------------------------------------------------------------------------
*/
#include "TerrainRayTest.h"


#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <memory.h>


// To include OpenSteer::maxXXX instead of using __max
#include "OpenSteer/Utilities.h"


using namespace std;

//#include "util.h"




RayTester::RayTester() : data(NULL) {
}


RayTester::~RayTester() {
	if( data!=NULL )
		free( data );
	data=NULL;
}


void RayTester::LoadData( char *fname,	TRTScalar xMin, TRTScalar xMax,
										TRTScalar yMin, TRTScalar yMax,
										TRTScalar zMin, TRTScalar zMax ) {
	FILE *inf=fopen(fname,"rb");

	fread(&width,sizeof(width),1,inf);
	fread(&height,sizeof(height),1,inf);

	data = (GridCell *)calloc( width*height, sizeof(GridCell) );

	int x,y,curVert;
	float tempVert[3];

	maxx=-TRT_INFINITY;
	minx=TRT_INFINITY;
	maxy=-TRT_INFINITY;
	miny=TRT_INFINITY;
	maxz=-TRT_INFINITY;
	minz=TRT_INFINITY;

	for(y=0, curVert=0; y<height; ++y)
		for(x=0; x<width; x++, ++curVert){
			fread(tempVert,sizeof(float),3,inf);
			data[curVert].pos[0]=(TRTScalar)tempVert[0];
			data[curVert].pos[1]=(TRTScalar)tempVert[1];
			data[curVert].pos[2]=(TRTScalar)tempVert[2];

			if( data[curVert].pos[0]<minx )
				minx=data[curVert].pos[0];
			if( data[curVert].pos[0]>maxx )
				maxx=data[curVert].pos[0];
			if( data[curVert].pos[1]<miny )
				miny=data[curVert].pos[1];
			if( data[curVert].pos[1]>maxy )
				maxy=data[curVert].pos[1];
			if( data[curVert].pos[2]<minz )
				minz=data[curVert].pos[2];
			if( data[curVert].pos[2]>maxz )
				maxz=data[curVert].pos[2];
		}

	fclose(inf);

	xrange=maxx-minx;
	yrange=maxy-miny;
	zrange=maxz-minz;

	if( ( transformData = (xMin!=xMax && yMin!=yMax && zMin!=zMax) ) ) {
		#ifdef TRT_TRANSFORM_DATA
			TRTScalar xNewRange = xMax-xMin;
			TRTScalar yNewRange = yMax-yMin;
			TRTScalar zNewRange = zMax-zMin;
			for(y=0, curVert=0; y<height; ++y)
				for(x=0; x<width; x++, ++curVert){
					fread(tempVert,sizeof(float),3,inf);
					data[curVert].pos[0] = xNewRange*((data[curVert].pos[0]-minx)/xrange)+xMin;
					data[curVert].pos[1] = yNewRange*((data[curVert].pos[1]-miny)/yrange)+yMin;
					data[curVert].pos[2] = zNewRange*((data[curVert].pos[2]-minz)/zrange)+zMin;
				}

			minx=xMin;
			maxx=xMax;
			miny=yMin;
			maxy=yMax;
			minz=zMin;
			maxz=zMax;
			xrange=xNewRange;
			yrange=yNewRange;
			zrange=zNewRange;
		#else
			_xMin=xMin;
			_xRange=xMax-xMin;
			_yMin=yMin;
			_yRange=yMax-yMin;
			_zMin=zMin;
			_zRange=zMax-zMin;
		#endif
	}

	xstep=xrange/(width-1);
	zstep=zrange/(height-1);

	for(y=0, curVert=0; y<height; ++y)
		for(x=0; x<width; x++, ++curVert)
			if( x<width-1 && y<height-1) {
				data[curVert].maxy = OpenSteer::maxXXX( OpenSteer::maxXXX( data[curVert].pos[1], data[curVert+1].pos[1] ), 
											OpenSteer::maxXXX( data[curVert+width].pos[1], data[curVert+width+1].pos[1] ) );

				#ifdef TRT_PRECOMPUTE_NORMALS
					GetNormal( data[curVert].upLeftNorm, data[curVert].pos, data[curVert+width].pos, data[curVert+1].pos );
					Normalize( data[curVert].upLeftNorm );

					GetNormal( data[curVert].lowRightNorm, data[curVert+width+1].pos, data[curVert+1].pos, data[curVert+width].pos );
					Normalize( data[curVert].lowRightNorm );
				#endif
			}

}


void RayTester::RayCast( RayTestInfo &results, const TRTScalar *eyePos, const TRTScalar *viewNorm, TRTScalar maxt ) const {

	TRTScalar realEyePos[3], realViewNorm[3];

	#ifndef TRT_TRANSFORM_DATA
		if( transformData ) {
			realEyePos[0] = xrange*((eyePos[0]-_xMin)/_xRange)+minx;
			realEyePos[1] = yrange*((eyePos[1]-_yMin)/_yRange)+miny;
			realEyePos[2] = zrange*((eyePos[2]-_zMin)/_zRange)+minz;
			realViewNorm[0] = xrange*viewNorm[0]/_xRange;
			realViewNorm[1] = yrange*viewNorm[1]/_yRange;
			realViewNorm[2] = zrange*viewNorm[2]/_zRange;
		}
		else
	#endif
		{
			realEyePos[0]=eyePos[0];
			realEyePos[1]=eyePos[1];
			realEyePos[2]=eyePos[2];
			realViewNorm[0]=viewNorm[0];
			realViewNorm[1]=viewNorm[1];
			realViewNorm[2]=viewNorm[2];
		}

	// find the initial grid cell
	int xidx=(int)((realEyePos[0]-minx)/xstep);
	int zidx=(int)((realEyePos[2]-minz)/zstep);

	// need starting point of trace for intersection test
	TRTScalar lasty=realEyePos[1];
	TRTScalar newLasty=lasty;

	int idx = xidx+zidx*width;			// the current grid cell
	TRTScalar tval,tval1=-1,tval2=-1;	// t parameter values for intersection
	bool mustTest;						// flag for whether or not we test against triangles

	// for speed, perform computations according to quadrant

	if( realViewNorm[0]>0 ) {				// Moving to the right
		if( realViewNorm[2]>0 ) {			// ... and moving downwards

			// Out-of-bounds check
			if( xidx<0 || zidx<0 ) {
				tval1=( minx-realEyePos[0] )/realViewNorm[0];
				tval2=( minz-realEyePos[2] )/realViewNorm[2];

				if( tval1 > tval2 ){		// Hits left edge of terrain
					tval=tval1;
					xidx=0;
					zidx=(int)((realEyePos[2]+tval*realViewNorm[2]-minz)/zstep);
				} else {					// Hits top edge of terrain
					tval=tval2;
					xidx=(int)((realEyePos[0]+tval*realViewNorm[0]-minx)/xstep);
					zidx=0;
				}

				if( tval>maxt || tval<0 ) {		// Check if maxt value surpassed
					results.hitOccurred=false;
					return;
				}

				lasty = realEyePos[1]+tval*realViewNorm[1];
				idx = xidx+zidx*width;
			}

			while( xidx<width-1 && zidx<height-1 ) {

				// only compute intersection if we have to -- we may have to test triangles anyway
				if( !( mustTest=(lasty<data[idx].maxy) ) ) {
					tval1=( data[idx+width+1].pos[0]-realEyePos[0] )/realViewNorm[0];
					tval2=( data[idx+width+1].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( tval1 < tval2 ){		// Hits right edge
						xidx++;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx++;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}

				// if the mustTest is true, the ray intersects the y-bounds of the cell
				if( mustTest ) {
					RayCastTriangle( results, realEyePos, realViewNorm, data[idx].pos, data[idx+width].pos, data[idx+1].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].upLeftNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}

					RayCastTriangle( results, realEyePos, realViewNorm, data[idx+width+1].pos, data[idx+1].pos, data[idx+width].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].lowRightNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}
				}

				// we didn't intersect -- so update the index
				if( lasty<data[idx].maxy ) {	// in this case, we haven't updated xidx or zidx yet
					tval1=( data[idx+width+1].pos[0]-realEyePos[0] )/realViewNorm[0];
					tval2=( data[idx+width+1].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( tval1 < tval2 ){		// Hits right edge
						xidx++;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx++;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}
				idx = xidx+zidx*width;
				lasty=newLasty;

			}

		} else {						// ... else moving upwards

			// Out-of-bounds check
			if( xidx<0 || zidx>=height-1 ) {
				tval1=( minx-realEyePos[0] )/realViewNorm[0];
				if( realViewNorm[2]!=0 )
					tval2=( maxz-realEyePos[2] )/realViewNorm[2];

				if( realViewNorm[2]==0 || tval1 > tval2 ){		// Hits left edge of terrain
					tval=tval1;
					xidx=0;
					zidx=(int)((realEyePos[2]+tval*realViewNorm[2]-minz)/zstep);
				} else {					// Hits bottom edge of terrain
					tval=tval2;
					xidx=(int)((realEyePos[0]+tval*realViewNorm[0]-minx)/xstep);
					zidx=height-1;
				}

				if( tval>maxt || tval<0 ) {		// Check if maxt value surpassed
					results.hitOccurred=false;
					return;
				}

				lasty = realEyePos[1]+tval*realViewNorm[1];
				idx = xidx+zidx*width;
			}

			while( xidx<width-1 && zidx>=0 ) {

				// only compute intersection if we have to -- we may have to test triangles anyway
				if( !( mustTest=(lasty<data[idx].maxy) ) ) {
					tval1 = ( data[idx+1].pos[0]-realEyePos[0] )/realViewNorm[0];
					if( realViewNorm[2]!=0 )
						tval2 = ( data[idx+1].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( realViewNorm[2]==0 || tval1 < tval2 ){		// Hits right edge
						xidx++;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx--;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}

				// if the mustTest is true, the ray intersects the y-bounds of the cell
				if( mustTest ) {
					RayCastTriangle( results, realEyePos, realViewNorm, data[idx].pos, data[idx+width].pos, data[idx+1].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].upLeftNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}

					RayCastTriangle( results, realEyePos, realViewNorm, data[idx+width+1].pos, data[idx+1].pos, data[idx+width].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].lowRightNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}
				}

				// we didn't intersect -- so update the index
				if( lasty<data[idx].maxy ) {	// in this case, we haven't updated xidx or zidx yet
					tval1 = ( data[idx+1].pos[0]-realEyePos[0] )/realViewNorm[0];
					if( realViewNorm[2]!=0 )
						tval2 = ( data[idx+1].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( realViewNorm[2]==0 || tval1 < tval2 ){		// Hits right edge
						xidx++;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx--;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}
				idx = xidx+zidx*width;
				lasty=newLasty;
			}
		}
	} else {							// else moving to the left
		if( realViewNorm[2]>0 ) {			// ... and moving downwards

			// Out-of-bounds check
			if( xidx>=width-1 || zidx<0 ) {
				if( realViewNorm[0]!=0 )
					tval1=( maxx-realEyePos[0] )/realViewNorm[0];
				tval2=( minz-realEyePos[2] )/realViewNorm[2];

				if( realViewNorm[0]!=0 && tval1 > tval2 ){		// Hits right edge of terrain
					tval=tval1;
					xidx=width-1;
					zidx=(int)((realEyePos[2]+tval*realViewNorm[2]-minz)/zstep);
				} else {					// Hits top edge of terrain
					tval=tval2;
					xidx=(int)((realEyePos[0]+tval*realViewNorm[0]-minx)/xstep);
					zidx=0;
				}

				if( tval>maxt || tval<0 ) {		// Check if maxt value surpassed
					results.hitOccurred=false;
					return;
				}

				lasty = realEyePos[1]+tval*realViewNorm[1];
				idx = xidx+zidx*width;
			}

			while( xidx>=0 && zidx<height-1 ) {

				// only compute intersection if we have to -- we may have to test triangles anyway
				if( !( mustTest=(lasty<data[idx].maxy) ) ) {
					if( realViewNorm[0]!=0 )
						tval1 = ( data[idx+width].pos[0]-realEyePos[0] )/realViewNorm[0];
					tval2 = ( data[idx+width].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( realViewNorm[0]!=0 && tval1 < tval2 ){		// Hits right edge
						xidx--;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx++;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}

				// if the mustTest is true, the ray intersects the y-bounds of the cell
				if( mustTest ) {
					RayCastTriangle( results, realEyePos, realViewNorm, data[idx].pos, data[idx+width].pos, data[idx+1].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].upLeftNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}

					RayCastTriangle( results, realEyePos, realViewNorm, data[idx+width+1].pos, data[idx+1].pos, data[idx+width].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].lowRightNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}
				}

				// we didn't intersect -- so update the index
				if( lasty<data[idx].maxy ) {	// in this case, we haven't updated xidx or zidx yet
					if( realViewNorm[0]!=0 )
						tval1 = ( data[idx+width].pos[0]-realEyePos[0] )/realViewNorm[0];
					tval2 = ( data[idx+width].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( realViewNorm[0]!=0 && tval1 < tval2 ){		// Hits right edge
						xidx--;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx++;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}
				idx = xidx+zidx*width;
				lasty=newLasty;
			}
		} else{							// ... else moving upwards

			// Out-of-bounds check
			if( xidx>=width-1 || zidx>=height-1 ) {
				if( realViewNorm[0]!=0 )
					tval1=( maxx-realEyePos[0] )/realViewNorm[0];
				if( realViewNorm[2]!=0 )
					tval2=( maxz-realEyePos[2] )/realViewNorm[2];

				if( realViewNorm[2]==0 || ( realViewNorm[0]!=0 && tval1 > tval2 ) ){		// Hits right edge of terrain
					tval=tval1;
					xidx=width-1;
					zidx=(int)((realEyePos[2]+tval*realViewNorm[2]-minz)/zstep);
				} else {					// Hits bottom edge of terrain
					tval=tval2;
					xidx=(int)((realEyePos[0]+tval*realViewNorm[0]-minx)/xstep);
					zidx=height-1;
				}

				if( tval>maxt || tval<0 ) {		// Check if maxt value surpassed
					results.hitOccurred=false;
					return;
				}

				lasty = realEyePos[1]+tval*realViewNorm[1];
				idx = xidx+zidx*width;
			}

			while( xidx>=0 && zidx>=0 ) {

				// only compute intersection if we have to -- we may have to test triangles anyway
				if( realViewNorm[0]==0 && realViewNorm[2]==0 )
					mustTest=true;
                else if( !( mustTest=(lasty<data[idx].maxy) ) ) {
					if( realViewNorm[0]!=0 )
						tval1 = ( data[idx].pos[0]-realEyePos[0] )/realViewNorm[0];
					if( realViewNorm[2]!=0 )
						tval2 = ( data[idx].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( realViewNorm[2]==0 || ( realViewNorm[0]!=0 && tval1 < tval2 ) ){		// Hits right edge
						xidx--;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx--;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}

				// if the mustTest is true, the ray intersects the y-bounds of the cell
				if( mustTest ) {
					RayCastTriangle( results, realEyePos, realViewNorm, data[idx].pos, data[idx+width].pos, data[idx+1].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].upLeftNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}

					RayCastTriangle( results, realEyePos, realViewNorm, data[idx+width+1].pos, data[idx+1].pos, data[idx+width].pos );
					if( results.hitOccurred ){
						if( results.t>maxt ) {		// Check if maxt value surpassed
							results.hitOccurred=false;
							return;
						}
						#ifdef TRT_PRECOMPUTE_NORMALS
							memcpy( results.norm, data[idx].lowRightNorm, sizeof(TRTScalar)*3 );
						#endif
						RectifyResults( results );
						return;
					}

					if( realViewNorm[0]==0 && realViewNorm[2]==0 )
						return;
				}

				// we didn't intersect -- so update the index
				if( lasty<data[idx].maxy ) {	// in this case, we haven't updated xidx or zidx yet
					tval1 = ( data[idx].pos[0]-realEyePos[0] )/realViewNorm[0];
					tval2 = ( data[idx].pos[2]-realEyePos[2] )/realViewNorm[2];

					if( realViewNorm[2]==0 || ( realViewNorm[0]!=0 && tval1 < tval2 ) ){		// Hits right edge
						xidx--;
						tval=tval1;
					} else {					// Hits bottom edge
						zidx--;
						tval=tval2;
					}

					if( tval>maxt ) {		// Check if maxt value surpassed
						results.hitOccurred=false;
						return;
					}

					newLasty=realEyePos[1]+tval*realViewNorm[1];
					mustTest=( newLasty<data[idx].maxy );
				}
				idx = xidx+zidx*width;
				lasty=newLasty;
			}
		}
	}

	results.hitOccurred = false;
}


void RayTester::RayCastTriangle( RayTestInfo &results, const TRTScalar *eyePos, const TRTScalar *viewNorm,
									const TRTScalar *vert0, const TRTScalar *vert1, const TRTScalar *vert2 ) const {
	// Code taken from http://www.acm.org/jgt/papers/MollerTrumbore97/code.html

	#define EPSILON 0.000001
	#define CROSS(dest,v1,v2) \
			dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
			dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
			dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
	#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
	#define SUB(dest,v1,v2) \
			dest[0]=v1[0]-v2[0]; \
			dest[1]=v1[1]-v2[1]; \
			dest[2]=v1[2]-v2[2]; 

	TRTScalar edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	TRTScalar det;//,inv_det;
	TRTScalar u, v;

	/* find vectors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pvec, viewNorm, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pvec);

	if (det < EPSILON) {
		results.hitOccurred=false;
		return;
	}

	/* calculate distance from vert0 to ray eyePosin */
	SUB(tvec, eyePos, vert0);

	/* calculate U parameter and test bounds */
	u = DOT(tvec, pvec);
	if (u < 0.0 || u > det) {
		results.hitOccurred=false;
		return;
	}

	/* prepare to test V parameter */
	CROSS(qvec, tvec, edge1);

	/* calculate V parameter and test bounds */
	v = DOT(viewNorm, qvec);
	if (v < 0.0 || u + v > det) {
		results.hitOccurred=false;
		return;
	}

	/* calculate t, scale parameters, ray intersects triangle */
//	t = DOT(edge2, qvec);		// We don't need the uv coords, so this is commented out
//	inv_det = 1.0 / det;
//	t *= inv_det;
//	u *= inv_det;
//	v *= inv_det;

	results.hitOccurred=true;
	results.t = DOT(edge2, qvec) / det;
	results.pos[0]=eyePos[0]+results.t*viewNorm[0];
	results.pos[1]=eyePos[1]+results.t*viewNorm[1];
	results.pos[2]=eyePos[2]+results.t*viewNorm[2];

#ifndef TRT_PRECOMPUTE_NORMALS
	GetNormal( results.norm, vert0, vert1, vert2 );

	#ifdef TRT_NORMALIZE
		Normalize( results.norm );
	#endif
#endif

}


void RayTester::RectifyResults( RayTestInfo &results ) const {
	#ifndef TRT_TRANSFORM_DATA
		if( transformData ) {
			results.pos[0] = _xRange*((results.pos[0]-minx)/xrange)+_xMin;
			results.pos[1] = _yRange*((results.pos[1]-miny)/yrange)+_yMin;
			results.pos[2] = _zRange*((results.pos[2]-minz)/zrange)+_zMin;
			results.norm[0] = _xRange*results.norm[0]/xrange;
			results.norm[1] = _yRange*results.norm[1]/yrange;
			results.norm[2] = _zRange*results.norm[2]/zrange;
		}
	#endif
}


void RayTester::GetNormal( TRTScalar *r, const TRTScalar *u, const TRTScalar *v, const TRTScalar *w) const {
	static TRTScalar vx,vy,vz,wx,wy,wz;

	vx=v[0]-u[0];
	wx=w[0]-u[0];
	vy=v[1]-u[1];
	wy=w[1]-u[1];
	vz=v[2]-u[2];
	wz=w[2]-u[2];
	r[0]=vy*wz-wy*vz;
	r[1]=vz*wx-wz*vx;
	r[2]=vx*wy-wx*vy;
}


void RayTester::Normalize( TRTScalar *v ) const {
	#ifdef TRT_DOUBLE_PRECISION
		TRTScalar mag = sqrt( v[0]*v[0]+v[1]*v[1]+v[2]*v[2] );
	#else
		TRTScalar mag = sqrtf( v[0]*v[0]+v[1]*v[1]+v[2]*v[2] );
	#endif

	v[0]/=mag;
	v[1]/=mag;
	v[2]/=mag;
}
