/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/adobe/future/widgets/headers/factory.hpp>

#include <GG/adobe/static_table.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

/*
    REVISIT (sparent) : Token is starting to look like a "binding" abstraction of some sort.
    
    We're going to add a bind function to the factory token to take care of the complexity
    of binding to a layout_sheet or sheet.
    
    REVISIT (sparent) : This code also indicates problems with the current sheet interfaces:
    
    1. There should be some way to build a single index for both sheets to cover the entire
        scope.
    2. The set() functions may become functions which return setter objects and don't need to
        do the lookup.
*/


/*************************************************************************************************/

size_enum_t enumerate_size(const name_t& size)
{
    typedef static_table<name_t, size_enum_t, 4> name_size_table_t;

    aggregate_name_t key_size_large  = { "size_large" };
    aggregate_name_t key_size_normal = { "size_normal" };
    aggregate_name_t key_size_small  = { "size_small" };
    aggregate_name_t key_size_mini   = { "size_mini" };
    static bool          init(false);

    static name_size_table_t size_table =
    {{
        name_size_table_t::entry_type(key_size_large,   size_normal_s), // REVISIT (fbrereto) : stubbed to normal
        name_size_table_t::entry_type(key_size_normal,  size_normal_s),
        name_size_table_t::entry_type(key_size_small,   size_small_s),
        name_size_table_t::entry_type(key_size_mini,    size_mini_s)
    }};

    if (!init)
    {
        size_table.sort();

        init = true;
    }

    return size_table(size);
}

/*************************************************************************************************/

theme_t size_to_theme(size_enum_t size)
{
    theme_t                       theme = theme_normal_s;
    if (size == size_small_s)     theme = theme_small_s;
    else if (size == size_mini_s) theme = theme_mini_s;

    return theme;
}

/*************************************************************************************************/

touch_set_t touch_set(const dictionary_t& parameters)
{
    array_t  touch_set_array;
    touch_set_t     touch_set;

    get_value(parameters, key_touch, touch_set_array);
    
    /*
    REVISIT (sparent) : interesting - needs to be a way to treat an array as a homogenous type.
    */
    
    // touch_set_m.insert(touch_set_m.end(), touch_set_array.begin(), touch_set_array.end());
    
    for (array_t::const_iterator iter(touch_set_array.begin()),
                last(touch_set_array.end()); iter != last; ++iter)
    {
        touch_set.push_back(iter->cast<name_t>());
    }

    return touch_set;
}

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
