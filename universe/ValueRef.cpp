#include "ValueRef.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "Planet.h"
#include "System.h"
#include "UniverseObject.h"
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
            std::string property_name = *first;
            if (boost::iequals(property_name, "Planet")) {
                if (const Building* b = universe_object_cast<const Building*>(obj)) {
                    obj = objects.Object<Planet>(b->PlanetID());
                } else {
                    obj = 0;
                }
            } else if (boost::iequals(property_name, "System")) {
                if (obj)
                    obj = objects.Object<System>(obj->SystemID());
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
        default:            return "?";
        }
    }

    template <>
    std::string Constant<FocusType>::Dump() const
    {
        switch (m_value) {
        case INVALID_FOCUS_TYPE:    return "Unknown";
        case FOCUS_BALANCED:        return "Balanced";
        case FOCUS_FARMING:         return "Farming";
        case FOCUS_INDUSTRY:        return "Industry";
        case FOCUS_MINING:          return "Mining";
        case FOCUS_RESEARCH:        return "Research";
        case FOCUS_TRADE:           return "Trade";
        default:                    return "?";
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
}


///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
namespace ValueRef {

#define IF_CURRENT_VALUE_ELSE(T)                                        \
    if (boost::iequals(property_name, "Value")) {                       \
        if (current_value.empty())                                      \
            throw std::runtime_error(                                   \
                "Variable<" #T ">::Eval(): Value could not be evaluated, " \
                "because no current value was provided.");              \
        try {                                                           \
            retval = boost::any_cast<T>(current_value);                 \
        } catch (const boost::bad_any_cast&) {                          \
            throw std::runtime_error(                                   \
                "Variable<" #T ">::Eval(): Value could not be evaluated, " \
                "because the provided current value is not an " #T "."); \
        }                                                               \
    } else

    template <>
    PlanetSize Variable<PlanetSize>::Eval(const UniverseObject* source, const UniverseObject* target,
                                          const boost::any& current_value) const
    {
        PlanetSize retval = INVALID_PLANET_SIZE;
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(PlanetSize)
        if (boost::iequals(property_name, "PlanetSize")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                retval = p->Size();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetSize value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetSize.");
        }
        return retval;
    }

    template <>
    PlanetType Variable<PlanetType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                          const boost::any& current_value) const
    {
        PlanetType retval = INVALID_PLANET_TYPE;
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(PlanetType)
        if (boost::iequals(property_name, "PlanetType")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                retval = p->Type();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetType.");
        }
        return retval;
    }

    template <>
    PlanetEnvironment Variable<PlanetEnvironment>::Eval(const UniverseObject* source, const UniverseObject* target,
                                                        const boost::any& current_value) const
    {
        PlanetEnvironment retval = INVALID_PLANET_ENVIRONMENT;
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(PlanetEnvironment)
        if (boost::iequals(property_name, "PlanetEnvironment")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (const Planet* p = universe_object_cast<const Planet*>(object))
                retval = p->Environment();
        } else {
            throw std::runtime_error("Attempted to read a non-PlanetEnvironment value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetEnvironment.");
        }
        return retval;
    }

    template <>
    UniverseObjectType Variable<UniverseObjectType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                                          const boost::any& current_value) const
    {
        UniverseObjectType retval = INVALID_UNIVERSE_OBJECT_TYPE;
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(UniverseObjectType)
        if (boost::iequals(property_name, "ObjectType")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
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
        return retval;
    }

    template <>
    StarType Variable<StarType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                      const boost::any& current_value) const
    {
        StarType retval = INVALID_STAR_TYPE;
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(StarType)
        if (boost::iequals(property_name, "StarType")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (const System* s = universe_object_cast<const System*>(object))
                retval = s->GetStarType();
        } else {
            throw std::runtime_error("Attempted to read a non-StarType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type StarType.");
        }
        return retval;
    }

    template <>
    FocusType Variable<FocusType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                        const boost::any& current_value) const
    {
        FocusType retval = INVALID_FOCUS_TYPE;
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(FocusType)
        if (boost::iequals(property_name, "PrimaryFocus")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (const ResourceCenter* pc = dynamic_cast<const ResourceCenter*>(object))
                retval = pc->PrimaryFocus();
        } else if (boost::iequals(property_name, "SecondaryFocus")) {
            const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
            if (const ResourceCenter* pc = dynamic_cast<const ResourceCenter*>(object))
                retval = pc->SecondaryFocus();
        } else {
            throw std::runtime_error("Attempted to read a non-FocusType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type FocusType.");
        }
        return retval;
    }

    template <>
    double Variable<double>::Eval(const UniverseObject* source, const UniverseObject* target,
                                  const boost::any& current_value) const
    {
        double retval = 0.0;
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(double)
        if        (boost::iequals(property_name, "Population")) {
            retval = object->InitialMeterValue(METER_POPULATION);
        } else if (boost::iequals(property_name, "TargetPopulation")) {
            retval = object->InitialMeterValue(METER_TARGET_POPULATION);
        } else if (boost::iequals(property_name, "Health")) {
            retval = object->InitialMeterValue(METER_HEALTH);
        } else if (boost::iequals(property_name, "TargetHealth")) {
            retval = object->InitialMeterValue(METER_TARGET_HEALTH);

        } else if (boost::iequals(property_name, "Farming")) {
            retval = object->InitialMeterValue(METER_FARMING);
        } else if (boost::iequals(property_name, "TargetFarming")) {
            retval = object->InitialMeterValue(METER_TARGET_FARMING);
        } else if (boost::iequals(property_name, "Industry")) {
            retval = object->InitialMeterValue(METER_INDUSTRY);
        } else if (boost::iequals(property_name, "TargetIndustry")) {
            retval = object->InitialMeterValue(METER_TARGET_INDUSTRY);
        } else if (boost::iequals(property_name, "Research")) {
            retval = object->InitialMeterValue(METER_RESEARCH);
        } else if (boost::iequals(property_name, "TargetResearch")) {
            retval = object->InitialMeterValue(METER_TARGET_RESEARCH);
        } else if (boost::iequals(property_name, "Trade")) {
            retval = object->InitialMeterValue(METER_TRADE);
        } else if (boost::iequals(property_name, "TargetTrade")) {
            retval = object->InitialMeterValue(METER_TARGET_TRADE);
        } else if (boost::iequals(property_name, "Mining")) {
            retval = object->InitialMeterValue(METER_MINING);
        } else if (boost::iequals(property_name, "TargetMining")) {
            retval = object->InitialMeterValue(METER_TARGET_MINING);
        } else if (boost::iequals(property_name, "Construction")) {
            retval = object->InitialMeterValue(METER_CONSTRUCTION);
        } else if (boost::iequals(property_name, "TargetConstruction")) {
            retval = object->InitialMeterValue(METER_TARGET_CONSTRUCTION);

        } else if (boost::iequals(property_name, "MaxFuel")) {
            retval = object->InitialMeterValue(METER_MAX_FUEL);
        } else if (boost::iequals(property_name, "Fuel")) {
            retval = object->InitialMeterValue(METER_FUEL);
        } else if (boost::iequals(property_name, "MaxStructure")) {
            retval = object->InitialMeterValue(METER_MAX_STRUCTURE);
        } else if (boost::iequals(property_name, "Structure")) {
            retval = object->InitialMeterValue(METER_STRUCTURE);
        } else if (boost::iequals(property_name, "MaxShield")) {
            retval = object->InitialMeterValue(METER_MAX_SHIELD);
        } else if (boost::iequals(property_name, "Shield")) {
            retval = object->InitialMeterValue(METER_SHIELD);
        } else if (boost::iequals(property_name, "MaxDefense")) {
            retval = object->InitialMeterValue(METER_MAX_DEFENSE);
        } else if (boost::iequals(property_name, "Defense")) {
            retval = object->InitialMeterValue(METER_DEFENSE);

        } else if (boost::iequals(property_name, "Supply")) {
            retval = object->InitialMeterValue(METER_SUPPLY);
        } else if (boost::iequals(property_name, "Stealth")) {
            retval = object->InitialMeterValue(METER_STEALTH);
        } else if (boost::iequals(property_name, "Detection")) {
            retval = object->InitialMeterValue(METER_DETECTION);
        } else if (boost::iequals(property_name, "BattleSpeed")) {
            retval = object->InitialMeterValue(METER_BATTLE_SPEED);
        } else if (boost::iequals(property_name, "StarlaneSpeed")) {
            retval = object->InitialMeterValue(METER_STARLANE_SPEED);

        } else if (boost::iequals(property_name, "Damage")) {
            retval = object->InitialMeterValue(METER_DAMAGE);
        } else if (boost::iequals(property_name, "ROF")) {
            retval = object->InitialMeterValue(METER_ROF);
        } else if (boost::iequals(property_name, "Range")) {
            retval = object->InitialMeterValue(METER_RANGE);
        } else if (boost::iequals(property_name, "Speed")) {
            retval = object->InitialMeterValue(METER_SPEED);
        } else if (boost::iequals(property_name, "Capacity")) {
            retval = object->InitialMeterValue(METER_CAPACITY);
        } else if (boost::iequals(property_name, "AntiShipDamage")) {
            retval = object->InitialMeterValue(METER_ANTI_SHIP_DAMAGE);
        } else if (boost::iequals(property_name, "AntiFighterDamage")) {
            retval = object->InitialMeterValue(METER_ANTI_FIGHTER_DAMAGE);
        } else if (boost::iequals(property_name, "LaunchRate")) {
            retval = object->InitialMeterValue(METER_LAUNCH_RATE);
        } else if (boost::iequals(property_name, "FighterWeaponRange")) {
            retval = object->InitialMeterValue(METER_FIGHTER_WEAPON_RANGE);

        } else if (boost::iequals(property_name, "TradeStockpile")) {
            if (object->Owners().size() == 1) {
                Empire* empire = Empires().Lookup(*object->Owners().begin());
                retval = empire->ResourceStockpile(RE_TRADE);
            }
        } else if (boost::iequals(property_name, "MineralStockpile")) {
            if (object->Owners().size() == 1) {
                Empire* empire = Empires().Lookup(*object->Owners().begin());
                retval = empire->ResourceStockpile(RE_MINERALS);
            }
        } else if (boost::iequals(property_name, "FoodStockpile")) {
            if (object->Owners().size() == 1) {
                Empire* empire = Empires().Lookup(*object->Owners().begin());
                retval = empire->ResourceStockpile(RE_FOOD);
            }
        } else {
            throw std::runtime_error("Attempted to read a non-double value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type double.");
        }

        return retval;
    }

    template <>
    int Variable<int>::Eval(const UniverseObject* source, const UniverseObject* target,
                            const boost::any& current_value) const
    {
        int retval = 0;
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        std::string property_name = m_property_name.back();
        IF_CURRENT_VALUE_ELSE(int)
        if (boost::iequals(property_name, "Owner")) {
            if (object->Owners().size() == 1)
                retval = *object->Owners().begin();
            else
                retval = -1;
        } else if (boost::iequals(property_name, "ID")) {
            retval = object->ID();
        } else if (boost::iequals(property_name, "CurrentTurn")) {
            retval = CurrentTurn();
        } else if (boost::iequals(property_name, "CreationTurn")) {
            retval = object->CreationTurn();
        } else if (boost::iequals(property_name, "Age")) {
            retval = object->AgeInTurns();
        } else if (boost::iequals(property_name, "CurrentTurn")) {
            retval = CurrentTurn();
        } else {
            throw std::runtime_error("Attempted to read a non-int value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type int.");
        }

        return retval;
    }

#undef IF_CURRENT_VALUE_ELSE
}

std::string DumpIndent()
{
    return std::string(g_indent * 4, ' ');
}
