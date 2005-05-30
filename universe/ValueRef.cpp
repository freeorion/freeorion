#include "ValueRef.h"

#include "Building.h"
#include "Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "Planet.h"
#include "System.h"
#include "UniverseObject.h"

#include <boost/spirit.hpp>

namespace detail {
    std::vector<std::string> TokenizeDottedReference(const std::string& str)
    {
        using namespace boost::spirit;
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
        while (first != last) {
            if (*first == "Planet") {
                if (const Building* b = universe_object_cast<const Building*>(obj)) {
                    obj = b->GetPlanet();
                } else {
                    obj = 0;
                }
            } else if (*first == "System") {
                obj = obj->GetSystem();
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

    bool temp_header_bool = RecordHeaderFile(ValueRefRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <>
PlanetSize ValueRef::Variable<PlanetSize>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    PlanetSize retval = INVALID_PLANET_SIZE;
    if (m_property_name.back() == "PlanetSize") {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (const Planet* p = universe_object_cast<const Planet*>(object))
            retval = p->Size();
    } else {
        throw std::runtime_error("Attempted to read a non-PlanetSize value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetSize.");
    }
    return retval;
}

template <>
PlanetType ValueRef::Variable<PlanetType>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    PlanetType retval = INVALID_PLANET_TYPE;
    if (m_property_name.back() == "PlanetType") {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (const Planet* p = universe_object_cast<const Planet*>(object))
            retval = p->Type();
    } else {
        throw std::runtime_error("Attempted to read a non-PlanetType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetType.");
    }
    return retval;
}

template <>
PlanetEnvironment ValueRef::Variable<PlanetEnvironment>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    PlanetEnvironment retval = INVALID_PLANET_ENVIRONMENT;
    if (m_property_name.back() == "PlanetEnvironment") {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (const Planet* p = universe_object_cast<const Planet*>(object))
            retval = p->Environment();
    } else {
        throw std::runtime_error("Attempted to read a non-PlanetEnvironment value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type PlanetEnvironment.");
    }
    return retval;
}

template <>
UniverseObjectType ValueRef::Variable<UniverseObjectType>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    UniverseObjectType retval = INVALID_UNIVERSE_OBJECT_TYPE;
    if (m_property_name.back() == "ObjectType") {
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
StarType ValueRef::Variable<StarType>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    StarType retval = INVALID_STAR_TYPE;
    if (m_property_name.back() == "StarType") {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (const System* s = universe_object_cast<const System*>(object))
            retval = s->Star();
    } else {
        throw std::runtime_error("Attempted to read a non-StarType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type StarType.");
    }
    return retval;
}

template <>
FocusType ValueRef::Variable<FocusType>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    FocusType retval = INVALID_FOCUS_TYPE;
    if (m_property_name.back() == "PrimaryFocus") {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (const ResourceCenter* pc = dynamic_cast<const ResourceCenter*>(object))
            retval = pc->PrimaryFocus();
    }else if (m_property_name.back() == "SecondaryFocus") {
        const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);
        if (const ResourceCenter* pc = dynamic_cast<const ResourceCenter*>(object))
            retval = pc->SecondaryFocus();
    } else {
        throw std::runtime_error("Attempted to read a non-FocusType value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type FocusType.");
    }
    return retval;
}

template <>
double ValueRef::Variable<double>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    double retval = 0.0;

    const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);

    if (m_property_name.back() == "CurrentFarming") {
        const Meter* m = object->GetMeter(METER_FARMING);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxFarming") {
        const Meter* m = object->GetMeter(METER_FARMING);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "CurrentIndustry") {
        const Meter* m = object->GetMeter(METER_INDUSTRY);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxIndustry") {
        const Meter* m = object->GetMeter(METER_INDUSTRY);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "CurrentResearch") {
        const Meter* m = object->GetMeter(METER_RESEARCH);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxResearch") {
        const Meter* m = object->GetMeter(METER_RESEARCH);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "CurrentTrade") {
        const Meter* m = object->GetMeter(METER_TRADE);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxTrade") {
        const Meter* m = object->GetMeter(METER_TRADE);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "CurrentMining") {
        const Meter* m = object->GetMeter(METER_MINING);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxMining") {
        const Meter* m = object->GetMeter(METER_MINING);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "CurrentConstruction") {
        const Meter* m = object->GetMeter(METER_CONSTRUCTION);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxConstruction") {
        const Meter* m = object->GetMeter(METER_CONSTRUCTION);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "CurrentHealth") {
        const Meter* m = object->GetMeter(METER_HEALTH);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxHealth") {
        const Meter* m = object->GetMeter(METER_HEALTH);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "CurrentPopulation") {
        const Meter* m = object->GetMeter(METER_POPULATION);
        retval = m ? m->Current() : 0;
    } else if (m_property_name.back() == "MaxPopulation") {
        const Meter* m = object->GetMeter(METER_POPULATION);
        retval = m ? m->Max() : 0;
    } else if (m_property_name.back() == "TradeStockpile") {
        if (object->Owners().size() == 1) {
            Empire* empire = Empires().Lookup(*object->Owners().begin());
            retval = empire->TradeResPool().Stockpile();
        }
    } else if (m_property_name.back() == "MineralStockpile") {
        if (object->Owners().size() == 1) {
            Empire* empire = Empires().Lookup(*object->Owners().begin());
            retval = empire->MineralResPool().Stockpile();
        }
    } else if (m_property_name.back() == "FoodStockpile") {
        if (object->Owners().size() == 1) {
            Empire* empire = Empires().Lookup(*object->Owners().begin());
            retval = empire->FoodResPool().Stockpile();
        }
    } else if (m_property_name.back() == "TradeProduction") {
        if (const ResourceCenter* prod_center = dynamic_cast<const ResourceCenter*>(object)) {
            retval = prod_center->TradePoints();
        }
    } else if (m_property_name.back() == "FoodProduction") {
        if (const ResourceCenter* prod_center = dynamic_cast<const ResourceCenter*>(object)) {
            retval = prod_center->FarmingPoints();
        }
    } else if (m_property_name.back() == "MineralProduction") {
        if (const ResourceCenter* prod_center = dynamic_cast<const ResourceCenter*>(object)) {
            retval = prod_center->MiningPoints();
        }
    } else if (m_property_name.back() == "IndustryProduction") {
        if (const ResourceCenter* prod_center = dynamic_cast<const ResourceCenter*>(object)) {
            retval = prod_center->IndustryPoints();
        }
    } else if (m_property_name.back() == "ResearchProduction") {
        if (const ResourceCenter* prod_center = dynamic_cast<const ResourceCenter*>(object)) {
            retval = prod_center->ResearchPoints();
        }
    } else {
        throw std::runtime_error("Attempted to read a non-double value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type double.");
    }

    return retval;
}

template <>
int ValueRef::Variable<int>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    int retval = 0;

    const UniverseObject* object = FollowReference(m_property_name.begin(), m_property_name.end(), m_source_ref ? source : target);

    if (m_property_name.back() == "Owner") {
        if (object->Owners().size() == 1)
            retval = *object->Owners().begin();
        else
            retval = -1;
    } else if (m_property_name.back() == "ID") {
        retval = object->ID();
    } else {
        throw std::runtime_error("Attempted to read a non-int value \"" + ReconstructName(m_property_name, m_source_ref) + "\" using a ValueRef of type int.");
    }

    return retval;
}
