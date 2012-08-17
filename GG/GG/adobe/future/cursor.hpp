/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_CURSOR_HPP
#define ADOBE_CURSOR_HPP

#include <boost/shared_ptr.hpp>


namespace GG {
    class Cursor;
}

/****************************************************************************************************/

typedef boost::shared_ptr<GG::Cursor> adobe_cursor_t;

/****************************************************************************************************/

// Allocation and deallocation of cursors. Cursor will be loaded relative to the resource root path.
adobe_cursor_t make_cursor(const char* cursor_path, float hot_spot_x, float hot_spot_y);
void           delete_cursor(adobe_cursor_t cursor);

// Stack-based cursor manipulation; you are still responsible for deleting the cursors you 
// push and pop here; these functions do no memory management.
void           push_cursor(adobe_cursor_t cursor);
adobe_cursor_t pop_cursor();

// Flushes any stack and sets the cursor to the arrow cursor-- be sure you still have
// references to all your cursors so you can delete them from memory!
void           reset_cursor();

/****************************************************************************************************/

// ADOBE_CURSOR_HPP
#endif

/****************************************************************************************************/
