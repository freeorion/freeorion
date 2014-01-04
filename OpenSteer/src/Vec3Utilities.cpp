/**
 * OpenSteer -- Steering Behaviors for Autonomous Characters
 *
 * Copyright (c) 2002-2005, Sony Computer Entertainment America
 * Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *
 * @author Bjoern Knafla <bknafla@uni-kassel.de>
 */

#include "OpenSteer/Vec3Utilities.h"

// Include assert
#include <cassert>



// Include OpenSteer::clamp
#include "OpenSteer/Utilities.h"

/**
 * @todo Is this useful?
std::pair< Vec3, Vec3 >
OpenSteer::convertPointAndSegmentToVectors( const Vec3& point,
                                 const Vec3& segmentPoint0,
                                 const Vec3& segmentPoint1 )
{
    
    
    
    
}
*/

OpenSteer::Vec3
OpenSteer::nearestPointOnSegment( const Vec3& point,
                                  const Vec3& segmentPoint0,
                                  const Vec3& segmentPoint1 )
{
    // convert the test point to be "local" to ep0
    Vec3 const local( point - segmentPoint0 );
    
    // find the projection of "local" onto "segmentNormal"
    Vec3 const segment( segmentPoint1 - segmentPoint0 );
    float const segmentLength( segment.length() );
    
    assert( 0 != segmentLength && "Segment mustn't be of length zero." );
    
    Vec3 const segmentNormalized( segment / segmentLength ); 
    float segmentProjection = segmentNormalized.dot (local);
    
    segmentProjection = clamp( segmentProjection, 0.0f, segmentLength );
    
    Vec3 result( segmentNormalized * segmentProjection );
    result +=  segmentPoint0;
    return result;    
    
}



float 
OpenSteer::pointToSegmentDistance ( const Vec3& point,
                                    const Vec3& segmentPoint0,
                                    const Vec3& segmentPoint1)
{
    return distance( point, nearestPointOnSegment( point, segmentPoint0, segmentPoint1 ) );
}


