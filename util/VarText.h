// -*- C++ -*-
#ifndef _VarText_h_
#define _VarText_h_


#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <sstream>

/**
*   VarText is a string tagged with variable names which are substituded for actual data at runtime
*   The variables are stored in a XML element tree.
*   The actual string to substitute using the defined variables is not a member, rather it's
*   sent into a static function. In this way, UI can use strings provided by a string table when needed.
*
*   The format for the VarText template string is as follows:
*   use % as delimiters for the variable. For example: The planet %planet% has been conlonized will look
*   through the VarText data for the xml element with the tag name of planet and substitute it's name in
*   the game universe
*
*   An example of VarText implementation are SitReps. They are created by the server which knows nothing about
*   what the final string will look like. ClientUI.cpp ultimately generates a string form the variables
*/

class VarText
{
public:
    VarText( ) { };
    VarText(const XMLElement& elem);

    XMLElement XMLEncode() const;

    /** combines the given template with the varaibles ontained in object to create a string with live variables replaced with text
        will produce exceptions if invalid variables are found ( no not exist in XML data or in universe ) */
    void GenerateVarText( const std::string& template_str );

    XMLElement& GetVariables( )   { return m_variables; }
    std::string& GetText( )           { return m_text; }
    void SetText( std::string &text ) { m_text = text; }

    static const std::string START_VAR;
    static const std::string END_VAR;
    static const std::string PLANET_ID_TAG;
    static const std::string SYSTEM_ID_TAG;
    static const std::string TECH_ID_TAG;
    static const std::string SHIP_ID_TAG;
    static const std::string BUILDING_ID_TAG;

protected:
    XMLElement  m_variables; ///< the data describing the sitrep. See class comments for description
    std::string m_text;      ///< the text, including hyperlinks, that describes this entry. Built from XML data

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void VarText::serialize(Archive& ar, const unsigned int version)
{
    std::string variables;
    if (Archive::is_saving::value) {
        XMLDoc doc;
        doc.root_node = m_variables;
        std::stringstream stream;
        doc.WriteDoc(stream);
    }
    ar  & BOOST_SERIALIZATION_NVP(m_text)
        & BOOST_SERIALIZATION_NVP(variables);
    if (Archive::is_loading::value) {
        XMLDoc doc;
        std::stringstream stream;
        stream << variables;
        doc.ReadDoc(stream);
        m_variables = doc.root_node;
    }
}

inline std::string VarTextRevision()
{return "$Id$";}

#endif // _VarText_h_
