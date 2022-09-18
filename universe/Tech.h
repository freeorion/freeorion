#ifndef _Tech_h_
#define _Tech_h_


#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/optional/optional.hpp>
#include "EnumsFwd.h"
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Effect
{ class EffectsGroup; }
namespace ValueRef {
    template <typename T>
    struct ValueRef;
}
class TechManager;
struct UnlockableItem;
struct ScriptingContext;

/** encasulates the data for a single FreeOrion technology */
class FO_COMMON_API Tech {
public:
    /** Helper struct for parsing tech definitions */
    struct TechInfo {
        TechInfo() = default;
        TechInfo(std::string& name_, std::string& description_,
                 std::string& short_description_, std::string& category_,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& research_cost_,
                 std::unique_ptr<ValueRef::ValueRef<int>>&& research_turns_,
                 bool researchable_,
                 std::set<std::string>& tags_);
        ~TechInfo();

        std::string             name;
        std::string             description;
        std::string             short_description;
        std::string             category;
        std::unique_ptr<ValueRef::ValueRef<double>> research_cost;
        std::unique_ptr<ValueRef::ValueRef<int>>    research_turns;
        bool                    researchable = false;
        std::set<std::string>   tags;
    };

    Tech(std::string&& name, std::string&& description,
         std::string&& short_description, std::string&& category,
         std::unique_ptr<ValueRef::ValueRef<double>>&& research_cost,
         std::unique_ptr<ValueRef::ValueRef<int>>&& research_turns,
         bool researchable, std::set<std::string>&& tags,
         std::vector<std::shared_ptr<Effect::EffectsGroup>>&& effects,
         std::set<std::string>&& prerequisites,
         std::vector<UnlockableItem>&& unlocked_items,
         std::string&& graphic);

    bool operator==(const Tech& rhs) const;
    bool operator!=(const Tech& rhs) const
    { return !(*this == rhs); }

    [[nodiscard]] const std::string&  Name() const                { return m_name; }              //!< returns name of this tech
    [[nodiscard]] const std::string&  Description() const         { return m_description; }       //!< Returns the text description of this tech
    [[nodiscard]] const std::string&  ShortDescription() const    { return m_short_description; } //!< Returns the single-line short text description of this tech
    [[nodiscard]] std::string         Dump(uint8_t ntabs = 0) const;                                      //!< Returns a text representation of this object
    [[nodiscard]] const std::string&  Category() const            { return m_category; }                  //!< retursn the name of the category to which this tech belongs
    [[nodiscard]] float               ResearchCost(int empire_id, const ScriptingContext& context) const; //!< returns the total research cost in RPs required to research this tech
    [[nodiscard]] float               PerTurnCost(int empire_id, const ScriptingContext& context) const;  //!< returns the maximum number of RPs per turn allowed to be spent on researching this tech
    [[nodiscard]] int                 ResearchTime(int empire_id, const ScriptingContext& context) const; //!< returns the number of turns required to research this tech, if ResearchCost() RPs are spent per turn
    [[nodiscard]] bool                Researchable() const        { return m_researchable; }              //!< returns whether this tech is researchable by players and appears on the tech tree

    [[nodiscard]] const auto&         Tags() const noexcept { return m_tags; }
    [[nodiscard]] const auto&         PediaTags() const noexcept { return m_pedia_tags; }
    [[nodiscard]] bool                HasTag(std::string_view tag) const
    { return std::any_of(m_tags.begin(), m_tags.end(), [tag](const auto& t) { return t == tag; }); }

    /** returns the effects that are applied to the discovering empire's capital
      * when this tech is researched; not all techs have effects, in which case
      * this returns 0 */
    [[nodiscard]] const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const { return m_effects; }

    [[nodiscard]] const std::set<std::string>&       Prerequisites() const   { return m_prerequisites; }       //!< returns the set of names of all techs required before this one can be researched
    [[nodiscard]] const std::string&                 Graphic() const         { return m_graphic; }             //!< returns the name of the grapic file for this tech
    [[nodiscard]] const std::vector<UnlockableItem>& UnlockedItems() const   { return m_unlocked_items; }      //! Returns the set all items that are unlocked by researching this tech
    [[nodiscard]] const ValueRef::ValueRef<double>*  ResearchCostRef() const { return m_research_cost.get(); } //!< return value ref of research cost
    [[nodiscard]] const ValueRef::ValueRef<int>*     ResearchTurnsRef() const{ return m_research_turns.get(); }//!< return value ref of research turns
    [[nodiscard]] const std::set<std::string>&       UnlockedTechs() const   { return m_unlocked_techs; }      //!< returns the set of names of all techs for which this one is a prerequisite

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] unsigned int                      GetCheckSum() const;

private:
    Tech(const Tech&) = delete;
    Tech& operator=(const Tech&) = delete;
    void Init();

    std::string                     m_name;
    std::string                     m_description;
    std::string                     m_short_description;
    std::string                     m_category;
    std::unique_ptr<ValueRef::ValueRef<double>> m_research_cost;
    std::unique_ptr<ValueRef::ValueRef<int>>    m_research_turns;
    bool                            m_researchable = false;
    const std::string                   m_tags_concatenated;
    const std::vector<std::string_view> m_tags;
    const std::vector<std::string_view> m_pedia_tags;
    std::vector<std::shared_ptr<Effect::EffectsGroup>> m_effects;
    std::set<std::string>           m_prerequisites;
    std::vector<UnlockableItem>     m_unlocked_items;
    std::string                     m_graphic;
    std::set<std::string>           m_unlocked_techs;

    friend class TechManager;
};


/** specifies a category of techs, with associated \a name, \a graphic (icon), and \a colour.*/
struct FO_COMMON_API TechCategory {
    TechCategory() = default;
    TechCategory(std::string name_, std::string&& graphic_,
                 std::array<uint8_t, 4> colour_):
        name(std::move(name_)),
        graphic(std::move(graphic_)),
        colour(colour_)
    {}
    std::string            name;                           ///< name of category
    std::string            graphic;                        ///< icon that represents catetegory
    std::array<uint8_t, 4> colour{{255, 255, 255, 255}};   ///< colour associatied with category
};

namespace CheckSums {
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

    using TechCategoryMap = std::map<std::string, std::unique_ptr<TechCategory>, std::less<>>;

    /** iterator that runs over techs within a category */
    typedef TechContainer::index<CategoryIndex>::type::const_iterator category_iterator;

    /** iterator that runs over all techs */
    typedef TechContainer::index<NameIndex>::type::const_iterator iterator;

    /** returns the tech with the name \a name; you should use the free function GetTech() instead */
    [[nodiscard]] const Tech*              GetTech(std::string_view name) const;

    /** returns the tech category with the name \a name; you should use the free function GetTechCategory() instead */
    [[nodiscard]] const TechCategory*      GetTechCategory(std::string_view name) const;

    /** returns the list of category names */
    [[nodiscard]] std::vector<std::string_view> CategoryNames() const;

    /** returns list of all tech names */
    [[nodiscard]] std::vector<std::string_view> TechNames() const;

    /** returns list of names of techs in specified category */
    [[nodiscard]] std::vector<std::string_view> TechNames(const std::string& name) const;

    /** returns all researchable techs */
    [[nodiscard]] std::vector<const Tech*> AllNextTechs(const std::set<std::string>& known_techs);

    /** returns the cheapest researchable tech */
    [[nodiscard]] const Tech* CheapestNextTech(
        const std::set<std::string>& known_techs, int empire_id,
        const ScriptingContext& context);

    /** returns all researchable techs that progress from the given known techs to the given desired tech */
    [[nodiscard]] std::vector<const Tech*> NextTechsTowards(
        const std::set<std::string>& known_techs,
        const std::string& desired_tech, int empire_id);

    /** returns the cheapest researchable tech that progresses from the given known techs to the given desired tech */
    [[nodiscard]] const Tech* CheapestNextTechTowards(
        const std::set<std::string>& known_techs, const std::string& desired_tech,
        int empire_id, const ScriptingContext& context);

    [[nodiscard]] std::size_t              size() const;

    /** iterator to the first tech */
    [[nodiscard]] iterator                 begin() const;

    /** iterator to the last + 1th tech */
    [[nodiscard]] iterator                 end() const;

    /** iterator to the first tech in category \a name */
    [[nodiscard]] category_iterator        category_begin(const std::string& name) const;

    /** iterator to the last + 1th tech in category \a name */
    [[nodiscard]] category_iterator        category_end(const std::string& name) const;

    /** Returns names of indicated tech's prerequisites, and all prereqs of
      * those techs, etc. recursively. If \a min_required is false then prereqs
      * will be included and recursed into even if already known to the empire. */
    [[nodiscard]] std::vector<std::string> RecursivePrereqs(
        const std::string& tech_name, int empire_id, bool min_required,
        const ScriptingContext& context) const;

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] unsigned int             GetCheckSum() const;

    using TechParseTuple = std::tuple<
        TechManager::TechContainer, // techs_
        std::map<std::string, std::unique_ptr<TechCategory>, std::less<>>, // tech_categories,
        std::set<std::string> // categories_seen
        >;
    /** Sets types to the value of \p future. */
    FO_COMMON_API void SetTechs(Pending::Pending<TechParseTuple>&& future);


    /** returns the instance of this singleton class; you should use the free function GetTechManager() instead */
    [[nodiscard]] static TechManager& GetTechManager();

private:
    TechManager();

    /** Assigns any m_pending_types to m_techs. */
    void CheckPendingTechs() const;

    /** returns an error string indicating the first instance of an illegal prerequisite relationship between
        two techs in m_techs, or an empty string if there are no illegal dependencies  */
    [[nodiscard]] std::string FindIllegalDependencies() const;

    /** returns an error string indicating the first prerequisite dependency cycle found in m_techs, or an
        empty string if there are no dependency cycles */
    [[nodiscard]] std::string FindFirstDependencyCycle() const;

    /** returns an error string indicating the first instance of a redundant dependency, or an empty string if there
        are no redundant dependencies.  An example of a redundant dependency is A --> C, if A --> B and B --> C. */
    [[nodiscard]] std::string FindRedundantDependency() const;

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
FO_COMMON_API const Tech* GetTech(std::string_view name);

/** returns a pointer to the tech category with the name \a name, or 0 if no such category exists */
FO_COMMON_API const TechCategory* GetTechCategory(std::string_view name);

#endif
