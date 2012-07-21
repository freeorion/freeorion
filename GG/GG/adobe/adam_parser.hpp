/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ADAM_PARSER_HPP
#define ADOBE_ADAM_PARSER_HPP

#include <GG/adobe/config.hpp>

#include <iosfwd>
#include <string>

#include <boost/function.hpp>

#include <GG/adobe/array.hpp>
#include <GG/adobe/istream.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct adam_callback_suite_t
{
    enum cell_type_t
    {
        input_k,            // array_t is an initializer
        output_k,           // array_t is an expression
        constant_k,         // array_t is an initializer
        logic_k,            // array_t is an expression
        invariant_k         // array_t is an expression
    };

    struct relation_t
    {
        name_t          name_m;
        line_position_t position_m;
        array_t         expression_m;
        std::string     detailed_m;
        std::string     brief_m;

        relation_t& operator=(const relation_t& x) 
        { /*REVISIT MOVE*/ 
            name_m = x.name_m; 
            position_m = x.position_m; 
            expression_m = x.expression_m; 
            detailed_m = x.detailed_m;
            brief_m = x.brief_m;
            return *this;
        }
    };

    typedef boost::function<void (  cell_type_t             type,
                                    name_t                  cell_name,
                                    const line_position_t&  position,
                                    const array_t&          expr_or_init,
                                    const std::string&      brief,
                                    const std::string&      detailed)>      add_cell_proc_t;

    typedef boost::function<void (  const line_position_t&  position,
                                    const array_t&          conditional,
                                    const relation_t*       first,
                                    const relation_t*       last,
                                    const std::string&      brief,
                                    const std::string&      detailed)>      add_relation_proc_t; // REVISIT (sparent) where's brief?
    
    typedef boost::function<void (  name_t                  cell_name,
                                    bool                    linked,
                                    const line_position_t&  position1,
                                    const array_t&          initializer,
                                    const line_position_t&  position2,
                                    const array_t&          expression,
                                    const std::string&      brief,
                                    const std::string&      detailed)>      add_interface_proc_t;

    add_cell_proc_t         add_cell_proc_m;
    add_relation_proc_t     add_relation_proc_m;
    add_interface_proc_t    add_interface_proc_m;
};

/*************************************************************************************************/

void parse(std::istream& stream, const line_position_t& position, const adam_callback_suite_t& callbacks);

/*************************************************************************************************/

array_t parse_adam_expression(const std::string& expression);

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif // ADOBE_ADAM_PARSER_HPP

/*************************************************************************************************/
