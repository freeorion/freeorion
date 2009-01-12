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
#include "PolylineSegmentedPathTest.h"


// Include std::cout, std:.endl
// #include <iostream>

// Include OpenSteer::Vec3
#include "OpenSteer/Vec3.h"

// Include OpenSteer::sqrtXXX, OpenSteer::equalsRelative
#include "OpenSteer/Utilities.h"

// Include OpenSteer::equalsRelative for Vec3s
#include "OpenSteer/Vec3Utilities.h"




// Register test suite.
CPPUNIT_TEST_SUITE_REGISTRATION( OpenSteer::PolylineSegmentedPathTest );


OpenSteer::size_t const OpenSteer::PolylineSegmentedPathTest::pointCount_;
OpenSteer::size_t const OpenSteer::PolylineSegmentedPathTest::cyclicPointCount_;
OpenSteer::size_t const OpenSteer::PolylineSegmentedPathTest::segmentCount_ = 3;
OpenSteer::size_t const OpenSteer::PolylineSegmentedPathTest::cyclicSegmentCount_ = 4;
OpenSteer::Vec3 const OpenSteer::PolylineSegmentedPathTest::points_[ OpenSteer::PolylineSegmentedPathTest::pointCount_ ] = {  
    OpenSteer::Vec3( 0.0f, 0.0f, 0.0f ),
    OpenSteer::Vec3( 2.0f, 0.0f, 0.0f ),
    OpenSteer::Vec3( 3.0f, 0.0f, 0.0f ),
    OpenSteer::Vec3( 2.0f, 2.0f, 0.0f ) };
float const OpenSteer::PolylineSegmentedPathTest::pathLength_ = 3.0f + OpenSteer::sqrtXXX( 5.0f );
float const OpenSteer::PolylineSegmentedPathTest::cyclicPathLength_ = OpenSteer::PolylineSegmentedPathTest::pathLength_ + OpenSteer::sqrtXXX( 8.0f );


OpenSteer::PolylineSegmentedPathTest::PolylineSegmentedPathTest()
{
    // Nothing to do.
}



OpenSteer::PolylineSegmentedPathTest::~PolylineSegmentedPathTest()
{
    // Nothing to do.
}




void 
OpenSteer::PolylineSegmentedPathTest::setUp()
{
    TestFixture::setUp();
    
    path_.reset( new PolylineSegmentedPath( pointCount_, points_ , false )  );
    cyclicPath_.reset( new PolylineSegmentedPath( pointCount_, points_, true ) );
    
}



void 
OpenSteer::PolylineSegmentedPathTest::tearDown()
{
    TestFixture::tearDown();
    
    path_.release();
    cyclicPath_.release();
}





void 
OpenSteer::PolylineSegmentedPathTest::testConstruction()
{
    // Empty path construction.
    PolylineSegmentedPath path0;
    CPPUNIT_ASSERT( ! path0.isCyclic() );
    CPPUNIT_ASSERT_EQUAL( 0.0f, path0.length() );
    CPPUNIT_ASSERT_EQUAL( static_cast< OpenSteer::size_t >( 0 ), path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( static_cast< OpenSteer::size_t >( 0 ), path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( false, path0.isValid() );
    
    // Predefined path test.
    CPPUNIT_ASSERT( !path_->isCyclic() );    
    CPPUNIT_ASSERT( equalsRelative( path_->length(), pathLength_ ) );
    CPPUNIT_ASSERT_EQUAL( pointCount_, path_->pointCount() );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path_->segmentCount() );
    CPPUNIT_ASSERT_EQUAL( true, path_->isValid() );
    
    // Predefined cyclic path test.
    CPPUNIT_ASSERT( cyclicPath_->isCyclic() );    
    CPPUNIT_ASSERT( equalsRelative( cyclicPath_->length(), cyclicPathLength_ ) );
    CPPUNIT_ASSERT_EQUAL( cyclicPointCount_, cyclicPath_->pointCount() );
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, cyclicPath_->segmentCount() );
    CPPUNIT_ASSERT_EQUAL( true, cyclicPath_->isValid() );
    
    // Copy constructor test.
    PolylineSegmentedPath path1( *path_ );
    CPPUNIT_ASSERT( !path1.isCyclic() );    
    CPPUNIT_ASSERT( equalsRelative( path1.length(), pathLength_ ) );
    CPPUNIT_ASSERT_EQUAL( pointCount_, path1.pointCount() );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path1.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( true, path1.isValid() );
    
    PolylineSegmentedPath path2( *cyclicPath_ );
    CPPUNIT_ASSERT( path2.isCyclic() );    
    CPPUNIT_ASSERT( equalsRelative( path2.length(), cyclicPathLength_ ) );
    CPPUNIT_ASSERT_EQUAL( cyclicPointCount_, path2.pointCount() );
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, path2.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( true, path2.isValid() );
}



void 
OpenSteer::PolylineSegmentedPathTest::testAssignment()
{
    // Test assignment operator
    PolylineSegmentedPath path0( *path_ );
    path0 = *cyclicPath_;
    CPPUNIT_ASSERT( path0.isCyclic() );    
    CPPUNIT_ASSERT( equalsRelative( path0.length(), cyclicPathLength_ ) );
    CPPUNIT_ASSERT_EQUAL( cyclicPointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( true, path0.isValid() );    
    
    // Test setPath.
    path0.setPath( pointCount_, points_, false );
    CPPUNIT_ASSERT( !path0.isCyclic() );    
    CPPUNIT_ASSERT( equalsRelative( path0.length(), pathLength_ ) );
    CPPUNIT_ASSERT_EQUAL( pointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( true, path0.isValid() );
}



void 
OpenSteer::PolylineSegmentedPathTest::testSegmentData()
{
    // Test non-cyclic path segments.
    PolylineSegmentedPath path0( *path_ );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 0 ], path0.segmentStart( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 1 ], path0.segmentEnd( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 1 ], path0.segmentStart( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.segmentEnd( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.segmentStart( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 3 ], path0.segmentEnd( 2 ) );
    
    
    // Test cyclic path segments.
    path0 =  *cyclicPath_;
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 8.0f ), path0.segmentLength( 3 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 0 ], path0.segmentStart( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 1 ], path0.segmentEnd( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 1 ], path0.segmentStart( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.segmentEnd( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.segmentStart( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 3 ], path0.segmentEnd( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 3 ], path0.segmentStart( 3 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 0 ], path0.segmentEnd( 3 ) );
    CPPUNIT_ASSERT_EQUAL( path0.point( 0 ), path0.point( 4 ) );
}



void 
OpenSteer::PolylineSegmentedPathTest::testMovePoints()
{
    // Move one point of non-cyclic path.
    PolylineSegmentedPath path0( *path_ );
    // Just move the second point to (1, 0, 0).
    size_t movePointCount = 1;
    size_t startIndex = 1;
    Vec3 points[] = { Vec3( 1.0f, 0.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount , points );
    CPPUNIT_ASSERT_EQUAL( pointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points_[ 0 ], path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points[ 0 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 3 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT( equalsRelative( pathLength_, path0.length() ) );
    
    // Move two points including start point of non-cyclic path.
    path0 = *path_;
    movePointCount = 2;
    startIndex = 0;
    Vec3 points1[] = { Vec3( 1.0f, -1.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount, points1 );
    CPPUNIT_ASSERT_EQUAL( pointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points1[ 0 ], path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points1[ 1 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 3 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT( equalsRelative( pathLength_, path0.length() ) );    
    
    // Move end point of non-cyclic path.
    path0 = *path_;
    movePointCount = 2;
    startIndex = 2;
    Vec3 points2[] = { Vec3( 4.0f, 0.0f, 0.0f ), Vec3( 3.0f, 2.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount, points2 );
    CPPUNIT_ASSERT_EQUAL( pointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points_[ 0 ] , path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 1 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points2[ 0 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points2[ 1 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT( equalsRelative( pathLength_ + 1 , path0.length() ) );
    
    // Move all points of non-cyclic path.
    path0 = *path_;
    movePointCount = 4;
    startIndex = 0;
    Vec3 points3[] = { Vec3( -1.0f, 0.0f, 0.0f ), Vec3( -1.0f, -2.0f, 0.0f ), Vec3( -1.0, -3.0f, 0.0f ), Vec3( 1.0f, -2.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount, points3 );
    CPPUNIT_ASSERT_EQUAL( pointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( segmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points3[ 0 ] , path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points3[ 1 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points3[ 2 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points3[ 3 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT( equalsRelative( pathLength_ , path0.length() ) );    
}



void 
OpenSteer::PolylineSegmentedPathTest::testMovePointsCyclicPath()
{
    // Move one point of cyclic path.
    PolylineSegmentedPath path0( *cyclicPath_ );
    // Just move the second point to (1, 0, 0).
    size_t movePointCount = 1;
    size_t startIndex = 1;
    Vec3 points[] = { Vec3( 1.0f, 0.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount , points );
    CPPUNIT_ASSERT_EQUAL( cyclicPointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points_[ 0 ], path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points[ 0 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 3 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 8.0f ), path0.segmentLength( 3 ) );
    CPPUNIT_ASSERT( equalsRelative( cyclicPathLength_, path0.length() ) );
    
    // Move two points including start point of cyclic path.
    path0 = *cyclicPath_;
    movePointCount = 2;
    startIndex = 0;
    Vec3 points1[] = { Vec3( 1.0f, -1.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount, points1 );
    CPPUNIT_ASSERT_EQUAL( cyclicPointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points1[ 0 ], path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points1[ 1 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 2 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 3 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );    
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 10.0f ), path0.segmentLength( 3 ) );
    CPPUNIT_ASSERT( equalsRelative( 3.0f + sqrtXXX( 5.0f ) + sqrtXXX( 10.0f ), path0.length() ) );    
    
    // Move end point of cyclic path.
    path0 = *cyclicPath_;
    movePointCount = 2;
    startIndex = 2;
    Vec3 points2[] = { Vec3( 4.0f, 0.0f, 0.0f ), Vec3( 3.0f, 2.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount, points2 );
    CPPUNIT_ASSERT_EQUAL( cyclicPointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points_[ 0 ] , path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points_[ 1 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points2[ 0 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points2[ 1 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );    
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 13.0f ), path0.segmentLength( 3 ) );
    CPPUNIT_ASSERT( equalsRelative( 4.0f + sqrtXXX( 5 ) + sqrtXXX( 13.0f ) , path0.length() ) );
    
    // Move all points of cyclic path.
    path0 = *cyclicPath_;
    movePointCount = 4;
    startIndex = 0;
    Vec3 points3[] = { Vec3( -1.0f, 0.0f, 0.0f ), Vec3( -1.0f, -2.0f, 0.0f ), Vec3( -1.0, -3.0f, 0.0f ), Vec3( 1.0f, -2.0f, 0.0f ) };
    path0.movePoints( startIndex, movePointCount, points3 );
    CPPUNIT_ASSERT_EQUAL( cyclicPointCount_, path0.pointCount() );
    CPPUNIT_ASSERT_EQUAL( cyclicSegmentCount_, path0.segmentCount() );
    CPPUNIT_ASSERT_EQUAL( points3[ 0 ] , path0.point( 0 ) );
    CPPUNIT_ASSERT_EQUAL( points3[ 1 ], path0.point( 1 ) );
    CPPUNIT_ASSERT_EQUAL( points3[ 2 ], path0.point( 2 ) );
    CPPUNIT_ASSERT_EQUAL( points3[ 3 ], path0.point( 3 ) );
    CPPUNIT_ASSERT_EQUAL( 2.0f, path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 1.0f, path0.segmentLength( 1 ) );
    CPPUNIT_ASSERT_EQUAL( sqrtXXX( 5.0f ), path0.segmentLength( 2 ) );
    CPPUNIT_ASSERT( equalsRelative( cyclicPathLength_ , path0.length() ) );    
}




void 
OpenSteer::PolylineSegmentedPathTest::testSegmentMappings()
{
    PolylineSegmentedPath path0( *path_ );
    
    // Test mapPointToSegmentDistanceAndPointAndTangent aside the other mapping tests..
    Vec3 pointOnPathResult( 0.0f, 0.0f, 0.0f );
    Vec3 tangentResult( 0.0f, 0.0f, 0.0f );
    float distanceResult = 0.0f;
    
    
    // Test point to segment mappings.
    Vec3 const point0( -1.0f, 0.0f, 0.0f );
    float const distanceMapping0( 0.0f );
    CPPUNIT_ASSERT_EQUAL( distanceMapping0, path0.mapPointToSegmentDistance( 0, point0 ) );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point0, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distanceMapping0, distanceResult );
    
    Vec3 const point1( -1.0f, 1.0f, 0.0f );
    float const distanceMapping1( 0.0f );
    CPPUNIT_ASSERT_EQUAL( distanceMapping1, path0.mapPointToSegmentDistance( 0, point1 ) );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point1, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distanceMapping1, distanceResult );
    
    
    Vec3 const point2( 0.0f, 1.0f, 0.0f );
    float const distanceMapping2( 0.0f );
    CPPUNIT_ASSERT_EQUAL( distanceMapping2, path0.mapPointToSegmentDistance( 0, point2 ) );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point2, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distanceMapping2, distanceResult );
    
    
    Vec3 const point3( 1.0f, 1.0f, 0.0f );
    float const distanceMapping3( 1.0f );
    CPPUNIT_ASSERT_EQUAL( distanceMapping3, path0.mapPointToSegmentDistance( 0, point3 ) );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point3, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distanceMapping3, distanceResult );
    
    
    Vec3 const point4( 2.0f, 1.0f, 0.0f );
    float const distanceMapping4( path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( distanceMapping4, path0.mapPointToSegmentDistance( 0, point4 ) );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point4, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distanceMapping4, distanceResult );
    
    
    Vec3 const point5( 3.0f, 1.0f, 0.0f );
    float const distanceMapping5( path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( distanceMapping5, path0.mapPointToSegmentDistance( 0, point5 ) );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point5, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distanceMapping5, distanceResult );
    
    
    Vec3 const point6( 3.0f, 0.0f, 0.0f );
    float const distanceMapping6( path0.segmentLength( 0 ) );
    CPPUNIT_ASSERT_EQUAL( distanceMapping6, path0.mapPointToSegmentDistance( 0, point6 ) );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point6, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distanceMapping6, distanceResult );
    
    
    
    // Test distance to point mapping
    Vec3 const point10( 0.0f, 0.0f, 0.0f );
    float const distance10 = 0.0f;
    CPPUNIT_ASSERT_EQUAL( point10, path0.mapSegmentDistanceToPoint( 0, distance10 ) );
    path0.mapDistanceToSegmentPointAndTangent( 0, distance10, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( point10, pointOnPathResult );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point10, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distance10, distanceResult );
    CPPUNIT_ASSERT_EQUAL( point10, pointOnPathResult );
    CPPUNIT_ASSERT_EQUAL( Vec3( 1.0f, 0.0f, 0.0f ), tangentResult );
    
    Vec3 const point11( 1.0f, 0.0f, 0.0f );
    float const distance11 = 1.0f;
    CPPUNIT_ASSERT_EQUAL( point11, path0.mapSegmentDistanceToPoint( 0, distance11 ) );
    path0.mapDistanceToSegmentPointAndTangent( 0, distance11, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( point11, pointOnPathResult );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point11, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distance11, distanceResult );
    CPPUNIT_ASSERT_EQUAL( point11, pointOnPathResult );
    CPPUNIT_ASSERT_EQUAL( Vec3( 1.0f, 0.0f, 0.0f ), tangentResult );
    
    Vec3 const point12( 2.0f, 0.0f, 0.0f );
    float const distance12 = 2.0f;
    CPPUNIT_ASSERT_EQUAL( point12, path0.mapSegmentDistanceToPoint( 0, distance12 ) );
    path0.mapDistanceToSegmentPointAndTangent( 0, distance12, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( point12, pointOnPathResult );
    path0.mapPointToSegmentDistanceAndPointAndTangent( 0, point12, distanceResult, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( distance12, distanceResult );
    CPPUNIT_ASSERT_EQUAL( point12, pointOnPathResult );
    CPPUNIT_ASSERT_EQUAL( Vec3( 1.0f, 0.0f, 0.0f ), tangentResult );
    
    Vec3 const point13( 2.0f, 0.0f, 0.0f );
    float const distance13 = 3.0f;
    CPPUNIT_ASSERT_EQUAL( point13, path0.mapSegmentDistanceToPoint( 0, distance13 ) );
    
    // Removed negative distance handling of path alikes.
    // Vec3 const point14( 1.0f, 0.0f, 0.0f );
    Vec3 const point14( 0.0f, 0.0f, 0.0f );
    float const distance14 = -1.0f;
    CPPUNIT_ASSERT_EQUAL( point14, path0.mapSegmentDistanceToPoint( 0, distance14 ) );
    
    // Test distance to tangent mapping
    Vec3 const tangent( 1.0f, 0.0f, 0.0f );
    float const distance20 = 0.0f;
    Vec3 const point20( 0.0f, 0.0f, 0.0f );
    CPPUNIT_ASSERT_EQUAL( tangent, path0.mapSegmentDistanceToTangent( 0, distance20 ) );
    path0.mapDistanceToSegmentPointAndTangent( 0, distance20, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( tangent, tangentResult );
    CPPUNIT_ASSERT_EQUAL( point20, pointOnPathResult );

    
    float const distance21 = 1.0f;
    Vec3 const point21( 1.0f, 0.0f, 0.0f );
    CPPUNIT_ASSERT_EQUAL( tangent, path0.mapSegmentDistanceToTangent( 0, distance21 ) );
    path0.mapDistanceToSegmentPointAndTangent( 0, distance21, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( tangent, tangentResult );
    CPPUNIT_ASSERT_EQUAL( point21, pointOnPathResult );
    
    float const distance22 = 2.0f;
    Vec3 const point22( 2.0f, 0.0f, 0.0f );
    CPPUNIT_ASSERT_EQUAL( tangent, path0.mapSegmentDistanceToTangent( 0, distance22 ) );
    path0.mapDistanceToSegmentPointAndTangent( 0, distance22, pointOnPathResult, tangentResult );
    CPPUNIT_ASSERT_EQUAL( tangent, tangentResult );
    CPPUNIT_ASSERT_EQUAL( point22, pointOnPathResult );
    
    float const distance23 = 3.0f;
    Vec3 const point23( 3.0f, 0.0f, 0.0f );
    CPPUNIT_ASSERT_EQUAL( tangent, path0.mapSegmentDistanceToTangent( 0, distance23 ) );
    
    // Removed negative distance handling of path alikes.
    // float const distance24 = -1.0f;
    // Vec3 const point24( -1.0f, 0.0f, 0.0f );
    // CPPUNIT_ASSERT_EQUAL( tangent, path0.mapSegmentDistanceToTangent( 0, distance24 ) );
    
}




void 
OpenSteer::PolylineSegmentedPathTest::testPointToPathMappings()
{
    // Map a point to a path
    PolylineSegmentedPath path0( *path_ );
    Vec3 pointResult( 0.0f, 0.0f, 0.0f );
    Vec3 tangentResult( 0.0f, 0.0f, 0.0f );
    float outsideResult( 0.0f );
    
    
    Vec3 point0( -1.0f, 0.0f, 0.0f );
    Vec3 pointOnPath0( 0.0f, 0.0f, 0.0f );
    Vec3 tangent0( 1.0f, 0.0f, 0.0f );
    float outside0( 1.0f );
    float distance0( 0.0f );
    pointResult = path0.mapPointToPath( point0, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath0, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent0, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside0, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance0, path0.mapPointToPathDistance( point0 ) );
    
    
    Vec3 point1( -1.0f, 1.0f, 0.0f );
    Vec3 pointOnPath1( 0.0f, 0.0f, 0.0f );
    Vec3 tangent1( 1.0f, 0.0f, 0.0f );
    float outside1( sqrtXXX( 2.0f ) );
    float distance1( 0.0f );
    pointResult = path0.mapPointToPath( point1, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath1, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent1, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside1, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance1, path0.mapPointToPathDistance( point1 ) );
    
    Vec3 point2(  0.0f, 1.0f, 0.0f );
    Vec3 pointOnPath2( 0.0f, 0.0f, 0.0f );
    Vec3 tangent2( 1.0f, 0.0f, 0.0f );
    float outside2( 1.0f );
    float distance2( 0.0f );
    pointResult = path0.mapPointToPath( point2, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath2, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent2, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside2, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance2, path0.mapPointToPathDistance( point2 ) );
    
    Vec3 point3(  1.0f, 1.0f, 0.0f );
    Vec3 pointOnPath3( 1.0f, 0.0f, 0.0f );
    Vec3 tangent3( 1.0f, 0.0f, 0.0f );
    float outside3( 1.0f );
    float distance3( 1.0f );
    pointResult = path0.mapPointToPath( point3, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath3, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent3, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside3, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance3, path0.mapPointToPathDistance( point3 ) );
    
    Vec3 point4(  1.0f, 0.1f, 0.0f );
    Vec3 pointOnPath4( 1.0f, 0.0f, 0.0f );
    Vec3 tangent4( 1.0f, 0.0f, 0.0f );
    float outside4( 0.1f );
    float distance4( 1.0f );
    pointResult = path0.mapPointToPath( point4, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath4, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent4, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside4, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance4, path0.mapPointToPathDistance( point4 ) );
    
    Vec3 point5(  2.0f, 0.1f, 0.0f );
    Vec3 pointOnPath5( 2.0f, 0.0f, 0.0f );
    Vec3 tangent5( 1.0f, 0.0f, 0.0f );
    float outside5( 0.1f );
    float distance5(2.0f );
    pointResult = path0.mapPointToPath( point5, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath5, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent5, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside5, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance5, path0.mapPointToPathDistance( point5 ) );
    
    Vec3 point6(  2.5f, 0.1f, 0.0f );
    Vec3 pointOnPath6( 2.5f, 0.0f, 0.0f );
    Vec3 tangent6( 1.0f, 0.0f, 0.0f );
    float outside6( 0.1f );
    float distance6( 2.5f );
    pointResult = path0.mapPointToPath( point6, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath6, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent6, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside6, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance6, path0.mapPointToPathDistance( point6 ) );
    
    Vec3 point7(  2.5f, 0.0f, 0.0f );
    Vec3 pointOnPath7( 2.5f, 0.0f, 0.0f );
    Vec3 tangent7( 1.0f, 0.0f, 0.0f );
    float outside7( 0.0f );
    float distance7( 2.5f );
    pointResult = path0.mapPointToPath( point7, tangentResult, outsideResult );
    CPPUNIT_ASSERT_EQUAL( pointOnPath7, pointResult );
    CPPUNIT_ASSERT_EQUAL( tangent7, tangentResult );
    CPPUNIT_ASSERT_EQUAL( outside7, outsideResult );
    CPPUNIT_ASSERT_EQUAL( distance7, path0.mapPointToPathDistance( point7 ) );
}



void
OpenSteer::PolylineSegmentedPathTest::testDistanceToPathMappings()
{
    // Test on non-cyclic path
    PolylineSegmentedPath path0( *path_ );
    
    Vec3 pointResult( 0.0f, 0.0f, 0.0f );
    
    Vec3 point0( 0.0f, 0.0f, 0.0f );
    float distance0( 0.0f );
    pointResult = path0.mapPathDistanceToPoint( distance0 );
    CPPUNIT_ASSERT_EQUAL( point0, pointResult );
    
    Vec3 point1( 1.0f, 0.0f, 0.0f );
    float distance1( 1.0f );
    pointResult = path0.mapPathDistanceToPoint( distance1 );
    CPPUNIT_ASSERT_EQUAL( point1, pointResult );
    
    Vec3 point2( 2.5f, 1.0f, 0.0f );
    float distance2( 3.0f + 0.5f * sqrtXXX( 5.0f ) );
    pointResult = path0.mapPathDistanceToPoint( distance2 );
    CPPUNIT_ASSERT( equalsRelative( point2, pointResult ) );
    
    Vec3 point3( 2.0f, 2.0f, 0.0f );
    float distance3( 10.0f );
    pointResult = path0.mapPathDistanceToPoint( distance3 );
    CPPUNIT_ASSERT( equalsRelative( point3, pointResult ) );
    
    // Removed negative distance handling of path alikes.
    // Vec3 point4( 3.0f, 0.0f, 0.0f );
    Vec3 point4( 0.0f, 0.0f, 0.0f );
    float distance4( -sqrtXXX( 5.0f ) );
    pointResult = path0.mapPathDistanceToPoint( distance4 );
    CPPUNIT_ASSERT( equalsRelative( point4, pointResult ) );

    // Test distance mapping shortly after the path end point.
    Vec3 point5( 2.0f, 2.0f, 0.0f );
    float distance5( 3.0f + sqrtXXX( 5.0f ) + 0.01f );
    pointResult = path0.mapPathDistanceToPoint( distance5 );
    CPPUNIT_ASSERT( equalsRelative( point5, pointResult ) );
    
    // Test distance mapping onto the path end point.
    Vec3 point6( 2.0f, 2.0f, 0.0f );
    float distance6( 3.0f + sqrtXXX( 5.0f ) );
    pointResult = path0.mapPathDistanceToPoint( distance6 );
    CPPUNIT_ASSERT( equalsRelative( point6, pointResult ) );
    

    
    // Test on cyclic path
    path0 = *cyclicPath_;
    
    Vec3 point10( 0.0f, 0.0f, 0.0f );
    float distance10( 0.0f );
    pointResult = path0.mapPathDistanceToPoint( distance10 );
    CPPUNIT_ASSERT_EQUAL( point10, pointResult );
    
    Vec3 point11( 1.0f, 0.0f, 0.0f );
    float distance11( 1.0f );
    pointResult = path0.mapPathDistanceToPoint( distance11 );
    CPPUNIT_ASSERT_EQUAL( point11, pointResult );
    
    Vec3 point12( 2.5f, 1.0f, 0.0f );
    float distance12( 3.0f + 0.5f * sqrtXXX( 5.0f ) );
    pointResult = path0.mapPathDistanceToPoint( distance12 );
    CPPUNIT_ASSERT( equalsRelative( point12, pointResult ) );
    
    Vec3 point13( 1.935504897754f, 0.0f, 0.0f );
    float distance13( 10.0f );
    pointResult = path0.mapPathDistanceToPoint( distance13 );
    CPPUNIT_ASSERT( equalsRelative( point13, pointResult ) );
    
    // Removed negative distance handling of path alikes.
    // Vec3 point14( 2.0f, 2.0f, 0.0f );
    Vec3 point14( 0.0f, 0.0f, 0.0f );
    float distance14( -sqrtXXX( 8.0f ) );
    pointResult = path0.mapPathDistanceToPoint( distance14 );
    CPPUNIT_ASSERT( equalsRelative( point14, pointResult ) );    
    
    // Removed negative distance handling of path alikes.
    // Vec3 point15( 2.0f, 2.0f, 0.0f );
    Vec3 point15( 0.0f, 0.0f, 0.0f );
    float distance15( -( 2.0f * sqrtXXX( 8.0f ) + 3.0f + sqrtXXX( 5.0f ) ) );
    pointResult = path0.mapPathDistanceToPoint( distance15 );
    // CPPUNIT_ASSERT( equalsRelative( point15, pointResult ) );    
}




void
OpenSteer::PolylineSegmentedPathTest::testCompareWithOldPathImplementation() 
{
    // Reserved for tests if necessary.
}



