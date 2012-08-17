/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_EVE_PARSER_HPP
#define ADOBE_EVE_PARSER_HPP

#include <GG/adobe/config.hpp>

#include <string>

#include <boost/any.hpp>
#include <boost/function.hpp>

#include <GG/adobe/array_fwd.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/name_fwd.hpp>

/*************************************************************************************************/

/*
Eve Grammer:
----------------------------------------

layout_specifier        = [lead_comment] "layout" identifier "{" { qualified_cell_decl }
                            "view" view_definition "}" [trail_comment].

qualified_cell_decl     = interface_set_decl | constant_set_decl.

interface_set_decl      = "interface"   ":" { cell_decl }.
constant_set_decl       = "constant"    ":" { cell_decl }.

cell_decl               = [lead_comment] identifier initializer end_statement.
initializer             = ":" expression.
    
view_definition         = [lead_comment] view_class_decl ((";" [trail_comment])
                            | ([trail_comment] view_statement_list)).
view_statment_sequence  = { view_definition }.
view_class_decl         = ident "(" [ named_argument_list ] ")".
view_statment_list      = "{" view_statement_sequence "}".

end_statement           = ";" [trail_comment].
*/

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct eve_callback_suite_t
{
    enum cell_type_t
    {
        constant_k,
        interface_k
    };
    
    typedef boost::any position_t;

    typedef boost::function<position_t (    const position_t&       parent,
                                            const line_position_t&  parse_location,
                                            name_t                  name,
                                            const array_t&          parameters,
                                            const std::string&      brief,
                                            const std::string&      detailed)>  add_view_proc_t;
                                            
    typedef boost::function<void (  cell_type_t             type,
                                    name_t                  name,
                                    const line_position_t&  position,
                                    const array_t&          initializer,
                                    const std::string&      brief,
                                    const std::string&      detailed)>          add_cell_proc_t;
                                    
                                            
    add_view_proc_t add_view_proc_m;
    add_cell_proc_t add_cell_proc_m;
};

line_position_t parse(std::istream& in, const line_position_t&,
    const eve_callback_suite_t::position_t&, const eve_callback_suite_t&);

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif // ADOBE_EVE_PARSER_HPP

/*************************************************************************************************/
