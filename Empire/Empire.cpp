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


using std::find;
using boost::lexical_cast;

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

    struct reverseComparator {
        bool operator()(int a, int b) {
            return a > b;
        }
    };

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


////////////////////////////////////////
// ProductionQueue                    //
////////////////////////////////////////

// ProductionQueue::ProductionItem
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

// ProductionQueue::Elemnt
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

// ProductionQueue
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


////////////////////////////////////////
// class Empire                       //
////////////////////////////////////////
Empire::Empire() :
    m_id(-1),
    m_homeworld_id(-1),
    m_mineral_resource_pool(RE_MINERALS),
    m_food_resource_pool(RE_FOOD),
    m_research_resource_pool(RE_RESEARCH),
    m_industry_resource_pool(RE_INDUSTRY),
    m_trade_resource_pool(RE_TRADE),
    m_population_pool(),
    m_food_total_distributed(0),
    m_maintenance_total_cost(0)
{}

Empire::Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id) :
    m_id(ID),
    m_name(name),
    m_player_name(player_name),
    m_color(color), 
    m_homeworld_id(homeworld_id), 
    m_mineral_resource_pool(RE_MINERALS),
    m_food_resource_pool(RE_FOOD),
    m_research_resource_pool(RE_RESEARCH),
    m_industry_resource_pool(RE_INDUSTRY),
    m_trade_resource_pool(RE_TRADE),
    m_population_pool(),
    m_food_total_distributed(0),
    m_maintenance_total_cost(0)
{}

Empire::~Empire()
{
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
    Empire::BuildingTypeItr item = m_available_building_types.find(name);
    return item != m_available_building_types.end();
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
    /* currently any ship design is available, but in future this will need to determine if 
       the specific design is buildable */
    return ShipDesignKept(ship_design_id);
}

bool Empire::ShipDesignKept(int ship_design_id) const {
    return (m_ship_designs.find(ship_design_id) != m_ship_designs.end());
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
    case BT_ORBITAL:
        return std::make_pair(20.0, 10); // v0.3 only
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
        return std::make_pair(ship_design->Cost(), 5); // v0.3 only
    }
    default:
        break;
    }
    return std::make_pair(-1.0, -1);
}

std::pair<double, int> Empire::ProductionCostAndTime(const ProductionQueue::ProductionItem& item) const
{
    if (item.build_type == BT_BUILDING || item.build_type == BT_ORBITAL)
        return ProductionCostAndTime(item.build_type, item.name);
    else if (item.build_type == BT_SHIP)
        return ProductionCostAndTime(item.build_type, item.design_id);
    else
        throw std::invalid_argument("Empire::ProductionCostAndTime was passed a ProductionItem with an invalid BuildType");
    return std::make_pair(-1.0, -1);
}

bool Empire::HasExploredSystem(int ID) const
{
    Empire::SystemIDItr item = find(ExploredBegin(), ExploredEnd(), ID);
    return (item != ExploredEnd());
}

bool Empire::BuildableItem(BuildType build_type, std::string name, int location) const
{
    // special case to check for ships being passed with names, not design ids
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Empire::BuildableItem was passed BuildType BT_SHIP with a name, but ship designs are tracked by number");

    if (build_type == BT_BUILDING && !BuildingTypeAvailable(name)) return false;

    //if (build_type == BT_ORBITAL) -> nothing to do yet; orbitals are currently always buildable

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

    } else if (build_type == BT_ORBITAL) {
        // this empire must be only owner of the build location
        return build_location->Owners().size() == 1 && *build_location->Owners().begin() == m_id;
    } else {
        throw std::invalid_argument("Empire::BuildableItem was passed an invalid BuildType");
    }
}

bool Empire::BuildableItem(BuildType build_type, int design_id, int location) const
{
    // special case to check for buildings or orbitals being passed with ids, not names
    if (build_type == BT_BUILDING || build_type == BT_ORBITAL)
        throw std::invalid_argument("Empire::BuildableItem was passed BuildType BT_BUILDING OR BT_ORBITAL with a design id number, but these types are tracked by name");

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

        // the design must be available
        if (!ShipDesignAvailable(design_id)) return false;

        // this empire must be only owner of the build location
        if (!( build_location->Owners().size() == 1 && *build_location->Owners().begin() == m_id)) return false;

        return true;

    } else {
        throw std::invalid_argument("Empire::BuildableItem was passed an invalid BuildType");
    }
}


bool Empire::BuildableItem(const ProductionQueue::ProductionItem& item, int location) const
{
    if (item.build_type == BT_BUILDING || item.build_type == BT_ORBITAL)
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

const std::map<const System*, int>& Empire::GetSupplyableSystems()
{
    Universe::ObjectVec object_vec = GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(m_id));
    //erase previous result - TODO erase only after begin turn
    m_sup_systems.clear();
    std::multimap<const int, const System*, reverseComparator> sortedSystems;
    //find all ResourceCenter and add it to pop_vec
    for (unsigned i = 0; i < object_vec.size(); i++){
        if (dynamic_cast<ResourceCenter*>(object_vec[i])){
            //TODO add supply rating information from system, when is implemented
            const System* sys = object_vec[i]->GetSystem();
            sortedSystems.insert(std::pair<const int, const System*>(3, sys));
            //TODO add real count
            m_sup_systems.insert(std::pair<const System*, int>(sys, 1000));
        }
    }
    //process wave until reach all systems
    //TODO remove interupted supply route system - enemy fleet and colony
    while (!sortedSystems.empty()){
        std::multimap<const int, const System*, reverseComparator>::iterator it = sortedSystems.begin();
        System::const_lane_iterator end = it->second->end_lanes();
        for (System::const_lane_iterator csi = it->second->begin_lanes(); csi!= end; csi++) {
            const System* system = dynamic_cast<const System*>(GetUniverse().Object(csi->first));
            bool add = HasExploredSystem(csi->first);
            if (add) {
                System::ConstObjectVec fleets = system->FindObjects(StationaryFleetVisitor());
                System::ConstObjectVec::iterator end = fleets.end();
                for (System::ConstObjectVec::iterator it=fleets.begin();it!=end;it++) {
                    //empire isn't beetween owner of fleet
                    if (!((*it)->Owners().count(m_id))) {
                        add = false;
                        break;
                    }
                }
            }
            //test if system isn`t allready added or is end of wave or isn`t explored
            if (add && m_sup_systems.insert(std::pair<const System*, int>(system, 1000)).second && it->first) {
                //OK, new system, lets wave flow
                sortedSystems.insert(std::pair<const int, const System*>(it->first-1, system));
            } else if (add) {
                //TODO implements how many can system produce supply
                if (m_sup_systems[system] > 1000){
                    m_sup_systems[system] = 1000;
                }
            }
        }
        sortedSystems.erase(it);
    }
    return m_sup_systems;
}

/**************************************
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

Empire::TechItr Empire::AvailableBuildingTypeBegin() const
{
    return m_available_building_types.begin();
}
Empire::TechItr Empire::AvailableBuildingTypeEnd() const
{
    return m_available_building_types.end();
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

double Empire::ProductionPoints() const
{
    return std::min(m_industry_resource_pool.Available(), m_mineral_resource_pool.Available());
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
    m_research_queue.Update(this, m_research_resource_pool.Available(), m_research_progress);
}

void Empire::RemoveTechFromQueue(const Tech* tech)
{
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (it != m_research_queue.end()) {
        m_research_queue.erase(it);
        m_research_queue.Update(this, m_research_resource_pool.Available(), m_research_progress);
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
    if (item.build_type == BT_BUILDING || item.build_type == BT_ORBITAL)
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
    m_techs.insert(name);
}

void Empire::UnlockItem(const ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    if (item.type == UIT_BUILDING)
        AddBuildingType(item.name);
}

void Empire::AddBuildingType(const std::string& name)
{
    m_available_building_types.insert(name);
}

void Empire::AddExploredSystem(int ID)
{
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

void Empire::RemoveShipDesign(int ship_design_id)
{
        m_ship_designs.erase(ship_design_id);
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

    if (new_design_id == UniverseObject::INVALID_OBJECT_ID)
        throw std::runtime_error("Unable to get new design id");

    bool success = universe.InsertShipDesignID(ship_design, new_design_id);

    if (!success)
        throw std::runtime_error("Unable to add new design to universe");

    m_ship_designs.insert(new_design_id);

    return new_design_id;
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
    // m_research_queue.Update(this, m_research_resource_pool.Available(), m_research_progress);
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
    // m_research_resource_pool.SetStockpile(m_industry_resource_pool.Available() - m_research_queue.TotalRPsSpent());
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
                // TODO: What is the mechanism for determining new fleet name?
                std::string fleet_name("New fleet ");
                fleet_name += boost::lexical_cast<std::string>(fleet_id);
                fleet->Rename(fleet_name);
                system->Insert(fleet);
                Logger().debugStream() << "New Fleet created on turn: " << fleet->CreationTurn();
  
                // add ship
                Ship *ship = new Ship(m_id, m_production_queue[i].item.design_id);
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
                Logger().debugStream() << "New Ship created on turn: " << ship->CreationTurn();

                // add sitrep
                SitRepEntry *entry = CreateShipBuiltSitRep(ship_id, system->ID());
                AddSitRepEntry(entry);
                break;
            }

            case BT_ORBITAL: {
                // v0.3 only
                Planet* planet = GetUniverse().Object<Planet>(m_production_queue[i].location);
                assert(planet);
                planet->AdjustDefBases(1);
                AddSitRepEntry(CreateBaseBuiltSitRep(planet->SystemID(), planet->ID()));
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

    m_mineral_resource_pool.SetStockpile(m_mineral_resource_pool.Available() - m_production_queue.TotalPPsSpent());
    // can uncomment following line when / if industry stockpiling is allowed...
    // m_industry_resource_pool.SetStockpile(m_industry_resource_pool.Available() - m_production_queue.TotalPPsSpent());
}

void Empire::CheckTradeSocialProgress()
{
    m_trade_resource_pool.SetStockpile(m_trade_resource_pool.Available() - m_maintenance_total_cost);
}

void Empire::CheckGrowthFoodProgress()
{
    m_food_resource_pool.SetStockpile(m_food_resource_pool.Available() - m_food_total_distributed);
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

    m_mineral_resource_pool.SetResourceCenters(res_vec);
    m_food_resource_pool.SetResourceCenters(res_vec);
    m_research_resource_pool.SetResourceCenters(res_vec);
    m_industry_resource_pool.SetResourceCenters(res_vec);
    m_trade_resource_pool.SetResourceCenters(res_vec);

    m_population_pool.SetPopCenters(pop_vec);

    UpdateResearchQueue();
    UpdateProductionQueue();
    UpdateTradeSpending();
    UpdateFoodDistribution();
    UpdatePopulationGrowth();
}

void Empire::UpdateResearchQueue()
{
    m_research_resource_pool.Update();
    m_research_queue.Update(this, m_research_resource_pool.Available(), m_research_progress);
    m_research_resource_pool.ChangedSignal();
}

void Empire::UpdateProductionQueue()
{
    m_mineral_resource_pool.Update();
    m_industry_resource_pool.Update();
    m_production_queue.Update(this, ProductionPoints(), m_production_progress);
    m_mineral_resource_pool.ChangedSignal();
    m_industry_resource_pool.ChangedSignal();
}

void Empire::UpdateTradeSpending()
{
    m_trade_resource_pool.Update(); // recalculate total trade production

    // TODO: Replace with call to some other subsystem, similar to the Update...Queue functions
    m_maintenance_total_cost = 0.0;

    Universe::ObjectVec buildings = GetUniverse().FindObjects(OwnedVisitor<Building>(m_id));
    for (Universe::ObjectVec::const_iterator it = buildings.begin(); it != buildings.end(); ++it)
    {
        Building *building = universe_object_cast<Building*>(*it);
        if (!building) continue;
        if (building->Operating())
            m_maintenance_total_cost += GetBuildingType(building->BuildingTypeName())->MaintenanceCost();
    }
    m_trade_resource_pool.ChangedSignal();
}

void Empire::UpdateFoodDistribution()
{
    m_food_resource_pool.Update();  // recalculate total food production

    double available_food = GetFoodResPool().Available();
    m_food_total_distributed = 0.0;

    std::vector<PopCenter*> pop_centers = GetPopulationPool().PopCenters(); //GetUniverse().FindObjects(OwnedVisitor<PopCenter>(m_id));
    std::vector<PopCenter*>::iterator pop_it;
    std::vector<ResourceCenter*> resource_centers = GetFoodResPool().ResourceCenters(); //GetUniverse().FindObjects(OwnedVisitor<ResourceCenter>(m_id));
    std::vector<ResourceCenter*>::iterator res_it;

    // compile map of food production of ResourceCenters, indexed by center's id
    std::map<int, double> fp_map;
    for (res_it = resource_centers.begin(); res_it != resource_centers.end(); ++res_it) {
        ResourceCenter *center = *res_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);
        fp_map[obj->ID()] = obj->MeterPoints(METER_FARMING);
    }

    // first pass: give food to PopCenters that produce food, limited by their food need and their food production
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it) {
        PopCenter *center = *pop_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);

        double need = obj->MeterPoints(METER_POPULATION);   // basic need is current population - prevents starvation
        
        // determine if, and if so how much, food this center produces locally
        double food_prod = 0.0;
        std::map<int, double>::iterator fp_map_it = fp_map.find(obj->ID());
        if (fp_map_it != fp_map.end())
            food_prod = fp_map_it->second;

        // allocate food to this PopCenter, deduct from pool, add to total food distribution tally
        double allocation = std::min(available_food, std::min(need, food_prod));

        center->SetAvailableFood(allocation);
        m_food_total_distributed += allocation;
        available_food -= allocation;
    }

    //Logger().debugStream() << "Empire::UpdateFoodDistribution: m_food_total_distributed: " << m_food_total_distributed;

    // second pass: give as much food as needed to PopCenters to maintain current population
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it) {
        PopCenter *center = *pop_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);

        double need = obj->MeterPoints(METER_POPULATION);
        double has = center->AvailableFood();
        double addition = std::min(need - has, available_food);

        center->SetAvailableFood(center->AvailableFood() + addition);
        available_food -= addition;

        m_food_total_distributed += addition;
    }

    // third pass: give as much food as needed to PopCenters to allow max possible growth
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it) {
        PopCenter *center = *pop_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        assert(obj);

        double addition = center->FuturePopGrowthMax();

        center->SetAvailableFood(center->AvailableFood() + addition);
        available_food -= addition;

        m_food_total_distributed += addition;
    }

    // after changing food distribution, population growth predictions may need to be redone
    // by calling UpdatePopulationGrowth()  

    m_food_resource_pool.ChangedSignal();
}

/** Has m_population_pool recalculate all PopCenters' and empire's total expected population growth
  * Assumes UpdateFoodDistribution() has been called to determine food allocations to each planet (which
  * are a factor in the growth prediction calculation).
  */
void Empire::UpdatePopulationGrowth()
{
    m_population_pool.Update();
}
