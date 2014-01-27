#include "Empire.h"

#include "../parse/Parse.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/MultiplayerCommon.h"
#include "../util/ScopedTimer.h"
#include "../util/Random.h"
#include "../util/Logger.h"
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
#include "boost/date_time/posix_time/posix_time.hpp"

namespace {
    const float EPSILON = 1.0e-5f;
    const std::string EMPTY_STRING;

    /** sets the .allocated_rp, value for each Tech in the queue.  Only sets
      * nonzero funding to a Tech if it is researchable this turn.  Also
      * determines total number of spent RP (returning by reference in
      * total_RPs_spent) */
    void SetTechQueueElementSpending(float RPs, const std::map<std::string, float>& research_progress,
                                     const std::map<std::string, TechStatus>& research_status,
                                     ResearchQueue::QueueType& queue,
                                     float& total_RPs_spent, int& projects_in_progress, int empire_id)
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
                std::map<std::string, float>::const_iterator progress_it = research_progress.find(it->name);
                float progress = progress_it == research_progress.end() ? 0.0 : progress_it->second;
                float RPs_needed = tech->ResearchCost(empire_id) - progress;
                float RPs_per_turn_limit = tech->PerTurnCost(empire_id);
                float RPs_to_spend = std::min(RPs_needed, RPs_per_turn_limit);

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

    /** Sets the allocated_pp value for each Element in the passed
      * ProductionQueue \a queue.  Elements are allocated PP based on their need,
      * the limits they can be given per turn, and the amount available at their
      * production location (which is itself limited by the resource supply
      * system groups that are able to exchange resources with the build
      * location and the amount of minerals and industry produced in the group).
      * Elements will not receive funding if they cannot be produced by the
      * empire with the indicated \a empire_id this turn at their build location. */
    void SetProdQueueElementSpending(std::map<std::set<int>, float> available_pp,
                                     const std::vector<std::set<int> >& queue_element_resource_sharing_object_groups,
                                     ProductionQueue::QueueType& queue,
                                     std::map<std::set<int>, float>& allocated_pp,
                                     int& projects_in_progress, int empire_id)
    {
        //Logger().debugStream() << "========SetProdQueueElementSpending========";
        //Logger().debugStream() << "production status: ";
        //for (std::vector<float>::const_iterator it = production_status.begin(); it != production_status.end(); ++it)
        //    Logger().debugStream() << " ... " << *it;
        //Logger().debugStream() << "queue: ";
        //for (ProductionQueue::QueueType::const_iterator it = queue.begin(); it != queue.end(); ++it)
        //    Logger().debugStream() << " ... name: " << it->item.name << "id: " << it->item.design_id << " allocated: " << it->allocated_pp << " locationid: " << it->location << " ordered: " << it->ordered;

        if (queue.size() != queue_element_resource_sharing_object_groups.size()) {
            Logger().errorStream() << "SetProdQueueElementSpending queue size and sharing groups size inconsistent. aborting";
            return;
        }

        projects_in_progress = 0;
        allocated_pp.clear();

        //Logger().debugStream() << "queue size: " << queue.size();
        const Empire* empire = Empires().Lookup(empire_id);
        if (!empire)
            return;

        // cache production item costs and times
        std::map<std::pair<ProductionQueue::ProductionItem, int>,
                 std::pair<float, int> >                           queue_item_costs_and_times;
        for (ProductionQueue::iterator it = queue.begin(); it != queue.end(); ++it) {
            ProductionQueue::Element& elem = *it;

            // for items that don't depend on location, only store cost/time once
            int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
            std::pair<ProductionQueue::ProductionItem, int> key(elem.item, location_id);

            if (queue_item_costs_and_times.find(key) == queue_item_costs_and_times.end())
                queue_item_costs_and_times[key] = empire->ProductionCostAndTime(elem);
        }


        int i = 0;
        for (ProductionQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            ProductionQueue::Element& queue_element = *it;

            // get resource sharing group and amount of resource available to build this item
            const std::set<int>& group = queue_element_resource_sharing_object_groups[i];
            if (group.empty()) {
                //Logger().debugStream() << "resource sharing group for queue element is empty.  not allocating any resources to element";
                queue_element.allocated_pp = 0.0;
                continue;
            }

            std::map<std::set<int>, float>::iterator available_pp_it = available_pp.find(group);
            if (available_pp_it == available_pp.end()) {
                // item is not being built at an object that has access to resources, so it can't be built.
                //Logger().debugStream() << "no resource sharing group for production queue element";
                queue_element.allocated_pp = 0.0;
                continue;
            }

            float& group_pp_available = available_pp_it->second;


            // if group has no pp available, can't build anything this turn
            if (group_pp_available <= 0.0) {
                //Logger().debugStream() << "no pp available in group";
                queue_element.allocated_pp = 0.0;
                continue;
            }
            //Logger().debugStream() << "group has " << group_pp_available << " PP available";

            // see if item is buildable this turn...
            if (!empire->ProducibleItem(queue_element.item, queue_element.location)) {
                // can't be built at this location this turn.
                queue_element.allocated_pp = 0.0;
                //Logger().debugStream() << "item can't be built at location this turn";
                continue;
            }


            // get max contribution per turn and turns to build at max contribution rate
            int location_id = (queue_element.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : queue_element.location);
            std::pair<ProductionQueue::ProductionItem, int> key(queue_element.item, location_id);
            float item_cost;
            int build_turns;
            boost::tie(item_cost, build_turns) = queue_item_costs_and_times[key];
            //Logger().debugStream() << "item " << queue_element.item.name << " costs " << item_cost << " for " << build_turns << " turns";

            item_cost *= queue_element.blocksize;
            // determine additional PP needed to complete build queue element: total cost - progress
            float element_accumulated_PP = queue_element.progress;                                 // PP accumulated by this element towards building next item
            float element_total_cost = item_cost * queue_element.remaining;                        // total PP to build all items in this element
            float additional_pp_to_complete_element = element_total_cost - element_accumulated_PP; // additional PP, beyond already-accumulated PP, to build all items in this element
            float element_per_turn_limit = item_cost / std::max(build_turns, 1);

            // determine how many pp to allocate to this queue element this turn.  allocation is limited by the
            // item cost, which is the max number of PP per turn that can be put towards this item, and by the
            // total cost remaining to complete the last item in the queue element (eg. the element has all but
            // the last item complete already) and by the total pp available in this element's production location's
            // resource sharing group
            float allocation = std::max(std::min(std::min(additional_pp_to_complete_element,
                                                          element_per_turn_limit),
                                                 group_pp_available),
                                        0.0f);       // max(..., 0.0) prevents negative-allocations

            //Logger().debugStream() << "element accumulated " << element_accumulated_PP << " of total cost "
            //                       << element_total_cost << " and needs " << additional_pp_to_complete_element
            //                       << " more to be completed";
            //Logger().debugStream() << "... allocating " << allocation;

            // allocate pp
            queue_element.allocated_pp = allocation;

            // record alloation in group, if group is not empty
            allocated_pp[group] += allocation;  // assuming the float indexed by group will be default initialized to 0.0 if that entry doesn't already exist in the map
            group_pp_available -= allocation;

            //Logger().debugStream() << "... leaving " << group_pp_available << " PP available to group";

            if (allocation > 0.0)
                ++projects_in_progress;
        }
    }
}

////////////////////////////////////////
// ResearchQueue                      //
////////////////////////////////////////
bool ResearchQueue::InQueue(const std::string& tech_name) const
{ return find(tech_name) != end(); }

int ResearchQueue::ProjectsInProgress() const
{ return m_projects_in_progress; }

float ResearchQueue::TotalRPsSpent() const
{ return m_total_RPs_spent; }

bool ResearchQueue::empty() const
{ return !m_queue.size(); }

unsigned int ResearchQueue::size() const
{ return m_queue.size(); }

ResearchQueue::const_iterator ResearchQueue::begin() const
{ return m_queue.begin(); }

ResearchQueue::const_iterator ResearchQueue::end() const
{ return m_queue.end(); }

ResearchQueue::const_iterator ResearchQueue::find(const std::string& tech_name) const {
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->name == tech_name)
            return it;
    }
    return end();
}

const ResearchQueue::Element& ResearchQueue::operator[](int i) const {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ResearchQueue::const_iterator ResearchQueue::UnderfundedProject() const {
    for (const_iterator it = begin(); it != end(); ++it) {
        if (const Tech* tech = GetTech(it->name)) {
            if (it->allocated_rp &&
                it->allocated_rp < tech->PerTurnCost(m_empire_id)
                && 1 < it->turns_left)
            {
                return it;
            }
        }
    }
    return end();
}

void ResearchQueue::Update(float RPs, const std::map<std::string, float>& research_progress) {
    // status of all techs for this empire
    const Empire* empire = Empires().Lookup(m_empire_id);
    if (!empire)
        return;
    TechManager& tech_manager = GetTechManager();



    std::map<std::string, TechStatus> sim_tech_status_map;
    for (TechManager::iterator tech_it = tech_manager.begin(); tech_it != tech_manager.end(); ++tech_it) {
        std::string tech_name = (*tech_it)->Name();
        sim_tech_status_map[tech_name] = empire->GetTechStatus(tech_name);
    }

    SetTechQueueElementSpending(RPs, research_progress, sim_tech_status_map, m_queue,
                                m_total_RPs_spent, m_projects_in_progress, m_empire_id);

    if (m_queue.empty()) {
        ResearchQueueChangedSignal();
        return;    // nothing more to do...
    }

    const int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops

    // initialize status of everything to never getting done
    for (unsigned int i = 0; i < m_queue.size(); ++i)
        m_queue[i].turns_left = -1;

    if (RPs <= EPSILON) {
        ResearchQueueChangedSignal();
        return;    // nothing more to do if not enough RP...
    }

    boost::posix_time::ptime dp_time_start;
    boost::posix_time::ptime dp_time_end;

    // "Dynamic Programming" version of research queue simulator -- copy the queue simulator containers
    // perform dynamic programming calculation of completion times, then after regular simulation is done compare results (if both enabled)

    //record original order & progress
    // will take advantage of fact that sets (& map keys) are by default kept in sorted order lowest to highest
    std::map<std::string, float> dp_prog = research_progress;
    std::map< std::string, int > origQueueOrder;
    std::map<int, float> dpsim_research_progress;
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        std::string tname = m_queue[i].name;
        origQueueOrder[tname] = i;
        dpsim_research_progress[i] = dp_prog[tname];
    }

    std::map<std::string, TechStatus> dpsim_tech_status_map = sim_tech_status_map;

    // initialize simulation_results with -1 for all techs, so that any techs that aren't
    // finished in simulation by turn TOO_MANY_TURNS will be left marked as never to be finished
    std::vector<int>  dpsimulation_results(m_queue.size(), -1);

    const int DP_TURNS = TOO_MANY_TURNS; // track up to this many turns

    std::map<std::string, std::set<std::string> > waitingForPrereqs;
    std::set<int> dpResearchableTechs;
 
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        std::string techname = m_queue[i].name;
        const Tech* tech = GetTech( techname );
        if (!tech)
            continue;
        if ( dpsim_tech_status_map[ techname ] == TS_RESEARCHABLE ) {
            dpResearchableTechs.insert(i);
        } else if ( dpsim_tech_status_map[ techname  ] == TS_UNRESEARCHABLE ) {
            std::set<std::string> thesePrereqs = tech->Prerequisites();
            for (std::set<std::string>::iterator ptech_it = thesePrereqs.begin(); ptech_it != thesePrereqs.end(); ++ptech_it){
                if (dpsim_tech_status_map[ *ptech_it ] == TS_COMPLETE ) {
                    std::set<std::string>::iterator eraseit = ptech_it--;
                    thesePrereqs.erase( eraseit);
                }
            }
            waitingForPrereqs[ techname ] = thesePrereqs;
        }
    }

    int dpturns = 0;
    //ppStillAvailable[turn-1] gives the RP still available in this resource pool at turn "turn"
    std::vector<float> rpStillAvailable(DP_TURNS, RPs );  // initialize to the  full RP allocation for every turn

    while ((dpturns < DP_TURNS) && !(dpResearchableTechs.empty())) {// if we haven't used up our turns and still have techs to process
        ++dpturns;
        std::map<int, bool> alreadyProcessed;
        std::set<int>::iterator curTechIt;
        for (curTechIt = dpResearchableTechs.begin(); curTechIt != dpResearchableTechs.end(); ++curTechIt) {
            alreadyProcessed[ *curTechIt ] = false;
        }
        curTechIt = dpResearchableTechs.begin();
        while ((rpStillAvailable[dpturns-1] > EPSILON)) { // try to use up this turns RPs
            if (curTechIt == dpResearchableTechs.end()) {
                break; //will be wasting some RP this turn
            }
            int curTech = *curTechIt;
            if (alreadyProcessed[curTech]) {
                ++curTechIt;
                continue;
            }
            alreadyProcessed[curTech] = true;
            const std::string& tech_name = m_queue[curTech].name;
            const Tech* tech = GetTech(tech_name);
            float progress = dpsim_research_progress[curTech];
            float RPs_needed = tech ? tech->ResearchCost(m_empire_id) - progress : 0.0;
            float RPs_per_turn_limit = tech ? tech->PerTurnCost(m_empire_id) : 1.0;
            float RPs_to_spend = std::min(std::min(RPs_needed, RPs_per_turn_limit), rpStillAvailable[dpturns-1]);
            progress += RPs_to_spend;
            dpsim_research_progress[curTech] = progress;
            rpStillAvailable[dpturns-1] -= RPs_to_spend;
            std::set<int>::iterator nextResTechIt = curTechIt;
            int nextResTechIdx;
            if (++nextResTechIt == dpResearchableTechs.end()) {
                nextResTechIdx = m_queue.size()+1;
            } else {
                nextResTechIdx = *(nextResTechIt);
            }
            float tech_cost = tech ? tech->ResearchCost(m_empire_id) : 0.0;
            if (tech_cost - EPSILON <= progress) {
                dpsim_tech_status_map[tech_name] = TS_COMPLETE;
                dpsimulation_results[curTech] = dpturns;
#ifndef ORIG_RES_SIMULATOR
                m_queue[curTech].turns_left = dpturns;
#endif
                dpResearchableTechs.erase(curTechIt);
                std::set<std::string> unlockedTechs;
                if (tech)
                    unlockedTechs = tech->UnlockedTechs();
                for (std::set<std::string>::iterator utechIt = unlockedTechs.begin(); utechIt!=unlockedTechs.end(); ++utechIt) {
                    std::string utechName = *utechIt;
                    std::map<std::string,std::set<std::string> >::iterator prereqTechIt = waitingForPrereqs.find(utechName);
                    if (prereqTechIt != waitingForPrereqs.end() ){
                        std::set<std::string> &thesePrereqs = prereqTechIt->second;
                        std::set<std::string>::iterator justFinishedIt = thesePrereqs.find( tech_name );
                        if (justFinishedIt != thesePrereqs.end() ) {  //should always find it
                            thesePrereqs.erase( justFinishedIt );
                            if ( thesePrereqs.empty() ) { // tech now fully unlocked
                                int thisTechIdx = origQueueOrder[utechName];
                                dpResearchableTechs.insert(thisTechIdx);
                                waitingForPrereqs.erase( prereqTechIt );
                                alreadyProcessed[ thisTechIdx ] = true;//doesn't get any allocation on current turn
                                if (thisTechIdx < nextResTechIdx ) {
                                    nextResTechIdx = thisTechIdx;
                                }
                            }
                        } else { //couldnt find tech_name in prereqs list  
                            Logger().debugStream() << "ResearchQueue::Update tech unlocking problem:"<< tech_name << "thought it was a prereq for " << utechName << "but the latter disagreed";
                        }
                    } //else { //tech_name thinks itself a prereq for ytechName, but utechName not in prereqs -- not a problem so long as utechName not in our queue at all
                      //  Logger().debugStream() << "ResearchQueue::Update tech unlocking problem:"<< tech_name << "thought it was a prereq for " << utechName << "but the latter disagreed";
                      //}
                }
            }// if (tech->ResearchCost() - EPSILON <= progress)
            curTechIt = dpResearchableTechs.find(nextResTechIdx);
        }//while ((rpStillAvailable[dpturns-1]> EPSILON))
        //dp_time = dpsim_queue_timer.elapsed() * 1000;
        // Logger().debugStream() << "ProductionQueue::Update queue dynamic programming sim time: " << dpsim_queue_timer.elapsed() * 1000.0;
    } // while ((dpturns < DP_TURNS ) && !(dpResearchableTechs.empty() ) )

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
                it->allocated_rp < tech->ResearchCost(m_empire_id)
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
    name(),
    design_id(design_id_)
{
    if (build_type == BT_SHIP) {
        if (const ShipDesign* ship_design = GetShipDesign(design_id))
            name = ship_design->Name();
        else
            Logger().errorStream() << "ProductionItem::ProductionItem couldn't get ship design with id: " << design_id;
    }
}

bool ProductionQueue::ProductionItem::CostIsProductionLocationInvariant() const {
    if (build_type == BT_BUILDING) {
        const BuildingType* type = GetBuildingType(name);
        if (!type)
            return true;
        return type->ProductionCostTimeLocationInvariant();

    } else if (build_type == BT_SHIP) {
        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return true;
        return design->ProductionCostTimeLocationInvariant();
    }
    return false;
}

bool ProductionQueue::ProductionItem::operator<(const ProductionItem& rhs) const {
    if (build_type < rhs.build_type)
        return true;
    if (build_type > rhs.build_type)
        return false;
    if (build_type == BT_BUILDING)
        return name < rhs.name;
    else if (build_type == BT_SHIP)
        return design_id < rhs.design_id;

    return false;
}

//////////////////////////////
// ProductionQueue::Element //
//////////////////////////////
ProductionQueue::Element::Element() :
    ordered(0),
    blocksize(1),
    remaining(0),
    location(INVALID_OBJECT_ID),
    allocated_pp(0.0),
    progress(0.0),
    progress_memory(0.0),
    blocksize_memory(1),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(ProductionItem item_, int ordered_,
                                  int remaining_, int location_) :
    item(item_),
    ordered(ordered_),
    blocksize(1),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    progress(0.0),
    progress_memory(0.0),
    blocksize_memory(1),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, std::string name, int ordered_,
                                  int remaining_, int location_) :
    item(build_type, name),
    ordered(ordered_),
    blocksize(1),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    progress(0.0),
    progress_memory(0.0),
    blocksize_memory(1),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, int design_id, int ordered_,
                                  int remaining_, int location_) :
    item(build_type, design_id),
    ordered(ordered_),
    blocksize(1),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    progress(0.0),
    progress_memory(0.0),
    blocksize_memory(1),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

/////////////////////
// ProductionQueue //
/////////////////////
ProductionQueue::ProductionQueue(int empire_id) :
    m_projects_in_progress(0),
    m_empire_id(empire_id)
{}

int ProductionQueue::ProjectsInProgress() const
{ return m_projects_in_progress; }

float ProductionQueue::TotalPPsSpent() const {
    // add up allocated PP from all resource sharing object groups
    float retval = 0.0;
    for (std::map<std::set<int>, float>::const_iterator it = m_object_group_allocated_pp.begin();
         it != m_object_group_allocated_pp.end(); ++it)
    { retval += it->second; }
    return retval;
}

std::map<std::set<int>, float> ProductionQueue::AvailablePP(
    const boost::shared_ptr<ResourcePool>& industry_pool) const
{
    std::map<std::set<int>, float> retval;
    if (!industry_pool) {
        Logger().errorStream() << "ProductionQueue::AvailablePP passed invalid industry resource pool";
        return retval;
    }

    // determine available PP (ie. industry) in each resource sharing group of systems
    std::map<std::set<int>, float> available_industry = industry_pool->Available();

    for (std::map<std::set<int>, float>::const_iterator ind_it = available_industry.begin();
         ind_it != available_industry.end(); ++ind_it)
    {
        // get group of systems in industry pool
        const std::set<int>& group = ind_it->first;
        retval[group] = ind_it->second;
    }

    return retval;
}

const std::map<std::set<int>, float>& ProductionQueue::AllocatedPP() const
{ return m_object_group_allocated_pp; }

std::set<std::set<int> > ProductionQueue::ObjectsWithWastedPP(
    const boost::shared_ptr<ResourcePool>& industry_pool) const
{
    std::set<std::set<int> > retval;
    if (!industry_pool) {
        Logger().errorStream() << "ProductionQueue::ObjectsWithWastedPP passed invalid industry resource pool";
        return retval;
    }

    std::map<std::set<int>, float> available_PP_groups = AvailablePP(industry_pool);
    //std::cout << "available PP groups size: " << available_PP_groups.size() << std::endl;

    for (std::map<std::set<int>, float>::const_iterator avail_it = available_PP_groups.begin();
         avail_it != available_PP_groups.end(); ++avail_it)
    {
        //std::cout << "available PP groups size: " << avail_it->first.size() << " pp: " << avail_it->second << std::endl;

        if (avail_it->second <= 0)
            continue;   // can't waste if group has no PP
        const std::set<int>& group = avail_it->first;
        // find this group's allocated PP
        std::map<std::set<int>, float>::const_iterator alloc_it = m_object_group_allocated_pp.find(group);
        // is less allocated than is available?  if so, some is wasted
        if (alloc_it == m_object_group_allocated_pp.end() || alloc_it->second < avail_it->second)
            retval.insert(avail_it->first);
    }
    return retval;
}

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

const ProductionQueue::Element& ProductionQueue::operator[](int i) const {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ProductionQueue::const_iterator ProductionQueue::UnderfundedProject() const {
    const Empire* empire = Empires().Lookup(m_empire_id);
    if (!empire)
        return end();
    for (const_iterator it = begin(); it != end(); ++it) {

        float item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(*it);

        item_cost *= it->blocksize;
        float maxPerTurn = item_cost / std::max(build_turns,1);
        if (it->allocated_pp && (it->allocated_pp < (maxPerTurn-EPSILON)) && (1 < it->turns_left_to_next_item) )
            return it;
    }
    return end();
}

void ProductionQueue::Update() {
    const Empire* empire = Empires().Lookup(m_empire_id);
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

        ProductionQueueChangedSignal(); // need this so BuildingsPanel updates properly after removing last building
        return;                         // nothing to do for an empty queue
    }

    ScopedTimer update_timer("ProductionQueue::Update");

    std::map<std::set<int>, float> available_pp = AvailablePP(empire->GetResourcePool(RE_INDUSTRY));

    // determine which resource sharing group each queue item is located in
    std::vector<std::set<int> > queue_element_groups;
    for (ProductionQueue::const_iterator queue_it = m_queue.begin(); queue_it != m_queue.end(); ++queue_it) {
        // get location object for element
        const ProductionQueue::Element& element = *queue_it;
        int location_id = element.location;

        // search through groups to find object
        for (std::map<std::set<int>, float>::const_iterator groups_it = available_pp.begin();
             true; ++groups_it)
        {
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
    SetProdQueueElementSpending(available_pp, queue_element_groups, m_queue,
                                m_object_group_allocated_pp, m_projects_in_progress, m_empire_id);


    // if at least one resource-sharing system group have available PP, simulate
    // future turns to predict when build items will be finished
    bool simulate_future = false;
    for (std::map<std::set<int>, float>::const_iterator available_it = available_pp.begin();
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
        for (ProductionQueue::QueueType::iterator queue_it = m_queue.begin();
             queue_it != m_queue.end(); ++queue_it)
        {
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
    std::vector<std::set<int> > sim_queue_element_groups = queue_element_groups;
    std::vector<int>            simulation_results(sim_queue.size(), -1);
    std::vector<unsigned int>   sim_queue_original_indices(sim_queue.size());
    for (unsigned int i = 0; i < sim_queue_original_indices.size(); ++i)
        sim_queue_original_indices[i] = i;


    const int TOO_MANY_TURNS = 500;     // stop counting turns to completion after this long, to prevent seemingly endless loops
    const float TOO_LONG_TIME = 0.5f;   // max time in ms to spend simulating queue


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
        if (group.empty() || !empire->ProducibleItem(sim_queue[i].item, sim_queue[i].location)) {        // empty group or not buildable
            remove = true;
        } else {
            std::map<std::set<int>, float>::const_iterator available_it = available_pp.find(group);
            if (available_it == available_pp.end() || available_it->second < EPSILON)                   // missing group or non-empty group with no PP available
                remove = true;
        }

        if (remove) {
            // remove unbuildable items from the simulated queue, since they'll never finish...
            m_queue[sim_queue_original_indices[i]].turns_left_to_completion = -1;   // turns left is indeterminate for this item
            m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = -1;   // turns left is indeterminate for this item
            sim_queue.erase(sim_queue.begin() + i);
            sim_queue_element_groups.erase(sim_queue_element_groups.begin() + i);
            sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
        }
    }

    boost::posix_time::ptime dp_time_start;
    boost::posix_time::ptime dp_time_end;
    long dp_time;

    // "Dynamic Programming" version of queue simulator -- copy the queue simulator containers at this point, after removal of unbuildable items,
    // perform dynamic programming calculation of completion times, then after regular simulation is done compare results

    // initialize production queue to 'never' status
    for (ProductionQueue::QueueType::iterator queue_it = m_queue.begin(); queue_it != m_queue.end(); ++queue_it) {
        queue_it->turns_left_to_next_item = -1;     // -1 is sentinel value indicating never to be complete.  ProductionWnd checks for turns to completeion less than 0 and displays "NEVER" when appropriate
        queue_it->turns_left_to_completion = -1;
    }

    // duplicate simulation production queue state (post-bad-item-removal) for dynamic programming
    QueueType                   dpsim_queue = sim_queue;
    //std::vector<std::set<int> > sim_queue_element_groups = queue_element_groups;  //not necessary to duplicate this since won't be further modified
    std::vector<int>            dpsimulation_results_to_next(sim_queue.size(), -1);
    std::vector<int>            dpsimulation_results_to_completion(sim_queue.size(), -1);
    std::vector<unsigned int>   dpsim_queue_original_indices(sim_queue_original_indices); 

    const unsigned int DP_TURNS = TOO_MANY_TURNS; // track up to this many turns
    const float DP_TOO_LONG_TIME = TOO_LONG_TIME;   // max time in ms to spend simulating queue

    // The DP version will do calculations for one resource group at a time
    // unfortunately need to copy code from SetProdQueueElementSpending  in order to work it in more efficiently here
    dp_time_start = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 

    //invert lookup direction of sim_queue_element_groups:
    std::map< std::set<int>, std::vector<int>  > elementsByGroup;
    for (unsigned int i = 0; i < dpsim_queue.size(); ++i)
        elementsByGroup[sim_queue_element_groups[i]].push_back(i);


    // cache production item costs and times
    std::map<std::pair<ProductionQueue::ProductionItem, int>,
                std::pair<float, int> >                           queue_item_costs_and_times;
    for (ProductionQueue::iterator it = m_queue.begin(); it != m_queue.end(); ++it) {
        ProductionQueue::Element& elem = *it;

        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        std::pair<ProductionQueue::ProductionItem, int> key(elem.item, location_id);

        if (queue_item_costs_and_times.find(key) == queue_item_costs_and_times.end())
            queue_item_costs_and_times[key] = empire->ProductionCostAndTime(elem);
    }


    // within each group, allocate PP to queue items
    for (std::map<std::set<int>, float>::const_iterator groups_it = available_pp.begin();
         groups_it != available_pp.end(); ++groups_it)
    {
        unsigned int firstTurnPPAvailable = 1; //the first turn any pp in this resource group is available to the next item for this group
        unsigned int turnJump = 0;
        //ppStillAvailable[turn-1] gives the PP still available in this resource pool at turn "turn"
        std::vector<float> ppStillAvailable(DP_TURNS, groups_it->second );  // initialize to the groups full PP allocation for each turn modeled

        std::vector<int> &thisGroupsElements = elementsByGroup[ groups_it->first ];
        std::vector<int>::const_iterator groupBegin = thisGroupsElements.begin();
        std::vector<int>::const_iterator groupEnd = thisGroupsElements.end();

        // cycle through items on queue, if in this resource group then allocate production costs over time against those available to group
        for (std::vector<int>::const_iterator el_it = groupBegin;
             (el_it != groupEnd) && ((boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time())-dp_time_start).total_microseconds()*1e-6 < DP_TOO_LONG_TIME);
             ++el_it)
        {
            firstTurnPPAvailable += turnJump;
            turnJump = 0;
            if (firstTurnPPAvailable > DP_TURNS) {
                Logger().debugStream()  << "ProductionQueue::Update: Projections for Resource Group halted at " 
                                        << DP_TURNS << " turns; remaining items in this RG marked completing 'Never'.";
                break; // this resource group is allocated-out for span of simulation; remaining items in group left as never completing
            }

            unsigned int i = *el_it;
            ProductionQueue::Element& element = dpsim_queue[i];

            // get cost and time from cache
            int location_id = (element.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : element.location);
            std::pair<ProductionQueue::ProductionItem, int> key(element.item, location_id);
            float item_cost;
            int build_turns;
            boost::tie(item_cost, build_turns) = queue_item_costs_and_times[key];


            item_cost *= element.blocksize;
            float element_total_cost = item_cost * element.remaining;              // total PP to build all items in this element
            float element_per_turn_limit = item_cost / std::max(build_turns, 1);
            float additional_pp_to_complete_element = element_total_cost - element.progress; // additional PP, beyond already-accumulated PP, to build all items in this element
            if (additional_pp_to_complete_element < EPSILON) {
                m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = 1;
                m_queue[sim_queue_original_indices[i]].turns_left_to_completion = 1;
                continue;
            }

            unsigned int max_turns = std::max(std::max(build_turns, 1),
                1 + static_cast<int>(additional_pp_to_complete_element /
                                     ppStillAvailable[firstTurnPPAvailable-1]));

            max_turns = std::min(max_turns, DP_TURNS - firstTurnPPAvailable + 1);

            float allocation;
            //Logger().debugStream() << "ProductionQueue::Update Queue index   Queue Item: " << element.item.name;

            for (unsigned int j = 0; j < max_turns; j++) {  // iterate over the turns necessary to complete item
                // determine how many pp to allocate to this queue element this turn.  allocation is limited by the
                // item cost, which is the max number of PP per turn that can be put towards this item, and by the
                // total cost remaining to complete the last item in the queue element (eg. the element has all but
                // the last item complete already) and by the total pp available in this element's production location's
                // resource sharing group
                allocation = std::min(std::min(additional_pp_to_complete_element, element_per_turn_limit), ppStillAvailable[firstTurnPPAvailable+j-1]);
                allocation = std::max(allocation, 0.0f);     // added max (..., 0.0) to prevent any negative-allocation bugs that might come up...
                element.progress += allocation;   // add turn's allocation
                additional_pp_to_complete_element = element_total_cost - element.progress;
                ppStillAvailable[firstTurnPPAvailable+j-1] -= allocation;
                if (ppStillAvailable[firstTurnPPAvailable+j-1] <= EPSILON ) {
                    ppStillAvailable[firstTurnPPAvailable+j-1] = 0;
                    ++turnJump;
                }

                // check if additional turn's PP allocation was enough to finish next item in element
                if (item_cost - EPSILON <= element.progress ) {
                    // an item has been completed. 
                    // deduct cost of one item from accumulated PP.  don't set
                    // accumulation to zero, as this would eliminate any partial
                    // completion of the next item
                    element.progress -= item_cost;
                    --element.remaining;  //pretty sure this just effects the dp version & should do even if also doing ORIG_SIMULATOR

                    //Logger().debugStream() << "ProductionQueue::Recording DP sim results for item " << element.item.name;

                    // if this was the first item in the element to be completed in
                    // this simuation, update the original queue element with the
                    // turns required to complete the next item in the element
                    if (element.remaining +1 == m_queue[sim_queue_original_indices[i]].remaining) //had already decremented element.remaining above
                        m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = firstTurnPPAvailable+j;
                    if (!element.remaining) {
                        m_queue[sim_queue_original_indices[i]].turns_left_to_completion = firstTurnPPAvailable+j;    // record the (estimated) turns to complete the whole element on the original queue
                    }
                }
                if (!element.remaining) {
                    break; // this element all done
                }
            } //j-loop : turns relative to firstTurnPPAvailable
        } // queue element loop
    } // resource groups loop

    dp_time_end = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
    dp_time = (dp_time_end - dp_time_start).total_microseconds();
    if ((dp_time * 1e-6) >= DP_TOO_LONG_TIME)
        Logger().debugStream()  << "ProductionQueue::Update: Projections timed out after " << dp_time 
                                << " microseconds; all remaining items in queue marked completing 'Never'.";

    ProductionQueueChangedSignal();
}

void ProductionQueue::push_back(const Element& element)
{ m_queue.push_back(element); }

void ProductionQueue::insert(iterator it, const Element& element)
{ m_queue.insert(it, element); }

void ProductionQueue::erase(int i) {
    assert(i <= static_cast<int>(size()));
    m_queue.erase(begin() + i);
}

ProductionQueue::iterator ProductionQueue::erase(iterator it) {
    assert(it != end());
    return m_queue.erase(it);
}

ProductionQueue::iterator ProductionQueue::begin()
{ return m_queue.begin(); }

ProductionQueue::iterator ProductionQueue::end()
{ return m_queue.end(); }

ProductionQueue::iterator ProductionQueue::find(int i)
{ return (0 <= i && i < static_cast<int>(size())) ? (begin() + i) : end(); }

ProductionQueue::Element& ProductionQueue::operator[](int i) {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ProductionQueue::iterator ProductionQueue::UnderfundedProject() {
    const Empire* empire = Empires().Lookup(m_empire_id);
    if (!empire)
        return end();

    for (iterator it = begin(); it != end(); ++it) {

        float item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(*it);

        item_cost *= it->blocksize;
        float maxPerTurn = item_cost / std::max(build_turns,1);
        if (it->allocated_pp && (it->allocated_pp < (maxPerTurn-EPSILON)) && (1 < it->turns_left_to_next_item) )
            return it;
    }
    return end();
}

void ProductionQueue::clear() {
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
    m_capital_id(INVALID_OBJECT_ID),
    m_research_queue(m_id),
    m_production_queue(m_id),
    m_resource_pools(),
    m_population_pool()
{ Init(); }

Empire::Empire(const std::string& name, const std::string& player_name,
               int empire_id, const GG::Clr& color) :
    m_id(empire_id),
    m_name(name),
    m_player_name(player_name),
    m_color(color),
    m_capital_id(INVALID_OBJECT_ID),
    m_research_queue(m_id),
    m_production_queue(m_id),
    m_resource_pools(),
    m_population_pool()
{
    Logger().debugStream() << "Empire::Empire(" << name << ", " << player_name << ", " << empire_id << ", colour)";
    Init();
}

void Empire::Init() {
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
    m_meters[UserStringNop("METER_DETECTION_STRENGTH")];
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

int Empire::StockpileID(ResourceType res) const {
    switch (res) {
    case RE_TRADE:
        return m_capital_id;
        break;
    case RE_INDUSTRY:
    case RE_RESEARCH:
    default:
        return INVALID_OBJECT_ID;
        break;
    }
}

std::string Empire::Dump() const {
    std::string retval = "Empire name: " + m_name +
                         " ID: "+ boost::lexical_cast<std::string>(m_id) +
                         " Capital ID: " + boost::lexical_cast<std::string>(m_capital_id);
    retval += " meters:\n";
    for (std::map<std::string, Meter>::const_iterator meter_it = meter_begin();
            meter_it != meter_end(); ++meter_it)
    {
        retval += UserString(meter_it->first) + ": " +
                  boost::lexical_cast<std::string>(meter_it->second.Initial()) + "\n";
    }
    return retval;
}

void Empire::SetCapitalID(int id)
{ m_capital_id = id; }

Meter* Empire::GetMeter(const std::string& name) {
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

void Empire::BackPropegateMeters() {
    for (std::map<std::string, Meter>::iterator it = m_meters.begin(); it != m_meters.end(); ++it)
        it->second.BackPropegate();
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

float Empire::ResearchProgress(const std::string& name) const {
    std::map<std::string, float>::const_iterator it = m_research_progress.find(name);
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

const std::string& Empire::TopPriorityEnqueuedTech(bool only_consider_available_techs/* = false*/) const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    ResearchQueue::const_iterator it = m_research_queue.begin();
    const std::string& tech = it->name;
    return tech;
}

const std::string& Empire::MostExpensiveEnqueuedTech(bool only_consider_available_techs/* = false*/) const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float biggest_cost = -99999.9f; // arbitrary small number
    ResearchQueue::const_iterator best_it = m_research_queue.end();

    for (ResearchQueue::const_iterator it = m_research_queue.begin();
         it != m_research_queue.end(); ++it)
    {
        const Tech* tech = GetTech(it->name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id);
        if (tech_cost > biggest_cost) {
            biggest_cost = tech_cost;
            best_it = it;
        }
    }

    if (best_it != m_research_queue.end())
        return best_it->name;
    return EMPTY_STRING;
}

const std::string& Empire::LeastExpensiveEnqueuedTech(bool only_consider_available_techs/* = false*/) const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float smallest_cost = 999999.9f; // arbitrary large number
    ResearchQueue::const_iterator best_it = m_research_queue.end();

    for (ResearchQueue::const_iterator it = m_research_queue.begin();
         it != m_research_queue.end(); ++it)
    {
        const Tech* tech = GetTech(it->name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id);
        if (tech_cost < smallest_cost) {
            smallest_cost = tech_cost;
            best_it = it;
        }
    }

    if (best_it != m_research_queue.end())
        return best_it->name;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPSpentEnqueuedTech(bool only_consider_available_techs/* = false*/) const {
    float most_spent = -999999.9f;  // arbitrary small number
    std::map<std::string, float>::const_iterator best_it = m_research_progress.end();

    for (std::map<std::string, float>::const_iterator it = m_research_progress.begin();
         it != m_research_progress.end(); ++it)
    {
        const std::string& tech_name = it->first;
        if (m_research_queue.find(tech_name) == m_research_queue.end())
            continue;
        float rp_spent = it->second;
        if (rp_spent > most_spent) {
            best_it = it;
            most_spent = rp_spent;
        }
    }

    if (best_it != m_research_progress.end())
        return best_it->first;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPCostLeftEnqueuedTech(bool only_consider_available_techs/* = false*/) const {
    float most_left = -999999.9f;  // arbitrary small number
    std::map<std::string, float>::const_iterator best_it = m_research_progress.end();

    for (std::map<std::string, float>::const_iterator it = m_research_progress.begin();
         it != m_research_progress.end(); ++it)
    {
        const std::string& tech_name = it->first;
        const Tech* tech = GetTech(tech_name);
        if (!tech)
            continue;

        if (m_research_queue.find(tech_name) == m_research_queue.end())
            continue;

        float rp_spent = it->second;
        float rp_total_cost = tech->ResearchCost(m_id);
        float rp_left = std::max(0.0f, rp_total_cost - rp_spent);

        if (rp_left > most_left) {
            best_it = it;
            most_left = rp_left;
        }
    }

    if (best_it != m_research_progress.end())
        return best_it->first;
    return EMPTY_STRING;
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
    //// if design isn't kept by this empire, it can't be built.
    //if (!ShipDesignKept(ship_design_id))
    //    return false;   //   The empire needs to issue a ShipDesignOrder to add this design id to its kept designs

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

float Empire::ProductionStatus(int i) const
{ return (0 <= i && i < static_cast<int>(m_production_queue.size())) ? m_production_queue[i].progress : -1.0; }

std::pair<float, int> Empire::ProductionCostAndTime(const ProductionQueue::Element& element) const
{ return ProductionCostAndTime(element.item, element.location); }

std::pair<float, int> Empire::ProductionCostAndTime(const ProductionQueue::ProductionItem& item,
                                                     int location_id) const
{
    if (item.build_type == BT_BUILDING) {
        const BuildingType* type =  GetBuildingType(item.name);
        if (!type)
            return std::make_pair(-1.0, -1);
        return std::make_pair(type->ProductionCost(m_id, location_id),
                              type->ProductionTime(m_id, location_id));
    } else if (item.build_type == BT_SHIP) {
        const ShipDesign* design = GetShipDesign(item.design_id);
        if (design)
            return std::make_pair(design->ProductionCost(m_id, location_id),
                                  design->ProductionTime(m_id, location_id));
        return std::make_pair(-1.0, -1);
    }
    Logger().errorStream() << "Empire::ProductionCostAndTime was passed a ProductionItem with an invalid BuildType";
    return std::make_pair(-1.0, -1);
}

bool Empire::HasExploredSystem(int ID) const
{ return (m_explored_systems.find(ID) != m_explored_systems.end()); }

bool Empire::ProducibleItem(BuildType build_type, const std::string& name, int location) const {
    // special case to check for ships being passed with names, not design ids
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_SHIP with a name, but ship designs are tracked by number");

    if (build_type == BT_BUILDING && !BuildingTypeAvailable(name))
        return false;

    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    TemporaryPtr<UniverseObject> build_location = GetUniverseObject(location);
    if (!build_location)
        return false;

    if (build_type == BT_BUILDING) {
        // specified location must be a valid production location for that building type
        return building_type->ProductionLocation(m_id, location);

    } else {
        Logger().errorStream() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::ProducibleItem(BuildType build_type, int design_id, int location) const {
    // special case to check for buildings being passed with ids, not names
    if (build_type == BT_BUILDING)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_BUILDING with a design id number, but these types are tracked by name");

    if (build_type == BT_SHIP && !ShipDesignAvailable(design_id))
        return false;

    // design must be known to this empire
    const ShipDesign* ship_design = GetShipDesign(design_id);
    if (!ship_design || !ship_design->Producible())
        return false;

    TemporaryPtr<UniverseObject> build_location = GetUniverseObject(location);
    if (!build_location) return false;

    if (build_type == BT_SHIP) {
        // specified location must be a valid production location for this design
        return ship_design->ProductionLocation(m_id, location);

    } else {
        Logger().errorStream() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::ProducibleItem(const ProductionQueue::ProductionItem& item, int location) const {
    if (item.build_type == BT_BUILDING)
        return ProducibleItem(item.build_type, item.name, location);
    else if (item.build_type == BT_SHIP)
        return ProducibleItem(item.build_type, item.design_id, location);
    else
        throw std::invalid_argument("Empire::ProducibleItem was passed a ProductionItem with an invalid BuildType");
    return false;
}

int Empire::NumSitRepEntries(int turn/* = INVALID_GAME_TURN*/) const {
    if (turn == INVALID_GAME_TURN)
        return m_sitrep_entries.size();
    int count = 0;
    for (SitRepItr it = SitRepBegin(); it != SitRepEnd(); ++it)
        if (it->GetTurn() == turn)
            count++;
    return count;
}

void Empire::EliminationCleanup() {
    // some Empire data not cleared when eliminating since it might be useful
    // to remember later, and having it doesn't hurt anything (as opposed to
    // the production queue that might actually cause some problems if left
    // uncleared after elimination

    m_capital_id = INVALID_OBJECT_ID;
    // m_techs
    m_research_queue.clear();
    m_research_progress.clear();
    m_production_queue.clear();
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

    // m_ship_names_used;
    m_supply_system_ranges.clear();
    m_supply_unobstructed_systems.clear();
    m_supply_starlane_traversals.clear();
    m_supply_starlane_obstructed_traversals.clear();
    m_fleet_supplyable_system_ids.clear();
    m_resource_supply_groups.clear();
}

void Empire::UpdateSystemSupplyRanges(const std::set<int>& known_objects) {
    //std::cout << "Empire::UpdateSystemSupplyRanges() for empire " << this->Name() << std::endl;
    m_supply_system_ranges.clear();

    // as of this writing, only planets can generate supply propegation
    std::vector<TemporaryPtr<const UniverseObject> > owned_planets;
    for (std::set<int>::const_iterator it = known_objects.begin(); it != known_objects.end(); ++it) {
        if (TemporaryPtr<const Planet> planet = GetPlanet(*it))
            if (planet->OwnedBy(this->EmpireID()))
                owned_planets.push_back(planet);
    }

    //std::cout << "... empire owns " << owned_planets.size() << " planets" << std::endl;
    for (std::vector<TemporaryPtr<const UniverseObject> >::const_iterator it = owned_planets.begin(); it != owned_planets.end(); ++it) {
        TemporaryPtr<const UniverseObject> obj = *it;
        //std::cout << "... considering owned planet: " << obj->Name() << std::endl;

        // ensure object is within a system, from which it can distribute supplies
        int system_id = obj->SystemID();
        if (system_id == INVALID_OBJECT_ID)
            continue;   // TODO: consider future special case if current object is itself a system

        // check if object has a supply meter
        if (obj->GetMeter(METER_SUPPLY)) {
            // get resource supply range for next turn for this object
            int supply_range = static_cast<int>(floor(obj->NextTurnCurrentMeterValue(METER_SUPPLY)));

            // if this object can provide more supply range than the best previously checked object in this system, record its range as the new best for the system
            std::map<int, int>::iterator system_it = m_supply_system_ranges.find(system_id);               // try to find a previous entry for this system's supply range
            if (system_it == m_supply_system_ranges.end() || supply_range > system_it->second) {  // if there is no previous entry, or the previous entry is shorter than the new one, add or replace the entry
                //std::cout << " ... object " << obj->Name() << " has resource supply range: " << resource_supply_range << std::endl;
                m_supply_system_ranges[system_id] = supply_range;
            }
        }
    }
}

void Empire::UpdateSystemSupplyRanges() {
    const Universe& universe = GetUniverse();
    const ObjectMap& empire_known_objects = EmpireKnownObjects(this->EmpireID());

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

void Empire::UpdateUnobstructedFleets() {
    const std::set<int>& known_destroyed_objects =
        GetUniverse().EmpireKnownDestroyedObjectIDs(this->EmpireID());

    for (std::set<int>::const_iterator sys_it = m_supply_unobstructed_systems.begin();
         sys_it != m_supply_unobstructed_systems.end(); ++sys_it)
    {
        TemporaryPtr<const System> system = GetSystem(*sys_it);
        if (!system)
            continue;

        std::vector<TemporaryPtr<Fleet> > fleets = Objects().FindObjects<Fleet>(system->FleetIDs());

        for (std::vector<TemporaryPtr<Fleet> >::iterator fleet_it = fleets.begin();
             fleet_it != fleets.end(); ++fleet_it)
        {
            TemporaryPtr<Fleet> fleet = *fleet_it;
            if (known_destroyed_objects.find(fleet->ID()) != known_destroyed_objects.end())
                continue;
            if (fleet->OwnedBy(m_id))
                fleet->SetArrivalStarlane(*sys_it);
        }
    }
}

void Empire::UpdateSupplyUnobstructedSystems() {
    Universe& universe = GetUniverse();

    // get ids of systems partially or better visible to this empire.
    // TODO: make a UniverseObjectVisitor for objects visible to an empire at a specified visibility or greater
    std::vector<int> known_systems_vec = EmpireKnownObjects(this->EmpireID()).FindObjectIDs<System>();
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
    const std::vector<TemporaryPtr<Fleet> > fleets = GetUniverse().Objects().FindObjects<Fleet>();
    const std::set<int>& known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(this->EmpireID());

    // find systems that contain fleets that can either maintaining supply or block supply
    // to affect supply in either manner a fleet must be armed & aggressive, & must be not 
    // trying to depart the systme.  Qualifying enemy fleets will blockade if no friendly fleets
    // are present, or if the friendly fleets were already blockade-restricted and the enemy
    // fleets were not (meaning that the enemy fleets were continuing an existing blockade)
    // Friendly fleets can preserve available starlane accesss even if they are trying to leave the system

    // Unrestricted lane access (i.e, (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for 
    // order of arrival -- if an enemy has unrestricted lane access and you don't, they must have arrived
    // before you, or be in cahoots with someone who did.
    std::set<int> systems_containing_friendly_fleets;
    std::set<int> systems_with_lane_preserving_fleets;
    std::set<int> unrestricted_friendly_systems;
    std::set<int> systems_containing_obstructing_objects;
    std::set<int> unrestricted_obstruction_systems;
    for (std::vector<TemporaryPtr<Fleet> >::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
        TemporaryPtr<const Fleet> fleet = *it;
        int system_id = fleet->SystemID();
        if (system_id == INVALID_OBJECT_ID) {
            continue;   // not in a system, so can't affect system obstruction
        } else if (known_destroyed_objects.find(fleet->ID()) != known_destroyed_objects.end()) {
            continue; //known to be destroyed so can't affect supply, important just in case being updated on client side
        }
        if (fleet->HasArmedShips() && fleet->Aggressive()) {
            if (fleet->OwnedBy(m_id)) {
                if (fleet->NextSystemID()==INVALID_OBJECT_ID) {
                    systems_containing_friendly_fleets.insert(system_id);
                    if (fleet->ArrivalStarlane()==system_id)
                        unrestricted_friendly_systems.insert(system_id);
                } else {
                    systems_with_lane_preserving_fleets.insert(system_id);
                }
            } else if (fleet->NextSystemID()==INVALID_OBJECT_ID) {
                int fleet_owner = fleet->Owner();
                if (fleet_owner == ALL_EMPIRES || Empires().GetDiplomaticStatus(m_id, fleet_owner) == DIPLO_WAR) {
                    systems_containing_obstructing_objects.insert(system_id);
                    if (fleet->ArrivalStarlane()==system_id)
                        unrestricted_obstruction_systems.insert(system_id);
                }
            }
        }
    }

    // check each potential supplyable system for whether it can propagate supply.
    for (std::set<int>::const_iterator known_systems_it = known_systems.begin(); known_systems_it != known_systems.end(); ++known_systems_it) {
        int sys_id = *known_systems_it;

        // has empire ever seen this system with partial or better visibility?
        if (systems_with_at_least_partial_visibility_at_some_point.find(sys_id) == systems_with_at_least_partial_visibility_at_some_point.end())
            continue;

        // if system is explored, then whether it can propagate supply depends
        // on what friendly / enemy ships are in the system

        if (unrestricted_friendly_systems.find(sys_id) != unrestricted_friendly_systems.end()) 
        {
            // if there are unrestricted friendly ships, supply can propagate
            m_supply_unobstructed_systems.insert(sys_id);
        } else if (systems_containing_friendly_fleets.find(sys_id) != systems_containing_friendly_fleets.end()) {
            if (unrestricted_obstruction_systems.find(sys_id) == unrestricted_obstruction_systems.end()) {
                // if there are (previously) restricted friendly ships, and no unrestricted enemy fleets, supply can propagate
                m_supply_unobstructed_systems.insert(sys_id);
            }
        } else if (systems_containing_obstructing_objects.find(sys_id) == systems_containing_obstructing_objects.end()) {
            // if there are no friendly ships and no enemy ships, supply can propegate
            m_supply_unobstructed_systems.insert(sys_id);
        } else if (systems_with_lane_preserving_fleets.find(sys_id) == systems_with_lane_preserving_fleets.end()) {
        // otherwise, if system contains no friendly fleets capable of maintaining lane access but does contain an
        // unfriendly fleet, so it is obstructed, so isn't included in the
        // unobstructed systems set.  Furthermore, this empire's available 
        // system exit lanes for this system are cleared
            if (!m_available_system_exit_lanes[sys_id].empty()) {
                //Logger().debugStream() << "Empire::UpdateSupplyUnobstructedSystems clearing available lanes for system ("<<sys_id<<"); available lanes were:";
                //for (std::set<int>::iterator lane_it = m_available_system_exit_lanes[sys_id].begin(); lane_it != m_available_system_exit_lanes[sys_id].end(); lane_it++)
                //    Logger().debugStream() << "...... "<< *lane_it;
                m_available_system_exit_lanes[sys_id].clear();
            }
        }
    }
}

void Empire::RecordPendingLaneUpdate(int start_system_id, int dest_system_id) {
    if (m_supply_unobstructed_systems.find(start_system_id) == m_supply_unobstructed_systems.end()) {
        m_pending_system_exit_lanes[start_system_id].insert(dest_system_id); 

    } else { // if the system is unobstructed, mark all its lanes as avilable
        TemporaryPtr<const System> system = GetSystem(start_system_id);
        const std::map<int, bool>& lanes = system->StarlanesWormholes();
        for (std::map<int, bool>::const_iterator it = lanes.begin(); it != lanes.end(); ++it) {
            m_pending_system_exit_lanes[start_system_id].insert(it->first); // will add both starlanes and wormholes
        }
    }
}

void Empire::UpdateAvailableLanes() {
    for (std::map<int, std::set<int> >::iterator sys_it = m_pending_system_exit_lanes.begin(); 
         sys_it != m_pending_system_exit_lanes.end(); sys_it++)
    {
        m_available_system_exit_lanes[sys_it->first].insert(sys_it->second.begin(), sys_it->second.end());
        sys_it->second.clear();
    }
    m_pending_system_exit_lanes.clear(); // TODO: consider: not really necessary, & may be more efficient to not clear.
}

void Empire::UpdateSupply()
{ UpdateSupply(this->KnownStarlanes()); }

void Empire::UpdateSupply(const std::map<int, std::set<int> >& starlanes) {
    //std::cout << "Empire::UpdateSupply for empire " << this->Name() << std::endl;

    m_supply_starlane_traversals.clear();
    m_supply_starlane_obstructed_traversals.clear();
    m_fleet_supplyable_system_ids.clear();
    m_resource_supply_groups.clear();

    // need to get a set of sets of systems that can exchange resources.  some
    // sets may be just one system, in which resources can be exchanged between
    // UniverseObjects producing or consuming them, but which can't exchange
    // with any other systems.

    // map from system id to set of systems that are supply-connected to it
    // directly (which may involve multiple starlane jumps
    std::map<int, std::set<int> > supply_groups_map;


    // store supply range in jumps of all unobstructed systems before
    // propegation, and add to list of systems to propegate from.
    std::map<int, int> propegating_supply_ranges;
    std::list<int> propegating_systems_list;
    for (std::map<int, int>::const_iterator it = m_supply_system_ranges.begin();
         it != m_supply_system_ranges.end(); ++it)
    {
        if (m_supply_unobstructed_systems.find(it->first) != m_supply_unobstructed_systems.end())
            propegating_supply_ranges.insert(*it);

        // system can supply itself, so store this fact
        supply_groups_map[it->first].insert(it->first);

        // add system to list of systems to popegate supply from
        propegating_systems_list.push_back(it->first);
    }


    // iterate through list of accessible systems, processing each in order it
    // was added (like breadth first search) until no systems are left able to
    // further propregate
    std::list<int>::iterator sys_list_it = propegating_systems_list.begin();
    std::list<int>::iterator sys_list_end = propegating_systems_list.end();
    while (sys_list_it != sys_list_end) {
        int cur_sys_id = *sys_list_it;
        int cur_sys_range = propegating_supply_ranges[cur_sys_id];    // range away from this system that supplies can be transported

        // any unobstructed system can share resources within itself
        supply_groups_map[cur_sys_id].insert(cur_sys_id);

        if (cur_sys_range <= 0) {
            // can't propegate supply out a system that has no range
            ++sys_list_it;
            continue;
        }

        // any system with nonzero fleet supply range can provide fleet supply
        m_fleet_supplyable_system_ids.insert(cur_sys_id);

        // can propegate further, if adjacent systems have smaller supply range
        // than one less than this system's range
        std::map<int, std::set<int> >::const_iterator system_it = starlanes.find(cur_sys_id);
        if (system_it == starlanes.end()) {
            // no starlanes out of this system
            ++sys_list_it;
            continue;
        }


        const std::set<int>& starlane_ends = system_it->second;
        for (std::set<int>::const_iterator lane_it = starlane_ends.begin();
             lane_it != starlane_ends.end(); ++lane_it)
        {
            int lane_end_sys_id = *lane_it;

            if (m_supply_unobstructed_systems.find(lane_end_sys_id) == m_supply_unobstructed_systems.end()) {
                // can't propegate here
                m_supply_starlane_obstructed_traversals.insert(std::make_pair(cur_sys_id, lane_end_sys_id));
                continue;
            }

            // can supply fleets here
            m_fleet_supplyable_system_ids.insert(lane_end_sys_id);

            // compare next system's supply range to this system's supply range.  propegate if necessary.
            std::map<int, int>::const_iterator lane_end_sys_it = propegating_supply_ranges.find(lane_end_sys_id);
            if (lane_end_sys_it == propegating_supply_ranges.end() || lane_end_sys_it->second <= cur_sys_range) {
                // next system has no supply yet, or its range equal to or smaller than this system's

                // update next system's range, if propegating from this system would make it larger
                if (lane_end_sys_it == propegating_supply_ranges.end() || lane_end_sys_it->second < cur_sys_range - 1) {
                    // update with new range
                    propegating_supply_ranges[lane_end_sys_id] = cur_sys_range - 1;
                    // add next system to list of systems to propegate further
                    propegating_systems_list.push_back(lane_end_sys_id);
                }

                // regardless of whether propegating from current to next system
                // increased its range, add the traversed lane to show
                // redundancies in supply network to player
                m_supply_starlane_traversals.insert(std::make_pair(cur_sys_id, lane_end_sys_id));

                // current system can share resources with next system
                supply_groups_map[cur_sys_id].insert(lane_end_sys_id);
                supply_groups_map[lane_end_sys_id].insert(cur_sys_id);
                //Logger().debugStream() << "added sys(" << lane_end_sys_id << ") to supply_groups_map[ " << cur_sys_id<<"]";
                //Logger().debugStream() << "added sys(" << cur_sys_id << ") to supply_groups_map[ " << lane_end_sys_id <<"]";
            }
        }
        ++sys_list_it;
        sys_list_end = propegating_systems_list.end();
    }

    // Need to merge interconnected supply groups into as few sets of mutually-
    // supply-exchanging systems as possible.  This requires finding the
    // connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.

    // create graph
    boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

    // boost expects vertex labels to range from 0 to num vertices - 1, so need
    // to map from system id to graph id and back when accessing vertices
    std::vector<int> graph_id_to_sys_id;
    graph_id_to_sys_id.reserve(supply_groups_map.size());

    std::map<int, int> sys_id_to_graph_id;
    int graph_id = 0;
    for (std::map<int, std::set<int> >::const_iterator sys_it = supply_groups_map.begin();
         sys_it != supply_groups_map.end(); ++sys_it, ++graph_id)
    {
        int sys_id = sys_it->first;
        //TemporaryPtr<const System> sys = GetSystem(sys_id);
        //std::string name = sys->Name();
        //Logger().debugStream() << "supply-exchanging system: " << name << " ID (" << sys_id <<")";

        boost::add_vertex(graph);   // should add with index = graph_id

        graph_id_to_sys_id.push_back(sys_id);
        sys_id_to_graph_id[sys_id] = graph_id;
    }

    // add edges for all direct connections between systems
    for (std::map<int, std::set<int> >::const_iterator maps_it = supply_groups_map.begin();
         maps_it != supply_groups_map.end(); ++maps_it)
    {
        int start_graph_id = sys_id_to_graph_id[maps_it->first];
        const std::set<int>& set = maps_it->second;
        for(std::set<int>::const_iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
            int end_graph_id = sys_id_to_graph_id[*set_it];
            boost::add_edge(start_graph_id, end_graph_id, graph);

            //int sys_id1 = graph_id_to_sys_id[start_graph_id];
            //TemporaryPtr<const System> sys1 = GetSystem(sys_id1);
            //std::string name1 = sys1->Name();
            //int sys_id2 = graph_id_to_sys_id[end_graph_id];
            //TemporaryPtr<const System> sys2 = GetSystem(sys_id2);
            //std::string name2 = sys2->Name();
            //Logger().debugStream() << "added edge to graph: " << name1 << " and " << name2;
        }
    }

    // declare storage and fill with the component id (group id of connected systems) for each graph vertex
    std::vector<int> components(boost::num_vertices(graph));
    boost::connected_components(graph, &components[0]);

    //for (std::vector<int>::size_type i = 0; i != components.size(); ++i) {
    //    int sys_id = graph_id_to_sys_id[i];
    //    TemporaryPtr<const System> sys = GetSystem(sys_id);
    //    std::string name = sys->Name();
    //    Logger().debugStream() << "system " << name <<" is in component " << components[i];
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
        //    TemporaryPtr<const UniverseObject> obj = GetUniverse().Object(*set_it);
        //    if (!obj) {
        //        Logger().debugStream() << " ... missing object!";
        //        continue;
        //    }
        //    Logger().debugStream() << " ... " << obj->Name();
        //}
    }
}

const std::map<int, int>& Empire::SystemSupplyRanges() const
{ return m_supply_system_ranges; }

const std::set<int>& Empire::SupplyUnobstructedSystems() const
{ return m_supply_unobstructed_systems; }

const std::set<std::pair<int, int> >& Empire::SupplyStarlaneTraversals() const
{ return m_supply_starlane_traversals; }

const std::set<std::pair<int, int> >& Empire::SupplyObstructedStarlaneTraversals() const
{ return m_supply_starlane_obstructed_traversals; }

const std::set<int>& Empire::FleetSupplyableSystemIDs() const
{ return m_fleet_supplyable_system_ids; }

bool Empire::SystemHasFleetSupply(int system_id) const {
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (m_fleet_supplyable_system_ids.find(system_id) != m_fleet_supplyable_system_ids.end())
        return true;
    return false;
}

const bool Empire::UnrestrictedLaneTravel(int start_system_id, int dest_system_id) const {
    std::map<int, std::set<int> >::const_iterator find_it = m_available_system_exit_lanes.find(start_system_id);
    if (find_it != m_available_system_exit_lanes.end() ) {
        if (find_it->second.find(dest_system_id) != find_it->second.end())
            return true;
    }
    return false;
}

const std::set<std::set<int> >& Empire::ResourceSupplyGroups() const
{ return m_resource_supply_groups; }

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

const std::map<int, std::set<int> > Empire::KnownStarlanes() const {
    // compile starlanes leading into or out of each system
    std::map<int, std::set<int> > retval;

    const Universe& universe = GetUniverse();

    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());
    for (ObjectMap::const_iterator<System> sys_it = Objects().const_begin<System>();
         sys_it != Objects().const_end<System>(); ++sys_it)
    {
        int start_id = sys_it->ID();

        // exclude lanes starting at systems known to be destroyed
        if (known_destroyed_objects.find(start_id) != known_destroyed_objects.end())
            continue;

        const std::map<int, bool>& lanes = sys_it->StarlanesWormholes();
        for (std::map<int, bool>::const_iterator lane_it = lanes.begin();
             lane_it != lanes.end(); ++lane_it)
        {
            if (lane_it->second || known_destroyed_objects.find(lane_it->second) != known_destroyed_objects.end())
                continue;   // is a wormhole, not a starlane, or is connected to a known destroyed system
            int end_id = lane_it->first;
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }
    }

    return retval;
}

const std::map<int, std::set<int> > Empire::VisibleStarlanes() const {
    std::map<int, std::set<int> > retval;   // compile starlanes leading into or out of each system

    const Universe& universe = GetUniverse();
    const ObjectMap& objects = universe.Objects();

    for (ObjectMap::const_iterator<System> sys_it = objects.const_begin<System>();
         sys_it != objects.const_end<System>(); ++sys_it)
    {
        int start_id = sys_it->ID();

        // is system visible to this empire?
        if (universe.GetObjectVisibilityByEmpire(start_id, m_id) <= VIS_NO_VISIBILITY)
            continue;

        // get system's visible lanes for this empire
        std::map<int, bool> lanes = sys_it->VisibleStarlanesWormholes(m_id);

        // copy to retval
        for (std::map<int, bool>::const_iterator lane_it = lanes.begin();
             lane_it != lanes.end(); ++lane_it)
        {
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

float Empire::ProductionPoints() const
{ return GetResourcePool(RE_INDUSTRY)->TotalAvailable(); }

const boost::shared_ptr<ResourcePool> Empire::GetResourcePool(ResourceType resource_type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        return boost::shared_ptr<ResourcePool>();
    return it->second;
}

float Empire::ResourceStockpile(ResourceType type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceStockpile passed invalid ResourceType");
    return it->second->Stockpile();
}

float Empire::ResourceProduction(ResourceType type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceProduction passed invalid ResourceType");
    return it->second->Production();
}

float Empire::ResourceAvailable(ResourceType type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceAvailable passed invalid ResourceType");
    return it->second->TotalAvailable();
}

const PopulationPool& Empire::GetPopulationPool() const
{ return m_population_pool; }

float Empire::Population() const
{ return m_population_pool.Population(); }

void Empire::SetResourceStockpile(ResourceType resource_type, float stockpile) {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::SetResourceStockpile passed invalid ResourceType");
    return it->second->SetStockpile(stockpile);
}

void Empire::PlaceTechInQueue(const std::string& name, int pos/* = -1*/) {
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
}

void Empire::RemoveTechFromQueue(const std::string& name) {
    ResearchQueue::iterator it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        m_research_queue.erase(it);
}

void Empire::SetTechResearchProgress(const std::string& name, float progress) {
    const Tech* tech = GetTech(name);
    if (!tech) {
        Logger().errorStream() << "Empire::SetTechResearchProgress no such tech as: " << name;
        return;
    }
    if (TechResearched(name))
        return; // can't affect already-researched tech

    // set progress
    float clamped_progress = std::min(tech->ResearchCost(m_id), std::max(0.0f, progress));
    m_research_progress[name] = clamped_progress;

    // if tech is complete, ensure it is on the queue, so it will be researched next turn
    if (clamped_progress >= tech->ResearchCost(m_id) &&
        m_research_queue.find(name) == m_research_queue.end())
    { m_research_queue.push_back(name); }

    // don't just give tech to empire, as another effect might reduce its progress before end of turn
}

const unsigned int MAX_PROD_QUEUE_SIZE = 500;

void Empire::PlaceBuildInQueue(BuildType build_type, const std::string& name, int number, int location, int pos/* = -1*/) {
    if (!ProducibleItem(build_type, name, location))
        Logger().debugStream() << "Empire::PlaceBuildInQueue() : Placed a non-buildable item in queue...";

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE)
        return;

    ProductionQueue::Element build(build_type, name, number, number, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(build);
    else
        m_production_queue.insert(m_production_queue.begin() + pos, build);
}

void Empire::PlaceBuildInQueue(BuildType build_type, int design_id, int number, int location, int pos/* = -1*/) {
    if (!ProducibleItem(build_type, design_id, location))
        Logger().debugStream() << "Empire::PlaceBuildInQueue() : Placed a non-buildable item in queue...";

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE)
        return;

    ProductionQueue::Element build(build_type, design_id, number, number, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(build);
    else
        m_production_queue.insert(m_production_queue.begin() + pos, build);
}

void Empire::PlaceBuildInQueue(const ProductionQueue::ProductionItem& item, int number, int location, int pos/* = -1*/) {
    if (item.build_type == BT_BUILDING)
        PlaceBuildInQueue(item.build_type, item.name, number, location, pos);
    else if (item.build_type == BT_SHIP)
        PlaceBuildInQueue(item.build_type, item.design_id, number, location, pos);
    else
        throw std::invalid_argument("Empire::PlaceBuildInQueue was passed a ProductionQueue::ProductionItem with an invalid BuildType");
}

void Empire::SetBuildQuantityAndBlocksize(int index, int quantity, int blocksize) {
    Logger().debugStream() << "Empire::SetBuildQuantityAndBlocksize() called for item "<< m_production_queue[index].item.name << "with new quant " << quantity << " and new blocksize " << blocksize;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && ((1 < quantity) || ( 1 < blocksize) ))
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    int original_blocksize = m_production_queue[index].blocksize;
    blocksize = std::max(1, blocksize);
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
    m_production_queue[index].blocksize = blocksize;
    if (blocksize !=original_blocksize) // if reducing, may lose the progress from the excess former blocksize, or min-turns-to-build could be bypassed; if increasing, may be able to claim credit if undoing a recent decrease
        m_production_queue[index].progress = (m_production_queue[index].progress_memory / m_production_queue[index].blocksize_memory ) * std::min( m_production_queue[index].blocksize_memory, blocksize);
}

void Empire::SetBuildQuantity(int index, int quantity) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && 1 < quantity)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
}

void Empire::MoveBuildWithinQueue(int index, int new_index) {
    if (index < new_index)
        --new_index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index ||
        new_index < 0 || static_cast<int>(m_production_queue.size()) <= new_index)
    {
        Logger().debugStream() << "Empire::MoveBuildWithinQueue index: " << index << "  new index: "
                               << new_index << "  queue size: " << m_production_queue.size();
        Logger().errorStream() << "Attempted to move a production queue item to or from an invalid index.";
        return;
    }
    ProductionQueue::Element build = m_production_queue[index];
    m_production_queue.erase(index);
    m_production_queue.insert(m_production_queue.begin() + new_index, build);
}

void Empire::RemoveBuildFromQueue(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        Logger().debugStream() << "Empire::RemoveBuildFromQueue index: " << index << "  queue size: " << m_production_queue.size();
        Logger().errorStream() << "Attempted to delete a production queue item with an invalid index.";
        return;
    }
    m_production_queue.erase(index);
}

void Empire::ConquerProductionQueueItemsAtLocation(int location_id, int empire_id) {
    if (location_id == INVALID_OBJECT_ID) {
        Logger().errorStream() << "Empire::ConquerProductionQueueItemsAtLocation: tried to conquer build items located at an invalid location";
        return;
    }

    Logger().debugStream() << "Empire::ConquerProductionQueueItemsAtLocation: conquering items located at "
                           << location_id << " to empire " << empire_id;

    Empire* to_empire = Empires().Lookup(empire_id);    // may be null
    if (!to_empire && empire_id != ALL_EMPIRES) {
        Logger().errorStream() << "Couldn't get empire with id " << empire_id;
        return;
    }


    for (EmpireManager::iterator from_empire_it = Empires().begin(); from_empire_it != Empires().end(); ++from_empire_it) {
        int from_empire_id = from_empire_it->first;
        if (from_empire_id == empire_id) continue;    // skip this empire; can't capture one's own ProductionItems

        Empire* from_empire = from_empire_it->second;
        ProductionQueue& queue = from_empire->m_production_queue;

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
                if (!type) {
                    Logger().errorStream() << "ConquerProductionQueueItemsAtLocation couldn't get building with name " << name;
                    continue;
                }

                CaptureResult result = type->GetCaptureResult(from_empire_id, empire_id, location_id, true);

                if (result == CR_DESTROY) {
                    // item removed from current queue, NOT added to conquerer's queue
                    queue_it = queue.erase(queue_it);

                } else if (result == CR_CAPTURE) {
                    if (to_empire) {
                        // item removed from current queue, added to conquerer's queue
                        ProductionQueue::Element build(item, elem.ordered, elem.remaining, location_id);
                        build.progress=elem.progress;
                        to_empire->m_production_queue.push_back(build);

                        queue_it = queue.erase(queue_it);
                    } else {
                        // else do nothing; no empire can't capure things
                        ++queue_it;
                    }

                } else if (result == INVALID_CAPTURE_RESULT) {
                    Logger().errorStream() << "Empire::ConquerBuildsAtLocationFromEmpire: BuildingType had an invalid CaptureResult";
                } else {
                    ++queue_it;
                }
                // otherwise do nothing: item left on current queue, conquerer gets nothing
            } else {
                ++queue_it;
            }

            // TODO: other types of build item...
        }
    }
}

void Empire::AddTech(const std::string& name) {
    const Tech* tech = GetTech(name);
    if (!tech) {
        Logger().errorStream() << "Empire::AddTech given and invalid tech: " << name;
        return;
    }

    if (m_techs.find(name) == m_techs.end())
        AddSitRepEntry(CreateTechResearchedSitRep(name));

    const std::vector<ItemSpec>& unlocked_items = tech->UnlockedItems();
    for (unsigned int i = 0; i < unlocked_items.size(); ++i)
        UnlockItem(unlocked_items[i]);  // potential infinite if a tech (in)directly unlocks itself?

    if (m_techs.find(name) == m_techs.end())
        m_techs.insert(name);
}

void Empire::UnlockItem(const ItemSpec& item) {
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
        AddShipDesign(GetPredefinedShipDesignManager().GetDesignID(item.name));
        break;
    case UIT_TECH:
        AddTech(item.name);
        break;
    default:
        Logger().errorStream() << "Empire::UnlockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::AddBuildingType(const std::string& name) {
    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type) {
        Logger().errorStream() << "Empire::AddBuildingType given an invalid building type name: " << name;
        return;
    }
    if (!building_type->Producible())
        return;
    if (m_available_building_types.find(name) != m_available_building_types.end())
        return;
    m_available_building_types.insert(name);
    AddSitRepEntry(CreateBuildingTypeUnlockedSitRep(name));
}

void Empire::AddPartType(const std::string& name) {
    const PartType* part_type = GetPartType(name);
    if (!part_type) {
        Logger().errorStream() << "Empire::AddPartType given an invalid part type name: " << name;
        return;
    }
    if (!part_type->Producible())
        return;
    m_available_part_types.insert(name);
    AddSitRepEntry(CreateShipPartUnlockedSitRep(name));
}

void Empire::AddHullType(const std::string& name) {
    const HullType* hull_type = GetHullType(name);
    if (!hull_type) {
        Logger().errorStream() << "Empire::AddHullType given an invalid hull type name: " << name;
        return;
    }
    if (!hull_type->Producible())
        return;
    m_available_hull_types.insert(name);
    AddSitRepEntry(CreateShipHullUnlockedSitRep(name));
}

void Empire::AddExploredSystem(int ID) {
    if (GetSystem(ID))
        m_explored_systems.insert(ID);
    else
        Logger().errorStream() << "Empire::AddExploredSystem given an invalid system id: " << ID;
}

std::string Empire::NewShipName() {
    static std::vector<std::string> ship_names;
    if (ship_names.empty()) {
        // load potential names from stringtable
        std::list<std::string> ship_names_list;
        UserStringList("SHIP_NAMES", ship_names_list);

        ship_names.reserve(ship_names_list.size());
        std::copy(ship_names_list.begin(), ship_names_list.end(), std::back_inserter(ship_names));
        if (ship_names.empty()) // safety check to ensure not leaving list empty in case of stringtable failure
            ship_names.push_back(UserString("SHIP"));
    }

    // select name randomly from list
    int ship_name_idx = RandSmallInt(0, static_cast<int>(ship_names.size()) - 1);
    std::string retval = ship_names[ship_name_idx];
    int times_name_used = ++m_ship_names_used[retval];
    if (1 < times_name_used)
        retval += " " + RomanNumber(times_name_used);
    return retval;
}

void Empire::AddShipDesign(int ship_design_id) {
    /* Check if design id is valid.  That is, check that it corresponds to an
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

int Empire::AddShipDesign(ShipDesign* ship_design) {
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
        return INVALID_OBJECT_ID;
    }

    m_ship_designs.insert(new_design_id);

    ShipDesignsChangedSignal();

    return new_design_id;
}

void Empire::RemoveShipDesign(int ship_design_id) {
    if (m_ship_designs.find(ship_design_id) != m_ship_designs.end()) {
        m_ship_designs.erase(ship_design_id);
        ShipDesignsChangedSignal();
    } else {
        Logger().debugStream() << "Empire::RemoveShipDesign: this empire did not have design with id " << ship_design_id;
    }
}

void Empire::AddSitRepEntry(const SitRepEntry& entry)
{ m_sitrep_entries.push_back(entry); }

void Empire::RemoveTech(const std::string& name)
{ m_techs.erase(name); }

void Empire::LockItem(const ItemSpec& item) {
    switch (item.type) {
    case UIT_BUILDING:
        RemoveBuildingType(item.name);
        break;
    case UIT_SHIP_PART:
        RemovePartType(item.name);
        break;
    case UIT_SHIP_HULL:
        RemoveHullType(item.name);
        break;
    case UIT_SHIP_DESIGN:
        RemoveShipDesign(GetPredefinedShipDesignManager().GetDesignID(item.name));
        break;
    case UIT_TECH:
        RemoveTech(item.name);
        break;
    default:
        Logger().errorStream() << "Empire::LockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::RemoveBuildingType(const std::string& name) {
    std::set<std::string>::const_iterator it = m_available_building_types.find(name);
    if (it == m_available_building_types.end())
        Logger().debugStream() << "Empire::RemoveBuildingType asked to remove building type " << name << " that was no available to this empire";
    m_available_building_types.erase(name);
}

void Empire::RemovePartType(const std::string& name) {
    std::set<std::string>::const_iterator it = m_available_part_types.find(name);
    if (it == m_available_part_types.end())
        Logger().debugStream() << "Empire::RemovePartType asked to remove part type " << name << " that was no available to this empire";
    m_available_part_types.erase(name);
}

void Empire::RemoveHullType(const std::string& name) {
    std::set<std::string>::const_iterator it = m_available_hull_types.find(name);
    if (it == m_available_hull_types.end())
        Logger().debugStream() << "Empire::RemoveHullType asked to remove hull type " << name << " that was no available to this empire";
    m_available_hull_types.erase(name);
}

void Empire::ClearSitRep()
{ m_sitrep_entries.clear(); }

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

void Empire::CheckResearchProgress() {
    SanitizeResearchQueue(m_research_queue);

    std::vector<std::string> to_erase;
    for (ResearchQueue::iterator it = m_research_queue.begin(); it != m_research_queue.end(); ++it) {
        const Tech* tech = GetTech(it->name);
        if (!tech) {
            Logger().errorStream() << "Empire::CheckResearchProgress couldn't find tech on queue, even after sanitizing!";
            continue;
        }
        float& progress = m_research_progress[it->name];
        progress += it->allocated_rp;
        if (tech->ResearchCost(m_id) - EPSILON <= progress) {
            AddTech(it->name);
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

void Empire::CheckProductionProgress() {
    Logger().debugStream() << "========Empire::CheckProductionProgress=======";
    // following commented line should be redundant, as previous call to
    // UpdateResourcePools should have generated necessary info
    // m_production_queue.Update();

    Universe& universe = GetUniverse();

    std::map<int, std::vector<TemporaryPtr<Ship> > >  system_new_ships;

    // preprocess the queue to get all the costs and times of all items
    // at every location at which they are being produced,
    // before doing any generation of new objects or other modifications
    // of the gamestate. this will ensure that the cost of items doesn't
    // change while the queue is being processed, so that if there is
    // sufficent PP to complete an object at the start of a turn,
    // items above it on the queue getting finished don't increase the
    // cost and result in it not being finished that turn.
    std::map<std::pair<ProductionQueue::ProductionItem, int>,
             std::pair<float, int> >                           queue_item_costs_and_times;
    for (ProductionQueue::iterator it = m_production_queue.begin();
         it != m_production_queue.end(); ++it)
    {
        ProductionQueue::Element& elem = *it;

        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        std::pair<ProductionQueue::ProductionItem, int> key(elem.item, location_id);

        if (queue_item_costs_and_times.find(key) == queue_item_costs_and_times.end())
            queue_item_costs_and_times[key] = ProductionCostAndTime(elem);
    }

    //for (std::map<std::pair<ProductionQueue::ProductionItem, int>, std::pair<float, int> >::const_iterator
    //     it = queue_item_costs_and_times.begin(); it != queue_item_costs_and_times.end(); ++it)
    //{ Logger().debugStream() << it->first.first.design_id << " : " << it->second.first; }


    // go through queue, updating production progress.  If a production item is
    // completed, create the produced object or take whatever other action is
    // appropriate, and record that queue item as complete, so it can be erased
    // from the queue
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        ProductionQueue::Element& elem = m_production_queue[i];
        float item_cost;
        int build_turns;

        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        std::pair<ProductionQueue::ProductionItem, int> key(elem.item, location_id);

        boost::tie(item_cost, build_turns) = queue_item_costs_and_times[key];

        item_cost *= elem.blocksize;
        elem.progress += elem.allocated_pp;   // add allocated PP to queue item
        elem.progress_memory = elem.progress;
        elem.blocksize_memory = elem.blocksize;

        // if accumulated PP is sufficient, the item is complete
        if (item_cost - EPSILON <= elem.progress) {
            // deduct cost of complete item from progress, so that next
            // repetition can continue accumulating PP, but don't set progress
            // to 0, as this way overflow PP allocated this turn can be used
            // for the next repetition of the item.
            elem.progress -= item_cost;

            elem.progress_memory = elem.progress;
            Logger().debugStream() << "Completed an item: " << elem.item.name;

            switch (elem.item.build_type) {
            case BT_BUILDING: {
                TemporaryPtr<Planet> planet = GetPlanet(elem.location);
                if (!planet) {
                    Logger().errorStream() << "Couldn't get planet with id  " << elem.location << " on which to create building";
                    break;
                }

                TemporaryPtr<System> system = GetSystem(planet->SystemID());
                if (!system) {
                    Logger().errorStream() << "Empire::CheckProductionProgress couldn't get system for producing new building";
                    break;
                }

                // check location condition before each building is created, so
                // that buildings being produced can prevent subsequent
                // buildings completions on the same turn from going through
                if (!this->ProducibleItem(elem.item, elem.location)) {
                    Logger().debugStream() << "Location test failed for building " << elem.item.name << " on planet " << planet->Name();
                    break;
                }

                // create new building
                TemporaryPtr<Building> building = universe.CreateBuilding(m_id, elem.item.name, m_id);
                planet->AddBuilding(building->ID());
                building->SetPlanetID(planet->ID());
                system->Insert(building);

                AddSitRepEntry(CreateBuildingBuiltSitRep(building->ID(), planet->ID()));
                Logger().debugStream() << "New Building created on turn: " << CurrentTurn();
                break;
            }

            case BT_SHIP: {
                if (elem.blocksize < 1)
                    break;   // nothing to do!

                TemporaryPtr<UniverseObject> build_location = GetUniverseObject(elem.location);
                if (!build_location) {
                    Logger().errorStream() << "Couldn't get build location for completed ship";
                    break;
                }
                TemporaryPtr<System> system = GetSystem(build_location->SystemID());
                // TODO: account for shipyards and/or other ship production
                // sites that are in interstellar space, if needed
                if (!system) {
                    Logger().errorStream() << "Empire::CheckProductionProgress couldn't get system for producing new ship";
                    break;
                }

                // check location condition before each ship is created, so
                // that ships being produced can prevent subsequent
                // ship completions on the same turn from going through
                if (!this->ProducibleItem(elem.item, elem.location))
                    break;

                // get species for this ship.  use popcenter species if build
                // location is a popcenter, or use ship species if build
                // location is a ship, or use empire capital species if there
                // is a valid capital, or otherwise ???
                // TODO: Add more fallbacks if necessary
                std::string species_name;
                if (TemporaryPtr<const PopCenter> location_pop_center = boost::dynamic_pointer_cast<const PopCenter>(build_location))
                    species_name = location_pop_center->SpeciesName();
                else if (TemporaryPtr<const Ship> location_ship = universe_object_ptr_cast<const Ship>(build_location))
                    species_name = location_ship->SpeciesName();
                else if (TemporaryPtr<const Planet> capital_planet = GetPlanet(this->CapitalID()))
                    species_name = capital_planet->SpeciesName();
                // else give up...
                if (species_name.empty()) {
                    // only really a problem for colony ships, which need to have a species to function
                    const ShipDesign* design = GetShipDesign(elem.item.design_id);
                    if (!design) {
                        Logger().errorStream() << "Couldn't get ShipDesign with id: " << elem.item.design_id;
                        break;
                    }
                    if (design->CanColonize()) {
                        Logger().errorStream() << "Couldn't get species in order to make colony ship!";
                        break;
                    }
                }

                TemporaryPtr<Ship> ship;

                for (int count = 0; count < elem.blocksize; count++) {
                    // create ship
                    ship = universe.CreateShip(m_id, elem.item.design_id, species_name, m_id);
                    system->Insert(ship);

                    // set active meters that have associated max meters to an
                    // initial very large value, so that when the active meters are
                    // later clamped, they will equal the max meter after effects
                    // have been applied, letting new ships start with maxed
                    // everything that is traced with an associated max meter.
                    ship->UniverseObject::GetMeter(METER_FUEL)->SetCurrent(Meter::LARGE_VALUE);
                    ship->UniverseObject::GetMeter(METER_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
                    ship->UniverseObject::GetMeter(METER_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);
                    ship->BackPropegateMeters();

                    ship->Rename(NewShipName());

                    // store ships to put into fleets later
                    system_new_ships[system->ID()].push_back(ship);
                }
                // add sitrep
                if (elem.blocksize == 1) {
                    AddSitRepEntry(CreateShipBuiltSitRep(ship->ID(), system->ID(), ship->DesignID()));
                    Logger().debugStream() << "New Ship, id " << ship->ID() << ", created on turn: " << ship->CreationTurn();
                } else {
                    AddSitRepEntry(CreateShipBlockBuiltSitRep(system->ID(), ship->DesignID(), elem.blocksize));
                    Logger().debugStream() << "New block of "<< elem.blocksize << " ships created on turn: " << ship->CreationTurn();
                }
                break;
            }

            default:
                Logger().debugStream() << "Build item of unknown build type finished on production queue.";
                break;
            }

            if (!--m_production_queue[i].remaining) {   // decrement number of remaining items to be produced in current queue element
                to_erase.push_back(i);                  // remember completed element so that it can be removed from queue
                Logger().debugStream() << "Marking completed production queue item to be removed form queue";
            }
        }
    }

    // create fleets for new ships and put ships into fleets
    for (std::map<int, std::vector<TemporaryPtr<Ship> > >::iterator it = system_new_ships.begin();
         it != system_new_ships.end(); ++it)
    {
        TemporaryPtr<System> system = GetSystem(it->first);
        if (!system) {
            Logger().errorStream() << "Couldn't get system with id " << it->first << " for creating new fleets for newly produced ships";
            continue;
        }

        std::vector<TemporaryPtr<Ship> >& allShips = it->second;
        if (allShips.empty())
            continue;

        // group ships into fleets, by design
        std::map<int,std::vector<TemporaryPtr<Ship> > > shipsByDesign;
        for (std::vector<TemporaryPtr<Ship> >::iterator it = allShips.begin(); it != allShips.end(); ++it) {
            TemporaryPtr<Ship> ship = *it;
            shipsByDesign[ship->DesignID()].push_back(ship);
        }

        for (std::map<int, std::vector<TemporaryPtr<Ship> > >::iterator design_it = shipsByDesign.begin();
             design_it != shipsByDesign.end(); ++design_it)
        {
            std::vector<int> ship_ids;

            std::vector<TemporaryPtr<Ship> >& ships = design_it->second;
            if (ships.empty())
                continue;

            // create new fleet for ships
            TemporaryPtr<Fleet> fleet = universe.CreateFleet("", system->X(), system->Y(), m_id);

            system->Insert(fleet);

            for (std::vector<TemporaryPtr<Ship> >::iterator it = ships.begin(); it != ships.end(); ++it) {
                TemporaryPtr<Ship> ship = *it;
                ship_ids.push_back(ship->ID());
                fleet->AddShip(ship->ID());
                ship->SetFleetID(fleet->ID());
            }

            // rename fleet, given its id and the ship that is in it
            fleet->Rename(Fleet::GenerateFleetName(ship_ids, fleet->ID()));

            Logger().debugStream() << "New Fleet \"" + fleet->Name() + "\" created on turn: " << fleet->CreationTurn();
        }
    }

    // removed completed items from queue
    for (std::vector<int>::reverse_iterator it = to_erase.rbegin(); it != to_erase.rend(); ++it)
        m_production_queue.erase(*it);
}

void Empire::CheckTradeSocialProgress()
{ m_resource_pools[RE_TRADE]->SetStockpile(m_resource_pools[RE_TRADE]->TotalAvailable()); }

void Empire::SetColor(const GG::Clr& color)
{ m_color = color; }

void Empire::SetName(const std::string& name)
{ m_name = name; }

void Empire::SetPlayerName(const std::string& player_name)
{ m_player_name = player_name; }

void Empire::InitResourcePools() {
    // get this empire's owned resource and population centres
    std::vector<int> res_centers;
    res_centers.reserve(Objects().NumExistingResourceCenters());
    for (std::map<int, TemporaryPtr<UniverseObject> >::iterator it = Objects().ExistingResourceCentersBegin();
         it != Objects().ExistingResourceCentersEnd(); ++it)
    {
        if (it->second->OwnedBy(m_id))
            res_centers.push_back(it->first);
    }

    std::vector<int> pop_centers;
    pop_centers.reserve(Objects().NumExistingPopCenters());
    for (std::map<int, TemporaryPtr<UniverseObject> >::iterator it = Objects().ExistingPopCentersBegin();
         it != Objects().ExistingPopCentersEnd(); ++it)
    {
        if (it->second->OwnedBy(m_id))
            pop_centers.push_back(it->first);
    }

    m_population_pool.SetPopCenters(pop_centers);

    // determine if each object owned by this empire is a ResourceCenter, and
    // store ids of ResourceCenters and objects in appropriate vectors
    m_resource_pools[RE_RESEARCH]->SetObjects(res_centers);
    m_resource_pools[RE_INDUSTRY]->SetObjects(res_centers);
    m_resource_pools[RE_TRADE]->SetObjects(res_centers);


    // inform the blockadeable resource pools about systems that can share
    m_resource_pools[RE_INDUSTRY]->SetConnectedSupplyGroups(m_resource_supply_groups);


    // set non-blockadeable resource pools to share resources between all systems
    std::set<std::set<int> > sets_set;
    std::set<int> all_systems_set;
    for (std::map<int, TemporaryPtr<UniverseObject> >::iterator it = Objects().ExistingSystemsBegin();
         it != Objects().ExistingSystemsEnd(); ++it)
    {
        all_systems_set.insert(it->first);
    }
    sets_set.insert(all_systems_set);
    m_resource_pools[RE_RESEARCH]->SetConnectedSupplyGroups(sets_set);
    m_resource_pools[RE_TRADE]->SetConnectedSupplyGroups(sets_set);


    // set stockpile object locations for each resource, ensuring those systems exist
    std::vector<ResourceType> res_type_vec;
    res_type_vec.push_back(RE_INDUSTRY);
    res_type_vec.push_back(RE_TRADE);
    res_type_vec.push_back(RE_RESEARCH);

    for (std::vector<ResourceType>::const_iterator res_it = res_type_vec.begin();
         res_it != res_type_vec.end(); ++res_it)
    {
        ResourceType res_type = *res_it;
        int stockpile_object_id = INVALID_OBJECT_ID;
        if (TemporaryPtr<const UniverseObject> stockpile_obj = GetUniverseObject(StockpileID(res_type)))
            stockpile_object_id = stockpile_obj->ID();
        m_resource_pools[res_type]->SetStockpileObject(stockpile_object_id);
    }
}

void Empire::UpdateResourcePools() {
    // updating queues, allocated_rp, distribution and growth each update their respective pools,
    // (as well as the ways in which the resources are used, which needs to be done
    // simultaneously to keep things consistent)
    UpdateResearchQueue();
    UpdateProductionQueue();
    UpdateTradeSpending();
    UpdatePopulationGrowth();
}

void Empire::UpdateResearchQueue() {
    m_resource_pools[RE_RESEARCH]->Update();
    m_research_queue.Update(m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
    m_resource_pools[RE_RESEARCH]->ChangedSignal();
}

void Empire::UpdateProductionQueue() {
    Logger().debugStream() << "========= Production Update for empire: " << EmpireID() << " ========";

    m_resource_pools[RE_INDUSTRY]->Update();
    m_production_queue.Update();
    m_resource_pools[RE_INDUSTRY]->ChangedSignal();
}

void Empire::UpdateTradeSpending() {
    m_resource_pools[RE_TRADE]->Update(); // recalculate total trade production

    //std::vector<TemporaryPtr<UniverseObject> > buildings = GetUniverse().Objects().FindObjects(OwnedVisitor<Building>(m_id));
    //for (std::vector<TemporaryPtr<UniverseObject> >::const_iterator it = buildings.begin(); it != buildings.end(); ++it) {
    //    TemporaryPtr<Building> building = universe_object_ptr_cast<Building>(*it);
    //    if (!building)
    //        continue;
    //}
    m_resource_pools[RE_TRADE]->ChangedSignal();
}

void Empire::UpdatePopulationGrowth()
{ m_population_pool.Update(); }

void Empire::ResetMeters() {
    for (std::map<std::string, Meter>::iterator it = m_meters.begin(); it != m_meters.end(); ++it) {
        it->second.ResetCurrent();
    }
}
