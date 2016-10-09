#ifndef _XMLDoc_h_
#define _XMLDoc_h_

/* Copyright (C) 2006 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@hotmail.com */

#ifndef _GG_Enum_h_
#include <GG/Enum.h>
#endif

#ifndef _GG_Exception_h_
#include <GG/Exception.h>
#endif

#include <boost/lexical_cast.hpp>

#include <map>
#include <string>
#include <vector>

#include "Export.h"

/** @file
 * @brief  Declares free functions and classes to modify, read and write simple
 *      XML files.
 */

/** @brief  Represents a simplified XML markup element.
 *
 * An XMLElement represent represents a XML element from the opening tag
 * \<tag-name\>, including its attributes to the corresponding closing tag
 * \</tag-name\>.  This may or may not include a text data section \b OR child
 * XML elements.
 *
 * For take the "burns" example:
   \code{.xml}
   <burns>Say <quote>Goodnight</quote> Gracie.</burns>
   \endcode
 *
 * may not work as expected.  The resulting XMLElement \<burns\> node will
 * contain both the "Say " and the " Gracie." text fragment.  Or represented
 * differently:
 *
   \code{.js}
   burns.Text() == "Say  Gracie."
   \endcode
 *
 * However, if used to represent data structures like the example struct foo:
 *
   \code{.cpp}
   struct foo
   {
      int ref_ct;
      double data;
   };
   \endcode
 *
 * the current implementation creates a XML representation similar to:
 *
   \code{.xml}
   <bar>
     <foo>
       <ref_ct>13<ref_ct/>
       <data>0.364951<data/>
     </foo>
   </bar>
   \endcode
 *
 * Further, while the "burns" example is standard XML, an XMLElement optionally
 * accepts its single text string in quotes, and strips off trailing white
 * space, in direct contrary to the XML standard.  So "burns" from above is
 * equivalent to:
   \code{.xml}
   <burns>"Say  Gracie."<quote>Goodnight</quote></burns>
   \endcode
 * or:
   \code{.xml}
   <burns>Say  Gracie.<quote>Goodnight</quote></burns>
   \endcode
 * or:
   \code{.xml}
   <burns>"Say  Gracie."
     <quote>Goodnight</quote>
   </burns>
   \endcode
 * or:
    \code{.xml}
    <burns>Say  Gracie.
      <quote>Goodnight</quote>
    </burns>
    \endcode
 *
 * Each of these examples yields to
   \code{.js}
   burns.Text() == "Say  Gracie."
   \endcode
 *
 * When an XMLElement is saved, its text is saved within a CDATA section.  Any
 * string can be put inside one of these quoted text fields, even text that
 * includes an arbitrary number of quotes.  So any std::string or c-string can
 * be assigned to an element.  However, when hand-editing an XML file containing
 * such text strings, need to be a bit careful.  The closing quote must be the
 * last thing other than white space.  Adding more than one quoted text string
 * to the XML element, with each string separated by other elements, will result
 * in a single concatenated string, as illustrated above.
 *
 * This is not the most time- or space-efficient way to organize object data,
 * but it may just be one of the simplest and most easily read.
 */
class FO_COMMON_API XMLElement
{
public:
    /** \name  Exceptions */ //@{
    /** @brief  The base class for XMLElement based exceptions. */
    class Exception : public GG::ExceptionBase
    {
    public:
        /** @brief  Create a new exception with the given @p message. */
        Exception (const std::string& message) :
            ExceptionBase(message)
        {}
    };

    /** @brief  Thrown when a request for a tag-name named  child element cannot
     *      be fulfilled.
     */
    class NoSuchChild : public Exception
    {
    public:
        /** @copydoc Exception::Exception(const std::string&) */
        NoSuchChild (const std::string& message) :
            Exception (message)
        {}

        virtual const char* type() const throw()
        { return "XMLElement::NoSuchChild"; }
    };

    /** @brief  Thrown when a request for an indexed child element cannot be
     *      fulfilled.
     */
    class NoSuchIndex : public Exception
    {
    public:
        /** @copydoc Exception::Exception(const std::string&) */
        NoSuchIndex (const std::string& message) :
            Exception (message)
        {}

        virtual const char* type() const throw()
        { return "XMLElement::NoSuchIndex"; }
    };
    //@}

    /** @name  Iterators */ //@{
    //@{
    /** @brief Iterator to iterate over child XMLElement%s. */
    typedef std::vector<XMLElement>::iterator       child_iterator;
    typedef std::vector<XMLElement>::const_iterator const_child_iterator;
    //@}
    //@{
    /** @brief Iterator to iterate over a XMLElement attributes. */
    typedef std::map<std::string, std::string>::iterator       attr_iterator;
    typedef std::map<std::string, std::string>::const_iterator const_attr_iterator;
    //@}
    //@}

    /** @name  Structors */ //@{
    /** @brief  Creates a new XMLElement with an empty tag-name assigned.
     *
     * Create a new XMLElement with no tag-name, text, attribute or child nodes
     * set.  Also the new instance isn't marked as root node.
     */
    XMLElement() : m_root(false)
    {}


    /** @brief  Creates a new XMLElement with the given @p tag tag-name.
     *
     * Create a new XMLElement with the given @p tag tag-name but no text,
     * attribute or child nodes set.  Also the new instance isn't marked as root
     * node.
     *
     * @param[in] tag  The tag name of this XML element.
     */
    XMLElement(const std::string& tag) :
        m_tag(tag),
        m_text(""),
        m_root(false)
    {}

    /** @brief  Creates a new XMLElement with the given @p tag tag-name and
     *      @p text content.
     *
     * Create a new XMLElement with the given @p tag tag-name and @p text
     * content, but no attribute or child nodes set.  Also the new instance
     * isn't marked as root node.
     *
     * @param[in] tag  The tag name of this XML element.
     * @param[in] text  The text assigned to this XML element.
     */
    XMLElement(const std::string& tag, const std::string& text) :
        m_tag(tag),
        m_text(text),
        m_root(false)
    {}

    /** @brief  Creates a new XMLElement with the given @p tag tag-name and
     *      @p body a single child element.
     *
     * @param[in] tag  The tag name of this XML element.
     * @param[in] body  The child XML element assigned as body to this XML
     *      element.
     */
    XMLElement(const std::string& tag, const XMLElement& body) :
        m_tag(tag),
        m_children(std::vector<XMLElement>(1, body)), m_root(false)
    {}
    //@}

    /** @name  Accessors */ //@{
    /** @brief  Returns the the tag-name of this XMLElement.
     *
     * @return  The tag-name of this XMLElement.  Can be an empty string.
     */
    const std::string& Tag() const;

    /** @brief  Returns the text body of this XMLElement.
     *
     * @return  The text content of this XMLElement.  Can be an empty string.
     */
    const std::string& Text() const;

    /** @brief  Returns the number of child nodes inside this XMLElement.
     *
     * @return  The number of child XMLElement%s contained by this this
     *      XMLElement.  Is always positive.
     */
    int NumChildren() const;

    /** @brief  Returns the number of attributes attached to this XMLElement.
     *
     * @return  The number of attributes attached to this XMLElement.  Is always
     *      positive.
     */
    int NumAttributes() const;

    /** @brief  Returns if this XMLElement contains a child with @p tag as
     *      tag-name.
     *
     * @param[in] tag  The tag-name of the searched child XMLElement.
     * @return  True if there is at least one child with a @p tag tag-name,
     *      false if not.
     */
    bool ContainsChild(const std::string& tag) const;

    /** @brief  Returns if this XMLElement contains an attribute with the
     *      attribute key @p key.
     *
     * @param[in] key  The attribute key of the searched attribute.
     * @return  True if there is an attribute with a @p key, false if not.
     */
    bool ContainsAttribute(const std::string& key) const;

    /** @brief  Returns the index of the first child XMLElement with @p tag as
     *      tag-name or -1 if none found.
     *
     * @param[in] tag  The tag-name of the searched child XMLElement.
     * @return  The index of the first found child XMLElement with @p tag as
     *      tag-name or -1 of no such child XMLElement can be found.
     */
    int ChildIndex(const std::string& tag) const;

    /** @brief  Returns the XMLElement child at the @p index%ed position of the
     *      child list of this XMLElement.
     *
     * @param[in] index  The index of the child XMLElement requested. The passed
     *      value must be between 0 and NumChildren() - 1.
     * @return  A reference to the XMLElement child at the @p index%ed position
     *      of this XMLElement child list.
     * @throw XMLElement::NoSuchIndex  When @p index is out of range in this
     *      XMLElement child list.
     */
    const XMLElement& Child(unsigned int index) const;

    /** @brief  Returns the first XMLElement child that has @p tag as tag-name.
     *
     * @param[in] tag  The tag-name of the child XMLElement requested.
     * @return  A reference to the first XMLElement child which has the tag-name
     *      @p tag.
     * @throw XMLElement::NoSuchChild  When no child with a tag-name @p tag
     *      exists.
     */
    const XMLElement& Child(const std::string& tag) const;

    /** @brief  Returns the last XMLElement child of this XMLElement.
     *
     * @return  A reference to the last XMLElement child.
     * \throw XMLElement:NoSuchIndex  When there are no child XMLElement%s.
     */
    const XMLElement& LastChild() const;

    /** @brief  Returns the value of the attribute with name @p key or an empty
     *      string if there is no with attribute with this key.
     *
     * @param[in] key  The attribute key of the searched attribute.
     * @return  The value of the attribute with the @p key or an empty string if
     *      there is no such attribute.
     */
    const std::string& Attribute(const std::string& key) const;

    /** @brief  Write this XMLElement XML formatted into the given output
     *      stream @p os with indentation level @p indent when @p whitespace
     *      is set.
     *
     * @param [in] os  The output stream this document should be written to.
     * @param [in] indent  The indentation level this element should be
     *      indented.
     * @param [in] whitespace  If set to true the child XMLElement%s are
     *      indented and newline separated.
     * @return  The given @p os output stream.
     */
    std::ostream& WriteElement(std::ostream& os, int indent = 0, bool whitespace = true) const;

    /** @brief  Return this XMLElement XML formatted as string with indentation
     * level @p indent when @p whitespace is set.
     *
     * @param [in] indent  The indentation level this element should be
     *      indented.
     * @param [in] whitespace  If set to true the child XMLElement%s are
     *      indented and newline separated.
     * @return  A string containing the XML formatted representation of this
     *      XMLElement.
     */
    std::string WriteElement(int indent = 0, bool whitespace = true) const;

    /** @brief  Return an iterator to the first child in this XMLElement. */
    const_child_iterator child_begin() const;

    /** @brief  Return an iterator to the one beyond last child in this
     * XMLElement.
     */
    const_child_iterator child_end() const;

    /** @brief  Return an iterator to the first attribute in this XMLElement. */
    const_attr_iterator attr_begin() const;

    /** @brief  Return an iterator to the one beyond last attribute in this
     * XMLElement.
     */
    const_attr_iterator attr_end() const;
    //@}

    /** @name  Mutators */ //@{
    /** @copydoc XMLElement::Child(unsigned int) const */
    XMLElement& Child(unsigned int index);

    /** @copydoc XMLElement::Child(const std::string&) const */
    XMLElement& Child(const std::string& tag);

    /** @copydoc XMLElement::LastChild() const */
    XMLElement& LastChild();

    /** @brief  Sets the @p value of the attribute @p key in this XMLElement.
     *
     * When the attribute @p key already has a value set it will be overwritten
     * with the new @p value.
     *
     * @param[in] key  The key of the attribute to set.
     * @param[in] value  The new value the attribute should be set to.
     */
    void SetAttribute(const std::string& key, const std::string& value);

    /** @brief  Sets the tag-name of this XMLElement to @p tag.
     *
     * @param[in] tag  The new tag-name this XMLElement should have.
     */
    void SetTag(const std::string& tag);

    /** @brief  Sets the text content of this XMLEement to @p text.
     *
     * @param[in] text  The new text content this XMLElement should have.
     */
    void SetText(const std::string& text);

    /** @brief  Removes the attribute with @p key from this XMLElement.
     *
     * @param[in] key  The key of the attribute to be deleted.  If no such
     *      attribute exist no action is taken.
     */
    void RemoveAttribute(const std::string& key);

    /** @brief  Removes all attributes from this XMLElement. */
    void RemoveAttributes();

    /** @brief  Adds a given child XMLElement @p child to the end of the child
     *      list of this XMLElement.
     *
     * @param[in] child  The XMLElement to be appended to the child list.
     */
    void AppendChild(const XMLElement& child);

    /** @brief  Creates an new empty XMLElement with the tag-name @p tag and
     *      appends it to the child list of this XMLElement.
     *
     * @param[in] tag  The tag of the created XMLElement.
     */
    void AppendChild(const std::string& tag);

    /** @brief  Adds a given child XMLElement @p child before the child element
     *      at the @p index%ed position in the child list of this XMLElement.
     *
     * @param[in] child  The XMLElement to be inserted into the child list.
     * @param[in] index  The index of the child XMLElement where the @p child
     *      put be in front of. The passed value must be between 0 and
     *      NumChildren() - 1.
     * @throw XMLElement::NoSuchIndex  When @p index is out of range in this
     *      XMLElement child list.
     */
    void AddChildBefore(const XMLElement& child, unsigned int index);

    /** @brief  Removes the XMLElement child at the @p index%ed position of the
     *      child list of this XMLElement.
     *
     * @param[in] index  The index of the child XMLElement that should be
     *      removed.  The passed value must be between 0 and NumChildren() - 1.
     * @throw XMLElement::NoSuchIndex  When @p index is out of range in this
     *      XMLElement child list.
     */
    void RemoveChild(unsigned int index);

    /** @brief  Removes the XMLElement child with the tag-name @p tag from this
     *      XMLElement.
     *
     * @param[in] tag  The tag-name of the child XMLElement to be deleted.
     * @throw XMLElement::NoSuchChild  When no child with a tag-name @p tag
     *      exists.
     */
    void RemoveChild(const std::string& tag);

    /** @brief Removes all children from this XMLElement. */
    void RemoveChildren();

    /** @copydoc XMLElement::child_begin() const */
    child_iterator child_begin();

    /** @copydoc XMLElement::child_end() const */
    child_iterator child_end();

    /** @copydoc XMLElement::attr_begin() const */
    attr_iterator attr_begin();

    /** @copydoc XMLElement::attr_end() const */
    attr_iterator attr_end();
    //@}

private:
    /** @name  Structors */ //@{
    /** @brief  Creates a new XMLElement with the given @p tag tag-name and
     *      marked as root node, if @p root is set.
     *
     * @param[in] tag  The tag name of this XML element.
     * @param[in] root  When true this XMLElement should be interpreted as root
     *      node in a XMLElement tree.
     *
     * @note Called by friend XMLDoc.
     */
    XMLElement(const std::string& tag, bool root);
    //@}

    /** @brief  Stores the tag-name associated to this XMLElement.
     *
     * @bug  Currently this can contain an empty string but I doubt it will
     *      be useful as it will cause invalid XML documents serializations.
     */
    std::string m_tag;

    /** @brief  Stores the text content associated to this XMLElement. */
    std::string m_text;

    /** @brief  Stores the attributes associated to this XMLElement by key name
     *      mapping.
     */
    std::map<std::string, std::string> m_attributes;

    /** @brief  Stores a list of the child XMLElement%s associated to this
     *      XMLElement.
     *
     * This list can be empty when this XMLElement has no associated child
     * elements.
     */
    std::vector<XMLElement> m_children;

    /** @brief  Set to true if this XMLElement is the root element of an XMLDoc
     *      document.
     */
    bool m_root;

    friend class XMLDoc;
};

/** @brief  Represents a document formatted with XML markup.
 *
 * Each XMLDoc instance is assumed to represent a complete document.  It
 * contains a tree of nested XMLElement%s, starting from the always existing
 * root XMLElement node.
 */
class FO_COMMON_API XMLDoc
{
public:
    /** @name  Structors */ //@{
    /** @brief  Create an empty document with the given tag-name @p root_tag.
     *
     * @param[in] root_tag  The tag-name of the created root XMLElement.
     */
    XMLDoc(const std::string& root_tag = "XMLDoc");

    /** @brief  Construct a document from the given input stream @p is.
     *
     * @param[in] is  An input stream that provides an XML markup document when
     *      reading.
     *
     * @bug  @p is isn't actually read but ignored and an empty (and maybe
     *      invalid) document is created.  Use XMLDoc::ReadDoc(std::istream&)
     *      instead.
     */
    XMLDoc(const std::istream& is);
    //@}

    /** @name  Accessors */ //@{
    /** @brief  Write the contents of the XMLDoc into the given output stream
     *      @p os with optional @p indent.
     *
     * @param [in] os  The output stream this document should be written to.
     * @param [in] indent  If set to true the XML elements are indented and
     *      newline separated.
     * @return  The given @p os output stream.
     */
    std::ostream& WriteDoc(std::ostream& os, bool indent = true) const;
    //@}

    /** @name  Mutators */ //@{
    /** @brief  Clears the current content of this XMLDoc instance and read a
     *      new document from the given input stream @p is.
     *
     * @param[in] is  An input stream that provides an XML markup document when
     *      reading.
     * @return  The given @p is input stream.
     */
    std::istream& ReadDoc(std::istream& is);

    /** @brief  Clears the current content of this XMLDoc instance and read a
     *      new document from the given string @p s.
     *
     * @param[in] s  An string containing an XML markup document.
     */
    void ReadDoc(const std::string& s);
    //@}

    /** @brief  The root element that contains the parsed document, which is
     *      represented by XMLElement%s.
     */
    XMLElement root_node;

private:
    /** @brief  Creates the XML parsing rules at static initialization time. */
    struct RuleDefiner { RuleDefiner(); };
    static RuleDefiner s_rule_definer;

    /** @brief  Holds the XMLDoc to which the XML parser should add parsed
     *      elements.
     *
     * @todo  No need to hold a static instance here, pass the document into
     *      the parser as additional attribute.  This also avoids potential
     *      problems in multithreaded setups.
     */
    static XMLDoc* s_curr_parsing_doc;

    /** @brief  Holds the current environment for reading XMLElement%s (the
     *      current enclosing XMLElement%s). */
    static std::vector<XMLElement*> s_element_stack;

    static XMLElement s_temp_elem;
    static std::string s_temp_attr_name;

    // these are used along with the static members above during XML parsing
    static void SetElemName(const char* first, const char* last);
    static void SetAttributeName(const char* first, const char* last);
    static void AddAttribute(const char* first, const char* last);
    static void PushElem1(const char*, const char*);
    static void PushElem2(const char*, const char*);
    static void PopElem(const char*, const char*);
    static void AppendToText(const char* first, const char* last);
};

#endif // _XMLDoc_h_
