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
//
// Pathway and PolylinePathway, for path following.
//
// 06-08-05 bk:  Deprecated
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 06-03-02 cwr: created
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_OLD_PATHWAY_H
#define OPENSTEER_OLD_PATHWAY_H


#include "OpenSteer/Vec3.h"


namespace OpenSteer {
    
    namespace Old {

        /**
         * Pathway: a pure virtual base class for an abstract pathway in space, as for
         * example would be used in path following.
         *
         * @deprecated Use @c OpenSteer::Pathway instead.
         */
        class Pathway
        {
        public:
            Pathway() { }
            virtual ~Pathway() { }

            // Given an arbitrary point ("A"), returns the nearest point ("P") on
            // this path.  Also returns, via output arguments, the path tangent at
            // P and a measure of how far A is outside the Pathway's "tube".  Note
            // that a negative distance indicates A is inside the Pathway.
            virtual Vec3 mapPointToPath (const Vec3& point,
                Vec3& tangent,
                float& outside) = 0;

            // given a distance along the path, convert it to a point on the path
            virtual Vec3 mapPathDistanceToPoint (float pathDistance) = 0;

            // Given an arbitrary point, convert it to a distance along the path.
            virtual float mapPointToPathDistance (const Vec3& point) = 0;

            // is the given point inside the path tube?
            bool isInsidePath (const Vec3& point)
            {
                float outside; Vec3 tangent;
                mapPointToPath (point, tangent, outside);
                return outside < 0;
            }

            // how far outside path tube is the given point?  (negative is inside)
            float howFarOutsidePath (const Vec3& point)
            {
                float outside; Vec3 tangent;
                mapPointToPath (point, tangent, outside);
                return outside;
            }
        };


        /**
         * PolylinePathway: a simple implementation of the Pathway protocol.  The path
         * is a "polyline" a series of line segments between specified points.  A
         * radius defines a volume for the path which is the union of a sphere at each
         * point and a cylinder along each segment.
         *
         * @deprecated Use @c OpenSteer::PolylineSegmentedPathway instead.
         */
        class PolylinePathway: public virtual Pathway
        {
        public:

            int pointCount;
            Vec3* points;
            float radius;
            bool cyclic;

            PolylinePathway (void) {}
            virtual ~PolylinePathway() { }

            // construct a PolylinePathway given the number of points (vertices),
            // an array of points, and a path radius.
            PolylinePathway (const int _pointCount,
                const Vec3 _points[],
                const float _radius,
                const bool _cyclic);

            // utility for constructors in derived classes
            void initialize (const int _pointCount,
                const Vec3 _points[],
                const float _radius,
                const bool _cyclic);

            // utility for constructors in derived classes
            void 
            setupLengths ();

            // move existing points safely
            void movePoints (const int _firstPoint,
                            const int _numPoints,
                            const Vec3 _points[]);

            // Given an arbitrary point ("A"), returns the nearest point ("P") on
            // this path.  Also returns, via output arguments, the path tangent at
            // P and a measure of how far A is outside the Pathway's "tube".  Note
            // that a negative distance indicates A is inside the Pathway.
            Vec3 mapPointToPath (const Vec3& point, Vec3& tangent, float& outside);


            // given an arbitrary point, convert it to a distance along the path
            float mapPointToPathDistance (const Vec3& point);

            // given a distance along the path, convert it to a point on the path
            Vec3 mapPathDistanceToPoint (float pathDistance);

            // utility methods

            // compute minimum distance from a point to a line segment
            float pointToSegmentDistance (const Vec3& point,
                const Vec3& ep0,
                const Vec3& ep1);

            // assessor for total path length;
            float getTotalPathLength (void) {return totalPathLength;};

            // XXX removed the "private" because it interfered with derived
            // XXX classes later this should all be rewritten and cleaned up
            // private:

            // xxx shouldn't these 5 just be local variables?
            // xxx or are they used to pass secret messages between calls?
            // xxx seems like a bad design
            float segmentLength;
            float segmentProjection;
            Vec3 local;
            Vec3 chosen;
            Vec3 segmentNormal;

            float* lengths;
            Vec3* normals;
            float totalPathLength;
        };

    } // namespace Old
        
} // namespace OpenSteer
    
    
// ----------------------------------------------------------------------------
#endif // OPENSTEER_OLD_PATHWAY_H
