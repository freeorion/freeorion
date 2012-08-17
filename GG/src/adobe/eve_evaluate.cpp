/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/eve_evaluate.hpp>

#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/static_table.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/virtual_machine.hpp>
#include <GG/adobe/once.hpp>

/*************************************************************************************************/

ADOBE_ONCE_DECLARATION(adobe_eve_evaluate)

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

aggregate_name_t key_spacing             = { "spacing" };
aggregate_name_t key_indent              = { "indent" };
aggregate_name_t key_margin              = { "margin" };

aggregate_name_t key_placement           = { "placement" };

aggregate_name_t key_horizontal          = { "horizontal" };
aggregate_name_t key_vertical            = { "vertical" };

aggregate_name_t key_child_horizontal    = { "child_horizontal" };
aggregate_name_t key_child_vertical      = { "child_vertical" };

aggregate_name_t key_align_left          = { "align_left" };
aggregate_name_t key_align_right         = { "align_right" };
aggregate_name_t key_align_top           = { "align_top" };
aggregate_name_t key_align_bottom        = { "align_bottom" };
aggregate_name_t key_align_center        = { "align_center" };
aggregate_name_t key_align_proportional  = { "align_proportional" };
aggregate_name_t key_align_fill          = { "align_fill" };

aggregate_name_t key_place_row           = { "place_row" };
aggregate_name_t key_place_column        = { "place_column" };
aggregate_name_t key_place_overlay       = { "place_overlay" };


aggregate_name_t key_guide_mask          = { "guide_mask" };
aggregate_name_t key_guide_balance       = { "guide_balance" };
    
aggregate_name_t key_guide_baseline      = { "guide_baseline" };
aggregate_name_t key_guide_label         = { "guide_label" };

} // namespace adobe

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

typedef std::pair<adobe::name_t*, adobe::name_t*>                                       reflected_table_range_t;
typedef adobe::static_table<adobe::name_t, adobe::layout_attributes_t::alignment_t, 7>  alignment_table_t;
typedef adobe::static_table<adobe::name_t, adobe::layout_attributes_placement_t::placement_t, 3>    placement_table_t;

/*************************************************************************************************/

// blank(guide_attach: {left: @guide_baseline});

alignment_table_t*          alignment_table_g;
placement_table_t*          placement_table_g;
reflected_table_range_t*    reflected_range_g;

/*************************************************************************************************/

void init_once()
{
    static alignment_table_t alignment_table_s =
    {{
        alignment_table_t::entry_type(adobe::key_align_left,           adobe::layout_attributes_t::align_left),
        alignment_table_t::entry_type(adobe::key_align_right,          adobe::layout_attributes_t::align_right),
        alignment_table_t::entry_type(adobe::key_align_top,            adobe::layout_attributes_t::align_top),
        alignment_table_t::entry_type(adobe::key_align_bottom,         adobe::layout_attributes_t::align_bottom),
        alignment_table_t::entry_type(adobe::key_align_center,         adobe::layout_attributes_t::align_center),
        alignment_table_t::entry_type(adobe::key_align_proportional,   adobe::layout_attributes_t::align_proportional),
        alignment_table_t::entry_type(adobe::key_align_fill,           adobe::layout_attributes_t::align_fill)
    }};

    static placement_table_t placement_table_s =
    {{
        placement_table_t::entry_type(adobe::key_place_row,            adobe::layout_attributes_placement_t::place_row),
        placement_table_t::entry_type(adobe::key_place_column,         adobe::layout_attributes_placement_t::place_column),
        placement_table_t::entry_type(adobe::key_place_overlay,         adobe::layout_attributes_placement_t::place_overlay)
    }};

    alignment_table_s.sort();
    placement_table_s.sort();

    alignment_table_g = &alignment_table_s;
    placement_table_g = &placement_table_s;
    
    static adobe::name_t    reflected[] =
    {
        adobe::key_align_left,
        adobe::key_align_right,
        adobe::key_align_top,
        adobe::key_align_bottom,
        adobe::key_align_center,
        adobe::key_align_proportional,
        adobe::key_align_fill,
        
        adobe::key_place_row,
        adobe::key_place_column,
        adobe::key_place_overlay
    };
    static reflected_table_range_t reflected_table_range_s;

    adobe::sort(reflected);

    reflected_table_range_s.first  = boost::begin(reflected);
    reflected_table_range_s.second = boost::end(reflected);

    reflected_range_g = &reflected_table_range_s;
}

/*************************************************************************************************/

adobe::any_regular_t reflected_variables(const adobe::basic_sheet_t& layout_sheet, adobe::name_t name)
{
    adobe::name_t* found(std::lower_bound(reflected_range_g->first, reflected_range_g->second, name));

    if (found != reflected_range_g->second && *found == name) return adobe::any_regular_t(name);
    
    return layout_sheet[name];
}

/*************************************************************************************************/

adobe::dictionary_t evaluate_named_arguments(adobe::virtual_machine_t& evaluator,
        const adobe::array_t& arguments)
{
    evaluator.evaluate(arguments);
    
    adobe::dictionary_t result(::adobe::move(evaluator.back().cast<adobe::dictionary_t>()));
    
    evaluator.pop_back();
    return result;
}

/*************************************************************************************************/

adobe::any_regular_t evaluate_initializer(adobe::virtual_machine_t& evaluator,
        const adobe::array_t& expression)
{
    evaluator.evaluate(expression);
    
    adobe::any_regular_t result(evaluator.back());
    
    evaluator.pop_back();
    return result;
}

/*************************************************************************************************/

void add_cell(  adobe::basic_sheet_t&                       sheet,
                adobe::eve_callback_suite_t::cell_type_t    type,
                adobe::name_t                               name,
                const adobe::any_regular_t&                 value)
{
    switch(type)
    {
        case adobe::eve_callback_suite_t::constant_k:
            sheet.add_constant(name, value);
            break;
        case adobe::eve_callback_suite_t::interface_k:
            sheet.add_interface(name, value);
            break;
        default:
            assert(false); // Type not supported
    }
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

ADOBE_ONCE_DEFINITION(adobe_eve_evaluate, init_once)

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

eve_callback_suite_t bind_layout(const bind_layout_proc_t& proc, basic_sheet_t& layout_sheet,
        virtual_machine_t& evaluator)
{
    ADOBE_ONCE_INSTANCE(adobe_eve_evaluate);
    
    eve_callback_suite_t suite;
    
    evaluator.set_variable_lookup(
            boost::bind(&reflected_variables, boost::cref(layout_sheet), _1));

    suite.add_view_proc_m   = boost::bind(proc, _1, _3, boost::bind(&evaluate_named_arguments,
            boost::ref(evaluator), _4));
    suite.add_cell_proc_m   = boost::bind(&add_cell, boost::ref(layout_sheet), _1, _2,
            boost::bind(&evaluate_initializer, boost::ref(evaluator), _4));

    return suite;
}

/*************************************************************************************************/

void apply_layout_parameters(   layout_attributes_t&     data,
                                const dictionary_t&      parameters)
{
    ADOBE_ONCE_INSTANCE(adobe_eve_evaluate);

    get_value(parameters, key_indent, data.indent_m);

    if (parameters.count(key_horizontal))
    {
        (*alignment_table_g)(   get_value(parameters, key_horizontal).cast<name_t>(),
                                data.slice_m[extents_slices_t::horizontal].alignment_m);
    }

    if (parameters.count(key_vertical))
    {
        (*alignment_table_g)(   get_value(parameters, key_vertical).cast<name_t>(),
                                data.slice_m[extents_slices_t::vertical].alignment_m);
    }
    
    // REVISIT (sparent) If we had named guides then named guide suppression would go here.
    
    if (parameters.count(key_guide_mask)) // an empty array would allow baselines out.
    {
        // Turn on all guides - then selectively suppress
        data.slice_m[extents_slices_t::vertical].suppress_m      = false;
        data.slice_m[extents_slices_t::horizontal].suppress_m    = false;
    
        array_t guide_mask(get_value(parameters, key_guide_mask).cast<array_t>());

        for (array_t::const_iterator iter(guide_mask.begin()), last(guide_mask.end());
                iter != last; ++iter)
        {
            if (iter->cast<name_t>() == key_guide_baseline)   data.slice_m[extents_slices_t::vertical].suppress_m = true;
            if (iter->cast<name_t>() == key_guide_label)      data.slice_m[extents_slices_t::horizontal].suppress_m = true;
        }
    }
    
    // REVISIT (sparent) : If we had named guides then named guide balancing would go here...
    
    /*
        Balanced guides must be supressed to avoid having them attach to outside guides which could
        overconstrain the system.
    */
    
    if (parameters.count(key_guide_balance))
    {
        data.slice_m[extents_slices_t::vertical].balance_m       = false;
        data.slice_m[extents_slices_t::horizontal].balance_m     = false;
    
        array_t guide_balance(get_value(parameters, key_guide_balance).cast<array_t>());

        for (array_t::const_iterator iter(guide_balance.begin()), last(guide_balance.end());
                iter != last; ++iter)
        {
            if (iter->cast<name_t>() == key_guide_baseline)
            {
                data.slice_m[extents_slices_t::vertical].balance_m = true;
                data.slice_m[extents_slices_t::vertical].suppress_m = true;
            }
            if (iter->cast<name_t>() == key_guide_label)
            {
                data.slice_m[extents_slices_t::horizontal].balance_m = true;
                data.slice_m[extents_slices_t::horizontal].suppress_m = true;
            }
        }
    }
    

    // REVISIT (sparent) : I'm seeing a pattern here - with three cases this could be factored

    {
    dictionary_t::const_iterator iter (parameters.find(key_placement));
    
    if (iter != parameters.end())
    {
        (*placement_table_g)(iter->second.cast<name_t>(), data.placement_m);
    }

    // Adjust defaults
    
    // Specifying a row from the parameters implies enabling baselines unless otherwise specified.
    
    if (iter != parameters.end() && data.placement_m == layout_attributes_placement_t::place_row && !parameters.count(key_guide_mask))
    {
        data.slice_m[extents_slices_t::vertical].suppress_m = false;
    }
    
    }
    
    {
    dictionary_t::const_iterator iter (parameters.find(key_child_horizontal));
    
    if (iter != parameters.end())
    {
        (*alignment_table_g)(   iter->second.cast<name_t>(),
                                data.slice_m[extents_slices_t::horizontal].child_alignment_m);
    }
    }

    {
    dictionary_t::const_iterator iter (parameters.find(key_child_vertical));
    
    if (iter != parameters.end())
    {
        (*alignment_table_g)(   iter->second.cast<name_t>(),
                                data.slice_m[extents_slices_t::vertical].child_alignment_m);
    }
    }
    
    // spacing
    {
    dictionary_t::const_iterator iter (parameters.find(key_spacing));
    if (iter != parameters.end())
    {
        if (iter->second.type_info() == type_info<array_t>())
        {
            const array_t& spacing_array = iter->second.cast<array_t>();
            data.spacing_m.resize(spacing_array.size() + 1);
            
            layout_attributes_t::spacing_t::iterator dest_iter(data.spacing_m.begin() + 1);
            
            for (array_t::const_iterator iter(spacing_array.begin()); iter != spacing_array.end();
                    ++iter)
            {
                *dest_iter = iter->cast<long>();
                ++dest_iter;
            }
        }
        else
        {
            double tmp(data.spacing_m[1]);
            iter->second.cast(tmp); // Try getting as number
            data.spacing_m[1] = long(tmp);
        }
    }
    }
    
    // margin
    {
    dictionary_t::const_iterator iter(parameters.find(key_margin));
    if (iter != parameters.end())
    {
        if (iter->second.type_info() == type_info<array_t>())
        {
            const array_t& margin_array = iter->second.cast<array_t>();

            data.vertical().margin_m.first    = margin_array[0].cast<long>();
            data.horizontal().margin_m.first  = margin_array[1].cast<long>();
            data.vertical().margin_m.second   = margin_array[2].cast<long>();
            data.horizontal().margin_m.second = margin_array[3].cast<long>();
        }
        else
        {
            long margin = iter->second.cast<long>();

            data.vertical().margin_m.first    = margin;
            data.horizontal().margin_m.first  = margin;
            data.vertical().margin_m.second   = margin;
            data.horizontal().margin_m.second = margin;
        }
    }
    }
}


/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
