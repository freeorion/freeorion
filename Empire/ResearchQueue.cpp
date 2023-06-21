#include "ResearchQueue.h"

#include "Empire.h"
#include "../universe/Tech.h"
#include "../util/AppInterface.h"

namespace {
    constexpr float EPSILON = 0.01f;

    /** sets the .allocated_rp, value for each Tech in the queue.  Only sets
      * nonzero funding to a Tech if it is researchable this turn.  Also
      * determines total number of spent RP (returning by reference in
      * total_RPs_spent) */
    void SetTechQueueElementSpending(
        float RPs, const std::map<std::string, float>& research_progress, // TODO: make flat_map<std::string_view, float> ?
        const std::map<std::string, TechStatus>& research_status,
        ResearchQueue::QueueType& queue, float& total_RPs_spent,
        int& projects_in_progress, int empire_id, const ScriptingContext& context)
    {
        total_RPs_spent = 0.0f;
        projects_in_progress = 0;

        for (ResearchQueue::Element& elem : queue) {
            elem.allocated_rp = 0.0f;    // default, may be modified below

            if (elem.paused)
                continue;

            // get details on what is being researched...
            const Tech* tech = GetTech(elem.name);
            if (!tech) {
                ErrorLogger() << "SetTechQueueElementSpending found null tech on research queue?!";
                continue;
            }
            const auto status_it = research_status.find(elem.name);
            if (status_it == research_status.end()) {
                ErrorLogger() << "SetTechQueueElementSpending couldn't find tech with name " << elem.name << " in the research status map";
                continue;
            }
            const bool researchable = status_it->second == TechStatus::TS_RESEARCHABLE;

            if (researchable && !elem.paused) {
                const auto progress_it = research_progress.find(elem.name);
                const float tech_cost = tech->ResearchCost(empire_id, context);
                const float progress = progress_it == research_progress.end() ? 0.0f : progress_it->second;
                const float RPs_needed = tech_cost - progress*tech_cost;
                const int tech_min_turns = std::max(1, tech ? tech->ResearchTime(empire_id, context) : 1);

                const float RPs_per_turn_limit = tech ? (tech_cost / tech_min_turns) : 1.0f;
                const float RPs_to_spend = std::min(RPs_needed, RPs_per_turn_limit);

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

bool ResearchQueue::InQueue(const std::string& tech_name) const
{ return std::any_of(m_queue.begin(), m_queue.end(), [&tech_name](const Element& e){ return e.name == tech_name; }); }

bool ResearchQueue::Paused(const std::string& tech_name) const {
    const auto it = find(tech_name);
    if (it == end())
        return false;
    return it->paused;
}

bool ResearchQueue::Paused(int idx) const {
    if (idx >= static_cast<int>(m_queue.size()))
        return false;
    return std::next(begin(), idx)->paused;
}

std::vector<std::string> ResearchQueue::AllEnqueuedProjects() const {
    std::vector<std::string> retval;
    retval.reserve(m_queue.size());
    std::transform(m_queue.begin(), m_queue.end(), std::back_inserter(retval),
                   [](const auto& elem) { return elem.name; });
    return retval;
}

std::string ResearchQueue::Dump() const {
    std::stringstream retval;
    retval << "ResearchQueue:\n";
    float spent_rp = 0.0f;
    for (const auto& elem : m_queue) {
        retval << " ... " << elem.Dump();
        spent_rp += elem.allocated_rp;
    }
    retval << "ResearchQueue Total Spent RP: " << spent_rp;
    return retval.str();
}

ResearchQueue::const_iterator ResearchQueue::find(const std::string& tech_name) const
{ return std::find_if(begin(), end(), [&tech_name](const auto& elem) { return elem.name == tech_name; }); }

const ResearchQueue::Element& ResearchQueue::operator[](int i) const {
    if (i < 0 || i >= static_cast<int>(m_queue.size()))
        throw std::out_of_range("Tried to access ResearchQueue element out of bounds");
    return m_queue[i];
}

void ResearchQueue::Update(float RPs, const std::map<std::string, float>& research_progress,
                           const ScriptingContext& context)
{
    // status of all techs for this empire
    const auto empire = context.GetEmpire(m_empire_id);
    if (!empire) {
        ErrorLogger() << "ResearchQueue::Update passed null empire.  doing nothing.";
        m_projects_in_progress = 0;
        m_total_RPs_spent = 0.0f;
        return;
    }

    const auto& tm{GetTechManager()};
    std::map<std::string, TechStatus> sim_tech_status_map;
    using STSM_vt = decltype(sim_tech_status_map)::value_type;
    std::transform(tm.begin(), tm.end(), std::inserter(sim_tech_status_map, sim_tech_status_map.end()),
                   [&empire](const auto& name_tech)
                   { return STSM_vt{name_tech.first, empire->GetTechStatus(name_tech.first)}; });

    SetTechQueueElementSpending(RPs, research_progress, sim_tech_status_map, m_queue,
                                m_total_RPs_spent, m_projects_in_progress, m_empire_id,
                                context);

    if (m_queue.empty()) {
        ResearchQueueChangedSignal();
        return;    // nothing more to do...
    }

    constexpr int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops

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
    std::map<std::string, int> orig_queue_order;
    std::map<int, float> dpsim_research_progress;
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        std::string tname = m_queue[i].name;
        orig_queue_order[tname] = i;
        dpsim_research_progress[i] = dp_prog[tname];
    }

    std::map<std::string, TechStatus> dpsim_tech_status_map = std::move(sim_tech_status_map);

    // initialize simulation_results with -1 for all techs, so that any techs that aren't
    // finished in simulation by turn TOO_MANY_TURNS will be left marked as never to be finished
    std::vector<int> dpsimulation_results(m_queue.size(), -1);

    constexpr int DP_TURNS = TOO_MANY_TURNS; // track up to this many turns

    std::map<std::string, std::set<std::string>> waiting_for_prereqs;
    std::set<int> dp_researchable_techs;

    boost::container::flat_map<int, std::pair<float, int>> tech_cost_time;
    tech_cost_time.reserve(m_queue.size());

    auto is_incomplete = [&dpsim_tech_status_map](const auto& tech) {
        const auto t_it = dpsim_tech_status_map.find(tech);
        return t_it != dpsim_tech_status_map.end() && t_it->second != TechStatus::TS_COMPLETE;
    };

    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        const auto& elem = m_queue[i];
        if (elem.paused)
            continue;
        const Tech* tech = GetTech(elem.name);
        if (!tech)
            continue;

        tech_cost_time.emplace(std::piecewise_construct,
                               std::forward_as_tuple(i),
                               std::forward_as_tuple(
                                   tech ? tech->ResearchCost(m_empire_id, context) : 1.0f,
                                   std::max(1, tech ? tech->ResearchTime(m_empire_id, context) : 1)));

        if (dpsim_tech_status_map[elem.name] == TechStatus::TS_RESEARCHABLE) {
            dp_researchable_techs.insert(i);
            continue;
        }

        if (dpsim_tech_status_map[elem.name] == TechStatus::TS_UNRESEARCHABLE ||
            dpsim_tech_status_map[elem.name] == TechStatus::TS_HAS_RESEARCHED_PREREQ)
        {
            const auto& these_prereqs{tech->Prerequisites()};
            std::set<std::string> incomplete_prereqs;
            std::copy_if(these_prereqs.begin(), these_prereqs.end(),
                         std::inserter(incomplete_prereqs, incomplete_prereqs.end()),
                         is_incomplete);

            waiting_for_prereqs[elem.name] = std::move(incomplete_prereqs);
        }
    }

    int dp_turns = 0;
    //pp_still_available[turn-1] gives the RP still available in this resource pool at turn "turn"
    std::vector<float> rp_still_available(DP_TURNS, RPs);  // initialize to the  full RP allocation for every turn


    while ((dp_turns < DP_TURNS) && !(dp_researchable_techs.empty())) {// if we haven't used up our turns and still have techs to process
        ++dp_turns;
        std::map<int, bool> already_processed;
        for (int tech_id : dp_researchable_techs)
            already_processed.emplace(tech_id, false);

        auto cur_tech_it = dp_researchable_techs.begin();
        while ((rp_still_available[dp_turns-1] > EPSILON)) { // try to use up this turns RPs
            if (cur_tech_it == dp_researchable_techs.end())
                break; // will be wasting some RP this turn

            const int cur_tech = *cur_tech_it;
            if (already_processed[cur_tech]) {
                ++cur_tech_it;
                continue;
            }

            already_processed[cur_tech] = true;
            const auto& tech_name = m_queue[cur_tech].name;
            const Tech* tech = GetTech(tech_name);
            float progress = dpsim_research_progress[cur_tech];

            const auto [tech_cost, tech_min_turns] = tech_cost_time[cur_tech];
            const float RPs_needed = tech_cost * (1.0f - std::min(progress, 1.0f));
            const float RPs_per_turn_limit = tech_cost / tech_min_turns;

            const float RPs_to_spend = std::min(std::min(RPs_needed, RPs_per_turn_limit), rp_still_available[dp_turns-1]);
            progress += RPs_to_spend / std::max(EPSILON, tech_cost);
            dpsim_research_progress[cur_tech] = progress;
            rp_still_available[dp_turns-1] -= RPs_to_spend;

            auto next_res_tech_it = cur_tech_it;
            int next_res_tech_idx = 0;
            if (++next_res_tech_it == dp_researchable_techs.end())
                next_res_tech_idx = m_queue.size() + 1;
            else
                next_res_tech_idx = *(next_res_tech_it);

            const bool tech_completed = (tech_cost - EPSILON <= progress * tech_cost);


            if (tech_completed) {
                dpsim_tech_status_map[tech_name] = TechStatus::TS_COMPLETE;
                dpsimulation_results[cur_tech] = dp_turns;

                m_queue[cur_tech].turns_left = dp_turns;
                dp_researchable_techs.erase(cur_tech_it);

                if (tech) {
                    for (const auto& u_tech_name : tech->UnlockedTechs()) {
                        const auto prereq_tech_it = waiting_for_prereqs.find(u_tech_name);
                        if (prereq_tech_it != waiting_for_prereqs.end() ){
                            auto& these_prereqs = prereq_tech_it->second;
                            const auto just_finished_it = these_prereqs.find(tech_name);
                            if (just_finished_it != these_prereqs.end() ) { // should always find it
                                these_prereqs.erase(just_finished_it);
                                if (these_prereqs.empty()) { // tech now fully unlocked
                                    const int this_tech_idx = orig_queue_order[u_tech_name];
                                    dp_researchable_techs.insert(this_tech_idx);
                                    waiting_for_prereqs.erase(prereq_tech_it);
                                    already_processed[this_tech_idx] = true; // doesn't get any allocation on current turn
                                    if (this_tech_idx < next_res_tech_idx )
                                        next_res_tech_idx = this_tech_idx;
                                }

                            } else { //couldnt find tech_name in prereqs list
                                DebugLogger() << "ResearchQueue::Update tech unlocking problem:"<< tech_name << "thought it was a prereq for " << u_tech_name << "but the latter disagreed";
                            }
                        }
                    }
                }
            }
            cur_tech_it = dp_researchable_techs.find(next_res_tech_idx);
        }
    }

    ResearchQueueChangedSignal();
}

void ResearchQueue::push_back(const std::string& tech_name, bool paused)
{ m_queue.push_back(Element{tech_name, m_empire_id, 0.0f, -1, paused}); }

void ResearchQueue::insert(iterator it, const std::string& tech_name, bool paused)
{ m_queue.insert(it, Element{tech_name, m_empire_id, 0.0f, -1, paused}); }

void ResearchQueue::erase(iterator it) {
    if (it == end())
        throw std::out_of_range("Tried to erase ResearchQueue element out of bounds");
    m_queue.erase(it);
}

ResearchQueue::iterator ResearchQueue::find(const std::string& tech_name) {
    for (iterator it = begin(); it != end(); ++it) {
        if (it->name == tech_name)
            return it;
    }
    return end();
}

void ResearchQueue::clear() {
    m_queue.clear();
    m_projects_in_progress = 0;
    m_total_RPs_spent = 0.0f;
    ResearchQueueChangedSignal();
}
