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
    VarText()
{}

SitRepEntry::SitRepEntry(const std::string& template_string) :
    VarText(template_string, true)
{}

SitRepEntry* CreateTechResearchedSitRep(const std::string& tech_name) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_TECH_RESEARCHED");
    sitrep->AddVariable(VarText::TECH_TAG,          tech_name);
    return(sitrep);
}

SitRepEntry* CreateShipBuiltSitRep(int ship_id, int system_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_SHIP_BUILT");
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    sitrep->AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(ship_id));
    return(sitrep);
}

SitRepEntry* CreateBuildingBuiltSitRep(int building_id, int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_BUILDING_BUILT");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    sitrep->AddVariable(VarText::BUILDING_ID_TAG,   boost::lexical_cast<std::string>(building_id));
    return(sitrep);
}

SitRepEntry* CreateCombatSitRep(int system_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_COMBAT_SYSTEM");
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    return(sitrep);
}

SitRepEntry* CreatePlanetCapturedSitRep(int planet_id, int empire_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_PLANET_CAPTURED");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return(sitrep);
}

namespace {
    SitRepEntry* GenericCombatDestroyedObjectSitrep(int combat_system_id) {
        SitRepEntry* sitrep = new SitRepEntry("SITREP_OBJECT_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));
        return sitrep;
    }
}

SitRepEntry* CreateCombatDestroyedObjectSitRep(int object_id, int combat_system_id, int empire_id) {
    const UniverseObject* obj = GetUniverse().EmpireKnownObjects(empire_id).Object(object_id);
    if (!obj)
        return GenericCombatDestroyedObjectSitrep(combat_system_id);

    SitRepEntry* sitrep(0);

    if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
        sitrep = new SitRepEntry("SITREP_SHIP_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::SHIP_ID_TAG,       boost::lexical_cast<std::string>(object_id));

    } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
        SitRepEntry* sitrep = new SitRepEntry("SITREP_FLEET_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(object_id));

    } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
        SitRepEntry* sitrep = new SitRepEntry("SITREP_PLANET_DESTROYED_AT_SYSTEM");
        sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(object_id));

    } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
        sitrep = new SitRepEntry("SITREP_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM");
        sitrep->AddVariable(VarText::BUILDING_ID_TAG,   boost::lexical_cast<std::string>(object_id));
        sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(building->PlanetID()));
    } else {
        sitrep = GenericCombatDestroyedObjectSitrep(combat_system_id);
    }

    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(combat_system_id));

    return sitrep;
}

SitRepEntry* CreatePlanetStarvedToDeathSitRep(int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_PLANET_LOST_STARVED_TO_DEATH");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    return(sitrep);
}

SitRepEntry* CreatePlanetColonizedSitRep(int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_PLANET_COLONIZED");
    sitrep->AddVariable(VarText::PLANET_ID_TAG,     boost::lexical_cast<std::string>(planet_id));
    return(sitrep);
}

SitRepEntry* CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_FLEET_ARRIVED_AT_DESTINATION");
    sitrep->AddVariable(VarText::SYSTEM_ID_TAG,     boost::lexical_cast<std::string>(system_id));
    sitrep->AddVariable(VarText::FLEET_ID_TAG,      boost::lexical_cast<std::string>(fleet_id));
    return(sitrep);
}

SitRepEntry* CreateEmpireEliminatedSitRep(int empire_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_EMPIRE_ELIMINATED");
    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return(sitrep);
}

SitRepEntry* CreateVictorySitRep(const std::string& reason_string, int empire_id) {
    SitRepEntry* sitrep = new SitRepEntry("SITREP_VICTORY");
    sitrep->AddVariable(VarText::TEXT_TAG,          reason_string);
    sitrep->AddVariable(VarText::EMPIRE_ID_TAG,     boost::lexical_cast<std::string>(empire_id));
    return(sitrep);
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

