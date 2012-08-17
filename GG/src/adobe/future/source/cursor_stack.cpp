/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/source/cursor_stack.hpp>

#include <vector>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

std::vector<adobe_cursor_t>& cursor_stack()
{
    static std::vector<adobe_cursor_t> cursor_stack_s;

    return cursor_stack_s;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

void cursor_stack_push(adobe_cursor_t cursor)
{
    cursor_stack().push_back(cursor);
}

/****************************************************************************************************/

void cursor_stack_pop()
{
    cursor_stack().pop_back();
}

/****************************************************************************************************/

adobe_cursor_t cursor_stack_top()
{
    return cursor_stack().empty() ? adobe_cursor_t() : cursor_stack().back();
}

/****************************************************************************************************/

void cursor_stack_reset()
{
    return cursor_stack().clear();
}

/****************************************************************************************************/
