#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_

//! @file
//!     Declares the SitRepEntry calls and related factory functions.

#include "VarText.h"
#include "../universe/ConstantsFwd.h"
#include "../util/ranges.h"

#include <cstdint>
#include <string>
#include <vector>

#include "Export.h"

class ObjectMap;
class UniverseObject;
struct ScriptingContext;

#if !defined(CONSTEXPR_VEC_AND_STRING)
#  if defined(__cpp_lib_constexpr_vector) && defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
#    define CONSTEXPR_VEC_AND_STRING constexpr
#  else
#    define CONSTEXPR_VEC_AND_STRING
#  endif
#endif

//! Represents a situation report entry for a significant game event.
class FO_COMMON_API SitRepEntry final : public VarText {
public:
    SitRepEntry() noexcept = default;

    SitRepEntry(std::string template_string, int turn, std::string icon,
                std::string label, bool stringtable_lookup) noexcept :
        VarText(std::move(template_string), stringtable_lookup),
        m_turn(turn),
        m_icon(std::move(icon)),
        m_label(std::move(label))
    {}

    SitRepEntry(std::string template_string, int turn, std::string icon,
                std::string label, bool stringtable_lookup,
                VarText::VariablesVec data) noexcept :
        VarText(std::move(template_string), std::move(data), stringtable_lookup),
        m_turn(turn),
        m_icon(std::move(icon)),
        m_label(std::move(label))
    {}

    SitRepEntry(std::string template_string, int turn, std::string icon,
                std::string label, bool stringtable_lookup,
                std::pair<std::string, std::string> param_tag_data) noexcept :
        SitRepEntry(std::move(template_string), turn, std::move(icon),
                    std::move(label), stringtable_lookup,
                    [](auto&& param) {
                        VarText::VariablesVec retval;
                        retval.push_back(std::move(param));
                        return retval;
                    }(std::move(param_tag_data)))
    {}

    [[nodiscard]] const std::string& GetDataString(const std::string& tag) const;
    [[nodiscard]] int                GetTurn() const noexcept        { return m_turn; }
    [[nodiscard]] const std::string& GetIcon() const noexcept        { return m_icon; }
    [[nodiscard]] const std::string& GetLabelString() const noexcept { return m_label; }
    [[nodiscard]] std::string        Dump() const;

    [[nodiscard]] std::size_t SizeInMemory() const;

    struct FixedInfo {
        std::string m_template_string;
        std::string m_icon;
        std::string m_label;
        std::vector<std::string> m_variable_names;
        bool m_stringtable_lookup_flag = false;

        [[nodiscard]] CONSTEXPR_VEC_AND_STRING FixedInfo() noexcept = default;

        [[nodiscard]] CONSTEXPR_VEC_AND_STRING FixedInfo(std::string&& template_string,
                                                         std::string&& icon,
                                                         std::string&& label,
                                                         std::vector<std::string>&& var_names,
                                                         bool lookup) noexcept :
            m_template_string(std::move(template_string)),
            m_icon(std::move(icon)),
            m_label(std::move(label)),
            m_variable_names(std::move(var_names)),
            m_stringtable_lookup_flag(lookup)
        {}

        [[nodiscard]] CONSTEXPR_VEC_AND_STRING FixedInfo(const std::string& template_string,
                                                         const std::string& icon,
                                                         const std::string& label,
                                                         std::vector<std::string>&& var_names,
                                                         bool lookup) :
            m_template_string(template_string),
            m_icon(icon),
            m_label(label),
            m_variable_names(std::move(var_names)),
            m_stringtable_lookup_flag(lookup)
        {}

        CONSTEXPR_VEC_AND_STRING auto operator<=>(const FixedInfo&) const noexcept = default;

        friend class boost::serialization::access;
        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    struct UniqueInfo {
        CONSTEXPR_VEC_AND_STRING UniqueInfo() noexcept = default;

        CONSTEXPR_VEC_AND_STRING UniqueInfo(std::vector<std::string>&& variable_values, int turn) noexcept :
            m_variable_values(std::move(variable_values)),
            m_turn(turn)
        {}

        CONSTEXPR_VEC_AND_STRING UniqueInfo(const std::vector<std::string>& variable_values, int turn) :
            m_variable_values(variable_values),
            m_turn(turn)
        {}

        CONSTEXPR_VEC_AND_STRING UniqueInfo(std::string_view str, std::span<const std::pair<uint16_t, uint16_t>> offsets_sizes, int turn) :
            m_variable_values([str, offsets_sizes]() {
                const auto str_sz = static_cast<uint16_t>(str.size());
                std::vector<std::string> vals;
                vals.reserve(offsets_sizes.size());
                for (const auto& [offset, sz] : offsets_sizes)
                    vals.emplace_back(str.substr(std::min(offset, str_sz), sz));
                return vals;
            }()),
            m_turn(turn)
        {}

        CONSTEXPR_VEC_AND_STRING auto operator<=>(const UniqueInfo&) const = default;

        std::vector<std::string> m_variable_values;
        int m_turn = INVALID_GAME_TURN;
    };

    // separates data of SitRepEntry into parts likely to recur many times,
    // and parts that are likely to vary between otherwise similar sitreps
    auto SplitInfo() const& {
        return std::pair<FixedInfo, UniqueInfo>(std::piecewise_construct,
                                                std::forward_as_tuple(this->m_template_string, m_icon, m_label,
                                                                      this->m_variables | range_keys | range_to_vec,
                                                                      this->m_stringtable_lookup_flag),
                                                std::forward_as_tuple(this->m_variables | range_values | range_to_vec, m_turn));
    }
    auto SplitInfo() && {
        static constexpr auto extract_vec = [](auto&& rng) -> std::vector<std::string>
        { return std::vector(std::make_move_iterator(rng.begin()), std::make_move_iterator(rng.end())); };

        return std::pair<FixedInfo, UniqueInfo>(std::piecewise_construct,
                                                std::forward_as_tuple(std::move(this->m_template_string), std::move(m_icon), std::move(m_label),
                                                                      extract_vec(this->m_variables | range_keys),
                                                                      this->m_stringtable_lookup_flag),
                                                std::forward_as_tuple(extract_vec(this->m_variables | range_values), m_turn));
    }

    // recombines separated SitRep entry from parts
    SitRepEntry(FixedInfo fixed, UniqueInfo unique) :
        SitRepEntry(std::move(fixed.m_template_string), unique.m_turn, std::move(fixed.m_icon),
                    std::move(fixed.m_label), fixed.m_stringtable_lookup_flag,
                    [](auto&& var_names, auto&& var_vals) {
                        std::size_t sz = std::min(var_names.size(), var_vals.size());
                        VarText::VariablesVec vars;
                        vars.reserve(sz);
                        for (std::size_t idx = 0; idx < sz; ++idx)
                            vars.emplace_back(std::move(var_names[idx]), std::move(var_vals[idx]));
                        return vars;
                    }(std::move(fixed.m_variable_names), std::move(unique.m_variable_values)))
    {}

private:
    int         m_turn = INVALID_GAME_TURN;
    std::string m_icon;
    std::string m_label;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

//! @name SitRepEntry factories
//!
//! Factory functions to create SitRepEntry(s) for specific situation report
//! events.
//!
//! @{
[[nodiscard]] SitRepEntry CreateTechResearchedSitRep(std::string tech_name, int current_turn);
[[nodiscard]] SitRepEntry CreateShipBuiltSitRep(int ship_id, int system_id, int shipdesign_id, int current_turn);
[[nodiscard]] SitRepEntry CreateShipBlockBuiltSitRep(int system_id, int shipdesign_id, int number, int current_turn);
[[nodiscard]] SitRepEntry CreateBuildingBuiltSitRep(int building_id, int planet_id, int current_turn);

[[nodiscard]] SitRepEntry CreateTechUnlockedSitRep(std::string tech_name, int current_turn);
[[nodiscard]] SitRepEntry CreatePolicyUnlockedSitRep(std::string policy_name, int current_turn);
[[nodiscard]] SitRepEntry CreateBuildingTypeUnlockedSitRep(std::string building_type_name, int current_turn);
[[nodiscard]] SitRepEntry CreateShipHullUnlockedSitRep(std::string ship_hull_name, int current_turn);
[[nodiscard]] SitRepEntry CreateShipPartUnlockedSitRep(std::string ship_part_name, int current_turn);

[[nodiscard]] FO_COMMON_API SitRepEntry CreateCombatSitRep(int system_id, int log_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateGroundCombatSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetCapturedSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetRebelledSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateCombatDamagedObjectSitRep(
    const UniverseObject* obj, int combat_system_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateCombatDestroyedObjectSitRep(
    const UniverseObject* obj, int combat_system_id, int empire_id, int current_turn);
[[nodiscard]] SitRepEntry               CreatePlanetDepopulatedSitRep(int planet_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetAnnexedSitRep(int planet_id, int original_owner_id, int annexer_empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetColonizedSitRep(int planet_id, std::string species, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetOutpostedSitRep(int planet_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetEstablishFailedSitRep(int planet_id, int ship_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetEstablishFailedVisibleOtherSitRep(int planet_id, int ship_id, int other_empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetEstablishFailedArmedSitRep(int planet_id, int ship_id, int other_empire_id, int current_turn);

[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetGiftedSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetGiftedSitRep(int fleet_id, int empire_id, int current_turn);

[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetArrivedAtDestinationSitRep(
    int system_id, int fleet_id, int recipient_empire_id, const ScriptingContext& context);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetBlockadedSitRep(
    int system_id, int blockaded_fleet_id, int blockaded_empire_id,
    int blockading_empire_id, const ScriptingContext& context);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetBlockadedSitRep(
    int system_id, int blockaded_fleet_id, int blockaded_empire_id, const ScriptingContext& context);

[[nodiscard]] SitRepEntry               CreateEmpireEliminatedSitRep(int empire_id, int current_turn);
[[nodiscard]] SitRepEntry               CreateVictorySitRep(std::string reason_string, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateSitRep(std::string template_string, int turn,
                                                     std::string icon,
                                                     std::vector<std::pair<std::string, std::string>> parameters,
                                                     std::string label = "", bool stringtable_lookup = true);
//! @}

template <typename Archive>
void SitRepEntry::FixedInfo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_template_string)
        & BOOST_SERIALIZATION_NVP(m_stringtable_lookup_flag)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_label)
        & BOOST_SERIALIZATION_NVP(m_variable_names);
}

template <typename Archive>
void SitRepEntry::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VarText)
        & BOOST_SERIALIZATION_NVP(m_turn)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_label);
}


#endif
