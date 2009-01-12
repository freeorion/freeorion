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
 * Functionality to map points or distances to path alikes and to extract
 * the assocaited information.
 */
#ifndef OPENSTEER_QUERYPATHALIKE_H
#define OPENSTEER_QUERYPATHALIKE_H


// Include std::numeric_limits< float >::max
#include <limits>



// Include OpenSteer::Vec3
#include "OpenSteer/Vec3.h"

// Include OpenSteer::distance
#include "OpenSteer/Vec3Utilities.h"

// Include OpenSteer::clamp, OpenSteer::modulo
#include "OpenSteer/Utilities.h"

// Include OpenSteer::PointToPathAlikeBaseDataExtractionPolicy, OpenSteer::DistanceToPathAlikeBaseDataExtractionPolicy
#include "OpenSteer/QueryPathAlikeBaseDataExtractionPolicies.h"

#ifdef _MSC_VER
#undef min
#undef max
#endif


namespace OpenSteer {

    /**
     * Provides functionality to map points or distances to path alikes and
     * to extract associated information.
     */
    template< class PathAlike, class Mapping, class BaseDataExtractionPolicy = PointToPathAlikeBaseDataExtractionPolicy< PathAlike > >
    class PointToPathAlikeMapping {
    public:
        
        /**
         * Maps @a queryPoint to a path alike @a pathAlike and returns the  
         * queried data in @a mapping.
         *
         * @c Mapping must provide the following member functions to
         * set queried values. Member functions might be empty if the specific 
         * data isn't needed. A good compiler should optimize the
         * empty member function calls and the associated calculations for its
         * parameters away.
         *
         * <code> void setPointOnPathCenterLine( Vec3 const& ) </code>
         * <code> void setPointOnPathBoundary( Vec3 const& ) </code>
         * <code> void setRadius( float ) </code>
         * <code> void setTangent( Vec3 const& ) </code>
         * <code> void setSegmentIndex( typename SegmentedPathAlike::size_type ) </code>
         * <code> void setDistancePointToPath( float ) </code>
         * <code> void setDistancePointToPathCenterLine( float ) </code>
         * <code> void setDistanceOnPath( float ) </code>
         * <code> void setDistanceOnSegment( float ) </code>
         *
         * To query for the distance on the path the two following member functions
         * of @c Mapping must be provided too:
         *
         * <code> void setDistanceOnPathFlag( float ) </code> and
         * <code> float distanceOnPathFlag() const </code>.
         *
         * If the distance along the path shouldn't be extracted empty 
         * versions of these member functions are possible.
         *
         * @c QueryPathAlikeUtilities.h provides some base classes to inherit
         * from to automatically get some of the functionality described above.
         */
        static void map( PathAlike const& pathAlike, Vec3 const& queryPoint, Mapping& mapping ) {
            float minDistancePointToPath = std::numeric_limits< float >::max();
            mapping.setDistanceOnPathFlag( 0.0f );
            
            typedef typename PathAlike::size_type size_type;
            size_type const segmentCount = pathAlike.segmentCount();
            for ( size_type segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex ) {
                
                float segmentDistance = 0.0f;
                float radius = 0.0f;
                float distancePointToPath = 0.0f;
                Vec3 pointOnPathCenterLine( 0.0f, 0.0f, 0.0f );
                Vec3 tangent( 0.0f, 0.0f, 0.0f );
                
                BaseDataExtractionPolicy::extract( pathAlike, segmentIndex, queryPoint, segmentDistance, radius, distancePointToPath, pointOnPathCenterLine, tangent );
                
                if ( distancePointToPath < minDistancePointToPath ) {
                    minDistancePointToPath = distancePointToPath;
                    mapping.setPointOnPathCenterLine( pointOnPathCenterLine );
                    mapping.setPointOnPathBoundary( pointOnPathCenterLine + ( ( queryPoint - pointOnPathCenterLine ).normalize() * radius ) );
                    mapping.setRadius( radius );
                    mapping.setTangent( tangent );
                    mapping.setSegmentIndex( segmentIndex );
                    mapping.setDistancePointToPath( distancePointToPath );
                    mapping.setDistancePointToPathCenterLine( distancePointToPath + radius );
                    mapping.setDistanceOnPath( mapping.distanceOnPathFlag() + segmentDistance );
                    mapping.setDistanceOnSegment( segmentDistance );
                }
                
                mapping.setDistanceOnPathFlag( mapping.distanceOnPathFlag() + pathAlike.segmentLength( segmentIndex ) );
            }
        }
        
    }; // class PointToPathAlikeMapping
    
    /**
     * Maps @a point to @a pathAlike and returns the data extracted in 
     * @a mapping.
     *
     * See @c MapPointToPathAlike::map for further information.
     */
    template< class PathAlike, class Mapping >
    void mapPointToPathAlike( PathAlike const& pathAlike, Vec3 const& point, Mapping& mapping ) {
        PointToPathAlikeMapping< PathAlike, Mapping >::map( pathAlike, point, mapping );
    }
    
        
    
    /**
     * Provides functionality to map distances to path alikes and to extract
     * the associated data.
     */
    template< class PathAlike, class Mapping, class BaseDataExtractionPolicy = DistanceToPathAlikeBaseDataExtractionPolicy< PathAlike > > 
    class DistanceToPathAlikeMapping {
    public:
    
        /**
         * Maps @a distanceOnPath to a path alike @a pathAlike and returns the 
         * queried data in @a mapping.
         *
         * @c Mapping must provide the following member functions 
         * to set queried values. Member functions might be empty if the  
         * specific data isn't needed. A good compiler should optimize the
         * empty member function calls and the associated calculations for its
         * parameters away.
         *
         * <code> void setPointOnPathCenterLine( Vec3 const& ) </code>
         * <code> void setRadius( float ) </code>
         * <code> void setTangent( Vec3 const& ) </code>
         * <code> void setSegmentIndex( typename SegmentedPathAlike::size_type ) </code>
         * <code> void setDistanceOnPath( float ) </code>
         * <code> void setDistanceOnSegment( float ) </code>
         */
        static void map( PathAlike const& pathAlike, float distanceOnPath, Mapping& mapping ) {
            float const pathLength = pathAlike.length();
            
            // Modify @c distanceOnPath to applicable values.
            if ( pathAlike.isCyclic() ) {
                distanceOnPath = modulo( distanceOnPath, pathLength );       
            }
            distanceOnPath = clamp( distanceOnPath, 0.0f, pathLength );
            
            // Which path alike segment is reached by @c distanceOnPath?
            float remainingDistance = distanceOnPath;
            typedef typename PathAlike::size_type size_type;
            size_type segmentIndex = 0;        
            size_type const maxSegmentIndex = pathAlike.segmentCount() - 1;
            while( ( segmentIndex < maxSegmentIndex ) && 
                   ( remainingDistance > pathAlike.segmentLength( segmentIndex ) ) ) {
                remainingDistance -= pathAlike.segmentLength( segmentIndex );
                ++segmentIndex;
            }
            
            // Extract the path related data associated with the segment reached
            // by @c distanceOnPath.
            Vec3 pointOnPathCenterLine( 0.0f, 0.0f, 0.0f );
            Vec3 tangent( 0.0f, 0.0f, 0.0f );
            float radius = 0.0f;
            BaseDataExtractionPolicy::extract( pathAlike, segmentIndex, remainingDistance, pointOnPathCenterLine, tangent, radius );
            
            // Store the extracted data in @c mapping to return it to the caller.
            mapping.setPointOnPathCenterLine( pointOnPathCenterLine );
            mapping.setRadius( radius );
            mapping.setTangent( tangent );
            mapping.setSegmentIndex( segmentIndex );
            mapping.setDistanceOnPath( distanceOnPath );
            mapping.setDistanceOnSegment( remainingDistance );            
        }
        
    }; // class DistanceToPathAlikeMapping
    
    
    
    /**
     * Maps @a distance to @a pathAlike and stores the data queried in
     * @a mapping.
     *
     * See @c DistanceToPathAlikeMapping::map for further information.
     */
    template< class PathAlike, class Mapping >
    void mapDistanceToPathAlike( PathAlike const& pathAlike, float distance, Mapping& mapping ) {
        DistanceToPathAlikeMapping< PathAlike, Mapping >::map( pathAlike, distance, mapping );
    }
    
    
} // namespace OpenSteer

#endif // OPENSTEER_QUERYPATHALIKE_H
