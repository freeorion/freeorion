#ifndef _VarText_h_
#define _VarText_h_

#include <map>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "Export.h"

/** VarText is a string tagged with variable names which are substituded with
  * actual data at runtime. The variables are stored in a XML element tree.
  * 
  * The format for the VarText template string is as follows:
  * use % as delimiters for the variable. For example: The planet %planet% has
  * been conlonized will look through the VarText data for the xml element with
  * the tag name of "planet" and substitute the name of the planet in the game
  * universe with the appropriate ID number that is stored in that VarText
  * entry.
  * 
  * If you need to use more than one item of the same type, you must label
  * the tags. For example: %planet:my_planet% crashed into %planet:other_planet%.
  * That will look for the xml elements "my_planet" and "other_planet" and
  * do the planet lookup for them.
  *
  * An example of VarText implementation are SitReps. They are created by the
  * server which knows nothing about what the final string will look like.
  * ClientUI.cpp ultimately generates a string from the variables. 
  */
class FO_COMMON_API VarText {
public:
    /** \name Structors */ //@{
    VarText();
    explicit VarText(const std::string& template_string, bool stringtable_lookup_template = true);
    //@}

    /** \name Accessors */ //@{
    const std::string&          GetText() const;        //!< Returns text generated after substituting all variables.
    bool                        Validate() const;       //!< Does text generation succeed without any errors occurring?
    const std::string&          GetTemplateString() const   { return m_template_string; }
    bool                        GetStringtableLookupFlag() const { return m_stringtable_lookup_flag; }
    std::vector<std::string>    GetVariableTags() const;//!< returns a list of tags for the variables this vartext has
    //@}

    /** \name Mutators */ //@{
    void                        SetTemplateString(const std::string& text, bool stringtable_lookup_template = true);
    void                        AddVariable(const std::string& tag, const std::string& data);
    //@}

    /** Tag strings that are recognized and replaced in VarText. */
    static const std::string TEXT_TAG;      // stringtable lookup
    static const std::string RAW_TEXT_TAG;  // no stringtable lookup

    static const std::string PLANET_ID_TAG;
    static const std::string SYSTEM_ID_TAG;
    static const std::string SHIP_ID_TAG;
    static const std::string FLEET_ID_TAG;
    static const std::string BUILDING_ID_TAG;
    static const std::string FIELD_ID_TAG;

    static const std::string COMBAT_ID_TAG;

    static const std::string EMPIRE_ID_TAG;
    static const std::string DESIGN_ID_TAG;
    static const std::string PREDEFINED_DESIGN_TAG;

    static const std::string TECH_TAG;
    static const std::string BUILDING_TYPE_TAG;
    static const std::string SPECIAL_TAG;
    static const std::string SHIP_HULL_TAG;
    static const std::string SHIP_PART_TAG;
    static const std::string SPECIES_TAG;
    static const std::string FIELD_TYPE_TAG;
    static const std::string METER_TYPE_TAG;

protected:
    /** Combines the template with the variables contained in object to
      * create a string with variables replaced with text. */
    void                GenerateVarText() const;

    std::string         m_template_string;          ///< the template string for this VarText, into which variables are substituted to render the text as user-readable
    bool                m_stringtable_lookup_flag;  ///< should the template string be looked up in the stringtable prior to substitution for variables?
    std::map<std::string, std::string> m_variables; ///< the data about variables to be substitued into the template string to render the VarText
    mutable std::string m_text;                     ///< the user-readable rendered text with substitutions made
    mutable bool        m_validated = false;        ///< did vartext generation succeed without problems?

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void VarText::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_template_string)
        & BOOST_SERIALIZATION_NVP(m_stringtable_lookup_flag)
        & BOOST_SERIALIZATION_NVP(m_variables);
}

#endif // _VarText_h_
