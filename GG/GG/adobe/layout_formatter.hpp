/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_LAYOUT_FORMATTER_HPP
#define ADOBE_LAYOUT_FORMATTER_HPP

/******************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/forest.hpp>
#include <GG/adobe/formatter_tokens.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/utility/pair.hpp>
#include <GG/adobe/vector.hpp>

/******************************************************************************/
/*!
    @defgroup apl_layout_formatter Layout Formatter
        @ingroup apl_libraries

    @brief Layout description assembler/disassembler

    This code is useful for parsing a layout description to an intermediate
    format for later modification and/or formatting back out to a CEL-based
    layout.

    The layout disassembler finds two basic layout components during a parse.
    The first a forest of view declarations, the second is a set of layout
    cells.

    Each parsed view and cell in the layout is is represented by its own
    dictionary. Each dictionary contains within it a set of associated values
    that completely describe the layout cell or view that has been parsed. They
    are:

    @htmlonly
    <style type="text/css">
        table.parameter_set {
            width: 100%;
            border-width: 1px;
            border-spacing: 2px;
            border-style: outset;
            border-color: gray;
            border-collapse: collapse;
        }
        table.parameter_set th {
            border-width: 1px;
            padding: 1px;
            border-style: inset;
            border-color: gray;
        }
        table.parameter_set td {
            border-width: 1px;
            padding: 3px;
            border-style: inset;
            border-color: gray;
            background-color: white;
        }
    </style>

    <table class='parameter_set'>
        <tr><th>key</th><th>mapped type</th><th>cell/view/both</th><th>description</th><th>notes</th></th></tr>
        <tr>
            <td><code>cell_type</code></td>
            <td>adobe::name_t</td>
            <td>cell</td>
            <td>The type of cell parsed</td>
            <td>Value is one of <code>interface</code> or <code>constant</code>.</td>
        </tr>
        <tr>
            <td><code>comment_brief</code></td>
            <td>adobe::string_t</td>
            <td>both</td>
            <td></td>
            <td>The comment found after the view parsed in C++ "<code>//...</code>" format</td>
        </tr>
        <tr>
            <td><code>comment_detailed</code></td>
            <td>adobe::string_t</td>
            <td>both</td>
            <td></td>
            <td>The comment found before the view parsed in C "<code>/</code><code>*...*</code><code>/</code>" format</td>
        </tr>
        <tr>
            <td><code>initializer</code></td>
            <td>adobe::array_t</td>
            <td>cell</td>
            <td>Parsed token expression</td>
            <td>Use a virtual_machine_t to evaluate the token vector to get the initial state of the cell</td>
        </tr>
        <tr>
            <td><code>name</code></td>
            <td>adobe::name_t</td>
            <td>both</td>
            <td>Name of the cell or view</td>
            <td></td>
        </tr>
        <tr>
            <td><code>parameters</code></td>
            <td>adobe::array_t</td>
            <td>view</td>
            <td>Named argument set parsed as the attributes of the view</td>
            <td>The array is an expression vector that always evaluates to an adobe::dictionary_t. To get the dictionary the expression must be evaluated by an adobe::virtual_machine_t</td>
        </tr>
    </table>
    @endhtmlonly

    @par Example
    @par Source File:
    @verbatim
layout sample_dialog
{
    constant:
        pi : 355/113; // approximation

    interface:
        advanced : false;

    view dialog(name: "Sample Dialog")
    {
        edit_text(name: 'Name:', bind: @cell_name); // Maybe full name?
        checkbox(name: 'Advanced Options', bind: @advanced);
    }
}
    @endverbatim
    @par View Forest and Cell Set Result:
    @verbatim
[ {
        comment_brief: '',
        comment_detailed: '',
        name: @dialog,
        parameters: [ @name, 'Sample Dialog', 1, @.dictionary ]
}, [ {
        comment_brief: ' Maybe full name?',
        comment_detailed: '',
        name: @edit_text,
        parameters: [ @name, 'Name:', @bind, @cell_name, 2, @.dictionary ]
}, {
        comment_brief: '',
        comment_detailed: '',
        name: @checkbox,
        parameters: [ @name, 'Advanced Options', @bind, @advanced, 2, @.dictionary ]
} ] ]

[ {
        cell_type: @constant,
        comment_brief: ' approximation',
        comment_detailed: '',
        initializer: [ 355, 113, @.divide ],
        name: @pi
}, {
        cell_type: @interface,
        comment_brief: '',
        comment_detailed: '',
        initializer: [ false ],
        name: @advanced
} ]
    @endverbatim

    @see
        - adobe::format_expression
*/
/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    @ingroup apl_layout_formatter

    The result type of adobe::disassemble_layout. <code>first</code> is a forest
    of dictionaries, each dictionary describing a node parsed in the layout.
    <code>second</code> is a vector of dictionaries, each representing a layout
    cell parsed in the layout.
*/
typedef pair<forest<dictionary_t>, vector<dictionary_t> > layout_assembly_t;

/******************************************************************************/
/*!
    @ingroup apl_layout_formatter

    Breaks down a layout definition to its fundamental components.

    @param stream   the layout definition stream
    @param position an adobe::line_position_t describing the stream

    @return an adobe::layout_assembly_t fully describing the layout components
*/
layout_assembly_t disassemble_layout(std::istream&          stream,
                                     const line_position_t& position);

/******************************************************************************/
/*!
    @ingroup apl_layout_formatter

    Assembles fundamental layout components into a CEL-syntax layout definition.

    @param layout_name the name to give to the final layout.
    @param assembly    the fundamental layout components to be used in the
                       layout creation
    @param out         the stream to which the final result is to be output
*/
void assemble_layout(const string_t&          layout_name,
                     const layout_assembly_t& assembly,
                     std::ostream&            out);

/******************************************************************************/

} // namespace adobe

/******************************************************************************/
// ADOBE_LAYOUT_FORMATTER_HPP
#endif

/******************************************************************************/
