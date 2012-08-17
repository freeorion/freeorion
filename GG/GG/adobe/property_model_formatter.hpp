/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_PROPERTY_MODEL_FORMATTER_HPP
#define ADOBE_PROPERTY_MODEL_FORMATTER_HPP

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
    @defgroup apl_property_model_formatter Property Model Formatter
        @ingroup apl_libraries

    @brief Property model description assembler/disassembler

    This code is useful for parsing a property model description to an
    intermediate format for later modification and/or formatting back out to a
    CEL-based property model.

    The layout disassembler finds three basic property model components during a
    parse called <i>meta cells</i>. The first meta cell type is a <i>cell</i>,
    the second a <i>relation</i>, and the third an <i>interface</i>. Cells are
    further broken down to one of several <i>cell types</i>, one of constant,
    input, output, logic, or invariant.

    The set of parsed property model cells are stored in a vector of dictionaries. Each
    dictionary contains within it a set of associated values that completely
    describe the property model cell parsed. They are:

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
        <tr><th>key</th><th>mapped type</th><th>found in</th><th>description</th><th>notes</th></th></tr>
        <tr>
            <td><code>meta_cell_type</code></td>
            <td>adobe::name_t</td>
            <td>all dictionaries except relation subdictionaries</td>
            <td>The meta type of the cell parsed</td>
            <td>Value is one of <code>key_meta_type_cell</code>, <code>key_meta_type_relation</code> or <code>key_meta_type_interface</code>.</td>
        </tr>
        <tr>
            <td><code>cell_type</code></td>
            <td>adobe::name_t</td>
            <td>Cells whose <code>meta_type_cell</code> is <code>meta_type_cell</code>.</td>
            <td>The type of cell parsed</td>
            <td>Value is one of <code>interface</code>, <code>constant</code>, <code>input</code>, <code>output</code>, <code>logic</code>, <code>invariant</code>.</td>
        </tr>
        <tr>
            <td><code>comment_brief</code></td>
            <td>adobe::string_t</td>
            <td>all</td>
            <td></td>
            <td>The comment found after the cell parsed in C++ "<code>//...</code>" format</td>
        </tr>
        <tr>
            <td><code>comment_detailed</code></td>
            <td>adobe::string_t</td>
            <td>all</td>
            <td></td>
            <td>The comment found before the cell parsed in C "<code>/</code><code>*...*</code><code>/</code>" format</td>
        </tr>
        <tr>
            <td><code>conditional</code></td>
            <td>adobe::array_t</td>
            <td>relation cells</td>
            <td>Parsed token expression</td>
            <td>Used only with relation cells, and contains the expression <code>p</code> in the relation expression <code>when (p) relate { ... }</code></td>
        </tr>
        <tr>
            <td><code>expression</code></td>
            <td>adobe::array_t</td>
            <td>interface, output, logic, and invariant cells; relation subdictionary expressions</td>
            <td>Parsed token expression</td>
            <td>Expressions are the specific expressions found after the use of <code><==</code> in the CEL grammar</td>
        </tr>
        <tr>
            <td><code>initializer</code></td>
            <td>adobe::array_t</td>
            <td>interface, input, and constant cells</td>
            <td>Parsed token expression</td>
            <td>Initializers are the specific expressions found after the use of <code>:</code> in the CEL grammar</td>
        </tr>
        <tr>
            <td><code>linked</code></td>
            <td>bool</td>
            <td>interface cells</td>
            <td></td>
            <td>Denotes the existence of the <code>unlink</code> token in the CEL description</td>
        </tr>
        <tr>
            <td><code>name</code></td>
            <td>adobe::name_t</td>
            <td>all dictionaries</td>
            <td>Name of the cell</td>
            <td></td>
        </tr>
        <tr>
            <td><code>relation_set</code></td>
            <td>adobe::array_t</td>
            <td>relation cells</td>
            <td>Contain a vector of dictionaries sotred in an array_t.</td>
            <td>Each relation subdictionary completely describes one relation in the set. Each relation dictionary contains the following keys as previously described: <code>name</code>, <code>expression</code>, <code>comment_brief</code> and <code>comment_detailed</code></td>
        </tr>
    </table>
    @endhtmlonly

    @par Example
    @par Source File:
    @verbatim
sheet travel
{
interface:
    rate : 1; // "meters/sec" implied unit
    time : 1; // "seconds" implied unit
    dist : 1; // "meters" implied unit

logic:
    relate {
        rate <== round(dist / time);
        time <== round(dist / rate);
        dist <== rate * time;
    }

output:
    result <== {
        rate: rate,
        time: time,
        dist: dist
    };

invariant:
    rate_min <== rate >= 0;
    time_min <== time >= 0;
    dist_min <== dist >= 0;
}
    @endverbatim
    @par Cell Set Result:
    @verbatim
[ {
        cell_meta_type: @key_meta_type_interface,
        comment_brief: ' "meters/sec" implied unit',
        comment_detailed: '',
        expression: [ ],
        initializer: [ 1 ],
        linked: true,
        name: @rate
}, {
        cell_meta_type: @key_meta_type_interface,
        comment_brief: ' "seconds" implied unit',
        comment_detailed: '',
        expression: [ ],
        initializer: [ 1 ],
        linked: true,
        name: @time
}, {
        cell_meta_type: @key_meta_type_interface,
        comment_brief: ' "meters" implied unit',
        comment_detailed: '',
        expression: [ ],
        initializer: [ 1 ],
        linked: true,
        name: @dist
}, {
        cell_meta_type: @key_meta_type_relation,
        comment_brief: '',
        comment_detailed: '',
        conditional: [ ],
        relation_set: [ {
                comment_brief: '',
                comment_detailed: '',
                expression: [ @dist, @.variable, @time, @.variable, @.divide, 1, @.array, @round, @.function ],
                name: @rate
        }, {
                comment_brief: '',
                comment_detailed: '',
                expression: [ @dist, @.variable, @rate, @.variable, @.divide, 1, @.array, @round, @.function ],
                name: @time
        }, {
                comment_brief: '',
                comment_detailed: '',
                expression: [ @rate, @.variable, @time, @.variable, @.multiply ],
                name: @dist
        } ]
}, {
        cell_meta_type: @key_meta_type_cell,
        cell_type: @output,
        comment_brief: '',
        comment_detailed: '',
        expression: [ @rate, @rate, @.variable, @time, @time, @.variable, @dist, @dist, @.variable, 3, @.dictionary ],
        name: @result
}, {
        cell_meta_type: @key_meta_type_cell,
        cell_type: @invariant,
        comment_brief: '',
        comment_detailed: '',
        expression: [ @rate, @.variable, 0, @.greater_equal ],
        name: @rate_min
}, {
        cell_meta_type: @key_meta_type_cell,
        cell_type: @invariant,
        comment_brief: '',
        comment_detailed: '',
        expression: [ @time, @.variable, 0, @.greater_equal ],
        name: @time_min
}, {
        cell_meta_type: @key_meta_type_cell,
        cell_type: @invariant,
        comment_brief: '',
        comment_detailed: '',
        expression: [ @dist, @.variable, 0, @.greater_equal ],
        name: @dist_min
} ]
    @endverbatim

    @see
        - adobe::format_expression
*/
/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    @ingroup apl_property_model_formatter

    The result type of adobe::disassemble_sheet. It is a vector of dictionaries,
    each representing a property model cell parsed in the sheet.
*/
typedef vector<dictionary_t> sheet_assembly_t;

/******************************************************************************/
/*!
    @ingroup apl_property_model_formatter

    Breaks down a property model definition to its fundamental components.

    @param stream   the property model definition stream
    @param position an adobe::line_position_t describing the stream

    @return an adobe::sheet_assembly_t fully describing the property model cells
*/
sheet_assembly_t disassemble_sheet(std::istream&          stream,
                                   const line_position_t& position);

/******************************************************************************/
/*!
    @ingroup apl_property_model_formatter

    Assembles fundamental property model components into a CEL-syntax property
    model definition.

    @param sheet_name the name to give to the final property model.
    @param assembly   the fundamental property model components to be used in
                      the property model creation
    @param out        the stream to which the final result is to be output
*/
void assemble_sheet(const string_t&         sheet_name,
                    const sheet_assembly_t& assembly,
                    std::ostream&           out);

/******************************************************************************/

} // namespace adobe

/******************************************************************************/
// ADOBE_PROPERTY_MODEL_FORMATTER_HPP
#endif

/******************************************************************************/
