#include "SitRepEntry.h"

#include "MultiplayerCommon.h"


const std::string SitRepEntry::SITREP_UPDATE_TAG = "SitRepUpdate";

SitRepEntry* CreateTechResearchedSitRep(const std::string& tech_name) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::TECH_RESEARCHED);

    XMLElement techID_elem(VarText::TECH_ID_TAG);
    techID_elem.SetAttribute("value", tech_name);
    sitrep->GetVariables().AppendChild(techID_elem);

    return(sitrep);
}

SitRepEntry* CreateBaseBuiltSitRep(int system_id, int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::BASE_BUILT);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    return(sitrep);
}

SitRepEntry* CreateShipBuiltSitRep(int ship_id, int system_id) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::SHIP_BUILT);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    XMLElement ship_elem(VarText::SHIP_ID_TAG);
    ship_elem.SetAttribute("value", boost::lexical_cast<std::string>(ship_id));
    sitrep->GetVariables().AppendChild(ship_elem);

    return(sitrep);
}

SitRepEntry* CreateBuildingBuiltSitRep(const std::string& building_name, int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::BUILDING_BUILT);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    XMLElement building_elem(VarText::BUILDING_ID_TAG);
    building_elem.SetAttribute("value", building_name);
    sitrep->GetVariables().AppendChild(building_elem);

    return(sitrep);
}

SitRepEntry* CreateCombatSitRep(int empire_id, int victor_id, int system_id) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(victor_id == -1 ? SitRepEntry::COMBAT_SYSTEM_NO_VICTOR
                                     : (empire_id == victor_id ? SitRepEntry::COMBAT_SYSTEM_WON : SitRepEntry::COMBAT_SYSTEM_LOST));

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    return(sitrep);
}

SitRepEntry* CreatePlanetStarvedToDeathSitRep(int system_id, int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::PLANET_LOST_STARVED_TO_DEATH);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    return(sitrep);
}

SitRepEntry* CreatePlanetColonizedSitRep(int system_id, int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::PLANET_COLONIZED);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

     return(sitrep);
}

SitRepEntry* CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::FLEET_ARRIVED_AT_DESTINATION);

    XMLElement fleet_elem(VarText::FLEET_ID_TAG);
    fleet_elem.SetAttribute("value", boost::lexical_cast<std::string>(fleet_id));
    sitrep->GetVariables().AppendChild(fleet_elem);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    return(sitrep);
}

SitRepEntry* CreateEmpireEliminatedSitRep(const std::string& empire_name) {
    SitRepEntry* sitrep = new SitRepEntry();

    sitrep->SetType(SitRepEntry::EMPIRE_ELIMINATED);

    XMLElement name_elem("name");
    name_elem.SetAttribute("value", empire_name);
    sitrep->GetVariables().AppendChild(name_elem);

    return(sitrep);
}
