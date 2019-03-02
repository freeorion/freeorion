#include "ValueRef.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "Planet.h"
#include "Predicates.h"
#include "Species.h"
#include "System.h"
#include "Field.h"
#include "Fighter.h"
#include "Pathfinder.h"
#include "Universe.h"
#include "UniverseObject.h"
#include "Condition.h"
#include "Enums.h"
#include "Tech.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../Empire/Supply.h"
#include "../util/Random.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/GameRules.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

#include <iomanip>
#include <iterator>


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
        //DebugLogger() << "FollowReference: source: " << (context.source ? context.source->Name() : "0")
        //              << " target: " << (context.effect_target ? context.effect_target->Name() : "0")
        //              << " local c: " << (context.condition_local_candidate ? context.condition_local_candidate->Name() : "0")
        //              << " root c: " << (context.condition_root_candidate ? context.condition_root_candidate->Name() : "0");

        std::shared_ptr<const UniverseObject> obj;
        switch (ref_type) {
        case ValueRef::NON_OBJECT_REFERENCE:                    return context.condition_local_candidate;   break;
        case ValueRef::SOURCE_REFERENCE:                        obj = context.source;                       break;
        case ValueRef::EFFECT_TARGET_REFERENCE:                 obj = context.effect_target;                break;
        case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:      obj = context.condition_root_candidate;     break;
        case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE:
        default:                                                obj = context.condition_local_candidate;    break;
        }

        if (!obj) {
            std::string type_string;
            switch (ref_type) {
            case ValueRef::SOURCE_REFERENCE:                        type_string = "Source";         break;
            case ValueRef::EFFECT_TARGET_REFERENCE:                 type_string = "Target";         break;
            case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:      type_string = "RootCandidate";  break;
            case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE:
            default:                                                type_string = "LocalCandidate"; break;
            }
            ErrorLogger() << "FollowReference : top level object (" << type_string << ") not defined in scripting context";
            return nullptr;
        }

        while (first != last) {
            std::string property_name = *first;
            if (property_name == "Planet") {
                if (auto b = std::dynamic_pointer_cast<const Building>(obj)) {
                    obj = GetPlanet(b->PlanetID());
                } else {
                    ErrorLogger() << "FollowReference : object not a building, so can't get its planet.";
                    obj = nullptr;
                }
            } else if (property_name == "System") {
                if (obj)
                    obj = GetSystem(obj->SystemID());
                if (!obj)
                    ErrorLogger() << "FollowReference : Unable to get system for object";
            } else if (property_name == "Fleet") {
                if (auto s = std::dynamic_pointer_cast<const Ship>(obj)) {
                    obj = GetFleet(s->FleetID());
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
        case ValueRef::NON_OBJECT_REFERENCE:
            retval += " | Non Object Reference |";
            return retval;
            break;
        case ValueRef::SOURCE_REFERENCE:
            retval += " | Source: ";
            obj = context.source;
            break;
        case ValueRef::EFFECT_TARGET_REFERENCE:
            retval += " | Effect Target: ";
            obj = context.effect_target;
            break;
        case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:
            retval += " | Root Candidate: ";
            obj = context.condition_root_candidate;
            break;
        case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE:
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
                    obj = GetPlanet(b->PlanetID());
                } else
                    obj = nullptr;
            } else if (property_name_part == "System") {
                if (obj) {
                    retval += "(" + std::to_string(obj->SystemID()) + "): ";
                    obj = GetSystem(obj->SystemID());
                }
            } else if (property_name_part == "Fleet") {
                if (auto s = std::dynamic_pointer_cast<const Ship>(obj))  {
                    retval += "(" + std::to_string(s->FleetID()) + "): ";
                    obj = GetFleet(s->FleetID());
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
        ObjectTypeVisitor() : m_type(INVALID_UNIVERSE_OBJECT_TYPE) {}

        std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Building> obj) const override
        { m_type = OBJ_BUILDING; return obj; }

        std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Fleet> obj) const override
        { m_type = OBJ_FLEET; return obj; }

        std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Planet> obj) const override
        { m_type = OBJ_PLANET; return obj; }

        std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Ship> obj) const override
        { m_type = OBJ_SHIP; return obj; }

        std::shared_ptr<UniverseObject> Visit(std::shared_ptr<System> obj) const override
        { m_type = OBJ_SYSTEM; return obj; }

        std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Field> obj) const override
        { m_type = OBJ_FIELD; return obj; }

        mutable UniverseObjectType m_type;
    };

    const std::map<std::string, MeterType>& GetMeterNameMap() {
        static std::map<std::string, MeterType> meter_name_map;
        if (meter_name_map.empty()) {
            // todo: maybe need some thread guards here?
            meter_name_map["Population"] =         METER_POPULATION;
            meter_name_map["TargetPopulation"] =   METER_TARGET_POPULATION;
            meter_name_map["Industry"] =           METER_INDUSTRY;
            meter_name_map["TargetIndustry"] =     METER_TARGET_INDUSTRY;
            meter_name_map["Research"] =           METER_RESEARCH;
            meter_name_map["TargetResearch"] =     METER_TARGET_RESEARCH;
            meter_name_map["Trade"] =              METER_TRADE;
            meter_name_map["TargetTrade"] =        METER_TARGET_TRADE;
            meter_name_map["Construction"] =       METER_CONSTRUCTION;
            meter_name_map["TargetConstruction"] = METER_TARGET_CONSTRUCTION;
            meter_name_map["Happiness"] =          METER_HAPPINESS;
            meter_name_map["TargetHappiness"] =    METER_TARGET_HAPPINESS;
            meter_name_map["MaxFuel"] =            METER_MAX_FUEL;
            meter_name_map["Fuel"] =               METER_FUEL;
            meter_name_map["MaxStructure"] =       METER_MAX_STRUCTURE;
            meter_name_map["Structure"] =          METER_STRUCTURE;
            meter_name_map["MaxShield"] =          METER_MAX_SHIELD;
            meter_name_map["Shield"] =             METER_SHIELD;
            meter_name_map["MaxDefense"] =         METER_MAX_DEFENSE;
            meter_name_map["Defense"] =            METER_DEFENSE;
            meter_name_map["MaxTroops"] =          METER_MAX_TROOPS;
            meter_name_map["Troops"] =             METER_TROOPS;
            meter_name_map["RebelTroops"] =        METER_REBEL_TROOPS;
            meter_name_map["Supply"] =             METER_SUPPLY;
            meter_name_map["MaxSupply"] =          METER_MAX_SUPPLY;
            meter_name_map["Stockpile"] =          METER_STOCKPILE;
            meter_name_map["MaxStockpile"] =       METER_MAX_STOCKPILE;
            meter_name_map["Stealth"] =            METER_STEALTH;
            meter_name_map["Detection"] =          METER_DETECTION;
            meter_name_map["Speed"] =              METER_SPEED;
            meter_name_map["Damage"] =             METER_CAPACITY;
            meter_name_map["Capacity"] =           METER_CAPACITY;
            meter_name_map["MaxCapacity"] =        METER_MAX_CAPACITY;
            meter_name_map["SecondaryStat"] =      METER_SECONDARY_STAT;
            meter_name_map["MaxSecondaryStat"] =   METER_MAX_SECONDARY_STAT;
            meter_name_map["Size"] =               METER_SIZE;
        }
        return meter_name_map;
    }

    // force early init to avoid threading issues later
    std::map<std::string, MeterType> dummy = GetMeterNameMap();
}

namespace ValueRef {
MeterType NameToMeter(const std::string& name) {
    MeterType retval = INVALID_METER_TYPE;
    auto it = GetMeterNameMap().find(name);
    if (it != GetMeterNameMap().end())
        retval = it->second;
    return retval;
}

std::string MeterToName(MeterType meter) {
    for (const auto& entry : GetMeterNameMap()) {
        if (entry.second == meter)
            return entry.first;
    }
    return "";
}

std::string ReconstructName(const std::vector<std::string>& property_name,
                            ReferenceType ref_type,
                            bool return_immediate_value)
{
    std::string retval;

    if (return_immediate_value)
        retval += "Value(";

    switch (ref_type) {
    case SOURCE_REFERENCE:                    retval = "Source";          break;
    case EFFECT_TARGET_REFERENCE:             retval = "Target";          break;
    case EFFECT_TARGET_VALUE_REFERENCE:       retval = "Value";           break;
    case CONDITION_LOCAL_CANDIDATE_REFERENCE: retval = "LocalCandidate";  break;
    case CONDITION_ROOT_CANDIDATE_REFERENCE:  retval = "RootCandidate";   break;
    case NON_OBJECT_REFERENCE:                retval = "";                break;
    default:                                  retval = "?????";           break;
    }

    if (ref_type != EFFECT_TARGET_VALUE_REFERENCE) {
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
    if (ref_type == NON_OBJECT_REFERENCE)
        num_references--;
    for (const std::string& property_name_part : property_names)
        if (property_name_part.empty())
             num_references--;
    num_references = std::max(0, num_references);
    std::string format_string;
    switch (num_references) {
    case 0: format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLE0"); break;
    case 1: format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLE1"); break;
    case 2: format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLE2"); break;
    case 3: format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLE3"); break;
    case 4: format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLE4"); break;
    case 5: format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLE5"); break;
    case 6: format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLE6"); break;
    default:format_string = UserString("DESC_VALUE_REF_MULTIPART_VARIABLEMANY"); break;
    }

    boost::format formatter = FlexibleFormat(format_string);

    switch (ref_type) {
    case SOURCE_REFERENCE:                    formatter % UserString("DESC_VAR_SOURCE");          break;
    case EFFECT_TARGET_REFERENCE:             formatter % UserString("DESC_VAR_TARGET");          break;
    case EFFECT_TARGET_VALUE_REFERENCE:       formatter % UserString("DESC_VAR_VALUE");           break;
    case CONDITION_LOCAL_CANDIDATE_REFERENCE: formatter % UserString("DESC_VAR_LOCAL_CANDIDATE"); break;
    case CONDITION_ROOT_CANDIDATE_REFERENCE:  formatter % UserString("DESC_VAR_ROOT_CANDIDATE");  break;
    case NON_OBJECT_REFERENCE:                                                                    break;
    default:                                  formatter % "???";                                  break;
    }

    for (const std::string& property_name_part : property_names) {
        if (property_name_part.empty())  // apparently is empty for a EFFECT_TARGET_VALUE_REFERENCE
            continue;
        std::string stringtable_key("DESC_VAR_" + boost::to_upper_copy(property_name_part));
        formatter % UserString(stringtable_key);
    }

    std::string retval = boost::io::str(formatter);
    //std::cerr << "ret: " << retval << std::endl;
    return retval;
}

std::string ComplexVariableDescription(const std::vector<std::string>& property_names,
                                       const ValueRef::ValueRefBase<int>* int_ref1,
                                       const ValueRef::ValueRefBase<int>* int_ref2,
                                       const ValueRef::ValueRefBase<int>* int_ref3,
                                       const ValueRef::ValueRefBase<std::string>* string_ref1,
                                       const ValueRef::ValueRefBase<std::string>* string_ref2)
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
                                const ValueRef::ValueRefBase<int>* int_ref1,
                                const ValueRef::ValueRefBase<int>* int_ref2,
                                const ValueRef::ValueRefBase<int>* int_ref3,
                                const ValueRef::ValueRefBase<std::string>* string_ref1,
                                const ValueRef::ValueRefBase<std::string>* string_ref2)
{
    std::string retval;
    if (property_names.empty()) {
        ErrorLogger() << "ComplexVariableDump passed empty property names?!";
        return "ComplexVariable";
    } else {
        retval += property_names.back();
    }

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
    case SZ_TINY:       return "Tiny";
    case SZ_SMALL:      return "Small";
    case SZ_MEDIUM:     return "Medium";
    case SZ_LARGE:      return "Large";
    case SZ_HUGE:       return "Huge";
    case SZ_ASTEROIDS:  return "Asteroids";
    case SZ_GASGIANT:   return "GasGiant";
    default:            return "?";
    }
}

template <>
std::string Constant<PlanetType>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case PT_SWAMP:      return "Swamp";
    case PT_TOXIC:      return "Toxic";
    case PT_INFERNO:    return "Inferno";
    case PT_RADIATED:   return "Radiated";
    case PT_BARREN:     return "Barren";
    case PT_TUNDRA:     return "Tundra";
    case PT_DESERT:     return "Desert";
    case PT_TERRAN:     return "Terran";
    case PT_OCEAN:      return "Ocean";
    case PT_ASTEROIDS:  return "Asteroids";
    case PT_GASGIANT:   return "GasGiant";
    default:            return "?";
    }
}

template <>
std::string Constant<PlanetEnvironment>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case PE_UNINHABITABLE:  return "Uninhabitable";
    case PE_HOSTILE:        return "Hostile";
    case PE_POOR:           return "Poor";
    case PE_ADEQUATE:       return "Adequate";
    case PE_GOOD:           return "Good";
    default:                return "?";
    }
}

template <>
std::string Constant<UniverseObjectType>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case OBJ_BUILDING:      return "Building";
    case OBJ_SHIP:          return "Ship";
    case OBJ_FLEET:         return "Fleet"; 
    case OBJ_PLANET:        return "Planet";
    case OBJ_POP_CENTER:    return "PopulationCenter";
    case OBJ_PROD_CENTER:   return "ProductionCenter";
    case OBJ_SYSTEM:        return "System";
    case OBJ_FIELD:         return "Field";
    default:                return "?";
    }
}

template <>
std::string Constant<StarType>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case STAR_BLUE:     return "Blue";
    case STAR_WHITE:    return "White";
    case STAR_YELLOW:   return "Yellow";
    case STAR_ORANGE:   return "Orange";
    case STAR_RED:      return "Red";
    case STAR_NEUTRON:  return "Neutron";
    case STAR_BLACK:    return "BlackHole";
    case STAR_NONE:     return "NoStar";
    default:            return "Unknown";
    }
}

template <>
std::string Constant<Visibility>::Dump(unsigned short ntabs) const
{
    switch (m_value) {
    case VIS_NO_VISIBILITY:     return "Invisible";
    case VIS_BASIC_VISIBILITY:  return "Basic";
    case VIS_PARTIAL_VISIBILITY:return "Partial";
    case VIS_FULL_VISIBILITY:   return "Full";
    default:                    return "Unknown";
    }
}

template <>
std::string Constant<int>::Dump(unsigned short ntabs) const
{ return Description(); }

template <>
std::string Constant<double>::Dump(unsigned short ntabs) const
{ return Description(); }

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

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
#define IF_CURRENT_VALUE(T)                                            \
if (m_ref_type == EFFECT_TARGET_VALUE_REFERENCE) {                     \
    if (context.current_value.empty())                                 \
        throw std::runtime_error(                                      \
            "Variable<" #T ">::Eval(): Value could not be evaluated, " \
            "because no current value was provided.");                 \
    try {                                                              \
        return boost::any_cast<T>(context.current_value);              \
    } catch (const boost::bad_any_cast&) {                             \
        throw std::runtime_error(                                      \
            "Variable<" #T ">::Eval(): Value could not be evaluated, " \
            "because the provided current value is not an " #T ".");   \
    }                                                                  \
}

template <>
PlanetSize Variable<PlanetSize>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(PlanetSize)

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<PlanetSize>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        return INVALID_PLANET_SIZE;
    }

    if (auto p = std::dynamic_pointer_cast<const Planet>(object)) {
        if (property_name == "PlanetSize")
            return p->Size();
        else if (property_name == "NextLargerPlanetSize")
            return p->NextLargerPlanetSize();
        else if (property_name == "NextSmallerPlanetSize")
            return p->NextSmallerPlanetSize();
    }

    ErrorLogger() << "Variable<PlanetSize>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return INVALID_PLANET_SIZE;
}

template <>
PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(PlanetType)

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<PlanetType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        return INVALID_PLANET_TYPE;
    }

    if (auto p = std::dynamic_pointer_cast<const Planet>(object)) {
        if (property_name == "PlanetType")
            return p->Type();
        else if (property_name == "OriginalType")
            return p->OriginalType();
        else if (property_name == "NextCloserToOriginalPlanetType")
            return p->NextCloserToOriginalPlanetType();
        else if (property_name == "NextBetterPlanetType")
            return p->NextBetterPlanetTypeForSpecies();
        else if (property_name == "ClockwiseNextPlanetType")
            return p->ClockwiseNextPlanetType();
        else if (property_name == "CounterClockwiseNextPlanetType")
            return p->CounterClockwiseNextPlanetType();
    }

    ErrorLogger() << "Variable<PlanetType>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return INVALID_PLANET_TYPE;
}

template <>
PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(PlanetEnvironment)

    if (property_name == "PlanetEnvironment") {
        auto object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<PlanetEnvironment>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return INVALID_PLANET_ENVIRONMENT;
        }
        if (auto p = std::dynamic_pointer_cast<const Planet>(object))
            return p->EnvironmentForSpecies();
    }

    ErrorLogger() << "Variable<PlanetEnvironment>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return INVALID_PLANET_ENVIRONMENT;
}

template <>
UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(UniverseObjectType)

    if (property_name == "ObjectType") {
        auto object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<UniverseObjectType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return INVALID_UNIVERSE_OBJECT_TYPE;
        }
        ObjectTypeVisitor v;
        if (object->Accept(v))
            return v.m_type;
        else if (std::dynamic_pointer_cast<const PopCenter>(object))
            return OBJ_POP_CENTER;
        else if (std::dynamic_pointer_cast<const ResourceCenter>(object))
            return OBJ_PROD_CENTER;
    }

    ErrorLogger() << "Variable<UniverseObjectType>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return INVALID_UNIVERSE_OBJECT_TYPE;
}

template <>
StarType Variable<StarType>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(StarType)

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<StarType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        return INVALID_STAR_TYPE;
    }

    if (auto s = std::dynamic_pointer_cast<const System>(object)) {
        if (property_name == "StarType")
            return s->GetStarType();
        else if (property_name == "NextOlderStarType")
            return s->NextOlderStarType();
        else if (property_name == "NextYoungerStarType")
            return s->NextYoungerStarType();
    }

    ErrorLogger() << "Variable<StarType>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << std::to_string(context.source->ID()) << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return INVALID_STAR_TYPE;
}

template <>
Visibility Variable<Visibility>::Eval(const ScriptingContext& context) const
{
    IF_CURRENT_VALUE(Visibility)

    // As of this writing, there are no properties of objects that directly
    // return a Visibility, as it will normally need to be queried for a
    // particular empire

    ErrorLogger() << "Variable<Visibility>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);

    return INVALID_VISIBILITY;
}

template <>
double Variable<double>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(float)

    if (m_ref_type == NON_OBJECT_REFERENCE) {
        if ((property_name == "UniverseCentreX") |
            (property_name == "UniverseCentreY"))
        {
            return GetUniverse().UniverseWidth() / 2;
        } else if (property_name == "UniverseWidth") {
            return GetUniverse().UniverseWidth();
        }

        // add more non-object reference double functions here
        ErrorLogger() << "Variable<double>::Eval unrecognized non-object property: "
                      << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return 0.0;
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<double>::Eval unable to follow reference: "
                      << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return 0.0;
    }

    MeterType meter_type = NameToMeter(property_name);
    if (object && meter_type != INVALID_METER_TYPE) {
        if (object->GetMeter(meter_type)) {
            if (m_return_immediate_value)
                return object->CurrentMeterValue(meter_type);
            else
                return object->InitialMeterValue(meter_type);
        }

    } else if (property_name == "X") {
        return object->X();

    } else if (property_name == "Y") {
        return object->Y();

    } else if (property_name == "SizeAsDouble") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->Size();

    } else if (property_name == "HabitableSize") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->HabitableSize();

    } else if (property_name == "DistanceFromOriginalType") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->DistanceFromOriginalType();

    } else if (property_name == "CurrentTurn") {
        return CurrentTurn();

    } else if (property_name == "Attack") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->Damage();
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->TotalWeaponsDamage();
        if (auto fighter = std::dynamic_pointer_cast<const Fighter>(object))
            return fighter->Damage();

    } else if (property_name == "PropagatedSupplyRange") {
        const auto& ranges = GetSupplyManager().PropagatedSupplyRanges();
        auto range_it = ranges.find(object->SystemID());
        if (range_it == ranges.end())
            return 0.0;
        return range_it->second;

    } else if (property_name == "PropagatedSupplyDistance") {
        const auto& ranges = GetSupplyManager().PropagatedSupplyDistances();
        auto range_it = ranges.find(object->SystemID());
        if (range_it == ranges.end())
            return 0.0;
        return range_it->second;
    }

    ErrorLogger() << "Variable<double>::Eval unrecognized object property: "
                  << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return 0.0;
}

template <>
int Variable<int>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(int)

    if (m_ref_type == NON_OBJECT_REFERENCE) {
        if (property_name == "CurrentTurn")
            return CurrentTurn();
        if (property_name == "GalaxySize")
            return GetGalaxySetupData().GetSize();
        if (property_name == "GalaxyShape")
            return static_cast<int>(GetGalaxySetupData().GetShape());
        if (property_name == "GalaxyAge")
            return static_cast<int>(GetGalaxySetupData().GetAge());
        if (property_name == "GalaxyStarlaneFrequency")
            return static_cast<int>(GetGalaxySetupData().GetStarlaneFreq());
        if (property_name == "GalaxyPlanetDensity")
            return static_cast<int>(GetGalaxySetupData().GetPlanetDensity());
        if (property_name == "GalaxySpecialFrequency")
            return static_cast<int>(GetGalaxySetupData().GetSpecialsFreq());
        if (property_name == "GalaxyMonsterFrequency")
            return static_cast<int>(GetGalaxySetupData().GetMonsterFreq());
        if (property_name == "GalaxyNativeFrequency")
            return static_cast<int>(GetGalaxySetupData().GetNativeFreq());
        if (property_name == "GalaxyMaxAIAggression")
            return static_cast<int>(GetGalaxySetupData().GetAggression());

        // add more non-object reference int functions here
        ErrorLogger() << "Variable<int>::Eval unrecognized non-object property: " << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return 0;
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<int>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return 0;
    }

    if (property_name == "Owner") {
        return object->Owner();
    }
    else if (property_name == "SupplyingEmpire") {
        return GetSupplyManager().EmpireThatCanSupplyAt(object->SystemID());
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
    else if (property_name == "TurnsSinceFocusChange") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->TurnsSinceFocusChange();
        else
            return 0;

    }
    else if (property_name == "ProducedByEmpireID") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->ProducedByEmpireID();
        else if (auto building = std::dynamic_pointer_cast<const Building>(object))
            return building->ProducedByEmpireID();
        else
            return ALL_EMPIRES;

    }
    else if (property_name == "ArrivedOnTurn") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->ArrivedOnTurn();
        else
            return INVALID_GAME_TURN;

    }
    else if (property_name == "DesignID") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->DesignID();
        else
            return INVALID_DESIGN_ID;

    }
    else if (property_name == "SpeciesID") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return GetSpeciesManager().GetSpeciesID(planet->SpeciesName());
        else if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return GetSpeciesManager().GetSpeciesID(ship->SpeciesName());
        else
            return -1;

    }
    else if (property_name == "FleetID") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->FleetID();
        else if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->ID();
        else
            return INVALID_OBJECT_ID;

    }
    else if (property_name == "PlanetID") {
        if (auto building = std::dynamic_pointer_cast<const Building>(object))
            return building->PlanetID();
        else if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->ID();
        else
            return INVALID_OBJECT_ID;

    }
    else if (property_name == "SystemID") {
        return object->SystemID();

    }
    else if (property_name == "FinalDestinationID") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->FinalDestinationID();
        else
            return INVALID_OBJECT_ID;

    }
    else if (property_name == "NextSystemID") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->NextSystemID();
        else
            return INVALID_OBJECT_ID;

    }
    else if (property_name == "PreviousSystemID") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->PreviousSystemID();
        else
            return INVALID_OBJECT_ID;

    }
    else if (property_name == "NearestSystemID") {
        if (object->SystemID() != INVALID_OBJECT_ID)
            return object->SystemID();

        return GetPathfinder()->NearestSystemTo(object->X(), object->Y());

    }
    else if (property_name == "NumShips") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->NumShips();
        else
            return 0;

    }
    else if (property_name == "NumStarlanes") {
        if (auto system = std::dynamic_pointer_cast<const System>(object))
            return system->NumStarlanes();
        else
            return 0;

    }
    else if (property_name == "LastTurnBattleHere") {
        if (auto const_system = std::dynamic_pointer_cast<const System>(object))
            return const_system->LastTurnBattleHere();
        else if (auto system = GetSystem(object->SystemID()))
            return system->LastTurnBattleHere();
        else
            return INVALID_GAME_TURN;

    }
    else if (property_name == "LastTurnActiveInBattle") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->LastTurnActiveInCombat();
        else
            return INVALID_GAME_TURN;

    }
    else if (property_name == "LastTurnAttackedByShip") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->LastTurnAttackedByShip();
        else
            return INVALID_GAME_TURN;

    }
    else if (property_name == "LastTurnConquered") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->LastTurnConquered();
        else
            return INVALID_GAME_TURN;

    }
    else if (property_name == "LastTurnResupplied") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->LastResuppliedOnTurn();
        else
            return INVALID_GAME_TURN;

    }
    else if (property_name == "Orbit") {
        if (auto system = GetSystem(object->SystemID()))
            return system->OrbitOfPlanet(object->ID());
        return -1;

    }
    else if (property_name == "ETA") {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            return fleet->ETA().first;
        else
            return 0;

    }
    else if (property_name == "NumSpecials") {
        return object->Specials().size();

    }
    else if (property_name == "LaunchedFrom") {
        if (auto fighter = std::dynamic_pointer_cast<const Fighter>(object))
            return fighter->LaunchedFrom();
        else
            return INVALID_OBJECT_ID;
    }

    ErrorLogger() << "Variable<int>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return 0;
}

template <>
std::vector<std::string> Variable<std::vector<std::string>>::Eval(
    const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(std::vector<std::string>)

    if (m_ref_type == NON_OBJECT_REFERENCE) {
        // add more non-object reference int functions here
        ErrorLogger() << "std::vector<std::string>::Eval unrecognized non-object property: " << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return {};
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<int>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return {};
    }

    if (property_name == "Tags") {
        std::vector<std::string> retval;
        for (auto tag : object->Tags())
            retval.push_back(tag);
        return retval;
    }
    else if (property_name == "Specials") {
        std::vector<std::string> retval;
        for (auto spec : object->Specials())
            retval.push_back(spec.first);
        return retval;
    }
    else if (property_name == "AvailableFoci") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->AvailableFoci();
    }
    else if (property_name == "Parts") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            if (const ShipDesign* design = ship->Design())
                return design->Parts();
    }

    ErrorLogger() << "std::vector<std::string>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return {};
}

template <>
std::string Variable<std::string>::Eval(const ScriptingContext& context) const
{
    const std::string& property_name = m_property_name.empty() ? "" : m_property_name.back();

    IF_CURRENT_VALUE(std::string)

    if (m_ref_type == NON_OBJECT_REFERENCE) {
        if (property_name == "GalaxySeed")
            return GetGalaxySetupData().GetSeed();

        ErrorLogger() << "Variable<std::string>::Eval unrecognized non-object property: " << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return "";
    }

    auto object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                  m_ref_type, context);
    if (!object) {
        ErrorLogger() << "Variable<std::string>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
        if (context.source)
            ErrorLogger() << "source: " << context.source->ObjectType() << " "
                          << context.source->ID() << " ( " << context.source->Name() << " ) ";
        else
            ErrorLogger() << "source (none)";

        return "";
    }

    if (property_name == "Name") {
        return object->Name();

    } else if (property_name == "OwnerName") {
        int owner_empire_id = object->Owner();
        if (Empire* empire = GetEmpire(owner_empire_id))
            return empire->Name();
        return "";

    } else if (property_name == "TypeName") {
        return boost::lexical_cast<std::string>(object->ObjectType());

    } else if (property_name == "Species") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->SpeciesName();
        else if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            return ship->SpeciesName();
        else if (auto fighter = std::dynamic_pointer_cast<const Fighter>(object))
            return fighter->SpeciesName();

    } else if (property_name == "Hull") {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(object))
            if (const ShipDesign* design = ship->Design())
                return design->Hull();

    } else if (property_name == "BuildingType") {
        if (auto building = std::dynamic_pointer_cast<const Building>(object))
            return building->BuildingTypeName();

    } else if (property_name == "Focus") {
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object))
            return planet->Focus();

    } else if (property_name == "PreferredFocus") {
        const Species* species = nullptr;
        if (auto planet = std::dynamic_pointer_cast<const Planet>(object)) {
            species = GetSpecies(planet->SpeciesName());
        } else if (auto ship = std::dynamic_pointer_cast<const Ship>(object)) {
            species = GetSpecies(ship->SpeciesName());
        }
        if (species)
            return species->PreferredFocus();
        return "";

    } else if (property_name == "OwnerLeastExpensiveEnqueuedTech") {
        const Empire* empire = GetEmpire(object->Owner());
        if (!empire)
            return "";
        return empire->LeastExpensiveEnqueuedTech();

    } else if (property_name == "OwnerMostExpensiveEnqueuedTech") {
        const Empire* empire = GetEmpire(object->Owner());
        if (!empire)
            return "";
        return empire->MostExpensiveEnqueuedTech();

    } else if (property_name == "OwnerMostRPCostLeftEnqueuedTech") {
        const Empire* empire = GetEmpire(object->Owner());
        if (!empire)
            return "";
        return empire->MostRPCostLeftEnqueuedTech();

    } else if (property_name == "OwnerMostRPSpentEnqueuedTech") {
        const Empire* empire = GetEmpire(object->Owner());
        if (!empire)
            return "";
        return empire->MostRPSpentEnqueuedTech();

    } else if (property_name == "OwnerTopPriorityEnqueuedTech") {
        const Empire* empire = GetEmpire(object->Owner());
        if (!empire)
            return "";
        return empire->TopPriorityEnqueuedTech();
    }

    ErrorLogger() << "Variable<std::string>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
    if (context.source)
        ErrorLogger() << "source: " << context.source->ObjectType() << " "
                      << context.source->ID() << " ( " << context.source->Name() << " ) ";
    else
        ErrorLogger() << "source (none)";

    return "";
}

#undef IF_CURRENT_VALUE

///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////
template <>
double Statistic<double>::Eval(const ScriptingContext& context) const
{
    Condition::ObjectSet condition_matches;
    GetConditionMatches(context, condition_matches, m_sampling_condition.get());

    // these two statistic types don't depend on the object property values,
    // so can be evaluated without getting those values.
    if (m_stat_type == COUNT)
        return static_cast<double>(condition_matches.size());
    if (m_stat_type == IF)
        return condition_matches.empty() ? 0.0 : 1.0;

    // evaluate property for each condition-matched object
    std::map<std::shared_ptr<const UniverseObject>, double> object_property_values;
    GetObjectPropertyValues(context, condition_matches, object_property_values);

    return ReduceData(object_property_values);
}

template <>
int Statistic<int>::Eval(const ScriptingContext& context) const
{
    Condition::ObjectSet condition_matches;
    GetConditionMatches(context, condition_matches, m_sampling_condition.get());

    // these two statistic types don't depend on the object property values,
    // so can be evaluated without getting those values.
    if (m_stat_type == COUNT)
        return static_cast<int>(condition_matches.size());
    if (m_stat_type == IF)
        return condition_matches.empty() ? 0 : 1;

    // evaluate property for each condition-matched object
    std::map<std::shared_ptr<const UniverseObject>, int> object_property_values;
    GetObjectPropertyValues(context, condition_matches, object_property_values);

    return ReduceData(object_property_values);
}

template <>
std::string Statistic<std::string>::Eval(const ScriptingContext& context) const
{
    Condition::ObjectSet condition_matches;
    GetConditionMatches(context, condition_matches, m_sampling_condition.get());

    if (condition_matches.empty())
        return "";

    // special case for IF statistic... return a non-empty string for true
    if (m_stat_type == IF)
        return " "; // not an empty string

    // todo: consider allowing MAX and MIN using string sorting?

    // the only other statistic that can be computed on non-number property
    // types and that is itself of a non-number type is the most common value
    if (m_stat_type != MODE) {
        ErrorLogger() << "Statistic<std::string>::Eval has invalid statistic type: "
                      << m_stat_type;
        return "";
    }

    // evaluate property for each condition-matched object
    std::map<std::shared_ptr<const UniverseObject>, std::string> object_property_values;
    GetObjectPropertyValues(context, condition_matches, object_property_values);

    // count number of each result, tracking which has the most occurances
    std::map<std::string, unsigned int> histogram;
    auto most_common_property_value_it = histogram.begin();
    unsigned int max_seen(0);

    for (const auto& entry : object_property_values) {
        const std::string& property_value = entry.second;

        auto hist_it = histogram.find(property_value);
        if (hist_it == histogram.end())
            hist_it = histogram.insert({property_value, 0}).first;
        unsigned int& num_seen = hist_it->second;

        num_seen++;

        if (num_seen > max_seen) {
            most_common_property_value_it = hist_it;
            max_seen = num_seen;
        }
    }

    // return result (property value) that occured most frequently
    return most_common_property_value_it->first;
}

///////////////////////////////////////////////////////////
// ComplexVariable                                       //
///////////////////////////////////////////////////////////
template <>
PlanetSize ComplexVariable<PlanetSize>::Eval(const ScriptingContext& context) const
{ return INVALID_PLANET_SIZE; }

template <>
PlanetType ComplexVariable<PlanetType>::Eval(const ScriptingContext& context) const
{ return INVALID_PLANET_TYPE; } // TODO: Species favourite planet type?

template <>
PlanetEnvironment ComplexVariable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
{ return INVALID_PLANET_ENVIRONMENT; }

template <>
UniverseObjectType ComplexVariable<UniverseObjectType>::Eval(const ScriptingContext& context) const
{ return INVALID_UNIVERSE_OBJECT_TYPE; }

template <>
StarType ComplexVariable<StarType>::Eval(const ScriptingContext& context) const
{ return INVALID_STAR_TYPE; }

template <>
Visibility ComplexVariable<Visibility>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    if (variable_name == "EmpireObjectVisiblity") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return VIS_NO_VISIBILITY;
        }

        int object_id = INVALID_OBJECT_ID;
        if (m_int_ref2) {
            object_id = m_int_ref2->Eval(context);
            if (object_id == INVALID_OBJECT_ID)
                return VIS_NO_VISIBILITY;
        }

        return GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id);
    }

    return INVALID_VISIBILITY;
}

namespace {
    static std::map<std::string, int> EMPTY_STRING_INT_MAP;
    static std::map<int, int> EMPTY_INT_INT_MAP;
    static std::map<int, float> EMPTY_INT_FLOAT_MAP;

    const std::map<std::string, int>& GetEmpireStringIntMap(int empire_id, const std::string& parsed_map_name) {
        Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return EMPTY_STRING_INT_MAP;

        if (parsed_map_name == "BuildingTypesOwned")
            return empire->BuildingTypesOwned();
        if (parsed_map_name == "BuildingTypesProduced")
            return empire->BuildingTypesProduced();
        if (parsed_map_name == "BuildingTypesScrapped")
            return empire->BuildingTypesScrapped();
        if (parsed_map_name == "SpeciesColoniesOwned")
            return empire->SpeciesColoniesOwned();
        if (parsed_map_name == "SpeciesPlanetsBombed")
            return empire->SpeciesPlanetsBombed();
        if (parsed_map_name == "SpeciesPlanetsDepoped")
            return empire->SpeciesPlanetsDepoped();
        if (parsed_map_name == "SpeciesPlanetsInvaded")
            return empire->SpeciesPlanetsInvaded();
        if (parsed_map_name == "SpeciesShipsDestroyed")
            return empire->SpeciesShipsDestroyed();
        if (parsed_map_name == "SpeciesShipsLost")
            return empire->SpeciesShipsLost();
        if (parsed_map_name == "SpeciesShipsOwned")
            return empire->SpeciesShipsOwned();
        if (parsed_map_name == "SpeciesShipsProduced")
            return empire->SpeciesShipsProduced();
        if (parsed_map_name == "SpeciesShipsScrapped")
            return empire->SpeciesShipsScrapped();
        if (parsed_map_name == "ShipPartsOwned")
            return empire->ShipPartTypesOwned();
        if (parsed_map_name == "TurnTechResearched")
            return empire->ResearchedTechs();

        return EMPTY_STRING_INT_MAP;
    }

    const std::map<int, int>& GetEmpireIntIntMap(int empire_id, const std::string& parsed_map_name) {
        Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return EMPTY_INT_INT_MAP;

        if (parsed_map_name == "EmpireShipsDestroyed")
            return empire->EmpireShipsDestroyed();
        if (parsed_map_name == "ShipDesignsDestroyed")
            return empire->ShipDesignsDestroyed();
        if (parsed_map_name == "ShipDesignsLost")
            return empire->ShipDesignsLost();
        if (parsed_map_name == "ShipDesignsOwned")
            return empire->ShipDesignsOwned();
        if (parsed_map_name == "ShipDesignsProduced")
            return empire->ShipDesignsProduced();
        if (parsed_map_name == "ShipDesignsScrapped")
            return empire->ShipDesignsScrapped();

        return EMPTY_INT_INT_MAP;
    }

    const std::map<int, float>& GetEmpireIntFloatMap(int empire_id, const std::string& parsed_map_name) {
        Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return EMPTY_INT_FLOAT_MAP;

        if (parsed_map_name == "PropagatedSystemSupplyRange")
            return GetSupplyManager().PropagatedSupplyRanges(empire_id);
        if (parsed_map_name == "SystemSupplyRange")
            return empire->SystemSupplyRanges();
        if (parsed_map_name == "PropagatedSystemSupplyDistance")
            return GetSupplyManager().PropagatedSupplyDistances(empire_id);

        return EMPTY_INT_FLOAT_MAP;
    }

    int GetIntEmpirePropertyNoKeyImpl(int empire_id, const std::string& parsed_property_name) {
        Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return 0;

        if (parsed_property_name == "OutpostsOwned")
            return empire->OutpostsOwned();
        // todo: add all the various OwnerWhatever ValueRef stuff here

        return 0;
    }

    // gets property for a particular map key string for one or all empires
    int GetIntEmpirePropertySingleKey(int empire_id, const std::string& parsed_property_name,
                                      const std::string& map_key)
    {
        int sum = 0;
        if (map_key.empty())
            return sum;

        // single empire
        if (empire_id != ALL_EMPIRES) {
            const auto& map = GetEmpireStringIntMap(empire_id, parsed_property_name);
            auto it = map.find(map_key);
            if (it == map.end())
                return 0;
            return it->second;
        }

        // all empires summed
        for (auto& entry : Empires()) {
            const auto& map = GetEmpireStringIntMap(entry.first, parsed_property_name);
            auto map_it = map.find(map_key);
            if (map_it != map.end())
                sum += map_it->second;
        }
        return sum;
    }

    // gets property for the sum of all map keys for one or all empires
    int GetIntEmpirePropertySumAllStringKeys(int empire_id, const std::string& parsed_property_name) {
        int sum = 0;

        // single empire
        if (empire_id != ALL_EMPIRES) {
            // sum of all key entries for this empire
            for (const auto& entry : GetEmpireStringIntMap(empire_id, parsed_property_name))
                sum += entry.second;
            return sum;
        }

        // all empires summed
        for (const auto& empire_entry : Empires()) {
            for (const auto& property_entry : GetEmpireStringIntMap(empire_entry.first, parsed_property_name))
                sum += property_entry.second;
        }
        return sum;
    }

    // gets property for a particular map key int for one or all empires
    int GetIntEmpirePropertySingleKey(int empire_id, const std::string& parsed_property_name,
                                      int map_key)
    {
        int sum = 0;

        // single empire
        if (empire_id != ALL_EMPIRES) {
            const auto& map = GetEmpireIntIntMap(empire_id, parsed_property_name);
            auto it = map.find(map_key);
            if (it == map.end())
                return 0;
            return it->second;
        }

        // all empires summed
        for (const auto& empire_entry : Empires()) {
            const auto& map = GetEmpireIntIntMap(empire_entry.first, parsed_property_name);
            auto map_it = map.find(map_key);
            if (map_it != map.end())
                sum += map_it->second;
        }
        return sum;
    }

    float GetFloatEmpirePropertySingleKey(int empire_id, const std::string& parsed_property_name,
                                          int map_key)
    {
        float sum = 0.0f;

        // single empire
        if (empire_id != ALL_EMPIRES) {
            const auto& map = GetEmpireIntFloatMap(empire_id, parsed_property_name);
            auto it = map.find(map_key);
            if (it == map.end())
                return 0.0f;
            return it->second;
        }

        // all empires summed
        for (const auto& empire_entry : Empires()) {
            const auto& map = GetEmpireIntFloatMap(empire_entry.first, parsed_property_name);
            auto map_it = map.find(map_key);
            if (map_it != map.end())
                sum += map_it->second;
        }
        return sum;
    }

    // gets property for the sum of all map keys for one or all empires
    int GetIntEmpirePropertySumAllIntKeys(int empire_id, const std::string& parsed_property_name) {
        int sum = 0;

        // single empire
        if (empire_id != ALL_EMPIRES) {
            // sum of all key entries for this empire
            for (const auto& property_entry : GetEmpireIntIntMap(empire_id, parsed_property_name))
                sum += property_entry.second;
            return sum;
        }

        // all empires summed
        for (const auto& empire_entry : Empires()) {
            for (const auto& property_entry : GetEmpireIntIntMap(empire_entry.first, parsed_property_name))
                sum += property_entry.second;
        }
        return sum;
    }

    float  GetFloatEmpirePropertySumAllIntKeys(int empire_id, const std::string& parsed_property_name) {
        float sum = 0.0f;

        // single empire
        if (empire_id != ALL_EMPIRES) {
            // sum of all key entries for this empire
            for (const auto& property_entry : GetEmpireIntFloatMap(empire_id, parsed_property_name))
                sum += property_entry.second;
            return sum;
        }

        // all empires summed
        for (const auto& empire_entry : Empires()) {
            for (const auto& property_entry : GetEmpireIntFloatMap(empire_entry.first, parsed_property_name))
                sum += property_entry.second;
        }
        return sum;
    }

    // gets map index int for content specified by a string. eg. look up
    // predefined ship design id by name.
    // how to look up such strings depends on the parsed property name.
    int GetIntKeyFromContentStringKey(const std::string& parsed_property_name,
                                      const std::string& key_string)
    {
        if (boost::istarts_with(parsed_property_name, "ShipDesign")) {
            // look up ship design id corresponding to specified predefined ship design name
            const ShipDesign* design = GetPredefinedShipDesign(key_string);
            if (design)
                return design->ID();
        }
        return -1;
    }

    // gets unkeyed property for one or all empires
    int GetIntEmpirePropertyNoKey(int empire_id, const std::string& parsed_property_name) {
        int sum = 0;

        if (empire_id != ALL_EMPIRES) {
            // single empire
            return GetIntEmpirePropertyNoKeyImpl(empire_id, parsed_property_name);
        }

        // all empires summed
        for (const auto& empire_entry : Empires())
            sum += GetIntEmpirePropertyNoKeyImpl(empire_entry.first, parsed_property_name);
        return sum;
    }
}

template <>
int ComplexVariable<int>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    // empire properties indexed by strings
    if (variable_name == "BuildingTypesOwned" ||
        variable_name == "BuildingTypesProduced" ||
        variable_name == "BuildingTypesScrapped" ||
        variable_name == "SpeciesColoniesOwned" ||
        variable_name == "SpeciesPlanetsBombed" ||
        variable_name == "SpeciesPlanetsDepoped" ||
        variable_name == "SpeciesPlanetsInvaded" ||
        variable_name == "SpeciesShipsDestroyed" ||
        variable_name == "SpeciesShipsLost" ||
        variable_name == "SpeciesShipsOwned" ||
        variable_name == "SpeciesShipsProduced" ||
        variable_name == "SpeciesShipsScrapped" ||
        variable_name == "TurnTechResearched")
    {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
        }
        std::string key_string;
        if (m_string_ref1) {
            key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;

            // if a string specified, get just that entry (for single empire, or
            // summed for all empires)
            return GetIntEmpirePropertySingleKey(empire_id, variable_name,
                                                 key_string);
        }

        // if no string specified, get sum of all entries (for single empire
        // or summed for all empires)
        return GetIntEmpirePropertySumAllStringKeys(empire_id, variable_name);
    }

    // empire properties indexed by integers
    if (variable_name == "EmpireShipsDestroyed" ||
        variable_name == "ShipDesignsDestroyed" ||
        variable_name == "ShipDesignsLost" ||
        variable_name == "ShipDesignsOwned" ||
        variable_name == "ShipDesignsProduced" ||
        variable_name == "ShipDesignsScrapped")
    {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
        }

        // if a key integer specified, get just that entry (for single empire or sum of all empires)
        if (m_int_ref2) {
            int key_int = m_int_ref2->Eval(context);
            return GetIntEmpirePropertySingleKey(empire_id, variable_name, key_int);
        }

        // although indexed by integers, some of these may be specified by a
        // string that needs to be looked up. if a key string specified, get
        // just that entry (for single empire or sum of all empires)
        if (m_string_ref1) {
            std::string key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;
            int key_int = GetIntKeyFromContentStringKey(variable_name, key_string);
            return GetIntEmpirePropertySingleKey(empire_id, variable_name, key_int);
        }

        // if no key specified, get sum of all entries (for single empire or sum of all empires)
        return GetIntEmpirePropertySumAllIntKeys(empire_id, variable_name);
    }

    // empire properties indexed by string or ShipPartClass (as int)
    if (variable_name == "ShipPartsOwned") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
        }

        // get key if either was specified
        std::string key_string;
        int key_int = NUM_SHIP_PART_CLASSES;
        if (m_string_ref1) {
            key_string = m_string_ref1->Eval(context);
            if (key_string.empty())
                return 0;
        } else if (m_int_ref2) {
            key_int = m_int_ref2->Eval(context);
            if (key_int <= INVALID_SHIP_PART_CLASS || key_int >= NUM_SHIP_PART_CLASSES)
                return 0;
        }

        // if a key string specified, get just that entry (for single empire or sum of all empires)
        if (m_string_ref1)
            return GetIntEmpirePropertySingleKey(empire_id, variable_name, key_string);

        // if a key integer specified, get just that entry (for single empire or sum of all empires)
        if (m_int_ref2) {
            ShipPartClass part_class = ShipPartClass(key_int);
            int sum = 0;
            // single empire
            if (empire_id != ALL_EMPIRES) {
                Empire* empire = GetEmpire(empire_id);
                for (const auto& property_entry : empire->ShipPartClassOwned())
                    if (part_class == NUM_SHIP_PART_CLASSES || property_entry.first == part_class)
                        sum += property_entry.second;
                return sum;
            }

            // all empires summed
            for (const auto& empire_entry : Empires()) {
                Empire* empire = GetEmpire(empire_entry.first);
                for (const auto& property_entry : empire->ShipPartClassOwned())
                    if (part_class == NUM_SHIP_PART_CLASSES || property_entry.first == part_class)
                        sum += property_entry.second;
            }
            return sum;
        }

        // no key string or integer provided, get all (for single empire or sum of all empires)
        return GetIntEmpirePropertySumAllStringKeys(empire_id, variable_name);
    }

    // unindexed empire proprties
    if (variable_name == "OutpostsOwned") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0;
        }

        return GetIntEmpirePropertyNoKey(empire_id, variable_name);
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

        std::string part_type_name;
        if (m_string_ref1) {
            part_type_name = m_string_ref1->Eval(context);
        }

        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return 0;

        int count = 0;
        for (const std::string& part : design->Parts()) {
            if (part_type_name.empty() && !part.empty())
                count++;
            else if (!part_type_name.empty() && part_type_name == part)
                count ++;
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
        ShipPartClass part_class = INVALID_SHIP_PART_CLASS;
        try {
            part_class = boost::lexical_cast<ShipPartClass>(part_class_name);
        } catch (...) {
            return 0;
        }

        int count = 0;
        for (const std::string& part_name : design->Parts()) {
            if (part_name.empty())
                continue;
            const PartType* part = GetPartType(part_name);
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

        int retval = GetPathfinder()->JumpDistanceBetweenObjects(object1_id, object2_id);
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

        int retval = GetPathfinder()->JumpDistanceBetweenObjects(object1_id, object2_id/*, empire_id*/);
        if (retval == INT_MAX)
            return -1;
        return retval;
    }
    else if (variable_name == "SlotsInHull") {
        const HullType* hull_type = nullptr;
        if (m_string_ref1) {
            std::string hull_name = m_string_ref1->Eval(context);
            hull_type = GetHullType(hull_name);
            if (!hull_type)
                return 0;
        } else {
            return 0;
        }
        return hull_type->Slots().size();
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

        const HullType* hull_type = GetHullType(design->Hull());
        if (!hull_type)
            return 0;
        return hull_type->Slots().size();
    }
    else if (variable_name == "SpecialAddedOnTurn") {
        int object_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object_id = m_int_ref1->Eval(context);
        if (object_id == INVALID_OBJECT_ID)
            return 0;
        auto object = GetUniverseObject(object_id);
        if (!object)
            return 0;

        std::string special_name;
        if (m_string_ref1)
            special_name = m_string_ref1->Eval(context);
        if (special_name.empty())
            return 0;

        return object->SpecialAddedOnTurn(special_name);
    }

    return 0;
}

template <>
double ComplexVariable<double>::Eval(const ScriptingContext& context) const
{
    const std::string& variable_name = m_property_name.back();

    // empire properties indexed by integers
    if (variable_name == "PropagatedSystemSupplyRange" ||
        variable_name == "SystemSupplyRange" ||
        variable_name == "PropagatedSystemSupplyDistance")
    {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return 0.0;
        }

        // if a key integer is specified, get just that entry (for single empire or sum of all empires)
        if (m_int_ref2) {
            int key_int = m_int_ref2->Eval(context);
            return GetFloatEmpirePropertySingleKey(empire_id, variable_name, key_int);
        }

        // if no key specified, get sum of all entries (for single empire or sum of all empires)
        return GetFloatEmpirePropertySumAllIntKeys(empire_id, variable_name);
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
    else if (variable_name == "HullFuel") {
        std::string hull_type_name;
        if (m_string_ref1)
            hull_type_name = m_string_ref1->Eval(context);

        const HullType* hull_type = GetHullType(hull_type_name);
        if (!hull_type)
            return 0.0;

        return hull_type->Fuel();

    }
    else if (variable_name == "HullStealth") {
        std::string hull_type_name;
        if (m_string_ref1)
            hull_type_name = m_string_ref1->Eval(context);

        const HullType* hull_type = GetHullType(hull_type_name);
        if (!hull_type)
            return 0.0;

        return hull_type->Stealth();

    }
    else if (variable_name == "HullStructure") {
        std::string hull_type_name;
        if (m_string_ref1)
            hull_type_name = m_string_ref1->Eval(context);

        const HullType* hull_type = GetHullType(hull_type_name);
        if (!hull_type)
            return 0.0f;

        return hull_type->Structure();

    }
    else if (variable_name == "HullSpeed") {
        std::string hull_type_name;
        if (m_string_ref1)
            hull_type_name = m_string_ref1->Eval(context);

        const HullType* hull_type = GetHullType(hull_type_name);
        if (!hull_type)
            return 0.0;

        return hull_type->Speed();

    }
    else if (variable_name == "PartCapacity") {
        std::string part_type_name;
        if (m_string_ref1)
            part_type_name = m_string_ref1->Eval(context);

        const PartType* part_type = GetPartType(part_type_name);
        if (!part_type)
            return 0.0;

        return part_type->Capacity();

    }
    else if (variable_name == "PartSecondaryStat") {
        std::string part_type_name;
        if (m_string_ref1)
            part_type_name = m_string_ref1->Eval(context);

        const PartType* part_type = GetPartType(part_type_name);
        if (!part_type)
            return 0.0;

        return part_type->SecondaryStat();

    }
    else if (variable_name == "EmpireMeterValue") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1)
            empire_id = m_int_ref1->Eval(context);
        Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return 0.0;

        std::string empire_meter_name;
        if (m_string_ref1)
            empire_meter_name = m_string_ref1->Eval(context);
        Meter* meter = empire->GetMeter(empire_meter_name);
        if (!meter)
            return 0.0;
        return meter->Current();
    }
    else if (variable_name == "DirectDistanceBetween") {
        int object1_id = INVALID_OBJECT_ID;
        if (m_int_ref1)
            object1_id = m_int_ref1->Eval(context);
        auto obj1 = GetUniverseObject(object1_id);
        if (!obj1)
            return 0.0;

        int object2_id = INVALID_OBJECT_ID;
        if (m_int_ref2)
            object2_id = m_int_ref2->Eval(context);
        auto obj2 = GetUniverseObject(object2_id);
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

        return GetPathfinder()->ShortestPathDistance(object1_id, object2_id);

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
        auto object = GetUniverseObject(object_id);
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
        auto object = GetUniverseObject(object_id);
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

        MeterType meter_type = ValueRef::NameToMeter(meter_name);
        if (meter_type != INVALID_METER_TYPE) {
            if (m_return_immediate_value)
                return ship->CurrentPartMeterValue(meter_type, part_name);
            else
                return ship->InitialPartMeterValue(meter_type, part_name);
        }
    }

    return 0.0;
}

namespace {
    std::vector<std::string> TechsResearchedByEmpire(int empire_id) {
        std::vector<std::string> retval;
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return retval;
        for (const auto& tech : GetTechManager()) {
            if (empire->TechResearched(tech->Name()))
                retval.push_back(tech->Name());
        }
        return retval;
    }

    std::vector<std::string> TechsResearchableByEmpire(int empire_id) {
        std::vector<std::string> retval;
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return retval;
        for (const auto& tech : GetTechManager()) {
            if (empire->ResearchableTech(tech->Name()))
                retval.push_back(tech->Name());
        }
        return retval;
    }

    std::vector<std::string> TransferrableTechs(int sender_empire_id, int receipient_empire_id) {
        std::vector<std::string> sender_researched_techs = TechsResearchedByEmpire(sender_empire_id);
        std::vector<std::string> recepient_researchable = TechsResearchableByEmpire(receipient_empire_id);

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

    // unindexed empire properties
    if (variable_name == "LowestCostEnqueuedTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->LeastExpensiveEnqueuedTech();

    } else if (variable_name == "HighestCostEnqueuedTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->MostExpensiveEnqueuedTech();

    } else if (variable_name == "TopPriorityEnqueuedTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->TopPriorityEnqueuedTech();

    } else if (variable_name == "MostSpentEnqueuedTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->MostRPSpentEnqueuedTech();

    } else if (variable_name == "RandomEnqueuedTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        // get all techs on queue, randomly pick one
        const ResearchQueue& queue = empire->GetResearchQueue();
        std::vector<std::string> all_enqueued_techs = queue.AllEnqueuedProjects();
        if (all_enqueued_techs.empty())
            return "";
        std::size_t idx = RandSmallInt(0, static_cast<int>(all_enqueued_techs.size()) - 1);
        return *std::next(all_enqueued_techs.begin(), idx);
    } else if (variable_name == "LowestCostResearchableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->LeastExpensiveResearchableTech();

    } else if (variable_name == "HighestCostResearchableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->MostExpensiveResearchableTech();

    } else if (variable_name == "TopPriorityResearchableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->TopPriorityResearchableTech();

    } else if (variable_name == "MostSpentResearchableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
        return empire->MostExpensiveResearchableTech();

    } else if (variable_name == "RandomResearchableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

        std::vector<std::string> researchable_techs = TechsResearchableByEmpire(empire_id);
        if (researchable_techs.empty())
            return "";
        std::size_t idx = RandSmallInt(0, static_cast<int>(researchable_techs.size()) - 1);
        return *std::next(researchable_techs.begin(), idx);
    } else if (variable_name == "RandomCompleteTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

        std::vector<std::string> complete_techs = TechsResearchedByEmpire(empire_id);
        if (complete_techs.empty())
            return "";
        std::size_t idx = RandSmallInt(0, static_cast<int>(complete_techs.size()) - 1);
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

        std::vector<std::string> sendable_techs = TransferrableTechs(empire1_id, empire2_id);
        if (sendable_techs.empty())
            return "";
        std::size_t idx = RandSmallInt(0, static_cast<int>(sendable_techs.size()) - 1);
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

        std::vector<std::string> sendable_techs = TransferrableTechs(empire1_id, empire2_id);
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
        const Empire* empire2 = GetEmpire(empire2_id);
        if (!empire2)
            return "";

        std::vector<std::string> sendable_techs = TransferrableTechs(empire1_id, empire2_id);
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

    } else if (variable_name == "MostSpentTransferrableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

    } else if (variable_name == "RandomTransferrableTech") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

    } else if (variable_name == "MostPopulousSpecies") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

    } else if (variable_name == "MostHappySpecies") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

    } else if (variable_name == "LeastHappySpecies") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

    } else if (variable_name == "RandomColonizableSpecies") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";

    } else if (variable_name == "RandomControlledSpecies") {
        int empire_id = ALL_EMPIRES;
        if (m_int_ref1) {
            empire_id = m_int_ref1->Eval(context);
            if (empire_id == ALL_EMPIRES)
                return "";
        }
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return "";
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

#undef IF_CURRENT_VALUE

template <>
std::string ComplexVariable<Visibility>::Dump(unsigned short ntabs) const
{
    const std::string& variable_name = m_property_name.back();
    std::string retval = variable_name;

    if (variable_name == "EmpireObjectVisiblity") {
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

    return retval;
}

template <>
std::string ComplexVariable<std::string>::Dump(unsigned short ntabs) const
{
    const std::string& variable_name = m_property_name.back();
    std::string retval = variable_name;

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
NameLookup::NameLookup(std::unique_ptr<ValueRefBase<int>>&& value_ref, LookupType lookup_type) :
    Variable<std::string>(NON_OBJECT_REFERENCE),
    m_value_ref(std::move(value_ref)),
    m_lookup_type(lookup_type)
{}

bool NameLookup::operator==(const ValueRefBase<std::string>& rhs) const {
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
    if (!m_value_ref || m_lookup_type == INVALID_LOOKUP)
        return "";

    switch (m_lookup_type) {
    case OBJECT_NAME: {
        auto obj = GetUniverseObject(m_value_ref->Eval(context));
        return obj ? obj->Name() : "";
        break;
    }
    case EMPIRE_NAME: {
        const Empire* empire = GetEmpire(m_value_ref->Eval(context));
        return empire ? empire->Name() : "";
        break;
    }
    case SHIP_DESIGN_NAME: {
        const ShipDesign* design = GetShipDesign(m_value_ref->Eval(context));
        return design ? design->Name() : "";
        break;
    }
    default:
        return "";
    }
}

bool NameLookup::RootCandidateInvariant() const
{ return m_value_ref->RootCandidateInvariant(); }

bool NameLookup::LocalCandidateInvariant() const
{ return !m_value_ref || m_value_ref->LocalCandidateInvariant(); }

bool NameLookup::TargetInvariant() const
{ return !m_value_ref || m_value_ref->TargetInvariant(); }

bool NameLookup::SourceInvariant() const
{ return !m_value_ref || m_value_ref->SourceInvariant(); }

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
    std::cout << "GetCheckSum(NameLookup): " << typeid(*this).name() << " retval: " << retval << std::endl << std::endl;
    return retval;
}

///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
template <>
std::string Operation<std::string>::EvalImpl(const ScriptingContext& context) const
{
    if (m_op_type == PLUS) {
        return LHS()->Eval(context) + RHS()->Eval(context);

    } else if (m_op_type == TIMES) {
        // useful for writing a "Statistic If" expression with strings. Number-
        // valued types return 0 or 1 for nothing or something matching the sampling
        // condition. For strings, an empty string indicates no matches, and non-empty
        // string indicates matches, which is treated like a multiplicative identity
        // operation, so just returns the RHS of the expression.
        if (LHS()->Eval(context).empty())
            return "";
        return RHS()->Eval(context);

    } else if (m_op_type == MINIMUM || m_op_type == MAXIMUM) {
        // evaluate all operands, return sorted first/last
        std::set<std::string> vals;
        for (auto& vr : m_operands) {
            if (vr)
                vals.insert(vr->Eval(context));
        }
        if (m_op_type == MINIMUM)
            return vals.empty() ? "" : *vals.begin();
        else
            return vals.empty() ? "" : *vals.rbegin();

    } else if (m_op_type == RANDOM_PICK) {
        // select one operand, evaluate it, return result
        if (m_operands.empty())
            return "";
        unsigned int idx = RandSmallInt(0, m_operands.size() - 1);
        auto& vr = *std::next(m_operands.begin(), idx);
        if (!vr)
            return "";
        return vr->Eval(context);

    } else if (m_op_type == SUBSTITUTION) {
        // insert string into other string in place of %1% or similar placeholder
        if (m_operands.empty())
            return "";
        auto& template_op = *(m_operands.begin());
        if (!template_op)
            return "";
        std::string template_str = template_op->Eval(context);

        boost::format formatter = FlexibleFormat(template_str);

        for (auto& op : m_operands) {
            if (!op) {
                formatter % "";
                continue;
            }
            formatter % op->Eval(context);
        }
        return formatter.str();

    } else if (m_op_type >= COMPARE_EQUAL && m_op_type <= COMPARE_NOT_EQUAL) {
        const std::string&& lhs_val = LHS()->Eval(context);
        const std::string&& rhs_val = RHS()->Eval(context);
        bool test_result = false;
        switch (m_op_type) {
            case COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
            case COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
            case COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
            case COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
            case COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
            case COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
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
        case PLUS:
            return LHS()->Eval(context) + RHS()->Eval(context); break;

        case MINUS:
            return LHS()->Eval(context) - RHS()->Eval(context); break;

        case TIMES: {
            double op1 = LHS()->Eval(context);
            if (op1 == 0.0)
                return 0.0;
            return op1 * RHS()->Eval(context);
            break;
        }

        case DIVIDE: {
            double op2 = RHS()->Eval(context);
            if (op2 == 0.0)
                return 0.0;
            return LHS()->Eval(context) / op2;
            break;
        }

        case NEGATE:
            return -(LHS()->Eval(context)); break;

        case EXPONENTIATE: {
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

        case ABS:
            return std::abs(LHS()->Eval(context)); break;

        case LOGARITHM: {
            double op1 = LHS()->Eval(context);
            if (op1 <= 0.0)
                return 0.0;
            return std::log(op1);
            break;
        }

        case SINE:
            return std::sin(LHS()->Eval(context)); break;

        case COSINE:
            return std::cos(LHS()->Eval(context)); break;

        case MINIMUM:
        case MAXIMUM: {
            std::set<double> vals;
            for (auto& vr : m_operands) {
                if (vr)
                    vals.insert(vr->Eval(context));
            }
            if (m_op_type == MINIMUM)
                return vals.empty() ? 0.0 : *vals.begin();
            else
                return vals.empty() ? 0.0 : *vals.rbegin();
            break;
        }

        case RANDOM_UNIFORM: {
            double op1 = LHS()->Eval(context);
            double op2 = RHS()->Eval(context);
            double min_val = std::min(op1, op2);
            double max_val = std::max(op1, op2);
            return RandDouble(min_val, max_val);
            break;
        }

        case RANDOM_PICK: {
            // select one operand, evaluate it, return result
            if (m_operands.empty())
                return 0.0;
            unsigned int idx = RandSmallInt(0, m_operands.size() - 1);
            auto& vr = *std::next(m_operands.begin(), idx);
            if (!vr)
                return 0.0;
            return vr->Eval(context);
            break;
        }

        case COMPARE_EQUAL:
        case COMPARE_GREATER_THAN:
        case COMPARE_GREATER_THAN_OR_EQUAL:
        case COMPARE_LESS_THAN:
        case COMPARE_LESS_THAN_OR_EQUAL:
        case COMPARE_NOT_EQUAL: {
            const double&& lhs_val = LHS()->Eval(context);
            const double&& rhs_val = RHS()->Eval(context);
            bool test_result = false;
            switch (m_op_type) {
                case COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
                case COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
                case COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
                case COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
                case COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
                case COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
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
        }

        case ROUND_NEAREST:
            return std::round(LHS()->Eval(context)); break;
        case ROUND_UP:
            return std::ceil(LHS()->Eval(context)); break;
        case ROUND_DOWN:
            return std::floor(LHS()->Eval(context)); break;

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
        case PLUS:
            return LHS()->Eval(context) + RHS()->Eval(context);     break;

        case MINUS:
            return LHS()->Eval(context) - RHS()->Eval(context);     break;

        case TIMES: {
            double op1 = LHS()->Eval(context);
            if (op1 == 0)
                return 0;
            return op1 * RHS()->Eval(context);
            break;
        }

        case DIVIDE: {
            int op2 = RHS()->Eval(context);
            if (op2 == 0)
                return 0;
            return LHS()->Eval(context) / op2;
            break;
        }

        case NEGATE:
            return -LHS()->Eval(context); break;

        case EXPONENTIATE: {
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

        case ABS: {
            return static_cast<int>(std::abs(LHS()->Eval(context)));
            break;
        }

        case LOGARITHM: {
            double op1 = LHS()->Eval(context);
            if (op1 <= 0.0)
                return 0;
            return static_cast<int>(std::log(op1));
            break;
        }

        case SINE: {
            double op1 = LHS()->Eval(context);
            return static_cast<int>(std::sin(op1));
            break;
        }

        case COSINE: {
            double op1 = LHS()->Eval(context);
            return static_cast<int>(std::cos(op1));
            break;
        }

        case MINIMUM:
        case MAXIMUM: {
            std::set<int> vals;
            for (auto& vr : m_operands) {
                if (vr)
                    vals.insert(vr->Eval(context));
            }
            if (m_op_type == MINIMUM)
                return vals.empty() ? 0 : *vals.begin();
            else
                return vals.empty() ? 0 : *vals.rbegin();
            break;
        }

        case RANDOM_UNIFORM: {
            double op1 = LHS()->Eval(context);
            double op2 = RHS()->Eval(context);
            int min_val = static_cast<int>(std::min(op1, op2));
            int max_val = static_cast<int>(std::max(op1, op2));
            return RandInt(min_val, max_val);
            break;
        }

        case RANDOM_PICK: {
            // select one operand, evaluate it, return result
            if (m_operands.empty())
                return 0;
            unsigned int idx = RandSmallInt(0, m_operands.size() - 1);
            auto& vr = *std::next(m_operands.begin(), idx);
            if (!vr)
                return 0;
            return vr->Eval(context);
            break;
        }

        case ROUND_NEAREST:
        case ROUND_UP:
        case ROUND_DOWN: {
            // integers don't need to be rounded...
            return LHS()->Eval(context);
            break;
        }

        case COMPARE_EQUAL:
        case COMPARE_GREATER_THAN:
        case COMPARE_GREATER_THAN_OR_EQUAL:
        case COMPARE_LESS_THAN:
        case COMPARE_LESS_THAN_OR_EQUAL:
        case COMPARE_NOT_EQUAL: {
            const int&& lhs_val = LHS()->Eval(context);
            const int&& rhs_val = RHS()->Eval(context);
            bool test_result = false;
            switch (m_op_type) {
                case COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
                case COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
                case COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
                case COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
                case COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
                case COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
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
        }

        default:    break;
    }

    throw std::runtime_error("double ValueRef evaluated with an unknown or invalid OpType.");
    return 0;
}
} // namespace ValueRef
