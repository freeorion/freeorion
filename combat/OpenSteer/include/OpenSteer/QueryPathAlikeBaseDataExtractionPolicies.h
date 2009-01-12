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
 * @file
 *
 * @author Bjoern Knafla <bknafla@uni-kassel.de>
 *
 * Declarations of policies used by @c OpenSteer::mapPointToPathAlike and
 * @c OpenSteer::mapDistanceToPathAlike to extract informations of path alikes.
 */
#ifndef OPENSTEER_QUERYPATHALIKEBASEDATAEXTRACTIONPOLICY_H
#define OPENSTEER_QUERYPATHALIKEBASEDATAEXTRACTIONPOLICY_H

namespace OpenSteer {
    
    /**
     * Extracts the base data like the segment distance, the radius, the
     * distance of the query point to the path alike, the point on the path
     * alike center line and the tangent at that point.
     *
     * Specialize it for the path alike to use and provide a static member
     * function with the following signature:
     *
     * <code>static void extract( PathAlike const& pathAlike, typename PathAlike::size_type segmentIndex, Vec3 const& point, float& segmentDistance, float& radius, float& distancePointToPath, Vec3& pointOnPathCenterLine, Vec3& tangent )</code>
     *
     * @attention Be aware of the references that are passed in.
     */
    template< class PathAlike >
    class PointToPathAlikeBaseDataExtractionPolicy;
    
    
    /**
     * Extracts the base data like the radius, the point on the path
     * alike center line and the tangent at that point.
     *
     * Specialize it for the path alike to use and provide a static member
     * function with the following signature:
     *
     * <code>static void extract( PathAlike const& pathAlike, typename PathAlike::size_type segmentIndex, float segmentDistance, Vec3& pointOnPathCenterLine, Vec3& tangent, float& radius )</code>
     *
     * @attention Be aware of the references that are passed in.
     */    
    template< class PathAlike >
    class DistanceToPathAlikeBaseDataExtractionPolicy;
    
} // namespace OpenSteer


#endif // OPENSTEER_QUERYPATHALIKEBASEDATAEXTRACTIONPOLICY_H
