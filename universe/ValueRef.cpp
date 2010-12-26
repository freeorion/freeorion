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

#include <boost/spirit/include/classic.hpp>
#include <boost/algorithm/string.hpp>

int g_indent = 0;

namespace detail {
    std::vector<std::string> TokenizeDottedReference(const std::string& str)
    {
        using namespace boost::spirit::classic;
        std::vector<std::string> retval;
        rule<> tokenizer = *((+(anychar_p - '.'))[append(retval)] >> !ch_p('.'));
        parse(str.c_str(), tokenizer);
        return retval;
    }
}

namespace {
    const UniverseObject* FollowReference(std::vector<std::string>::const_iterator first,
                                          std::vector<std::string>::const_iterator last,
                                          const UniverseObject* obj)
    {
        const ObjectMap& objects = GetMainObjectMap();
        while (first != last) {
            const std::string& property_name = *first;
            if (boost::iequals(property_name, "Planet")) {
                if (const Building* b = universe_object_cast<const Building*>(obj))
                    obj = objects.Object<Planet>(b->PlanetID());
                else
                    obj = 0;
            } else if (boost::iequals(property_name, "System")) {
                if (obj)
                    obj = objects.Object<System>(obj->SystemID());
            } else if (boost::iequals(property_name, "Fleet")) {
                if (const Ship* s = universe_object_cast<const Ship*>(obj))
                    obj = objects.Object<Fleet>(s->FleetID());
                else
                    obj = 0;
            }
            ++first;
        }
        return obj;
    }

    std::string ReconstructName(const std::vector<std::string>& name, bool source_ref)
    {
        std::string retval(source_ref ? "Source" : "Target");
        for (unsigned int i = 0; i < name.size(); ++i) {
            retval += '.';
            retval += name[i];
        }
        return retval;
    }
}

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    std::string Constant<int>::Description() const
    {
        return boost::lexical_cast<std::string>(m_value);   // might be nicer to return "DoubleToString(m_value, 3, false);" but this would require building ClientUI.cpp on the Server and AI client ...
    }

    template <>
    std::string Constant<double>::Description() const
    {
        return boost::lexical_cast<std::string>(m_value);
    }

    template <>
    std::string Constant<std::string>::Description() const
    {
        return m_value;
    }

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
    {
        return Description();
    }
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
namespace ValueRef {

#define IF_CURRENT_VALUE_ELSE(T)                                           \
    if (boost::iequals(property_name, "Value")) {                          \
        if (current_value.empty())                                         \
            throw std::runtime_error(                                      \
                "Variable<" #T ">::Eval(): Value could not be evaluated, " \
                "because no current value was provided.");                 \
        try {                                                              \
            return boost::any_cast<T>(current_value);                      \
        } catch (const boost::bad_any_cast&) {                             \
            throw std::runtime_error(                                      \
                "Variable<" #T ">::Eval(): Value could not be evaluated, " \
                "because the provided current value is not an " #T ".");   \
        }                                                                  \
    } else

    template <>
    PlanetSize Variable<PlanetSize>::Eval(const UniverseObject* source, const UniverseObject* target,
                                          const boost::any& current_value) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(PlanetSize)

        if (boost::iequals(property_name, "PlanetSize")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (!object) {
                Logger().errorStream() << "Variable<PlanetSize>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
                return INVALID_PLANET_SIZE;
            }
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->Size();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetSize value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetSize.");
        }
        return INVALID_PLANET_SIZE;
    }

    template <>
    PlanetType Variable<PlanetType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                          const boost::any& current_value) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(PlanetType)

        if (boost::iequals(property_name, "PlanetType")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (!object) {
                Logger().errorStream() << "Variable<PlanetType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
                return INVALID_PLANET_TYPE;
            }
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->Type();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetType.");
        }
        return INVALID_PLANET_TYPE;
    }

    template <>
    PlanetEnvironment Variable<PlanetEnvironment>::Eval(const UniverseObject* source, const UniverseObject* target,
                                                        const boost::any& current_value) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(PlanetEnvironment)

        if (boost::iequals(property_name, "PlanetEnvironment")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (!object) {
                Logger().errorStream() << "Variable<PlanetEnvironment>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
                return INVALID_PLANET_ENVIRONMENT;
            }
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                return p->EnvironmentForSpecies();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetEnvironment value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetEnvironment.");
        }
        return INVALID_PLANET_ENVIRONMENT;
    }

    template <>
    UniverseObjectType Variable<UniverseObjectType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                                          const boost::any& current_value) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(UniverseObjectType)

        if (boost::iequals(property_name, "ObjectType")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (!object) {
                Logger().errorStream() << "Variable<UniverseObjectType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
                return INVALID_UNIVERSE_OBJECT_TYPE;
            }
            if (universe_object_cast<const Planet*>(object)) {
                return OBJ_PLANET;
            } else if (universe_object_cast<const System*>(object)) {
                return OBJ_SYSTEM;
            } else if (universe_object_cast<const Building*>(object)) {
                return OBJ_BUILDING;
            } else if (universe_object_cast<const Ship*>(object)) {
                return OBJ_SHIP;
            } else if (universe_object_cast<const Fleet*>(object)) {
                return OBJ_FLEET;
            } else if (dynamic_cast<const PopCenter*>(object)) {
                return OBJ_POP_CENTER;
            } else if (dynamic_cast<const ResourceCenter*>(object)) {
                return OBJ_PROD_CENTER;
            }
        } else {
            throw std::runtime_error("Attempted to read a non-ObjectType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type ObjectType.");
        }
        return INVALID_UNIVERSE_OBJECT_TYPE;
    }

    template <>
    StarType Variable<StarType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                      const boost::any& current_value) const
    {
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(StarType)

        if (boost::iequals(property_name, "StarType")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (!object) {
                Logger().errorStream() << "Variable<StarType>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
                return INVALID_STAR_TYPE;
            }
            if (const System* s = universe_object_cast<const System*>(object))
                return s->GetStarType();
        } else {
            throw std::runtime_error("Attempted to read a non-StarType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type StarType.");
        }
        return INVALID_STAR_TYPE;
    }

    template <>
    double Variable<double>::Eval(const UniverseObject* source, const UniverseObject* target,
                                  const boost::any& current_value) const
    {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (!object) {
            Logger().errorStream() << "Variable<double>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
            return 0.0;
        }
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(double)

        if        (boost::iequals(property_name, "Population")) {
            return object->InitialMeterValue(METER_POPULATION);
        } else if (boost::iequals(property_name, "TargetPopulation")) {
            return object->InitialMeterValue(METER_TARGET_POPULATION);
        } else if (boost::iequals(property_name, "Health")) {
            return object->InitialMeterValue(METER_HEALTH);
        } else if (boost::iequals(property_name, "TargetHealth")) {
            return object->InitialMeterValue(METER_TARGET_HEALTH);

        } else if (boost::iequals(property_name, "Farming")) {
            return object->InitialMeterValue(METER_FARMING);
        } else if (boost::iequals(property_name, "TargetFarming")) {
            return object->InitialMeterValue(METER_TARGET_FARMING);
        } else if (boost::iequals(property_name, "Industry")) {
            return object->InitialMeterValue(METER_INDUSTRY);
        } else if (boost::iequals(property_name, "TargetIndustry")) {
            return object->InitialMeterValue(METER_TARGET_INDUSTRY);
        } else if (boost::iequals(property_name, "Research")) {
            return object->InitialMeterValue(METER_RESEARCH);
        } else if (boost::iequals(property_name, "TargetResearch")) {
            return object->InitialMeterValue(METER_TARGET_RESEARCH);
        } else if (boost::iequals(property_name, "Trade")) {
            return object->InitialMeterValue(METER_TRADE);
        } else if (boost::iequals(property_name, "TargetTrade")) {
            return object->InitialMeterValue(METER_TARGET_TRADE);
        } else if (boost::iequals(property_name, "Mining")) {
            return object->InitialMeterValue(METER_MINING);
        } else if (boost::iequals(property_name, "TargetMining")) {
            return object->InitialMeterValue(METER_TARGET_MINING);
        } else if (boost::iequals(property_name, "Construction")) {
            return object->InitialMeterValue(METER_CONSTRUCTION);
        } else if (boost::iequals(property_name, "TargetConstruction")) {
            return object->InitialMeterValue(METER_TARGET_CONSTRUCTION);

        } else if (boost::iequals(property_name, "MaxFuel")) {
            return object->InitialMeterValue(METER_MAX_FUEL);
        } else if (boost::iequals(property_name, "Fuel")) {
            return object->InitialMeterValue(METER_FUEL);
        } else if (boost::iequals(property_name, "MaxStructure")) {
            return object->InitialMeterValue(METER_MAX_STRUCTURE);
        } else if (boost::iequals(property_name, "Structure")) {
            return object->InitialMeterValue(METER_STRUCTURE);
        } else if (boost::iequals(property_name, "MaxShield")) {
            return object->InitialMeterValue(METER_MAX_SHIELD);
        } else if (boost::iequals(property_name, "Shield")) {
            return object->InitialMeterValue(METER_SHIELD);
        } else if (boost::iequals(property_name, "MaxDefense")) {
            return object->InitialMeterValue(METER_MAX_DEFENSE);
        } else if (boost::iequals(property_name, "Defense")) {
            return object->InitialMeterValue(METER_DEFENSE);

        } else if (boost::iequals(property_name, "FoodConsumption")) {
            return object->InitialMeterValue(METER_FOOD_CONSUMPTION);
        } else if (boost::iequals(property_name, "Supply")) {
            return object->InitialMeterValue(METER_SUPPLY);
        } else if (boost::iequals(property_name, "Stealth")) {
            return object->InitialMeterValue(METER_STEALTH);
        } else if (boost::iequals(property_name, "Detection")) {
            return object->InitialMeterValue(METER_DETECTION);
        } else if (boost::iequals(property_name, "BattleSpeed")) {
            return object->InitialMeterValue(METER_BATTLE_SPEED);
        } else if (boost::iequals(property_name, "StarlaneSpeed")) {
            return object->InitialMeterValue(METER_STARLANE_SPEED);

        } else if (boost::iequals(property_name, "Damage")) {
            return object->InitialMeterValue(METER_DAMAGE);
        } else if (boost::iequals(property_name, "ROF")) {
            return object->InitialMeterValue(METER_ROF);
        } else if (boost::iequals(property_name, "Range")) {
            return object->InitialMeterValue(METER_RANGE);
        } else if (boost::iequals(property_name, "Speed")) {
            return object->InitialMeterValue(METER_SPEED);
        } else if (boost::iequals(property_name, "Capacity")) {
            return object->InitialMeterValue(METER_CAPACITY);
        } else if (boost::iequals(property_name, "AntiShipDamage")) {
            return object->InitialMeterValue(METER_ANTI_SHIP_DAMAGE);
        } else if (boost::iequals(property_name, "AntiFighterDamage")) {
            return object->InitialMeterValue(METER_ANTI_FIGHTER_DAMAGE);
        } else if (boost::iequals(property_name, "LaunchRate")) {
            return object->InitialMeterValue(METER_LAUNCH_RATE);
        } else if (boost::iequals(property_name, "FighterWeaponRange")) {
            return object->InitialMeterValue(METER_FIGHTER_WEAPON_RANGE);

        } else if (boost::iequals(property_name, "TradeStockpile")) {
            if (object->Owners().size() == 1) {
                Empire* empire = Empires().Lookup(*object->Owners().begin());
                return empire->ResourceStockpile(RE_TRADE);
            }
        } else if (boost::iequals(property_name, "MineralStockpile")) {
            if (object->Owners().size() == 1) {
                Empire* empire = Empires().Lookup(*object->Owners().begin());
                return empire->ResourceStockpile(RE_MINERALS);
            }
        } else if (boost::iequals(property_name, "FoodStockpile")) {
            if (object->Owners().size() == 1) {
                Empire* empire = Empires().Lookup(*object->Owners().begin());
                return empire->ResourceStockpile(RE_FOOD);
            }
        } else if (boost::iequals(property_name, "DistanceToSource")) {
            double delta_x = object->X() - source->X();
            double delta_y = object->Y() - source->Y();
            return std::sqrt(delta_x * delta_x + delta_y * delta_y);

        } else {
            throw std::runtime_error("Attempted to read a non-double value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type double.");
        }

        return 0.0;
    }

    template <>
    int Variable<int>::Eval(const UniverseObject* source, const UniverseObject* target,
                            const boost::any& current_value) const
    {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (!object) {
            Logger().errorStream() << "Variable<int>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
            return 0;
        }
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(int)

        if (boost::iequals(property_name, "Owner")) {
            if (object->Owners().size() == 1)
                return *object->Owners().begin();
            else
                return ALL_EMPIRES;
        } else if (boost::iequals(property_name, "ID")) {
            return object->ID();
        } else if (boost::iequals(property_name, "CreationTurn")) {
            return object->CreationTurn();
        } else if (boost::iequals(property_name, "Age")) {
            return object->AgeInTurns();
        } else if (boost::iequals(property_name, "DesignID")) {
            if (const Ship* ship = universe_object_cast<const Ship*>(object))
                return ship->DesignID();
            else
                return ShipDesign::INVALID_DESIGN_ID;
        } else if (boost::iequals(property_name, "FleetID")) {
            if (const Ship* ship = universe_object_cast<const Ship*>(object))
                return ship->FleetID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (boost::iequals(property_name, "PlanetID")) {
            if (const Building* building = universe_object_cast<const Building*>(object))
                return building->PlanetID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (boost::iequals(property_name, "SystemID")) {
            return object->SystemID();
        } else if (boost::iequals(property_name, "FinalDestinationID")) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->FinalDestinationID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (boost::iequals(property_name, "NextSystemID")) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->NextSystemID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (boost::iequals(property_name, "PreviousSystemID")) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->PreviousSystemID();
            else
                return UniverseObject::INVALID_OBJECT_ID;
        } else if (boost::iequals(property_name, "NumShips")) {
            if (const Fleet* fleet = universe_object_cast<const Fleet*>(object))
                return fleet->NumShips();
            else
                return 0;
        } else if (boost::iequals(property_name, "CurrentTurn")) {
            return CurrentTurn();
        } else {
            throw std::runtime_error("Attempted to read a non-int value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type int.");
        }

        return 0;
    }

    template <>
    std::string Variable<std::string>::Eval(const UniverseObject* source, const UniverseObject* target,
                                            const boost::any& current_value) const
    {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (!object) {
            Logger().errorStream() << "Variable<std::string>::Eval unable to follow reference: " << ReconstructName(m_property_name, m_source_ref);
            return "";
        }
        const std::string& property_name = m_property_name.back();

        IF_CURRENT_VALUE_ELSE(std::string)

        if (boost::iequals(property_name, "Name")) {
            return object->Name();
        } else if (boost::iequals(property_name, "Species")) {
            if (const Planet* planet = universe_object_cast<const Planet*>(object))
                return planet->SpeciesName();
            else if (const Ship* ship = universe_object_cast<const Ship*>(object))
                return ship->SpeciesName();
        } else if (boost::iequals(property_name, "BuildingType")) {
            if (const Building* building = universe_object_cast<const Building*>(object))
                return building->BuildingTypeName();
        } else if (boost::iequals(property_name, "Focus")) {
            if (const Planet* planet = universe_object_cast<const Planet*>(object))
                return planet->Focus();
        } else {
            throw std::runtime_error("Attempted to read a non-string value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type std::string.");
        }

        return "";
    }

#undef IF_CURRENT_VALUE_ELSE
}

///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////
namespace ValueRef {
    template <>
    double Statistic<double>::Eval(const UniverseObject* source, const UniverseObject* target,
                                   const boost::any& current_value) const
    {
        Condition::ObjectSet condition_matches;
        GetConditionMatches(source, condition_matches, m_sampling_condition);

        if (m_stat_type == NUMBER)
            return static_cast<double>(condition_matches.size());

        // evaluate property for each condition-matched object
        std::map<const UniverseObject*, double> object_property_values;
        GetObjectPropertyValues(source, condition_matches, current_value, object_property_values);

        return ReduceData(object_property_values);
    }

    template <>
    int Statistic<int>::Eval(const UniverseObject* source, const UniverseObject* target,
                             const boost::any& current_value) const
    {
        Condition::ObjectSet condition_matches;
        GetConditionMatches(source, condition_matches, m_sampling_condition);

        if (m_stat_type == NUMBER)
            return static_cast<int>(condition_matches.size());

        // evaluate property for each condition-matched object
        std::map<const UniverseObject*, int> object_property_values;
        GetObjectPropertyValues(source, condition_matches, current_value, object_property_values);

        return ReduceData(object_property_values);
    }

    template <>
    std::string Statistic<std::string>::Eval(const UniverseObject* source, const UniverseObject* target,
                                             const boost::any& current_value) const
    {
        // the only statistic that can be computed on non-number property types
        // and that is itself of a non-number type is the most common value
        if (m_stat_type != MODE)
            throw std::runtime_error("ValueRef evaluated with an invalid StatisticType for the return type (string).");

        Condition::ObjectSet condition_matches;
        GetConditionMatches(source, condition_matches, m_sampling_condition);

        if (condition_matches.empty())
            return "";

        // evaluate property for each condition-matched object
        std::map<const UniverseObject*, std::string> object_property_values;
        GetObjectPropertyValues(source, condition_matches, current_value, object_property_values);

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
    std::string Operation<std::string>::Eval(const UniverseObject* source, const UniverseObject* target,
                                             const boost::any& current_value) const
    {
        std::string op_link;
        switch (m_op_type) {
            case PLUS:      op_link = " + ";    break;
            case MINUS:     op_link = " - ";    break;
            case TIMES:     op_link = " * ";    break;
            case DIVIDES:   op_link = " / ";    break;
            case NEGATE:
                return "-" + m_operand1->Eval(source, target, current_value);
                break;
            default:
                throw std::runtime_error("std::string ValueRef evaluated with an unknown OpType.");
                break;
        }
        return m_operand1->Eval(source, target, current_value)
               + op_link +
               m_operand2->Eval(source, target, current_value);
    }
}

std::string DumpIndent()
{
    return std::string(g_indent * 4, ' ');
}
