#include "VarText.h"

#include "../universe/NamedValueRefManager.h"
#include "../universe/ValueRefs.h"
#include "../universe/Universe.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Species.h"
#include "../Empire/Empire.h"
#include "i18n.h"
#include "Logger.h"
#include "AppInterface.h"

#include <boost/xpressive/xpressive.hpp>

#if __has_include(<charconv>)
#include <charconv>
#endif
#include <functional>
#include <map>

namespace xpr = boost::xpressive;

class Tech;
class Policy;
class BuildingType;
class Special;
class Species;
class FieldType;
class ShipHull;
class ShipPart;
const Tech*         GetTech(std::string_view name);
const Policy*       GetPolicy(std::string_view name);
const BuildingType* GetBuildingType(std::string_view name);
const Special*      GetSpecial(std::string_view name);
const FieldType*    GetFieldType(std::string_view name);
const ShipHull*     GetShipHull(std::string_view name);
const ShipPart*     GetShipPart(std::string_view name);

namespace {
    constexpr int ToIntCX(std::string_view sv, int default_result = -1) {
        if (sv.empty())
            return default_result;

        bool is_negative = (sv.front() == '-');
        sv = sv.substr(is_negative);
        for (auto c : sv)
            if (c > '9' || c < '0')
                return default_result;

        int64_t retval = 0;
        for (auto c : sv) {
            retval *= 10;
            retval += (c - '0');
        }

        retval *= (is_negative ? -1 : 1);

        constexpr int64_t max_int = std::numeric_limits<int>::max();
        constexpr int64_t min_int = std::numeric_limits<int>::min();

        return (retval > max_int) ? max_int :
            (retval < min_int) ? min_int :
            retval;
    }
    static_assert(ToIntCX("2147483647") == 2147483647);
    static_assert(ToIntCX("-104") == -104);
    static_assert(ToIntCX("banana", -10) == -10);
    static_assert(ToIntCX("-banana") == -1);
    static_assert(ToIntCX("-0banana", -20) == -20);
    static_assert(ToIntCX("") == -1);
    static_assert(ToIntCX("0") == 0);
    static_assert(ToIntCX("0000") == 0);
    static_assert(ToIntCX("-0000") == 0);

    // wrapper for converting string to integer
    int ToInt(std::string_view sv, int default_result = -1) {
#if defined(__cpp_lib_to_chars)
        int retval = default_result;
        std::from_chars(sv.data(), sv.data() + sv.size(), retval);
        return retval;
#else
        return ToIntCX(sv, default_result);
#endif
    }

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
    std::string WithTags(std::string_view content, std::string_view tag, std::string_view data) {
        std::string retval;
        retval.reserve(1 + tag.size() + 1 + data.size() + 1 + content.size() + 2 + tag.size() + 2);
        retval.append("<").append(tag).append(" ").append(data).append(">")
              .append(content)
              .append("</").append(tag).append(">");
        return retval;
    }

    //! Get string substitute for a tag that is a universe object
    boost::optional<std::string> UniverseObjectString(
        std::string_view data, std::string_view tag, const ObjectMap& objects)
    {
        const int object_id = ToInt(data, INVALID_OBJECT_ID);
        auto obj = objects.getRaw(object_id);
        if (!obj)
            return boost::none;

        return WithTags(GetVisibleObjectName(*obj), tag, data); // TODO: pass universe instead of relying on client's
    }

    //! Returns substitution string for an in-Universe ship design tag
    boost::optional<std::string> ShipDesignString(std::string_view data,
                                                  const Universe& universe)
    {
        const int design_id = ToInt(data, INVALID_DESIGN_ID);
        if (const auto design = universe.GetShipDesign(design_id))
            return WithTags(design->Name(), VarText::DESIGN_ID_TAG, data);

        return UserString("FW_UNKNOWN_DESIGN_NAME");
    }

    //! Returns substitution string for a predefined ship design tag
    boost::optional<std::string> PredefinedShipDesignString(std::string_view data, const ScriptingContext& context) {
        if (const ShipDesign* design = context.ContextUniverse().GetGenericShipDesign(data))
            return WithTags(design->Name(), VarText::PREDEFINED_DESIGN_TAG, data);
        return boost::none;
    }

    boost::optional<std::string> SpeciesString(std::string_view data, const SpeciesManager& sm) {
        if (const Species* species = sm.GetSpecies(data))
            return WithTags(species->Name(), VarText::SPECIES_TAG, data);
        return boost::none;
    }

    boost::optional<std::string> MeterTypeString(std::string_view data, const ScriptingContext&) {
        boost::optional<std::string> retval = boost::none;
        // validate data
        MeterType meter_type = MeterTypeFromString(data, MeterType::INVALID_METER_TYPE);

        if (meter_type > MeterType::INVALID_METER_TYPE && meter_type < MeterType::NUM_METER_TYPES) {
            auto mt_string{to_string(meter_type)};
            if (UserStringExists(mt_string))
                retval = WithTags(UserString(mt_string), VarText::METER_TYPE_TAG, mt_string);
            else
                retval = std::string{mt_string};
        }

        return retval;
    }

    //! Returns substitution string for an empire
    boost::optional<std::string> EmpireString(
        std::string_view data, const EmpireManager::const_container_type& empires)
    {
        const int id = ToInt(data, ALL_EMPIRES);
        auto it = empires.find(id);
        if (it != empires.end())
            return WithTags(it->second->Name(), VarText::EMPIRE_ID_TAG, data);

        return boost::none;
    }

    //! Returns substitution string for tag and data, where \a data is looked up using the
    //! GetByName function and the returned name is wrapped in \a tag to linkifying it.
    //! If GetByName returns an empty optional or null pointer, then an empty optional<string>
    //! is returned
    template <typename T, const T* (*GetByName)(std::string_view)>
    boost::optional<std::string> NameString(std::string_view data, std::string_view tag) {
        if (!GetByName(data))
            return boost::none;
        return WithTags(UserString(data), tag, data);
    }

    //! Function signature of tag substitution functions.
    using TagStringFunc = std::function<boost::optional<std::string> (std::string_view, const ScriptingContext&)>;

    //! Functions to evaluate to get substitution of user-readable text for a tag, indexed by tag
    const std::array<std::pair<std::string_view, TagStringFunc>, 25> substitution_map{{
        {VarText::TEXT_TAG, +[](std::string_view data, const ScriptingContext&)
            { return UserString(data); }},
        {VarText::RAW_TEXT_TAG, +[](std::string_view data, const ScriptingContext&)
            { return std::string{data}; }},
        {VarText::PLANET_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return UniverseObjectString(data, VarText::PLANET_ID_TAG, context.ContextObjects()); }},
        {VarText::SYSTEM_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return UniverseObjectString(data, VarText::SYSTEM_ID_TAG, context.ContextObjects()); }},
        {VarText::SHIP_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return UniverseObjectString(data, VarText::SHIP_ID_TAG, context.ContextObjects()); }},
        {VarText::FLEET_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return UniverseObjectString(data, VarText::FLEET_ID_TAG, context.ContextObjects()); }},
        {VarText::BUILDING_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return UniverseObjectString(data, VarText::BUILDING_ID_TAG, context.ContextObjects()); }},
        {VarText::FIELD_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return UniverseObjectString(data, VarText::FIELD_ID_TAG, context.ContextObjects()); }},
        {VarText::COMBAT_ID_TAG, [](std::string_view data, const ScriptingContext&)
            { return WithTags(UserString("COMBAT"), VarText::COMBAT_ID_TAG, data); }},
        {VarText::TECH_TAG, [](std::string_view data, const ScriptingContext&)
            { return NameString<Tech, GetTech>(data, VarText::TECH_TAG); }},
        {VarText::POLICY_TAG, [](std::string_view data, const ScriptingContext&)
            { return NameString<Policy, GetPolicy>(data, VarText::POLICY_TAG); }},
        {VarText::BUILDING_TYPE_TAG, [](std::string_view data, const ScriptingContext&)
            { return NameString<BuildingType, GetBuildingType>(data, VarText::BUILDING_TYPE_TAG); }},
        {VarText::SHIP_HULL_TAG, [](std::string_view data, const ScriptingContext&)
            { return NameString<ShipHull, GetShipHull>(data, VarText::SHIP_HULL_TAG); }},
        {VarText::SHIP_PART_TAG, [](std::string_view data, const ScriptingContext&)
            { return NameString<ShipPart, GetShipPart>(data, VarText::SHIP_PART_TAG); }},
        {VarText::SPECIAL_TAG, [](std::string_view data, const ScriptingContext&)
            { return NameString<Special, GetSpecial>(data, VarText::SPECIAL_TAG); }},
        {VarText::SPECIES_TAG, [](std::string_view data, const ScriptingContext& context)
            { return SpeciesString(data, context.species); }},
        {VarText::FIELD_TYPE_TAG, [](std::string_view data, const ScriptingContext&)
            { return NameString<FieldType, GetFieldType>(data, VarText::FIELD_TYPE_TAG); }},
        {VarText::METER_TYPE_TAG, MeterTypeString},
        {VarText::DESIGN_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return ShipDesignString(data, context.ContextUniverse()); }},
        {VarText::PREDEFINED_DESIGN_TAG, PredefinedShipDesignString},
        {VarText::EMPIRE_ID_TAG, [](std::string_view data, const ScriptingContext& context)
            { return EmpireString(data, context.Empires().GetEmpires()); }},
        {VarText::FOCS_VALUE_TAG, [](std::string_view data, const ScriptingContext&) -> boost::optional<std::string>
            {
                if (const ValueRef::ValueRefBase* vr = GetValueRefBase(data))
                    return WithTags(UserString(data), VarText::FOCS_VALUE_TAG, vr->EvalAsString());
                else
                    return WithTags(data, VarText::FOCS_VALUE_TAG, UserString("UNKNOWN_VALUE_REF_NAME"));
            }},
        {VarText::ENVIRONMENT_TAG, [](std::string_view data, const ScriptingContext& context)
            {
                // Assume that we have no userstring which is also a number
                if (UserStringExists(data))
                    return UserString(data);
                const int planet_id = ToInt(data, INVALID_OBJECT_ID);
                if (auto planet = context.ContextObjects().getRaw<Planet>(planet_id))
                    return UserString(to_string(planet->EnvironmentForSpecies(context)));
                return UserString("UNKNOWN_PLANET");
            }},
        {VarText::USER_STRING_TAG, [](std::string_view data, const ScriptingContext&)
            { return UserString(data); }},
        {VarText::PLANET_TYPE_TAG, [](std::string_view data, const ScriptingContext& context)
            {
                // Assume that we have no userstring which is also a number
                if (UserStringExists(data))
                    return UserString(data);
                const int planet_id = ToInt(data, INVALID_OBJECT_ID);
                if (auto planet = context.ContextObjects().getRaw<Planet>(planet_id))
                    return UserString(to_string(planet->Type()));
                return UserString("UNKNOWN_PLANET");
            }},
    }};


    //! Looks up the given match in the Universe and returns the Universe
    //! entities value. If the lookup or the substitution fails, sets
    //! \a valid to false.
    struct Substitute {
        Substitute(const std::map<std::string, std::string, std::less<>>& variables, bool& valid) :
            m_variables(variables),
            m_valid(valid)
        {}

        std::string operator()(xpr::smatch const& match) const {
            // Labelled variables have the form %tag:label%,  unlabelled are just %tag%
            // Use the label value. When missing, use the tag submatch as label instead.

            const ScriptingContext context;

            const int idx = match[2].matched ? 2 : 1;
            const auto& m{match[idx]};
            std::string_view label{&*m.first, static_cast<std::size_t>(std::max(0, static_cast<int>(m.length())))};

            // look up child
            auto elem = m_variables.find(label);
            if (elem == m_variables.end()) {
                ErrorLogger() << "Substitute::operator(): No value found for label: " << label
                              << "  from token: " << match.str();
                m_valid = false;
                return UserString("ERROR");
            }

            std::string_view tag{&*match[1].first, static_cast<std::size_t>(std::max(0, static_cast<int>(match[1].length())))};

            auto substituter_it = std::find_if(substitution_map.begin(), substitution_map.end(),
                                               [tag](const auto& e) { return e.first == tag; });
            if (substituter_it != substitution_map.end()) {
                const auto& substitution_func = substituter_it->second;
                const auto& variable_value = elem->second;
                if (auto substitution = substitution_func(variable_value, context))
                    return *substitution; // optional<std::string> contains a temporary string, which can't be returned by reference
            }

            ErrorLogger() << "Substitute::operator(): No substitution executed for tag: " << tag
                          << " from token: " << match.str();
            m_valid = false;
            return UserString("ERROR");
        }

        const std::map<std::string, std::string, std::less<>>& m_variables;
        bool& m_valid;
    };
}


VarText::VarText(std::string template_string, bool stringtable_lookup) :
    m_template_string(std::move(template_string)),
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

void VarText::SetTemplateString(std::string template_string, bool stringtable_lookup) {
    m_template_string = std::move(template_string);
    m_stringtable_lookup_flag = stringtable_lookup;
}

std::vector<std::string_view> VarText::GetVariableTags() const {
    std::vector<std::string_view> retval;
    retval.reserve(m_variables.size());
    for (const auto& [tag, data] : m_variables) {
        (void)data;
        retval.push_back(tag);
    }
    return retval;
}

void VarText::AddVariable(std::string tag, std::string data)
{ m_variables[std::move(tag)] = std::move(data); }

void VarText::AddVariables(std::vector<std::pair<std::string, std::string>>&& data) {
    for (auto& dat : data)
        m_variables.insert(std::move(dat));
}

void VarText::GenerateVarText() const {
    // generate a string complete with substituted variables and hyperlinks
    // the procedure here is to replace any tokens within %% with variables of
    // the same name in the SitRep XML data
    m_text.clear();
    m_validated = true;
    if (m_template_string.empty())
        return;

    // get string into which to substitute variables
    const auto& template_str = m_stringtable_lookup_flag ? UserString(m_template_string) : m_template_string;

    xpr::sregex var = '%' >> (xpr::s1 = -+xpr::_w) >> !(':' >> (xpr::s2 = -+xpr::_w)) >> '%';
    m_text = xpr::regex_replace(template_str, var, Substitute(m_variables, m_validated));
}
