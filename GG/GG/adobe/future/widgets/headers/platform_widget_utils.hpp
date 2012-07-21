/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_UI_CORE_OS_UTILITIES_HPP
#define ADOBE_UI_CORE_OS_UTILITIES_HPP

/****************************************************************************************************/

#include <GG/WndEvent.h>

#include <GG/adobe/config.hpp>

#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/filesystem/path.hpp>

#include <string>


namespace GG {
    class Control;
    class Edit;
    class Font;
    class MultiEdit;
    class StyleFactory;
    class TextControl;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

typedef GG::Wnd* platform_control_type;

/****************************************************************************************************/

void get_control_bounds(GG::Wnd* control, GG::Rect& bounds);

void set_control_bounds(GG::Wnd* control, const place_data_t& place_data);

template <typename T>
const std::string& get_field_text(T& x)
{ return get_field_text(x.control_m); }

template <>
const std::string& get_field_text<GG::Edit*>(GG::Edit*& x);
template <>
const std::string& get_field_text<GG::MultiEdit*>(GG::MultiEdit*& x);
template <>
const std::string& get_field_text<GG::TextControl*>(GG::TextControl*& x);

bool is_focused(GG::Wnd* w);

GG::StyleFactory& Factory();

boost::shared_ptr<GG::Font> DefaultFont();

GG::X CharWidth();

GG::Y CharHeight();

GG::Y StandardHeight();

GG::Pt NonClientSize(GG::Wnd& w);

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

void set_control_visible(GG::Wnd* control, bool make_visible);

/****************************************************************************************************/

bool context_menu(const GG::Pt& pt,
                  const name_t* first,
                  const name_t* last,
                  name_t& result);

modifiers_t convert_modifiers(GG::Flags<GG::ModKey> modifiers);

modifiers_t modifier_state();

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#ifndef ADOBE_THROW_LAST_ERROR
    #define ADOBE_THROW_LAST_ERROR adobe::implementation::throw_last_error_exception(__FILE__, __LINE__)
#endif

/****************************************************************************************************/

#endif

/****************************************************************************************************/
