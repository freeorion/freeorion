/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_FORMATTER_TOKENS_HPP
#define ADOBE_FORMATTER_TOKENS_HPP

/******************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/name.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/

#define ADOBE_TOKEN(x) extern const aggregate_name_t key_##x;

ADOBE_TOKEN(cell_type)
ADOBE_TOKEN(comment_brief)
ADOBE_TOKEN(comment_detailed)
ADOBE_TOKEN(conditional)
ADOBE_TOKEN(expression)
ADOBE_TOKEN(initializer)
ADOBE_TOKEN(linked)
// ADOBE_TOKEN(name) // comes in from widget_tokens.hpp
ADOBE_TOKEN(parameters)
ADOBE_TOKEN(relation_set)


// The meta type values do not come from the parser; instead it is a type
// used to denote the type of adam node added with "this node". We use the
// meta type instead of storing the nodes in three vectors (one for each meta
// type) to preserve the order in which they were parsed, which is important.

ADOBE_TOKEN(cell_meta_type)
ADOBE_TOKEN(meta_type_cell)
ADOBE_TOKEN(meta_type_relation)
ADOBE_TOKEN(meta_type_interface)

#undef ADOBE_TOKEN

/******************************************************************************/

} // namespace adobe

/******************************************************************************/
// ADOBE_FORMATTER_TOKENS_HPP
#endif

/******************************************************************************/
