#include "VarText.h"

#include "../universe/Universe.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Enums.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "i18n.h"
#include "Logger.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/static_assert.hpp>
#include <boost/spirit/include/classic.hpp>

#include <map>

FO_COMMON_API extern const int INVALID_DESIGN_ID;

// Forward declarations
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
    const std::string START_VAR("%");
    const std::string END_VAR("%");
    const std::string LABEL_SEPARATOR(":");

    //////////////////////////////////////////
    ///// Tag substitution generators////////
    ////////////////////////////////////////

    /// Surround content with approprite tags based on tag_of
    std::string WithTags(const std::string& content, const std::string& tag, const std::string& data) {
        std::string open_tag = "<" + tag + " " + data + ">";
        std::string close_tag = "</" + tag + ">";
        return open_tag + content + close_tag;
    }

    /// The signature of functions that generate substitution strings for
    /// tags.
    typedef std::string (*TagString)(const std::string& data, const std::string& tag, bool& valid);

    /// Get string substitute for a translated text tag
    std::string TextString(const std::string& data, const std::string& tag, bool& valid) {
        return UserString(data);
    }

    /// Get string substitute for a raw text tag
    std::string RawTextString(const std::string& data, const std::string& tag, bool& valid) {
        return data;
    }

    ///Get string substitute for a tag that is a universe object
    std::string UniverseObjectString(const std::string& data, const std::string& tag, bool& valid) {
        int object_id = INVALID_OBJECT_ID;
        try {
            object_id = boost::lexical_cast<int>(data);
        } catch (const std::exception&) {
            ErrorLogger() << "UniverseObjectString couldn't cast \"" << data << "\" to int for object ID.";
            valid = false;
            return UserString("ERROR");
        }
        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            //ErrorLogger() << "UniverseObjectString couldn't get object with ID " << object_id;
            valid = false;
            return UserString("ERROR");
        }

        return WithTags(GetVisibleObjectName(obj), tag, data);
    }

    /// combat links always just labelled "Combat"; don't need to look up details
    std::string CombatLogString(const std::string& data, const std::string& tag, bool& valid)
    { return WithTags(UserString("COMBAT"), tag, data); }

    /// Returns substitution string for a ship design tag
    std::string ShipDesignString(const std::string& data, const std::string& tag, bool& valid) {
        int design_id = INVALID_DESIGN_ID;
        try {
            design_id = boost::lexical_cast<int>(data);
        } catch (const std::exception&) {
            ErrorLogger() << "SubstituteAndAppend couldn't cast \"" << data << "\" to int for ship design ID.";
            valid = false;
            return UserString("ERROR");
        }
        const ShipDesign* design = GetShipDesign(design_id);
        if (!design) {
            ErrorLogger() << "SubstituteAndAppend couldn't get ship design with ID " << design_id;
            valid = false;
            return UserString("ERROR");
        }
        return WithTags(design->Name(), tag, data);
    }

    /// Returns substitution string for a predefined ship design tag
    std::string PredefinedShipDesignString(const std::string& data, const std::string& tag, bool& valid) {
        const ShipDesign* design = GetPredefinedShipDesign(data);
        if (!design) {
            ErrorLogger() << "SubstituteAndAppend couldn't get predefined ship design with name " << data;
            valid = false;
            return UserString("ERROR");
        }
        return WithTags(design->Name(), tag, data);
    }

    /// Returns substitution string for an empire tag
    std::string EmpireString(const std::string& data, const std::string& tag, bool& valid) {
        int empire_id = ALL_EMPIRES;
        try {
            empire_id = boost::lexical_cast<int>(data);
        } catch (const std::exception&) {
            ErrorLogger() << "SubstituteAndAppend couldn't cast \"" << data << "\" to int for empire ID.";
            valid = false;
            return UserString("ERROR");
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "SubstituteAndAppend couldn't get empire with ID " << empire_id;
            valid = false;
            return UserString("ERROR");
        }
        return WithTags(empire->Name(), tag, data);
    }

    std::string MeterTypeString(const std::string& data, const std::string& tag, bool& valid) {
        std::string retval = UserString("ERROR");
        // validate data
        MeterType meter_type = INVALID_METER_TYPE;
        std::istringstream data_ss(data);
        data_ss >> meter_type;

        if (meter_type > INVALID_METER_TYPE && meter_type < NUM_METER_TYPES) {
            retval = boost::lexical_cast<std::string>(meter_type);
            if (UserStringExists(retval))
                retval = WithTags(UserString(retval), tag, retval);
        }

        return retval;
    }

    /// Interprets value of data as a name.
    /// Returns translation of name, if Get says
    /// that a thing by that name exists, otherwise ERROR.
    template <typename T,const T* (*GetByName)(const std::string&)>
    std::string NameString(const std::string& data, const std::string& tag, bool& valid) {
        if (!GetByName(data)) {
            valid = false;
            return UserString("ERROR");
        }
        return WithTags(UserString(data), tag, data);
    }

    /// Returns a map that tells shich function should be used to
    /// generate a substitution for which tag.
    std::map<std::string, TagString> CreateSubstituterMap() {
        std::map<std::string, TagString> subs;
        subs[VarText::TEXT_TAG] = TextString;
        subs[VarText::RAW_TEXT_TAG] = RawTextString;

        subs[VarText::PLANET_ID_TAG] =
            subs[VarText::SYSTEM_ID_TAG] =
            subs[VarText::SHIP_ID_TAG] =
            subs[VarText::FLEET_ID_TAG] =
            subs[VarText::BUILDING_ID_TAG] =
            subs[VarText::FIELD_ID_TAG] = UniverseObjectString;
        subs[VarText::COMBAT_ID_TAG] = CombatLogString;
        subs[VarText::TECH_TAG] = NameString<Tech, GetTech>;
        subs[VarText::BUILDING_TYPE_TAG] = NameString<BuildingType, GetBuildingType>;
        subs[VarText::SHIP_HULL_TAG] = NameString<HullType, GetHullType>;
        subs[VarText::SHIP_PART_TAG] = NameString<PartType, GetPartType>;
        subs[VarText::SPECIAL_TAG] = NameString<Special, GetSpecial>;
        subs[VarText::SPECIES_TAG] = NameString<Species, GetSpecies>;
        subs[VarText::FIELD_TYPE_TAG] = NameString<FieldType, GetFieldType>;
        subs[VarText::METER_TYPE_TAG] = MeterTypeString;
        subs[VarText::DESIGN_ID_TAG] = ShipDesignString;
        subs[VarText::PREDEFINED_DESIGN_TAG] = PredefinedShipDesignString;
        subs[VarText::EMPIRE_ID_TAG] = EmpireString;
        return subs;
    }

    /// Global substitution map, wrapped in a function to avoid initialization order issues
    const std::map<std::string, TagString>& SubstitutionMap() {
        static std::map<std::string, TagString> subs = CreateSubstituterMap();
        return subs;
    }


    /** Converts (first, last) to a string, looks up its value in the Universe,
      * then appends this to the end of a std::string. */
    struct SubstituteAndAppend {
        SubstituteAndAppend(const std::map<std::string, std::string>& variables,
                            std::string& str, bool& valid) :
            m_variables(variables),
            m_str(str),
            m_valid(valid)
        {}

        void operator()(const char* first, const char* last) const {
            std::string token(first, last);

            // special case: "%%" is interpreted to be a '%' character
            if (token.empty()) {
                m_str += "%";
                return;
            }

            // Labelled tokens have the form %tag:label%,  unlabelled are just %tag%
            std::vector<std::string> pieces;
            boost::split(pieces, token, boost::is_any_of(LABEL_SEPARATOR));

            std::string tag; //< The tag of the token (the type)
            std::string label; //< The label of the token (the key to fetch data by)
            if (pieces.size() == 1) {
                // No separator. There is only a tag. The tag is the default label
                tag = token;
                label = token;
            } else if (pieces.size() == 2) {
                // Had a separator
                tag = pieces[0];
                label = pieces[1];
            }

            // look up child
            auto elem = m_variables.find(label);
            if (m_variables.end() == elem) {
                m_str += UserString("ERROR");
                m_valid = false;
                return;
            }

            auto substituter = SubstitutionMap().find(tag);
            if (substituter != SubstitutionMap().end()) {
                m_str += substituter->second(elem->second, tag, m_valid);
            } else {
                ErrorLogger() << "SubstituteAndAppend::operator(): No substitution scheme defined for tag: " << tag << " from token: " << token;
                m_str += UserString("ERROR");
                m_valid = false;
            }
        }

        const std::map<std::string, std::string>& m_variables;
        std::string& m_str;
        bool& m_valid;
    };

    // sticks a sequence of characters onto the end of a std::string
    struct StringAppend {
        StringAppend(std::string& str) :
            m_str(str)
        {}

        void operator()(const char* first, const char* last) const
        { m_str += std::string(first, last); }
        std::string& m_str;
    };
}

// static(s)
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

VarText::VarText(const std::string& template_string, bool stringtable_lookup_template/* = true*/) :
    m_template_string(template_string),
    m_stringtable_lookup_flag(stringtable_lookup_template)
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

void VarText::SetTemplateString(const std::string& text, bool stringtable_lookup_template/* = true*/) {
    m_text = text;
    m_stringtable_lookup_flag = stringtable_lookup_template;
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

    // set up parser
    namespace classic = boost::spirit::classic;
    classic::rule<> token = *(classic::anychar_p - classic::space_p - END_VAR.c_str());
    classic::rule<> var = START_VAR.c_str() >> token[SubstituteAndAppend(m_variables, m_text, m_validated)] >> END_VAR.c_str();
    classic::rule<> non_var = classic::anychar_p - START_VAR.c_str();

    // parse and substitute variables
    try {
        classic::parse(template_str.c_str(), *(non_var[StringAppend(m_text)] | var));
    } catch (const std::exception&) {
        ErrorLogger() << "VarText::GenerateVartText caught exception when parsing template string: " << m_template_string;
    }
}
