#include "ValueRefs.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iterator>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/numeric.hpp>
#include "Building.h"
#include "Enums.h"
#include "Field.h"
#include "Fighter.h"
#include "Fleet.h"
#include "Pathfinder.h"
#include "Planet.h"
#include "ShipDesign.h"
#include "ShipHull.h"
#include "ShipPart.h"
#include "Ship.h"
#include "Species.h"
#include "System.h"
#include "Tech.h"
#include "UniverseObjectVisitors.h"
#include "UniverseObject.h"
#include "Universe.h"
#include "../Empire/Empire.h"
#include "../Empire/Supply.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Random.h"


std::string DoubleToString(double val, int digits, bool always_show_sign);
bool UserStringExists(const std::string& str);

FO_COMMON_API extern const int INVALID_DESIGN_ID;

namespace {
    std::shared_ptr<const UniverseObject> FollowReference(
        std::vector<std::string>::const_iterator first,
        std::vector<std::string>::const_iterator last,
        ValueRef::ReferenceType ref_type,
        const ScriptingContext& context)
    {
        std::shared_ptr<const UniverseObject> obj;
        switch (ref_type) {
        case ValueRef::ReferenceType::NON_OBJECT_REFERENCE:                return context.condition_local_candidate;   break;
        case ValueRef::ReferenceType::SOURCE_REFERENCE:                    obj = context.source;                       break;
        case ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE:             obj = context.effect_target;                break;
        case ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:  obj = context.condition_root_candidate;     break;
        case ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE:
        default:                                                           obj = context.condition_local_candidate;    break;
        }

        if (!obj) {
            std::string type_string;
            switch (ref_type) {
            case ValueRef::ReferenceType::SOURCE_REFERENCE:                    type_string = "Source";         break;
            case ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE:             type_string = "Target";         break;
            case ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:  type_string = "RootCandidate";  break;
            case ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE:
            default:                                                           type_string = "LocalCandidate"; break;
            }
            ErrorLogger() << "FollowReference : top level object (" << type_string << ") not defined in scripting context. "
                          << "  strings: " << [it=first, last]() mutable -> std::string {
                std::string retval;
                for (; it != last; ++it)
                    retval += *it + " ";
                return retval;
            }()           << " source: " << (context.source ? context.source->Name() : "0")
                          << " target: " << (context.effect_target ? context.effect_target->Name() : "0")
                          << " local c: " << (context.condition_local_candidate ? context.condition_local_candidate->Name() : "0")
                          << " root c: " << (context.condition_root_candidate ? context.condition_root_candidate->Name() : "0");

            return nullptr;
        }

        while (first != last) {
            std::string property_name = *first;
            if (property_name == "Planet") {
                if (auto b = std::dynamic_pointer_cast<const Building>(obj)) {
                    obj = context.ContextObjects().get<Planet>(b->PlanetID());
                } else {
                    ErrorLogger() << "FollowReference : object not a building, so can't get its planet.";
                    obj = nullptr;
                }
            } else if (property_name == "System") {
                if (obj)
                    obj = context.ContextObjects().get<System>(obj->SystemID());
                if (!obj)
                    ErrorLogger() << "FollowReference : Unable to get system for object";
            } else if (property_name == "Fleet") {
                if (auto s = std::dynamic_pointer_cast<const Ship>(obj)) {
                    obj = context.ContextObjects().get<Fleet>(s->FleetID());
                } else {
                    ErrorLogger() << "FollowReference : object not a ship, so can't get its fleet";
                    obj = nullptr;
                }
            }
            ++first;
        }
        return obj;
    }

    // Generates a debug trace that can be included in error logs, augmenting
    // the ReconstructName() info with additional info identifying the object
    // references that were successfully followed.
    std::string TraceReference(const std::vector<std::string>& property_name,
                               ValueRef::ReferenceType ref_type,
                               const ScriptingContext& context)
    {
        std::shared_ptr<const UniverseObject> obj, initial_obj;
        std::string retval = ReconstructName(property_name, ref_type, false) + " : ";
        switch (ref_type) {
        case ValueRef::ReferenceType::NON_OBJECT_REFERENCE:
            retval += " | Non Object Reference |";
            return retval;
            break;
        case ValueRef::ReferenceType::SOURCE_REFERENCE:
            retval += " | Source: ";
            obj = context.source;
            break;
        case ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE:
            retval += " | Effect Target: ";
            obj = context.effect_target;
            break;
        case ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:
            retval += " | Root Candidate: ";
            obj = context.condition_root_candidate;
            break;
        case ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE:
        default:
            retval += " | Local Candidate: ";
            obj = context.condition_local_candidate;
            break;
        }
        if (obj) {
            retval += UserString(boost::lexical_cast<std::string>(obj->ObjectType())) + " "
                    + std::to_string(obj->ID()) + " ( " + obj->Name() + " ) ";
            initial_obj = obj;
        }
        retval += " | ";

        auto first = property_name.begin();
        auto last = property_name.end();
        while (first != last) {
            std::string property_name_part = *first;
            retval += " " + property_name_part + " ";
            if (property_name_part == "Planet") {
                if (auto b = std::dynamic_pointer_cast<const Building>(obj)) {
                    retval += "(" + std::to_string(b->PlanetID()) + "): ";
                    obj = context.ContextObjects().get<Planet>(b->PlanetID());
                } else
                    obj = nullptr;
            } else if (property_name_part == "System") {
                if (obj) {
                    retval += "(" + std::to_string(obj->SystemID()) + "): ";
                    obj = context.ContextObjects().get<System>(obj->SystemID());
                }
            } else if (property_name_part == "Fleet") {
                if (auto s = std::dynamic_pointer_cast<const Ship>(obj))  {
                    retval += "(" + std::to_string(s->FleetID()) + "): ";
                    obj = context.ContextObjects().get<Fleet>(s->FleetID());
                } else
                    obj = nullptr;
            }

            ++first;

            if (obj && initial_obj != obj) {
                retval += "  Referenced Object: " + UserString(boost::lexical_cast<std::string>(obj->ObjectType())) + " "
                        + std::to_string(obj->ID()) + " ( " + obj->Name() + " )";
            }
            retval += " | ";
        }
        return retval;
    }

    struct ObjectTypeVisitor : UniverseObjectVisitor {
        ObjectTypeVisitor() : m_type(UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE) {}

        std::shared_ptr<UniverseObject> Visit(const std::shared_ptr<Building>& obj) const override
        { m_type = UniverseObjectType::OBJ_BUILDING; return obj; }

        std::shared_ptr<UniverseObject> Visit(const std::shared_ptr<Fleet>& obj) const override
        { m_type = UniverseObjectType::OBJ_FLEET; return obj; }

        std::shared_ptr<UniverseObject> Visit(const std::shared_ptr<Planet>& obj) const override
        { m_type = UniverseObjectType::OBJ_PLANET; return obj; }

        std::shared_ptr<UniverseObject> Visit(const std::shared_ptr<Ship>& obj) const override
        { m_type = UniverseObjectType::OBJ_SHIP; return obj; }

        std::shared_ptr<UniverseObject> Visit(const std::shared_ptr<System>& obj) const override
        { m_type = UniverseObjectType::OBJ_SYSTEM; return obj; }

        std::shared_ptr<UniverseObject> Visit(const std::shared_ptr<Field>& obj) const override
        { m_type = UniverseObjectType::OBJ_FIELD; return obj; }

        mutable UniverseObjectType m_type;
    };

    const std::map<std::string, MeterType>& GetMeterNameMap() {
        static const std::map<std::string, MeterType> meter_name_map{
            {"Population",           MeterType::METER_POPULATION},
            {"TargetPopulation",     MeterType::METER_TARGET_POPULATION},
            {"Industry",             MeterType::METER_INDUSTRY},
            {"TargetIndustry",       MeterType::METER_TARGET_INDUSTRY},
            {"Research",             MeterType::METER_RESEARCH},
            {"TargetResearch",       MeterType::METER_TARGET_RESEARCH},
            {"Influence",            MeterType::METER_INFLUENCE},
            {"TargetInfluence",      MeterType::METER_TARGET_INFLUENCE},
            {"Construction",         MeterType::METER_CONSTRUCTION},
            {"TargetConstruction",   MeterType::METER_TARGET_CONSTRUCTION},
            {"Happiness",            MeterType::METER_HAPPINESS},
            {"TargetHappiness",      MeterType::METER_TARGET_HAPPINESS},
            {"MaxFuel",              MeterType::METER_MAX_FUEL},
            {"Fuel",                 MeterType::METER_FUEL},
            {"MaxStructure",         MeterType::METER_MAX_STRUCTURE},
            {"Structure",            MeterType::METER_STRUCTURE},
            {"MaxShield",            MeterType::METER_MAX_SHIELD},
            {"Shield",               MeterType::METER_SHIELD},
            {"MaxDefense",           MeterType::METER_MAX_DEFENSE},
            {"Defense",              MeterType::METER_DEFENSE},
            {"MaxTroops",            MeterType::METER_MAX_TROOPS},
            {"Troops",               MeterType::METER_TROOPS},
            {"RebelTroops",          MeterType::METER_REBEL_TROOPS},
            {"Supply",               MeterType::METER_SUPPLY},
            {"MaxSupply",            MeterType::METER_MAX_SUPPLY},
            {"Stockpile",            MeterType::METER_STOCKPILE},
            {"MaxStockpile",         MeterType::METER_MAX_STOCKPILE},
            {"Stealth",              MeterType::METER_STEALTH},
            {"Detection",            MeterType::METER_DETECTION},
            {"Speed",                MeterType::METER_SPEED},
            {"Damage",               MeterType::METER_CAPACITY},
            {"Capacity",             MeterType::METER_CAPACITY},
            {"MaxCapacity",          MeterType::METER_MAX_CAPACITY},
            {"SecondaryStat",        MeterType::METER_SECONDARY_STAT},
            {"MaxSecondaryStat",     MeterType::METER_MAX_SECONDARY_STAT},
            {"Size",                 MeterType::METER_SIZE}
        };
        return meter_name_map;
    }

    // force early init to avoid threading issues later
    std::map<std::string, MeterType> dummy = GetMeterNameMap();

    const std::string EMPTY_STRING;
}

namespace ValueRef {
std::string ValueRefBase::InvariancePattern() const {
    return std::string(RootCandidateInvariant()?"R":"r") + (LocalCandidateInvariant()?"L":"l")
        + (SourceInvariant()?"S":"s") + (TargetInvariant()?"T":"t")
        + (SimpleIncrement()?"I":"i") + (ConstantExpr()?"C":"c");
}

MeterType NameToMeter(const std::string& name) {
    MeterType retval = MeterType::INVALID_METER_TYPE;
    auto it = GetMeterNameMap().find(name);
    if (it != GetMeterNameMap().end())
        retval = it->second;
    return retval;
}

const std::string& MeterToName(MeterType meter) {
    for (auto& [name, type] : GetMeterNameMap()) {
        if (type == meter)
            return name;
    }
    return EMPTY_STRING;
}

std::string ReconstructName(const std::vector<std::string>& property_name,
                            ReferenceType ref_type,
                            bool return_immediate_value)
{
    std::string retval;
    retval.reserve(64);

    if (return_immediate_value)
        retval += "Value(";

    switch (ref_type) {
    case ReferenceType::SOURCE_REFERENCE:                    retval = "Source";          break;
    case ReferenceType::EFFECT_TARGET_REFERENCE:             retval = "Target";          break;
    case ReferenceType::EFFECT_TARGET_VALUE_REFERENCE:       retval = "Value";           break;
    case ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE: retval = "LocalCandidate";  break;
    case ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:  retval = "RootCandidate";   break;
    case ReferenceType::NON_OBJECT_REFERENCE:                retval = "";                break;
    default:                                                 retval = "?????";           break;
    }

    if (ref_type != ReferenceType::EFFECT_TARGET_VALUE_REFERENCE) {
        for (const std::string& property_name_part : property_name) {
            if (!retval.empty())
                retval += '.';
            retval += property_name_part;
        }
    }

    if (return_immediate_value)
        retval += ")";

    return retval;
}

std::string FormatedDescriptionPropertyNames(ReferenceType ref_type,
                                             const std::vector<std::string>& property_names,
                                             bool return_immediate_value)
{
    int num_references = property_names.size();
    if (ref_type == ReferenceType::NON_OBJECT_REFERENCE)
        num_references--;
    for (const std::string& property_name_part : property_names)
        if (property_name_part.empty())
             num_references--;
    num_references = std::max(0, num_references);
    const std::string& format_string{
        [num_references]() {
            switch (num_references) {
            case 0: return UserString("DESC_VALUE_REF_MULTIPART_VARIABLE0"); break;
            case 1: return UserString("DESC_VALUE_REF_MULTIPART_VARIABLE1"); break;
            case 2: return UserString("DESC_VALUE_REF_MULTIPART_VARIABLE2"); break;
            case 3: return UserString("DESC_VALUE_REF_MULTIPART_VARIABLE3"); break;
            case 4: return UserString("DESC_VALUE_REF_MULTIPART_VARIABLE4"); break;
            case 5: return UserString("DESC_VALUE_REF_MULTIPART_VARIABLE5"); break;
            case 6: return UserString("DESC_VALUE_REF_MULTIPART_VARIABLE6"); break;
            default:return UserString("DESC_VALUE_REF_MULTIPART_VARIABLEMANY"); break;
            }
        }()};

    boost::format formatter = FlexibleFormat(format_string);

    switch (ref_type) {
    case ReferenceType::SOURCE_REFERENCE:                    formatter % UserString("DESC_VAR_SOURCE");          break;
    case ReferenceType::EFFECT_TARGET_REFERENCE:             formatter % UserString("DESC_VAR_TARGET");          break;
    case ReferenceType::EFFECT_TARGET_VALUE_REFERENCE:       formatter % UserString("DESC_VAR_VALUE");           break;
    case ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE: formatter % UserString("DESC_VAR_LOCAL_CANDIDATE"); break;
    case ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:  formatter % UserString("DESC_VAR_ROOT_CANDIDATE");  break;
    case ReferenceType::NON_OBJECT_REFERENCE:                                                                    break;
    default:                                                 formatter % "???";                                  break;
    }

    for (const std::string& property_name_part : property_names) {
        if (property_name_part.empty())  // apparently is empty for a ReferenceType::EFFECT_TARGET_VALUE_REFERENCE
            continue;
        formatter % UserString("DESC_VAR_" + boost::to_upper_copy(property_name_part));
    }

    return boost::io::str(formatter);
}

std::string ComplexVariableDescription(const std::vector<std::string>& property_names,
                                       const ValueRef<int>* int_ref1,
                                       const ValueRef<int>* int_ref2,
                                       const ValueRef<int>* int_ref3,
                                       const ValueRef<std::string>* string_ref1,
                                       const ValueRef<std::string>* string_ref2)
{
    if (property_names.empty()) {
        ErrorLogger() << "ComplexVariableDescription passed empty property names?!";
        return "";
    }

    std::string stringtable_key("DESC_VAR_" + boost::to_upper_copy(property_names.back()));
    if (!UserStringExists(stringtable_key))
        return "";

    boost::format formatter = FlexibleFormat(UserString(stringtable_key));

    if (int_ref1)
        formatter % int_ref1->Description();
    if (int_ref2)
        formatter % int_ref2->Description();
    if (int_ref3)
        formatter % int_ref3->Description();
    if (string_ref1)
        formatter % string_ref1->Description();
    if (string_ref2)
        formatter % string_ref2->Description();

    return boost::io::str(formatter);
}

std::string ComplexVariableDump(const std::vector<std::string>& property_names,
                                const ValueRef<int>* int_ref1,
                                const ValueRef<int>* int_ref2,
                                const ValueRef<int>* int_ref3,
                                const ValueRef<std::string>* string_ref1,
                                const ValueRef<std::string>* string_ref2)
{
    if (property_names.empty()) {
        ErrorLogger() << "ComplexVariableDump passed empty property names?!";
        return "ComplexVariable";
    }
    std::string retval{property_names.back()};

    // TODO: Depending on the property name, the parameter names will vary.
    //       Need to handle each case correctly, in order for the Dumped
    //       text to be parsable as the correct ComplexVariable.

    if (int_ref1)
        retval += " int1 = " + int_ref1->Dump();
    if (int_ref2)
        retval += " int2 = " + int_ref2->Dump();
    if (int_ref3)
        retval += " int3 = " + int_ref3->Dump();
    if (string_ref1)
        retval += " string1 = " + string_ref1->Dump();
    if (string_ref2)
        retval += " string2 = " + string_ref2->Dump();

    return retval;
}

std::string StatisticDescription(StatisticType stat_type,
                                 const std::string& value_desc,
                                 const std::string& condition_desc)
{
    std::string stringtable_key("DESC_VAR_" + boost::to_upper_copy(
        boost::lexical_cast<std::string>(stat_type)));

    if (UserStringExists(stringtable_key)) {
        boost::format formatter = FlexibleFormat(stringtable_key);
        formatter % value_desc % condition_desc;
        return boost::io::str(formatter);
    }

    return UserString("DESC_VAR_STATISITIC");
}

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
template <>
std::string Constant<int>::Description() const
{
    if (std::abs(m_value) < 1000)
        return std::to_string(m_value);
    return DoubleToString(m_value, 3, false);
}

template <>
std::string Constant<double>::Description() const
{ return DoubleToString(m_value, 3, false); }

template <>
std::string Constant<std::string>::Description() const
{
    if (m_value == "CurrentContent")
        return m_top_level_content;
    return m_value;
}

template <>
std::string Constant<PlanetSize>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case PlanetSize::SZ_TINY:      return "Tiny";
    case PlanetSize::SZ_SMALL:     return "Small";
    case PlanetSize::SZ_MEDIUM:    return "Medium";
    case PlanetSize::SZ_LARGE:     return "Large";
    case PlanetSize::SZ_HUGE:      return "Huge";
    case PlanetSize::SZ_ASTEROIDS: return "Asteroids";
    case PlanetSize::SZ_GASGIANT:  return "GasGiant";
    default:                       return "?";
    }
}

template <>
std::string Constant<PlanetType>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case PlanetType::PT_SWAMP:      return "Swamp";
    case PlanetType::PT_TOXIC:      return "Toxic";
    case PlanetType::PT_INFERNO:    return "Inferno";
    case PlanetType::PT_RADIATED:   return "Radiated";
    case PlanetType::PT_BARREN:     return "Barren";
    case PlanetType::PT_TUNDRA:     return "Tundra";
    case PlanetType::PT_DESERT:     return "Desert";
    case PlanetType::PT_TERRAN:     return "Terran";
    case PlanetType::PT_OCEAN:      return "Ocean";
    case PlanetType::PT_ASTEROIDS:  return "Asteroids";
    case PlanetType::PT_GASGIANT:   return "GasGiant";
    default:            return "?";
    }
}

template <>
std::string Constant<PlanetEnvironment>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case PlanetEnvironment::PE_UNINHABITABLE: return "Uninhabitable";
    case PlanetEnvironment::PE_HOSTILE:       return "Hostile";
    case PlanetEnvironment::PE_POOR:          return "Poor";
    case PlanetEnvironment::PE_ADEQUATE:      return "Adequate";
    case PlanetEnvironment::PE_GOOD:          return "Good";
    default:                                  return "?";
    }
}

template <>
std::string Constant<UniverseObjectType>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case UniverseObjectType::OBJ_BUILDING:    return "Building";
    case UniverseObjectType::OBJ_SHIP:        return "Ship";
    case UniverseObjectType::OBJ_FLEET:       return "Fleet"; 
    case UniverseObjectType::OBJ_PLANET:      return "Planet";
    case UniverseObjectType::OBJ_POP_CENTER:  return "PopulationCenter";
    case UniverseObjectType::OBJ_PROD_CENTER: return "ProductionCenter";
    case UniverseObjectType::OBJ_SYSTEM:      return "System";
    case UniverseObjectType::OBJ_FIELD:       return "Field";
    default:                                  return "?";
    }
}

template <>
std::string Constant<StarType>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case StarType::STAR_BLUE:    return "Blue";
    case StarType::STAR_WHITE:   return "White";
    case StarType::STAR_YELLOW:  return "Yellow";
    case StarType::STAR_ORANGE:  return "Orange";
    case StarType::STAR_RED:     return "Red";
    case StarType::STAR_NEUTRON: return "Neutron";
    case StarType::STAR_BLACK:   return "BlackHole";
    case StarType::STAR_NONE:    return "NoStar";
    default:                     return "Unknown";
    }
}

template <>
std::string Constant<Visibility>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case Visibility::VIS_NO_VISIBILITY:      return "Invisible";
    case Visibility::VIS_BASIC_VISIBILITY:   return "Basic";
    case Visibility::VIS_PARTIAL_VISIBILITY: return "Partial";
    case Visibility::VIS_FULL_VISIBILITY:    return "Full";
    default:                                 return "Unknown";
    }
}

template <>
std::string Constant<int>::Dump(unsigned short ntabs) const
{ return std::to_string(m_value); }

template <>
std::string Constant<double>::Dump(unsigned short ntabs) const
{ return std::to_string(m_value); }

template <>
std::string Constant<std::string>::Dump(unsigned short ntabs) const
{ return "\"" + Description() + "\""; }

template <>
std::string Constant<std::string>::Eval(const ScriptingContext& context) const
{
    if (m_value == "CurrentContent")
        return m_top_level_content;
    return m_value;
}

template <>
void Constant<std::string>::SetTopLevelContent(const std::string& content_name)
{
    if (m_value == "CurrentContent" && content_name == "THERE_IS_NO_TOP_LEVEL_CONTENT")
        ErrorLogger() << "Constant<std::string>::SetTopLevelContent()  Scripted Content illegal. Trying to set THERE_IS_NO_TOP_LEVEL_CONTENT for CurrentContent (maybe you tried to use CurrentContent in named_values.focs.txt)";
    if (!m_top_level_content.empty()) // expected to happen if this value ref is part of a non-named-in-the-middle named value ref 
        DebugLogger() << "Constant<std::string>::SetTopLevelContent()  Skip overwriting top level content from '" << m_top_level_content << "' to '" << content_name << "'";
    else
        m_top_level_content = content_name;
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
#define IF_CURRENT_VALUE(T)                                            \
if (m_ref_type == ReferenceType::EFFECT_TARGET_VALUE_REFERENCE) {      \
    if (context.current_value.empty())                                 \
        throw std::runtime_error(                                      \
            "Variable<" #T ">::Eval(): Value could not be evaluated, " \
            "because no current value was provided.");                 \
    try {                                                              \
        return boost::get<T>(context.current_value);                   \
    } catch (const boost::bad_get&) {                                  \
        throw std::runtime_error(                                      \
            "Variable<" #T ">::Eval(): Value could not be evaluated, " \
            "because the provided current value is not an " #T ".");   \
    }                                                                  \
}

#define LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(T)                         \
ErrorLogger() << "Variable<" #T ">::Eval unrecognized object "         \
                 "property: "                                          \
              << TraceReference(m_property_name, m_ref_type, context); \
if (context.source)                                                    \
    ErrorLogger() << "source: " << context.source->ObjectType() << " " \
                  << context.source->ID() << " ( "                     \
                  << context.source->Name() << " ) ";                  \
else                                                                   \
    ErrorLogger() << "source (none)";

template <>
PlanetSize Variable<PlanetSize>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(PlanetSize)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<PlanetSize>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        return PlanetSize::INVALID_PLANET_SIZE;
    }

    std::function<PlanetSize (const Planet&)> planet_property{nullptr};

    if (property_name == "PlanetSize")
        planet_property = &Planet::Size;
    else if (property_name == "NextLargerPlanetSize")
        planet_property = &Planet::NextLargerPlanetSize;
    else if (property_name == "NextSmallerPlanetSize")
        planet_property = &Planet::NextSmallerPlanetSize;

    if (planet_property) {
        if (auto p = std::dynamic_pointer_cast<const Planet>(object))
            return planet_property(*p);
        return PlanetSize::INVALID_PLANET_SIZE;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(PlanetSize)

    return PlanetSize::INVALID_PLANET_SIZE;
}

template <>
PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(PlanetType)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<PlanetType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        return PlanetType::INVALID_PLANET_TYPE;
    }

    std::function<PlanetType (const Planet&)> planet_property{nullptr};

    if (property_name == "PlanetType")
        planet_property = &Planet::Type;
    else if (property_name == "OriginalType")
        planet_property = &Planet::OriginalType;
    else if (property_name == "NextCloserToOriginalPlanetType")
        planet_property = &Planet::NextCloserToOriginalPlanetType;
    else if (property_name == "NextBetterPlanetType")
        planet_property = boost::bind(&Planet::NextBetterPlanetTypeForSpecies, boost::placeholders::_1, "");
    else if (property_name == "ClockwiseNextPlanetType")
        planet_property = &Planet::ClockwiseNextPlanetType;
    else if (property_name == "CounterClockwiseNextPlanetType")
        planet_property = &Planet::CounterClockwiseNextPlanetType;

    if (planet_property) {
        if (auto p = std::dynamic_pointer_cast<const Planet>(object))
            return planet_property(*p);
        return PlanetType::INVALID_PLANET_TYPE;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(PlanetType)

    return PlanetType::INVALID_PLANET_TYPE;
}

template <>
PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(PlanetEnvironment)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    if (property_name == "PlanetEnvironment") {
        auto object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<PlanetEnvironment>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
        }
        if (auto p = std::dynamic_pointer_cast<const Planet>(object))
            return p->EnvironmentForSpecies();

        return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(PlanetEnvironment)

    return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
}

template <>
UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(UniverseObjectType)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    if (property_name == "ObjectType") {
        auto object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<UniverseObjectType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE;
        }
        ObjectTypeVisitor v;
        if (object->Accept(v))
            return v.m_type;
        else if (std::dynamic_pointer_cast<const PopCenter>(object))
            return UniverseObjectType::OBJ_POP_CENTER;
        else if (std::dynamic_pointer_cast<const ResourceCenter>(object))
            return UniverseObjectType::OBJ_PROD_CENTER;

        return UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(UniverseObjectType)

    return UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE;
}

template <>
StarType Variable<StarType>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(StarType)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<StarType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        return StarType::INVALID_STAR_TYPE;
    }

    std::function<StarType (const System&)> system_property{nullptr};

    if (property_name == "StarType")
        system_property = &System::GetStarType;
    else if (property_name == "NextOlderStarType")
        system_property = &System::NextOlderStarType;
    else if (property_name == "NextYoungerStarType")
        system_property = &System::NextYoungerStarType;

    if (system_property) {
        if (auto s = std::dynamic_pointer_cast<const System>(object))
            return system_property(*s);
        return StarType::INVALID_STAR_TYPE;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(StarType)

    return StarType::INVALID_STAR_TYPE;
}

template <>
Visibility Variable<Visibility>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(Visibility)

    // As of this writing, there are no properties of objects that directly
    // return a Visibility, as it will normally need to be queried for a
    // particular empire

    ErrorLogger() << "Variable<Visibility>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);

    return Visibility::INVALID_VISIBILITY;
}

template <>
double Variable<double>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(double)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        if ((property_name == "UniverseCentreX") ||
            (property_name == "UniverseCentreY"))
        {
            return context.ContextUniverse().UniverseWidth() / 2;   // TODO: get Universe from ScriptingContext
        } else if (property_name == "UniverseWidth") {
            return context.ContextUniverse().UniverseWidth();
        }

        // add more non-object reference double functions here

        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(float)

        return 0.0;
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(float)

        return 0.0;
    }

    MeterType meter_type = NameToMeter(property_name);
    if (object && meter_type != MeterType::INVALID_METER_TYPE) {
        if (auto* m = object->GetMeter(meter_type))
            return m_return_immediate_value ? m->Current() : m->Initial();
        return 0.0;

    } else if (property_name == "X") {
        return object->X();

    } else if (property_name == "Y") {
        return object->Y();

    }

    std::function<double (const Planet&)> planet_property{nullptr};

    if (property_name == "SizeAsDouble")
        planet_property = [](const Planet& planet) -> double { return static_cast<double>(planet.Size()); };
    else if (property_name == "HabitableSize")
        planet_property = &Planet::HabitableSize;
    else if (property_name == "DistanceFromOriginalType")
        planet_property = &Planet::DistanceFromOriginalType;

    if (planet_property) {
        if (auto planet = dynamic_cast<const Planet*>(object.get()))
            return planet_property(*planet);
        return 0.0;

    }

    if (property_name == "CombatBout") {
        return context.combat_bout;

    } else if (property_name == "CurrentTurn") {
        return context.current_turn;

    } else if (property_name == "Attack") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->Damage(context.ContextObjects());
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->TotalWeaponsDamage();
        if (auto fighter = std::dynamic_pointer_cast<const Fighter>(object))
            return fighter->Damage();
        return 0.0;

    } else if (property_name == "PropagatedSupplyRange") {
        const auto& ranges = GetSupplyManager().PropagatedSupplyRanges();   // TODO: Get from Context..
        auto range_it = ranges.find(object->SystemID());
        if (range_it == ranges.end())
            return 0.0;
        return range_it->second;

    } else if (property_name == "PropagatedSupplyDistance") {
        const auto& ranges = GetSupplyManager().PropagatedSupplyDistances(); // TODO: get from context
        auto range_it = ranges.find(object->SystemID());
        if (range_it == ranges.end())
            return 0.0;
        return range_it->second;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(float)

    return 0.0;
}

template <>
int Variable<int>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(int)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        if (property_name == "CombatBout")
            return context.combat_bout;
        if (property_name == "CurrentTurn")
            return context.current_turn;
        if (property_name == "GalaxySize")
            return context.galaxy_setup_data.GetSize();
        if (property_name == "GalaxyShape")
            return static_cast<int>(context.galaxy_setup_data.GetShape());
        if (property_name == "GalaxyAge")
            return static_cast<int>(context.galaxy_setup_data.GetAge());
        if (property_name == "GalaxyStarlaneFrequency")
            return static_cast<int>(context.galaxy_setup_data.GetStarlaneFreq());
        if (property_name == "GalaxyPlanetDensity")
            return static_cast<int>(context.galaxy_setup_data.GetPlanetDensity());
        if (property_name == "GalaxySpecialFrequency")
            return static_cast<int>(context.galaxy_setup_data.GetSpecialsFreq());
        if (property_name == "GalaxyMonsterFrequency")
            return static_cast<int>(context.galaxy_setup_data.GetMonsterFreq());
        if (property_name == "GalaxyNativeFrequency")
            return static_cast<int>(context.galaxy_setup_data.GetNativeFreq());
        if (property_name == "GalaxyMaxAIAggression")
            return static_cast<int>(context.galaxy_setup_data.GetAggression());

        // non-object values passed by abuse of context.current_value
        if (property_name == "UsedInDesignID") {
            // check if an int was passed as the current_value, as would be
            // done when evaluating a ValueRef for the cost or production
            // time of a part or hull in a ship design. this should be the id
            // of the design.
            try {
                return boost::get<int>(context.current_value);
            } catch (...) {
                ErrorLogger() << "Variable<int>::Eval could get ship design id for property: " << TraceReference(m_property_name, m_ref_type, context);
            }
            return 0;
        }

        // add more non-object reference int functions here

        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(int)

        return 0;
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(int)

        return 0;
    }

    if (property_name == "Owner") {
        return object->Owner();
    }
    else if (property_name == "SystemID") {
        return object->SystemID();

    }
    else if (property_name == "ContainerID") {
        return object->ContainerObjectID();

    }
    else if (property_name == "SupplyingEmpire") {
        return GetSupplyManager().EmpireThatCanSupplyAt(object->SystemID()); // TODO: Get SupplyManager from Context
    }
    else if (property_name == "ID") {
        return object->ID();
    }
    else if (property_name == "CreationTurn") {
        return object->CreationTurn();
    }
    else if (property_name == "Age") {
        return object->AgeInTurns();

    }

    std::function<int (const Ship&)> ship_property{nullptr};

    if (property_name == "ArrivedOnTurn")
        ship_property = &Ship::ArrivedOnTurn;
    else if (property_name == "LastTurnActiveInBattle")
        ship_property = &Ship::LastTurnActiveInCombat;
    else if (property_name == "LastTurnResupplied")
        ship_property = &Ship::LastResuppliedOnTurn;

    if (ship_property) {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship_property(*ship);
        return INVALID_GAME_TURN;
    }

    std::function<int (const Fleet&)> fleet_property{nullptr};

    if (property_name == "FinalDestinationID")
        fleet_property = &Fleet::FinalDestinationID;
    else if (property_name == "NextSystemID")
        fleet_property = &Fleet::NextSystemID;
    else if (property_name == "PreviousSystemID")
        fleet_property = &Fleet::PreviousSystemID;
    else if (property_name == "ArrivalStarlaneID")
        fleet_property = &Fleet::ArrivalStarlane;
    else if (property_name == "LastTurnMoveOrdered")
        fleet_property = &Fleet::LastTurnMoveOrdered;

    if (fleet_property) {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet_property(*fleet);
        return INVALID_OBJECT_ID;
    }

    std::function<int (const Planet&)> planet_property{nullptr};

    if (property_name == "LastTurnAttackedByShip")
        planet_property = &Planet::LastTurnAttackedByShip;
    else if (property_name == "LastTurnColonized")
        planet_property = &Planet::LastTurnColonized;
    else if (property_name == "LastTurnConquered")
        planet_property = &Planet::LastTurnConquered;

    if (planet_property) {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet_property(*planet);
        return INVALID_GAME_TURN;
    }

    if (property_name == "TurnsSinceFocusChange") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->TurnsSinceFocusChange();
        return 0;

    }
    else if (property_name == "ProducedByEmpireID") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->ProducedByEmpireID();
        else if (auto building = std::dynamic_pointer_cast<const Building>(object))
            return building->ProducedByEmpireID();
        return ALL_EMPIRES;

    }
    else if (property_name == "DesignID") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->DesignID();
        return INVALID_DESIGN_ID;

    }
    else if (property_name == "FleetID") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->FleetID();
        else if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->ID();
        return INVALID_OBJECT_ID;

    }
    else if (property_name == "PlanetID") {
        if (auto building = std::dynamic_pointer_cast<const Building>(object))
            return building->PlanetID();
        else if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->ID();
        return INVALID_OBJECT_ID;

    }
    else if (property_name == "NearestSystemID") {
        if (object->SystemID() != INVALID_OBJECT_ID)
            return object->SystemID();
        return GetUniverse().GetPathfinder()->NearestSystemTo(object->X(), object->Y(), context.ContextObjects());  // TODO: Get PathFinder from ScriptingContext

    }
    else if (property_name == "NumShips") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->NumShips();
        return 0;

    }
    else if (property_name == "NumStarlanes") {
        if (auto system = std::dynamic_pointer_cast<const System>(object))
            return system->NumStarlanes();
        return 0;

    }
    else if (property_name == "LastTurnBattleHere") {
        if (auto const_system = std::dynamic_pointer_cast<const System>(object))
            return const_system->LastTurnBattleHere();
        else if (auto system = context.ContextObjects().get<System>(object->SystemID()))
            return system->LastTurnBattleHere();
        return INVALID_GAME_TURN;

    }
    else if (property_name == "Orbit") {
        if (auto system = context.ContextObjects().get<System>(object->SystemID()))
            return system->OrbitOfPlanet(object->ID());
        return -1;

    }
    else if (property_name == "ETA") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->ETA(context).first;
        return 0;

    }
    else if (property_name == "NumSpecials") {
        return object->Specials().size();

    }
    else if (property_name == "LaunchedFrom") {
        if (auto fighter = std::dynamic_pointer_cast<const Fighter>(object))
            return fighter->LaunchedFrom();
        return INVALID_OBJECT_ID;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(int)

    return 0;
}

template <>
std::vector<std::string> Variable<std::vector<std::string>>::Eval(
    const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(std::vector<std::string>)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        // add more non-object reference string vector functions here
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::vector<std::string>)

        return {};
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::vector<std::string>)

        return {};
    }

    if (property_name == "Tags") {
        auto tags = object->Tags();
        return {tags.begin(), tags.end()};
    }
    else if (property_name == "Specials") {
        auto obj_special_names_range = object->Specials() | boost::adaptors::map_keys;
        return {obj_special_names_range.begin(), obj_special_names_range.end()};
    }
    else if (property_name == "AvailableFoci") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->AvailableFoci();
        return {};
    }
    else if (property_name == "Parts") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            if (const ShipDesign* design = ship->Design())
                return design->Parts();
        return {};
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::vector<std::string>)

    return {};
}

template <>
std::string Variable<std::string>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(std::string)

    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        if (property_name == "GalaxySeed")
            return context.galaxy_setup_data.GetSeed();

        // add more non-object reference string functions here
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::string)

        return "";
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::string)

        return "";
    }

    if (property_name == "Name") {
        return object->Name();

    } else if (property_name == "OwnerName") {
        int owner_empire_id = object->Owner();
        if (auto empire = context.GetEmpire(owner_empire_id))
            return empire->Name();
        return "";

    } else if (property_name == "TypeName") {
        return boost::lexical_cast<std::string>(object->ObjectType());

    }

    std::function<std::string (const Empire&)> empire_property{nullptr};

    if (property_name == "OwnerLeastExpensiveEnqueuedTech")
        empire_property = &Empire::LeastExpensiveEnqueuedTech;
    else if (property_name == "OwnerMostExpensiveEnqueuedTech")
        empire_property = &Empire::MostExpensiveEnqueuedTech;
    else if (property_name == "OwnerMostRPCostLeftEnqueuedTech")
        empire_property = &Empire::MostRPCostLeftEnqueuedTech;
    else if (property_name == "OwnerMostRPSpentEnqueuedTech")
        empire_property = &Empire::MostRPSpentEnqueuedTech;
    else if (property_name == "OwnerTopPriorityEnqueuedTech")
        empire_property = &Empire::TopPriorityEnqueuedTech;

    if (empire_property) {
        auto empire = context.GetEmpire(object->Owner());
        if (!empire)
            return "";
        return empire_property(*empire);
    }

    if (property_name == "Species") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->SpeciesName();
        else if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->SpeciesName();
        else if (auto fighter = std::dynamic_pointer_cast<const Fighter>(object))
            return fighter->SpeciesName();
        return "";

    } else if (property_name == "Hull") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            if (const ShipDesign* design = ship->Design())
                return design->Hull();
        return "";

    } else if (property_name == "FieldType") {
        if (auto field = std::dynamic_pointer_cast<const Field>(object))
            return field->FieldTypeName();
        return "";

    } else if (property_name == "BuildingType") {
        if (auto building = std::dynamic_pointer_cast<const Building>(object))
            return building->BuildingTypeName();
        return "";

    } else if (property_name == "Focus") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->Focus();
        return "";

    } else if (property_name == "DefaultFocus") {
        const Species* species = nullptr;
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object)) {
            species = GetSpecies(planet->SpeciesName());
        } else if (auto ship = std::dynamic_pointer_cast<const Ship>(object)) {
            species = GetSpecies(ship->SpeciesName());
        }
        if (species)
            return species->DefaultFocus();
        return "";

    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::string)

    return "";
}

#undef IF_CURRENT_VALUE

///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////
template <>
std::string Statistic<std::string, std::string>::Eval(const ScriptingContext& context) const
{
    Condition::ObjectSet condition_matches;
    GetConditionMatches(context, condition_matches, m_sampling_condition.get());

    if (condition_matches.empty())
        return "";  // empty string

    // special case for IF statistic... return a non-empty string for true
    if (m_stat_type == StatisticType::IF)
        return " "; // not an empty string

    // todo: consider allowing MAX and MIN using string sorting?

    // the only other statistic that can be computed on non-number property
    // types and that is itself of a non-number type is the most common value
    if (m_stat_type != StatisticType::MODE) {
        ErrorLogger() << "Statistic<std::string, std::string>::Eval has invalid statistic type: "
                      << m_stat_type;
        return "";
    }

    // evaluate property for each condition-matched object
    std::vector<std::string> object_property_values;
    GetObjectPropertyValues(context, condition_matches, object_property_values);

    // value that appears the most often
    std::map<std::string, unsigned int> observed_values;
    for (auto& entry : object_property_values)
        observed_values[std::move(entry)]++;

    auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                [](auto p1, auto p2) { return p1.second < p2.second; });

    return max->first;
}

///////////////////////////////////////////////////////////
// ComplexVariable                                       //
///////////////////////////////////////////////////////////
template <>
PlanetSize ComplexVariable<PlanetSize>::Eval(const ScriptingContext& context) const
{ return PlanetSize::INVALID_PLANET_SIZE; }

template <>
PlanetType ComplexVariable<PlanetType>::Eval(const ScriptingContext& context) const
{ return PlanetType::INVALID_PLANET_TYPE; } // TODO: Species favourite planet type?

template <>
PlanetEnvironment ComplexVariable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    if (variable_name == "PlanetEnvironmentForSpecies") {
        int planet_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            planet_id = m_int_ref1->Eval(context);
        const auto planet = context.ContextObjects().get<Planet>(planet_id);
        if (!planet)
            return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;

        std::string species_name;
        if (m_string_ref1)
            species_name = m_string_ref1->Eval(context);
        return planet->EnvironmentForSpecies(species_name);
    }

    return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
}

template <>
UniverseObjectType ComplexVariable<UniverseObjectType>::Eval(const ScriptingContext& context) const
{ return UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE; }

template <>
StarType ComplexVariable<StarType>::Eval(const ScriptingContext& context) const
{ return StarType::INVALID_STAR_TYPE; }

template <>
Visibility ComplexVariable<Visibility>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    if (variable_name == "EmpireObjectVisibility") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return Visibility::VIS_NO_VISIBILITY;
        }

        int object_id = INVALID_OBJECT_ID;
        if (m_int_ref2) {
            object_id = m_int_ref2->Eval(context);
            if (object_id == INVALID_OBJECT_ID)
                return Visibility::VIS_NO_VISIBILITY;
        }

        return GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id);
    }

    return Visibility::INVALID_VISIBILITY;
}

template <>
int ComplexVariable<int>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    std::function<const std::map<std::string, int>& (const Empire&)> empire_property_string_key{nullptr};

    if (variable_name == "BuildingTypesOwned")
        empire_property_string_key = &Empire::BuildingTypesOwned;
    if (variable_name == "BuildingTypesProduced")
        empire_property_string_key = &Empire::BuildingTypesProduced;
    if (variable_name == "BuildingTypesScrapped")
        empire_property_string_key = &Empire::BuildingTypesScrapped;
    if (variable_name == "SpeciesColoniesOwned")
        empire_property_string_key = &Empire::SpeciesColoniesOwned;
    if (variable_name == "SpeciesPlanetsBombed")
        empire_property_string_key = &Empire::SpeciesPlanetsBombed;
    if (variable_name == "SpeciesPlanetsDepoped")
        empire_property_string_key = &Empire::SpeciesPlanetsDepoped;
    if (variable_name == "SpeciesPlanetsInvaded")
        empire_property_string_key = &Empire::SpeciesPlanetsInvaded;
    if (variable_name == "SpeciesShipsDestroyed")
        empire_property_string_key = &Empire::SpeciesShipsDestroyed;
    if (variable_name == "SpeciesShipsLost")
        empire_property_string_key = &Empire::SpeciesShipsLost;
    if (variable_name == "SpeciesShipsOwned")
        empire_property_string_key = &Empire::SpeciesShipsOwned;
    if (variable_name == "SpeciesShipsProduced")
        empire_property_string_key = &Empire::SpeciesShipsProduced;
    if (variable_name == "SpeciesShipsScrapped")
        empire_property_string_key = &Empire::SpeciesShipsScrapped;
    if (variable_name == "ShipPartsOwned")
        empire_property_string_key = &Empire::ShipPartsOwned;
    if (variable_name == "TurnTechResearched")
        empire_property_string_key = &Empire::ResearchedTechs;

    // empire properties indexed by strings
    if (empire_property_string_key) {
        using namespace boost::adaptors;

        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
        }

        std::function<bool (const std::map<std::string, int>::value_type&)> key_filter{nullptr};
        key_filter = [](auto e){ return true; };

        if (m_string_ref1) {
            std::string key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;

            if (empire && variable_name == "TurnTechResearched" && !empire->TechResearched(key_string))
                // special case for techs: make unresearched-tech's research-turn a big number
                return IMPOSSIBLY_LARGE_TURN;

            key_filter = [k{std::move(key_string)}](auto e){ return k == e.first; };
        }
        else if (variable_name == "ShipPartsOwned" && m_int_ref2) {
            int key_int = m_int_ref2->Eval(context);
            if (key_int <= int(ShipPartClass::INVALID_SHIP_PART_CLASS) ||
                key_int >= int(ShipPartClass::NUM_SHIP_PART_CLASSES))
            { return 0; }

            auto key_filter = [part_class = ShipPartClass(key_int)](const std::map<ShipPartClass, int>::value_type& e){ return e.first == part_class; };

            if (empire)
                return boost::accumulate(empire->ShipPartClassOwned() | filtered(key_filter) | map_values, 0);

            int sum = 0;
            for ([[maybe_unused]] auto& [ignored_id, empire] : context.Empires()) {
                (void)ignored_id; // quiet unused variable warning
                sum += boost::accumulate(empire->ShipPartClassOwned() | filtered(key_filter) | map_values, 0);
            }
            return sum;
        }

        if (empire)
            return boost::accumulate(empire_property_string_key(*empire) | filtered(key_filter) | map_values, 0);

        int sum = 0;
        for ([[maybe_unused]] auto& [ignored_id, empire] : context.Empires()) {
            (void)ignored_id; // quiet unused variable warning
            sum += boost::accumulate(empire_property_string_key(*empire) | filtered(key_filter) | map_values, 0);
        }
        return sum;
    }

    std::function<const std::map<int, int>& (const Empire&)> empire_property_int_key{nullptr};

    if (variable_name == "EmpireShipsDestroyed")
        empire_property_int_key = &Empire::EmpireShipsDestroyed;
    if (variable_name == "ShipDesignsDestroyed")
        empire_property_int_key = &Empire::ShipDesignsDestroyed;
    if (variable_name == "ShipDesignsLost")
        empire_property_int_key = &Empire::ShipDesignsLost;
    if (variable_name == "ShipDesignsOwned")
        empire_property_int_key = &Empire::ShipDesignsOwned;
    if (variable_name == "ShipDesignsInProduction")
        empire_property_int_key = &Empire::ShipDesignsInProduction;
    if (variable_name == "ShipDesignsProduced")
        empire_property_int_key = &Empire::ShipDesignsProduced;
    if (variable_name == "ShipDesignsScrapped")
        empire_property_int_key = &Empire::ShipDesignsScrapped;

    // empire properties indexed by integers
    if (empire_property_int_key) {
        using namespace boost::adaptors;

        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
        }

        std::function<bool (const std::map<int, int>::value_type&)> key_filter{nullptr};
        key_filter = [](auto e){ return true; };

        // if a key integer specified, get just that entry (for single empire or sum of all empires)
        if (m_int_ref2)
            key_filter = [k = m_int_ref2->Eval(context)](auto e){ return k == e.first; };

        // although indexed by integers, some of these may be specified by a
        // string that needs to be looked up. if a key string specified, get
        // just that entry (for single empire or sum of all empires)
        if (m_string_ref1) {
            std::string key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;
            int key_int = -1;
            if (boost::istarts_with(variable_name, "ShipDesign")) {
                // look up ship design id corresponding to specified predefined ship design name
                const ShipDesign* design = GetPredefinedShipDesign(key_string);
                if (design)
                    key_int = design->ID();
            }
            key_filter = [k = key_int](auto e){ return k == e.first; };
        }

        if (empire)
            return boost::accumulate(empire_property_int_key(*empire) | filtered(key_filter) | map_values, 0);

        int sum = 0;
        for ([[maybe_unused]] auto& [ignored_id, empire] : context.Empires()) {
            (void)ignored_id; // quiet unused variable warning
            sum += boost::accumulate(empire_property_int_key(*empire) | filtered(key_filter) | map_values, 0);
        }
        return sum;
    }

    // unindexed empire proprties
    if (variable_name == "OutpostsOwned") {
        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
            if (!empire)
                return 0;
        }

        std::function<int (const Empire*)> empire_property{nullptr};
        empire_property = &Empire::OutpostsOwned;

        using namespace boost::adaptors;
        auto GetRawPtr = [](const auto& smart_ptr){ return smart_ptr.get(); };

        if (!empire) {
            return boost::accumulate(context.Empires() | map_values | transformed(GetRawPtr) |
                                     transformed(empire_property), 0);
        }

        return empire_property(empire.get());
    }

    // non-empire properties
    if (variable_name == "GameRule") {
        if (!m_string_ref1)
            return 0;
        std::string rule_name = m_string_ref1->Eval();
        if (rule_name.empty())
            return 0;
        if (!GetGameRules().RuleExists(rule_name))
            return 0;
        try {
            // can cast boolean, int, or double-valued rules to int
            switch (GetGameRules().GetType(rule_name)) {
            case GameRules::Type::TOGGLE: {
                return GetGameRules().Get<bool>(rule_name);
                break;
            }
            case GameRules::Type::INT: {
                return GetGameRules().Get<int>(rule_name);
                break;
            }
            case GameRules::Type::DOUBLE: {
                return static_cast<int>(GetGameRules().Get<double>(rule_name));
                break;
            }
            default:
                break;
            }
        } catch (...) {
        }
        return 0;
    }
    else if (variable_name == "PartsInShipDesign") {
        int design_id = INVALID_DESIGN_ID;
        if (m_int_ref1) {
            design_id = m_int_ref1->Eval(context);
            if (design_id == INVALID_DESIGN_ID)
                return 0;
        } else {
            return 0;
        }

        std::string ship_part_name;
        if (m_string_ref1)
            ship_part_name = m_string_ref1->Eval(context);

        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return 0;

        if (ship_part_name.empty())
            return design->PartCount();

        int count = 0;
        for (const std::string& part : design->Parts()) {
            if (ship_part_name == part)
                count++;
        }
        return count;
    }
    else if (variable_name == "PartOfClassInShipDesign") {
        int design_id = INVALID_DESIGN_ID;
        if (m_int_ref1) {
            design_id = m_int_ref1->Eval(context);
            if (design_id == INVALID_DESIGN_ID)
                return 0;
        } else {
            return 0;
        }

        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return 0;

        std::string part_class_name;
        if (m_string_ref1) {
            part_class_name = m_string_ref1->Eval(context);
        } else {
            return 0;
        }
        ShipPartClass part_class = ShipPartClass::INVALID_SHIP_PART_CLASS;
        try {
            part_class = boost::lexical_cast<ShipPartClass>(part_class_name);
        } catch (...) {
            return 0;
        }

        int count = 0;
        for (const std::string& part_name : design->Parts()) {
            if (part_name.empty())
                continue;
            const ShipPart* part = GetShipPart(part_name);
            if (!part)
                continue;
            if (part->Class() == part_class)
                count++;
        }
        return count;
    }
    else if (variable_name == "JumpsBetween") {
        int object1_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object1_id = m_int_ref1->Eval(context);

        int object2_id = INVALID_OBJECT_ID;
        if (m_int_ref2)
            object2_id = m_int_ref2->Eval(context);

        int retval = context.ContextUniverse().GetPathfinder()->JumpDistanceBetweenObjects(
            object1_id, object2_id, context.ContextObjects());
        if (retval == INT_MAX)
            return -1;
        return retval;
    }
    else if (variable_name == "JumpsBetweenByEmpireSupplyConnections") {
        int object1_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object1_id = m_int_ref1->Eval(context);

        int object2_id = INVALID_OBJECT_ID;
        if (m_int_ref2)
            object2_id = m_int_ref2->Eval(context);

        // TODO: implement supply-connect-restriction path length determination...
        // in the meantime, leave empire_id commented out to avoid unused var warning
        //int empire_id = ALL_EMPIRES;
        //if (m_int_ref3)
        //    empire_id = m_int_ref3->Eval(context);

        int retval = context.ContextUniverse().GetPathfinder()->JumpDistanceBetweenObjects(
            object1_id, object2_id, context.ContextObjects());
        if (retval == INT_MAX)
            return -1;
        return retval;
    }
    else if (variable_name == "SlotsInHull") {
        const ShipHull* ship_hull = nullptr;
        if (m_string_ref1) {
            std::string hull_name = m_string_ref1->Eval(context);
            ship_hull = GetShipHull(hull_name);
            if (!ship_hull)
                return 0;
        } else {
            return 0;
        }
        return ship_hull->Slots().size();
    }
    else if (variable_name == "SlotsInShipDesign") {
        int design_id = INVALID_DESIGN_ID;
        if (m_int_ref1) {
            design_id = m_int_ref1->Eval(context);
            if (design_id == INVALID_DESIGN_ID)
                return 0;
        } else {
            return 0;
        }

        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return 0;

        const ShipHull* ship_hull = GetShipHull(design->Hull());
        if (!ship_hull)
            return 0;
        return ship_hull->Slots().size();
    }
    else if (variable_name == "SpecialAddedOnTurn") {
        int object_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object_id = m_int_ref1->Eval(context);
        if (object_id == INVALID_OBJECT_ID)
            return 0;
        auto object = context.ContextObjects().get(object_id);
        if (!object)
            return 0;

        std::string special_name;
        if (m_string_ref1)
            special_name = m_string_ref1->Eval(context);
        if (special_name.empty())
            return 0;

        return object->SpecialAddedOnTurn(special_name);

    }
    else if (variable_name == "TurnPolicyAdopted") {
        if (!m_string_ref1)
            return 0;
        std::string policy_name = m_string_ref1->Eval();
        if (policy_name.empty())
            return 0;

        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return 0;

        return empire->TurnPolicyAdopted(policy_name);
    }

    return 0;
}

template <>
double ComplexVariable<double>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    std::function<float (const ShipHull&)> hull_property{nullptr};

    if (variable_name == "HullFuel")
        hull_property = &ShipHull::Fuel;
    else if (variable_name == "HullStealth")
        hull_property = &ShipHull::Stealth;
    else if (variable_name == "HullStructure")
        hull_property = &ShipHull::Structure;
    else if (variable_name == "HullSpeed")
        hull_property = &ShipHull::Speed;

    if (hull_property) {
        std::string ship_hull_name;
        if (m_string_ref1)
            ship_hull_name = m_string_ref1->Eval(context);

        const ShipHull* ship_hull = GetShipHull(ship_hull_name);
        if (!ship_hull)
            return 0.0f;

        return hull_property(*ship_hull);
    }

    // empire properties indexed by integers
    std::function<const std::map<int, float>& (const Empire&)> empire_property{nullptr};

    if (variable_name == "PropagatedSystemSupplyRange")
        empire_property = [](const Empire& empire){ return GetSupplyManager().PropagatedSupplyRanges(empire.EmpireID()); };
    if (variable_name == "SystemSupplyRange")
        empire_property = &Empire::SystemSupplyRanges;
    if (variable_name == "PropagatedSystemSupplyDistance")
        empire_property = [](const Empire& empire){ return GetSupplyManager().PropagatedSupplyDistances(empire.EmpireID()); };

    if (empire_property) {
        using namespace boost::adaptors;

        std::shared_ptr<const Empire> empire;

        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0.0;
            empire = context.GetEmpire(empire_id);
        }

        std::function<bool (const std::map<int, float>::value_type&)> key_filter;
        key_filter = [](auto k){ return true; };

        if (m_int_ref2)
            key_filter = [k = m_int_ref2->Eval(context)](auto e){ return k == e.first; };

        if (empire)
            return boost::accumulate(empire_property(*empire) | filtered(key_filter) | map_values, 0.0f);

        float sum = 0.0f;
        for ([[maybe_unused]] auto& [unused_id, loop_empire] : context.Empires()) {
            (void)unused_id; // quiet unused variable warning
            sum += boost::accumulate(empire_property(*loop_empire) | filtered(key_filter) | map_values, 0.0f);
        }
        return sum;
    }

    // non-empire properties
    if (variable_name == "GameRule") {
        if (!m_string_ref1)
            return 0.0;
        std::string rule_name = m_string_ref1->Eval();
        if (rule_name.empty())
            return 0.0;
        if (!GetGameRules().RuleExists(rule_name))
            return 0.0;
        try {
            // can cast boolean, int, or double-valued rules to double
            switch (GetGameRules().GetType(rule_name)) {
            case GameRules::Type::TOGGLE: {
                return GetGameRules().Get<bool>(rule_name);
                break;
            }
            case GameRules::Type::INT: {
                return GetGameRules().Get<int>(rule_name);
                break;
            }
            case GameRules::Type::DOUBLE: {
                return GetGameRules().Get<double>(rule_name);
                break;
            }
            default:
                break;
            }
        } catch (...) {
        }
        return 0.0;
    }
    else if (variable_name == "PartCapacity") {
        std::string ship_part_name;
        if (m_string_ref1)
            ship_part_name = m_string_ref1->Eval(context);

        const ShipPart* ship_part = GetShipPart(ship_part_name);
        if (!ship_part)
            return 0.0;

        return ship_part->Capacity();

    }
    else if (variable_name == "PartSecondaryStat") {
        std::string ship_part_name;
        if (m_string_ref1)
            ship_part_name = m_string_ref1->Eval(context);

        const ShipPart* ship_part = GetShipPart(ship_part_name);
        if (!ship_part)
            return 0.0;

        return ship_part->SecondaryStat();

    }
    else if (variable_name == "ShipDesignCost") {
        int design_id = INVALID_DESIGN_ID;
        if (m_int_ref1)
            design_id = m_int_ref1->Eval(context);

        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return 0.0;

        int empire_id = ALL_EMPIRES;
        if (m_int_ref2)
            empire_id = m_int_ref2->Eval(context);

        int location_id = INVALID_OBJECT_ID;
        if (m_int_ref3)
            location_id = m_int_ref3->Eval(context);

        return design->ProductionCost(empire_id, location_id);

    }
    else if (variable_name == "EmpireMeterValue") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1)
            empire_id = m_int_ref1->Eval(context);
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return 0.0;

        std::string empire_meter_name;
        if (m_string_ref1)
            empire_meter_name = m_string_ref1->Eval(context);
        auto meter = empire->GetMeter(empire_meter_name);
        if (!meter)
            return 0.0;
        return meter->Current();
    }
    else if (variable_name == "DirectDistanceBetween") {
        int object1_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object1_id = m_int_ref1->Eval(context);
        auto obj1 = context.ContextObjects().get(object1_id);
        if (!obj1)
            return 0.0;

        int object2_id = INVALID_OBJECT_ID;
        if (m_int_ref2)
            object2_id = m_int_ref2->Eval(context);
        auto obj2 = context.ContextObjects().get(object2_id);
        if (!obj2)
            return 0.0;

        double dx = obj2->X() - obj1->X();
        double dy = obj2->Y() - obj1->Y();
        return static_cast<float>(std::sqrt(dx*dx + dy*dy));

    }
    else if (variable_name == "ShortestPath") {
        int object1_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object1_id = m_int_ref1->Eval(context);

        int object2_id = INVALID_OBJECT_ID;
        if (m_int_ref2)
            object2_id = m_int_ref2->Eval(context);

        return GetUniverse().GetPathfinder()->ShortestPathDistance(object1_id, object2_id, context.ContextObjects()); // TODO: Get PathFinder from ScriptingContext

    }
    else if (variable_name == "SpeciesContentOpinion") {
        std::string opinionated_species_name;
        if (m_string_ref1)
            opinionated_species_name = m_string_ref1->Eval(context);
        const auto species = GetSpecies(opinionated_species_name);
        if (!species)
            return 0.0;

        std::string liked_or_disliked_content_name;
        if (m_string_ref2)
            liked_or_disliked_content_name = m_string_ref2->Eval(context);
        if (liked_or_disliked_content_name.empty())
            return 0.0;

        if (species->Likes().count(liked_or_disliked_content_name))
            return 1.0;
        else if (species->Dislikes().count(liked_or_disliked_content_name))
            return -1.0;
        return 0.0;

    }
    else if (variable_name == "SpeciesEmpireOpinion") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1)
            empire_id = m_int_ref1->Eval(context);

        std::string species_name;
        if (m_string_ref1)
            species_name = m_string_ref1->Eval(context);

        return GetSpeciesManager().SpeciesEmpireOpinion(species_name, empire_id);

    }
    else if (variable_name == "SpeciesSpeciesOpinion") {
        std::string opinionated_species_name;
        if (m_string_ref1)
            opinionated_species_name = m_string_ref1->Eval(context);

        std::string rated_species_name;
        if (m_string_ref2)
            rated_species_name = m_string_ref2->Eval(context);

        return GetSpeciesManager().SpeciesSpeciesOpinion(opinionated_species_name, rated_species_name);

    }
    else if (variable_name == "SpecialCapacity") {
        int object_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object_id = m_int_ref1->Eval(context);
        auto object = context.ContextObjects().get(object_id);
        if (!object)
            return 0.0;

        std::string special_name;
        if (m_string_ref1)
            special_name = m_string_ref1->Eval(context);
        if (special_name.empty())
            return 0.0;

        return object->SpecialCapacity(special_name);

    }
    else if (variable_name == "ShipPartMeter") {
        int object_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object_id = m_int_ref1->Eval(context);
        auto object = context.ContextObjects().get(object_id);
        if (!object)
            return 0.0;
        auto ship = std::dynamic_pointer_cast<const Ship>(object);
        if (!ship)
            return 0.0;

        std::string part_name;
        if (m_string_ref1)
            part_name = m_string_ref1->Eval(context);
        if (part_name.empty())
            return 0.0;

        std::string meter_name;
        if (m_string_ref2)
            meter_name = m_string_ref2->Eval(context);
        if (meter_name.empty())
            return 0.0;

        MeterType meter_type = NameToMeter(meter_name);
        if (meter_type != MeterType::INVALID_METER_TYPE) {
            if (m_return_immediate_value)
                return ship->CurrentPartMeterValue(meter_type, part_name);
            else
                return ship->InitialPartMeterValue(meter_type, part_name);
        }
    }

    return 0.0;
}

namespace {
    std::vector<std::string> TechsResearchedByEmpire(int empire_id, const ScriptingContext& context) {
        auto empire = context.GetEmpire(empire_id);
        if (!empire) return {};

        auto researched_techs_range = empire->ResearchedTechs() | boost::adaptors::map_keys;
        return {researched_techs_range.begin(), researched_techs_range.end()};
    }

    std::vector<std::string> TechsResearchableByEmpire(int empire_id, const ScriptingContext& context) {
        auto empire = context.GetEmpire(empire_id);
        if (!empire) return {};

        std::vector<std::string> retval;
        retval.reserve(GetTechManager().size());
        for (const auto& tech : GetTechManager()) {
            if (tech && empire->ResearchableTech(tech->Name()))
                retval.push_back(tech->Name());
        }
        return retval;
    }

    std::vector<std::string> TransferrableTechs(int sender_empire_id, int receipient_empire_id,
                                                const ScriptingContext& context)
    {
        std::vector<std::string> sender_researched_techs = TechsResearchedByEmpire(sender_empire_id, context);
        std::vector<std::string> recepient_researchable = TechsResearchableByEmpire(receipient_empire_id, context);

        std::vector<std::string> retval;

        if (sender_researched_techs.empty() || recepient_researchable.empty())
            return retval;

        // find intersection of two lists
        std::sort(sender_researched_techs.begin(), sender_researched_techs.end());
        std::sort(recepient_researchable.begin(), recepient_researchable.end());
        std::set_intersection(sender_researched_techs.begin(), sender_researched_techs.end(),
                              recepient_researchable.begin(), recepient_researchable.end(),
                              std::back_inserter(retval));

        // find techs common to both lists
        return retval;
    }
}

template <>
std::string ComplexVariable<std::string>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    std::function<std::string (const Empire&)> empire_property{nullptr};
    auto null_property = [](const Empire&) -> std::string { return ""; };

    // unindexed empire properties
    if (variable_name == "LowestCostEnqueuedTech")
        empire_property = &Empire::LeastExpensiveEnqueuedTech;
    else if (variable_name == "HighestCostEnqueuedTech")
        empire_property = &Empire::MostExpensiveEnqueuedTech;
    else if (variable_name == "TopPriorityEnqueuedTech")
        empire_property = &Empire::TopPriorityEnqueuedTech;
    else if (variable_name == "MostSpentEnqueuedTech")
        empire_property = &Empire::MostRPSpentEnqueuedTech;
    else if (variable_name == "LowestCostResearchableTech")
        empire_property = &Empire::LeastExpensiveResearchableTech;
    else if (variable_name == "HighestCostResearchableTech")
        empire_property = &Empire::MostExpensiveResearchableTech;
    else if (variable_name == "TopPriorityResearchableTech")
        empire_property = &Empire::TopPriorityResearchableTech;
    else if (variable_name == "MostSpentResearchableTech")
        empire_property = &Empire::MostExpensiveResearchableTech;
    else if (variable_name == "MostSpentTransferrableTech")
        empire_property = null_property;
    else if (variable_name == "RandomTransferrableTech")
        empire_property = null_property;
    else if (variable_name == "MostPopulousSpecies")
        empire_property = null_property;
    else if (variable_name == "MostHappySpecies")
        empire_property = null_property;
    else if (variable_name == "LeastHappySpecies")
        empire_property = null_property;
    else if (variable_name == "RandomColonizableSpecies")
        empire_property = null_property;
    else if (variable_name == "RandomControlledSpecies")
        empire_property = null_property;

    if (empire_property) {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";

        return empire_property(*empire);
    }

    if (variable_name == "RandomEnqueuedTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";
        // get all techs on queue, randomly pick one
        const auto& queue = empire->GetResearchQueue();
        auto all_enqueued_techs = queue.AllEnqueuedProjects();
        if (all_enqueued_techs.empty())
            return "";
        std::size_t idx = RandInt(0, static_cast<int>(all_enqueued_techs.size()) - 1);
        return *std::next(all_enqueued_techs.begin(), idx);

    } else if (variable_name == "RandomResearchableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";

        auto researchable_techs = TechsResearchableByEmpire(empire_id, context);
        if (researchable_techs.empty())
            return "";
        std::size_t idx = RandInt(0, static_cast<int>(researchable_techs.size()) - 1);
        return *std::next(researchable_techs.begin(), idx);
    } else if (variable_name == "RandomCompleteTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";

        auto complete_techs = TechsResearchedByEmpire(empire_id, context);
        if (complete_techs.empty())
            return "";
        std::size_t idx = RandInt(0, static_cast<int>(complete_techs.size()) - 1);
        return *std::next(complete_techs.begin(), idx);
    } else if (variable_name == "LowestCostTransferrableTech") {
        int empire1_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire1_id = m_int_ref1->Eval(context);
            if (empire1_id == ALL_EMPIRES)
                return "";
        }

        int empire2_id = ALL_EMPIRES;
        if (m_int_ref2) {
            empire2_id = m_int_ref2->Eval(context);
            if (empire2_id == ALL_EMPIRES)
                return "";
        }

        std::vector<std::string> sendable_techs = TransferrableTechs(empire1_id, empire2_id, context);
        if (sendable_techs.empty())
            return "";
        std::size_t idx = RandInt(0, static_cast<int>(sendable_techs.size()) - 1);
        return *std::next(sendable_techs.begin(), idx);

    } else if (variable_name == "HighestCostTransferrableTech") {
        int empire1_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire1_id = m_int_ref1->Eval(context);
            if (empire1_id == ALL_EMPIRES)
                return "";
        }

        int empire2_id = ALL_EMPIRES;
        if (m_int_ref2) {
            empire2_id = m_int_ref2->Eval(context);
            if (empire2_id == ALL_EMPIRES)
                return "";
        }

        std::vector<std::string> sendable_techs = TransferrableTechs(empire1_id, empire2_id, context);
        if (sendable_techs.empty())
            return "";

        std::string retval;
        float highest_cost = 0.0f;
        for (const std::string& tech_name : sendable_techs) {
            const Tech* tech = GetTech(tech_name);
            if (!tech)
                continue;
            float rc = tech->ResearchCost(empire2_id);
            if (rc > highest_cost) {
                highest_cost = rc;
                retval = tech_name;
            }
        }
        return retval;

    } else if (variable_name == "TopPriorityTransferrableTech") {
        int empire1_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire1_id = m_int_ref1->Eval(context);
            if (empire1_id == ALL_EMPIRES)
                return "";
        }

        int empire2_id = ALL_EMPIRES;
        if (m_int_ref2) {
            empire2_id = m_int_ref2->Eval(context);
            if (empire2_id == ALL_EMPIRES)
                return "";
        }
        auto empire2 = context.GetEmpire(empire2_id);
        if (!empire2)
            return "";

        std::vector<std::string> sendable_techs = TransferrableTechs(empire1_id, empire2_id, context);
        if (sendable_techs.empty())
            return "";

        std::string retval = *sendable_techs.begin();   // pick first tech by default, hopefully to be replaced below
        int position_of_top_found_tech = INT_MAX;

        // search queue to find which transferrable tech is at the top of the list
        const ResearchQueue& queue = empire2->GetResearchQueue();
        for (const std::string& tech : sendable_techs) {
            auto queue_it = queue.find(tech);
            if (queue_it == queue.end())
                continue;
            int queue_pos = std::distance(queue.begin(), queue_it);
            if (queue_pos < position_of_top_found_tech) {
                retval = tech;
                position_of_top_found_tech = queue_pos;
            }
        }
        return retval;
    }

    // non-empire properties
    if (variable_name == "GameRule") {
        if (!m_string_ref1)
            return "";
        std::string rule_name = m_string_ref1->Eval();
        if (rule_name.empty())
            return "";
        if (!GetGameRules().RuleExists(rule_name))
            return "";
        try {
            // can cast boolean, int, double, or string-valued rules to strings
            switch (GetGameRules().GetType(rule_name)) {
            case GameRules::Type::TOGGLE: {
                return std::to_string(GetGameRules().Get<bool>(rule_name));
                break;
            }
            case GameRules::Type::INT: {
                return std::to_string(GetGameRules().Get<int>(rule_name));
                break;
            }
            case GameRules::Type::DOUBLE: {
                return DoubleToString(GetGameRules().Get<double>(rule_name), 3, false);
                break;
            }
            case GameRules::Type::STRING: {
                return GetGameRules().Get<std::string>(rule_name);
                break;
            }
            default:
                break;
            }
        } catch (...) {
        }
        return "";
    }

    return "";
}

template <>
std::vector<std::string> ComplexVariable<std::vector<std::string>>::Eval(
    const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    // unindexed empire properties
    if (variable_name == "EmpireAdoptedPolices") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return {};
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return {};

        return empire->AdoptedPolicies();

    } else if (variable_name == "EmpireAvailablePolices") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return {};
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return {};

        std::vector<std::string> retval;
        const auto& pols = empire->AvailablePolicies();
        retval.reserve(pols.size());
        std::copy(pols.begin(), pols.end(), std::back_inserter(retval));
        return retval;
    }

    return {};
}

#undef IF_CURRENT_VALUE

template <>
std::string ComplexVariable<Visibility>::Dump(unsigned short ntabs) const
{
    const std::string& variable_name = m_property_name.back();
    std::string retval = variable_name;

    if (variable_name == "EmpireObjectVisibility") {
        if (m_int_ref1)
            retval += " empire = " + m_int_ref1->Dump(ntabs);
        if (m_int_ref2)
            retval += " object = " + m_int_ref2->Dump(ntabs);
    }

    return retval;
}

template <>
std::string ComplexVariable<double>::Dump(unsigned short ntabs) const
{
    const std::string& variable_name = m_property_name.back();
    std::string retval = variable_name;

    // empire properties indexed by integers
    if (variable_name == "PropagatedSystemSupplyRange" ||
        variable_name == "SystemSupplyRange" ||
        variable_name == "PropagatedSystemSupplyDistance")
    {
        if (m_int_ref1)
            retval += " empire = " + m_int_ref1->Dump(ntabs);
        if (m_int_ref2)
            retval += " system = " + m_int_ref2->Dump(ntabs);

    }
    else if (variable_name == "GameRule" ||
             variable_name == "HullFuel" ||
             variable_name == "HullStealth" ||
             variable_name == "HullStructure" ||
             variable_name == "HullSpeed" ||
             variable_name == "PartCapacity" ||
             variable_name == "PartSecondaryStat")
    {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);

    }
    else if (variable_name == "EmpireMeterValue") {
        if (m_int_ref1)
            retval += " empire = " + m_int_ref1->Dump(ntabs);
        if (m_string_ref1)
            retval += " meter = " + m_string_ref1->Dump(ntabs);

    }
    else if (variable_name == "ShipPartMeter") {
        // ShipPartMeter part = "SR_WEAPON_1_1" meter = Capacity object = Source.ID
        if (m_string_ref1)
            retval += " part = " + m_string_ref1->Dump(ntabs);
        if (m_string_ref2)
            retval += " meter = " + m_string_ref2->Dump(ntabs); // wrapped in quotes " but shouldn't be to be consistent with parser
        if (m_int_ref1)
            retval += " object = " + m_int_ref1->Dump(ntabs);

    }
    else if (variable_name == "DirectDistanceBetween" ||
             variable_name == "ShortestPath")
    {
        if (m_int_ref1)
            retval += " object = " + m_int_ref1->Dump(ntabs);
        if (m_int_ref2)
            retval += " object = " + m_int_ref2->Dump(ntabs);

    }
    else if (variable_name == "SpeciesContentOpinion") {
        if (m_string_ref1)
            retval += " species = " + m_string_ref1->Dump(ntabs);
        if (m_string_ref2)
            retval += " name = " + m_string_ref2->Dump(ntabs);

    }
    else if (variable_name == "SpeciesEmpireOpinion") {
        if (m_int_ref1)
            retval += " empire = " + m_int_ref1->Dump(ntabs);
        if (m_string_ref1)
            retval += " species = " + m_string_ref1->Dump(ntabs);

    }
    else if (variable_name == "SpeciesSpeciesOpinion") {
        if (m_string_ref1)
            retval += " species = " + m_string_ref1->Dump(ntabs);
        if (m_string_ref2)
            retval += " species = " + m_string_ref2->Dump(ntabs);

    }
    else if (variable_name == "SpecialCapacity") {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);
        if (m_int_ref1)
            retval += " object = " + m_int_ref1->Dump(ntabs);

    }

    return retval;
}

template <>
std::string ComplexVariable<int>::Dump(unsigned short ntabs) const
{
    const std::string& variable_name = m_property_name.back();
    std::string retval = variable_name;
    // todo: implement like <double> case
    if (variable_name == "GameRule") {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);
    }

    return retval;
}

template <>
std::string ComplexVariable<std::string>::Dump(unsigned short ntabs) const
{
    const std::string& variable_name = m_property_name.back();
    std::string retval = variable_name;
    // todo: implement like <double> case
    if (variable_name == "GameRule") {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);
    }

    return retval;
}

template <>
std::string ComplexVariable<std::vector<std::string>>::Dump(unsigned short ntabs) const
{
    const std::string& variable_name = m_property_name.back();
    std::string retval = variable_name;
    // todo: implement like <double> case
    if (variable_name == "GameRule") {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);
    }

    return retval;
}

///////////////////////////////////////////////////////////
// StringCast                                            //
///////////////////////////////////////////////////////////
template <>
std::string StringCast<double>::Eval(const ScriptingContext& context) const
{
    if (!m_value_ref)
        return "";
    double temp = m_value_ref->Eval(context);

    // special case for a few sub-value-refs to help with UI representation
    if (Variable<double>* int_var = dynamic_cast<Variable<double>*>(m_value_ref.get())) {
        if (int_var->PropertyName().back() == "X" || int_var->PropertyName().back() == "Y") {
            if (temp == UniverseObject::INVALID_POSITION)
                return UserString("INVALID_POSITION");

            std::stringstream ss;
            ss << std::setprecision(6) << temp;
            return ss.str();
        }
    }

    return DoubleToString(temp, 3, false);
}

template <>
std::string StringCast<int>::Eval(const ScriptingContext& context) const
{
    if (!m_value_ref)
        return "";
    int temp = m_value_ref->Eval(context);

    // special case for a few sub-value-refs to help with UI representation
    if (Variable<int>* int_var = dynamic_cast<Variable<int>*>(m_value_ref.get())) {
        if (int_var->PropertyName().back() == "ETA") {
            if (temp == Fleet::ETA_UNKNOWN) {
                return UserString("FW_FLEET_ETA_UNKNOWN");
            } else if (temp == Fleet::ETA_NEVER) {
                return UserString("FW_FLEET_ETA_NEVER");
            } else if (temp == Fleet::ETA_OUT_OF_RANGE) {
                return UserString("FW_FLEET_ETA_OUT_OF_RANGE");
            }
        }
    }

    return std::to_string(temp);
}

template <>
std::string StringCast<std::vector<std::string>>::Eval(const ScriptingContext& context) const
{
    if (!m_value_ref)
        return "";
    std::vector<std::string> temp = m_value_ref->Eval(context);

    // concatenate strings into one big string
    std::string retval;
    for (auto str : temp)
        retval += str + " ";
    return retval;
}

///////////////////////////////////////////////////////////
// UserStringLookup                                      //
///////////////////////////////////////////////////////////
template <>
std::string UserStringLookup<std::string>::Eval(const ScriptingContext& context) const {
    if (!m_value_ref)
        return "";
    std::string ref_val = m_value_ref->Eval(context);
    if (ref_val.empty() || !UserStringExists(ref_val))
        return "";
    return UserString(ref_val);
}

template <>
std::string UserStringLookup<std::vector<std::string>>::Eval(const ScriptingContext& context) const {
    if (!m_value_ref)
        return "";
    std::vector<std::string> ref_vals = m_value_ref->Eval(context);
    if (ref_vals.empty())
        return "";
    std::string retval;
    for (auto val : ref_vals) {
        if (val.empty() || !UserStringExists(val))
            continue;
        retval += UserString(val) + " ";
    }
    return retval;
}

/////////////////////////////////////////////////////
// NameLookup                                      //
/////////////////////////////////////////////////////
NameLookup::NameLookup(std::unique_ptr<ValueRef<int>>&& value_ref, LookupType lookup_type) :
    Variable<std::string>(ReferenceType::NON_OBJECT_REFERENCE),
    m_value_ref(std::move(value_ref)),
    m_lookup_type(lookup_type)
{
    m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
}

bool NameLookup::operator==(const ValueRef<std::string>& rhs) const {
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const NameLookup& rhs_ =
        static_cast<const NameLookup&>(rhs);

    if (m_lookup_type == rhs_.m_lookup_type) {
        // check next member
    } else {
        return false;
    }

    if (m_value_ref == rhs_.m_value_ref) {
        // check next member
    } else if (!m_value_ref || !rhs_.m_value_ref) {
        return false;
    } else {
        if (*m_value_ref != *(rhs_.m_value_ref))
            return false;
    }

    return true;
}

std::string NameLookup::Eval(const ScriptingContext& context) const {
    if (!m_value_ref || m_lookup_type == LookupType::INVALID_LOOKUP)
        return "";

    switch (m_lookup_type) {
    case LookupType::OBJECT_NAME: {
        auto obj = context.ContextObjects().get(m_value_ref->Eval(context));
        return obj ? obj->Name() : "";
        break;
    }
    case LookupType::EMPIRE_NAME: {
        auto empire = context.GetEmpire(m_value_ref->Eval(context));
        return empire ? empire->Name() : "";
        break;
    }
    case LookupType::SHIP_DESIGN_NAME: {
        const ShipDesign* design = GetShipDesign(m_value_ref->Eval(context));
        return design ? design->Name() : "";
        break;
    }
    default:
        return "";
    }
}

std::string NameLookup::Description() const
{ return m_value_ref->Description(); }

std::string NameLookup::Dump(unsigned short ntabs) const
{ return m_value_ref->Dump(ntabs); }

void NameLookup::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

unsigned int NameLookup::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::NameLookup");
    CheckSums::CheckSumCombine(retval, m_value_ref);
    CheckSums::CheckSumCombine(retval, m_lookup_type);
    TraceLogger() << "GetCheckSum(NameLookup): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
template <>
std::string Operation<std::string>::EvalImpl(const ScriptingContext& context) const
{
    if (m_op_type == OpType::PLUS) {
        return LHS()->Eval(context) + RHS()->Eval(context);

    } else if (m_op_type == OpType::TIMES) {
        // useful for writing a "Statistic If" expression with strings. Number-
        // valued types return 0 or 1 for nothing or something matching the sampling
        // condition. For strings, an empty string indicates no matches, and non-empty
        // string indicates matches, which is treated like a multiplicative identity
        // operation, so just returns the RHS of the expression.
        if (LHS()->Eval(context).empty())
            return "";
        return RHS()->Eval(context);

    } else if (m_op_type == OpType::MINIMUM || m_op_type == OpType::MAXIMUM) {
        // evaluate all operands, return sorted first/last
        std::set<std::string> vals;
        for (auto& vr : m_operands) {
            if (vr)
                vals.emplace(vr->Eval(context));
        }
        if (m_op_type == OpType::MINIMUM)
            return vals.empty() ? "" : *vals.begin();
        else
            return vals.empty() ? "" : *vals.rbegin();

    } else if (m_op_type == OpType::RANDOM_PICK) {
        // select one operand, evaluate it, return result
        if (m_operands.empty())
            return "";
        unsigned int idx = RandInt(0, m_operands.size() - 1);
        auto& vr = *std::next(m_operands.begin(), idx);
        if (!vr)
            return "";
        return vr->Eval(context);

    } else if (m_op_type == OpType::SUBSTITUTION) {
        // insert string into other string in place of %1% or similar placeholder
        if (m_operands.empty())
            return "";
        auto& template_op = *(m_operands.begin());
        if (!template_op)
            return "";
        std::string template_str = template_op->Eval(context);

        boost::format formatter = FlexibleFormat(template_str);

        for (auto& op : m_operands)
            formatter % (op ? op->Eval(context) : "");
        return formatter.str();

    } else if (m_op_type >= OpType::COMPARE_EQUAL && m_op_type <= OpType::COMPARE_NOT_EQUAL) {
        const std::string&& lhs_val = LHS()->Eval(context);
        const std::string&& rhs_val = RHS()->Eval(context);
        bool test_result = false;
        switch (m_op_type) {
            case OpType::COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
            case OpType::COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
            case OpType::COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
            case OpType::COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
            case OpType::COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
            case OpType::COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
            default:    break;  // ??? do nothing, default to false
        }
        if (m_operands.size() < 3) {
            return test_result ? "true" : "false";
        } else if (m_operands.size() < 4) {
            if (test_result)
                return m_operands[2]->Eval(context);
            else
                return "false";
        } else {
            if (test_result)
                return m_operands[2]->Eval(context);
            else
                return m_operands[3]->Eval(context);
        }
    }

    throw std::runtime_error("std::string ValueRef evaluated with an unknown or invalid OpType.");
    return "";
}

template <>
double Operation<double>::EvalImpl(const ScriptingContext& context) const
{
    switch (m_op_type) {
        case OpType::PLUS:
            return LHS()->Eval(context) + RHS()->Eval(context); break;

        case OpType::MINUS:
            return LHS()->Eval(context) - RHS()->Eval(context); break;

        case OpType::TIMES: {
            double op1 = LHS()->Eval(context);
            if (op1 == 0.0)
                return 0.0;
            return op1 * RHS()->Eval(context);
            break;
        }

        case OpType::DIVIDE: {
            double op2 = RHS()->Eval(context);
            if (op2 == 0.0)
                return 0.0;
            return LHS()->Eval(context) / op2;
            break;
        }

        case OpType::NEGATE:
            return -(LHS()->Eval(context)); break;

        case OpType::EXPONENTIATE: {
            double op2 = RHS()->Eval(context);
            if (op2 == 0.0)
                return 1.0;
            try {
                double op1 = LHS()->Eval(context);
                return std::pow(op1, op2);
            } catch (...) {
                ErrorLogger() << "Error evaluating exponentiation ValueRef::Operation";
                return 0.0;
            }
            break;
        }

        case OpType::ABS:
            return std::abs(LHS()->Eval(context)); break;

        case OpType::LOGARITHM: {
            double op1 = LHS()->Eval(context);
            if (op1 <= 0.0)
                return 0.0;
            return std::log(op1);
            break;
        }

        case OpType::SINE:
            return std::sin(LHS()->Eval(context)); break;

        case OpType::COSINE:
            return std::cos(LHS()->Eval(context)); break;

        case OpType::MINIMUM:
        case OpType::MAXIMUM: {
            std::set<double> vals;
            for (auto& vr : m_operands) {
                if (vr)
                    vals.emplace(vr->Eval(context));
            }
            if (m_op_type == OpType::MINIMUM)
                return vals.empty() ? 0.0 : *vals.begin();
            else
                return vals.empty() ? 0.0 : *vals.rbegin();
            break;
        }

        case OpType::RANDOM_UNIFORM: {
            double op1 = LHS()->Eval(context);
            double op2 = RHS()->Eval(context);
            double min_val = std::min(op1, op2);
            double max_val = std::max(op1, op2);
            return RandDouble(min_val, max_val);
            break;
        }

        case OpType::RANDOM_PICK: {
            // select one operand, evaluate it, return result
            if (m_operands.empty())
                return 0.0;
            unsigned int idx = RandInt(0, m_operands.size() - 1);
            auto& vr = *std::next(m_operands.begin(), idx);
            if (!vr)
                return 0.0;
            return vr->Eval(context);
            break;
        }

        case OpType::COMPARE_EQUAL:
        case OpType::COMPARE_GREATER_THAN:
        case OpType::COMPARE_GREATER_THAN_OR_EQUAL:
        case OpType::COMPARE_LESS_THAN:
        case OpType::COMPARE_LESS_THAN_OR_EQUAL:
        case OpType::COMPARE_NOT_EQUAL: {
            const double&& lhs_val = LHS()->Eval(context);
            const double&& rhs_val = RHS()->Eval(context);
            bool test_result = false;
            switch (m_op_type) {
                case OpType::COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
                case OpType::COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
                case OpType::COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
                case OpType::COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
                case OpType::COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
                case OpType::COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
                default:    break;  // ??? do nothing, default to false
            }
            if (m_operands.size() < 3) {
                return static_cast<double>(test_result);
            } else if (m_operands.size() < 4) {
                if (test_result)
                    return m_operands[2]->Eval(context);
                else
                    return 0.0;
            } else {
                if (test_result)
                    return m_operands[2]->Eval(context);
                else
                    return m_operands[3]->Eval(context);
            }
            break;
        }

        case OpType::ROUND_NEAREST:
            return std::round(LHS()->Eval(context)); break;
        case OpType::ROUND_UP:
            return std::ceil(LHS()->Eval(context)); break;
        case OpType::ROUND_DOWN:
            return std::floor(LHS()->Eval(context)); break;

        case OpType::SIGN: {
            auto lhs{LHS()->Eval(context)};
            return lhs < 0.0 ? -1.0 : lhs > 0.0 ? 1.0 : 0.0;
            break;
        }

        default:
            break;
    }

    throw std::runtime_error("double ValueRef evaluated with an unknown or invalid OpType.");
    return 0.0;
}

template <>
int Operation<int>::EvalImpl(const ScriptingContext& context) const
{
    switch (m_op_type) {
        case OpType::PLUS:
            return LHS()->Eval(context) + RHS()->Eval(context);     break;

        case OpType::MINUS:
            return LHS()->Eval(context) - RHS()->Eval(context);     break;

        case OpType::TIMES: {
            double op1 = LHS()->Eval(context);
            if (op1 == 0)
                return 0;
            return op1 * RHS()->Eval(context);
            break;
        }

        case OpType::DIVIDE: {
            int op2 = RHS()->Eval(context);
            if (op2 == 0)
                return 0;
            return LHS()->Eval(context) / op2;
            break;
        }

        case OpType::NEGATE:
            return -LHS()->Eval(context); break;

        case OpType::EXPONENTIATE: {
            double op2 = RHS()->Eval(context);
            if (op2 == 0)
                return 1;
            try {
                double op1 = LHS()->Eval(context);
                return static_cast<int>(std::pow(op1, op2));
            } catch (...) {
                ErrorLogger() << "Error evaluating exponentiation ValueRef::Operation";
                return 0;
            }
            break;
        }

        case OpType::ABS: {
            return static_cast<int>(std::abs(LHS()->Eval(context)));
            break;
        }

        case OpType::LOGARITHM: {
            double op1 = LHS()->Eval(context);
            if (op1 <= 0.0)
                return 0;
            return static_cast<int>(std::log(op1));
            break;
        }

        case OpType::SINE: {
            double op1 = LHS()->Eval(context);
            return static_cast<int>(std::sin(op1));
            break;
        }

        case OpType::COSINE: {
            double op1 = LHS()->Eval(context);
            return static_cast<int>(std::cos(op1));
            break;
        }

        case OpType::MINIMUM:
        case OpType::MAXIMUM: {
            std::set<int> vals;
            for (auto& vr : m_operands) {
                if (vr)
                    vals.emplace(vr->Eval(context));
            }
            if (m_op_type == OpType::MINIMUM)
                return vals.empty() ? 0 : *vals.begin();
            else
                return vals.empty() ? 0 : *vals.rbegin();
            break;
        }

        case OpType::RANDOM_UNIFORM: {
            double op1 = LHS()->Eval(context);
            double op2 = RHS()->Eval(context);
            int min_val = static_cast<int>(std::min(op1, op2));
            int max_val = static_cast<int>(std::max(op1, op2));
            return RandInt(min_val, max_val);
            break;
        }

        case OpType::RANDOM_PICK: {
            // select one operand, evaluate it, return result
            if (m_operands.empty())
                return 0;
            unsigned int idx = RandInt(0, m_operands.size() - 1);
            auto& vr = *std::next(m_operands.begin(), idx);
            if (!vr)
                return 0;
            return vr->Eval(context);
            break;
        }

        case OpType::COMPARE_EQUAL:
        case OpType::COMPARE_GREATER_THAN:
        case OpType::COMPARE_GREATER_THAN_OR_EQUAL:
        case OpType::COMPARE_LESS_THAN:
        case OpType::COMPARE_LESS_THAN_OR_EQUAL:
        case OpType::COMPARE_NOT_EQUAL: {
            const int&& lhs_val = LHS()->Eval(context);
            const int&& rhs_val = RHS()->Eval(context);
            bool test_result = false;
            switch (m_op_type) {
                case OpType::COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
                case OpType::COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
                case OpType::COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
                case OpType::COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
                case OpType::COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
                case OpType::COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
                default:    break;  // ??? do nothing, default to false
            }
            if (m_operands.size() < 3) {
                return static_cast<int>(test_result);
            } else if (m_operands.size() < 4) {
                if (test_result)
                    return m_operands[2]->Eval(context);
                else
                    return 0;
            } else {
                if (test_result)
                    return m_operands[2]->Eval(context);
                else
                    return m_operands[3]->Eval(context);
            }
            break;
        }

        case OpType::ROUND_NEAREST:
        case OpType::ROUND_UP:
        case OpType::ROUND_DOWN: {
            // integers don't need to be rounded...
            return LHS()->Eval(context);
            break;
        }

        case OpType::SIGN: {
            auto lhs{LHS()->Eval(context)};
            return lhs < 0 ? -1 : lhs > 0 ? 1 : 0;
            break;
        }

        default:    break;
    }

    throw std::runtime_error("double ValueRef evaluated with an unknown or invalid OpType.");
    return 0;
}
} // namespace ValueRef
