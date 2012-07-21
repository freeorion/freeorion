/*
    Copyright 2005-2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ADAM_PARSER_IMPL_HPP
#define ADOBE_ADAM_PARSER_IMPL_HPP

#include <GG/adobe/config.hpp>

#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/implementation/expression_parser.hpp>
#include <GG/adobe/adam.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*
translation_unit                = { sheet_specifier }.
sheet_specifier                 = [lead_comment] "sheet" identifier "{" { qualified_cell_decl } "}"
                                                        [trail_comment].

qualified_cell_decl     = interface_set_decl | input_set_decl | output_set_decl
                            | constant_set_decl | logic_set_decl | invariant_set_decl.

interface_set_decl      = "interface"   ":" { [lead_comment] interface_cell_decl }.
input_set_decl          = "input"       ":" { [lead_comment] input_cell_decl }.
output_set_decl         = "output"      ":" { [lead_comment] output_cell_decl }.
constant_set_decl       = "constant"    ":" { [lead_comment] constant_cell_decl }.
logic_set_decl          = "logic"       ":" { [lead_comment] logic_cell_decl }.
invariant_set_decl      = "invariant"   ":" { [lead_comment] invariant_cell_decl }.

interface_cell_decl     = ["unlink"] identifier [initializer] [define_expression] end_statement.
input_cell_decl         = identifier [initializer] end_statement.

output_cell_decl        = named_decl.
constant_cell_decl      = identifier initializer end_statement.
logic_cell_decl         = named_decl | relate_decl.
invariant_cell_decl     = named_decl.

relate_decl                             = [conditional] "relate" "{" relate_expression relate_expression
                                                        { relate_expression } "}" [trail_comment].
relate_expression               = [lead_comment] named_decl.
named_decl                              = identifier define_expression end_statement.

initializer             = ":" expression.
conditional             = "when" "(" expression ")".

define_expression       = "<==" expression.

end_statement           = ";" [trail_comment].

keywords                        += "sheet" | "interface" | "input" | "output" | "constant" | "logic"
                                         | "invariant" | "unlink" | "when" | "relate"
*/

/*************************************************************************************************/

class adam_parser : protected expression_parser
{
 public:
    adam_parser(std::istream& in, const line_position_t& position, const adam_callback_suite_t& callbacks);

    adam_parser(std::istream& in, const line_position_t& position);

    using expression_parser::require_expression; // Export is_expression for client
    
//  translation_unit = { sheet_specifier }.
    void parse();

//  sheet_specifier = [lead_comment] "sheet" identifier "{" { qualified_cell_decl } "}" [trail_comment].
    bool is_sheet_specifier(name_t& name);

    
    adam_callback_suite_t                       adam_callback_suite_m;

private:
    typedef adam_callback_suite_t::relation_t   relation_t;
    typedef std::vector<relation_t>             relation_set_t;

    
//  qualified_cell_decl     = interface_set_decl | input_set_decl | output_set_decl
//                              | constant_set_decl | logic_set_decl | invariant_set_decl.
    bool is_qualified_cell_decl();

//  interface_set_decl      = "interface"   ":" { [lead_comment] interface_cell_decl }.
    bool is_interface_set_decl();
//  input_set_decl          = "input"       ":" { [lead_comment] input_cell_decl }.
    bool is_input_set_decl();
//  output_set_decl         = "output"      ":" { [lead_comment] output_cell_decl }.
    bool is_output_set_decl();
//  constant_set_decl       = "constant"    ":" { [lead_comment] constant_cell_decl }.
    bool is_constant_set_decl();
//  logic_set_decl          = "logic"       ":" { [lead_comment] logic_cell_decl }.
    bool is_logic_set_decl();
//  invariant_set_decl      = "invariant"   ":" { [lead_comment] invariant_cell_decl }.
    bool is_invariant_set_decl();
    
//  interface_cell_decl     = ["unlink"] identifier [initializer] [define_expression] end_statement.
    bool is_interface_cell_decl(const std::string& detailed);
//  input_cell_decl         = identifier [initializer] end_statement.
    bool is_input_cell_decl(const std::string& detailed);
//  output_cell_decl        = named_decl.
    bool is_output_cell_decl(const std::string& detailed);
//  constant_cell_decl      = identifier initializer end_statement.
    bool is_constant_cell_decl(const std::string& detailed);
//  logic_cell_decl         = named_decl | relate_decl.
    bool is_logic_cell_decl(const std::string& detailed);
//  invariant_cell_decl     = named_decl.
    bool is_invariant_cell_decl(const std::string& detailed);

//  relate_decl             = [conditional] "relate" "{" relate_expression relate_expression { relate_expression } "}" [trail_comment].
    bool is_relate_decl(line_position_t& position, array_t& expression, relation_set_t&, std::string&);
//  relate_expression       = [lead_comment] named_decl.
    bool is_relate_expression_decl(relation_t&);
//  named_decl              = identifier define_expression end_statement.
    bool is_named_decl(name_t& cell_name, line_position_t& position, array_t& expression, std::string&);

//  initializer             = ":" expression.
    bool is_initializer(line_position_t&, array_t& initializer);
//  conditional             = "when" "(" expression ")".
    bool is_conditional(line_position_t& position, array_t& expression);
//  define_expression       = "<==" expression.
    bool is_define_expression(line_position_t&, array_t&);
 
//  end_statement           = ";" [trail_comment].
    void require_end_statement(std::string& brief);
    
    typedef void (sheet_t::*sheet_add_t)(name_t, const relation_t&);
    typedef bool (adam_parser::*set_decl_t)(const std::string& detailed);

    bool is_logic_or_invariant_cell_decl(sheet_add_t);
    bool is_set_decl(name_t, set_decl_t);
};

/*************************************************************************************************/

bool adam_keyword_lookup(const name_t& name);

/*************************************************************************************************/

}

/*************************************************************************************************/

#endif
