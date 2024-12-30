#ifndef _Tech_h_
#define _Tech_h_


#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/optional/optional.hpp>
#include "Effect.h"
#include "EnumsFwd.h"
#include "../util/Export.h"
#include "../util/Pending.h"


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

    Tech(std::string&& name, std::string&& description,
         std::string&& short_description, std::string&& category,
         std::unique_ptr<ValueRef::ValueRef<double>>&& research_cost,
         std::unique_ptr<ValueRef::ValueRef<int>>&& research_turns,
         bool researchable, std::set<std::string>&& tags,
         std::vector<Effect::EffectsGroup>&& effects,
         std::set<std::string>&& prerequisites,
         std::vector<UnlockableItem>&& unlocked_items,
         std::string&& graphic);

    [[nodiscard]] bool operator==(const Tech& rhs) const;
    Tech(const Tech&) = delete;
    Tech(Tech&&) = default;
    Tech& operator=(const Tech&) = delete;
    Tech& operator=(Tech&&) = default;

    [[nodiscard]] const auto& Name() const noexcept             { return m_name; }
    [[nodiscard]] const auto& Description() const noexcept      { return m_description; }
    [[nodiscard]] const auto& ShortDescription() const noexcept { return m_short_description; }
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const;
    [[nodiscard]] const auto& Category() const noexcept         { return m_category; }
    [[nodiscard]] float       ResearchCost(int empire_id, const ScriptingContext& context) const;
    [[nodiscard]] float       PerTurnCost(int empire_id, const ScriptingContext& context) const;
    [[nodiscard]] int         ResearchTime(int empire_id, const ScriptingContext& context) const;
    [[nodiscard]] bool        Researchable() const noexcept     { return m_researchable; }

    [[nodiscard]] const auto& Tags() const noexcept { return m_tags; }
    [[nodiscard]] const auto& PediaTags() const noexcept { return m_pedia_tags; }
    [[nodiscard]] bool        HasTag(std::string_view tag) const
    { return std::any_of(m_tags.begin(), m_tags.end(), [tag](const auto& t) noexcept { return t == tag; }); }

    [[nodiscard]] const auto& Effects() const noexcept          { return m_effects; }
    [[nodiscard]] const auto& Prerequisites() const noexcept    { return m_prerequisites; }
    [[nodiscard]] const auto& Graphic() const noexcept          { return m_graphic; }
    [[nodiscard]] const auto& UnlockedItems() const noexcept    { return m_unlocked_items; }
    [[nodiscard]] const auto* ResearchCostRef() const noexcept  { return m_research_cost.get(); }
    [[nodiscard]] const auto* ResearchTurnsRef() const noexcept { return m_research_turns.get(); }
    [[nodiscard]] const auto& UnlockedTechs() const noexcept    { return m_unlocked_techs; }

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] uint32_t GetCheckSum() const;

private:
    std::string                       m_name;
    std::string                       m_description;
    std::string                       m_short_description;
    std::string                       m_category;
    std::unique_ptr<ValueRef::ValueRef<double>> m_research_cost;
    std::unique_ptr<ValueRef::ValueRef<int>>    m_research_turns;
    bool                              m_researchable = false;
    std::string                       m_tags_concatenated;
    std::vector<std::string_view>     m_tags;
    std::vector<std::string_view>     m_pedia_tags;
    std::vector<Effect::EffectsGroup> m_effects;
    std::vector<std::string>          m_prerequisites;
    std::vector<UnlockableItem>       m_unlocked_items;
    std::string                       m_graphic;
    std::vector<std::string>          m_unlocked_techs;

    friend class TechManager;
};


/** specifies a category of techs, with associated \a name, \a graphic (icon), and \a colour.*/
struct FO_COMMON_API TechCategory {
    TechCategory() = default;
    TechCategory(std::string name_, std::string&& graphic_,
                 std::array<uint8_t, 4> colour_) noexcept :
        name(std::move(name_)),
        graphic(std::move(graphic_)),
        colour(colour_)
    {}
    std::string            name;                           ///< name of category
    std::string            graphic;                        ///< icon that represents catetegory
    std::array<uint8_t, 4> colour{{255, 255, 255, 255}};   ///< colour associatied with category
};

namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, const TechCategory& cat);
}

/** holds all FreeOrion techs.  Techs may be looked up by name and by category, and the next researchable techs can be querried,
    given a set of currently-known techs. */
class FO_COMMON_API TechManager {
public:
    using TechContainer = boost::container::flat_map<std::string, Tech, std::less<>>;
    using const_iterator = TechContainer::const_iterator;
    using iterator = TechContainer::const_iterator;
    using TechCategoryContainer = boost::container::flat_map<std::string, TechCategory, std::less<>>;

    /** returns the tech with the name \a name; you should use the free function GetTech() instead */
    [[nodiscard]] const Tech* GetTech(std::string_view name) const;

    /** returns the tech category with the name \a name; you should use the free function GetTechCategory() instead */
    [[nodiscard]] const TechCategory* GetTechCategory(std::string_view name) const;

    /** returns the list of category names */
    [[nodiscard]] std::vector<std::string_view> CategoryNames() const;

    /** returns list of all tech names */
    [[nodiscard]] std::vector<std::string_view> TechNames() const;

    /** returns list of names of techs in specified category */
    [[nodiscard]] std::vector<std::string_view> TechNames(std::string_view name) const;

    /** returns all researchable techs */
    [[nodiscard]] std::vector<const Tech*> AllNextTechs(const std::vector<std::string_view>& researched_techs);

    /** returns the cheapest researchable tech */
    [[nodiscard]] const Tech* CheapestNextTech(
        const std::vector<std::string_view>& researched_techs, int empire_id, const ScriptingContext& context);

    [[nodiscard]] TechContainer::size_type size() const;
    [[nodiscard]] iterator begin() const;
    [[nodiscard]] iterator end() const;

    /** Returns names of indicated tech's prerequisites, and all prereqs of
      * those techs, etc. recursively. */
    [[nodiscard]] std::vector<std::string> RecursivePrereqs(
        std::string_view tech_name, int empire_id, const ScriptingContext& context) const;

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] uint32_t GetCheckSum() const;

    using TechParseTuple = std::tuple<
        TechContainer,          // techs
        TechCategoryContainer,  // tech_categories,
        std::set<std::string>   // categories_seen
        >;
    /** Sets types to the value of \p future. */
    FO_COMMON_API void SetTechs(Pending::Pending<TechParseTuple>&& future);

private:
    /** Assigns any m_pending_types to m_techs. */
    void CheckPendingTechs() const;

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

    mutable TechContainer         m_techs;
    mutable TechCategoryContainer m_categories;
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
