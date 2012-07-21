/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/adam_evaluate.hpp>

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

void add_cell(  adobe::sheet_t&                             sheet,
                adobe::adam_callback_suite_t::cell_type_t   type,
                adobe::name_t                               name,
                const adobe::line_position_t&               position,
                const adobe::array_t&                       init_or_expr)
{
    switch(type)
    {
        case adobe::adam_callback_suite_t::input_k:
            sheet.add_input(name, position, init_or_expr);
            break;

        case adobe::adam_callback_suite_t::output_k:
            sheet.add_output(name, position, init_or_expr);
            break;

        case adobe::adam_callback_suite_t::constant_k:
            sheet.add_constant(name, position, init_or_expr);
            break;

        case adobe::adam_callback_suite_t::logic_k:
            sheet.add_logic(name, position, init_or_expr);
            break;

        case adobe::adam_callback_suite_t::invariant_k:
            sheet.add_invariant(name, position, init_or_expr);
            break;

        default:
            assert(false); // Type not supported
    }
}

/*************************************************************************************************/

void add_relation(  adobe::sheet_t&                                 sheet,
                    const adobe::line_position_t&                   position,
                    const adobe::array_t&                           conditional,
                    const adobe::adam_callback_suite_t::relation_t* first,
                    const adobe::adam_callback_suite_t::relation_t* last)
{
    typedef std::vector<adobe::sheet_t::relation_t> relation_buffer_t;
    
    relation_buffer_t   relations;
    
    relations.reserve(relation_buffer_t::size_type(last - first));
        
    /*
        REVISIT (sparent) : It would be nice to find a simple way to handle a transformed copy
        in a generic fashion when multiple members are needed.
    */

    while (first != last) // copy
    {
        relations.push_back(adobe::sheet_t::relation_t(first->name_m, first->position_m, first->expression_m));
        ++first;
    }

    sheet.add_relation(position, conditional, &relations[0], &relations[0] + relations.size());
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

adam_callback_suite_t bind_to_sheet(sheet_t& sheet)
{
    adam_callback_suite_t suite;

    suite.add_cell_proc_m       = boost::bind(&add_cell, boost::ref(sheet), _1, _2, _3, _4);
    suite.add_relation_proc_m   = boost::bind(&add_relation, boost::ref(sheet), _1, _2, _3, _4);
    suite.add_interface_proc_m  = boost::bind(&adobe::sheet_t::add_interface, boost::ref(sheet), _1, _2, _3, _4, _5, _6);

    return suite;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
