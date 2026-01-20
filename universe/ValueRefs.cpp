#include "ValueRefs.h"

#include <algorithm>
#if __has_include(<charconv>)
  #include <charconv>
#endif
#include <chrono>
#include <functional>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include "Building.h"
#include "BuildingType.h"
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
#include "../util/ranges.h"

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

#include <utility>
#if !defined(__cpp_lib_integer_comparison_functions)
namespace std {
    constexpr auto cmp_less(auto&& lhs, auto&& rhs) { return lhs < rhs; }
}
#endif


std::string DoubleToString(double val, int digits, bool always_show_sign);
bool UserStringExists(const std::string& str);

namespace {
    void LogStackTrace(const std::string_view what) {
        // only output stack trace some times per minute, as this was very slow on windows
        static std::atomic<uint32_t> trace_count = 0;
        const auto clock_now = std::chrono::system_clock::now();
        const auto now_mins = std::chrono::duration_cast<std::chrono::minutes>(clock_now.time_since_epoch()).count();
        static auto previous_mins = std::chrono::duration_cast<std::chrono::minutes>(clock_now.time_since_epoch()).count();
        if (now_mins > previous_mins) {
            trace_count = 0;
            previous_mins = now_mins;
            return;
        }
        // only output stack trace some times per minute, as this was very slow on windows
        if (trace_count < 11) {
            trace_count++;
            ErrorLogger() << what << ": " << boost::stacktrace::stacktrace();
        }
    }

    using ReferenceType = ValueRef::ReferenceType;
    using ContainerType = ValueRef::ContainerType;
#if defined(__cpp_using_enum)
    using enum ValueRef::ReferenceType;
    using enum ValueRef::ContainerType;
#else
    static constexpr auto INVALID_REFERENCE_TYPE = ReferenceType::INVALID_REFERENCE_TYPE;
    static constexpr auto NON_OBJECT_REFERENCE = ReferenceType::NON_OBJECT_REFERENCE;
    static constexpr auto SOURCE_REFERENCE = ReferenceType::SOURCE_REFERENCE;
    static constexpr auto EFFECT_TARGET_REFERENCE = ReferenceType::EFFECT_TARGET_REFERENCE;
    static constexpr auto EFFECT_TARGET_VALUE_REFERENCE = ReferenceType::EFFECT_TARGET_VALUE_REFERENCE;
    static constexpr auto CONDITION_ROOT_CANDIDATE_REFERENCE = ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE;
    static constexpr auto CONDITION_LOCAL_CANDIDATE_REFERENCE = ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE;
    static constexpr auto PLANET = ContainerType::PLANET;
    static constexpr auto SYSTEM = ContainerType::SYSTEM;
    static constexpr auto FLEET = ContainerType::FLEET;
    static constexpr auto NONE = ContainerType::NONE;
#endif

    [[nodiscard]] constexpr std::string_view to_string(ValueRef::ReferenceType ref_type) noexcept {
        switch (ref_type) {
        case NON_OBJECT_REFERENCE:                return "<non-object>";   break;
        case SOURCE_REFERENCE:                    return "Source";         break;
        case EFFECT_TARGET_REFERENCE:             return "Target";         break;
        case EFFECT_TARGET_VALUE_REFERENCE:       return "Value(Target)";  break;
        case CONDITION_ROOT_CANDIDATE_REFERENCE:  return "RootCandidate";  break;
        case CONDITION_LOCAL_CANDIDATE_REFERENCE: return "LocalCandidate"; break;
        default:                                  return "<invalid-ref>";
        }
    }

    [[nodiscard]] constexpr std::string_view to_string(ValueRef::ContainerType container_type) noexcept {
        switch (container_type) {
        case PLANET: return "Planet"; break;
        case SYSTEM: return "System"; break;
        case FLEET:  return "Fleet";  break;
        case NONE:   return {};       break;
        }
        return {};
    }

    [[nodiscard]] const UniverseObjectCXBase* GetRefObject(ValueRef::ReferenceType ref_type,
                                                           const ScriptingContext& context) noexcept
    {
        switch (ref_type) {
        case SOURCE_REFERENCE:                   return context.source;                      break;
        case EFFECT_TARGET_REFERENCE:            return context.effect_target;               break;
        case CONDITION_ROOT_CANDIDATE_REFERENCE: return context.condition_root_candidate;    break;
        default:                                 return context.condition_local_candidate;   break;
        }
    }

    const UniverseObjectCXBase* FollowReference(ValueRef::ContainerType container_type, ValueRef::ReferenceType ref_type,
                                                const ScriptingContext& context)
    {
        if (ref_type == NON_OBJECT_REFERENCE)
            return context.condition_local_candidate;

        const auto* const obj = GetRefObject(ref_type, context);
        if (!obj) {
            static constexpr auto name_or_0 = [](const auto* obj) noexcept -> std::string_view
            { return obj ? std::string_view{obj->Name()} : std::string_view{"0"}; };

            ErrorLogger() << "FollowReference : top level object (" << to_string(ref_type)
                          << ") not defined in scripting context."
                          << "  container type: " << to_string(container_type)
                          << "  source: " << name_or_0(context.source)
                          << " target: " << name_or_0(context.effect_target)
                          << " local c: " << name_or_0(context.condition_local_candidate)
                          << " root c: " << name_or_0(context.condition_root_candidate);
            LogStackTrace(std::string{"FollowReference stacktrace : top level object ("}
                          .append(to_string(ref_type)).append(")"));

            return nullptr;
        }

        switch (container_type) {
        case PLANET: {
            if (obj->ObjectType() != UniverseObjectType::OBJ_BUILDING) [[unlikely]] {
                ErrorLogger() << "FollowReference : object not a building, so can't get its planet.";
                return nullptr;
            }
            const auto* const building = static_cast<const Building*>(obj);
            const auto* const planet = context.ContextObjects().getRaw<Planet>(building->PlanetID());
            if (!planet)
                ErrorLogger() << "FollowReference : Unable to get planet for building";
            return planet;
            break;
        }
        case SYSTEM: {
            const auto* const system = context.ContextObjects().getRaw<System>(obj->SystemID());
            if (!system)
                ErrorLogger() << "FollowReference : Unable to get system for object";
            return system;
            break;
        }
        case FLEET: {
            if (obj->ObjectType() != UniverseObjectType::OBJ_SHIP) [[unlikely]] {
                ErrorLogger() << "FollowReference : object not a ship, so can't get its fleet";
                return nullptr;
            }
            const auto* const ship = static_cast<const Ship*>(obj);
            const auto* const fleet = context.ContextObjects().getRaw<Fleet>(ship->FleetID());
            if (!fleet)
                ErrorLogger() << "FollowReference : Unable to get fleet for ship";
            return fleet;
            break;
        }
        case NONE: {
            return obj;
            break;
        }
        }
        return obj;
    }

    // Generates a debug trace that can be included in error logs, augmenting
    // the ReconstructName() info with additional info identifying the object
    // references that were successfully followed.
    std::string TraceReference(std::string_view property_name, ValueRef::ContainerType container_type,
                               ValueRef::ReferenceType ref_type, const ScriptingContext& context)
    {
        const UniverseObjectCXBase* obj = nullptr;
        const UniverseObjectCXBase* initial_obj = nullptr;
        std::string retval = ReconstructName(property_name, container_type, ref_type, false) + " : ";
        switch (ref_type) {
        case NON_OBJECT_REFERENCE:
            retval += " | Non Object Reference |";
            return retval;
            break;
        case SOURCE_REFERENCE:
            retval += " | Source: ";
            obj = context.source;
            break;
        case EFFECT_TARGET_REFERENCE:
            retval += " | Effect Target: ";
            obj = context.effect_target;
            break;
        case CONDITION_ROOT_CANDIDATE_REFERENCE:
            retval += " | Root Candidate: ";
            obj = context.condition_root_candidate;
            break;
        case CONDITION_LOCAL_CANDIDATE_REFERENCE:
        default:
            retval += " | Local Candidate: ";
            obj = context.condition_local_candidate;
            break;
        }
        if (obj) {
            retval += UserString(to_string(obj->ObjectType())) + " "
                    + std::to_string(obj->ID()) + " ( " + obj->Name() + " ) ";
            initial_obj = obj;
        } else {
            retval += "(no object)";
        }
        retval += " | ";

        switch (container_type) {
        case PLANET: {
            if (obj->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
                const auto b = static_cast<const Building*>(obj);
                obj = context.ContextObjects().getRaw<Planet>(b->PlanetID());
                retval.append("(Planet ").append(std::to_string(b->PlanetID())).append("): ");
            } else {
                obj = nullptr;
            }
            break;
        }
        case SYSTEM: {
            retval.append("(System ").append(std::to_string(obj->SystemID())).append("): ");
            obj = context.ContextObjects().getRaw<System>(obj->SystemID());
            break;
        }
        case FLEET: {
            if (obj->ObjectType() == UniverseObjectType::OBJ_SHIP) {
                const auto s = static_cast<const Ship*>(obj);
                obj = context.ContextObjects().getRaw<Fleet>(s->FleetID());
                retval.append("(Fleet ").append(std::to_string(s->FleetID())).append("): ");
            } else {
                obj = nullptr;
            }
            break;
        }
        case ::ValueRef::ContainerType::NONE:
        default:
            return retval;
            break;
        }

        if (initial_obj == obj)
            return retval; // don't need to add anything else

        // output the final reference object, if any
        if (obj)
            retval.append("  Referenced Object: ").append(UserString(to_string(obj->ObjectType())))
                  .append(" ").append(std::to_string(obj->ID())).append(" ( ").append(obj->Name()).append(" )");
        else
            retval.append("  Referenced Object: (no object)");

        return retval;
    }

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
static_assert(to_string(Property::PlanetSize) == "PlanetSize");
static_assert(StringToPropertyWithEmptyNone("GameRule") == Property::GameRule);
static_assert(StringToPropertyWithEmptyNone("Invalid!!!") == Property::Unknown);
static_assert(StringToPropertyWithEmptyNone("") == Property::None);

template <typename EnumT>
std::string EnumToString(EnumT t)
{
    static_assert(std::is_enum_v<EnumT>);
    const auto maybe_retval = to_string(t);
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

namespace {
    constexpr std::string_view PlanetTypeToStringCX(PlanetType planet) noexcept {
        // NOTE: INVALID_PLANET_TYPE (enum's -1 position) <= planet < NUM_PLANET_TYPES (enum's final position)
        return NAME_BY_PLANET[static_cast<std::size_t>(static_cast<std::underlying_type_t<PlanetType>>(planet) + 1)];
    }
}

std::string_view PlanetTypeToString(PlanetType planet) noexcept { return PlanetTypeToStringCX(planet); }

namespace {
    // @return the correct PlanetType enum for a user friendly planet type string (e.g. "Ocean"), else it returns PlanetType::INVALID_PLANET_TYPE
    constexpr PlanetType StringToPlanetTypeCX(std::string_view name) noexcept {
        for (int i = 0; std::cmp_less(i, NAME_BY_PLANET.size()); i++) {
            if (NAME_BY_PLANET[i] == name)
                return static_cast<PlanetType>(i - 1);
        }
        return PlanetType::INVALID_PLANET_TYPE;
    }
    static_assert(StringToPlanetTypeCX("not a planet") == PlanetType::INVALID_PLANET_TYPE, "Name to Planet conversion failed for invalid planet type!");
    static_assert(StringToPlanetTypeCX("Swamp") == PlanetType::PT_SWAMP, "Name to Planet conversion failed for 'Swamp' planet!");
    static_assert(StringToPlanetTypeCX("GasGiant") == PlanetType::PT_GASGIANT, "Name to Planet conversion failed for 'GasGiant' planet!");
}

PlanetType StringToPlanetType(std::string_view name) noexcept { return StringToPlanetTypeCX(name); }

namespace {
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
}

std::string_view PlanetEnvironmentToString(PlanetEnvironment env) noexcept { return PlanetEnvironmentToStringCX(env); }

std::string ReconstructName(std::string_view property_name, ContainerType container_type,
                            ReferenceType ref_type, bool return_immediate_value)
{
    std::string retval;
    retval.reserve(64);

    if (return_immediate_value)
        retval += "Value(";

    switch (ref_type) {
    case SOURCE_REFERENCE:                    retval += "Source";          break;
    case EFFECT_TARGET_REFERENCE:             retval += "Target";          break;
    case EFFECT_TARGET_VALUE_REFERENCE:       retval += "Value";           break;
    case CONDITION_LOCAL_CANDIDATE_REFERENCE: retval += "LocalCandidate";  break;
    case CONDITION_ROOT_CANDIDATE_REFERENCE:  retval += "RootCandidate";   break;
    case NON_OBJECT_REFERENCE:                retval += property_name;     break;
    default:                                  retval += "?????";           break;
    }

    if (ref_type != EFFECT_TARGET_VALUE_REFERENCE && ref_type != NON_OBJECT_REFERENCE) {
        switch (container_type) {
        case PLANET: retval += ".Planet"; break;
        case SYSTEM: retval += ".System"; break;
        case FLEET:  retval += ".Fleet";  break;
        case NONE:                        break;
        }

        if (!property_name.empty())
            retval.append(".").append(property_name);
    }

    if (return_immediate_value)
        retval += ")";

    return retval;
}

std::string FormatedDescriptionPropertyNames(ReferenceType ref_type, std::string_view property_name,
                                             ContainerType container_type, bool return_immediate_value)
{
    const uint8_t bits_count = uint8_t(container_type != ContainerType::NONE ? 1u : 0u) +
        uint8_t((ref_type != NON_OBJECT_REFERENCE && ref_type != INVALID_REFERENCE_TYPE) ? 1u : 0u);

    const auto& format_string =
        (bits_count == 0u) ? UserString("DESC_VALUE_REF_MULTIPART_VARIABLE0") :
        (bits_count == 1u) ? UserString("DESC_VALUE_REF_MULTIPART_VARIABLE1") :
                             UserString("DESC_VALUE_REF_MULTIPART_VARIABLE2");

    boost::format formatter = FlexibleFormat(format_string);

    switch (ref_type) {
    case SOURCE_REFERENCE:                    formatter % UserString("DESC_VAR_SOURCE");          break;
    case EFFECT_TARGET_REFERENCE:             formatter % UserString("DESC_VAR_TARGET");          break;
    case EFFECT_TARGET_VALUE_REFERENCE:       formatter % UserString("DESC_VAR_VALUE");           break;
    case CONDITION_LOCAL_CANDIDATE_REFERENCE: formatter % UserString("DESC_VAR_LOCAL_CANDIDATE"); break;
    case CONDITION_ROOT_CANDIDATE_REFERENCE:  formatter % UserString("DESC_VAR_ROOT_CANDIDATE");  break;
    case NON_OBJECT_REFERENCE:
    default:                                                                                      break;
    }

    switch (container_type) {
    case ContainerType::PLANET: formatter % UserString("DESC_VAR_PLANET"); break;
    case ContainerType::SYSTEM: formatter % UserString("DESC_VAR_SYSTEM"); break;
    case ContainerType::FLEET:  formatter % UserString("DESC_VAR_FLEET");  break;
    case ContainerType::NONE:                                              break;
    }

    if (!property_name.empty()) {
        std::string PROP_NAME{property_name};
        boost::to_upper(PROP_NAME);
        formatter % UserString("DESC_VAR_" + PROP_NAME);
    }

    return boost::io::str(formatter);
}

std::string ComplexVariableDescription(std::string_view property_name,
                                       const ValueRef<int>* int_ref1,
                                       const ValueRef<int>* int_ref2,
                                       const ValueRef<int>* int_ref3,
                                       const ValueRef<std::string>* string_ref1,
                                       const ValueRef<std::string>* string_ref2)
{
    if (property_name.empty()) {
        ErrorLogger() << "ComplexVariableDescription passed empty property name?!";
        return "";

    } else if (property_name == "GameRule") {
        // TODO: look up game rule description
    }

    std::string PROP{property_name};
    boost::to_upper(PROP);
    const std::string stringtable_key("DESC_VAR_" + PROP);
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

std::string ComplexVariableDump(std::string_view property_name,
                                const ValueRef<int>* int_ref1,
                                const ValueRef<int>* int_ref2,
                                const ValueRef<int>* int_ref3,
                                const ValueRef<std::string>* string_ref1,
                                const ValueRef<std::string>* string_ref2)
{
    if (property_name.empty()) {
        ErrorLogger() << "ComplexVariableDump passed empty property name?!";
        return "ComplexVariable";
    }
    std::string retval{property_name};

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

    return UserString("DESC_VAR_STATISTIC");
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
std::string Constant<PlanetSize>::Dump(uint8_t) const
{ return std::string{DumpEnum(m_value)}; }

template <>
std::string Constant<PlanetType>::Dump(uint8_t) const
{ return std::string{DumpEnum(m_value)}; }

template <>
std::string Constant<PlanetEnvironment>::Dump(uint8_t) const
{ return std::string{DumpEnum(m_value)}; }

template <>
std::string Constant<UniverseObjectType>::Dump(uint8_t) const
{ return std::string{DumpEnum(m_value)}; }

template <>
std::string Constant<StarType>::Dump(uint8_t) const
{ return std::string{DumpEnum(m_value)}; }

template <>
std::string Constant<Visibility>::Dump(uint8_t) const
{ return std::string{DumpEnum(m_value)}; }

template <>
std::string Constant<int>::Dump(uint8_t) const
{ return std::to_string(m_value); }

template <>
std::string Constant<double>::Dump(uint8_t) const
{ return std::to_string(m_value); }

namespace StaticTests {
    constexpr double test_val = 42.6;
    constexpr ::ValueRef::Constant<double> cdvr(test_val);
    static_assert(cdvr.Value() == test_val);
    static_assert(cdvr.GetReferenceType() == ::ValueRef::ReferenceType::INVALID_REFERENCE_TYPE);
    static_assert(cdvr.RootCandidateInvariant());
    static_assert(cdvr.LocalCandidateInvariant());
    static_assert(cdvr.TargetInvariant());
    static_assert(cdvr.SourceInvariant());
    static_assert(!cdvr.SimpleIncrement());
    static_assert(cdvr.ConstantExpr());
    static_assert(cdvr.GetCheckSum() == 4018064);

    constexpr auto cdvr_copy(cdvr);
    static_assert(cdvr == cdvr_copy);
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
namespace StaticTests {
    using VRVI_t = ::ValueRef::Variable<int>;
#if defined(__cpp_using_enum)
    using enum ::ValueRef::ReferenceType;
    using enum ::ValueRef::ContainerType;
#endif

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr VRVI_t visrvr(SOURCE_REFERENCE);
    static_assert(visrvr != VRVI_t(SOURCE_REFERENCE, ::ValueRef::ValueToReturn::Immediate));
    static_assert(visrvr != VRVI_t(NON_OBJECT_REFERENCE));
    static_assert(visrvr.Property() == ::ValueRef::Property::None);
    static_assert(visrvr.MeterType() == ::MeterType::INVALID_METER_TYPE);

    constexpr auto visrvr_copy(visrvr);
    static_assert(visrvr == visrvr_copy);

#  if !defined(__GNUC__) || (__GNUC__ > 13) || (__GNUC__ == 13 && __GNUC_MINOR__ >= 3)
    static_assert(VRVI_t(SOURCE_REFERENCE, "ID") != visrvr);
    static_assert(VRVI_t(SOURCE_REFERENCE, "", NONE) == visrvr);
    static_assert(VRVI_t(SOURCE_REFERENCE, "ID", SYSTEM) == VRVI_t(SOURCE_REFERENCE, "ID", SYSTEM));

    static_assert([]() {
        const VRVI_t visrprop(ReferenceType::SOURCE_REFERENCE, "ID", PLANET);
        return (visrprop.GetContainerType() == PLANET) &&
               (visrprop.Property() == Property::ID) &&
               (visrprop.MeterType() == ::MeterType::INVALID_METER_TYPE);
    }());

    static_assert([]() {
        const VRVI_t visrprop(ReferenceType::SOURCE_REFERENCE, "ID");
        const auto visrprop_copy(visrprop);
        return visrprop == visrprop_copy;
    }());

    static_assert([]() {
        const VRVI_t visrprop(ReferenceType::SOURCE_REFERENCE, "ID");
        const ::ValueRef::ValueRef<int>& vri_ref{visrprop};
        return (vri_ref == visrprop) && (visrprop == vri_ref);
    }());

    static_assert([]() {
        const VRVI_t visrprop(ReferenceType::SOURCE_REFERENCE, "Stealth");
        return (visrprop.Property() == Property::MeterType) &&
               (visrprop.MeterType() == ::MeterType::METER_STEALTH);
    }());
#  endif

    static_assert(visrvr.GetCheckSum() == 1755);
#endif
}

#define IF_CURRENT_VALUE(T)                                            \
if (m_ref_type == ReferenceType::EFFECT_TARGET_VALUE_REFERENCE) {      \
    try {                                                              \
        return std::get<T>(context.current_value);                     \
    } catch (const std::bad_variant_access&) {                         \
        LogStackTrace("Variable<" #T ">::Eval()");                     \
        throw std::runtime_error(                                      \
            "Variable<" #T ">::Eval(): Value could not be evaluated, " \
            "because the provided current value is not an " #T ".");   \
    }                                                                  \
}

#define LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(T)                                                 \
ErrorLogger() << "Variable<" #T ">::Eval unrecognized object "                                 \
                 "property: "                                                                  \
              << TraceReference(GetPropertyAsString(), m_container_type, m_ref_type, context); \
if (context.source)                                                                            \
    ErrorLogger() << "source: " << context.source->ObjectType() << " "                         \
                  << context.source->ID() << " ( "                                             \
                  << context.source->Name() << " ) ";                                          \
else                                                                                           \
    ErrorLogger() << "source (none)";

template <>
PlanetSize Variable<PlanetSize>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(PlanetSize)

    const auto* const object = FollowReference(m_container_type, m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<PlanetSize>::Eval unable to follow reference: "
                      << TraceReference(GetPropertyAsString(), m_container_type, m_ref_type, context);
        return PlanetSize::INVALID_PLANET_SIZE;
    }

    std::function<PlanetSize (const Planet&)> planet_property{nullptr};

    if (m_property == Property::PlanetSize)
        planet_property = &Planet::Size;
    else if (m_property == Property::NextLargerPlanetSize)
        planet_property = &Planet::NextLargerPlanetSize;
    else if (m_property == Property::NextSmallerPlanetSize)
        planet_property = &Planet::NextSmallerPlanetSize;

    if (planet_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET)
            return planet_property(*static_cast<const Planet*>(object));
        return PlanetSize::INVALID_PLANET_SIZE;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(PlanetSize)

    return PlanetSize::INVALID_PLANET_SIZE;
}

template <>
PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(PlanetType)

    auto object = FollowReference(m_container_type, m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<PlanetType>::Eval unable to follow reference: "
                      << TraceReference(GetPropertyAsString(), m_container_type, m_ref_type, context);
        return PlanetType::INVALID_PLANET_TYPE;
    }

    std::function<PlanetType (const Planet&)> planet_property{nullptr};

    if (m_property == Property::PlanetType)
        planet_property = &Planet::Type;
    else if (m_property == Property::OriginalType)
        planet_property = &Planet::OriginalType;
    else if (m_property == Property::NextCloserToOriginalPlanetType)
        planet_property = &Planet::NextCloserToOriginalPlanetType;
    else if (m_property == Property::NextBestPlanetType)
        planet_property = [&context](const Planet& p) { return p.NextBestPlanetTypeForSpecies(context); };
    else if (m_property == Property::NextBetterPlanetType)
        planet_property = [&context](const Planet& p) { return p.NextBetterPlanetTypeForSpecies(context); };
    else if (m_property == Property::ClockwiseNextPlanetType)
        planet_property = &Planet::ClockwiseNextPlanetType;
    else if (m_property == Property::CounterClockwiseNextPlanetType)
        planet_property = &Planet::CounterClockwiseNextPlanetType;

    if (planet_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET)
            return planet_property(*static_cast<const Planet*>(object));
        return PlanetType::INVALID_PLANET_TYPE;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(PlanetType)

    return PlanetType::INVALID_PLANET_TYPE;
}

template <>
PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(PlanetEnvironment)

    if (m_property == Property::PlanetEnvironment) {
        auto object = FollowReference(m_container_type, m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<PlanetEnvironment>::Eval unable to follow reference: "
                          << TraceReference(GetPropertyAsString(), m_container_type, m_ref_type, context);
            return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
        }
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET)
            return static_cast<const Planet*>(object)->EnvironmentForSpecies(context.species);

        return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
    }

    LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(PlanetEnvironment)

    return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
}

template <>
UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(UniverseObjectType)

    if (m_property == Property::ObjectType) {
        auto object = FollowReference(m_container_type, m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<UniverseObjectType>::Eval unable to follow reference: "
                          << TraceReference(GetPropertyAsString(), m_container_type, m_ref_type, context);
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

    auto object = FollowReference(m_container_type, m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<StarType>::Eval unable to follow reference: "
                      << TraceReference(GetPropertyAsString(), m_container_type, m_ref_type, context);
        return StarType::INVALID_STAR_TYPE;
    }

    std::function<StarType (const System&)> system_property{nullptr};

    if (m_property == Property::StarType)
        system_property = &System::GetStarType;
    else if (m_property == Property::NextOlderStarType)
        system_property = &System::NextOlderStarType;
    else if (m_property == Property::NextYoungerStarType)
        system_property = &System::NextYoungerStarType;

    if (system_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SYSTEM)
            return system_property(*static_cast<const System*>(object));
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

    ErrorLogger() << "Variable<Visibility>::Eval unrecognized object property: "
                  << TraceReference(GetPropertyAsString(), m_container_type, m_ref_type, context);

    return Visibility::INVALID_VISIBILITY;
}

template <>
double Variable<double>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(double)

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        if ((m_property == Property::UniverseCentreX) || (m_property == Property::UniverseCentreY))
            return context.ContextUniverse().UniverseWidth() / 2;
        else if (m_property == Property::UniverseWidth)
            return context.ContextUniverse().UniverseWidth();

        // add more non-object reference double functions here

        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(float)

        return 0.0;
    }

    auto object = FollowReference(m_container_type, m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(float)

        return 0.0;
    }

    if (object && m_meter_type != MeterType::INVALID_METER_TYPE) {
        if (auto* m = object->GetMeter(m_meter_type))
            return m_return_immediate_value ? m->Current() : m->Initial();
        return 0.0;

    } else if (m_property == Property::X) {
        return object->X();

    } else if (m_property == Property::Y) {
        return object->Y();

    }

    std::function<double (const Planet&)> planet_property{nullptr};

    if (m_property == Property::SizeAsDouble)
        planet_property = [](const Planet& planet) noexcept -> double { return static_cast<double>(planet.Size()); };
    else if (m_property == Property::HabitableSize)
        planet_property = &Planet::HabitableSize;
    else if (m_property == Property::DistanceFromOriginalType)
        planet_property = &Planet::DistanceFromOriginalType;

    if (planet_property) {
        if (auto planet = dynamic_cast<const Planet*>(object))
            return planet_property(*planet);
        return 0.0;

    }

    if (m_property == Property::CombatBout) {
        return context.combat_bout;

    } else if (m_property == Property::CurrentTurn) {
        return context.current_turn;

    } else if (m_property == Property::DestroyFightersPerBattleMax) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            auto retval = ship->TotalWeaponsFighterDamage(context);
            TraceLogger() << "DestroyFightersPerBattleMax" << retval;
            // TODO: prevent recursion; disallowing the ValueRef inside of destroyFightersPerBattleMax via parsers would be best.
            return retval;
        }
        return 0.0;

    } else if (m_property == Property::DamageStructurePerBattleMax) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            // TODO: prevent recursion; disallowing the ValueRef inside of damageStructurePerBattleMax via parsers would be best.
            auto retval = ship->TotalWeaponsShipDamage(context);
            TraceLogger() << "DamageStructurePerBattleMax" << retval;
            return retval;
        }
        return 0.0;

    } else if (m_property == Property::PropagatedSupplyRange) {
        const auto& ranges = context.supply.PropagatedSupplyRanges();
        auto range_it = ranges.find(object->SystemID());
        if (range_it == ranges.end())
            return 0.0;
        return range_it->second;

    } else if (m_property == Property::PropagatedSupplyDistance) {
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

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        if (m_property == Property::CombatBout)
            return context.combat_bout;
        else if (m_property == Property::CurrentTurn)
            return context.current_turn;
        else if (m_property == Property::GalaxySize)
            return context.galaxy_setup_data.GetSize();
        else if (m_property == Property::GalaxyShape)
            return static_cast<int>(context.galaxy_setup_data.GetShape());
        else if (m_property == Property::GalaxyAge)
            return static_cast<int>(context.galaxy_setup_data.GetAge());
        else if (m_property == Property::GalaxyStarlaneFrequency)
            return static_cast<int>(context.galaxy_setup_data.GetStarlaneFreq());
        else if (m_property == Property::GalaxyPlanetDensity)
            return static_cast<int>(context.galaxy_setup_data.GetPlanetDensity());
        else if (m_property == Property::GalaxySpecialFrequency)
            return static_cast<int>(context.galaxy_setup_data.GetSpecialsFreq());
        else if (m_property == Property::GalaxyMonsterFrequency)
            return static_cast<int>(context.galaxy_setup_data.GetMonsterFreq());
        else if (m_property == Property::GalaxyNativeFrequency)
            return static_cast<int>(context.galaxy_setup_data.GetNativeFreq());
        else if (m_property == Property::GalaxyMaxAIAggression)
            return static_cast<int>(context.galaxy_setup_data.GetAggression());
        else if (m_property == Property::UsedInDesignID)
            return context.in_design_id;
        else if (m_property == Property::SelectedSystemID)
            return IApp::GetApp()->SelectedSystemID();
        else if (m_property == Property::SelectedPlanetID)
            return IApp::GetApp()->SelectedPlanetID();
        else if (m_property == Property::SelectedFleetID)
            return IApp::GetApp()->SelectedFleetID();
        else if (m_property == Property::SelectedPlanetID)
            return IApp::GetApp()->SelectedPlanetID();
        else if (m_property == Property::ThisClientEmpireID)
            return IApp::GetApp()->EmpireID();

        // add more non-object reference int functions here

        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(int)

        return 0;
    }

    auto object = FollowReference(m_container_type, m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(int)

        return 0;
    }

    if (m_property == Property::Owner)
        return object->Owner();
    else if (m_property == Property::SystemID)
        return object->SystemID();
    else if (m_property == Property::ContainerID)
        return object->ContainerObjectID();
    else if (m_property == Property::SupplyingEmpire)
        return context.supply.EmpireThatCanSupplyAt(object->SystemID());
    else if (m_property == Property::ID)
        return object->ID();
    else if (m_property == Property::CreationTurn)
        return object->CreationTurn();
    else if (m_property == Property::Age)
        return object->AgeInTurns(context.current_turn);


    std::function<int (const Ship&)> ship_property{nullptr};

    if (m_property == Property::ArrivedOnTurn)
        ship_property = &Ship::ArrivedOnTurn;
    else if (m_property == Property::LastTurnActiveInBattle)
        ship_property = &Ship::LastTurnActiveInCombat;
    else if (m_property == Property::LastTurnResupplied)
        ship_property = &Ship::LastResuppliedOnTurn;
    else if (m_property == Property::OrderedColonizePlanetID)
        ship_property = &Ship::OrderedColonizePlanet;

    if (ship_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship_property(*ship);
        }
        return INVALID_GAME_TURN;
    }

    std::function<int (const Fleet&)> fleet_property{nullptr};

    if (m_property == Property::FinalDestinationID)
        fleet_property = &Fleet::FinalDestinationID;
    else if (m_property == Property::NextSystemID)
        fleet_property = &Fleet::NextSystemID;
    else if (m_property == Property::PreviousSystemID)
        fleet_property = &Fleet::PreviousSystemID;
    else if (m_property == Property::PreviousToFinalDestinationID)
        fleet_property = &Fleet::PreviousToFinalDestinationID;
    else if (m_property == Property::ArrivalStarlaneID)
        fleet_property = &Fleet::ArrivalStarlane;
    else if (m_property == Property::LastTurnMoveOrdered)
        fleet_property = &Fleet::LastTurnMoveOrdered;

    if (fleet_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return fleet_property(*fleet);
        }

	LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(int);

	return INVALID_OBJECT_ID;
    }

    std::function<int (const Planet&)> planet_property{nullptr};

    if (m_property == Property::LastTurnAttackedByShip)
        planet_property = &Planet::LastTurnAttackedByShip;
    else if (m_property == Property::LastTurnColonized)
        planet_property = &Planet::LastTurnColonized;
    else if (m_property == Property::LastTurnConquered)
        planet_property = &Planet::LastTurnConquered;
    else if (m_property == Property::LastTurnAnnexed)
        planet_property = &Planet::LastTurnAnnexed;
    else if (m_property == Property::OrderedGivenToEmpire)
        planet_property = &Planet::OrderedGivenToEmpire;
    else if (m_property == Property::OwnerBeforeLastConquered)
        planet_property = &Planet::OwnerBeforeLastConquered;
    else if (m_property == Property::LastInvadedByEmpire)
        planet_property = &Planet::LastInvadedByEmpire;
    else if (m_property == Property::LastColonizedByEmpire)
        planet_property = &Planet::LastColonizedByEmpire;

    if (planet_property) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet_property(*planet);
        }
        return INVALID_GAME_TURN;
    }

    if (m_property == Property::TurnsSinceFocusChange) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->TurnsSinceFocusChange(context.current_turn);
        }
        return 0; // here, not using planet_property, so that fallback is 0
    }
    else if (m_property == Property::TurnsSinceAnnexation) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->TurnsSinceLastAnnexed(context.current_turn);
        }
        return 0;
    }
    else if (m_property == Property::TurnsSinceColonization) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->TurnsSinceColonization(context.current_turn);
        }
        return 0;
    }
    else if (m_property == Property::TurnsSinceLastConquered) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->TurnsSinceLastConquered(context.current_turn);
        }
        return 0;
    }
    else if (m_property == Property::ProducedByEmpireID) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship->ProducedByEmpireID();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
            auto building = static_cast<const Building*>(object);
            return building->ProducedByEmpireID();
        }
        return ALL_EMPIRES;

    }
    else if (m_property == Property::DesignID) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship->DesignID();
        }
        return INVALID_DESIGN_ID;

    }
    else if (m_property == Property::FleetID) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            return ship->FleetID();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return fleet->ID();
        }
        return INVALID_OBJECT_ID;

    }
    else if (m_property == Property::PlanetID) {
        if (object->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
            auto building = static_cast<const Building*>(object);
            return building->PlanetID();

        } else if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->ID();
        }
        return INVALID_OBJECT_ID;

    }
    else if (m_property == Property::NearestSystemID) {
        if (object->SystemID() != INVALID_OBJECT_ID)
            return object->SystemID();
        return context.ContextUniverse().GetPathfinder().NearestSystemTo(
            object->X(), object->Y(), context.ContextObjects());

    }
    else if (m_property == Property::NumShips) {
        if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return static_cast<int>(fleet->NumShips());
        }
        return 0;

    }
    else if (m_property == Property::NumStarlanes) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
            auto system = static_cast<const System*>(object);
            return system->NumStarlanes();
        }
        return 0;

    }
    else if (m_property == Property::LastTurnBattleHere) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
            auto system = static_cast<const System*>(object);
            return system->LastTurnBattleHere();

        } else if (auto system = context.ContextObjects().getRaw<System>(object->SystemID())) {
            return system->LastTurnBattleHere();
        }
        return INVALID_GAME_TURN;

    }
    else if (m_property == Property::Orbit) {
        if (auto system = context.ContextObjects().getRaw<System>(object->SystemID()))
            return system->OrbitOfPlanet(object->ID());
        return -1;

    }
    else if (m_property == Property::ETA) {
        if (object->ObjectType() == UniverseObjectType::OBJ_FLEET) {
            auto fleet = static_cast<const Fleet*>(object);
            return fleet->ETA(context).first;
        }
        return 0;

    }
    else if (m_property == Property::NumSpecials) {
        return static_cast<int>(object->Specials().size());

    }
    else if (m_property == Property::LaunchedFrom) {
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
std::vector<std::string> Variable<std::vector<std::string>>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(std::vector<std::string>)

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        // add more non-object reference string vector functions here
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::vector<std::string>)

        return {};
    }

    auto object = FollowReference(m_container_type, m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::vector<std::string>)

        return {};
    }

    if (m_property == Property::Tags) {
        std::vector<std::string> retval;
        auto tags = object->Tags(context);
        retval.reserve(tags.size());
        std::transform(tags.first.begin(), tags.first.end(), std::back_inserter(retval), [](auto sv) { return std::string{sv}; });
        std::transform(tags.second.begin(), tags.second.end(), std::back_inserter(retval), [](auto sv) { return std::string{sv}; });
        return retval;
    }
    else if (m_property == Property::Specials) {
        const auto obj_special_names_range = object->Specials() | range_keys;
        return {obj_special_names_range.begin(), obj_special_names_range.end()};
    }
    else if (m_property == Property::AvailableFoci) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            auto foci_views = planet->AvailableFoci(context);
            return {foci_views.begin(), foci_views.end()};
        }
        return {};
    }
    else if (m_property == Property::Parts) {
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

    if (m_ref_type == ReferenceType::NON_OBJECT_REFERENCE) {
        if (m_property == Property::GalaxySeed)
            return context.galaxy_setup_data.GetSeed();

        // add more non-object reference string functions here
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::string)

        return "";
    }

    auto object = FollowReference(m_container_type, m_ref_type, context);
    if (!object) {
        LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE(std::string)

        return "";
    }

    if (m_property == Property::Name) {
        return object->Name();

    } else if (m_property == Property::OwnerName) {
        int owner_empire_id = object->Owner();
        if (auto empire = context.GetEmpire(owner_empire_id))
            return empire->Name();
        return "";

    } else if (m_property == Property::TypeName) {
        return std::string{to_string(object->ObjectType())};

    }

    std::function<std::string (const Empire&)> empire_property{nullptr};

    if (m_property == Property::OwnerLeastExpensiveEnqueuedTech)
        empire_property = [&context](const auto& empire) { return empire.LeastExpensiveEnqueuedTech(context); };
    else if (m_property == Property::OwnerMostExpensiveEnqueuedTech)
        empire_property = [&context](const auto& empire) { return empire.MostExpensiveEnqueuedTech(context); };
    else if (m_property == Property::OwnerMostRPCostLeftEnqueuedTech)
        empire_property = [&context](const auto& empire) { return empire.MostRPCostLeftEnqueuedTech(context); };
    else if (m_property == Property::OwnerMostRPSpentEnqueuedTech)
        empire_property = &Empire::MostRPSpentEnqueuedTech;
    else if (m_property == Property::OwnerTopPriorityEnqueuedTech)
        empire_property = &Empire::TopPriorityEnqueuedTech;

    if (empire_property) {
        auto empire = context.GetEmpire(object->Owner());
        if (!empire)
            return "";
        return empire_property(*empire);
    }

    if (m_property == Property::Species) {
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

    } else if (m_property == Property::Hull) {
        if (object->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<const Ship*>(object);
            if (const ShipDesign* design = context.ContextUniverse().GetShipDesign(ship->DesignID()))
                return design->Hull();
        }
        return "";

    } else if (m_property == Property::FieldType) {
        if (object->ObjectType() == UniverseObjectType::OBJ_FIELD) {
            auto field = static_cast<const Field*>(object);
            return field->FieldTypeName();
        }
        return "";

    } else if (m_property == Property::BuildingType) {
        if (object->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
            auto building = static_cast<const Building*>(object);
            return building->BuildingTypeName();
        }
        return "";

    } else if (m_property == Property::Focus) {
        if (object->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto planet = static_cast<const Planet*>(object);
            return planet->Focus();
        }
        return "";

    } else if (m_property == Property::DefaultFocus) {
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
    if (this->m_stat_type == StatisticType::IF)
        return scond->EvalAny(context) ? " " : "";

    // the only other statistic that can be computed on non-number property
    // types and that is itself of a non-number type is the most common value
    if (this->m_stat_type != StatisticType::MODE) {
        ErrorLogger() << "Statistic<std::string, std::string>::Eval has invalid statistic type: "
                      << this->m_stat_type;
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

    static constexpr auto second_less = [](const auto& p1, const auto& p2) -> bool
    { return p1.second < p2.second; };
    auto max = range_max_element(observed_values, second_less);

    return max->first;
}

namespace StaticTests {
    static_assert(::ValueRef::decays_to<const int&&, int>);
    static_assert(std::is_same_v<int, ::ValueRef::decayed_value_t<std::array<int, 1>>>);

    constexpr auto test_arr = std::array{1, 1, 2, 5, 4, 2, 1, 1, 1, 2, 2, 4, 4};

    static_assert(ReduceData<int>(StatisticType::IF, test_arr));
    static_assert(ReduceData<int>(StatisticType::IF, test_arr) == 1);
    static_assert(ReduceData<int>(StatisticType::IF, std::array{::PlanetType::PT_OCEAN}) == 1);
    static_assert(ReduceData<int>(StatisticType::COUNT, std::array<const int, 3>{1, 2, 3}) == 3);
#if defined (__cpp_lib_constexpr_algorithms)
    static_assert(UniqueCount<uint8_t>(test_arr) == 4);
    static_assert(UniqueCount<float>(std::array{::PlanetType::PT_OCEAN, ::PlanetType::PT_SWAMP}) == 2.0f);
    static_assert(ReduceData<int>(StatisticType::UNIQUE_COUNT, test_arr) == 4);
#endif
#if defined(__cpp_lib_constexpr_vector)
    static_assert(HistoMinMax<int, MinMax::Min>(test_arr) == 1);
    static_assert(HistoMinMax<int, MinMax::Max>(test_arr) == 5);
    static_assert(HistoMinMax<int, MinMax::Mode>(test_arr) == 1);
    static_assert(HistoMinMax<int, MinMax::Spread>(test_arr) == 4);
    static_assert(ReduceData<uint16_t>(StatisticType::HISTO_MIN, test_arr) == 1);
    static_assert(ReduceData<uint16_t>(StatisticType::HISTO_MAX, test_arr) == 5);
    static_assert(ReduceData<uint16_t>(StatisticType::MODE, test_arr) == 1);
    static_assert(ReduceData<::PlanetType>(StatisticType::MODE, std::array{::PlanetType::PT_OCEAN}) == ::PlanetType::PT_OCEAN);
    static_assert(ReduceData<uint16_t>(StatisticType::HISTO_SPREAD, test_arr) == 4);
#  if defined (__cpp_lib_constexpr_algorithms)
    static_assert(ReduceData<uint16_t>(StatisticType::SUM, test_arr) == 30);
    static_assert(ReduceData<uint16_t>(StatisticType::MEAN, test_arr) == 2); // after rounding
#  endif
#endif
    static_assert(ReduceData<uint16_t>(StatisticType::MAX, test_arr) == 5);
    static_assert(ReduceData<uint16_t>(StatisticType::MIN, test_arr) == 1);
    static_assert(ReduceData<uint16_t>(StatisticType::SPREAD, test_arr) == 4);
    static_assert(ReduceData<uint16_t>(StatisticType::PRODUCT, test_arr) == 5120);
    static_assert(RMS<uint8_t>(std::array<const int, 3>{1, 1, 1}) == 1);
    static_assert(RMS<uint8_t>(std::array<const int, 3>{14, -2, 10}) == 10);
    static_assert(ReduceData<uint16_t>(StatisticType::RMS, test_arr) == 2);
    static_assert(STD<int8_t>(std::array{1, 2, 3}) == 1);
    constexpr auto testf_stdev = STD<float>(std::array{3, 3, 3, 4, 4, 5, 5, 7, 11, 15});
    static_assert(testf_stdev == 4.0f);
    static_assert(STD<uint16_t>(std::array{2.0f, 3.0f, 5.0f, 7.0f, 7.0f, 7.0f, 11.0f}) == 3);
    constexpr auto testd_stdev = ReduceData<double>(StatisticType::STDEV, test_arr);
    static_assert(testd_stdev > 1.43 && testd_stdev < 1.44);

#if defined(__cpp_lib_constexpr_vector)
    static_assert(ReduceData<int>(StatisticType::UNIQUE_COUNT, std::vector{1, 2, 2, 2}) == 2);
#endif
}

///////////////////////////////////////////////////////////
// TotalFighterShots (of a carrier during one battle)    //
///////////////////////////////////////////////////////////
bool TotalFighterShots::operator==(const ValueRef<int>& rhs) const {
    if (std::addressof(rhs) == this)
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

std::string TotalFighterShots::Dump(uint8_t) const {
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
PlanetSize ComplexVariable<PlanetSize>::Eval(const ScriptingContext&) const
{ return PlanetSize::INVALID_PLANET_SIZE; }

template <>
PlanetType ComplexVariable<PlanetType>::Eval(const ScriptingContext&) const
{ return PlanetType::INVALID_PLANET_TYPE; } // TODO: Species favourite planet type?

template <>
PlanetEnvironment ComplexVariable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
{
    if (m_property == Property::PlanetEnvironmentForSpecies) {
        const int planet_id = m_int_ref1 ? m_int_ref1->Eval(context) : INVALID_OBJECT_ID;
        const auto planet = context.ContextObjects().get<Planet>(planet_id);
        if (!planet)
            return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;

        const std::string species_name = m_string_ref1 ? m_string_ref1->Eval(context) : "";
        return planet->EnvironmentForSpecies(context.species, species_name);
    }

    return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
}

template <>
UniverseObjectType ComplexVariable<UniverseObjectType>::Eval(const ScriptingContext&) const
{ return UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE; }

template <>
StarType ComplexVariable<StarType>::Eval(const ScriptingContext&) const
{ return StarType::INVALID_STAR_TYPE; }

template <>
Visibility ComplexVariable<Visibility>::Eval(const ScriptingContext& context) const
{
    if (m_property == Property::EmpireObjectVisibility) {
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
    using boost::container::flat_map;
    // empire properties indexed by strings
    std::function<const std::map<std::string, int>& (const Empire&)> empire_property_string_key {nullptr};
    std::function<const flat_map<std::string, int, std::less<>>& (const Empire&)> empire_property_string_key3{nullptr};

    if (m_property == Property::TurnTechResearched)
        empire_property_string_key3 = &Empire::ResearchedTechs;

    else if (m_property == Property::BuildingTypesOwned)
        empire_property_string_key = &Empire::BuildingTypesOwned;
    else if (m_property == Property::BuildingTypesProduced)
        empire_property_string_key = &Empire::BuildingTypesProduced;
    else if (m_property == Property::BuildingTypesScrapped)
        empire_property_string_key = &Empire::BuildingTypesScrapped;
    else if (m_property == Property::SpeciesColoniesOwned)
        empire_property_string_key = &Empire::SpeciesColoniesOwned;
    else if (m_property == Property::SpeciesPlanetsBombed)
        empire_property_string_key = &Empire::SpeciesPlanetsBombed;
    else if (m_property == Property::SpeciesPlanetsDepoped)
        empire_property_string_key = &Empire::SpeciesPlanetsDepoped;
    else if (m_property == Property::SpeciesPlanetsInvaded)
        empire_property_string_key = &Empire::SpeciesPlanetsInvaded;
    else if (m_property == Property::SpeciesShipsDestroyed)
        empire_property_string_key = &Empire::SpeciesShipsDestroyed;
    else if (m_property == Property::SpeciesShipsLost)
        empire_property_string_key = &Empire::SpeciesShipsLost;
    else if (m_property == Property::SpeciesShipsOwned)
        empire_property_string_key = &Empire::SpeciesShipsOwned;
    else if (m_property == Property::SpeciesShipsProduced)
        empire_property_string_key = &Empire::SpeciesShipsProduced;
    else if (m_property == Property::SpeciesShipsScrapped)
        empire_property_string_key = &Empire::SpeciesShipsScrapped;
    else if (m_property == Property::ShipPartsOwned)
        empire_property_string_key = &Empire::ShipPartsOwned;
    else if (m_property == Property::TurnsSincePolicyAdopted)
        empire_property_string_key = &Empire::PolicyCurrentAdoptedDurations;
    else if (m_property == Property::CumulativeTurnsPolicyAdopted)
        empire_property_string_key = &Empire::PolicyTotalAdoptedDurations;
    else if (m_property == Property::LatestTurnPolicyAdopted)
        empire_property_string_key = &Empire::PolicyLatestTurnsAdopted;

    if (empire_property_string_key || empire_property_string_key3) {
        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
        }

        std::function<bool(const std::map<std::string, int>::value_type&)> key_filter{nullptr};
        key_filter = [](auto) -> bool { return true; };

        if (m_string_ref1) {
            std::string key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;

            if (empire && m_property == Property::TurnTechResearched && !empire->TechResearched(key_string))
                // special case for techs: make unresearched-tech's research-turn a big number
                return IMPOSSIBLY_LARGE_TURN;

            key_filter = [k{std::move(key_string)}](auto e) -> bool { return k == e.first; };
        }
        else if (m_property == Property::ShipPartsOwned && m_int_ref2) {
            int key_int = m_int_ref2->Eval(context);
            if (key_int <= int(ShipPartClass::INVALID_SHIP_PART_CLASS) ||
                key_int >= int(ShipPartClass::NUM_SHIP_PART_CLASSES))
            { return 0; }

            using map_val_t = const std::map<ShipPartClass, int>::value_type;
            auto key_filter_class = [part_class = ShipPartClass(key_int)](map_val_t& e) { return e.first == part_class; };

            if (empire) {
                auto filtered_values = empire->ShipPartClassOwned() | range_filter(key_filter_class) | range_values;
                return std::accumulate(filtered_values.begin(), filtered_values.end(), 0);
            }

            int sum = 0;
            for (const auto& loop_empire : context.Empires() | range_values) {
                auto filtered_values = loop_empire->ShipPartClassOwned() | range_filter(key_filter_class) | range_values;
                sum += std::accumulate(filtered_values.begin(), filtered_values.end(), 0);
            }
            return sum;
        }

        if (empire) {
            if (empire_property_string_key) {
                auto filtered_values = empire_property_string_key(*empire) | range_filter(key_filter) | range_values;
                return std::accumulate(filtered_values.begin(), filtered_values.end(), 0);

            } else if (empire_property_string_key3) {
                auto filtered_values = empire_property_string_key3(*empire) | range_filter(key_filter) | range_values;
                return std::accumulate(filtered_values.begin(), filtered_values.end(), 0);
            }
        }

        int sum = 0;
        for (const auto& loop_empire : context.Empires() | range_values) {
            if (empire_property_string_key) {
                auto filtered_values = empire_property_string_key(*loop_empire) | range_filter(key_filter) | range_values;
                sum += std::accumulate(filtered_values.begin(), filtered_values.end(), 0);

            } else if (empire_property_string_key3) {
                auto filtered_values = empire_property_string_key3(*loop_empire) | range_filter(key_filter) | range_values;
                sum += std::accumulate(filtered_values.begin(), filtered_values.end(), 0);
            }
        }
        return sum;
    }


    // empire properties indexed by integers
    std::function<const std::map<int, int>& (const Empire&)> empire_property_int_key{nullptr};

    if (m_property == Property::EmpireShipsDestroyed)
        empire_property_int_key = &Empire::EmpireShipsDestroyed;
    else if (m_property == Property::ShipDesignsDestroyed)
        empire_property_int_key = &Empire::ShipDesignsDestroyed;
    else if (m_property == Property::ShipDesignsLost)
        empire_property_int_key = &Empire::ShipDesignsLost;
    else if (m_property == Property::ShipDesignsOwned)
        empire_property_int_key = &Empire::ShipDesignsOwned;
    else if (m_property == Property::ShipDesignsInProduction)
        empire_property_int_key = &Empire::ShipDesignsInProduction;
    else if (m_property == Property::ShipDesignsProduced)
        empire_property_int_key = &Empire::ShipDesignsProduced;
    else if (m_property == Property::ShipDesignsScrapped)
        empire_property_int_key = &Empire::ShipDesignsScrapped;
    else if (m_property == Property::TurnSystemExplored)
        empire_property_int_key = &Empire::TurnsSystemsExplored;

    if (empire_property_int_key) {
        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
        }

        std::function<bool(const std::map<int, int>::value_type&)> key_filter{nullptr};
        key_filter = [](auto) -> bool { return true; };

        // if a key integer specified, get just that entry (for single empire or sum of all empires)
        if (m_int_ref2)
            key_filter = [k = m_int_ref2->Eval(context)](auto e) -> bool { return k == e.first; };

        // although indexed by integers, some of these may be specified by a
        // string that needs to be looked up. if a key string specified, get
        // just that entry (for single empire or sum of all empires)
        if (m_string_ref1) {
            const std::string key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;
            int key_int = -1; // default

            static constexpr std::array ship_design_properties{
                Property::ShipDesignsDestroyed, Property::ShipDesignsLost, Property::ShipDesignsOwned,
                Property::ShipDesignsInProduction, Property::ShipDesignsProduced, Property::ShipDesignsScrapped};

            if (range_contains(ship_design_properties, m_property)) {
                // look up ship design id corresponding to specified predefined ship design name
                if (const ShipDesign* design = context.ContextUniverse().GetGenericShipDesign(key_string))
                    key_int = design->ID();
            }
            key_filter = [k = key_int](auto e) -> bool { return k == e.first; };
        }

        if (empire) {
            auto filtered_values = empire_property_int_key(*empire) | range_filter(key_filter) | range_values;
            return std::accumulate(filtered_values.begin(), filtered_values.end(), 0);
        }

        int sum = 0;
        for (const auto& loop_empire : context.Empires() | range_values) {
            auto filtered_values = empire_property_int_key(*loop_empire) | range_filter(key_filter) | range_values;
            sum += std::accumulate(filtered_values.begin(), filtered_values.end(), 0);
        }
        return sum;
    }


    // unindexed empire proprties
    if (m_property == Property::OutpostsOwned) {
        std::shared_ptr<const Empire> empire;
        if (m_int_ref1) {
            int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
            empire = context.GetEmpire(empire_id);
            if (!empire)
                return 0;
        }

        std::function<int(const Empire*)> empire_property = &Empire::OutpostsOwned;


        if (!empire) {
            static constexpr auto GetRawPtr = [](const auto& smart_ptr) { return smart_ptr.get(); };

            // sum property over all empires
            auto empire_properties = context.Empires() | range_values |
                range_transform(GetRawPtr) | range_transform(empire_property);
            return std::accumulate(empire_properties.begin(), empire_properties.end(), 0);
        }

        return empire_property(empire.get());
    }


    // non-empire properties
    if (m_property == Property::GameRule) {
        if (!m_string_ref1)
            return 0;
        const std::string rule_name = m_string_ref1->Eval();
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
        }
        catch (...) {
        }
        return 0;
    }
    else if (m_property == Property::PartsInShipDesign) {
        int design_id = INVALID_DESIGN_ID;
        if (m_int_ref1) {
            design_id = m_int_ref1->Eval(context);
            if (design_id == INVALID_DESIGN_ID)
                return 0;
        }
        else {
            return 0;
        }

        const std::string ship_part_name = m_string_ref1 ? m_string_ref1->Eval(context) : std::string{};

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
        if (!design)
            return 0;

        if (ship_part_name.empty())
            return design->PartCount();
        const auto is_part = [&ship_part_name](const auto& part) { return part == ship_part_name; };

        const auto& parts = design->Parts();
        return static_cast<int>(range_count_if(parts, is_part));
    }
    else if (m_property == Property::PartOfClassInShipDesign) {
        int design_id = INVALID_DESIGN_ID;
        if (m_int_ref1) {
            design_id = m_int_ref1->Eval(context);
            if (design_id == INVALID_DESIGN_ID)
                return 0;
        }
        else {
            return 0;
        }

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
        if (!design)
            return 0;

        if (!m_string_ref1)
            return 0;

        const auto part_class_name = m_string_ref1->Eval(context);
        const ShipPartClass part_class = ShipPartClassFromString(part_class_name);

        const auto part_of_class = [part_class](const auto& name) {
            if (name.empty()) return false;
            const auto* part = GetShipPart(name);
            return part && part->Class() == part_class;
        };
        return static_cast<int>(range_count_if(design->Parts(), part_of_class));
    }
    else if (m_property == Property::NumPartClassesInShipDesign) {
        if (!m_int_ref1)
            return 0;

        const int design_id = m_int_ref1->Eval(context);
        if (design_id == INVALID_DESIGN_ID)
            return 0;

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
        if (!design)
            return 0;

        return design->PartClassCount().size();
    }
    else if (m_property == Property::JumpsBetween) {
        int object1_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object1_id = m_int_ref1->Eval(context);

        int object2_id = INVALID_OBJECT_ID;
        if (m_int_ref2)
            object2_id = m_int_ref2->Eval(context);

        const int retval = context.ContextUniverse().GetPathfinder().JumpDistanceBetweenObjects(
            object1_id, object2_id, context.ContextObjects());
        if (retval == INT_MAX)
            return -1;
        return retval;
    }
    else if (m_property == Property::JumpsBetweenByEmpireSupplyConnections) {
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

        const int retval = context.ContextUniverse().GetPathfinder().JumpDistanceBetweenObjects(
            object1_id, object2_id, context.ContextObjects());
        if (retval == INT_MAX)
            return -1;
        return retval;
    }
    else if (m_property == Property::SlotsInHull) {
        const ShipHull* ship_hull = nullptr;
        if (m_string_ref1) {
            std::string hull_name = m_string_ref1->Eval(context);
            ship_hull = GetShipHull(hull_name);
            if (!ship_hull)
                return 0;
        }
        else {
            return 0;
        }
        return static_cast<int>(ship_hull->Slots().size());
    }
    else if (m_property == Property::SlotsInShipDesign) {
        int design_id = INVALID_DESIGN_ID;
        if (m_int_ref1) {
            design_id = m_int_ref1->Eval(context);
            if (design_id == INVALID_DESIGN_ID)
                return 0;
        }
        else {
            return 0;
        }

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
        if (!design)
            return 0;

        const ShipHull* ship_hull = GetShipHull(design->Hull());
        if (!ship_hull)
            return 0;
        return static_cast<int>(ship_hull->Slots().size());
    }
    else if (m_property == Property::SpecialAddedOnTurn) {
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
    else if (m_property == Property::TurnPolicyAdopted) { // returns by value, so can't assign &Empire::TurnsPoliciesAdopted to empire_property_string_key above
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
    else if (m_property == Property::NumPoliciesAdopted) { // similar to a string-keyed empire property, but does specialized lookups of adopted policy info
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

        }
        else {
            for (auto& [cat_name, policy_names_turns] : empire->CategoriesSlotsPoliciesAdopted()) {
                if (cat_name == policy_category_name) {
                    const auto not_empty = [](auto& pnt) { return !pnt.second.empty(); };
                    count += std::count_if(policy_names_turns.begin(), policy_names_turns.end(), not_empty);
                }
            }
            return count;
        }
    }
    else if (m_property == Property::PlanetTypeDifference) {
        // get planet types to find difference between...
        PlanetType pt1;
        if (m_int_ref1) {
            int pt_int1 = m_int_ref1->Eval(context);
            pt1 = PlanetType(pt_int1);
        }
        else if (m_string_ref1) {
            const std::string pt_name1 = m_string_ref1->Eval(context);
            pt1 = StringToPlanetTypeCX(pt_name1);
        }
        else {
            return 0;
        }

        PlanetType pt2;
        if (m_int_ref2) {
            int pt_int2 = m_int_ref2->Eval(context);
            pt2 = PlanetType(pt_int2);
        }
        else if (m_string_ref2) {
            const std::string pt_name2 = m_string_ref2->Eval(context);
            pt2 = StringToPlanetTypeCX(pt_name2);
        }
        else {
            return 0;
        }

        return Planet::TypeDifference(pt1, pt2);
    }
    else if (m_property == Property::EmpireObjectVisibilityTurn) {
        if (!m_int_ref1 || !m_int_ref2)
            return 0;
        const auto& vis_turn_map{ context.empire_object_vis_turns };

        const int empire_id = m_int_ref1->Eval(context);
        const auto empire_it = vis_turn_map.find(empire_id);
        if (empire_it == vis_turn_map.end())
            return 0;
        const auto& object_vis_turns_map = empire_it->second;

        const int object_id = m_int_ref2->Eval(context);
        const auto object_it = object_vis_turns_map.find(object_id);
        if (object_it == object_vis_turns_map.end())
            return 0;

        const auto& vis_turns = object_it->second;
        const auto vis_it = vis_turns.find(Visibility::VIS_BASIC_VISIBILITY);
        if (vis_it == vis_turns.end())
            return 0;
        return vis_it->second;
    }

    return 0;
}

template <>
double ComplexVariable<double>::Eval(const ScriptingContext& context) const
{
    std::function<float (const ShipHull&)> hull_property{nullptr};

    if (m_property == Property::HullFuel)
        hull_property = &ShipHull::Fuel;
    else if (m_property == Property::HullStealth)
        hull_property = &ShipHull::Stealth;
    else if (m_property == Property::HullStructure)
        hull_property = &ShipHull::Structure;
    else if (m_property == Property::HullSpeed)
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

    if (m_property == Property::SystemSupplyRange) {
        if (!m_int_ref2)
            return 0.0; // no system specified... doesn't make sense to sum over systems...
        int system_id = m_int_ref2->Eval(context);
        if (system_id == INVALID_OBJECT_ID)
            return 0.0;

        if (m_int_ref1) {
            // single empire ID specified
            const int empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0.0;
            const auto empire = context.GetEmpire(empire_id);
            if (!empire)
                return 0.0;
            const auto& data = empire->SystemSupplyRanges();

            const auto it = data.find(system_id);
            return (it != data.end()) ? it->second : 0.0f;

        } else {
            const auto& empires{context.Empires()};
            if (empires.NumEmpires() < 1)
                return 0.0f;
            // no empire ID specified, use max of all empires' ranges at specified system
            const auto to_sys_supply_range = [system_id](const auto& e) {
                const auto& ssr = e.second->SystemSupplyRanges();
                const auto it = ssr.find(system_id);
                return (it != ssr.end()) ? it->second : 0.0f;
            };
            auto sup_rng_rng = empires | range_transform(to_sys_supply_range);
            return *range_max_element(sup_rng_rng);
        }
    }


    if (m_property == Property::EmpireStockpile) {
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
            res_type = MeterToResource(NameToMeter(res_name));
        }
        if (res_type == ResourceType::INVALID_RESOURCE_TYPE)
            return 0;

        std::function<int (const Empire*)> empire_property{nullptr};

        empire_property = [res_type](const Empire* empire) { return empire->ResourceStockpile(res_type); };

        static constexpr auto to_raw_ptr = [](const auto& smart_ptr){ return smart_ptr.get(); };

        if (!empire) {
            auto empire_props = context.Empires() | range_values | range_transform(to_raw_ptr)
                | range_transform(empire_property);
            return std::accumulate(empire_props.begin(), empire_props.end(), 0);
        }

        return empire_property(empire.get());
    }


    // non-empire properties
    std::function<const std::map<int, float>& ()> property_int_key{nullptr};

    if (m_property == Property::PropagatedSystemSupplyRange) // int_ref2 is system ID
        property_int_key = [&context]() -> const auto& { return context.supply.PropagatedSupplyRanges(); };
    else if (m_property == Property::PropagatedSystemSupplyDistance) // int_ref2 is system ID
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


    if (m_property == Property::GameRule) {
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
    else if (m_property == Property::PartCapacity) {
        std::string ship_part_name;
        if (m_string_ref1)
            ship_part_name = m_string_ref1->Eval(context);

        const ShipPart* ship_part = GetShipPart(ship_part_name);
        if (!ship_part)
            return 0.0;

        return ship_part->Capacity();

    }
    else if (m_property == Property::PartSecondaryStat) {
        std::string ship_part_name;
        if (m_string_ref1)
            ship_part_name = m_string_ref1->Eval(context);

        const ShipPart* ship_part = GetShipPart(ship_part_name);
        if (!ship_part)
            return 0.0;

        return ship_part->SecondaryStat();

    }
    else if (m_property == Property::ShipDesignCost) {
        const int design_id = m_int_ref1 ? m_int_ref1->Eval(context) : INVALID_DESIGN_ID;
        const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
        if (!design)
            return 0.0;

        const int empire_id = m_int_ref2 ? m_int_ref2->Eval(context) : ALL_EMPIRES;
        const int location_id = m_int_ref3 ? m_int_ref3->Eval(context) : INVALID_OBJECT_ID;

        return design->ProductionCost(empire_id, location_id, context); // overrides source and local candidate to specify empire and location

    }
    else if (m_property == Property::BuildingTypeCost) {
        const std::string building_type_name = m_string_ref1 ? m_string_ref1->Eval(context) : "";
        const auto* building_type = GetBuildingType(building_type_name);
        if (!building_type)
            return 0.0;

        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        const int location_id = m_int_ref2 ? m_int_ref2->Eval(context) : INVALID_OBJECT_ID;

        return building_type->ProductionCost(empire_id, location_id, context); // overrides source and local candidate to specify empire and location

    }
    else if (m_property == Property::EmpireMeterValue) {
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
    else if (m_property == Property::EmpireAnnexationCost) { // intended for use in UI, not in scripted content, due to ambiguity about what the "source" object would be, since this sets the source to specify the empire
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        const auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return std::numeric_limits<double>::quiet_NaN();
        const auto source_for_empire = empire->Source(context.ContextObjects()); // not (necessarily) the source in context, but used to specify empire

        const int location_id = m_int_ref2 ? m_int_ref2->Eval(context) : INVALID_OBJECT_ID;
        const auto* location = context.ContextObjects().getRaw(location_id); // could be nullptr

        if (!location || location->ObjectType() != UniverseObjectType::OBJ_PLANET)
            return std::numeric_limits<double>::quiet_NaN();
        const auto* planet = static_cast<const Planet*>(location);
        if (planet->SpeciesName().empty())
            return std::numeric_limits<double>::quiet_NaN();
        return planet->AnnexationCost(empire_id, context);

    }
    else if (m_property == Property::DirectDistanceBetween) {
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
    else if (m_property == Property::ShortestPath || m_property == Property::ShortestPathDistance) {
        int object1_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object1_id = m_int_ref1->Eval(context);

        int object2_id = INVALID_OBJECT_ID;
        if (m_int_ref2)
            object2_id = m_int_ref2->Eval(context);

        return context.ContextUniverse().GetPathfinder().ShortestPathDistance(
            object1_id, object2_id, context.ContextObjects());

    }
    else if (m_property == Property::SpeciesContentOpinion) {
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

        if (range_contains(species->Likes(), liked_or_disliked_content_name))
            return 1.0;
        else if (range_contains(species->Dislikes(), liked_or_disliked_content_name))
            return -1.0;
        else
            return 0.0;

    }
    else if (m_property == Property::SpeciesEmpireOpinion || m_property == Property::SpeciesEmpireTargetOpinion) {
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        const std::string species_name = m_string_ref1 ? m_string_ref1->Eval(context) : "";
        const bool target = m_property == Property::SpeciesEmpireTargetOpinion;
        static constexpr bool current_value = false;

        return context.species.SpeciesEmpireOpinion(species_name, empire_id, target, current_value);

    }
    else if (m_property == Property::SpeciesSpeciesOpinion || m_property == Property::SpeciesSpeciesTargetOpinion) {
        const std::string opinionated_species_name = m_string_ref1 ? m_string_ref1->Eval(context) : "";
        const std::string rated_species_name = m_string_ref2 ? m_string_ref2->Eval(context) : "";
        const bool target = m_property == Property::SpeciesSpeciesTargetOpinion;
        static constexpr bool current_value = false;

        return context.species.SpeciesSpeciesOpinion(opinionated_species_name, rated_species_name, target, current_value);

    }
    else if (m_property == Property::SpecialCapacity) {
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
    else if (m_property == Property::ShipPartMeter) {
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

        const auto meter_type = NameToMeter(meter_name);
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
        auto researched_techs_range = empire->ResearchedTechs() | range_keys;
        return {researched_techs_range.begin(), researched_techs_range.end()};
    }

    [[nodiscard]] std::vector<std::string> TechsResearchableByEmpire(int empire_id, const ScriptingContext& context) {
        auto empire = context.GetEmpire(empire_id);
        if (!empire) return {};
        const auto res_tech = [&empire](const auto& name_tech) { return empire->ResearchableTech(name_tech.first); };
        auto res_techs = GetTechManager() | range_filter(res_tech) | range_keys;
        return {res_techs.begin(), res_techs.end()};
    }

    [[nodiscard]] auto TransferrableTechs(int sender_empire_id, int receipient_empire_id,
                                          const ScriptingContext& context)
    {
        auto sender_researched_techs = TechsResearchedByEmpire(sender_empire_id, context);
        auto recepient_researchable = TechsResearchableByEmpire(receipient_empire_id, context);

        std::vector<std::string> retval;

        if (sender_researched_techs.empty() || recepient_researchable.empty())
            return retval;
        retval.reserve(std::max(sender_researched_techs.size(), recepient_researchable.size()));

        // find intersection of two lists
        std::stable_sort(sender_researched_techs.begin(), sender_researched_techs.end());
        std::stable_sort(recepient_researchable.begin(), recepient_researchable.end());
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
    std::function<std::string (const Empire&)> empire_property{nullptr};
    static constexpr auto null_property =
        [](const Empire&) noexcept(noexcept(std::string{})) { return std::string{}; };

    // unindexed empire properties
    if (m_property == Property::LowestCostEnqueuedTech)
        empire_property = [&context](const Empire& e) { return e.LeastExpensiveEnqueuedTech(context); };
    else if (m_property == Property::HighestCostEnqueuedTech)
        empire_property = [&context](const Empire& e) { return e.MostExpensiveEnqueuedTech(context); };
    else if (m_property == Property::TopPriorityEnqueuedTech)
        empire_property = &Empire::TopPriorityEnqueuedTech;
    else if (m_property == Property::MostSpentEnqueuedTech)
        empire_property = &Empire::MostRPSpentEnqueuedTech;
    else if (m_property == Property::LowestCostResearchableTech)
        empire_property = [&context](const auto& empire) { return empire.LeastExpensiveResearchableTech(context); };
    else if (m_property == Property::HighestCostResearchableTech)
        empire_property = &Empire::MostExpensiveResearchableTech;
    else if (m_property == Property::TopPriorityResearchableTech)
        empire_property = &Empire::TopPriorityResearchableTech;
    else if (m_property == Property::MostSpentResearchableTech)
        empire_property = &Empire::MostExpensiveResearchableTech;
    else if (m_property == Property::MostSpentTransferrableTech)
        empire_property = null_property;
    else if (m_property == Property::RandomTransferrableTech)
        empire_property = null_property;
    else if (m_property == Property::MostPopulousSpecies)
        empire_property = null_property;
    else if (m_property == Property::MostHappySpecies)
        empire_property = null_property;
    else if (m_property == Property::LeastHappySpecies)
        empire_property = null_property;
    else if (m_property == Property::RandomColonizableSpecies)
        empire_property = null_property;
    else if (m_property == Property::RandomControlledSpecies)
        empire_property = null_property;

    if (empire_property) {
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire_id == ALL_EMPIRES)
            return "";
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";

        return empire_property(*empire);
    }

    if (m_property == Property::RandomEnqueuedTech) {
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire_id == ALL_EMPIRES)
            return "";
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";

        // get all techs on queue, randomly pick one
        auto all_enqueued_techs = empire->GetResearchQueue().AllEnqueuedProjects();
        if (all_enqueued_techs.empty())
            return "";
        using diff_t = decltype(all_enqueued_techs.begin())::difference_type;
        const diff_t idx = static_cast<diff_t>(RandInt(0, static_cast<int>(all_enqueued_techs.size()) - 1));
        return std::move(*std::next(all_enqueued_techs.begin(), idx));

    } else if (m_property == Property::RandomResearchableTech) {
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire_id == ALL_EMPIRES)
            return "";
        const auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";

        auto researchable_techs = TechsResearchableByEmpire(empire_id, context);
        if (researchable_techs.empty())
            return "";
        using diff_t = decltype(researchable_techs.begin())::difference_type;
        const diff_t idx = static_cast<diff_t>(RandInt(0, static_cast<int>(researchable_techs.size()) - 1));
        return std::move(*std::next(researchable_techs.begin(), idx));

    } else if (m_property == Property::RandomCompleteTech) {
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire_id == ALL_EMPIRES)
            return "";
        const auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return "";

        auto complete_techs = TechsResearchedByEmpire(empire_id, context);
        if (complete_techs.empty())
            return "";
        using diff_t = decltype(complete_techs.begin())::difference_type;
        const diff_t idx = static_cast<diff_t>(RandInt(0, static_cast<int>(complete_techs.size()) - 1));
        return std::move(*std::next(complete_techs.begin(), idx));

    } else if (m_property == Property::LowestCostTransferrableTech) {
        const int empire1_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire1_id == ALL_EMPIRES)
            return "";
        const int empire2_id = m_int_ref2 ? m_int_ref2->Eval(context) : ALL_EMPIRES;
        if (empire2_id == ALL_EMPIRES)
            return "";

        auto sendable_techs = TransferrableTechs(empire1_id, empire2_id, context);
        if (sendable_techs.empty())
            return "";
        using diff_t = decltype(sendable_techs.begin())::difference_type;
        const diff_t idx = static_cast<std::size_t>(RandInt(0, static_cast<int>(sendable_techs.size()) - 1));
        return std::move(*std::next(sendable_techs.begin(), idx));

    } else if (m_property == Property::HighestCostTransferrableTech) {
        const int empire1_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire1_id == ALL_EMPIRES)
            return "";
        const int empire2_id = m_int_ref2 ? m_int_ref2->Eval(context) : ALL_EMPIRES;
        if (empire2_id == ALL_EMPIRES)
            return "";

        auto sendable_techs = TransferrableTechs(empire1_id, empire2_id, context);
        if (sendable_techs.empty())
            return "";

        const auto name_to_name_tech_and_cost =
            [empire2_id, &context](std::string& tech_name) -> std::pair<std::string&, float> {
                const auto* tech = GetTech(tech_name);
                static constexpr auto flt_low = std::numeric_limits<float>::lowest();
                auto cost = tech ? tech->ResearchCost(empire2_id, context) : -flt_low;
                return {tech_name, cost};
            };
        auto sendable_techs_costs_rng = sendable_techs | range_transform(name_to_name_tech_and_cost);

        using qq = decltype(*range_max_element(sendable_techs_costs_rng, second_less));
        static_assert(std::is_same_v<qq, std::pair<std::string&, float>>);

        return std::move((*range_max_element(sendable_techs_costs_rng, second_less)).first);

    } else if (m_property == Property::TopPriorityTransferrableTech) {
        const int empire1_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire1_id == ALL_EMPIRES)
            return "";
        const int empire2_id = m_int_ref2 ? m_int_ref2->Eval(context) : ALL_EMPIRES;
        if (empire2_id == ALL_EMPIRES)
            return "";

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
        for (std::string& tech : sendable_techs) {
            auto queue_it = queue.find(tech);
            if (queue_it == queue.end())
                continue;
            int queue_pos = std::distance(queue.begin(), queue_it);
            if (queue_pos < position_of_top_found_tech) {
                retval = std::move(tech);
                position_of_top_found_tech = queue_pos;
            }
        }
        return retval;
    }

    // non-empire properties
    if (m_property == Property::GameRule) {
        if (!m_string_ref1)
            return "";
        const auto rule_name = m_string_ref1->Eval();
        if (rule_name.empty() || !GetGameRules().RuleExists(rule_name))
            return "";

        try {
            // can cast boolean, int, double, or string-valued rules to strings
            switch (GetGameRules().GetType(rule_name)) {
            case GameRule::Type::TOGGLE:    return std::to_string(GetGameRules().Get<bool>(rule_name));             break;
            case GameRule::Type::INT:       return std::to_string(GetGameRules().Get<int>(rule_name));              break;
            case GameRule::Type::DOUBLE:    return DoubleToString(GetGameRules().Get<double>(rule_name), 3, false); break;
            case GameRule::Type::STRING:    return GetGameRules().Get<std::string>(rule_name);                      break;
            default: break;
            }
        } catch (...) {}
        return "";
    }

    return "";
}

template <>
std::vector<std::string> ComplexVariable<std::vector<std::string>>::Eval(
    const ScriptingContext& context) const
{
    // unindexed empire properties
    if (m_property == Property::EmpireAdoptedPolices) {
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire_id == ALL_EMPIRES)
            return {};
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return {};

        auto pols = empire->AdoptedPolicies();
        std::vector<std::string> retval;
        retval.reserve(pols.size());
        std::transform(pols.begin(), pols.end(), std::back_inserter(retval),
                       [](const std::string_view sv) -> std::string { return std::string{sv}; });
        return retval;

    } else if (m_property == Property::EmpireAvailablePolices) {
        const int empire_id = m_int_ref1 ? m_int_ref1->Eval(context) : ALL_EMPIRES;
        if (empire_id == ALL_EMPIRES)
            return {};
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
    std::string retval{this->GetPropertyAsString()};

    if (m_property == Property::EmpireObjectVisibility) {
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
    std::string retval{this->GetPropertyAsString()};

    // empire properties indexed by integers
    if (m_property == Property::PropagatedSystemSupplyRange ||
        m_property == Property::SystemSupplyRange ||
        m_property == Property::PropagatedSystemSupplyDistance)
    {
        if (m_int_ref1)
            retval += " empire = " + m_int_ref1->Dump(ntabs);
        if (m_int_ref2)
            retval += " system = " + m_int_ref2->Dump(ntabs);

    }
    else if (m_property == Property::GameRule ||
             m_property == Property::HullFuel ||
             m_property == Property::HullStealth ||
             m_property == Property::HullStructure ||
             m_property == Property::HullSpeed ||
             m_property == Property::PartCapacity ||
             m_property == Property::PartSecondaryStat)
    {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);

    }
    else if (m_property == Property::EmpireMeterValue) {
        if (m_int_ref1)
            retval += " empire = " + m_int_ref1->Dump(ntabs);
        if (m_string_ref1)
            retval += " meter = " + m_string_ref1->Dump(ntabs);

    }
    else if (m_property == Property::ShipPartMeter) {
        // ShipPartMeter part = "SR_WEAPON_1_1" meter = Capacity object = Source.ID
        if (m_string_ref1)
            retval += " part = " + m_string_ref1->Dump(ntabs);
        if (m_string_ref2)
            retval += " meter = " + m_string_ref2->Dump(ntabs); // wrapped in quotes " but shouldn't be to be consistent with parser
        if (m_int_ref1)
            retval += " object = " + m_int_ref1->Dump(ntabs);

    }
    else if (m_property == Property::DirectDistanceBetween ||
             m_property == Property::ShortestPath ||
             m_property == Property::ShortestPathDistance)
    {
        if (m_int_ref1)
            retval += " object = " + m_int_ref1->Dump(ntabs);
        if (m_int_ref2)
            retval += " object = " + m_int_ref2->Dump(ntabs);

    }
    else if (m_property == Property::SpeciesContentOpinion) {
        if (m_string_ref1)
            retval += " species = " + m_string_ref1->Dump(ntabs);
        if (m_string_ref2)
            retval += " name = " + m_string_ref2->Dump(ntabs);

    }
    else if (m_property == Property::SpeciesEmpireOpinion) {
        if (m_int_ref1)
            retval += " empire = " + m_int_ref1->Dump(ntabs);
        if (m_string_ref1)
            retval += " species = " + m_string_ref1->Dump(ntabs);

    }
    else if (m_property == Property::SpeciesSpeciesOpinion) {
        if (m_string_ref1)
            retval += " species = " + m_string_ref1->Dump(ntabs);
        if (m_string_ref2)
            retval += " species = " + m_string_ref2->Dump(ntabs);

    }
    else if (m_property == Property::SpecialCapacity) {
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
    std::string retval{this->GetPropertyAsString()};
    // todo: implement like <double> case
    if (m_property == Property::GameRule) {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);
    }

    return retval;
}

template <>
std::string ComplexVariable<std::string>::Dump(uint8_t ntabs) const
{
    std::string retval{this->GetPropertyAsString()};
    // TODO: implement like <double> case
    if (m_property == Property::GameRule) {
        if (m_string_ref1)
            retval += " name = " + m_string_ref1->Dump(ntabs);
    }

    return retval;
}

template <>
std::string ComplexVariable<std::vector<std::string>>::Dump(uint8_t ntabs) const
{
    std::string retval{this->GetPropertyAsString()};
    // todo: implement like <double> case
    if (m_property == Property::GameRule) {
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

    const auto raw_ref = m_value_ref.get();
    if (!raw_ref)
        return "";

    double result = raw_ref->Eval(context);
    auto Stringify = [](double num) -> std::string {
        if (std::isnan(num))
            return "";
        else if (std::isinf(num))
            return num > 0 ? "∞" : "-∞";
        else if (!std::isfinite(num))
            return "";

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


    const auto* const ref = dynamic_cast<const Variable<double>*>(raw_ref);
    if (!ref)
        return Stringify(result);

    // special case for position / distance related sub-value-refs, to improve with UI representation
    const auto property = ref->Property();
    if (property == Property::X || property == Property::Y ||
        property == Property::DirectDistanceBetween ||
        property == Property::ShortestPathDistance)
    {
        if (result == UniverseObject::INVALID_POSITION)
            return UserString("INVALID_POSITION");

        std::stringstream ss;
        ss << std::setprecision(5) << result;
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
    const auto property = int_ref->Property();
    //if (property == Propery::None || propery == Property::Unknown)
    //    return std::to_string(result);

    if (property == Property::ETA) {
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
bool NameLookup::operator==(const ValueRef<std::string>& rhs) const {
    if (std::addressof(rhs) == this)
        return true;
    if (!this->ValueRefBase::operator==(rhs) || typeid(*this) != typeid(rhs))
        return false;

    const NameLookup& rhs_ = static_cast<const NameLookup&>(rhs);

    if (m_lookup_type != rhs_.m_lookup_type)
        return false;
    if (m_value_ref == rhs_.m_value_ref)
        return true;
    if (!m_value_ref || !rhs_.m_value_ref)
        return false;
    return *m_value_ref == *(rhs_.m_value_ref);
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

namespace StaticTests {
    using namespace ::ValueRef;

    // auto abs(auto val)
    static_assert(abs(0) == 0);
    static_assert(abs(1) == 1);
    static_assert(abs(-4.0) == 4.0);

    // auto sign(auto val)
    static_assert(sign(0) == 0);
    static_assert(sign(1) == 1);
    static_assert(sign(-4.0) == -1.0);

    // auto pow(auto val)
    static_assert(pow(0) == 1);

    // auto pow(auto base, auto exp)
    static_assert(pow(0, 1) == 0);
    static_assert(pow(1, 1) == 1);
    static_assert(pow(-2, 3) == -8);
    static_assert(pow(1.0, 2.0) == 1.0);
    static_assert(pow(2.0, 2.0) == 4.0);

    // auto round(auto val)
    static_assert(round(0.0) == 0.0);
    static_assert(round(42) == 42);
    static_assert(round(42.0) == 42);
    static_assert(round(42.9) == 43);
    static_assert(round(42.1) == 42);
    static_assert(round(-42.0) == -42);
    static_assert(round(-42.9) == -43);
    static_assert(round(-42.1) == -42);

    // auto ceil(auto val)
    static_assert(ceil(0.0) == 0.0);
    static_assert(ceil(42) == 42);
    static_assert(ceil(42.0) == 42);
    static_assert(ceil(42.9) == 43);
    static_assert(ceil(42.1) == 43);
    static_assert(ceil(-42.0) == -42);
    static_assert(ceil(-42.9) == -42);
    static_assert(ceil(-42.1) == -42);

    // auto floor(auto val)
    static_assert(floor(0.0) == 0.0);
    static_assert(floor(42) == 42);
    static_assert(floor(42.0) == 42);
    static_assert(floor(42.9) == 42);
    static_assert(floor(42.1) == 42);
    static_assert(floor(-42.0) == -42);
    static_assert(floor(-42.9) == -43);
    static_assert(floor(-42.1) == -43);

    static constexpr double ok_test_frac = 0.00000000001;

    // auto log(auto val)
    static_assert(log(1) == 0);
    static_assert(log(1.0) == 0.0);
    static_assert(abs(log(2.0) - 0.6931471805599453) < ok_test_frac);
    static_assert(abs(log(0.5) + 0.6931471805599453) < ok_test_frac);
    static_assert(abs(log(2.0) + log(0.5)) < ok_test_frac);

    // auto log(auto val, auto base)
    static_assert(log(2, 1) == 0);
    static_assert(abs(log(2.0, 0.5) + 1.0) < ok_test_frac);
    static_assert(log(2.0, 1.0) == 0.0);
    static_assert(log(2.0, 2.0) == 1.0);
    static_assert(log(2, 4) == 2);
    static_assert(abs(log(2.0, 4.0) - 2.0) < ok_test_frac);
    static_assert(log(10, 1000) == 3);
    static_assert(abs(log(10.0, 1000.0) - 3.0) < ok_test_frac);
    static_assert(abs(log(10.0f, 1000.0f) - 3.0f) < ok_test_frac);

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 13) || (__GNUC__ == 13 && __GNUC_MINOR__ >= 3))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
    static_assert(OperateData(OpType::PLUS, std::string{"start"}, std::string{"end"}) == "startend");
    static_assert(OperateData(OpType::SUBSTITUTION, std::string{""}, std::string{"anything"}).empty());
    static_assert(OperateData(OpType::NOOP, std::string{"noop"}) == "noop");
    static_assert(OperateData(OpType::COMPARE_EQUAL, std::string{"one"}, std::string{"two"}).empty());
    static_assert(!OperateData(OpType::COMPARE_NOT_EQUAL, std::string{"one"}, std::string{"two"}).empty());
    static_assert(OperateData(OpType::RANDOM_PICK, std::string{"one"}, std::string{"two"}, [](auto, auto) { return 0; }) == "one");
    static_assert(OperateData(OpType::RANDOM_PICK, std::string{"one"}, std::string{"two"}, [](auto, auto) { return 1; }) == "two");
    static_assert(OperateData(OpType::RANDOM_PICK, std::string{"one"}, std::string{"two"}, [](auto, auto) { return 4; }) == "two");
    static_assert(OperateData(OpType::RANDOM_PICK, std::string{"one"}, std::string{"two"}, [](auto, auto) { return -4; }) == "two");
#endif
#if defined(__cpp_lib_constexpr_vector)
    static_assert(OperateData(OpType::PLUS, std::vector{-1, 0, 1, 2, 3}) == 5);
    static_assert(OperateData(OpType::TIMES, std::vector{-1, 1, 2, 3}) == -6);
    static_assert(OperateData(OpType::RANDOM_PICK, std::vector{-1, 1, 2, 3}, [](auto, auto) { return 0; }) == -1);
    static_assert(OperateData(OpType::RANDOM_PICK, std::vector{-1, 1, 2, 3}, [](auto, auto) { return 100; }) == 3);
    static_assert(OperateData(OpType::RANDOM_PICK, std::vector{-1, 1, 2, 3}, [](auto, auto) { return -3; }) == 3);
#endif

    static_assert(OperateData(OpType::PLUS, 0, 42) == 42);
    static_assert(OperateData(OpType::PLUS, 0.25f, 1.25f) == 1.5f);
    static_assert(OperateData(OpType::MINUS, 0, 42) == -42);
    static_assert(OperateData(OpType::MINUS, 0.25f, 1.25f) == -1.0f);
    static_assert(OperateData(OpType::TIMES, 0, 1) == 0);
    static_assert(OperateData(OpType::TIMES, 2, 21) == 42);
    static_assert(OperateData(OpType::TIMES, -2.0, 21.0) == -42.0);
    static_assert(OperateData(OpType::DIVIDE, 5, 9) == 0);
    static_assert(OperateData(OpType::DIVIDE, 84, 2) == 42);
    static_assert(OperateData(OpType::DIVIDE, -84.0f, 2.0f) == -42.0f);
    static_assert(OperateData(OpType::REMAINDER, 5, 9) == 5);
    static_assert(OperateData(OpType::REMAINDER, -5, 9) == -5);
    static_assert(OperateData(OpType::REMAINDER, 5, -9) == 5);
    static_assert(OperateData(OpType::REMAINDER, -5, -9) == -5);
    static_assert(OperateData(OpType::EXPONENTIATE, 0, 1) == 0);
    static_assert(OperateData(OpType::EXPONENTIATE, 5, 0) == 1);
    static_assert(OperateData(OpType::EXPONENTIATE, 2.0f, 4.0f) == 16.0f);
    static_assert(OperateData(OpType::LOGARITHM, 2, 256) == 8);
    static_assert(OperateData(OpType::MINIMUM, -5, -9) == -9);
    static_assert(OperateData(OpType::MAXIMUM, -5, -9) == -5);

    constexpr struct {
        constexpr int operator()(int l, int h) noexcept { return RandIntCx(std::min(l, h), std::max(l, h)); }
        constexpr double operator()(double l, double h) noexcept { return RandDoubleCx(std::min(l, h), std::max(l, h)); }
    } rand_wrapper;
    static_assert(OperateData(OpType::RANDOM_UNIFORM, 5, -5, rand_wrapper) >= -5);
    static_assert(OperateData(OpType::RANDOM_UNIFORM, 5.0, 9.0, rand_wrapper, rand_wrapper) <= 9.0);
    static_assert(OperateData(OpType::RANDOM_UNIFORM, 5.0, 9.0, rand_wrapper, rand_wrapper) >= 5.0);
    static_assert(OperateData(OpType::RANDOM_PICK, 3, 4, [](auto, auto) { return 1; }) == 4);
    static_assert(OperateData(OpType::RANDOM_PICK, 3, 4, [](auto, auto) { return 0; }) == 3);
    static_assert(OperateData(OpType::RANDOM_PICK, 3, 4, [](auto, auto) { return 10; }) == 4);
    static_assert(OperateData(OpType::RANDOM_PICK, 3, 4, [](auto, auto) { return -4; }) == 4);

    static_assert(OperateData(OpType::COMPARE_EQUAL, 2, 4) == 0);
    static_assert(OperateData(OpType::COMPARE_GREATER_THAN, 2, 4) == 0);
    static_assert(OperateData(OpType::COMPARE_GREATER_THAN_OR_EQUAL, 2, 4) == 0);
    static_assert(OperateData(OpType::COMPARE_LESS_THAN, 2, 4) == 1);
    static_assert(OperateData(OpType::COMPARE_LESS_THAN_OR_EQUAL, 2, 4) == 1);
    static_assert(OperateData(OpType::COMPARE_NOT_EQUAL, 2, 4) == 1);

    static_assert(OperateData(OpType::NOOP, 5) == 5);
    static_assert(OperateData(OpType::NEGATE, 5) == -5);
    static_assert(OperateData(OpType::NEGATE, 42.0) == -42.0);
    static_assert(OperateData(OpType::EXPONENTIATE, 0.0) == 1.0);
    static_assert(OperateData(OpType::ABS, 0) == 0);
    static_assert(OperateData(OpType::ABS, 1) == 1);
    static_assert(OperateData(OpType::ABS, -3.25) == 3.25);
    static_assert(OperateData(OpType::LOGARITHM, 1.0) == 0.0);
    static_assert(OperateData(OpType::ROUND_NEAREST, 1.0) == 1.0);
    static_assert(OperateData(OpType::ROUND_NEAREST, 42.6f) == 43.0f);
    static_assert(OperateData(OpType::ROUND_NEAREST, -1.3) == -1);
    static_assert(OperateData(OpType::ROUND_UP, 1.0) == 1.0);
    static_assert(OperateData(OpType::ROUND_UP, 42.6f) == 43.0f);
    static_assert(OperateData(OpType::ROUND_UP, -1.3) == -1);
    static_assert(OperateData(OpType::ROUND_DOWN, 1.0) == 1.0);
    static_assert(OperateData(OpType::ROUND_DOWN, 42.6f) == 42.0f);
    static_assert(OperateData(OpType::ROUND_DOWN, -1.3) == -2);
    static_assert(OperateData(OpType::SIGN, -1.3) == -1);
    static_assert(OperateData(OpType::SIGN, 0) == 0);
    static_assert(OperateData(OpType::SIGN, 42.6f) == 1.0f);

#if defined(__cpp_lib_constexpr_vector)
    static_assert(OperateConstantValueRefs(OpType::PLUS, std::array<::ValueRef::Constant<int>*, 4>{}) == 0);
#  if !defined(_MSC_VER) || (_MSC_VER >= 1939)
    constexpr ::ValueRef::Constant<int> cxvc1{1}, cxvc2{2}, cxvc3{3};
    constexpr std::array<const ::ValueRef::Constant<int>*, 4> test_refs{&cxvc1, &cxvc2, &cxvc3, nullptr};
    static_assert(OperateConstantValueRefs(OpType::PLUS, test_refs) == 6);
    static_assert(OperateConstantValueRefs(OpType::TIMES, test_refs) == 0);
    static_assert(OperateConstantValueRefs(OpType::MINIMUM, test_refs) == 0);
    static_assert(OperateConstantValueRefs(OpType::MAXIMUM, test_refs) == 3);
    static_assert(OperateConstantValueRefs(OpType::COMPARE_EQUAL, test_refs) == 0);
    static_assert(OperateConstantValueRefs(OpType::COMPARE_GREATER_THAN, test_refs) == 0);
    static_assert(OperateConstantValueRefs(OpType::COMPARE_GREATER_THAN_OR_EQUAL, test_refs) == 0);
    static_assert(OperateConstantValueRefs(OpType::COMPARE_LESS_THAN, test_refs) == 3);
    static_assert(OperateConstantValueRefs(OpType::COMPARE_LESS_THAN_OR_EQUAL, test_refs) == 3);
    static_assert(OperateConstantValueRefs(OpType::COMPARE_NOT_EQUAL, test_refs) == 3);
#  endif
#endif

    constexpr auto test_checksum_nullptr = CheckSums::GetCheckSum(nullptr);
    static_assert(test_checksum_nullptr == 0);

    constexpr auto test_checksum_sv_short = CheckSums::GetCheckSum("short text");
    constexpr auto test_checksum_sv_long = CheckSums::GetCheckSum("longer text that should not be within string sso");
    static_assert(test_checksum_sv_short != test_checksum_sv_long);

    constexpr auto test_checksum_variadic3 = CheckSums::GetCheckSum("longer text that should not be within string sso", 5, nullptr);
    static_assert(test_checksum_variadic3 == 4696);

    constexpr ::ValueRef::Constant<int> const_ref_8{8};
    static_assert(const_ref_8.Eval() == 8);
    static_assert(const_ref_8.GetCheckSum() == CheckSums::GetCheckSum("ValueRef::Constant", 8));

    constexpr auto test_checksum_variadic4 = CheckSums::GetCheckSum("text", false, &const_ref_8, nullptr);
    static_assert(test_checksum_variadic4 == 2235);

    constexpr auto test_checksum_like_constant_string = CheckSums::GetCheckSum("ValueRef::Constant<string>", "RULE_ANNEX_COST_MINIMUM");
    static_assert(test_checksum_like_constant_string == 4414);

    constexpr auto test_checksum_like_complex1 = CheckSums::GetCheckSum("ValueRef::ComplexVariable", "GameRule", false,
                                                                        nullptr, nullptr, nullptr, 
                                                                        test_checksum_like_constant_string, nullptr);
    constexpr auto test_checksum_like_complex2 = CheckSums::GetCheckSum("ValueRef::ComplexVariable", "GameRule",
                                                                        "ValueRef::Constant<string>", "RULE_ANNEX_COST_MINIMUM");
    static_assert(test_checksum_like_complex1 == 7677);
    static_assert(test_checksum_like_complex2 == 7677);


#if defined(__cpp_lib_constexpr_string) && (!defined(__GNUC__) || (__GNUC__ > 13) || (__GNUC__ == 13 && __GNUC_MINOR__ >= 3)) && (!defined(_MSC_VER) || (_MSC_VER >= 1934)) && (!defined(__clang_major__) || (__clang_major__ >= 17))
    constexpr auto test_checksum_combo_with_string = []() {
        const Constant<std::string> const_string_ref_annex_min{"RULE_ANNEX_COST_MINIMUM"};
        return const_string_ref_annex_min.GetCheckSum();
    }();
    static_assert(test_checksum_combo_with_string == test_checksum_like_constant_string);

    constexpr auto test_checksum_combo_with_string_ref = []() {
        const Constant<std::string> const_string_ref_annex_min{"RULE_ANNEX_COST_MINIMUM"};
        return CheckSums::GetCheckSum("ValueRef::ComplexVariable", "GameRule", false,
                                      nullptr, nullptr, nullptr, &const_string_ref_annex_min, nullptr);
    }();
    static_assert(test_checksum_combo_with_string_ref == 7677);
#endif

}

} // namespace ValueRef
