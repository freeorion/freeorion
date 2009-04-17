#include "VarText.h"

#include "AppInterface.h"
#include "MultiplayerCommon.h"
#include "../universe/UniverseObject.h"

#include <boost/spirit.hpp>

// Forward declarations
class Tech;
class BuildingType;
const Tech* GetTech(const std::string& name);
const BuildingType* GetBuildingType(const std::string& name);

namespace {
    // converts (first, last) to a string, looks up its value in the Universe, then appends this to the end of a std::string
    struct SubstituteAndAppend
    {
        SubstituteAndAppend(const XMLElement& variables, std::string& str) : m_variables(variables), m_str(str) {}
        void operator()(const char* first, const char* last) const
        {
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
            std::string open_tag = "<" + token_elem.Tag() + " " + token_elem.Attribute("value") + ">";
            std::string close_tag = "</" + token_elem.Tag() + ">";

            // universe object token types
            if (token == VarText::PLANET_ID_TAG || token == VarText::SYSTEM_ID_TAG || token == VarText::SHIP_ID_TAG || token == VarText::FLEET_ID_TAG) {
                int object_id = boost::lexical_cast<int>(token_elem.Attribute("value"));
                const UniverseObject* obj = GetUniverse().Object(object_id);

                if (!obj)
                    obj = GetUniverse().DestroyedObject(object_id);

                if (!obj) {
                    m_str += UserString("ERROR");
                    return;
                }

                m_str += open_tag + obj->Name() + close_tag;
            } else if (token == VarText::TECH_ID_TAG) {
                std::string tech_name = token_elem.Attribute("value");

                if (!GetTech(tech_name)) {
                    m_str += UserString("ERROR");
                    return;
                }

                m_str += open_tag + UserString(tech_name) + close_tag;
            } else if (token == VarText::BUILDING_ID_TAG) {
                std::string building_name = token_elem.Attribute("value");

                if (!GetBuildingType(building_name)) {
                    m_str += UserString("ERROR");
                    return;
                }

                m_str += open_tag + UserString(building_name) + close_tag;
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
        StringAppend(std::string& str) : m_str(str) {}
        void operator()(const char* first, const char* last) const
        {
            m_str += std::string(first, last);
        }
        std::string& m_str;
    };
}

// static(s)
const std::string VarText::START_VAR = "%";
const std::string VarText::END_VAR = "%";
const std::string VarText::PLANET_ID_TAG = "planet";
const std::string VarText::SYSTEM_ID_TAG = "system";
const std::string VarText::TECH_ID_TAG = "tech";
const std::string VarText::SHIP_ID_TAG = "ship";
const std::string VarText::FLEET_ID_TAG = "fleet";
const std::string VarText::BUILDING_ID_TAG = "building";
const std::string VarText::EMPIRE_ID_TAG = "empire";

void VarText::GenerateVarText(const std::string& template_str)
{
    // generates a string complete with substituted variables and hyperlinks
    // the procedure here is to replace any tokens within %% with variables of the same name in the SitRep XML data

    // get template string
    std::string final_str;

    using namespace boost::spirit;
    rule<> token = *(anychar_p - space_p - END_VAR.c_str());
    rule<> var = START_VAR.c_str() >> token[SubstituteAndAppend(m_variables, final_str)] >> END_VAR.c_str();
    rule<> non_var = anychar_p - START_VAR.c_str();

    parse(template_str.c_str(), *(non_var[StringAppend(final_str)] | var));

    m_text = final_str;
}
