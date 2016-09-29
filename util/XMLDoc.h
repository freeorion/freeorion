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

/** \file
 *
 * Contains free functions and classes to modify, read and write XML files.
 */

/** encapsulates an XML element (from a <> tag to a </> tag).  XMLElement is a simplified XML element, 
    consisting only of a tag, a single text string, attributes and child elements.  It is designed to represent 
    C++ objects to allow them to easily be saved to/loaded from disk and serialized to be sent over a network 
    connection. It is *not* designed to represent documents. So this:
    \verbatim
      <burns>Say <quote>Goodnight</quote> Gracie.</burns> 
    \endverbatim
    may not work as you might think. You will end up with both the "Say " and the " Gracie." together.  
    So burns.Text() == "Say  Gracie.". If, however, you wanted to represent an object bar of class foo:
    \verbatim
      class foo
      {
         int ref_ct;
         double data;
      };
    \endverbatim
    you might do something like this: 
    \verbatim
      <bar>
         <foo>
            <ref_ct>13<ref_ct/>
            <data>0.364951<data/>
         </foo>
      </bar>
    \endverbatim
    Further, while that example is standard XML, an XMLElement optionally accepts its single text string in quotes, 
    and strips off trailing whitespace, in direct contrary to the XML standard.  
    So "burns" from above is equivalent to:
    \verbatim
      <burns>"Say  Gracie."<quote>Goodnight</quote></burns> 
    \endverbatim
    or:
    \verbatim
      <burns>Say  Gracie.<quote>Goodnight</quote></burns> 
    \endverbatim
    or:
    \verbatim
      <burns>"Say  Gracie."
         <quote>Goodnight</quote>
      </burns> 
    \endverbatim
    or:
    \verbatim
      <burns>Say  Gracie.
         <quote>Goodnight</quote>
      </burns> 
    \endverbatim
    Each of these yields a burns.Text() of "Say  Gracie.".  When an XMLElement is saved, its text is saved within a CDATA section.
    Any string can be put inside one of these quoted text fields, even text that includes an arbitrary number of quotes.  So you 
    can assign any std::string or c-string to an element.  However, when hand-editing an XML file containing such text strings, you
    need to be a bit careful.  The closing quote must be the last thing other than whitespace.  Adding 
    more than one quoted text string to the XML element, with each string separated by other elements, will result in a 
    single concatenated string, as illustrated above.
    This is not the most time- or space-efficient way to organize object data, but it may just be one of the simplest 
    and most easily read. */
class FO_COMMON_API XMLElement
{
public:
    typedef std::vector<XMLElement>::iterator                  child_iterator;
    typedef std::vector<XMLElement>::const_iterator            const_child_iterator;
    typedef std::map<std::string, std::string>::iterator       attr_iterator;
    typedef std::map<std::string, std::string>::const_iterator const_attr_iterator;

    /** \name Structors */ //@{
    XMLElement() : m_root(false) {} ///< default ctor
    XMLElement(const std::string& tag) : m_tag(tag), m_text(""), m_root(false) {}  ///< ctor that constructs an XMLElement with a tag-name \a tag
    XMLElement(const std::string& tag, const std::string& text) : m_tag(tag), m_text(text), m_root(false) {}  ///< ctor that constructs an XMLElement with a tag-name \a tag and text \a text
    XMLElement(const std::string& tag, const XMLElement& body) : m_tag(tag), m_children(std::vector<XMLElement>(1, body)), m_root(false) {}  ///< ctor that constructs an XMLElement with a tag-name \a tag and a single child \a body
    //@}

    /** \name Accessors */ //@{
    const std::string& Tag() const;                          ///< returns the tag-name of the XMLElement
    const std::string& Text() const;                         ///< returns the text of this XMLElement
    int NumChildren() const;                                 ///< returns the number of children in the XMLElement
    int NumAttributes() const;                               ///< returns the number of attributes in the XMLElement
    bool ContainsChild(const std::string& child) const;      ///< returns true if the element contains a child called \a name
    bool ContainsAttribute(const std::string& attrib) const; ///< returns true if the element contains an attribute called \a name
    int  ChildIndex(const std::string& child) const;         ///< returns the index of the child called \a name, or -1 if not found

    /**  returns the child in the \a idx-th position of the child list of the XMLElement.  \throw
         XMLElement::NoSuchIndex An out of range index will cause an exception. */
    const XMLElement& Child(unsigned int idx) const;

    /**  returns the child in child list of the XMLElement that has the tag-name \a str.  \throw
         XMLElement::NoSuchChild An exception is thrown if no child named \a child exists. */   
    const XMLElement& Child(const std::string& child) const;

    /**  returns the last child in child list of the XMLElement.  \throw XMLElement:NoSuchIndex Calling this on an
         empty element will cause an exception. */   
    const XMLElement& LastChild() const;

    /** returns the value of the attribute with name \a key, or "" if no such named attribute is found */
    const std::string& Attribute(const std::string& attrib) const;

    /** writes the XMLElement to an output stream; returns the stream */
    std::ostream& WriteElement(std::ostream& os, int indent = 0, bool whitespace = true) const;
    std::string WriteElement(int indent = 0, bool whitespace = true) const;

    const_child_iterator child_begin() const; ///< const_iterator to the first child in the XMLElement
    const_child_iterator child_end() const;   ///< const_iterator to the last + 1 child in the XMLElement
    const_attr_iterator  attr_begin() const;  ///< const_iterator to the first attribute in the XMLElement
    const_attr_iterator  attr_end() const;    ///< const_iterator to the last + 1 attribute in the XMLElement
    //@}

    /** \name Mutators */ //@{
    /**  returns the child in the \a idx-th position of the child list of the XMLElement.  \throw
         XMLElement:NoSuchIndex An out of range index will cause an exception. */
    XMLElement& Child(unsigned int idx);

    /**  returns the child in child list of the XMLElement that has the tag-name \a child.  \throw
         XMLElement::NoSuchChild An exception is thrown if no child named \a child exists. */   
    XMLElement& Child(const std::string& child);

    /**  returns the last child in child list of the XMLElement.  \throw XMLElement:NoSuchIndex Calling this on an
         empty element will cause an exception. */   
    XMLElement& LastChild();

    /** sets (and possibly overwrites) an attribute \a attrib, with the value \a val. */
    void SetAttribute(const std::string& attrib, const std::string& val);

    /** sets the tag to \a tag */
    void SetTag(const std::string& tag);

    /** sets the text to \a text */
    void SetText(const std::string& text);

    /** removes attribute \a attrib from the XMLElement*/
    void RemoveAttribute(const std::string& attrib);

    /** removes all attributes from the XMLElement*/
    void RemoveAttributes();

    /** adds child XMLElement \a e to the end of the child list of the XMLElement */
    void AppendChild(const XMLElement& e);

    /** creates an empty XMLElement with tag-name \a child, and adds it to the end of the child list of the
        XMLElement */
    void AppendChild(const std::string& child);

    /** adds a child \a e in the \a idx-th position of the child list of the XMLElement. \throw
         XMLElement::NoSuchIndex An out of range index will cause an exception. */
    void AddChildBefore(const XMLElement& e, unsigned int idx);

    /** removes the child in the \a idx-th position of the child list of the XMLElement.  \throw
         XMLElement::NoSuchIndex An out of range index will cause an exception. */
    void RemoveChild(unsigned int idx);

    /** removes the child called \a child from the XMLElement.  \throw XMLElement:NoSuchChild An exception is thrown
        if no child named \a child exists. */
    void RemoveChild(const std::string& child);

    /** removes all children from the XMLElement*/
    void RemoveChildren();

    child_iterator child_begin();     ///< iterator to the first child in the XMLElement
    child_iterator child_end();       ///< iterator to the last + 1 child in the XMLElement
    attr_iterator  attr_begin();   ///< iterator to the first attribute in the XMLElement
    attr_iterator  attr_end();     ///< iterator to the last + 1 attribute in the XMLElement
    //@}

    /** \name Exceptions */ //@{
    /** The base class for XMLElement exceptions. */
    class Exception : public GG::ExceptionBase
    {
    public:
        Exception () throw() : ExceptionBase() {}
        Exception (const std::string& msg) throw() : ExceptionBase(msg) {}
        virtual const char* type() const throw() = 0;
    };

    /** Thrown when a request for a named child element cannot be fulfilled. */
    class NoSuchChild : public Exception
    {
    public:
        NoSuchChild () throw() : Exception () {}
        NoSuchChild (const std::string& msg) throw() : Exception (msg) {}
        virtual const char* type() const throw()
            {return "XMLElement::NoSuchChild" ;}
    };

    /** Thrown when a request for an indexed child element cannot be fulfilled. */
    class NoSuchIndex : public Exception
    {
    public:
        NoSuchIndex () throw() : Exception () {}
        NoSuchIndex (const std::string& msg) throw() : Exception (msg) {}
        virtual const char* type() const throw()
            {return "XMLElement::NoSuchIndex" ;}
    };
    //@}

private:
    /** ctor that constructs an XMLElement from a tag-name \a t and a bool \a r indicating whether it is the root
        XMLElement in an XMLDoc document*/
    XMLElement(const std::string& t, bool r);

    std::string                        m_tag;        ///< the tag-name of the XMLElement
    std::string                        m_text;       ///< the text of this XMLElement
    std::map<std::string, std::string> m_attributes; ///< the attributes of the XMLElement, stored as key-value pairs
    std::vector<XMLElement>            m_children;   ///< the XMLElement children of this XMLElement

    bool                               m_root;       ///< true if this XMLElement is the root element of an XMLDoc document

    /** allows XMLDoc to create root XMLElements and call the non-const overload of LastChild() */
    friend class XMLDoc;
};

/** \deprecated All the GG XML classes are deprecated and will be removed upon the next major release.
    encapsulates an entire XML document.  Each XMLDoc is assumed to take up an entire file, and to contain an arbitrary
    number of XMLElements within its root_node member. */
class FO_COMMON_API XMLDoc
{
public:
    /** \name Structors */ //@{
    /** ctor that constructs an empty XML document with a root node with tag-name \a root_tag */
    XMLDoc(const std::string& root_tag = "XMLDoc");

    /** ctor that constructs an XML document from an input stream \a is */
    XMLDoc(const std::istream& is);
    //@}

    /** \name Accessors */ //@{
    /** writes the XMLDoc to an output stream; returns the stream.  If \a whitespace is false, the document is written
        without whitespace (one long line with no spaces between XMLElements).  Otherwise, the document is formatted in
        a more standard human-readable form. */
    std::ostream& WriteDoc(std::ostream& os, bool whitespace = true) const;
    //@}

    /** \name Mutators */ //@{
    /** destroys the current contents of the XMLDoc, and replaces them with the contents of the document in the input
        stream \a is; returns the stream*/
    std::istream& ReadDoc(std::istream& is);

    /** destroys the current contents of the XMLDoc, and replaces them with the contents of the document in the input
        string \a s */
    void ReadDoc(const std::string& s);
    //@}

    XMLElement root_node;  ///< the single XMLElement in the document, under which all other XMLElement are children

private:
    struct RuleDefiner {RuleDefiner();};  ///< used to create XML parsing rules at static initialization time
    static RuleDefiner s_rule_definer;

    /** holds the XMLDoc to which the code should add elements */
    static XMLDoc* s_curr_parsing_doc;

    /** maintains the current environment for reading XMLElements (the current enclosing XMLElement) */
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

