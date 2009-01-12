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

#include "OpenSteer/Color.h"

// Include OpenSteer::Vec3
#include "OpenSteer/Vec3.h"

// Include assert
#include <cassert>


OpenSteer::Color::Color()
    : r_(1.0f), g_(1.0f), b_(1.0f), a_ (1.0f)
{
    
}


OpenSteer::Color::Color( float greyValue )
    : r_( greyValue ), g_( greyValue ), b_( greyValue ), a_ (1.0f)
{
    
}


OpenSteer::Color::Color( float rValue, float gValue, float bValue, float aValue )
    : r_( rValue ), g_( gValue ), b_( bValue ), a_( aValue )
{
    
}


OpenSteer::Color::Color( Vec3 const& vector )
    : r_( vector.x ), g_( vector.y ), b_( vector.z ), a_ (1.0f)
{
    
}



float 
OpenSteer::Color::r() const
{
    return r_;
}


float 
OpenSteer::Color::g() const
{
    return g_;
}


float 
OpenSteer::Color::b() const
{
    return b_;
}


float 
OpenSteer::Color::a() const
{
    return a_;
}



void 
OpenSteer::Color::setR( float value )
{
    r_ = value;
}


void 
OpenSteer::Color::setG( float value )
{
    g_ = value;
}


void 
OpenSteer::Color::setB( float value )
{
    b_ = value;
}

void 
OpenSteer::Color::setA( float value )
{
	a_ = value;
}

void
OpenSteer::Color::set( float rValue, float gValue, float bValue, float aValue )
{
    r_ = rValue;
    g_ = gValue;
    b_ = bValue;
	a_ = aValue;
}


OpenSteer::Vec3 
OpenSteer::Color::convertToVec3() const
{
    return Vec3( r_, g_, b_ );
}


OpenSteer::Color& 
OpenSteer::Color::operator+=( Color const& other )
{
    r_ += other.r_;
    g_ += other.g_;
    b_ += other.b_;
    return *this;
}


OpenSteer::Color& 
OpenSteer::Color::operator-=( Color const& other )
{
    r_ -= other.r_;
    g_ -= other.g_;
    b_ -= other.b_;
    return *this;    
}


OpenSteer::Color& 
OpenSteer::Color::operator*=( float factor )
{
    r_ *= factor;
    g_ *= factor;
    b_ *= factor;
    return *this;    
}


OpenSteer::Color& 
OpenSteer::Color::operator/=( float factor )
{
    assert( 0.0f != factor && "Division by zero." );
    return operator*=( 1.0f / factor );
}






OpenSteer::Color 
OpenSteer::grayColor( float value )
{
    return Color( value );
}



OpenSteer::Color 
OpenSteer::operator+( Color const& lhs, Color const& rhs )
{
    Color result( lhs );
    return result += rhs;
}


OpenSteer::Color 
OpenSteer::operator-( Color const& lhs, Color const& rhs )
{
    Color result( lhs );
    return result -= rhs;    
}


OpenSteer::Color 
OpenSteer::operator*( Color const& lhs, float rhs )
{
    Color result( lhs );
    return result *= rhs;
}


OpenSteer::Color 
OpenSteer::operator*( float lhs, Color const& rhs )
{
    return operator*( rhs, lhs );
}


OpenSteer::Color 
OpenSteer::operator/( Color const& lhs, float rhs )
{
    Color result( lhs );
    return result /= rhs;
}







OpenSteer::Color const OpenSteer::gBlack(0.0f, 0.0f, 0.0f);
OpenSteer::Color const OpenSteer::gWhite(1.0f, 1.0f, 1.0f);

OpenSteer::Color const OpenSteer::gRed(1.0f, 0.0f, 0.0f); 
OpenSteer::Color const OpenSteer::gGreen(0.0f, 1.0f, 0.0f);
OpenSteer::Color const OpenSteer::gBlue(0.0f, 0.0f, 1.0f);
OpenSteer::Color const OpenSteer::gYellow(1.0f, 1.0f, 0.0f);
OpenSteer::Color const OpenSteer::gCyan(0.0f, 1.0f, 1.0f);
OpenSteer::Color const OpenSteer::gMagenta(1.0f, 0.0f, 1.0f);
OpenSteer::Color const OpenSteer::gOrange(1.0f, 0.5f, 0.0f);

OpenSteer::Color const OpenSteer::gDarkRed(0.5f, 0.0f, 0.0f);
OpenSteer::Color const OpenSteer::gDarkGreen(0.0f, 0.5f, 0.0f);
OpenSteer::Color const OpenSteer::gDarkBlue(0.0f, 0.0f, 0.5f);
OpenSteer::Color const OpenSteer::gDarkYellow(0.5f, 0.5f, 0.0f);
OpenSteer::Color const OpenSteer::gDarkCyan(0.0f, 0.5f, 0.5f);
OpenSteer::Color const OpenSteer::gDarkMagenta(0.5f, 0.0f, 0.5f);
OpenSteer::Color const OpenSteer::gDarkOrange(0.5f, 0.25f, 0.0f);

OpenSteer::Color const OpenSteer::gGray10(0.1f);
OpenSteer::Color const OpenSteer::gGray20(0.2f);
OpenSteer::Color const OpenSteer::gGray30(0.3f);
OpenSteer::Color const OpenSteer::gGray40(0.4f);
OpenSteer::Color const OpenSteer::gGray50(0.5f);
OpenSteer::Color const OpenSteer::gGray60(0.6f);
OpenSteer::Color const OpenSteer::gGray70(0.7f);
OpenSteer::Color const OpenSteer::gGray80(0.8f);
OpenSteer::Color const OpenSteer::gGray90(0.9f);

