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
 * Segmented pathway build of polylines. The whole path has one associated 
 * radius.
 */
#ifndef OPENSTEER_POLYLINESEGMENTEDPATHWAYSINGLERADIUS_H
#define OPENSTEER_POLYLINESEGMENTEDPATHWAYSINGLERADIUS_H

// Include OpenSteer::SegmentedPathway
#include "OpenSteer/SegmentedPathway.h"

// Include OpenSteer::PolylineSegmentedPath
#include "OpenSteer/PolylineSegmentedPath.h"

// Include OpenSteer::PointToPathAlikeBaseDataExtractionPolicy
#include "OpenSteer/QueryPathAlikeBaseDataExtractionPolicies.h"

// Include OpenSteer::Vec3
#include "OpenSteer/Vec3.h"

// Include OpenSteer::distance
#include "OpenSteer/Vec3Utilities.h"



namespace OpenSteer {
    
    /**
     * Segmented pathway build by polylines with a single radius for the whole
     * pathway.
     */
    class PolylineSegmentedPathwaySingleRadius : public SegmentedPathway {
    public:
        PolylineSegmentedPathwaySingleRadius();
        explicit PolylineSegmentedPathwaySingleRadius( float r );
        PolylineSegmentedPathwaySingleRadius( size_type numOfPoints,
                                              Vec3 const points[],
                                              float r,
                                              bool closeCycle );
        PolylineSegmentedPathwaySingleRadius( PolylineSegmentedPathwaySingleRadius const& other );
        virtual ~PolylineSegmentedPathwaySingleRadius();
        PolylineSegmentedPathwaySingleRadius& operator=( PolylineSegmentedPathwaySingleRadius other );
        
        /**
         * Swaps the content with @a other.
         */
        void swap( PolylineSegmentedPathwaySingleRadius& other );
        
        /**
         * Replaces @a numOfPoints points starting at @a startIndex.
         *
         * In the resulting sequence of points there mustn't be two adjacent 
         * ones that are equal. The first and last point mustn't be identical,
         * too.
         *
         * If the first point is changed and the path is cyclic the duplication
         * of the first point at the end of the sequence representing the
         * path closing segment is updated automatically.
         *
         * @param startIndex First point to be moved or replaced.
         * @param numOfPoints Number of points to move or replace. 
         *                    <code> numOfPoints + startIndex </code> must be
         *                    lesser or equal to @c pointCount.
         * @param newPointValues Moved points to replace the old ones.
         */
        void movePoints( size_type startIndex,
                         size_type numOfPoints,
                         Vec3 const newPointValues[] );
        /**
         * Replaces the pathway information completely.
         *
         * If @a closedCycle is @c true then the pathway has @a numOfPoints
         * segments and the first point is duplicated and added as the last
         * point to represent the end point of the segment closing the pathway
         * cycle. If @a closedCycle is false the pathway has 
         * <code>numOfPoints - 1</code> segments.
         *
         * @param numOfPoints Number of points defining the pathway.
         * @param points The actual points.
         * @param r Radius of the whole pathway.
         * @param closedCycle @c true if the pathway is cyclic, @a false 
         *        otherwise.
         */
        void setPathway( size_type numOfPoints,
                         Vec3 const points[],
                         float r,
                         bool closedCycle );
        
        /**
         * Sets the radius of the whole pathway to @a r.
         */
        void setRadius( float r );
        
        /**
         * Returns the pathway radius.
         */
        float radius() const;
        
        
        virtual bool isValid() const;
		virtual Vec3 mapPointToPath (const Vec3& point,
                                     Vec3& tangent,
                                     float& outside) const;
		virtual Vec3 mapPathDistanceToPoint (float pathDistance) const;
		virtual float mapPointToPathDistance (const Vec3& point) const;
        virtual bool isCyclic() const;
        virtual float length() const;
        
        
        virtual size_type pointCount() const;
        virtual Vec3 point( size_type pointIndex ) const;
        
        
        virtual size_type segmentCount() const;
        virtual float segmentLength( size_type segmentIndex ) const;
        virtual Vec3 segmentStart( size_type segmentIndex ) const;
        virtual Vec3 segmentEnd( size_type segmentIndex ) const;
        virtual float mapPointToSegmentDistance( size_type segmentIndex, 
                                                 Vec3 const& point ) const;
        virtual Vec3 mapSegmentDistanceToPoint( size_type segmentIndex, 
                                                float segmentDistance ) const;
        virtual float mapSegmentDistanceToRadius( size_type segmentIndex, 
                                                 float distanceOnSegment ) const;
        virtual Vec3 mapSegmentDistanceToTangent( size_type segmentIndex, 
                                                  float segmentDistance ) const;
        
        virtual void mapDistanceToSegmentPointAndTangentAndRadius( size_type segmentIndex,
                                                                   float segmentDistance,
                                                                   Vec3& pointOnPath,
                                                                   Vec3& tangent,
                                                                   float& radius ) const;
        
        virtual void mapPointToSegmentDistanceAndPointAndTangentAndRadius( size_type segmentIndex,
                                                                           Vec3 const& point,
                                                                           float& distance,
                                                                           Vec3& pointOnPath,
                                                                           Vec3& tangent,
                                                                           float& radius) const;
         
    private:
        PolylineSegmentedPath path_;
        float radius_;
    }; // class PolylineSegmentedPathwaySingleRadius
    
    
    /**
     * Swaps the content of @a lhs and @a rhs.
     */
    inline void swap( PolylineSegmentedPathwaySingleRadius& lhs, 
               PolylineSegmentedPathwaySingleRadius& rhs ) {
        lhs.swap( rhs );
    }
    
    
    /**
     * Extracts the base data of @c PolylineSegmentedPathwaySingleRadius.
     */
    template<>
    class PointToPathAlikeBaseDataExtractionPolicy< PolylineSegmentedPathwaySingleRadius > {
    public:
            
        static void extract( PolylineSegmentedPathwaySingleRadius const& pathAlike,
                             PolylineSegmentedPathwaySingleRadius::size_type segmentIndex,
                             Vec3 const& point, 
                             float& segmentDistance, 
                             float& radius, 
                             float& distancePointToPath, 
                             Vec3& pointOnPathCenterLine, 
                             Vec3& tangent ) {
            pathAlike.mapPointToSegmentDistanceAndPointAndTangentAndRadius( segmentIndex, point, segmentDistance, pointOnPathCenterLine, tangent, radius );
            distancePointToPath = distance( point, pointOnPathCenterLine ) - radius;
        }
            
    }; // class PointToPathAlikeBaseDataExtractionPolicy
    
    
    /**
     * Extracts the base data of @c PolylineSegmentedPathwaySingleRadius.
     */
    template<>
    class DistanceToPathAlikeBaseDataExtractionPolicy< PolylineSegmentedPathwaySingleRadius > {
    public:
        static void extract( PolylineSegmentedPathwaySingleRadius const& pathAlike,
                             PolylineSegmentedPathwaySingleRadius::size_type segmentIndex,
                             float segmentDistance, 
                             Vec3& pointOnPathCenterLine, 
                             Vec3& tangent, 
                             float& radius )  {
            pathAlike.mapDistanceToSegmentPointAndTangentAndRadius( segmentIndex, segmentDistance, pointOnPathCenterLine, tangent, radius );     
        }
        
        
    }; // DistanceToPathAlikeBaseDataExtractionPolicy 
    
} // namespace OpenSteer


#endif // OPENSTEER_POLYLINESEGMENTEDPATHWAYSINGLERADIUS_H
