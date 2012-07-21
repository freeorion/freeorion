/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifdef ADOBE_STD_SERIALIZATION

/*************************************************************************************************/

#include <boost/cstdint.hpp>

#include <GG/adobe/iomanip_asl_cel.hpp>
#include <GG/adobe/implementation/expression_filter.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void asl_cel_format::begin_format(stream_type& os)
{ push_stack(os, format_element_t(name_t("asl_cel"))); }

/*************************************************************************************************/

void asl_cel_format::begin_bag(stream_type& os, const std::string& ident)
{ push_stack(os, format_element_t(bag_name_g, ident)); }

/*************************************************************************************************/

void asl_cel_format::begin_sequence(stream_type& os)
{ push_stack(os, format_element_t(seq_name_g)); }

/*************************************************************************************************/

void asl_cel_format::begin_atom(stream_type& os, const any_regular_t& value)
{ push_stack(os, format_element_t(atom_name_g, value)); }

/*************************************************************************************************/

void asl_cel_format::stack_event(stream_type& os, bool is_push)
{
    const format_element_t& top(stack_top());
    name_t                  self(top.tag());
    name_t                  parent(stack_depth() >= 2 ? stack_n(1).tag() : name_t());
    name_t                  grandparent(stack_depth() >= 3 ? stack_n(2).tag() : name_t());

    if (self == atom_name_g)
    {
        handle_atom(os, is_push);
    }
    else if (is_push)
    {
        if (self == bag_name_g)
        {
            os << '{';

            up();
        }
        else if (self == seq_name_g && parent != bag_name_g)
        {
            os << "[ ";
        }
    }
    else
    {
        if (self == bag_name_g)
        {
            down();

            if (top.num_out_m > 0)
                os << '\n' << indents(depth());
            else
                os << ' ';

            os << '}';
        }
        else if (self == seq_name_g && parent != bag_name_g)
        {
            if (top.num_out_m > 0)
                os << ' ';

            os << ']';
        }
    }
}

/*************************************************************************************************/

void asl_cel_format::handle_atom(stream_type& os, bool is_push)
{
    const format_element_t& top(stack_top());
    name_t                  self(top.tag());
    name_t                  parent(stack_depth() >= 2 ? stack_n(1).tag() : name_t());
    name_t                  grandparent(stack_depth() >= 3 ? stack_n(2).tag() : name_t());
    const any_regular_t&    value(top.value());
    bool                    outputting_bag(parent == seq_name_g && grandparent == bag_name_g);
    std::size_t&            num_out(outputting_bag ? stack_n(2).num_out_m : stack_n(1).num_out_m);
    bool                    named_argument(outputting_bag && (num_out & 0x1) == 0);

    if (is_push)
    {
        // if this is not the first item in the element, add a comma and set up a newline
        if (num_out > 0)
        {
            if (!outputting_bag)
            {
                os << ", ";
            }
            else if (named_argument)
            {
                os << ",\n" << indents(depth());
            }
        }
        else if (outputting_bag)
        {
            os << '\n' << indents(depth());
        }

        if (value.type_info() == adobe::type_info<string_t>())
        {
            bool escape(needs_entity_escape(value.cast<adobe::string_t>()));

            if (escape_m && escape)
                os << "xml_unescape(";

            os << '\"'
               << (escape_m && escape ?
                   entity_escape(value.cast<string_t>()) :
                   value.cast<string_t>())
               << '\"';

            if (escape_m && escape)
                os << ")";
        }
        else if (value.type_info() == adobe::type_info<name_t>())
        {
            if (!named_argument)
                os << '@';

            os << value.cast<name_t>();

            if (outputting_bag && named_argument)
                os << ": ";
        }
        else if (value.type_info() == adobe::type_info<bool>())
        {
            os << (value.cast<bool>() ? "true" : "false");
        }
        else if (value.type_info() == adobe::type_info<double>())
        {
            double         dbl_val(value.cast<double>());
            boost::int64_t int_val(static_cast<boost::int64_t>(dbl_val));

            if (dbl_val == int_val)
            {
                os << int_val;
            }
            else
            {
                // For asl_cel, we want to output floating-point values in decimal-based
                // fixed-point notation (asl_cel doesn't support any other format) with
                // a very high precision for accceptable roundtrip values.
    
                os.setf(std::ios_base::dec, std::ios_base::basefield);
                os.setf(std::ios_base::fixed, std::ios_base::floatfield);
                os.precision(16);

                os << dbl_val;
            }
        }
        else if (value.type_info() == adobe::type_info<empty_t>())
        {
            os << value.cast<empty_t>();
        }
        else if (value.type_info() == adobe::type_info<dictionary_t>())
        {
            os << value.cast<dictionary_t>();
        }
        else if (value.type_info() == adobe::type_info<array_t>())
        {
            os << value.cast<array_t>();
        }
        else
        {
            os << "'cel_unknown: " << value.type_info().name() << "'";
        }
    }
    else
    {
        // up the number of outputted items for the parent to this atom
        ++num_out;
    }
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
