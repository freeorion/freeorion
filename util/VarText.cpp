#include "VarText.h"

#include "../universe/Universe.h"
#include "AppInterface.h"
#include "MultiplayerCommon.h"

#include <boost/spirit/include/classic.hpp>

// Forward declarations
class Tech;
class BuildingType;
class Special;
const Tech*         GetTech(const std::string& name);
const BuildingType* GetBuildingType(const std::string& name);
const Special*      GetSpecial(const std::string& name);

namespace {
    // converts (first, last) to a string, looks up its value in the Universe, then appends this to the end of a std::string
    struct SubstituteAndAppend
    {
        SubstituteAndAppend(const XMLElement& variables, std::string& str) :
            m_variables(variables),
            m_str(str)
        {}

        void operator()(const char* first, const char* last) const
        {
            const ObjectMap& objects = GetMainObjectMap();

            std::string token(first, last);

            // special case: "%%" is interpreted to be a '%' character
            if (token.empty()) {
                m_str += "%";
                return;
            }

            // look up child
            if (!m_variables.ContainsChild(token)) {
                m_str += UserString("ERROR");
                return;
            }


            const XMLElement& token_elem = m_variables.Child(token);

            // plain text substitution token
            if (token == VarText::TEXT_TAG) {
                std::string text = token_elem.Attribute("value");
                m_str += UserString(text);
                return;
            }

            std::string open_tag = "<" + token_elem.Tag() + " " + token_elem.Attribute("value") + ">";
            std::string close_tag = "</" + token_elem.Tag() + ">";

            // universe object token types
            if (token == VarText::PLANET_ID_TAG ||
                token == VarText::SYSTEM_ID_TAG ||
                token == VarText::SHIP_ID_TAG ||
                token == VarText::FLEET_ID_TAG ||
                token == VarText::BUILDING_ID_TAG)
            {
                int object_id = boost::lexical_cast<int>(token_elem.Attribute("value"));
                const UniverseObject* obj = objects.Object(object_id);
                if (!obj) {
                    m_str += UserString("ERROR");
                    return;
                }
                m_str += open_tag + obj->Name() + close_tag;

            // technology token
            } else if (token == VarText::TECH_TAG) {
                std::string name = token_elem.Attribute("value");
                if (!GetTech(name)) {
                    m_str += UserString("ERROR");
                    return;
                }
                m_str += open_tag + UserString(name) + close_tag;

            // building type token
            } else if (token == VarText::BUILDING_TYPE_TAG) {
                std::string name = token_elem.Attribute("value");
                if (!GetBuildingType(name)) {
                    m_str += UserString("ERROR");
                    return;
                }
                m_str += open_tag + UserString(name) + close_tag;

            // ship hull token
            } else if (token == VarText::SHIP_HULL_TAG) {
                std::string name = token_elem.Attribute("value");
                if (!GetHullType(name)) {
                    m_str += UserString("ERROR");
                    return;
                }
                m_str += open_tag + UserString(name) + close_tag;

            // ship part token
            } else if (token == VarText::SHIP_PART_TAG) {
                std::string name = token_elem.Attribute("value");
                if (!GetPartType(name)) {
                    m_str += UserString("ERROR");
                    return;
                }
                m_str += open_tag + UserString(name) + close_tag;

            // special token
            } else if (token == VarText::SPECIAL_TAG) {
                std::string name = token_elem.Attribute("value");
                if (!GetSpecial(name)) {
                    m_str += UserString("ERROR");
                    return;
                }
                m_str += open_tag + UserString(name) + close_tag;

            /* TODO: SPECIES_TAG */

            // empire token
            } else if (token == VarText::EMPIRE_ID_TAG) {
                std::string empire_name = token_elem.Attribute("value");
                m_str += empire_name;
            }
        }

        const XMLElement&  m_variables;
        std::string&       m_str;
    };

    // sticks a sequence of characters onto the end of a std::string
    struct StringAppend
    {
        StringAppend(std::string& str) :
            m_str(str)
        {}

        void operator()(const char* first, const char* last) const
        {
            m_str += std::string(first, last);
        }
        std::string& m_str;
    };
}

// static(s)
const std::string VarText::TEXT_TAG = "text";

const std::string VarText::START_VAR = "%";
const std::string VarText::END_VAR = "%";
const std::string VarText::PLANET_ID_TAG = "planet";
const std::string VarText::SYSTEM_ID_TAG = "system";
const std::string VarText::SHIP_ID_TAG = "ship";
const std::string VarText::FLEET_ID_TAG = "fleet";
const std::string VarText::BUILDING_ID_TAG = "building";

const std::string VarText::EMPIRE_ID_TAG = "empire";

const std::string VarText::TECH_TAG = "tech";
const std::string VarText::BUILDING_TYPE_TAG = "buildingtype";
const std::string VarText::SPECIAL_TAG = "special";
const std::string VarText::SHIP_HULL_TAG = "shiphull";
const std::string VarText::SHIP_PART_TAG = "shippart";
const std::string VarText::SPECIES_TAG = "species";

void VarText::GenerateVarText(const std::string& template_str)
{
    // generates a string complete with substituted variables and hyperlinks
    // the procedure here is to replace any tokens within %% with variables of the same name in the SitRep XML data

    // get template string
    std::string final_str;

    using namespace boost::spirit::classic;
    rule<> token = *(anychar_p - space_p - END_VAR.c_str());
    rule<> var = START_VAR.c_str() >> token[SubstituteAndAppend(m_variables, final_str)] >> END_VAR.c_str();
    rule<> non_var = anychar_p - START_VAR.c_str();

    parse(template_str.c_str(), *(non_var[StringAppend(final_str)] | var));

    m_text = final_str;
}
