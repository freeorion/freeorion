#include "ValueRef.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "Planet.h"
#include "System.h"
#include "UniverseObject.h"
#include "Condition.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/MultiplayerCommon.h"

#include <GG/adobe/closed_hash.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

int g_indent = 0;

namespace {
    const UniverseObject* FollowReference(std::vector<adobe::name_t>::const_iterator first,
                                          std::vector<adobe::name_t>::const_iterator last,
                                          ValueRef::ReferenceType ref_type,
                                          const ScriptingContext& context)
    {
        //Logger().debugStream() << "FollowReference: source: " << (context.source ? context.source->Name() : "0")
        //    << " target: " << (context.effect_target ? context.effect_target->Name() : "0")
        //    << " local c: " << (context.condition_local_candidate ? context.condition_local_candidate->Name() : "0")
        //    << " root c: " << (context.condition_root_candidate ? context.condition_root_candidate->Name() : "0");

        const UniverseObject* obj(0);
        switch(ref_type) {
        case ValueRef::SOURCE_REFERENCE:                        obj = context.source;                       break;
        case ValueRef::EFFECT_TARGET_REFERENCE:                 obj = context.effect_target;                break;
        case ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE:     obj = context.condition_local_candidate;    break;
        case ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE:      obj = context.condition_root_candidate;     break;
        default:
            return 0;
        }

        const ObjectMap& objects = GetMainObjectMap();
        while (first != last) {
            adobe::name_t property_name = *first;
            if (property_name == Planet_name) {
                if (const Building* b = universe_object_cast<const Building*>(obj))
                    obj = objects.Object<Planet>(b->PlanetID());
                else
                    obj = 0;
            } else if (property_name == System_name) {
                if (obj)
                    obj = objects.Object<System>(obj->SystemID());
            } else if (property_name == Fleet_name) {
                if (const Ship* s = universe_object_cast<const Ship*>(obj))
                    obj = objects.Object<Fleet>(s->FleetID());
                else
                    obj = 0;
            }
            ++first;
        }
        return obj;
    }

    struct ObjectTypeVisitor : UniverseObjectVisitor
    {
        ObjectTypeVisitor() : m_type(INVALID_UNIVERSE_OBJECT_TYPE) {}

        virtual UniverseObject* Visit(Building* obj) const
            { m_type = OBJ_BUILDING; return obj; }
        virtual UniverseObject* Visit(Fleet* obj) const
            { m_type = OBJ_FLEET; return obj; }
        virtual UniverseObject* Visit(Planet* obj) const
            { m_type = OBJ_PLANET; return obj; }
        virtual UniverseObject* Visit(Ship* obj) const
            { m_type = OBJ_SHIP; return obj; }
        virtual UniverseObject* Visit(System* obj) const
            { m_type = OBJ_SYSTEM; return obj; }

        mutable UniverseObjectType m_type;
    };

    MeterType NameToMeter(adobe::name_t name)
    {
        typedef adobe::closed_hash_map<adobe::name_t, MeterType> NameToMeterMap;
        static NameToMeterMap map;
        static bool once = true;
        if (once) {
            map[Population_name] = METER_POPULATION;
            map[TargetPopulation_name] = METER_TARGET_POPULATION;
            map[Health_name] = METER_HEALTH;
            map[TargetHealth_name] = METER_TARGET_HEALTH;
            map[Farming_name] = METER_FARMING;
            map[TargetFarming_name] = METER_TARGET_FARMING;
            map[Industry_name] = METER_INDUSTRY;
            map[TargetIndustry_name] = METER_TARGET_INDUSTRY;
            map[Research_name] = METER_RESEARCH;
            map[TargetResearch_name] = METER_TARGET_RESEARCH;
            map[Trade_name] = METER_TRADE;
            map[TargetTrade_name] = METER_TARGET_TRADE;
            map[Mining_name] = METER_MINING;
            map[TargetMining_name] = METER_TARGET_MINING;
            map[Construction_name] = METER_CONSTRUCTION;
            map[TargetConstruction_name] = METER_TARGET_CONSTRUCTION;
            map[MaxFuel_name] = METER_MAX_FUEL;
            map[Fuel_name] = METER_FUEL;
            map[MaxStructure_name] = METER_MAX_STRUCTURE;
            map[Structure_name] = METER_STRUCTURE;
            map[MaxShield_name] = METER_MAX_SHIELD;
            map[Shield_name] = METER_SHIELD;
            map[MaxDefense_name] = METER_MAX_DEFENSE;
            map[Defense_name] = METER_DEFENSE;
            map[MaxTroops_name] = METER_MAX_TROOPS;
            map[Troops_name] = METER_TROOPS;
            map[FoodConsumption_name] = METER_FOOD_CONSUMPTION;
            map[Supply_name] = METER_SUPPLY;
            map[Stealth_name] = METER_STEALTH;
            map[Detection_name] = METER_DETECTION;
            map[BattleSpeed_name] = METER_BATTLE_SPEED;
            map[StarlaneSpeed_name] = METER_STARLANE_SPEED;
            map[Damage_name] = METER_DAMAGE;
            map[ROF_name] = METER_ROF;
            map[Range_name] = METER_RANGE;
            map[Speed_name] = METER_SPEED;
            map[Capacity_name] = METER_CAPACITY;
            map[AntiShipDamage_name] = METER_ANTI_SHIP_DAMAGE;
            map[AntiFighterDamage_name] = METER_ANTI_FIGHTER_DAMAGE;
            map[LaunchRate_name] = METER_LAUNCH_RATE;
            map[FighterWeaponRange_name] = METER_FIGHTER_WEAPON_RANGE;
            once = false;
        }
        MeterType retval = INVALID_METER_TYPE;
        NameToMeterMap::const_iterator it = map.find(name);
        if (it != map.end())
            retval = it->second;
        return retval;
    }

}

std::string ValueRef::ReconstructName(const std::vector<adobe::name_t>& property_name,
                                      ValueRef::ReferenceType ref_type)
{
    std::string retval;
    switch (ref_type) {
    case ValueRef::SOURCE_REFERENCE:                    retval = "Source";          break;
    case ValueRef::EFFECT_TARGET_REFERENCE: {
        // "Value" is actually a reference to the target object, but we
        // don't want to output "Target.Value", so if "Value" is the
        // property name, skip prepending "Target".  Otherwise, prepend
        // target as with other direct object references.
        if (property_name[0] != Value_name)
            retval = "Target";
        break;
    }
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
    {
        return Description();
    }

    template <>
    std::string Constant<double>::Dump() const
    {
        return Description();
    }

    template <>
    std::string Constant<std::string>::Dump() const
    { return "\"" + Description() + "\""; }
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
namespace ValueRef {

#define IF_CURRENT_VALUE(T)                                                \
    if (property_name == Value_name) {                                     \
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
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(PlanetSize)

        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<PlanetSize>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return INVALID_PLANET_SIZE;
        }

        if (property_name == PlanetSize_name) {
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->Size();
        } else if (property_name == NextLargerPlanetSize_name) {
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->NextLargerPlanetSize();
        } else if (property_name == NextSmallerPlanetSize_name) {
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->NextSmallerPlanetSize();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetSize value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type PlanetSize.");
        }
        return INVALID_PLANET_SIZE;
    }

    template <>
    PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const
    {
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(PlanetType)

        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<PlanetType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return INVALID_PLANET_TYPE;
        }

        if (property_name == PlanetType_name) {
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->Type();
        } else if (property_name == NextBetterPlanetType_name) {
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->NextBetterPlanetTypeForSpecies();
        } else if (property_name == ClockwiseNextPlanetType_name) {
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->ClockwiseNextPlanetType();
        } else if (property_name == CounterClockwiseNextPlanetType_name) {
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->CounterClockwiseNextPlanetType();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetType value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type PlanetType.");
        }
        return INVALID_PLANET_TYPE;
    }

    template <>
    PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const
    {
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(PlanetEnvironment)

        if (property_name == PlanetEnvironment_name) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
            if (!object) {
                Logger().errorStream() << "Variable<PlanetEnvironment>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
                return INVALID_PLANET_ENVIRONMENT;
            }
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->EnvironmentForSpecies();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetEnvironment value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type PlanetEnvironment.");
        }
        return INVALID_PLANET_ENVIRONMENT;
    }

    template <>
    UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const
    {
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(UniverseObjectType)

        if (property_name == ObjectType_name) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
            if (!object) {
                Logger().errorStream() << "Variable<UniverseObjectType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
                return INVALID_UNIVERSE_OBJECT_TYPE;
            }
            ObjectTypeVisitor v;
            if (object->Accept(v))
                return v.m_type;
            else if (dynamic_cast<const PopCenter*>(object))
                return OBJ_POP_CENTER;
            else if (dynamic_cast<const ResourceCenter*>(object))
                return OBJ_PROD_CENTER;
        } else {
            throw std::runtime_error("Attempted to read a non-ObjectType value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type ObjectType.");
        }
        return INVALID_UNIVERSE_OBJECT_TYPE;
    }

    template <>
    StarType Variable<StarType>::Eval(const ScriptingContext& context) const
    {
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(StarType)

        if (property_name == StarType_name) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
            if (!object) {
                Logger().errorStream() << "Variable<StarType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
                return INVALID_STAR_TYPE;
            }
            if (const System* s = universe_object_cast<const System*>(object))
                return s->GetStarType();
        } else {
            throw std::runtime_error("Attempted to read a non-StarType value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type StarType.");
        }
        return INVALID_STAR_TYPE;
    }

    template <>
    double Variable<double>::Eval(const ScriptingContext& context) const
    {
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(double)

        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(),
                                                       m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<double>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return 0.0;
        }

        MeterType meter_type = NameToMeter(property_name);
        if        (meter_type != INVALID_METER_TYPE) {
            return object->InitialMeterValue(meter_type);

        } else if (property_name == TradeStockpile_name) {
            if (const Empire* empire = Empires().Lookup(object->Owner()))
                return empire->ResourceStockpile(RE_TRADE);
        } else if (property_name == MineralStockpile_name) {
            if (const Empire* empire = Empires().Lookup(object->Owner()))
                return empire->ResourceStockpile(RE_MINERALS);
        } else if (property_name == FoodStockpile_name) {
            if (const Empire* empire = Empires().Lookup(object->Owner()))
                return empire->ResourceStockpile(RE_FOOD);

        } else if (property_name == AllocatedFood_name) {
            if (const PopCenter* pop = dynamic_cast<const PopCenter*>(object))
                return pop->AllocatedFood();
        } else if (property_name == FoodAllocationForMaxGrowth_name) {
            if (const PopCenter* pop = dynamic_cast<const PopCenter*>(object))
                return pop->FoodAllocationForMaxGrowth();

        } else if (property_name == DistanceToSource_name) {
            if (!context.source) {
                Logger().errorStream() << "ValueRef::Variable<double>::Eval can't find distance to source because no source was passed";
                return 0.0;
            }
            double delta_x = object->X() - context.source->X();
            double delta_y = object->Y() - context.source->Y();
            return std::sqrt(delta_x * delta_x + delta_y * delta_y);

        } else {
            throw std::runtime_error("Attempted to read a non-double value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type double.");
        }

        return 0.0;
    }

    template <>
    int Variable<int>::Eval(const ScriptingContext& context) const
    {
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(int)

        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<int>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return 0;
        }

        if (property_name == Owner_name) {
            return object->Owner();
        } else if (property_name == ID_name) {
            return object->ID();
        } else if (property_name == CreationTurn_name) {
            return object->CreationTurn();
        } else if (property_name == Age_name) {
            return object->AgeInTurns();
        } else if (property_name == ProducedByEmpireID_name) {
            if (const Ship* ship = universe_object_cast<const Ship*>(object))
                return ship->ProducedByEmpireID();
            else if (const Building* building = universe_object_cast<const Building*>(object))
                return building->ProducedByEmpireID();
            else
                return ALL_EMPIRES;
        } else if (property_name == DesignID_name) {
            if (const Ship* ship = universe_object_cast<const Ship*>(object))
                return ship->DesignID();
            else
                return ShipDesign::INVALID_DESIGN_ID;
        } else if (property_name == FleetID_name) {
            if (const Ship* ship = universe_object_cast<const Ship*>(object))
                return ship->FleetID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (property_name == PlanetID_name) {
            if (const Building* building = universe_object_cast<const Building*>(object))
                return building->PlanetID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (property_name == SystemID_name) {
            return object->SystemID();
        } else if (property_name == FinalDestinationID_name) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->FinalDestinationID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (property_name == NextSystemID_name) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->NextSystemID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (property_name == PreviousSystemID_name) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->PreviousSystemID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (property_name == NumShips_name) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->NumShips();
            else
                return 0;
        } else if (property_name == LastTurnBattleHere_name) {
            if (const System* system = universe_object_cast<const System*>(object))
                return system->LastTurnBattleHere();
            else
                return INVALID_GAME_TURN;
        } else if (property_name == CurrentTurn_name) {
            return CurrentTurn();
        } else {
            throw std::runtime_error("Attempted to read a non-int value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type int.");
        }

        return 0;
    }

    template <>
    std::string Variable<std::string>::Eval(const ScriptingContext& context) const
    {
        const adobe::name_t& property_name = m_property_name.back();

        IF_CURRENT_VALUE(std::string)

        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_ref_type, context);
        if (!object) {
            Logger().errorStream() << "Variable<std::string>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_ref_type);
            return "";
        }

        if (property_name == Name_name) {
            return object->Name();
        } else if (property_name == Species_name) {
            if (const Planet* planet = universe_object_cast<const Planet*>(object))
                return planet->SpeciesName();
            else if (const Ship* ship = universe_object_cast<const Ship*>(object))
                return ship->SpeciesName();
        } else if (property_name == BuildingType_name) {
            if (const Building* building = universe_object_cast<const Building*>(object))
                return building->BuildingTypeName();
        } else if (property_name == Focus_name) {
            if (const Planet* planet = universe_object_cast<const Planet*>(object))
                return planet->Focus();
        } else {
            throw std::runtime_error("Attempted to read a non-string value \"" + ReconstructName(m_property_name, m_ref_type) + "\" using a ValueRef of type std::string.");
        }

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
        condition_matches.reserve(RESERVE_SET_SIZE);
        GetConditionMatches(context, condition_matches, m_sampling_condition);

        if (m_stat_type == COUNT)
            return static_cast<double>(condition_matches.size());

        // evaluate property for each condition-matched object
        std::map<const UniverseObject*, double> object_property_values;
        GetObjectPropertyValues(context, condition_matches, object_property_values);

        return ReduceData(object_property_values);
    }

    template <>
    int Statistic<int>::Eval(const ScriptingContext& context) const
    {
        Condition::ObjectSet condition_matches;
        condition_matches.reserve(RESERVE_SET_SIZE);
        GetConditionMatches(context, condition_matches, m_sampling_condition);

        if (m_stat_type == COUNT)
            return static_cast<int>(condition_matches.size());

        // evaluate property for each condition-matched object
        std::map<const UniverseObject*, int> object_property_values;
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
        condition_matches.reserve(RESERVE_SET_SIZE);
        GetConditionMatches(context, condition_matches, m_sampling_condition);

        if (condition_matches.empty())
            return "";

        // evaluate property for each condition-matched object
        std::map<const UniverseObject*, std::string> object_property_values;
        GetObjectPropertyValues(context, condition_matches, object_property_values);

        // count number of each result, tracking which has the most occurances
        std::map<std::string, unsigned int> histogram;
        std::map<std::string, unsigned int>::const_iterator most_common_property_value_it = histogram.begin();
        unsigned int max_seen(0);

        for (std::map<const UniverseObject*, std::string>::const_iterator it = object_property_values.begin();
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
    std::string Operation<std::string>::Eval(const ScriptingContext& context) const
    {
        std::string op_link;
        switch (m_op_type) {
            case PLUS:      op_link = " + ";    break;
            case MINUS:     op_link = " - ";    break;
            case TIMES:     op_link = " * ";    break;
            case DIVIDES:   op_link = " / ";    break;
            case NEGATE:
                return "-" + m_operand1->Eval(context);
                break;
            default:
                throw std::runtime_error("std::string ValueRef evaluated with an unknown OpType.");
                break;
        }
        return m_operand1->Eval(context)
               + op_link +
               m_operand2->Eval(context);
    }
}

std::string DumpIndent()
{ return std::string(g_indent * 4, ' '); }
