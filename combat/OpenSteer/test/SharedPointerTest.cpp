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
#include "SharedPointerTest.h"





CPPUNIT_TEST_SUITE_REGISTRATION( OpenSteer::SharedPointerTest );



OpenSteer::SharedPointerTest::SharedPointerTest()
{
    // Nothing to do.
}



OpenSteer::SharedPointerTest::~SharedPointerTest()
{
    // Nothing to do.
}




void 
OpenSteer::SharedPointerTest::setUp()
{
    TestFixture::setUp();
}



void 
OpenSteer::SharedPointerTest::tearDown()
{
    TestFixture::tearDown();
}



namespace {
 
    
    template< int i >
    struct SharedPointerTester {
        
        SharedPointerTester() : counter_( i ) {
            ++static_counter_;
        }
        
        ~SharedPointerTester() {
            --static_counter_;
        }
        
        int const counter_;
        static int static_counter_;
    };
    
    template< int i > int SharedPointerTester<i>::static_counter_ = 0;
    
} // anonymous namespace





void 
OpenSteer::SharedPointerTest::testConstruction()
{
    // Testing automatic destruction of a raw pointer hold by a single 
    // shared pointer.
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    SharedPointerTester< 0 >* rawPointer = new SharedPointerTester< 0 >();
    
    CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
    
    {
        SharedPointer< SharedPointerTester< 0 > > sp( rawPointer );
    }
    
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    
    // Testing copy constructor.
    rawPointer = new SharedPointerTester< 0 >();
    
    {
        SharedPointer< SharedPointerTester< 0 > > sp0( rawPointer );
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>( 1 ), sp0.useCount() );
        {
                SharedPointer< SharedPointerTester< 0 > > sp1( sp0 );
                CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
                CPPUNIT_ASSERT_EQUAL( static_cast<size_t>( 2 ), sp0.useCount() );
                CPPUNIT_ASSERT_EQUAL( static_cast<size_t>( 2 ), sp1.useCount() );
                CPPUNIT_ASSERT_EQUAL( rawPointer, sp0.get() );
                CPPUNIT_ASSERT_EQUAL( rawPointer, sp1.get() );
        }
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>( 1 ), sp0.useCount() );
        CPPUNIT_ASSERT_EQUAL( rawPointer, sp0.get() );
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    
    // Testing construction of an empty shared pointer.
    {
        SharedPointer< SharedPointerTester< 0 > > sp0;
        CPPUNIT_ASSERT( 0 == sp0.get() );
    }

    
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
}



void 
OpenSteer::SharedPointerTest::testAssignment()
{
    // Testing assignment of shared pointers pointing to 0.
    {
        SharedPointer< SharedPointerTester< 0 > > sp0;
        SharedPointer< SharedPointerTester< 0 > > sp1( sp0 );
        CPPUNIT_ASSERT( 0 == sp0.get() );
        CPPUNIT_ASSERT( 0 == sp1.get() );
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    
    // Testing assignment to a shared pointer holding 0.
    {
        SharedPointerTester< 0 >* rawPointer = new SharedPointerTester< 0 >();
        SharedPointer< SharedPointerTester< 0 > > sp0;
        SharedPointer< SharedPointerTester< 0 > > sp1( rawPointer );
        SharedPointer< SharedPointerTester< 0 > > sp2( sp0 );
        
        sp0 = sp1;
        CPPUNIT_ASSERT( 2 == sp0.useCount() );
        CPPUNIT_ASSERT( 2 == sp1.useCount() );
        CPPUNIT_ASSERT_EQUAL( rawPointer, sp0.get() );
        CPPUNIT_ASSERT_EQUAL( rawPointer, sp1.get() );
        CPPUNIT_ASSERT( 0 == sp2.get() );
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    
    // Testing assignment that should lead to a  destruction.
    {
        SharedPointerTester< 0 >* rawPointer0 = new SharedPointerTester< 0 >();
        SharedPointerTester< 0 >* rawPointer1 = new SharedPointerTester< 0 >();
        CPPUNIT_ASSERT_EQUAL( 2, SharedPointerTester< 0 >::static_counter_ );
        
        SharedPointer< SharedPointerTester< 0 > > sp0( rawPointer0 );
        SharedPointer< SharedPointerTester< 0 > > sp1( rawPointer1 );
        sp1 = sp0;
        
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( 2 == sp0.useCount() );
        CPPUNIT_ASSERT( 2 == sp1.useCount() );
        CPPUNIT_ASSERT_EQUAL( rawPointer0, sp0.get() );
        CPPUNIT_ASSERT_EQUAL( rawPointer0, sp1.get() );
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
}



void 
OpenSteer::SharedPointerTest::testReset()
{
    // Reset a shared pointer that manages a 0-pointer.
    {
        SharedPointer< SharedPointerTester< 0 > > sp0;
        sp0.reset();
        CPPUNIT_ASSERT( 0 == sp0.get() );
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    

    // Reset a shared pointer that manages a raw pointer.
    {
        SharedPointerTester< 0 >* rawPointer0 = new SharedPointerTester< 0 >();
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        
        SharedPointer< SharedPointerTester< 0 > > sp0( rawPointer0 );
        sp0.reset();
        CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( 0 == sp0.get() );
        
        
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );

    // Reset more than once.
    {
        SharedPointerTester< 0 >* rawPointer0 = new SharedPointerTester< 0 >();
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        
        SharedPointer< SharedPointerTester< 0 > > sp0( rawPointer0 );
        sp0.reset();
        CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( 0 == sp0.get() );
        
        sp0.reset();
        CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( 0 == sp0.get() );
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    
    // Reset a shared pointer managing a 0-pointer with a new raw pointer.
    {
        SharedPointerTester< 0 >* rawPointer0 = new SharedPointerTester< 0 >();
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        
        SharedPointer< SharedPointerTester< 0 > > sp0;
        sp0.reset( rawPointer0 );
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( rawPointer0 == sp0.get() );
        
        sp0.reset();
        CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( 0 == sp0.get() );
        
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    
    // Reset a shared pointer managing a raw pointer with a new raw pointer.
    {
        SharedPointerTester< 0 >* rawPointer0 = new SharedPointerTester< 0 >();
        SharedPointerTester< 0 >* rawPointer1 = new SharedPointerTester< 0 >();
        CPPUNIT_ASSERT_EQUAL( 2, SharedPointerTester< 0 >::static_counter_ );
        
        SharedPointer< SharedPointerTester< 0 > > sp0( rawPointer0 );
        sp0.reset( rawPointer1 );
        CPPUNIT_ASSERT_EQUAL( 1, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( rawPointer1 == sp0.get() );
        
        sp0.reset();
        CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
        CPPUNIT_ASSERT( 0 == sp0.get() );
    }
    CPPUNIT_ASSERT_EQUAL( 0, SharedPointerTester< 0 >::static_counter_ );
    
    
    
}



namespace {
 
    
    class Super {
    public:
        
        Super() {
            ++superCount_;
        }
        
        virtual ~Super() {
            --superCount_;
        }
        
        
        static int superCount_; 
    }; // class Super
    
    
    int Super::superCount_ = 0;
    
    
    class Sub: public Super {
    public:
        Sub(): Super() {
            ++subCount_;
        }
        
        virtual ~Sub() {
            --subCount_;
        }
        
        static int subCount_;
    }; // class Sub
    
    int Sub::subCount_ = 0;
    
} // anonymous namespace




void
OpenSteer::SharedPointerTest::testInheritance()
{
    {
        Sub* rawSubPointer0 = new Sub();
        SharedPointer< Super > sp0( rawSubPointer0 );
        CPPUNIT_ASSERT_EQUAL( 1, Super::superCount_ );
    }
    CPPUNIT_ASSERT_EQUAL( 0, Super::superCount_ );
    
    {
        Super* rawSuperPointer0 = new Super();
        SharedPointer< Super > sp0( rawSuperPointer0 );
        CPPUNIT_ASSERT_EQUAL( 1, Super::superCount_ );
        CPPUNIT_ASSERT_EQUAL( 0, Sub::subCount_ );
        
        Sub* rawSubPointer0 = new Sub();
        SharedPointer< Sub > sp1( rawSubPointer0 );
        CPPUNIT_ASSERT_EQUAL( 2, Super::superCount_ );
        CPPUNIT_ASSERT_EQUAL( 1, Sub::subCount_ );
        
        
        sp0 = sp1;
        CPPUNIT_ASSERT_EQUAL( 1, Super::superCount_ );
        CPPUNIT_ASSERT_EQUAL( 1, Sub::subCount_ );
    }
    CPPUNIT_ASSERT_EQUAL( 0, Super::superCount_ );
    CPPUNIT_ASSERT_EQUAL( 0, Sub::subCount_ );
}



void 
OpenSteer::SharedPointerTest::testComparisons()
{
    Super* rawSuperPointer0 = new Super();
    SharedPointer< Super > sp0( rawSuperPointer0 );
    
    Sub* rawSubPointer1 = new Sub();
    SharedPointer< Super > sp1( rawSubPointer1 );
    
    CPPUNIT_ASSERT( sp0 != sp1 );
    CPPUNIT_ASSERT( ( sp0 < sp1 ) || ( sp1 < sp0 ) );
    
    sp1 = sp0;
    
    CPPUNIT_ASSERT( sp0 == sp1 );
    CPPUNIT_ASSERT( !( sp0 < sp1 ) && !( sp1 < sp0 ) );
    
    sp0.reset();
    sp1.reset();
    
    CPPUNIT_ASSERT( sp0 == sp1 );
    // @todo Are these semantics really a good idea?
    CPPUNIT_ASSERT( ( sp0 < sp1 ) || ( sp1 < sp0 ) );
    
}




void 
OpenSteer::SharedPointerTest::testImplicitBoolCast()
{
    Super* rawSuperPointer0 = new Super();
    SharedPointer< Super > sp0;
    
    if ( !sp0 ) {
        CPPUNIT_ASSERT( true );
    } else {
        CPPUNIT_ASSERT( false );
    }
    
    sp0.reset( rawSuperPointer0 );
    if ( sp0 ) {
        CPPUNIT_ASSERT( true );
    } else {
        CPPUNIT_ASSERT( false );
    }
    
}




void
OpenSteer::SharedPointerTest::testSwap()
{
    Super* rawSuperPointer0 = new Super();
    SharedPointer< Super > sp0( rawSuperPointer0 );
    
    Super* rawSuperPointer1 = new Super();
    SharedPointer< Super > sp1( rawSuperPointer1 );
    
    CPPUNIT_ASSERT_EQUAL( 2, Super::superCount_ );
    
    sp0.swap( sp1 );
    
    CPPUNIT_ASSERT( rawSuperPointer0 == sp1.get() );
    CPPUNIT_ASSERT( rawSuperPointer1 == sp0.get() );
    CPPUNIT_ASSERT_EQUAL( 2, Super::superCount_ );
    
}




