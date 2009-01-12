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
// ------------------------------------------------------------------------
//
//
// Main: top level routine for OpenSteerDemo application
//
//  5-29-02 cwr: created
//
//
// ------------------------------------------------------------------------


#include "OpenSteer/OpenSteerDemo.h"        // OpenSteerDemo application
#include "OpenSteer/Draw.h"                 // OpenSteerDemo graphics

// To include EXIT_SUCCESS
#include <cstdlib>


int main (int argc, char **argv) 
{
    // initialize OpenSteerDemo application
    OpenSteer::OpenSteerDemo::initialize ();

    // initialize graphics
    OpenSteer::initializeGraphics (argc, argv);

    // run the main event processing loop
    OpenSteer::runGraphics ();  
    return EXIT_SUCCESS;
}


// ------------------------------------------------------------------------
