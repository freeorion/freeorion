//! Copyright (C) 2006 T. Zachary Laine
//!
//! This library is free software; you can redistribute it and/or
//! modify it under the terms of the GNU Lesser General Public License
//! as published by the Free Software Foundation; either version 2.1
//! of the License, or (at your option) any later version.
//!
//! This library is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//! Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public
//! License along with this library; if not, write to the Free
//! Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//! 02111-1307 USA
//!
//! If you do not wish to comply with the terms of the LGPL please
//! contact the author as other terms are available for a fee.
//!
//! ----
//!
//! Zach Laine
//! whatwasthataddress@hotmail.com/
//!
//! This notice came from the original file from which all the XML parsing code
//! was taken, part of the spirit distribution.  The code was modified slightly
//! by me, and doesn't contain all the original code.  Thanks to Daniel Nuffer
//! for his great work.
//!
//! ----
//!
//! simplexml.cpp
//!
//! Spirit V1.3
//! URL: http://spirit.sourceforge.net/
//!
//! Copyright (c) 2001, Daniel C. Nuffer
//!
//! This software is provided 'as-is', without any express or implied
//! warranty. In no event will the copyright holder be held liable for
//! any damages arising from the use of this software.
//!
//! Permission is granted to anyone to use this software for any purpose,
//! including commercial applications, and to alter it and redistribute
//! it freely, subject to the following restrictions:
//!
//! 1.  The origin of this software must not be misrepresented; you must
//!     not claim that you wrote the original software. If you use this
//!     software in a product, an acknowledgment in the product documentation
//!     would be appreciated but is not required.
//!
//! 2.  Altered source versions must be plainly marked as such, and must
//!     not be misrepresented as being the original software.
//!
//! 3.  This notice may not be removed or altered from any source
//!     distribution.


#include "XMLDoc.h"

#if defined(_MSC_VER) && _MSC_VER >= 1930
struct IUnknown; // Workaround for "combaseapi.h(229,21): error C2760: syntax error: 'identifier' was unexpected here; expected 'type specifier'"
#endif
#include <boost/spirit/include/classic.hpp>

#include <algorithm>
#include <stdexcept>
#include <sstream>


namespace {
    using namespace boost::spirit::classic;

    using chset_t = chset<uint8_t>;

    //! XML grammar rules
    rule<> document, prolog, element, Misc, Reference, CData, doctypedecl,
           XMLDecl, SDDecl, VersionInfo, EncodingDecl, VersionNum, Eq,
           EmptyElemTag, STag, content, ETag, Attribute, AttValue, CharData,
           Comment, CDSect, CharRef, EntityRef, EncName, Name, Comment1, S;

    //! XML Character classes
    const chset_t Char("\x9\xA\xD\x20-\xFF");
    const chset_t Letter("\x41-\x5A\x61-\x7A\xC0-\xD6\xD8-\xF6\xF8-\xFF");
    const chset_t Digit("0-9");
    const chset_t Extender(static_cast<uint8_t>('\xB7'));
    const chset_t NameChar = Letter | Digit | chset_t("._:-") | Extender;
    const chset_t Sch("\x20\x9\xD\xA");
}

//! Creates the XML parsing rules at static initialization time.
struct RuleDefiner {
    RuleDefiner() {
        // This is the start rule for XML parsing
        document =
            prolog >> element >> *Misc
            ;

        S =
            +(Sch)
            ;

        Name =
            (Letter | '_' | ':')
            >> *(NameChar)
            ;

        AttValue =
            '"'
            >> (
                (*(anychar_p - (chset_t('<') | '&' | '"')))[&XMLDoc::AddAttribute]
                | *(Reference)
                )
            >> '"'
            |   '\''
            >> (
                (*(anychar_p - (chset_t('<') | '&' | '\'')))[&XMLDoc::AddAttribute]
                | *(Reference)
                )
            >> '\''
            ;

        chset_t CharDataChar(anychar_p - (chset_t('<') | chset_t('&')));

        CharData =
            (*(CharDataChar - str_p("]]>")))[&XMLDoc::AppendToText]
            ;

        Comment1 =
            *(
                (Char - ch_p('-'))
                | (ch_p('-') >> (Char - ch_p('-')))
                )
            ;

        Comment =
            str_p("<!--") >> Comment1 >> str_p("-->")
            ;

        CDSect =
            str_p("<![CDATA[") >> CData >> str_p("]]>")
            ;

        CData =
            (*(Char - str_p("]]>")))[&XMLDoc::AppendToText]
            ;

        prolog =
            !XMLDecl >> *Misc >> !(doctypedecl >> *Misc)
            ;

        XMLDecl =
            str_p("<?xml")
            >> VersionInfo
            >> !EncodingDecl
            >> !SDDecl
            >> !S
            >> str_p("?>")
            ;

        VersionInfo =
            S
            >> str_p("version")
            >> Eq
            >> (
                ch_p('\'') >> VersionNum >> '\''
                | ch_p('"')  >> VersionNum >> '"'
                )
            ;

        Eq =
            !S >> '=' >> !S
            ;

        chset_t VersionNumCh("A-Za-z0-9_.:-");

        VersionNum =
            +(VersionNumCh)
            ;

        Misc =
            Comment | S
            ;

        doctypedecl =
            str_p("<!DOCTYPE")
            >> *(Char - (chset_t('[') | '>'))
            >> !('[' >> *(Char - ']') >> ']')
            >> '>'
            ;

        SDDecl =
            S
            >> str_p("standalone")
            >> Eq
            >> (
                (ch_p('\'') >> (str_p("yes") | str_p("no")) >> '\'')
                | (ch_p('"')  >> (str_p("yes") | str_p("no")) >> '"')
                )
            ;

        element =
            STag[&XMLDoc::PushElem1] >> content >> ETag
            | EmptyElemTag[&XMLDoc::PushElem2]
            ;

        STag =
            '<'
            >> Name[&XMLDoc::SetElemName]
            >> *(S >> Attribute)
            >> !S
            >> '>'
            ;

        Attribute =
            Name[&XMLDoc::SetAttributeName] >> Eq >> AttValue
            ;

        ETag =
            str_p("</") >> Name[&XMLDoc::PopElem] >> !S >> '>'
            ;

        content =
            !CharData
            >> *(
                (
                    element
                    | Reference
                    | CDSect
                    | Comment
                    )
                >> !CharData
                )
            ;

        EmptyElemTag =
            '<'
            >> Name[&XMLDoc::SetElemName]
            >> *(S >> Attribute)
            >> !S
            >> str_p("/>")
            ;

        CharRef =
            str_p("&#") >> +digit_p >> ';'
            | str_p("&#x") >> +xdigit_p >> ';'
            ;

        Reference =
            EntityRef
            | CharRef
            ;

        EntityRef =
            '&' >> Name >> ';'
            ;

        EncodingDecl =
            S
            >> str_p("encoding")
            >> Eq
            >> (
                ch_p('"')  >> EncName >> '"'
                | ch_p('\'') >> EncName >> '\''
                )
            ;

        chset_t EncNameCh = VersionNumCh - chset_t(':');

        EncName =
            alpha_p >> *(EncNameCh)
            ;
    }
};

namespace {
    RuleDefiner s_rule_definer;

    constinit XMLDoc*        s_curr_parsing_doc = nullptr;
#if defined(__cpp_lib_constexpr_vector)
    constinit
#endif
    std::vector<XMLElement*> s_element_stack;
#if defined(__cpp_lib_constexpr_vector) && defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constinit
#endif
    XMLElement               s_temp_elem;
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constinit
#endif
    std::string              s_temp_attr_name;
}


bool XMLElement::ContainsChild(const std::string& tag) const {
    return children.end() != std::find_if(children.begin(), children.end(),
        [&tag] (const XMLElement& e) { return e.m_tag == tag; });
}

const XMLElement& XMLElement::Child(const std::string& tag) const {
    auto match = std::find_if(children.begin(), children.end(),
        [&tag] (const XMLElement& e) { return e.m_tag == tag; });

    if (match == children.end())
        throw std::out_of_range("XMLElement::Child(): The XMLElement \"" + Tag() + "\" contains no child \"" + tag + "\".");

    return *match;
}

std::string XMLElement::WriteElement(int indent, bool whitespace) const {
    std::stringstream ss;
    WriteElement(ss, indent, whitespace);
    return ss.str();
}

std::ostream& XMLElement::WriteElement(std::ostream& os, int indent, bool whitespace) const {
    if (whitespace)
        os << std::string(indent * 2, ' ');
    os << '<' << m_tag;
    for (const auto& attribute : attributes)
        os << ' ' << attribute.first << "=\"" << attribute.second << "\"";
    if (children.empty() && m_text.empty() && !m_root) {
        os << "/>";
        if (whitespace)
            os << "\n";
    } else {
        os << ">";
        if (!m_text.empty() && m_text.find_first_of("<&") != std::string::npos)
            os << "<![CDATA[" << m_text << "]]>";
        else
            os << m_text;
        if (whitespace && !children.empty())
            os << "\n";
        for (const XMLElement& child : children)
            child.WriteElement(os, indent + 1, whitespace);
        if (whitespace && !children.empty())
            os << std::string(static_cast<std::size_t>(indent) * 2, ' ');
        os << "</" << m_tag << ">";
        if (whitespace) os << "\n";
    }
    return os;
}

XMLElement& XMLElement::Child(const std::string& tag) {
    auto match = std::find_if(children.begin(), children.end(),
                              [&tag] (const XMLElement& e) { return e.m_tag == tag; });

    if (match == children.end())
        throw std::out_of_range("XMLElement::Child(): The XMLElement \"" + Tag() + "\" contains no child \"" + tag + "\".");

    return *match;
}

void XMLElement::SetTag(std::string tag)
{ m_tag = std::move(tag); }

void XMLElement::SetText(std::string text)
{ m_text = std::move(text); }

const std::string& XMLElement::Attribute(const std::string& name) const {
    auto match = std::find_if(attributes.begin(), attributes.end(),
                              [&name] (const auto& p) { return p.first == name; });

    if (match == attributes.end())
        throw std::out_of_range("XMLElement::Attribute(): The XMLElement \"" + Tag() + "\" contains no attribute \"" + name + "\".");

    return match->second;
}

bool XMLElement::HasAttribute(const std::string& name) const noexcept {
    return std::any_of(attributes.begin(), attributes.end(),
                       [&name](const auto& a) { return a.first == name; });
}

XMLDoc::XMLDoc(std::string root_tag) :
    root_node(XMLElement(std::move(root_tag), true))
{}

XMLDoc::XMLDoc(const std::istream& is) :
    root_node(XMLElement())
{}

std::ostream& XMLDoc::WriteDoc(std::ostream& os, bool whitespace) const {
    os << "<?xml version=\"1.0\"?>";
    if (whitespace) os << "\n";
    return root_node.WriteElement(os, 0, whitespace);
}

void XMLDoc::ReadDoc(const std::string& s) {
    std::stringstream ss(s);
    ReadDoc(ss);
}

std::istream& XMLDoc::ReadDoc(std::istream& is) {
    root_node = XMLElement(); // clear doc contents
    s_element_stack.clear();  // clear this to start a fresh read
    s_curr_parsing_doc = this;  // indicate where to add elements
    std::string parse_str;
    std::string temp_str;
    while (is) {
        getline(is, temp_str);
        parse_str += temp_str + '\n';
    }
    parse(parse_str.c_str(), document);
    s_curr_parsing_doc = nullptr;
    return is;
}

void XMLDoc::SetElemName(const char* first, const char* last)
{ s_temp_elem = XMLElement(std::string(first, last)); }

void XMLDoc::SetAttributeName(const char* first, const char* last)
{ s_temp_attr_name = std::string(first, last); }

void XMLDoc::AddAttribute(const char* first, const char* last) {
    auto& attribs = s_temp_elem.attributes;
    auto it = std::find_if(attribs.begin(), attribs.end(),
                           [](const auto& a) { return a.first == s_temp_attr_name; });
    if (it == attribs.end())
        attribs.emplace_back(s_temp_attr_name, std::string{first, last});
    else
        it->second = std::string{first, last};
}

void XMLDoc::PushElem1(const char* first, const char* last) {
    if (XMLDoc* this_ = s_curr_parsing_doc) {
        if (s_element_stack.empty()) {
            this_->root_node = s_temp_elem;
            s_element_stack.emplace_back(&this_->root_node);
        } else {
            s_element_stack.back()->children.emplace_back(s_temp_elem);
            s_element_stack.emplace_back(&s_element_stack.back()->children.back());
        }
    }
}

void XMLDoc::PushElem2(const char* first, const char* last) {
    if (XMLDoc* this_ = s_curr_parsing_doc) {
        if (s_element_stack.empty()) {
            this_->root_node = s_temp_elem;
        } else {
            s_element_stack.back()->children.emplace_back(s_temp_elem);
        }
    }
}

void XMLDoc::PopElem(const char*, const char*) {
    if (!s_element_stack.empty())
        s_element_stack.pop_back();
}

void XMLDoc::AppendToText(const char* first, const char* last) {
    if (!s_element_stack.empty()) {
        std::string text(first, last);
        std::string::size_type first_good_posn = (text[0] != '\"') ? 0 : 1;
        std::string::size_type last_good_posn = text.find_last_not_of(" \t\n\"\r\f");
        // strip of leading quote and/or trailing quote, and/or trailing whitespace
        if (last_good_posn != std::string::npos)
            s_element_stack.back()->m_text += text.substr(first_good_posn, (last_good_posn + 1) - first_good_posn);
    }
}
