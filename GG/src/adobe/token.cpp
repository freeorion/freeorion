/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/implementation/token.hpp>
#include <GG/adobe/name.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

aggregate_name_t ifelse_k               = { ".ifelse" };
aggregate_name_t number_k               = { "number" };

aggregate_name_t identifier_k           = { "identifier" };
aggregate_name_t string_k               = { "string" };
aggregate_name_t lead_comment_k         = { "lead_comment" };
aggregate_name_t trail_comment_k        = { "trail_comment" };

aggregate_name_t semicolon_k            = { "semicolon" };
aggregate_name_t comma_k                = { "comma" };
aggregate_name_t assign_k               = { "assign" };
aggregate_name_t question_k             = { "question" };
aggregate_name_t colon_k                = { "colon" };
aggregate_name_t open_brace_k           = { "open_brace" };
aggregate_name_t close_brace_k          = { "close_brace" };
aggregate_name_t open_parenthesis_k     = { "open_parenthesis" };
aggregate_name_t close_parenthesis_k    = { "close_parenthesis" };
aggregate_name_t dot_k                  = { "dot" };
aggregate_name_t open_bracket_k         = { "open_bracket" };
aggregate_name_t close_bracket_k        = { "close_bracket" };
aggregate_name_t at_k                   = { ".at" };
aggregate_name_t is_k                   = { ".is" };

aggregate_name_t add_k                  = { ".add" };
aggregate_name_t subtract_k             = { ".subtract" };
aggregate_name_t multiply_k             = { ".multiply" };
aggregate_name_t divide_k               = { ".divide" };
aggregate_name_t modulus_k              = { ".modulus" };

aggregate_name_t not_k                  = { ".not" };
aggregate_name_t unary_negate_k         = { ".unary_negate" };

aggregate_name_t less_k                 = { ".less" };
aggregate_name_t greater_k              = { ".greater" };

aggregate_name_t and_k                  = { ".and" };
aggregate_name_t or_k                   = { ".or" };
aggregate_name_t less_equal_k           = { ".less_equal" };
aggregate_name_t greater_equal_k        = { ".greater_equal" };
aggregate_name_t not_equal_k            = { ".not_equal" };
aggregate_name_t equal_k                = { ".equal" };

aggregate_name_t keyword_k              = { "keyword" };

aggregate_name_t empty_k                = { "empty" };
aggregate_name_t true_k                 = { "true" };
aggregate_name_t false_k                = { "false" };

aggregate_name_t function_k             = { ".function" };
aggregate_name_t variable_k             = { ".variable" };
aggregate_name_t bracket_index_k        = { ".bracket_index" };
aggregate_name_t dot_index_k            = { ".dot_index" };
aggregate_name_t array_k                = { ".array" };
aggregate_name_t dictionary_k           = { ".dictionary" };

aggregate_name_t parenthesized_expression_k = { ".parenthesized_expression_k" };
aggregate_name_t name_k                 = { ".name_k" };

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
