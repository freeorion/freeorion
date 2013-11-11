#include "ValueRef.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Planet.h"
#include "Species.h"
#include "System.h"
#include "Field.h"
#include "Universe.h"
#include "UniverseObject.h"
#include "Condition.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/Random.h"
#include "../util/Logger.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

namespace {
    TemporaryPtr<const UniverseObject> FollowReference(std::vector<std::string>::const_iterator first,
                                                       std::vector<std::string>::const_iterator last,
                                                       ValueRef::ReferenceType ref_type,
                                                       const ScriptingContext& context)
    {
        //Logger().debugStream() << "FollowReference: source: " << (context.source ? context.source->Name() : "0")
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
                if (TemporaryPtr<const Building> b = universe_object_ptr_cast<const Building>(obj))
                    obj = GetPlanet(b->PlanetID());
                else
                    obj = TemporaryPtr<const UniverseObject>();
            } else if (property_name == "System") {
                if (obj)
                    obj = GetSystem(obj->SystemID());
            } else if (property_name == "Fleet") {
                if (TemporaryPtr<const Ship> s = universe_object_ptr_cast<const Ship>(obj))
                    obj = GetFleet(s->FleetID());
                else
                    obj = TemporaryPtr<const UniverseObject>();
            }
            ++first;
        }
        return obj;
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

    MeterType NameToMeter(std::string name) {
        typedef std::map<std::string, MeterType> NameToMeterMap;
        static NameToMeterMap map;
        static bool once = true;
        if (once) {
            map["Population"] = METER_POPULATION;
            map["TargetPopulation"] = METER_TARGET_POPULATION;
            map["Industry"] = METER_INDUSTRY;
            map["TargetIndustry"] = METER_TARGET_INDUSTRY;
            map["Research"] = METER_RESEARCH;
            map["TargetResearch"] = METER_TARGET_RESEARCH;
            map["Trade"] = METER_TRADE;
            map["TargetTrade"] = METER_TARGET_TRADE;
            map["Construction"] = METER_CONSTRUCTION;
            map["TargetConstruction"] = METER_TARGET_CONSTRUCTION;
            map["Happiness"] = METER_HAPPINESS;
            map["TargetHappiness"] = METER_TARGET_HAPPINESS;
            map["MaxFuel"] = METER_MAX_FUEL;
            map["Fuel"] = METER_FUEL;
            map["MaxStructure"] = METER_MAX_STRUCTURE;
            map["Structure"] = METER_STRUCTURE;
            map["MaxShield"] = METER_MAX_SHIELD;
            map["Shield"] = METER_SHIELD;
            map["MaxDefense"] = METER_MAX_DEFENSE;
            map["Defense"] = METER_DEFENSE;
            map["MaxTroops"] = METER_MAX_TROOPS;
            map["Troops"] = METER_TROOPS;
            map["RebelTroops"] = METER_REBEL_TROOPS;
            map["Supply"] = METER_SUPPLY;
            map["Stealth"] = METER_STEALTH;
            map["Detection"] = METER_DETECTION;
            map["BattleSpeed"] = METER_BATTLE_SPEED;
            map["StarlaneSpeed"] = METER_STARLANE_SPEED;
            map["Damage"] = METER_DAMAGE;
            map["ROF"] = METER_ROF;
            map["Range"] = METER_RANGE;
            map["Speed"] = METER_SPEED;
            map["Capacity"] = METER_CAPACITY;
            map["AntiShipDamage"] = METER_ANTI_SHIP_DAMAGE;
            map["AntiFighterDamage"] = METER_ANTI_FIGHTER_DAMAGE;
            map["LaunchRate"] = METER_LAUNCH_RATE;
            map["FighterWeaponRange"] = METER_FIGHTER_WEAPON_RANGE;
            map["Size"] = METER_SIZE;
            once = false;
        }
        MeterType retval = INVALID_METER_TYPE;
        NameToMeterMap::const_iterator it = map.find(name);
        if (it != map.end())
            retval = it->second;
        return retval;
    }
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
    { return boost::lexical_cast<std::string>(m_value); }   // might be nicer to return "DoubleToString(m_value, 3, false);" but this would require building ClientUI.cpp on the Server and AI client ...

    template <>
    std::string Constant<double>::Description() const
    { return boost::lexical_cast<std::string>(m_value); }

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
    std::string names_size = boost::lexical_cast<std::string>(property_names.size());
    boost::format formatter = FlexibleFormat(UserString("DESC_VALUE_REF_MULTIPART_VARIABLE" +
                                             names_size));

    switch (ref_type) {
    case ValueRef::SOURCE_REFERENCE:                    formatter % UserString("DESC_VAR_SOURCE");          break;
    case ValueRef::EFFECT_TARGET_REFERENCE:             formatter % UserString("DESC_VAR_TARGET");          break;
    case ValueRef::EFFECT_TARGET_VALUE_REFERENCE:       formatter % UserString("DESC_VAR_VALUE");           break;
    case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE: formatter % UserString("DESC_VAR_LOCAL_CANDIDATE"); break;
    case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:  formatter % UserString("DESC_VAR_ROOT_CANDIDATE");  break;
    case ValueRef::NON_OBJECT_REFERENCE:                formatter % "";                                     break;
    default:                                            formatter % "???";                                  break;
    }

    for (unsigned int i = 0; i < property_names.size(); ++i) {
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
            Logger().errorStream() << "Variable<PlanetSize>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return INVALID_PLANET_SIZE;
        }

        if (TemporaryPtr<const Planet> p = universe_object_ptr_cast<const Planet>(object)) {
            if (property_name == "PlanetSize")
                return p->Size();
            else if (property_name == "NextLargerPlanetSize")
                return p->NextLargerPlanetSize();
            else if (property_name == "NextSmallerPlanetSize")
                return p->NextSmallerPlanetSize();
        }

        Logger().errorStream() << "Variable<PlanetSize>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
        return INVALID_PLANET_SIZE;
    }

    template <>
    PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(PlanetType)

        TemporaryPtr<const UniverseObject> object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<PlanetType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return INVALID_PLANET_TYPE;
        }

        if (TemporaryPtr<const Planet> p = universe_object_ptr_cast<const Planet>(object)) {
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

        Logger().errorStream() << "Variable<PlanetType>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
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
                Logger().errorStream() << "Variable<PlanetEnvironment>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
                return INVALID_PLANET_ENVIRONMENT;
            }
            if (TemporaryPtr<const Planet> p = universe_object_ptr_cast<const Planet>(object))
                return p->EnvironmentForSpecies();
        }

        Logger().errorStream() << "Variable<PlanetEnvironment>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
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
                Logger().errorStream() << "Variable<UniverseObjectType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
                return INVALID_UNIVERSE_OBJECT_TYPE;
            }
            ObjectTypeVisitor v;
            if (object->Accept(v))
                return v.m_type;
            else if (dynamic_ptr_cast<const PopCenter>(object))
                return OBJ_POP_CENTER;
            else if (dynamic_ptr_cast<const ResourceCenter>(object))
                return OBJ_PROD_CENTER;
        }

        Logger().errorStream() << "Variable<UniverseObjectType>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
        return INVALID_UNIVERSE_OBJECT_TYPE;
    }

    template <>
    StarType Variable<StarType>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(StarType)

        TemporaryPtr<const UniverseObject> object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<StarType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return INVALID_STAR_TYPE;
        }

        if (TemporaryPtr<const System> s = universe_object_ptr_cast<const System>(object)) {
            if (property_name == "StarType")
                return s->GetStarType();
            else if (property_name == "NextOlderStarType")
                return s->NextOlderStarType();
            else if (property_name == "NextYoungerStarType")
                return s->NextYoungerStarType();
        }

        Logger().errorStream() << "Variable<StarType>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
        return INVALID_STAR_TYPE;
    }

    template <>
    double Variable<double>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(float)

        if (m_ref_type == ValueRef::NON_OBJECT_REFERENCE) {
            if (property_name == "CurrentTurn") {
                return CurrentTurn();
            } else if (property_name == "UniverseCentreX" |
                       property_name == "UniverseCentreY")
            {
                return GetUniverse().UniverseWidth() / 2;
            }

            // add more non-object reference double functions here
            Logger().errorStream() << "Variable<double>::Eval unrecognized non-object property: " << ReconstructName(m_property_name, m_ref_type);
            return 0.0;
        }

        TemporaryPtr<const UniverseObject> object =
            FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<double>::Eval unable to follow reference: "
                                   << ReconstructName(m_property_name, m_ref_type);
            return 0.0;
        }

        MeterType meter_type = NameToMeter(property_name);
        if (object && meter_type != INVALID_METER_TYPE) {
            if (object->GetMeter(meter_type))
                return object->InitialMeterValue(meter_type);

        } else if (property_name == "TradeStockpile") {
            if (const Empire* empire = Empires().Lookup(object->Owner()))
                return empire->ResourceStockpile(RE_TRADE);

        } else if (property_name == "DistanceToSource") {
            if (!context.source) {
                Logger().errorStream() << "ValueRef::Variable<double>::Eval can't find distance to source because no source was passed";
                return 0.0;
            }
            double delta_x = object->X() - context.source->X();
            double delta_y = object->Y() - context.source->Y();
            return std::sqrt(delta_x * delta_x + delta_y * delta_y);

        } else if (property_name == "X") {
            return object->X();

        } else if (property_name == "Y") {
            return object->Y();

        } else if (property_name == "SizeAsDouble") {
            if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object))
                return planet->SizeAsInt();

        } else if (property_name == "DistanceFromOriginalType") {
            if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object))
                return planet->DistanceFromOriginalType();

        } else if (property_name == "NextTurnPopGrowth") {
            if (TemporaryPtr<const PopCenter> pop = dynamic_ptr_cast<const PopCenter>(object))
                return pop->NextTurnPopGrowth();

        } else if (property_name == "CurrentTurn") {
            return CurrentTurn();

        }

        Logger().errorStream() << "Variable<double>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
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

            // add more non-object reference int functions here
            Logger().errorStream() << "Variable<int>::Eval unrecognized non-object property: " << ReconstructName(m_property_name, m_ref_type);
            return 0;
        }

        TemporaryPtr<const UniverseObject> object =
            FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<int>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
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
            if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object))
                return planet->TurnsSinceFocusChange();
            else
                return 0;
        } else if (property_name == "ProducedByEmpireID") {
            if (TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(object))
                return ship->ProducedByEmpireID();
            else if (TemporaryPtr<const Building> building = universe_object_ptr_cast<const Building>(object))
                return building->ProducedByEmpireID();
            else
                return ALL_EMPIRES;
        } else if (property_name == "DesignID") {
            if (TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(object))
                return ship->DesignID();
            else
                return ShipDesign::INVALID_DESIGN_ID;
        } else if (property_name == "Species") {
            if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object))
                return GetSpeciesManager().GetSpeciesID(planet->SpeciesName());
            else if (TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(object))
                return GetSpeciesManager().GetSpeciesID(ship->SpeciesName());
            else
                return -1;
        } else if (property_name == "FleetID") {
            if (TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(object))
                return ship->FleetID();
            else if (TemporaryPtr<const Fleet> fleet = universe_object_ptr_cast<const Fleet>(object))
                return fleet->ID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "PlanetID") {
            if (TemporaryPtr<const Building> building = universe_object_ptr_cast<const Building>(object))
                return building->PlanetID();
            else if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object))
                return planet->ID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "SystemID") {
            return object->SystemID();
        } else if (property_name == "FinalDestinationID") {
            if (TemporaryPtr<const Fleet> fleet = universe_object_ptr_cast<const Fleet>(object))
                return fleet->FinalDestinationID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "NextSystemID") {
            if (TemporaryPtr<const Fleet> fleet = universe_object_ptr_cast<const Fleet>(object))
                return fleet->NextSystemID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "PreviousSystemID") {
            if (TemporaryPtr<const Fleet> fleet = universe_object_ptr_cast<const Fleet>(object))
                return fleet->PreviousSystemID();
            else
                return INVALID_OBJECT_ID;
        } else if (property_name == "NumShips") {
            if (TemporaryPtr<const Fleet> fleet = universe_object_ptr_cast<const Fleet>(object))
                return fleet->NumShips();
            else
                return 0;
        } else if (property_name == "LastTurnBattleHere") {
            if (TemporaryPtr<const System> system = universe_object_ptr_cast<const System>(object))
                return system->LastTurnBattleHere();
            else
                return INVALID_GAME_TURN;
        } else if (property_name == "Orbit") {
            if (TemporaryPtr<const System> system = GetSystem(object->SystemID()))
                return system->OrbitOfObjectID(object->ID());
            return -1;
        }

        Logger().errorStream() << "Variable<int>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
        return 0;
    }

    template <>
    std::string Variable<std::string>::Eval(const ScriptingContext& context) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE(std::string)

        if (m_ref_type == ValueRef::NON_OBJECT_REFERENCE) {
            // add non-object reference string functions here
            Logger().errorStream() << "Variable<std::string>::Eval unrecognized non-object property: " << ReconstructName(m_property_name, m_ref_type);
            return "";
        }

        TemporaryPtr<const UniverseObject> object =
            FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<std::string>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return "";
        }

        if (property_name == "Name") {
            return object->Name();

        } else if (property_name == "Species") {
            if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object))
                return planet->SpeciesName();
            else if (TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(object))
                return ship->SpeciesName();

        } else if (property_name == "BuildingType") {
            if (TemporaryPtr<const Building> building = universe_object_ptr_cast<const Building>(object))
                return building->BuildingTypeName();

        } else if (property_name == "Focus") {
            if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object))
                return planet->Focus();

        } else if (property_name == "PreferredFocus") {
            const Species* species = 0;
            if (TemporaryPtr<const Planet> planet = universe_object_ptr_cast<const Planet>(object)) {
                species = GetSpecies(planet->SpeciesName());
            } else if (TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(object)) {
                species = GetSpecies(ship->SpeciesName());
            }
            if (species)
                return species->PreferredFocus();
            return "";

        } else if (property_name == "OwnerMostExpensiveEnqueuedTech") {
            const Empire* empire = Empires().Lookup(object->Owner());
            if (!empire)
                return "";
            return empire->LeastExpensiveEnqueuedTech(true);

        } else if (property_name == "OwnerMostExpensiveEnqueuedTech") {
            const Empire* empire = Empires().Lookup(object->Owner());
            if (!empire)
                return "";
            return empire->MostExpensiveEnqueuedTech(true);

        } else if (property_name == "OwnerMostRPCostLeftEnqueuedTech") {
            const Empire* empire = Empires().Lookup(object->Owner());
            if (!empire)
                return "";
            return empire->MostRPCostLeftEnqueuedTech(true);

        } else if (property_name == "OwnerMostRPSpentEnqueuedTech") {
            const Empire* empire = Empires().Lookup(object->Owner());
            if (!empire)
                return "";
            return empire->MostRPSpentEnqueuedTech(true);

        } else if (property_name == "OwnerTopPriorityEnqueuedTech") {
            const Empire* empire = Empires().Lookup(object->Owner());
            if (!empire)
                return "";
            return empire->TopPriorityEnqueuedTech(true);
        }

        Logger().errorStream() << "Variable<std::string>::Eval unrecognized object property: " << ReconstructName(m_property_name, m_ref_type);
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
// Operation                                             //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    std::string Operation<double>::Description() const
    {
        if (m_op_type == NEGATE) {
            //Logger().debugStream() << "Operation is negation";
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
            return "min(" + m_operand1->Description() + ", " + m_operand1->Description() + ")";
        if (m_op_type == MAXIMUM)
            return "max(" + m_operand1->Description() + ", " + m_operand1->Description() + ")";
        if (m_op_type == RANDOM_UNIFORM)
            return "random(" + m_operand1->Description() + ", " + m_operand1->Description() + ")";


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
}


namespace ValueRef {
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
