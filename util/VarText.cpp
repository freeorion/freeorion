#include "VarText.h"

#include "../universe/Universe.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"
#include "i18n.h"
#include "Logger.h"

#include <boost/xpressive/xpressive.hpp>

#include <map>

namespace xpr = boost::xpressive;

class Tech;
class BuildingType;
class Special;
class Species;
class FieldType;
const Tech*         GetTech(const std::string& name);
const BuildingType* GetBuildingType(const std::string& name);
const Special*      GetSpecial(const std::string& name);
const Species*      GetSpecies(const std::string& name);
const FieldType*    GetFieldType(const std::string& name);


namespace {
    //! Return @p content surrounded by the given @p tags.
    //!
    //! @param content
    //!     The text that should be wrapped into @p tags.
    //! @param tag
    //!     The tags that should be wrapping the given @p content.  The tag
    //!     shouldn't contain whitespace.
    //! @param data
    //!     Additional data assigned to the @p tag.
    //!
    //! @return
    //!     The tagged content.
    std::string WithTags(const std::string& content, const std::string& tag, const std::string& data) {
        std::string open_tag = "<" + tag + " " + data + ">";
        std::string close_tag = "</" + tag + ">";
        return open_tag + content + close_tag;
    }

    //! Function signature of tag substitution functions.
    //!
    //! @param data
    //!     Data values The signature of functions that generate substitution
    //!     strings for tags.
    typedef boost::optional<std::string> (*TagString)(const std::string& data);

    //! Get string substitute for a tag that is a universe object
    boost::optional<std::string> UniverseObjectString(const std::string& data, const std::string& tag) {
        int object_id = INVALID_OBJECT_ID;
        try {
            object_id = boost::lexical_cast<int>(data);
        } catch (const std::exception&) {
            return boost::none;
        }
        auto obj = GetUniverseObject(object_id);
        if (!obj)
            return boost::none;

        return WithTags(GetVisibleObjectName(obj), tag, data);
    }

    //! Returns substitution string for a predefined ship design tag
    boost::optional<std::string> PredefinedShipDesignString(const std::string& data) {
        const ShipDesign* design = GetPredefinedShipDesign(data);
        if (!design)
            return boost::none;

        return WithTags(design->Name(), VarText::PREDEFINED_DESIGN_TAG, data);
    }

    boost::optional<std::string> MeterTypeString(const std::string& data) {
        boost::optional<std::string> retval = boost::none;
        // validate data
        MeterType meter_type = INVALID_METER_TYPE;
        std::istringstream data_ss(data);
        data_ss >> meter_type;

        if (meter_type > INVALID_METER_TYPE && meter_type < NUM_METER_TYPES) {
            retval = boost::lexical_cast<std::string>(meter_type);
            if (UserStringExists(*retval))
                retval = WithTags(UserString(*retval), VarText::METER_TYPE_TAG, *retval);
        }

        return retval;
    }

    //! Returns substitution string for a ship design tag
    template <typename T,T* (*GetByID)(int)>
    boost::optional<std::string> IDString(const std::string& data, const std::string& tag) {
        int id{};
        try {
            id = boost::lexical_cast<int>(data);
        } catch (const std::exception&) {
            return boost::none;
        }
        T* object = GetByID(id);
        if (!object)
            return boost::none;

        return WithTags(object->Name(), tag, data);
    }

    //! Returns substitution string for an empire tag
    //! Interprets value of data as a name.
    //! Returns translation of name, if Get says
    //! that a thing by that name exists, otherwise boost::none.
    template <typename T,const T* (*GetByName)(const std::string&)>
    boost::optional<std::string> NameString(const std::string& data, const std::string& tag) {
        if (!GetByName(data))
            return boost::none;
        return WithTags(UserString(data), tag, data);
    }

    //! Global substitution map, wrapped in a function to avoid initialization order issues
    const std::map<std::string, TagString>& SubstitutionMap() {
        static std::map<std::string, TagString> substitute_map{
            {VarText::TEXT_TAG, [](const std::string& data) -> boost::optional<std::string>
                { return UserString(data); }},
            {VarText::RAW_TEXT_TAG, [](const std::string& data) -> boost::optional<std::string>
                { return data; }},
            {VarText::PLANET_ID_TAG, [](const std::string& data)
                { return UniverseObjectString(data, VarText::PLANET_ID_TAG); }},
            {VarText::SYSTEM_ID_TAG, [](const std::string& data)
                { return UniverseObjectString(data, VarText::SYSTEM_ID_TAG); }},
            {VarText::SHIP_ID_TAG, [](const std::string& data)
                { return UniverseObjectString(data, VarText::SHIP_ID_TAG); }},
            {VarText::FLEET_ID_TAG, [](const std::string& data)
                { return UniverseObjectString(data, VarText::FLEET_ID_TAG); }},
            {VarText::BUILDING_ID_TAG, [](const std::string& data)
                { return UniverseObjectString(data, VarText::BUILDING_ID_TAG); }},
            {VarText::FIELD_ID_TAG, [](const std::string& data)
                { return UniverseObjectString(data, VarText::FIELD_ID_TAG); }},
            {VarText::COMBAT_ID_TAG, [](const std::string& data) -> boost::optional<std::string>
                { return WithTags(UserString("COMBAT"), VarText::COMBAT_ID_TAG, data); }},
            {VarText::TECH_TAG, [](const std::string& data)
                { return NameString<Tech, GetTech>(data, VarText::TECH_TAG); }},
            {VarText::BUILDING_TYPE_TAG, [](const std::string& data)
                { return NameString<BuildingType, GetBuildingType>(data, VarText::BUILDING_TYPE_TAG); }},
            {VarText::SHIP_HULL_TAG, [](const std::string& data)
                { return NameString<HullType, GetHullType>(data, VarText::SHIP_HULL_TAG); }},
            {VarText::SHIP_PART_TAG, [](const std::string& data)
                { return NameString<PartType, GetPartType>(data, VarText::SHIP_PART_TAG); }},
            {VarText::SPECIAL_TAG, [](const std::string& data)
                { return NameString<Special, GetSpecial>(data, VarText::SPECIAL_TAG); }},
            {VarText::SPECIES_TAG, [](const std::string& data)
                { return NameString<Species, GetSpecies>(data, VarText::SPECIES_TAG); }},
            {VarText::FIELD_TYPE_TAG, [](const std::string& data)
                { return NameString<FieldType, GetFieldType>(data, VarText::FIELD_TYPE_TAG); }},
            {VarText::METER_TYPE_TAG, MeterTypeString},
            {VarText::DESIGN_ID_TAG, [](const std::string& data)
                { return IDString<const ShipDesign, GetShipDesign>(data, VarText::DESIGN_ID_TAG); }},
            {VarText::PREDEFINED_DESIGN_TAG, PredefinedShipDesignString},
            {VarText::EMPIRE_ID_TAG, [](const std::string& data)
                { return IDString<Empire, GetEmpire>(data, VarText::EMPIRE_ID_TAG); }},
        };

        return substitute_map;
    }


    //! Looks up the given match in the Universe and returns the Universe
    //! entities value.
    struct Substitute {
        Substitute(const std::map<std::string, std::string>& variables,
                   bool& valid) :
            m_variables(variables),
            m_valid(valid)
        {}

        std::string operator()(xpr::smatch const& match) const {
            // Labelled variables have the form %tag:label%,  unlabelled are just %tag%
            std::string tag = match[1];
            // Use the label value. When missing, use the tag submatch as label instead.
            std::string label = match[match[2].matched ? 2 : 1];

            // look up child
            auto elem = m_variables.find(label);
            if (m_variables.end() == elem) {
                ErrorLogger() << "Substitute::operator(): No value found for label: " << label << " from token: " << match.str();
                m_valid = false;
                return UserString("ERROR");
            }

            auto substituter = SubstitutionMap().find(tag);
            if (substituter != SubstitutionMap().end()) {
                if (auto substitution = substituter->second(elem->second)) {
                    return *substitution;
                }
            }

            ErrorLogger() << "Substitute::operator(): No substitution executed for tag: " << tag << " from token: " << match.str();
            m_valid = false;
            return UserString("ERROR");
        }

        const std::map<std::string, std::string>& m_variables;
        bool& m_valid;
    };
}


const std::string VarText::TEXT_TAG = "text";
const std::string VarText::RAW_TEXT_TAG = "rawtext";

const std::string VarText::PLANET_ID_TAG = "planet";
const std::string VarText::SYSTEM_ID_TAG = "system";
const std::string VarText::SHIP_ID_TAG = "ship";
const std::string VarText::FLEET_ID_TAG = "fleet";
const std::string VarText::BUILDING_ID_TAG = "building";
const std::string VarText::FIELD_ID_TAG = "field";

const std::string VarText::COMBAT_ID_TAG = "combat";

const std::string VarText::EMPIRE_ID_TAG = "empire";
const std::string VarText::DESIGN_ID_TAG = "shipdesign";
const std::string VarText::PREDEFINED_DESIGN_TAG = "predefinedshipdesign";

const std::string VarText::TECH_TAG = "tech";
const std::string VarText::BUILDING_TYPE_TAG = "buildingtype";
const std::string VarText::SPECIAL_TAG = "special";
const std::string VarText::SHIP_HULL_TAG = "shiphull";
const std::string VarText::SHIP_PART_TAG = "shippart";
const std::string VarText::SPECIES_TAG = "species";
const std::string VarText::FIELD_TYPE_TAG = "fieldtype";
const std::string VarText::METER_TYPE_TAG = "metertype";


VarText::VarText() :
    m_stringtable_lookup_flag(false)
{}

VarText::VarText(const std::string& template_string, bool stringtable_lookup/* = true*/) :
    m_template_string(template_string),
    m_stringtable_lookup_flag(stringtable_lookup)
{}

const std::string& VarText::GetText() const {
    if (m_text.empty())
        GenerateVarText();
    return m_text;
}

bool VarText::Validate() const {
    if (m_text.empty())
        GenerateVarText();
    return m_validated;
}

void VarText::SetTemplateString(const std::string& template_string, bool stringtable_lookup/* = true*/) {
    m_template_string = template_string;
    m_stringtable_lookup_flag = stringtable_lookup;
}

std::vector<std::string> VarText::GetVariableTags() const {
    std::vector<std::string> retval;
    for (const auto& variable : m_variables)
        retval.push_back(variable.first);
    return retval;
}

void VarText::AddVariable(const std::string& tag, const std::string& data)
{ m_variables[tag] = data; }

void VarText::GenerateVarText() const {
    // generate a string complete with substituted variables and hyperlinks
    // the procedure here is to replace any tokens within %% with variables of
    // the same name in the SitRep XML data
    m_text.clear();
    m_validated = true;
    if (m_template_string.empty())
        return;

    // get string into which to substitute variables
    std::string template_str = m_stringtable_lookup_flag ? UserString(m_template_string) : m_template_string;

    xpr::sregex var = '%' >> (xpr::s1 = -+xpr::_w) >> !(':' >> (xpr::s2 = -+xpr::_w)) >> '%';
    m_text = xpr::regex_replace(template_str, var, Substitute(m_variables, m_validated));
}
