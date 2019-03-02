#include "ResearchQueue.h"

#include "Empire.h"
#include "../universe/Enums.h"
#include "../universe/Tech.h"
#include "../util/AppInterface.h"

namespace {
    const float EPSILON = 0.01f;

    /** sets the .allocated_rp, value for each Tech in the queue.  Only sets
      * nonzero funding to a Tech if it is researchable this turn.  Also
      * determines total number of spent RP (returning by reference in
      * total_RPs_spent) */
    void SetTechQueueElementSpending(
        float RPs, const std::map<std::string, float>& research_progress,
        const std::map<std::string, TechStatus>& research_status,
        ResearchQueue::QueueType& queue, float& total_RPs_spent,
        int& projects_in_progress, int empire_id)
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
            bool researchable = status_it->second == TS_RESEARCHABLE;

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
}


std::string ResearchQueue::Element::Dump() const {
    std::stringstream retval;
    retval << "ResearchQueue::Element: tech: " << name << "  empire id: " << empire_id;
    retval << "  allocated: " << allocated_rp << "  turns left: " << turns_left;
    if (paused)
        retval << "  (paused)";
    retval << "\n";
    return retval.str();
}

bool ResearchQueue::InQueue(const std::string& tech_name) const {
    return std::count_if(m_queue.begin(), m_queue.end(),
                       [tech_name](const Element& e){ return e.name == tech_name; });
}

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
    for (const auto& tech : GetTechManager()) {
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

    std::map<std::string, TechStatus> dpsim_tech_status_map = std::move(sim_tech_status_map);

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
            for (auto ptech_it = these_prereqs.begin(); ptech_it != these_prereqs.end();) {
                if (dpsim_tech_status_map[*ptech_it] != TS_COMPLETE) {
                    ++ptech_it;
                } else {
                    auto erase_it = ptech_it;
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
        auto cur_tech_it = dp_researchable_techs.begin();
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
            float tech_cost = tech ? tech->ResearchCost(m_empire_id) : 0.0f;
            float RPs_needed = tech ? tech_cost * (1.0f - std::min(progress, 1.0f)) : 0.0f;
            float RPs_per_turn_limit = tech ? tech->PerTurnCost(m_empire_id) : 1.0f;
            float RPs_to_spend = std::min(std::min(RPs_needed, RPs_per_turn_limit), rp_still_available[dp_turns-1]);
            progress += RPs_to_spend / std::max(EPSILON, tech_cost);
            dpsim_research_progress[cur_tech] = progress;
            rp_still_available[dp_turns-1] -= RPs_to_spend;
            auto next_res_tech_it = cur_tech_it;
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
                    auto prereq_tech_it = waiting_for_prereqs.find(u_tech_name);
                    if (prereq_tech_it != waiting_for_prereqs.end() ){
                        std::set<std::string>& these_prereqs = prereq_tech_it->second;
                        auto just_finished_it = these_prereqs.find(tech_name);
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
