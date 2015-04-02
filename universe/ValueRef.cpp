#include "ValueRef.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Planet.h"
#include "Predicates.h"
#include "Species.h"
#include "System.h"
#include "Field.h"
#include "Universe.h"
#include "UniverseObject.h"
#include "Condition.h"
#include "Enums.h"
#include "Tech.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/Random.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>


std::string DoubleToString(double val, int digits, bool always_show_sign);
bool UserStringExists(const std::string& str);

namespace {
    TemporaryPtr<const UniverseObject> FollowReference(std::vector<std::string>::const_iterator first,
                                                       std::vector<std::string>::const_iterator last,
                                                       ValueRef::ReferenceType ref_type,
                                                       const ScriptingContext& context)
    {
        //DebugLogger() << "FollowReference: source: " << (context.source ? context.source->Name() : "0")
        //    << " target: " << (context.effect_target ? context.effect_target->Name() : "0")
        //    << " local c: " << (context.condition_local_candidate ? context.condition_local_candidate->Name() : "0")
        //    << " root c: " << (context.condition_root_candidate ? context.condition_root_candidate->Name() : "0");

        TemporaryPtr<const UniverseObject> obj;
        switch(ref_type) {
        case ValueRef::NON_OBJECT_REFERENCE:                    return context.condition_local_candidate;   break;
        case ValueRef::SOURCE_REFERENCE:                        obj = context.source;                       break;
        case ValueRef::EFFECT_TARGET_REFERENCE:                 obj = context.effect_target;                break;
        case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:      obj = context.condition_root_candidate;     break;
        case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE:
        default:                                                obj = context.condition_local_candidate;    break;
        }

        while (first != last) {
            std::string property_name = *first;
            if (property_name == "Planet") {
                if (TemporaryPtr<const Building> b = boost::dynamic_pointer_cast<const Building>(obj))
                    obj = GetPlanet(b->PlanetID());
                else
                    obj = TemporaryPtr<const UniverseObject>();
            } else if (property_name == "System") {
                if (obj)
                    obj = GetSystem(obj->SystemID());
            } else if (property_name == "Fleet") {
                if (TemporaryPtr<const Ship> s = boost::dynamic_pointer_cast<const Ship>(obj))
                    obj = GetFleet(s->FleetID());
                else
                    obj = TemporaryPtr<const UniverseObject>();
            }
            ++first;
        }
        return obj;
    }

    // Generates a debug  trace that can be included in error logs, augmenting the ReconstructName() info with
    // additional info identifying the object references that were successfully followed.
    std::string TraceReference(const std::vector<std::string>& property_name, ValueRef::ReferenceType ref_type,
                                                       const ScriptingContext& context)
    {
        TemporaryPtr<const UniverseObject> obj;
        std::string retval = ReconstructName(property_name, ref_type) + " :  ";
        switch(ref_type) {
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
            retval += boost::lexical_cast<std::string>(obj->ObjectType()) + " " + boost::lexical_cast<std::string>(obj->ID()) + " ( " + obj->Name() + " ) ";
        }
        retval += " | ";

        std::vector<std::string>::const_iterator first = property_name.begin();
        std::vector<std::string>::const_iterator last = property_name.end();
        while (first != last) {
            std::string property_name = *first;
            retval += " " + property_name;
            if (property_name == "Planet") {
                if (TemporaryPtr<const Building> b = boost::dynamic_pointer_cast<const Building>(obj)) {
                    retval += " (" + boost::lexical_cast<std::string>(b->PlanetID()) + "): ";
                    obj = GetPlanet(b->PlanetID());
                } else
                    obj = TemporaryPtr<const UniverseObject>();
            } else if (property_name == "System") {
                if (obj) {
                    retval += " (" + boost::lexical_cast<std::string>(obj->SystemID()) + "): ";
                    obj = GetSystem(obj->SystemID());
                }
            } else if (property_name == "Fleet") {
                if (TemporaryPtr<const Ship> s = boost::dynamic_pointer_cast<const Ship>(obj))  {
                    retval += " (" + boost::lexical_cast<std::string>(s->FleetID()) + "): ";
                    obj = GetFleet(s->FleetID());
                } else
                    obj = TemporaryPtr<const UniverseObject>();
            }
            ++first;
            if (obj) {
                retval += boost::lexical_cast<std::string>(obj->ObjectType()) + " " + boost::lexical_cast<std::string>(obj->ID()) + " ( " + obj->Name() + " )";
            }
            retval += " | ";
        }
        return retval;
    }

    struct ObjectTypeVisitor : UniverseObjectVisitor {
        ObjectTypeVisitor() : m_type(INVALID_UNIVERSE_OBJECT_TYPE) {}

        virtual TemporaryPtr<UniverseObject> Visit(TemporaryPtr<Building> obj) const
        { m_type = OBJ_BUILDING; return obj; }
        virtual TemporaryPtr<UniverseObject> Visit(TemporaryPtr<Fleet> obj) const
        { m_type = OBJ_FLEET; return obj; }
        virtual TemporaryPtr<UniverseObject> Visit(TemporaryPtr<Planet> obj) const
        { m_type = OBJ_PLANET; return obj; }
        virtual TemporaryPtr<UniverseObject> Visit(TemporaryPtr<Ship> obj) const
        { m_type = OBJ_SHIP; return obj; }
        virtual TemporaryPtr<UniverseObject> Visit(TemporaryPtr<System> obj) const
        { m_type = OBJ_SYSTEM; return obj; }
        virtual TemporaryPtr<UniverseObject> Visit(TemporaryPtr<Field> obj) const
        { m_type = OBJ_FIELD; return obj; }

        mutable UniverseObjectType m_type;
    };
}

namespace {
    const std::map<std::string, MeterType>& GetMeterNameMap() {
        static std::map<std::string, MeterType> meter_name_map;
        if (meter_name_map.empty()) {
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
            meter_name_map["Stealth"] =            METER_STEALTH;
            meter_name_map["Detection"] =          METER_DETECTION;
            meter_name_map["Speed"] =              METER_SPEED;
            meter_name_map["Damage"] =             METER_DAMAGE;
            meter_name_map["Capacity"] =           METER_CAPACITY;
            meter_name_map["Size"] =               METER_SIZE;
        }
        return meter_name_map;
    }
}

MeterType ValueRef::NameToMeter(const std::string& name) {
    MeterType retval = INVALID_METER_TYPE;
    std::map<std::string, MeterType>::const_iterator it = GetMeterNameMap().find(name);
    if (it != GetMeterNameMap().end())
        retval = it->second;
    return retval;
}

std::string ValueRef::MeterToName(MeterType meter) {
    for (std::map<std::string, MeterType>::const_iterator it = GetMeterNameMap().begin();
         it != GetMeterNameMap().end(); ++it)
    {
        if (it->second == meter)
            return it->first;
    }
    return "";
}

std::string ValueRef::ReconstructName(const std::vector<std::string>& property_name,
                                      ValueRef::ReferenceType ref_type)
{
    std::string retval;
    switch (ref_type) {
    case ValueRef::SOURCE_REFERENCE:                    retval = "Source";          break;
    case ValueRef::EFFECT_TARGET_REFERENCE:             retval = "Target";          break;
    case ValueRef::EFFECT_TARGET_VALUE_REFERENCE:       retval = "Value";           break;
    case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE: retval = "LocalCandidate";  break;
    case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:  retval = "RootCandidate";   break;
    case ValueRef::NON_OBJECT_REFERENCE:                retval = "";                break;
    default:                                            retval = "?????";           break;
    }

    for (std::size_t i = 0; i < property_name.size(); ++i) {
        if (!retval.empty())
            retval += '.';
        retval += property_name[i].c_str();
    }
    return retval;
}

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    std::string Constant<int>::Description() const
    {
        if (std::abs(m_value) < 1000)
            return boost::lexical_cast<std::string>(m_value);
        return DoubleToString(m_value, 3, false);
    }

    template <>
    std::string Constant<double>::Description() const
    { return DoubleToString(m_value, 3, false); }

    template <>
    std::string Constant<std::string>::Description() const
    { return m_value; }

    template <>
    std::string Constant<PlanetSize>::Dump() const
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
    std::string Constant<PlanetType>::Dump() const
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
    std::string Constant<PlanetEnvironment>::Dump() const
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
    std::string Constant<UniverseObjectType>::Dump() const
    {
        switch (m_value) {
        case OBJ_BUILDING:      return "Building";
        case OBJ_SHIP:          return "Ship";
        case OBJ_FLEET:         return "Fleet"; 
        case OBJ_PLANET:        return "Planet";
        case OBJ_POP_CENTER:    return "PopulationCenter";
        case OBJ_PROD_CENTER:   return "ProductionCenter";
        case OBJ_SYSTEM:        return "System";
        default:                return "?";
        }
    }

    template <>
    std::string Constant<StarType>::Dump() const
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
    std::string Constant<int>::Dump() const
    { return Description(); }

    template <>
    std::string Constant<double>::Dump() const
    { return Description(); }

    template <>
    std::string Constant<std::string>::Dump() const
    { return "\"" + Description() + "\""; }
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
std::string FormatedDescriptionPropertyNames(ValueRef::ReferenceType ref_type,
                                             const std::vector<std::string>& property_names)
{
    int num_references = property_names.size();
    if (ref_type == ValueRef::NON_OBJECT_REFERENCE)
        num_references--;
    for (unsigned int i = 0; i < property_names.size(); ++i)
        if (property_names[i].empty())
             num_references--;
    num_references = std::max(0, num_references);
    std::string names_size = boost::lexical_cast<std::string>(num_references);
    boost::format formatter = FlexibleFormat(UserString("DESC_VALUE_REF_MULTIPART_VARIABLE" +
                                             names_size));

    switch (ref_type) {
    case ValueRef::SOURCE_REFERENCE:                    formatter % UserString("DESC_VAR_SOURCE");          break;
    case ValueRef::EFFECT_TARGET_REFERENCE:             formatter % UserString("DESC_VAR_TARGET");          break;
    case ValueRef::EFFECT_TARGET_VALUE_REFERENCE:       formatter % UserString("DESC_VAR_VALUE");           break;
    case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE: formatter % UserString("DESC_VAR_LOCAL_CANDIDATE"); break;
    case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:  formatter % UserString("DESC_VAR_ROOT_CANDIDATE");  break;
    case ValueRef::NON_OBJECT_REFERENCE:                                                                    break;
    default:                                            formatter % "???";                                  break;
    }

    for (unsigned int i = 0; i < property_names.size(); ++i) {
        if (property_names[i].empty())  // apparently is empty for a ValueRef::EFFECT_TARGET_VALUE_REFERENCE
            continue;
        std::string property_string_temp(std::string(property_names[i].c_str()));
        std::string stringtable_key("DESC_VAR_" + boost::to_upper_copy(property_string_temp));
        formatter % UserString(stringtable_key);
        //std::string blag = property_string_temp + "  " +
        //                   stringtable_key + "  " +
        //                   UserString(stringtable_key);
        //std::cerr << blag << std::endl << std::endl;
    }

    std::string retval = boost::io::str(formatter);
    //std::cerr << "ret: " << retval << std::endl;
    return retval;
}

namespace ValueRef {

#define IF_CURRENT_VALUE(T)                                                \
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
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(PlanetSize)

        TemporaryPtr<const UniverseObject> object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<PlanetSize>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return INVALID_PLANET_SIZE;
        }

        if (TemporaryPtr<const Planet> p = boost::dynamic_pointer_cast<const Planet>(object)) {
            if (property_name == "PlanetSize")
                return p->Size();
            else if (property_name == "NextLargerPlanetSize")
                return p->NextLargerPlanetSize();
            else if (property_name == "NextSmallerPlanetSize")
                return p->NextSmallerPlanetSize();
        }

        ErrorLogger() << "Variable<PlanetSize>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
        return INVALID_PLANET_SIZE;
    }

    template <>
    PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(PlanetType)

        TemporaryPtr<const UniverseObject> object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<PlanetType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return INVALID_PLANET_TYPE;
        }

        if (TemporaryPtr<const Planet> p = boost::dynamic_pointer_cast<const Planet>(object)) {
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
        return INVALID_PLANET_TYPE;
    }

    template <>
    PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(PlanetEnvironment)

        if (property_name == "PlanetEnvironment") {
            TemporaryPtr<const UniverseObject> object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
            if (!object) {
                ErrorLogger() << "Variable<PlanetEnvironment>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
                return INVALID_PLANET_ENVIRONMENT;
            }
            if (TemporaryPtr<const Planet> p = boost::dynamic_pointer_cast<const Planet>(object))
                return p->EnvironmentForSpecies();
        }

        ErrorLogger() << "Variable<PlanetEnvironment>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
        return INVALID_PLANET_ENVIRONMENT;
    }

    template <>
    UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(UniverseObjectType)

        if (property_name == "ObjectType") {
            TemporaryPtr<const UniverseObject> object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
            if (!object) {
                ErrorLogger() << "Variable<UniverseObjectType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
                return INVALID_UNIVERSE_OBJECT_TYPE;
            }
            ObjectTypeVisitor v;
            if (object->Accept(v))
                return v.m_type;
            else if (boost::dynamic_pointer_cast<const PopCenter>(object))
                return OBJ_POP_CENTER;
            else if (boost::dynamic_pointer_cast<const ResourceCenter>(object))
                return OBJ_PROD_CENTER;
        }

        ErrorLogger() << "Variable<UniverseObjectType>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
        return INVALID_UNIVERSE_OBJECT_TYPE;
    }

    template <>
    StarType Variable<StarType>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(StarType)

        TemporaryPtr<const UniverseObject> object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<StarType>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return INVALID_STAR_TYPE;
        }

        if (TemporaryPtr<const System> s = boost::dynamic_pointer_cast<const System>(object)) {
            if (property_name == "StarType")
                return s->GetStarType();
            else if (property_name == "NextOlderStarType")
                return s->NextOlderStarType();
            else if (property_name == "NextYoungerStarType")
                return s->NextYoungerStarType();
        }

        ErrorLogger() << "Variable<StarType>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
        return INVALID_STAR_TYPE;
    }

    template <>
    double Variable<double>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(float)

        if (m_ref_type == ValueRef::NON_OBJECT_REFERENCE) {
            if (property_name == "UniverseCentreX" |
                property_name == "UniverseCentreY")
            {
                return GetUniverse().UniverseWidth() / 2;
            }

            // add more non-object reference double functions here
            ErrorLogger() << "Variable<double>::Eval unrecognized non-object property: " << TraceReference(m_property_name, m_ref_type, context);
            return 0.0;
        }

        TemporaryPtr<const UniverseObject> object =
            FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<double>::Eval unable to follow reference: "
                                   << TraceReference(m_property_name, m_ref_type, context);
            return 0.0;
        }

        MeterType meter_type = NameToMeter(property_name);
        if (object && meter_type != INVALID_METER_TYPE) {
            if (object->GetMeter(meter_type))
                return object->InitialMeterValue(meter_type);

        } else if (property_name == "TradeStockpile") {
            if (const Empire* empire = GetEmpire(object->Owner()))
                return empire->ResourceStockpile(RE_TRADE);

        } else if (property_name == "X") {
            return object->X();

        } else if (property_name == "Y") {
            return object->Y();

        } else if (property_name == "SizeAsDouble") {
            if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object))
                return planet->SizeAsInt();

        } else if (property_name == "DistanceFromOriginalType") {
            if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object))
                return planet->DistanceFromOriginalType();

        } else if (property_name == "NextTurnPopGrowth") {
            if (TemporaryPtr<const PopCenter> pop = boost::dynamic_pointer_cast<const PopCenter>(object))
                return pop->NextTurnPopGrowth();

        } else if (property_name == "CurrentTurn") {
            return CurrentTurn();

        }

        ErrorLogger() << "Variable<double>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
        return 0.0;
    }

    template <>
    int Variable<int>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(int)

        if (m_ref_type == ValueRef::NON_OBJECT_REFERENCE) {
            if (property_name == "CurrentTurn")
                return CurrentTurn();
            if (property_name == "GalaxySize")
                return GetGalaxySetupData().m_size;
            if (property_name == "GalaxyShape")
                return static_cast<int>(GetGalaxySetupData().m_shape);
            if (property_name == "GalaxyAge")
                return static_cast<int>(GetGalaxySetupData().m_age);
            if (property_name == "GalaxyStarlaneFrequency")
                return static_cast<int>(GetGalaxySetupData().m_starlane_freq);
            if (property_name == "GalaxyPlanetDensity")
                return static_cast<int>(GetGalaxySetupData().m_planet_density);
            if (property_name == "GalaxySpecialFrequency")
                return static_cast<int>(GetGalaxySetupData().m_specials_freq);
            if (property_name == "GalaxyMonsterFrequency")
                return static_cast<int>(GetGalaxySetupData().m_monster_freq);
            if (property_name == "GalaxyNativeFrequency")
                return static_cast<int>(GetGalaxySetupData().m_native_freq);
            if (property_name == "GalaxyMaxAIAggression")
                return static_cast<int>(GetGalaxySetupData().m_ai_aggr);

            // add more non-object reference int functions here
            ErrorLogger() << "Variable<int>::Eval unrecognized non-object property: " << TraceReference(m_property_name, m_ref_type, context);
            return 0;
        }

        TemporaryPtr<const UniverseObject> object =
            FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<int>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
            return 0;
        }

        if (property_name == "Owner") {
            return object->Owner();
        } else if (property_name == "ID") {
            return object->ID();
        } else if (property_name == "CreationTurn") {
            return object->CreationTurn();
        } else if (property_name == "Age") {
            return object->AgeInTurns();
        } else if (property_name == "TurnsSinceFocusChange") {
            if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object))
                return planet->TurnsSinceFocusChange();
            else
                return 0;
        } else if (property_name == "ProducedByEmpireID") {
            if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(object))
                return ship->ProducedByEmpireID();
            else if (TemporaryPtr<const Building> building = boost::dynamic_pointer_cast<const Building>(object))
                return building->ProducedByEmpireID();
            else
                return ALL_EMPIRES;
        } else if (property_name == "DesignID") {
            if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(object))
                return ship->DesignID();
            else
                return ShipDesign::INVALID_DESIGN_ID;
        } else if (property_name == "Species") {
            if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object))
                return GetSpeciesManager().GetSpeciesID(planet->SpeciesName());
            else if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(object))
                return GetSpeciesManager().GetSpeciesID(ship->SpeciesName());
            else
                return -1;
        } else if (property_name == "FleetID") {
            if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(object))
                return ship->FleetID();
            else if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(object))
                return fleet->ID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "PlanetID") {
            if (TemporaryPtr<const Building> building = boost::dynamic_pointer_cast<const Building>(object))
                return building->PlanetID();
            else if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object))
                return planet->ID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "SystemID") {
            return object->SystemID();
        } else if (property_name == "FinalDestinationID") {
            if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(object))
                return fleet->FinalDestinationID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "NextSystemID") {
            if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(object))
                return fleet->NextSystemID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "PreviousSystemID") {
            if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(object))
                return fleet->PreviousSystemID();
            else
                return INVALID_OBJECT_ID;

        } else if (property_name == "NearestSystemID") {
            if (object->SystemID() != INVALID_OBJECT_ID)
                return object->SystemID();

            return GetUniverse().NearestSystemTo(object->X(), object->Y());

        } else if (property_name == "NumShips") {
            if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(object))
                return fleet->NumShips();
            else
                return 0;
        } else if (property_name == "LastTurnBattleHere") {
            if (TemporaryPtr<const System> system = boost::dynamic_pointer_cast<const System>(object))
                return system->LastTurnBattleHere();
            else
                return INVALID_GAME_TURN;
        } else if (property_name == "LastTurnActiveInBattle") {
            if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(object))
                return ship->LastTurnActiveInCombat();
            else
                return INVALID_GAME_TURN;
        } else if (property_name == "Orbit") {
            if (TemporaryPtr<const System> system = GetSystem(object->SystemID()))
                return system->OrbitOfPlanet(object->ID());
            return -1;
        }

        ErrorLogger() << "Variable<int>::Eval unrecognized object property: " << TraceReference(m_property_name, m_ref_type, context);
        return 0;
    }

    template <>
    std::string Variable<std::string>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(std::string)

        if (m_ref_type == ValueRef::NON_OBJECT_REFERENCE) {
            if (property_name == "GalaxySeed")
                return GetGalaxySetupData().m_seed;

            ErrorLogger() << "Variable<std::string>::Eval unrecognized non-object property: " << TraceReference(m_property_name, m_ref_type, context);
            return "";
        }

        TemporaryPtr<const UniverseObject> object =
            FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            ErrorLogger() << "Variable<std::string>::Eval unable to follow reference: " << TraceReference(m_property_name, m_ref_type, context);
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
            if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object))
                return planet->SpeciesName();
            else if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(object))
                return ship->SpeciesName();

        } else if (property_name == "BuildingType") {
            if (TemporaryPtr<const Building> building = boost::dynamic_pointer_cast<const Building>(object))
                return building->BuildingTypeName();

        } else if (property_name == "Focus") {
            if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object))
                return planet->Focus();

        } else if (property_name == "PreferredFocus") {
            const Species* species = 0;
            if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(object)) {
                species = GetSpecies(planet->SpeciesName());
            } else if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(object)) {
                species = GetSpecies(ship->SpeciesName());
            }
            if (species)
                return species->PreferredFocus();
            return "";

        } else if (property_name == "OwnerMostExpensiveEnqueuedTech") {
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
        return "";
    }

#undef IF_CURRENT_VALUE
}

///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    double Statistic<double>::Eval(const ScriptingContext& context) const
    {
        Condition::ObjectSet condition_matches;
        GetConditionMatches(context, condition_matches, m_sampling_condition);

        // these two statistic types don't depend on the object property values,
        // so can be evaluated without getting those values.
        if (m_stat_type == COUNT)
            return static_cast<double>(condition_matches.size());
        if (m_stat_type == IF)
            return condition_matches.empty() ? 0.0 : 1.0;

        // evaluate property for each condition-matched object
        std::map<TemporaryPtr<const UniverseObject>, double> object_property_values;
        GetObjectPropertyValues(context, condition_matches, object_property_values);

        return ReduceData(object_property_values);
    }

    template <>
    int Statistic<int>::Eval(const ScriptingContext& context) const
    {
        Condition::ObjectSet condition_matches;
        GetConditionMatches(context, condition_matches, m_sampling_condition);

        // these two statistic types don't depend on the object property values,
        // so can be evaluated without getting those values.
        if (m_stat_type == COUNT)
            return static_cast<int>(condition_matches.size());
        if (m_stat_type == IF)
            return condition_matches.empty() ? 0 : 1;

        // evaluate property for each condition-matched object
        std::map<TemporaryPtr<const UniverseObject>, int> object_property_values;
        GetObjectPropertyValues(context, condition_matches, object_property_values);

        return ReduceData(object_property_values);
    }

    template <>
    std::string Statistic<std::string>::Eval(const ScriptingContext& context) const
    {
        // the only statistic that can be computed on non-number property types
        // and that is itself of a non-number type is the most common value
        if (m_stat_type != MODE)
            throw std::runtime_error("ValueRef evaluated with an invalid StatisticType for the return type (string).");

        Condition::ObjectSet condition_matches;
        GetConditionMatches(context, condition_matches, m_sampling_condition);

        if (condition_matches.empty())
            return "";

        // evaluate property for each condition-matched object
        std::map<TemporaryPtr<const UniverseObject>, std::string> object_property_values;
        GetObjectPropertyValues(context, condition_matches, object_property_values);

        // count number of each result, tracking which has the most occurances
        std::map<std::string, unsigned int> histogram;
        std::map<std::string, unsigned int>::const_iterator most_common_property_value_it = histogram.begin();
        unsigned int max_seen(0);

        for (std::map<TemporaryPtr<const UniverseObject>, std::string>::const_iterator it = object_property_values.begin();
             it != object_property_values.end(); ++it)
        {
            const std::string& property_value = it->second;

            std::map<std::string, unsigned int>::iterator hist_it = histogram.find(property_value);
            if (hist_it == histogram.end())
                hist_it = histogram.insert(std::make_pair(property_value, 0)).first;
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
}

///////////////////////////////////////////////////////////
// ComplexVariable                                       //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    PlanetSize ComplexVariable<PlanetSize>::Eval(const ScriptingContext& context) const
    { return INVALID_PLANET_SIZE; }

    template <>
    PlanetType ComplexVariable<PlanetType>::Eval(const ScriptingContext& context) const
    { return INVALID_PLANET_TYPE; }

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
    double ComplexVariable<double>::Eval(const ScriptingContext& context) const
    {
        const std::string& variable_name = m_property_name.back();

        if (variable_name == "HullFuel") {
            std::string hull_type_name;
            if (m_string_ref1)
                hull_type_name = m_string_ref1->Eval(context);

            const HullType* hull_type = GetHullType(hull_type_name);
            if (!hull_type)
                return 0.0;

            return hull_type->Fuel();

        } else if (variable_name == "HullStealth") {
            std::string hull_type_name;
            if (m_string_ref1)
                hull_type_name = m_string_ref1->Eval(context);

            const HullType* hull_type = GetHullType(hull_type_name);
            if (!hull_type)
                return 0.0;

            return hull_type->Stealth();

        } else if (variable_name == "HullStructure") {
            std::string hull_type_name;
            if (m_string_ref1)
                hull_type_name = m_string_ref1->Eval(context);

            const HullType* hull_type = GetHullType(hull_type_name);
            if (!hull_type)
                return 0.0f;

            return hull_type->Structure();

        } else if (variable_name == "HullSpeed") {
            std::string hull_type_name;
            if (m_string_ref1)
                hull_type_name = m_string_ref1->Eval(context);

            const HullType* hull_type = GetHullType(hull_type_name);
            if (!hull_type)
                return 0.0;

            return hull_type->Speed();

        } else if (variable_name == "PartCapacity") {
            std::string part_type_name;
            if (m_string_ref1)
                part_type_name = m_string_ref1->Eval(context);

            const PartType* part_type = GetPartType(part_type_name);
            if (!part_type)
                return 0.0;

            return part_type->Capacity();

        } else if (variable_name == "DirectDistanceBetween") {
            int object1_id = INVALID_OBJECT_ID;
            if (m_int_ref1)
                object1_id = m_int_ref1->Eval(context);
            TemporaryPtr<const UniverseObject> obj1 = GetUniverseObject(object1_id);
            if (!obj1)
                return 0.0;

            int object2_id = INVALID_OBJECT_ID;
            if (m_int_ref2)
                object2_id = m_int_ref2->Eval(context);
            TemporaryPtr<const UniverseObject> obj2 = GetUniverseObject(object2_id);
            if (!obj2)
                return 0.0;

            double dx = obj2->X() - obj1->X();
            double dy = obj2->Y() - obj1->Y();
            return static_cast<float>(std::sqrt(dx*dx + dy*dy));

        } else if (variable_name == "SpeciesEmpireOpinion") {
            int empire_id = ALL_EMPIRES;
            if (m_int_ref1)
                empire_id = m_int_ref1->Eval(context);

            std::string species_name;
            if (m_string_ref1)
                species_name = m_string_ref1->Eval(context);

            return GetSpeciesManager().SpeciesEmpireOpinion(species_name, empire_id);

        } else if (variable_name == "SpeciesSpeciesOpinion") {
            std::string opinionated_species_name;
            if (m_string_ref1)
                opinionated_species_name = m_string_ref1->Eval(context);

            std::string rated_species_name;
            if (m_string_ref2)
                rated_species_name = m_string_ref2->Eval(context);

            return GetSpeciesManager().SpeciesSpeciesOpinion(opinionated_species_name, rated_species_name);
        }

        return 0.0;
    }

    namespace {
        static std::map<std::string, int> EMPTY_STRING_INT_MAP;
        static std::map<int, int> EMPTY_INT_INT_MAP;

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

        int GetEmpirePropertyNoKeyImpl(int empire_id, const std::string& parsed_property_name) {
            Empire* empire = GetEmpire(empire_id);
            if (!empire)
                return 0;

            if (parsed_property_name == "OutpostsOwned")
                return empire->OutpostsOwned();
            // todo: add all the various OwnerWhatever ValueRef stuff here

            return 0;
        }

        // gets property for a particular map key string for one or all empires
        int GetEmpirePropertySingleKey(int empire_id, const std::string& parsed_property_name,
                                       const std::string& map_key)
        {
            int sum = 0;
            if (map_key.empty())
                return sum;

            // single empire
            if (empire_id != ALL_EMPIRES) {
                const std::map<std::string, int>& map = GetEmpireStringIntMap(empire_id, parsed_property_name);
                std::map<std::string, int>::const_iterator it = map.find(map_key);
                if (it == map.end())
                    return 0;
                return it->second;
            }

            // all empires summed
            for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
                const std::map<std::string, int>& map = GetEmpireStringIntMap(it->first, parsed_property_name);
                std::map<std::string, int>::const_iterator map_it = map.find(map_key);
                if (map_it != map.end())
                    sum += map_it->second;
            }
            return sum;
        }

        // gets property for the sum of all map keys for one or all empires
        int GetEmpirePropertySumAllStringKeys(int empire_id, const std::string& parsed_property_name) {
            int sum = 0;

            // single empire
            if (empire_id != ALL_EMPIRES) {
                // sum of all key entries for this empire
                const std::map<std::string, int>& map = GetEmpireStringIntMap(empire_id, parsed_property_name);
                for (std::map<std::string, int>::const_iterator it = map.begin(); it != map.end(); ++it)
                    sum += it->second;
                return sum;
            }

            // all empires summed
            for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
                const std::map<std::string, int>& map = GetEmpireStringIntMap(it->first, parsed_property_name);
                for (std::map<std::string, int>::const_iterator it = map.begin(); it != map.end(); ++it)
                    sum += it->second;
            }
            return sum;
        }

        // gets property for a particular map key int for one or all empires
        int GetEmpirePropertySingleKey(int empire_id, const std::string& parsed_property_name,
                                       int map_key)
        {
            int sum = 0;

            // single empire
            if (empire_id != ALL_EMPIRES) {
                const std::map<int, int>& map = GetEmpireIntIntMap(empire_id, parsed_property_name);
                std::map<int, int>::const_iterator it = map.find(map_key);
                if (it == map.end())
                    return 0;
                return it->second;
            }

            // all empires summed
            for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
                const std::map<int, int>& map = GetEmpireIntIntMap(it->first, parsed_property_name);
                std::map<int, int>::const_iterator map_it = map.find(map_key);
                if (map_it != map.end())
                    sum += map_it->second;
            }
            return sum;
        }

        // gets property for the sum of all map keys for one or all empires
        int GetEmpirePropertySumAllIntKeys(int empire_id, const std::string& parsed_property_name) {
            int sum = 0;

            // single empire
            if (empire_id != ALL_EMPIRES) {
                // sum of all key entries for this empire
                const std::map<int, int>& map = GetEmpireIntIntMap(empire_id, parsed_property_name);
                for (std::map<int, int>::const_iterator it = map.begin(); it != map.end(); ++it)
                    sum += it->second;
                return sum;
            }

            // all empires summed
            for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
                const std::map<int, int>& map = GetEmpireIntIntMap(it->first, parsed_property_name);
                for (std::map<int, int>::const_iterator it = map.begin(); it != map.end(); ++it)
                    sum += it->second;
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
        int GetEmpirePropertyNoKey(int empire_id, const std::string& parsed_property_name) {
            int sum = 0;

            if (empire_id != ALL_EMPIRES) {
                // single empire
                return GetEmpirePropertyNoKeyImpl(empire_id, parsed_property_name);
            }

            // all empires summed
            for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it)
                sum += GetEmpirePropertyNoKeyImpl(it->first, parsed_property_name);
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
            variable_name == "SpeciesShipsDestroyed" ||
            variable_name == "SpeciesColoniesOwned" ||
            variable_name == "SpeciesPlanetsBombed" ||
            variable_name == "SpeciesPlanetsDepoped" ||
            variable_name == "SpeciesPlanetsOwned" ||
            variable_name == "SpeciesPlanetsInvaded" ||
            variable_name == "SpeciesShipsDestroyed" ||
            variable_name == "SpeciesShipsLost" ||
            variable_name == "SpeciesShipsOwned" ||
            variable_name == "SpeciesShipsProduced" ||
            variable_name == "SpeciesShipsScrapped")
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
            }

            // if a string specified, get just that entry (for single empire, or
            // summed for all empires)
            if (m_string_ref1)
                return GetEmpirePropertySingleKey(empire_id, variable_name, key_string);

            // if no string specified, get sum of all entries (for single empire
            // or summed for all empires)
            return GetEmpirePropertySumAllStringKeys(empire_id, variable_name);
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
                return GetEmpirePropertySingleKey(empire_id, variable_name, key_int);
            }

            // although indexed by integers, some of these may be specified by a
            // string that needs to be looked up. if a key string specified, get
            // just that entry (for single empire or sum of all empires)
            if (m_string_ref1) {
                std::string key_string = m_string_ref1->Eval(context);
                if (key_string.empty())
                    return 0;
                int key_int = GetIntKeyFromContentStringKey(variable_name, key_string);
                return GetEmpirePropertySingleKey(empire_id, variable_name, key_int);
            }

            // if no key specified, get sum of all entries (for single empire or sum of all empires)
            return GetEmpirePropertySumAllIntKeys(empire_id, variable_name);
        }

        // unindexed empire proprties
        if (variable_name == "OutpostsOwned") {
            int empire_id = ALL_EMPIRES;
            if (m_int_ref1) {
                empire_id = m_int_ref1->Eval(context);
                if (empire_id == ALL_EMPIRES)
                    return 0;
            }

            return GetEmpirePropertyNoKey(empire_id, variable_name);
        }

        // non-empire properties
        if (variable_name == "PartsInShipDesign") {
            int design_id = ShipDesign::INVALID_DESIGN_ID;
            if (m_int_ref1) {
                design_id = m_int_ref1->Eval(context);
                if (design_id == ShipDesign::INVALID_DESIGN_ID)
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
            const std::vector<std::string>& parts = design->Parts();
            for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
                if (part_type_name.empty() && !it->empty())
                    count++;
                else if (!part_type_name.empty() && part_type_name == *it)
                    count ++;
            }
            return count;

        } else if (variable_name == "PartOfClassInShipDesign") {
            int design_id = ShipDesign::INVALID_DESIGN_ID;
            if (m_int_ref1) {
                design_id = m_int_ref1->Eval(context);
                if (design_id == ShipDesign::INVALID_DESIGN_ID)
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
            const std::vector<std::string>& parts = design->Parts();
            for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
                if (it->empty())
                    continue;
                const PartType* part = GetPartType(*it);
                if (!part)
                    continue;
                if (part->Class() == part_class)
                    count++;
            }
            return count;

        } else if (variable_name == "JumpsBetween") {
            int object1_id = INVALID_OBJECT_ID;
            if (m_int_ref1)
                object1_id = m_int_ref1->Eval(context);

            int object2_id = INVALID_OBJECT_ID;
            if (m_int_ref2)
                object2_id = m_int_ref2->Eval(context);

            int retval = GetUniverse().JumpDistanceBetweenObjects(object1_id, object2_id);
            if (retval == INT_MAX)
                return -1;
            return retval;

        } else if (variable_name == "JumpsBetweenByEmpireSupplyConnections") {
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


            int retval = GetUniverse().JumpDistanceBetweenObjects(object1_id, object2_id/*, empire_id*/);
            if (retval == INT_MAX)
                return -1;
            return retval;
        }

        return 0;
    }

    namespace {
        std::vector<std::string> TechsResearchedByEmpire(int empire_id) {
            std::vector<std::string> retval;
            const Empire* empire = GetEmpire(empire_id);
            if (!empire)
                return retval;
            for (TechManager::iterator tech_it = GetTechManager().begin();
                 tech_it != GetTechManager().end(); ++tech_it)
            {
                if (empire->TechResearched((*tech_it)->Name()))
                    retval.push_back((*tech_it)->Name());
            }
            return retval;
        }

        std::vector<std::string> TechsResearchableByEmpire(int empire_id) {
            std::vector<std::string> retval;
            const Empire* empire = GetEmpire(empire_id);
            if (!empire)
                return retval;
            for (TechManager::iterator tech_it = GetTechManager().begin();
                 tech_it != GetTechManager().end(); ++tech_it)
            {
                if (empire->ResearchableTech((*tech_it)->Name()))
                    retval.push_back((*tech_it)->Name());
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
            std::vector<std::string>::const_iterator tech_it = all_enqueued_techs.begin();
            std::size_t idx = RandSmallInt(0, static_cast<int>(all_enqueued_techs.size()) - 1);
            std::advance(tech_it, idx);
            return *tech_it;

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
            std::vector<std::string>::const_iterator tech_it = researchable_techs.begin();
            std::size_t idx = RandSmallInt(0, static_cast<int>(researchable_techs.size()) - 1);
            std::advance(tech_it, idx);
            return *tech_it;

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
            std::vector<std::string>::const_iterator tech_it = complete_techs.begin();
            std::size_t idx = RandSmallInt(0, static_cast<int>(complete_techs.size()) - 1);
            std::advance(tech_it, idx);
            return *tech_it;

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
            std::vector<std::string>::const_iterator tech_it = sendable_techs.begin();
            std::size_t idx = RandSmallInt(0, static_cast<int>(sendable_techs.size()) - 1);
            std::advance(tech_it, idx);
            return *tech_it;

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
            for (std::vector<std::string>::const_iterator tech_it = sendable_techs.begin();
                 tech_it != sendable_techs.end(); ++tech_it)
            {
                const Tech* tech = GetTech(*tech_it);
                if (!tech)
                    continue;
                float rc = tech->ResearchCost(empire2_id);
                if (rc > highest_cost) {
                    highest_cost = rc;
                    retval = *tech_it;
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
            for (std::vector<std::string>::const_iterator tech_it = sendable_techs.begin();
                 tech_it != sendable_techs.end(); ++tech_it)
            {
                ResearchQueue::const_iterator queue_it = queue.find(*tech_it);
                if (queue_it == queue.end())
                    continue;
                int queue_pos = std::distance(queue.begin(), queue_it);
                if (queue_pos < position_of_top_found_tech) {
                    retval = *tech_it;
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
        return "";
    }

#undef IF_CURRENT_VALUE
}

///////////////////////////////////////////////////////////
// StringCast                                            //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    std::string StringCast<double>::Eval(const ScriptingContext& context) const
    {
        double temp = m_value_ref->Eval(context);
        return DoubleToString(temp, 3, false);
    }
}

///////////////////////////////////////////////////////////
// UserStringLookup                                      //
///////////////////////////////////////////////////////////
ValueRef::UserStringLookup::UserStringLookup(const ValueRef::Variable<std::string>* value_ref) :
    ValueRef::Variable<std::string>(value_ref->GetReferenceType(), value_ref->PropertyName()),
    m_value_ref(value_ref)
{}

ValueRef::UserStringLookup::UserStringLookup(const ValueRef::ValueRefBase<std::string>* value_ref) :
    ValueRef::Variable<std::string>(ValueRef::NON_OBJECT_REFERENCE),
    m_value_ref(value_ref)
{}

ValueRef::UserStringLookup::~UserStringLookup()
{ delete m_value_ref; }

bool ValueRef::UserStringLookup::operator==(const ValueRef::ValueRefBase<std::string>& rhs) const {
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::UserStringLookup& rhs_ =
        static_cast<const ValueRef::UserStringLookup&>(rhs);

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

std::string ValueRef::UserStringLookup::Eval(const ScriptingContext& context) const {
    if (!m_value_ref)
        return "";
    std::string ref_val = m_value_ref->Eval(context);
    if (ref_val.empty() || !UserStringExists(ref_val))
        return "";
    return UserString(ref_val);
}

bool ValueRef::UserStringLookup::RootCandidateInvariant() const
{ return m_value_ref->RootCandidateInvariant(); }

bool ValueRef::UserStringLookup::LocalCandidateInvariant() const
{ return m_value_ref->LocalCandidateInvariant(); }

bool ValueRef::UserStringLookup::TargetInvariant() const
{ return m_value_ref->TargetInvariant(); }

bool ValueRef::UserStringLookup::SourceInvariant() const
{ return m_value_ref->SourceInvariant(); }

std::string ValueRef::UserStringLookup::Description() const
{ return m_value_ref->Description(); }

std::string ValueRef::UserStringLookup::Dump() const
{ return m_value_ref->Dump(); }

///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    std::string Operation<double>::Description() const
    {
        if (m_op_type == NEGATE) {
            //DebugLogger() << "Operation is negation";
            if (const ValueRef::Operation<double>* rhs = dynamic_cast<const ValueRef::Operation<double>*>(m_operand1)) {
                OpType op_type = rhs->GetOpType();
                if (op_type == PLUS     || op_type == MINUS ||
                    op_type == TIMES    || op_type == DIVIDE ||
                    op_type == NEGATE   || op_type == EXPONENTIATE)
                return "-(" + m_operand1->Description() + ")";
            } else {
                return "-" + m_operand1->Description();
            }
        }

        if (m_op_type == ABS)
            return "abs(" + m_operand1->Description() + ")";
        if (m_op_type == LOGARITHM)
            return "log(" + m_operand1->Description() + ")";
        if (m_op_type == SINE)
            return "sin(" + m_operand1->Description() + ")";
        if (m_op_type == COSINE)
            return "cos(" + m_operand1->Description() + ")";
        if (m_op_type == MINIMUM)
            return "min(" + m_operand1->Description() + ", " + m_operand2->Description() + ")";
        if (m_op_type == MAXIMUM)
            return "max(" + m_operand1->Description() + ", " + m_operand2->Description() + ")";
        if (m_op_type == RANDOM_UNIFORM)
            return "random(" + m_operand1->Description() + ", " + m_operand2->Description() + ")";


        bool parenthesize_lhs = false;
        bool parenthesize_rhs = false;
        if (const ValueRef::Operation<double>* lhs = dynamic_cast<const ValueRef::Operation<double>*>(m_operand1)) {
            OpType op_type = lhs->GetOpType();
            if (
                (m_op_type == EXPONENTIATE &&
                 (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
                  op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
                ) ||
                ((m_op_type == TIMES        || m_op_type == DIVIDE) &&
                 (op_type == PLUS           || op_type == MINUS)    || op_type == NEGATE)
               )
                parenthesize_lhs = true;
        }
        if (const ValueRef::Operation<double>* rhs = dynamic_cast<const ValueRef::Operation<double>*>(m_operand2)) {
            OpType op_type = rhs->GetOpType();
            if (
                (m_op_type == EXPONENTIATE &&
                 (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
                  op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
                ) ||
                ((m_op_type == TIMES        || m_op_type == DIVIDE) &&
                 (op_type == PLUS           || op_type == MINUS)    || op_type == NEGATE)
               )
                parenthesize_rhs = true;
        }

        std::string retval;
        if (parenthesize_lhs)
            retval += '(' + m_operand1->Description() + ')';
        else
            retval += m_operand1->Description();

        switch (m_op_type) {
        case PLUS:          retval += " + "; break;
        case MINUS:         retval += " - "; break;
        case TIMES:         retval += " * "; break;
        case DIVIDE:        retval += " / "; break;
        case EXPONENTIATE:  retval += " ^ "; break;
        default:            retval += " ? "; break;
        }

        if (parenthesize_rhs)
            retval += '(' + m_operand2->Description() + ')';
        else
            retval += m_operand2->Description();

        return retval;
    }

    template <>
    std::string Operation<std::string>::Eval(const ScriptingContext& context) const
    {
        std::string op_link;
        if (m_op_type == PLUS)
            return m_operand1->Eval(context) + m_operand2->Eval(context);
        throw std::runtime_error("std::string ValueRef evaluated with an unknown or invalid OpType.");
        return "";
    }

    template <>
    double      Operation<double>::Eval(const ScriptingContext& context) const
    {
        switch (m_op_type) {
            case PLUS:
                return m_operand1->Eval(context) + m_operand2->Eval(context);   break;

            case MINUS:
                return m_operand1->Eval(context) - m_operand2->Eval(context);   break;

            case TIMES:
                return m_operand1->Eval(context) * m_operand2->Eval(context);   break;

            case DIVIDE: {
                double op2 = m_operand2->Eval(context);
                if (op2 == 0.0)
                    return 0.0;
                return m_operand1->Eval(context) / op2;
                break;
            }

            case NEGATE:
                return -m_operand1->Eval(context);                              break;

            case EXPONENTIATE: {
                return std::pow(m_operand1->Eval(context),
                                m_operand2->Eval(context));
                break;
            }

            case ABS: {
                return std::abs(m_operand1->Eval(context));
                break;
            }

            case LOGARITHM: {
                double op1 = m_operand1->Eval(context);
                if (op1 <= 0.0)
                    return 0.0;
                return std::log(op1);
                break;
            }

            case SINE:
                return std::sin(m_operand1->Eval(context));                     break;

            case COSINE:
                return std::cos(m_operand1->Eval(context));                     break;

            case MINIMUM:
                return std::min(m_operand1->Eval(context),
                                m_operand2->Eval(context));
                break;

            case MAXIMUM:
                return std::max(m_operand1->Eval(context),
                                m_operand2->Eval(context));
                break;

            case RANDOM_UNIFORM: {
                double op1 = m_operand1->Eval(context);
                double op2 = m_operand2->Eval(context);
                double min_val = std::min(op1, op2);
                double max_val = std::max(op1, op2);
                return RandDouble(min_val, max_val);
                break;
            }

            default:
                throw std::runtime_error("double ValueRef evaluated with an unknown or invalid OpType.");
                break;
        }
    }

    template <>
    int         Operation<int>::Eval(const ScriptingContext& context) const
    {
        switch (m_op_type) {
            case PLUS:
                return m_operand1->Eval(context) + m_operand2->Eval(context);   break;

            case MINUS:
                return m_operand1->Eval(context) - m_operand2->Eval(context);   break;

            case TIMES:
                return m_operand1->Eval(context) * m_operand2->Eval(context);   break;

            case DIVIDE: {
                int op2 = m_operand2->Eval(context);
                if (op2 == 0)
                    return 0;
                return m_operand1->Eval(context) / op2;
                break;
            }

            case NEGATE:
                return -m_operand1->Eval(context);                              break;

            case EXPONENTIATE: {
                double op1 = m_operand1->Eval(context);
                double op2 = m_operand2->Eval(context);
                return static_cast<int>(std::pow(op1, op2));
                break;
            }

            case ABS: {
                return static_cast<int>(std::abs(m_operand1->Eval(context)));
                break;
            }

            case LOGARITHM: {
                double op1 = m_operand1->Eval(context);
                if (op1 <= 0.0)
                    return 0;
                return static_cast<int>(std::log(op1));
                break;
            }

            case SINE: {
                double op1 = m_operand1->Eval(context);
                return static_cast<int>(std::sin(op1));
                break;
            }

            case COSINE: {
                double op1 = m_operand1->Eval(context);
                return static_cast<int>(std::cos(op1));
                break;
            }

            case MINIMUM:
                return std::min<int>(m_operand1->Eval(context),
                                     m_operand2->Eval(context));
                break;

            case MAXIMUM:
                return std::max<int>(m_operand1->Eval(context),
                                     m_operand2->Eval(context));
                break;

            case RANDOM_UNIFORM: {
                double op1 = m_operand1->Eval(context);
                double op2 = m_operand2->Eval(context);
                int min_val = static_cast<int>(std::min(op1, op2));
                int max_val = static_cast<int>(std::max(op1, op2));
                return RandInt(min_val, max_val);
                break;
            }

            default:
                throw std::runtime_error("int ValueRef evaluated with an unknown or invalid OpType.");
                break;
        }
    }
}
