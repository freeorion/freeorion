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
 * Unit test fot @c OpenSteer::PolylineSegmentedPathwaySingleRadius.
 */

#ifndef OPENSTEER_POLYLINESEGMENTEDPATHWAYSINGLERADIUSTEST_H
#define OPENSTEER_POLYLINESEGMENTEDPATHWAYSINGLERADIUSTEST_H

// Include std::auto_ptr
#include <memory>


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>



// Include OpenSteer::PolylineSegmentedPathwaySingleRadius
#include "OpenSteer/PolylineSegmentedPathwaySingleRadius.h"

// Include OpenSteer::size_t
#include "OpenSteer/StandardTypes.h"

// Include OpenSteer::Vec3
#include "OpenSteer/Vec3.h"


namespace OpenSteer {
    
    
    class PolylineSegmentedPathwaySingleRadiusTest : public CppUnit::TestFixture {
    public:
        PolylineSegmentedPathwaySingleRadiusTest();
        virtual ~PolylineSegmentedPathwaySingleRadiusTest();
        
        virtual void setUp();
        virtual void tearDown();
        
        CPPUNIT_TEST_SUITE(PolylineSegmentedPathwaySingleRadiusTest);
        /*
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testAssignment);
        CPPUNIT_TEST(testSegmentData);
        CPPUNIT_TEST(testMovePoints);
        CPPUNIT_TEST(testMovePointsCyclicPath);
        CPPUNIT_TEST(testSegmentMappings);
        CPPUNIT_TEST(testPointToPathMappings);
        CPPUNIT_TEST(testDistanceToPathMappings);
         */
        CPPUNIT_TEST(testCompareWithOldPathImplementation);
        CPPUNIT_TEST_SUITE_END();
        
    private:
        /**
         * Not implemented to make it non-copyable.
         */
        PolylineSegmentedPathwaySingleRadiusTest( PolylineSegmentedPathwaySingleRadiusTest const& );
        
        /**
         * Not implemented to make it non-copyable.
         */
        PolylineSegmentedPathwaySingleRadiusTest& operator=( PolylineSegmentedPathwaySingleRadiusTest );
        
    private:
        /*
        void testConstruction();
        void testAssignment();
        void testSegmentData();
        void testMovePoints();
        void testMovePointsCyclicPath();
        void testSegmentMappings();        
        void testPointToPathMappings();
        void testDistanceToPathMappings();
         */
        void testCompareWithOldPathImplementation();
        
        
        std::auto_ptr< PolylineSegmentedPathwaySingleRadius > path_;
        std::auto_ptr< PolylineSegmentedPathwaySingleRadius > cyclicPath_;
        static size_t const pointCount_ = 4;
        static size_t const cyclicPointCount_ = 5;
        static size_t const segmentCount_;
        static size_t const cyclicSegmentCount_;
        static Vec3 const points_[ pointCount_ ];
        static float const pathLength_;
        static float const cyclicPathLength_;
        static float const radius_;
        
    }; // PolylineSegmentedPathwaySingleRadiusTest

    
} // namespace OpenSteer

#endif // OPENSTEER_POLYLINESEGMENTEDPATHWAYSINGLERADIUSTEST_H
