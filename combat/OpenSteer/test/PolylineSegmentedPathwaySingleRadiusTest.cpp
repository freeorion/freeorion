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
 */
#include "PolylineSegmentedPathwaySingleRadiusTest.h"




// Include OpenSteer::sqrtXXX
#include "OpenSteer/Utilities.h"

// Include OpenSteer::Old::PolylinePathway
#include "OpenSteer/OldPathway.h"


// Register test suite.
CPPUNIT_TEST_SUITE_REGISTRATION( OpenSteer::PolylineSegmentedPathwaySingleRadiusTest );


OpenSteer::size_t const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::pointCount_;
OpenSteer::size_t const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::cyclicPointCount_;
OpenSteer::size_t const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::segmentCount_ = 3;
OpenSteer::size_t const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::cyclicSegmentCount_ = 4;
OpenSteer::Vec3 const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::points_[ OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::pointCount_ ] = {  
    OpenSteer::Vec3( 0.0f, 0.0f, 0.0f ),
    OpenSteer::Vec3( 2.0f, 0.0f, 0.0f ),
    OpenSteer::Vec3( 3.0f, 0.0f, 0.0f ),
    OpenSteer::Vec3( 2.0f, 2.0f, 0.0f ) };
float const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::pathLength_ = 3.0f + OpenSteer::sqrtXXX( 5.0f );
float const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::cyclicPathLength_ = OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::pathLength_ + OpenSteer::sqrtXXX( 8.0f );
float const OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::radius_ = 1.0f;



OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::PolylineSegmentedPathwaySingleRadiusTest()
{
    // Nothing to do.
}



OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::~PolylineSegmentedPathwaySingleRadiusTest()
{
    // Nothing to do.
}




void 
OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::setUp()
{
    TestFixture::setUp();
    
    path_.reset( new PolylineSegmentedPathwaySingleRadius( pointCount_, points_ , radius_, false )  );
    cyclicPath_.reset( new PolylineSegmentedPathwaySingleRadius( pointCount_, points_, radius_, true ) );
    
}



void 
OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::tearDown()
{
    TestFixture::tearDown();
    
    path_.release();
    cyclicPath_.release();
}


void
OpenSteer::PolylineSegmentedPathwaySingleRadiusTest::testCompareWithOldPathImplementation()
{
    // Create old pathway construct to compare semantics of new and old pathway
    // operations.
    Old::PolylinePathway oldPathway( pointCount_, points_, radius_, false);
    
    PolylineSegmentedPathwaySingleRadius newPathway( *path_ );
    
    CPPUNIT_ASSERT_EQUAL( oldPathway.getTotalPathLength(), newPathway.length() );

    float outsideNew = 0.0f;
    float outsideOld = 0.0f;
    Vec3 tangentNew( 0.0f, 0.0f, 0.0f );
    Vec3 tangentOld( 0.0f, 0.0f, 0.0f );
    Vec3 pointOnPathNew( 0.0f, 0.0f, 0.0f );
    Vec3 pointOnPathOld( 0.0f, 0.0f, 0.0f );
    
    
    // point near the beginning ouside the pathway
    Vec3 const point0( 0.0f, 1.5f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point0, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point0, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld );
    
    
    // point near the beginning inside the pathway
    Vec3 const point1( -0.5f, 0.0f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point1, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point1, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld );    
    
    // point near the beginning on the boundary
    Vec3 const point1_1( -1.0f, 0.0f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point1_1, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point1_1, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld );  
    
    // point in the middle of the pathway but ouside of it
    Vec3 const point2( 2.5f, -3.0f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point2, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point2, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld );   
    
    // point in the middle of the pathway and inside of it
    Vec3 const point3( 1.5f, 0.7f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point3, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point3, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld );   
    
    // point in the middle of the pathway on the boundary
    Vec3 const point3_1( 1.5f, -1.0f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point3_1, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point3_1, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld ); 
    
    // point at the end of the pathway but ouside of it
    Vec3 const point4( 2.0f, 3.5f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point4, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point4, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld );   
    
    // point at the end of the pathway and inside of it
    Vec3 const point5( 2.3f, 2.3f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point5, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point5, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld );   
    
    // point at the end of the pathway on the boundary
    Vec3 const point6( 2.0f, 3.0f, 0.0f );
    pointOnPathNew = newPathway.mapPointToPath( point6, tangentNew, outsideNew );
    pointOnPathOld = oldPathway.mapPointToPath( point6, tangentOld, outsideOld );
    CPPUNIT_ASSERT_EQUAL( pointOnPathNew, pointOnPathOld );
    CPPUNIT_ASSERT_EQUAL( tangentNew, tangentOld );
    CPPUNIT_ASSERT_EQUAL( outsideNew, outsideOld ); 
    
    
    
    // oldPathway.mapPointToPath;
    // oldPathway.mapPointToPathDistance;
    // oldPathway.mapPathDistanceToPoint;
    
    
    
}






