#include "Tech.h"

#include <boost/filesystem/fstream.hpp>
#include "CommonParams.h"
#include "Effect.h"
#include "ObjectMap.h"
#include "UniverseObject.h"
#include "UnlockableItem.h"
#include "ValueRef.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/GameRules.h"
#include "../util/GameRuleRanks.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include <numeric>


namespace {
    #define UserStringNop(key) key

    void AddRules(GameRules& rules) {
        // makes all techs cost 1 RP and take 1 turn to research
        rules.Add<bool>(UserStringNop("RULE_CHEAP_AND_FAST_TECH_RESEARCH"),
                        UserStringNop("RULE_CHEAP_AND_FAST_TECH_RESEARCH_DESC"),
                        GameRuleCategories::GameRuleCategory::TEST, false, true,
                        GameRuleRanks::RULE_CHEAP_AND_FAST_TECH_RESEARCH_RANK);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

namespace {
    // returns techs in \a techs that are not in \a researched_techs
    // and of which all prereqs are in \a researched_techs
    auto NextTechs(std::vector<std::string_view> researched_techs, const TechManager::TechContainer& techs) {
        const auto is_researched = [rt{std::move(researched_techs)}](const auto& tech)
        { return std::find(rt.begin(), rt.end(), tech) != rt.end(); };

        // are prereqs researched but not itself researched
        const auto is_researchable_now = [is_researched](const auto& name_tech) -> bool {
            if (!name_tech.second.Researchable() || is_researched(name_tech.first))
                return false;
            const auto& prereqs = name_tech.second.Prerequisites();
            return std::all_of(prereqs.begin(), prereqs.end(), is_researched);
        };

        std::vector<const Tech*> retval;
        retval.reserve(techs.size());
        // transform_if
        for (const auto& name_tech : techs) {
            if (is_researchable_now(name_tech))
                retval.push_back(&name_tech.second);
        }

        return retval;
    }

    const Tech* Cheapest(const std::vector<const Tech*>& next_techs, int empire_id,
                         const ScriptingContext& context)
    {
        if (next_techs.empty())
            return nullptr;
        std::vector<float> costs;
        costs.reserve(next_techs.size());
        static constexpr float BIG_COST = std::numeric_limits<float>::max();
        std::transform(next_techs.begin(), next_techs.end(), std::back_inserter(costs),
                       [&context, empire_id](const Tech* tech)
                       { return tech ? tech->ResearchCost(empire_id, context) : BIG_COST; });
        const auto min_cost_it = std::min_element(costs.begin(), costs.end());
        if (min_cost_it == costs.end() || *min_cost_it == BIG_COST)
            return nullptr;
        const auto idx = std::distance(costs.begin(), min_cost_it);
        return next_techs[idx];
    }
}

namespace CheckSums {
    void CheckSumCombine(uint32_t& sum, const TechCategory& cat) {
        TraceLogger() << "CheckSumCombine(Slot): " << typeid(cat).name();
        CheckSumCombine(sum, cat.name);
        CheckSumCombine(sum, cat.graphic);
        CheckSumCombine(sum, cat.colour);
    }
}

///////////////////////////////////////////////////////////
// Tech Info                                             //
///////////////////////////////////////////////////////////
Tech::TechInfo::TechInfo(std::string& name_, std::string& description_,
                         std::string& short_description_, std::string& category_,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& research_cost_,
                         std::unique_ptr<ValueRef::ValueRef<int>>&& research_turns_,
                         bool researchable_,
                         std::set<std::string>& tags_) :
    name(std::move(name_)),
    description(std::move(description_)),
    short_description(std::move(short_description_)),
    category(std::move(category_)),
    research_cost(std::move(research_cost_)),
    research_turns(std::move(research_turns_)),
    researchable(researchable_),
    tags(std::move(tags_))
{}

Tech::TechInfo::~TechInfo() = default;

///////////////////////////////////////////////////////////
// Tech                                                  //
///////////////////////////////////////////////////////////
Tech::Tech(std::string&& name, std::string&& description,
           std::string&& short_description, std::string&& category,
           std::unique_ptr<ValueRef::ValueRef<double>>&& research_cost,
           std::unique_ptr<ValueRef::ValueRef<int>>&& research_turns,
           bool researchable,
           std::set<std::string>&& tags,
           std::vector<std::shared_ptr<Effect::EffectsGroup>>&& effects,
           std::set<std::string>&& prerequisites,
           std::vector<UnlockableItem>&& unlocked_items,
           std::string&& graphic) :
    Tech(std::move(name), std::move(description), std::move(short_description), std::move(category),
         std::move(research_cost), std::move(research_turns), researchable, std::move(tags),
         [](auto& effects) {
             std::vector<Effect::EffectsGroup> retval;
             retval.reserve(effects.size());
             for (auto& e : effects)
                 retval.push_back(std::move(*e)); // extract from shared_ptr
             return retval;
         }(effects),
         std::move(prerequisites), std::move(unlocked_items), std::move(graphic))
{}

Tech::Tech(std::string&& name, std::string&& description,
           std::string&& short_description, std::string&& category,
           std::unique_ptr<ValueRef::ValueRef<double>>&& research_cost,
           std::unique_ptr<ValueRef::ValueRef<int>>&& research_turns,
           bool researchable,
           std::set<std::string>&& tags,
           std::vector<Effect::EffectsGroup>&& effects,
           std::set<std::string>&& prerequisites,
           std::vector<UnlockableItem>&& unlocked_items,
           std::string&& graphic) :
    m_name(name), // not a move so it can be used later in member intializer list
    m_description(std::move(description)),
    m_short_description(std::move(short_description)),
    m_category(std::move(category)),
    m_research_cost([](auto&& rc, const auto& name) {
        if (rc)
            rc->SetTopLevelContent(name);
        return std::move(rc);
    }(std::move(research_cost), name)),
    m_research_turns([](auto&& rt, const auto& name) {
        if (rt)
            rt->SetTopLevelContent(name);
        return std::move(rt);
    }(std::move(research_turns), name)),
    m_researchable(researchable),
    m_tags_concatenated([&tags]() {
        // allocate storage for concatenated tags
        std::size_t params_sz = std::transform_reduce(tags.begin(), tags.end(), 0u, std::plus{},
                                                      [](const auto& tag) { return tag.size(); });
        std::string retval;
        retval.reserve(params_sz);

        // concatenate tags
        std::for_each(tags.begin(), tags.end(), [&retval](const auto& t)
        { retval.append(boost::to_upper_copy<std::string>(t)); });
        return retval;
    }()),
    m_tags([&tags, this]() {
        std::vector<std::string_view> retval;
        std::size_t next_idx = 0;
        retval.reserve(tags.size());
        std::string_view sv{m_tags_concatenated};

        // store views into concatenated tags string
        std::for_each(tags.begin(), tags.end(),
                      [&next_idx, &retval, sv](const auto& t)
        {
            std::string upper_t = boost::to_upper_copy<std::string>(t);
            retval.push_back(sv.substr(next_idx, upper_t.size()));
            next_idx += upper_t.size();
        });
        return retval;
    }()),
    m_pedia_tags([&tags, this]() {
        std::vector<std::string_view> retval;
        std::size_t next_idx = 0;
        retval.reserve(tags.size());
        std::string_view sv{m_tags_concatenated};

        // store views into concatenated tags string
        std::for_each(tags.begin(), tags.end(),
                      [&next_idx, &retval, sv](const auto& t)
        {
            std::string upper_t = boost::to_upper_copy<std::string>(t);
            auto tag = sv.substr(next_idx, upper_t.size());
            static constexpr auto len{TAG_PEDIA_PREFIX.length()};
            if (tag.substr(0, len) == TAG_PEDIA_PREFIX)
                retval.push_back(tag);
            next_idx += upper_t.size();
        });
        return retval;
    }()),
    m_effects([](auto& effects, const auto& name) {
        for (auto& e : effects)
            e.SetTopLevelContent(name);
        return std::move(effects);
    }(effects, name)),
    m_prerequisites{prerequisites.begin(), prerequisites.end()},
    m_unlocked_items([](auto& unlocked_items) {
        // ensure uniqueness
        std::stable_sort(unlocked_items.begin(), unlocked_items.end());
        auto unique_it = std::unique(unlocked_items.begin(), unlocked_items.end());
        unlocked_items.erase(unique_it, unlocked_items.end());
        return std::move(unlocked_items);
    }(unlocked_items)),
    m_graphic(std::move(graphic))
{}

bool Tech::operator==(const Tech& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_short_description != rhs.m_short_description ||
        m_category != rhs.m_category ||
        m_researchable != rhs.m_researchable ||
        m_tags != rhs.m_tags ||
        m_prerequisites != rhs.m_prerequisites ||
        m_unlocked_items != rhs.m_unlocked_items ||
        m_graphic != rhs.m_graphic ||
        m_unlocked_techs != rhs.m_unlocked_techs)
    { return false; }

    if (m_research_cost == rhs.m_research_cost) { // could be nullptr
        // check next member
    } else if (!m_research_cost || !rhs.m_research_cost) {
        return false;
    } else if (*m_research_cost != *(rhs.m_research_cost)) {
        return false;
    }

    if (m_research_turns == rhs.m_research_turns) { // could be nullptr
        // check next member
    } else if (!m_research_turns || !rhs.m_research_turns) {
        return false;
    } else if (*m_research_turns != *(rhs.m_research_turns)) {
        return false;
    }

    return m_effects == rhs.m_effects;
}

std::string Tech::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Tech\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "short_description = \"" + m_short_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "category = \"" + m_category + "\"\n";
    retval += DumpIndent(ntabs+1) + "researchcost = " + m_research_cost->Dump(ntabs+1) + "\n";
    retval += DumpIndent(ntabs+1) + "researchturns = " + m_research_turns->Dump(ntabs+1) + "\n";
    if (!m_tags.empty()) {
        retval += DumpIndent(ntabs+1) + "tags = ";
        if (m_tags.size() == 1) {
            retval.append("[ \"").append(m_tags.front()).append("\" ]\n");
        } else {
            retval += "[\n";
            for (const auto& tag : m_tags)
                retval.append(DumpIndent(ntabs+2)).append("\"").append(tag).append("\"\n");
            retval += DumpIndent(ntabs+1) + "]\n";
        }
    }
    retval += DumpIndent(ntabs+1) + "prerequisites = ";
    if (m_prerequisites.empty()) {
        retval += "[]\n";
    } else if (m_prerequisites.size() == 1) {
        retval += "\"" + *m_prerequisites.begin() + "\"\n";
    } else {
        retval += "[\n";
        for (const std::string& prerequisite : m_prerequisites)
            retval += DumpIndent(ntabs+2) + "\"" + prerequisite + "\"\n";
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "unlock = ";
    if (m_unlocked_items.empty()) {
        retval += "[]\n";
    } else if (m_unlocked_items.size() == 1) {
        retval += m_unlocked_items[0].Dump();
    } else {
        retval += "[\n";
        for (const UnlockableItem& unlocked_item : m_unlocked_items)
            retval += DumpIndent(ntabs+2) + unlocked_item.Dump();
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    if (!m_effects.empty()) {
        if (m_effects.size() == 1) {
            retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
            retval += m_effects.front().Dump(ntabs+2);
        } else {
            retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
            for (auto& effect : m_effects)
                retval += effect.Dump(ntabs+2);
            retval += DumpIndent(ntabs+1) + "]\n";
        }
    }
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

float Tech::ResearchCost(int empire_id, const ScriptingContext& context) const {
    static constexpr auto ARBITRARY_LARGE_COST = 999999.9f;

    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_TECH_RESEARCH") || !m_research_cost) {
        return 1.0;

    } else if (m_research_cost->ConstantExpr()) {
        return m_research_cost->Eval();

    } else if (m_research_cost->SourceInvariant()) {
        return m_research_cost->Eval();

    } else if (empire_id == ALL_EMPIRES) {
        return ARBITRARY_LARGE_COST;

    } else {
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return ARBITRARY_LARGE_COST;
        auto source = empire->Source(context.ContextObjects());
        if (!source)
            return ARBITRARY_LARGE_COST;
        const ScriptingContext source_context{context, ScriptingContext::Source{}, source.get()};
        return m_research_cost->Eval(source_context);
    }
}

float Tech::PerTurnCost(int empire_id, const ScriptingContext& context) const
{ return ResearchCost(empire_id, context) / std::max(1, ResearchTime(empire_id, context)); }

int Tech::ResearchTime(int empire_id, const ScriptingContext& context) const {
    static constexpr auto ARBITRARY_LARGE_TURNS = 9999;

    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_TECH_RESEARCH") || !m_research_turns) {
        return 1;

    } else if (m_research_turns->ConstantExpr()) {
        return m_research_turns->Eval();

    } else if (m_research_turns->SourceInvariant()) {
        return m_research_turns->Eval();

    } else if (empire_id == ALL_EMPIRES) {
        return ARBITRARY_LARGE_TURNS;

    } else {
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return ARBITRARY_LARGE_TURNS;
        auto source = empire->Source(context.ContextObjects());
        if (!source)
            return ARBITRARY_LARGE_TURNS;
        ScriptingContext source_context{context, ScriptingContext::Source{}, source.get()};

        return m_research_turns->Eval(source_context);
    }
}

uint32_t Tech::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_short_description);
    CheckSums::CheckSumCombine(retval, m_category);
    CheckSums::CheckSumCombine(retval, m_research_cost);
    CheckSums::CheckSumCombine(retval, m_research_turns);
    CheckSums::CheckSumCombine(retval, m_researchable);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_prerequisites);
    CheckSums::CheckSumCombine(retval, m_unlocked_items);
    CheckSums::CheckSumCombine(retval, m_graphic);
    CheckSums::CheckSumCombine(retval, m_unlocked_techs);

    return retval;
}


///////////////////////////////////////////////////////////
// TechManager                                           //
///////////////////////////////////////////////////////////
const Tech* TechManager::GetTech(std::string_view name) const {
    CheckPendingTechs();
    const auto it = m_techs.find(name);
    return it == m_techs.end() ? nullptr : &it->second;
}

const TechCategory* TechManager::GetTechCategory(std::string_view name) const {
    CheckPendingTechs();
    const auto it = m_categories.find(name);
    return it == m_categories.end() ? nullptr : &it->second;
}

std::vector<std::string_view> TechManager::CategoryNames() const {
    CheckPendingTechs();
    std::vector<std::string_view> retval;
    retval.reserve(m_categories.size());
    std::transform(m_categories.begin(), m_categories.end(), std::back_inserter(retval),
                   [](const auto& name_cat) -> std::string_view { return name_cat.first; });
    return retval;
}

std::vector<std::string_view> TechManager::TechNames() const {
    CheckPendingTechs();
    std::vector<std::string_view> retval;
    retval.reserve(m_techs.size());
    std::transform(m_techs.begin(), m_techs.end(), std::back_inserter(retval),
                   [](const auto& name_tech) -> std::string_view { return name_tech.first; });
    return retval;
}

std::vector<std::string_view> TechManager::TechNames(std::string_view name) const {
    CheckPendingTechs();
    std::vector<std::string_view> retval;
    retval.reserve(m_techs.size());
    // transform_if
    for (const auto& name_tech : m_techs)
        if (name_tech.second.Category() == name)
            retval.emplace_back(name_tech.first);
    return retval;
}

std::vector<const Tech*> TechManager::AllNextTechs(const std::vector<std::string_view>& researched_techs) {
    CheckPendingTechs();
    return NextTechs(researched_techs, m_techs);
}

const Tech* TechManager::CheapestNextTech(const std::vector<std::string_view>& researched_techs,
                                          int empire_id, const ScriptingContext& context)
{
    CheckPendingTechs();
    return Cheapest(NextTechs(researched_techs, m_techs), empire_id, context);
}

TechManager::TechContainer::size_type TechManager::size() const {
    CheckPendingTechs();
    return m_techs.size();
}

TechManager::iterator TechManager::begin() const {
    CheckPendingTechs();
    return m_techs.begin();
}

TechManager::iterator TechManager::end() const {
    CheckPendingTechs();
    return m_techs.end();
}

void TechManager::SetTechs(Pending::Pending<TechManager::TechParseTuple>&& future)
{ m_pending_types = std::move(future); }

void TechManager::CheckPendingTechs() const {
    if (!m_pending_types)
        return;

    auto parsed = WaitForPending(m_pending_types);
    if (!parsed)
        return;

    std::set<std::string> categories_seen_in_techs;
    std::tie(m_techs, m_categories, categories_seen_in_techs) = std::move(*parsed);

    // check for missing prerequisites
    for (const auto& [tech_name, tech] : m_techs) {
        for (const auto& prereq_name : tech.Prerequisites()) {
            if (m_techs.find(prereq_name) != m_techs.end())
                continue; // prereq exists

            std::string error_str = "ERROR: Tech \"" + tech_name + "\" requires a missing or malformed tech \"" +
                prereq_name + "\" as its prerequisite.";
            ErrorLogger() << error_str;
            throw std::runtime_error(error_str.c_str());
        }
    }

    // fill in the unlocked techs data for each tech
    for (auto& [tech_name, tech] : m_techs) {
        for (auto& prereq_name : tech.Prerequisites()) {
            auto prereq_it = m_techs.find(prereq_name);
            if (prereq_it == m_techs.end()) {
                // shouldn't reach here if previous check for missing prereqs worked
                ErrorLogger() << "Couldn't find prerequisite tech " << prereq_name << " of tech " << tech_name;
                continue;
            }
            prereq_it->second.m_unlocked_techs.push_back(tech_name);
        }
    }

    // check for empty categories
    std::vector<std::string> empty_defined_categories;
    empty_defined_categories.reserve(m_categories.size());
    for (const auto& cat_name : m_categories | range_keys) {
        auto set_it = categories_seen_in_techs.find(cat_name);
        if (set_it == categories_seen_in_techs.end())
            empty_defined_categories.push_back(cat_name);
        else
            categories_seen_in_techs.erase(set_it);
    }
    if (!empty_defined_categories.empty()) {
        std::stringstream stream;
        for (const auto& empty_defined_category : empty_defined_categories)
            stream << " \"" << empty_defined_category << "\"";
        std::string error_str = "ERROR: The following tech categories were defined, but no "
            "techs were defined that fell within them:" + stream.str();
        ErrorLogger() << error_str;
        std::cerr << error_str << std::endl;
    }
    if (!categories_seen_in_techs.empty()) {
        std::stringstream stream;
        for (const auto& category_seen_in_techs : categories_seen_in_techs)
            stream << " \"" << category_seen_in_techs << "\"";
        std::string error_str = "ERROR: The following tech categories were never defined, but some "
            "techs were defined that fell within them:" + stream.str();
        ErrorLogger() << error_str;
        std::cerr << error_str << std::endl;
    }

    // find cyclical dependencies
    const std::string cycle_str = FindFirstDependencyCycle();
    if (!cycle_str.empty()) {
        ErrorLogger() << cycle_str;
        throw std::runtime_error(cycle_str.c_str());
    }

    const std::string redundant_dependency = FindRedundantDependency();
    if (!redundant_dependency.empty())
        ErrorLogger() << redundant_dependency;
}

std::string TechManager::FindFirstDependencyCycle() const {
    CheckPendingTechs();
    assert(!m_techs.empty());

    std::set<const Tech*> checked_techs; // the list of techs that are not part of any cycle
    for (const auto& [tech_name, tech] : m_techs) {
        if (checked_techs.contains(&tech))
            continue;

        std::vector<const Tech*> stack;
        stack.reserve(m_techs.size());
        stack.push_back(&tech);
        while (!stack.empty()) {
            // Examine the tech on top of the stack.  If the tech has no prerequisite techs, or if all
            // of its prerequisite techs have already been checked, pop it off the stack and mark it as
            // checked; otherwise, push all its unchecked prerequisites onto the stack.
            const Tech* current_tech = stack.back();
            const auto starting_stack_size = stack.size();

            if (current_tech) {
                for (auto& prereq_name : current_tech->Prerequisites()) {
                    const Tech* prereq_tech = this->GetTech(prereq_name);
                    if (!prereq_tech || checked_techs.contains(prereq_tech))
                        continue;

                    // since this is not a checked prereq, see if it is already in the stack somewhere;
                    // if it is, we have a cycle
                    const auto stack_duplicate_it = std::find(stack.rbegin(), stack.rend(), prereq_tech);
                    if (stack_duplicate_it == stack.rend()) {
                        // OK! no cycle, move to next prereq
                        stack.push_back(prereq_tech);
                        continue;
                    }

                    std::string_view current_tech_name = prereq_tech->Name();
                    std::stringstream stream;
                    stream << "ERROR: Tech dependency cycle found (A <-- B means A is a prerequisite of B): \""
                           << current_tech_name << "\"";
                    for (auto stack_it = stack.rbegin(); stack_it != stack_duplicate_it; ++stack_it) {
                        const auto& prereqs = (*stack_it)->Prerequisites();
                        if (std::count(prereqs.begin(), prereqs.end(), current_tech_name)) {
                            current_tech_name = (*stack_it)->Name();
                            stream << " <-- \"" << current_tech_name << "\"";
                        }
                    }
                    stream << " <-- \"" << prereq_tech->Name() << "\" ... ";
                    return stream.str();
                }
            }
            if (starting_stack_size == stack.size()) {
                stack.pop_back();
                checked_techs.insert(current_tech);
            }
        }
    }
    return "";
}

std::string TechManager::FindRedundantDependency() const {
    CheckPendingTechs();
    assert(!m_techs.empty());

    for (const auto& [tech_name, tech] : m_techs) {
        const auto& prereqs = tech.Prerequisites();
        std::map<std::string, std::string> techs_unlocked_by_prereqs;
        for (const auto& prereq_name : prereqs) {
            const Tech* prereq_tech = GetTech(prereq_name);
            if (!prereq_tech)
                return std::string{"ERROR: Tech \""}.append(tech_name)
                    .append("\" requires a missing or malformed tech \"")
                    .append(prereq_name).append("\" as its prerequisite.");
            AllChildren(prereq_tech, techs_unlocked_by_prereqs);
        }
        for (const auto& prereq_name : prereqs) {
            const auto map_it = techs_unlocked_by_prereqs.find(prereq_name);
            if (map_it == techs_unlocked_by_prereqs.end())
                continue;

            return std::string{"ERROR: Redundant tech dependency found (A <-- B means A is a prerequisite of B): "}
                .append(map_it->second).append(" <-- ").append(map_it->first).append(", ")
                .append(map_it->first).append(" <-- ").append(tech_name).append(", ")
                .append(map_it->second).append(" <-- ").append(tech_name).append("; remove the ")
                .append(map_it->second).append(" <-- ").append(tech_name).append(" dependency.");
        }
    }
    return "";
}

void TechManager::AllChildren(const Tech* tech, std::map<std::string, std::string>& children) const {
    for (const std::string& unlocked_tech : tech->UnlockedTechs()) {
        if (unlocked_tech == tech->Name()) {
            // infinite loop
            ErrorLogger() << "Tech " << unlocked_tech << " unlocks itself";
            continue;
        }
        children[unlocked_tech] = tech->Name();
        AllChildren(GetTech(unlocked_tech), children);
    }
}

std::vector<std::string> TechManager::RecursivePrereqs(std::string_view tech_name, int empire_id,
                                                       const ScriptingContext& context) const
{
    const Tech* initial_tech = this->GetTech(tech_name);
    if (!initial_tech)
        return {};

    // compile set of recursive prereqs
    std::list<std::string> prereqs_list{initial_tech->Prerequisites().begin(), // working list of prereqs as being processed.  may contain duplicates
                                        initial_tech->Prerequisites().end()};  // initialized with 1st order prereqs
    std::set<std::string> prereqs_set;                                         // set of (unique) prereqs leading to tech
    std::multimap<float, std::string> techs_to_add_map;                        // indexed and sorted by cost per turn

    // traverse list, appending new prereqs to it, and putting unique prereqs into set
    for (std::string& cur_name : prereqs_list) {
        // check if this tech is already in the map of prereqs.  If so, it has already been processed, and can be skipped.
        if (prereqs_set.contains(cur_name))
            continue;

        // tech is new, so put it into the set of already-processed prereqs
        prereqs_set.insert(cur_name);

        // and the map of techs, sorted by cost
        const Tech* cur_tech = this->GetTech(cur_name);
        techs_to_add_map.emplace(cur_tech->ResearchCost(empire_id, context), std::move(cur_name));

        // get prereqs of new tech, append to list
        prereqs_list.insert(prereqs_list.end(), cur_tech->Prerequisites().begin(),
                            cur_tech->Prerequisites().end());
    }

    // extract sorted techs into vector, to be passed to signal...
    std::vector<std::string> retval;
    retval.reserve(techs_to_add_map.size());
    for (auto& tech_to_add : techs_to_add_map)
        retval.push_back(std::move(tech_to_add.second));

    return retval;
}

uint32_t TechManager::GetCheckSum() const {
    CheckPendingTechs();
    uint32_t retval{0};
    for (auto const& name_type_pair : m_categories)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_categories.size());

    for (auto const& tech : *this)
        CheckSums::CheckSumCombine(retval, tech);
    CheckSums::CheckSumCombine(retval, m_techs.size());

    DebugLogger() << "TechManager checksum: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
namespace {
    TechManager tech_manager;
}

TechManager& GetTechManager()
{ return tech_manager; }

const Tech* GetTech(std::string_view name)
{ return tech_manager.GetTech(name); }

const TechCategory* GetTechCategory(std::string_view name)
{ return tech_manager.GetTechCategory(name); }
