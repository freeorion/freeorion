/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_EXPRESSION_FORMATTER_HPP
#define ADOBE_EXPRESSION_FORMATTER_HPP

/******************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/function.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/token.hpp>
#include <GG/adobe/iomanip.hpp>
#include <GG/adobe/static_table.hpp>
#include <GG/adobe/string.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/

class spaces : public basic_omanipulator<std::size_t, char, std::char_traits<char> >
{
    typedef basic_omanipulator<std::size_t, char, std::char_traits<char> >      inherited_t;

public:
    typedef inherited_t::stream_type    stream_type;
    typedef inherited_t::argument_type  argument_type;

    spaces(argument_type num) :
        inherited_t(spaces::fct, num)
        { }

    inherited_t& operator() (argument_type i)
        { arg_m = i; return *this; }

private:
    static stream_type& fct(stream_type& os, const argument_type& i)
    {
        for (argument_type count(0); count < i; ++count)
            os.put(' ');

        return os;
    }
};

/******************************************************************************/

namespace implementation {

/******************************************************************************/

struct expression_formatter_t
{
    expression_formatter_t();

    string_t format(const array_t& expression,
                    std::size_t    indent,
                    bool           tight);

private:
    typedef boost::function<void ()>             operator_t;
    typedef static_table<name_t, operator_t, 21> operator_table_t;

    void unary_operation(const char* operation);
    void binary_operation(const char* operation);

    void op_ifelse();
    void op_index();
    void op_function();
    void op_array();
    void op_dictionary();
    void op_variable();

    string_t strip_expression(const string_t& expr_str);
    string_t add_indentation(const string_t& expr_str, std::size_t indent = 4);

    void assert_stack_ok() const;

    vector<string_t> stack_m;
    operator_table_t operator_table_m;
    std::size_t      indent_m;
    bool             tight_m;
};

/******************************************************************************/

} // namespace implementation

/******************************************************************************/
/*!
    @brief "Unparses" a parsed token stream to a CEL-syntax expression

    @param expression The parsed expression as a token vector
    @param indent The number of spaces to indent the output by
    @param tight When true does not add a newline after opening or before closing an array or dictionary expression

    @return A CEL-syntax formatted expression representing the original parsed string
*/
string_t format_expression(const array_t& expression,
                           std::size_t    indent = 0,
                           bool           tight = false);

/******************************************************************************/

} // namespace adobe

/******************************************************************************/
// ADOBE_EXPRESSION_FORMATTER_HPP
#endif

/******************************************************************************/
