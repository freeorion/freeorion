/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/button_helper.hpp>

// REVISIT (fbrereto) : This source file should not reach into widgets
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/adobe/array.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/string.hpp>

/****************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

modifiers_t name_to_modifer(name_t name)
{
    /*
        REVISIT (sparent) : need a little lookup table here. I would still like to see us get
        to a semantic notion for modifiers.
    */
    if (name == key_modifier_option) return modifiers_any_option_s;
    if (name == key_modifier_command) return modifiers_any_command_s;
    if (name == key_modifier_control) return modifiers_any_control_s;
    if (name == key_modifier_shift) return modifiers_any_shift_s;
    
    // REVISIT (sparent) : put an assert here later to clean out these older names.
    
    if (name == key_modifiers_cmd)
        return modifiers_any_command_s;
    if (name == key_modifiers_ctl)
        return modifiers_any_control_s;
    if (name == key_modifiers_ctlcmd)
        return modifiers_any_control_s | modifiers_any_command_s;
    if (name == key_modifiers_opt)
        return modifiers_any_option_s;
    if (name == key_modifiers_optcmd)
        return modifiers_any_option_s | modifiers_any_command_s;
    if (name == key_modifiers_optctl)
        return modifiers_any_option_s | modifiers_any_control_s;
    if (name == key_modifiers_optctlcmd)
        return modifiers_any_option_s | modifiers_any_control_s | modifiers_any_command_s;
    
    std::string error("unknown modifier: ");
    error << name.c_str();
    throw std::invalid_argument(error);
}

/*************************************************************************************************/

modifiers_t value_to_modifier(const any_regular_t& modifier_set)
{
    modifiers_t result(modifiers_none_s);

    if (modifier_set.type_info() == type_info<name_t>())
    {
        result |= name_to_modifer(modifier_set.cast<name_t>());
    }
    else
    {
        const array_t& array = modifier_set.cast<array_t>();
        
        /*
            REVISIT (sparent) : This is a transform and accumulate - should be an
            easy way to compose such a thing.
        */
        for (array_t::const_iterator first(array.begin()), last(array.end());
                first != last; ++first)
        {
            result |= name_to_modifer(first->cast<name_t>());
        }
        
    }
    return result;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
