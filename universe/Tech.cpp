#include "Tech.h"

#include "Effect.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <sstream>


namespace {
    void NextTechs(std::vector<const Tech*>& retval, const std::set<std::string>& known_techs, std::set<const Tech*>& checked_techs,
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
                const std::set<std::string>& prereqs = current_tech->Prerequisites();
                bool all_prereqs_known = true;
                for (std::set<std::string>::const_iterator prereq_it = prereqs.begin(); prereq_it != prereqs.end(); ++prereq_it) {
                    const Tech* prereq_tech = GetTech(*prereq_it);
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

    const Tech* Cheapest(const std::vector<const Tech*>& next_techs)
    {
        if (next_techs.empty())
            return 0;

        double min_price = next_techs[0]->ResearchCost() * next_techs[0]->ResearchTurns();
        int min_index = 0;
        for (unsigned int i = 0; i < next_techs.size(); ++i) {
            double price = next_techs[i]->ResearchCost() * next_techs[i]->ResearchTurns();
            if (price < min_price) {
                min_price = price;
                min_index = i;
            }
        }

        return next_techs[min_index];
    }

    bool temp_header_bool = RecordHeaderFile(TechRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


///////////////////////////////////////////////////////////
// Tech                                                  //
///////////////////////////////////////////////////////////
Tech::Tech(const GG::XMLElement& elem) :
    m_effects(0)
{
    if (elem.Tag() != "Tech")
        throw std::invalid_argument(("Attempted to construct a Tech from an XMLElement that had a tag other than \"Tech\"" + (" (\"" + elem.Tag() + ")")).c_str());

    using boost::lexical_cast;

    m_name = elem.Child("name").Text();
    m_description = elem.Child("description").Text();
    m_type = lexical_cast<TechType>(elem.Child("type").Text());
    m_category = elem.Child("category").Text();
    m_research_cost = lexical_cast<double>(elem.Child("research_cost").Text());
    m_research_turns = lexical_cast<int>(elem.Child("research_turns").Text());

    if (elem.ContainsChild("EffectsGroup"))
        m_effects = new Effect::EffectsGroup(elem.Child("EffectsGroup"));

    for (GG::XMLElement::const_child_iterator it = elem.Child("prerequisites").child_begin();
         it != elem.Child("prerequisites").child_end();
         ++it) {
        m_prerequisites.insert(it->Text());
    }

    for (GG::XMLElement::const_child_iterator it = elem.Child("unlocked_items").child_begin();
         it != elem.Child("unlocked_items").child_end();
         ++it) {
        m_unlocked_items.push_back(ItemSpec(*it));
    }
}

const std::string& Tech::Name() const
{
    return m_name;
}

const std::string& Tech::Description() const
{
    return m_description;
}

TechType Tech::Type() const
{
    return m_type;
}

const std::string& Tech::Category() const
{
    return m_category;
}

double Tech::ResearchCost() const
{
    return m_research_cost;
}

int Tech::ResearchTurns() const
{
    return m_research_turns;
}

const Effect::EffectsGroup* Tech::Effects() const
{
    return m_effects;
}

const std::set<std::string>& Tech::Prerequisites() const
{
    return m_prerequisites;
}

const std::vector<Tech::ItemSpec>& Tech::UnlockedItems() const
{
    return m_unlocked_items;
}


///////////////////////////////////////////////////////////
// Tech::ItemSpec                                        //
///////////////////////////////////////////////////////////
Tech::ItemSpec::ItemSpec() :
    type(INVALID_UNLOCKABLE_ITEM_TYPE),
    name("")
{
}

Tech::ItemSpec::ItemSpec(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Item")
        throw std::invalid_argument(("Attempted to construct a Item from an XMLElement that had a tag other than \"Item\"" + (" (\"" + elem.Tag() + ")")).c_str());

    type = boost::lexical_cast<UnlockableItemType>(elem.Child("type").Text());
    name = elem.Child("name").Text();
}


///////////////////////////////////////////////////////////
// TechManager                                           //
///////////////////////////////////////////////////////////
// static(s)
TechManager* TechManager::s_instance = 0;

const Tech* TechManager::GetTech(const std::string& name)
{
    iterator it = m_techs.get<NameIndex>().find(name);
    return it == m_techs.get<NameIndex>().end() ? 0 : *it;
}

const std::vector<std::string>& TechManager::CategoryNames() const
{
    return m_categories;
}

std::vector<const Tech*> TechManager::AllNextTechs(const std::set<std::string>& known_techs)
{
    std::vector<const Tech*> retval;
    std::set<const Tech*> checked_techs;
    iterator end_it = m_techs.get<NameIndex>().end();
    for (iterator it = m_techs.get<NameIndex>().begin(); it != end_it; ++it) {
        NextTechs(retval, known_techs, checked_techs, it, end_it);
    }
    return retval;
}

const Tech* TechManager::CheapestNextTech(const std::set<std::string>& known_techs)
{
    return Cheapest(AllNextTechs(known_techs));
}

std::vector<const Tech*> TechManager::NextTechsTowards(const std::set<std::string>& known_techs,
                                                       const std::string& desired_tech)
{
    std::vector<const Tech*> retval;
    std::set<const Tech*> checked_techs;
    NextTechs(retval, known_techs, checked_techs, m_techs.get<NameIndex>().find(desired_tech), m_techs.get<NameIndex>().end());
    return retval;
}

const Tech* TechManager::CheapestNextTechTowards(const std::set<std::string>& known_techs,
                                                 const std::string& desired_tech)
{
    return Cheapest(NextTechsTowards(known_techs, desired_tech));
}

TechManager::iterator TechManager::begin() const
{
    return m_techs.get<NameIndex>().begin();
}

TechManager::iterator TechManager::end() const
{
    return m_techs.get<NameIndex>().end();
}

TechManager::category_iterator TechManager::category_begin(const std::string& name) const
{
    return m_techs.get<ContainerIndex>().lower_bound(name);
}

TechManager::category_iterator TechManager::category_end(const std::string& name) const
{
    return m_techs.get<ContainerIndex>().upper_bound(name);
}

TechManager::TechManager()
{
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one TechManager.");

    s_instance = this;

    std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
    if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
        settings_dir += '/';
    std::ifstream ifs((settings_dir + "techs.xml").c_str());
    GG::XMLDoc doc;
    doc.ReadDoc(ifs);
    std::set<std::string> categories_seen_in_techs;
    for (GG::XMLElement::const_child_iterator it = doc.root_node.child_begin(); it != doc.root_node.child_end(); ++it) {
        if (it->Tag() != "Tech" && it->Tag() != "Category")
            throw std::runtime_error("ERROR: Encountered non-Tech, non-Category in techs.xml!");
        if (it->Tag() == "Tech") {
            Tech* tech = new Tech(*it);
            categories_seen_in_techs.insert(tech->Category());
            if (m_techs.get<NameIndex>().find(tech->Name()) != m_techs.get<NameIndex>().end())
                throw std::runtime_error(("ERROR: More than one tech in techs.xml has the name " + tech->Name()).c_str());
            if (tech->Prerequisites().find(tech->Name()) != tech->Prerequisites().end())
                throw std::runtime_error(("ERROR: Tech " + tech->Name() + " depends on itself!").c_str());
            m_techs.insert(tech);
        } else {
            m_categories.push_back(it->Text());
        }
    }
    ifs.close();

    std::set<std::string> empty_defined_categories;
    for (unsigned int i = 0; i < m_categories.size(); ++i) {
        std::set<std::string>::iterator it = categories_seen_in_techs.find(m_categories[i]);
        if (it == categories_seen_in_techs.end()) {
            empty_defined_categories.insert(m_categories[i]);
        } else {
            categories_seen_in_techs.erase(it);
        }
    }

    if (!empty_defined_categories.empty()) {
        std::stringstream stream;
        for (std::set<std::string>::iterator it = empty_defined_categories.begin(); it != empty_defined_categories.end(); ++it) {
            stream << " \"" << *it << "\"";
        }
        throw std::runtime_error(("ERROR: The following categories were defined in techs.xml, but no "
                                  "techs were defined that fell within them:" + stream.str()).c_str());
    }

    if (!categories_seen_in_techs.empty()) {
        std::stringstream stream;
        for (std::set<std::string>::iterator it = categories_seen_in_techs.begin(); it != categories_seen_in_techs.end(); ++it) {
            stream << " \"" << *it << "\"";
        }
        throw std::runtime_error(("ERROR: The following categories were never defined in techs.xml, but some "
                                  "techs were defined that fell within them:" + stream.str()).c_str());
    }

    std::string illegal_dependency_str = FindIllegalDependencies();
    if (!illegal_dependency_str.empty()) {
        throw std::runtime_error(illegal_dependency_str.c_str());
    }

    std::string cycle_str = FindFirstDependencyCycle();
    if (!cycle_str.empty()) {
        throw std::runtime_error(cycle_str.c_str());
    }
}

std::string TechManager::FindIllegalDependencies()
{
    assert(!m_techs.empty());

    std::string retval;
    for (iterator it = begin(); it != end(); ++it) {
        const Tech* tech = *it;
        TechType tech_type = tech->Type();
        const std::set<std::string>& prereqs = tech->Prerequisites();
        for (std::set<std::string>::const_iterator it = prereqs.begin(); it != prereqs.end(); ++it) {
            const Tech* prereq_tech = GetTech(*it);
            TechType prereq_type = prereq_tech->Type();
            if (tech_type == TT_THEORY && prereq_type != TT_THEORY)
                retval += "ERROR: Theory tech \"" + tech->Name() + "\" requires non-Theory tech \"" + prereq_tech->Name() + "\"; Theory techs can only require other Theory techs.\n";
            if (prereq_type == TT_REFINEMENT && tech_type != TT_REFINEMENT)
                retval += "ERROR: Non-Refinement Tech \"" + tech->Name() + "\" requires Refinement tech \"" + prereq_tech->Name() + "\"; Refinement techs cannot be requirements for anything but other Refinement techs.\n";
        }
    }
    return retval;
}

std::string TechManager::FindFirstDependencyCycle()
{
    assert(!m_techs.empty());

    std::set<const Tech*> checked_techs; // the list of techs that are not part of any cycle
    for (iterator it = begin(); it != end(); ++it) {
        if (checked_techs.find(*it) != checked_techs.end())
            continue;

        std::vector<const Tech*> stack;
        stack.push_back(*it);
        while (!stack.empty()) {
            // Examine the tech on top of the stack.  If the tech has no prerequisite techs, or if all
            // of its prerequisite techs have already been checked, pop it off the stack and mark it as
            // checked; otherwise, push all its unchecked prerequisites onto the stack.
            const Tech* current_tech = stack.back();
            unsigned int starting_stack_size = stack.size();
            const std::set<std::string>& prereqs = current_tech->Prerequisites();
            for (std::set<std::string>::const_iterator prereq_it = prereqs.begin(); prereq_it != prereqs.end(); ++prereq_it) {
                const Tech* prereq_tech = GetTech(*prereq_it);
                if (checked_techs.find(prereq_tech) == checked_techs.end()) {
                    // since this is not a checked prereq, see if it is already in the stack somewhere; if so, we have a cycle
                    std::vector<const Tech*>::reverse_iterator stack_duplicate_it =
                        std::find(stack.rbegin(), stack.rend(), prereq_tech);
                    if (stack_duplicate_it != stack.rend()) {
                        std::stringstream stream;
                        std::string current_tech_name = prereq_tech->Name();
                        stream << "ERROR: Tech dependency cycle found in techs.xml (A <-- B means A is a prerequisite of B): \""
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
            }
            if (starting_stack_size == stack.size()) {
                stack.pop_back();
                checked_techs.insert(current_tech);
            }
        }
    }
    return "";
}

TechManager& TechManager::GetTechManager()
{
    static TechManager manager;
    return manager;
}


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
TechManager& GetTechManager()
{
    return TechManager::GetTechManager();
}

const Tech* GetTech(const std::string& name)
{
    return GetTechManager().GetTech(name);
}
