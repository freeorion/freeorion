/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://opensource.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_XML_ELEMENT_FOREST_HPP
#define ADOBE_XML_ELEMENT_FOREST_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <algorithm>
#include <iostream>
#include <string>

#include <GG/adobe/algorithm/find.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/forest.hpp>
#include <GG/adobe/iterator.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/xml_parser.hpp>

#include <boost/bind.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/
/*!
    \defgroup xml_element_forest XML to forest round trip utility
        \ingroup apl_libraries

    For every node in the forest, the following keys may be present. The
    description of the keys will also describe additional keys that may be
    present in the dictionary:
        - name_t('> type') (use name_type from the header) : Either of the
          strings "element" or "chardata", denoting the type of information
          found in this particular node.
        - name_t('> chardata') (use name_chardata from the header) : A node in
          the forest whose sole content is character data found between element
          tags.
        - name_t('> element_name') (use name_element_name from the header) : A
          node in the forest that contains the name of an element. Additional
          keys are allowed in the dictionary, and will be output as attributes
          of the element in the XML serialization.
*/
/**************************************************************************************************/
/*!
    \ingroup xml_element_forest
*/
typedef forest<dictionary_t> element_forest_t;

/**************************************************************************************************/

#define ADOBE_XML_NODE_METADATA_NAME(x) \
inline name_t name_##x() { static const name_t name_s(static_name_t("> "#x)); return name_s; }

// These names are prefixed with a '> ' to prevent collisions with XML 
// attribute names (which are not allowed to have the '>' in them).

ADOBE_XML_NODE_METADATA_NAME(type)
ADOBE_XML_NODE_METADATA_NAME(chardata)
ADOBE_XML_NODE_METADATA_NAME(element_name)

/**************************************************************************************************/
#if !defined(ADOBE_NO_DOCUMENTATION)
namespace implementation {

/**************************************************************************************************/

inline std::string token_to_string(const token_range_t& token)
{
    return std::string(reinterpret_cast<const char*>(token.first),
                       std::distance(token.first, token.second));
}

/**************************************************************************************************/

inline name_t token_to_name(const token_range_t& token)
{
    return name_t(token_to_string(token).c_str());
}

/**************************************************************************************************/

inline dictionary_t attribute_set_to_dictionary(const attribute_set_t& set)
{
    dictionary_t result;

    for (attribute_set_t::iterator iter(set.begin()), last(set.end()); iter != last; ++iter)
        result[implementation::token_to_name(iter->first)] =
            any_regular_t(implementation::token_to_string(iter->second));

    return result;
}

/**************************************************************************************************/

struct converter_t
{
    element_forest_t parse(const char* xml)
    {
        element_forest_t root;

        token_range_t xml_range(static_token_range(xml));
    
        make_xml_parser(xml_range.first, xml_range.second,
                               line_position_t("top level xml"),
                               always_true<token_range_t>(),
                               boost::bind(&converter_t::xml_element_node,
                                           boost::ref(*this),
                                           _1, _2, _3, _4,
                                           boost::ref(root),
                                           root.begin()),
                               std::back_inserter(chardata_m))
            .parse_content();
    
        return root;
    }

    token_range_t xml_element_node(const token_range_t&       /*entire_element_range*/,
                                   const token_range_t&       name,
                                   const attribute_set_t&     attribute_set,
                                   const token_range_t&       value,
                                   element_forest_t&          forest,
                                   element_forest_t::iterator parent_node)
    {
        push_chardata(forest, parent_node);    

        element_forest_t::iterator new_parent =
            forest.insert(trailing_of(parent_node), dictionary_t());

        (*new_parent) = implementation::attribute_set_to_dictionary(attribute_set);
    
        (*new_parent)[name_type()] = any_regular_t(std::string("element"));
        (*new_parent)[name_element_name()] = any_regular_t(implementation::token_to_string(name));
    
        make_xml_parser(value.first, value.second,
                               line_position_t("xml_node"),
                               always_true<token_range_t>(),
                               boost::bind(&converter_t::xml_element_node,
                                           boost::ref(*this),
                                           _1, _2, _3, _4,
                                           boost::ref(forest),
                                           new_parent),
                               std::back_inserter(chardata_m))
            .parse_content();

        push_chardata(forest, new_parent);

        return token_range_t();
    }

private:
    std::vector<char> chardata_m;

    void push_chardata(element_forest_t& forest, element_forest_t::iterator parent)
    {
        if (chardata_m.empty())
            return;

        std::string chardata_str(repair_whitespace(std::string(&chardata_m[0], chardata_m.size())));

        if (chardata_str.empty() == false)
        {
            dictionary_t chardata;

            chardata[name_type()] = any_regular_t(std::string("chardata"));
            chardata[name_chardata()] = any_regular_t(chardata_str);

            forest.insert(trailing_of(parent), chardata);
        }

        chardata_m = std::vector<char>();
    }

    static std::string repair_whitespace(const std::string& src)
    {
        std::string                               result;
        std::string::const_iterator               iter(src.begin());
        std::string::const_iterator               last(src.end());
        static const boost::function<bool (char)> isspace =
            boost::bind(&std::isspace<char>, _1, std::locale());

        while (true)
        {
            std::string::const_iterator ws_begin(std::find_if(iter, last, isspace));

            if (iter != ws_begin)
                result << std::string(iter, ws_begin);

            if (ws_begin == last)
                break;

            std::string::const_iterator ws_end(find_if_not(ws_begin, last, isspace));

            result += ' ';

            iter = ws_end;
        }

        return result;
    }
};

/**************************************************************************************************/

template <typename T>
inline void indent_stream(std::ostream& stream, T count)
{ for (; count != 0; --count) stream << "  "; }

/**************************************************************************************************/

} // namespace implementation
#endif
/**************************************************************************************************/
/*!
    \ingroup xml_element_forest

    \brief Converts a forest of dictionaries to an XML file.

    \param f A depth adaptor range of a forest of dictionaries; the source of
             the eventual XML file.
    \param output The output stream to which the XML serialization will be
                  written.
    \param verbose If true, will print a more-formatted, easier-to-read version
                   of XML; if false, will not introduce spacing or newlines to
                   the XML output.
*/
template <typename R> // R is a depth adaptor range
void element_forest_to_xml(const R& f, std::ostream& output, bool verbose = true)
{
    typedef typename boost::range_iterator<R>::type iterator;

    for (iterator first(boost::begin(f)), last(boost::end(f)); first != last; ++first)
    {
        const dictionary_t& node(*first);
        const std::string&  type(get_value(node, name_type()).cast<std::string>());

        if (type == "element")
        {
            if (first.edge() == forest_leading_edge)
            {
                if (verbose)
                    implementation::indent_stream(output, first.depth());
    
                output << "<" << get_value(node, name_element_name());
    
                for (dictionary_t::const_iterator iter(node.begin()), last(node.end());
                     iter != last; ++iter)
                    if (iter->first.c_str()[0] != '>')
                        output << ' ' << iter->first.c_str() << "='" << iter->second << "'";
    
                if (has_children(first))
                {
                    output << ">";

                    if (verbose)
                        output << " <!-- "
                               << std::distance(child_begin(first), child_end(first))
                               << " children -->";
                }
                else
                {
                    output << "/>";
                }

                if (verbose)
                    output << std::endl;
            }
            else if (has_children(first))
            {
                if (verbose)
                    implementation::indent_stream(output, first.depth());

                output << "</" << get_value(node, name_element_name()) << ">";

                if (verbose)
                    output << std::endl;
            }
        }
        else if (type == "chardata" && first.edge() == forest_leading_edge)
        {
            const std::string& chardata(get_value(node, name_chardata()).cast<std::string>());

            if (!verbose)
            {
                output << chardata;
            }
            else
            {
                if (adobe::find_if_not(chardata,
                                       boost::bind(&std::isspace<char>,
                                                   _1,
                                                   std::locale())) == chardata.end())
                    continue;

                implementation::indent_stream(output, first.depth());

                output << chardata << std::endl;
            }
        }
    }
}

/**************************************************************************************************/
/*!
    \ingroup xml_element_forest

    \brief Parses an XML string into a forest of dictionaries.

    \param xml the string to parse into the node forest.

    \return an element forest. See the documentation on the node format above
            for details on what will be within each node's dictionary.
*/
element_forest_t xml_parse_to_forest(const char* xml)
{
    return implementation::converter_t().parse(xml);
}

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

#endif

/**************************************************************************************************/
