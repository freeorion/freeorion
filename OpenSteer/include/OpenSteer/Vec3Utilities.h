// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2005, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
// Utilities to work with Vec3.
//
// 05-12-05 bk:  Created based on code of PolylinePathway.
//
// ----------------------------------------------------------------------------

#ifndef OPENSTEER_VEC3UTILITIES_H
#define OPENSTEER_VEC3UTILITIES_H


// Include OpenSteer::Vec3
#include "OpenSteer/Vec3.h"

// Include OpenSteer::size_t
#include "OpenSteer/StandardTypes.h"

// Include OpenSteer::equalsRelative
#include "OpenSteer/Utilities.h"



namespace OpenSteer {

    /**
     * Returns the nearest point on the segment @a segmentPoint0 to 
     * @a segmentPoint1 from @a point.
     */
    OpenSteer::Vec3  nearestPointOnSegment( const Vec3& point,
                                            const Vec3& segmentPoint0,
                                            const Vec3& segmentPoint1 );
    
    /**
     * Computes minimum distance from @a point to the line segment defined by
     * @a segmentPoint0 and @a segmentPoint1.
     */
    float pointToSegmentDistance( const Vec3& point,
                                  const Vec3& segmentPoint0,
                                  const Vec3& segmentPoint1);
        
    /**
     * Retuns distance between @a a and @a b.
     */
    inline float distance (const Vec3& a, const Vec3& b) {
        return (a-b).length();
    } 
    
    
    /**
     * Elementwise relative tolerance comparison of @a lhs and @a rhs taking
     * the range of the elements into account.
     *
     * See Christer Ericson, Real-Time Collision Detection, Morgan Kaufmann, 
     * 2005, pp. 441--443.
     *
     * @todo Rewrite using the stl or providing an own range based function.
     */
    inline
    bool
    equalsRelative( Vec3 const& lhs, 
                     Vec3 const& rhs, 
                     float const& tolerance = std::numeric_limits< float >::epsilon()  ) {
        return equalsRelative( lhs.x, rhs.x, tolerance ) && equalsRelative( lhs.y, rhs.y ) && equalsRelative( lhs.z, rhs.z );
    }
    
} // namespace OpenSteer

#endif // OPENSTEER_VEC3UTILITIES_H
