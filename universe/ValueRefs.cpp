#include "ValueRefs.h"

#include <algorithm>
#if __has_include(<charconv>)
  #include <charconv>
#endif
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
#include "UniverseObject.h"
#include "Universe.h"
#include "../combat/CombatDamage.h"
#include "../Empire/Empire.h"
#include "../Empire/Supply.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Random.h"

// define needed on Windows due to conflict with windows.h and std::min and std::max
#ifndef NOMINMAX
#  define NOMINMAX
#endif
// define needed in GCC
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif
#if defined(_MSC_VER) && _MSC_VER >= 1930
struct IUnknown; // Workaround for "combaseapi.h(229,21): error C2760: syntax error: 'identifier' was unexpected here; expected 'type specifier'"
#endif

#include <boost/stacktrace.hpp>

std::string DoubleToString(double val, int digits, bool always_show_sign);
bool UserStringExists(const std::string& str);

namespace {
    std::string StackTrace() {
        static std::atomic<int> string_error_lookup_count = 0;
        if (string_error_lookup_count++ > 10)
            return "";
        using namespace boost::stacktrace;
        return "stacktrace:\n" + to_string(stacktrace());
    }

    const UniverseObject* FollowReference(
        std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last,
        ValueRef::ReferenceType ref_type, const ScriptingContext& context)
    {
        const UniverseObject* obj = nullptr;
        switch (ref_type) {
        case ValueRef::ReferenceType::NON_OBJECT_REFERENCE:                return context.condition_local_candidate;   break;
        case ValueRef::ReferenceType::SOURCE_REFERENCE:                    obj = context.source;                       break;
        case ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE:             obj = context.effect_target;                break;
        case ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:  obj = context.condition_root_candidate;     break;
        case ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE:
        default:                                                           obj = context.condition_local_candidate;    break;
        }

        if (!obj) {
            std::string_view type_string = "";
            switch (ref_type) {
            case ValueRef::ReferenceType::SOURCE_REFERENCE:                    type_string = "Source";         break;
            case ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE:             type_string = "Target";         break;
            case ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:  type_string = "RootCandidate";  break;
            case ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE:
            default:                                                           type_string = "LocalCandidate"; break;
            }
            ErrorLogger() << "FollowReference : top level object (" << type_string << ") not defined in scripting context. "
                          << "  strings: " << [it=first, last]() mutable -> std::string
                            {
                                std::string retval;
                                retval.reserve(100); // guesstimate
                                for (; it != last; ++it)
                                    retval.append(*it).append(" ");
                                return retval;
                            }()
                          << " source: " << (context.source ? context.source->Name() : "0")
                          << " target: " << (context.effect_target ? context.effect_target->Name() : "0")
                          << " local c: " << (context.condition_local_candidate ? context.condition_local_candidate->Name() : "0")
                          << " root c: " << (context.condition_root_candidate ? context.condition_root_candidate->Name() : "0")
                          << "  " << StackTrace();
            return nullptr;
        }

        while (first != last) {
            std::string_view property_name = *first;
            if (property_name == "Planet") {
                if (obj->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
                    auto b = static_cast<const Building*>(obj);
                    obj = context.ContextObjects().getRaw<Planet>(b->PlanetID());
                } else {
                    ErrorLogger() << "FollowReference : object not a building, so can't get its planet.";
                    obj = nullptr;
                }
            } else if (property_name == "System") {
                if (obj)
                    obj = context.ContextObjects().getRaw<System>(obj->SystemID());
                if (!obj)
                    ErrorLogger() << "FollowReference : Unable to get system for object";
            } else if (property_name == "Fleet") {
                if (obj->ObjectType() == UniverseObjectType::OBJ_SHIP) {
                    auto s = static_cast<const Ship*>(obj);
                    obj = context.ContextObjects().getRaw<Fleet>(s->FleetID());
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
        const UniverseObject* obj = nullptr;
        const UniverseObject* initial_obj = nullptr;
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
            retval += UserString(to_string(obj->ObjectType())) + " "
                    + std::to_string(obj->ID()) + " ( " + obj->Name() + " ) ";
            initial_obj = obj;
        }
        retval += " | ";

        auto first = property_name.begin();
        const auto last = property_name.end();
        while (first != last) {
            std::string property_name_part = *first;
            retval.append(" ").append(property_name_part).append(" ");
            if (property_name_part == "Planet") {
                if (obj->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
                    auto b = static_cast<const Building*>(obj);
                    retval.append("(").append(std::to_string(b->PlanetID())).append("): ");
                    obj = context.ContextObjects().getRaw<Planet>(b->PlanetID());
                } else {
                    obj = nullptr;
                }
            } else if (property_name_part == "System") {
                if (obj) {
                    retval.append("(").append(std::to_string(obj->SystemID())).append("): ");
                    obj = context.ContextObjects().getRaw<System>(obj->SystemID());
                }
            } else if (property_name_part == "Fleet") {
                if (obj->ObjectType() == UniverseObjectType::OBJ_SHIP) {
                    auto s = static_cast<const Ship*>(obj);
                    retval.append("(").append(std::to_string(s->FleetID())).append("): ");
                    obj = context.ContextObjects().getRaw<Fleet>(s->FleetID());
                } else
                    obj = nullptr;
            }

            ++first;

            if (obj && initial_obj != obj) {
                retval.append("  Referenced Object: ").append(UserString(to_string(obj->ObjectType())))
                      .append(" ").append(std::to_string(obj->ID())).append(" ( ").append(obj->Name()).append(" )");
            }
            retval += " | ";
        }
        return retval;
    }

    // Array of meter names enumerated by MeterType with INVALID_METER_TYPE as first element
    constexpr std::array<std::string_view, static_cast<std::size_t>(MeterType::NUM_METER_TYPES) + 1> NAME_BY_METER = {
        "",
        "TargetPopulation",
        "TargetIndustry",
        "TargetResearch",
        "TargetInfluence",
        "TargetConstruction",
        "TargetHappiness",

        "MaxCapacity",
        "MaxSecondaryStat",

        "MaxFuel",
        "MaxShield",
        "MaxStructure",
        "MaxDefense",
        "MaxSupply",
        "MaxStockpile",
        "MaxTroops",

        "Population",
        "Industry",
        "Research",
        "Influence",
        "Construction",
        "Happiness",

        "Capacity",
        "SecondaryStat",

        "Fuel",
        "Shield",
        "Structure",
        "Defense",
        "Supply",
        "Stockpile",
        "Troops",

        "RebelTroops",
        "Size",
        "Stealth",
        "Detection",
        "Speed"
    };

    // Array of planet types enumerated by PlanetType with INVALID_PLANET_TYPE as first element
    constexpr std::array<std::string_view, static_cast<std::size_t>(PlanetType::NUM_PLANET_TYPES) + 1> NAME_BY_PLANET = {
        "?",
        "Swamp",
        "Toxic",
        "Inferno",
        "Radiated",
        "Barren",
        "Tundra",
        "Desert",
        "Terran",
        "Ocean",
        "Asteroids",
        "GasGiant"
    };
}

namespace ValueRef {

template <typename EnumT>
std::string EnumToString(EnumT t)
{
    static_assert(std::is_enum_v<EnumT>);
    auto maybe_retval = to_string(t);
    if (UserStringExists(maybe_retval))
        return UserString(maybe_retval);
    else
        return std::string{maybe_retval};
}

std::string FlexibleToString(StarType t) { return EnumToString(t); }
std::string FlexibleToString(PlanetEnvironment t) { return EnumToString(t); }
std::string FlexibleToString(PlanetType t) { return EnumToString(t); }
std::string FlexibleToString(PlanetSize t) { return EnumToString(t); }
std::string FlexibleToString(Visibility t) { return EnumToString(t); }
std::string FlexibleToString(UniverseObjectType t) { return EnumToString(t); }

std::string ValueRefBase::InvariancePattern() const {
    return std::string{RootCandidateInvariant() ? "R" : "r"}
        .append(LocalCandidateInvariant()       ? "L" : "l")
        .append(SourceInvariant()               ? "S" : "s")
        .append(TargetInvariant()               ? "T" : "t")
        .append(SimpleIncrement()               ? "I" : "i")
        .append(ConstantExpr()                  ? "C" : "c");
}

constexpr MeterType NameToMeterCX(std::string_view name) noexcept {
    for (int i = 0; i < static_cast<int>(NAME_BY_METER.size()); i++) {
        if (NAME_BY_METER[i] == name)
            return static_cast<MeterType>(i - 1);
    }

    return MeterType::INVALID_METER_TYPE;
}
MeterType NameToMeter(std::string_view name) noexcept { return NameToMeterCX(name); }

static_assert(NameToMeterCX("not a meter") == MeterType::INVALID_METER_TYPE, "Name to Meter conversion failed for invalid meter type!");
static_assert(NameToMeterCX("Population") == MeterType::METER_POPULATION, "Name to Meter conversion failed for 'Population' meter!");
static_assert(NameToMeterCX("Speed") == MeterType::METER_SPEED, "Name to Meter conversion failed for 'Speed' meter!");

constexpr std::string_view MeterToNameCX(MeterType meter) noexcept {
    // NOTE: INVALID_METER_TYPE (enum's -1 position) <= meter < NUM_METER_TYPES (enum's final position)
    return NAME_BY_METER[static_cast<std::underlying_type_t<MeterType>>(meter) + 1];
}
std::string_view MeterToName(MeterType meter) noexcept { return MeterToNameCX(meter); }

constexpr std::string_view PlanetTypeToStringCX(PlanetType planet) noexcept {
    // NOTE: INVALID_PLANET_TYPE (enum's -1 position) <= planet < NUM_PLANET_TYPES (enum's final position)
    return NAME_BY_PLANET[static_cast<std::underlying_type_t<PlanetType>>(planet) + 1]; 
}
std::string_view PlanetTypeToString(PlanetType planet) noexcept { return PlanetTypeToStringCX(planet); }

// @return the correct PlanetType enum for a user friendly planet type string (e.g. "Ocean"), else it returns PlanetType::INVALID_PLANET_TYPE
constexpr PlanetType StringToPlanetTypeCX(std::string_view name) noexcept {
    for (int i = 0; i < static_cast<int>(NAME_BY_PLANET.size()); i++) {
        if (NAME_BY_PLANET[i] == name)
            return static_cast<PlanetType>(i - 1);
    }
    return PlanetType::INVALID_PLANET_TYPE;
}
PlanetType StringToPlanetType(std::string_view name) noexcept { return StringToPlanetTypeCX(name); }

static_assert(StringToPlanetTypeCX("not a planet") == PlanetType::INVALID_PLANET_TYPE, "Name to Planet conversion failed for invalid planet type!");
static_assert(StringToPlanetTypeCX("Swamp") == PlanetType::PT_SWAMP, "Name to Planet conversion failed for 'Swamp' planet!");
static_assert(StringToPlanetTypeCX("GasGiant") == PlanetType::PT_GASGIANT, "Name to Planet conversion failed for 'GasGiant' planet!");

constexpr std::string_view PlanetEnvironmentToStringCX(PlanetEnvironment env) noexcept {
    switch (env) {
    case PlanetEnvironment::PE_UNINHABITABLE: return "Uninhabitable";
    case PlanetEnvironment::PE_HOSTILE:       return "Hostile";
    case PlanetEnvironment::PE_POOR:          return "Poor";
    case PlanetEnvironment::PE_ADEQUATE:      return "Adequate";
    case PlanetEnvironment::PE_GOOD:          return "Good";
    default:                                  return "?";
    }
}
std::string_view PlanetEnvironmentToString(PlanetEnvironment env) noexcept { return PlanetEnvironmentToStringCX(env); }

std::string ReconstructName(const std::vector<std::string>& property_name,
                            ReferenceType ref_type, bool return_immediate_value)
{
    std::string retval;
    retval.reserve(64);

    if (return_immediate_value)
        retval += "Value(";

    switch (ref_type) {
    case ReferenceType::SOURCE_REFERENCE:                    retval += "Source";          break;
    case ReferenceType::EFFECT_TARGET_REFERENCE:             retval += "Target";          break;
    case ReferenceType::EFFECT_TARGET_VALUE_REFERENCE:       retval += "Value";           break;
    case ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE: retval += "LocalCandidate";  break;
    case ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:  retval += "RootCandidate";   break;
    case ReferenceType::NON_OBJECT_REFERENCE:                retval += "";                break;
    default:                                                 retval += "?????";           break;
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

std::string StatisticDescription(StatisticType stat_type, std::string_view value_desc,
                                 std::string_view condition_desc)
{
    std::string stringtable_key{"DESC_VAR_"};
    stringtable_key.append(to_string(stat_type)); // assumes that all StatisticType names are ALL_CAPS

    if (UserStringExists(stringtable_key)) {
        boost::format formatter = FlexibleFormat(UserString(stringtable_key));
        formatter % value_desc % condition_desc;
        return boost::io::str(formatter);
    }

    return UserString("DESC_VAR_STATISITIC");
}

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
template <>
std::string Constant<PlanetType>::Description() const
{ return UserString(to_string(m_value)); }

template <>
std::string Constant<PlanetSize>::Description() const
{ return UserString(to_string(m_value)); }

template <>
std::string Constant<PlanetEnvironment>::Description() const
{ return UserString(to_string(m_value)); }

template <>
std::string Constant<UniverseObjectType>::Description() const
{ return UserString(to_string(m_value)); }

template <>
std::string Constant<StarType>::Description() const
{ return UserString(to_string(m_value)); }

template <>
std::string Constant<Visibility>::Description() const
{ return UserString(to_string(m_value)); }

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
    if (m_value == current_content)
        return m_top_level_content;
    return m_value;
}

template <>
std::string Constant<PlanetSize>::Dump(uint8_t ntabs) const
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
std::string Constant<PlanetType>::Dump(uint8_t ntabs) const
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
std::string Constant<PlanetEnvironment>::Dump(uint8_t ntabs) const
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
std::string Constant<UniverseObjectType>::Dump(uint8_t ntabs) const
{
    switch (m_value) {
    case UniverseObjectType::OBJ_BUILDING:    return "Building";
    case UniverseObjectType::OBJ_SHIP:        return "Ship";
    case UniverseObjectType::OBJ_FLEET:       return "Fleet"; 
    case UniverseObjectType::OBJ_PLANET:      return "Planet";
    case UniverseObjectType::OBJ_SYSTEM:      return "System";
    case UniverseObjectType::OBJ_FIELD:       return "Field";
    default:                                  return "?";
    }
}

template <>
std::string Constant<StarType>::Dump(uint8_t ntabs) const
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
std::string Constant<Visibility>::Dump(uint8_t ntabs) const
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
std::string Constant<int>::Dump(uint8_t ntabs) const
{ return std::to_string(m_value); }

template <>
std::string Constant<double>::Dump(uint8_t ntabs) const
{ return std::to_string(m_value); }

template <>
std::string Constant<std::string>::Dump(uint8_t ntabs) const
{ return "\"" + Description() + "\""; }

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
#define IF_CURRENT_VALUE(T)                                            \
if (m_ref_type == ReferenceType::EFFECT_TARGET_VALUE_REFERENCE) {      \
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
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto p = static_cast<const Planet*>(object);
            return planet_property(*p);
        }
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
    else if (property_name == "NextBestPlanetType")
        planet_property = [&context](const Planet& p) { return p.NextBestPlanetTypeForSpecies(context); };
    else if (property_name == "NextBetterPlanetType")
        planet_property = [&context](const Planet& p) { return p.NextBetterPlanetTypeForSpecies(context); };
    else if (property_name == "ClockwiseNextPlanetType")
        planet_property = &Planet::ClockwiseNextPlanetType;
    else if (property_name == "CounterClockwiseNextPlanetType")
        planet_property = &Planet::CounterClockwiseNextPlanetType;

    if (planet_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto p = static_cast<const Planet*>(object);
            return planet_property(*p);
        }
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
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto p = static_cast<const Planet*>(object);
            return p->EnvironmentForSpecies(context);
        }

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
        return object->ObjectType();
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
        if (object->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
            auto s = static_cast<const System*>(object);
            return system_property(*s);
        }
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
            return context.ContextUniverse().UniverseWidth() / 2;
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

    MeterType meter_type = NameToMeterCX(property_name);
    if (object && meter_type != MeterType::INVALID_METER_TYPE) {
        if (auto* m = object->GetMeter(meter_type)) {
            return m_return_immediate_value ? m->Current() : m->Initial();
        }
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
        if (auto planet = dynamic_cast<const Planet*>(object))
            return planet_property(*planet);
        return 0.0;

    }

    if (property_name == "CombatBout") {
        return context.combat_bout;

    } else if (property_name == "CurrentTurn") {
        return context.current_turn;

    } else if (property_name == "DestroyFightersPerBattleMax") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            auto retval = ship->TotalWeaponsFighterDamage(context);
            TraceLogger() << "DestroyFightersPerBattleMax" << retval;
            // TODO: prevent recursion; disallowing the ValueRef inside of destroyFightersPerBattleMax via parsers would be best.
            return retval;
        }
        return 0.0;

    } else if (property_name == "DamageStructurePerBattleMax") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            // TODO: prevent recursion; disallowing the ValueRef inside of damageStructurePerBattleMax via parsers would be best.
            auto retval = ship->TotalWeaponsShipDamage(context);
            TraceLogger() << "DamageStructurePerBattleMax" << retval;
            return retval;
        }
        return 0.0;

    } else if (property_name == "PropagatedSupplyRange") {
        const auto& ranges = context.supply.PropagatedSupplyRanges();
        auto range_it = ranges.find(object->SystemID());
        if (range_it == ranges.end())
            return 0.0;
        return range_it->second;

    } else if (property_name == "PropagatedSupplyDistance") {
        const auto& ranges = context.supply.PropagatedSupplyDistances();
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
        if (property_name == "UsedInDesignID")
            return context.in_design_id;
        if (property_name == "SelectedSystemID")
            return IApp::GetApp()->SelectedSystemID();
        if (property_name == "SelectedPlanetID")
            return IApp::GetApp()->SelectedPlanetID();
        if (property_name == "SelectedFleetID")
            return IApp::GetApp()->SelectedFleetID();
        if (property_name == "SelectedPlanetID")
            return IApp::GetApp()->SelectedPlanetID();

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
        return context.supply.EmpireThatCanSupplyAt(object->SystemID());
    }
    else if (property_name == "ID") {
        return object->ID();
    }
    else if (property_name == "CreationTurn") {
        return object->CreationTurn();
    }
    else if (property_name == "Age") {
        return object->AgeInTurns(context.current_turn);

    }

    std::function<int (const Ship&)> ship_property{nullptr};

    if (property_name == "ArrivedOnTurn")
        ship_property = &Ship::ArrivedOnTurn;
    else if (property_name == "LastTurnActiveInBattle")
        ship_property = &Ship::LastTurnActiveInCombat;
    else if (property_name == "LastTurnResupplied")
        ship_property = &Ship::LastResuppliedOnTurn;
    else if (property_name == "OrderedColonizePlanetID")
        ship_property = &Ship::OrderedColonizePlanet;

    if (ship_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship_property(*ship);
        }
        return INVALID_GAME_TURN;
    }

    std::function<int (const Fleet&)> fleet_property{nullptr};

    if (property_name == "FinalDestinationID")
        fleet_property = &Fleet::FinalDestinationID;
    else if (property_name == "NextSystemID")
        fleet_property = &Fleet::NextSystemID;
    else if (property_name == "PreviousSystemID")
        fleet_property = &Fleet::PreviousSystemID;
    else if (property_name == "PreviousToFinalDestinationID")
        fleet_property = &Fleet::PreviousToFinalDestinationID;
    else if (property_name == "ArrivalStarlaneID")
        fleet_property = &Fleet::ArrivalStarlane;
    else if (property_name == "LastTurnMoveOrdered")
        fleet_property = &Fleet::LastTurnMoveOrdered;

    if (fleet_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return fleet_property(*fleet);
        }
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
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet_property(*planet);
        }
        return INVALID_GAME_TURN;
    }

    if (property_name == "TurnsSinceFocusChange") {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->TurnsSinceFocusChange(context.current_turn);
        }
        return 0;

    }
    else if (property_name == "TurnsSinceColonization") {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->TurnsSinceColonization(context.current_turn);
        }
        return 0;
    }
    else if (property_name == "TurnsSinceLastConquered") {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->TurnsSinceLastConquered(context.current_turn);
        }
        return 0;
    }
    else if (property_name == "ProducedByEmpireID") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship->ProducedByEmpireID();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
            auto building = static_cast<const Building*>(object);
            return building->ProducedByEmpireID();
        }
        return ALL_EMPIRES;

    }
    else if (property_name == "DesignID") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship->DesignID();
        }
        return INVALID_DESIGN_ID;

    }
    else if (property_name == "FleetID") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship->FleetID();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return fleet->ID();
        }
        return INVALID_OBJECT_ID;

    }
    else if (property_name == "PlanetID") {
        if (object->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
            auto building = static_cast<const Building*>(object);
            return building->PlanetID();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->ID();
        }
        return INVALID_OBJECT_ID;

    }
    else if (property_name == "NearestSystemID") {
        if (object->SystemID() != INVALID_OBJECT_ID)
            return object->SystemID();
        return context.ContextUniverse().GetPathfinder()->NearestSystemTo(
            object->X(), object->Y(), context.ContextObjects());

    }
    else if (property_name == "NumShips") {
        if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return fleet->NumShips();
        }
        return 0;

    }
    else if (property_name == "NumStarlanes") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
            auto system = static_cast<const System*>(object);
            return system->NumStarlanes();
            }
        return 0;

    }
    else if (property_name == "LastTurnBattleHere") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
            auto system = static_cast<const System*>(object);
            return system->LastTurnBattleHere();

        } else if (auto system = context.ContextObjects().getRaw<System>(object->SystemID())) {
            return system->LastTurnBattleHere();
        }
        return INVALID_GAME_TURN;

    }
    else if (property_name == "Orbit") {
        if (auto system = context.ContextObjects().getRaw<System>(object->SystemID()))
            return system->OrbitOfPlanet(object->ID());
        return -1;

    }
    else if (property_name == "ETA") {
        if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return fleet->ETA(context).first;
        }
        return 0;

    }
    else if (property_name == "NumSpecials") {
        return object->Specials().size();

    }
    else if (property_name == "LaunchedFrom") {
        if (object->ObjectType() == UniverseObjectType::OBJ_FIGHTER) {
            auto fighter = static_cast<const Fighter*>(object);
            return fighter->LaunchedFrom();
        }
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
        std::vector<std::string> retval;
        auto tags = object->Tags(context);
        retval.reserve(tags.size());
        std::transform(tags.first.begin(), tags.first.end(), std::back_inserter(retval), [](auto sv) { return std::string{sv}; });
        std::transform(tags.second.begin(), tags.second.end(), std::back_inserter(retval), [](auto sv) { return std::string{sv}; });
        return retval;
    }
    else if (property_name == "Specials") {
        auto obj_special_names_range = object->Specials() | boost::adaptors::map_keys;
        return {obj_special_names_range.begin(), obj_special_names_range.end()};
    }
    else if (property_name == "AvailableFoci") {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            auto foci_views = planet->AvailableFoci(context);
            return {foci_views.begin(), foci_views.end()};
        }
        return {};
    }
    else if (property_name == "Parts") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            if (const ShipDesign* design = context.ContextUniverse().GetShipDesign(ship->DesignID()))
                return design->Parts();
        }
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
        return std::string{to_string(object->ObjectType())};

    }

    std::function<std::string (const Empire&)> empire_property{nullptr};

    if (property_name == "OwnerLeastExpensiveEnqueuedTech")
        empire_property = [&context](const auto& empire) { return empire.LeastExpensiveEnqueuedTech(context); };
    else if (property_name == "OwnerMostExpensiveEnqueuedTech")
        empire_property = [&context](const auto& empire) { return empire.MostExpensiveEnqueuedTech(context); };
    else if (property_name == "OwnerMostRPCostLeftEnqueuedTech")
        empire_property = [&context](const auto& empire) { return empire.MostRPCostLeftEnqueuedTech(context); };
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
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->SpeciesName();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship->SpeciesName();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_FIGHTER) {
            auto fighter = static_cast<const Fighter*>(object);
            return fighter->SpeciesName();
        }
        return "";

    } else if (property_name == "Hull") {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            if (const ShipDesign* design = context.ContextUniverse().GetShipDesign(ship->DesignID()))
                return design->Hull();
        }
        return "";

    } else if (property_name == "FieldType") {
        if (object->ObjectType() == UniverseObjectType::OBJ_FIELD) {
            auto field = static_cast<const Field*>(object);
            return field->FieldTypeName();
        }
        return "";

    } else if (property_name == "BuildingType") {
        if (object->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
            auto building = static_cast<const Building*>(object);
            return building->BuildingTypeName();
        }
        return "";

    } else if (property_name == "Focus") {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->Focus();
        }
        return "";

    } else if (property_name == "DefaultFocus") {
        const Species* species = nullptr;
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            species = context.species.GetSpecies(planet->SpeciesName());

        } else if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            species = context.species.GetSpecies(ship->SpeciesName());
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
    const auto* scond = m_sampling_condition.get();
    if (!scond)
        return "";

    // special case for IF statistic... return a non-empty string for true
    if (m_stat_type == StatisticType::IF)
        return scond->EvalAny(context) ? " " : "";

    // the only other statistic that can be computed on non-number property
    // types and that is itself of a non-number type is the most common value
    if (m_stat_type != StatisticType::MODE) {
        ErrorLogger() << "Statistic<std::string, std::string>::Eval has invalid statistic type: "
                      << m_stat_type;
        return "";
    }
    // TODO: consider allowing MAX and MIN using string sorting?

    const Condition::ObjectSet condition_matches = scond->Eval(context);
    if (condition_matches.empty())
        return "";  // empty string

    // evaluate property for each condition-matched object
    auto object_property_values = GetObjectPropertyValues(context, condition_matches);

    // count appearances to determine the value that appears the most often
    std::unordered_map<std::string, uint32_t> observed_values;
    observed_values.reserve(object_property_values.size());
    for (auto& entry : object_property_values)
        observed_values[std::move(entry)]++;

    auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                [](const auto& p1, const auto& p2) -> bool { return p1.second < p2.second; });

    return max->first;
}

///////////////////////////////////////////////////////////
// TotalFighterShots (of a carrier during one battle)    //
///////////////////////////////////////////////////////////
TotalFighterShots::TotalFighterShots(std::unique_ptr<ValueRef<int>>&& carrier_id,
                                     std::unique_ptr<Condition::Condition>&& sampling_condition) :
    Variable<int>(ReferenceType::NON_OBJECT_REFERENCE),
    m_carrier_id(std::move(carrier_id)),
    m_sampling_condition(std::move(sampling_condition))
{
    this->m_root_candidate_invariant = (!m_sampling_condition || m_sampling_condition->RootCandidateInvariant())
                                       && (!m_carrier_id || m_carrier_id->RootCandidateInvariant()) ;

    // no condition can explicitly reference the parent context's local candidate.
    // so local candidate invariance does not depend on the sampling condition
    this->m_local_candidate_invariant = (!m_carrier_id || m_carrier_id->LocalCandidateInvariant()) ;

    this->m_target_invariant = (!m_sampling_condition || m_sampling_condition->TargetInvariant())
                               && (!m_carrier_id || m_carrier_id->TargetInvariant()) ;

    this->m_source_invariant = true;
}

bool TotalFighterShots::operator==(const ValueRef<int>& rhs) const {
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const TotalFighterShots& rhs_ = static_cast<const TotalFighterShots&>(rhs);

    if (m_sampling_condition == rhs_.m_sampling_condition) {
        // check next member
    } else if (m_carrier_id == rhs_.m_carrier_id) {
        return true;
    }
    return false;
}

std::string TotalFighterShots::Description() const {
    std::string retval = "TotalFighterShots(";
    if (m_carrier_id) {
        retval += m_carrier_id->Description();
        retval += " ";
    }
    if (m_sampling_condition) {
        retval += m_sampling_condition->Description();
    }
    retval += ")";
    return retval;
}

std::string TotalFighterShots::Dump(uint8_t ntabs) const {
    std::string retval = "TotalFighterShots";
    if (m_carrier_id)
        retval += " carrier = " + m_carrier_id->Dump();
    if (m_sampling_condition)
        retval += " condition = " + m_sampling_condition->Dump();
    return retval;
}

void TotalFighterShots::SetTopLevelContent(const std::string& content_name) {
    if (m_sampling_condition)
        m_sampling_condition->SetTopLevelContent(content_name);
}

uint32_t TotalFighterShots::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::TotalFighterShots");
    CheckSums::CheckSumCombine(retval, m_carrier_id);
    CheckSums::CheckSumCombine(retval, m_sampling_condition);
    TraceLogger() << "GetCheckSum(TotalFighterShots):  retval: " << retval;
    return retval;
}

int TotalFighterShots::Eval(const ScriptingContext& context) const {
    if (!m_carrier_id) {
        ErrorLogger() << "TotalFighterShots condition without carrier id";
        return 0;
    } else {
        auto carrier = context.ContextObjects().getRaw<Ship>(m_carrier_id->Eval(context));
        if (!carrier) {
            ErrorLogger() << "TotalFighterShots condition referenced a carrier which is not a ship";
            return 0;
        }
        return Combat::TotalFighterShots(context, *carrier, m_sampling_condition.get());
    }
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
        return planet->EnvironmentForSpecies(context, species_name);
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
            if (empire_id == ALL_EMPIRES && context.combat_bout < 1)
                return Visibility::VIS_FULL_VISIBILITY; // outside of battle neutral forces have full visibility per default
        }

        int object_id = INVALID_OBJECT_ID;
        if (m_int_ref2) {
            object_id = m_int_ref2->Eval(context);
            if (object_id == INVALID_OBJECT_ID)
                return Visibility::VIS_NO_VISIBILITY;
        }

        return context.ContextVis(object_id, empire_id);
    }

    return Visibility::INVALID_VISIBILITY;
}

template <>
int ComplexVariable<int>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    using boost::container::flat_map;
    // empire properties indexed by strings
    std::function<const std::map<std::string, int>&              (const Empire&)> empire_property_string_key {nullptr};
    std::function<const std::map<std::string, int, std::less<>>& (const Empire&)> empire_property_string_key2{nullptr};
    std::function<const flat_map<std::string, int, std::less<>>& (const Empire&)> empire_property_string_key3{nullptr};

    if (variable_name == "TurnTechResearched")
        empire_property_string_key3 = &Empire::ResearchedTechs;

    else if (variable_name == "BuildingTypesOwned")
        empire_property_string_key = &Empire::BuildingTypesOwned;
    else if (variable_name == "BuildingTypesProduced")
        empire_property_string_key = &Empire::BuildingTypesProduced;
    else if (variable_name == "BuildingTypesScrapped")
        empire_property_string_key = &Empire::BuildingTypesScrapped;
    else if (variable_name == "SpeciesColoniesOwned")
        empire_property_string_key = &Empire::SpeciesColoniesOwned;
    else if (variable_name == "SpeciesPlanetsBombed")
        empire_property_string_key = &Empire::SpeciesPlanetsBombed;
    else if (variable_name == "SpeciesPlanetsDepoped")
        empire_property_string_key = &Empire::SpeciesPlanetsDepoped;
    else if (variable_name == "SpeciesPlanetsInvaded")
        empire_property_string_key = &Empire::SpeciesPlanetsInvaded;
    else if (variable_name == "SpeciesShipsDestroyed")
        empire_property_string_key = &Empire::SpeciesShipsDestroyed;
    else if (variable_name == "SpeciesShipsLost")
        empire_property_string_key = &Empire::SpeciesShipsLost;
    else if (variable_name == "SpeciesShipsOwned")
        empire_property_string_key = &Empire::SpeciesShipsOwned;
    else if (variable_name == "SpeciesShipsProduced")
        empire_property_string_key = &Empire::SpeciesShipsProduced;
    else if (variable_name == "SpeciesShipsScrapped")
        empire_property_string_key = &Empire::SpeciesShipsScrapped;
    else if (variable_name == "ShipPartsOwned")
        empire_property_string_key = &Empire::ShipPartsOwned;
    else if (variable_name == "TurnsSincePolicyAdopted")
        empire_property_string_key = &Empire::PolicyCurrentAdoptedDurations;
    else if (variable_name == "CumulativeTurnsPolicyAdopted")
        empire_property_string_key = &Empire::PolicyTotalAdoptedDurations;

    if (empire_property_string_key || empire_property_string_key2 || empire_property_string_key3) {
        using namespace boost::adaptors;

        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
        }

        std::function<bool (const std::map<std::string, int>::value_type&)> key_filter{nullptr};
        key_filter = [](auto e) -> bool { return true; };

        if (m_string_ref1) {
            std::string key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;

            if (empire && variable_name == "TurnTechResearched" && !empire->TechResearched(key_string))
                // special case for techs: make unresearched-tech's research-turn a big number
                return IMPOSSIBLY_LARGE_TURN;

            key_filter = [k{std::move(key_string)}](auto e) -> bool { return k == e.first; };
        }
        else if (variable_name == "ShipPartsOwned" && m_int_ref2) {
            int key_int = m_int_ref2->Eval(context);
            if (key_int <= int(ShipPartClass::INVALID_SHIP_PART_CLASS) ||
                key_int >= int(ShipPartClass::NUM_SHIP_PART_CLASSES))
            { return 0; }

            auto key_filter_class = [part_class = ShipPartClass(key_int)](const std::map<ShipPartClass, int>::value_type& e){ return e.first == part_class; };

            if (empire)
                return boost::accumulate(empire->ShipPartClassOwned() | filtered(key_filter_class) | map_values, 0);

            int sum = 0;
            for ([[maybe_unused]] auto& [ignored_id, loop_empire] : context.Empires()) {
                (void)ignored_id; // quiet unused variable warning
                sum += boost::accumulate(loop_empire->ShipPartClassOwned() | filtered(key_filter_class) | map_values, 0);
            }
            return sum;
        }

        if (empire) {
            if (empire_property_string_key)
                return boost::accumulate(empire_property_string_key(*empire) | filtered(key_filter) | map_values, 0);
            else if (empire_property_string_key2)
                return boost::accumulate(empire_property_string_key2(*empire) | filtered(key_filter) | map_values, 0);
            else if (empire_property_string_key3)
                return boost::accumulate(empire_property_string_key3(*empire) | filtered(key_filter) | map_values, 0);
        }

        int sum = 0;
        for ([[maybe_unused]] auto& [ignored_id, loop_empire] : context.Empires()) {
            (void)ignored_id; // quiet unused variable warning
            if (empire_property_string_key)
                sum += boost::accumulate(empire_property_string_key(*loop_empire) | filtered(key_filter) | map_values, 0);
            else if (empire_property_string_key2)
                sum += boost::accumulate(empire_property_string_key2(*loop_empire) | filtered(key_filter) | map_values, 0);
            else if (empire_property_string_key3)
                sum += boost::accumulate(empire_property_string_key3(*loop_empire) | filtered(key_filter) | map_values, 0);
        }
        return sum;
    }


    // empire properties indexed by integers
    std::function<const std::map<int, int>& (const Empire&)> empire_property_int_key{nullptr};

    if (variable_name == "EmpireShipsDestroyed")
        empire_property_int_key = &Empire::EmpireShipsDestroyed;
    else if (variable_name == "ShipDesignsDestroyed")
        empire_property_int_key = &Empire::ShipDesignsDestroyed;
    else if (variable_name == "ShipDesignsLost")
        empire_property_int_key = &Empire::ShipDesignsLost;
    else if (variable_name == "ShipDesignsOwned")
        empire_property_int_key = &Empire::ShipDesignsOwned;
    else if (variable_name == "ShipDesignsInProduction")
        empire_property_int_key = &Empire::ShipDesignsInProduction;
    else if (variable_name == "ShipDesignsProduced")
        empire_property_int_key = &Empire::ShipDesignsProduced;
    else if (variable_name == "ShipDesignsScrapped")
        empire_property_int_key = &Empire::ShipDesignsScrapped;
    else if (variable_name == "TurnSystemExplored")
        empire_property_int_key = &Empire::TurnsSystemsExplored;

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
        key_filter = [](auto e) -> bool { return true; };

        // if a key integer specified, get just that entry (for single empire or sum of all empires)
        if (m_int_ref2)
            key_filter = [k = m_int_ref2->Eval(context)](auto e) -> bool { return k == e.first; };

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
                const ShipDesign* design = context.ContextUniverse().GetGenericShipDesign(key_string);
                if (design)
                    key_int = design->ID();
            }
            key_filter = [k = key_int](auto e) -> bool { return k == e.first; };
        }

        if (empire)
            return boost::accumulate(empire_property_int_key(*empire) | filtered(key_filter) | map_values, 0);

        int sum = 0;
        for ([[maybe_unused]] auto& [ignored_id, loop_empire] : context.Empires()) {
            (void)ignored_id; // quiet unused variable warning
            sum += boost::accumulate(empire_property_int_key(*loop_empire) | filtered(key_filter) | map_values, 0);
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
        auto GetRawPtr = [](const auto& smart_ptr) { return smart_ptr.get(); };

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
            case GameRule::Type::TOGGLE: {
                return GetGameRules().Get<bool>(rule_name);
                break;
            }
            case GameRule::Type::INT: {
                return GetGameRules().Get<int>(rule_name);
                break;
            }
            case GameRule::Type::DOUBLE: {
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

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
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

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
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

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
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
    else if (variable_name == "TurnPolicyAdopted") { // returns by value, so can't assign &Empire::TurnsPoliciesAdopted to empire_property_string_key above
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
    else if (variable_name == "NumPoliciesAdopted") { // similar to a string-keyed empire property, but does specialized lookups of adopted policy info
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
        }
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return 0;

        std::string policy_category_name;
        if (m_string_ref1)
            policy_category_name = m_string_ref1->Eval();

        int count = 0;
        if (policy_category_name.empty()) {
            for (auto& p : empire->AdoptedPolicies()) {
                if (!p.empty())
                    ++count;
            }
            return count;

        } else {
            for (auto& [cat_name, policy_names_turns] : empire->CategoriesSlotsPoliciesAdopted()) {
                if (cat_name == policy_category_name) {
                    for (auto& [ignored_turn, policy_name] : policy_names_turns) {
                        (void)ignored_turn;
                        if (!policy_name.empty())
                            ++count;
                    }
                }
            }
            return count;
        }
    }
    else if (variable_name == "PlanetTypeDifference") {
        // get planet types to find difference between...
        PlanetType pt1;
        if (m_int_ref1) {
            int pt_int1 = m_int_ref1->Eval(context);
            pt1 = PlanetType(pt_int1);
        } else if (m_string_ref1) {
            const std::string pt_name1 = m_string_ref1->Eval(context);
            pt1 = StringToPlanetTypeCX(pt_name1);
        } else {
            return 0;
        }

        PlanetType pt2;
        if (m_int_ref2) {
            int pt_int2 = m_int_ref2->Eval(context);
            pt2 = PlanetType(pt_int2);
        } else if (m_string_ref2) {
            const std::string pt_name2 = m_string_ref2->Eval(context);
            pt2 = StringToPlanetTypeCX(pt_name2);
        } else {
            return 0;
        }

        return Planet::TypeDifference(pt1, pt2);
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
    std::function<const std::map<int, float>& (const Empire&)> empire_property_int_key{nullptr};

    if (variable_name == "SystemSupplyRange") {
        using namespace boost::adaptors;

        if (!m_int_ref2)
            return 0.0; // no system specified... doesn't make sense to sum over systems...
        int system_id = m_int_ref2->Eval(context);
        if (system_id == INVALID_OBJECT_ID)
            return 0.0;

        if (m_int_ref1) {
            // single empire ID specified
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0.0;
            auto empire = context.GetEmpire(empire_id);
            if (!empire)
                return 0.0;
            const auto& data = empire->SystemSupplyRanges();

            auto it = data.find(system_id);
            if (it != data.end())
                return it->second;
            else
                return 0.0;

        } else {
            float empires_max = 0.0f;
            // no empire ID specified, use max of all empires' ranges at specified system
            for ([[maybe_unused]] auto& [unused_id, loop_empire] : context.Empires()) {
                (void)unused_id; // quiet unused variable warning
                const auto& empire_data = loop_empire->SystemSupplyRanges();
                auto it = empire_data.find(system_id);
                if (it != empire_data.end())
                    empires_max = std::max(empires_max, it->second);
            }
            return empires_max;
        }
    }


    if (variable_name == "EmpireStockpile") {
        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
            if (!empire)
                return 0;
        }

        ResourceType res_type = ResourceType::INVALID_RESOURCE_TYPE;
        if (m_string_ref1) {
            const std::string res_name = m_string_ref1->Eval(context);
            const auto meter_type = NameToMeterCX(res_name);
            res_type = MeterToResource(meter_type);
        }
        if (res_type == ResourceType::INVALID_RESOURCE_TYPE)
            return 0;

        std::function<int (const Empire*)> empire_property{nullptr};

        empire_property = [res_type](const Empire* empire) { return empire->ResourceStockpile(res_type); };

        using namespace boost::adaptors;
        auto GetRawPtr = [](const auto& smart_ptr){ return smart_ptr.get(); };

        if (!empire) {
            return boost::accumulate(context.Empires() | map_values | transformed(GetRawPtr) |
                                     transformed(empire_property), 0);
        }

        return empire_property(empire.get());
    }


    // non-empire properties
    std::function<const std::map<int, float>& ()> property_int_key{nullptr};

    if (variable_name == "PropagatedSystemSupplyRange") // int_ref2 is system ID
        property_int_key = [&context]() -> const auto& { return context.supply.PropagatedSupplyRanges(); };
    else if (variable_name == "PropagatedSystemSupplyDistance") // int_ref2 is system ID
        property_int_key = [&context]() -> const auto& { return context.supply.PropagatedSupplyDistances(); };

    if (property_int_key) {
        if (!m_int_ref2)
            return 0.0; // no system specified... doesn't make sense to sum over systems...
        int system_id = m_int_ref2->Eval(context);
        if (system_id == INVALID_OBJECT_ID)
            return 0.0;

        const auto& data = property_int_key();
        auto it = data.find(system_id);
        if (it != data.end())
            return it->second;
        else
            return 0.0;
    }


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
            case GameRule::Type::TOGGLE: {
                return GetGameRules().Get<bool>(rule_name);
                break;
            }
            case GameRule::Type::INT: {
                return GetGameRules().Get<int>(rule_name);
                break;
            }
            case GameRule::Type::DOUBLE: {
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

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
        if (!design)
            return 0.0;

        int empire_id = ALL_EMPIRES;
        if (m_int_ref2)
            empire_id = m_int_ref2->Eval(context);

        int location_id = INVALID_OBJECT_ID;
        if (m_int_ref3)
            location_id = m_int_ref3->Eval(context);

        return design->ProductionCost(empire_id, location_id, context);

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

        return context.ContextUniverse().GetPathfinder()->ShortestPathDistance(
            object1_id, object2_id, context.ContextObjects());

    }
    else if (variable_name == "SpeciesContentOpinion") {
        std::string opinionated_species_name;
        if (m_string_ref1)
            opinionated_species_name = m_string_ref1->Eval(context);
        const auto species = context.species.GetSpecies(opinionated_species_name);
        if (!species)
            return 0.0;

        std::string liked_or_disliked_content_name;
        if (m_string_ref2)
            liked_or_disliked_content_name = m_string_ref2->Eval(context);
        if (liked_or_disliked_content_name.empty())
            return 0.0;

        if (std::any_of(species->Likes().begin(), species->Likes().end(),
                        [&liked_or_disliked_content_name](const auto& l) { return l == liked_or_disliked_content_name; }))
        { return 1.0; }
        else if (std::any_of(species->Dislikes().begin(), species->Dislikes().end(),
                             [&liked_or_disliked_content_name](const auto& d) { return d == liked_or_disliked_content_name; }))
        { return -1.0; }
        else
        { return 0.0; }

    }
    else if (variable_name == "SpeciesEmpireOpinion") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1)
            empire_id = m_int_ref1->Eval(context);

        std::string species_name;
        if (m_string_ref1)
            species_name = m_string_ref1->Eval(context);

        return context.species.SpeciesEmpireOpinion(species_name, empire_id);

    }
    else if (variable_name == "SpeciesSpeciesOpinion") {
        std::string opinionated_species_name;
        if (m_string_ref1)
            opinionated_species_name = m_string_ref1->Eval(context);

        std::string rated_species_name;
        if (m_string_ref2)
            rated_species_name = m_string_ref2->Eval(context);

        return context.species.SpeciesSpeciesOpinion(opinionated_species_name, rated_species_name);

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
        auto ship = context.ContextObjects().getRaw<const Ship>(object_id);
        if (!ship)
            return 0.0;

        if (!m_string_ref1)
            return 0.0;
        std::string part_name = m_string_ref1->Eval(context);
        if (part_name.empty())
            return 0.0;

        if (!m_string_ref2)
            return 0.0;
        std::string meter_name = m_string_ref2->Eval(context);
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
    [[nodiscard]] std::vector<std::string> TechsResearchedByEmpire(int empire_id, const ScriptingContext& context) {
        auto empire = context.GetEmpire(empire_id);
        if (!empire) return {};

        auto researched_techs_range = empire->ResearchedTechs() | boost::adaptors::map_keys;
        return {researched_techs_range.begin(), researched_techs_range.end()};
    }

    [[nodiscard]] std::vector<std::string> TechsResearchableByEmpire(int empire_id, const ScriptingContext& context) {
        std::vector<std::string> retval;
        auto empire = context.GetEmpire(empire_id);
        if (!empire) return retval;

        retval.reserve(GetTechManager().size());
        // transform_if
        for (const auto& [tech_name, ignored] : GetTechManager()) {
            (void)ignored;
            if (empire->ResearchableTech(tech_name))
                retval.push_back(tech_name);
        }
        return retval;
    }

    [[nodiscard]] std::vector<std::string> TransferrableTechs(int sender_empire_id, int receipient_empire_id,
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
        empire_property = [&context](const Empire& e) { return e.LeastExpensiveEnqueuedTech(context); };
    else if (variable_name == "HighestCostEnqueuedTech")
        empire_property = [&context](const Empire& e) { return e.MostExpensiveEnqueuedTech(context); };
    else if (variable_name == "TopPriorityEnqueuedTech")
        empire_property = &Empire::TopPriorityEnqueuedTech;
    else if (variable_name == "MostSpentEnqueuedTech")
        empire_property = &Empire::MostRPSpentEnqueuedTech;
    else if (variable_name == "LowestCostResearchableTech")
        empire_property = [&context](const auto& empire) { return empire.LeastExpensiveResearchableTech(context); };
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
            float rc = tech->ResearchCost(empire2_id, context);
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

        auto sendable_techs = TransferrableTechs(empire1_id, empire2_id, context);
        if (sendable_techs.empty())
            return "";

        std::string retval = sendable_techs.front();   // pick first tech by default, hopefully to be replaced below
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
            case GameRule::Type::TOGGLE: {
                return std::to_string(GetGameRules().Get<bool>(rule_name));
                break;
            }
            case GameRule::Type::INT: {
                return std::to_string(GetGameRules().Get<int>(rule_name));
                break;
            }
            case GameRule::Type::DOUBLE: {
                return DoubleToString(GetGameRules().Get<double>(rule_name), 3, false);
                break;
            }
            case GameRule::Type::STRING: {
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

        auto pols = empire->AdoptedPolicies();
        std::vector<std::string> retval;
        retval.reserve(pols.size());
        std::transform(pols.begin(), pols.end(), std::back_inserter(retval),
                       [](const std::string_view sv) -> std::string { return std::string{sv}; });
        return retval;

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

        const auto& pols = empire->AvailablePolicies();
        return std::vector<std::string>{pols.begin(), pols.end()};
    }

    return {};
}

#undef IF_CURRENT_VALUE

template <>
std::string ComplexVariable<Visibility>::Dump(uint8_t ntabs) const
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
std::string ComplexVariable<double>::Dump(uint8_t ntabs) const
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
std::string ComplexVariable<int>::Dump(uint8_t ntabs) const
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
std::string ComplexVariable<std::string>::Dump(uint8_t ntabs) const
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
std::string ComplexVariable<std::vector<std::string>>::Dump(uint8_t ntabs) const
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

    auto raw_ref = m_value_ref.get();
    if (!raw_ref)
        return "";

    double result = raw_ref->Eval(context);
    auto Stringify = [](double num) -> std::string {
        const auto abs_num = std::abs(num);
        if (abs_num < 0.1 || abs_num >= 1000)
            return DoubleToString(num, 3, false);

        int precision = abs_num < 10 ? 2 : 1; // number of fractional decimal places

        // TODO: check if locale correctly does round trip of something with
        // a decimal place indicator using to_chars and from_chars.
        // if not, need to use streaming always?
#if defined(__cpp_lib_to_chars)
            std::array<std::string::value_type, 32> buf = {};
            std::to_chars(buf.data(), buf.data() + buf.size(), num, std::chars_format::fixed, precision);
            return buf.data();
#else
            std::stringstream ss;
            ss << std::fixed << std::setprecision(precision) << num;
            return ss.str();
#endif
    };


    auto ref = dynamic_cast<Variable<double>*>(raw_ref);
    if (!ref)
        return Stringify(result);

    // special case for a few sub-value-refs to help with UI representation
    const auto& property = ref->PropertyName();
    if (property.empty())
        return Stringify(result);

    const auto& end_of_property = property.back();
    if (end_of_property.empty())
        return Stringify(result);

    // special case for a few sub-value-refs to help with UI representation
    if (end_of_property == "X" || end_of_property == "Y" || end_of_property == "DirectDistanceBetween") {
        if (result == UniverseObject::INVALID_POSITION)
            return UserString("INVALID_POSITION");

        std::stringstream ss;
        ss << std::setprecision(6) << result;
        return ss.str();
    }

    return Stringify(result);
}

template <>
std::string StringCast<int>::Eval(const ScriptingContext& context) const
{
    if (!m_value_ref)
        return "";

    auto raw_ref = m_value_ref.get();
    if (!raw_ref)
        return "";

    int result = raw_ref->Eval(context);

    auto int_ref = dynamic_cast<Variable<int>*>(raw_ref);
    if (!int_ref)
        return std::to_string(result);

    // special case for a few sub-value-refs to help with UI representation
    const auto& property = int_ref->PropertyName();
    if (property.empty())
        return std::to_string(result);

    const auto& end_of_property = property.back();
    if (end_of_property.empty())
        return std::to_string(result);

    if (end_of_property == "ETA") {
        if (result == Fleet::ETA_UNKNOWN) {
            return UserString("FW_FLEET_ETA_UNKNOWN");
        } else if (result == Fleet::ETA_NEVER) {
            return UserString("FW_FLEET_ETA_NEVER");
        } else if (result == Fleet::ETA_OUT_OF_RANGE) {
            return UserString("FW_FLEET_ETA_OUT_OF_RANGE");
        }
    }

    return std::to_string(result);
}

template <>
std::string StringCast<std::vector<std::string>>::Eval(const ScriptingContext& context) const
{
    if (!m_value_ref)
        return "";
    std::vector<std::string> temp = m_value_ref->Eval(context);

    // concatenate strings into one big string
    std::string retval;
    retval.reserve(16 * temp.size()); // rough guesstimate to avoid reallocations
    for (const auto& str : temp)
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
    std::string retval;
    if (!m_value_ref)
        return retval;
    auto ref_vals = m_value_ref->Eval(context);
    if (ref_vals.empty())
        return retval;
    for (auto& val : ref_vals) {
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
    m_constant_expr = !m_value_ref; // should be false if an object ID is provided, since the name of that object is gamestate and is not known at initialization time and can vary with time
    //m_simple_increment = false; // should be always false for this class
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
        const ShipDesign* design = context.ContextUniverse().GetShipDesign(m_value_ref->Eval(context));
        return design ? design->Name() : "";
        break;
    }
    default:
        return "";
    }
}

std::string NameLookup::Description() const
{ return m_value_ref->Description(); }

std::string NameLookup::Dump(uint8_t ntabs) const
{ return m_value_ref->Dump(ntabs); }

void NameLookup::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

uint32_t NameLookup::GetCheckSum() const {
    uint32_t retval{0};

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
std::string Operation<std::string>::EvalImpl(OpType op_type, std::string lhs, std::string rhs)
{
    switch (op_type) {
    case OpType::PLUS: {
        return lhs + rhs;
        break;
    }

    case OpType::TIMES: {
        // useful for writing a "Statistic If" expression with strings. Number-
        // valued types return 0 or 1 for nothing or something matching the sampling
        // condition. For strings, an empty string indicates no matches, and non-empty
        // string indicates matches, which is treated like a multiplicative identity
        // operation, so just returns the RHS of the expression.
        return lhs.empty() ? lhs : rhs;
        break;
    }

    case OpType::MINIMUM: {
        return std::min(lhs, rhs);
        break;
    }

    case OpType::MAXIMUM: {
        return std::max(lhs, rhs);
        break;
    }

    case OpType::RANDOM_PICK: {
        return (RandInt(0, 1) == 0) ? lhs : rhs;
        break;
    }

    case OpType::COMPARE_EQUAL: {
        return (lhs == rhs) ? "true" : "false";
        break;
    }
    case OpType::COMPARE_GREATER_THAN: {
        return (lhs > rhs) ? "true" : "false";
        break;
    }
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL: {
        return (lhs >= rhs) ? "true" : "false";
        break;
    }
    case OpType::COMPARE_LESS_THAN: {
        return (lhs < rhs) ? "true" : "false";
        break;
    }
    case OpType::COMPARE_LESS_THAN_OR_EQUAL: {
        return (lhs <= rhs) ? "true" : "false";
        break;
    }
    case OpType::COMPARE_NOT_EQUAL:  {
        return (lhs != rhs) ? "true" : "false";
        break;
    }

    case OpType::SUBSTITUTION: {
        // insert string into other string in place of %1% or similar placeholder
        if (lhs.empty())
            return lhs;

        boost::format formatter = FlexibleFormat(lhs);
        formatter % rhs;
        return formatter.str();
        break;
    }

    default: break;
    }

    throw std::runtime_error("ValueRef::Operation<std::string> evaluated with an unknown or invalid OpType.");
    return "";
}

template <>
double Operation<double>::EvalImpl(OpType op_type, double lhs, double rhs)
{
    switch (op_type) {
    case OpType::PLUS: {
        return lhs + rhs;
        break;
    }

    case OpType::MINUS: {
        return lhs - rhs;
        break;
    }

    case OpType::TIMES: {
        return lhs * rhs;
        break;
    }

    case OpType::DIVIDE: {
        if (rhs == 0.0)
            return 0.0;
        return lhs / rhs;
        break;
    }

    case OpType::REMAINDER: {
        double divisor = std::abs(rhs);
        if (divisor == 0.0)
            return 0.0;
        auto dividend = lhs;
        auto quotient = std::floor(dividend / divisor);
        return dividend - quotient * divisor;
        break;
    }

    case OpType::NEGATE: {
        return -lhs;
        break;
    }

    case OpType::EXPONENTIATE: {
        if (rhs == 0.0)
            return 1.0;
        try {
            return static_cast<int>(std::pow(static_cast<double>(lhs), static_cast<double>(rhs)));
        } catch (...) {
            ErrorLogger() << "Error evaluating exponentiation ValueRef::Operation";
            return 0;
        }
        break;
    }

    case OpType::NOOP: {
        return lhs;
        break;
    }

    case OpType::ABS: {
        return std::abs(lhs);
        break;
    }

    case OpType::LOGARITHM: {
        if (lhs <= 0.0)
            return 0.0;
        return std::log(lhs);
        break;
    }

    case OpType::SINE: {
        return std::sin(lhs);
        break;
    }

    case OpType::COSINE: {
        return std::cos(lhs);
        break;
    }

    case OpType::MINIMUM: {
        return std::min(lhs, rhs);
        break;
    }

    case OpType::MAXIMUM: {
        return std::max(lhs, rhs);
        break;
    }

    case OpType::RANDOM_UNIFORM: {
        return RandDouble(std::min(lhs, rhs), std::max(rhs, lhs));
        break;
    }

    case OpType::RANDOM_PICK: {
        return (RandInt(0, 1) == 0) ? lhs : rhs;
        break;
    }

    case OpType::COMPARE_EQUAL: {
        return (lhs == rhs);
        break;
    }
    case OpType::COMPARE_GREATER_THAN: {
        return (lhs > rhs);
        break;
    }
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL: {
        return (lhs >= rhs);
        break;
    }
    case OpType::COMPARE_LESS_THAN: {
        return (lhs < rhs);
        break;
    }
    case OpType::COMPARE_LESS_THAN_OR_EQUAL: {
        return (lhs <= rhs);
        break;
    }
    case OpType::COMPARE_NOT_EQUAL:  {
        return (lhs != rhs);
        break;
    }

    case OpType::ROUND_NEAREST: {
        return std::round(lhs);
        break;
    }
    case OpType::ROUND_UP: {
        return std::ceil(lhs);
        break;
    }
    case OpType::ROUND_DOWN: {
        return std::floor(lhs);
        break;
    }

    case OpType::SIGN: {
        static constexpr double test_case = -42.1;
        static constexpr double branchless_test = (0.0 < test_case) - (test_case < 0.0);
        static constexpr double ternary_condition_test = test_case < 0.0 ? -1.0 : test_case > 0.0 ? 1.0 : 0.0;
        static_assert(branchless_test == ternary_condition_test);
        return (0.0 < lhs) - (lhs < 0.0);
        break;
    }

    default:    break;
    }

    throw std::runtime_error("ValueRef::Operation<double> evaluated with an unknown or invalid OpType.");
    return 0.0;
}

template <>
int Operation<int>::EvalImpl(OpType op_type, int lhs, int rhs)
{
    switch (op_type) {
    case OpType::PLUS: {
        return lhs + rhs;
        break;
    }

    case OpType::MINUS: {
        return lhs - rhs;
        break;
    }

    case OpType::TIMES: {
        return lhs * rhs;
        break;
    }

    case OpType::DIVIDE: {
        if (rhs == 0)
            return 0;
        return lhs / rhs;
        break;
    }

    case OpType::REMAINDER: {
        if (rhs == 0)
            return 0;
        return lhs % rhs;
        break;
    }

    case OpType::NEGATE: {
        return -lhs;
        break;
    }

    case OpType::EXPONENTIATE: {
        if (rhs == 0)
            return 1;
        try {
            return static_cast<int>(std::pow(static_cast<double>(lhs), static_cast<double>(rhs)));
        } catch (...) {
            ErrorLogger() << "Error evaluating exponentiation ValueRef::Operation";
            return 0;
        }
        break;
    }

    case OpType::NOOP: {
        return lhs;
        break;
    }

    case OpType::ABS: {
        return std::abs(lhs);
        break;
    }

    case OpType::LOGARITHM: {
        if (lhs <= 0)
            return 0;
        return static_cast<int>(std::log(static_cast<double>(lhs)));
        break;
    }

    case OpType::SINE: {
        return static_cast<int>(std::round(std::sin(static_cast<double>(lhs))));
        break;
    }

    case OpType::COSINE: {
        return static_cast<int>(std::round(std::cos(static_cast<double>(lhs))));
        break;
    }

    case OpType::MINIMUM: {
        return std::min(lhs, rhs);
        break;
    }

    case OpType::MAXIMUM: {
        return std::max(lhs, rhs);
        break;
    }

    case OpType::RANDOM_UNIFORM: {
        return RandInt(std::min(lhs, rhs), std::max(rhs, lhs));
        break;
    }

    case OpType::RANDOM_PICK: {
        return (RandInt(0, 1) == 0) ? lhs : rhs;
        break;
    }

    case OpType::COMPARE_EQUAL: {
        return (lhs == rhs);
        break;
    }
    case OpType::COMPARE_GREATER_THAN: {
        return (lhs > rhs);
        break;
    }
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL: {
        return (lhs >= rhs);
        break;
    }
    case OpType::COMPARE_LESS_THAN: {
        return (lhs < rhs);
        break;
    }
    case OpType::COMPARE_LESS_THAN_OR_EQUAL: {
        return (lhs <= rhs);
        break;
    }
    case OpType::COMPARE_NOT_EQUAL:  {
        return (lhs != rhs);
        break;
    }

    case OpType::ROUND_NEAREST:
    case OpType::ROUND_UP:
    case OpType::ROUND_DOWN: {
        // integers don't need to be rounded...
        return lhs;
        break;
    }

    case OpType::SIGN: {
        static constexpr int test_case = -42;
        static constexpr int branchless_test = (0 < test_case) - (test_case < 0);
        static constexpr int ternary_condition_test = test_case < 0 ? -1 : test_case > 0 ? 1 : 0;
        static_assert(branchless_test == ternary_condition_test);
        return (0 < lhs) - (lhs < 0);
        break;
    }

    default:    break;
    }

    throw std::runtime_error("ValueRef::Operation<int> evaluated with an unknown or invalid OpType.");
    return 0;
}

template <>
std::string Operation<std::string>::EvalImpl(const ScriptingContext& context) const
{
    if (m_simple_increment)
        return EvalImpl(m_op_type, LHS()->Eval(context), RHS()->Eval(context));

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
        if (m_operands.empty())
            return "";

        // evaluate all operands, return smallest or biggest
        std::vector<std::string> vals;
        vals.reserve(m_operands.size());
        for (auto& vr : m_operands) {
            if (vr)
                vals.emplace_back(vr->Eval(context));
        }
        if (m_op_type == OpType::MINIMUM)
            return *std::min_element(vals.begin(), vals.end());
        else
            return *std::max_element(vals.begin(), vals.end());

    } else if (m_op_type == OpType::RANDOM_PICK) {
        // select one operand, evaluate it, return result
        if (m_operands.empty())
            return "";
        std::ptrdiff_t idx = RandInt(0, m_operands.size() - 1);
        auto& vr = *std::next(m_operands.begin(), idx);
        if (!vr)
            return "";
        return vr->Eval(context);

    } else if (m_op_type == OpType::SUBSTITUTION) {
        // insert string into other string in place of %1% or similar placeholder
        if (m_operands.empty())
            return "";
        auto& template_op = m_operands.front();
        if (!template_op)
            return "";
        const std::string template_str = template_op->Eval(context);

        boost::format formatter = FlexibleFormat(template_str);

        for (auto& op : m_operands)
            formatter % (op ? op->Eval(context) : "");
        return formatter.str();

    } else if (m_op_type >= OpType::COMPARE_EQUAL && m_op_type <= OpType::COMPARE_NOT_EQUAL) {
        std::string lhs_val = LHS()->Eval(context);
        std::string rhs_val = RHS()->Eval(context);
        bool test_result = false;
        if (m_operands.size() == 2)
            return EvalImpl(m_op_type, lhs_val, rhs_val);

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

    throw std::runtime_error("ValueRef::Operation<std::string> evaluated with an unknown or invalid OpType.");
    return "";
}

template <>
double Operation<double>::EvalImpl(const ScriptingContext& context) const
{
    if (m_simple_increment)
        return EvalImpl(m_op_type, LHS()->Eval(context), RHS()->Eval(context));

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

        case OpType::REMAINDER: {
            double divisor = std::abs(RHS()->Eval(context));
            if (divisor == 0.0)
                return 0.0;
            auto dividend = LHS()->Eval(context);
            auto quotient = std::floor(dividend / divisor);
            return dividend - quotient * divisor;
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

        case OpType::NOOP: {
            DebugLogger() << "ValueRef::Operation<double>::NoOp::EvalImpl";
            auto retval = LHS()->Eval(context);
            DebugLogger() << "ValueRef::Operation<double>::NoOp::EvalImpl. Sub-Expression returned: " << retval
                          << " from: " << LHS()->Dump();
            return retval;
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
            std::vector<double> vals;
            vals.reserve(m_operands.size());
            for (auto& vr : m_operands) {
                if (vr)
                    vals.push_back(vr->Eval(context));
            }
            if (vals.empty())
                return 0.0;
            auto [min_el, max_el] = std::minmax_element(vals.begin(), vals.end());
            return m_op_type == OpType::MINIMUM ? *min_el : *max_el;
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
            std::ptrdiff_t idx = RandInt(0, m_operands.size() - 1);
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
            if (m_operands.size() == 2)
                return EvalImpl(m_op_type, lhs_val, rhs_val);

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
            static constexpr double test_case = -42.1;
            static constexpr double branchless_test = (0.0 < test_case) - (test_case < 0.0);
            static constexpr double ternary_condition_test = test_case < 0.0 ? -1.0 : test_case > 0.0 ? 1.0 : 0.0;
            static_assert(branchless_test == ternary_condition_test);
            return (0.0 < lhs) - (lhs < 0.0);
            break;
        }

        default:
            break;
    }

    throw std::runtime_error("ValueRef::Operation<double> evaluated with an unknown or invalid OpType.");
    return 0.0;
}

template <>
int Operation<int>::EvalImpl(const ScriptingContext& context) const
{
    if (m_simple_increment)
        return EvalImpl(m_op_type, LHS()->Eval(context), RHS()->Eval(context));

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

        case OpType::REMAINDER: {
            int op2 = RHS()->Eval(context);
            if (op2 == 0)
                return 0;
            return LHS()->Eval(context) % op2;
            break;
        }

        case OpType::NEGATE: {
            return -LHS()->Eval(context);
            break;
        }

        case OpType::EXPONENTIATE: {
            int op2 = RHS()->Eval(context);
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

        case OpType::NOOP: {
            DebugLogger() << "ValueRef::Operation<int>::NoOp::EvalImpl";
            auto retval = LHS()->Eval(context);
            DebugLogger() << "ValueRef::Operation<int>::NoOp::EvalImpl. Sub-Expression returned: " << retval
                          << " from: " << LHS()->Dump();
            return retval;
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
            return static_cast<int>(std::round(std::sin(op1)));
            break;
        }

        case OpType::COSINE: {
            double op1 = LHS()->Eval(context);
            return static_cast<int>(std::round(std::cos(op1)));
            break;
        }

        case OpType::MINIMUM:
        case OpType::MAXIMUM: {
            std::vector<int> vals;
            vals.reserve(m_operands.size());
            for (auto& vr : m_operands) {
                if (vr)
                    vals.push_back(vr->Eval(context));
            }
            if (vals.empty())
                return 0.0;
            auto [min_el, max_el] = std::minmax_element(vals.begin(), vals.end());
            return m_op_type == OpType::MINIMUM ? *min_el : *max_el;
            break;
        }

        case OpType::RANDOM_UNIFORM: {
            int op1 = LHS()->Eval(context);
            int op2 = RHS()->Eval(context);
            int min_val = std::min(op1, op2);
            int max_val = std::max(op1, op2);
            return RandInt(min_val, max_val);
            break;
        }

        case OpType::RANDOM_PICK: {
            // select one operand, evaluate it, return result
            if (m_operands.empty())
                return 0;
            std::ptrdiff_t idx = RandInt(0, m_operands.size() - 1);
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
            if (m_operands.size() == 2)
                return EvalImpl(m_op_type, lhs_val, rhs_val);

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
            static constexpr int test_case = -42;
            static constexpr int branchless_test = (0 < test_case) - (test_case < 0);
            static constexpr int ternary_condition_test = test_case < 0 ? -1 : test_case > 0 ? 1 : 0;
            static_assert(branchless_test == ternary_condition_test);
            return (0 < lhs) - (lhs < 0);
            break;
        }

        default:    break;
    }

    throw std::runtime_error("ValueRef::Operation<int> evaluated with an unknown or invalid OpType.");
    return 0;
}
} // namespace ValueRef
