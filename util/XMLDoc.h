#ifndef _XMLDoc_h_
#define _XMLDoc_h_

//! @copyright
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
//! Zach Laine
//! whatwasthataddress@hotmail.com

//! @file
//!     Declares XMLElement and XMLDoc class to modify, read and write simple
//!     XML files.

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "Export.h"

//! Represents a simplified XML markup element.
//!
//! An XMLElement represents a XML element from the opening tag \<tag-name\>,
//! including its attributes to the corresponding closing tag \</tag-name\>.
//! This may or may not include a text data section @b OR child XML elements.
//! Using the "burns" example:
//!
//! ```{.xml}
//!   <burns>Say <quote>Goodnight</quote> Gracie.</burns>
//! ```
//!
//! may not work as expected.  The resulting XMLElement \<burns\> node will
//! contain both the "Say " and the " Gracie." text fragment.  Or represented
//! differently:
//!
//! ```{.js}
//! burns.Text() == "Say  Gracie."
//! ```
//!
//! However, if used to represent data structures like the example struct foo:
//!
//! ```{.cpp}
//! struct foo
//! {
//!    int ref_ct;
//!    double data;
//! };
//! ```
//!
//! the current implementation creates a XML representation similar to:
//!
//! ```{.xml}
//! <bar>
//!   <foo>
//!     <ref_ct>13<ref_ct/>
//!     <data>0.364951<data/>
//!   </foo>
//! </bar>
//! ```
//!
//! Further, while the "burns" example is standard XML, an XMLElement optionally
//! accepts its single text string in quotes, and strips off trailing white
//! space, in direct contrary to the XML standard.  So "burns" from above is
//! equivalent to:
//!
//! ```{.xml}
//! <burns>"Say  Gracie."<quote>Goodnight</quote></burns>
//! ```
//!
//! or:
//!
//! ```{.xml}
//! <burns>Say  Gracie.<quote>Goodnight</quote></burns>
//! ```
//! or:
//!
//! ```{.xml}
//! <burns>"Say  Gracie."
//!   <quote>Goodnight</quote>
//! </burns>
//! ```
//!
//! or:
//!
//! ```{.xml}
//! <burns>Say  Gracie.
//!   <quote>Goodnight</quote>
//! </burns>
//! ```
//!
//! Each of these examples yields to
//!
//! ```{.js}
//! burns.Text() == "Say  Gracie."
//! ```
//!
//! When an XMLElement is saved, its text is saved within a CDATA section.  Any
//! string can be put inside one of these quoted text fields, even text that
//! includes an arbitrary number of quotes.  So any std::string or c-string can
//! be assigned to an element.  However, when hand-editing an XML file
//! containing such text strings, one needs to be careful.  The closing quote
//! must be the last thing other than white space.  Adding more than one quoted
//! text string to the XML element, with each string separated by other
//! elements, will result in a single concatenated string, as illustrated above.
//!
//! This is not the most time- or space-efficient way to organize object data,
//! but it may just be one of the simplest and most easily read.
class FO_COMMON_API XMLElement
{
public:
    //! Creates a new XMLElement with an empty tag-name assigned.
    //!
    //! Create a new XMLElement with no tag-name, text, attribute or child nodes
    //! set.  Also the new instance isn't marked as root node.
#if defined(__cpp_lib_constexpr_vector) && defined(__cpp_lib_constexpr_string)
    constexpr
#endif
    XMLElement() noexcept = default;

    //! Creates a new XMLElement with the given @p tag tag-name and @p text
    //! content.
    //!
    //! Create a new XMLElement with the given @p tag tag-name and @p text
    //! content, but no attribute or child nodes set.  Also the new instance
    //! isn't marked as root node.
    //!
    //! @param[in] tag
    //!     The tag name of this XML element.
    //! @param[in] text
    //!     The text assigned to this XML element.
#if defined(__cpp_lib_constexpr_vector)
    constexpr
#endif
    explicit XMLElement(std::string tag, std::string text = "") noexcept :
        m_tag(std::move(tag)),
        m_text(std::move(text))
    {}

    //! Returns the tag-name of this XMLElement.
    //!
    //! @return
    //!     The tag-name of this XMLElement.  Can be an empty string.
    const auto& Tag() const noexcept { return m_tag; }

    //! Returns the text body of this XMLElement.
    //!
    //! @return
    //!     The text content of this XMLElement.  Can be an empty string.
    const auto& Text() const noexcept { return m_text; }

    const auto& Children() const noexcept { return children; }

    //! Returns if this XMLElement contains a child with @p tag as tag-name.
    //!
    //! @param[in] tag
    //!     The tag-name of the searched child XMLElement.
    //! @return
    //!     True if there is at least one child with a @p tag tag-name, false if
    //!     not.
    bool ContainsChild(const std::string& tag) const;

    //! Returns the first XMLElement child that has @p tag as tag-name.
    //!
    //! @param[in] tag
    //!     The tag-name of the child XMLElement requested.
    //! @return
    //!     A reference to the first XMLElement child which has the tag-name
    //!     @p tag.
    //! @throw std::out_of_range
    //!     When no child with a tag-name @p tag exists.
    const XMLElement& Child(const std::string& tag) const;

    //! Write this XMLElement XML formatted into the given output stream @p os
    //! with indentation level @p indent when @p whitespace is set.
    //!
    //! @param[in] os
    //!     The output stream this document should be written to.
    //! @param[in] indent
    //!     The indentation level this element should be indented.
    //! @param[in] whitespace
    //!     If set to true the child XMLElement%s are indented and newline
    //!     separated.
    //! @return
    //!     The given @p os output stream.
    std::ostream& WriteElement(std::ostream& os, int indent = 0, bool whitespace = true) const;

    //! Return this XMLElement XML formatted as string with indentation level
    //! @p indent when @p whitespace is set.
    //!
    //! @param[in] indent
    //!     The indentation level this element should be indented.
    //! @param[in] whitespace
    //!     If set to true the child XMLElement%s are indented and newline
    //!     separated.
    //! @return
    //!     A string containing the XML formatted representation of this
    //!     XMLElement.
    std::string WriteElement(int indent = 0, bool whitespace = true) const;

    //! @see  XMLElement::Child(const std::string&) const
    XMLElement& Child(const std::string& tag);

    const std::string& Attribute(const std::string& name) const;
    bool HasAttribute(const std::string& name) const noexcept;

    //! Sets the tag-name of this XMLElement to @p tag.
    //!
    //! @param[in] tag
    //!     The new tag-name this XMLElement should have.
    void SetTag(std::string tag);

    //! Sets the text content of this XMLEement to @p text.
    //!
    //! @param[in] text
    //!     The new text content this XMLElement should have.
    void SetText(std::string text);

    void AddChild(XMLElement child) { children.push_back(std::move(child)); }

private:
    //! The attributes associated to this XMLElement by key name mapping.
    std::vector<std::pair<std::string, std::string>> attributes;

    //! Stores a list of the child XMLElement%s associated to this XMLElement.
    //!
    //! This list can be empty when this XMLElement has no associated child
    //! elements.
    std::vector<XMLElement> children;

    //! Creates a new XMLElement with the given @p tag tag-name and marked as
    //! root node, if @p root is set.
    //!
    //! @param[in] tag
    //!     The tag name of this XML element.
    //! @param[in] root
    //!     When true this XMLElement should be interpreted as root node in
    //!     a XMLElement tree.
    //!
    //! @note
    //!     Called by friend XMLDoc.
    XMLElement(std::string tag, bool root) :
        m_tag(std::move(tag)),
        m_root(root)
    {}

    //! Stores the tag-name associated to this XMLElement.
    //!
    //! @bug
    //!     Currently this can contain an empty string but I doubt it will be
    //!     useful as it will cause invalid XML documents serializations.
    std::string m_tag;

    //! Stores the text content associated to this XMLElement.
    std::string m_text;

    //! Set to true if this XMLElement is the root element of an XMLDoc
    //! document.
    bool m_root = false;

    friend class XMLDoc;
};

//! Represents a document formatted with XML markup.
//!
//! Each XMLDoc instance is assumed to represent a complete document.  It
//! contains a tree of nested XMLElement%s, starting from the always existing
//! root XMLElement node.
class FO_COMMON_API XMLDoc
{
public:
    //! Create an empty document with the given tag-name @p root_tag.
    //!
    //! @param[in] root_tag
    //!     The tag-name of the created root XMLElement.
    XMLDoc(std::string root_tag = "XMLDoc");

    //! Construct a document from the given input stream @p is.
    //!
    //! @param[in] is
    //!     An input stream that provides an XML markup document once read.
    //!
    //! @bug
    //!     @p is isn't actually read but ignored and an empty (and maybe
    //!     invalid) document is created.  Use XMLDoc::ReadDoc(std::istream&)
    //!     instead.
    XMLDoc(const std::istream& is);

    //! Write the contents of the XMLDoc into the given output stream @p os
    //! with optional @p indent.
    //!
    //! @param[in] os
    //!     The output stream this document should be written to.
    //! @param[in] indent
    //!     If set to true the XML elements are indented and newline separated.
    //!
    //! @return
    //!     The given @p os output stream.
    std::ostream& WriteDoc(std::ostream& os, bool indent = true) const;

    //! Clears the current content of this XMLDoc instance and read a new
    //! document from the given input stream @p is.
    //!
    //! @param[in] is
    //!     An input stream that provides an XML markup document once read.
    //!
    //! @return
    //!     The given @p is input stream.
    std::istream& ReadDoc(std::istream& is);

    //! Clears the current content of this XMLDoc instance and read a new
    //! document from the given string @p s.
    //!
    //! @param[in] s
    //!     A string containing an XML markup document.
    void ReadDoc(const std::string& s);

    //! The root element that contains the parsed document, which is represented
    //! by XMLElement%s.
    XMLElement root_node;

private:
    static void SetElemName(const char* first, const char* last);
    static void SetAttributeName(const char* first, const char* last);
    static void AddAttribute(const char* first, const char* last);
    static void PushElem1(const char*, const char*);
    static void PushElem2(const char*, const char*);
    static void PopElem(const char*, const char*);
    static void AppendToText(const char* first, const char* last);
    //!@}

    friend struct RuleDefiner;
};


#endif
