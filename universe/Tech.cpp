#include "Tech.h"

#include "Effect.h"
#include "UniverseObject.h"
#include "ObjectMap.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "ValueRef.h"
#include "Enums.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    void AddRules(GameRules& rules) {
        // makes all techs cost 1 RP and take 1 turn to research
        rules.Add<bool>("RULE_CHEAP_AND_FAST_TECH_RESEARCH",
                        "RULE_CHEAP_AND_FAST_TECH_RESEARCH_DESC",
                        "", false, true);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

namespace {
    void NextTechs(std::vector<const Tech*>& retval, const std::set<std::string>& known_techs,
                   std::set<const Tech*>& checked_techs,
                   TechManager::iterator it, TechManager::iterator end_it)
    {
        if (checked_techs.count(it->get()))
            return;

        if (!known_techs.count((*it)->Name()) && it != end_it) {
            std::vector<const Tech*> stack;
            stack.push_back(it->get());
            while (!stack.empty()) {
                const Tech* current_tech = stack.back();
                unsigned int starting_stack_size = stack.size();
                bool all_prereqs_known = true;
                for (const std::string& prereq_name : current_tech->Prerequisites()) {
                    const Tech* prereq_tech = GetTech(prereq_name);
                    bool prereq_unknown = !known_techs.count(prereq_tech->Name());
                    if (prereq_unknown)
                        all_prereqs_known = false;
                    if (!checked_techs.count(prereq_tech) && prereq_unknown)
                        stack.push_back(prereq_tech);
                }
                if (starting_stack_size == stack.size()) {
                    stack.pop_back();
                    checked_techs.insert(current_tech);
                    if (all_prereqs_known)
                        retval.push_back(current_tech);
                }
            }
        }
    }

    const Tech* Cheapest(const std::vector<const Tech*>& next_techs, int empire_id) {
        if (next_techs.empty())
            return nullptr;

        float min_price = next_techs[0]->ResearchCost(empire_id);
        int min_index = 0;
        for (unsigned int i = 0; i < next_techs.size(); ++i) {
            float price = next_techs[i]->ResearchCost(empire_id);
            if (price < min_price) {
                min_price = price;
                min_index = i;
            }
        }

        return next_techs[min_index];
    }
}

namespace CheckSums {
    void CheckSumCombine(unsigned int& sum, const ItemSpec& item) {
        TraceLogger() << "CheckSumCombine(Slot): " << typeid(item).name();
        CheckSumCombine(sum, item.type);
        CheckSumCombine(sum, item.name);
    }

    void CheckSumCombine(unsigned int& sum, const TechCategory& cat) {
        TraceLogger() << "CheckSumCombine(Slot): " << typeid(cat).name();
        CheckSumCombine(sum, cat.name);
        CheckSumCombine(sum, cat.graphic);
        CheckSumCombine(sum, cat.colour.r);
        CheckSumCombine(sum, cat.colour.g);
        CheckSumCombine(sum, cat.colour.b);
        CheckSumCombine(sum, cat.colour.a);
    }
}

///////////////////////////////////////////////////////////
// Tech Info                                             //
///////////////////////////////////////////////////////////
Tech::TechInfo::TechInfo() :
    research_cost(nullptr),
    research_turns(nullptr),
    researchable(false)
{}

Tech::TechInfo::TechInfo(const std::string& name_, const std::string& description_, const std::string& short_description_,
                   const std::string& category_,
                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& research_cost_,
                   std::unique_ptr<ValueRef::ValueRefBase<int>>&& research_turns_,
                   bool researchable_,
                   const std::set<std::string>& tags_) :
    name(name_),
    description(description_),
    short_description(short_description_),
    category(category_),
    research_cost(std::move(research_cost_)),
    research_turns(std::move(research_turns_)),
    researchable(researchable_),
    tags(tags_)
{}

Tech::TechInfo::~TechInfo()
{}

///////////////////////////////////////////////////////////
// Tech                                                  //
///////////////////////////////////////////////////////////
Tech::Tech(const std::string& name, const std::string& description, const std::string& short_description,
           const std::string& category,
           std::unique_ptr<ValueRef::ValueRefBase<double>>&& research_cost,
           std::unique_ptr<ValueRef::ValueRefBase<int>>&& research_turns,
           bool researchable,
           const std::set<std::string>& tags,
           const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects,
           const std::set<std::string>& prerequisites, const std::vector<ItemSpec>& unlocked_items,
           const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_short_description(short_description),
    m_category(category),
    m_research_cost(std::move(research_cost)),
    m_research_turns(std::move(research_turns)),
    m_researchable(researchable),
    m_tags(),
    m_effects(effects),
    m_prerequisites(prerequisites),
    m_unlocked_items(unlocked_items),
    m_graphic(graphic)
{
    for (const std::string& tag : tags)
        m_tags.insert(boost::to_upper_copy<std::string>(tag));
    Init();
}

Tech::Tech(TechInfo& tech_info,
           std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
           const std::set<std::string>& prerequisites, const std::vector<ItemSpec>& unlocked_items,
           const std::string& graphic) :
    m_name(tech_info.name),
    m_description(tech_info.description),
    m_short_description(tech_info.short_description),
    m_category(tech_info.category),
    m_research_cost(std::move(tech_info.research_cost)),
    m_research_turns(std::move(tech_info.research_turns)),
    m_researchable(tech_info.researchable),
    m_tags(),
    m_effects(),
    m_prerequisites(prerequisites),
    m_unlocked_items(unlocked_items),
    m_graphic(graphic)
{
    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));
    for (const std::string& tag : tech_info.tags)
        m_tags.insert(boost::to_upper_copy<std::string>(tag));
    Init();
}

Tech::~Tech()
{}

void Tech::Init() {
    if (m_research_cost)
        m_research_cost->SetTopLevelContent(m_name);
    if (m_research_turns)
        m_research_turns->SetTopLevelContent(m_name);

    for (auto& effect : m_effects)
    { effect->SetTopLevelContent(m_name); }
}

std::string Tech::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Tech\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "shortdescription = \"" + m_short_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "category = \"" + m_category + "\"\n";
    retval += DumpIndent(ntabs+1) + "researchcost = " + m_research_cost->Dump(ntabs+1) + "\n";
    retval += DumpIndent(ntabs+1) + "researchturns = " + m_research_turns->Dump(ntabs+1) + "\n";
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
        for (const ItemSpec& unlocked_item : m_unlocked_items)
            retval += DumpIndent(ntabs+2) + unlocked_item.Dump();
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    if (!m_effects.empty()) {
        if (m_effects.size() == 1) {
            retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
            retval += m_effects[0]->Dump(ntabs+2);
        } else {
            retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
            for (auto& effect : m_effects)
                retval += effect->Dump(ntabs+2);
            retval += DumpIndent(ntabs+1) + "]\n";
        }
    }
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

float Tech::ResearchCost(int empire_id) const {
    const auto arbitrary_large_number = 999999.9f;

    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_TECH_RESEARCH") || !m_research_cost) {
        return 1.0;

    } else if (m_research_cost->ConstantExpr()) {
        return m_research_cost->Eval();

    } else if (m_research_cost->SourceInvariant()) {
        return m_research_cost->Eval();

    } else if (empire_id == ALL_EMPIRES) {
        return arbitrary_large_number;

    } else {
        std::shared_ptr<const UniverseObject> source = Empires().GetSource(empire_id);
        if (!source && !m_research_cost->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source);
        return m_research_cost->Eval(context);
    }
}

float Tech::PerTurnCost(int empire_id) const
{ return ResearchCost(empire_id) / std::max(1, ResearchTime(empire_id)); }

int Tech::ResearchTime(int empire_id) const {
    const auto arbitrary_large_number = 9999;

    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_TECH_RESEARCH") || !m_research_turns) {
        return 1;

    } else if (m_research_turns->ConstantExpr()) {
            return m_research_turns->Eval();

    } else if (m_research_turns->SourceInvariant()) {
            return m_research_turns->Eval();

    } else if (empire_id == ALL_EMPIRES) {
        return arbitrary_large_number;

    } else {
        std::shared_ptr<const UniverseObject> source = Empires().GetSource(empire_id);
        if (!source && !m_research_turns->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source);

        return m_research_turns->Eval(context);
    }
}

unsigned int Tech::GetCheckSum() const {
    unsigned int retval{0};

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
// ItemSpec                                              //
///////////////////////////////////////////////////////////
ItemSpec::ItemSpec() :
    type(INVALID_UNLOCKABLE_ITEM_TYPE),
    name()
{}

std::string ItemSpec::Dump(unsigned short ntabs) const {
    std::string retval = "Item type = ";
    switch (type) {
    case UIT_BUILDING:      retval += "Building";   break;
    case UIT_SHIP_PART:     retval += "ShipPart";   break;
    case UIT_SHIP_HULL:     retval += "ShipHull";   break;
    case UIT_SHIP_DESIGN:   retval += "ShipDesign"; break;
    case UIT_TECH:          retval += "Tech"    ;   break;
    default:                retval += "?"       ;   break;
    }
    retval += " name = \"" + name + "\"\n";
    return retval;
}

bool operator==(const ItemSpec& lhs, const ItemSpec& rhs) {
    return lhs.type == rhs.type &&
    lhs.name == rhs.name;
}

bool operator!=(const ItemSpec& lhs, const ItemSpec& rhs)
{ return !(lhs == rhs); }

///////////////////////////////////////////////////////////
// TechManager                                           //
///////////////////////////////////////////////////////////
// static(s)
TechManager* TechManager::s_instance = nullptr;

const Tech* TechManager::GetTech(const std::string& name) const {
    CheckPendingTechs();
    iterator it = m_techs.get<NameIndex>().find(name);
    return it == m_techs.get<NameIndex>().end() ? nullptr : it->get();
}

const TechCategory* TechManager::GetTechCategory(const std::string& name) const {
    CheckPendingTechs();
    auto it = m_categories.find(name);
    return it == m_categories.end() ? nullptr : it->second.get();
}

std::vector<std::string> TechManager::CategoryNames() const {
    CheckPendingTechs();
    std::vector<std::string> retval;
    for (const auto& entry : m_categories)
        retval.push_back(entry.first);
    return retval;
}

std::vector<std::string> TechManager::TechNames() const {
    CheckPendingTechs();
    std::vector<std::string> retval;
    for (const auto& tech : m_techs.get<NameIndex>())
        retval.push_back(tech->Name());
    return retval;
}

std::vector<std::string> TechManager::TechNames(const std::string& name) const {
    CheckPendingTechs();
    std::vector<std::string> retval;
    for (TechManager::category_iterator it = category_begin(name); it != category_end(name); ++it) {
        retval.push_back((*it)->Name());
    }
    return retval;
}

std::vector<const Tech*> TechManager::AllNextTechs(const std::set<std::string>& known_techs) {
    CheckPendingTechs();
    std::vector<const Tech*> retval;
    std::set<const Tech*> checked_techs;
    iterator end_it = m_techs.get<NameIndex>().end();
    for (iterator it = m_techs.get<NameIndex>().begin(); it != end_it; ++it) {
        NextTechs(retval, known_techs, checked_techs, it, end_it);
    }
    return retval;
}

const Tech* TechManager::CheapestNextTech(const std::set<std::string>& known_techs, int empire_id) {
    CheckPendingTechs();
    return Cheapest(AllNextTechs(known_techs), empire_id);
}

std::vector<const Tech*> TechManager::NextTechsTowards(const std::set<std::string>& known_techs,
                                                       const std::string& desired_tech,
                                                       int empire_id)
{
    CheckPendingTechs();
    std::vector<const Tech*> retval;
    std::set<const Tech*> checked_techs;
    NextTechs(retval, known_techs, checked_techs, m_techs.get<NameIndex>().find(desired_tech),
              m_techs.get<NameIndex>().end());
    return retval;
}

const Tech* TechManager::CheapestNextTechTowards(const std::set<std::string>& known_techs,
                                                 const std::string& desired_tech,
                                                 int empire_id)
{ return Cheapest(NextTechsTowards(known_techs, desired_tech, empire_id), empire_id); }

TechManager::iterator TechManager::begin() const {
    CheckPendingTechs();
    return m_techs.get<NameIndex>().begin();
}

TechManager::iterator TechManager::end() const {
    CheckPendingTechs();
    return m_techs.get<NameIndex>().end();
}

TechManager::category_iterator TechManager::category_begin(const std::string& name) const {
    CheckPendingTechs();
    return m_techs.get<CategoryIndex>().lower_bound(name);
}

TechManager::category_iterator TechManager::category_end(const std::string& name) const {
    CheckPendingTechs();
    return m_techs.get<CategoryIndex>().upper_bound(name);
}

TechManager::TechManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one TechManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
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


    std::set<std::string> empty_defined_categories;
    for (const auto& map : m_categories) {
        auto set_it = categories_seen_in_techs.find(map.first);
        if (set_it == categories_seen_in_techs.end()) {
            empty_defined_categories.insert(map.first);
        } else {
            categories_seen_in_techs.erase(set_it);
        }
    }

    if (!empty_defined_categories.empty()) {
        std::stringstream stream;
        for (const std::string& empty_defined_category : empty_defined_categories) {
            stream << " \"" << empty_defined_category << "\"";
        }
        std::string error_str = "ERROR: The following tech categories were defined, but no "
            "techs were defined that fell within them:" + stream.str();
        ErrorLogger() << error_str;
        std::cerr << error_str << std::endl;
    }

    if (!categories_seen_in_techs.empty()) {
        std::stringstream stream;
        for (const std::string& category_seen_in_techs : categories_seen_in_techs) {
            stream << " \"" << category_seen_in_techs << "\"";
        }
        std::string error_str = "ERROR: The following tech categories were never defined, but some "
            "techs were defined that fell within them:" + stream.str();
        ErrorLogger() << error_str;
        std::cerr << error_str << std::endl;
    }

    std::string illegal_dependency_str = FindIllegalDependencies();
    if (!illegal_dependency_str.empty()) {
        ErrorLogger() << illegal_dependency_str;
        throw std::runtime_error(illegal_dependency_str.c_str());
    }

    std::string cycle_str = FindFirstDependencyCycle();
    if (!cycle_str.empty()) {
        ErrorLogger() << cycle_str;
        throw std::runtime_error(cycle_str.c_str());
    }

    // fill in the unlocked techs data for each loaded tech
    for (const auto& tech : m_techs) {
        for (const std::string& prereq : tech->Prerequisites()) {
            if (Tech* prereq_tech = const_cast<Tech*>(GetTech(prereq)))
                prereq_tech->m_unlocked_techs.insert(tech->Name());
        }
    }

    std::string redundant_dependency = FindRedundantDependency();
    if (!redundant_dependency.empty())
        ErrorLogger() << redundant_dependency;

#ifdef OUTPUT_TECH_LIST
    for (const Tech* tech : m_techs.get<NameIndex>()) {
        std::cerr << UserString(tech->Name()) << " ("
                  << UserString(tech->Category()) << ") - "
                  << tech->Graphic() << std::endl;
    }
#endif
}

std::string TechManager::FindIllegalDependencies() const {
    CheckPendingTechs();
    assert(!m_techs.empty());
    std::string retval;
    for (const auto& tech : m_techs) {
        if (!tech) {
            std::stringstream stream;
            stream << "ERROR: Missing tech referenced in other tech, for unknown reasons...";
            return stream.str();
        }

        for (const std::string& prereq : tech->Prerequisites()) {
            const Tech* prereq_tech = GetTech(prereq);
            if (!prereq_tech) {
                std::stringstream stream;
                stream << "ERROR: Tech \"" << tech->Name() << "\" requires a missing or malformed tech \"" << prereq << "\" as its prerequisite.";
                return stream.str();
            }
        }
    }
    return retval;
}

std::string TechManager::FindFirstDependencyCycle() const {
    CheckPendingTechs();
    assert(!m_techs.empty());
    static const std::set<std::string> EMPTY_STRING_SET;    // used in case an invalid tech is processed

    std::set<const Tech*> checked_techs; // the list of techs that are not part of any cycle
    for (const auto& tech : *this) {
        if (checked_techs.count(tech.get()))
            continue;

        std::vector<const Tech*> stack;
        stack.push_back(tech.get());
        while (!stack.empty()) {
            // Examine the tech on top of the stack.  If the tech has no prerequisite techs, or if all
            // of its prerequisite techs have already been checked, pop it off the stack and mark it as
            // checked; otherwise, push all its unchecked prerequisites onto the stack.
            const Tech* current_tech = stack.back();
            unsigned int starting_stack_size = stack.size();

            const std::set<std::string>& prereqs = (current_tech ? current_tech->Prerequisites() : EMPTY_STRING_SET);
            for (const std::string& prereq_name : prereqs) {
                const Tech* prereq_tech = GetTech(prereq_name);
                if (!prereq_tech || checked_techs.count(prereq_tech))
                    continue;

                // since this is not a checked prereq, see if it is already in the stack somewhere; if so, we have a cycle
                std::vector<const Tech*>::reverse_iterator stack_duplicate_it =
                    std::find(stack.rbegin(), stack.rend(), prereq_tech);
                if (stack_duplicate_it != stack.rend()) {
                    std::stringstream stream;
                    std::string current_tech_name = prereq_tech->Name();
                    stream << "ERROR: Tech dependency cycle found (A <-- B means A is a prerequisite of B): \""
                            << current_tech_name << "\"";
                    for (std::vector<const Tech*>::reverse_iterator stack_it = stack.rbegin();
                            stack_it != stack_duplicate_it;
                            ++stack_it) {
                        if ((*stack_it)->Prerequisites().count(current_tech_name)) {
                            current_tech_name = (*stack_it)->Name();
                            stream << " <-- \"" << current_tech_name << "\"";
                        }
                    }
                    stream << " <-- \"" << prereq_tech->Name() << "\" ... ";
                    return stream.str();
                } else {
                    stack.push_back(prereq_tech);
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

    for (const auto& tech : m_techs) {
        if (!tech) {
            std::stringstream stream;
            stream << "ERROR: Missing referenced tech for unknown reasons...";
            return stream.str();
        }
        std::set<std::string> prereqs = tech->Prerequisites();
        std::map<std::string, std::string> techs_unlocked_by_prereqs;
        for (const std::string& prereq_name : prereqs) {
            const Tech* prereq_tech = GetTech(prereq_name);
            if (!prereq_tech) {
                std::stringstream stream;
                stream << "ERROR: Tech \"" << tech->Name() << "\" requires a missing or malformed tech \"" << prereq_name << "\" as its prerequisite.";
                return stream.str();
            }
            AllChildren(prereq_tech, techs_unlocked_by_prereqs);
        }
        for (const std::string& prereq_name : prereqs) {
            auto map_it = techs_unlocked_by_prereqs.find(prereq_name);
            if (map_it != techs_unlocked_by_prereqs.end()) {
                std::stringstream stream;
                stream << "ERROR: Redundant tech dependency found (A <-- B means A is a prerequisite of B): "
                       << map_it->second << " <-- " << map_it->first << ", "
                       << map_it->first << " <-- " << tech->Name() << ", "
                       << map_it->second << " <-- " << tech->Name() << "; remove the " << map_it->second << " <-- " << tech->Name()
                       << " dependency.";
                return stream.str();
            }
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

TechManager& TechManager::GetTechManager() {
    static TechManager manager;
    return manager;
}

std::vector<std::string> TechManager::RecursivePrereqs(const std::string& tech_name, int empire_id, bool min_required /*= true*/) const {
    const Tech* tech = this->GetTech(tech_name);
    if (!tech)
        return std::vector<std::string>();

    // compile set of recursive prereqs
    std::list<std::string> prereqs_list;                    // working list of prereqs as being processed.  may contain duplicates
    std::set<std::string> prereqs_set;                      // set of (unique) prereqs leading to tech
    std::multimap<float, std::string> techs_to_add_map;    // indexed and sorted by cost per turn

    // initialize working list with 1st order prereqs
    std::set<std::string> cur_prereqs = tech->Prerequisites();
    std::copy(cur_prereqs.begin(), cur_prereqs.end(), std::back_inserter(prereqs_list));
    const Empire* empire = GetEmpire(empire_id);

    // traverse list, appending new prereqs to it, and putting unique prereqs into set
    for (const std::string& cur_name : prereqs_list) {
        const Tech* cur_tech = this->GetTech(cur_name);

        // check if this tech is already in the map of prereqs.  If so, it has already been processed, and can be skipped.
        if (prereqs_set.count(cur_name))
            continue;

        // if this tech is already known and min_required==true, can skip.
        if (min_required && empire && (empire->GetTechStatus(cur_name) == TS_COMPLETE))
            continue;

        // tech is new, so put it into the set of already-processed prereqs
        prereqs_set.insert(cur_name);
        // and the map of techs, sorted by cost
        techs_to_add_map.insert(std::pair<float, std::string>(cur_tech->ResearchCost(empire_id), cur_name));

        // get prereqs of new tech, append to list
        cur_prereqs = cur_tech->Prerequisites();
        std::copy(cur_prereqs.begin(), cur_prereqs.end(), std::back_inserter(prereqs_list));
    }

    // extract sorted techs into vector, to be passed to signal...
    std::vector<std::string> retval;
    for (const auto& tech_to_add : techs_to_add_map)
    { retval.push_back(tech_to_add.second); }

    return retval;
}

unsigned int TechManager::GetCheckSum() const {
    CheckPendingTechs();
    unsigned int retval{0};
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
TechManager& GetTechManager()
{ return TechManager::GetTechManager(); }

const Tech* GetTech(const std::string& name)
{ return GetTechManager().GetTech(name); }

const TechCategory* GetTechCategory(const std::string& name)
{ return GetTechManager().GetTechCategory(name); }
