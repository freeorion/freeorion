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
 * Unit test for @c OpenSteer::SharedPointer.
 */
#ifndef OPENSTEER_SHAREDPOINTERTEST_H
#define OPENSTEER_SHAREDPOINTERTEST_H




#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>


// Include OpenSteer::SharedPointer
#include "OpenSteer/SharedPointer.h"



namespace OpenSteer {
    
    
    class SharedPointerTest : public CppUnit::TestFixture {
    public:
        SharedPointerTest();
        virtual ~SharedPointerTest();
        
        virtual void setUp();
        virtual void tearDown();
        
        CPPUNIT_TEST_SUITE(SharedPointerTest);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testAssignment);
        CPPUNIT_TEST(testReset);
        CPPUNIT_TEST(testInheritance);
        CPPUNIT_TEST(testComparisons);
        CPPUNIT_TEST(testImplicitBoolCast);
        CPPUNIT_TEST(testSwap);
        CPPUNIT_TEST_SUITE_END();
        
    private:
        /**
         * Not implemented to make it non-copyable.
         */
        SharedPointerTest( SharedPointerTest const& );
        
        /**
         * Not implemented to make it non-copyable.
         */
        SharedPointerTest& operator=( SharedPointerTest const& );
        
    private:
        /**
         * Tests the different constructors.
         */
        void testConstruction();
        
        /**
         * Test different assignment situations.
         */
        void testAssignment();
        
        /**
         * Tests resetting a shared pointer.
         */
        void testReset();
        
        /**
         * Tests assigning instances of sub classes to a shared pointer for the
         * super class.
         */
        void testInheritance();
        
        /**
         * Tests the equality and inequality operators.
         */
        void testComparisons();
        
        /**
         * Test the implicit conversion of a shared pointer to bool or a type
         * that can be used inside a conditional like @c if or @c while.
         */
        void testImplicitBoolCast();
        
        /**
         * Explicit test of @c swap which is already tested by @c testAssignment.
         */
        void testSwap();
        
        

        
    }; // SharedPointerTest
    
    
    
    
} // namespace OpenSteer


#endif // OPENSTEER_SHAREDPOINTERTEST_H
