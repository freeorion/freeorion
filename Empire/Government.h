#ifndef _Government_h_
#define _Government_h_

#include "../universe/Effect.h"
#include "../universe/ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/Export.h"
#include "../util/Pending.h"

#include <boost/optional/optional.hpp>

#include <memory>
#include <string>
#include <map>
#include <set>


class FO_COMMON_API Policy {
public:
    Policy(std::string name, std::string description,
           std::string short_description, std::string category,
           std::unique_ptr<ValueRef::ValueRef<double>>&& adoption_cost,
           std::set<std::string>&& prerequisites,
           std::set<std::string>&& exclusions,
           std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
           std::vector<UnlockableItem>&& unlocked_items,
           std::string graphic);

    Policy(Policy&&) = default;
    Policy& operator=(Policy&&) = default;

    [[nodiscard]] const auto& Name() const noexcept             { return m_name; }
    [[nodiscard]] const auto& Description() const noexcept      { return m_description; }
    [[nodiscard]] const auto& ShortDescription() const noexcept { return m_short_description; }
    [[nodiscard]] const auto& Category() const noexcept         { return m_category; }
    [[nodiscard]] const auto& Prerequisites() const noexcept    { return m_prerequisites; }
    [[nodiscard]] const auto& Exclusions() const noexcept       { return m_exclusions; }
    [[nodiscard]] const auto& Effects() const noexcept          { return m_effects; }
    [[nodiscard]] const auto& Graphic() const noexcept          { return m_graphic; }
    [[nodiscard]] const auto& UnlockedItems() const noexcept    { return m_unlocked_items; }

    [[nodiscard]] float       AdoptionCost(int empire_id, const ScriptingContext& context) const;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server. */
    [[nodiscard]] uint32_t GetCheckSum() const;

private:
    Policy(const Policy&) = delete;
    Policy& operator=(const Policy&) = delete;

    std::string                                 m_name;
    std::string                                 m_description;
    std::string                                 m_short_description;
    std::string                                 m_category;
    std::unique_ptr<ValueRef::ValueRef<double>> m_adoption_cost;
    std::vector<std::string>                    m_prerequisites;
    std::vector<std::string>                    m_exclusions;
    std::vector<Effect::EffectsGroup>           m_effects;
    std::vector<UnlockableItem>                 m_unlocked_items;
    std::string                                 m_graphic;

    friend class PolicyManager;
};

//! Keeps track of policies that can be chosen by empires.
class FO_COMMON_API PolicyManager {
public:
    using PoliciesTypeMap = boost::container::flat_map<std::string, Policy, std::less<>>;
    using iterator = PoliciesTypeMap::const_iterator;
    using const_iterator = iterator;

    //! returns the policy with the name \a name; you should use the free
    //! function GetPolicy() instead
    [[nodiscard]] const Policy*                 GetPolicy(std::string_view name) const;
    [[nodiscard]] std::vector<std::string_view> PolicyNames() const;
    //! returns list of names of policies in specified category
    [[nodiscard]] std::vector<std::string_view> PolicyNames(const std::string& category_name) const;
    [[nodiscard]] std::vector<std::string_view> PolicyCategories() const; // sorted
    [[nodiscard]] uint32_t                      GetCheckSum() const;

    [[nodiscard]] iterator begin() const; //! iterator to the first policy
    [[nodiscard]] iterator end() const;   //! iterator to the last + 1th policy

    //! sets types to the value of \p future
    void SetPolicies(Pending::Pending<std::vector<Policy>>&& future);

private:
    void CheckPendingPolicies() const;  //! Assigns any m_pending_types to m_policies.

    //! Future types being parsed by parser.  mutable so that it can
    //! be assigned to m_species_types when completed.
    mutable boost::optional<Pending::Pending<std::vector<Policy>>> m_pending_types = boost::none;

    mutable PoliciesTypeMap m_policies;
};

[[nodiscard]] FO_COMMON_API PolicyManager& GetPolicyManager();
[[nodiscard]] FO_COMMON_API const Policy* GetPolicy(std::string_view name);


#endif
