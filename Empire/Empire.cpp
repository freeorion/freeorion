#include "Empire.h"

#include "../universe/Building.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "ResourcePool.h"
#include "../universe/ShipDesign.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include <algorithm>

#include <boost/lexical_cast.hpp>

using std::find;
using boost::lexical_cast;


namespace {
    const double EPSILON = 1.0e-5;

    void UpdateQueue(double RPs, const std::map<std::string, double>& research_status, Empire::ResearchQueue::QueueType& queue, double& total_RPs_spent, int& projects_in_progress)
    {
        total_RPs_spent = 0.0;
        projects_in_progress = 0;
        for (Empire::ResearchQueue::iterator it = queue.begin(); it != queue.end(); ++it) {
            const Tech* tech = it->get<0>();
            std::map<std::string, double>::const_iterator progress_it = research_status.find(tech->Name());
            double progress = progress_it == research_status.end() ? 0.0 : progress_it->second;
            double RPs_needed = tech->ResearchCost() * tech->ResearchTurns() - progress;
            double RPs_to_spend = std::min(RPs_needed, tech->ResearchCost());
            if (total_RPs_spent + RPs_to_spend <= RPs - EPSILON) {
                it->get<1>() = RPs_to_spend;
                total_RPs_spent += it->get<1>();
                ++projects_in_progress;
            } else if (total_RPs_spent < RPs - EPSILON) {
                it->get<1>() = RPs - total_RPs_spent;
                total_RPs_spent += it->get<1>();
                ++projects_in_progress;
            } else {
                it->get<1>() = 0.0;
            }
        }
    }

    bool temp_header_bool = RecordHeaderFile(EmpireRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


Empire::ResearchQueue::ResearchQueue() :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{}

Empire::ResearchQueue::ResearchQueue(const GG::XMLElement& elem) :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{
    if (elem.Tag() != "Empire::ResearchQueue")
        throw std::invalid_argument("Attempted to construct a Empire::ResearchQueue from an XMLElement that had a tag other than \"Empire::ResearchQueue\"");

    // note that this leaves the queue elements incompletely specified, and does not find values for m_projects_in_progress or m_total_RPs_spent.
    // the owner of this object must call Update() after this object is constructed
    for (GG::XMLElement::const_child_iterator it = elem.Child("m_queue").child_begin(); it != elem.Child("m_queue").child_end(); ++it) {
        push_back(GetTech(it->Tag()));
    }
}

bool Empire::ResearchQueue::InQueue(const Tech* tech) const
{
    return find(tech) != end();
}

int Empire::ResearchQueue::ProjectsInProgress() const
{
    return m_projects_in_progress;
}

double Empire::ResearchQueue::TotalRPsSpent() const
{
    return m_total_RPs_spent;
}

bool Empire::ResearchQueue::empty() const
{
    return !m_queue.size();
}

unsigned int Empire::ResearchQueue::size() const
{
    return m_queue.size();
}

Empire::ResearchQueue::const_iterator Empire::ResearchQueue::begin() const
{
    return m_queue.begin();
}

Empire::ResearchQueue::const_iterator Empire::ResearchQueue::end() const
{
    return m_queue.end();
}

Empire::ResearchQueue::const_iterator Empire::ResearchQueue::find(const Tech* tech) const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->get<0>() == tech)
            return it;
    }
    return end();
}

Empire::ResearchQueue::const_iterator Empire::ResearchQueue::UnderfundedProject() const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->get<1>() && it->get<1>() < it->get<0>()->ResearchCost())
            return it;
    }
    return end();
}

GG::XMLElement Empire::ResearchQueue::XMLEncode() const
{
    GG::XMLElement retval("Empire::ResearchQueue");
    retval.AppendChild("m_queue");
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        retval.LastChild().AppendChild(m_queue[i].get<0>()->Name());
    }
    return retval;
}

void Empire::ResearchQueue::Update(double RPs, const std::map<std::string, double>& research_status)
{
    UpdateQueue(RPs, research_status, m_queue, m_total_RPs_spent, m_projects_in_progress);

    if (EPSILON < RPs) {
        // simulate future turns in order to determine when the techs in the queue will be finished
        int turns = 1;
        QueueType sim_queue = m_queue;
        std::map<std::string, double> sim_research_status = research_status;
        std::map<const Tech*, int> simulation_results;
        while (!sim_queue.empty()) {
            double total_RPs_spent = 0.0;
            int projects_in_progress = 0;
            UpdateQueue(RPs, sim_research_status, sim_queue, total_RPs_spent, projects_in_progress);
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                const Tech* tech = sim_queue[i].get<0>();
                double& status = sim_research_status[tech->Name()];
                status += sim_queue[i].get<1>();
                if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= status) {
                    simulation_results[tech] = turns;
                    sim_queue.erase(sim_queue.begin() + i--);
                }
            }
            ++turns;
        }
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].get<2>() = simulation_results[m_queue[i].get<0>()];
        }
    } else {
        // since there are so few RPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].get<2>() = -1;
        }
    }
}

void Empire::ResearchQueue::push_back(const Tech* tech)
{
    m_queue.push_back(QueueElement(tech, 0.0, -1));
}

void Empire::ResearchQueue::insert(iterator it, const Tech* tech)
{
    m_queue.insert(it, QueueElement(tech, 0.0, -1));
}

void Empire::ResearchQueue::erase(iterator it)
{
    m_queue.erase(it);
}

Empire::ResearchQueue::iterator Empire::ResearchQueue::find(const Tech* tech)
{
    for (iterator it = begin(); it != end(); ++it) {
        if (it->get<0>() == tech)
            return it;
    }
    return end();
}

Empire::ResearchQueue::iterator Empire::ResearchQueue::begin()
{
    return m_queue.begin();
}

Empire::ResearchQueue::iterator Empire::ResearchQueue::end()
{
    return m_queue.end();
}

Empire::ResearchQueue::iterator Empire::ResearchQueue::UnderfundedProject()
{
    for (iterator it = begin(); it != end(); ++it) {
        if (it->get<2>() && it->get<2>() < it->get<0>()->ResearchCost())
            return it;
    }
    return end();
}

    
/** Constructors */ 
Empire::Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id) :
    m_id(ID),
    m_name(name),
    m_player_name(player_name),
    m_color(color), 
    m_homeworld_id(homeworld_id), 
    m_mineral_resource_pool(),m_food_resource_pool(),m_research_resource_pool(),m_population_resource_pool(),m_trade_resource_pool()
{}

Empire::Empire(const GG::XMLElement& elem) :
    m_research_queue(elem.Child("m_research_queue").Child("Empire::ResearchQueue")),
    m_mineral_resource_pool(elem.Child("m_mineral_resource_pool").Child("MineralResourcePool")),
    m_food_resource_pool(elem.Child("m_food_resource_pool").Child("FoodResourcePool")),
    m_research_resource_pool(elem.Child("m_research_resource_pool").Child("ResearchResourcePool")),
    m_population_resource_pool(elem.Child("m_population_resource_pool").Child("PopulationResourcePool")),
    m_trade_resource_pool(elem.Child("m_trade_resource_pool").Child("TradeResourcePool"))

{
    if (elem.Tag() != "Empire")
        throw std::invalid_argument("Attempted to construct a Empire from an XMLElement that had a tag other than \"Empire\"");

    using GG::XMLElement;

    m_id = lexical_cast<int>(elem.Child("m_id").Text());
    m_name = elem.Child("m_name").Text();
    m_player_name = elem.Child("m_player_name").Text();
    m_color = GG::Clr(elem.Child("m_color").Child("GG::Clr"));
    m_homeworld_id = elem.Child("m_homeworld_id").Text().empty() ? 
        UniverseObject::INVALID_OBJECT_ID :
        lexical_cast<int>(elem.Child("m_homeworld_id").Text());

    const XMLElement& sitreps_elem = elem.Child("m_sitrep_entries");
    for (int i = 0; i < sitreps_elem.NumChildren(); ++i) {
        AddSitRepEntry(new SitRepEntry(sitreps_elem.Child(i)));
    }

    const XMLElement& designs_elem = elem.Child("m_ship_designs");
    for (int i = 0; i < designs_elem.NumChildren(); ++i) {
        ShipDesign ship_design(designs_elem.Child(i).Child(0));
        m_ship_designs[ship_design.name] = ship_design;
    }

    m_explored_systems = GG::ContainerFromString<std::set<int> >(elem.Child("m_explored_systems").Text());
    const XMLElement& techs_elem = elem.Child("m_techs");
    for (int i = 0; i < techs_elem.NumChildren(); ++i) {
        m_techs.insert(techs_elem.Child(i).Text());
    }

    const XMLElement& research_status_elem = elem.Child("m_research_status");
    for (int i = 0; i < research_status_elem.NumChildren(); ++i) {
        m_research_status[research_status_elem.Child(i).Tag()] = lexical_cast<double>(research_status_elem.Child(i).Text());
    }

    const XMLElement& building_types_elem = elem.Child("m_building_types");
    for (int i = 0; i < building_types_elem.NumChildren(); ++i) {
        m_building_types.insert(building_types_elem.Child(i).Text());
    }

    m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
}

Empire::~Empire()
{
    for (std::map<std::string, BuildingType*>::const_iterator it = m_modified_building_types.begin(); it != m_modified_building_types.end(); ++it) {
        delete it->second;
    }
    ClearSitRep();
}

/** Misc Accessors */
const std::string& Empire::Name() const
{
    return m_name;
}

const std::string& Empire::PlayerName() const
{
    return m_player_name;
}

int Empire::EmpireID() const
{
    return m_id;
}

const GG::Clr& Empire::Color() const
{
    return m_color;
}

int Empire::HomeworldID() const
{
    return m_homeworld_id;
}

int Empire::CapitolID() const
{
    // TODO: come up with a system for changing (moving) the capitol from the homeworld to somewhere else
    return m_homeworld_id;
}

const ShipDesign* Empire::GetShipDesign(const std::string& name) const
{
    Empire::ShipDesignItr it = m_ship_designs.find(name);
    return (it == m_ship_designs.end()) ? 0 : &it->second;
}

bool Empire::ResearchableTech(const std::string& name) const
{
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    const std::set<std::string>& prereqs = tech->Prerequisites();
    for (std::set<std::string>::const_iterator it = prereqs.begin(); it != prereqs.end(); ++it) {
        if (m_techs.find(*it) == m_techs.end())
            return false;
    }
    return true;
}

const Empire::ResearchQueue& Empire::GetResearchQueue() const
{
    return m_research_queue;
}

double Empire::ResearchStatus(const std::string& name) const
{
    std::map<std::string, double>::const_iterator it = m_research_status.find(name);
    return (it == m_research_status.end()) ? -1.0 : it->second;
}

const std::set<std::string>& Empire::AvailableTechs() const
{
    return m_techs;
}

bool Empire::TechAvailable(const std::string& name) const
{
    Empire::TechItr item = m_techs.find(name);
    return item != m_techs.end();
}

bool Empire::BuildingTypeAvailable(const std::string& name) const
{
    Empire::BuildingTypeItr item = m_building_types.find(name);
    return item != m_building_types.end();
}

const BuildingType* Empire::GetBuildingType(const std::string& name) const
{
    std::map<std::string, BuildingType*>::const_iterator it = m_modified_building_types.find(name);
    if (it != m_modified_building_types.end()) {
        return it->second;
    } else {
        return ::GetBuildingType(name);
    }
}

bool Empire::HasExploredSystem(int ID) const
{
    Empire::SystemIDItr item = find(ExploredBegin(), ExploredEnd(), ID);
    return (item != ExploredEnd());
}

int Empire::NumSitRepEntries() const
{
    return m_sitrep_entries.size();
}


/* *************************************
    (const) Iterators over our various lists
***************************************/
Empire::TechItr Empire::TechBegin() const
{
    return m_techs.begin();
}
Empire::TechItr Empire::TechEnd() const
{
    return m_techs.end();
}

Empire::TechItr Empire::BuildingTypeBegin() const
{
    return m_building_types.begin();
}
Empire::TechItr Empire::BuildingTypeEnd() const
{
    return m_building_types.end();
}

Empire::SystemIDItr Empire::ExploredBegin()  const
{
    return m_explored_systems.begin();
}
Empire::SystemIDItr Empire::ExploredEnd() const
{
    return m_explored_systems.end();
}

Empire::ShipDesignItr Empire::ShipDesignBegin() const
{
    return m_ship_designs.begin();
}
Empire::ShipDesignItr Empire::ShipDesignEnd() const
{
    return m_ship_designs.end();
}

Empire::SitRepItr Empire::SitRepBegin() const
{
    return m_sitrep_entries.begin();
}
Empire::SitRepItr Empire::SitRepEnd() const
{
    return m_sitrep_entries.end();
}


/*************************************************
    Methods to add items to our various lists
**************************************************/
void Empire::PlaceTechInQueue(const Tech* tech, int pos/* = -1*/)
{
    if (!ResearchableTech(tech->Name()) || m_techs.find(tech->Name()) != m_techs.end())
        return;
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (it != m_research_queue.end()) {
        if (std::distance(m_research_queue.begin(), it) < pos)
            --pos;
        m_research_queue.erase(it);
    }
    if (pos < 0 || static_cast<int>(m_research_queue.size()) <= pos)
        m_research_queue.push_back(tech);
    else
        m_research_queue.insert(m_research_queue.begin() + pos, tech);
    m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
}

void Empire::RemoveTechFromQueue(const Tech* tech)
{
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (it != m_research_queue.end()) {
        m_research_queue.erase(it);
        m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
    }
}

void Empire::AddTech(const std::string& name)
{
    m_techs.insert(name);
}

void Empire::UnlockItem(const Tech::ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    if (item.type == UIT_BUILDING) {
        AddBuildingType(item.name);
    }
}

void Empire::AddBuildingType(const std::string& name)
{
    m_building_types.insert(name);
}

void Empire::RefineBuildingType(const std::string& name, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects)
{
    BuildingType*& building = m_modified_building_types[name];
    if (!building)
        building = new BuildingType(*::GetBuildingType(name));
    building->AddEffects(effects);
}

void Empire::ClearRefinements()
{
    for (std::map<std::string, BuildingType*>::iterator it = m_modified_building_types.begin(); it != m_modified_building_types.end(); ++it) {
        delete it->second;
    }
    m_modified_building_types.clear();
}

void Empire::AddExploredSystem(int ID)
{
    m_explored_systems.insert(ID);
}

void Empire::AddShipDesign(const ShipDesign& design)
{
   m_ship_designs[design.name] = design;
}

void Empire::AddSitRepEntry(SitRepEntry* entry)
{
    m_sitrep_entries.push_back(entry);
}


/*************************************************
    Methods to remove items from our various lists
**************************************************/
void Empire::RemoveTech(const std::string& name)
{
    m_techs.erase(name);
}

void Empire::LockItem(const Tech::ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    if (item.type == UIT_BUILDING) {
        RemoveBuildingType(item.name);
    }
}

void Empire::RemoveBuildingType(const std::string& name)
{
    m_building_types.erase(name);
}

void Empire::ClearSitRep()
{
    for (SitRepItr it = m_sitrep_entries.begin(); it != m_sitrep_entries.end(); ++it)
        delete *it;
    m_sitrep_entries.clear();
}


/*************************************************
    Methods to support XML Serialization
**************************************************/
GG::XMLElement Empire::XMLEncode() const
{
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_homeworld_id", lexical_cast<std::string>(m_homeworld_id)));

    retval.AppendChild(XMLElement("m_sitrep_entries"));
    for (SitRepItr it = SitRepBegin(); it != SitRepEnd(); ++it) {
       retval.LastChild().AppendChild((*it)->XMLEncode());
    }

    retval.AppendChild(XMLElement("m_ship_designs"));
    int i = 0;
    for (ShipDesignItr it = ShipDesignBegin(); it != ShipDesignEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("design" + lexical_cast<std::string>(i++), it->second.XMLEncode()));
    }

    retval.AppendChild(XMLElement("m_explored_systems", GG::StringFromContainer<std::set<int> >(m_explored_systems)));
    retval.AppendChild(XMLElement("m_techs"));
    i = 0;
    for (TechItr it = TechBegin(); it != TechEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("tech" + lexical_cast<std::string>(i++), *it));
    }

    retval.AppendChild(XMLElement("m_research_queue", m_research_queue.XMLEncode()));
    retval.AppendChild(XMLElement("m_research_status"));
    for (std::map<std::string, double>::const_iterator it = m_research_status.begin(); it != m_research_status.end(); ++it) {
        retval.LastChild().AppendChild(XMLElement(it->first, lexical_cast<std::string>(it->second)));
    }

    retval.AppendChild(XMLElement("m_building_types"));
    i = 0;
    for (BuildingTypeItr it = BuildingTypeBegin(); it != BuildingTypeEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("building_type" + lexical_cast<std::string>(i++), *it));
    }

    retval.AppendChild(XMLElement("m_mineral_resource_pool", m_mineral_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_food_resource_pool", m_food_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_research_resource_pool", m_research_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_population_resource_pool", m_population_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_trade_resource_pool", m_trade_resource_pool.XMLEncode()));

    return retval;
}

GG::XMLElement Empire::XMLEncode(const Empire& viewer) const
{
    // same empire --->  call other version
    if (viewer.EmpireID() == this->EmpireID())
    {
        return XMLEncode();
    }
    
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));

    // leave these in, but unpopulated or default-populated
    retval.AppendChild(XMLElement("m_homeworld_id"));
    retval.AppendChild(XMLElement("m_sitrep_entries"));
    retval.AppendChild(XMLElement("m_ship_designs"));
    retval.AppendChild(XMLElement("m_explored_systems"));
    retval.AppendChild(XMLElement("m_techs"));
    retval.AppendChild(XMLElement("m_research_queue", ResearchQueue().XMLEncode()));
    retval.AppendChild(XMLElement("m_research_status"));
    retval.AppendChild(XMLElement("m_building_types"));
    retval.AppendChild(XMLElement("m_mineral_resource_pool", MineralResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_food_resource_pool", FoodResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_research_resource_pool", ResearchResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_population_resource_pool", PopulationResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_trade_resource_pool", TradeResourcePool().XMLEncode()));

    return retval;
}


/*************************************************
    Miscellaneous mutators
**************************************************/
void Empire::CheckResearchProgress()
{
    m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
    for (ResearchQueue::iterator it = m_research_queue.begin(); it != m_research_queue.end(); ) {
        const Tech* tech = it->get<0>();
        double& status = m_research_status[tech->Name()];
        status += it->get<1>();
        if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= status) {
            m_techs.insert(tech->Name());
            const std::vector<Tech::ItemSpec>& unlocked_items = tech->UnlockedItems();
            for (unsigned int i = 0; i < unlocked_items.size(); ++i) {
                UnlockItem(unlocked_items[i]);
            }
            AddSitRepEntry(CreateTechResearchedSitRep(tech->Name()));
            // TODO: create unlocked item sitreps?
            m_research_status.erase(tech->Name());
            ResearchQueue::iterator temp_it = it++;
            bool last_element = it == m_research_queue.end();
            m_research_queue.erase(temp_it);
            if (last_element)
                break;
        } else {
            ++it;
        }
    }
}

void Empire::SetColor(const GG::Clr& color)
{
    m_color = color;
}

void Empire::SetName(const std::string& name)
{
    m_name = name;
}

void Empire::SetPlayerName(const std::string& player_name)
{
    m_player_name = player_name;
}

void Empire::UpdateResourcePool()
{
  m_mineral_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
  m_food_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
  m_research_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
  m_population_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
  m_industry_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
  m_trade_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
}

void Empire::UpdateResearchQueue()
{
    m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
}
