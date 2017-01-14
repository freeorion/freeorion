#include "Tech.h"

#include "Effect.h"
#include "UniverseObject.h"
#include "ObjectMap.h"
#include "../parse/Parse.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "ValueRef.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>

namespace {
    const bool CHEAP_AND_FAST_TECH_RESEARCH = false;    // makes all techs cost 1 RP and take 1 turn to research
}

namespace {
    void NextTechs(std::vector<const Tech*>& retval, const std::set<std::string>& known_techs,
                   std::set<const Tech*>& checked_techs,
                   TechManager::iterator it, TechManager::iterator end_it)
    {
        if (checked_techs.find(*it) != checked_techs.end())
            return;

        if (known_techs.find((*it)->Name()) == known_techs.end() && it != end_it) {
            std::vector<const Tech*> stack;
            stack.push_back(*it);
            while (!stack.empty()) {
                const Tech* current_tech = stack.back();
                unsigned int starting_stack_size = stack.size();
                bool all_prereqs_known = true;
                for (const std::string& prereq_name : current_tech->Prerequisites()) {
                    const Tech* prereq_tech = GetTech(prereq_name);
                    bool prereq_unknown = known_techs.find(prereq_tech->Name()) == known_techs.end();
                    if (prereq_unknown)
                        all_prereqs_known = false;
                    if (checked_techs.find(prereq_tech) == checked_techs.end() && prereq_unknown)
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
            return 0;

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


///////////////////////////////////////////////////////////
// Tech                                                  //
///////////////////////////////////////////////////////////
void Tech::Init() {
    if (m_research_cost)
        m_research_cost->SetTopLevelContent(m_name);
    if (m_research_turns)
        m_research_turns->SetTopLevelContent(m_name);

    for (boost::shared_ptr<Effect::EffectsGroup> effect : m_effects)
    { effect->SetTopLevelContent(m_name); }
}

std::string Tech::Dump() const {
    using boost::lexical_cast;

    std::string retval = DumpIndent() + "Tech\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    retval += DumpIndent() + "shortdescription = \"" + m_short_description + "\"\n";
    retval += DumpIndent() + "category = \"" + m_category + "\"\n";
    retval += DumpIndent() + "researchcost = " + m_research_cost->Dump() + "\n";
    retval += DumpIndent() + "researchturns = " + m_research_turns->Dump() + "\n";
    retval += DumpIndent() + "prerequisites = ";
    if (m_prerequisites.empty()) {
        retval += "[]\n";
    } else if (m_prerequisites.size() == 1) {
        retval += "\"" + *m_prerequisites.begin() + "\"\n";
    } else {
        retval += "[\n";
        ++g_indent;
        for (const std::string& prerequisite : m_prerequisites) {
            retval += DumpIndent() + "\"" + prerequisite + "\"\n";
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    retval += DumpIndent() + "unlock = ";
    if (m_unlocked_items.empty()) {
        retval += "[]\n";
    } else if (m_unlocked_items.size() == 1) {
        retval += m_unlocked_items[0].Dump();
    } else {
        retval += "[\n";
        ++g_indent;
        for (const ItemSpec& unlocked_item : m_unlocked_items) {
            retval += DumpIndent() + unlocked_item.Dump();
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    if (!m_effects.empty()) {
        if (m_effects.size() == 1) {
            retval += DumpIndent() + "effectsgroups =\n";
            ++g_indent;
            retval += m_effects[0]->Dump();
            --g_indent;
        } else {
            retval += DumpIndent() + "effectsgroups = [\n";
            ++g_indent;
            for (boost::shared_ptr<Effect::EffectsGroup> effect : m_effects) {
                retval += effect->Dump();
            }
            --g_indent;
            retval += DumpIndent() + "]\n";
        }
    }
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

namespace {
    TemporaryPtr<const UniverseObject> SourceForEmpire(int empire_id) {
        const Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            DebugLogger() << "SourceForEmpire: Unable to get empire with ID: " << empire_id;
            return TemporaryPtr<const UniverseObject>();
        }
        // get a source object, which is owned by the empire with the passed-in
        // empire id.  this is used in conditions to reference which empire is
        // doing the building.  Ideally this will be the capital, but any object
        // owned by the empire will work.
        TemporaryPtr<const UniverseObject> source = GetUniverseObject(empire->CapitalID());
        // no capital?  scan through all objects to find one owned by this empire
        if (!source) {
            for (TemporaryPtr<const UniverseObject> obj : Objects()) {
                if (obj->OwnedBy(empire_id)) {
                    source = obj;
                    break;
                }
            }
        }
        return source;
    }
}

float Tech::ResearchCost(int empire_id) const {
    if (CHEAP_AND_FAST_TECH_RESEARCH || !m_research_cost) {
        return 1.0;

    } else if (m_research_cost->ConstantExpr()) {
        return m_research_cost->Eval();

    } else if (empire_id == ALL_EMPIRES) {
        return 999999.9f;

    } else {
        TemporaryPtr<const UniverseObject> source = SourceForEmpire(empire_id);
        if (!source && !m_research_cost->SourceInvariant())
            return 999999.9f;

        ScriptingContext context(source);
        return m_research_cost->Eval(context);
    }
}

float Tech::PerTurnCost(int empire_id) const
{ return ResearchCost(empire_id) / std::max(1, ResearchTime(empire_id)); }

int Tech::ResearchTime(int empire_id) const {
    if (CHEAP_AND_FAST_TECH_RESEARCH || !m_research_turns) {
        return 1;

    } else if (m_research_turns->ConstantExpr()) {
            return m_research_turns->Eval();

    } else if (empire_id == ALL_EMPIRES) {
        return 9999;

    } else {
        TemporaryPtr<const UniverseObject> source = SourceForEmpire(empire_id);
        if (!source && !m_research_turns->SourceInvariant())
            return 9999;

        ScriptingContext context(source);

        return m_research_turns->Eval(context);
    }
}


///////////////////////////////////////////////////////////
// ItemSpec                                        //
///////////////////////////////////////////////////////////
std::string ItemSpec::Dump() const {
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
TechManager* TechManager::s_instance = 0;

const Tech* TechManager::GetTech(const std::string& name) const {
    iterator it = m_techs.get<NameIndex>().find(name);
    return it == m_techs.get<NameIndex>().end() ? 0 : *it;
}

const TechCategory* TechManager::GetTechCategory(const std::string& name) const {
    std::map<std::string, TechCategory*>::const_iterator it = m_categories.find(name);
    return it == m_categories.end() ? 0 : it->second;
}

std::vector<std::string> TechManager::CategoryNames() const {
    std::vector<std::string> retval;
    for (const std::map<std::string, TechCategory*>::value_type& entry : m_categories)
        retval.push_back(entry.first);
    return retval;
}

std::vector<std::string> TechManager::TechNames() const {
    std::vector<std::string> retval;
    for (const TechContainer::value_type& tech : m_techs.get<NameIndex>())
        retval.push_back(tech->Name());
    return retval;
}

std::vector<std::string> TechManager::TechNames(const std::string& name) const {
    std::vector<std::string> retval;
    for (TechManager::category_iterator it = category_begin(name); it != category_end(name); ++it) {
        retval.push_back((*it)->Name());
    }
    return retval;
}

std::vector<const Tech*> TechManager::AllNextTechs(const std::set<std::string>& known_techs) {
    std::vector<const Tech*> retval;
    std::set<const Tech*> checked_techs;
    iterator end_it = m_techs.get<NameIndex>().end();
    for (iterator it = m_techs.get<NameIndex>().begin(); it != end_it; ++it) {
        NextTechs(retval, known_techs, checked_techs, it, end_it);
    }
    return retval;
}

const Tech* TechManager::CheapestNextTech(const std::set<std::string>& known_techs, int empire_id)
{ return Cheapest(AllNextTechs(known_techs), empire_id); }

std::vector<const Tech*> TechManager::NextTechsTowards(const std::set<std::string>& known_techs,
                                                       const std::string& desired_tech,
                                                       int empire_id)
{
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

TechManager::iterator TechManager::begin() const
{ return m_techs.get<NameIndex>().begin(); }

TechManager::iterator TechManager::end() const
{ return m_techs.get<NameIndex>().end(); }

TechManager::category_iterator TechManager::category_begin(const std::string& name) const
{ return m_techs.get<CategoryIndex>().lower_bound(name); }

TechManager::category_iterator TechManager::category_end(const std::string& name) const
{ return m_techs.get<CategoryIndex>().upper_bound(name); }

TechManager::TechManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one TechManager.");

    s_instance = this;

    std::set<std::string> categories_seen_in_techs;

    try {
        parse::techs(m_techs, m_categories, categories_seen_in_techs);
    } catch (const std::exception& e) {
        ErrorLogger() << "Failed parsing techs: error: " << e.what();
        throw e;
    }

    std::set<std::string> empty_defined_categories;
    for (const std::map<std::string, TechCategory*>::value_type& map : m_categories) {
        std::set<std::string>::iterator set_it = categories_seen_in_techs.find(map.first);
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
        std::string error_str = "ERROR: The following categories were defined in techs.txt, but no "
            "techs were defined that fell within them:" + stream.str();
        ErrorLogger() << error_str;
        std::cerr << error_str << std::endl;
    }

    if (!categories_seen_in_techs.empty()) {
        std::stringstream stream;
        for (const std::string& category_seen_in_techs : categories_seen_in_techs) {
            stream << " \"" << category_seen_in_techs << "\"";
        }
        std::string error_str = "ERROR: The following categories were never defined in techs.txt, but some "
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
    for (const Tech* tech : m_techs) {
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
                  << UserString(tech->Category()) << " "
                  << UserString(boost::lexical_cast<std::string>(tech->Type())) << ") - "
                  << tech->Graphic() << std::endl;
    }
#endif
}

TechManager::~TechManager() {
    for (std::map<std::string, TechCategory*>::value_type& entry : m_categories)
        delete entry.second;
    for (const Tech* tech : m_techs)
        delete tech;
}

std::string TechManager::FindIllegalDependencies() {
    assert(!m_techs.empty());
    std::string retval;
    for (const Tech* tech : m_techs) {
        if (!tech) {
            std::stringstream stream;
            stream << "ERROR: Missing tech referenced in techs.txt for unknown reasons...";
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

std::string TechManager::FindFirstDependencyCycle() {
    assert(!m_techs.empty());
    static const std::set<std::string> EMPTY_STRING_SET;    // used in case an invalid tech is processed

    std::set<const Tech*> checked_techs; // the list of techs that are not part of any cycle
    for (const Tech* tech : *this) {
        if (checked_techs.find(tech) != checked_techs.end())
            continue;

        std::vector<const Tech*> stack;
        stack.push_back(tech);
        while (!stack.empty()) {
            // Examine the tech on top of the stack.  If the tech has no prerequisite techs, or if all
            // of its prerequisite techs have already been checked, pop it off the stack and mark it as
            // checked; otherwise, push all its unchecked prerequisites onto the stack.
            const Tech* current_tech = stack.back();
            unsigned int starting_stack_size = stack.size();

            const std::set<std::string>& prereqs = (current_tech ? current_tech->Prerequisites() : EMPTY_STRING_SET);
            for (const std::string& prereq_name : prereqs) {
                const Tech* prereq_tech = GetTech(prereq_name);
                if (!prereq_tech || checked_techs.find(prereq_tech) != checked_techs.end())
                    continue;

                // since this is not a checked prereq, see if it is already in the stack somewhere; if so, we have a cycle
                std::vector<const Tech*>::reverse_iterator stack_duplicate_it =
                    std::find(stack.rbegin(), stack.rend(), prereq_tech);
                if (stack_duplicate_it != stack.rend()) {
                    std::stringstream stream;
                    std::string current_tech_name = prereq_tech->Name();
                    stream << "ERROR: Tech dependency cycle found in techs.txt (A <-- B means A is a prerequisite of B): \""
                            << current_tech_name << "\"";
                    for (std::vector<const Tech*>::reverse_iterator stack_it = stack.rbegin();
                            stack_it != stack_duplicate_it;
                            ++stack_it) {
                        if ((*stack_it)->Prerequisites().find(current_tech_name) != (*stack_it)->Prerequisites().end()) {
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

std::string TechManager::FindRedundantDependency() {
    assert(!m_techs.empty());

    for (const Tech* tech : m_techs) {
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
            std::map<std::string, std::string>::const_iterator map_it = techs_unlocked_by_prereqs.find(prereq_name);
            if (map_it != techs_unlocked_by_prereqs.end()) {
                std::stringstream stream;
                stream << "ERROR: Redundant dependency found in techs.txt (A <-- B means A is a prerequisite of B): "
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

void TechManager::AllChildren(const Tech* tech, std::map<std::string, std::string>& children) {
    for (const std::string& unlocked_tech : tech->UnlockedTechs()) {
        if (unlocked_tech == tech->Name()) {
            // infinite loop
            FatalLogger() << "Tech " << unlocked_tech << " unlocks itself";
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
        if (prereqs_set.find(cur_name) != prereqs_set.end()) continue;

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
    for (const std::multimap<float, std::string>::value_type& tech_to_add : techs_to_add_map)
    { retval.push_back(tech_to_add.second); }

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
