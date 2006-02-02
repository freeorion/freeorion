// -*- C++ -*-
#ifndef _Tech_h_
#define _Tech_h_

#include <boost/serialization/shared_ptr.hpp>

#include "Enums.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <set>
#include <string>
#include <vector>

namespace Effect {
    class EffectsGroup;
}
class TechManager;
class XMLElement;


/** encasulates the data for a single FreeOrion technology */
class Tech
{
public:
    struct ItemSpec;

    /** \name Structors */ //@{
    Tech(const XMLElement& elem); ///< XML ctor
    //@}

    /** \name Accessors */ //@{
    /** returns the name of this tech */
    const std::string& Name() const;

    /** Returns the text description of this tech */
    const std::string& Description() const;

    /** Returns the type (theory/application/refinement) of this tech */
    TechType Type() const;

    /** retursn the name of the category to which this tech belongs */
    const std::string& Category() const;

    /** returns the maximum number of RPs per turn allowed to be spent on researching this tech */
    double ResearchCost() const;

    /** returns the number of turns required to research this tech, if ResearchCost() RPs are spent per turn */
    int ResearchTurns() const;

    /** returns the effects that are applied to the discovering empire's capitol when this tech is researched;
        not all techs have effects, in which case this returns 0 */
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Effects() const;

    /** returns the set of names of all techs required before this one can be researched */
    const std::set<std::string>& Prerequisites() const;
    
    /** returns the name of the grapic file for this tech */
    const std::string& Graphic() const;

    /** returns the set all items that are unlocked by researching this tech */
    const std::vector<ItemSpec>& UnlockedItems() const;

    /** returns the set of names of all techs for which this one is a prerequisite */
    const std::set<std::string>& UnlockedTechs() const;
    //@}

private:
    Tech(const Tech&);                  // disabled
    const Tech& operator=(const Tech&); // disabled

    std::string                m_name;
    std::string                m_description;
    std::string                m_category;
    TechType                   m_type;
    double                     m_research_cost;
    int                        m_research_turns;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                               m_effects;
    std::set<std::string>      m_prerequisites;
    std::vector<ItemSpec>      m_unlocked_items;
    std::string                m_graphic;

    // note that m_unlocked_techs is not part of the XML representation of a tech; it is filled in by the TechManager at load-time
    std::set<std::string>      m_unlocked_techs;

    friend class TechManager;
};


/** specifies a single item that may be unlocked by researching a tech.  The \a type field stores the type of item that
    is being unlocked, such as a building or ship component, and the \a name field contains the name of the actual item
    (e.g. (UIT_BUILDING, "Superfarm") or (UIT_SHIP_COMPONENT, "Death Ray")). */
struct Tech::ItemSpec
{
    ItemSpec(); ///< default ctor
    ItemSpec(const XMLElement& elem); ///< XML ctor

    UnlockableItemType type; ///< the kind of item this is
    std::string        name; ///< the exact item this is
};


/** holds all FreeOrion techs.  Techs may be looked up by name and by category, and the next researchable techs can be querried,
    given a set of currently-known techs. */
class TechManager
{
private:
    struct CategoryIndex {};
    struct NameIndex {};
    typedef boost::multi_index_container<
        const Tech*,
        boost::multi_index::indexed_by<
            boost::multi_index::ordered_non_unique<
                boost::multi_index::tag<CategoryIndex>,
                boost::multi_index::const_mem_fun<
                    Tech,
                    const std::string&,
                    &Tech::Category
                >
            >,
            boost::multi_index::ordered_unique<
                boost::multi_index::tag<NameIndex>,
                boost::multi_index::const_mem_fun<
                    Tech,
                    const std::string&,
                    &Tech::Name
                >
            >
        >
    > TechContainer;

public:
    /** iterator that runs over techs within a category */
    typedef TechContainer::index<CategoryIndex>::type::const_iterator category_iterator;

    /** iterator that runs over all techs */
    typedef TechContainer::index<NameIndex>::type::const_iterator     iterator;

    /** \name Accessors */ //@{
    /** returns the tech with the name \a name; you should use the free function GetTech() instead */
    const Tech*                     GetTech(const std::string& name);

    /** returns the list of category names */
    const std::vector<std::string>& CategoryNames() const;

    /** returns all researchable techs */
    std::vector<const Tech*>        AllNextTechs(const std::set<std::string>& known_techs);

    /** returns the cheapest researchable tech */
    const Tech*                     CheapestNextTech(const std::set<std::string>& known_techs);

    /** returns all researchable techs that progress from the given known techs to the given desired tech */
    std::vector<const Tech*>        NextTechsTowards(const std::set<std::string>& known_techs,
                                                     const std::string& desired_tech);

    /** returns the cheapest researchable tech that progresses from the given known techs to the given desired tech */
    const Tech*                     CheapestNextTechTowards(const std::set<std::string>& known_techs,
                                                            const std::string& desired_tech);

    /** iterator to the first tech */
    iterator                        begin() const;

    /** iterator to the last + 1th tech */
    iterator                        end() const;

    /** iterator to the first tech in category \a name */
    category_iterator               category_begin(const std::string& name) const;

    /** iterator to the last + 1th tech in category \a name */
    category_iterator               category_end(const std::string& name) const;
    //@}

    /** returns the instance of this singleton class; you should use the free function GetTechManager() instead */
    static TechManager& GetTechManager();

private:
    TechManager();

    /** returns an error string indicating the first instance of an illegal prerequisite relationship between
        two techs in m_techs, or an empty string if there are no illegal dependencies  */
    std::string FindIllegalDependencies();

    /** returns an error string indicating the first prerequisite dependency cycle found in m_techs, or an
        empty string if there are no dependency cycles */
    std::string FindFirstDependencyCycle();

    /** returns an error string indicating the first instance of a redundant dependency, or an empty string if there
        are no redundant dependencies.  An example of a redundant dependency is A --> C, if A --> B and B --> C. */
    std::string FindRedundantDependency();

    void AllChildren(const Tech* tech, std::map<std::string, std::string>& children);

    std::vector<std::string> m_categories;
    TechContainer            m_techs;

    static TechManager* s_instance;
};

/** returns the singleton tech manager */
TechManager& GetTechManager();

/** returns a pointer to the tech with the name \a name, or 0 if no such tech exists */
const Tech* GetTech(const std::string& name);

inline std::string TechRevision()
{return "$Id$";}

#endif // _Tech_h_
