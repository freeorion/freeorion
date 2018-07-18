#ifndef _Tech_h_
#define _Tech_h_

#include "ValueRefFwd.h"

#include "EnumsFwd.h"
#include "../util/Export.h"
#include "../util/Pending.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/optional/optional.hpp>

#include <set>
#include <string>
#include <vector>
#include <map>

#include <GG/Clr.h>

namespace Effect
{ class EffectsGroup; }
class TechManager;
struct ItemSpec;

/** encasulates the data for a single FreeOrion technology */
class FO_COMMON_API Tech {
public:
    /** Helper struct for parsing tech definitions */
    struct TechInfo {
        TechInfo();
        TechInfo(const std::string& name_, const std::string& description_,
                 const std::string& short_description_, const std::string& category_,
                 std::unique_ptr<ValueRef::ValueRefBase<double>>&& research_cost_,
                 std::unique_ptr<ValueRef::ValueRefBase<int>>&& research_turns_,
                 bool researchable_,
                 const std::set<std::string>& tags_);
        ~TechInfo();

        std::string                     name;
        std::string                     description;
        std::string                     short_description;
        std::string                     category;
        std::unique_ptr<ValueRef::ValueRefBase<double>> research_cost;
        std::unique_ptr<ValueRef::ValueRefBase<int>>    research_turns;
        bool                            researchable;
        std::set<std::string>           tags;
    };

    /** \name Structors */ //@{
    Tech(const std::string& name, const std::string& description,
         const std::string& short_description, const std::string& category,
         std::unique_ptr<ValueRef::ValueRefBase<double>>&& research_cost,
         std::unique_ptr<ValueRef::ValueRefBase<int>>&& research_turns,
         bool researchable,
         const std::set<std::string>& tags,
         const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects,
         const std::set<std::string>& prerequisites,
         const std::vector<ItemSpec>& unlocked_items,
         const std::string& graphic);

    /** basic ctor taking helper struct to reduce number of direct parameters
      * in order to making parsing work. */
    Tech(TechInfo& tech_info,
         std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
         const std::set<std::string>& prerequisites,
         const std::vector<ItemSpec>& unlocked_items,
         const std::string& graphic);

    ~Tech();
    //@}

    /** \name Accessors */ //@{
    const std::string&  Name() const                { return m_name; }              //!< returns name of this tech
    const std::string&  Description() const         { return m_description; }       //!< Returns the text description of this tech
    const std::string&  ShortDescription() const    { return m_short_description; } //!< Returns the single-line short text description of this tech
    std::string         Dump(unsigned short ntabs = 0) const;                                               //!< Returns a text representation of this object
    const std::string&  Category() const            { return m_category; }          //!< retursn the name of the category to which this tech belongs
    float               ResearchCost(int empire_id) const;                          //!< returns the total research cost in RPs required to research this tech
    float               PerTurnCost(int empire_id) const;                           //!< returns the maximum number of RPs per turn allowed to be spent on researching this tech
    int                 ResearchTime(int empire_id) const;                          //!< returns the number of turns required to research this tech, if ResearchCost() RPs are spent per turn
    bool                Researchable() const        { return m_researchable; }      //!< returns whether this tech is researchable by players and appears on the tech tree

    const std::set<std::string>&    Tags() const    { return m_tags; }

    /** returns the effects that are applied to the discovering empire's capital
      * when this tech is researched; not all techs have effects, in which case
      * this returns 0 */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    const std::set<std::string>&    Prerequisites() const { return m_prerequisites; }   //!< returns the set of names of all techs required before this one can be researched
    const std::string&              Graphic() const       { return m_graphic; }         //!< returns the name of the grapic file for this tech
    const std::vector<ItemSpec>&    UnlockedItems() const { return m_unlocked_items; }  //!< returns the set all items that are unlocked by researching this tech
    const std::set<std::string>&    UnlockedTechs() const { return m_unlocked_techs; }  //!< returns the set of names of all techs for which this one is a prerequisite

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                    GetCheckSum() const;
    //@}

private:
    Tech(const Tech&);                  // disabled
    const Tech& operator=(const Tech&); // disabled
    void Init();

    std::string                     m_name;
    std::string                     m_description;
    std::string                     m_short_description;
    std::string                     m_category;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_research_cost;
    std::unique_ptr<ValueRef::ValueRefBase<int>>    m_research_turns;
    bool                            m_researchable;
    std::set<std::string>           m_tags;
    std::vector<std::shared_ptr<Effect::EffectsGroup>> m_effects;
    std::set<std::string>           m_prerequisites;
    std::vector<ItemSpec>           m_unlocked_items;
    std::string                     m_graphic;
    std::set<std::string>           m_unlocked_techs;

    friend class TechManager;
};


/** specifies a single item of game content that may be unlocked for an empire.  The \a type field
  * stores the type of item that is being unlocked, such as a building or ship component, and the
  * \a name field contains the name of the actual item (e.g. (UIT_BUILDING, "Superfarm") or
  * (UIT_SHIP_PART, "Death Ray")). */
struct FO_COMMON_API ItemSpec {
    ItemSpec();
    ItemSpec(UnlockableItemType type_, const std::string& name_) :
        type(type_),
        name(name_)
    {}
    std::string Dump(unsigned short ntabs = 0) const;   ///< returns a data file format representation of this object
    UnlockableItemType type;    ///< the kind of item this is
    std::string        name;    ///< the exact item this is
};

FO_COMMON_API bool operator==(const ItemSpec& lhs, const ItemSpec& rhs);
bool operator!=(const ItemSpec& lhs, const ItemSpec& rhs);

/** specifies a category of techs, with associated \a name, \a graphic (icon), and \a colour.*/
struct FO_COMMON_API TechCategory {
    TechCategory() :
        name(""),
        graphic(""),
        colour(GG::Clr(255, 255, 255, 255))
    {}
    TechCategory(const std::string& name_, const std::string& graphic_,
                 const GG::Clr& colour_):
        name(name_),
        graphic(graphic_),
        colour(colour_)
    {}
    std::string name;       ///< name of category
    std::string graphic;    ///< icon that represents catetegory
    GG::Clr     colour;     ///< colour associatied with category
};

namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const ItemSpec& item);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const TechCategory& cat);
}

/** holds all FreeOrion techs.  Techs may be looked up by name and by category, and the next researchable techs can be querried,
    given a set of currently-known techs. */
class FO_COMMON_API TechManager {
public:
    struct CategoryIndex {};
    struct NameIndex {};
    typedef boost::multi_index_container<
        std::unique_ptr<Tech>,
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

    using TechCategoryMap = std::map<std::string, std::unique_ptr<TechCategory>>;

    /** iterator that runs over techs within a category */
    typedef TechContainer::index<CategoryIndex>::type::const_iterator category_iterator;

    /** iterator that runs over all techs */
    typedef TechContainer::index<NameIndex>::type::const_iterator     iterator;

    /** \name Accessors */ //@{
    /** returns the tech with the name \a name; you should use the free function GetTech() instead */
    const Tech*                     GetTech(const std::string& name) const;

    /** returns the tech category with the name \a name; you should use the free function GetTechCategory() instead */
    const TechCategory*             GetTechCategory(const std::string& name) const;

    /** returns the list of category names */
    std::vector<std::string>        CategoryNames() const;

    /** returns list of all tech names */
    std::vector<std::string>        TechNames() const;

    /** returns list of names of techs in specified category */
    std::vector<std::string>        TechNames(const std::string& name) const;

    /** returns all researchable techs */
    std::vector<const Tech*>        AllNextTechs(const std::set<std::string>& known_techs);

    /** returns the cheapest researchable tech */
    const Tech*                     CheapestNextTech(const std::set<std::string>& known_techs, int empire_id);

    /** returns all researchable techs that progress from the given known techs to the given desired tech */
    std::vector<const Tech*>        NextTechsTowards(const std::set<std::string>& known_techs,
                                                     const std::string& desired_tech,
                                                     int empire_id);

    /** returns the cheapest researchable tech that progresses from the given known techs to the given desired tech */
    const Tech*                     CheapestNextTechTowards(const std::set<std::string>& known_techs,
                                                            const std::string& desired_tech,
                                                            int empire_id);

    /** iterator to the first tech */
    iterator                        begin() const;

    /** iterator to the last + 1th tech */
    iterator                        end() const;

    /** iterator to the first tech in category \a name */
    category_iterator               category_begin(const std::string& name) const;

    /** iterator to the last + 1th tech in category \a name */
    category_iterator               category_end(const std::string& name) const;

    /** Returns names of indicated tech's prerequisites, and all prereqs of
      * those techs, etc. recursively. If \a min_required is false then prereqs
      * will be included and recursed into even if already known to the empire. */
    std::vector<std::string>        RecursivePrereqs(const std::string& tech_name, int empire_id, bool min_required = true) const;

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                    GetCheckSum() const;
    //@}

    using TechParseTuple = std::tuple<
        TechManager::TechContainer, // techs_
        std::map<std::string, std::unique_ptr<TechCategory>>, // tech_categories,
        std::set<std::string> // categories_seen
        >;
    /** Sets types to the value of \p future. */
    FO_COMMON_API void SetTechs(Pending::Pending<TechParseTuple>&& future);


    /** returns the instance of this singleton class; you should use the free function GetTechManager() instead */
    static TechManager& GetTechManager();

private:
    TechManager();

    /** Assigns any m_pending_types to m_techs. */
    void CheckPendingTechs() const;

    /** returns an error string indicating the first instance of an illegal prerequisite relationship between
        two techs in m_techs, or an empty string if there are no illegal dependencies  */
    std::string FindIllegalDependencies() const;

    /** returns an error string indicating the first prerequisite dependency cycle found in m_techs, or an
        empty string if there are no dependency cycles */
    std::string FindFirstDependencyCycle() const;

    /** returns an error string indicating the first instance of a redundant dependency, or an empty string if there
        are no redundant dependencies.  An example of a redundant dependency is A --> C, if A --> B and B --> C. */
    std::string FindRedundantDependency() const;

    void AllChildren(const Tech* tech, std::map<std::string, std::string>& children) const;

    /** Future types being parsed by parser.  mutable so that it can
        be assigned to m_species_types when completed.*/
    mutable boost::optional<Pending::Pending<TechParseTuple>> m_pending_types = boost::none;

    mutable TechCategoryMap m_categories;
    mutable TechContainer   m_techs;

    static TechManager*     s_instance;
};

/** returns the singleton tech manager */
FO_COMMON_API TechManager& GetTechManager();

//! @brief Returns the ::Tech identified by @p name
//!
//! @param name
//! The identifying name of the requested ::Tech.
//!
//! @return
//! A pointer to the ::Tech matching @p name or nullptr if no ::Tech with that
//! name was found.
FO_COMMON_API const Tech* GetTech(const std::string& name);

/** returns a pointer to the tech category with the name \a name, or 0 if no such category exists */
FO_COMMON_API const TechCategory* GetTechCategory(const std::string& name);

#endif // _Tech_h_
