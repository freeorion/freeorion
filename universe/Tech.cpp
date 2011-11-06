#include "Tech.h"

#include "Effect.h"
#include "../parse/Parse.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>

std::string DumpIndent();

extern int g_indent;

namespace {
    const bool CHEAP_AND_FAST_TECH_RESEARCH = false;    // makes all techs cost 1 RP and take 1 turn to research
}

struct store_tech_impl
{
    template <class T1, class T2, class T3>
    struct result {typedef void type;};
    template <class T>
    void operator()(TechManager::TechContainer& techs, std::set<std::string>& categories_seen, const T& tech) const
    {
        categories_seen.insert(tech->Category());
        if (techs.get<TechManager::NameIndex>().find(tech->Name()) != techs.get<TechManager::NameIndex>().end()) {
            std::string error_str = "ERROR: More than one tech in techs.txt has the name " + tech->Name();
            throw std::runtime_error(error_str.c_str());
        }
        if (tech->Prerequisites().find(tech->Name()) != tech->Prerequisites().end()) {
            std::string error_str = "ERROR: Tech " + tech->Name() + " depends on itself!";
            throw std::runtime_error(error_str.c_str());
        }
        techs.insert(tech);
    }
};

struct store_category_impl
{
    template <class T1, class T2>
    struct result {typedef void type;};
    template <class T>
    void operator()(std::map<std::string, TechCategory*>& categories, const T& category) const
    {
        if (categories.find(category->name) != categories.end()) {
            std::string error_str = "ERROR: More than one tech category in techs.txt name " + category->name;
            throw std::runtime_error(error_str.c_str());
        }
        categories[category->name] = category;
    }
};

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

        double min_price = next_techs[0]->ResearchCost() * next_techs[0]->ResearchTime();
        int min_index = 0;
        for (unsigned int i = 0; i < next_techs.size(); ++i) {
            double price = next_techs[i]->ResearchCost() * next_techs[i]->ResearchTime();
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
Tech::Tech(const std::string& name, const std::string& description, const std::string& short_description,
           const std::string& category, TechType type, double research_cost, int research_turns,
           bool researchable, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
           const std::set<std::string>& prerequisites, const std::vector<ItemSpec>& unlocked_items,
           const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_short_description(short_description),
    m_category(category),
    m_type(type),
    m_research_cost(research_cost),
    m_research_turns(research_turns),
    m_researchable(researchable),
    m_effects(effects),
    m_prerequisites(prerequisites),
    m_unlocked_items(unlocked_items),
    m_graphic(graphic)
{}

Tech::Tech(const TechInfo& tech_info,
           const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
           const std::set<std::string>& prerequisites, const std::vector<ItemSpec>& unlocked_items,
           const std::string& graphic) :
    m_name(tech_info.name),
    m_description(tech_info.description),
    m_short_description(tech_info.short_description),
    m_category(tech_info.category),
    m_type(tech_info.type),
    m_research_cost(tech_info.research_cost),
    m_research_turns(tech_info.research_turns),
    m_researchable(tech_info.researchable),
    m_effects(effects),
    m_prerequisites(prerequisites),
    m_unlocked_items(unlocked_items),
    m_graphic(graphic)
{}

const std::string& Tech::Name() const
{ return m_name; }

const std::string& Tech::Description() const
{ return m_description; }

const std::string& Tech::ShortDescription() const
{ return m_short_description; }

std::string Tech::Dump() const
{
    using boost::lexical_cast;

    std::string retval = DumpIndent() + "Tech\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    retval += DumpIndent() + "shortdescription = \"" + m_short_description + "\"\n";
    retval += DumpIndent() + "techtype = ";
    switch (m_type) {
    case TT_THEORY:      retval += "Theory"; break;
    case TT_APPLICATION: retval += "Application"; break;
    case TT_REFINEMENT:  retval += "Refinement"; break;
    default: retval += "?"; break;
    }
    retval += "\n";
    retval += DumpIndent() + "category = \"" + m_category + "\"\n";
    retval += DumpIndent() + "researchcost = " + lexical_cast<std::string>(ResearchCost()) + "\n";
    retval += DumpIndent() + "researchturns = " + lexical_cast<std::string>(ResearchTime()) + "\n";
    retval += DumpIndent() + "prerequisites = ";
    if (m_prerequisites.empty()) {
        retval += "[]\n";
    } else if (m_prerequisites.size() == 1) {
        retval += "\"" + *m_prerequisites.begin() + "\"\n";
    } else {
        retval += "[\n";
        ++g_indent;
        for (std::set<std::string>::const_iterator it = m_prerequisites.begin(); it != m_prerequisites.end(); ++it) {
            retval += DumpIndent() + "\"" + *it + "\"\n";
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
        for (unsigned int i = 0; i < m_unlocked_items.size(); ++i) {
            retval += DumpIndent() + m_unlocked_items[i].Dump();
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
            for (unsigned int i = 0; i < m_effects.size(); ++i) {
                retval += m_effects[i]->Dump();
            }
            --g_indent;
            retval += DumpIndent() + "]\n";
        }
    }
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

TechType Tech::Type() const
{ return m_type; }

const std::string& Tech::Category() const
{ return m_category; }

double Tech::ResearchCost() const
{
    if (!CHEAP_AND_FAST_TECH_RESEARCH)
        return m_research_cost;
    else
        return 1.0;
}

double Tech::PerTurnCost() const
{ return ResearchCost() / std::max(1, ResearchTime()); }

int Tech::ResearchTime() const
{
    if (!CHEAP_AND_FAST_TECH_RESEARCH)
        return m_research_turns;
    else
        return 1;
}

bool Tech::Researchable() const
{ return m_researchable; }

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Tech::Effects() const
{ return m_effects; }

const std::set<std::string>& Tech::Prerequisites() const
{ return m_prerequisites; }

const std::string& Tech::Graphic() const
{ return m_graphic; }

const std::vector<ItemSpec>& Tech::UnlockedItems() const
{ return m_unlocked_items; }

const std::set<std::string>& Tech::UnlockedTechs() const
{ return m_unlocked_techs; }


///////////////////////////////////////////////////////////
// ItemSpec                                        //
///////////////////////////////////////////////////////////
ItemSpec::ItemSpec() :
    type(INVALID_UNLOCKABLE_ITEM_TYPE),
    name("")
{}

ItemSpec::ItemSpec(UnlockableItemType type_, const std::string& name_) :
    type(type_),
    name(name_)
{}

std::string ItemSpec::Dump() const
{
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

///////////////////////////////////////////////////////////
// TechCategory                               //
///////////////////////////////////////////////////////////
TechCategory::TechCategory() :
    name(""),
    graphic(""),
    colour(GG::Clr(255, 255, 255, 255))
{}

TechCategory::TechCategory(const std::string& name_, const std::string& graphic_, const GG::Clr& colour_) :
    name(name_),
    graphic(graphic_),
    colour(colour_)
{}


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

const TechCategory* TechManager::GetTechCategory(const std::string& name)
{
    std::map<std::string, TechCategory*>::const_iterator it = m_categories.find(name);
    return it == m_categories.end() ? 0 : it->second;
}

std::vector<std::string> TechManager::CategoryNames() const
{
    std::vector<std::string> retval;
    for (std::map<std::string, TechCategory*>::const_iterator it = m_categories.begin(); it != m_categories.end(); ++it)
        retval.push_back(it->first);
    return retval;
}

std::vector<std::string> TechManager::TechNames() const {
    std::vector<std::string> retval;
    for (TechManager::iterator it = begin(); it != end(); ++it)
        retval.push_back((*it)->Name());
    return retval;
}

std::vector<std::string> TechManager::TechNames(const std::string& name) const {
    std::vector<std::string> retval;
    for (TechManager::category_iterator it = category_begin(name); it != category_end(name); ++it) {
        retval.push_back((*it)->Name());
    }
    return retval;
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
{ return Cheapest(AllNextTechs(known_techs)); }

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
{ return Cheapest(NextTechsTowards(known_techs, desired_tech)); }

TechManager::iterator TechManager::begin() const
{ return m_techs.get<NameIndex>().begin(); }

TechManager::iterator TechManager::end() const
{ return m_techs.get<NameIndex>().end(); }

TechManager::category_iterator TechManager::category_begin(const std::string& name) const
{ return m_techs.get<CategoryIndex>().lower_bound(name); }

TechManager::category_iterator TechManager::category_end(const std::string& name) const
{ return m_techs.get<CategoryIndex>().upper_bound(name); }

TechManager::TechManager()
{
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one TechManager.");

    s_instance = this;

    std::set<std::string> categories_seen_in_techs;

    parse::techs(GetResourceDir() / "techs.txt", m_techs, m_categories, categories_seen_in_techs);

    std::set<std::string> empty_defined_categories;
    for (std::map<std::string, TechCategory*>::iterator map_it = m_categories.begin(); map_it != m_categories.end(); ++map_it) {
        std::set<std::string>::iterator set_it = categories_seen_in_techs.find(map_it->first);
        if (set_it == categories_seen_in_techs.end()) {
            empty_defined_categories.insert(map_it->first);
        } else {
            categories_seen_in_techs.erase(set_it);
        }
    }

    if (!empty_defined_categories.empty()) {
        std::stringstream stream;
        for (std::set<std::string>::iterator it = empty_defined_categories.begin(); it != empty_defined_categories.end(); ++it) {
            stream << " \"" << *it << "\"";
        }
        std::string error_str = "ERROR: The following categories were defined in techs.txt, but no "
            "techs were defined that fell within them:" + stream.str();
        Logger().errorStream() << error_str;
        std::cerr << error_str << std::endl;
    }

    if (!categories_seen_in_techs.empty()) {
        std::stringstream stream;
        for (std::set<std::string>::iterator it = categories_seen_in_techs.begin(); it != categories_seen_in_techs.end(); ++it) {
            stream << " \"" << *it << "\"";
        }
        std::string error_str = "ERROR: The following categories were never defined in techs.txt, but some "
            "techs were defined that fell within them:" + stream.str();
        Logger().errorStream() << error_str;
        std::cerr << error_str << std::endl;
    }

    std::string illegal_dependency_str = FindIllegalDependencies();
    if (!illegal_dependency_str.empty()) {
        Logger().errorStream() << illegal_dependency_str;
        throw std::runtime_error(illegal_dependency_str.c_str());
    }

    std::string cycle_str = FindFirstDependencyCycle();
    if (!cycle_str.empty()) {
        Logger().errorStream() << cycle_str;
        throw std::runtime_error(cycle_str.c_str());
    }

    // fill in the unlocked techs data for each loaded tech
    for (iterator it = begin(); it != end(); ++it) {
        const std::set<std::string>& prereqs = (*it)->Prerequisites();
        for (std::set<std::string>::const_iterator prereq_it = prereqs.begin(); prereq_it != prereqs.end(); ++prereq_it) {
            if (Tech* tech = const_cast<Tech*>(GetTech(*prereq_it)))
                tech->m_unlocked_techs.insert((*it)->Name());
        }
    }

    std::string redundant_dependency = FindRedundantDependency();
    if (!redundant_dependency.empty())
        Logger().errorStream() << redundant_dependency;

#ifdef OUTPUT_TECH_LIST
    for (iterator it = begin(); it != end(); ++it) {
        const Tech* tech = *it;
        std::cerr << UserString(tech->Name()) << " (" 
                  << UserString(tech->Category()) << " "
                  << UserString(boost::lexical_cast<std::string>(tech->Type())) << ") - "
                  << tech->Graphic() << std::endl;
    }
#endif
}

TechManager::~TechManager()
{
    for (std::map<std::string, TechCategory*>::iterator it = m_categories.begin(); it != m_categories.end(); ++it)
        delete it->second;
    for (TechContainer::iterator it = m_techs.begin(); it != m_techs.end(); ++it)
        delete *it;
}

std::string TechManager::FindIllegalDependencies()
{
    assert(!m_techs.empty());
    std::string retval;
    for (iterator it = begin(); it != end(); ++it) {
        const Tech* tech = *it;
        if (!tech) {
            std::stringstream stream;
            stream << "ERROR: Missing tech referenced in techs.txt for unknown reasons...";
            return stream.str();
        }
        const std::set<std::string>& prereqs = tech->Prerequisites();
        for (std::set<std::string>::const_iterator prereq_it = prereqs.begin(); prereq_it != prereqs.end(); ++prereq_it) {
            const Tech* prereq_tech = GetTech(*prereq_it);
            if (!prereq_tech) {
                std::stringstream stream;
                stream << "ERROR: Tech \"" << tech->Name() << "\" requires a missing or malformed tech \"" << *prereq_it << "\" as its prerequisite.";
                return stream.str();
            }
            //TechType prereq_type = prereq_tech->Type();
            //if (tech_type == TT_THEORY && prereq_type != TT_THEORY)
            //    retval += "ERROR: Theory tech \"" + tech->Name() + "\" requires non-Theory tech \"" + prereq_tech->Name() + "\"; Theory techs can only require other Theory techs.\n";
            //if (prereq_type == TT_REFINEMENT && tech_type != TT_REFINEMENT)
            //    retval += "ERROR: Non-Refinement Tech \"" + tech->Name() + "\" requires Refinement tech \"" + prereq_tech->Name() + "\"; Refinement techs cannot be requirements for anything but other Refinement techs.\n";
        }
    }
    return retval;
}

std::string TechManager::FindFirstDependencyCycle()
{
    assert(!m_techs.empty());
    static const std::set<std::string> EMPTY_STRING_SET;    // used in case an invalid tech is processed

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

            const std::set<std::string>& prereqs = (current_tech ? current_tech->Prerequisites() : EMPTY_STRING_SET);
            for (std::set<std::string>::const_iterator prereq_it = prereqs.begin();
                 prereq_it != prereqs.end(); ++prereq_it)
            {
                const Tech* prereq_tech = GetTech(*prereq_it);
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

std::string TechManager::FindRedundantDependency()
{
    assert(!m_techs.empty());

    for (iterator it = begin(); it != end(); ++it) {
        const Tech* tech = *it;
        if (!tech) {
            std::stringstream stream;
            stream << "ERROR: Missing tech referenced in techs.txt for unknown reasons...";
            return stream.str();
        }
        std::set<std::string> prereqs = tech->Prerequisites();
        std::map<std::string, std::string> techs_unlocked_by_prereqs;
        for (std::set<std::string>::const_iterator prereq_it = prereqs.begin(); prereq_it != prereqs.end(); ++prereq_it) {
            const Tech* prereq_tech = GetTech(*prereq_it);
            if (!prereq_tech) {
                std::stringstream stream;
                stream << "ERROR: Tech \"" << tech->Name() << "\" requires a missing or malformed tech \"" << *prereq_it << "\" as its prerequisite.";
                return stream.str();
            }
            AllChildren(prereq_tech, techs_unlocked_by_prereqs);
        }
        for (std::set<std::string>::const_iterator prereq_it = prereqs.begin(); prereq_it != prereqs.end(); ++prereq_it) {
            std::map<std::string, std::string>::const_iterator map_it = techs_unlocked_by_prereqs.find(*prereq_it);
            if (map_it != techs_unlocked_by_prereqs.end()) {
                std::stringstream stream;
                stream << "ERROR: Redundant dependency found in techs.txt (A <-- B means A is a prerequisite of B): "
                       << map_it->second << " <-- " << map_it->first << ", "
                       << map_it->first << " <-- " << (*it)->Name() << ", "
                       << map_it->second << " <-- " << (*it)->Name() << "; remove the " << map_it->second << " <-- " << (*it)->Name()
                       << " dependency.";
                return stream.str();
            }
        }
    }
    return "";
}

void TechManager::AllChildren(const Tech* tech, std::map<std::string, std::string>& children)
{
    const std::set<std::string>& unlocked_techs = tech->UnlockedTechs();
    for (std::set<std::string>::const_iterator it = unlocked_techs.begin(); it != unlocked_techs.end(); ++it) {
        children[*it] = tech->Name();
        AllChildren(GetTech(*it), children);
    }
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
{ return TechManager::GetTechManager(); }

const Tech* GetTech(const std::string& name)
{ return GetTechManager().GetTech(name); }

const TechCategory* GetTechCategory(const std::string& name)
{ return GetTechManager().GetTechCategory(name); }
