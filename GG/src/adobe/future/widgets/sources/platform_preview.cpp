/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_preview.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

preview_t::preview_t(const std::string& alt_text,
                     theme_t            theme) :
    alt_text_m(alt_text),
    sublayout_m(theme),
    theme_m(theme)
{ }

/****************************************************************************************************/

void preview_t::measure(extents_t& result)
{ sublayout_m.measure(result); }

/****************************************************************************************************/

void preview_t::place(const place_data_t& place_data)
{ sublayout_m.place(place_data); }

/****************************************************************************************************/

void preview_t::display(const view_model_type& value)
{ sublayout_m.sublayout_sheet_set_update(static_name_t("image"), value); }

/****************************************************************************************************/

void preview_t::enable(bool /*make_enabled*/)
{}

/****************************************************************************************************/

void preview_t::monitor(const setter_proc_type& /*proc*/)
{}

/****************************************************************************************************/

template <>
platform_display_type insert<preview_t>(display_t&             display,
                                        platform_display_type& parent,
                                        preview_t&             element)
{ return parent; }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
