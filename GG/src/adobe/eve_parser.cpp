/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/eve_parser.hpp>

#include <string>

#include <boost/array.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/once.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <GG/adobe/implementation/expression_parser.hpp>

/*************************************************************************************************/

namespace { void init_keyword_table(); }

ADOBE_ONCE_DECLARATION(adobe_eve_parser)
ADOBE_ONCE_DEFINITION(adobe_eve_parser, init_keyword_table)

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

typedef boost::array<adobe::name_t, 4> keyword_table_t;

/*************************************************************************************************/

/*
    WARNING (sparent) : Initialization of these const_once_name_t items is defered until
    eve_parser::eve_parser().
*/

keyword_table_t*            keyword_table_g;
adobe::aggregate_name_t     interface_k = { "interface" };
adobe::aggregate_name_t     constant_k  = { "constant" };
adobe::aggregate_name_t     layout_k    = { "layout" };
adobe::aggregate_name_t     view_k      = { "view" };

/*************************************************************************************************/

void init_keyword_table()
{
    static keyword_table_t keyword_table_s =
    {{
        interface_k,
        constant_k,
        layout_k,
        view_k
    }};

    adobe::sort(keyword_table_s);

    keyword_table_g = &keyword_table_s;
}

/*************************************************************************************************/

bool keyword_lookup(const adobe::name_t& name)
{
    keyword_table_t::const_iterator iter(adobe::lower_bound(*keyword_table_g, name));

    return (iter != keyword_table_g->end() && *iter == name);   
}

void once_instance()
{
    ADOBE_ONCE_INSTANCE(adobe_eve_parser);
}

/*************************************************************************************************/

class eve_parser : public adobe::expression_parser
{
public:
    typedef adobe::eve_callback_suite_t::position_t position_t;

    eve_parser(const adobe::eve_callback_suite_t& assembler, std::istream& in,
            const adobe::line_position_t& position) :
        expression_parser(in, position), assembler_m(assembler)
    {
        once_instance();
        set_keyword_extension_lookup(boost::bind(&keyword_lookup, _1));
    
        assert(assembler_m.add_view_proc_m);
    //  assert(assembler_m.add_cell_proc_m); Only required if you have a sheet state.
    }

    void parse(const position_t&);

private:

    bool is_layout_specifier(const position_t&);
    bool is_qualified_cell_decl();
    bool is_interface_set_decl();
    bool is_constant_set_decl();
    bool is_cell_decl(adobe::eve_callback_suite_t::cell_type_t);
    bool is_initializer(adobe::line_position_t& position, adobe::array_t& expression);
    void require_end_statement(std::string&);
    bool is_view_definition(const position_t&);
    bool is_view_statement_sequence(const position_t&);
    bool is_view_class_decl(adobe::name_t& class_name, adobe::array_t& arguments);
    void require_view_statement_list(const position_t&);

    adobe::eve_callback_suite_t     assembler_m;
};

/*************************************************************************************************/
    
void eve_parser::parse(const position_t& position)
{
    if (!is_layout_specifier(position)) throw_exception("layout specifier required");
}

/*************************************************************************************************/

bool eve_parser::is_layout_specifier(const position_t& position)
{
/* REVISIT (sparent) : Top level block is ignored. */
    
    using namespace adobe;
    
    is_token(lead_comment_k);
    
    if (!is_keyword(layout_k)) return false;
    
    require_token(identifier_k);
    require_token(open_brace_k);
    while (is_qualified_cell_decl()) { };
    
    require_keyword(view_k);
    if (!is_view_definition(position)) throw_exception("view definition required");
    require_token(close_brace_k);
    is_token(trail_comment_k);
    return true;
}

/*************************************************************************************************/

bool eve_parser::is_qualified_cell_decl()
{
    if (is_interface_set_decl() || is_constant_set_decl()) return true;
    return false;
}

/*************************************************************************************************/

// interface_set_decl       = "interface"   ":" { cell_decl }.
bool eve_parser::is_interface_set_decl()
{
    using namespace adobe;

    if (!is_keyword(interface_k)) return false;
    require_token(colon_k);
    
    while (is_cell_decl(eve_callback_suite_t::interface_k)) { }
    return true;
}

/*************************************************************************************************/

// constant_set_decl        = "constant"    ":" { cell_decl }.
bool eve_parser::is_constant_set_decl()
{
    using namespace adobe;

    if (!is_keyword(constant_k)) return false;
    require_token(colon_k);
    
    while (is_cell_decl(eve_callback_suite_t::constant_k)) { }
    return true;
}

/*************************************************************************************************/

// cell_decl                = [lead_comment] identifier initializer end_statement.
bool eve_parser::is_cell_decl(adobe::eve_callback_suite_t::cell_type_t type)
{
    using namespace adobe;

    std::string     detailed;
    std::string     brief;
    name_t          cell_name;
    line_position_t position;
    array_t         initializer;

    (void)is_lead_comment(detailed);
    if (!is_identifier(cell_name)) return false;
    if (!is_initializer(position, initializer)) throw_exception("initializer required");
    require_end_statement(brief);
    
    assembler_m.add_cell_proc_m(type, cell_name, position, initializer, brief, detailed);

    return true;
}

/*************************************************************************************************/

// initializer              = ":" expression.
bool eve_parser::is_initializer(adobe::line_position_t& position, adobe::array_t& expression)
{
    using namespace adobe;

    if (!is_token(colon_k)) return false;
    
    position = next_position();
    require_expression(expression);

    return true;
}

/*************************************************************************************************/

// end_statement            = ";" [trail_comment].
void eve_parser::require_end_statement(std::string& brief)
{
    using namespace adobe;

    require_token(semicolon_k);
    (void)is_trail_comment(brief);
}

/*************************************************************************************************/
    
bool eve_parser::is_view_definition(const position_t& location)
{
    using namespace adobe;

    std::string     detailed;
    std::string     brief;
    name_t          class_name;
    array_t         arguments;
    
    (void)is_lead_comment(detailed);
    
    // Report any errors in calling the client at the start of the class decl.
    line_position_t line_position = next_position();
    
    if (!is_view_class_decl(class_name, arguments)) return false;
    
    bool leaf (is_token(semicolon_k));
    
    (void)is_trail_comment(brief);
    
    position_t node_location;
    
    try
    {
        node_location = assembler_m.add_view_proc_m(location, line_position, class_name, arguments,
            brief, detailed);
    }
    catch (const std::exception& error)
    {
        throw adobe::stream_error_t(error, line_position);
    }
    
    if (!leaf)
    {
        require_view_statement_list(node_location);
    }
    
    return true;
}

/*************************************************************************************************/
    
bool eve_parser::is_view_statement_sequence(const position_t& location)
{
    while (is_view_definition(location)) { }
    return true;
}

/*************************************************************************************************/
    
bool eve_parser::is_view_class_decl(adobe::name_t& class_name, adobe::array_t& arguments)
{
    using namespace adobe;
    
    if (!is_identifier(class_name)) return false;
    
    require_token(open_parenthesis_k);
    if (!is_named_argument_list(arguments)) push_back(arguments, adobe::dictionary_t());
    require_token(close_parenthesis_k);
    
    return true;
}

/*************************************************************************************************/
    
void eve_parser::require_view_statement_list(const position_t& location)
{
    using namespace adobe;
    
    require_token(open_brace_k);
    is_view_statement_sequence(location);
    require_token(close_brace_k);
}

/*************************************************************************************************/

} // namespace

namespace adobe {

/*************************************************************************************************/

line_position_t parse(std::istream& in,
        const line_position_t& line_position,
        const eve_callback_suite_t::position_t& position,
        const eve_callback_suite_t& assembler)
{
    eve_parser parser(assembler, in, line_position);
    parser.parse(position);
    return parser.next_position();
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
