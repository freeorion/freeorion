/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_CURSOR_STACK_HPP
#define ADOBE_CURSOR_STACK_HPP

/****************************************************************************************************/

#include <GG/adobe/future/cursor.hpp>

/****************************************************************************************************/

void           cursor_stack_push(adobe_cursor_t cursor);
void           cursor_stack_pop();
adobe_cursor_t cursor_stack_top();
void           cursor_stack_reset();

/****************************************************************************************************/

// ADOBE_CURSOR_STACK_HPP
#endif

/****************************************************************************************************/
