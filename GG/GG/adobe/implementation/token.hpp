/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_TOKEN_HPP
#define ADOBE_TOKEN_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/name_fwd.hpp>

/*************************************************************************************************/

/*
REVISIT (sparent) : This code is an implementation detail and should be moved into an implementation
namespace but at this moment that would require too many changes.
*/

namespace adobe {

/*************************************************************************************************/

extern aggregate_name_t ifelse_k;

extern aggregate_name_t number_k;
extern aggregate_name_t identifier_k;
extern aggregate_name_t string_k;
extern aggregate_name_t lead_comment_k;
extern aggregate_name_t trail_comment_k;

extern aggregate_name_t semicolon_k;
extern aggregate_name_t comma_k;
extern aggregate_name_t assign_k;
extern aggregate_name_t question_k;
extern aggregate_name_t colon_k;
extern aggregate_name_t open_brace_k;
extern aggregate_name_t close_brace_k;
extern aggregate_name_t open_parenthesis_k;
extern aggregate_name_t close_parenthesis_k;
extern aggregate_name_t dot_k;
extern aggregate_name_t open_bracket_k;
extern aggregate_name_t close_bracket_k;
extern aggregate_name_t at_k;
extern aggregate_name_t is_k;

extern aggregate_name_t add_k;
extern aggregate_name_t subtract_k;
extern aggregate_name_t multiply_k;
extern aggregate_name_t divide_k;
extern aggregate_name_t modulus_k;

extern aggregate_name_t not_k;
extern aggregate_name_t unary_negate_k;

extern aggregate_name_t less_k;
extern aggregate_name_t greater_k;

extern aggregate_name_t and_k;
extern aggregate_name_t or_k;
extern aggregate_name_t less_equal_k;
extern aggregate_name_t greater_equal_k;
extern aggregate_name_t not_equal_k;
extern aggregate_name_t equal_k;

extern aggregate_name_t keyword_k;
extern aggregate_name_t empty_k;
extern aggregate_name_t true_k;
extern aggregate_name_t false_k;

extern aggregate_name_t function_k;
extern aggregate_name_t variable_k;
extern aggregate_name_t bracket_index_k;
extern aggregate_name_t dot_index_k;
extern aggregate_name_t array_k;
extern aggregate_name_t dictionary_k;

extern aggregate_name_t parenthesized_expression_k;
extern aggregate_name_t name_k;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
