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
 * Utilitites to get the next or previous segment of a segmented path alike.
 */
#ifndef OPENSTEER_SEGMENTEDPATHALIKEUTILITIES_H
#define OPENSTEER_SEGMENTEDPATHALIKEUTILITIES_H


// Include assert
#include <cassert>


// Include OpenSteer::size_t
#include "OpenSteer/StandardTypes.h"


namespace OpenSteer {
    
    /**
     * Returns the next Segment after @a segmentIndex for @a pathAlike.
     * 
     * If @a segmentIndex is the last valid index and @a pathAlike is cylic the
     * first segment index is returned. If @a pathAlike isn't cyclic the last 
     * valid segment index is returned.
     *
     * @c SegmentedPathAlike must provide the following member functions:
     * <code>size_t  segmentCount() const</code>
     * <code>bool isValid() const </code>
     * <code>bool isCyclic() const </code>
     */
    template< typename SegmentedPathAlike >
    size_t nextSegment( SegmentedPathAlike const& pathAlike, size_t segmentIndex ) {
        assert( pathAlike.isValid() && "pathAlike isn't valid." );
        assert( segmentIndex < pathAlike.segmentCount() && "segmentIndex out of range." );
        
        ++segmentIndex;
        
        if ( segmentIndex == pathAlike.segmentCount() ) {
            
            if ( pathAlike.isCyclic() ) {
                segmentIndex = 0;
            } else {
                --segmentIndex;
            }
        } 
        
        return segmentIndex;
    }
    
    
    /**
     * Returns the previous Segment before @a segmentIndex for @a pathAlike.
     * 
     * If @a segmentIndex is the first valid index and @a pathAlike is cylic the
     * last segment index is returned. If @a pathAlike isn't cyclic the first 
     * valid segment index is returned.
     *
     * @c SegmentedPathAlike must provide the following member functions:
     * <code>size_t  segmentCount() const</code>
     * <code>bool isValid() const </code>
     * <code>bool isCyclic() const </code>
     */
    template< typename SegmentedPathAlike >
    size_t previousSegment( SegmentedPathAlike const& pathAlike, size_t segmentIndex ) {
        assert( pathAlike.isValid() && "pathAlike isn't valid." );
        assert( segmentIndex < pathAlike.segmentCount() && "segmentIndex out of range." );
        

        if ( 0 != segmentIndex ) {
            --segmentIndex;

        } else if ( pathAlike.isCyclic() ) {
            segmentIndex = pathAlike.segmentCount() - 1;
        }
        
        return segmentIndex;
    }  
    
    
    
} // namespace OpenSteer


#endif // OPENSTEER_SEGMENTEDPATHALIKEUTILITIES_H
