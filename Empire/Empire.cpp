#include "Empire.h"

#include "../parse/Parse.h"
#include "../util/Directories.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Random.h"
#include "../util/AppInterface.h"
#include "../util/OptionsDB.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../universe/Enums.h"
#include "../universe/UniverseObject.h"
#include "ResourcePool.h"
#include "EmpireManager.h"

#include <algorithm>

#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/timer.hpp>


namespace {
    const double EPSILON = 1.0e-5;

    /** sets the .allocated_rp, value for each Tech in the queue.  Only sets
      * nonzero funding to a Tech if it is researchable this turn.  Also
      * determines total number of spent RP (returning by reference in
      * total_RPs_spent) */
    void SetTechQueueElementSpending(double RPs, const std::map<std::string, double>& research_progress,
                                     const std::map<std::string, TechStatus>& research_status,
                                     ResearchQueue::QueueType& queue,
                                     double& total_RPs_spent, int& projects_in_progress)
    {
        total_RPs_spent = 0.0;
        projects_in_progress = 0;
        int i = 0;

        for (ResearchQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            // get details on what is being researched...
            const Tech* tech = GetTech(it->name);
            if (!tech) {
                Logger().errorStream() << "SetTechQueueElementSpending found null tech on research queue?!";
                continue;
            }
            std::map<std::string, TechStatus>::const_iterator status_it = research_status.find(it->name);
            if (status_it == research_status.end()) {
                Logger().errorStream() << "SetTechQueueElementSpending couldn't find tech with name " << it->name << " in the research status map";
                continue;
            }
            bool researchable = false;
            if (status_it->second == TS_RESEARCHABLE) researchable = true;

            if (researchable) {
                std::map<std::string, double>::const_iterator progress_it = research_progress.find(it->name);
                double progress = progress_it == research_progress.end() ? 0.0 : progress_it->second;
                double RPs_needed = tech->ResearchCost() - progress;
                double RPs_per_turn_limit = tech->ResearchCost() / std::max(tech->ResearchTime(), 1);
                double RPs_to_spend = std::min(RPs_needed, RPs_per_turn_limit);

                if (total_RPs_spent + RPs_to_spend <= RPs - EPSILON) {
                    it->allocated_rp = RPs_to_spend;
                    total_RPs_spent += it->allocated_rp;
                    ++projects_in_progress;
                } else if (total_RPs_spent < RPs - EPSILON) {
                    it->allocated_rp = RPs - total_RPs_spent;
                    total_RPs_spent += it->allocated_rp;
                    ++projects_in_progress;
                } else {
                    it->allocated_rp = 0.0;
                }
            } else {
                // item can't be researched this turn
                it->allocated_rp = 0.0;
            }
        }
    }

    /** sets the allocated_pp value for each Element in the passed
      * ProductionQueue \a queue.  Elements are allocated PP based on their need,
      * the limits they can be given per turn, and the amount available at their
      * production location (which is itself limited by the resource supply
      * system groups that are able to exchange resources with the build
      * location and the amount of minerals and industry produced in the group).
      * Elements will not receive funding if they cannot be built by \a empire
      * this turn, or can't be built at their build location. */
    void SetProdQueueElementSpending(Empire* empire, std::map<std::set<int>, double> available_pp,
                                     const std::vector<std::set<int> >& queue_element_resource_sharing_object_groups,
                                     const std::vector<double>& production_status, ProductionQueue::QueueType& queue,
                                     std::map<std::set<int>, double>& allocated_pp, int& projects_in_progress)
    {
        //Logger().debugStream() << "========SetProdQueueElementSpending========";
        //Logger().debugStream() << "production status: ";
        //for (std::vector<double>::const_iterator it = production_status.begin(); it != production_status.end(); ++it)
        //    Logger().debugStream() << " ... " << *it;
        //Logger().debugStream() << "queue: ";
        //for (ProductionQueue::QueueType::const_iterator it = queue.begin(); it != queue.end(); ++it)
        //    Logger().debugStream() << " ... name: " << it->item.name << "id: " << it->item.design_id << " allocated: " << it->allocated_pp << " locationid: " << it->location << " ordered: " << it->ordered;

        if (production_status.size() != queue.size() || production_status.size() != queue_element_resource_sharing_object_groups.size()) {
            Logger().errorStream() << "SetProdQueueElementSpending status size and queue size or sharing groups size inconsistent. aborting";
            return;
        }

        projects_in_progress = 0;
        allocated_pp.clear();

        //Logger().debugStream() << "queue size: " << queue.size();

        int i = 0;
        for (ProductionQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            ProductionQueue::Element& queue_element = *it;

            // get resource sharing group and amount of resource available to build this item
            const std::set<int>& group = queue_element_resource_sharing_object_groups[i];
            if (group.empty()) {
                Logger().debugStream() << "resource sharing group for queue element is empty.  no allocating any resources to element";
                queue_element.allocated_pp = 0.0;
                continue;
            }

            std::map<std::set<int>, double>::iterator available_pp_it = available_pp.find(group);
            if (available_pp_it == available_pp.end()) {
                // item is not being built at an object that has access to resources, so it can't be built.
                Logger().debugStream() << "no resource sharing group for production queue element";
                queue_element.allocated_pp = 0.0;
                continue;
            }

            double& group_pp_available = available_pp_it->second;


            // if group has no pp available, can't build anything this turn
            if (group_pp_available <= 0.0) {
                //Logger().debugStream() << "no pp available in group";
                queue_element.allocated_pp = 0.0;
                continue;
            }
            //Logger().debugStream() << "group has " << group_pp_available << " PP available";


            ProductionQueue::ProductionItem& item = queue_element.item;


            // see if item is buildable this turn...
            int location = queue_element.location;
            bool buildable = empire->BuildableItem(item, location);

            if (!buildable) {
                // can't be built at this location this turn.
                queue_element.allocated_pp = 0.0;
                Logger().debugStream() << "item can't be built at location this turn";
                continue;
            }


            // get max contribution per turn and turns to build at max contribution rate
            double item_cost;
            int build_turns;
            boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(item);
            //Logger().debugStream() << "item " << item.name << " costs " << item_cost << " for " << build_turns << " turns";


            // determine additional PP needed to complete build queue element: total cost - progress
            double element_accumulated_PP = production_status[i];                                   // PP accumulated by this element towards building next item
            double element_total_cost = item_cost * queue_element.remaining;                        // total PP to build all items in this element
            double additional_pp_to_complete_element = element_total_cost - element_accumulated_PP; // additional PP, beyond already-accumulated PP, to build all items in this element
            double element_per_turn_limit = item_cost / std::max(build_turns, 1);

            // determine how many pp to allocate to this queue element this turn.  allocation is limited by the
            // item cost, which is the max number of PP per turn that can be put towards this item, and by the
            // total cost remaining to complete the last item in the queue element (eg. the element has all but
            // the last item complete already) and by the total pp available in this element's production location's
            // resource sharing group
            double allocation = std::max(std::min(std::min(additional_pp_to_complete_element, element_per_turn_limit), group_pp_available), 0.0);       // added max (..., 0.0) to prevent any negative-allocation bugs that might come up...

            //Logger().debugStream() << "element accumulated " << element_accumulated_PP << " of total cost " << element_total_cost << " and needs " << additional_pp_to_complete_element << " more to be completed";
            //Logger().debugStream() << "... allocating " << allocation;

            // allocate pp
            queue_element.allocated_pp = allocation;

            // record alloation in group, if group is not empty
            allocated_pp[group] += allocation;  // assuming the double indexed by group will be default initialized to 0.0 if that entry doesn't already exist in the map
            group_pp_available -= allocation;

            //Logger().debugStream() << "... leaving " << group_pp_available << " PP available to group";


            // check if this will complete the element
            if (allocation >= additional_pp_to_complete_element - EPSILON)
                ++projects_in_progress;
        }
    }

    void LoadShipNames(std::vector<std::string>& names)
    {
        std::string file_name = "shipnames.txt";

        boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
        while (ifs) {
            std::string latest_name;
            std::getline(ifs, latest_name);
            if (latest_name != "") {
                names.push_back(latest_name.substr(0, latest_name.find_last_not_of(" \t") + 1)); // strip off trailing whitespace
            }
        }
    }

    /** Wrapper for boost::timer that outputs time during which this object
      * existed.  Created in the scope of a function, and passed the appropriate
      * name, it will output to Logger().debugStream() the time elapsed while
      * the function was executing. */
    class ScopedTimer {
    public:
        ScopedTimer(const std::string& timed_name = "scoped timer") :
            m_timer(),
            m_name(timed_name)
        {}
        ~ScopedTimer() {
            Logger().debugStream() << m_name << " time: " << (m_timer.elapsed() * 1000.0);
        }
    private:
        boost::timer    m_timer;
        std::string     m_name;
    };
}


////////////////////////////////////////
// ResearchQueue::Element             //
////////////////////////////////////////
ResearchQueue::Element::Element() :
    name(),
    allocated_rp(0.0),
    turns_left(0)
{}

ResearchQueue::Element::Element(const std::string& name_, double spending_, int turns_left_) :
    name(name_),
    allocated_rp(spending_),
    turns_left(turns_left_)
{}


////////////////////////////////////////
// ResearchQueue                      //
////////////////////////////////////////
ResearchQueue::ResearchQueue() :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{}

bool ResearchQueue::InQueue(const std::string& tech_name) const
{ return find(tech_name) != end(); }

int ResearchQueue::ProjectsInProgress() const
{ return m_projects_in_progress; }

double ResearchQueue::TotalRPsSpent() const
{ return m_total_RPs_spent; }

bool ResearchQueue::empty() const
{ return !m_queue.size(); }

unsigned int ResearchQueue::size() const
{ return m_queue.size(); }

ResearchQueue::const_iterator ResearchQueue::begin() const
{ return m_queue.begin(); }

ResearchQueue::const_iterator ResearchQueue::end() const
{ return m_queue.end(); }

ResearchQueue::const_iterator ResearchQueue::find(const std::string& tech_name) const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->name == tech_name)
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
        if (const Tech* tech = GetTech(it->name)) {
            if (it->allocated_rp &&
                it->allocated_rp < tech->ResearchCost()
                && 1 < it->turns_left)
            {
                return it;
            }
            return end();
        }
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

        std::map<std::string, int> simulation_results;
        // initialize simulation_results with -1 for all techs, so that any techs that aren't
        // finished in simulation by turn TOO_MANY_TURNS will be left marked as never to be finished
        for (unsigned int i = 0; i < sim_queue.size(); ++i)
            simulation_results[m_queue[i].name] = -1;

        while (!sim_queue.empty() && turns < TOO_MANY_TURNS) {
            double total_RPs_spent = 0.0;
            int projects_in_progress = 0;
            SetTechQueueElementSpending(RPs, sim_research_progress, sim_tech_status_map, sim_queue, total_RPs_spent, projects_in_progress);
            QueueType::iterator sim_q_it = sim_queue.begin();
            while (sim_q_it != sim_queue.end()) {
                const std::string& name = sim_q_it->name;
                const Tech* tech = GetTech(name);
                if (!tech) {
                    Logger().errorStream() << "ResearchQueue::Update found null tech on future simulated research queue.  skipping.";
                    ++sim_q_it;
                    continue;
                }
                double& tech_progress = sim_research_progress[name];
                tech_progress += sim_q_it->allocated_rp;
                if (tech->ResearchCost() - EPSILON <= tech_progress) {
                    sim_tech_status_map[name] = TS_COMPLETE;
                    sim_q_it->turns_left = simulation_results[name];
                    simulation_results[name] = turns;
                    sim_q_it = sim_queue.erase(sim_q_it);
                } else {
                    ++sim_q_it;
                }
            }

            // update simulated status of techs: some may be not researchable that were previously not.
            // only need to check techs that are actually on the queue, as these are the only ones
            // that might now be researched
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                const std::string& tech_name = sim_queue[i].name;
                const Tech* tech = GetTech(tech_name);
                if (!tech) continue;   // already output error message above
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
        for (unsigned int i = 0; i < m_queue.size(); ++i)
            m_queue[i].turns_left = simulation_results[m_queue[i].name];

    } else {
        // since there are so few RPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (unsigned int i = 0; i < m_queue.size(); ++i)
            m_queue[i].turns_left = -1;
    }

    ResearchQueueChangedSignal();
}

void ResearchQueue::push_back(const std::string& tech_name)
{ m_queue.push_back(Element(tech_name, 0.0, -1)); }

void ResearchQueue::insert(iterator it, const std::string& tech_name)
{ m_queue.insert(it, Element(tech_name, 0.0, -1)); }

void ResearchQueue::erase(iterator it) {
    assert(it != end());
    m_queue.erase(it);
}

ResearchQueue::iterator ResearchQueue::find(const std::string& tech_name) {
    for (iterator it = begin(); it != end(); ++it) {
        if (it->name == tech_name)
            return it;
    }
    return end();
}

ResearchQueue::iterator ResearchQueue::begin()
{ return m_queue.begin(); }

ResearchQueue::iterator ResearchQueue::end()
{ return m_queue.end(); }

ResearchQueue::iterator ResearchQueue::UnderfundedProject() {
    for (iterator it = begin(); it != end(); ++it) {
        if (const Tech* tech = GetTech(it->name)) {
            if (it->allocated_rp &&
                it->allocated_rp < tech->ResearchCost()
                && 1 < it->turns_left)
            {
                return it;
            }
            return end();
        }
    }
    return end();
}

void ResearchQueue::clear() {
    m_queue.clear();
    m_projects_in_progress = 0;
    m_total_RPs_spent = 0;
    ResearchQueueChangedSignal();
}

/////////////////////////////////////
// ProductionQueue::ProductionItem //
/////////////////////////////////////
ProductionQueue::ProductionItem::ProductionItem()
{}

ProductionQueue::ProductionItem::ProductionItem(BuildType build_type_, std::string name_) :
    build_type(build_type_),
    name(name_),
    design_id(ShipDesign::INVALID_DESIGN_ID)
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
    allocated_pp(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(ProductionItem item_, int ordered_, int remaining_, int location_) :
    item(item_),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, std::string name, int ordered_, int remaining_, int location_) :
    item(build_type, name),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, int design_id, int ordered_, int remaining_, int location_) :
    item(build_type, design_id),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

/////////////////////
// ProductionQueue //
/////////////////////
ProductionQueue::ProductionQueue() :
    m_projects_in_progress(0)
{}

int ProductionQueue::ProjectsInProgress() const
{ return m_projects_in_progress; }

double ProductionQueue::TotalPPsSpent() const
{
    // add up allocated PP from all resource sharing object groups
    double retval = 0;
    for (std::map<std::set<int>, double>::const_iterator it = m_object_group_allocated_pp.begin(); it != m_object_group_allocated_pp.end(); ++it)
        retval += it->second;
    return retval;
}

std::map<std::set<int>, double> ProductionQueue::AvailablePP(const std::map<ResourceType, boost::shared_ptr<ResourcePool> >& resource_pools) const
{
    std::map<std::set<int>, double> available_pp;

    // get resource pools used for production...
    boost::shared_ptr<ResourcePool> industry_pool, minerals_pool;
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator pool_it = resource_pools.find(RE_INDUSTRY);
    if (pool_it != resource_pools.end()) {
        industry_pool = pool_it->second;
    } else {
        Logger().errorStream() << "ProductionQueue::AvailablePP couldn't get an industry resource pool from passed resource pools";
        return available_pp;
    }
    pool_it = resource_pools.find(RE_MINERALS);
    if (pool_it != resource_pools.end()) {
        minerals_pool = pool_it->second;
    } else {
        Logger().errorStream() << "ProductionQueue::AvailablePP couldn't get a minerals resource pool from passed resource pools";
        return available_pp;
    }


    // determine available PP in each resource sharing group of systems for this empire.  PP are minimum of available minerals
    // and industry in each resource-sharing group of systems
    std::map<std::set<int>, double> available_industry = industry_pool->Available();
    std::map<std::set<int>, double> available_minerals = minerals_pool->Available();


    for (std::map<std::set<int>, double>::const_iterator ind_it = available_industry.begin(); ind_it != available_industry.end(); ++ind_it) {
        // get group of systems in industry pool
        const std::set<int>& group = ind_it->first;

        // find same group in minerals pool
        std::map<std::set<int>, double>::const_iterator min_it = available_minerals.find(group);

        if (min_it == available_minerals.end())
            continue;       // this group doesn't appear in both pools, so has no PP production

        available_pp[group] = std::min(ind_it->second, min_it->second); // available pp needs minerals and industry.  whichever is less is the available pp
    }

    return available_pp;
}

std::map<std::set<int>, double> ProductionQueue::AllocatedPP() const
{ return m_object_group_allocated_pp; }

bool ProductionQueue::empty() const
{ return !m_queue.size(); }

unsigned int ProductionQueue::size() const
{ return m_queue.size(); }

ProductionQueue::const_iterator ProductionQueue::begin() const
{ return m_queue.begin(); }

ProductionQueue::const_iterator ProductionQueue::end() const
{ return m_queue.end(); }

ProductionQueue::const_iterator ProductionQueue::find(int i) const
{ return (0 <= i && i < static_cast<int>(size())) ? (begin() + i) : end(); }

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
        if (it->allocated_pp && it->allocated_pp < item_cost && 1 < it->turns_left_to_next_item)
            return it;
    }
    return end();
}

void ProductionQueue::Update(Empire* empire, const std::map<ResourceType,
                             boost::shared_ptr<ResourcePool> >& resource_pools,
                             const std::vector<double>& production_status)
{
    if (!empire) {
        Logger().errorStream() << "ProductionQueue::Update passed null empire.  doing nothing.";
        m_projects_in_progress = 0;
        m_object_group_allocated_pp.clear();
        return;
    }

    if (m_queue.empty()) {
        //Logger().debugStream() << "ProductionQueue::Update aborting early due to an empty queue";
        m_projects_in_progress = 0;
        m_object_group_allocated_pp.clear();

        if (!production_status.empty())
            Logger().errorStream() << "warning: ProductionQueue::Update queue was empty, but passed production_status was not.";

        ProductionQueueChangedSignal(); // need this so BuildingsPanel updates properly after removing last building
        return;                         // nothing to do for an empty queue
    }

    ScopedTimer update_timer("ProductionQueue::Update");


    std::map<std::set<int>, double> available_pp = AvailablePP(resource_pools);


    // determine which resource sharing group each queue item is located in
    std::vector<std::set<int> > queue_element_groups;
    for (ProductionQueue::const_iterator queue_it = m_queue.begin(); queue_it != m_queue.end(); ++queue_it) {
        // get location object for element
        const ProductionQueue::Element& element = *queue_it;
        int location_id = element.location;

        // search through groups to find object
        for (std::map<std::set<int>, double>::const_iterator groups_it = available_pp.begin(); true; ++groups_it) {
            if (groups_it == available_pp.end()) {
                // didn't find a group containing this object, so add an empty group as this element's queue element group
                queue_element_groups.push_back(std::set<int>());
                break;
            }

            // check if location object id is in this group
            const std::set<int>& group = groups_it->first;
            std::set<int>::const_iterator set_it = group.find(location_id);
            if (set_it != group.end()) {
                // system is in this group.
                queue_element_groups.push_back(group);  // record this discovery
                break;                                  // stop searching for a group containing a system, since one has been found
            }
        }
    }


    // allocate pp to queue elements, returning updated available pp and updated
    // allocated pp for each group of resource sharing objects
    SetProdQueueElementSpending(empire, available_pp, queue_element_groups, production_status,
                                m_queue, m_object_group_allocated_pp, m_projects_in_progress);


    // if at least one resource-sharing system group have available PP, simulate
    // future turns to predict when build items will be finished
    bool simulate_future = false;
    for (std::map<std::set<int>, double>::const_iterator available_it = available_pp.begin();
         available_it != available_pp.end(); ++available_it)
    {
        if (available_it->second > EPSILON) {
            simulate_future = true;
            break;
        }
    }


    if (!simulate_future) {
        Logger().debugStream() << "not enough PP to be worth simulating future turns production.  marking everything as never complete";
        // since there are so few PPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (ProductionQueue::QueueType::iterator queue_it = m_queue.begin(); queue_it != m_queue.end(); ++queue_it) {
            queue_it->turns_left_to_next_item = -1;     // -1 is sentinel value indicating never to be complete.  ProductionWnd checks for turns to completeion less than 0 and displays "NEVER" when appropriate
            queue_it->turns_left_to_completion = -1;
        }
        ProductionQueueChangedSignal();
        return;
    }


    // there are enough PP available in at least one group to make it worthwhile to simulate the future.
    Logger().debugStream() << "ProductionQueue::Update: Simulating future turns of production queue";


    // duplicate production queue state for future simulation
    QueueType sim_queue = m_queue;
    std::vector<double>         sim_production_status = production_status;
    std::vector<std::set<int> > sim_queue_element_groups = queue_element_groups;
    std::vector<int>            simulation_results(sim_production_status.size(), -1);
    std::vector<int>            sim_queue_original_indices(sim_production_status.size());
    for (unsigned int i = 0; i < sim_queue_original_indices.size(); ++i)
        sim_queue_original_indices[i] = i;


    int turns = 1;  // to keep track of how man turn-iterations simulation takes to finish items
    const int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops



    // remove from simulated queue any items that can't be built due to not
    // meeting their location conditions might be better to re-check
    // buildability each turn, but this would require creating a simulated
    // universe into which simulated completed buildings could be inserted, as
    // well as spoofing the current turn, or otherwise faking the results for
    // evaluating arbitrary location conditions for the simulated universe.
    // this would also be inaccurate anyway due to player choices or random
    // chance, so for simplicity, it is assumed that building location
    // conditions evaluated at the present turn apply indefinitely
    //
    // also remove from simulated queue any items that are located in a resource
    // sharing object group that is empty or that does not have any PP available
    for (unsigned int i = 0; i < sim_queue.size(); ++i) {
        const std::set<int>& group = queue_element_groups[sim_queue_original_indices[i]];

        // if any removal condition is met, remove item from queue
        bool remove = false;
        if (group.empty() || !empire->BuildableItem(sim_queue[i].item, sim_queue[i].location)) {        // empty group or not buildable
            remove = true;
        } else {
            std::map<std::set<int>, double>::const_iterator available_it = available_pp.find(group);
            if (available_it == available_pp.end() || available_it->second < EPSILON)                   // missing group or non-empty group with no PP available
                remove = true;
        }

        if (remove) {
            // remove unbuildable items from the simulated queue, since they'll never finish...
            m_queue[sim_queue_original_indices[i]].turns_left_to_completion = -1;   // turns left is indeterminate for this item
            sim_queue.erase(sim_queue.begin() + i);
            sim_production_status.erase(sim_production_status.begin() + i);
            sim_queue_element_groups.erase(sim_queue_element_groups.begin() + i);
            sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
        }
    }



    // cycle through items on queue, adding up their allotted PP until each is
    // finished and removed from queue until everything on queue has been
    // finished, in order to calculate expected completion times
    while (!sim_queue.empty() && turns < TOO_MANY_TURNS) {
        std::map<std::set<int>, double> allocated_pp;
        int projects_in_progress = 0;


        // update allocation of PP on the simulated queue.  previous iterations
        // of simulation may have removed elements from the queue, freeing up PP
        // to be spent on other elements further down the queue
        SetProdQueueElementSpending(empire, available_pp, sim_queue_element_groups, sim_production_status,
                                    sim_queue, allocated_pp, projects_in_progress);


        // cycle through items on queue, apply one turn's PP towards items, remove items that are done
        for (unsigned int i = 0; i < sim_queue.size(); ++i) {
            ProductionQueue::Element& element = sim_queue[i];

            double item_cost;
            int build_turns;
            boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(element.item);


            double& status = sim_production_status[i];  // get previous iteration's accumulated PP for this element
            status += element.allocated_pp;             // add one turn's allocated PP to element


            // check if additional turn's PP allocation was enough to finish next item in element
            if (item_cost - EPSILON <= status) {
                // an item has been completed.

                // deduct cost of one item from accumulated PP.  don't set
                // accumulation to zero, as this would eliminate any partial
                // completion of the next item
                sim_production_status[i] -= item_cost;

                // if this was the first item in the element to be completed in
                // this simuation, update the original queue element with the
                // turns required to complete the next item in the element
                if (element.remaining == m_queue[sim_queue_original_indices[i]].remaining)
                    m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = turns;

                // if all items in the element are completed in the simulation...
                if (!--element.remaining) {
                    //Logger().debugStream() << "    ITEM COMPLETE!  REMOVING";
                    m_queue[sim_queue_original_indices[i]].turns_left_to_completion = turns;    // record the (estimated) turns to complete the whole element on the original queue
                    sim_queue.erase(sim_queue.begin() + i);                                     // remove the completed item from the simulated queue
                    sim_production_status.erase(sim_production_status.begin() + i);             // and production status
                    sim_queue_element_groups.erase(sim_queue_element_groups.begin() + i);       // and the group of systems this element could access resources from
                    sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--); // and bookkeeping
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


    ProductionQueueChangedSignal();
}

void ProductionQueue::push_back(const Element& element)
{ m_queue.push_back(element); }

void ProductionQueue::insert(iterator it, const Element& element)
{ m_queue.insert(it, element); }

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
{ return m_queue.begin(); }

ProductionQueue::iterator ProductionQueue::end()
{ return m_queue.end(); }

ProductionQueue::iterator ProductionQueue::find(int i)
{ return (0 <= i && i < static_cast<int>(size())) ? (begin() + i) : end(); }

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
        if (it->allocated_pp && it->allocated_pp < item_cost && 1 < it->turns_left_to_next_item)
            return it;
    }
    return end();
}

void ProductionQueue::clear()
{
    m_queue.clear();
    m_projects_in_progress = 0;
    m_object_group_allocated_pp.clear();
    ProductionQueueChangedSignal();
}

////////////////
// Alignments //
////////////////
const std::string& Alignment::Name() const
{ return m_name; }

const std::string& Alignment::Description() const
{ return m_description; }

const std::string& Alignment::Graphic() const
{ return m_graphic; }


namespace {
    class AlignmentManager {
    public:
        /** \name Accessors */ //@{
        const std::vector<Alignment>&           Alignments() const { return m_alignments; }

        const std::vector<boost::shared_ptr<
            const Effect::EffectsGroup> >       EffectsGroups() const { return m_effects_groups; }
        //@}

        /** returns the instance of this singleton class; you should use the
          * free function GetAlignmentManager() instead */
        static const AlignmentManager& GetAlignmentManager();

    private:
        AlignmentManager();

        std::vector<Alignment>              m_alignments;
        std::vector<boost::shared_ptr<
            const Effect::EffectsGroup> >   m_effects_groups;

        static AlignmentManager*    s_instance;
    };
    // static(s)
    AlignmentManager* AlignmentManager::s_instance = 0;

    const AlignmentManager& AlignmentManager::GetAlignmentManager() {
        static AlignmentManager manager;
        return manager;
    }

    AlignmentManager::AlignmentManager() {
        if (s_instance)
            throw std::runtime_error("Attempted to create more than one AlignmentManager.");

        s_instance = this;

        Logger().debugStream() << "Initializing AlignmentManager";

        parse::alignments(GetResourceDir() / "alignments.txt", m_alignments, m_effects_groups);

        if (GetOptionsDB().Get<bool>("verbose-logging")) {
            Logger().debugStream() << "Alignments:";
            for (std::vector<Alignment>::const_iterator it = m_alignments.begin(); it != m_alignments.end(); ++it) {
                const Alignment& p = *it;
                Logger().debugStream() << " ... " << p.Name();
            }
            Logger().debugStream() << "Alignment Effects:";
            for (std::vector<boost::shared_ptr<const Effect::EffectsGroup> >::const_iterator it = m_effects_groups.begin();
                 it != m_effects_groups.end(); ++it)
            {
                //const boost::shared_ptr<const Effect::EffectsGroup>& p = *it;
                Logger().debugStream() << " ... " /*<< p->Dump()*/;
            }
        }
    }

    /** returns the singleton alignment manager */
    const AlignmentManager& GetAlignmentManager() {
        return AlignmentManager::GetAlignmentManager();
    }
};


////////////
// Empire //
////////////
Empire::Empire() :
    m_id(-1),
    m_capital_id(UniverseObject::INVALID_OBJECT_ID),
    m_resource_pools(),
    m_population_pool(),
    m_maintenance_total_cost(0)
{ Init(); }

Empire::Empire(const std::string& name, const std::string& player_name, int empire_id, const GG::Clr& color) :
    m_id(empire_id),
    m_name(name),
    m_player_name(player_name),
    m_color(color),
    m_capital_id(UniverseObject::INVALID_OBJECT_ID),
    m_resource_pools(),
    m_population_pool(),
    m_maintenance_total_cost(0)
{
    Logger().debugStream() << "Empire::Empire(" << name << ", " << player_name << ", " << empire_id << ", colour)";
    Init();
}

void Empire::Init()
{
    m_resource_pools[RE_MINERALS] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_MINERALS));
    m_resource_pools[RE_FOOD] =     boost::shared_ptr<ResourcePool>(new ResourcePool(RE_FOOD));
    m_resource_pools[RE_RESEARCH] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_RESEARCH));
    m_resource_pools[RE_INDUSTRY] = boost::shared_ptr<ResourcePool>(new ResourcePool(RE_INDUSTRY));
    m_resource_pools[RE_TRADE] =    boost::shared_ptr<ResourcePool>(new ResourcePool(RE_TRADE));

    // Add alignment meters to empire
    const AlignmentManager& alignment_manager = GetAlignmentManager();
    const std::vector<Alignment>& alignments = alignment_manager.Alignments();
    for (std::vector<Alignment>::const_iterator it = alignments.begin(); it != alignments.end(); ++it) {
        const Alignment& alignment = *it;
        m_meters[alignment.Name()];
    }
}

Empire::~Empire()
{ ClearSitRep(); }

const std::string& Empire::Name() const
{ return m_name; }

const std::string& Empire::PlayerName() const
{ return m_player_name; }

int Empire::EmpireID() const
{ return m_id; }

const GG::Clr& Empire::Color() const
{ return m_color; }

int Empire::CapitalID() const
{ return m_capital_id; }

int Empire::StockpileID(ResourceType res) const
{
    switch (res) {
    case RE_MINERALS:
    case RE_FOOD:
    case RE_TRADE:
        return m_capital_id;
        break;
    case RE_INDUSTRY:
    case RE_RESEARCH:
    default:
        return UniverseObject::INVALID_OBJECT_ID;
        break;
    }
}

void Empire::SetCapitalID(int id)
{ m_capital_id = id; }

Meter* Empire::GetMeter(const std::string& name)
{
    std::map<std::string, Meter>::iterator it = m_meters.find(name);
    if (it != m_meters.end())
        return &(it->second);
    else
        return 0;
}

const Meter* Empire::GetMeter(const std::string& name) const {
    std::map<std::string, Meter>::const_iterator it = m_meters.find(name);
    if (it != m_meters.end())
        return &(it->second);
    else
        return 0;
}

bool Empire::ResearchableTech(const std::string& name) const {
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
{ return m_research_queue; }

double Empire::ResearchProgress(const std::string& name) const {
    std::map<std::string, double>::const_iterator it = m_research_progress.find(name);
    return (it == m_research_progress.end()) ? 0.0 : it->second;
}

const std::set<std::string>& Empire::AvailableTechs() const
{ return m_techs; }

bool Empire::TechResearched(const std::string& name) const
{ return m_techs.find(name) != m_techs.end(); }

TechStatus Empire::GetTechStatus(const std::string& name) const {
    if (TechResearched(name)) return TS_COMPLETE;
    if (ResearchableTech(name)) return TS_RESEARCHABLE;
    return TS_UNRESEARCHABLE;
}

const std::set<std::string>& Empire::AvailableBuildingTypes() const
{ return m_available_building_types; }

bool Empire::BuildingTypeAvailable(const std::string& name) const
{ return m_available_building_types.find(name) != m_available_building_types.end(); }

const std::set<int>& Empire::ShipDesigns() const
{ return m_ship_designs; }

std::set<int> Empire::AvailableShipDesigns() const {
    // create new map containing all ship designs that are available
    std::set<int> retval;
    for (ShipDesignItr it = m_ship_designs.begin(); it != m_ship_designs.end(); ++it) {
        if (ShipDesignAvailable(*it))
            retval.insert(*it);
    }
    return retval;
}

bool Empire::ShipDesignAvailable(int ship_design_id) const {
    // if design isn't kept by this empire, it can't be built.
    if (!ShipDesignKept(ship_design_id))
        return false;   //   The empire needs to issue a ShipDesignOrder to add this design id to its kept designs

    const ShipDesign* design = GetShipDesign(ship_design_id);
    if (!design || !design->Producible()) return false;

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

bool Empire::ShipDesignKept(int ship_design_id) const
{ return (m_ship_designs.find(ship_design_id) != m_ship_designs.end()); }

const std::set<std::string>& Empire::AvailableShipParts() const
{ return m_available_part_types; }

bool Empire::ShipPartAvailable(const std::string& name) const
{ return m_available_part_types.find(name) != m_available_part_types.end(); }

const std::set<std::string>& Empire::AvailableShipHulls() const
{ return m_available_hull_types; }

bool Empire::ShipHullAvailable(const std::string& name) const
{ return m_available_hull_types.find(name) != m_available_hull_types.end(); }

const ProductionQueue& Empire::GetProductionQueue() const
{ return m_production_queue; }

double Empire::ProductionStatus(int i) const
{ return (0 <= i && i < static_cast<int>(m_production_progress.size())) ? m_production_progress[i] : -1.0; }

std::pair<double, int> Empire::ProductionCostAndTime(BuildType build_type, std::string name) const {
    switch (build_type) {
    case BT_BUILDING: {
        const BuildingType* building_type = GetBuildingType(name);
        if (!building_type)
            break;
        return std::make_pair(building_type->ProductionCost(), building_type->ProductionTime());
    }
    default:
        break;
    }
    return std::make_pair(-1.0, -1);
}

std::pair<double, int> Empire::ProductionCostAndTime(BuildType build_type, int design_id) const {
    switch (build_type) {
    case BT_SHIP: {
        const ShipDesign* ship_design = GetShipDesign(design_id);
        if (!ship_design)
            break;
        return std::make_pair(ship_design->ProductionCost(), ship_design->ProductionTime());
    }
    default:
        break;
    }
    return std::make_pair(-1.0, -1);
}

std::pair<double, int> Empire::ProductionCostAndTime(const ProductionQueue::ProductionItem& item) const {
    if (item.build_type == BT_BUILDING)
        return ProductionCostAndTime(BT_BUILDING, item.name);
    else if (item.build_type == BT_SHIP)
        return ProductionCostAndTime(BT_SHIP, item.design_id);
    else
        throw std::invalid_argument("Empire::ProductionCostAndTime was passed a ProductionItem with an invalid BuildType");
    return std::make_pair(-1.0, -1);
}

bool Empire::HasExploredSystem(int ID) const
{ return (m_explored_systems.find(ID) != m_explored_systems.end()); }

bool Empire::BuildableItem(BuildType build_type, const std::string& name, int location) const {
    // special case to check for ships being passed with names, not design ids
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Empire::BuildableItem was passed BuildType BT_SHIP with a name, but ship designs are tracked by number");

    if (build_type == BT_BUILDING && !BuildingTypeAvailable(name)) return false;

    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible()) return false;

    if (ProductionCostAndTime(build_type, name) == std::make_pair(-1.0, -1)) {
        // item is unknown, unavailable, or invalid.
        return false;
    }

    UniverseObject* build_location = GetObject(location);
    if (!build_location) return false;

    if (build_type == BT_BUILDING) {
        // specified location must be a valid production location for that building type
        return building_type->ProductionLocation(m_id, location);

    } else {
        Logger().errorStream() << "Empire::BuildableItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::BuildableItem(BuildType build_type, int design_id, int location) const {
    // special case to check for buildings being passed with ids, not names
    if (build_type == BT_BUILDING)
        throw std::invalid_argument("Empire::BuildableItem was passed BuildType BT_BUILDING with a design id number, but these types are tracked by name");

    if (build_type == BT_SHIP && !ShipDesignAvailable(design_id)) return false;

    // design must be known to this empire
    const ShipDesign* ship_design = GetShipDesign(design_id);
    if (!ship_design || !ship_design->Producible()) return false;


    if (ProductionCostAndTime(build_type, design_id) == std::make_pair(-1.0, ShipDesign::INVALID_DESIGN_ID)) {
        // item is unknown, unavailable, or invalid.
        return false;
    }

    UniverseObject* build_location = GetObject(location);
    if (!build_location) return false;

    if (build_type == BT_SHIP) {
        // specified location must be a valid production location for this design
        return ship_design->ProductionLocation(m_id, location);

    } else {
        Logger().errorStream() << "Empire::BuildableItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::BuildableItem(const ProductionQueue::ProductionItem& item, int location) const {
    if (item.build_type == BT_BUILDING)
        return BuildableItem(item.build_type, item.name, location);
    else if (item.build_type == BT_SHIP)
        return BuildableItem(item.build_type, item.design_id, location);
    else
        throw std::invalid_argument("Empire::BuildableItem was passed a ProductionItem with an invalid BuildType");
    return false;
}

int Empire::NumSitRepEntries() const
{ return m_sitrep_entries.size(); }

void Empire::EliminationCleanup() {
    // some Empire data not cleared when eliminating since it might be useful
    // to remember later, and having it doesn't hurt anything (as opposed to
    // the production queue that might actually cause some problems if left
    // uncleared after elimination

    m_capital_id = UniverseObject::INVALID_OBJECT_ID;
    // m_techs
    m_research_queue.clear();
    m_research_progress.clear();
    m_production_queue.clear();
    m_production_progress.clear();
    // m_available_building_types;
    // m_available_part_types;
    // m_available_hull_types;
    // m_explored_systems;
    // m_ship_designs;
    m_sitrep_entries.clear();
    for (std::map<ResourceType, boost::shared_ptr<ResourcePool> >::iterator it = m_resource_pools.begin();
         it != m_resource_pools.end(); ++it)
    {
        it->second->SetObjects(std::vector<int>());
    }
    m_population_pool.SetPopCenters(std::vector<int>());
    m_maintenance_total_cost = 0;
    // m_ship_names_used;
    m_fleet_supplyable_system_ids.clear();
    m_fleet_supply_starlane_traversals.clear();
    m_fleet_supply_system_ranges.clear();
    m_resource_supply_groups.clear();
    m_resource_supply_starlane_traversals.clear();
    m_resource_supply_obstructed_starlane_traversals.clear();
    m_resource_supply_system_ranges.clear();
    m_supply_unobstructed_systems.clear();
}

void Empire::UpdateSystemSupplyRanges(const std::set<int>& known_objects) {
    //std::cout << "Empire::UpdateSystemSupplyRanges() for empire " << this->Name() << std::endl;
    m_fleet_supply_system_ranges.clear();
    m_resource_supply_system_ranges.clear();

    const ObjectMap& objects = GetMainObjectMap();

    // as of this writing, only planets can distribute supplies to fleets or other planets.  If other objects
    // get the ability to distribute supplies, this should be expanded to them as well
    std::vector<const UniverseObject*> owned_planets;
    for (std::set<int>::const_iterator it = known_objects.begin(); it != known_objects.end(); ++it) {
        if (const Planet* planet = objects.Object<Planet>(*it))
            if (planet->OwnedBy(this->EmpireID()))
                owned_planets.push_back(planet);
    }

    //std::cout << "... empire owns " << owned_planets.size() << " planets" << std::endl;
    for (std::vector<const UniverseObject*>::const_iterator it = owned_planets.begin(); it != owned_planets.end(); ++it) {
        const UniverseObject* obj = *it;
        //std::cout << "... considering owned planet: " << obj->Name() << std::endl;

        // ensure object is within a system, from which it can distribute supplies
        int system_id = obj->SystemID();
        if (system_id == UniverseObject::INVALID_OBJECT_ID)
            continue;   // TODO: consider future special case if current object is itself a system

        // check if object has a construction meter
        if (obj->GetMeter(METER_CONSTRUCTION)) {
            // get resource supply range for next turn for this object
            int resource_supply_range = static_cast<int>(floor(obj->NextTurnCurrentMeterValue(METER_CONSTRUCTION) / 20.0));

            // if this object can provide more resource supply range than the best previously checked object in this system, record its range as the new best for the system
            std::map<int, int>::iterator system_it = m_resource_supply_system_ranges.find(system_id);               // try to find a previous entry for this system's supply range
            if (system_it == m_resource_supply_system_ranges.end() || resource_supply_range > system_it->second) {  // if there is no previous entry, or the previous entry is shorter than the new one, add or replace the entry
                //std::cout << " ... object " << obj->Name() << " has resource supply range: " << resource_supply_range << std::endl;
                m_resource_supply_system_ranges[system_id] = resource_supply_range;
            }
        }

        // check if object has a supply meter
        if (obj->GetMeter(METER_SUPPLY)) {
            // get fleet supply range for next turn for this object
            int fleet_supply_range = static_cast<int>(floor(obj->NextTurnCurrentMeterValue(METER_SUPPLY)));

            // if this object can provide more supply than the best previously checked object in this system, record its range as the new best for the system
            std::map<int, int>::iterator system_it = m_fleet_supply_system_ranges.find(system_id);          // try to find a previous entry for this system's supply range
            if (system_it == m_fleet_supply_system_ranges.end() || fleet_supply_range > system_it->second) {// if there is no previous entry, or the previous entry is shorter than the new one, add or replace the entry
                m_fleet_supply_system_ranges[system_id] = fleet_supply_range;
                //std::cout << " ... object " << obj->Name() << " has fleet supply range: " << fleet_supply_range << std::endl;
            }
        }
    }
}

void Empire::UpdateSystemSupplyRanges() {
    const Universe& universe = GetUniverse();
    const ObjectMap& empire_known_objects = universe.EmpireKnownObjects(this->EmpireID());

    // get ids of objects partially or better visible to this empire.
    std::vector<int> known_objects_vec = empire_known_objects.FindObjectIDs();
    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());

    std::set<int> known_objects_set;

    // exclude objects known to have been destroyed (or rather, include ones that aren't known by this empire to be destroyed)
    for (std::vector<int>::const_iterator it = known_objects_vec.begin(); it != known_objects_vec.end(); ++it)
        if (known_destroyed_objects.find(*it) == known_destroyed_objects.end())
            known_objects_set.insert(*it);
    UpdateSystemSupplyRanges(known_objects_set);
}

void Empire::UpdateSupplyUnobstructedSystems() {
    Universe& universe = GetUniverse();

    // get ids of systems partially or better visible to this empire.
    // TODO: make a UniverseObjectVisitor for objects visible to an empire at a specified visibility or greater
    std::vector<int> known_systems_vec = universe.EmpireKnownObjects(this->EmpireID()).FindObjectIDs<System>();
    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());

    std::set<int> known_systems_set;

    // exclude systems known to have been destroyed (or rather, include ones that aren't known to be destroyed)
    for (std::vector<int>::const_iterator it = known_systems_vec.begin(); it != known_systems_vec.end(); ++it)
        if (known_destroyed_objects.find(*it) == known_destroyed_objects.end())
            known_systems_set.insert(*it);
    UpdateSupplyUnobstructedSystems(known_systems_set);
}

void Empire::UpdateSupplyUnobstructedSystems(const std::set<int>& known_systems) {
    m_supply_unobstructed_systems.clear();

    // get systems with historically at least partial visibility
    std::set<int> systems_with_at_least_partial_visibility_at_some_point;
    for (std::set<int>::const_iterator sys_it = known_systems.begin(); sys_it != known_systems.end(); ++sys_it) {
        const Universe::VisibilityTurnMap& vis_turns = GetUniverse().GetObjectVisibilityTurnMapByEmpire(*sys_it, m_id);
        if (vis_turns.find(VIS_PARTIAL_VISIBILITY) != vis_turns.end())
            systems_with_at_least_partial_visibility_at_some_point.insert(*sys_it);
    }

    // get all fleets, or just fleets visible to this client's empire
    const std::vector<Fleet*> fleets = GetUniverse().Objects().FindObjects<Fleet>();

    // find systems that contain friendly fleets or objects that can block supply
    std::set<int> systems_containing_friendly_fleets;
    std::set<int> systems_containing_obstructing_objects;
    for (std::vector<Fleet*>::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
        const Fleet* fleet = *it;
        int system_id = fleet->SystemID();
        if (system_id == UniverseObject::INVALID_OBJECT_ID/* || known_systems.find(system_id) == known_systems.end()*/)
            continue;   // not in a (potential supply unobstructed) system
        else if (fleet->OwnedBy(m_id))
            systems_containing_friendly_fleets.insert(system_id);
        else            // owned by another empire, or has no owners
            systems_containing_obstructing_objects.insert(system_id);
    }

    // check each potential supplyable system for whether it can propegate supply.
    for (std::set<int>::const_iterator known_systems_it = known_systems.begin(); known_systems_it != known_systems.end(); ++known_systems_it) {
        int sys_id = *known_systems_it;

        // has empire ever seen this system with partial or better visibility?
        if (systems_with_at_least_partial_visibility_at_some_point.find(sys_id) == systems_with_at_least_partial_visibility_at_some_point.end())
            continue;

        // if system is explored, then whether it can propegate supply depends
        // on what friendly / enemy ships are in the system

        if (systems_containing_friendly_fleets.find(sys_id) != systems_containing_friendly_fleets.end()) {
            // if there are friendly ships, supply can propegate
            m_supply_unobstructed_systems.insert(sys_id);
        } else if (systems_containing_obstructing_objects.find(sys_id) == systems_containing_obstructing_objects.end()) {
            // if there are no friendly ships and no enemy ships, supply can propegate
            m_supply_unobstructed_systems.insert(sys_id);
        }
        // otherwise, system contains no friendly fleets but does contain an
        // unfriendly fleet, so it is obstructed, so isn't included in the
        // unobstructed systems set
    }
}

void Empire::UpdateFleetSupply()
{ UpdateFleetSupply(this->KnownStarlanes()); }

void Empire::UpdateFleetSupply(const std::map<int, std::set<int> >& starlanes) {
    //std::cout << "Empire::UpdateFleetSupply for empire " << this->Name() << std::endl;

    m_fleet_supplyable_system_ids.clear();
    m_fleet_supply_starlane_traversals.clear();

    // store range of all systems before propegation of supply in working map used to propegate that range to other systems.
    std::map<int, int> propegating_fleet_supply_ranges = m_fleet_supply_system_ranges;

    // insert all systems that produce supply on their own into a list of systems to process
    std::list<int> propegating_systems_list;    // working list of systems to propegate supply from
    for (std::map<int, int>::const_iterator it = propegating_fleet_supply_ranges.begin(); it != propegating_fleet_supply_ranges.end(); ++it)
        propegating_systems_list.push_back(it->first);

    // iterate through list of accessible systems, processing each in order it was added (like breadth first search) until no
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
        std::map<int, std::set<int> >::const_iterator system_it = starlanes.find(cur_sys_id);
        if (system_it == starlanes.end()) {
            // no starlanes out of this system
            ++sys_list_it;
            continue;
        }

        const std::set<int>& starlane_ends = system_it->second;
        for (std::set<int>::const_iterator lane_it = starlane_ends.begin(); lane_it != starlane_ends.end(); ++lane_it) {
            int lane_end_sys_id = *lane_it;

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
{ UpdateResourceSupply(this->KnownStarlanes()); }

void Empire::UpdateResourceSupply(const std::map<int, std::set<int> >& starlanes)
{
    //std::cout << "Empire::UpdateResourceSupply for empire " << this->Name() << std::endl;

    // need to get a set of sets of systems that can exchange resources.  some sets may be just one system,
    // in which resources can be exchanged between UniverseObjects producing or consuming them, but which
    // can't exchange with any other systems.

    m_resource_supply_groups.clear();
    m_resource_supply_starlane_traversals.clear();
    m_resource_supply_obstructed_starlane_traversals.clear();

    // map from system id to set of systems that are supply-connected to it directly (which may involve
    // multiple starlane jumps
    std::map<int, std::set<int> > supply_groups_map;

    // all systems that can supply another system or within themselves, or can be supplied by another system.
    // need to keep track of this so that only these systems are put into the boost adjacency graph.  if
    // additional systems were put in, they would be returned as being in their own "connected" component and
    // would have to be filtered out of the results before being returned
    std::set<int> all_supply_exchanging_systems;


    // loop through systems, getting set of systems that can be supplied by each.  (may be an empty set for
    // some systems that cannot supply within themselves, or may contain only source systems, or could contain
    // multiple other systems)
    //std::cout << " ... m_supply_unobstructed_systems.size(): " << m_supply_unobstructed_systems.size() << std::endl;
    for (std::set<int>::const_iterator source_sys_it = m_supply_unobstructed_systems.begin(); source_sys_it != m_supply_unobstructed_systems.end(); ++source_sys_it) {
        int source_sys_id = *source_sys_it;

        //// DEBUG
        //const UniverseObject* asdf = GetUniverse().Object(source_sys_id);
        //std::cout << " .. unobstructed system : " << asdf->Name() << std::endl;

        // skip systems that don't have any supply to propegate.
        std::map<int, int>::const_iterator system_supply_it = m_resource_supply_system_ranges.find(source_sys_id);
        if (system_supply_it == m_resource_supply_system_ranges.end())
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

        //std::cout << " ..... can propegate suppply " << system_supply_it->second << " jumps" << std::endl;


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
            std::map<int, std::set<int> >::const_iterator system_it = starlanes.find(cur_sys_id);
            if (system_it == starlanes.end()) {
                ++sys_list_it;
                continue; // no starlanes out of this system
            }

            const std::set<int>& starlane_ends = system_it->second;
            for (std::set<int>::const_iterator lane_it = starlane_ends.begin(); lane_it != starlane_ends.end(); ++lane_it) {
                int lane_end_sys_id = *lane_it;

                // ensure this adjacent system is unobstructed
                if (m_supply_unobstructed_systems.find(lane_end_sys_id) == m_supply_unobstructed_systems.end()) {
                    // can't propegate here
                    m_resource_supply_obstructed_starlane_traversals.insert(std::make_pair(cur_sys_id, lane_end_sys_id));
                    continue;
                }

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

    //// DEBUG
    //Logger().debugStream() << "resource supply traversals:";
    //for (std::set<std::pair<int, int> >::const_iterator it = m_resource_supply_starlane_traversals.begin(); it != m_resource_supply_starlane_traversals.end(); ++it) {
    //    Logger().debugStream() << " ... from: " << GetUniverse().Object(it->first)->Name() << " to: " << GetUniverse().Object(it->second)->Name();
    //}
    //
    //Logger().debugStream() << "obstructed resource supply traversals:";
    //for (std::set<std::pair<int, int> >::const_iterator it = m_resource_supply_obstructed_starlane_traversals.begin(); it != m_resource_supply_obstructed_starlane_traversals.end(); ++it) {
    //    const UniverseObject* from = GetUniverse().Object(it->first);
    //    const UniverseObject* to = GetUniverse().Object(it->second);
    //    if (from && to)
    //        Logger().debugStream() << " ... from: " << from->Name() << " to: " << to->Name();
    //    else if (from)
    //        Logger().debugStream() << " ... from: " << from->Name() << " to id: " << it->second;
    //    else
    //        Logger().debugStream() << " ... from id: " << it->first << " to id: " << it->second;
    //}

    if (supply_groups_map.empty()) return;  // need to avoid going to boost graph stuff below, which doesn't seem to like being fed empty graphs...


    // Need to merge interconnected supply groups into as few sets of mutually-supply-exchanging systems
    // as possible.  This requires finding the connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.

    // create graph
    boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

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

        //// DEBUG!
        //Logger().debugStream() << "Set: ";
        //for (std::set<int>::const_iterator set_it = map_it->second.begin(); set_it != map_it->second.end(); ++set_it) {
        //    const UniverseObject* obj = GetUniverse().Object(*set_it);
        //    if (!obj) {
        //        Logger().debugStream() << " ... missing object!";
        //        continue;
        //    }
        //    Logger().debugStream() << " ... " << obj->Name();
        //}
    }
}

const std::set<int>& Empire::FleetSupplyableSystemIDs() const
{ return m_fleet_supplyable_system_ids; }

const std::set<std::pair<int, int> >& Empire::FleetSupplyStarlaneTraversals() const
{ return m_fleet_supply_starlane_traversals; }

const std::map<int, int>& Empire::FleetSupplyRanges() const
{ return m_fleet_supply_system_ranges; }

const std::set<std::set<int> >& Empire::ResourceSupplyGroups() const
{ return m_resource_supply_groups; }

const std::set<std::pair<int, int> >& Empire::ResourceSupplyStarlaneTraversals() const
{ return m_resource_supply_starlane_traversals; }

const std::set<std::pair<int, int> >& Empire::ResourceSupplyOstructedStarlaneTraversals() const
{ return m_resource_supply_obstructed_starlane_traversals; }

const std::map<int, int>& Empire::ResourceSupplyRanges() const
{ return m_resource_supply_system_ranges; }

const std::set<int>& Empire::SupplyUnobstructedSystems() const
{ return m_supply_unobstructed_systems; }

bool Empire::FleetOrResourceSupplyableAtSystem(int system_id) const
{
    if (system_id == UniverseObject::INVALID_OBJECT_ID)
        return false;

    // check fleet supplyable systems
    if (m_fleet_supplyable_system_ids.find(system_id) != m_fleet_supplyable_system_ids.end())
        return true;

    // check all resource supply groups
    for (std::set<std::set<int> >::const_iterator groups_it = m_resource_supply_groups.begin(); groups_it != m_resource_supply_groups.end(); ++groups_it)
        if (groups_it->find(system_id) != groups_it->end())
            return true;

    // couldn't find any reason to supply specified system.  default to false
    return false;
}

Empire::TechItr Empire::TechBegin() const
{ return m_techs.begin(); }

Empire::TechItr Empire::TechEnd() const
{ return m_techs.end(); }

Empire::TechItr Empire::AvailableBuildingTypeBegin() const
{ return m_available_building_types.begin(); }

Empire::TechItr Empire::AvailableBuildingTypeEnd() const
{ return m_available_building_types.end(); }

const std::set<int>& Empire::ExploredSystems() const
{ return m_explored_systems; }

const std::map<int, std::set<int> > Empire::KnownStarlanes() const
{
    // compile starlanes leading into or out of each system
    std::map<int, std::set<int> > retval;

    const Universe& universe = GetUniverse();

    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());
    std::vector<const System*> systems = universe.EmpireKnownObjects(this->EmpireID()).FindObjects<const System>();

    for (std::vector<const System*>::const_iterator it = systems.begin(); it != systems.end(); ++it) {
        const System* system = *it;
        int start_id = system->ID();

        // exclude lanes starting at systems known to be destroyed
        if (known_destroyed_objects.find(start_id) != known_destroyed_objects.end())
            continue;

        System::StarlaneMap lanes = system->StarlanesWormholes();
        for (System::StarlaneMap::const_iterator lane_it = lanes.begin(); lane_it != lanes.end(); ++lane_it) {
            if (lane_it->second || known_destroyed_objects.find(lane_it->second) != known_destroyed_objects.end())
                continue;   // is a wormhole, not a starlane, or is connected to a known destroyed system
            int end_id = lane_it->first;
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }
    }

    return retval;
}

const std::map<int, std::set<int> > Empire::VisibleStarlanes() const
{
    std::map<int, std::set<int> > retval;   // compile starlanes leading into or out of each system

    const Universe& universe = GetUniverse();
    const ObjectMap& objects = universe.Objects();

    std::vector<const System*> systems = objects.FindObjects<const System>();

    for (std::vector<const System*>::const_iterator it = systems.begin(); it != systems.end(); ++it) {
        const System* system = *it;
        if (!system)
            continue;
        int start_id = system->ID();

        // is system visible to this empire?
        if (universe.GetObjectVisibilityByEmpire(start_id, m_id) <= VIS_NO_VISIBILITY)
            continue;

        // get system's visible lanes for this empire
        System::StarlaneMap lanes = system->VisibleStarlanesWormholes(m_id);

        // copy to retval
        for (System::StarlaneMap::const_iterator lane_it = lanes.begin(); lane_it != lanes.end(); ++lane_it) {
            if (lane_it->second)
                continue;   // is a wormhole, not a starlane
            int end_id = lane_it->first;
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }
    }

    return retval;
}

Empire::ShipDesignItr Empire::ShipDesignBegin() const
{ return m_ship_designs.begin(); }

Empire::ShipDesignItr Empire::ShipDesignEnd() const
{ return m_ship_designs.end(); }

Empire::SitRepItr Empire::SitRepBegin() const
{ return m_sitrep_entries.begin(); }

Empire::SitRepItr Empire::SitRepEnd() const
{ return m_sitrep_entries.end(); }

double Empire::ProductionPoints() const
{ return std::min(GetResourcePool(RE_INDUSTRY)->TotalAvailable(), GetResourcePool(RE_MINERALS)->TotalAvailable()); }

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
{ return 0.0; }

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
    return it->second->TotalAvailable();
}

const PopulationPool& Empire::GetPopulationPool() const
{ return m_population_pool; }

double Empire::Population() const
{ return m_population_pool.Population(); }

void Empire::SetResourceStockpile(ResourceType resource_type, double stockpile)
{
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::SetResourceStockpile passed invalid ResourceType");
    return it->second->SetStockpile(stockpile);
}

void Empire::SetResourceMaxStockpile(ResourceType resource_type, double max)
{}

void Empire::PlaceTechInQueue(const std::string& name, int pos/* = -1*/)
{
    if (name.empty() || TechResearched(name) || m_techs.find(name) != m_techs.end())
        return;
    const Tech* tech = GetTech(name);
    if (!tech || !tech->Researchable())
        return;

    ResearchQueue::iterator it = m_research_queue.find(name);
    if (pos < 0 || static_cast<int>(m_research_queue.size()) <= pos) {
        if (it != m_research_queue.end())
            m_research_queue.erase(it);
        m_research_queue.push_back(name);
    } else {
        if (it < m_research_queue.begin() + pos)
            --pos;
        if (it != m_research_queue.end())
            m_research_queue.erase(it);
        m_research_queue.insert(m_research_queue.begin() + pos, name);
    }
    m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
}

void Empire::RemoveTechFromQueue(const std::string& name) {
    ResearchQueue::iterator it = m_research_queue.find(name);
    if (it != m_research_queue.end()) {
        m_research_queue.erase(it);
        m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
    }
}

void Empire::SetTechResearchProgress(const std::string& name, double progress) {
    const Tech* tech = GetTech(name);
    if (!tech) {
        Logger().errorStream() << "Empire::SetTechResearchProgress no such tech as: " << name;
        return;
    }
    if (TechResearched(name))
        return; // can't affect already-researched tech

    // set progress
    double clamped_progress = std::min(tech->ResearchCost(), std::max(0.0, progress));
    m_research_progress[name] = clamped_progress;

    // if tech is complete, ensure it is on the queue, so it will be researched next turn
    if (clamped_progress >= tech->ResearchCost() &&
        m_research_queue.find(name) == m_research_queue.end())
    {
        m_research_queue.push_back(name);
    }

    // don't just give tech to empire, as another effect might reduce its progress before end of turn
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
    m_production_queue.Update(this, m_resource_pools, m_production_progress);
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
    m_production_queue.Update(this, m_resource_pools, m_production_progress);
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
    m_production_queue.Update(this, m_resource_pools, m_production_progress);
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
    m_production_queue.Update(this, m_resource_pools, m_production_progress);
}

void Empire::RemoveBuildFromQueue(int index)
{
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::RemoveBuildFromQueue() : Attempted to delete a production queue item with an invalid index.");
    m_production_queue.erase(index);
    m_production_progress.erase(m_production_progress.begin() + index);
    m_production_queue.Update(this, m_resource_pools, m_production_progress);
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
    const std::vector<ItemSpec>& unlocked_items = tech->UnlockedItems();
    for (unsigned int i = 0; i < unlocked_items.size(); ++i)
        UnlockItem(unlocked_items[i]);  // potential infinite if a tech (in)directly unlocks itself?
}

void Empire::UnlockItem(const ItemSpec& item)
{
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
    case UIT_SHIP_DESIGN:
        AddShipDesign(GetPredefinedShipDesignManager().GenericDesignID(item.name)); // this adds the generic version of the design, not created by any empire, to this empire's remembered designs.
        break;
    case UIT_TECH:
        AddTech(item.name);
        break;
    default:
        Logger().errorStream() << "Empire::UnlockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::AddBuildingType(const std::string& name)
{
    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type) {
        Logger().errorStream() << "Empire::AddBuildingType given an invalid building type name: " << name;
        return;
    }
    if (building_type->Producible())
        m_available_building_types.insert(name);
}

void Empire::AddPartType(const std::string& name)
{
    const PartType* part_type = GetPartType(name);
    if (!part_type) {
        Logger().errorStream() << "Empire::AddPartType given an invalid part type name: " << name;
        return;
    }
    if (part_type->Producible())
        m_available_part_types.insert(name);
}

void Empire::AddHullType(const std::string& name)
{
    const HullType* hull_type = GetHullType(name);
    if (!hull_type) {
        Logger().errorStream() << "Empire::AddHullType given an invalid hull type name: " << name;
        return;
    }
    if (hull_type->Producible())
        m_available_hull_types.insert(name);
}

void Empire::AddExploredSystem(int ID)
{
    if (GetObject<System>(ID))
        m_explored_systems.insert(ID);
    else
        Logger().errorStream() << "Empire::AddExploredSystem given an invalid system id: " << ID;
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
    /* Check if design id is valid.  that is, check that it corresponds to an
     * existing shipdesign in the universe.  On clients, this means that this
     * empire knows about this ship design and the server consequently sent the
     * design to this player.  On the server, all existing ship designs will be
     * valid, so this just adds this design's id to those that this empire will
     * retain as one of it's ship designs, which are those displayed in the GUI
     * list of available designs for human players, and */
    const ShipDesign* ship_design = GetUniverse().GetShipDesign(ship_design_id);
    if (ship_design) {  // don't check if design is producible; adding a ship design is useful for more than just producing it
        // design is valid, so just add the id to empire's set of ids that it knows about
        if (m_ship_designs.find(ship_design_id) == m_ship_designs.end()) {
            m_ship_designs.insert(ship_design_id);
            ShipDesignsChangedSignal();
        }
    } else {
        // design in not valid
        Logger().errorStream() << "Empire::AddShipDesign(int ship_design_id) was passed a design id that this empire doesn't know about, or that doesn't exist";
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

    if (new_design_id == ShipDesign::INVALID_DESIGN_ID) {
        Logger().errorStream() << "Empire::AddShipDesign Unable to get new design id";
        return new_design_id;
    }

    bool success = universe.InsertShipDesignID(ship_design, new_design_id);

    if (!success) {
        Logger().errorStream() << "Empire::AddShipDesign Unable to add new design to universe";
        return UniverseObject::INVALID_OBJECT_ID;
    }

    m_ship_designs.insert(new_design_id);

    ShipDesignsChangedSignal();

    return new_design_id;
}

void Empire::RemoveShipDesign(int ship_design_id)
{
    if (m_ship_designs.find(ship_design_id) != m_ship_designs.end()) {
        m_ship_designs.erase(ship_design_id);
        ShipDesignsChangedSignal();
    } else {
        Logger().debugStream() << "Empire::RemoveShipDesign: this empire did not have design with id " << ship_design_id;
    }
}

void Empire::AddSitRepEntry(SitRepEntry* entry)
{ m_sitrep_entries.push_back(entry); }

void Empire::RemoveTech(const std::string& name)
{ m_techs.erase(name); }

void Empire::LockItem(const ItemSpec& item)
{
    switch (item.type) {
    case UIT_BUILDING:
        RemoveBuildingType(item.name);
        (item.name);
        break;
    case UIT_SHIP_PART:
        RemovePartType(item.name);
        break;
    case UIT_SHIP_HULL:
        RemoveHullType(item.name);
        break;
    case UIT_SHIP_DESIGN:
        RemoveShipDesign(GetPredefinedShipDesignManager().GenericDesignID(item.name));
        break;
    case UIT_TECH:
        RemoveTech(item.name);
        break;
    default:
        Logger().errorStream() << "Empire::LockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::RemoveBuildingType(const std::string& name)
{
    std::set<std::string>::const_iterator it = m_available_building_types.find(name);
    if (it == m_available_building_types.end())
        Logger().debugStream() << "Empire::RemoveBuildingType asked to remove building type " << name << " that was no available to this empire";
    m_available_building_types.erase(name);
}

void Empire::RemovePartType(const std::string& name)
{
    std::set<std::string>::const_iterator it = m_available_part_types.find(name);
    if (it == m_available_part_types.end())
        Logger().debugStream() << "Empire::RemovePartType asked to remove part type " << name << " that was no available to this empire";
    m_available_part_types.erase(name);
}

void Empire::RemoveHullType(const std::string& name)
{
    std::set<std::string>::const_iterator it = m_available_hull_types.find(name);
    if (it == m_available_hull_types.end())
        Logger().debugStream() << "Empire::RemoveHullType asked to remove hull type " << name << " that was no available to this empire";
    m_available_hull_types.erase(name);
}

void Empire::ClearSitRep()
{
    for (SitRepItr it = m_sitrep_entries.begin(); it != m_sitrep_entries.end(); ++it)
        delete *it;
    m_sitrep_entries.clear();
}

namespace {
    // remove nonexistant / invalid techs from queue
    void SanitizeResearchQueue(ResearchQueue& queue) {
        bool done = false;
        while (!done) {
            ResearchQueue::iterator it = queue.begin();
            while (true) {
                if (it == queue.end()) {
                    done = true;        // got all the way through the queue without finding an invalid tech
                    break;
                } else if (!GetTech(it->name)) {
                    queue.erase(it);    // remove invalid tech, end inner loop without marking as finished
                    break;
                } else {
                    ++it;               // check next element
                }
            }
        }
    }
}

void Empire::CheckResearchProgress()
{
    SanitizeResearchQueue(m_research_queue);

    // following commented line should be redundant, as previous call to UpdateResourcePools should have generated necessary info
    // m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
    std::vector<std::string> to_erase;
    for (ResearchQueue::iterator it = m_research_queue.begin(); it != m_research_queue.end(); ++it) {
        const Tech* tech = GetTech(it->name);
        if (!tech) {
            Logger().errorStream() << "Empire::CheckResearchProgress couldn't find tech on queue, even after sanitizing!";
            continue;
        }
        double& progress = m_research_progress[it->name];
        progress += it->allocated_rp;
        if (tech->ResearchCost() - EPSILON <= progress) {
            AddTech(it->name);
            AddSitRepEntry(CreateTechResearchedSitRep(it->name));
            // TODO: create unlocked item sitreps?
            m_research_progress.erase(it->name);
            to_erase.push_back(it->name);
        }
    }

    for (std::vector<std::string>::iterator it = to_erase.begin(); it != to_erase.end(); ++it) {
        ResearchQueue::iterator temp_it = m_research_queue.find(*it);
        if (temp_it != m_research_queue.end())
            m_research_queue.erase(temp_it);
    }
    // can uncomment following line when / if research stockpiling is enabled...
    // m_resource_pools[RE_RESEARCH]->SetStockpile(m_resource_pools[RE_RESEARCH]->TotalAvailable() - m_research_queue.TotalRPsSpent());
}

void Empire::CheckProductionProgress()
{
    Logger().debugStream() << "========Empire::CheckProductionProgress=======";
    // following commented line should be redundant, as previous call to UpdateResourcePools should have generated necessary info
    // m_production_queue.Update(this, m_resource_pools, m_production_progress);

    Universe& universe = GetUniverse();
    ObjectMap& objects = universe.Objects();

    // go through queue, updating production progress.  If a build item is completed, create the built object or take whatever other
    // action is appropriate, and record that queue item as complete, so it can be erased from the queue
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = ProductionCostAndTime(m_production_queue[i].item);
        double& status = m_production_progress[i];
        status += m_production_queue[i].allocated_pp;               // add allocated PP to queue item


        // if accumulated PP is sufficient, the item is complete
        if (item_cost - EPSILON <= status) {
            m_production_progress[i] -= item_cost * build_turns;
            switch (m_production_queue[i].item.build_type) {
            case BT_BUILDING: {
                int planet_id = m_production_queue[i].location;
                Planet* planet = objects.Object<Planet>(planet_id);
                if (!planet) {
                    Logger().errorStream() << "Couldn't get planet with id  " << planet_id << " on which to create building";
                    continue;
                }

                Building* building = new Building(m_id, m_production_queue[i].item.name, m_id);

                int building_id = universe.Insert(building);

                planet->AddBuilding(building_id);

                SitRepEntry* entry = CreateBuildingBuiltSitRep(building_id, planet->ID());
                AddSitRepEntry(entry);
                //Logger().debugStream() << "New Building created on turn: " << building->CreationTurn();
                break;
            }

            case BT_SHIP: {
                UniverseObject* build_location = objects.Object(m_production_queue[i].location);
                System* system = universe_object_cast<System*>(build_location);
                if (!system && build_location)
                    system = objects.Object<System>(build_location->SystemID());
                // TODO: account for shipyards and/or other ship production sites that are in interstellar space, if needed
                if (!system) {
                    Logger().errorStream() << "Empire::CheckProductionProgress couldn't get system for building new ship";
                    continue;
                }

                // create new fleet with new ship
                Fleet* fleet = new Fleet("", system->X(), system->Y(), m_id);
                // start with max stealth to ensure new fleet won't be visisble
                // to other empires on first turn if it shouldn't be.  current
                // value will be clamped to max meter value after effects are
                // applied

                int fleet_id = universe.Insert(fleet);

                system->Insert(fleet);
                Logger().debugStream() << "New Fleet created on turn: " << fleet->CreationTurn();


                // get species for this ship.  use popcenter species if build
                // location is a popcenter, or use ship species if build
                // location is a ship, or use empire capital species if there
                // is a valid capital, or otherwise ???
                // TODO: Add more fallbacks if necessary
                std::string species_name;
                if (const PopCenter* location_pop_center = dynamic_cast<const PopCenter*>(build_location))
                    species_name = location_pop_center->SpeciesName();
                else if (const Ship* location_ship = universe_object_cast<const Ship*>(build_location))
                    species_name = location_ship->SpeciesName();
                else if (const Planet* capital_planet = objects.Object<Planet>(this->CapitalID()))
                    species_name = capital_planet->SpeciesName();
                // else give up...


                // add ship
                Ship* ship = new Ship(m_id, m_production_queue[i].item.design_id, species_name, m_id);
                // set active meters that have associated max meters to an
                // initial very large value, so that when the active meters are
                // later clamped, they will equal the max meter after effects
                // have been applied, letting new ships start with maxed
                // everything that is traced with an associated max meter.
                ship->UniverseObject::GetMeter(METER_FUEL)->SetCurrent(Meter::LARGE_VALUE);
                ship->UniverseObject::GetMeter(METER_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
                ship->UniverseObject::GetMeter(METER_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);
                ship->BackPropegateMeters();
                // for colony ships, set species
                if (ship->CanColonize())
                    if (const PopCenter* build_loc_pop_center = dynamic_cast<const PopCenter*>(build_location))
                        ship->SetSpecies(build_loc_pop_center->SpeciesName());

                int ship_id = universe.Insert(ship);

                ship->Rename(NewShipName());

                fleet->AddShip(ship_id);

                // rename fleet, given its id and the ship that is in it
                std::vector<int> ship_ids;  ship_ids.push_back(ship_id);
                fleet->Rename(Fleet::GenerateFleetName(ship_ids, fleet_id));

                Logger().debugStream() << "New Ship created on turn: " << ship->CreationTurn();

                // add sitrep
                SitRepEntry* entry = CreateShipBuiltSitRep(ship_id, system->ID());
                AddSitRepEntry(entry);
                break;
            }

            default:
                Logger().debugStream() << "Build item of unknown build type finished on production queue.";
                break;
            }


            if (!--m_production_queue[i].remaining)     // decrement number of remaining items to be produced in current queue element
                to_erase.push_back(i);                  // remember completed element so that it can be removed from queue
        }
    }

    // removed completed items from queue
    for (std::vector<int>::reverse_iterator it = to_erase.rbegin(); it != to_erase.rend(); ++it) {
        m_production_progress.erase(m_production_progress.begin() + *it);
        m_production_queue.erase(*it);
    }


    // update minerals stockpile

    // get minerals resource pool and stockpile location
    boost::shared_ptr<ResourcePool> pool = m_resource_pools[RE_MINERALS];
    int stockpile_object_id = pool->StockpileObjectID();


    if (stockpile_object_id == UniverseObject::INVALID_OBJECT_ID) {
        // empire has nowhere to stockpile food, so has no stockpile.
        pool->SetStockpile(0.0);
        //Logger().debugStream() << "no mineral stockpile location.  stockpile is set to 0.0";

    } else {
        // find minerals (PP) allocated to production elements located in systems in the group of
        // resource-sharing objects that has access to stockpile
        double stockpile_group_pp_allocation = 0.0;

        // find the set of objects that contains the stopile object, from the map of PP allocated within each group
        std::map<std::set<int>, double> allocated_pp = m_production_queue.AllocatedPP();

        //Logger().debugStream() << "trying to find stockpile object group...  stockpile object has id: " << stockpile_object_id;
        for (std::map<std::set<int>, double>::const_iterator it = allocated_pp.begin(); it != allocated_pp.end(); ++it) {
            const std::set<int>& group = it->first;                     // get group
            //Logger().debugStream() << "potential group:";
            for (std::set<int>::const_iterator qit = group.begin(); qit != group.end(); ++qit)
                Logger().debugStream() << "...." << *qit;

            if (group.find(stockpile_object_id) != group.end()) {       // check for stockpile object
                stockpile_group_pp_allocation = it->second;        // record allocation for this group
                //Logger().debugStream() << "Empire::CheckProductionProgress found group of objects for stockpile object.  size: " << it->first.size();
                break;
            }

            //Logger().debugStream() << "didn't find in group... trying next.";
        }
        // if the stockpile object is not found in any group of systems with allocated pp, assuming this is fine and that the
        // stockpile object's group of systems didn't have any allocated pp...


        double stockpile_object_group_available = pool->GroupAvailable(stockpile_object_id);
        //Logger().debugStream() << "minerals available in stockpile group is:  " << stockpile_object_group_available;
        //Logger().debugStream() << "minerals allocation in stockpile group is: " << stockpile_group_pp_allocation;       // as of this writing, PP consume one mineral and one industry point, so PP allocation is equal to minerals allocation

        //Logger().debugStream() << "Old stockpile was " << pool->Stockpile();

        double new_stockpile = stockpile_object_group_available - stockpile_group_pp_allocation;
        pool->SetStockpile(new_stockpile);
        //Logger().debugStream() << "New stockpile is: " << new_stockpile;
    }
}

void Empire::CheckTradeSocialProgress()
{ m_resource_pools[RE_TRADE]->SetStockpile(m_resource_pools[RE_TRADE]->TotalAvailable() - m_maintenance_total_cost); }

void Empire::CheckGrowthFoodProgress()
{
    Logger().debugStream() << "========Empire::CheckGrowthFoodProgress=======";

    boost::shared_ptr<ResourcePool> pool = m_resource_pools[RE_FOOD];
    const PopulationPool& pop_pool = m_population_pool;                 // adding a reference to a member variable of this object for consistency with other implementation of this code in MapWnd

    int stockpile_object_id = pool->StockpileObjectID();

    //Logger().debugStream() << "food stockpile object id: " << stockpile_object_id;

    if (stockpile_object_id == UniverseObject::INVALID_OBJECT_ID) {
        // empire has nowhere to stockpile food, so has no stockpile.
        pool->SetStockpile(0.0);

    } else {
        // find total food allocated to group that has access to stockpile...

        // first find the set of objects that contains the stockpile object
        std::map<std::set<int>, double> food_sharing_groups = pool->Available();    // don't actually need the available PP; just using the map as a set of sets of systems
        std::set<int> stockpile_group_object_ids;
        //Logger().debugStream() << "trying to find stockpile object group...  stockpile object has id: " << stockpile_object_id;
        for (std::map<std::set<int>, double>::const_iterator it = food_sharing_groups.begin();
             it != food_sharing_groups.end(); ++it)
        {
            const std::set<int>& group = it->first;                     // get group
            //Logger().debugStream() << "potential group:";
            //for (std::set<int>::const_iterator qit = group.begin(); qit != group.end(); ++qit)
                //Logger().debugStream() << "...." << *qit;

            if (group.find(stockpile_object_id) != group.end()) {       // check for stockpile object
                stockpile_group_object_ids = group;
                //Logger().debugStream() << "Empire::CheckGrowthFoodProgress found group of objects for stockpile object.  size: " << stockpile_group_object_ids.size();
                break;
            }

            //Logger().debugStream() << "didn't find in group... trying next.";
        }

        const std::vector<int>& pop_centers = pop_pool.PopCenterIDs();

        double stockpile_group_food_allocation = 0.0;

        // go through population pool, adding up food allocation of popcenters
        // that are in the group of objects that can access the stockpile
        for (std::vector<int>::const_iterator it = pop_centers.begin(); it != pop_centers.end(); ++it) {
            int object_id = *it;
            if (stockpile_group_object_ids.find(object_id) == stockpile_group_object_ids.end())
                continue;

            const UniverseObject* obj = GetObject(*it);
            if (!obj) {
                Logger().debugStream() << "Empire::CheckGrowthFoodProgress couldn't get an object with id " << object_id;
                continue;
            }
            const PopCenter* pop = dynamic_cast<const PopCenter*>(obj);
            if (!pop) {
                Logger().debugStream() << "Empire::CheckGrowthFoodProgress couldn't cast a UniverseObject* to an PopCenter*";
                continue;
            }

            stockpile_group_food_allocation += pop->AllocatedFood();    // finally add allocation for this PopCenter
            //Logger().debugStream() << "object " << obj->Name() << " is in stockpile object group that has " << pop->AllocatedFood() << " food allocated to it";
        }

        double stockpile_object_group_available = pool->GroupAvailable(stockpile_object_id);
        //Logger().debugStream() << "food available in stockpile group is:  " << stockpile_object_group_available;
        //Logger().debugStream() << "food allocation in stockpile group is: " << stockpile_group_food_allocation;

        //Logger().debugStream() << "Old stockpile was " << pool->Stockpile();

        double new_stockpile = stockpile_object_group_available - stockpile_group_food_allocation;
        pool->SetStockpile(new_stockpile);
        //Logger().debugStream() << "New stockpile is: " << new_stockpile;
    }
}

void Empire::SetColor(const GG::Clr& color)
{ m_color = color; }

void Empire::SetName(const std::string& name)
{ m_name = name; }

void Empire::SetPlayerName(const std::string& player_name)
{ m_player_name = player_name; }

void Empire::InitResourcePools()
{
    const ObjectMap& objects = GetUniverse().Objects();
    std::vector<const UniverseObject*> object_vec = objects.FindObjects(OwnedVisitor<UniverseObject>(m_id));
    std::vector<int> object_ids_vec, popcenter_ids_vec;

    // determine if each object owned by this empire is a PopCenter, and store
    // ids of PopCenters and objects in appropriate vectors
    for (std::vector<const UniverseObject*>::const_iterator it = object_vec.begin(); it != object_vec.end(); ++it) {
        const UniverseObject* obj = *it;
        object_ids_vec.push_back(obj->ID());
        if (dynamic_cast<const PopCenter*>(obj))
            popcenter_ids_vec.push_back(obj->ID());
    }
    m_population_pool.SetPopCenters(popcenter_ids_vec);
    m_resource_pools[RE_MINERALS]->SetObjects(object_ids_vec);
    m_resource_pools[RE_FOOD]->SetObjects(object_ids_vec);
    m_resource_pools[RE_RESEARCH]->SetObjects(object_ids_vec);
    m_resource_pools[RE_INDUSTRY]->SetObjects(object_ids_vec);
    m_resource_pools[RE_TRADE]->SetObjects(object_ids_vec);


    // inform the blockadeable resource pools about systems that can share
    m_resource_pools[RE_MINERALS]->SetConnectedSupplyGroups(m_resource_supply_groups);
    m_resource_pools[RE_FOOD]->SetConnectedSupplyGroups(m_resource_supply_groups);
    m_resource_pools[RE_INDUSTRY]->SetConnectedSupplyGroups(m_resource_supply_groups);


    // set non-blockadeable resource pools to share resources between all systems
    std::set<std::set<int> > sets_set;
    std::set<int> all_systems_set;
    const std::vector<const System*> all_systems_vec = objects.FindObjects<System>();
    for (std::vector<const System*>::const_iterator it = all_systems_vec.begin(); it != all_systems_vec.end(); ++it)
        all_systems_set.insert((*it)->ID());
    sets_set.insert(all_systems_set);
    m_resource_pools[RE_RESEARCH]->SetConnectedSupplyGroups(sets_set);
    m_resource_pools[RE_TRADE]->SetConnectedSupplyGroups(sets_set);


    // set stockpile object locations for each resource, ensuring those systems exist
    std::vector<ResourceType> res_type_vec;
    res_type_vec.push_back(RE_FOOD);
    res_type_vec.push_back(RE_MINERALS);
    res_type_vec.push_back(RE_INDUSTRY);
    res_type_vec.push_back(RE_TRADE);
    res_type_vec.push_back(RE_RESEARCH);

    for (std::vector<ResourceType>::const_iterator res_it = res_type_vec.begin(); res_it != res_type_vec.end(); ++res_it) {
        ResourceType res_type = *res_it;
        int stockpile_object_id = UniverseObject::INVALID_OBJECT_ID;
        if (const UniverseObject* stockpile_obj = objects.Object(StockpileID(res_type)))
            stockpile_object_id = stockpile_obj->ID();
        m_resource_pools[res_type]->SetStockpileObject(stockpile_object_id);
    }
}

void Empire::UpdateResourcePools()
{
    // updating queues, allocated_rp, distribution and growth each update their respective pools,
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
    m_research_queue.Update(this, m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
    m_resource_pools[RE_RESEARCH]->ChangedSignal();
}

void Empire::UpdateProductionQueue()
{
    Logger().debugStream() << "========= Production Update for empire: " << EmpireID() << " ========";

    m_resource_pools[RE_MINERALS]->Update();
    m_resource_pools[RE_INDUSTRY]->Update();
    m_production_queue.Update(this, m_resource_pools, m_production_progress);
    m_resource_pools[RE_MINERALS]->ChangedSignal();
    m_resource_pools[RE_INDUSTRY]->ChangedSignal();
}

void Empire::UpdateTradeSpending()
{
    m_resource_pools[RE_TRADE]->Update(); // recalculate total trade production

    // TODO: Replace with call to some other subsystem, similar to the Update...Queue functions
    m_maintenance_total_cost = 0.0;

    std::vector<UniverseObject*> buildings = GetUniverse().Objects().FindObjects(OwnedVisitor<Building>(m_id));
    for (std::vector<UniverseObject*>::const_iterator it = buildings.begin(); it != buildings.end(); ++it) {
        Building* building = universe_object_cast<Building*>(*it);
        if (!building)
            continue;

        const BuildingType* building_type = building->GetBuildingType();
        if (!building_type)
            continue;

        //if (building->Operating())
        m_maintenance_total_cost += building_type->MaintenanceCost();
    }
    m_resource_pools[RE_TRADE]->ChangedSignal();
}

void Empire::UpdateFoodDistribution()
{
    Logger().debugStream() << "======= Food distribution for empire: " << EmpireID() << " =======";

    m_resource_pools[RE_FOOD]->Update();  // recalculate total food production

    // get sets of resource-sharing objects and amount of resource available in
    // each, and distribute food within each group independently
    std::map<std::set<int>, double> groups_food_available = m_resource_pools[RE_FOOD]->Available();
    for (std::map<std::set<int>, double>::iterator groups_it = groups_food_available.begin();
         groups_it != groups_food_available.end(); ++groups_it)
    {
        const std::set<int>& group_object_ids = groups_it->first;

        // get subset of group objects that are PopCenters
        std::vector<std::pair<UniverseObject*, PopCenter*> > pop_in_group;
        for (std::set<int>::const_iterator obj_it = group_object_ids.begin(); obj_it != group_object_ids.end(); ++obj_it)
            if (UniverseObject* obj = GetObject(*obj_it))
                if (PopCenter* pop = dynamic_cast<PopCenter*>(obj))
                    pop_in_group.push_back(std::make_pair(obj, pop));


        //Logger().debugStream() << " !!  Zeroth Pass Food Distribution";
        // clear food allocations to all PopCenters to start, so that if no
        // further allocations occur due to insufficient food being
        // available, previous turns or iterations' allocations won't be left
        for (std::vector<std::pair<UniverseObject*, PopCenter*> >::const_iterator pop_it = pop_in_group.begin();
             pop_it != pop_in_group.end(); ++pop_it)
        {
            pop_it->second->SetAllocatedFood(0.0);
            //Logger().debugStream() << "allocating 0.0 food to " << pop_center_objects[*pop_it]->Name() << " to initialize";
        }


        double food_available = groups_it->second;
        //Logger().debugStream() << "group has " << food_available << " food available for allocation";


        //Logger().debugStream() << " !!  First Pass Food Distribution";

        // first pass: give food to PopCenters that produce food, limited by their food need and their food production
        for (std::vector<std::pair<UniverseObject*, PopCenter*> >::const_iterator pop_it = pop_in_group.begin();
             pop_it != pop_in_group.end(); ++pop_it)
        {
            if (food_available <= 0.0)
                break;

            double need = pop_it->second->CurrentMeterValue(METER_FOOD_CONSUMPTION);
            double prod = 0.0;
            if (pop_it->first->GetMeter(METER_FARMING))
                prod = pop_it->first->CurrentMeterValue(METER_FARMING); // preferential allocation for food producers

            // allocate food to this PopCenter, deduct from pool, add to total food distribution tally
            double allocation = std::min(std::min(need, prod), food_available);

            //Logger().debugStream() << "allocating " << allocation << " food to " << pop_center_objects[pc]->Name() << " limited by need and by production";

            pop_it->second->SetAllocatedFood(allocation);
            food_available -= allocation;
        }

        //Logger().debugStream() << " !!  Second Pass Food Distribution";

        // second pass: give as much food as needed to PopCenters to maintain current population
        for (std::vector<std::pair<UniverseObject*, PopCenter*> >::const_iterator pop_it = pop_in_group.begin();
             pop_it != pop_in_group.end(); ++pop_it)
        {
            if (food_available <= 0.0)
                break;

            double need = pop_it->second->CurrentMeterValue(METER_FOOD_CONSUMPTION);
            double has = pop_it->second->AllocatedFood();
            double addition = std::min(std::max(need - has, 0.0), food_available);
            double new_allocation = has + addition;

            //Logger().debugStream() << "allocating " << new_allocation << " food to " << pop_center_objects[pc]->Name() << " limited by need (to maintain population)";

            pop_it->second->SetAllocatedFood(new_allocation);
            food_available -= addition;
        }

        //Logger().debugStream() << " !!  Third Pass Food Distribution";

        // third pass: give as much food as needed to PopCenters to allow max possible growth
        for (std::vector<std::pair<UniverseObject*, PopCenter*> >::const_iterator pop_it = pop_in_group.begin();
             pop_it != pop_in_group.end(); ++pop_it)
        {
            if (food_available <= 0.0)
                break;

            double most_needed_to_grow = pop_it->second->FoodAllocationForMaxGrowth();
            double has = pop_it->second->AllocatedFood();
            double extra_needed_for_max_growth = most_needed_to_grow - has;
            double addition = std::min(std::max(extra_needed_for_max_growth, 0.0), food_available);
            double new_allocation = has + addition;

            //Logger().debugStream() << "allocating " << new_allocation << " food to " << pop_center_objects[pc]->Name() << " to allow max possible growth";

            pop_it->second->SetAllocatedFood(new_allocation);
            food_available -= addition;
        }
    }


    // after changing food distribution, population growth predictions may need to be redone
    // by calling UpdatePopulationGrowth()

    m_resource_pools[RE_FOOD]->ChangedSignal();
}

void Empire::UpdatePopulationGrowth()
{ m_population_pool.Update(); }
