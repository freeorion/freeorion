/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// group.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_group.hpp>

#include <GG/adobe/future/widgets/headers/group_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>


namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   group_t*&           group)
{
    std::string    name;
    std::string    alt_text;
    theme_t theme(implementation::size_to_theme(size));

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);

    group = new group_t(name, alt_text, theme);
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(group_t&,
                                const dictionary_t&,
                                const factory_token_t&,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    // no adam interaction
}

/****************************************************************************************************/

widget_node_t make_group(const dictionary_t&     parameters, 
                         const widget_node_t&    parent, 
                         const factory_token_t&  token,
                         const widget_factory_t& factory)
    {
        return create_and_hookup_widget<group_t, poly_placeable_t>(
            parameters, parent, token,
            factory.is_container(static_name_t("group")),
            factory.layout_attributes(static_name_t("group"))
        );
    }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
