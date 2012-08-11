#include "SitRepEntry.h"

#include "MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/Building.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/Fleet.h"

namespace {
    const std::string SITREP_UPDATE_TAG = "SitRepUpdate";
}

SitRepEntry::SitRepEntry() :
    VarText(),
    m_turn(INVALID_GAME_TURN)
{}

SitRepEntry::SitRepEntry(const std::string& template_string) :
    VarText(template_string, true),
    m_turn(CurrentTurn()+1) // sitreps typically created by server before incrementing the turn counter, so they first appear the turn after when CurrentTurn indicates
{}

SitRepEntry::SitRepEntry(const std::string& template_string, int turn) :
    VarText(template_string, true),
    m_turn(turn)
{}

int SitRepEntry::GetDataIDNumber(const std::string& tag) const {
    if (!m_variables.ContainsChild(tag))
        return -1;
    const XMLElement& token_elem = m_variables.Child(tag);
    try {
        const std::string& text = token_elem.Attribute("value");
        return boost::lexical_cast<int>(text);
    } catch (...) {
        return -1;
    }
    return -1;
}

const std::string& SitRepEntry::GetDataString(const std::string& tag) const {
    static const std::string EMPTY_STRING;
    if (!m_variables.ContainsChild(tag))
        return EMPTY_STRING;
    const XMLElement& token_elem = m_variables.Child(tag);
    return token_elem.Attribute("value");
    return EMPTY_STRING;
}

std::string SitRepEntry::Dump() const {
    std::string retval = "SitRep template_string = \"" + m_template_string + "\"";
    if (m_variables.NumChildren() > 0) {
        for (XMLElement::const_child_iterator it = m_variables.child_begin(); it != m_variables.child_end(); ++it)
            retval += " " + it->Tag() + " = " + it->Attribute("value");
    }
    retval += " turn = " + boost::lexical_cast<std::string>(m_turn);
    return retval;
}

SitRepEntry* CreateTechResearchedSitRep(const std::string& tech_name) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_TECH_RESEARCHED");
    sitrep->AddVariable(VarText::TECH_TAG,          tech_name);
    return sitrep;
}

SitRepEntry* CreateShipBuiltSitRep(int ship_id, int system_id, int shipdesign_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_SHIP_BUILT");
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    sitrep->AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(ship_id));
    sitrep->AddVariable(VarText::DESIGN_ID_TAG,     boost::lexical_cast<std::string>(shipdesign_id));
    return sitrep;
}

SitRepEntry* CreateBuildingBuiltSitRep(int building_id, int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_BUILDING_BUILT");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    sitrep->AddVariable(VarText::BUILDING_ID_TAG,   boost::lexical_cast<std::string>(building_id));
    return sitrep;
}

SitRepEntry* CreateCombatSitRep(int system_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_COMBAT_SYSTEM");
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    return sitrep;
}

SitRepEntry* CreateGroundCombatSitRep(int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_GROUND_BATTLE");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    return sitrep;
}

SitRepEntry* CreatePlanetCapturedSitRep(int planet_id, int empire_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_PLANET_CAPTURED");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return sitrep;
}

namespace {
    SitRepEntry* GenericCombatDamagedObjectSitrep(int combat_system_id) {
        SitRepEntry* sitrep = new SitRepEntry("SITREP_OBJECT_DAMAGED_AT_SYSTEM");
        sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));
        return sitrep;
    }

    SitRepEntry* GenericCombatDestroyedObjectSitrep(int combat_system_id) {
        SitRepEntry* sitrep = new SitRepEntry("SITREP_OBJECT_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));
        return sitrep;
    }
}

SitRepEntry* CreateCombatDamagedObjectSitRep(int object_id, int combat_system_id, int empire_id) {
    const UniverseObject* obj = GetUniverse().EmpireKnownObjects(empire_id).Object(object_id);
    if (!obj)
        return GenericCombatDamagedObjectSitrep(combat_system_id);

    SitRepEntry* sitrep(0);

    if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
        if (ship->Unowned())
            sitrep = new SitRepEntry("SITREP_UNOWNED_SHIP_DAMAGED_AT_SYSTEM");
        else
            sitrep = new SitRepEntry("SITREP_SHIP_DAMAGED_AT_SYSTEM");
        sitrep->AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(object_id));
        sitrep->AddVariable(VarText::DESIGN_ID_TAG,     boost::lexical_cast<std::string>(ship->DesignID()));

    } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
        if (planet->Unowned())
            sitrep = new SitRepEntry("SITREP_UNOWNED_PLANET_BOMBARDED_AT_SYSTEM");
        else
            sitrep = new SitRepEntry("SITREP_PLANET_BOMBARDED_AT_SYSTEM");
        sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(object_id));

    } else {
        sitrep = GenericCombatDestroyedObjectSitrep(combat_system_id);
    }

    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(obj->Owner()));
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));

    return sitrep;
}

SitRepEntry* CreateCombatDestroyedObjectSitRep(int object_id, int combat_system_id, int empire_id) {
    const UniverseObject* obj = GetUniverse().EmpireKnownObjects(empire_id).Object(object_id);
    if (!obj)
        return GenericCombatDestroyedObjectSitrep(combat_system_id);

    SitRepEntry* sitrep(0);

    if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
        if (ship->Unowned())
            sitrep = new SitRepEntry("SITREP_UNOWNED_SHIP_DESTROYED_AT_SYSTEM");
        else
            sitrep = new SitRepEntry("SITREP_SHIP_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(object_id));
        sitrep->AddVariable(VarText::DESIGN_ID_TAG,     boost::lexical_cast<std::string>(ship->DesignID()));

    } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
        if (fleet->Unowned())
            sitrep = new SitRepEntry("SITREP_UNOWNED_FLEET_DESTROYED_AT_SYSTEM");
        else
            sitrep = new SitRepEntry("SITREP_FLEET_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(object_id));

    } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
        if (planet->Unowned())
            sitrep = new SitRepEntry("SITREP_UNOWNED_PLANET_DESTROYED_AT_SYSTEM");
        else
            sitrep = new SitRepEntry("SITREP_PLANET_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(object_id));

    } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
        if (building->Unowned())
            sitrep = new SitRepEntry("SITREP_UNOWNED_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM");
        else
            sitrep = new SitRepEntry("SITREP_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM");
        sitrep->AddVariable(VarText::BUILDING_ID_TAG,   boost::lexical_cast<std::string>(object_id));
        sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(building->PlanetID()));
    } else {
        sitrep = GenericCombatDestroyedObjectSitrep(combat_system_id);
    }

    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(obj->Owner()));
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));

    return sitrep;
}

SitRepEntry* CreatePlanetStarvedToDeathSitRep(int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_PLANET_LOST_STARVED_TO_DEATH");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    return sitrep;
}

SitRepEntry* CreatePlanetColonizedSitRep(int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_PLANET_COLONIZED");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    return sitrep;
}

SitRepEntry* CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_FLEET_ARRIVED_AT_DESTINATION");
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    sitrep->AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(fleet_id));
    return sitrep;
}

SitRepEntry* CreateEmpireEliminatedSitRep(int empire_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_EMPIRE_ELIMINATED");
    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return sitrep;
}

SitRepEntry* CreateVictorySitRep(const std::string& reason_string, int empire_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_VICTORY");
    sitrep->AddVariable(VarText::TEXT_TAG,          reason_string);
    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return sitrep;
}

SitRepEntry* CreateSitRep(const std::string& template_string, const std::vector<std::pair<std::string, std::string> >& parameters) {
    SitRepEntry* sitrep = new SitRepEntry(template_string);
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = parameters.begin();
         it != parameters.end();
         ++it)
    {
        sitrep->AddVariable(it->first, it->second);
    }
    return sitrep;
}

