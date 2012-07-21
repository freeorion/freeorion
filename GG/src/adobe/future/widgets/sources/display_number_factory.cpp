/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_display_number.hpp>

#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/display_number_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/adobe/static_table.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t&       parameters,
                   size_enum_t               size,
                   display_number_t*&        display_number)
{
    std::string name;
    std::string alt_text;
    theme_t     theme(implementation::size_to_theme(size));
    long        characters(5);
    unit_t      default_unit(to_unit(parameters));

    std::vector<unit_t> unit_set;

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_characters, characters);

    if (parameters.count(key_units) == 0)
    {
        unit_set.push_back(default_unit);
    }
    else
    {
        array_t unit_array(get_value(parameters, key_units).cast<array_t>());

        for (array_t::iterator iter(unit_array.begin()), last(unit_array.end());
             iter != last; ++iter)
            unit_set.push_back(to_unit(iter->cast<dictionary_t>(), default_unit));
    }

    display_number = new display_number_t(name, alt_text, unit_set.begin(), unit_set.end(),
                                          theme, characters);
}

/*************************************************************************************************/

template <typename Sheet, typename FactoryToken>
inline void couple_controller_to_cell(display_number_t&,
                                      name_t,
                                      Sheet&,
                                      const FactoryToken&,
                                      const dictionary_t&)

{
    // no adam interaction
}

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/
    
widget_node_t make_display_number(const dictionary_t&     parameters, 
                                         const widget_node_t&    parent, 
                                         const factory_token_t&  token,
                                         const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<display_number_t, poly_placeable_twopass_t>(
        parameters, parent, token, 
        factory.is_container(static_name_t("display_number")), 
        factory.layout_attributes(static_name_t("display_number"))); 
}
 
/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

