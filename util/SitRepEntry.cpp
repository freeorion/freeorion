#include "SitRepEntry.h"

#include "i18n.h"
#include "Logger.h"
#include "../universe/Predicates.h"
#include "../universe/Building.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/Fleet.h"
#include "../universe/Universe.h"

SitRepEntry::SitRepEntry() :
    VarText(),
    m_turn(INVALID_GAME_TURN),
    m_icon("/icons/sitrep/generic.png"),
    m_label()
{}

SitRepEntry::SitRepEntry(const std::string& template_string, const std::string& icon) :
    VarText(template_string, true),
    m_turn(CurrentTurn()+1), // sitreps typically created by server before incrementing the turn counter, so they first appear the turn after when CurrentTurn indicates
    m_icon(icon.empty() ? "/icons/sitrep/generic.png" : icon),
    m_label()
{}

SitRepEntry::SitRepEntry(const std::string& template_string, int turn, const std::string& icon) :
    VarText(template_string, true),
    m_turn(turn),
    m_icon(icon.empty() ? "/icons/sitrep/generic.png" : icon),
    m_label()
{}

SitRepEntry::SitRepEntry(const std::string& template_string, const std::string& icon, const std::string label, bool stringtable_lookup) :
    VarText(template_string, stringtable_lookup),
    m_turn(CurrentTurn()+1), // sitreps typically created by server before incrementing the turn counter, so they first appear the turn after when CurrentTurn indicates
    m_icon(icon.empty() ? "/icons/sitrep/generic.png" : icon),
    m_label(label)
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
    retval += " icon = " + m_icon;
    retval += " label = " + m_label;
    return retval;
}

SitRepEntry CreateTechResearchedSitRep(const std::string& tech_name) {
    SitRepEntry sitrep(UserStringNop("SITREP_TECH_RESEARCHED"), "icons/sitrep/tech_researched.png");
    sitrep.AddVariable(VarText::TECH_TAG,          tech_name);
    return sitrep;
}

SitRepEntry CreateShipBuiltSitRep(int ship_id, int system_id, int shipdesign_id) {
    SitRepEntry sitrep(UserStringNop("SITREP_SHIP_BUILT"), "icons/sitrep/ship_produced.png");
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    sitrep.AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(ship_id));
    sitrep.AddVariable(VarText::DESIGN_ID_TAG,     boost::lexical_cast<std::string>(shipdesign_id));
    return sitrep;
}

SitRepEntry CreateShipBlockBuiltSitRep(int system_id, int shipdesign_id, int number) {
    SitRepEntry sitrep(UserStringNop("SITREP_SHIP_BATCH_BUILT"), "icons/sitrep/ship_produced.png");
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    sitrep.AddVariable(VarText::DESIGN_ID_TAG,     boost::lexical_cast<std::string>(shipdesign_id));
    sitrep.AddVariable(VarText::RAW_TEXT_TAG,      boost::lexical_cast<std::string>(number));
    return sitrep;
}

SitRepEntry CreateBuildingBuiltSitRep(int building_id, int planet_id) {
    SitRepEntry sitrep(UserStringNop("SITREP_BUILDING_BUILT"), "icons/sitrep/building_produced.png");
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    sitrep.AddVariable(VarText::BUILDING_ID_TAG,   boost::lexical_cast<std::string>(building_id));
    return sitrep;
}

SitRepEntry CreateTechUnlockedSitRep(const std::string& tech_name) {
    SitRepEntry sitrep(UserStringNop("SITREP_TECH_UNLOCKED"), "icons/sitrep/tech_unlocked.png");
    sitrep.AddVariable(VarText::TECH_TAG,          tech_name);
    return sitrep;
}

SitRepEntry CreateBuildingTypeUnlockedSitRep(const std::string& building_type_name) {
    SitRepEntry sitrep(UserStringNop("SITREP_BUILDING_TYPE_UNLOCKED"), "icons/sitrep/building_type_unlocked.png");
    sitrep.AddVariable(VarText::BUILDING_TYPE_TAG,  building_type_name);
    return sitrep;
}

SitRepEntry CreateShipHullUnlockedSitRep(const std::string& ship_hull_name) {
    SitRepEntry sitrep(UserStringNop("SITREP_SHIP_HULL_UNLOCKED"), "icons/sitrep/ship_hull_unlocked.png");
    sitrep.AddVariable(VarText::SHIP_HULL_TAG,      ship_hull_name);
    return sitrep;
}

SitRepEntry CreateShipPartUnlockedSitRep(const std::string& ship_part_name) {
    SitRepEntry sitrep(UserStringNop("SITREP_SHIP_PART_UNLOCKED"), "icons/sitrep/ship_part_unlocked.png");
    sitrep.AddVariable(VarText::SHIP_PART_TAG,      ship_part_name);
    return sitrep;
}

SitRepEntry CreateCombatSitRep(int system_id, int log_id, int enemy_id) {
    SitRepEntry sitrep(enemy_id == ALL_EMPIRES ? UserStringNop("SITREP_COMBAT_SYSTEM") : UserStringNop("SITREP_COMBAT_SYSTEM_ENEMY"),
                       "icons/sitrep/combat.png");
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  boost::lexical_cast<std::string>(system_id));
    sitrep.AddVariable(VarText::COMBAT_ID_TAG,  boost::lexical_cast<std::string>(log_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,  boost::lexical_cast<std::string>(enemy_id));
    return sitrep;
}

SitRepEntry CreateGroundCombatSitRep(int planet_id, int enemy_id) {
    SitRepEntry sitrep(enemy_id == ALL_EMPIRES ? UserStringNop("SITREP_GROUND_BATTLE") : UserStringNop("SITREP_GROUND_BATTLE_ENEMY"),
                       "icons/sitrep/ground_combat.png");
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(enemy_id));
    return sitrep;
}

SitRepEntry CreatePlanetCapturedSitRep(int planet_id, int empire_id) {
    SitRepEntry sitrep(UserStringNop("SITREP_PLANET_CAPTURED"), "icons/sitrep/planet_captured.png");
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return sitrep;
}

namespace {
    SitRepEntry GenericCombatDamagedObjectSitrep(int combat_system_id) {
        SitRepEntry sitrep(UserStringNop("SITREP_OBJECT_DAMAGED_AT_SYSTEM"), "icons/sitrep/combat_damage.png");
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));
        return sitrep;
    }

    SitRepEntry GenericCombatDestroyedObjectSitrep(int combat_system_id) {
        SitRepEntry sitrep(UserStringNop("SITREP_OBJECT_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));
        return sitrep;
    }
}

SitRepEntry CreateCombatDamagedObjectSitRep(int object_id, int combat_system_id, int empire_id) {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(object_id);
    if (!obj)
        return GenericCombatDamagedObjectSitrep(combat_system_id);

    SitRepEntry sitrep;

    if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(obj)) {
        if (ship->Unowned())
            sitrep = SitRepEntry(UserStringNop("SITREP_UNOWNED_SHIP_DAMAGED_AT_SYSTEM"), "icons/sitrep/combat_damage.png");
        else
            sitrep = SitRepEntry(UserStringNop("SITREP_SHIP_DAMAGED_AT_SYSTEM"), "icons/sitrep/combat_damage.png");
        sitrep.AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(object_id));
        sitrep.AddVariable(VarText::DESIGN_ID_TAG,     boost::lexical_cast<std::string>(ship->DesignID()));

    } else if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(obj)) {
        if (planet->Unowned())
            sitrep = SitRepEntry(UserStringNop("SITREP_UNOWNED_PLANET_BOMBARDED_AT_SYSTEM"), "icons/sitrep/colony_bombarded.png");
        else
            sitrep = SitRepEntry(UserStringNop("SITREP_PLANET_BOMBARDED_AT_SYSTEM"), "icons/sitrep/colony_bombarded_own.png");
        sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(object_id));

    } else {
        sitrep = GenericCombatDestroyedObjectSitrep(combat_system_id);
    }

    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(obj->Owner()));
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));

    return sitrep;
}

SitRepEntry CreateCombatDestroyedObjectSitRep(int object_id, int combat_system_id, int empire_id) {
    TemporaryPtr<const UniverseObject> obj = GetEmpireKnownObject(object_id, empire_id);
    if (!obj) {
        DebugLogger() << "Object " << object_id << " does not exist!!!";
        return GenericCombatDestroyedObjectSitrep(combat_system_id);
    }

    SitRepEntry sitrep;

    if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(obj)) {
        if (ship->Unowned())
            sitrep = SitRepEntry(UserStringNop("SITREP_UNOWNED_SHIP_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        else if (ship->OwnedBy(empire_id))
            sitrep = SitRepEntry(UserStringNop("SITREP_OWN_SHIP_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        else
            sitrep = SitRepEntry(UserStringNop("SITREP_SHIP_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        sitrep.AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(object_id));
        sitrep.AddVariable(VarText::DESIGN_ID_TAG,     boost::lexical_cast<std::string>(ship->DesignID()));

    } else if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(obj)) {
        if (fleet->Unowned())
            sitrep = SitRepEntry(UserStringNop("SITREP_UNOWNED_FLEET_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        else
            sitrep = SitRepEntry(UserStringNop("SITREP_FLEET_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        sitrep.AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(object_id));

    } else if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(obj)) {
        if (planet->Unowned())
            sitrep = SitRepEntry(UserStringNop("SITREP_UNOWNED_PLANET_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        else
            sitrep = SitRepEntry(UserStringNop("SITREP_PLANET_DESTROYED_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(object_id));

    } else if (TemporaryPtr<const Building> building = boost::dynamic_pointer_cast<const Building>(obj)) {
        if (building->Unowned())
            sitrep = SitRepEntry(UserStringNop("SITREP_UNOWNED_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        else
            sitrep = SitRepEntry(UserStringNop("SITREP_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM"), "icons/sitrep/combat_destroyed.png");
        sitrep.AddVariable(VarText::BUILDING_ID_TAG,   boost::lexical_cast<std::string>(object_id));
        sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(building->PlanetID()));
    } else {
        sitrep = GenericCombatDestroyedObjectSitrep(combat_system_id);
    }

    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(obj->Owner()));
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));

    return sitrep;
}

SitRepEntry CreatePlanetStarvedToDeathSitRep(int planet_id) {
    SitRepEntry sitrep(UserStringNop("SITREP_PLANET_LOST_STARVED_TO_DEATH"), "icons/sitrep/planet_starved.png");
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    return sitrep;
}

SitRepEntry CreatePlanetColonizedSitRep(int planet_id) {
    SitRepEntry sitrep(UserStringNop("SITREP_PLANET_COLONIZED"), "icons/sitrep/planet_colonized.png");
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    return sitrep;
}

SitRepEntry CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id, int recipient_empire_id) {
    TemporaryPtr<const Fleet> fleet = GetFleet(fleet_id);

    //bool system_contains_recipient_empire_planets = false;
    //if (const System* system = GetSystem(system_id)) {
    //    std::vector<int> system_planets = system->FindObjectIDs<Planet>();
    //    for (std::vector<int>::const_iterator planet_it = system_planets.begin();
    //         planet_it != system_planets.end(); ++planet_it)
    //    {
    //        const Planet* planet = GetPlanet(*planet_it);
    //        if (!planet || planet->Unowned())
    //            continue;
    //        if (planet->OwnedBy(recipient_empire_id)) {
    //            system_contains_recipient_empire_planets = true;
    //            break;
    //        }
    //    }
    //}

    //There are variants of this message for {monster, own, foreign} * {one ship, fleet}.
    // TODO: More variants for systems with / without recipient-owned planets
    //These should really be assembled from several pieces: a fleet description,
    //a system description, and a message template into which both are substituted.
    if (!fleet) {
        SitRepEntry sitrep(UserStringNop("SITREP_FLEET_ARRIVED_AT_SYSTEM"), "icons/sitrep/fleet_arrived.png");
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  boost::lexical_cast<std::string>(system_id));
        sitrep.AddVariable(VarText::FLEET_ID_TAG,   boost::lexical_cast<std::string>(fleet_id));
        return sitrep;
    } else if (fleet->Unowned() && fleet->HasMonsters()) {
        if (fleet->NumShips() == 1) {
            SitRepEntry sitrep(UserStringNop("SITREP_MONSTER_SHIP_ARRIVED_AT_DESTINATION"), "icons/sitrep/fleet_arrived.png");
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(fleet_id));
            int ship_id = *fleet->ShipIDs().begin();
            sitrep.AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(ship_id));
            if (TemporaryPtr<const Ship> ship = GetShip(ship_id))
                sitrep.AddVariable(VarText::DESIGN_ID_TAG, boost::lexical_cast<std::string>(ship->DesignID()));
            return sitrep;
        } else {
            SitRepEntry sitrep(UserStringNop("SITREP_MONSTER_FLEET_ARRIVED_AT_DESTINATION"), "icons/sitrep/fleet_arrived.png");
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  boost::lexical_cast<std::string>(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,   boost::lexical_cast<std::string>(fleet_id));
            sitrep.AddVariable(VarText::RAW_TEXT_TAG,   boost::lexical_cast<std::string>(fleet->NumShips()));
            return sitrep;
        }
    } else if (fleet->Unowned()) {
        SitRepEntry sitrep(UserStringNop("SITREP_FLEET_ARRIVED_AT_DESTINATION"), "icons/sitrep/fleet_arrived.png");
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  boost::lexical_cast<std::string>(system_id));
        sitrep.AddVariable(VarText::FLEET_ID_TAG,   boost::lexical_cast<std::string>(fleet_id));
        sitrep.AddVariable(VarText::RAW_TEXT_TAG,   boost::lexical_cast<std::string>(fleet->NumShips()));
        return sitrep;
    } else if (fleet->OwnedBy(recipient_empire_id)) {
        if (fleet->NumShips() == 1) {
            SitRepEntry sitrep(UserStringNop("SITREP_OWN_SHIP_ARRIVED_AT_DESTINATION"), "icons/sitrep/fleet_arrived.png");
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(fleet_id));
            sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(fleet->Owner()));
            int ship_id = *fleet->ShipIDs().begin();
            sitrep.AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(ship_id));
            if (TemporaryPtr<const Ship> ship = GetShip(ship_id))
                sitrep.AddVariable(VarText::DESIGN_ID_TAG, boost::lexical_cast<std::string>(ship->DesignID()));
            return sitrep;
        } else {
            SitRepEntry sitrep(UserStringNop("SITREP_OWN_FLEET_ARRIVED_AT_DESTINATION"), "icons/sitrep/fleet_arrived.png");
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  boost::lexical_cast<std::string>(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,   boost::lexical_cast<std::string>(fleet_id));
            sitrep.AddVariable(VarText::EMPIRE_ID_TAG,  boost::lexical_cast<std::string>(fleet->Owner()));
            sitrep.AddVariable(VarText::RAW_TEXT_TAG,   boost::lexical_cast<std::string>(fleet->NumShips()));
            return sitrep;
        }
    } else {
        if (fleet->NumShips() == 1) {
            SitRepEntry sitrep(UserStringNop("SITREP_FOREIGN_SHIP_ARRIVED_AT_DESTINATION"), "icons/sitrep/fleet_arrived.png");
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(fleet_id));
            sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(fleet->Owner()));
            int ship_id = *fleet->ShipIDs().begin();
            sitrep.AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(ship_id));
            if (TemporaryPtr<const Ship> ship = GetShip(ship_id))
                sitrep.AddVariable(VarText::DESIGN_ID_TAG, boost::lexical_cast<std::string>(ship->DesignID()));
            return sitrep;
        } else {
            SitRepEntry sitrep(UserStringNop("SITREP_FOREIGN_FLEET_ARRIVED_AT_DESTINATION"), "icons/sitrep/fleet_arrived.png");
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  boost::lexical_cast<std::string>(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,   boost::lexical_cast<std::string>(fleet_id));
            sitrep.AddVariable(VarText::EMPIRE_ID_TAG,  boost::lexical_cast<std::string>(fleet->Owner()));
            sitrep.AddVariable(VarText::RAW_TEXT_TAG,   boost::lexical_cast<std::string>(fleet->NumShips()));
            return sitrep;
        }
    }
}

SitRepEntry CreateEmpireEliminatedSitRep(int empire_id) {
    SitRepEntry sitrep(UserStringNop("SITREP_EMPIRE_ELIMINATED"), "icons/sitrep/empire_eliminated.png");
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return sitrep;
}

SitRepEntry CreateVictorySitRep(const std::string& reason_string, int empire_id) {
    SitRepEntry sitrep(UserStringNop("SITREP_VICTORY"), "icons/sitrep/victory.png");
    sitrep.AddVariable(VarText::TEXT_TAG,          reason_string);
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return sitrep;
}

SitRepEntry CreateSitRep(const std::string& template_string,
                         const std::string& icon,
                         const std::vector<std::pair<std::string, std::string> >& parameters,
                         const std::string label,
                         bool stringtable_lookup)
{
    SitRepEntry sitrep(template_string, icon, label, stringtable_lookup);
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = parameters.begin();
         it != parameters.end(); ++it)
    { sitrep.AddVariable(it->first, it->second); }
    return sitrep;
}
