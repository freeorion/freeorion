/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// preview.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_preview.hpp>

#include <GG/adobe/future/widgets/headers/preview_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace {

/****************************************************************************************************/

void preview_sublayout_notifier(adobe::preview_t&           /*widget*/,
                                adobe::name_t               /*action*/,
                                const adobe::any_regular_t& /*value*/)
{
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   preview_t*&         widget)
{
    std::string alt_text;

    get_value(parameters, key_alt_text, alt_text);

    widget = new preview_t(alt_text, implementation::size_to_theme(size));
}

/****************************************************************************************************/

widget_node_t make_preview(const dictionary_t&     parameters,
                           const widget_node_t&    parent,
                           const factory_token_t&  token,
                           const widget_factory_t& factory)
{
    static const char* layout_s =
        "layout preview_sublayout"
        "{"
        "    view group(name: 'Preview', child_horizontal: align_center)"
        "    {"
        "        image(bind_view: @image, bind_controller: @metadata);"
        ""
        "        row()"
        "        {"
        "            button(name: '-', action: @zoom_out, alt: 'Preview zoom out');"
        "            static_text(name: '100%', alt: 'Preview image zoom level');"
        "            button(name: localize('+'), action: @zoom_in, alt: 'Preview zoom in');"
        "        }"
        "    }"
        ""
        "}";

    static const char* sheet_s =
        "sheet preview_sublayout"
        "{"
        "interface:"
        "    metadata : { };"
        "    image    : image('stop.tga');"
        "}";

    assemblage_t& assemblage(token.client_holder_m.assemblage_m);
    preview_t*    widget(0);
    size_enum_t   size(parameters.count(key_size) ?
                       implementation::enumerate_size(get_value(parameters, key_size).cast<name_t>()) :
                       parent.size_m);

    create_widget(parameters, size, widget);

    assemblage_cleanup_ptr(assemblage, widget);

    widget_node_t sublayout_result(widget->evaluate(sheet_s,
                                   layout_s,
                                   parameters,
                                   parent,
                                   token,
                                   factory,
                                   boost::bind(&preview_sublayout_notifier, boost::ref(*widget), _1, _2),
                                   token.client_holder_m.root_behavior_m));

    //assemblage_cleanup_connection(assemblage, /*propagate controller_bind from sublayout to main sheet*/);
    //assemblage_cleanup_connection(assemblage, /*propagate view_bind from main sheet to sublayout*/);

    return sublayout_result;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
