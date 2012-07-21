/*
    Copyright 2005-2006 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/sublayout.hpp>

#include <GG/adobe/adam_evaluate.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

#include <sstream>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

widget_node_t sublayout_t::evaluate(const std::string&       sheet_description,
                                    const std::string&       layout_description,
                                    const dictionary_t&      parameters,
                                    const widget_node_t&     parent,
                                    const factory_token_t&   token,
                                    const widget_factory_t&  factory,
                                    const button_notifier_t& notifier,
                                    behavior_t&              behavior)
{
    static const name_t               panel_name_s(static_name_t("panel"));
    static const layout_attributes_t& attrs_s(factory.layout_attributes(panel_name_s));

    std::stringstream sheet_stream(sheet_description);

    parse(sheet_stream, line_position_t("sublayout sheet"), bind_to_sheet( sublayout_sheet_m ) );

    size_enum_t size(parameters.count(key_size) ?
                     implementation::enumerate_size(get_value(parameters, key_size).cast<name_t>()) :
                     parent.size_m);

    platform_display_type display_token(insert(get_main_display(), parent.display_token_m, root_m));

    std::stringstream layout_stream(layout_description);

    sublayout_holder_m = make_view(static_name_t("sublayout layout"),
                                   line_position_t::getline_proc_t(),
                                   layout_stream,
                                   sublayout_sheet_m,
                                   behavior,
                                   notifier,
                                   size,
                                   default_widget_factory_proc_with_factory(factory),
                                   root_m.control_m);

    eve_t::iterator eve_token =
        attach_placeable<poly_placeable_t>(parent.eve_token_m,
                                                 *this, parameters, token,
                                                 true, attrs_s);

    return widget_node_t(size, eve_token, display_token, parent.keyboard_token_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
