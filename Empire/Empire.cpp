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
#include "boost/date_time/posix_time/posix_time.hpp"

#undef ORIG_RES_SIMULATOR
#define  DP_RES_QUEUE_SIMULATOR
#undef  ORIG_SIMULATOR
#define DP_QUEUE_SIMULATOR
#undef COMPARE_SIMS



namespace {
    const double EPSILON = 1.0e-5;

    /** sets the .allocated_rp, value for each Tech in the queue.  Only sets
      * nonzero funding to a Tech if it is researchable this turn.  Also
      * determines total number of spent RP (returning by reference in
      * total_RPs_spent) */
    void SetTechQueueElementSpending(double RPs, const std::map<std::string, double>& research_progress,
                                     const std::map<std::string, TechStatus>& research_status,
                                     ResearchQueue::QueueType& queue,
                                     double& total_RPs_spent, int& projects_in_progress, int empire_id)
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
                double RPs_needed = tech->ResearchCost(empire_id) - progress;
                double RPs_per_turn_limit = tech->PerTurnCost(empire_id);
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

    /** Sets the allocated_pp value for each Element in the passed
      * ProductionQueue \a queue.  Elements are allocated PP based on their need,
      * the limits they can be given per turn, and the amount available at their
      * production location (which is itself limited by the resource supply
      * system groups that are able to exchange resources with the build
      * location and the amount of minerals and industry produced in the group).
      * Elements will not receive funding if they cannot be produced by the
      * empire with the indicated \a empire_id this turn at their build location. */
    void SetProdQueueElementSpending(std::map<std::set<int>, double> available_pp,
                                     const std::vector<std::set<int> >& queue_element_resource_sharing_object_groups,
                                     const std::vector<double>& production_status, ProductionQueue::QueueType& queue,
                                     std::map<std::set<int>, double>& allocated_pp, int& projects_in_progress,
                                     int empire_id)
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
        const Empire* empire = Empires().Lookup(empire_id);
        if (!empire)
            return;


        int i = 0;
        for (ProductionQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            ProductionQueue::Element& queue_element = *it;

            // get resource sharing group and amount of resource available to build this item
            const std::set<int>& group = queue_element_resource_sharing_object_groups[i];
            if (group.empty()) {
                Logger().debugStream() << "resource sharing group for queue element is empty.  not allocating any resources to element";
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

            // see if item is buildable this turn...
            if (!empire->BuildableItem(queue_element.item, queue_element.location)) {
                // can't be built at this location this turn.
                queue_element.allocated_pp = 0.0;
                Logger().debugStream() << "item can't be built at location this turn";
                continue;
            }


            // get max contribution per turn and turns to build at max contribution rate
            double item_cost;
            int build_turns;
            boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(queue_element);
            //Logger().debugStream() << "item " << queue_element.item.name << " costs " << item_cost << " for " << build_turns << " turns";

            item_cost *= queue_element.blocksize;
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

            if (allocation > 0.0)
                ++projects_in_progress;
        }
    }

    void LoadShipNames(std::vector<std::string>& names) {
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
}

////////////////////////////////////////
// ResearchQueue                      //
////////////////////////////////////////
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

void ResearchQueue::Update(double RPs, const std::map<std::string, double>& research_progress) {
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

    if ( RPs <= EPSILON ) {
        ResearchQueueChangedSignal();
        return;    // nothing more to do if not enough RP...
    }

#ifdef  DP_RES_QUEUE_SIMULATOR
    boost::posix_time::ptime dp_time_start;
    boost::posix_time::ptime dp_time_end;
#ifdef ORIG_RES_SIMULATOR
    long dp_time;
#endif

    // "Dynamic Programming" version of research queue simulator -- copy the queue simulator containers
    // perform dynamic programming calculation of completion times, then after regular simulation is done compare results (if both enabled)

    //record original order & progress
    // will take advantage of fact that sets (& map keys) are by default kept in sorted order lowest to highest
    std::map<std::string, double> dp_prog = research_progress;
    std::map< std::string, int > origQueueOrder;
    std::map<int, double> dpsim_research_progress;
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        std::string tname = m_queue[i].name;
        origQueueOrder[ tname ] = i;
        dpsim_research_progress[i] = dp_prog[ tname ];
    }

    std::map<std::string, TechStatus> dpsim_tech_status_map = sim_tech_status_map;

    // initialize simulation_results with -1 for all techs, so that any techs that aren't
    // finished in simulation by turn TOO_MANY_TURNS will be left marked as never to be finished
    std::vector<int>  dpsimulation_results(m_queue.size(), -1);

    const int DP_TURNS = TOO_MANY_TURNS; // track up to this many turns
#ifdef ORIG_RES_SIMULATOR
    dp_time_start = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
#endif
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
    std::vector<double> rpStillAvailable(DP_TURNS, RPs );  // initialize to the  full RP allocation for every turn

    while ((dpturns < DP_TURNS ) && !(dpResearchableTechs.empty() ) ) {// if we haven't used up our turns and still have techs to process
        ++dpturns;
        std::map<int, bool> alreadyProcessed;
        std::set<int>::iterator curTechIt;
        for (curTechIt= dpResearchableTechs.begin(); curTechIt!=dpResearchableTechs.end(); ++curTechIt) {
            alreadyProcessed[ *curTechIt ] = false;
        }
        curTechIt = dpResearchableTechs.begin();
        while ((rpStillAvailable[dpturns-1]> EPSILON)) { // try to use up this turns RPs
            if ( curTechIt==dpResearchableTechs.end() ) {
                break; //will be wasting some RP this turn
            }
            int curTech = *curTechIt;
            if ( alreadyProcessed[curTech ] ) {
                ++curTechIt;
                continue;
            }
            alreadyProcessed[curTech] = true;
            const std::string& tech_name = m_queue[curTech].name;
            const Tech* tech = GetTech(tech_name);
            double progress = dpsim_research_progress[curTech];
            double RPs_needed = tech ? tech->ResearchCost(m_empire_id) - progress : 0.0;
            double RPs_per_turn_limit = tech ? tech->PerTurnCost(m_empire_id) : 1.0;
            double RPs_to_spend = std::min( std::min(RPs_needed, RPs_per_turn_limit), rpStillAvailable[dpturns-1] );
            progress += RPs_to_spend;
            dpsim_research_progress[curTech] = progress;
            rpStillAvailable[dpturns-1] -= RPs_to_spend;
            std::set<int>::iterator nextResTechIt = curTechIt;
            int nextResTechIdx;
            if ( ++nextResTechIt == dpResearchableTechs.end() ) {
                nextResTechIdx = m_queue.size()+1;
            } else {
                nextResTechIdx = *(nextResTechIt);
            }
            double tech_cost = tech ? tech->ResearchCost(m_empire_id) : 0.0;
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

#ifdef ORIG_RES_SIMULATOR
        dp_time_end = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
        dp_time = (dp_time_end - dp_time_start).total_nanoseconds();
        dp_time = dp_time; // just to suppresss the compiler warning of unused var if the comparisons are not being done below.
#endif
        //dp_time = dpsim_queue_timer.elapsed() * 1000;
        // Logger().debugStream() << "ProductionQueue::Update queue dynamic programming sim time: " << dpsim_queue_timer.elapsed() * 1000.0;
    } // while ((dpturns < DP_TURNS ) && !(dpResearchableTechs.empty() ) )        
#endif //DP_RES_QUEUE_SIMULATOR

#ifdef ORIG_RES_SIMULATOR
#ifdef DP_RES_QUEUE_SIMULATOR
    boost::posix_time::ptime orig_time_start;
    boost::posix_time::ptime orig_time_end;
    long orig_time;
    orig_time_start = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
#endif
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
    
#ifdef DP_RES_QUEUE_SIMULATOR  // if both simulations were done, compare results
orig_time_end = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
orig_time = (orig_time_end - orig_time_start).total_nanoseconds();
orig_time = orig_time; // just to suppresss the compiler warning of unused var if the comparisons are not being done below.

#ifdef COMPARE_SIMS
    bool sameResults = true;
    for (unsigned int i = 0; i< m_queue.size(); ++i ) {
        if (m_queue[i].turns_left != dpsimulation_results[i]) {
            sameResults = false;
            break;
        }
    }
    if (sameResults ) {
        Logger().debugStream() << "ResearchQueue::Update orig sim and dp_sim gave same results";
        Logger().debugStream() << "ResearchQueue::Update orig sim time: " << orig_time*1e-3 << " , dp sim time: " << dp_time*1.0e-3 << " micro sec";
    } else  {
        Logger().debugStream() << "ResearchQueue::Update orig sim and dp_sim gave different results";
        Logger().debugStream() << "ResearchQueue::Update orig sim time: " << orig_time*1e-3 << " , dp sim time: " << dp_time*1.0e-3 << " micro sec";
        for (unsigned int i = 0; i< m_queue.size(); ++i ) {
            Logger().debugStream() << "ResearchQueue::Update    Queue Item: " << m_queue[i].name;
            Logger().debugStream() << "ResearchQueue::Update         orig sim gave next: " << m_queue[i].turns_left;
            Logger().debugStream() << "ResearchQueue::Update          dp  sim gave next: " << dpsimulation_results[i];
        }
    } 
#endif //  COMPARE_SIMS
#endif //  DP_RES_QUEUE_SIMULATOR
#endif //ORIG_RES_SIMULATOR

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
    blocksize(1),
    remaining(0),
    location(INVALID_OBJECT_ID),
    allocated_pp(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(ProductionItem item_, int ordered_, int remaining_, int location_) :
    item(item_),
    ordered(ordered_),
    blocksize(1),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, std::string name, int ordered_, int remaining_, int location_) :
    item(build_type, name),
    ordered(ordered_),
    blocksize(1),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, int design_id, int ordered_, int remaining_, int location_) :
    item(build_type, design_id),
    ordered(ordered_),
    blocksize(1),
    remaining(remaining_),
    location(location_),
    allocated_pp(0.0),
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

double ProductionQueue::TotalPPsSpent() const {
    // add up allocated PP from all resource sharing object groups
    double retval = 0;
    for (std::map<std::set<int>, double>::const_iterator it = m_object_group_allocated_pp.begin(); it != m_object_group_allocated_pp.end(); ++it)
        retval += it->second;
    return retval;
}

std::map<std::set<int>, double> ProductionQueue::AvailablePP(const std::map<ResourceType,
                                                             boost::shared_ptr<ResourcePool> >& resource_pools) const
{
    std::map<std::set<int>, double> available_pp;

    // get resource pools used for production...
    boost::shared_ptr<ResourcePool> industry_pool;
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator pool_it = resource_pools.find(RE_INDUSTRY);
    if (pool_it != resource_pools.end()) {
        industry_pool = pool_it->second;
    } else {
        Logger().errorStream() << "ProductionQueue::AvailablePP couldn't get an industry resource pool from passed resource pools";
        return available_pp;
    }


    // determine available PP in each resource sharing group of systems for this empire.  PP are available
    // industry in each resource-sharing group of systems
    std::map<std::set<int>, double> available_industry = industry_pool->Available();


    for (std::map<std::set<int>, double>::const_iterator ind_it = available_industry.begin(); ind_it != available_industry.end(); ++ind_it) {
        // get group of systems in industry pool
        const std::set<int>& group = ind_it->first;

        available_pp[group] = ind_it->second;
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

const ProductionQueue::Element& ProductionQueue::operator[](int i) const {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ProductionQueue::const_iterator ProductionQueue::UnderfundedProject() const {
    const Empire* empire = Empires().Lookup(m_empire_id);
    if (!empire)
        return end();
    for (const_iterator it = begin(); it != end(); ++it) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(*it);
        item_cost *= it->blocksize;
        double maxPerTurn = item_cost / std::max(build_turns,1);
        if (it->allocated_pp && (it->allocated_pp < (maxPerTurn-EPSILON)) && (1 < it->turns_left_to_next_item) )
            return it;
    }
    return end();
}

void ProductionQueue::Update(const std::map<ResourceType,
                             boost::shared_ptr<ResourcePool> >& resource_pools,
                             const std::vector<double>& production_status)
{
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
    SetProdQueueElementSpending(available_pp, queue_element_groups, production_status,
                                m_queue, m_object_group_allocated_pp, m_projects_in_progress, m_empire_id);


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
    std::vector< unsigned int>            sim_queue_original_indices(sim_production_status.size());
    for (unsigned int i = 0; i < sim_queue_original_indices.size(); ++i)
        sim_queue_original_indices[i] = i;


    const int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops
    const double TOO_LONG_TIME = 0.1;   // max time in ms to spend simulating queue


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
#ifdef DP_QUEUE_SIMULATOR
    boost::posix_time::ptime dp_time_start;
    boost::posix_time::ptime dp_time_end;
    long dp_time;

    // "Dynamic Programming" version of queue simulator -- copy the queue simulator containers at this point, after removal of unbuildable items,
    // perform dynamic programming calculation of completion times, then after regular simulation is done compare results

    //init production queue to 'never' status
    for (ProductionQueue::QueueType::iterator queue_it = m_queue.begin(); queue_it != m_queue.end(); ++queue_it) {
        queue_it->turns_left_to_next_item = -1;     // -1 is sentinel value indicating never to be complete.  ProductionWnd checks for turns to completeion less than 0 and displays "NEVER" when appropriate
        queue_it->turns_left_to_completion = -1;
    }

    // duplicate simulation production queue state (post-bad-item-removal) for dynamic programming
    QueueType                   dpsim_queue = sim_queue;
    std::vector<double>         dpsim_production_status = sim_production_status;
    //std::vector<std::set<int> > sim_queue_element_groups = queue_element_groups;  //not necessary to duplicate this since won't be further modified
    std::vector<int>            dpsimulation_results_to_next(production_status.size(), -1);
    std::vector<int>            dpsimulation_results_to_completion(production_status.size(), -1);
    std::vector<unsigned int>            dpsim_queue_original_indices(sim_queue_original_indices); 

    const unsigned int DP_TURNS = TOO_MANY_TURNS; // track up to this many turns
    const double DP_TOO_LONG_TIME = TOO_LONG_TIME;   // max time in ms to spend simulating queue

    // The DP version will do calculations for one resource group at a time
    // unfortunately need to copy code from SetProdQueueElementSpending  in order to work it in more efficiently here
    dp_time_start = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 

    //invert lookup direction of sim_queue_element_groups:
    std::map< std::set<int>, std::vector<int>  > elementsByGroup;
    for (unsigned int i = 0; i < dpsim_queue.size(); ++i) {
        elementsByGroup[ sim_queue_element_groups[i] ].push_back( i );
    }

    for (std::map<std::set<int>, double>::const_iterator groups_it = available_pp.begin(); groups_it != available_pp.end(); ++groups_it) {
        unsigned int firstTurnPPAvailable = 1; //the first turn any pp in this resource group is available to the next item for this group
        unsigned int turnJump = 0;
        //ppStillAvailable[turn-1] gives the PP still available in this resource pool at turn "turn"
        std::vector<double> ppStillAvailable(DP_TURNS, groups_it->second );  // initialize to the groups full PP allocation for each turn modeled

        std::vector<int> &thisGroupsElements = elementsByGroup[ groups_it->first ];
        std::vector<int>::const_iterator groupBegin = thisGroupsElements.begin();
        std::vector<int>::const_iterator groupEnd = thisGroupsElements.end();

        // cycle through items on queue, if in this resource group then allocate production costs over time against those available to group
        for (std::vector<int>::const_iterator el_it = groupBegin; 
             ( el_it != groupEnd ) &&  ((boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time())-dp_time_start).total_microseconds()*1e-6 < DP_TOO_LONG_TIME) ; ++el_it) {
            firstTurnPPAvailable += turnJump;
            turnJump = 0;
            if (firstTurnPPAvailable > DP_TURNS )
                break; // this resource group is allocated-out for span of simulation; remaining items in group left as never completing

            unsigned int i = *el_it;
            ProductionQueue::Element& element = dpsim_queue[i];
            double item_cost;
            int build_turns;
            boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(element);
            item_cost *= element.blocksize;
            double element_accumulated_PP = dpsim_production_status[i];                                   // PP accumulated by this element towards building next item
            double element_total_cost = item_cost * element.remaining;                        // total PP to build all items in this element
            double element_per_turn_limit = item_cost / std::max(build_turns, 1);
            double additional_pp_to_complete_element = element_total_cost - element_accumulated_PP; // additional PP, beyond already-accumulated PP, to build all items in this element

            unsigned int max_turns = std::max( std::max(build_turns, 1), 1+int(additional_pp_to_complete_element/ppStillAvailable[firstTurnPPAvailable-1]));
            max_turns = std::min( max_turns, DP_TURNS - firstTurnPPAvailable +1 );

            double allocation;
            //Logger().debugStream() << "ProductionQueue::Update Queue index   Queue Item: " << element.item.name;

            for (unsigned int j = 0; j < max_turns; j++ ) {  //iterate over the turns necessary to complete item
                // determine how many pp to allocate to this queue element this turn.  allocation is limited by the
                // item cost, which is the max number of PP per turn that can be put towards this item, and by the
                // total cost remaining to complete the last item in the queue element (eg. the element has all but
                // the last item complete already) and by the total pp available in this element's production location's
                // resource sharing group
                allocation = std::min( std::min( additional_pp_to_complete_element, element_per_turn_limit), ppStillAvailable[firstTurnPPAvailable+j-1] );
                allocation = std::max( allocation, 0.0);       // added max (..., 0.0) to prevent any negative-allocation bugs that might come up...
                dpsim_production_status[i] +=allocation;  // add turn's allocation
                ppStillAvailable[firstTurnPPAvailable+j-1] -= allocation;
                if (ppStillAvailable[firstTurnPPAvailable+j-1] <= EPSILON ) {
                    ppStillAvailable[firstTurnPPAvailable+j-1] = 0;
                    ++turnJump;
                }

                // check if additional turn's PP allocation was enough to finish next item in element
                if (item_cost - EPSILON <= dpsim_production_status[i] ) {
                    // an item has been completed. 
                    // deduct cost of one item from accumulated PP.  don't set
                    // accumulation to zero, as this would eliminate any partial
                    // completion of the next item
                    dpsim_production_status[i] -= item_cost;
                    --element.remaining;  //pretty sure this just effects the dp version & should do even if also doing ORIG_SIMULATOR

// see ~90 lines up for define or undef statement
#ifndef ORIG_SIMULATOR
                    //Logger().debugStream() << "ProductionQueue::Recording DP sim results for item " << element.item.name;

                    // if this was the first item in the element to be completed in
                    // this simuation, update the original queue element with the
                    // turns required to complete the next item in the element
                    if (element.remaining +1 == m_queue[sim_queue_original_indices[i]].remaining)//had already decremented element.remaining above
                        m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = firstTurnPPAvailable+j;
                    if (!element.remaining) {
                        m_queue[sim_queue_original_indices[i]].turns_left_to_completion = firstTurnPPAvailable+j;    // record the (estimated) turns to complete the whole element on the original queue
                    }
#endif // ORIG_SIMULATOR
#ifdef ORIG_SIMULATOR
                    //extra record of dp results, in case want to compare to orig results
                    if (element.remaining +1 == m_queue[sim_queue_original_indices[i]].remaining)//had already decremented element.remaining above
                        dpsimulation_results_to_next[i] = firstTurnPPAvailable+j;
                    if (!element.remaining) {
                        dpsimulation_results_to_completion[i] = firstTurnPPAvailable+j;    // record the (estimated) turns to complete the whole element on the original queue
                    }
#endif // ORIG_SIMULATOR
                }
                if (!element.remaining) {
                    break; // this element all done
                }
            } //j-loop : turns relative to firstTurnPPAvailable
        }// queue element loop
    }// resource groups loop

    dp_time_end = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
    dp_time = (dp_time_end - dp_time_start).total_nanoseconds();
    dp_time = dp_time; // just to suppresss the compiler warning of unused var if the comparisons are not being done below.

    //dp_time = dpsim_queue_timer.elapsed() * 1000;
    // Logger().debugStream() << "ProductionQueue::Update queue dynamic programming sim time: " << dpsim_queue_timer.elapsed() * 1000.0;

#endif //DP_SIMULATOR

// see ~110 lines up for define or undef statement
#ifdef  ORIG_SIMULATOR
    boost::posix_time::ptime orig_time_start;
    boost::posix_time::ptime orig_time_end;
    long orig_time;

    // cycle through items on queue, adding up their allotted PP until each is
    // finished and removed from queue until everything on queue has been
    // finished, in order to calculate expected completion times
    //boost::timer sim_queue_timer;
    int turns = 1;  // to keep track of how man turn-iterations simulation takes to finish items
    orig_time_start = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 

    while (!sim_queue.empty() && turns < TOO_MANY_TURNS && (boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time())-orig_time_start).seconds() < TOO_LONG_TIME) {
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
            boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(element);
            item_cost *= element.blocksize;
            

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
    // Logger().debugStream() << "ProductionQueue::Update queue orig sim time: " << sim_queue_timer.elapsed() * 1000.0;
    //orig_time = sim_queue_timer.elapsed() *1000;
    orig_time_end = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()); 
    orig_time = (orig_time_end - orig_time_start).total_nanoseconds();
    orig_time = orig_time; // just to suppresss the compiler warning of unused var if the comparisons are not being done below.
    // mark rest of items on simulated queue (if any) as never to be finished
    for (unsigned int i = 0; i < sim_queue.size(); ++i) {
        if (sim_queue[i].remaining == m_queue[sim_queue_original_indices[i]].remaining)
            m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = -1;
        m_queue[sim_queue_original_indices[i]].turns_left_to_completion = -1;
    }

#ifdef DP_QUEUE_SIMULATOR  // if both simulations were done, compare results
#ifdef COMPARE_SIMS
    bool sameResults = true;
    for (unsigned int i = 0; i< dpsim_queue_original_indices.size(); ++i ) {
        if ( (m_queue[dpsim_queue_original_indices[i]].turns_left_to_next_item != dpsimulation_results_to_next[i]) || 
            (m_queue[dpsim_queue_original_indices[i]].turns_left_to_completion != dpsimulation_results_to_completion[i]) ) {
            sameResults = false;
            break;
        }
    }
    if (sameResults ) {
        Logger().debugStream() << "ProductionQueue::Update orig sim and dp_sim gave same results";
        Logger().debugStream() << "ProductionQueue::Update orig sim time: " << orig_time*1e-3 << " , dp sim time: " << dp_time*1.0e-3 << " micro sec";
    } else  {
        Logger().debugStream() << "ProductionQueue::Update orig sim and dp_sim gave different results";
        Logger().debugStream() << "ProductionQueue::Update orig sim time: " << orig_time*1e-3 << " , dp sim time: " << dp_time*1.0e-3 << " micro sec";
        for (unsigned int i = 0; i< dpsimulation_results_to_next.size(); ++i ) {
            ProductionQueue::Element& el = m_queue[sim_queue_original_indices[i]];
            Logger().debugStream() << "ProductionQueue::Update    Queue Item: " << el.item.name;
            Logger().debugStream() << "ProductionQueue::Update         orig sim gave next: " << el.turns_left_to_next_item << " ; completion: " << el.turns_left_to_completion;
            Logger().debugStream() << "ProductionQueue::Update          dp  sim gave next: " << dpsimulation_results_to_next[i] << " ; completion: " << dpsimulation_results_to_completion[i];
        }
    }
#endif //  COMPARE_SIMS
#endif //  DP_QUEUE_SIMULATOR
#endif //  ORIG_SIMULATOR

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
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(*it);
        item_cost *= it->blocksize;
        double maxPerTurn = item_cost / std::max(build_turns,1);
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
    m_population_pool(),
    m_maintenance_total_cost(0)
{ Init(); }

Empire::Empire(const std::string& name, const std::string& player_name, int empire_id, const GG::Clr& color) :
    m_id(empire_id),
    m_name(name),
    m_player_name(player_name),
    m_color(color),
    m_capital_id(INVALID_OBJECT_ID),
    m_research_queue(m_id),
    m_production_queue(m_id),
    m_resource_pools(),
    m_population_pool(),
    m_maintenance_total_cost(0)
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
    m_meters["METER_DETECTION_STRENGTH"];
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

std::pair<double, int> Empire::ProductionCostAndTime(const ProductionQueue::Element& element) const
{ return ProductionCostAndTime(element.item, element.location); }

std::pair<double, int> Empire::ProductionCostAndTime(const ProductionQueue::ProductionItem& item,
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
            return std::make_pair(design->ProductionCost(m_id, location_id), design->ProductionTime(m_id, location_id));
        return std::make_pair(-1.0, -1);
    }
    Logger().errorStream() << "Empire::ProductionCostAndTime was passed a ProductionItem with an invalid BuildType";
    return std::make_pair(-1.0, -1);
}

bool Empire::HasExploredSystem(int ID) const
{ return (m_explored_systems.find(ID) != m_explored_systems.end()); }

bool Empire::BuildableItem(BuildType build_type, const std::string& name, int location) const {
    // special case to check for ships being passed with names, not design ids
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Empire::BuildableItem was passed BuildType BT_SHIP with a name, but ship designs are tracked by number");

    if (build_type == BT_BUILDING && !BuildingTypeAvailable(name))
        return false;

    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    UniverseObject* build_location = GetUniverseObject(location);
    if (!build_location)
        return false;

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

    if (build_type == BT_SHIP && !ShipDesignAvailable(design_id))
        return false;

    // design must be known to this empire
    const ShipDesign* ship_design = GetShipDesign(design_id);
    if (!ship_design || !ship_design->Producible())
        return false;

    UniverseObject* build_location = GetUniverseObject(location);
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
    std::vector<const UniverseObject*> owned_planets;
    for (std::set<int>::const_iterator it = known_objects.begin(); it != known_objects.end(); ++it) {
        if (const Planet* planet = GetPlanet(*it))
            if (planet->OwnedBy(this->EmpireID()))
                owned_planets.push_back(planet);
    }

    //std::cout << "... empire owns " << owned_planets.size() << " planets" << std::endl;
    for (std::vector<const UniverseObject*>::const_iterator it = owned_planets.begin(); it != owned_planets.end(); ++it) {
        const UniverseObject* obj = *it;
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
        if (system_id == INVALID_OBJECT_ID) {
            continue;   // not in a system, so can't affect system obstruction
        } else if (fleet->OwnedBy(m_id) && fleet->HasArmedShips() && fleet->Aggressive()) {
            systems_containing_friendly_fleets.insert(system_id);
        } else if (fleet->HasArmedShips() && fleet->Aggressive()) {
            int fleet_owner = fleet->Owner();
            if (fleet_owner == ALL_EMPIRES || Empires().GetDiplomaticStatus(m_id, fleet_owner) == DIPLO_WAR)
                systems_containing_obstructing_objects.insert(system_id);
        }
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
        //const UniverseObject* sys = GetUniverse().Object(sys_id);
        //std::string name = sys->Name();
        //Logger().debugStream() << "supply-exchanging system: " << name;

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

const std::map<int, int>& Empire::SystemSupplyRanges() const
{ return m_supply_system_ranges; }

const std::set<int>& Empire::SupplyUnobstructedSystems() const
{ return m_supply_unobstructed_systems; }

const std::set<std::pair<int, int> >& Empire::SupplyStarlaneTraversals() const
{ return m_supply_starlane_traversals; }

const std::set<std::pair<int, int> >& Empire::SupplyOstructedStarlaneTraversals() const
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

const std::map<int, std::set<int> > Empire::VisibleStarlanes() const {
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
{ return GetResourcePool(RE_INDUSTRY)->TotalAvailable(); }

const ResourcePool* Empire::GetResourcePool(ResourceType resource_type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        return 0;
    return it->second.get();
}

double Empire::ResourceStockpile(ResourceType type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceStockpile passed invalid ResourceType");
    return it->second->Stockpile();
}

double Empire::ResourceMaxStockpile(ResourceType type) const
{ return 0.0; }

double Empire::ResourceProduction(ResourceType type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceProduction passed invalid ResourceType");
    return it->second->Production();
}

double Empire::ResourceAvailable(ResourceType type) const {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceAvailable passed invalid ResourceType");
    return it->second->TotalAvailable();
}

const PopulationPool& Empire::GetPopulationPool() const
{ return m_population_pool; }

double Empire::Population() const
{ return m_population_pool.Population(); }

void Empire::SetResourceStockpile(ResourceType resource_type, double stockpile) {
    std::map<ResourceType, boost::shared_ptr<ResourcePool> >::const_iterator it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::SetResourceStockpile passed invalid ResourceType");
    return it->second->SetStockpile(stockpile);
}

void Empire::SetResourceMaxStockpile(ResourceType resource_type, double max)
{}

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
    m_research_queue.Update(m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
}

void Empire::RemoveTechFromQueue(const std::string& name) {
    ResearchQueue::iterator it = m_research_queue.find(name);
    if (it != m_research_queue.end()) {
        m_research_queue.erase(it);
        m_research_queue.Update(m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
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
    double clamped_progress = std::min(tech->ResearchCost(m_id), std::max(0.0, progress));
    m_research_progress[name] = clamped_progress;

    // if tech is complete, ensure it is on the queue, so it will be researched next turn
    if (clamped_progress >= tech->ResearchCost(m_id) &&
        m_research_queue.find(name) == m_research_queue.end())
    {
        m_research_queue.push_back(name);
    }

    // don't just give tech to empire, as another effect might reduce its progress before end of turn
}

const unsigned int MAX_PROD_QUEUE_SIZE = 500;

void Empire::PlaceBuildInQueue(BuildType build_type, const std::string& name, int number, int location, int pos/* = -1*/) {
    if (!BuildableItem(build_type, name, location))
        Logger().debugStream() << "Empire::PlaceBuildInQueue() : Placed a non-buildable item in queue...";

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE)
        return;

    ProductionQueue::Element build(build_type, name, number, number, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos) {
        m_production_queue.push_back(build);
        m_production_progress.push_back(0.0);
    } else {
        m_production_queue.insert(m_production_queue.begin() + pos, build);
        m_production_progress.insert(m_production_progress.begin() + pos, 0.0);
    }
    //m_production_queue.Update(m_resource_pools, m_production_progress);
}

void Empire::PlaceBuildInQueue(BuildType build_type, int design_id, int number, int location, int pos/* = -1*/) {
    if (!BuildableItem(build_type, design_id, location))
        Logger().debugStream() << "Empire::PlaceBuildInQueue() : Placed a non-buildable item in queue...";

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE)
        return;

    ProductionQueue::Element build(build_type, design_id, number, number, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos) {
        m_production_queue.push_back(build);
        m_production_progress.push_back(0.0);
    } else {
        m_production_queue.insert(m_production_queue.begin() + pos, build);
        m_production_progress.insert(m_production_progress.begin() + pos, 0.0);
    }
    //m_production_queue.Update(m_resource_pools, m_production_progress);
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
    if ( blocksize < original_blocksize ) // must lose the progress from the excess former blocksize, or min-turns-to-build could be bypassed
        m_production_progress[index] = (m_production_progress[index] / original_blocksize ) * blocksize;
    //m_production_queue.Update(m_resource_pools, m_production_progress);
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
    //m_production_queue.Update(m_resource_pools, m_production_progress);
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
    double status = m_production_progress[index];
    m_production_queue.erase(index);
    m_production_progress.erase(m_production_progress.begin() + index);
    m_production_queue.insert(m_production_queue.begin() + new_index, build);
    m_production_progress.insert(m_production_progress.begin() + new_index, status);
    //m_production_queue.Update(m_resource_pools, m_production_progress);
}

void Empire::RemoveBuildFromQueue(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        Logger().debugStream() << "Empire::RemoveBuildFromQueue index: " << index << "  queue size: " << m_production_queue.size();
        Logger().errorStream() << "Attempted to delete a production queue item with an invalid index.";
        return;
    }
    m_production_queue.erase(index);
    m_production_progress.erase(m_production_progress.begin() + index);
    //m_production_queue.Update(m_resource_pools, m_production_progress);
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
                if (!type) {
                    Logger().errorStream() << "ConquerProductionQueueItemsAtLocation couldn't get building with name " << name;
                    continue;
                }

                CaptureResult result = type->GetCaptureResult(from_empire_id, empire_id, location_id, true);

                if (result == CR_DESTROY) {
                    // item removed from current queue, NOT added to conquerer's queue
                    queue_it = queue.erase(queue_it);
                    status.erase(status.begin() + i);

                } else if (result == CR_CAPTURE) {
                    if (to_empire) {
                        // item removed from current queue, added to conquerer's queue
                        ProductionQueue::Element build(item, elem.ordered, elem.remaining, location_id);
                        to_empire->m_production_queue.push_back(build);
                        to_empire->m_production_progress.push_back(status[i]);

                        queue_it = queue.erase(queue_it);
                        status.erase(status.begin() + i);
                    } else {
                        // else do nothing; no empire can't capure things
                        ++queue_it;
                        ++i;
                    }

                } else if (result == INVALID_CAPTURE_RESULT) {
                    Logger().errorStream() << "Empire::ConquerBuildsAtLocationFromEmpire: BuildingType had an invalid CaptureResult";
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

void Empire::AddTech(const std::string& name) {
    m_techs.insert(name);

    const Tech* tech = GetTech(name);
    if (!tech) {
        Logger().errorStream() << "Empire::AddTech given and invalid tech: " << name;
        return;
    }

    const std::vector<ItemSpec>& unlocked_items = tech->UnlockedItems();
    for (unsigned int i = 0; i < unlocked_items.size(); ++i)
        UnlockItem(unlocked_items[i]);  // potential infinite if a tech (in)directly unlocks itself?
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
        AddShipDesign(GetPredefinedShipDesignManager().GenericDesignID(item.name)); // this adds the generic version of the design, not created by any empire, to this empire's remembered designs.
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
    if (building_type->Producible())
        m_available_building_types.insert(name);
}

void Empire::AddPartType(const std::string& name) {
    const PartType* part_type = GetPartType(name);
    if (!part_type) {
        Logger().errorStream() << "Empire::AddPartType given an invalid part type name: " << name;
        return;
    }
    if (part_type->Producible())
        m_available_part_types.insert(name);
}

void Empire::AddHullType(const std::string& name) {
    const HullType* hull_type = GetHullType(name);
    if (!hull_type) {
        Logger().errorStream() << "Empire::AddHullType given an invalid hull type name: " << name;
        return;
    }
    if (hull_type->Producible())
        m_available_hull_types.insert(name);
}

void Empire::AddExploredSystem(int ID) {
    if (GetSystem(ID))
        m_explored_systems.insert(ID);
    else
        Logger().errorStream() << "Empire::AddExploredSystem given an invalid system id: " << ID;
}

std::string Empire::NewShipName() {
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

void Empire::AddShipDesign(int ship_design_id) {
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
        RemoveShipDesign(GetPredefinedShipDesignManager().GenericDesignID(item.name));
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
        if (tech->ResearchCost(m_id) - EPSILON <= progress) {
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

void Empire::CheckProductionProgress() {
    Logger().debugStream() << "========Empire::CheckProductionProgress=======";
    // following commented line should be redundant, as previous call to UpdateResourcePools should have generated necessary info
    // m_production_queue.Update(this, m_resource_pools, m_production_progress);

    Universe& universe = GetUniverse();

    std::map<int, std::vector<Ship*> >  system_new_ships;

    // go through queue, updating production progress.  If a build item is completed, create the built object or take whatever other
    // action is appropriate, and record that queue item as complete, so it can be erased from the queue
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = ProductionCostAndTime(m_production_queue[i]);
        item_cost *= m_production_queue[i].blocksize;
        double& status = m_production_progress[i];
        status += m_production_queue[i].allocated_pp;               // add allocated PP to queue item


        // if accumulated PP is sufficient, the item is complete
        if (item_cost - EPSILON <= status) {
            m_production_progress[i] -= item_cost; // this is the correct calculation -- item_cost is now for one full item, not just one turn's portion
            switch (m_production_queue[i].item.build_type) {
            case BT_BUILDING: {
                int planet_id = m_production_queue[i].location;
                Planet* planet = GetPlanet(planet_id);
                if (!planet) {
                    Logger().errorStream() << "Couldn't get planet with id  " << planet_id << " on which to create building";
                    break;
                }

                // check location condition before each building is created, so
                // that buildings being produced can prevent subsequent
                // buildings completions on the same turn from going through
                if (!this->BuildableItem(m_production_queue[i].item, m_production_queue[i].location))
                    break;

                // create new building
                Building* building = new Building(m_id, m_production_queue[i].item.name, m_id);
                int building_id = universe.Insert(building);
                planet->AddBuilding(building_id);

                AddSitRepEntry(CreateBuildingBuiltSitRep(building_id, planet->ID()));
                //Logger().debugStream() << "New Building created on turn: " << building->CreationTurn();
                break;
            }

            case BT_SHIP: {
                if (m_production_queue[i].blocksize < 1)
                    break;   // nothing to do!

                UniverseObject* build_location = GetUniverseObject(m_production_queue[i].location);
                System* system = universe_object_cast<System*>(build_location);
                if (!system && build_location)
                    system = GetSystem(build_location->SystemID());
                // TODO: account for shipyards and/or other ship production
                // sites that are in interstellar space, if needed
                if (!system) {
                    Logger().errorStream() << "Empire::CheckProductionProgress couldn't get system for building new ship";
                    break;
                }

                // check location condition before each ship is created, so
                // that ships being produced can prevent subsequent
                // ship completions on the same turn from going through
                if (!this->BuildableItem(m_production_queue[i].item, m_production_queue[i].location))
                    break;

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
                else if (const Planet* capital_planet = GetPlanet(this->CapitalID()))
                    species_name = capital_planet->SpeciesName();
                // else give up...
                if (species_name.empty()) {
                    // only really a problem for colony ships, which need to have a species to function
                    const ShipDesign* design = GetShipDesign(m_production_queue[i].item.design_id);
                    if (!design) {
                        Logger().errorStream() << "Couldn't get ShipDesign with id: " << m_production_queue[i].item.design_id;
                        break;
                    }
                    if (design->CanColonize()) {
                        Logger().errorStream() << "Couldn't get species in order to make colony ship!";
                        break;
                    }
                }

                Ship* ship = 0;
                int ship_id = INVALID_OBJECT_ID;

                for (int count = 0; count < m_production_queue[i].blocksize; count++) {
                    // create ship
                    ship = new Ship(m_id, m_production_queue[i].item.design_id, species_name, m_id);
                    // set active meters that have associated max meters to an
                    // initial very large value, so that when the active meters are
                    // later clamped, they will equal the max meter after effects
                    // have been applied, letting new ships start with maxed
                    // everything that is traced with an associated max meter.
                    ship->UniverseObject::GetMeter(METER_FUEL)->SetCurrent(Meter::LARGE_VALUE);
                    ship->UniverseObject::GetMeter(METER_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
                    ship->UniverseObject::GetMeter(METER_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);
                    ship->BackPropegateMeters();

                    ship_id = universe.Insert(ship);
                    ship->Rename(NewShipName());

                    // store ships to put into fleets later
                    system_new_ships[system->ID()].push_back(ship);
                }
                // add sitrep
                if (m_production_queue[i].blocksize == 1) {
                    AddSitRepEntry(CreateShipBuiltSitRep(ship_id, system->ID(), ship->DesignID()));
                    Logger().debugStream() << "New Ship, id " << ship_id << ", created on turn: " << ship->CreationTurn();
                } else {
                    AddSitRepEntry(CreateShipBlockBuiltSitRep(system->ID(), ship->DesignID(), m_production_queue[i].blocksize));
                    Logger().debugStream() << "New block of "<< m_production_queue[i].blocksize << "ships created on turn: " << ship->CreationTurn();
                }
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

    // create fleets for new ships and put ships into fleets
    for (std::map<int, std::vector<Ship*> >::iterator it = system_new_ships.begin();
         it != system_new_ships.end(); ++it)
    {
        System* system = GetSystem(it->first);
        if (!system) {
            Logger().errorStream() << "Couldn't get system with id " << it->first << " for creating new fleets for newly produced ships";
            continue;
        }

        std::vector<Ship*>& allShips = it->second;
        if (allShips.empty())
            continue;

        //group ships into fleets, by design
        std::map<int,std::vector<Ship*> > shipsByDesign;
        for (std::vector<Ship*>::iterator it = allShips.begin(); it != allShips.end(); ++it) {
            Ship* ship = *it;
            shipsByDesign[ship->DesignID()].push_back(ship);
        }
        for (std::map<int, std::vector<Ship*> >::iterator design_it = shipsByDesign.begin();
             design_it != shipsByDesign.end(); ++design_it)
        {
            std::vector<int> ship_ids;

            std::vector<Ship*>& ships = design_it->second;
            if (ships.empty())
                continue;

            // create new fleet for ships
            Fleet* fleet = new Fleet("", system->X(), system->Y(), m_id);
            int fleet_id = universe.Insert(fleet);

            system->Insert(fleet);

            for (std::vector<Ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
                Ship* ship = *it;
                ship_ids.push_back(ship->ID());
                fleet->AddShip(ship->ID());
            }

            // rename fleet, given its id and the ship that is in it
            fleet->Rename(Fleet::GenerateFleetName(ship_ids, fleet_id));

            Logger().debugStream() << "New Fleet \"" + fleet->Name() + "\"created on turn: " << fleet->CreationTurn();
        }
    }

    // removed completed items from queue
    for (std::vector<int>::reverse_iterator it = to_erase.rbegin(); it != to_erase.rend(); ++it) {
        m_production_progress.erase(m_production_progress.begin() + *it);
        m_production_queue.erase(*it);
    }
}

void Empire::CheckTradeSocialProgress()
{ m_resource_pools[RE_TRADE]->SetStockpile(m_resource_pools[RE_TRADE]->TotalAvailable() - m_maintenance_total_cost); }

void Empire::SetColor(const GG::Clr& color)
{ m_color = color; }

void Empire::SetName(const std::string& name)
{ m_name = name; }

void Empire::SetPlayerName(const std::string& player_name)
{ m_player_name = player_name; }

void Empire::InitResourcePools() {
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
    m_resource_pools[RE_RESEARCH]->SetObjects(object_ids_vec);
    m_resource_pools[RE_INDUSTRY]->SetObjects(object_ids_vec);
    m_resource_pools[RE_TRADE]->SetObjects(object_ids_vec);


    // inform the blockadeable resource pools about systems that can share
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
    res_type_vec.push_back(RE_INDUSTRY);
    res_type_vec.push_back(RE_TRADE);
    res_type_vec.push_back(RE_RESEARCH);

    for (std::vector<ResourceType>::const_iterator res_it = res_type_vec.begin(); res_it != res_type_vec.end(); ++res_it) {
        ResourceType res_type = *res_it;
        int stockpile_object_id = INVALID_OBJECT_ID;
        if (const UniverseObject* stockpile_obj = GetUniverseObject(StockpileID(res_type)))
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
    m_production_queue.Update(m_resource_pools, m_production_progress);
    m_resource_pools[RE_INDUSTRY]->ChangedSignal();
}

void Empire::UpdateTradeSpending() {
    m_resource_pools[RE_TRADE]->Update(); // recalculate total trade production

    // TODO: Replace with call to some other subsystem, similar to the Update...Queue functions
    m_maintenance_total_cost = 0.0;

    std::vector<UniverseObject*> buildings = GetUniverse().Objects().FindObjects(OwnedVisitor<Building>(m_id));
    for (std::vector<UniverseObject*>::const_iterator it = buildings.begin(); it != buildings.end(); ++it) {
        Building* building = universe_object_cast<Building*>(*it);
        if (!building)
            continue;

        const BuildingType* building_type = GetBuildingType(building->BuildingTypeName());
        if (!building_type)
            continue;

        //if (building->Operating())
        m_maintenance_total_cost += building_type->MaintenanceCost();
    }
    m_resource_pools[RE_TRADE]->ChangedSignal();
}

void Empire::UpdatePopulationGrowth()
{ m_population_pool.Update(); }

void Empire::ResetMeters() {
    for (std::map<std::string, Meter>::iterator it = m_meters.begin(); it != m_meters.end(); ++it) {
        it->second.ResetCurrent();
    }
}
