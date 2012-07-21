/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// tab_group.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_progress_bar.hpp>

#include <GG/adobe/future/widgets/headers/progress_bar_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   progress_bar_t*&    widget)
{
    bool                                is_indeterminate(false);
    bool                                is_relevance(false);
    name_t                       orientation(key_horizontal);
    dictionary_t::const_iterator iter(parameters.find(key_format));
    value_range_format_t                format;
     
     if (iter != parameters.end())
        format.set(iter->second.cast<dictionary_t>());

    get_value(parameters, key_is_indeterminate, is_indeterminate);
    get_value(parameters, key_is_relevance, is_relevance);
    get_value(parameters, key_orientation, orientation);

    bool    is_vertical(orientation == key_vertical);

    pb_style_t           bar_theme(pb_style_progress_bar_s);
    if (is_relevance)           bar_theme = pb_style_relevance_bar_s;
    else if (is_indeterminate)  bar_theme = pb_style_indeterminate_bar_s;

    widget = new progress_bar_t(bar_theme, 
                                is_vertical, 
                                format, 
                                implementation::size_to_theme(size));
}

/*************************************************************************************************/

template <typename Sheet, typename FactoryToken>
inline void couple_controller_to_cell(progress_bar_t&,
                                      name_t,
                                      Sheet&,
                                      const FactoryToken&,
                                      const dictionary_t&)

{
    // no adam interaction
}

/****************************************************************************************************/

widget_node_t make_progress_bar(const dictionary_t&     parameters,
                                const widget_node_t&    parent,
                                const factory_token_t&  token,
                                const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<progress_bar_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("progress_bar")), 
        factory.layout_attributes(static_name_t("progress_bar"))); 
}

/*************************************************************************************************/

} // namespace adobe
