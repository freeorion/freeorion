/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/future/widgets/headers/number_unit.hpp>

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

unit_t to_unit(const dictionary_t& dict, const unit_t& default_unit)
{
    unit_t  single_unit(default_unit);
    array_t scale;

    get_value(dict,             key_name,                       single_unit.name_m);
    get_value(dict,             key_short_name,                 single_unit.short_name_m);
    get_value(dict,             key_bind,                       single_unit.base_unit_m);
    get_value(dict,             key_format,                     single_unit.format_m);
    get_value(dict,             key_min_max_filter,             single_unit.base_unit_filter_m);
    get_value(dict,             key_decimal_places,             single_unit.decimal_places_m);
    get_value(dict,             key_trailing_zeroes,            single_unit.trailing_zeroes_m);
    get_value(dict,             key_increment,                  single_unit.increment_m);
    get_value(dict,             key_min_value,                  single_unit.min_value_m);
    get_value(dict,             key_max_value,                  single_unit.max_value_m);
    get_value(dict,             key_scale,                              scale);

    if (scale.size() > 0)
        single_unit.scale_m_m = scale[0].cast<double>();

    if (scale.size() > 1)
        single_unit.scale_b_m = scale[1].cast<double>();

    return single_unit;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
