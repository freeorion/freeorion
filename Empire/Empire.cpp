#include "Empire.h"

#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../util/Directories.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Random.h"
#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "ResourcePool.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "../universe/Enums.h"
#include "../universe/UniverseObject.h"
#include "EmpireManager.h"

#include <algorithm>

#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>


namespace {
    const double EPSILON = 1.0e-5;

    // sets the .spending, value for each Tech in the queue.  Only sets nonzero funding to
    // a Tech if it is researchable this turn.  Also determines total number of spent RP
    // (returning by reference in total_RPs_spent)
    void SetTechQueueElementSpending(double RPs, const std::map<std::string, double>& research_progress, const std::map<std::string, TechStatus>& research_status, ResearchQueue::QueueType& queue, double& total_RPs_spent, int& projects_in_progress)
    {
        total_RPs_spent = 0.0;
        projects_in_progress = 0;
        int i = 0;

        for (ResearchQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            // get details on what is being researched...
            const Tech* tech = it->tech;
            const std::string name = tech->Name();
            std::map<std::string, TechStatus>::const_iterator status_it = research_status.find(name);
            if (status_it == research_status.end()) 
                throw std::runtime_error("SetTechQueueElementSpending couldn't find tech!");
            bool researchable = false;
            if (status_it->second == TS_RESEARCHABLE) researchable = true;
                        
            if (researchable) {
                std::map<std::string, double>::const_iterator progress_it = research_progress.find(name);
                double progress = progress_it == research_progress.end() ? 0.0 : progress_it->second;
                double RPs_needed = tech->ResearchCost() * tech->ResearchTurns() - progress;
                double RPs_to_spend = std::min(RPs_needed, tech->ResearchCost());

                if (total_RPs_spent + RPs_to_spend <= RPs - EPSILON) {
                    it->spending = RPs_to_spend;
                    total_RPs_spent += it->spending;
                    ++projects_in_progress;
                } else if (total_RPs_spent < RPs - EPSILON) {
                    it->spending = RPs - total_RPs_spent;
                    total_RPs_spent += it->spending;
                    ++projects_in_progress;
                } else {
                    it->spending = 0.0;
                }
            } else {
                // item can't be researched this turn
                it->spending = 0.0;
            }
        }
    }
    
    // sets the .spending, value for each Element in the queue.  Only sets nonzero funding to
    // an Element if its ProductionItem is buildable this turn.  Also determines total number
    // of spent PP (returning by reference in total_PPs_spent)
    void SetProdQueueElementSpending(Empire* empire, double PPs, const std::vector<double>& production_status, ProductionQueue::QueueType& queue, double& total_PPs_spent, int& projects_in_progress)
    {
        assert(production_status.size() == queue.size());
        total_PPs_spent = 0.0;
        projects_in_progress = 0;
        int i = 0;

        for (ProductionQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            // see if item is buildable this turn...
            int location = it->location;
            bool buildable = empire->BuildableItem(it->item, location);
                        
            if (buildable) {
                double item_cost;
                int build_turns;
                boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(it->item);
                
                double progress = production_status[i];
                double PPs_needed = item_cost * build_turns * it->remaining - progress;
                double PPs_to_spend = std::min(PPs_needed, item_cost);
                if (total_PPs_spent + PPs_to_spend <= PPs - EPSILON) {
                    it->spending = PPs_to_spend;
                    total_PPs_spent += it->spending;
                    ++projects_in_progress;
                } else if (total_PPs_spent < PPs - EPSILON) {
                    it->spending = PPs - total_PPs_spent;
                    total_PPs_spent += it->spending;
                    ++projects_in_progress;
                } else {
                    it->spending = 0.0;
                }
            } else {
                // item can't be produced at its location this turn
                it->spending = 0.0;
            }
        }
    }

    void LoadShipNames(std::vector<std::string>& names)
    {
        boost::filesystem::ifstream ifs(GetSettingsDir() / "shipnames.txt");
        while (ifs) {
            std::string latest_name;
            std::getline(ifs, latest_name);
            if (latest_name != "") {
                names.push_back(latest_name.substr(0, latest_name.find_last_not_of(" \t") + 1)); // strip off trailing whitespace
            }
        }
    }
}


////////////////////////////////////////
// ResearchQueue::Element             //
////////////////////////////////////////
ResearchQueue::Element::Element() :
    tech(0),
    spending(0.0),
    turns_left(0)
{}

ResearchQueue::Element::Element(const Tech* tech_, double spending_, int turns_left_) :
    tech(tech_),
    spending(spending_),
    turns_left(turns_left_)
{}


////////////////////////////////////////
// ResearchQueue                      //
////////////////////////////////////////
ResearchQueue::ResearchQueue() :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{}

bool ResearchQueue::InQueue(const Tech* tech) const
{
    return find(tech) != end();
}

int ResearchQueue::ProjectsInProgress() const
{
    return m_projects_in_progress;
}

double ResearchQueue::TotalRPsSpent() const
{
    return m_total_RPs_spent;
}

bool ResearchQueue::empty() const
{
    return !m_queue.size();
}

unsigned int ResearchQueue::size() const
{
    return m_queue.size();
}

ResearchQueue::const_iterator ResearchQueue::begin() const
{
    return m_queue.begin();
}

ResearchQueue::const_iterator ResearchQueue::end() const
{
    return m_queue.end();
}

ResearchQueue::const_iterator ResearchQueue::find(const Tech* tech) const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->tech == tech)
            return it;
    }
    return end();
}

const ResearchQueue::Element& ResearchQueue::operator[](int i) const
{
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ResearchQueue::const_iterator ResearchQueue::UnderfundedProject() const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->spending && it->spending < it->tech->ResearchCost() && 1 < it->turns_left)
            return it;
    }
    return end();
}

void ResearchQueue::Update(Empire* empire, double RPs, const std::map<std::string, double>& research_progress)
{
    // status of all techs for this empire
    TechManager& tech_manager = GetTechManager();
    std::map<std::string, TechStatus> sim_tech_status_map;
    for (TechManager::iterator tech_it = tech_manager.begin(); tech_it != tech_manager.end(); ++tech_it) {
        std::string tech_name = (*tech_it)->Name();
        sim_tech_status_map[tech_name] = empire->GetTechStatus(tech_name);
    }

    SetTechQueueElementSpending(RPs, research_progress, sim_tech_status_map, m_queue, m_total_RPs_spent, m_projects_in_progress);

    if (m_queue.empty()) return;    // nothing more to do...
    const int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops

    if (EPSILON < RPs) {
        // simulate future turns in order to determine when the techs in the queue will be finished
        int turns = 1;
        QueueType sim_queue = m_queue;
        std::map<std::string, double> sim_research_progress = research_progress;

        std::map<const Tech*, int> simulation_results;
        // initialize simulation_results with -1 for all techs, so that any techs that aren't
        // finished in simulation by turn TOO_MANY_TURNS will be left marked as never to be finished
        for (unsigned int i = 0; i < sim_queue.size(); ++i)
            simulation_results[m_queue[i].tech] = -1;

        while (!sim_queue.empty() && turns < TOO_MANY_TURNS) {
            double total_RPs_spent = 0.0;
            int projects_in_progress = 0;
            SetTechQueueElementSpending(RPs, sim_research_progress, sim_tech_status_map, sim_queue, total_RPs_spent, projects_in_progress);
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                const Tech* tech = sim_queue[i].tech;
                double& status = sim_research_progress[tech->Name()];
                status += sim_queue[i].spending;
                if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= status) {
                    m_queue[i].turns_left = simulation_results[m_queue[i].tech];
                    simulation_results[tech] = turns;
                    sim_queue.erase(sim_queue.begin() + i--);
                    sim_tech_status_map[tech->Name()] = TS_COMPLETE;
                }
            }

            // update simulated status of techs: some may be not researchable that were previously not.
            // only need to check techs that are actually on the queue, as these are the only ones
            // that might now be researched
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                const Tech* tech = sim_queue[i].tech;
                const std::string tech_name = tech->Name();
                // if tech is currently not researchable, this is because one or more of its prereqs is not researched
                if (sim_tech_status_map[tech_name] == TS_UNRESEARCHABLE) {
                    const std::set<std::string>& prereqs = tech->Prerequisites();
                    // find if any prereqs are presently not researched.  if all prereqs are researched, this tech should be researchable
                    bool has_not_complete_prereq = false;
                    for (std::set<std::string>::const_iterator it = prereqs.begin(); it != prereqs.end(); ++it) {
                        if (sim_tech_status_map[*it] != TS_COMPLETE) {
                            has_not_complete_prereq = true;
                            break;
                        }
                    }
                    if (!has_not_complete_prereq) {
                        // all prereqs were complete!  this tech is researchable!
                        sim_tech_status_map[tech_name] = TS_RESEARCHABLE;
                    }
                }
            }
            ++turns;
        }
        // return results
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].turns_left = simulation_results[m_queue[i].tech];
        }
    } else {
        // since there are so few RPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].turns_left = -1;
        }
    }
}

void ResearchQueue::push_back(const Tech* tech)
{
    m_queue.push_back(Element(tech, 0.0, -1));
}

void ResearchQueue::insert(iterator it, const Tech* tech)
{
    m_queue.insert(it, Element(tech, 0.0, -1));
}

void ResearchQueue::erase(iterator it)
{
    assert(it != end());
    m_queue.erase(it);
}

ResearchQueue::iterator ResearchQueue::find(const Tech* tech)
{
    for (iterator it = begin(); it != end(); ++it) {
        if (it->tech == tech)
            return it;
    }
    return end();
}

ResearchQueue::iterator ResearchQueue::begin()
{
    return m_queue.begin();
}

ResearchQueue::iterator ResearchQueue::end()
{
    return m_queue.end();
}

ResearchQueue::iterator ResearchQueue::UnderfundedProject()
{
    for (iterator it = begin(); it != end(); ++it) {
        if (it->spending && it->spending < it->tech->ResearchCost() && 1 < it->turns_left)
            return it;
    }
    return end();
}

/////////////////////////////////////
// ProductionQueue::ProductionItem //
/////////////////////////////////////
ProductionQueue::ProductionItem::ProductionItem()
{}

ProductionQueue::ProductionItem::ProductionItem(BuildType build_type_, std::string name_) :
    build_type(build_type_),
    name(name_),
    design_id(UniverseObject::INVALID_OBJECT_ID)
{}

ProductionQueue::ProductionItem::ProductionItem(BuildType build_type_, int design_id_) :
    build_type(build_type_),
    name(std::string("")),
    design_id(design_id_)
{
    if (build_type == BT_SHIP) {
        const ShipDesign* ship_design = GetShipDesign(design_id);
        name = ship_design->Name();
    } else {
        name = "???";
    }
}

//////////////////////////////
// ProductionQueue::Element //
//////////////////////////////
ProductionQueue::Element::Element() :
    ordered(0),
    remaining(0),
    location(UniverseObject::INVALID_OBJECT_ID),
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(ProductionItem item_, int ordered_, int remaining_, int location_) :
    item(item_),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, std::string name, int ordered_, int remaining_, int location_) :
    item(build_type, name),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, int design_id, int ordered_, int remaining_, int location_) :
    item(build_type, design_id),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

/////////////////////
// ProductionQueue //
/////////////////////
ProductionQueue::ProductionQueue() :
    m_projects_in_progress(0),
    m_total_PPs_spent(0.0)
{}

int ProductionQueue::ProjectsInProgress() const
{
    return m_projects_in_progress;
}

double ProductionQueue::TotalPPsSpent() const
{
    return m_total_PPs_spent;
}

bool ProductionQueue::empty() const
{
    return !m_queue.size();
}

unsigned int ProductionQueue::size() const
{
    return m_queue.size();
}

ProductionQueue::const_iterator ProductionQueue::begin() const
{
    return m_queue.begin();
}

ProductionQueue::const_iterator ProductionQueue::end() const
{
    return m_queue.end();
}

ProductionQueue::const_iterator ProductionQueue::find(int i) const
{
    return (0 <= i && i < static_cast<int>(size())) ? (begin() + i) : end();
}

const ProductionQueue::Element& ProductionQueue::operator[](int i) const
{
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ProductionQueue::const_iterator ProductionQueue::UnderfundedProject(const Empire* empire) const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(it->item);
        if (it->spending && it->spending < item_cost && 1 < it->turns_left_to_next_item)
            return it;
    }
    return end();
}

void ProductionQueue::Update(Empire* empire, double PPs, const std::vector<double>& production_status)
{
    SetProdQueueElementSpending(empire, PPs, production_status, m_queue, m_total_PPs_spent, m_projects_in_progress);

    if (m_queue.empty()) {
        ProductionQueueChangedSignal(); // need this so BuildingsPanel updates properly after removing last building
        return;   // nothing more to do...
    }

    const int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops
    
    if (EPSILON < PPs) {
        //Logger().debugStream() << "ProductionQueue::Update: Simulating future turns of production queue";
        // simulate future turns in order to determine when the builditems in the queue will be finished
        int turns = 1;
        QueueType sim_queue = m_queue;
        std::vector<double> sim_production_status = production_status;
        std::vector<int> simulation_results(sim_production_status.size(), -1);
        std::vector<int> sim_queue_original_indices(sim_production_status.size());
        for (unsigned int i = 0; i < sim_queue_original_indices.size(); ++i) {
            sim_queue_original_indices[i] = i;
        }
        
        // remove from simulated queue any items that can't be built due to not meeting their location conditions
        // might be better to re-check buildability each turn, but this would require creating a simulated universe
        // into which simulated completed buildings could be inserted, as well as spoofing the current turn, or
        // otherwise faking the results for evaluating arbitrary location conditions for the simulated universe.
        // this would also be inaccurate anyway due to player choices or random chance, so for simplicity, it is
        // assume that building location conditions evaluated at the present turn apply indefinitely
        for (unsigned int i = 0; i < sim_queue.size(); ++i) {
            if (empire->BuildableItem(sim_queue[i].item, sim_queue[i].location)) continue;
            
            // remove unbuildable items from the simulated queue, since they'll never finish...            
            m_queue[sim_queue_original_indices[i]].turns_left_to_completion = -1;   // turns left is indeterminate for this item
            sim_queue.erase(sim_queue.begin() + i);
            sim_production_status.erase(sim_production_status.begin() + i);
            sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
        }
        
        // cycle through items on queue, adding up their allotted PP until each is finished and removed from queue
        // until everything on queue has been finished, in order to calculate expected completion times
        while (!sim_queue.empty() && turns < TOO_MANY_TURNS) {
            double total_PPs_spent = 0.0;
            int projects_in_progress = 0;

            //Logger().debugStream() << "ProductionQueue::Update: Calling SetProdQueueElementSpending for simulated queue";
            SetProdQueueElementSpending(empire, PPs, sim_production_status, sim_queue, total_PPs_spent, projects_in_progress);
            
            // cycle through items on queue, apply one turn's PP towards items, remove items that are done
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                double item_cost;
                int build_turns;
                boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(sim_queue[i].item);
                
                double& status = sim_production_status[i];
                status += sim_queue[i].spending;
                
                if (item_cost * build_turns - EPSILON <= status) {
                    sim_production_status[i] -= item_cost * build_turns;    // might have spillover to next item in order, so don't set to exactly 0
                    if (sim_queue[i].remaining == m_queue[sim_queue_original_indices[i]].remaining) {
                        m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = turns;
                    }
                    if (!--sim_queue[i].remaining) {
                        //Logger().debugStream() << "    ITEM COMPLETE!  REMOVING";
                        m_queue[sim_queue_original_indices[i]].turns_left_to_completion = turns;
                        sim_queue.erase(sim_queue.begin() + i);
                        sim_production_status.erase(sim_production_status.begin() + i);
                        sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
                    }
                }
            }            
            ++turns;            
        }   // loop while (!sim_queue.empty() && turns < TOO_MANY_TURNS)
        
        // mark rest of items on simulated queue (if any) as never to be finished
        for (unsigned int i = 0; i < sim_queue.size(); ++i) {
            if (sim_queue[i].remaining == m_queue[sim_queue_original_indices[i]].remaining)
                m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = -1;
            m_queue[sim_queue_original_indices[i]].turns_left_to_completion = -1;
        }
        
    } else {
        // since there are so few PPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].turns_left_to_next_item = -1;
            m_queue[i].turns_left_to_completion = -1;
        }
    }
    ProductionQueueChangedSignal();
}

void ProductionQueue::push_back(const Element& element)
{
    m_queue.push_back(element);
}

void ProductionQueue::insert(iterator it, const Element& element)
{
    m_queue.insert(it, element);
}

void ProductionQueue::erase(int i)
{
    assert(i <= static_cast<int>(size()));
    m_queue.erase(begin() + i);
}

ProductionQueue::iterator ProductionQueue::erase(iterator it)
{
    assert(it != end());
    return m_queue.erase(it);
}

ProductionQueue::iterator ProductionQueue::begin()
{
    return m_queue.begin();
}

ProductionQueue::iterator ProductionQueue::end()
{
    return m_queue.end();
}

ProductionQueue::iterator ProductionQueue::find(int i)
{
    return (0 <= i && i < static_cast<int>(size())) ? (begin() + i) : end();
}

ProductionQueue::Element& ProductionQueue::operator[](int i)
{
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ProductionQueue::iterator ProductionQueue::UnderfundedProject(const Empire* empire)
{
    for (iterator it = begin(); it != end(); ++it) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(it->item);
        if (it->spending && it->spending < item_cost && 1 < it->turns_left_to_next_item)
            return it;
    }
    return end();
}


////////////
// Empire //
////////////
Empire::Empire() :
    m_id(-1),
    m_homeworld_id(UniverseObject::INVALID_OBJECT_ID),
    m_capitol_id(UniverseObject::INVALID_OBJECT_ID),
    m_resource_pools(),
    m_population_pool(),
    m_food_total_distributed(0),
    m_maintenance_total_cost(0)
{
    m_resource_pools[RE_MINERALS] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_MINERALS));
    m_resource_pools[RE_FOOD] =     boost::shared_ptr<ResourcePool>(new ResourcePool(RE_FOOD));
    m_resource_pools[RE_RESEARCH] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_RESEARCH));
    m_resource_pools[RE_INDUSTRY] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_INDUSTRY));
    m_resource_pools[RE_TRADE] =    boost::shared_ptr<ResourcePool>(new ResourcePool(RE_TRADE));
}

Empire::Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id) :
    m_id(ID),
    m_name(name),
    m_player_name(player_name),
    m_color(color), 
    m_homeworld_id(homeworld_id),
    m_capitol_id(homeworld_id),
    m_resource_pools(),
    m_population_pool(),
    m_food_total_distributed(0),
    m_maintenance_total_cost(0)
{
    m_resource_pools[RE_MINERALS] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_MINERALS));
    m_resource_pools[RE_FOOD] =     boost::shared_ptr<ResourcePool>(new ResourcePool(RE_FOOD));
    m_resource_pools[RE_RESEARCH] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_RESEARCH));
    m_resource_pools[RE_INDUSTRY] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_INDUSTRY));
    m_resource_pools[RE_TRADE] =    boost::shared_ptr<ResourcePool>(new ResourcePool(RE_TRADE));
}

Empire::~Empire()
{
    ClearSitRep();
}

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
    return m_capitol_id;
}

void Empire::SetCapitolID(int id)
{
    const Universe& universe = GetUniverse();
    const Planet* planet = universe.Object<Planet>(id);
    if (planet) {
        const std::set<int>& owners = planet->Owners();
        if (owners.size() == 1 && *owners.begin() == EmpireID()) {
            m_capitol_id = id;
        }
    }
};

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

const ResearchQueue& Empire::GetResearchQueue() const
{
    return m_research_queue;
}

double Empire::ResearchStatus(const std::string& name) const
{
    std::map<std::string, double>::const_iterator it = m_research_progress.find(name);
    return (it == m_research_progress.end()) ? -1.0 : it->second;
}

const std::set<std::string>& Empire::AvailableTechs() const
{
    return m_techs;
}

bool Empire::TechResearched(const std::string& name) const
{
    Empire::TechItr item = m_techs.find(name);
    return item != m_techs.end();
}

TechStatus Empire::GetTechStatus(const std::string& name) const
{
    if (TechResearched(name)) return TS_COMPLETE;
    if (ResearchableTech(name)) return TS_RESEARCHABLE;
    return TS_UNRESEARCHABLE;
}

const std::set<std::string>& Empire::AvailableBuildingTypes() const
{
    return m_available_building_types;
}

bool Empire::BuildingTypeAvailable(const std::string& name) const
{
    return m_available_building_types.find(name) != m_available_building_types.end();
}

const std::set<int>& Empire::ShipDesigns() const
{
    return m_ship_designs;
}

std::set<int> Empire::AvailableShipDesigns() const
{
    // create new map containing all ship designs that are available
    std::set<int> retval;
    for (ShipDesignItr it = m_ship_designs.begin(); it != m_ship_designs.end(); ++it) {
        if (ShipDesignAvailable(*it))
            retval.insert(*it);
    }
    return retval;
}

bool Empire::ShipDesignAvailable(int ship_design_id) const
{
    // if design isn't kept by this empire, it can't be built
    if (!ShipDesignKept(ship_design_id))
        return false;

    const ShipDesign* design = GetShipDesign(ship_design_id);
    if (!design) return false;

    // design is kept, but still need to verify that it is buildable at this time.  Part or hull tech 
    // requirements might prevent it from being built.
    const std::vector<std::string>& parts = design->Parts();
    for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
        std::string name = *it;
        if (name == "")
            continue;   // empty slot can't be unavailable
        if (!ShipPartAvailable(name))
            return false;
    }
    if (!ShipHullAvailable(design->Hull()))
        return false;

    // if there are no reasons the design isn't available, then by default it is available
    return true;
}

bool Empire::ShipDesignKept(int ship_design_id) const {
    return (m_ship_designs.find(ship_design_id) != m_ship_designs.end());
}

const std::set<std::string>& Empire::AvailableShipParts() const {
    return m_available_part_types;
}

bool Empire::ShipPartAvailable(const std::string& name) const {
    return m_available_part_types.find(name) != m_available_part_types.end();
}

const std::set<std::string>& Empire::AvailableShipHulls() const {
    return m_available_hull_types;
}

bool Empire::ShipHullAvailable(const std::string& name) const {
    return m_available_hull_types.find(name) != m_available_hull_types.end();
}

const ProductionQueue& Empire::GetProductionQueue() const
{
    return m_production_queue;
}

double Empire::ProductionStatus(int i) const
{
    return (0 <= i && i < static_cast<int>(m_production_progress.size())) ? m_production_progress[i] : -1.0;
}

std::pair<double, int> Empire::ProductionCostAndTime(BuildType build_type, std::string name) const
{
    switch (build_type) {
    case BT_BUILDING: {
        const BuildingType* building_type = GetBuildingType(name);
        if (!building_type)
            break;
        return std::make_pair(building_type->BuildCost(), building_type->BuildTime());
    }
    default:
        break;
    }
    return std::make_pair(-1.0, -1);
}

std::pair<double, int> Empire::ProductionCostAndTime(BuildType build_type, int design_id) const
{
    switch (build_type) {
    case BT_SHIP: {
        const ShipDesign* ship_design = GetShipDesign(design_id);
        if (!ship_design)
            break;
        return std::make_pair(ship_design->Cost(), ship_design->BuildTime());
    }
    default:
        break;
    }
    return std::make_pair(-1.0, -1);
}

std::pair<double, int> Empire::ProductionCostAndTime(const ProductionQueue::ProductionItem& item) const
{
    if (item.build_type == BT_SHIP)
        return ProductionCostAndTime(item.build_type, item.design_id);
    else
        throw std::invalid_argument("Empire::ProductionCostAndTime was passed a ProductionItem with an invalid BuildType");
    return std::make_pair(-1.0, -1);
}

bool Empire::HasExploredSystem(int ID) const
{
    return (m_explored_systems.find(ID) != m_explored_systems.end());
}

bool Empire::BuildableItem(BuildType build_type, const std::string& name, int location) const
{
    // special case to check for ships being passed with names, not design ids
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Empire::BuildableItem was passed BuildType BT_SHIP with a name, but ship designs are tracked by number");

    if (build_type == BT_BUILDING && !BuildingTypeAvailable(name)) return false;

    if (ProductionCostAndTime(build_type, name) == std::make_pair(-1.0, -1)) {
        // item is unknown, unavailable, or invalid.
        return false;
    }

    UniverseObject* build_location = GetUniverse().Object(location);
    if (!build_location) return false;

    if (build_type == BT_BUILDING) {
        // building type must be valid...
        const BuildingType* building_type = GetBuildingType(name);
        if (!building_type) return false;
        // ...and the specified location must be a valid production location for that building type
        return building_type->ProductionLocation(m_id, location);

    } else {
        throw std::invalid_argument("Empire::BuildableItem was passed an invalid BuildType");
    }
}

bool Empire::BuildableItem(BuildType build_type, int design_id, int location) const
{
    // special case to check for buildings or orbitals being passed with ids, not names
    if (build_type == BT_BUILDING)
        throw std::invalid_argument("Empire::BuildableItem was passed BuildType BT_BUILDING with a design id number, but these types are tracked by name");

    if (build_type == BT_SHIP && !ShipDesignAvailable(design_id)) return false;

    if (ProductionCostAndTime(build_type, design_id) == std::make_pair(-1.0, -1)) {
        // item is unknown, unavailable, or invalid.
        return false;
    }

    UniverseObject* build_location = GetUniverse().Object(location);
    if (!build_location) return false;

    if (build_type == BT_SHIP) {
        // design must be known to this empire
        const ShipDesign* ship_design = GetShipDesign(design_id);
        if (!ship_design) return false;
        // ...and the specified location must be a valid production location for this design
        return ship_design->ProductionLocation(m_id, location);

    } else {
        throw std::invalid_argument("Empire::BuildableItem was passed an invalid BuildType");
    }
}


bool Empire::BuildableItem(const ProductionQueue::ProductionItem& item, int location) const
{
    if (item.build_type == BT_BUILDING)
        return BuildableItem(item.build_type, item.name, location);
    else if (item.build_type == BT_SHIP)
        return BuildableItem(item.build_type, item.design_id, location);
    else
        throw std::invalid_argument("Empire::BuildableItem was passed a ProductionItem with an invalid BuildType"); 
    return false;
}
int Empire::NumSitRepEntries() const
{
    return m_sitrep_entries.size();
}

void Empire::UpdateSystemSupplyRanges()
{
    m_fleet_supply_system_ranges.clear();
    m_resource_supply_system_ranges.clear();

    // as of this writing, only planets can distribute supplies to fleets or other planets.  If other objects
    // get the ability to distribute supplies, this should be expanded to them as well
    Universe::ObjectVec owned_planets = GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id));
    for (Universe::ObjectVec::const_iterator it = owned_planets.begin(); it != owned_planets.end(); ++it) {
        const UniverseObject* obj = *it;

        // check if object has a supply meter
        const Meter* supply_meter = obj->GetMeter(METER_SUPPLY);
        if (!supply_meter) continue;

        // ensure object is within a system, from which it can distribute supplies
        int system_id = obj->SystemID();
        if (system_id == UniverseObject::INVALID_OBJECT_ID)
            continue;   // TODO: consider future special case if current object is itself a system


        // for now, all planet can trasport supplies only one starlane jump
        m_resource_supply_system_ranges[system_id] = 1;


        // get fleet supply range for next turn for this object
        int fleet_supply_range = static_cast<int>(floor(obj->ProjectedCurrentMeter(METER_SUPPLY)));

        // if this object can provide more supply than the best previously checked object in this system, record its range as the new best for the system
        std::map<int, int>::iterator system_it = m_fleet_supply_system_ranges.find(system_id);     // try to find a previous entry for this system's supply range
        if (system_it == m_fleet_supply_system_ranges.end() || fleet_supply_range > system_it->second) { // if there is no previous entry, or the previous entry is shorter than the new one, add or replace the entry
            m_fleet_supply_system_ranges[system_id] = fleet_supply_range;
        }
    }
}

void Empire::UpdateSupplyUnobstructedSystems()
{
    m_supply_unobstructed_systems.clear();

    // find "unobstructed" systems that can propegate fleet supply routes
    const std::vector<System*> all_systems = GetUniverse().FindObjects<System>();

    for (std::vector<System*>::const_iterator it = all_systems.begin(); it != all_systems.end(); ++it) {
        const System* system = *it;
        int system_id = system->ID();

        // to be unobstructed, systems must both:
        // be explored by this empire...
        if (m_explored_systems.find(system_id) == m_explored_systems.end())
            continue;

        // ...and ( contain a friendly fleet *or* not contain a hostile fleet )
        bool blocked = false;
        std::vector<const Fleet*> fleets = system->FindObjects<Fleet>();
        for (std::vector<const Fleet*>::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
            const Fleet* fleet = *it;

            // check if this emprie owns this fleet.
            if (fleet->OwnedBy(m_id)) {
                // this empire owns this fleet.  the system is unobstructed regardless of what else is here
                blocked = false;
                break;
            } else {
                // this empire doesn't own this fleet.  the system might be obstructed, but need to check the
                // rest of the fleets to be sure.  TODO: deal with other affiliations, like neutral empires or
                // allies of this empire
                blocked = true;
            }
        }
        if (!blocked)
            m_supply_unobstructed_systems.insert(system_id);
    }
}

void Empire::UpdateFleetSupply()
{
    m_fleet_supplyable_system_ids.clear();
    m_fleet_supply_starlane_traversals.clear();

    // store range of all systems before propegation of supply in working map used to propegate that range to other systems.
    std::map<int, int> propegating_fleet_supply_ranges = m_fleet_supply_system_ranges;

    // insert all systems that produce supply on their own into a list of systems to process
    std::list<int>      propegating_systems_list;           // working list of systems to propegate supply from
    for (std::map<int, int>::const_iterator it = propegating_fleet_supply_ranges.begin(); it != propegating_fleet_supply_ranges.end(); ++it)
        propegating_systems_list.push_back(it->first);

    // iterate through list of accessable systems, processing each in order it was added (like breadth first search) until no
    // systems are left able to further propregate
    std::list<int>::iterator sys_list_it = propegating_systems_list.begin();
    std::list<int>::iterator sys_list_end = propegating_systems_list.end();
    while (sys_list_it != sys_list_end) {
        int cur_sys_id = *sys_list_it;
        int cur_sys_range = propegating_fleet_supply_ranges[cur_sys_id];    // range away from this system that supplies can be transported

        if (cur_sys_range <= 0) {
            // can't propegate supply out a system that 
            ++sys_list_it;
            continue;
        }

        // can propegate further, if adjacent systems have smaller supply range than one less than this system's range
        const System* system = GetUniverse().Object<System>(cur_sys_id);
        System::StarlaneMap starlanes = system->VisibleStarlanes(m_id);
        for (System::StarlaneMap::const_iterator lane_it = starlanes.begin(); lane_it != starlanes.end(); ++lane_it) {
            int lane_end_sys_id = lane_it->first;

            if (m_supply_unobstructed_systems.find(lane_end_sys_id) == m_supply_unobstructed_systems.end()) continue; // can't propegate here

            // compare next system's supply range to this system's supply range.  propegate if necessary.
            std::map<int, int>::const_iterator lane_end_sys_it = propegating_fleet_supply_ranges.find(lane_end_sys_id);
            if (lane_end_sys_it == propegating_fleet_supply_ranges.end() || lane_end_sys_it->second <= cur_sys_range) {
                // next system has no supply yet, or its range equal to or smaller than this system's

                // update next system's range, if propegating from this system would make it larger
                if (lane_end_sys_it == propegating_fleet_supply_ranges.end() || lane_end_sys_it->second < cur_sys_range - 1) {
                    // update with new range
                    propegating_fleet_supply_ranges[lane_end_sys_id] = cur_sys_range - 1;
                    // add next system to list of systems to propegate further
                    propegating_systems_list.push_back(lane_end_sys_id);
                }

                // regardless of whether propegating from current to next system increased its range, add the
                // traversed lane to show redundancies in supply network to player
                m_fleet_supply_starlane_traversals.insert(std::make_pair(cur_sys_id, lane_end_sys_id));
            }
        }
        ++sys_list_it;
        sys_list_end = propegating_systems_list.end();
    }

    // convert supply ranges info into output set of supplyable systems
    for (std::map<int, int>::const_iterator it = propegating_fleet_supply_ranges.begin(); it != propegating_fleet_supply_ranges.end(); ++it)
        m_fleet_supplyable_system_ids.insert(it->first);
}

void Empire::UpdateResourceSupply()
{
    // need to get a set of sets of systems that can exchange resources.  some sets may be just one system,
    // in which resources can be exchanged between UniverseObjects producing or consuming them, but which 
    // can't exchange with any other systems.

    m_resource_supply_groups.clear();
    m_resource_supply_starlane_traversals.clear();


    // map from system id to set of systems that are connected to it directly
    std::map<int, std::set<int> > supply_groups_map;

    // all systems that can supply another system or within themselves, or can be supplied by another system.
    // need to keep track of this so that only these systems are put into the boost adjacency graph.  if
    // additional systems were put in, they would be returned as being in their own "connected" component and
    // would have to be filtered out of the results before being returned
    std::set<int> all_supply_exchanging_systems;


    // loop through systems, getting set of systems that can be supplied by each.  (may be an empty set for
    // some systems that cannot supply within themselves, or may contain only source systesm, or could contain
    // multiple other systsms)
    for (std::set<int>::const_iterator source_sys_it = m_supply_unobstructed_systems.begin(); source_sys_it != m_supply_unobstructed_systems.end(); ++source_sys_it) {
        int source_sys_id = *source_sys_it;

        // skip systems that don't have any supply to propegate.
        std::map<int, int>::const_iterator system_supply_it = m_resource_supply_system_ranges.find(source_sys_id);
        if (system_supply_it == m_resource_supply_system_ranges.end() || system_supply_it->second == 0)
            continue;

        // skip systems that can't propegate supply, even if they have supply to propegate
        std::set<int>::const_iterator unobstructed_systems_it = m_supply_unobstructed_systems.find(source_sys_id);
        if (unobstructed_systems_it == m_supply_unobstructed_systems.end())
            continue;


        // system can supply itself, so store this fact
        supply_groups_map[source_sys_id].insert(source_sys_id);


        // add source system to start of list of systems to propegate supply to
        std::list<int> propegating_systems_list;
        propegating_systems_list.push_back(source_sys_id);

        // set up map from system_id to number of supply jumps further supply can propegate from that system.
        // this ensures that supply propegation is limited in distance, and doesn't back-propegate
        std::map<int, int> propegating_system_supply_ranges;
        // initialize with source supply range
        propegating_system_supply_ranges[source_sys_id] = system_supply_it->second;


        // iterate through list of accessible systems, processing each in order it was added (like breadth first
        // search) adding adjacent systems as appropriate, until no systems are left able to further propregate
        std::list<int>::iterator sys_list_it = propegating_systems_list.begin();
        std::list<int>::iterator sys_list_end = propegating_systems_list.end();
        while (sys_list_it != sys_list_end) {
            int cur_sys_id = *sys_list_it;

            // get additional supply range this system can propegate
            int cur_sys_range = propegating_system_supply_ranges[cur_sys_id];

            // skip system if it can't propegate futher
            if (cur_sys_range <= 0) {
                ++sys_list_it;
                continue;
            }

            // attempt to propegate to unobstructed adjacent systems
            const System* system = GetUniverse().Object<System>(cur_sys_id);
            System::StarlaneMap starlanes = system->VisibleStarlanes(m_id);
            for (System::StarlaneMap::const_iterator lane_it = starlanes.begin(); lane_it != starlanes.end(); ++lane_it) {
                int lane_end_sys_id = lane_it->first;

                // ensure this adjacent system is unobstructed
                if (m_supply_unobstructed_systems.find(lane_end_sys_id) == m_supply_unobstructed_systems.end()) continue; // can't propegate here

                // compare next system's supply range to this system's supply range.  propegate if necessary.
                std::map<int, int>::const_iterator lane_end_sys_it = propegating_system_supply_ranges.find(lane_end_sys_id);
                if (lane_end_sys_it == propegating_system_supply_ranges.end() || propegating_system_supply_ranges[lane_end_sys_id] <= cur_sys_range) {
                    // next system has no supply yet, or its range equal to or smaller than this system's

                    int next_sys_range = cur_sys_range - 1;

                    // if propegating from this system would make it longer, update next system's range
                    if (lane_end_sys_it == propegating_system_supply_ranges.end() || propegating_system_supply_ranges[lane_end_sys_id] < next_sys_range) {
                        // update with new range
                        propegating_system_supply_ranges[lane_end_sys_id] = next_sys_range;
                        // add next system to list of systems to propegate further
                        propegating_systems_list.push_back(lane_end_sys_id);
                    }

                    // regardless of whether propegating from current to next system increased its range, add the
                    // traversed lane to show redundancies in supply network to player
                    m_resource_supply_starlane_traversals.insert(std::make_pair(cur_sys_id, lane_end_sys_id));
                }
            }
            ++sys_list_it;
            sys_list_end = propegating_systems_list.end();
        }

        // for debug purposes, output the propegated supply
        //Logger().debugStream() << "source system " << source_sys_id << " propegated ranges:";
        //for (std::map<int, int>::const_iterator pssr_it = propegating_system_supply_ranges.begin(); pssr_it != propegating_system_supply_ranges.end(); ++pssr_it)
        //    Logger().debugStream() << "....system id: " << pssr_it->first << " range: " << pssr_it->second;

        all_supply_exchanging_systems.insert(source_sys_id);
        // have now propegated supply as far as it can go from this source.  store results
        for (std::map<int, int>::const_iterator pssr_it = propegating_system_supply_ranges.begin(); pssr_it != propegating_system_supply_ranges.end(); ++pssr_it) {
            int cur_sys_id = pssr_it->first;
            supply_groups_map[source_sys_id].insert(cur_sys_id);    // source system can share with current system
            all_supply_exchanging_systems.insert(cur_sys_id);
        }
    }

    if (supply_groups_map.empty()) return;  // need to avoid going to boost graph stuff below, which doesn't seem to like being fed empty graphs...


    // Need to merge interconnected supply groups into as few sets of mutually-supply-exchanging systems
    // as possible.  This requires finding the connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.

    // create graph
    boost::adjacency_list <boost::vecS, boost::vecS, boost::undirectedS> graph;

    // boost expects vertex labels to range from 0 to num vertices - 1, so need to map from system id
    // to graph id and back when accessing vertices
    std::vector<int> graph_id_to_sys_id;
    graph_id_to_sys_id.reserve(all_supply_exchanging_systems.size());

    std::map<int, int> sys_id_to_graph_id;
    int graph_id = 0;
    for (std::set<int>::const_iterator sys_it = all_supply_exchanging_systems.begin(); sys_it != all_supply_exchanging_systems.end(); ++sys_it, ++graph_id) {
        int sys_id = *sys_it;
        //const UniverseObject* sys = GetUniverse().Object(sys_id);
        //std::string name = sys->Name();
        //Logger().debugStream() << "supply-exchanging system: " << name;

        boost::add_vertex(graph);   // should add with index = graph_id

        graph_id_to_sys_id.push_back(sys_id);
        sys_id_to_graph_id[sys_id] = graph_id;
    }

    // add edges for all direct connections between systems
    for (std::map<int, std::set<int> >::const_iterator maps_it = supply_groups_map.begin(); maps_it != supply_groups_map.end(); ++maps_it) {
        int start_graph_id = sys_id_to_graph_id[maps_it->first];
        const std::set<int>& set = maps_it->second;
        for(std::set<int>::const_iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
            int end_graph_id = sys_id_to_graph_id[*set_it];
            boost::add_edge(start_graph_id, end_graph_id, graph);

            //int sys_id1 = graph_id_to_sys_id[start_graph_id];
            //const UniverseObject* sys1 = GetUniverse().Object(sys_id1);
            //std::string name1 = sys1->Name();
            //int sys_id2 = graph_id_to_sys_id[end_graph_id];
            //const UniverseObject* sys2 = GetUniverse().Object(sys_id2);
            //std::string name2 = sys2->Name();
            //Logger().debugStream() << "added edge to graph: " << name1 << " and " << name2;
        }
    }

    // declare storage and fill with the component id (group id of connected systems) for each graph vertex
    std::vector<int> components(boost::num_vertices(graph));
    boost::connected_components(graph, &components[0]);

    //for (std::vector<int>::size_type i = 0; i != components.size(); ++i) {
    //    int sys_id = graph_id_to_sys_id[i];
    //    const UniverseObject* sys = GetUniverse().Object(sys_id);
    //    std::string name = sys->Name();
    //    std::cout << "ssytem " << name <<" is in component " << components[i] << std::endl;
    //}
    //std::cout << std::endl;

    // convert results back from graph id to system id, and into desired output format
    // output: std::set<std::set<int> >& m_resource_supply_groups

    // first, sort into a map from component id to set of system ids in component
    std::map<int, std::set<int> > component_sets_map;
    for (std::size_t graph_id = 0; graph_id != components.size(); ++graph_id) {
        int label = components[graph_id];
        int sys_id = graph_id_to_sys_id[graph_id];
        component_sets_map[label].insert(sys_id);
    }

    // copy sets in map into set of sets
    for (std::map<int, std::set<int> >::const_iterator map_it = component_sets_map.begin(); map_it != component_sets_map.end(); ++map_it) {
        m_resource_supply_groups.insert(map_it->second);
    }
}

const std::set<int>& Empire::FleetSupplyableSystemIDs() const
{
    return m_fleet_supplyable_system_ids;
}

const std::set<std::pair<int, int> >& Empire::FleetSupplyStarlaneTraversals() const
{
    return m_fleet_supply_starlane_traversals;
}

const std::map<int, int>& Empire::FleetSupplyRanges() const
{
    return m_fleet_supply_system_ranges;
}

const std::set<std::set<int> >& Empire::ResourceSupplyGroups() const
{
    return m_resource_supply_groups;
}

const std::set<std::pair<int, int> >& Empire::ResourceSupplyStarlaneTraversals() const
{
    return m_resource_supply_starlane_traversals;
}

const std::map<int, int>& Empire::ResourceSupplyRanges() const
{
    return m_resource_supply_system_ranges;
}

const std::set<int>& Empire::SupplyUnobstructedSystems() const
{
    return m_supply_unobstructed_systems;
}

Empire::TechItr Empire::TechBegin() const
{
    return m_techs.begin();
}
Empire::TechItr Empire::TechEnd() const
{
    return m_techs.end();
}

Empire::TechItr Empire::AvailableBuildingTypeBegin() const
{
    return m_available_building_types.begin();
}
Empire::TechItr Empire::AvailableBuildingTypeEnd() const
{
    return m_available_building_types.end();
}

const std::set<int>& Empire::ExploredSystems() const
{
    return m_explored_systems;
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

double Empire::ProductionPoints() const
{
    return std::min(GetResourcePool(RE_INDUSTRY)->Available(), GetResourcePool(RE_MINERALS)->Available());
}

const ResourcePool* Empire::GetResourcePool(ResourceType resource_type) const
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        return 0;
    return it->second.get();
}

double Empire::ResourceStockpile(ResourceType type) const
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceStockpile passed invalid ResourceType");
    return it->second->Stockpile();
}

double Empire::ResourceMaxStockpile(ResourceType type) const
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceMaxStockpile passed invalid ResourceType");
    return it->second->MaxStockpile();
}

double Empire::ResourceProduction(ResourceType type) const
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceProduction passed invalid ResourceType");
    return it->second->Production();
}

double Empire::ResourceAvailable(ResourceType type) const
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceAvailable passed invalid ResourceType");
    return it->second->Available();
}

const PopulationPool& Empire::GetPopulationPool() const
{
    return m_population_pool;
}

double Empire::Population() const
{
    return m_population_pool.Population();
}

void Empire::SetResourceStockpile(ResourceType resource_type, double stockpile)
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::SetResourceStockpile passed invalid ResourceType");
    return it->second->SetStockpile(stockpile);
}

void Empire::SetResourceMaxStockpile(ResourceType resource_type, double max)
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::SetResourceMaxStockpile passed invalid ResourceType");
    return it->second->SetMaxStockpile(max);
}

void Empire::PlaceTechInQueue(const Tech* tech, int pos/* = -1*/)
{
    if (TechResearched(tech->Name()) || m_techs.find(tech->Name()) != m_techs.end())
        return;
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (pos < 0 || static_cast<int>(m_research_queue.size()) <= pos) {
        if (it != m_research_queue.end())
            m_research_queue.erase(it);
        m_research_queue.push_back(tech);
    } else {
        if (it < m_research_queue.begin() + pos)
            --pos;
        if (it != m_research_queue.end())
            m_research_queue.erase(it);
        m_research_queue.insert(m_research_queue.begin() + pos, tech);
    }
    m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->Available(), m_research_progress);
}

void Empire::RemoveTechFromQueue(const Tech* tech)
{
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (it != m_research_queue.end()) {
        m_research_queue.erase(it);
        m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->Available(), m_research_progress);
    }
}

void Empire::PlaceBuildInQueue(BuildType build_type, const std::string& name, int number, int location, int pos/* = -1*/)
{
    if (!BuildableItem(build_type, name, location))
        Logger().debugStream() << "Empire::PlaceBuildInQueue() : Placed a non-buildable item in queue...";

    ProductionQueue::Element build(build_type, name, number, number, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos) {
        m_production_queue.push_back(build);
        m_production_progress.push_back(0.0);
    } else {
        m_production_queue.insert(m_production_queue.begin() + pos, build);
        m_production_progress.insert(m_production_progress.begin() + pos, 0.0);
    }
    m_production_queue.Update(this, ProductionPoints(), m_production_progress);
}

void Empire::PlaceBuildInQueue(BuildType build_type, int design_id, int number, int location, int pos/* = -1*/)
{
    if (!BuildableItem(build_type, design_id, location))
        Logger().debugStream() << "Empire::PlaceBuildInQueue() : Placed a non-buildable item in queue...";

    ProductionQueue::Element build(build_type, design_id, number, number, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos) {
        m_production_queue.push_back(build);
        m_production_progress.push_back(0.0);
    } else {
        m_production_queue.insert(m_production_queue.begin() + pos, build);
        m_production_progress.insert(m_production_progress.begin() + pos, 0.0);
    }
    m_production_queue.Update(this, ProductionPoints(), m_production_progress);
}

void Empire::PlaceBuildInQueue(const ProductionQueue::ProductionItem& item, int number, int location, int pos/* = -1*/)
{
    if (item.build_type == BT_BUILDING)
        PlaceBuildInQueue(item.build_type, item.name, number, location, pos);
    else if (item.build_type == BT_SHIP)
        PlaceBuildInQueue(item.build_type, item.design_id, number, location, pos);
    else
        throw std::invalid_argument("Empire::PlaceBuildInQueue was passed a ProductionQueue::ProductionItem with an invalid BuildType");
}

void Empire::SetBuildQuantity(int index, int quantity)
{
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && 1 < quantity)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
    m_production_queue.Update(this, ProductionPoints(), m_production_progress);
}

void Empire::MoveBuildWithinQueue(int index, int new_index)
{
    if (index < new_index)
        --new_index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index ||
        new_index < 0 || static_cast<int>(m_production_queue.size()) <= new_index)
        throw std::runtime_error("Empire::MoveBuildWithinQueue() : Attempted to move a production queue item to or from an invalid index.");
    ProductionQueue::Element build = m_production_queue[index];
    double status = m_production_progress[index];
    m_production_queue.erase(index);
    m_production_progress.erase(m_production_progress.begin() + index);
    m_production_queue.insert(m_production_queue.begin() + new_index, build);
    m_production_progress.insert(m_production_progress.begin() + new_index, status);
    m_production_queue.Update(this, ProductionPoints(), m_production_progress);
}

void Empire::RemoveBuildFromQueue(int index)
{
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::RemoveBuildFromQueue() : Attempted to delete a production queue item with an invalid index.");
    m_production_queue.erase(index);
    m_production_progress.erase(m_production_progress.begin() + index);
    m_production_queue.Update(this, ProductionPoints(), m_production_progress);
}

void Empire::ConquerBuildsAtLocation(int location_id) {
    if (location_id == UniverseObject::INVALID_OBJECT_ID)
        throw std::invalid_argument("Empire::ConquerBuildsAtLocationFromEmpire: tried to conquer build items located at an invalid location");

    Logger().debugStream() << "Empire::ConquerBuildsAtLocationFromEmpire: conquering items located at " << location_id << " to empire " << m_id;
    /** Processes Builditems on queues of empires other than this empire, at the location with id \a location_id and,
        as appropriate, adds them to the build queue of \a this empire, deletes them, or leaves them on the build 
        queue of their current empire */

    for (EmpireManager::iterator emp_it = Empires().begin(); emp_it != Empires().end(); ++emp_it) {
        int from_empire_id = emp_it->first;
        if (from_empire_id == m_id) continue;    // skip this empire; can't capture one's own builditems

        Empire* from_empire = emp_it->second;
        ProductionQueue& queue = from_empire->m_production_queue;
        std::vector<double>& status = from_empire->m_production_progress;

        int i = 0;
        for (ProductionQueue::iterator queue_it = queue.begin(); queue_it != queue.end(); ) {
            ProductionQueue::Element elem = *queue_it;
            if (elem.location != location_id) {
                ++queue_it;
                continue; // skip projects with wrong location
            }

            ProductionQueue::ProductionItem item = elem.item;

            if (item.build_type == BT_BUILDING) {
                std::string name = item.name;
                const BuildingType* type = GetBuildingType(name);
                if (!type)
                    throw std::invalid_argument("Empire::ConquerBuildsAtLocationFromEmpire: ProductionQueue item had an invalid BuildingType name");

                CaptureResult result = type->GetCaptureResult(from_empire_id, m_id, location_id, true);

                if (result == CR_DESTROY) {
                    // item removed from current queue, NOT added to conquerer's queue
                    queue_it = queue.erase(queue_it);
                    status.erase(status.begin() + i);

                } else if (result == CR_CAPTURE) {
                    // item removed from current queue, added to conquerer's queue
                    ProductionQueue::Element build(item, elem.ordered, elem.remaining, location_id);
                    m_production_queue.push_back(build);

                    m_production_progress.push_back(status[i]);

                    queue_it = queue.erase(queue_it);
                    status.erase(status.begin() + i);

                } else if (result == INVALID_CAPTURE_RESULT) {
                    throw std::invalid_argument("Empire::ConquerBuildsAtLocationFromEmpire: BuildingType had an invalid CaptureResult");
                } else {
                    ++queue_it;
                    ++i;
                }
                // otherwise do nothing: item left on current queue, conquerer gets nothing
            } else {
                ++queue_it;
                ++i;
            }

            // TODO: other types of build item...
        }
    }
}

void Empire::AddTech(const std::string& name)
{
    const Tech* tech = GetTech(name);
    if (!tech)
        Logger().errorStream() << "Empire::AddTech given and invalid tech: " << name;;
    m_techs.insert(name);
}

void Empire::UnlockItem(const ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    switch (item.type) {
    case UIT_BUILDING:
        AddBuildingType(item.name);
        break;
    case UIT_SHIP_PART:
        AddPartType(item.name);
        break;
    case UIT_SHIP_HULL:
        AddHullType(item.name);
        break;
    default:
        Logger().errorStream() << "Empire::UnlockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::AddBuildingType(const std::string& name)
{
    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type)
        Logger().errorStream() << "Empire::AddBuildingType given an invalid building type name: " << name;
    m_available_building_types.insert(name);
}

void Empire::AddPartType(const std::string& name)
{
    const PartType* part_type = GetPartType(name);
    if (!part_type)
        Logger().errorStream() << "Empire::AddPartType given an invalid part type name: " << name;
    m_available_part_types.insert(name);
}

void Empire::AddHullType(const std::string& name)
{
    const HullType* hull_type = GetHullType(name);
    if (!hull_type)
        Logger().errorStream() << "Empire::AddHullType given an invalid hull type name: " << name;
    m_available_hull_types.insert(name);
}

void Empire::AddExploredSystem(int ID)
{
    const System* system = GetUniverse().Object<System>(ID);
    if (!system)
        Logger().errorStream() << "Empire::AddExploredSystem given an invalid system id: " << ID;
    m_explored_systems.insert(ID);
}

std::string Empire::NewShipName()
{
    std::string retval;
    static std::vector<std::string> ship_names;
    if (ship_names.empty())
        LoadShipNames(ship_names);
    int star_name_idx = RandSmallInt(0, static_cast<int>(ship_names.size()) - 1);
    retval = ship_names[star_name_idx];
    int times_name_used = ++m_ship_names_used[retval];
    if (1 < times_name_used)
        retval += " " + RomanNumber(times_name_used);
    return retval;
}

void Empire::AddShipDesign(int ship_design_id)
{
    /* Check if design id is valid.  that is, check that it corresponds to an existing shipdesign in the
       universe.  On clients, this means that this empire knows about this ship design.  On the server, 
       all existing ship designs will be valid, so this just adds this design's id to those that this empire
       will remember */
    const ShipDesign* ship_design = GetUniverse().GetShipDesign(ship_design_id);
    if (ship_design) {
        // design is valid, so just add the id to empire's set of ids that it knows about
        m_ship_designs.insert(ship_design_id);
    } else {
        // design in not valid
        throw std::invalid_argument("Empire::AddShipDesign(int ship_design_id) was passed a design id that this empire doesn't know about, or that doesn't exist");
    }
}

int Empire::AddShipDesign(ShipDesign* ship_design)
{
    Universe& universe = GetUniverse();
    /* check if there already exists this same design in the universe.  On clients, this checks whether this empire
       knows of this exact design and is trying to re-add it.  On the server, this checks whether this exact design
       exists at all yet */
    for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it) {
        if (ship_design == it->second) {
            // ship design is already present in universe.  just need to add it to the empire's set of ship designs
            m_ship_designs.insert(it->first);
            return it->first;
        }
    }

    // design is apparently new, so add it to the universe and put its new id in the empire's set of designs
    int new_design_id = GetNewDesignID();   // on the sever, this just generates a new design id.  on clients, it polls the sever for a new id

    if (new_design_id == UniverseObject::INVALID_OBJECT_ID) {
        Logger().errorStream() << "Unable to get new design id";
        return new_design_id;
    }

    bool success = universe.InsertShipDesignID(ship_design, new_design_id);

    if (!success) {
        Logger().errorStream() << "Unable to add new design to universe";
        return UniverseObject::INVALID_OBJECT_ID;
    }

    m_ship_designs.insert(new_design_id);

    return new_design_id;
}

void Empire::RemoveShipDesign(int ship_design_id)
{
        m_ship_designs.erase(ship_design_id);
}

void Empire::AddSitRepEntry(SitRepEntry* entry)
{
    m_sitrep_entries.push_back(entry);
}

void Empire::RemoveTech(const std::string& name)
{
    m_techs.erase(name);
}

void Empire::LockItem(const ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    if (item.type == UIT_BUILDING) {
        RemoveBuildingType(item.name);
    }
}

void Empire::RemoveBuildingType(const std::string& name)
{
    m_available_building_types.erase(name);
}

void Empire::ClearSitRep()
{
    for (SitRepItr it = m_sitrep_entries.begin(); it != m_sitrep_entries.end(); ++it)
        delete *it;
    m_sitrep_entries.clear();
}

void Empire::CheckResearchProgress()
{
    // following commented line should be redundant, as previous call to UpdateResourcePools should have generated necessary info
    // m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->Available(), m_research_progress);
    std::vector<const Tech*> to_erase;
    for (ResearchQueue::iterator it = m_research_queue.begin(); it != m_research_queue.end(); ++it) {
        const Tech* tech = it->tech;
        double& progress = m_research_progress[tech->Name()];
        progress += it->spending;
        if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= progress) {
            m_techs.insert(tech->Name());
            const std::vector<ItemSpec>& unlocked_items = tech->UnlockedItems();
            for (unsigned int i = 0; i < unlocked_items.size(); ++i) {
                UnlockItem(unlocked_items[i]);
            }
            AddSitRepEntry(CreateTechResearchedSitRep(tech->Name()));
            // TODO: create unlocked item sitreps?
            m_research_progress.erase(tech->Name());
            to_erase.push_back(tech);
        }
    }

    for (std::vector<const Tech*>::iterator it = to_erase.begin(); it != to_erase.end(); ++it) {
        ResearchQueue::iterator temp_it = m_research_queue.find(*it);
        if (temp_it != m_research_queue.end())
            m_research_queue.erase(temp_it);
    }
    // can uncomment following line when / if research stockpiling is enabled...
    // m_resource_pools[RE_RESEARCH]->SetStockpile(m_resource_pools[RE_RESEARCH]->Available() - m_research_queue.TotalRPsSpent());
}

void Empire::CheckProductionProgress()
{
    // following commented line should be redundant, as previous call to UpdateResourcePools should have generated necessary info
    // m_production_queue.Update(this, ProductionPoints(), m_production_progress);
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = ProductionCostAndTime(m_production_queue[i].item);
        double& status = m_production_progress[i];
        status += m_production_queue[i].spending;
        if (item_cost * build_turns - EPSILON <= status) {
            m_production_progress[i] -= item_cost * build_turns;
            switch (m_production_queue[i].item.build_type) {
            case BT_BUILDING: {
                Universe& universe = GetUniverse();
                Planet* planet = universe.Object<Planet>(m_production_queue[i].location);
                assert(planet);
                Building* building = new Building(m_id, m_production_queue[i].item.name, planet->ID());
                int building_id = universe.Insert(building);
                planet->AddBuilding(building_id);
                SitRepEntry *entry = CreateBuildingBuiltSitRep(m_production_queue[i].item.name, planet->ID());
                AddSitRepEntry(entry);
                //Logger().debugStream() << "New Building created on turn: " << building->CreationTurn();
                break;
            }

            case BT_SHIP: {
                Universe& universe = GetUniverse();
                UniverseObject* build_location = universe.Object(m_production_queue[i].location);
                System* system = universe_object_cast<System*>(build_location);
                if (!system && build_location)
                    system = build_location->GetSystem();
                // TODO: account for shipyards and/or other ship production sites that are in interstellar space, if needed
                assert(system);

                // create new fleet with new ship
                Fleet* fleet = new Fleet("", system->X(), system->Y(), m_id);
                int fleet_id = universe.Insert(fleet);

                system->Insert(fleet);
                Logger().debugStream() << "New Fleet created on turn: " << fleet->CreationTurn();

                // add ship
                Ship* ship = new Ship(m_id, m_production_queue[i].item.design_id);
                ship->GetMeter(METER_FUEL)->SetCurrent(Meter::METER_MAX);  // ensures ship starts with some fuel.  will be clamped to max value after effects are applied to set that max value appropriately

                int ship_id = universe.Insert(ship);
#if 0
                const ShipDesign* ship_design = GetShipDesign(m_production_queue[i].item.design_id);
                std::string ship_name(ship_design->Name());
                ship_name += boost::lexical_cast<std::string>(ship_id);
                ship->Rename(ship_name);
#else
                ship->Rename(NewShipName());
#endif
                fleet->AddShip(ship_id);

                // rename fleet, given its id and the ship that is in it
                std::vector<int> ship_ids;  ship_ids.push_back(ship_id);
                fleet->Rename(Fleet::GenerateFleetName(ship_ids, fleet_id));

                Logger().debugStream() << "New Ship created on turn: " << ship->CreationTurn();

                // add sitrep
                SitRepEntry *entry = CreateShipBuiltSitRep(ship_id, system->ID());
                AddSitRepEntry(entry);
                break;
            }

            default:
                break;
            }

            if (!--m_production_queue[i].remaining)
                to_erase.push_back(i);
        }
    }

    for (std::vector<int>::reverse_iterator it = to_erase.rbegin(); it != to_erase.rend(); ++it) {
        m_production_progress.erase(m_production_progress.begin() + *it);
        m_production_queue.erase(*it);
    }

    m_resource_pools[RE_MINERALS]->SetStockpile(m_resource_pools[RE_MINERALS]->Available() - m_production_queue.TotalPPsSpent());
    // can uncomment following line when / if industry stockpiling is allowed...
    // m_resource_pools[RE_INDUSTRY]->SetStockpile(m_resource_pools[RE_INDUSTRY]->Available() - m_production_queue.TotalPPsSpent());
}

void Empire::CheckTradeSocialProgress()
{
    m_resource_pools[RE_TRADE]->SetStockpile(m_resource_pools[RE_TRADE]->Available() - m_maintenance_total_cost);
}

void Empire::CheckGrowthFoodProgress()
{
    m_resource_pools[RE_FOOD]->SetStockpile(m_resource_pools[RE_FOOD]->Available() - m_food_total_distributed);
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

void Empire::InitResourcePools()
{
    Universe::ObjectVec object_vec = GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(m_id));
    std::vector<ResourceCenter*> res_vec;
    std::vector<PopCenter*> pop_vec;
    // determine if each object owned by this empire is a ResourceCenter and/or PopCenter (could be one, neither or both)
    for (unsigned int i = 0; i < object_vec.size(); ++i)
    {
        if (ResourceCenter* rc = dynamic_cast<ResourceCenter*>(object_vec[i]))
            res_vec.push_back(rc);
        if (PopCenter* pc = dynamic_cast<PopCenter*>(object_vec[i]))
            pop_vec.push_back(pc);
    }

    m_resource_pools[RE_MINERALS]->SetResourceCenters(res_vec);
    m_resource_pools[RE_FOOD]->SetResourceCenters(res_vec);
    m_resource_pools[RE_RESEARCH]->SetResourceCenters(res_vec);
    m_resource_pools[RE_INDUSTRY]->SetResourceCenters(res_vec);
    m_resource_pools[RE_TRADE]->SetResourceCenters(res_vec);

    m_population_pool.SetPopCenters(pop_vec);


    // inform the blockadeable resource pools about systems that can share
    m_resource_pools[RE_MINERALS]->SetSystemSupplyGroups(m_resource_supply_groups);
    m_resource_pools[RE_FOOD]->SetSystemSupplyGroups(m_resource_supply_groups);
    m_resource_pools[RE_INDUSTRY]->SetSystemSupplyGroups(m_resource_supply_groups);

    // set non-blockadeable resrouce pools to share resources between all systems
    std::set<std::set<int> > sets_set;
    std::set<int> all_systems_set;
    const std::vector<System*> all_systems_vec = GetUniverse().FindObjects<System>();
    for (std::vector<System*>::const_iterator it = all_systems_vec.begin(); it != all_systems_vec.end(); ++it)
        all_systems_set.insert((*it)->ID());
    sets_set.insert(all_systems_set);
    m_resource_pools[RE_RESEARCH]->SetSystemSupplyGroups(sets_set);
    m_resource_pools[RE_TRADE]->SetSystemSupplyGroups(sets_set);

    // set stockpile location
    m_resource_pools[RE_MINERALS]->SetStockpileSystem(CapitolID());
    m_resource_pools[RE_FOOD]->SetStockpileSystem(CapitolID());
    m_resource_pools[RE_INDUSTRY]->SetStockpileSystem(CapitolID());
    m_resource_pools[RE_RESEARCH]->SetStockpileSystem(CapitolID());
    m_resource_pools[RE_TRADE]->SetStockpileSystem(CapitolID());
}

void Empire::UpdateResourcePools()
{
    // updating queues, spending, distribution and growth each update their respective pools,
    // (as well as the ways in which the resources are used, which needs to be done
    // simultaneously to keep things consistent)
    UpdateResearchQueue();
    UpdateProductionQueue();
    UpdateTradeSpending();
    UpdateFoodDistribution();
    UpdatePopulationGrowth();
}

void Empire::UpdateResearchQueue()
{
    m_resource_pools[RE_RESEARCH]->Update();
    m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->Available(), m_research_progress);
    m_resource_pools[RE_RESEARCH]->ChangedSignal();
}

void Empire::UpdateProductionQueue()
{
    m_resource_pools[RE_MINERALS]->Update();
    m_resource_pools[RE_INDUSTRY]->Update();
    m_production_queue.Update(this, ProductionPoints(), m_production_progress);
    m_resource_pools[RE_MINERALS]->ChangedSignal();
    m_resource_pools[RE_INDUSTRY]->ChangedSignal();
}

void Empire::UpdateTradeSpending()
{
    m_resource_pools[RE_TRADE]->Update(); // recalculate total trade production

    // TODO: Replace with call to some other subsystem, similar to the Update...Queue functions
    m_maintenance_total_cost = 0.0;

    Universe::ObjectVec buildings = GetUniverse().FindObjects(OwnedVisitor<Building>(m_id));
    for (Universe::ObjectVec::const_iterator it = buildings.begin(); it != buildings.end(); ++it)
    {
        Building *building = universe_object_cast<Building*>(*it);
        if (!building) continue;
        //if (building->Operating())
            m_maintenance_total_cost += GetBuildingType(building->BuildingTypeName())->MaintenanceCost();
    }
    m_resource_pools[RE_TRADE]->ChangedSignal();
}

void Empire::UpdateFoodDistribution()
{
    Logger().debugStream() << "Food distribution for empire " << EmpireID();
    m_resource_pools[RE_FOOD]->Update();  // recalculate total food production

    double available_food = m_resource_pools[RE_FOOD]->Available();
    m_food_total_distributed = 0.0;

    std::vector<PopCenter*> pop_centers = m_population_pool.PopCenters(); //GetUniverse().FindObjects(OwnedVisitor<PopCenter>(m_id));
    std::vector<PopCenter*>::iterator pop_it;
    std::vector<ResourceCenter*> resource_centers = m_resource_pools[RE_FOOD]->ResourceCenters(); //GetUniverse().FindObjects(OwnedVisitor<ResourceCenter>(m_id));
    std::vector<ResourceCenter*>::iterator res_it;

    // compile map of food production of ResourceCenters, indexed by center's id
    std::map<int, double> fp_map;
    for (res_it = resource_centers.begin(); res_it != resource_centers.end(); ++res_it) {
        ResourceCenter *center = *res_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);
        fp_map[obj->ID()] = obj->GetMeter(METER_FARMING)->Current();
    }

    // first pass: give food to PopCenters that produce food, limited by their food need and their food production
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it) {
        PopCenter *center = *pop_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);

        double need = obj->GetMeter(METER_POPULATION)->Current();   // basic need is current population - prevents starvation

        // determine if, and if so how much, food this center produces locally
        double food_prod = 0.0;
        std::map<int, double>::iterator fp_map_it = fp_map.find(obj->ID());
        if (fp_map_it != fp_map.end())
            food_prod = fp_map_it->second;

        // allocate food to this PopCenter, deduct from pool, add to total food distribution tally
        double allocation = std::min(available_food, std::min(need, food_prod));

        Logger().debugStream() << "allocating " << allocation << " food to " << obj->Name() << " for own production and need";

        center->SetAvailableFood(allocation);
        m_food_total_distributed += allocation;
        available_food -= allocation;
    }

    Logger().debugStream() << "Empire::UpdateFoodDistribution: m_food_total_distributed: " << m_food_total_distributed;

    // second pass: give as much food as needed to PopCenters to maintain current population
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it) {
        PopCenter *center = *pop_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);

        double need = obj->GetMeter(METER_POPULATION)->Current();
        double has = center->AvailableFood();
        double addition = std::min(need - has, available_food);
        double new_allocation = std::max(0.0, has + addition);

        Logger().debugStream() << "allocating " << new_allocation << " food to " << obj->Name() << " to maintain population";

        center->SetAvailableFood(new_allocation);
        available_food -= addition;

        m_food_total_distributed += addition;
    }

    Logger().debugStream() << "Empire::UpdateFoodDistribution: m_food_total_distributed: " << m_food_total_distributed;

    // third pass: give as much food as needed to PopCenters to allow max possible growth
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it) {
        PopCenter *center = *pop_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);

        double has = center->AvailableFood();
        double addition = center->FuturePopGrowthMax();
        double new_allocation = std::max(0.0, has + addition);

        Logger().debugStream() << "allocating " << new_allocation << " food to " << obj->Name() << " for max possible growth";

        center->SetAvailableFood(new_allocation);
        available_food -= addition;

        m_food_total_distributed += addition;
    }

    Logger().debugStream() << "Empire::UpdateFoodDistribution: m_food_total_distributed: " << m_food_total_distributed;


    // after changing food distribution, population growth predictions may need to be redone
    // by calling UpdatePopulationGrowth()  

    m_resource_pools[RE_FOOD]->ChangedSignal();
}

void Empire::UpdatePopulationGrowth()
{
    m_population_pool.Update();
}
