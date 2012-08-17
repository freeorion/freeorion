/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_progress_bar.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/Control.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void progress_bar_t::display(const any_regular_t& value)
{
    value_m = value.type_info() == type_info<empty_t>() ? 0 : value.cast<double>();
    
    std::size_t new_position(format_m.find(value));

    if (new_position != last_m)
    {
        last_m = new_position;
        if (bar_style_m != pb_style_indeterminate_bar_s)
        {
#if 0 // TODO
            (void) SendMessageW((HWND) control_m,     // handle to destination control     
                            (UINT) PBM_SETPOS,    // message ID     
                            (WPARAM)(int) last_m,  // = (WPARAM) (int) nNewPos;    
                            (LPARAM) 0);          // = 0; not used, must be zero );
#endif
        }
    }
}

/*************************************************************************************************/

void progress_bar_t::measure(extents_t& result)
{
    assert(control_m);

    result.height() = is_vertical_m ? 100 : 15;
    result.width() = is_vertical_m ? 15 : 100;
}

/*************************************************************************************************/

void progress_bar_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/****************************************************************************************************/

progress_bar_t::progress_bar_t(pb_style_t bar_style, 
                               bool is_vertical,
                               const value_range_format_t& format,
                               theme_t theme) :
    control_m(0),
    bar_style_m(bar_style),
    is_vertical_m(is_vertical),
    format_m(format),
    theme_m(theme)
{ }

/****************************************************************************************************/

template <>
platform_display_type insert<progress_bar_t>(display_t&              display,
                                             platform_display_type&  parent,
                                             progress_bar_t&         element)
{
    assert(!"GG has no progress bar type yet!");
#if 0 // TODO
    HWND parent_hwnd(parent);

    assert(!control_m);

    DWORD style(PBS_SMOOTH);
    if (is_vertical_m)
        style |= PBS_VERTICAL;
    if (bar_style_m == pb_style_indeterminate_bar_s)
        style |= PBS_MARQUEE;
    control_m = ::CreateWindowExW(  WS_EX_COMPOSITED,
                    PROGRESS_CLASS,
                    NULL,
                    WS_CHILD | WS_VISIBLE | style,
                    0, 0, 10, 10,
                    parent,
                    0,
                    ::GetModuleHandle(NULL),
                    NULL);

    if (bar_style_m == pb_style_indeterminate_bar_s)
    {
        (void) ::SendMessageW((HWND) control_m,    // handle to destination control     
                        (UINT) PBM_SETMARQUEE,   // message ID     
                        (WPARAM)(BOOL) 1, // = (WPARAM) (bool) on or off;    
                        (LPARAM) 100);           // = (LPARAM) (UINT) time in milliseconds between updates
    }
    else
    {
        // REVISIT: this is a hack to match mac version
        (void) ::SendMessageW(       // returns LRESULT in lResult
        (HWND) control_m,             // handle to destination control
        (UINT) PBM_SETRANGE,           // message ID
        (WPARAM) 0,                    // = (WPARAM) 0; not used, must be zero
        (LPARAM) MAKELPARAM (0, 100)   // = (LPARAM) MAKELPARAM (nMinRange, nMaxRange)
        );
    }
#endif
    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe
