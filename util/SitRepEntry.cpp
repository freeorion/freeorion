#include "SitRepEntry.h"

#include "MultiplayerCommon.h"


const std::string SitRepEntry::SITREP_UPDATE_TAG = "SitRepUpdate";

SitRepEntry::SitRepEntry() :
    m_type(INVALID_ENTRY_TYPE)
{}

SitRepEntry::SitRepEntry(EntryType entry_type) :
    m_type(entry_type)
{}

const std::string& SitRepEntry::SitRepTemplateString(EntryType entry_type)
{
    switch (entry_type) {
    case SHIP_BUILT:                    return UserString("SITREP_SHIP_BUILT");                     break;
    case BUILDING_BUILT:                return UserString("SITREP_BUILDING_BUILT");                 break;
    case TECH_RESEARCHED:               return UserString("SITREP_TECH_RESEARCHED");                break;
    case COMBAT_SYSTEM:                 return UserString("SITREP_COMBAT_SYSTEM");                  break;
    case PLANET_CAPTURED:               return UserString("SITREP_PLANET_CAPTURED");                break;
    case PLANET_LOST_STARVED_TO_DEATH:  return UserString("SITREP_PLANET_LOST_STARVED_TO_DEATH");   break;
    case PLANET_COLONIZED:              return UserString("SITREP_PLANET_COLONIZED");               break;
    case FLEET_ARRIVED_AT_DESTINATION:  return UserString("SITREP_FLEET_ARRIVED_AT_DESTINATION");   break;
    case EMPIRE_ELIMINATED:             return UserString("SITREP_EMPIRE_ELIMINATED");              break;
    case VICTORY:                       return UserString("SITREP_VICTORY");                        break;
    case INVALID_ENTRY_TYPE:
    default:                            return UserString("SITREP_ERROR");                          break;
    }
}

const std::string& SitRepEntry::TemplateString() const
{
    //std::cout << "SitRepEntry::TemplateString: " << SitRepTemplateString(m_type) << std::endl;
    return SitRepTemplateString(m_type);
}

SitRepEntry* CreateTechResearchedSitRep(const std::string& tech_name) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::TECH_RESEARCHED);

    XMLElement techID_elem(VarText::TECH_TAG);
    techID_elem.SetAttribute("value", tech_name);
    sitrep->GetVariables().AppendChild(techID_elem);

    return(sitrep);
}

SitRepEntry* CreateShipBuiltSitRep(int ship_id, int system_id) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::SHIP_BUILT);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    XMLElement ship_elem(VarText::SHIP_ID_TAG);
    ship_elem.SetAttribute("value", boost::lexical_cast<std::string>(ship_id));
    sitrep->GetVariables().AppendChild(ship_elem);

    return(sitrep);
}

SitRepEntry* CreateBuildingBuiltSitRep(const std::string& building_name, int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::BUILDING_BUILT);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    XMLElement building_elem(VarText::BUILDING_ID_TAG);
    building_elem.SetAttribute("value", building_name);
    sitrep->GetVariables().AppendChild(building_elem);

    return(sitrep);
}

SitRepEntry* CreateCombatSitRep(int system_id) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::COMBAT_SYSTEM);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    return(sitrep);
}

SitRepEntry* CreatePlanetCapturedSitRep(int planet_id, const std::string& empire_name) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::PLANET_CAPTURED);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    XMLElement name_elem(VarText::EMPIRE_ID_TAG);
    name_elem.SetAttribute("value", empire_name);
    sitrep->GetVariables().AppendChild(name_elem);

    return(sitrep);
}

SitRepEntry* CreatePlanetStarvedToDeathSitRep(int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::PLANET_LOST_STARVED_TO_DEATH);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    return(sitrep);
}

SitRepEntry* CreatePlanetColonizedSitRep(int planet_id) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::PLANET_COLONIZED);

    XMLElement planet_elem(VarText::PLANET_ID_TAG);
    planet_elem.SetAttribute("value", boost::lexical_cast<std::string>(planet_id));
    sitrep->GetVariables().AppendChild(planet_elem);

    return(sitrep);
}

SitRepEntry* CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::FLEET_ARRIVED_AT_DESTINATION);

    XMLElement fleet_elem(VarText::FLEET_ID_TAG);
    fleet_elem.SetAttribute("value", boost::lexical_cast<std::string>(fleet_id));
    sitrep->GetVariables().AppendChild(fleet_elem);

    XMLElement system_elem(VarText::SYSTEM_ID_TAG);
    system_elem.SetAttribute("value", boost::lexical_cast<std::string>(system_id));
    sitrep->GetVariables().AppendChild(system_elem);

    return(sitrep);
}

SitRepEntry* CreateEmpireEliminatedSitRep(const std::string& empire_name) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::EMPIRE_ELIMINATED);

    XMLElement name_elem(VarText::EMPIRE_ID_TAG);
    name_elem.SetAttribute("value", empire_name);
    sitrep->GetVariables().AppendChild(name_elem);

    return(sitrep);
}

SitRepEntry* CreateVictorySitRep(const std::string& reason_string, const std::string& empire_name) {
    SitRepEntry* sitrep = new SitRepEntry(SitRepEntry::VICTORY);

    XMLElement reason_elem(VarText::TEXT_TAG);
    reason_elem.SetAttribute("value", reason_string);
    sitrep->GetVariables().AppendChild(reason_elem);

    XMLElement name_elem(VarText::EMPIRE_ID_TAG);
    name_elem.SetAttribute("value", empire_name);
    sitrep->GetVariables().AppendChild(name_elem);

    return(sitrep);
}
