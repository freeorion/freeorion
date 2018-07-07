#ifndef _VarText_h_
#define _VarText_h_

//! @file
//!     Declares the VarText class.

#include <map>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "Export.h"

//! Provides a lazy evaluated template string with named variable tags.
//!
//! VarText is a template string tagged with variable names which are
//! substituted with actual data when when calling VarText::GetText().
//!
//! The format for VarText template string consists of named variable tags
//! enclosed by percent signs (%).  Depending on the parameter tag used the
//! value assigned to a named parameter will be evaluated with the known name of
//! the object.  When using multiple named variable tags it is possible to
//! separate the tag with an double colon (:) from an identifing label.  The
//! label should be limited to lower case latin characters and the underscore.
//!
//! See @ref variable_tags "Variable Tags" for a list of supported variable
//! tags.
//!
//!
//! For example:  When assigning template string:
//!
//!     On %planet%: the %building% has been produced.
//!
//! to a VarText instance it declares the two named variable tags @e %%planet%
//! and @e %%building%.
//!
//! By calling VarText::AddVariable() identifiers can be assigned to those named
//! variable tags.  For example:
//!
//! ```{.cpp}
//!     // the_capital_planet.VisibleName() == "Garmillas II"
//!     var_text.AddVariable("planet", the_capital_planet.ID());
//!     // the_imperial_palace.VisibleName() == "Imperial Palace"
//!     var_text.AddVariable("building", the_imperial_palace.ID());
//! ```
//!
//! Now when calling VarText::GetText() the references are replaced with the
//! actual object name (assuming the resolver can see all the objects):
//!
//!     On Garmillas II: the Imperial Palace has been produced.
//!
//! In case there are multiple named variable tags of the same type it is
//! possible to add labels to distinguish those.  For example the template
//! string:
//!
//!     In %system%: "%fleet:defender%" was overrun by "%fleet:attacker%"
//!
//! where the name variable tags are set to:
//!
//! ```{.cpp}
//!   // the_battleground.VisibleName() == "Balun"
//!   var_text.AddVariable("system", the_battleground.ID());
//!   // the_defender_fleet.VisibleName() == "Great Garmillas Armada"
//!   var_text.AddVariable("fleet:defender", the_defender_fleet.ID());
//!   // the_attacker_fleet.VisibleName() == "UNCN Special Ops fleet"
//!   var_text.AddVariable("fleet:attacker", the_attacker_fleet.ID());
//! ```
//!
//! would resolve into:
//!
//!     In Balun: "Geat Garmillas Armada" was overrun by "UNCN Special Ops fleet"
class FO_COMMON_API VarText {
public:
    //! Create a VarText instance with an empty #m_template_string.
    VarText();

    //! Create a VarText instance from the given @p template_string.
    //!
    //! @param  template_string  @see #m_template_string.
    //! @param  stringtable_lookup  @see #m_stringtable_lookup_flag
    explicit VarText(const std::string& template_string, bool stringtable_lookup = true);

    //! Return the text generated after substituting all variables.
    const std::string& GetText() const;

    //! Return if the text substitution was successful.
    bool Validate() const;

    //! Return the #m_template_string
    const std::string& GetTemplateString() const
    { return m_template_string; }

    //! Return the #m_stringtable_lookup_flag
    bool GetStringtableLookupFlag() const
    { return m_stringtable_lookup_flag; }

    //! Return the variables available for substitution.
    std::vector<std::string> GetVariableTags() const;

    //! Set the #m_template_string to the given @p template_string.
    //!
    //! @param  template_string  @see #m_template_string.
    //! @param  stringtable_lookup  @see #m_stringtable_lookup_flag
    void SetTemplateString(const std::string& template_string, bool stringtable_lookup = true);

    //! Assign @p data to a given @p tag.
    //!
    //! The @p data should match @p tag as listed in
    //!   @ref variable_tags "Variable tags".
    //!
    //! @param  tag
    //!     Tag of the #m_variables set, may be labled.
    //! @param  data
    //!     Data value of the #m_variables set.
    void AddVariable(const std::string& tag, const std::string& data);

    //! @name  Variable tags
    //! @anchor variable_tags
    //!
    //! Tag strings that are recognized and replaced in VarText with the
    //! corresponding reference to an specific game entity.
    //!
    //! @{

    //! Variable value is a StringTable key.
    static const std::string TEXT_TAG;
    //! Variable value is a literal string.
    static const std::string RAW_TEXT_TAG;
    //! Variable value is a Planet::ID().
    static const std::string PLANET_ID_TAG;
    //! Variable value is a System::ID().
    static const std::string SYSTEM_ID_TAG;
    //! Variable value is a Ship::ID().
    static const std::string SHIP_ID_TAG;
    //! Variable value is a Fleet::ID().
    static const std::string FLEET_ID_TAG;
    //! Variable value is a Building::ID().
    static const std::string BUILDING_ID_TAG;
    //! Variable value is a Field::ID().
    static const std::string FIELD_ID_TAG;
    //! Variable value represents a CombatLog.
    static const std::string COMBAT_ID_TAG;
    //! Variable value is an Empire::EmpireID().
    static const std::string EMPIRE_ID_TAG;
    //! Variable value is a ShipDesign::ID().
    static const std::string DESIGN_ID_TAG;
    //! Variable value is a ShipDesign::ID() of a predefined ShipDesign.
    static const std::string PREDEFINED_DESIGN_TAG;
    //! Variable value is a Tech::Name().
    static const std::string TECH_TAG;
    //! Variable value is a BuildingType::Name().
    static const std::string BUILDING_TYPE_TAG;
    //! Variable value is a Special::Name().
    static const std::string SPECIAL_TAG;
    //! Variable value is a HullType::Name().
    static const std::string SHIP_HULL_TAG;
    //! Variable value is a PartType::Name().
    static const std::string SHIP_PART_TAG;
    //! Variable value is a Species::Name().
    static const std::string SPECIES_TAG;
    //! Variable value is a FieldType::Name().
    static const std::string FIELD_TYPE_TAG;
    //! Variable value is a predefined MeterType string representation.
    static const std::string METER_TYPE_TAG;

    //! @}

protected:
    //! Combines the template with the variables contained in object to
    //! create a string with variables replaced with text.
    void GenerateVarText() const;

    //! The template text used by this VarText, into which variables are
    //! substituted to render the text as user-readable.
    //!
    //! @see  #m_stringtable_lookup_flag
    std::string m_template_string;

    //! If true the #m_template_string will be looked up in the stringtable
    //! prior to substitution for variables.
    bool m_stringtable_lookup_flag;

    //! Maps variable tags into values, which are used during text substitution.
    std::map<std::string, std::string> m_variables;

    //! #m_template_string with applied #m_variables substitute.
    mutable std::string m_text;

    //! True if the #m_template_string stubstitution was executed without
    //! errors.
    mutable bool m_validated = false;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template <class Archive>
void VarText::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_template_string)
        & BOOST_SERIALIZATION_NVP(m_stringtable_lookup_flag)
        & BOOST_SERIALIZATION_NVP(m_variables);
}

#endif // _VarText_h_
