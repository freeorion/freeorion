/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

/*
REVISIT (sparent) : [Old comment - what does this mean?] I need to handle the pos_type correctly
with regards to state.
*/

/*************************************************************************************************/

#include <utility>
#include <istream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/config.hpp>

#include <GG/adobe/array.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/dictionary.hpp>

#include <GG/adobe/implementation/token.hpp>

#include <GG/adobe/string.hpp>

#include <GG/adobe/implementation/lex_stream.hpp>
#include <GG/adobe/implementation/parser_shared.hpp>

#include <GG/adobe/implementation/expression_parser.hpp>

#ifdef BOOST_MSVC
namespace std {
    using ::isspace;
    using ::isalnum;
    using ::isalpha;
    using ::isdigit;
}
#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

class expression_parser::implementation
{
 public:
    implementation(std::istream& in, const line_position_t& position) :
        token_stream_m(in, position)
        { }

    void set_keyword_extension_lookup(const keyword_extension_lookup_proc_t& proc)
        { token_stream_m.set_keyword_extension_lookup(proc); }

    lex_stream_t token_stream_m;
};

expression_parser::expression_parser(std::istream& in, const line_position_t& position) :
        object(new implementation(in, position))
        { }

expression_parser::~expression_parser()
    { delete object; }

/*************************************************************************************************/

/* REVISIT (sparent) : Should this be const? And is there a way to specify the class to throw? */

void expression_parser::throw_exception(const char* errorString)
    { throw_parser_exception(errorString, next_position()); }

/*************************************************************************************************/

/* REVISIT (sparent) : Should this be const? And is there a way to specify the class to throw? */

void expression_parser::throw_exception(const name_t& found, const name_t& expected)
    { throw_parser_exception(found, expected, next_position()); }

/*************************************************************************************************/

const line_position_t& expression_parser::next_position()
{
    return object->token_stream_m.next_position();
}

/*************************************************************************************************/

void expression_parser::set_keyword_extension_lookup(const keyword_extension_lookup_proc_t& proc)
{
    object->set_keyword_extension_lookup(proc);
}

/*************************************************************************************************/

//  expression = or_expression ["?" expression ":" expression].
bool expression_parser::is_expression(array_t& expression_stack)
{
    if (!is_or_expression(expression_stack)) return false;
    
    if (!is_token(question_k)) return true;
    
    array_t operand2;
    array_t operand3;
    
    require_expression(operand2);
    require_token(colon_k);
    require_expression(operand3);
    
    push_back(expression_stack, operand2);
    push_back(expression_stack, operand3);
    expression_stack.push_back(any_regular_t(ifelse_k));


    return true;
}

/*************************************************************************************************/

void expression_parser::require_expression(array_t& expression_stack)
{
    if (!is_expression(expression_stack))
    {
         throw_exception("Expression required.");
    }
}



/*************************************************************************************************/

//  or_expression = and_expression { "||" and_expression }.
bool expression_parser::is_or_expression(array_t& expression_stack)
    {
    if (!is_and_expression(expression_stack)) return false;
    
    while (is_token(or_k))
        {
        array_t operand2;
        if (!is_and_expression(operand2)) throw_exception("and_expression required");
        push_back(expression_stack, operand2);
        expression_stack.push_back(any_regular_t(or_k));

        }
    
    return true;
    }

/*************************************************************************************************/

//  and_expression = equality_expression { "&&" equality_expression }.
bool expression_parser::is_and_expression(array_t& expression_stack)
    {
    if (!is_equality_expression(expression_stack)) return false;
    
    while (is_token(and_k))
        {
        array_t operand2;
        if (!is_equality_expression(operand2)) throw_exception("equality_expression required");
        push_back(expression_stack, operand2);
        expression_stack.push_back(any_regular_t(and_k));

        }
    
    return true;
    }

/*************************************************************************************************/

//  equality_expression = relational_expression { ("==" | "!=") relational_expression }.
bool expression_parser::is_equality_expression(array_t& expression_stack)
    {
    if (!is_relational_expression(expression_stack)) return false;
    
    bool is_equal = false;
    while ((is_equal = is_token(equal_k)) || is_token(not_equal_k))
        {
        if (!is_relational_expression(expression_stack)) throw_exception("Primary required.");
        expression_stack.push_back(is_equal ? any_regular_t(equal_k) : any_regular_t(not_equal_k));

        }
    
    return true;
    }

/*************************************************************************************************/

//  relational_expression = additive_expression { relational_operator additive_expression }.
bool expression_parser::is_relational_expression(array_t& expression_stack)
    {
    if (!is_additive_expression(expression_stack)) return false;
    
    name_t operator_l;
    
    while (is_relational_operator(operator_l))
        {
        if (!is_additive_expression(expression_stack)) throw_exception("Primary required.");
        expression_stack.push_back(any_regular_t(operator_l));

        }
    
    return true;
    }

/*************************************************************************************************/
    
//  additive_expression = multiplicative_expression { additive_operator multiplicative_expression }.
bool expression_parser::is_additive_expression(array_t& expression_stack)
    {
    if (!is_multiplicative_expression(expression_stack)) return false;
    
    name_t operator_l;
    
    while (is_additive_operator(operator_l))
        {
        if (!is_multiplicative_expression(expression_stack)) throw_exception("Primary required.");
        expression_stack.push_back(any_regular_t(operator_l));

        }
    
    return true;
    }

/*************************************************************************************************/
    
//  multiplicative_expression = unary_expression { ("*" | "/" | "%") unary_expression }.
bool expression_parser::is_multiplicative_expression(array_t& expression_stack)
    {
    if (!is_unary_expression(expression_stack)) return false;
    
    name_t operator_l;
    
    while (is_multiplicative_operator(operator_l))
        {
        if (!is_unary_expression(expression_stack)) throw_exception("Primary required.");
        expression_stack.push_back(any_regular_t(operator_l));

        }
    
    return true;
    }

/*************************************************************************************************/

//  unary_expression = postfix_expression | (unary_operator unary_expression).

bool expression_parser::is_unary_expression(array_t& expression_stack)
    {
    if (is_postfix_expression(expression_stack)) return true;
    
    name_t operator_l;
    
    if (is_unary_operator(operator_l))
        {
        if (!is_unary_expression(expression_stack)) throw_exception("Unary expression required.");
        if (operator_l != add_k)
            {
            expression_stack.push_back(any_regular_t(operator_l));

            }
        return true;
        }
        
    return false;
    }
    
/*************************************************************************************************/

//  postfix_expression = primary_expression { ("[" expression "]") | ("." identifier) }.

bool expression_parser::is_postfix_expression(array_t& expression_stack)
    {
    if (!is_primary_expression(expression_stack)) return false;
    
    while (true)
        {
        if (is_token(open_bracket_k))
            {
            require_expression(expression_stack);
            require_token(close_bracket_k);
            expression_stack.push_back(any_regular_t(bracket_index_k));
            }

        else if (is_token(dot_k))
            {
            any_regular_t result;
            require_token(identifier_k, result);
            expression_stack.push_back(result);
            expression_stack.push_back(any_regular_t(dot_index_k));
            }

        else break;
        }
    
    return true;
    }
    
/*************************************************************************************************/

//  argument_expression_list = named_argument_list | argument_list.
bool expression_parser::is_argument_expression_list(array_t& expression_stack)
{
    if (is_named_argument_list(expression_stack) || is_argument_list(expression_stack)) return true;
    
    return false;
}
    
/*************************************************************************************************/
    
//  argument_list = expression { "," expression }.
bool expression_parser::is_argument_list(array_t& expression_stack)
{
    if (!is_expression(expression_stack)) return false;
    
    std::size_t count = 1;
    
    while (is_token(comma_k))
    {
        require_expression(expression_stack);
        ++count;
    }
    
    expression_stack.push_back(any_regular_t(double(count)));
    expression_stack.push_back(any_regular_t(array_k));
    
    return true;
}
    
/*************************************************************************************************/
    
//  named_argument_list = named_argument { "," named_argument }.
bool expression_parser::is_named_argument_list(array_t& expression_stack)
{
    if (!is_named_argument(expression_stack)) return false;
    
    std::size_t count = 1;
    
    while (is_token(comma_k))
    {
        if (!is_named_argument(expression_stack)) throw_exception("Named argument required.");
        ++count;
    }
    
    expression_stack.push_back(any_regular_t(double(count)));
    expression_stack.push_back(any_regular_t(dictionary_k));
    
    return true;
}
    
/*************************************************************************************************/
    
//  named_argument = identifier ":" expression.
bool expression_parser::is_named_argument(array_t& expression_stack)
    {
    /* NOTE (sparent) : This is the one point in the grammar where we need the LL(2) parser. */
    
    name_t ident;
    
    if (!is_identifier(ident)) return false;
    
    if (!is_token(colon_k))
    {
        putback(); // the identifier
        return false;
    }

    expression_stack.push_back(any_regular_t(ident));
    require_expression(expression_stack);
    
    return true;
    }
    
/*************************************************************************************************/

//  primary_expression = name | number | boolean | string | "empty" | array | dictionary
//      | variable_or_fuction | ( "(" expression ")" ).

bool expression_parser::is_primary_expression(array_t& expression_stack)
    {
    any_regular_t result; // empty result used if is_keyword(empty_k)
    
    if (is_name(result))
        {
        expression_stack.push_back(::adobe::move(result));
        expression_stack.push_back(any_regular_t(name_k));
        return true;
        }
    else if (is_token(number_k, result)
             || is_boolean(result)
             || is_token(string_k, result)
             || is_keyword(empty_k))
        {
        expression_stack.push_back(::adobe::move(result));
        return true;
        }
    else if (is_array(expression_stack)) return true;
    else if (is_dictionary(expression_stack)) return true;
    else if (is_variable_or_function(expression_stack)) return true;
    else if (is_token(open_parenthesis_k))
        {
        require_expression(expression_stack);
        require_token(close_parenthesis_k);
        expression_stack.push_back(any_regular_t(parenthesized_expression_k));
        return true;
        }
    
    return false;
    }

/*************************************************************************************************/

//  variable_or_function = identifier ["(" [argument_expression_list] ")"].
bool expression_parser::is_variable_or_function(array_t& expression_stack)
    {
    any_regular_t result;
    
    if (!is_token(identifier_k, result)) return false;
    
    if (is_token(open_parenthesis_k))
        {
        // If there are no parameters then set the parameters to an empty array.
        if (!is_argument_expression_list(expression_stack))
            { expression_stack.push_back(any_regular_t(adobe::array_t())); }
        require_token(close_parenthesis_k);
        expression_stack.push_back(result);
        expression_stack.push_back(any_regular_t(function_k));
        }
    else
        {
        expression_stack.push_back(result);
        expression_stack.push_back(any_regular_t(variable_k));
        }
        

    
    return true;
    }

/*************************************************************************************************/

bool expression_parser::is_dictionary(array_t& expression_stack)
{
    if (!is_token(open_brace_k)) return false;
    if (!is_named_argument_list(expression_stack))
    {
        push_back(expression_stack, adobe::dictionary_t());
    }
    require_token(close_brace_k);
    return true;
}

/*************************************************************************************************/

bool expression_parser::is_array(array_t& expression_stack)
{
    if (!is_token(open_bracket_k)) return false;
    if (!is_argument_list(expression_stack))
    {
        push_back(expression_stack, adobe::array_t());
    }
    require_token(close_bracket_k);
    return true;
}

/*************************************************************************************************/

bool expression_parser::is_name(any_regular_t& result)
{
    if (!is_token(at_k)) return false;
    
    if (!is_token(keyword_k, result) && !is_token(identifier_k, result)) {
        throw_exception("identifier or keyword required.");
    }

    return true;
}

/*************************************************************************************************/

bool expression_parser::is_boolean(any_regular_t& result)
    {
    if (is_keyword(true_k))
        { result = any_regular_t(true); return true; }
    else if (is_keyword(false_k))
        { result = any_regular_t(false); return true; }
    
    return false;
    }

/*************************************************************************************************/

//  relational_operator = "<" | ">" | "<=" | ">=".
bool expression_parser::is_relational_operator(name_t& name_result)
    {
    const stream_lex_token_t& result (get_token());
    
    name_t name = result.first;
    if (name == less_k || name == greater_k || name == less_equal_k || name == greater_equal_k)
        {
        name_result = name;
        return true;
        }
    putback();
    return false;
    }

/*************************************************************************************************/

//  additive_operator = "+" | "-".
bool expression_parser::is_additive_operator(name_t& name_result)
    {
    const stream_lex_token_t& result (get_token());
    
    name_t name = result.first;
    if (name == add_k || name == subtract_k)
        {
        name_result = name;
        return true;
        }
    putback();
    return false;
    }

/*************************************************************************************************/

//  multiplicative_operator = "*" | "/" | "%".
bool expression_parser::is_multiplicative_operator(name_t& name_result)
    {
    const stream_lex_token_t& result (get_token());
    
    name_t name = result.first;
    if (name == multiply_k || name == divide_k || name == modulus_k)
        {
        name_result = name;
        return true;
        }
    putback();
    return false;
    }
    
/*************************************************************************************************/
    
//  unary_operator = "+" | "-" | "!".
bool expression_parser::is_unary_operator(name_t& name_result)
    {
    const stream_lex_token_t& result (get_token());
    
    name_t name = result.first;
    if (name == subtract_k)
        {
        name_result = unary_negate_k;
        return true;
        }
    else if (name == not_k || name == add_k)
        {
        name_result = name;
        return true;
        }
    putback();
    return false;
    }
    
/*************************************************************************************************/

/*
    REVISIT (sparent) : There should be a turn the simple lexical calls into templates
*/

bool expression_parser::is_identifier(name_t& name_result)
{
    const stream_lex_token_t& result (get_token());
    
    if (result.first == identifier_k)
    {
        name_result = result.second.cast<adobe::name_t>();
        return true;
    }
    
    putback();
    return false;
}

bool expression_parser::is_lead_comment(std::string& string_result)
{
    const stream_lex_token_t& result (get_token());
    
    if (result.first == lead_comment_k)
    {
        string_result = result.second.cast<std::string>();
        return true;
    }
    
    putback();
    return false;
}

bool expression_parser::is_trail_comment(std::string& string_result)
{
    const stream_lex_token_t& result (get_token());
    
    if (result.first == trail_comment_k)
    {
        string_result = result.second.cast<std::string>();
        return true;
    }
    
    putback();
    return false;
}
    
/*************************************************************************************************/

bool expression_parser::is_token(name_t tokenName, any_regular_t& tokenValue)
    {
    const stream_lex_token_t& result (get_token());
    if (result.first == tokenName)
        {
        tokenValue = result.second;
        return true;
        }
    putback();
    return false;
    }

/*************************************************************************************************/

bool expression_parser::is_token(name_t tokenName)
    {
    const stream_lex_token_t& result (get_token());
    if (result.first == tokenName)
        {
        return true;
        }
    putback();
    return false;
    }

/*************************************************************************************************/

bool expression_parser::is_keyword(name_t keyword_name)
{
    const stream_lex_token_t& result (get_token());
    if (result.first == keyword_k && result.second.cast<name_t>() == keyword_name) return true;
    putback();
    return false;
}

/*************************************************************************************************/

const stream_lex_token_t& expression_parser::get_token()
{
    return object->token_stream_m.get();
}

/*************************************************************************************************/

void expression_parser::putback()
{
    object->token_stream_m.putback();
}

/*************************************************************************************************/

void expression_parser::require_token(name_t tokenName, any_regular_t& tokenValue)
    {
    const stream_lex_token_t& result (get_token());
    if (result.first != tokenName)
        {
        throw_exception(tokenName, result.first);
        }

    tokenValue = result.second;
    }

/*************************************************************************************************/

void expression_parser::require_keyword(name_t keyword_name)
{
    const stream_lex_token_t& result (get_token());
    if (result.first == keyword_k && result.second.cast<name_t>() == keyword_name) return;
    
    throw_exception(keyword_name, result.second.cast<name_t>());
}

/*************************************************************************************************/

void expression_parser::require_token(name_t tokenName)
{
    const stream_lex_token_t& result (get_token());
    if (result.first == tokenName) return;

    throw_exception(tokenName, result.first);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
