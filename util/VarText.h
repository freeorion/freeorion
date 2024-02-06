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

struct ScriptingContext;

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
    VarText() = default;

    //! Create a VarText instance from the given @p template_string.
    //!
    //! @param  template_string  @see #m_template_string.
    //! @param  stringtable_lookup  @see #m_stringtable_lookup_flag
    explicit VarText(std::string template_string, bool stringtable_lookup = true);

    //! Return the text generated after substituting all variables, with or without a ScriptingContext
    [[nodiscard]] const std::string& GetText(const ScriptingContext& context) const;
    [[nodiscard]] const std::string& GetText() const;

    //! Return if the text substitution was successful, with or without a ScriptingContext
    bool Validate(const ScriptingContext& context) const;
    bool Validate() const;

    //! Return the #m_template_string
    [[nodiscard]] const std::string& GetTemplateString() const noexcept { return m_template_string; }

    //! Return the #m_stringtable_lookup_flag
    [[nodiscard]] bool GetStringtableLookupFlag() const noexcept { return m_stringtable_lookup_flag; }

    //! Return the variables available for substitution.
    [[nodiscard]] std::vector<std::string_view> GetVariableTags() const;

    //! Set the #m_template_string to the given @p template_string.
    //!
    //! @param  template_string  @see #m_template_string.
    //! @param  stringtable_lookup  @see #m_stringtable_lookup_flag
    void SetTemplateString(std::string template_string, bool stringtable_lookup = true);

    //! Assign @p data to a given @p tag. Overwrites / replaces data of existing tags.
    //!
    //! The @p data should match @p tag as listed in
    //!   @ref variable_tags "Variable tags".
    //!
    //! @param  tag
    //!     Tag of the #m_variables set, may be labled.
    //! @param  data
    //!     Data value of the #m_variables set.
    void AddVariable(std::string tag, std::string data);
    void AddVariable(const char* tag, std::string data) { AddVariable(std::string{tag}, std::move(data)); }
    void AddVariable(std::string_view tag, std::string data) { AddVariable(std::string{tag}, std::move(data)); }

    //! Assign @p data as tags. Does not overwrite or replace data of existing tags.
    //!
    //! The @p data should match tags as listed in
    //!   @ref variable_tags "Variable tags".
    //!
    //! @param  data
    //!     Tag and Data values of the #m_variables set.
    void AddVariables(std::vector<std::pair<std::string, std::string>>&& data);

    //! @name  Variable tags
    //! @anchor variable_tags
    //!
    //! Tag strings that are recognized and replaced in VarText with the
    //! corresponding reference to an specific game entity.
    //!
    //! @{

    //! Variable value is a StringTable key.
    static constexpr std::string_view TEXT_TAG = "text";
    //! Variable value is a literal string.
    static constexpr std::string_view RAW_TEXT_TAG = "rawtext";
    //! Variable value is a Planet::ID().
    static constexpr std::string_view PLANET_ID_TAG = "planet";
    //! Variable value is a System::ID().
    static constexpr std::string_view SYSTEM_ID_TAG = "system";
    //! Variable value is a Ship::ID().
    static constexpr std::string_view SHIP_ID_TAG = "ship";
    //! Variable value is a Fleet::ID().
    static constexpr std::string_view FLEET_ID_TAG = "fleet";
    //! Variable value is a Building::ID().
    static constexpr std::string_view BUILDING_ID_TAG = "building";
    //! Variable value is a Field::ID().
    static constexpr std::string_view FIELD_ID_TAG = "field";
    //! Variable value represents a CombatLog.
    static constexpr std::string_view COMBAT_ID_TAG = "combat";
    //! Variable value is an Empire::EmpireID().
    static constexpr std::string_view EMPIRE_ID_TAG = "empire";
    //! Variable value is a ShipDesign::ID().
    static constexpr std::string_view DESIGN_ID_TAG = "shipdesign";
    //! Variable value is a ShipDesign::ID() of a predefined ShipDesign.
    static constexpr std::string_view PREDEFINED_DESIGN_TAG = "predefinedshipdesign";
    //! Variable value is a Tech::Name().
    static constexpr std::string_view TECH_TAG = "tech";
    //! Variable value is a Policy::Name().
    static constexpr std::string_view POLICY_TAG = "policy";
    //! Variable value is a BuildingType::Name().
    static constexpr std::string_view BUILDING_TYPE_TAG = "buildingtype";
    //! Variable value is a Special::Name().
    static constexpr std::string_view SPECIAL_TAG = "special";
    //! Variable value is a ShipHull::Name().
    static constexpr std::string_view SHIP_HULL_TAG = "shiphull";
    //! Variable value is a ShipPart::Name().
    static constexpr std::string_view SHIP_PART_TAG = "shippart";
    //! Variable value is a Species::Name().
    static constexpr std::string_view SPECIES_TAG = "species";
    //! Variable value is a FieldType::Name().
    static constexpr std::string_view FIELD_TYPE_TAG = "fieldtype";
    //! Variable value is a predefined MeterType string representation.
    static constexpr std::string_view METER_TYPE_TAG = "metertype";
    //! Variable value is the name of a registed ValueRef.
    static constexpr std::string_view FOCS_VALUE_TAG = "value";
    //! Variable value is Planet::ID(), gives environment of that planet (Good, Poor...)
    static constexpr std::string_view ENVIRONMENT_TAG = "environment";
    //! Variable value is a user_string (translation string from <lang>.txt)
    static constexpr std::string_view USER_STRING_TAG = "userstring";
    //! Variable value is Planet::ID(), gives type of that planet (Terran, Swamp...)
    static constexpr std::string_view PLANET_TYPE_TAG = "planettype";

    //! @}

protected:
    //! Combines the template with the variables contained in object to
    //! create a string with variables replaced with text. If a
    //! ScriptingContext is provided, then additional variables can be
    //! substituted.
    void GenerateVarText(const ScriptingContext* context) const;

    //! The template text used by this VarText, into which variables are
    //! substituted to render the text as user-readable.
    //!
    //! @see  #m_stringtable_lookup_flag
    std::string m_template_string; // need to hold own copy of this string to support deserialization

    //! If true the #m_template_string will be looked up in the stringtable
    //! prior to substitution for variables.
    bool m_stringtable_lookup_flag = false;

    //! Maps variable tags into values, which are used during text substitution.
    std::map<std::string, std::string, std::less<>> m_variables; // need to hold own copies of strings here to support deserialization

    //! #m_template_string with applied #m_variables substitute.
    mutable std::string m_text;

    //! True if the #m_template_string stubstitution was executed without
    //! errors.
    mutable bool m_validated = false;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template <typename Archive>
void VarText::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_template_string)
        & BOOST_SERIALIZATION_NVP(m_stringtable_lookup_flag)
        & BOOST_SERIALIZATION_NVP(m_variables);
}


#endif
