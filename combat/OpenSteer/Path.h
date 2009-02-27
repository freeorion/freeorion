/**
 * OpenSteer -- Steering Behaviors for Autonomous Characters
 *
 * Copyright (c) 2002-2005, Sony Computer Entertainment America
 * Original authors: Craig Reynolds <craig_reynolds@playstation.sony.com>
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
 * Abstract interface for paths.
 */
#ifndef OPENSTEER_PATH_H
#define OPENSTEER_PATH_H





namespace OpenSteer {

    // Forward declaration.
    class Vec3;
    
    
    /**
     * Path in space that might be cyclic.
     *
     * Paths are infinitesimal thin.
     */
    class Path {
    public:
        
        
        virtual ~Path() = 0;
        
        
        /**
         * Returns @c true if the path is valid, @c false otherwise.
         */
        virtual bool isValid() const = 0;
        
        /**
         * Given an arbitrary point ("A"), returns the nearest point ("P") on
		 * this path center line.  Also returns, via output arguments, the path
         * tangent at P and a measure of how far A is outside the Pathway's 
         * "tube".  Note that a negative distance indicates A is inside the 
         * Pathway.
         *
         * If @c isValid is @c false the behavior is undefined.
         */
		virtual Vec3 mapPointToPath (const Vec3& point,
                                     Vec3& tangent,
                                     float& outside) const = 0;
        
		/**
         * Given a distance along the path, convert it to a point on the path.
         * If @c isValid is @c false the behavior is undefined.
         */
		virtual Vec3 mapPathDistanceToPoint (float pathDistance) const = 0;
        
		/**
         * Given an arbitrary point, convert it to a distance along the path.
         * If @c isValid is @c false the behavior is undefined.
         */
		virtual float mapPointToPathDistance (const Vec3& point) const = 0;
        
        /**
         * Returns @c true f the path is closed, otherwise @c false.
         */
        virtual bool isCyclic() const = 0;
        
        /**
         * Returns the length of the path.
         */
        virtual float length() const = 0;
        
    protected:
        /**
         * Protected to disable assigning instances of different inherited 
         * classes to each other.
         *
         * @todo Should this be added or not? Have to read a bit...
         */
        // Path& operator=( Path const& );
        
    }; // class Path
    
} // namespace OpenSteer


#endif // OPENSTEER_PATH_H
