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
//
// Vec3: OpenSteer's generic type for 3d vectors
//
// This file defines the class Vec3, which is used throughout OpenSteer to
// manipulate 3d geometric data.  It includes standard vector operations (like
// vector addition, subtraction, scale, dot, cross...) and more idiosyncratic
// utility functions.
//
// When integrating OpenSteer into a preexisting 3d application, it may be
// important to use the 3d vector type of that application.  In that case Vec3
// can be changed to inherit from the preexisting application' vector type and
// to match the interface used by OpenSteer to the interface provided by the
// preexisting 3d vector type.
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 03-26-03 cwr: created to replace for Hiranabe-san's execellent but larger
//               vecmath package (http://objectclub.esm.co.jp/vecmath/)
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_VEC3_H
#define OPENSTEER_VEC3_H


#include "OpenSteer/Utilities.h"  // for interpolate, etc.


namespace OpenSteer {

    // ----------------------------------------------------------------------------


    class Vec3
    {
    public:

        // ----------------------------------------- generic 3d vector operations

        // three-dimensional Cartesian coordinates
        float x, y, z;

        // constructors
        Vec3 (void): x( 0.0f ), y( 0.0f ), z( 0.0f ) {}
        Vec3 (float X, float Y, float Z) : x( X ), y( Y ), z( Z ) {}

        // vector addition
        Vec3 operator+ (const Vec3& v) const {return Vec3 (x+v.x, y+v.y, z+v.z);}

        // vector subtraction
        Vec3 operator- (const Vec3& v) const {return Vec3 (x-v.x, y-v.y, z-v.z);}

        // unary minus
        Vec3 operator- (void) const {return Vec3 (-x, -y, -z);}

        // vector times scalar product (scale length of vector times argument)
        Vec3 operator* (const float s) const {return Vec3 (x * s, y * s, z * s);}

        // vector divided by a scalar (divide length of vector by argument)
        Vec3 operator/ (const float s) const {return Vec3 (x / s, y / s, z / s);}

        // dot product
        float dot (const Vec3& v) const {return (x * v.x) + (y * v.y) + (z * v.z);}

        // length
        float length (void) const {return sqrtXXX (lengthSquared ());}

        // length squared
        float lengthSquared (void) const {return this->dot (*this);}

        // normalize: returns normalized version (parallel to this, length = 1)
        Vec3 normalize (void) const
        {
            // skip divide if length is zero
            const float len = length ();
            return (len>0) ? (*this)/len : (*this);
        }

        // cross product (modify "*this" to be A x B)
        // [XXX  side effecting -- deprecate this function?  XXX]
        void cross(const Vec3& a, const Vec3& b)
        {
            *this = Vec3 ((a.y * b.z) - (a.z * b.y),
                          (a.z * b.x) - (a.x * b.z),
                          (a.x * b.y) - (a.y * b.x));
        }

        // assignment
        Vec3 operator= (const Vec3& v) {x=v.x; y=v.y; z=v.z; return *this;}

        // set XYZ coordinates to given three floats
        Vec3 set (const float _x, const float _y, const float _z)
        {x = _x; y = _y; z = _z; return *this;}

        // +=
        Vec3 operator+= (const Vec3& v) {return *this = (*this + v);}

        // -=
        Vec3 operator-= (const Vec3& v) {return *this = (*this - v);}

        // *=
        Vec3 operator*= (const float& s) {return *this = (*this * s);}

        
        Vec3 operator/=( float d ) { return *this = (*this / d);  }
        
        // equality/inequality
        bool operator== (const Vec3& v) const {return x==v.x && y==v.y && z==v.z;}
        bool operator!= (const Vec3& v) const {return !(*this == v);}

        // @todo Remove - use @c distance from the Vec3Utilitites header instead.
        // XXX experimental (4-1-03 cwr): is this the right approach?  defining
        // XXX "Vec3 distance (vec3, Vec3)" collided with STL's distance template.
        static float distance (const Vec3& a, const Vec3& b){ return(a-b).length();}

        // --------------------------- utility member functions used in OpenSteer

        // return component of vector parallel to a unit basis vector
        // (IMPORTANT NOTE: assumes "basis" has unit magnitude (length==1))

        inline Vec3 parallelComponent (const Vec3& unitBasis) const
        {
            const float projection = this->dot (unitBasis);
            return unitBasis * projection;
        }

        // return component of vector perpendicular to a unit basis vector
        // (IMPORTANT NOTE: assumes "basis" has unit magnitude (length==1))

        inline Vec3 perpendicularComponent (const Vec3& unitBasis) const
        {
            return (*this) - parallelComponent (unitBasis);
        }

        // clamps the length of a given vector to maxLength.  If the vector is
        // shorter its value is returned unaltered, if the vector is longer
        // the value returned has length of maxLength and is paralle to the
        // original input.

        Vec3 truncateLength (const float maxLength) const
        {
            const float maxLengthSquared = maxLength * maxLength;
            const float vecLengthSquared = this->lengthSquared ();
            if (vecLengthSquared <= maxLengthSquared)
                return *this;
            else
                return (*this) * (maxLength / sqrtXXX (vecLengthSquared));
        }

        // forces a 3d position onto the XZ (aka y=0) plane

        Vec3 setYtoZero (void) const {return Vec3 (this->x, 0, this->z);}

        // rotate this vector about the global Y (up) axis by the given angle

        Vec3 rotateAboutGlobalY (float angle) const 
        {
            const float s = sinXXX (angle);
            const float c = cosXXX (angle);
            return Vec3 ((this->x * c) + (this->z * s),
                         (this->y),
                         (this->z * c) - (this->x * s));
        }

        // version for caching sin/cos computation
        Vec3 rotateAboutGlobalY (float angle, float& sin, float& cos) const 
        {
            // is both are zero, they have not be initialized yet
            if (sin==0 && cos==0)
            {
                sin = sinXXX (angle);
                cos = cosXXX (angle);
            }
            return Vec3 ((this->x * cos) + (this->z * sin),
                         (this->y),
                         (this->z * cos) - (this->x * sin));
        }

        // if this position is outside sphere, push it back in by one diameter

        Vec3 sphericalWrapAround (const Vec3& center, float radius)
        {
            const Vec3 offset = *this - center;
            const float r = offset.length();
            if (r > radius)
                return *this + ((offset/r) * radius * -2);
            else
                return *this;
        }

        // names for frequently used vector constants
        static const Vec3 zero;
        static const Vec3 side;
        static const Vec3 up;
        static const Vec3 forward;
    };


    // ----------------------------------------------------------------------------
    // scalar times vector product ("float * Vec3")


    inline Vec3 operator* (float s, const Vec3& v) {return v*s;}


	// return cross product a x b
	inline Vec3 crossProduct(const Vec3& a, const Vec3& b)
	{
		Vec3 result((a.y * b.z) - (a.z * b.y),
					(a.z * b.x) - (a.x * b.z),
					(a.x * b.y) - (a.y * b.x));
		return result;
	}


    // ----------------------------------------------------------------------------
    // default character stream output method

#ifndef NOT_OPENSTEERDEMO  // only when building OpenSteerDemo

    inline std::ostream& operator<< (std::ostream& o, const Vec3& v)
    {
        return o << "(" << v.x << "," << v.y << "," << v.z << ")";
    }

#endif // NOT_OPENSTEERDEMO

    // ----------------------------------------------------------------------------
    // Returns a position randomly distributed inside a sphere of unit radius
    // centered at the origin.  Orientation will be random and length will range
    // between 0 and 1


    Vec3 RandomVectorInUnitRadiusSphere (void);


    // ----------------------------------------------------------------------------
    // Returns a position randomly distributed on a disk of unit radius
    // on the XZ (Y=0) plane, centered at the origin.  Orientation will be
    // random and length will range between 0 and 1


    Vec3 randomVectorOnUnitRadiusXZDisk (void);


    // ----------------------------------------------------------------------------
    // Returns a position randomly distributed on the surface of a sphere
    // of unit radius centered at the origin.  Orientation will be random
    // and length will be 1


    inline Vec3 RandomUnitVector (void)
    {
        return RandomVectorInUnitRadiusSphere().normalize();
    }


    // ----------------------------------------------------------------------------
    // Returns a position randomly distributed on a circle of unit radius
    // on the XZ (Y=0) plane, centered at the origin.  Orientation will be
    // random and length will be 1


    inline Vec3 RandomUnitVectorOnXZPlane (void)
    {
        return RandomVectorInUnitRadiusSphere().setYtoZero().normalize();
    }


    // ----------------------------------------------------------------------------
    // used by limitMaxDeviationAngle / limitMinDeviationAngle below


    Vec3 vecLimitDeviationAngleUtility (const bool insideOrOutside,
                                        const Vec3& source,
                                        const float cosineOfConeAngle,
                                        const Vec3& basis);


    // ----------------------------------------------------------------------------
    // Enforce an upper bound on the angle by which a given arbitrary vector
    // diviates from a given reference direction (specified by a unit basis
    // vector).  The effect is to clip the "source" vector to be inside a cone
    // defined by the basis and an angle.


    inline Vec3 limitMaxDeviationAngle (const Vec3& source,
                                        const float cosineOfConeAngle,
                                        const Vec3& basis)
    {
        return vecLimitDeviationAngleUtility (true, // force source INSIDE cone
                                              source,
                                              cosineOfConeAngle,
                                              basis);
    }


    // ----------------------------------------------------------------------------
    // Enforce a lower bound on the angle by which a given arbitrary vector
    // diviates from a given reference direction (specified by a unit basis
    // vector).  The effect is to clip the "source" vector to be outside a cone
    // defined by the basis and an angle.


    inline Vec3 limitMinDeviationAngle (const Vec3& source,
                                        const float cosineOfConeAngle,
                                        const Vec3& basis)
    {    
        return vecLimitDeviationAngleUtility (false, // force source OUTSIDE cone
                                              source,
                                              cosineOfConeAngle,
                                              basis);
    }


    // ----------------------------------------------------------------------------
    // Returns the distance between a point and a line.  The line is defined in
    // terms of a point on the line ("lineOrigin") and a UNIT vector parallel to
    // the line ("lineUnitTangent")


    inline float distanceFromLine (const Vec3& point,
                                   const Vec3& lineOrigin,
                                   const Vec3& lineUnitTangent)
    {
        const Vec3 offset = point - lineOrigin;
        const Vec3 perp = offset.perpendicularComponent (lineUnitTangent);
        return perp.length();
    }


    // ----------------------------------------------------------------------------
    // given a vector, return a vector perpendicular to it (note that this
    // arbitrarily selects one of the infinitude of perpendicular vectors)


    Vec3 findPerpendicularIn3d (const Vec3& direction);


    // ----------------------------------------------------------------------------
    // candidates for global utility functions
    //
    // dot
    // cross
    // length
    // distance
    // normalized

    
} // namespace OpenSteer
    

// ----------------------------------------------------------------------------
#endif // OPENSTEER_VEC3_H
