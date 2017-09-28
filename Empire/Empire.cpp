#include "Empire.h"

#include "../parse/Parse.h"
#include "../util/i18n.h"
#include "../util/MultiplayerCommon.h"
#include "../util/ScopedTimer.h"
#include "../util/Random.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/SitRepEntry.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../universe/Enums.h"
#include "../universe/UniverseObject.h"
#include "../universe/ValueRef.h"
#include "ResourcePool.h"
#include "EmpireManager.h"
#include "Supply.h"

#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <algorithm>
#include <iterator>
#include <unordered_set>


namespace {
    const float EPSILON = 0.01f;
    const std::string EMPTY_STRING;

    float GetQueueFrontloadFactor() {
        static float front_load_factor = -1.0;
        if (front_load_factor == -1.0) {
            front_load_factor = 0.0;
            try {
                if (UserStringExists("FUNCTIONAL_PRODUCTION_QUEUE_FRONTLOAD_FACTOR")) {
                    float new_front_factor = std::atof(UserString("FUNCTIONAL_PRODUCTION_QUEUE_FRONTLOAD_FACTOR").c_str());
                    if (new_front_factor > 0.0f && new_front_factor <= 0.3f)
                        front_load_factor = new_front_factor;
                }
            } catch (...) {}
        }
        return front_load_factor;
    }

    float GetQueueToppingFactor() {
        static float topping_up_factor = -1.0;
        if (topping_up_factor == -1.0) {
            topping_up_factor = 0.0;
            try {
                if (UserStringExists("FUNCTIONAL_PRODUCTION_QUEUE_FRONTLOAD_FACTOR")) {
                    float new_front_factor = std::atof(UserString("FUNCTIONAL_PRODUCTION_QUEUE_FRONTLOAD_FACTOR").c_str());
                    if (new_front_factor > 0.0f && new_front_factor <= 0.3f)
                        topping_up_factor = new_front_factor;
                }
            } catch (...) {}
        }
        return topping_up_factor;
    }

// FUNCTIONAL_PRODUCTION_QUEUE_FRONTLOAD_FACTOR and FUNCTIONAL_PRODUCTION_QUEUE_TOPPING_UP_FACTOR specify global_settings.txt values that affect how the ProductionQueue will limit
// allocation towards building a given item on a given turn.  The base amount of maximum allocation per turn (if the player has enough PP available) is the item's total
// cost, divided over its minimum build time.  Sometimes complications arise, though, which unexpectedly delay the completion even if the item had been fully-funded 
// every turn, because costs have risen partway through (such as due to increasing ship costs resulting from recent ship constructoin completion and ensuing increase 
// of Fleet Maintenance costs.  These two settings provide a mechanism for some allocation leeway to deal with mid-build cost increases without causing the project 
// completion to take an extra turn because of the small bit of increased cost.  The settings differ in the timing of the extra allocation allowed.
// Both factors have a minimum value of 0.0 and a maximum value of 0.3.

// Making the frontloaded factor greater than zero increases the per-turn allocation cap by the specified percentage (so it always spreads the extra allocation across all turns).
// Making the topping-up option nonzero allows the final turn allocation cap to be increased by the specified percentage of the total cost, if needed (and then subject to 
// availability of course). They can both be nonzero, although to avoid that introducing too much interaction complexity into the minimum build time safeguard for topping-up, 
// the topping-up percentage will be reduced by the frontloading setting. 

// Note that for very small values of the options (less than 5%), when dealing with very low cost items the effect/protection may be noticeably less than expected because of
// interactions with the ProductionQueue Epsilon value (0.01)

    float CalculateProductionPerTurnLimit(const ProductionQueue::Element& queue_element, float item_cost, int build_turns) {
        const float frontload_limit_factor = GetQueueFrontloadFactor();
        // any allowed topping up is limited by how much frontloading was allowed
        const float topping_up_limit_factor = std::max(0.0f, GetQueueToppingFactor() - frontload_limit_factor);

        item_cost *= queue_element.blocksize;
        build_turns = std::max(build_turns, 1);
        float element_accumulated_PP = queue_element.progress*item_cost;                        // effective PP accumulated by this element towards producing next item. progress is a fraction between 0 and 1.
        float element_total_cost = item_cost * queue_element.remaining;                         // total PP to produce all items in this element
        float additional_pp_to_complete_element = element_total_cost - element_accumulated_PP;  // additional PP, beyond already-accumulated PP, to produce all items in this element
        float additional_pp_to_complete_item = item_cost - element_accumulated_PP;              // additional PP, beyond already-accumulated PP, to produce the current item of this element
        float basic_element_per_turn_limit = item_cost / build_turns;
        // the extra constraints on frontload and topping up amounts ensure that won't let complete in less than build_turns (so long as costs do not decrease)
        float frontload = (1.0 + frontload_limit_factor/std::max(build_turns-1,1)) * basic_element_per_turn_limit  - 2 * EPSILON;
        float topping_up_limit = basic_element_per_turn_limit + std::min(topping_up_limit_factor * item_cost, basic_element_per_turn_limit - 2 * EPSILON);
        float topping_up = (additional_pp_to_complete_item < topping_up_limit) ? additional_pp_to_complete_item : basic_element_per_turn_limit;
        float retval = std::min(additional_pp_to_complete_element, std::max(basic_element_per_turn_limit, std::max(frontload, topping_up)));
        //DebugLogger() << "CalculateProductionPerTurnLimit for item " << queue_element.item.build_type << " " << queue_element.item.name 
        //              << " " << queue_element.item.design_id << " :  accumPP: " << element_accumulated_PP << " pp_to_complete_elem: "
        //              << additional_pp_to_complete_element << " pp_to_complete_item: " << additional_pp_to_complete_item 
        //              <<  " basic_element_per_turn_limit: " << basic_element_per_turn_limit << " frontload: " << frontload
        //              << " topping_up_limit: " << topping_up_limit << " topping_up: " << topping_up << " retval: " << retval;

        return retval;
    }


    /** sets the .allocated_rp, value for each Tech in the queue.  Only sets
      * nonzero funding to a Tech if it is researchable this turn.  Also
      * determines total number of spent RP (returning by reference in
      * total_RPs_spent) */
    void SetTechQueueElementSpending(float RPs, const std::map<std::string, float>& research_progress,
                                     const std::map<std::string, TechStatus>& research_status,
                                     ResearchQueue::QueueType& queue,
                                     float& total_RPs_spent, int& projects_in_progress, int empire_id)
    {
        total_RPs_spent = 0.0f;
        projects_in_progress = 0;

        for (ResearchQueue::Element& elem : queue) {
            elem.allocated_rp = 0.0f;    // default, may be modified below

            if (elem.paused) {
                continue;
            }

            // get details on what is being researched...
            const Tech* tech = GetTech(elem.name);
            if (!tech) {
                ErrorLogger() << "SetTechQueueElementSpending found null tech on research queue?!";
                continue;
            }
            auto status_it = research_status.find(elem.name);
            if (status_it == research_status.end()) {
                ErrorLogger() << "SetTechQueueElementSpending couldn't find tech with name " << elem.name << " in the research status map";
                continue;
            }
            bool researchable = false;
            if (status_it->second == TS_RESEARCHABLE)
                researchable = true;

            if (researchable && !elem.paused) {
                auto progress_it = research_progress.find(elem.name);
                float tech_cost = tech->ResearchCost(empire_id);
                float progress = progress_it == research_progress.end() ? 0.0f : progress_it->second;
                float RPs_needed = tech->ResearchCost(empire_id) - progress*tech_cost;
                float RPs_per_turn_limit = tech->PerTurnCost(empire_id);
                float RPs_to_spend = std::min(RPs_needed, RPs_per_turn_limit);

                if (total_RPs_spent + RPs_to_spend <= RPs - EPSILON) {
                    elem.allocated_rp = RPs_to_spend;
                    total_RPs_spent += elem.allocated_rp;
                    ++projects_in_progress;
                } else if (total_RPs_spent < RPs - EPSILON) {
                    elem.allocated_rp = RPs - total_RPs_spent;
                    total_RPs_spent += elem.allocated_rp;
                    ++projects_in_progress;
                } else {
                    elem.allocated_rp = 0.0f;
                }
            } else {
                // item can't be researched this turn
                elem.allocated_rp = 0.0f;
            }
        }

        DebugLogger() << "SetTechQueueElementSpending allocated: " << total_RPs_spent << " of " << RPs << " available";
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
                                     const std::vector<std::set<int>>& queue_element_resource_sharing_object_groups,
                                     const std::map<std::pair<ProductionQueue::ProductionItem, int>, std::pair<float, int>>& queue_item_costs_and_times,
                                     const std::vector<bool>& is_producible,
                                     ProductionQueue::QueueType& queue,
                                     std::map<std::set<int>, float>& allocated_pp,
                                     int& projects_in_progress)
    {
        //DebugLogger() << "========SetProdQueueElementSpending========";
        //DebugLogger() << "production status: ";
        //DebugLogger() << "queue: ";
        //for (const ProductionQueue::Element& elem : queue)
        //    DebugLogger() << " ... name: " << elem.item.name << "id: " << elem.item.design_id << " allocated: " << elem.allocated_pp << " locationid: " << elem.location << " ordered: " << elem.ordered;

        if (queue.size() != queue_element_resource_sharing_object_groups.size()) {
            ErrorLogger() << "SetProdQueueElementSpending queue size and sharing groups size inconsistent. aborting";
            return;
        }

        // See explanation at CalculateProductionPerTurnLimit() above regarding operation of these factors.
        // any allowed topping up is limited by how much frontloading was allowed
        //const float frontload_limit_factor = GetQueueFrontloadFactor();
        //const float topping_up_limit_factor = std::max(0.0f, GetQueueToppingFactor() - frontload_limit_factor);
        // DebugLogger() << "SetProdQueueElementSpending frontload  factor " << frontload_limit_factor;
        // DebugLogger() << "SetProdQueueElementSpending topping up factor " << topping_up_limit_factor;

        projects_in_progress = 0;
        allocated_pp.clear();

        //DebugLogger() << "queue size: " << queue.size();

        int i = 0;
        for (ProductionQueue::Element& queue_element : queue) {
            if (queue_element.paused) {
                queue_element.allocated_pp = 0.0f;
                ++i;
                continue;
            }

            // get resource sharing group and amount of resource available to build this item
            const std::set<int>& group = queue_element_resource_sharing_object_groups[i];
            if (group.empty()) {
                //DebugLogger() << "resource sharing group for queue element is empty.  not allocating any resources to element";
                queue_element.allocated_pp = 0.0f;
                ++i;
                continue;
            }

            std::map<std::set<int>, float>::iterator available_pp_it = available_pp.find(group);
            if (available_pp_it == available_pp.end()) {
                // item is not being built at an object that has access to resources, so it can't be produced.
                //DebugLogger() << "no resource sharing group for production queue element";
                queue_element.allocated_pp = 0.0f;
                ++i;
                continue;
            }

            float& group_pp_available = available_pp_it->second;


            // if group has no pp available, can't produce anything this turn
            if (group_pp_available <= 0.0f) {
                //DebugLogger() << "no pp available in group";
                queue_element.allocated_pp = 0.0f;
                ++i;
                continue;
            }
            //DebugLogger() << "group has " << group_pp_available << " PP available";

            // see if item is producible this turn...
            if (!is_producible[i]) {
                // can't be produced at this location this turn.
                queue_element.allocated_pp = 0.0f;
                //DebugLogger() << "item can't be produced at location this turn";
                ++i;
                continue;
            }


            // get max contribution per turn and turns to build at max contribution rate
            int location_id = (queue_element.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : queue_element.location);
            std::pair<ProductionQueue::ProductionItem, int> key(queue_element.item, location_id);
            float item_cost = 1e6;  // dummy/default value, shouldn't ever really be needed
            int build_turns = 1;    // dummy/default value, shouldn't ever really be needed
            auto time_cost_it = queue_item_costs_and_times.find(key);
            if (time_cost_it != queue_item_costs_and_times.end()) {
                item_cost = time_cost_it->second.first;
                build_turns = time_cost_it->second.second;
            } else {
                ErrorLogger() << "item " << queue_element.item.name 
                              << " somehow failed time cost lookup for location " << location_id;
            }
            //DebugLogger() << "item " << queue_element.item.name << " costs " << item_cost << " for " << build_turns << " turns";

            float element_this_turn_limit = CalculateProductionPerTurnLimit(queue_element, item_cost, build_turns);

            // determine how many pp to allocate to this queue element this turn.  allocation is limited by the
            // item cost, which is the max number of PP per turn that can be put towards this item, and by the
            // total cost remaining to complete the last item in the queue element (eg. the element has all but
            // the last item complete already) and by the total pp available in this element's production location's
            // resource sharing group
            float allocation = std::max(0.0f, std::min(element_this_turn_limit, group_pp_available));

            //DebugLogger() << "element accumulated " << element_accumulated_PP << " of total cost "
            //                       << element_total_cost << " and needs " << additional_pp_to_complete_element
            //                       << " more to be completed";
            //DebugLogger() << "... allocating " << allocation;

            // allocate pp
            queue_element.allocated_pp = std::max(allocation, EPSILON);

            // record alloation in group, if group is not empty
            allocated_pp[group] += allocation;  // assuming the float indexed by group will be default initialized to 0.0f if that entry doesn't already exist in the map
            group_pp_available -= allocation;
            group_pp_available = std::max(group_pp_available, 0.0f);    // safety clamp

            //DebugLogger() << "... leaving " << group_pp_available << " PP available to group";

            if (allocation > 0.0f)
                ++projects_in_progress;

            ++i;
        }
    }
}

////////////////////////////////////////
// ResearchQueue                      //
////////////////////////////////////////
std::string ResearchQueue::Element::Dump() const {
    std::stringstream retval;
    retval << "ResearchQueue::Element: tech: " << name << "  empire id: " << empire_id;
    retval << "  allocated: " << allocated_rp << "  turns left: " << turns_left;
    if (paused)
        retval << "  (paused)";
    retval << "\n";
    return retval.str();
}

bool ResearchQueue::InQueue(const std::string& tech_name) const
{ return find(tech_name) != end(); }

bool ResearchQueue::Paused(const std::string& tech_name) const {
    auto it = find(tech_name);
    if (it == end())
        return false;
    return it->paused;
}

bool ResearchQueue::Paused(int idx) const {
    if (idx >= static_cast<int>(m_queue.size()))
        return false;
    return std::next(begin(), idx)->paused;
}

int ResearchQueue::ProjectsInProgress() const
{ return m_projects_in_progress; }

float ResearchQueue::TotalRPsSpent() const
{ return m_total_RPs_spent; }

std::vector<std::string> ResearchQueue::AllEnqueuedProjects() const {
    std::vector<std::string> retval;
    for (const auto& entry : m_queue)
        retval.push_back(entry.name);
    return retval;
}

std::string ResearchQueue::Dump() const {
    std::stringstream retval;
    retval << "ResearchQueue:\n";
    float spent_rp{0.0f};
    for (const auto& entry : m_queue) {
        retval << " ... " << entry.Dump();
        spent_rp += entry.allocated_rp;
    }
    retval << "ResearchQueue Total Spent RP: " << spent_rp;
    return retval.str();
}

bool ResearchQueue::empty() const
{ return !m_queue.size(); }

unsigned int ResearchQueue::size() const
{ return m_queue.size(); }

ResearchQueue::const_iterator ResearchQueue::begin() const
{ return m_queue.begin(); }

ResearchQueue::const_iterator ResearchQueue::end() const
{ return m_queue.end(); }

ResearchQueue::const_iterator ResearchQueue::find(const std::string& tech_name) const {
    for (auto it = begin(); it != end(); ++it) {
        if (it->name == tech_name)
            return it;
    }
    return end();
}

const ResearchQueue::Element& ResearchQueue::operator[](int i) const {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

void ResearchQueue::Update(float RPs, const std::map<std::string, float>& research_progress) {
    // status of all techs for this empire
    const Empire* empire = GetEmpire(m_empire_id);
    if (!empire)
        return;

    std::map<std::string, TechStatus> sim_tech_status_map;
    for (const Tech* tech : GetTechManager()) {
        const std::string& tech_name = tech->Name();
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
    for (Element& element : m_queue)
        element.turns_left = -1;

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
    std::map< std::string, int > orig_queue_order;
    std::map<int, float> dpsim_research_progress;
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        std::string tname = m_queue[i].name;
        orig_queue_order[tname] = i;
        dpsim_research_progress[i] = dp_prog[tname];
    }

    std::map<std::string, TechStatus> dpsim_tech_status_map = sim_tech_status_map;

    // initialize simulation_results with -1 for all techs, so that any techs that aren't
    // finished in simulation by turn TOO_MANY_TURNS will be left marked as never to be finished
    std::vector<int>  dpsimulation_results(m_queue.size(), -1);

    const int DP_TURNS = TOO_MANY_TURNS; // track up to this many turns

    std::map<std::string, std::set<std::string>> waiting_for_prereqs;
    std::set<int> dp_researchable_techs;

    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        std::string techname = m_queue[i].name;
        if (m_queue[i].paused)
            continue;
        const Tech* tech = GetTech(techname);
        if (!tech)
            continue;
        if (dpsim_tech_status_map[techname] == TS_RESEARCHABLE) {
            dp_researchable_techs.insert(i);
        } else if (dpsim_tech_status_map[techname] == TS_UNRESEARCHABLE ||
                   dpsim_tech_status_map[techname] == TS_HAS_RESEARCHED_PREREQ)
        {
            std::set<std::string> these_prereqs = tech->Prerequisites();
            for (std::set<std::string>::iterator ptech_it = these_prereqs.begin(); ptech_it != these_prereqs.end();) {
                if (dpsim_tech_status_map[*ptech_it] != TS_COMPLETE) {
                    ++ptech_it;
                } else {
                    std::set<std::string>::iterator erase_it = ptech_it;
                    ++ptech_it;
                    these_prereqs.erase(erase_it);
                }
            }
            waiting_for_prereqs[techname] = these_prereqs;
        }
    }

    int dp_turns = 0;
    //pp_still_available[turn-1] gives the RP still available in this resource pool at turn "turn"
    std::vector<float> rp_still_available(DP_TURNS, RPs);  // initialize to the  full RP allocation for every turn

    while ((dp_turns < DP_TURNS) && !(dp_researchable_techs.empty())) {// if we haven't used up our turns and still have techs to process
        ++dp_turns;
        std::map<int, bool> already_processed;
        for (int tech_id : dp_researchable_techs) {
            already_processed[tech_id] = false;
        }
        std::set<int>::iterator cur_tech_it = dp_researchable_techs.begin();
        while ((rp_still_available[dp_turns-1] > EPSILON)) { // try to use up this turns RPs
            if (cur_tech_it == dp_researchable_techs.end()) {
                break; //will be wasting some RP this turn
            }
            int cur_tech = *cur_tech_it;
            if (already_processed[cur_tech]) {
                ++cur_tech_it;
                continue;
            }
            already_processed[cur_tech] = true;
            const std::string& tech_name = m_queue[cur_tech].name;
            const Tech* tech = GetTech(tech_name);
            float progress = dpsim_research_progress[cur_tech];
            float tech_cost = tech->ResearchCost(m_empire_id);
            float RPs_needed = tech ? tech_cost * (1.0f - std::min(progress, 1.0f)) : 0.0f;
            float RPs_per_turn_limit = tech ? tech->PerTurnCost(m_empire_id) : 1.0f;
            float RPs_to_spend = std::min(std::min(RPs_needed, RPs_per_turn_limit), rp_still_available[dp_turns-1]);
            progress += RPs_to_spend / std::max(EPSILON, tech_cost);
            dpsim_research_progress[cur_tech] = progress;
            rp_still_available[dp_turns-1] -= RPs_to_spend;
            std::set<int>::iterator next_res_tech_it = cur_tech_it;
            int next_res_tech_idx;
            if (++next_res_tech_it == dp_researchable_techs.end()) {
                next_res_tech_idx = m_queue.size()+1;
            } else {
                next_res_tech_idx = *(next_res_tech_it);
            }

            if (tech_cost - EPSILON <= progress * tech_cost) {
                dpsim_tech_status_map[tech_name] = TS_COMPLETE;
                dpsimulation_results[cur_tech] = dp_turns;
#ifndef ORIG_RES_SIMULATOR
                m_queue[cur_tech].turns_left = dp_turns;
#endif
                dp_researchable_techs.erase(cur_tech_it);
                std::set<std::string> unlocked_techs;
                if (tech)
                    unlocked_techs = tech->UnlockedTechs();
                for (std::string u_tech_name : unlocked_techs) {
                    std::map<std::string,std::set<std::string>>::iterator prereq_tech_it = waiting_for_prereqs.find(u_tech_name);
                    if (prereq_tech_it != waiting_for_prereqs.end() ){
                        std::set<std::string> &these_prereqs = prereq_tech_it->second;
                        std::set<std::string>::iterator just_finished_it = these_prereqs.find(tech_name);
                        if (just_finished_it != these_prereqs.end() ) {  //should always find it
                            these_prereqs.erase(just_finished_it);
                            if (these_prereqs.empty()) { // tech now fully unlocked
                                int this_tech_idx = orig_queue_order[u_tech_name];
                                dp_researchable_techs.insert(this_tech_idx);
                                waiting_for_prereqs.erase(prereq_tech_it);
                                already_processed[this_tech_idx] = true;    //doesn't get any allocation on current turn
                                if (this_tech_idx < next_res_tech_idx ) {
                                    next_res_tech_idx = this_tech_idx;
                                }
                            }
                        } else { //couldnt find tech_name in prereqs list
                            DebugLogger() << "ResearchQueue::Update tech unlocking problem:"<< tech_name << "thought it was a prereq for " << u_tech_name << "but the latter disagreed";
                        }
                    } //else { //tech_name thinks itself a prereq for ytechName, but u_tech_name not in prereqs -- not a problem so long as u_tech_name not in our queue at all
                      //  DebugLogger() << "ResearchQueue::Update tech unlocking problem:"<< tech_name << "thought it was a prereq for " << u_tech_name << "but the latter disagreed";
                      //}
                }
            }// if (tech->ResearchCost() - EPSILON <= progress * tech_cost)
            cur_tech_it = dp_researchable_techs.find(next_res_tech_idx);
        }//while ((rp_still_available[dp_turns-1]> EPSILON))
        //dp_time = dpsim_queue_timer.elapsed() * 1000;
        // DebugLogger() << "ProductionQueue::Update queue dynamic programming sim time: " << dpsim_queue_timer.elapsed() * 1000.0;
    } // while ((dp_turns < DP_TURNS ) && !(dp_researchable_techs.empty() ) )

    ResearchQueueChangedSignal();
}

void ResearchQueue::push_back(const std::string& tech_name, bool paused)
{ m_queue.push_back(Element(tech_name, m_empire_id, 0.0f, -1, paused)); }

void ResearchQueue::insert(iterator it, const std::string& tech_name, bool paused)
{ m_queue.insert(it, Element(tech_name, m_empire_id, 0.0f, -1, paused)); }

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

void ResearchQueue::clear() {
    m_queue.clear();
    m_projects_in_progress = 0;
    m_total_RPs_spent = 0.0f;
    ResearchQueueChangedSignal();
}

/////////////////////////////////////
// ProductionQueue::ProductionItem //
/////////////////////////////////////
ProductionQueue::ProductionItem::ProductionItem() :
    build_type(INVALID_BUILD_TYPE)
{}

ProductionQueue::ProductionItem::ProductionItem(BuildType build_type_, std::string name_) :
    build_type(build_type_),
    name(name_)
{}

ProductionQueue::ProductionItem::ProductionItem(BuildType build_type_, int design_id_) :
    build_type(build_type_),
    design_id(design_id_)
{
    if (build_type == BT_SHIP) {
        if (const ShipDesign* ship_design = GetShipDesign(design_id))
            name = ship_design->Name();
        else
            ErrorLogger() << "ProductionItem::ProductionItem couldn't get ship design with id: " << design_id;
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

bool ProductionQueue::ProductionItem::EnqueueConditionPassedAt(int location_id) const {
    switch (build_type) {
    case BT_BUILDING: {
        if (const BuildingType* bt = GetBuildingType(name)) {
            auto location_obj = GetUniverseObject(location_id);
            const Condition::ConditionBase* c = bt->EnqueueLocation();
            if (!c)
                return true;
            return c->Eval(location_obj);
        }
        return true;
        break;
    }
    case BT_SHIP:   // ships don't have enqueue location conditions
    default:
        return true;
    }
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

std::map<std::string, std::map<int, float>>
ProductionQueue::ProductionItem::CompletionSpecialConsumption(int location_id) const {
    std::map<std::string, std::map<int, float>> retval;

    switch (build_type) {
    case BT_BUILDING: {
        if (const BuildingType* bt = GetBuildingType(name)) {
            auto location_obj = GetUniverseObject(location_id);
            ScriptingContext context(location_obj);

            for (const auto& psc : bt->ProductionSpecialConsumption()) {
                if (!psc.second.first)
                    continue;
                Condition::ObjectSet matches;
                // if a condition selectin gwhere to take resources from was specified, use it.
                // Otherwise take from the production location
                if (psc.second.second) {
                    psc.second.second->Eval(context, matches);
                } else {
                    matches.push_back(location_obj);
                }

                // determine how much to take from each matched object
                for (auto& object : matches) {
                    context.effect_target = std::const_pointer_cast<UniverseObject>(object);
                    retval[psc.first][object->ID()] += psc.second.first->Eval(context);
                }
            }
        }
        break;
    }
    case BT_SHIP: {
        if (const ShipDesign* sd = GetShipDesign(design_id)) {
            auto location_obj = GetUniverseObject(location_id);
            ScriptingContext context(location_obj);

            if (const HullType* ht = GetHullType(sd->Hull())) {
                for (const auto& psc : ht->ProductionSpecialConsumption()) {
                    if (!psc.second.first)
                        continue;
                    retval[psc.first][location_id] += psc.second.first->Eval(context);
                }
            }

            for (const std::string& part_name : sd->Parts()) {
                const PartType* pt = GetPartType(part_name);
                if (!pt)
                    continue;
                for (const auto& psc : pt->ProductionSpecialConsumption()) {
                    if (!psc.second.first)
                        continue;
                    retval[psc.first][location_id] += psc.second.first->Eval(context);
                }
            }
        }
        break;
    }
    case BT_PROJECT:    // TODO
    default:
        break;
    }

    return retval;
}

std::map<MeterType, std::map<int, float>>
ProductionQueue::ProductionItem::CompletionMeterConsumption(int location_id) const
{
    std::map<MeterType, std::map<int, float>> retval;

    switch (build_type) {
    case BT_BUILDING: {
        if (const BuildingType* bt = GetBuildingType(name)) {
            auto obj = GetUniverseObject(location_id);
            ScriptingContext context(obj);

            for (const auto& pmc : bt->ProductionMeterConsumption()) {
                if (!pmc.second.first)
                    continue;
                retval[pmc.first][location_id] = pmc.second.first->Eval(context);
            }
        }
        break;
    }
    case BT_SHIP: {
        if (const ShipDesign* sd = GetShipDesign(design_id)) {
            auto obj = GetUniverseObject(location_id);
            ScriptingContext context(obj);

            if (const HullType* ht = GetHullType(sd->Hull())) {
                for (const auto& pmc : ht->ProductionMeterConsumption()) {
                    if (!pmc.second.first)
                        continue;
                    retval[pmc.first][location_id] += pmc.second.first->Eval(context);
                }
            }

            for (const std::string& part_name : sd->Parts()) {
                const PartType* pt = GetPartType(part_name);
                if (!pt)
                    continue;
                for (const auto& pmc : pt->ProductionMeterConsumption()) {
                    if (!pmc.second.first)
                        continue;
                    retval[pmc.first][location_id] += pmc.second.first->Eval(context);
                }
            }
        }
        break;
    }
    case BT_PROJECT:    // TODO
    default:
        break;
    }

    return retval;
}

std::string ProductionQueue::ProductionItem::Dump() const {
    std::string retval = "ProductionItem: " + boost::lexical_cast<std::string>(build_type) + " ";
    if (!name.empty())
        retval += "name: " + name;
    if (design_id != INVALID_DESIGN_ID)
        retval += "id: " + std::to_string(design_id);
    return retval;
}


//////////////////////////////
// ProductionQueue::Element //
//////////////////////////////
ProductionQueue::Element::Element() :
    empire_id(ALL_EMPIRES),
    ordered(0),
    remaining(0),
    location(INVALID_OBJECT_ID),
    paused(false)
{}

ProductionQueue::Element::Element(ProductionItem item_, int empire_id_, int ordered_,
                                  int remaining_, int blocksize_, int location_, bool paused_) :
    item(item_),
    empire_id(empire_id_),
    ordered(ordered_),
    blocksize(blocksize_),
    remaining(remaining_),
    location(location_),
    blocksize_memory(blocksize_),
    paused(paused_)
{}

ProductionQueue::Element::Element(BuildType build_type, std::string name, int empire_id_, int ordered_,
                                  int remaining_, int blocksize_, int location_, bool paused_) :
    item(build_type, name),
    empire_id(empire_id_),
    ordered(ordered_),
    blocksize(blocksize_),
    remaining(remaining_),
    location(location_),
    blocksize_memory(blocksize_),
    paused(paused_)
{}

ProductionQueue::Element::Element(BuildType build_type, int design_id, int empire_id_, int ordered_,
                                  int remaining_, int blocksize_, int location_, bool paused_) :
    item(build_type, design_id),
    empire_id(empire_id_),
    ordered(ordered_),
    blocksize(blocksize_),
    remaining(remaining_),
    location(location_),
    blocksize_memory(blocksize_),
    paused(paused_)
{}

std::string ProductionQueue::Element::Dump() const {
    std::string retval = "ProductionQueue::Element (" + item.Dump() + ") (" +
        std::to_string(blocksize) + ") x" + std::to_string(ordered) + " ";
    retval += " (remaining: " + std::to_string(remaining) + ") ";
    return retval;
}


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
    float retval = 0.0f;
    for (const auto& entry : m_object_group_allocated_pp)
    { retval += entry.second; }
    return retval;
}

std::map<std::set<int>, float> ProductionQueue::AvailablePP(
    const std::shared_ptr<ResourcePool>& industry_pool) const
{
    std::map<std::set<int>, float> retval;
    if (!industry_pool) {
        ErrorLogger() << "ProductionQueue::AvailablePP passed invalid industry resource pool";
        return retval;
    }

    // determine available PP (ie. industry) in each resource sharing group of systems
    for (const auto& ind : industry_pool->Available()) {
        // get group of systems in industry pool
        const std::set<int>& group = ind.first;
        retval[group] = ind.second;
    }

    return retval;
}

const std::map<std::set<int>, float>& ProductionQueue::AllocatedPP() const
{ return m_object_group_allocated_pp; }

std::set<std::set<int>> ProductionQueue::ObjectsWithWastedPP(
    const std::shared_ptr<ResourcePool>& industry_pool) const
{
    std::set<std::set<int>> retval;
    if (!industry_pool) {
        ErrorLogger() << "ProductionQueue::ObjectsWithWastedPP passed invalid industry resource pool";
        return retval;
    }

    for (auto& avail_pp : AvailablePP(industry_pool)) {
        //std::cout << "available PP groups size: " << avail_pp.first.size() << " pp: " << avail_pp.second << std::endl;

        if (avail_pp.second <= 0)
            continue;   // can't waste if group has no PP
        const std::set<int>& group = avail_pp.first;
        // find this group's allocated PP
        auto alloc_it = m_object_group_allocated_pp.find(group);
        // is less allocated than is available?  if so, some is wasted
        if (alloc_it == m_object_group_allocated_pp.end() || alloc_it->second < avail_pp.second)
            retval.insert(avail_pp.first);
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

void ProductionQueue::Update() {
    const Empire* empire = GetEmpire(m_empire_id);
    if (!empire) {
        ErrorLogger() << "ProductionQueue::Update passed null empire.  doing nothing.";
        m_projects_in_progress = 0;
        m_object_group_allocated_pp.clear();
        return;
    }

    if (m_queue.empty()) {
        //DebugLogger() << "ProductionQueue::Update aborting early due to an empty queue";
        m_projects_in_progress = 0;
        m_object_group_allocated_pp.clear();

        ProductionQueueChangedSignal(); // need this so BuildingsPanel updates properly after removing last building
        return;                         // nothing to do for an empty queue
    }

    ScopedTimer update_timer("ProductionQueue::Update");

    auto available_pp = AvailablePP(empire->GetResourcePool(RE_INDUSTRY));

    // determine which resource sharing group each queue item is located in
    std::vector<std::set<int>> queue_element_groups;
    for (const auto& element : m_queue) {
        // get location object for element
        int location_id = element.location;

        // search through groups to find object
        for (auto groups_it = available_pp.begin();
             true; ++groups_it)
        {
            if (groups_it == available_pp.end()) {
                // didn't find a group containing this object, so add an empty group as this element's queue element group
                queue_element_groups.push_back(std::set<int>());
                break;
            }

            // check if location object id is in this group
            const auto& group = groups_it->first;
            auto set_it = group.find(location_id);
            if (set_it != group.end()) {
                // system is in this group.
                queue_element_groups.push_back(group);  // record this discovery
                break;                                  // stop searching for a group containing a system, since one has been found
            }
        }
    }

    // cache producibility, and production item costs and times
    // initialize production queue item completion status to 'never'
    std::map<std::pair<ProductionQueue::ProductionItem, int>,
             std::pair<float, int>> queue_item_costs_and_times;
    std::vector<bool> is_producible;
    for (auto& elem : m_queue) {
        is_producible.push_back(empire->ProducibleItem(elem.item, elem.location));
        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        std::pair<ProductionQueue::ProductionItem, int> key(elem.item, location_id);

        if (queue_item_costs_and_times.find(key) == queue_item_costs_and_times.end())
            queue_item_costs_and_times[key] = empire->ProductionCostAndTime(elem);

        elem.turns_left_to_next_item = -1;
        elem.turns_left_to_completion = -1;
    }

    // allocate pp to queue elements, returning updated available pp and updated
    // allocated pp for each group of resource sharing objects
    SetProdQueueElementSpending(available_pp, queue_element_groups,
                                queue_item_costs_and_times, is_producible, m_queue,
                                m_object_group_allocated_pp, m_projects_in_progress);

    // if at least one resource-sharing system group have available PP, simulate
    // future turns to predict when build items will be finished
    bool simulate_future = false;
    for (auto& available : available_pp) {
        if (available.second > EPSILON) {
            simulate_future = true;
            break;
        }
    }


    if (!simulate_future) {
        DebugLogger() << "not enough PP to be worth simulating future turns production.  marking everything as never complete";
        ProductionQueueChangedSignal();
        return;
    }


    // there are enough PP available in at least one group to make it worthwhile to simulate the future.
    DebugLogger() << "ProductionQueue::Update: Simulating future turns of production queue";


    // duplicate production queue state for future simulation
    QueueType sim_queue = m_queue;
    std::vector<unsigned int> sim_queue_original_indices(sim_queue.size());
    for (unsigned int i = 0; i < sim_queue_original_indices.size(); ++i)
        sim_queue_original_indices[i] = i;


    const int TOO_MANY_TURNS = 500;     // stop counting turns to completion after this long, to prevent seemingly endless loops
    const float TOO_LONG_TIME = 0.5f;   // max time in seconds to spend simulating queue


    // remove from simulated queue any items that can't be built due to not
    // meeting their location conditions; can't feasibly re-check
    // buildability each projected turn as this would require creating a simulated
    // universe into which simulated completed buildings could be inserted, as
    // well as spoofing the current turn, or otherwise faking the results for
    // evaluating arbitrary location conditions for the simulated universe.
    // this would also be inaccurate anyway due to player choices or random
    // chance, so for simplicity, it is assumed that building location
    // conditions evaluated at the present turn apply indefinitely.
    //
    // also remove from simulated queue any items that are located in a resource
    // sharing object group that is empty or that does not have any PP available
    for (unsigned int i = 0; i < sim_queue.size(); ++i) {
        const std::set<int>& group = queue_element_groups[i];

        // if any removal condition is met, remove item from queue
        bool remove = false;
        if (group.empty() || !is_producible[i]) {        // empty group or not buildable
            remove = true;
        } else {
            auto available_it = available_pp.find(group);
            if (available_it == available_pp.end() || available_it->second < EPSILON)   // missing group or non-empty group with no PP available
                remove = true;
        }

        if (remove) {
            // remove unbuildable items from the simulated queue, since they'll never finish...
            sim_queue.erase(sim_queue.begin() + i);
            is_producible.erase(is_producible.begin() + i);
            queue_element_groups.erase(queue_element_groups.begin() + i);
            sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
        }
    }

    boost::posix_time::ptime dp_time_start;
    boost::posix_time::ptime dp_time_end;
    long dp_time;

    // "Dynamic Programming" version of queue simulator -- copy the queue simulator containers at this point, after removal of unbuildable items,
    // perform dynamic programming calculation of completion times, then after regular simulation is done compare results

    // The DP version will do calculations for one resource group at a time
    // unfortunately need to copy code from SetProdQueueElementSpending  in order to work it in more efficiently here
    dp_time_start = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 

    //invert lookup direction of sim_queue_element_groups:
    std::map< std::set<int>, std::vector<int>  > elements_by_group;
    for (unsigned int i = 0; i < sim_queue.size(); ++i)
        elements_by_group[queue_element_groups[i]].push_back(i);

    // within each group, allocate PP to queue items
    for (const auto& group : available_pp) {
        unsigned int first_turn_pp_available = 1; //the first turn any pp in this resource group is available to the next item for this group
        unsigned int turn_jump = 0;
        //pp_still_available[turn-1] gives the PP still available in this resource pool at turn "turn"
        std::vector<float> pp_still_available(TOO_MANY_TURNS, group.second);  // initialize to the groups full PP allocation for each turn modeled

        std::vector<int> &this_group_elements = elements_by_group[group.first];
        auto group_begin = this_group_elements.begin();
        auto group_end = this_group_elements.end();

        // cycle through items on queue, if in this resource group then allocate production costs over time against those available to group
        for (auto el_it = group_begin;
             (el_it != group_end) && ((boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time())-dp_time_start).total_microseconds()*1e-6 < TOO_LONG_TIME);
             ++el_it)
        {
            first_turn_pp_available += turn_jump;
            turn_jump = 0;
            if (first_turn_pp_available > TOO_MANY_TURNS) {
                DebugLogger()  << "ProductionQueue::Update: Projections for Resource Group halted at " 
                               << TOO_MANY_TURNS << " turns; remaining items in this RG marked completing 'Never'.";
                break; // this resource group is allocated-out for span of simulation; remaining items in group left as never completing
            }

            unsigned int i = *el_it;
            auto& element = sim_queue[i];
            if (element.paused)
                continue;


            //DebugLogger()  << "     checking element " << element.item.name << " " << element.item.design_id << " at planet id " << element.location;

            // get cost and time from cache
            int location_id = (element.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : element.location);
            std::pair<ProductionQueue::ProductionItem, int> key(element.item, location_id);
            float item_cost;
            int build_turns;
            std::tie(item_cost, build_turns) = queue_item_costs_and_times[key];
            float total_item_cost = item_cost * element.blocksize;

            float allocation;
            float element_this_turn_limit;
            //DebugLogger() << "ProductionQueue::Update Queue index   Queue Item: " << element.item.name;

            // iterate over the turns necessary to complete item
            for (int j = 0; j < static_cast<int>(TOO_MANY_TURNS - first_turn_pp_available + 1); j++) {
                // determine how many pp to allocate to this queue element this turn.  allocation is limited by the
                // item cost, which is the max number of PP per turn that can be put towards this item, and by the
                // total cost remaining to complete the last item in the queue element (eg. the element has all but
                // the last item complete already) and by the total pp available in this element's production location's
                // resource sharing group

                //DebugLogger()  << "     turn: " << j << "; per turn limit: " << element_per_turn_limit << "; pp stil avail: " << pp_still_available[first_turn_pp_available+j-1];
                element_this_turn_limit = CalculateProductionPerTurnLimit(element, item_cost, build_turns);
                allocation = std::max(0.0f, std::min(element_this_turn_limit, pp_still_available[first_turn_pp_available+j-1]));
                element.progress += allocation / std::max(EPSILON, total_item_cost);    // add turn's progress due to allocation
                float item_cost_remaining = total_item_cost*(1.0f - element.progress);
                //DebugLogger()  << "     allocation: " << allocation << "; new progress: "<< element.progress << " with " << item_cost_remaining << " remaining";
                pp_still_available[first_turn_pp_available+j-1] -= allocation;
                if (pp_still_available[first_turn_pp_available+j-1] <= 0 ) {
                    pp_still_available[first_turn_pp_available+j-1] = 0;
                    ++turn_jump;
                }

                // check if additional turn's PP allocation was enough to finish next item in element
                if (item_cost_remaining < EPSILON ) {
                    //DebugLogger()  << "     finished an item";
                    // an item has been completed. 
                    // deduct cost of one item from accumulated PP.  don't set
                    // accumulation to zero, as this would eliminate any partial
                    // completion of the next item
                    element.progress = std::max(0.0f, element.progress - 1.0f);
                    --element.remaining;  //pretty sure this just effects the dp version & should do even if also doing ORIG_SIMULATOR

                    //DebugLogger() << "ProductionQueue::Recording DP sim results for item " << element.item.name;

                    // if this was the first item in the element to be completed in
                    // this simuation, update the original queue element with the
                    // turns required to complete the next item in the element
                    if (element.remaining + 1 == m_queue[sim_queue_original_indices[i]].remaining) //had already decremented element.remaining above
                        m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = first_turn_pp_available+j;
                    if (!element.remaining) {
                        m_queue[sim_queue_original_indices[i]].turns_left_to_completion = first_turn_pp_available+j;    // record the (estimated) turns to complete the whole element on the original queue
                    }
                }
                if (!element.remaining) {
                    //DebugLogger()  << "     finished this element";
                    break; // this element all done
                }
            } //j-loop : turns relative to first_turn_pp_available
        } // queue element loop
    } // resource groups loop

    dp_time_end = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
    dp_time = (dp_time_end - dp_time_start).total_microseconds();
    if ((dp_time * 1e-6) >= TOO_LONG_TIME) {
        DebugLogger()  << "ProductionQueue::Update: Projections timed out after " << dp_time
                       << " microseconds; all remaining items in queue marked completing 'Never'.";
    }
    DebugLogger() << "ProductionQueue::Update: Projections took "
                  << ((dp_time_end - dp_time_start).total_microseconds()) << " microseconds with "
                  << empire->ProductionPoints() << " total Production Points";
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

void ProductionQueue::clear() {
    m_queue.clear();
    m_projects_in_progress = 0;
    m_object_group_allocated_pp.clear();
    ProductionQueueChangedSignal();
}

////////////
// Empire //
////////////
Empire::Empire() :
    m_id(ALL_EMPIRES),
    m_research_queue(m_id),
    m_production_queue(m_id)
{ Init(); }

Empire::Empire(const std::string& name, const std::string& player_name,
               int empire_id, const GG::Clr& color) :
    m_id(empire_id),
    m_name(name),
    m_player_name(player_name),
    m_color(color),
    m_research_queue(m_id),
    m_production_queue(m_id)
{
    DebugLogger() << "Empire::Empire(" << name << ", " << player_name << ", " << empire_id << ", colour)";
    Init();
}

void Empire::Init() {
    m_resource_pools[RE_RESEARCH] = std::make_shared<ResourcePool>(RE_RESEARCH);
    m_resource_pools[RE_INDUSTRY] = std::make_shared<ResourcePool>(RE_INDUSTRY);
    m_resource_pools[RE_TRADE] = std::make_shared<ResourcePool>(RE_TRADE);

    m_eliminated = false;

    m_meters[UserStringNop("METER_DETECTION_STRENGTH")];
    m_meters[UserStringNop("METER_BUILDING_COST_FACTOR")];
    m_meters[UserStringNop("METER_SHIP_COST_FACTOR")];
    m_meters[UserStringNop("METER_TECH_COST_FACTOR")];
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

int Empire::SourceID() const {
    auto good_source = Source();
    return good_source ? good_source->ID() : INVALID_OBJECT_ID;
}

std::shared_ptr<const UniverseObject> Empire::Source() const {
    if (m_eliminated)
        return nullptr;

    // Use the current source if valid
    auto valid_current_source = GetUniverseObject(m_source_id);
    if (valid_current_source && valid_current_source->OwnedBy(m_id))
        return valid_current_source;

    // Try the capital
    auto capital_as_source = GetUniverseObject(m_capital_id);
    if (capital_as_source && capital_as_source->OwnedBy(m_id)) {
        m_source_id = m_capital_id;
        return capital_as_source;
    }

    // Find any object owned by the empire
    // TODO determine if ExistingObjects() is faster and acceptable
    for (const auto& obj_it : Objects()) {
        if (obj_it->OwnedBy(m_id)) {
            m_source_id = obj_it->ID();
            return (obj_it);
        }
    }

    m_source_id = INVALID_OBJECT_ID;
    return nullptr;
}

std::string Empire::Dump() const {
    std::string retval = "Empire name: " + m_name +
                         " ID: " + std::to_string(m_id) +
                         " Capital ID: " + std::to_string(m_capital_id);
    retval += " meters:\n";
    for (const auto& meter : m_meters) {
        retval += UserString(meter.first) + ": " +
                  std::to_string(meter.second.Initial()) + "\n";
    }
    return retval;
}

void Empire::SetCapitalID(int id) {
    m_capital_id = INVALID_OBJECT_ID;
    m_source_id = INVALID_OBJECT_ID;

    if (id == INVALID_OBJECT_ID)
        return;

    // Verify that the capital exists and is owned by the empire
    auto possible_capital = Objects().ExistingObject(id);
    if (possible_capital && possible_capital->OwnedBy(m_id))
        m_capital_id = id;

    auto possible_source = GetUniverseObject(id);
    if (possible_source && possible_source->OwnedBy(m_id))
        m_source_id = id;
}

Meter* Empire::GetMeter(const std::string& name) {
    auto it = m_meters.find(name);
    if (it != m_meters.end())
        return &(it->second);
    else
        return nullptr;
}

const Meter* Empire::GetMeter(const std::string& name) const {
    auto it = m_meters.find(name);
    if (it != m_meters.end())
        return &(it->second);
    else
        return nullptr;
}

void Empire::BackPropagateMeters() {
    for (auto& meter : m_meters)
        meter.second.BackPropagate();
}

bool Empire::ResearchableTech(const std::string& name) const {
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    for (const auto& prereq : tech->Prerequisites()) {
        if (m_techs.find(prereq) == m_techs.end())
            return false;
    }
    return true;
}

bool Empire::HasResearchedPrereqAndUnresearchedPrereq(const std::string& name) const {
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    bool one_unresearched = false;
    bool one_researched = false;
    for (const auto& prereq : tech->Prerequisites()) {
        if (m_techs.find(prereq) != m_techs.end())
            one_researched = true;
        else
            one_unresearched = true;
    }
    return one_unresearched && one_researched;
}

const ResearchQueue& Empire::GetResearchQueue() const
{ return m_research_queue; }

float Empire::ResearchProgress(const std::string& name) const {
    auto it = m_research_progress.find(name);
    if (it == m_research_progress.end())
        return 0.0f;
    const Tech* tech = GetTech(it->first);
    if (!tech)
        return 0.0f;
    float tech_cost = tech->ResearchCost(m_id);
    return it->second * tech_cost;
}

const std::map<std::string, int>& Empire::ResearchedTechs() const
{ return m_techs; }

bool Empire::TechResearched(const std::string& name) const
{ return m_techs.find(name) != m_techs.end(); }

TechStatus Empire::GetTechStatus(const std::string& name) const {
    if (TechResearched(name)) return TS_COMPLETE;
    if (ResearchableTech(name)) return TS_RESEARCHABLE;
    if (HasResearchedPrereqAndUnresearchedPrereq(name)) return TS_HAS_RESEARCHED_PREREQ;
    return TS_UNRESEARCHABLE;
}

const std::string& Empire::TopPriorityEnqueuedTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    auto it = m_research_queue.begin();
    const std::string& tech = it->name;
    return tech;
}

const std::string& Empire::MostExpensiveEnqueuedTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float biggest_cost = -99999.9f; // arbitrary small number

    const ResearchQueue::Element* best_elem = nullptr;

    for (const auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id);
        if (tech_cost > biggest_cost) {
            biggest_cost = tech_cost;
            best_elem = &elem;
        }
    }

    if (best_elem)
        return best_elem->name;
    return EMPTY_STRING;
}

const std::string& Empire::LeastExpensiveEnqueuedTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float smallest_cost = 999999.9f; // arbitrary large number

    const ResearchQueue::Element* best_elem = nullptr;

    for (const auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id);
        if (tech_cost < smallest_cost) {
            smallest_cost = tech_cost;
            best_elem = &elem;
        }
    }

    if (best_elem)
        return best_elem->name;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPSpentEnqueuedTech() const {
    float most_spent = -999999.9f;  // arbitrary small number
    const std::map<std::string, float>::value_type* best_progress = nullptr;

    for (const auto& progress : m_research_progress) {
        const auto& tech_name = progress.first;
        if (m_research_queue.find(tech_name) == m_research_queue.end())
            continue;
        float rp_spent = progress.second;
        if (rp_spent > most_spent) {
            best_progress = &progress;
            most_spent = rp_spent;
        }
    }

    if (best_progress)
        return best_progress->first;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPCostLeftEnqueuedTech() const {
    float most_left = -999999.9f;  // arbitrary small number
    const std::map<std::string, float>::value_type* best_progress = nullptr;

    for (const auto& progress : m_research_progress) {
        const auto& tech_name = progress.first;
        const Tech* tech = GetTech(tech_name);
        if (!tech)
            continue;

        if (m_research_queue.find(tech_name) == m_research_queue.end())
            continue;

        float rp_spent = progress.second;
        float rp_total_cost = tech->ResearchCost(m_id);
        float rp_left = std::max(0.0f, rp_total_cost - rp_spent);

        if (rp_left > most_left) {
            best_progress = &progress;
            most_left = rp_left;
        }
    }

    if (best_progress)
        return best_progress->first;
    return EMPTY_STRING;
}

const std::string& Empire::TopPriorityResearchableTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    for (const auto& elem : m_research_queue) {
        if (this->ResearchableTech(elem.name))
            return elem.name;
    }
    return EMPTY_STRING;
}

const std::string& Empire::MostExpensiveResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::LeastExpensiveResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::MostRPSpentResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::MostRPCostLeftResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
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
    for (int design_id : m_ship_designs) {
        if (ShipDesignAvailable(design_id))
            retval.insert(design_id);
    }
    return retval;
}

bool Empire::ShipDesignAvailable(int ship_design_id) const {
    const ShipDesign* design = GetShipDesign(ship_design_id);
    return design ? ShipDesignAvailable(*design) : false;
}

bool Empire::ShipDesignAvailable(const ShipDesign& design) const {
    if (!design.Producible()) return false;

    // design is kept, but still need to verify that it is buildable at this time.  Part or hull tech
    // requirements might prevent it from being built.
    for (const auto& name : design.Parts()) {
        if (name.empty())
            continue;   // empty slot can't be unavailable
        if (!ShipPartAvailable(name))
            return false;
    }
    if (!ShipHullAvailable(design.Hull()))
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

float Empire::ProductionStatus(int i) const {
    if (0 > i || i >= static_cast<int>(m_production_queue.size()))
        return -1.0f;
    float item_progress = m_production_queue[i].progress;
    float item_cost;
    int item_time;
    std::tie(item_cost, item_time) = this->ProductionCostAndTime(m_production_queue[i]);
    return item_progress * item_cost * m_production_queue[i].blocksize;
}

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
    ErrorLogger() << "Empire::ProductionCostAndTime was passed a ProductionItem with an invalid BuildType";
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

    const auto* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    auto build_location = GetUniverseObject(location);
    if (!build_location)
        return false;

    if (build_type == BT_BUILDING) {
        // specified location must be a valid production location for that building type
        return building_type->ProductionLocation(m_id, location);

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
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

    auto build_location = GetUniverseObject(location);
    if (!build_location) return false;

    if (build_type == BT_SHIP) {
        // specified location must be a valid production location for this design
        return ship_design->ProductionLocation(m_id, location);

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
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

bool Empire::EnqueuableItem(BuildType build_type, const std::string& name, int location) const {
    if (build_type != BT_BUILDING)
        return false;

    const auto* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    auto build_location = GetUniverseObject(location);
    if (!build_location)
        return false;

    // specified location must be a valid production location for that building type
    return building_type->EnqueueLocation(m_id, location);
}

bool Empire::EnqueuableItem(const ProductionQueue::ProductionItem& item, int location) const {
    if (item.build_type == BT_BUILDING)
        return EnqueuableItem(item.build_type, item.name, location);
    else if (item.build_type == BT_SHIP)    // ships don't have a distinction between enqueuable and producible
        return ProducibleItem(item.build_type, item.design_id, location);
    else
        throw std::invalid_argument("Empire::ProducibleItem was passed a ProductionItem with an invalid BuildType");
    return false;
}

int Empire::NumSitRepEntries(int turn/* = INVALID_GAME_TURN*/) const {
    if (turn == INVALID_GAME_TURN)
        return m_sitrep_entries.size();
    int count = 0;
    for (const SitRepEntry& sitrep : m_sitrep_entries)
        if (sitrep.GetTurn() == turn)
            count++;
    return count;
}

bool Empire::Eliminated() const {
    return m_eliminated;
}

void Empire::Eliminate() {
    m_eliminated = true;

    for (auto& entry : Empires())
        entry.second->AddSitRepEntry(CreateEmpireEliminatedSitRep(EmpireID()));

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
    for (auto& entry : m_resource_pools) {
        entry.second->SetObjects(std::vector<int>());
    }
    m_population_pool.SetPopCenters(std::vector<int>());

    // m_ship_names_used;
    m_supply_system_ranges.clear();
    m_supply_unobstructed_systems.clear();
}

bool Empire::Won() const {
    return !m_victories.empty();
}

void Empire::Win(const std::string& reason) {
    if (m_victories.insert(reason).second) {
        for (auto& entry : Empires()) {
            entry.second->AddSitRepEntry(CreateVictorySitRep(reason, EmpireID()));
        }
    }
}

void Empire::UpdateSystemSupplyRanges(const std::set<int>& known_objects) {
    //std::cout << "Empire::UpdateSystemSupplyRanges() for empire " << this->Name() << std::endl;
    m_supply_system_ranges.clear();

    // as of this writing, only planets can generate supply propagation
    std::vector<std::shared_ptr<const UniverseObject>> owned_planets;
    for (int object_id : known_objects) {
        if (auto planet = GetPlanet(object_id))
            if (planet->OwnedBy(this->EmpireID()))
                owned_planets.push_back(planet);
    }

    //std::cout << "... empire owns " << owned_planets.size() << " planets" << std::endl;
    for (auto& obj : owned_planets) {
        //std::cout << "... considering owned planet: " << obj->Name() << std::endl;

        // ensure object is within a system, from which it can distribute supplies
        int system_id = obj->SystemID();
        if (system_id == INVALID_OBJECT_ID)
            continue;   // TODO: consider future special case if current object is itself a system

        // check if object has a supply meter
        if (obj->GetMeter(METER_SUPPLY)) {
            // get resource supply range for next turn for this object
            float supply_range = obj->NextTurnCurrentMeterValue(METER_SUPPLY);

            // if this object can provide more supply range than the best previously checked object in this system, record its range as the new best for the system
            auto system_it = m_supply_system_ranges.find(system_id);  // try to find a previous entry for this system's supply range
            if (system_it == m_supply_system_ranges.end() || supply_range > system_it->second) {// if there is no previous entry, or the previous entry is shorter than the new one, add or replace the entry
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
    for (int object_id : known_objects_vec)
        if (known_destroyed_objects.find(object_id) == known_destroyed_objects.end())
            known_objects_set.insert(object_id);
    UpdateSystemSupplyRanges(known_objects_set);
}

void Empire::UpdateUnobstructedFleets() {
    const std::set<int>& known_destroyed_objects =
        GetUniverse().EmpireKnownDestroyedObjectIDs(this->EmpireID());

    for (int system_id : m_supply_unobstructed_systems) {
        auto system = GetSystem(system_id);
        if (!system)
            continue;

        for (auto& fleet : Objects().FindObjects<Fleet>(system->FleetIDs())) {
            if (known_destroyed_objects.find(fleet->ID()) != known_destroyed_objects.end())
                continue;
            if (fleet->OwnedBy(m_id))
                fleet->SetArrivalStarlane(system_id);
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
    for (int system_id : known_systems_vec)
        if (known_destroyed_objects.find(system_id) == known_destroyed_objects.end())
            known_systems_set.insert(system_id);
    UpdateSupplyUnobstructedSystems(known_systems_set);
}

void Empire::UpdateSupplyUnobstructedSystems(const std::set<int>& known_systems) {
    //DebugLogger() << "UpdateSupplyUnobstructedSystems for empire " << m_id;
    m_supply_unobstructed_systems.clear();

    // get systems with historically at least partial visibility
    std::set<int> systems_with_at_least_partial_visibility_at_some_point;
    for (int system_id : known_systems) {
        const auto& vis_turns = GetUniverse().GetObjectVisibilityTurnMapByEmpire(system_id, m_id);
        if (vis_turns.find(VIS_PARTIAL_VISIBILITY) != vis_turns.end())
            systems_with_at_least_partial_visibility_at_some_point.insert(system_id);
    }

    // get all fleets, or just those visible to this client's empire
    const auto& known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(this->EmpireID());

    // get empire supply ranges
    std::map<int, std::map<int, float>> empire_system_supply_ranges;
    for (const auto& entry : Empires()) {
        const Empire* empire = entry.second;
        empire_system_supply_ranges[entry.first] = empire->SystemSupplyRanges();
    }

    // find systems that contain fleets that can either maintain supply or block supply.
    // to affect supply in either manner, a fleet must be armed & aggressive, & must be not
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
    for (auto& fleet : GetUniverse().Objects().FindObjects<Fleet>()) {
        int system_id = fleet->SystemID();
        if (system_id == INVALID_OBJECT_ID) {
            continue;   // not in a system, so can't affect system obstruction
        } else if (known_destroyed_objects.find(fleet->ID()) != known_destroyed_objects.end()) {
            continue; //known to be destroyed so can't affect supply, important just in case being updated on client side
        }

        //DebugLogger() << "Fleet " << fleet->ID() << " is in system " << system_id << " with next system " << fleet->NextSystemID() << " and is owned by " << fleet->Owner() << " armed: " << fleet->HasArmedShips() << " and agressive: " << fleet->Aggressive();
        if ((fleet->HasArmedShips() || fleet->HasFighterShips()) && fleet->Aggressive()) {
            if (fleet->OwnedBy(m_id)) {
                if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                    systems_containing_friendly_fleets.insert(system_id);
                    if (fleet->ArrivalStarlane() == system_id)
                        unrestricted_friendly_systems.insert(system_id);
                    else
                        systems_with_lane_preserving_fleets.insert(system_id);
                }
            } else if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                int fleet_owner = fleet->Owner();
                if (fleet_owner == ALL_EMPIRES || Empires().GetDiplomaticStatus(m_id, fleet_owner) == DIPLO_WAR) {
                    systems_containing_obstructing_objects.insert(system_id);
                    if (fleet->ArrivalStarlane() == system_id)
                        unrestricted_obstruction_systems.insert(system_id);
                }
            }
        }
    }

    //std::stringstream ss;
    //for (int obj_id : systems_containing_obstructing_objects)
    //{ ss << obj_id << ", "; }
    //DebugLogger() << "systems with obstructing objects for empire " << m_id << " : " << ss.str();


    // check each potential supplyable system for whether it can propagate supply.
    for (int sys_id : known_systems) {
        //DebugLogger() << "deciding unobstructedness for system " << sys_id;

        // has empire ever seen this system with partial or better visibility?
        if (systems_with_at_least_partial_visibility_at_some_point.find(sys_id) ==
            systems_with_at_least_partial_visibility_at_some_point.end())
        { continue; }

        // if system is explored, then whether it can propagate supply depends
        // on what friendly / enemy ships and planets are in the system

        if (unrestricted_friendly_systems.find(sys_id) != unrestricted_friendly_systems.end()) {
            // if there are unrestricted friendly ships, supply can propagate
            m_supply_unobstructed_systems.insert(sys_id);

        } else if (systems_containing_friendly_fleets.find(sys_id) != systems_containing_friendly_fleets.end()) {
            if (unrestricted_obstruction_systems.find(sys_id) == unrestricted_obstruction_systems.end()) {
                // if there are (previously) restricted friendly ships, and no unrestricted enemy fleets, supply can propagate
                m_supply_unobstructed_systems.insert(sys_id);
            }

        } else if (systems_containing_obstructing_objects.find(sys_id) == systems_containing_obstructing_objects.end()) {
            m_supply_unobstructed_systems.insert(sys_id);

        } else if (systems_with_lane_preserving_fleets.find(sys_id) == systems_with_lane_preserving_fleets.end()) {
            // otherwise, if system contains no friendly fleets capable of
            // maintaining lane access but does contain an unfriendly fleet,
            // so it is obstructed, so isn't included in the unobstructed
            // systems set.  Furthermore, this empire's available system exit
            // lanes for this system are cleared
            if (!m_available_system_exit_lanes[sys_id].empty()) {
                //DebugLogger() << "Empire::UpdateSupplyUnobstructedSystems clearing available lanes for system ("<<sys_id<<"); available lanes were:";
                //for (int system_id : m_available_system_exit_lanes[sys_id])
                //    DebugLogger() << "...... "<< system_id;
                m_available_system_exit_lanes[sys_id].clear();
            }
        }
    }
}

void Empire::RecordPendingLaneUpdate(int start_system_id, int dest_system_id) {
    if (m_supply_unobstructed_systems.find(start_system_id) == m_supply_unobstructed_systems.end()) {
        m_pending_system_exit_lanes[start_system_id].insert(dest_system_id); 

    } else { // if the system is unobstructed, mark all its lanes as avilable
        auto system = GetSystem(start_system_id);
        for (const auto& lane : system->StarlanesWormholes()) {
            m_pending_system_exit_lanes[start_system_id].insert(lane.first); // will add both starlanes and wormholes
        }
    }
}

void Empire::UpdateAvailableLanes() {
    for (auto& system : m_pending_system_exit_lanes) {
        m_available_system_exit_lanes[system.first].insert(system.second.begin(), system.second.end());
        system.second.clear();
    }
    m_pending_system_exit_lanes.clear(); // TODO: consider: not really necessary, & may be more efficient to not clear.
}

const std::map<int, float>& Empire::SystemSupplyRanges() const
{ return m_supply_system_ranges; }

const std::set<int>& Empire::SupplyUnobstructedSystems() const
{ return m_supply_unobstructed_systems; }

const bool Empire::UnrestrictedLaneTravel(int start_system_id, int dest_system_id) const {
    auto find_it = m_available_system_exit_lanes.find(start_system_id);
    if (find_it != m_available_system_exit_lanes.end() ) {
        if (find_it->second.find(dest_system_id) != find_it->second.end())
            return true;
    }
    return false;
}

const std::set<int>& Empire::ExploredSystems() const
{ return m_explored_systems; }

const std::map<int, std::set<int>> Empire::KnownStarlanes() const {
    // compile starlanes leading into or out of each system
    std::map<int, std::set<int>> retval;

    const Universe& universe = GetUniverse();

    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());
    for (auto sys_it = Objects().const_begin<System>();
         sys_it != Objects().const_end<System>(); ++sys_it)
    {
        int start_id = sys_it->ID();

        // exclude lanes starting at systems known to be destroyed
        if (known_destroyed_objects.find(start_id) != known_destroyed_objects.end())
            continue;

        for (const auto& lane : sys_it->StarlanesWormholes()) {
            if (lane.second || known_destroyed_objects.find(lane.second) != known_destroyed_objects.end())
                continue;   // is a wormhole, not a starlane, or is connected to a known destroyed system
            int end_id = lane.first;
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }
    }

    return retval;
}

const std::map<int, std::set<int>> Empire::VisibleStarlanes() const {
    std::map<int, std::set<int>> retval;   // compile starlanes leading into or out of each system

    const Universe& universe = GetUniverse();
    const ObjectMap& objects = universe.Objects();

    for (auto sys_it = objects.const_begin<System>();
         sys_it != objects.const_end<System>(); ++sys_it)
    {
        int start_id = sys_it->ID();

        // is system visible to this empire?
        if (universe.GetObjectVisibilityByEmpire(start_id, m_id) <= VIS_NO_VISIBILITY)
            continue;

        // get system's visible lanes for this empire
        for (auto& lane : sys_it->VisibleStarlanesWormholes(m_id)) {
            if (lane.second)
                continue;   // is a wormhole, not a starlane
            int end_id = lane.first;
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }
    }

    return retval;
}

Empire::SitRepItr Empire::SitRepBegin() const
{ return m_sitrep_entries.begin(); }

Empire::SitRepItr Empire::SitRepEnd() const
{ return m_sitrep_entries.end(); }

float Empire::ProductionPoints() const
{ return GetResourcePool(RE_INDUSTRY)->TotalAvailable(); }

const std::shared_ptr<ResourcePool> Empire::GetResourcePool(ResourceType resource_type) const {
    auto it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        return nullptr;
    return it->second;
}

float Empire::ResourceStockpile(ResourceType type) const {
    auto it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceStockpile passed invalid ResourceType");
    return it->second->Stockpile();
}

float Empire::ResourceOutput(ResourceType type) const {
    auto it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceOutput passed invalid ResourceType");
    return it->second->Output();
}

float Empire::ResourceAvailable(ResourceType type) const {
    auto it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceAvailable passed invalid ResourceType");
    return it->second->TotalAvailable();
}

const PopulationPool& Empire::GetPopulationPool() const
{ return m_population_pool; }

float Empire::Population() const
{ return m_population_pool.Population(); }

void Empire::SetResourceStockpile(ResourceType resource_type, float stockpile) {
    auto it = m_resource_pools.find(resource_type);
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

    auto it = m_research_queue.find(name);

    if (pos < 0 || static_cast<int>(m_research_queue.size()) <= pos) {
        // default to putting at end
        bool paused = false;
        if (it != m_research_queue.end()) {
            paused = it->paused;
            m_research_queue.erase(it);
        }
        m_research_queue.push_back(name, paused);
    } else {
        // put at requested position
        if (it < m_research_queue.begin() + pos)
            --pos;
        bool paused = false;
        if (it != m_research_queue.end()) {
            paused = it->paused;
            m_research_queue.erase(it);
        }
        m_research_queue.insert(m_research_queue.begin() + pos, name, paused);
    }
}

void Empire::RemoveTechFromQueue(const std::string& name) {
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        m_research_queue.erase(it);
}

void Empire::PauseResearch(const std::string& name) {
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        it->paused = true;
}

void Empire::ResumeResearch(const std::string& name){
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        it->paused = false;
}

void Empire::SetTechResearchProgress(const std::string& name, float progress) {
    const Tech* tech = GetTech(name);
    if (!tech) {
        ErrorLogger() << "Empire::SetTechResearchProgress no such tech as: " << name;
        return;
    }
    if (TechResearched(name))
        return; // can't affect already-researched tech

    // set progress
    float clamped_progress = std::min(1.0f, std::max(0.0f, progress));
    m_research_progress[name] = clamped_progress;

    // if tech is complete, ensure it is on the queue, so it will be researched next turn
    if (clamped_progress >= tech->ResearchCost(m_id) &&
        m_research_queue.find(name) == m_research_queue.end())
    { m_research_queue.push_back(name); }

    // don't just give tech to empire, as another effect might reduce its progress before end of turn
}

const unsigned int MAX_PROD_QUEUE_SIZE = 500;

void Empire::PlaceProductionOnQueue(BuildType build_type, const std::string& name, int number,
                                    int blocksize, int location, int pos/* = -1*/)
{
    if (!EnqueuableItem(build_type, name, location)) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Attempted to place non-enqueuable item in queue: build_type: "
                      << boost::lexical_cast<std::string>(build_type) << "  name: " << name << "  location: " << location;
        return;
    }

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Maximum queue size reached. Aborting enqueue";
        return;
    }

    if (!ProducibleItem(build_type, name, location)) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: "
                      << boost::lexical_cast<std::string>(build_type) << "  name: " << name << "  location: " << location;
        return;
    }

    ProductionQueue::Element build(build_type, name, m_id, number, number,
                                   blocksize, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(build);
    else
        m_production_queue.insert(m_production_queue.begin() + pos, build);
}

void Empire::PlaceProductionOnQueue(BuildType build_type, int design_id, int number,
                                    int blocksize, int location, int pos/* = -1*/)
{
    // ship designs don't have a distinction between enqueuable and producible...

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Maximum queue size reached. Aborting enqueue";
        return;
    }

    if (!ProducibleItem(build_type, design_id, location)) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: "
                      << boost::lexical_cast<std::string>(build_type) << "  design_id: " << design_id << "  location: " << location;
        return;
    }

    ProductionQueue::Element elem(build_type, design_id, m_id, number, number, blocksize, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(elem);
    else
        m_production_queue.insert(m_production_queue.begin() + pos, elem);
}

void Empire::PlaceProductionOnQueue(const ProductionQueue::ProductionItem& item, int number,
                                    int blocksize, int location, int pos/* = -1*/)
{
    if (item.build_type == BT_BUILDING)
        PlaceProductionOnQueue(item.build_type, item.name, number, blocksize, location, pos);
    else if (item.build_type == BT_SHIP)
        PlaceProductionOnQueue(item.build_type, item.design_id, number, blocksize, location, pos);
    else
        throw std::invalid_argument("Empire::PlaceProductionOnQueue was passed a ProductionQueue::ProductionItem with an invalid BuildType");
}

void Empire::SetProductionQuantityAndBlocksize(int index, int quantity, int blocksize) {
    DebugLogger() << "Empire::SetProductionQuantityAndBlocksize() called for item "<< m_production_queue[index].item.name << "with new quant " << quantity << " and new blocksize " << blocksize;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && ((1 < quantity) || ( 1 < blocksize) ))
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    //int original_blocksize = m_production_queue[index].blocksize;
    blocksize = std::max(1, blocksize);
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
    m_production_queue[index].blocksize = blocksize;
    //std::cout << "original block size: " << original_blocksize << "  new blocksize: " << blocksize << "  memory blocksize: " << m_production_queue[index].blocksize_memory << std::endl;
    if (blocksize <= m_production_queue[index].blocksize_memory) {
        // if reducing block size, progress on retained portion is unchanged.
        // if increasing block size, progress is proportionally reduced, unless undoing a recent reduction in block size
        m_production_queue[index].progress = m_production_queue[index].progress_memory;
    } else {
        m_production_queue[index].progress = m_production_queue[index].progress_memory * m_production_queue[index].blocksize_memory / blocksize;
    }
}

void Empire::SplitIncompleteProductionItem(int index) {
    DebugLogger() << "Empire::SplitIncompleteProductionItem() called for index " << index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SplitIncompleteProductionItem() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (m_production_queue[index].item.build_type == BT_BUILDING)
        throw std::runtime_error("Empire::SplitIncompleteProductionItem() : Attempted to split a production item that is not a ship.");

    ProductionQueue::Element& elem = m_production_queue[index];

    // if "splitting" an item with just 1 remaining, do nothing
    if (elem.remaining <= 1)
        return;

    // add duplicate
    int new_item_quantity = elem.remaining - 1;
    elem.remaining = 1; // reduce remaining on specified to 1
    PlaceProductionOnQueue(elem.item, new_item_quantity, elem.blocksize, elem.location, index + 1);
}

void Empire::DuplicateProductionItem(int index) {
    DebugLogger() << "Empire::DuplicateProductionItem() called for index " << index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::DuplicateProductionItem() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");

    auto& elem = m_production_queue[index];
    PlaceProductionOnQueue(elem.item, elem.remaining, elem.blocksize, elem.location, index + 1);
}

void Empire::SetProductionRallyPoint(int index, int rally_point_id) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    m_production_queue[index].rally_point_id = rally_point_id;
}

void Empire::SetProductionQuantity(int index, int quantity) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && 1 < quantity)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
}

void Empire::MoveProductionWithinQueue(int index, int new_index) {
    if (index < new_index)
        --new_index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index ||
        new_index < 0 || static_cast<int>(m_production_queue.size()) <= new_index)
    {
        DebugLogger() << "Empire::MoveProductionWithinQueue index: " << index << "  new index: "
                               << new_index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to move a production queue item to or from an invalid index.";
        return;
    }
    auto build = m_production_queue[index];
    m_production_queue.erase(index);
    m_production_queue.insert(m_production_queue.begin() + new_index, build);
}

void Empire::RemoveProductionFromQueue(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::RemoveProductionFromQueue index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to delete a production queue item with an invalid index.";
        return;
    }
    m_production_queue.erase(index);
}

void Empire::PauseProduction(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::PauseProduction index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted pause a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].paused = true;
}

void Empire::ResumeProduction(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::ResumeProduction index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted resume a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].paused = false;
}

void Empire::ConquerProductionQueueItemsAtLocation(int location_id, int empire_id) {
    if (location_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire::ConquerProductionQueueItemsAtLocation: tried to conquer build items located at an invalid location";
        return;
    }

    DebugLogger() << "Empire::ConquerProductionQueueItemsAtLocation: conquering items located at "
                           << location_id << " to empire " << empire_id;

    Empire* to_empire = GetEmpire(empire_id);    // may be null
    if (!to_empire && empire_id != ALL_EMPIRES) {
        ErrorLogger() << "Couldn't get empire with id " << empire_id;
        return;
    }


    for (auto& entry : Empires()) {
        int from_empire_id = entry.first;
        if (from_empire_id == empire_id) continue;    // skip this empire; can't capture one's own ProductionItems

        Empire* from_empire = entry.second;
        ProductionQueue& queue = from_empire->m_production_queue;

        for (auto queue_it = queue.begin(); queue_it != queue.end(); ) {
            auto elem = *queue_it;
            if (elem.location != location_id) {
                ++queue_it;
                continue; // skip projects with wrong location
            }

            ProductionQueue::ProductionItem item = elem.item;

            if (item.build_type == BT_BUILDING) {
                std::string name = item.name;
                const BuildingType* type = GetBuildingType(name);
                if (!type) {
                    ErrorLogger() << "ConquerProductionQueueItemsAtLocation couldn't get building with name " << name;
                    continue;
                }

                CaptureResult result = type->GetCaptureResult(from_empire_id, empire_id, location_id, true);

                if (result == CR_DESTROY) {
                    // item removed from current queue, NOT added to conquerer's queue
                    queue_it = queue.erase(queue_it);

                } else if (result == CR_CAPTURE) {
                    if (to_empire) {
                        // item removed from current queue, added to conquerer's queue
                        ProductionQueue::Element new_elem(item, empire_id, elem.ordered, elem.remaining,
                                                          1, location_id);
                        new_elem.progress = elem.progress;
                        to_empire->m_production_queue.push_back(new_elem);

                        queue_it = queue.erase(queue_it);
                    } else {
                        // else do nothing; no empire can't capure things
                        ++queue_it;
                    }

                } else if (result == INVALID_CAPTURE_RESULT) {
                    ErrorLogger() << "Empire::ConquerBuildsAtLocationFromEmpire: BuildingType had an invalid CaptureResult";
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
        ErrorLogger() << "Empire::AddTech given and invalid tech: " << name;
        return;
    }

    if (m_techs.find(name) == m_techs.end())
        AddSitRepEntry(CreateTechResearchedSitRep(name));

    for (const ItemSpec& item : tech->UnlockedItems())
        UnlockItem(item);  // potential infinite if a tech (in)directly unlocks itself?

    if (m_techs.find(name) == m_techs.end())
        m_techs[name] = CurrentTurn();
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
        ErrorLogger() << "Empire::UnlockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::AddBuildingType(const std::string& name) {
    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type) {
        ErrorLogger() << "Empire::AddBuildingType given an invalid building type name: " << name;
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
        ErrorLogger() << "Empire::AddPartType given an invalid part type name: " << name;
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
        ErrorLogger() << "Empire::AddHullType given an invalid hull type name: " << name;
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
        ErrorLogger() << "Empire::AddExploredSystem given an invalid system id: " << ID;
}

std::string Empire::NewShipName() {
    static std::vector<std::string> ship_names = UserStringList("SHIP_NAMES");
    if (ship_names.empty())
        ship_names.push_back(UserString("OBJ_SHIP"));

    // select name randomly from list
    int ship_name_idx = RandSmallInt(0, static_cast<int>(ship_names.size()) - 1);
    std::string retval = ship_names[ship_name_idx];
    int times_name_used = ++m_ship_names_used[retval];
    if (1 < times_name_used)
        retval += " " + RomanNumber(times_name_used);
    return retval;
}

void Empire::AddShipDesign(int ship_design_id, int next_design_id) {
    /* Check if design id is valid.  That is, check that it corresponds to an
     * existing shipdesign in the universe.  On clients, this means that this
     * empire knows about this ship design and the server consequently sent the
     * design to this player.  On the server, all existing ship designs will be
     * valid, so this just adds this design's id to those that this empire will
     * retain as one of it's ship designs, which are those displayed in the GUI
     * list of available designs for human players, and */
    if (ship_design_id == next_design_id)
        return;
    const ShipDesign* ship_design = GetUniverse().GetShipDesign(ship_design_id);
    if (ship_design) {  // don't check if design is producible; adding a ship design is useful for more than just producing it
        // design is valid, so just add the id to empire's set of ids that it knows about
        if (m_ship_designs.find(ship_design_id) == m_ship_designs.end()) {
            m_ship_designs.insert(ship_design_id);

            ShipDesignsChangedSignal();

            TraceLogger() << "AddShipDesign::  " << ship_design->Name() << " (" << ship_design_id
                          << ") to empire #" << EmpireID();
        }
    } else {
        // design in not valid
        ErrorLogger() << "Empire::AddShipDesign(int ship_design_id) was passed a design id that this empire doesn't know about, or that doesn't exist";
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
            int ship_design_id = it->first;
            AddShipDesign(ship_design_id);
            return ship_design_id;
        }
    }

    bool success = universe.InsertShipDesign(ship_design);

    if (!success) {
        ErrorLogger() << "Empire::AddShipDesign Unable to add new design to universe";
        return INVALID_OBJECT_ID;
    }

    auto new_design_id = ship_design->ID();
    AddShipDesign(new_design_id);

    return new_design_id;
}

void Empire::RemoveShipDesign(int ship_design_id) {
    if (m_ship_designs.find(ship_design_id) != m_ship_designs.end()) {
        m_ship_designs.erase(ship_design_id);
        ShipDesignsChangedSignal();
    } else {
        DebugLogger() << "Empire::RemoveShipDesign: this empire did not have design with id " << ship_design_id;
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
        ErrorLogger() << "Empire::LockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::RemoveBuildingType(const std::string& name) {
    auto it = m_available_building_types.find(name);
    if (it == m_available_building_types.end())
        DebugLogger() << "Empire::RemoveBuildingType asked to remove building type " << name << " that was no available to this empire";
    m_available_building_types.erase(name);
}

void Empire::RemovePartType(const std::string& name) {
    auto it = m_available_part_types.find(name);
    if (it == m_available_part_types.end())
        DebugLogger() << "Empire::RemovePartType asked to remove part type " << name << " that was no available to this empire";
    m_available_part_types.erase(name);
}

void Empire::RemoveHullType(const std::string& name) {
    auto it = m_available_hull_types.find(name);
    if (it == m_available_hull_types.end())
        DebugLogger() << "Empire::RemoveHullType asked to remove hull type " << name << " that was no available to this empire";
    m_available_hull_types.erase(name);
}

void Empire::ClearSitRep()
{ m_sitrep_entries.clear(); }

namespace {
    // remove nonexistant / invalid techs from queue
    void SanitizeResearchQueue(ResearchQueue& queue) {
        bool done = false;
        while (!done) {
            auto it = queue.begin();
            while (true) {
                if (it == queue.end()) {
                    done = true;        // got all the way through the queue without finding an invalid tech
                    break;
                } else if (!GetTech(it->name)) {
                    DebugLogger() << "SanitizeResearchQueue for empire " << queue.EmpireID() << " removed invalid tech: " << it->name;
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

    float spent_rp{0.0f};
    float total_rp_available = m_resource_pools[RE_RESEARCH]->TotalAvailable();

    // process items on queue
    std::vector<std::string> to_erase;
    for (auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech) {
            ErrorLogger() << "Empire::CheckResearchProgress couldn't find tech on queue, even after sanitizing!";
            continue;
        }
        float& progress = m_research_progress[elem.name];
        float tech_cost = tech->ResearchCost(m_id);
        progress += elem.allocated_rp / std::max(EPSILON, tech_cost);
        spent_rp += elem.allocated_rp;
        if (tech->ResearchCost(m_id) - EPSILON <= progress * tech_cost)
            AddTech(elem.name);
        if (GetTechStatus(elem.name) == TS_COMPLETE) {
            m_research_progress.erase(elem.name);
            to_erase.push_back(elem.name);
        }
    }

    //DebugLogger() << m_research_queue.Dump();
    float rp_left_to_spend = total_rp_available - spent_rp;
    //DebugLogger() << "leftover RP: " << rp_left_to_spend;
    // auto-allocate any excess RP left over after player-specified queued techs

    // if there are left over RPs, any tech on the queue presumably can't
    // have RP allocated to it
    std::unordered_set<std::string> techs_not_suitable_for_auto_allocation;
    for (auto& elem : m_research_queue)
        techs_not_suitable_for_auto_allocation.insert(elem.name);

    // for all available and suitable techs, store ordered by cost to complete
    std::multimap<double, std::string> costs_to_complete_available_unpaused_techs;
    for (const Tech* tech : GetTechManager()) {
        const std::string& tech_name = tech->Name();
        if (techs_not_suitable_for_auto_allocation.count(tech_name) > 0)
            continue;
        if (this->GetTechStatus(tech_name) != TS_RESEARCHABLE)
            continue;
        if (!tech->Researchable())
            continue;
        double progress = this->ResearchProgress(tech_name);
        double total_cost = tech->ResearchCost(m_id);
        if (progress >= total_cost)
            continue;
        costs_to_complete_available_unpaused_techs.emplace(total_cost - progress, tech_name);
    }

    // in order of minimum additional cost to complete, allocate RP to
    // techs up to available RP and per-turn limits
    for (auto const& cost_tech : costs_to_complete_available_unpaused_techs) {
        if (rp_left_to_spend <= EPSILON)
            break;

        const Tech* tech = GetTech(cost_tech.second);
        if (!tech)
            continue;

        //DebugLogger() << "extra tech: " << cost_tech.second << " needs: " << cost_tech.first << " more RP to finish";

        float RPs_per_turn_limit = tech->PerTurnCost(m_id);
        float tech_total_cost = tech->ResearchCost(m_id);
        float progress_fraction = m_research_progress[cost_tech.second];

        float progress_fraction_left = 1.0f - progress_fraction;
        float max_progress_per_turn = RPs_per_turn_limit / tech_total_cost;
        float progress_possible_with_available_rp = rp_left_to_spend / tech_total_cost;

        //DebugLogger() << "... progress left: " << progress_fraction_left
        //              << " max per turn: " << max_progress_per_turn
        //              << " progress possible with available rp: " << progress_possible_with_available_rp;

        float progress_increase = std::min(
            progress_fraction_left,
            std::min(max_progress_per_turn, progress_possible_with_available_rp));

        float consumed_rp = progress_increase * tech_total_cost;

        m_research_progress[cost_tech.second] += progress_increase;
        rp_left_to_spend -= consumed_rp;

        if (tech->ResearchCost(m_id) - EPSILON <= m_research_progress[cost_tech.second] * tech_total_cost)
            AddTech(cost_tech.second);

        //DebugLogger() << "... allocated: " << consumed_rp << " to increase progress by: " << progress_increase;
    }

    // remove completed items from queue (after consuming extra RP, as that
    // determination uses the contents of the queue as input)
    for (const std::string& tech_name : to_erase) {
        ResearchQueue::iterator temp_it = m_research_queue.find(tech_name);
        if (temp_it != m_research_queue.end())
            m_research_queue.erase(temp_it);
    }

    // can uncomment following line when / if research stockpiling is enabled...
    // m_resource_pools[RE_RESEARCH]->SetStockpile(m_resource_pools[RE_RESEARCH]->TotalAvailable() - m_research_queue.TotalRPsSpent());
}

void Empire::CheckProductionProgress() {
    DebugLogger() << "========Empire::CheckProductionProgress=======";
    // following commented line should be redundant, as previous call to
    // UpdateResourcePools should have generated necessary info
    // m_production_queue.Update();

    Universe& universe = GetUniverse();

    std::map<int, std::vector<std::shared_ptr<Ship>>> system_new_ships;
    std::map<int, int> new_ship_rally_point_ids;

    // preprocess the queue to get all the costs and times of all items
    // at every location at which they are being produced,
    // before doing any generation of new objects or other modifications
    // of the gamestate. this will ensure that the cost of items doesn't
    // change while the queue is being processed, so that if there is
    // sufficent PP to complete an object at the start of a turn,
    // items above it on the queue getting finished don't increase the
    // cost and result in it not being finished that turn.
    std::map<std::pair<ProductionQueue::ProductionItem, int>, std::pair<float, int>>
        queue_item_costs_and_times;
    for (auto& elem : m_production_queue) {
        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        auto key = std::make_pair(elem.item, location_id);

        if (queue_item_costs_and_times.find(key) == queue_item_costs_and_times.end())
            queue_item_costs_and_times[key] = ProductionCostAndTime(elem);
    }

    //for (auto& entry : queue_item_costs_and_times)
    //{ DebugLogger() << entry.first.first.design_id << " : " << entry.second.first; }


    // go through queue, updating production progress.  If a production item is
    // completed, create the produced object or take whatever other action is
    // appropriate, and record that queue item as complete, so it can be erased
    // from the queue
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        auto& elem = m_production_queue[i];
        float item_cost;
        int build_turns;

        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        std::pair<ProductionQueue::ProductionItem, int> key(elem.item, location_id);

        std::tie(item_cost, build_turns) = queue_item_costs_and_times[key];
        if (item_cost < 0.01f || build_turns < 1) {
            ErrorLogger() << "Empire::CheckProductionProgress got strang cost/time: " << item_cost << " / " << build_turns;
            break;
        }

        item_cost *= elem.blocksize;

        DebugLogger() << "elem: " << elem.Dump();
        DebugLogger() << "   allocated: " << elem.allocated_pp;
        DebugLogger() << "   initial progress: " << elem.progress;

        elem.progress += elem.allocated_pp / std::max(EPSILON, item_cost);  // add progress for allocated PP to queue item
        elem.progress_memory = elem.progress;
        elem.blocksize_memory = elem.blocksize;

        DebugLogger() << "   updated progress: " << elem.progress;
        DebugLogger() << " ";

        std::string build_description;
        switch (elem.item.build_type) {
            case BT_BUILDING: {
                build_description = "BuildingType " + elem.item.name;
                break;
            }
            case BT_SHIP: {
                build_description = "Ships(s) with design id " + std::to_string(elem.item.design_id);
                break;
            }
            default: 
                build_description = "unknown build type";
        }

        auto build_location = GetUniverseObject(elem.location);
        if (!build_location || (elem.item.build_type == BT_BUILDING && build_location->ObjectType() != OBJ_PLANET)) {
            ErrorLogger() << "Couldn't get valid build location for completed " << build_description;
            continue;
        }
        auto system = GetSystem(build_location->SystemID());
        // TODO: account for shipyards and/or other ship production
        // sites that are in interstellar space, if needed
        if (!system) {
            ErrorLogger() << "Empire::CheckProductionProgress couldn't get system for producing new " << build_description;
            continue;
        }

        // check location condition before each item is created, so
        // that items being produced can prevent subsequent
        // completions on the same turn from going through
        if (!this->ProducibleItem(elem.item, elem.location)) {
            DebugLogger() << "Location test failed for " << build_description << " at location " << build_location->Name();
            continue;
        }


        // only if accumulated PP is sufficient, the item can be completed
        if (item_cost - EPSILON > elem.progress*item_cost)
            continue;


        // only if consumed resources are available, then item can be completd
        bool consumption_impossible = false;
        std::map<std::string, std::map<int, float>> sc = elem.item.CompletionSpecialConsumption(elem.location);
        for (auto& special_type : sc) {
            if (consumption_impossible)
                break;
            for (auto& special_meter : special_type.second) {
                auto obj = GetUniverseObject(special_meter.first);
                float capacity = obj ? obj->SpecialCapacity(special_type.first) : 0.0f;
                if (capacity < special_meter.second * elem.blocksize) {
                    consumption_impossible = true;
                    break;
                }
            }
        }
        auto mc = elem.item.CompletionMeterConsumption(elem.location);
        for (auto& meter_type : mc) {
            if (consumption_impossible)
                break;
            for (auto& object_meter : meter_type.second) {
                auto obj = GetUniverseObject(object_meter.first);
                const Meter* meter = obj ? obj->GetMeter(meter_type.first) : nullptr;
                if (!meter || meter->Current() < object_meter.second * elem.blocksize) {
                    consumption_impossible = true;
                    break;
                }
            }
        }
        if (consumption_impossible)
            continue;


        // deduct progress for complete item from accumulated progress, so that next
        // repetition can continue accumulating PP, but don't set progress to 0, as
        // this way overflow progress / PP allocated this turn can be used for the
        // next repetition of the item.
        elem.progress -= 1.0f;
        if (elem.progress < 0.0f) {
            if (elem.progress < -1e-3)
                ErrorLogger() << "Somehow got negative progress (" << elem.progress
                              << ") after deducting progress for completed item...";
            elem.progress = 0.0f;
        }

        elem.progress_memory = elem.progress;
        DebugLogger() << "Completed an item: " << elem.item.name;


        // consume the item's special and meter consumption
        for (auto& special_type : sc) {
            for (auto& special_meter : special_type.second) {
                auto obj = GetUniverseObject(special_meter.first);
                if (!obj)
                    continue;
                if (!obj->HasSpecial(special_type.first))
                    continue;
                float cur_capacity = obj->SpecialCapacity(special_type.first);
                float new_capacity = std::max(0.0f, cur_capacity - special_meter.second * elem.blocksize);
                obj->SetSpecialCapacity(special_type.first, new_capacity);
            }
        }
        for (auto& meter_type : mc) {
            for (const auto& object_meter : meter_type.second) {
                auto obj = GetUniverseObject(object_meter.first);
                if (!obj)
                    continue;
                Meter*meter = obj->GetMeter(meter_type.first);
                if (!meter)
                    continue;
                float cur_meter = meter->Current();
                float new_meter = cur_meter - object_meter.second * elem.blocksize;
                meter->SetCurrent(new_meter);
                meter->BackPropagate();
            }
        }


        // create actual thing(s) being produced
        switch (elem.item.build_type) {
        case BT_BUILDING: {
            auto planet = GetPlanet(elem.location);

            // create new building
            auto building = universe.InsertNew<Building>(m_id, elem.item.name, m_id);
            planet->AddBuilding(building->ID());
            building->SetPlanetID(planet->ID());
            system->Insert(building);

            // record building production in empire stats
            if (m_building_types_produced.find(elem.item.name) != m_building_types_produced.end())
                m_building_types_produced[elem.item.name]++;
            else
                m_building_types_produced[elem.item.name] = 1;

            AddSitRepEntry(CreateBuildingBuiltSitRep(building->ID(), planet->ID()));
            DebugLogger() << "New Building created on turn: " << CurrentTurn();
            break;
        }

        case BT_SHIP: {
            if (elem.blocksize < 1)
                break;   // nothing to do!

            // get species for this ship.  use popcenter species if build
            // location is a popcenter, or use ship species if build
            // location is a ship, or use empire capital species if there
            // is a valid capital, or otherwise ???
            // TODO: Add more fallbacks if necessary
            std::string species_name;
            if (auto location_pop_center = std::dynamic_pointer_cast<const PopCenter>(build_location))
                species_name = location_pop_center->SpeciesName();
            else if (auto location_ship = std::dynamic_pointer_cast<const Ship>(build_location))
                species_name = location_ship->SpeciesName();
            else if (auto capital_planet = GetPlanet(this->CapitalID()))
                species_name = capital_planet->SpeciesName();
            // else give up...
            if (species_name.empty()) {
                // only really a problem for colony ships, which need to have a species to function
                const auto* design = GetShipDesign(elem.item.design_id);
                if (!design) {
                    ErrorLogger() << "Couldn't get ShipDesign with id: " << elem.item.design_id;
                    break;
                }
                if (design->CanColonize()) {
                    ErrorLogger() << "Couldn't get species in order to make colony ship!";
                    break;
                }
            }

            std::shared_ptr<Ship> ship;

            for (int count = 0; count < elem.blocksize; count++) {
                // create ship
                ship = universe.InsertNew<Ship>(m_id, elem.item.design_id, species_name, m_id);
                system->Insert(ship);

                // record ship production in empire stats
                if (m_ship_designs_produced.find(elem.item.design_id) != m_ship_designs_produced.end())
                    m_ship_designs_produced[elem.item.design_id]++;
                else
                    m_ship_designs_produced[elem.item.design_id] = 1;
                if (m_species_ships_produced.find(species_name) != m_species_ships_produced.end())
                    m_species_ships_produced[species_name]++;
                else
                    m_species_ships_produced[species_name] = 1;


                // set active meters that have associated max meters to an
                // initial very large value, so that when the active meters are
                // later clamped, they will equal the max meter after effects
                // have been applied, letting new ships start with maxed
                // everything that is traced with an associated max meter.
                ship->SetShipMetersToMax();
                ship->BackPropagateMeters();

                ship->Rename(NewShipName());

                // store ships to put into fleets later
                system_new_ships[system->ID()].push_back(ship);

                // store ship rally points
                if (elem.rally_point_id != INVALID_OBJECT_ID)
                    new_ship_rally_point_ids[ship->ID()] = elem.rally_point_id;
            }
            // add sitrep
            if (elem.blocksize == 1) {
                AddSitRepEntry(CreateShipBuiltSitRep(ship->ID(), system->ID(), ship->DesignID()));
                DebugLogger() << "New Ship, id " << ship->ID() << ", created on turn: " << ship->CreationTurn();
            } else {
                AddSitRepEntry(CreateShipBlockBuiltSitRep(system->ID(), ship->DesignID(), elem.blocksize));
                DebugLogger() << "New block of "<< elem.blocksize << " ships created on turn: " << ship->CreationTurn();
            }
            break;
        }

        default:
            ErrorLogger() << "Build item of unknown build type finished on production queue.";
            break;
        }

        if (!--m_production_queue[i].remaining) {   // decrement number of remaining items to be produced in current queue element
            to_erase.push_back(i);                  // remember completed element so that it can be removed from queue
            DebugLogger() << "Marking completed production queue item to be removed from queue";
        }
    }

    // create fleets for new ships and put ships into fleets
    for (auto& entry : system_new_ships) {
        auto system = GetSystem(entry.first);
        if (!system) {
            ErrorLogger() << "Couldn't get system with id " << entry.first << " for creating new fleets for newly produced ships";
            continue;
        }

        auto& new_ships = entry.second;
        if (new_ships.empty())
            continue;

        // group ships into fleets by rally point and design
        std::map<int, std::map<int, std::vector<std::shared_ptr<Ship>>>>
            new_ships_by_rally_point_id_and_design_id;
        for (auto& ship : new_ships) {
            int rally_point_id = INVALID_OBJECT_ID;

            auto rally_it = new_ship_rally_point_ids.find(ship->ID());
            if (rally_it != new_ship_rally_point_ids.end())
                rally_point_id = rally_it->second;

            new_ships_by_rally_point_id_and_design_id[rally_point_id][ship->DesignID()].push_back(ship);
        }

        // create fleets for ships with the same rally point, grouped by
        // ship design
        // Do not group unarmed ships with no troops (i.e. scouts and
        // colony ships).
        for (auto& entry : new_ships_by_rally_point_id_and_design_id) {
            int rally_point_id = entry.first;
            auto& new_ships_by_design = entry.second;

            for (auto& ships_by_design : new_ships_by_design) {
                std::vector<int> ship_ids;

                auto& ships = ships_by_design.second;
                if (ships.empty())
                    continue;

                // create a single fleet for combat ships and individual
                // fleets for non-combat ships
                bool individual_fleets = !((*ships.begin())->IsArmed()
                                           || (*ships.begin())->HasFighters()
                                           || (*ships.begin())->CanHaveTroops()
                                           || (*ships.begin())->CanBombard());

                std::vector<std::shared_ptr<Fleet>> fleets;
                std::shared_ptr<Fleet> fleet;

                if (!individual_fleets) {
                    fleet = universe.InsertNew<Fleet>("", system->X(), system->Y(), m_id);

                    system->Insert(fleet);
                    fleet->SetNextAndPreviousSystems(system->ID(), system->ID());

                    fleets.push_back(fleet);
                }

                for (auto& ship : ships) {
                    if (individual_fleets) {
                        fleet = universe.InsertNew<Fleet>("", system->X(), system->Y(), m_id);

                        system->Insert(fleet);
                        fleet->SetNextAndPreviousSystems(system->ID(), system->ID());

                        fleets.push_back(fleet);
                    }
                    ship_ids.push_back(ship->ID());
                    fleet->AddShip(ship->ID());
                    ship->SetFleetID(fleet->ID());
                }

                for (auto& fleet : fleets) {
                    // rename fleet, given its id and the ship that is in it
                    fleet->Rename(fleet->GenerateFleetName());
                    fleet->SetAggressive(fleet->HasArmedShips() || fleet->HasFighterShips());

                    if (rally_point_id != INVALID_OBJECT_ID) {
                        if (GetSystem(rally_point_id)) {
                            fleet->CalculateRouteTo(rally_point_id);
                        } else if (auto rally_obj = GetUniverseObject(rally_point_id)) {
                            if (GetSystem(rally_obj->SystemID()))
                                fleet->CalculateRouteTo(rally_obj->SystemID());
                        } else {
                            ErrorLogger() << "Unable to find system to route to with rally point id: " << rally_point_id;
                        }
                    }

                    DebugLogger() << "New Fleet \"" + fleet->Name() + "\" created on turn: " << fleet->CreationTurn();
                }
            }
        }
    }

    // removed completed items from queue
    for (auto it = to_erase.rbegin(); it != to_erase.rend(); ++it)
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
    // get this empire's owned resource centers and ships (which can both produce resources)
    std::vector<int> res_centers;
    res_centers.reserve(Objects().ExistingResourceCenters().size());
    for (const auto& entry : Objects().ExistingResourceCenters()) {
        if (!entry.second->OwnedBy(m_id))
            continue;
        res_centers.push_back(entry.first);
    }
    for (const auto& entry : Objects().ExistingShips()) {
        if (!entry.second->OwnedBy(m_id))
            continue;
        res_centers.push_back(entry.first);
    }
    m_resource_pools[RE_RESEARCH]->SetObjects(res_centers);
    m_resource_pools[RE_INDUSTRY]->SetObjects(res_centers);
    m_resource_pools[RE_TRADE]->SetObjects(res_centers);

    // get this empire's owned population centers
    std::vector<int> pop_centers;
    pop_centers.reserve(Objects().ExistingPopCenters().size());
    for (const auto& entry : Objects().ExistingPopCenters()) {
        if (entry.second->OwnedBy(m_id))
            pop_centers.push_back(entry.first);
    }
    m_population_pool.SetPopCenters(pop_centers);


    // inform the blockadeable resource pools about systems that can share
    m_resource_pools[RE_INDUSTRY]->SetConnectedSupplyGroups(GetSupplyManager().ResourceSupplyGroups(m_id));

    // set non-blockadeable resource pools to share resources between all systems
    std::set<std::set<int>> sets_set;
    std::set<int> all_systems_set;
    for (const auto& entry : Objects().ExistingSystems()) {
        all_systems_set.insert(entry.first);
    }
    sets_set.insert(all_systems_set);
    m_resource_pools[RE_RESEARCH]->SetConnectedSupplyGroups(sets_set);
    m_resource_pools[RE_TRADE]->SetConnectedSupplyGroups(sets_set);


    // set stockpile object locations for each resource, ensuring those systems exist
    //static std::vector<ResourceType> res_type_vec {RE_INDUSTRY, RE_TRADE, RE_RESEARCH};
    for (ResourceType res_type : {RE_INDUSTRY, RE_TRADE, RE_RESEARCH}) {
        int stockpile_object_id = INVALID_OBJECT_ID;
        if (auto stockpile_obj = GetUniverseObject(StockpileID(res_type)))
            stockpile_object_id = stockpile_obj->ID();
        m_resource_pools[res_type]->SetStockpileObject(stockpile_object_id);
    }
}

void Empire::UpdateResourcePools() {
    // updating queues, allocated_rp, distribution and growth each update their
    // respective pools, (as well as the ways in which the resources are used,
    // which needs to be done simultaneously to keep things consistent)
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
    DebugLogger() << "========= Production Update for empire: " << EmpireID() << " ========";

    m_resource_pools[RE_INDUSTRY]->Update();
    m_production_queue.Update();
    m_resource_pools[RE_INDUSTRY]->ChangedSignal();
}

void Empire::UpdateTradeSpending() {
    m_resource_pools[RE_TRADE]->Update(); // recalculate total trade production
    m_resource_pools[RE_TRADE]->ChangedSignal();
}

void Empire::UpdatePopulationGrowth()
{ m_population_pool.Update(); }

void Empire::ResetMeters() {
    for (auto& entry : m_meters) {
        entry.second.ResetCurrent();
    }
}

void Empire::UpdateOwnedObjectCounters() {
    // ships of each species and design
    m_species_ships_owned.clear();
    m_ship_designs_owned.clear();
    for (const auto& entry : Objects().ExistingShips()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        auto ship = std::dynamic_pointer_cast<const Ship>(entry.second);
        if (!ship)
            continue;
        if (!ship->SpeciesName().empty())
            m_species_ships_owned[ship->SpeciesName()]++;
        m_ship_designs_owned[ship->DesignID()]++;
    }

    // update ship part counts
    m_ship_part_types_owned.clear();
    m_ship_part_class_owned.clear();
    for (const auto& design_count : m_ship_designs_owned) {
        const ShipDesign* design = GetShipDesign(design_count.first);
        if (!design)
            continue;

        // update count of PartTypes
        for (const auto& part_type : design->PartTypeCount())
            m_ship_part_types_owned[part_type.first] += part_type.second * design_count.second;

        // update count of ShipPartClasses
        for (const auto& part_class : design->PartClassCount())
            m_ship_part_class_owned[part_class.first] += part_class.second * design_count.second;
    }

    // colonies of each species, and unspecified outposts
    m_species_colonies_owned.clear();
    m_outposts_owned = 0;
    for (const auto& entry : Objects().ExistingPlanets()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        auto planet = std::dynamic_pointer_cast<const Planet>(entry.second);
        if (!planet)
            continue;
        if (planet->SpeciesName().empty())
            m_outposts_owned++;
        else
            m_species_colonies_owned[planet->SpeciesName()]++;
    }

    // buildings of each type
    m_building_types_owned.clear();
    for (const auto& entry : Objects().ExistingBuildings()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        auto building = std::dynamic_pointer_cast<const Building>(entry.second);
        if (!building)
            continue;
        m_building_types_owned[building->BuildingTypeName()]++;
    }
}

int Empire::TotalShipsOwned() const {
    // sum up counts for each ship design owned by this empire
    // (not using species ship counts, as an empire could potentially own a
    //  ship that has no species...)
    int counter = 0;
    for (const auto& entry : m_ship_designs_owned)
    { counter += entry.second; }
    return counter;
}

int Empire::TotalShipPartsOwned() const {
    // sum counts of all ship parts owned by this empire
    int retval = 0;

    for (const auto& part_class : m_ship_part_class_owned)
        retval += part_class.second;

    return retval;
}

int Empire::TotalBuildingsOwned() const {
    // sum up counts for each building type owned by this empire
    int counter = 0;
    for (const auto& entry : m_building_types_owned)
    { counter += entry.second; }
    return counter;
}
