/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/cursor.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/source/cursor_stack.hpp>
#include <GG/adobe/memory.hpp>

#include <GG/Cursor.h>
#include <GG/GUI.h>


namespace {
    adobe_cursor_t g_default_cursor;
    bool g_default_established = false;
}

/****************************************************************************************************/

adobe_cursor_t make_cursor(const char* cursor_path, float hot_spot_x, float hot_spot_y)
{
    if (!g_default_cursor)
        g_default_cursor = GG::GUI::GetGUI()->GetCursor();
    boost::shared_ptr<GG::Texture> texture = GG::GUI::GetGUI()->GetTexture(cursor_path);
    GG::Pt hotspot(GG::X(hot_spot_x + 0.5), GG::Y(hot_spot_y + 0.5));
    return adobe_cursor_t(new GG::TextureCursor(texture, hotspot));
}

/****************************************************************************************************/

void push_cursor(adobe_cursor_t cursor)
{
    if (!g_default_established) {
        g_default_cursor = GG::GUI::GetGUI()->GetCursor();
        g_default_established = true;
    }
    cursor_stack_push(cursor);
    GG::GUI::GetGUI()->SetCursor(cursor);
}

/****************************************************************************************************/

adobe_cursor_t pop_cursor()
{
    adobe_cursor_t old_cursor = cursor_stack_top();
    cursor_stack_pop();
    adobe_cursor_t new_cursor = cursor_stack_top();
    GG::GUI::GetGUI()->SetCursor(new_cursor ? new_cursor : g_default_cursor);
    return old_cursor;
}

/****************************************************************************************************/

void reset_cursor()
{
    cursor_stack_reset();
    GG::GUI::GetGUI()->SetCursor(g_default_cursor);
}

/****************************************************************************************************/

void delete_cursor(adobe_cursor_t cursor)
{}

/****************************************************************************************************/
