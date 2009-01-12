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
#include "OpenSteer/PolylineSegmentedPath.h"

// Include std::accumulate
#include <numeric>

// Include std::swap, std::adjacent_find
#include <algorithm>

// Include assert
#include <cassert>



// Include OpenSteer::Vec3
#include "OpenSteer/Vec3.h"

// Include OpenSteer::mapPointToPathway, OpenSteer::mapDistanceToPointOnPathCenterLine
#include "OpenSteer/QueryPathAlike.h"

// Include OpenSteer::PointToPathMapping, OpenSteer::PathDistanceToPointMapping, OpenSteer::PointToPathDistanceMapping
#include "OpenSteer/QueryPathAlikeMappings.h"

// Include OpenSteer::HasNoRadius
#include "OpenSteer/QueryPathAlikeUtilities.h"

// Include OpenSteer::clamp, OpenSteer::shrinkToFit
#include "OpenSteer/Utilities.h"



namespace {
    
    typedef OpenSteer::SegmentedPath::size_type size_type;
    typedef std::vector< OpenSteer::Vec3 > Vec3Container;
    typedef std::vector< float > FloatContainer;
    
    /**
     * Recalculates the segment tangent and length for segment @a segmentIndex.
     *
     * @attention @a segmentTangents and @a segmentLengths must have the right
     *            size.
     */
    void
    updateSegmentTangentAndLength( size_type segmentIndex,
                                   Vec3Container const& points , 
                                   Vec3Container& segmentTangents, 
                                   FloatContainer& segmentLengths ) 
    {
        assert( ( ( segmentIndex + 1 ) < points.size() ) && 
                "Not enough points for segment segmentIndex." );
        assert( segmentIndex < segmentTangents.size() &&
                "segmentIndex out of range." );
        assert( segmentTangents.size() == segmentLengths.size() && "segmentTangents and segmentLengths must have the same size." );
        
        OpenSteer::Vec3 tangent = points[ segmentIndex + 1 ] - points[ segmentIndex ];
        float const length = tangent.length();
        assert( ! OpenSteer::isZero( length ) && 
                "Segments must have lengths greater than 0." );
        
        tangent /= length;
        
        segmentTangents[ segmentIndex ] = tangent;
        segmentLengths[ segmentIndex] = length;        
    }
    
    
    /**
     * Helper function to calucate the segment tangent and lengths
     * in the given range.
     *
     * @param points points defining the path.
     * @param segmentTangents container to store the calculated segment 
     *                        tangents. Must have the right size.
     * @param segmentLengths container to store the calculated segment lengths.
     *                       Must have the right size.
     * @param firstChangedPointIndex the first point that changed. Segments have 
     *        to be updated starting with it.
     * @param numOfPoints number of points that changed beginning with 
     *        @a startIndex.
     * @param isCyclic Is the path cyclic or not.
     *
     */
    void 
    updateTangentsAndLengths( Vec3Container const& points , 
                              Vec3Container& segmentTangents, 
                              FloatContainer& segmentLengths, 
                              size_type firstChangedPointIndex, 
                              size_type numOfPoints,
                              bool isCyclic )
    {
        assert( 0 < numOfPoints && "Only call if points have really changed." );
        assert( 1 < points.size() && "Two points are needed for a segment." );
        assert( points.size() == segmentTangents.size() + 1 && 
                "Two points are needed for one segment, therefore there must be one segment less than points." );
        assert( segmentTangents.size() == segmentLengths.size() && "segmentTangents and segmentLengths must have the same size." );
        // Other assertions aren't tested because the function should only be
        // called by other functions that guarantee correctness of the 
        // parameters.
        
        
        // The segment with end point @a firstChangedPointIndex has also 
        // changed. Beware from range underflow by subtraction.      
        size_type firstSegmentIndex = firstChangedPointIndex;
        if ( 0 < firstSegmentIndex ) {
            firstSegmentIndex -= 1;
        }
        
        // The last segment to update has the last changed point as its start
        // point. This only holds true if the last changed point isn't the last
        // point.
        // lastSegmentIndex is one greater than the real last segment to update
        // like the last iterators in the stl.
        size_type lastSegmentIndex = firstChangedPointIndex + numOfPoints;
        lastSegmentIndex = OpenSteer::clamp( lastSegmentIndex, static_cast< size_t >( 0 ), segmentTangents.size() );
        
        for ( size_type i = firstSegmentIndex; i < lastSegmentIndex; ++i ) {
            updateSegmentTangentAndLength( i, points, segmentTangents, segmentLengths );
        }
        
        // If path is cyclic and the first point changed and the cycle closing
        // segment hasn't been updated update it now.
        if ( isCyclic && 
             ( 0 == firstSegmentIndex ) && 
             ! ( lastSegmentIndex == segmentTangents.size() ) ) {
            
            updateSegmentTangentAndLength( segmentTangents.size() - 1, 
                                           points, 
                                           segmentTangents, 
                                           segmentLengths );
        }
        
    }
    
    
    /**
     * Checks that no adjacent points are equal. Checks the first and last
     * point if the path is cyclic, too.
     */
    template< typename Iterator >
    bool adjacentPathPointsDifferent( Iterator first, Iterator last, bool closedCycle  ) {
        
        assert( last - first > 1 && "A path needs at least two waypoints." );
        
        if ( last != std::adjacent_find( first, last ) ) {
            return false;
        }
    
        if ( closedCycle ) {
            Iterator before_last( first );
            std::advance( before_last, last - first - 1  );
            
            return *first == *before_last;
        }
        
        return true;
    }
    
    
        
} // anonymous namespace







OpenSteer::PolylineSegmentedPath::PolylineSegmentedPath()
    : points_( 0 ), segmentTangents_( 0 ), segmentLengths_( 0 ), closedCycle_( false )
{
    
}


OpenSteer::PolylineSegmentedPath::PolylineSegmentedPath( size_type numOfPoints,
                                                         Vec3 const newPoints[],
                                                         bool closedCycle )
    : points_( 0 ), segmentTangents_( 0 ), segmentLengths_( 0 ), closedCycle_( closedCycle )
{
        setPath( numOfPoints, newPoints, closedCycle );
}


OpenSteer::PolylineSegmentedPath::PolylineSegmentedPath( PolylineSegmentedPath const& other )
    : SegmentedPath( other ), points_( other.points_ ), segmentTangents_( other.segmentTangents_ ), segmentLengths_( other.segmentLengths_ ), closedCycle_( other.closedCycle_ )
{
    // Nothing to do.
}



OpenSteer::PolylineSegmentedPath::~PolylineSegmentedPath()
{
    // Nothing to do.
}



OpenSteer::PolylineSegmentedPath& 
OpenSteer::PolylineSegmentedPath::operator=( PolylineSegmentedPath other )
{
    swap( other ); 
    return *this;
}



void 
OpenSteer::PolylineSegmentedPath::swap( PolylineSegmentedPath& other )
{
    points_.swap( other.points_ );
    segmentTangents_.swap( other.segmentTangents_ );
    segmentLengths_.swap( other.segmentLengths_ );
    std::swap( closedCycle_, other.closedCycle_ );
}





void 
OpenSteer::PolylineSegmentedPath::setPath( size_type numOfPoints,
                                           Vec3 const newPoints[],
                                           bool closedCycle )
{
    assert( 1 < numOfPoints && "Path must have at least two distinct points." );
    // While testing say that no cyclus is used because the first point hasn't 
    // been copied to the back.
    assert( adjacentPathPointsDifferent( newPoints, newPoints + numOfPoints, false ) && "Adjacent path points must be different." );
    
    closedCycle_ = closedCycle;
    
    size_type numberOfPoints = numOfPoints;
    if ( closedCycle_ ) {
        ++numberOfPoints;
    }
    
    points_.reserve( numberOfPoints );
    segmentTangents_.resize( numberOfPoints - 1 );
    segmentLengths_.resize( numberOfPoints - 1 );
    
    points_.assign( newPoints, newPoints + numOfPoints );
    
    if ( closedCycle_ ) {
        points_.push_back( points_[ 0 ] );
    }
    
    updateTangentsAndLengths( points_ , 
                              segmentTangents_, 
                              segmentLengths_, 
                              0, 
                              numOfPoints,
                              closedCycle_ );
    
    shrinkToFit( points_ );
    shrinkToFit( segmentTangents_ );
    shrinkToFit( segmentLengths_ );
}



void 
OpenSteer::PolylineSegmentedPath::movePoints( size_type startIndex,
                                              size_type numOfPoints,
                                              Vec3 const newPoints[] )
{
    assert( ( startIndex < ( pointCount() - ( isCyclic() ? 1 : 0 ) ) ) && 
            "startIndex must be inside index range." );
    assert( ( ( startIndex + numOfPoints ) <= ( pointCount() - ( isCyclic() ? 1 : 0 ) ) ) && 
            "The max. index of a point to set must be inside the index range." ); 
    
    // Update the point positions.
    // @todo Remove this line size_type const pathPointCount = pointCount();
    for ( size_type i = 0; i < numOfPoints; ++i ) {
        points_[ startIndex + i ] = newPoints[ i ];
    }
    
    // If the first point is changed and the path is cyclic also change the
    // last point, which is just a copy of the first point.
    if ( isCyclic() && ( 0 == startIndex ) ) {
        points_.back() = points_.front();
    }
    
    // Recalculate the tangents and lengths.
    updateTangentsAndLengths( points_, 
                              segmentTangents_, 
                              segmentLengths_, 
                              startIndex, 
                              numOfPoints, 
                              isCyclic() );
    
    
    assert( adjacentPathPointsDifferent( points_.begin(), points_.end(), isCyclic() ) && "Adjacent path points must be different." );
}


bool
OpenSteer::PolylineSegmentedPath::isValid() const 
{
    return pointCount() > 1;
}



OpenSteer::Vec3 
OpenSteer::PolylineSegmentedPath::mapPointToPath (const Vec3& point,
                                                  Vec3& tangent,
                                                  float& outside) const
{
    PointToPathMapping mapping;
    mapPointToPathAlike( *this, point, mapping );
    tangent = mapping.tangent;
    outside = mapping.distancePointToPath;
    return mapping.pointOnPathCenterLine;
}


OpenSteer::Vec3 
OpenSteer::PolylineSegmentedPath::mapPathDistanceToPoint (float pathDistance) const
{
    PathDistanceToPointMapping mapping;
    mapDistanceToPathAlike( *this, pathDistance, mapping );
    return mapping.pointOnPathCenterLine;
}


float 
OpenSteer::PolylineSegmentedPath::mapPointToPathDistance (const Vec3& point) const
{
    PointToPathDistanceMapping mapping;
    mapPointToPathAlike( *this, point, mapping );
    return mapping.distanceOnPath;
}


bool 
OpenSteer::PolylineSegmentedPath::isCyclic() const
{
    return closedCycle_;
}


float 
OpenSteer::PolylineSegmentedPath::length() const
{
    return std::accumulate( segmentLengths_.begin(), segmentLengths_.end(), 0.0f );
}


OpenSteer::SegmentedPath::size_type 
OpenSteer::PolylineSegmentedPath::pointCount() const
{
    return points_.size();
}



OpenSteer::Vec3 
OpenSteer::PolylineSegmentedPath::point( size_type pointIndex ) const
{
    assert( pointIndex < pointCount() && "pointIndex out of range." );
    return points_[ pointIndex ];
}




OpenSteer::PolylineSegmentedPath::size_type 
OpenSteer::PolylineSegmentedPath::segmentCount() const
{
    return segmentTangents_.size();
}


float 
OpenSteer::PolylineSegmentedPath::segmentLength( size_type segmentIndex ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex out of range." );
    return segmentLengths_[ segmentIndex ];
}


OpenSteer::Vec3 
OpenSteer::PolylineSegmentedPath::segmentStart( size_type segmentIndex ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex out of range." );
    assert( segmentIndex < pointCount() && "The max. index of a point must be inside range." );
    return points_[ segmentIndex ];
}


OpenSteer::Vec3 
OpenSteer::PolylineSegmentedPath::segmentEnd( size_type segmentIndex ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex out of range." );
    assert( segmentIndex + 1< pointCount() && "The max. index of a point must be inside range." );

    return points_[ segmentIndex + 1 ];
}


float 
OpenSteer::PolylineSegmentedPath::mapPointToSegmentDistance( size_type segmentIndex, 
                                                             Vec3 const& point ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex is out of range." );
    
    Vec3 const segmentStartToPoint( point - points_[ segmentIndex ] );
    float const distance = segmentStartToPoint.dot( segmentTangents_[ segmentIndex ] );
    
    return clamp( distance, 0.0f, segmentLengths_[ segmentIndex ] );
}


OpenSteer::Vec3 
OpenSteer::PolylineSegmentedPath::mapSegmentDistanceToPoint( size_type segmentIndex, 
                                                             float segmentDistance ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex is out of range." );
    
    float const segmentLength = segmentLengths_[ segmentIndex ];
    /*
     * bk: remove behavior that treats negative numbers as distances beginning 
     * from the end of the segment
    if ( 0.0f > segmentDistance ) {
        segmentDistance += segmentLength;
    }
    */
    segmentDistance = clamp( segmentDistance, 0.0f, segmentLength );
    
    return segmentTangents_[ segmentIndex ] * segmentDistance + points_[ segmentIndex ];
}


OpenSteer::Vec3 
OpenSteer::PolylineSegmentedPath::mapSegmentDistanceToTangent( size_type segmentIndex, 
                                                               float ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex is out of range." );
    return segmentTangents_[ segmentIndex ];
}


void 
OpenSteer::PolylineSegmentedPath::mapDistanceToSegmentPointAndTangent( size_type segmentIndex,
                                                                       float segmentDistance,
                                                                       Vec3& pointOnPath,
                                                                       Vec3& tangent ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex is out of range." );
    
    float const segmentLength = segmentLengths_[ segmentIndex ];
    
    /* 
     * bk: remove behavior that treats negative numbers as distances beginning 
     * from the end of the segment
    if ( 0.0f > segmentDistance ) {
        segmentDistance += segmentLength;
    }
    */
    segmentDistance = clamp( segmentDistance, 0.0f, segmentLength );
    
    pointOnPath = segmentTangents_[ segmentIndex ] * segmentDistance + points_[ segmentIndex ];
    tangent = segmentTangents_[ segmentIndex ];
}


void 
OpenSteer::PolylineSegmentedPath::mapPointToSegmentDistanceAndPointAndTangent( size_type segmentIndex,
                                                                               Vec3 const& point,
                                                                               float& distance,
                                                                               Vec3& pointOnPath,
                                                                               Vec3& tangent ) const
{
    assert( segmentIndex < segmentCount() && "segmentIndex is out of range." );
    
    Vec3 const segmentStartPoint = points_[ segmentIndex ];
    Vec3 const segmentStartToPoint( point - segmentStartPoint );
    tangent = segmentTangents_[ segmentIndex ];
    distance = segmentStartToPoint.dot( tangent );
    distance =  clamp( distance, 0.0f, segmentLengths_[ segmentIndex ] );
    pointOnPath = tangent * distance + segmentStartPoint;
}




