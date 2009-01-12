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
// OpenSteerDemo PlugIn class
//
// Provides AbstractPlugIn a pure abstract base class, and PlugIn a partial
// implementation providing default methods to be sub-classed by the
// programmer defining a new "MyPlugIn".
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 11-13-02 cwr: created 
//
//
// ----------------------------------------------------------------------------
// A pattern for a specific user-defined OpenSteerDemo PlugIn class called Foo.
// Defines class FooPlugIn, then makes a single instance (singleton) of it.
/*


class FooPlugIn : public PlugIn
{
    // required methods:
    const char* name (void) {return "Foo";}
    void open (void) {...}
    void update (const float currentTime, const float elapsedTime) {...}
    void redraw (const float currentTime, const float elapsedTime) {...}
    void close (void) {...}
    const AVGroup& allVehicles (void) {...}

    // optional methods (see comments in AbstractPlugIn for explanation):
    void reset (void) {...} // default is to reset by doing close-then-open
    float selectionOrderSortKey (void) {return 1234;}
    bool requestInitialSelection (void) {return true;}
    void handleFunctionKeys (int keyNumber) {...} // fkeys reserved for PlugIns
    void printMiniHelpForFunctionKeys (void) {...} // if fkeys are used
};

FooPlugIn gFooPlugIn;


*/
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_PLUGIN_H
#define OPENSTEER_PLUGIN_H

#include <iostream>
#include "OpenSteer/AbstractVehicle.h"


// ----------------------------------------------------------------------------


namespace OpenSteer {

    class AbstractPlugIn
    {
    public:
        
        virtual ~AbstractPlugIn() { /* Nothing to do. */ }
        
        // generic PlugIn actions: open, update, redraw, close and reset
        virtual void open (void) = 0;
        virtual void update (const float currentTime, const float elapsedTime) = 0;
        virtual void redraw (const float currentTime, const float elapsedTime) = 0;
        virtual void close (void) = 0;
        virtual void reset (void) = 0;

        // return a pointer to this instance's character string name
        virtual const char* name (void) = 0;

        // numeric sort key used to establish user-visible PlugIn ordering
        // ("built ins" have keys greater than 0 and less than 1)
        virtual float selectionOrderSortKey (void) = 0;

        // allows a PlugIn to nominate itself as OpenSteerDemo's initially selected
        // (default) PlugIn, which is otherwise the first in "selection order"
        virtual bool requestInitialSelection (void) = 0;

        // handle function keys (which are reserved by SterTest for PlugIns)
        virtual void handleFunctionKeys (int keyNumber) = 0;

        // print "mini help" documenting function keys handled by this PlugIn
        virtual void printMiniHelpForFunctionKeys (void) = 0;

        // return an AVGroup (an STL vector of AbstractVehicle pointers) of
        // all vehicles(/agents/characters) defined by the PlugIn
        virtual const AVGroup& allVehicles (void) = 0;
    };


    class PlugIn : public AbstractPlugIn
    {
    public:
        // prototypes for function pointers used with PlugIns
        typedef void (* plugInCallBackFunction) (PlugIn& clientObject);
        typedef void (* voidCallBackFunction) (void);
        typedef void (* timestepCallBackFunction) (const float currentTime,
                                                   const float elapsedTime);

        // constructor
        PlugIn (void);

        // destructor
        virtual ~PlugIn();

        // default reset method is to do a close then an open
        void reset (void) {close (); open ();}

        // default sort key (after the "built ins")
        float selectionOrderSortKey (void) {return 1.0f;}

        // default is to NOT request to be initially selected
        bool requestInitialSelection (void) {return false;}

        // default function key handler: ignore all
        // (parameter names commented out to prevent compiler warning from "-W")
        void handleFunctionKeys (int /*keyNumber*/) {}

        // default "mini help": print nothing
        void printMiniHelpForFunctionKeys (void) {}

        // returns pointer to the next PlugIn in "selection order"
        PlugIn* next (void);

        // format instance to characters for printing to stream
        friend std::ostream& operator<< (std::ostream& os, PlugIn& pi)
        {
            os << "<PlugIn " << '"' << pi.name() << '"' << ">";
            return os;
        }

        // CLASS FUNCTIONS

        // search the class registry for a Plugin with the given name
        static PlugIn* findByName (const char* string);

        // apply a given function to all PlugIns in the class registry
        static void applyToAll (plugInCallBackFunction f);

        // sort PlugIn registry by "selection order"
        static void sortBySelectionOrder (void);

        // returns pointer to default PlugIn (currently, first in registry)
        static PlugIn* findDefault (void);

    private:

        // save this instance in the class's registry of instances
        void addToRegistry (void);

        // This array stores a list of all PlugIns.  It is manipulated by the
        // constructor and destructor, and used in findByName and applyToAll.
        static const int totalSizeOfRegistry;
        static int itemsInRegistry;
        static PlugIn* registry[];
    };

} // namespace OpenSteer    
    

// ----------------------------------------------------------------------------
#endif // OPENSTEER_PLUGIN_H
