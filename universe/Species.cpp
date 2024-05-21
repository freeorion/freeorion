#include "Species.h"

#include <iterator>
#include <boost/filesystem/fstream.hpp>
#include "Conditions.h"
#include "CommonParams.h"
#include "Effect.h"
#include "Planet.h"
#include "Ship.h"
#include "UniverseObject.h"
#include "ValueRefs.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/GameRules.h"
#include "../util/GameRuleRanks.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/ScopedTimer.h"


/////////////////////////////////////////////////
// FocusType                                   //
/////////////////////////////////////////////////
FocusType::FocusType(std::string name, std::string description,
                     std::unique_ptr<Condition::Condition>&& location,
                     std::string graphic) :
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_location(std::move(location)),
    m_graphic(std::move(graphic))
{}

FocusType::~FocusType() = default;

bool FocusType::operator==(const FocusType& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_graphic != rhs.m_graphic)
    { return false; }

    if (m_location == rhs.m_location) { // could be nullptr
        // check next member
    } else if (!m_location || !rhs.m_location) {
        return false;
    } else if (*m_location != *(rhs.m_location)) {
        return false;
    }

    return true;
}

std::string FocusType::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "FocusType\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "location = \n";
    retval += m_location->Dump(ntabs+2);
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

uint32_t FocusType::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

/////////////////////////////////////////////////
// Species                                     //
/////////////////////////////////////////////////
namespace {
    void AddRules(GameRules& rules) {
        rules.Add<double>(UserStringNop("RULE_ANNEX_COST_OPINION_EXP_BASE"),
                          UserStringNop("RULE_ANNEX_COST_OPINION_EXP_BASE_DESC"),
                          GameRuleCategories::GameRuleCategory::BALANCE_STABILITY, 1.2, true,
                          GameRuleRanks::RULE_ANNEX_COST_OPINION_EXP_BASE_RANK,
                          RangedValidator<double>(0.0, 3.0));
        rules.Add<double>(UserStringNop("RULE_ANNEX_COST_STABILITY_EXP_BASE"),
                          UserStringNop("RULE_ANNEX_COST_STABILITY_EXP_BASE_DESC"),
                          GameRuleCategories::GameRuleCategory::BALANCE_STABILITY, 1.1, true,
                          GameRuleRanks::RULE_ANNEX_COST_STABILITY_EXP_BASE_RANK,
                          RangedValidator<double>(0.0, 3.0));
        rules.Add<double>(UserStringNop("RULE_ANNEX_COST_SCALING"),
                          UserStringNop("RULE_ANNEX_COST_SCALING_DESC"),
                          GameRuleCategories::GameRuleCategory::BALANCE_STABILITY, 5.0, true,
                          GameRuleRanks::RULE_ANNEX_COST_SCALING_RANK,
                          RangedValidator<double>(0.0, 50.0));
        rules.Add<double>(UserStringNop("RULE_BUILDING_ANNEX_COST_SCALING"),
                          UserStringNop("RULE_BUILDING_ANNEX_COST_SCALING_DESC"),
                          GameRuleCategories::GameRuleCategory::BALANCE_STABILITY, 0.25, true,
                          GameRuleRanks::RULE_BUILDING_ANNEX_COST_SCALING_RANK,
                          RangedValidator<double>(0.0, 5.0));
        rules.Add<double>(UserStringNop("RULE_ANNEX_COST_MINIMUM"),
                          UserStringNop("RULE_ANNEX_COST_MINIMUM_DESC"),
                          GameRuleCategories::GameRuleCategory::BALANCE_STABILITY, 5.0, true,
                          GameRuleRanks::RULE_ANNEX_COST_MINIMUM_RANK,
                          RangedValidator<double>(0.0, 50.0));
    }
    bool temp_bool = RegisterGameRules(&AddRules);

    auto ConcatenateAsVector(auto&& stringset1, auto&& stringset2, auto&& stringset3) {
        std::vector<std::string::value_type> retval;
        retval.reserve((stringset1.size() + stringset2.size() + stringset3.size())*30); // guesstimate
        for (const auto& s : {stringset1, stringset2, stringset3})
            for (const auto& t : s) {
                const auto upperized_t = boost::to_upper_copy<std::string>(t);
                retval.insert(retval.end(), upperized_t.begin(), upperized_t.end());
            }
        return retval;
    }

    std::vector<std::string_view> StringViewsForTags(const auto& tags, std::string_view concat_tags) {
        std::vector<std::string_view> retval;
        retval.reserve(tags.size());
        std::size_t next_idx = 0;

        // store views into concatenated tags/likes string
        std::for_each(tags.begin(), tags.end(), [&next_idx, &retval, concat_tags](const auto tag) {
            // determine how much space each tag takes up after being converted to upper case
            const auto upperized_tag = boost::to_upper_copy<std::string>(tag);
            const auto upper_sz = upperized_tag.size();
            retval.push_back(concat_tags.substr(next_idx, upper_sz));
            next_idx += upper_sz;
        });
        return retval;
    }

    std::vector<std::string_view> StringViewsForPediaTags(const auto& tags, std::string_view concat_tags) {
        std::vector<std::string_view> retval;
        retval.reserve(tags.size());
        std::size_t next_idx = 0;

        // store views into concatenated tags/likes string
        std::for_each(tags.begin(), tags.end(), [&next_idx, &retval, concat_tags] (const auto tag) {
            const auto upperized_tag = boost::to_upper_copy<std::string>(tag);
            const auto upper_sz = upperized_tag.size();
            static constexpr auto len{TAG_PEDIA_PREFIX.length()};
            if (tag.substr(0, len) == TAG_PEDIA_PREFIX) {
                // store string views into the pedia tag after the "PEDIA" prefix
                const auto full_tag = concat_tags.substr(next_idx, upper_sz);
                const auto after_prefix_tag = full_tag.substr(len);
                retval.push_back(after_prefix_tag);
            }
            next_idx += upper_sz;
        });
        return retval;
    }

    auto InitLocation(const std::string& name) {
        // set up a Condition structure to match Planets that have
        // (not uninhabitable) environment for this species

        std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>> environments;
        environments.push_back(
            std::make_unique<ValueRef::Constant<PlanetEnvironment>>(PlanetEnvironment::PE_UNINHABITABLE));

        auto this_species_name_ref =
            std::make_unique<ValueRef::Constant<std::string>>(name); // name specifies this species

        auto enviro_cond = std::unique_ptr<Condition::Condition>(
            std::make_unique<Condition::Not>(
                std::unique_ptr<Condition::Condition>(
                    std::make_unique<Condition::PlanetEnvironment>(
                        std::move(environments), std::move(this_species_name_ref)))));

        auto type_cond = std::make_unique<Condition::Type>(
            std::make_unique<ValueRef::Constant<UniverseObjectType>>(UniverseObjectType::OBJ_PLANET));

        auto retval = std::make_unique<Condition::And>(std::move(enviro_cond), std::move(type_cond));
        retval->SetTopLevelContent(name);

        return retval;
    }

    auto SourceOwner()
    { return std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner"); }

    auto NoRebelTroops() {
        return std::make_unique<Condition::MeterValue>(MeterType::METER_REBEL_TROOPS,
                                                       nullptr,
                                                       std::make_unique<ValueRef::Constant<double>>(0.0));
    }

    auto HasPopulation() {
        return std::make_unique<Condition::MeterValue>(MeterType::METER_POPULATION,
                                                       std::make_unique<ValueRef::Constant<double>>(0.001),
                                                       nullptr);
    }

    auto NotConqueredRecently(ValueRef::ReferenceType ref_type = ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE) {
        return std::make_unique<Condition::ValueTest>(
            std::make_unique<ValueRef::Variable<int>>(ref_type, "TurnsSinceLastConquered"),
            Condition::ComparisonType::GREATER_THAN,
            std::make_unique<ValueRef::Constant<int>>(1)
        );
    }
    auto NotAnnexedRecently(ValueRef::ReferenceType ref_type = ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE) {
        return std::make_unique<Condition::ValueTest>(
            std::make_unique<ValueRef::Variable<int>>(ref_type, "TurnsSinceAnnexation"),
            Condition::ComparisonType::GREATER_THAN,
            std::make_unique<ValueRef::Constant<int>>(1)
        );
    }
    auto NotColonizedRecently(ValueRef::ReferenceType ref_type = ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE) {
        return std::make_unique<Condition::ValueTest>(
            std::make_unique<ValueRef::Variable<int>>(ref_type, "TurnsSinceColonization"),
            Condition::ComparisonType::GREATER_THAN,
            std::make_unique<ValueRef::Constant<int>>(1)
        );
    }

    auto ResourceSupplyConnectedToAcceptableAnnexerPlanet() {
        return std::make_unique<Condition::ResourceSupplyConnectedByEmpire>(
            SourceOwner(),
            std::make_unique<Condition::And>(
                std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET),
                std::make_unique<Condition::EmpireAffiliation>(SourceOwner()),
                NotConqueredRecently(),
                NotAnnexedRecently(),
                NotColonizedRecently()
            )
        );
    }

    auto DefaultAnnexationCondition() {
        return std::make_unique<Condition::And>(
            std::make_unique<Condition::Or>(
                std::make_unique<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_NONE),
                std::make_unique<Condition::EmpireAffiliation>(SourceOwner(), EmpireAffiliationType::AFFIL_ENEMY)
            ),
            NotConqueredRecently(),
            NotAnnexedRecently(),
            NotColonizedRecently(),
            std::make_unique<Condition::VisibleToEmpire>(SourceOwner()),
            HasPopulation(),
            NoRebelTroops(),
            ResourceSupplyConnectedToAcceptableAnnexerPlanet()
        );
    }

    auto LocalCandidateOwner()
    { return std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Owner"); }

    auto SpeciesOpinion(std::unique_ptr<ValueRef::Variable<int>>&& empire_id_ref) {
        return std::make_unique<ValueRef::ComplexVariable<double>>(
            "SpeciesEmpireOpinion",
            std::move(empire_id_ref),
            nullptr, nullptr,
            std::make_unique<ValueRef::Variable<std::string>>(
                ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Species")
        );
    }

    auto HasOwner() { // 0.0 if unowned or 1.0 if owned by an empire
        return std::make_unique<ValueRef::StaticCast<int, double>>(
            std::make_unique<ValueRef::Operation<int>>(
                ValueRef::OpType::COMPARE_NOT_EQUAL,
                std::make_unique<ValueRef::Constant<int>>(ALL_EMPIRES),
                LocalCandidateOwner()
            )
        );
    }

    template <typename VR>
    concept evalable = requires(const VR vr, const ScriptingContext c) { vr->Eval(c); };

    auto operator+(evalable auto&& lhs, evalable auto&& rhs) {
        return std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::PLUS, std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
    }

    auto operator-(evalable auto&& lhs, evalable auto&& rhs) {
        return std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::MINUS, std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
    }

    auto operator*(evalable auto&& lhs, evalable auto&& rhs) {
        return std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::TIMES, std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
    }

    auto operator^(evalable auto&& lhs, evalable auto&& rhs) {
        return std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::EXPONENTIATE, std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
    }

    auto operator-(evalable auto&& rhs) {
        return std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::NEGATE, std::forward<decltype(rhs)>(rhs));
    }

    auto SpeciesOpinionOfOwnerOrZero() // 0.0 if unowned, or the opinion of the species on the planet of the owner empire
    { return HasOwner() * SpeciesOpinion(LocalCandidateOwner()); }

    auto PlanetStability() {
        return std::make_unique<ValueRef::Variable<double>>(
            ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Happiness"
        );
    }

    auto AnnexCostScaling() {
        return std::make_unique<ValueRef::ComplexVariable<double>>(
            "GameRule", nullptr, nullptr, nullptr,
            std::make_unique<ValueRef::Constant<std::string>>("RULE_ANNEX_COST_SCALING"));
    }

    auto LocalCandidatePop() {
        return std::make_unique<ValueRef::Variable<double>>(
            ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Population");
    }

    auto ScaledPop() // (pop * scale_factor)
    { return AnnexCostScaling() * LocalCandidatePop(); }

    auto AnnexCostOpinionBase() {
        return std::make_unique<ValueRef::ComplexVariable<double>>(
            "GameRule", nullptr, nullptr, nullptr,
            std::make_unique<ValueRef::Constant<std::string>>("RULE_ANNEX_COST_OPINION_EXP_BASE"));
    }

    auto ExpBaseSpeciesOpinion() // (base^(owner_opinion - annexer_opinion))
    { return AnnexCostOpinionBase() ^ (SpeciesOpinionOfOwnerOrZero() - SpeciesOpinion(SourceOwner())); }

    auto AnnexCostStabilityBase() {
        return std::make_unique<ValueRef::ComplexVariable<double>>(
            "GameRule", nullptr, nullptr, nullptr,
            std::make_unique<ValueRef::Constant<std::string>>("RULE_ANNEX_COST_STABILITY_EXP_BASE"));
    }

    auto ExpBaseStability() // (base^stability)
    { return AnnexCostStabilityBase() ^ PlanetStability(); }

    auto MinimumAnnexCost() {
        return std::make_unique<ValueRef::ComplexVariable<double>>(
            "GameRule", nullptr, nullptr, nullptr,
            std::make_unique<ValueRef::Constant<std::string>>("RULE_ANNEX_COST_MINIMUM"));
    }

    auto BuildingAnnexCostScaling() {
        return std::make_unique<ValueRef::ComplexVariable<double>>(
            "GameRule", nullptr, nullptr, nullptr,
            std::make_unique<ValueRef::Constant<std::string>>("RULE_BUILDING_ANNEX_COST_SCALING"));
    }

    auto BuildingCostsSum(std::unique_ptr<ValueRef::Variable<int>>&& for_empire_ref) {
        return std::make_unique<ValueRef::Statistic<double>>(
            std::make_unique<ValueRef::ComplexVariable<double>>(
                "BuildingTypeCost",                                                               // cost to produce buildings
                std::move(for_empire_ref),                                                        // by passed-in empire
                std::make_unique<ValueRef::Variable<int>>(
                    ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "PlanetID"),    // at the planet the building is already on
                nullptr,
                std::make_unique<ValueRef::Variable<std::string>>(
                    ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "BuildingType") // for each building's building type
            ),
            ValueRef::StatisticType::SUM,
            std::make_unique<Condition::And>(
                std::make_unique<Condition::Type>(UniverseObjectType::OBJ_BUILDING),
                std::make_unique<Condition::OnPlanet>(                  // for buildings on the planet being annexed
                    std::make_unique<ValueRef::Variable<int>>(
                        ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE, "ID")
                )
            )
        );
    }

    auto DefaultAnnexationCost() {
        return std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::MAXIMUM,
            MinimumAnnexCost(),
            (ScaledPop() * ExpBaseSpeciesOpinion() * ExpBaseStability()) + (BuildingAnnexCostScaling() * BuildingCostsSum(SourceOwner()))
        );
    }
}

Species::Species(std::string&& name, std::string&& desc,
                 std::string&& gameplay_desc, std::vector<FocusType>&& foci,
                 std::string&& default_focus,
                 std::map<PlanetType, PlanetEnvironment>&& planet_environments,
                 std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                 std::unique_ptr<Condition::Condition>&& combat_targets,
                 bool playable, bool native, bool can_colonize, bool can_produce_ships,
                 const std::set<std::string>& tags,
                 std::set<std::string>&& likes, std::set<std::string>&& dislikes,
                 std::unique_ptr<Condition::Condition>&& annexation_condition,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& annexation_cost,
                 std::string&& graphic,
                 double spawn_rate, int spawn_limit) :
    m_name(name), // not moving so available later in member initializer list
    m_description(std::move(desc)),
    m_gameplay_description(std::move(gameplay_desc)),
    m_foci(std::move(foci)),
    m_default_focus(std::move(default_focus)),
    m_planet_environments(planet_environments.begin(), planet_environments.end()),
    m_effects([](auto&& effects, const auto& name) {
        std::vector<Effect::EffectsGroup> retval;
        retval.reserve(effects.size());
        for (auto& e : effects) {
            e->SetTopLevelContent(name);
            retval.push_back(std::move(*e));
        }
        return retval;
    }(effects, name)),
    m_location(InitLocation(name)),
    m_combat_targets([](auto&& cond, const auto& name) {
        if (cond)
            cond->SetTopLevelContent(name);
        return std::move(cond);
    }(std::move(combat_targets), name)),
    m_annexation_condition([](auto&& annexation_condition, const auto& name) {
        if (!annexation_condition)
            annexation_condition = DefaultAnnexationCondition();
        annexation_condition->SetTopLevelContent(name);
        return std::move(annexation_condition);
    }(std::move(annexation_condition), name)),
    m_annexation_cost([](auto&& annexation_cost, const auto& name) {
        if (!annexation_cost)
            annexation_cost = DefaultAnnexationCost();
        annexation_cost->SetTopLevelContent(name);
        return std::move(annexation_cost);
    }(std::move(annexation_cost), name)),
    m_playable(playable),
    m_native(native),
    m_can_colonize(can_colonize),
    m_can_produce_ships(can_produce_ships),
    m_spawn_rate(spawn_rate),
    m_spawn_limit(spawn_limit),
    m_tags_concatenated(ConcatenateAsVector(tags, likes, dislikes)),
    m_tags(StringViewsForTags(tags, m_tags_concatenated.data())),
    m_pedia_tags(StringViewsForPediaTags(tags, m_tags_concatenated.data())),
    m_likes([&likes, this]() {
        std::vector<std::string_view> retval;
        retval.reserve(likes.size());

        const std::string_view sv{m_tags_concatenated.data(), m_tags_concatenated.size()};
        std::size_t next_idx = 0;
        // find starting point for first like, after end of tags, within m_tags_concatenated
        std::for_each(m_tags.begin(), m_tags.end(), [&next_idx](const auto& t) { next_idx += t.size(); });

        // store views into concatenated tags/likes string
        std::for_each(likes.begin(), likes.end(), [&next_idx, &retval, sv](const auto& t) {
            std::string upper_t = boost::to_upper_copy<std::string>(t);
            retval.push_back(sv.substr(next_idx, upper_t.size()));
            next_idx += upper_t.size();
        });

        return retval;
    }()),
    m_dislikes([&dislikes, this]() {
        std::vector<std::string_view> retval;
        retval.reserve(dislikes.size());

        const std::string_view sv{m_tags_concatenated.data(), m_tags_concatenated.size()};
        std::size_t next_idx = 0;
        // find starting point for first dislike, after end of tags and likes, within m_tags_concatenated
        std::for_each(m_tags.begin(), m_tags.end(), [&next_idx](const auto& t) { next_idx += t.size(); });
        std::for_each(m_likes.begin(), m_likes.end(), [&next_idx](const auto& t) { next_idx += t.size(); });

        // store views into concatenated tags/likes string
        std::for_each(dislikes.begin(), dislikes.end(), [&next_idx, &retval, sv](const auto& t) {
            std::string upper_t = boost::to_upper_copy<std::string>(t);
            retval.push_back(sv.substr(next_idx, upper_t.size()));
            next_idx += upper_t.size();
        });

        return retval;
    }()),
    m_graphic(std::move(graphic))
{}

Species::Species(std::string&& name, std::string&& desc,
                 std::string&& gameplay_desc, std::vector<FocusType>&& foci,
                 std::string&& default_focus,
                 std::map<PlanetType, PlanetEnvironment>&& planet_environments,
                 std::vector<std::shared_ptr<Effect::EffectsGroup>>&& effects,
                 std::unique_ptr<Condition::Condition>&& combat_targets,
                 bool playable, bool native, bool can_colonize, bool can_produce_ships,
                 const std::set<std::string>& tags,
                 std::set<std::string>&& likes, std::set<std::string>&& dislikes,
                 std::unique_ptr<Condition::Condition>&& annexation_condition,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& annexation_cost,
                 std::string&& graphic,
                 double spawn_rate, int spawn_limit) :
    Species(
        std::move(name), std::move(desc), std::move(gameplay_desc), std::move(foci), std::move(default_focus),
        std::move(planet_environments),
        [&effects]() {
            std::vector<std::unique_ptr<Effect::EffectsGroup>> retval;
            retval.reserve(effects.size());
            std::transform(effects.begin(), effects.end(), std::back_inserter(retval),
                           [](auto& e) {
                               Effect::EffectsGroup&& er = std::move(*e);
                               return std::make_unique<Effect::EffectsGroup>(std::move(er));
                           });
            return retval;
        }(),
        std::move(combat_targets), playable, native, can_colonize, can_produce_ships,
        tags, std::move(likes), std::move(dislikes),
        std::move(annexation_condition), std::move(annexation_cost),
        std::move(graphic), spawn_rate, spawn_limit)
{}

Species::~Species() = default;

bool Species::operator==(const Species& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_gameplay_description != rhs.m_gameplay_description ||
        m_foci != rhs.m_foci ||
        m_default_focus != rhs.m_default_focus ||
        m_planet_environments != rhs.m_planet_environments ||
        m_playable != rhs.m_playable ||
        m_native != rhs.m_native ||
        m_can_colonize != rhs.m_can_colonize ||
        m_can_produce_ships != rhs.m_can_produce_ships ||
        m_spawn_rate != rhs.m_spawn_rate ||
        m_spawn_limit != rhs.m_spawn_limit ||
        m_tags != rhs.m_tags ||
        m_likes != rhs.m_likes ||
        m_dislikes != rhs.m_dislikes ||
        m_graphic != rhs.m_graphic)
    { return false; }

    if (m_location == rhs.m_location) { // could be nullptr
        // check next member
    } else if (!m_location || !rhs.m_location) {
        return false;
    } else if (*m_location != *(rhs.m_location)) {
        return false;
    }

    if (m_combat_targets == rhs.m_combat_targets) { // could be nullptr
        // check next member
    } else if (!m_combat_targets || !rhs.m_combat_targets) {
        return false;
    } else if (*m_combat_targets != *(rhs.m_combat_targets)) {
        return false;
    }

    return m_effects == rhs.m_effects;
}

std::string Species::Dump(uint8_t ntabs) const {
    std::string retval;
    retval.reserve(500); // guesstimate
    retval += DumpIndent(ntabs) + "Species\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "gameplay_description = \"" + m_gameplay_description + "\"\n";
    if (m_playable)
        retval += DumpIndent(ntabs+1) + "Playable\n";
    if (m_native)
        retval += DumpIndent(ntabs+1) + "Native\n";
    if (m_can_produce_ships)
        retval += DumpIndent(ntabs+1) + "CanProduceShips\n";
    if (m_can_colonize)
        retval += DumpIndent(ntabs+1) + "CanColonize\n";
    if (m_foci.size() == 1) {
        retval += DumpIndent(ntabs+1) + "foci =\n";
        retval += m_foci.front().Dump(ntabs+1);
    } else {
        retval += DumpIndent(ntabs+1) + "foci = [\n";
        for (const FocusType& focus : m_foci)
            retval += focus.Dump(ntabs+2);
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "defaultfocus = \"" + m_default_focus + "\"\n";
    retval += DumpIndent(ntabs+1) + "likes = [";
    for (const auto& entry : m_likes) {
        retval += "\"";
        retval += entry;
        retval += "\" ";
    }
    retval += "]\n";
    retval += DumpIndent(ntabs+1) + "dislikes = [";
    for (const auto& entry : m_dislikes) {
        retval += "\"";
        retval += entry;
        retval += "\" ";
    }
    retval += "]\n";
    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
        retval += m_effects.front().Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects)
            retval += effect.Dump(ntabs+2);
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    if (m_combat_targets)
        retval += DumpIndent(ntabs+1) + "combatTargets = " + m_combat_targets->Dump(ntabs+2);
    if (m_planet_environments.size() == 1) {
        retval += DumpIndent(ntabs+1) + "environments =\n";
        retval.append(DumpIndent(ntabs+2))
              .append("type = ")
              .append(ValueRef::PlanetTypeToString(m_planet_environments.begin()->first))
              .append(" environment = ")
              .append(ValueRef::PlanetEnvironmentToString(m_planet_environments.begin()->second))
              .append("\n");
    } else {
        retval += DumpIndent(ntabs+1) + "environments = [\n";
        for (const auto& entry : m_planet_environments) {
            retval.append(DumpIndent(ntabs+2))
                  .append("type = ")
                  .append(ValueRef::PlanetTypeToString(entry.first))
                  .append(" environment = ")
                  .append(ValueRef::PlanetEnvironmentToString(entry.second))
                  .append("\n");
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    retval += DumpIndent(ntabs+1) + "spawnrate = " + std::to_string(m_spawn_rate) + "\n"
           +  DumpIndent(ntabs+1) + "spawnlimit = " + std::to_string(m_spawn_limit) + "\n"
           +  DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

std::string Species::GameplayDescription() const {
    std::stringstream result;

    result << UserString(m_gameplay_description);

    bool requires_separator = true;

    for (auto& effect : m_effects) {
        const std::string& description = effect.GetDescription();
        if (description.empty())
            continue;

        if (requires_separator) {
            result << "\n";
            requires_separator = false;
        }

        result << UserString(description) << "\n";
    }

    return result.str();
}

PlanetEnvironment Species::GetPlanetEnvironment(PlanetType planet_type) const {
    auto it = m_planet_environments.find(planet_type);
    if (it == m_planet_environments.end())
        return PlanetEnvironment::PE_UNINHABITABLE;
    else
        return it->second;
}

namespace {
    constexpr PlanetType RingNextPlanetType(PlanetType current_type) noexcept {
        PlanetType next(PlanetType(int(current_type)+1));
        if (next >= PlanetType::PT_ASTEROIDS)
            next = PlanetType::PT_SWAMP;
        return next;
    }
    constexpr PlanetType RingPreviousPlanetType(PlanetType current_type) noexcept {
        PlanetType next(PlanetType(int(current_type)-1));
        if (next <= PlanetType::INVALID_PLANET_TYPE)
            next = PlanetType::PT_OCEAN;
        return next;
    }
}

template <typename Func>
PlanetType Species::TheNextBestPlanetTypeApply(PlanetType initial_planet_type,
                                               Func apply_for_best_forward_backward) const
{
    // some types can't be terraformed
    if (initial_planet_type == PlanetType::PT_GASGIANT)
        return PlanetType::PT_GASGIANT;
    if (initial_planet_type == PlanetType::PT_ASTEROIDS)
        return PlanetType::PT_ASTEROIDS;
    if (initial_planet_type == PlanetType::INVALID_PLANET_TYPE)
        return PlanetType::INVALID_PLANET_TYPE;
    if (initial_planet_type == PlanetType::NUM_PLANET_TYPES)
        return PlanetType::NUM_PLANET_TYPES;
    // and sometimes there's no variation data
    if (m_planet_environments.empty())
        return initial_planet_type;

    // determine which environment rating is the best available for this species,
    // excluding gas giants and asteroids
    PlanetEnvironment best_environment = PlanetEnvironment::PE_UNINHABITABLE;
    //std::set<PlanetType> best_types;
    for (const auto& [pt, pe] : m_planet_environments) {
        if (pt < PlanetType::PT_ASTEROIDS) {
            if (pe == best_environment) {
                //best_types.insert(pt);
            } else if (pe > best_environment) {
                best_environment = pe;
                //best_types.clear();
                //best_types.insert(pt);
            }
        }
    }

    // if no improvement available, abort early
    PlanetEnvironment initial_environment = GetPlanetEnvironment(initial_planet_type);
    if (initial_environment >= best_environment)
        return initial_planet_type;

    int forward_steps_to_best = 0;
    PlanetType next_best_planet_type = initial_planet_type;
    for (PlanetType type = RingNextPlanetType(initial_planet_type);
         type != initial_planet_type; type = RingNextPlanetType(type))
    {
        forward_steps_to_best++;
        if (GetPlanetEnvironment(type) == best_environment) {
            next_best_planet_type = type;
            break;
        }
    }
    int backward_steps_to_best = 0;
    for (PlanetType type = RingPreviousPlanetType(initial_planet_type);
         type != initial_planet_type; type = RingPreviousPlanetType(type))
    {
        backward_steps_to_best++;
        if (GetPlanetEnvironment(type) == best_environment) {
            if (backward_steps_to_best < forward_steps_to_best)
                next_best_planet_type = type;
            break;
        }
    }

    return apply_for_best_forward_backward(next_best_planet_type, forward_steps_to_best, backward_steps_to_best);
}

PlanetType Species::NextBestPlanetType(PlanetType initial_planet_type) const {
    return TheNextBestPlanetTypeApply(initial_planet_type,
                                      [](PlanetType best_planet_type, int forward_steps_to_best, int backward_steps_to_best)
                                      { return best_planet_type; });
}

PlanetType Species::NextBetterPlanetType(PlanetType initial_planet_type) const {
    return TheNextBestPlanetTypeApply(initial_planet_type,
        [initial_planet_type](PlanetType best_planet_type, int forward_steps_to_best, int backward_steps_to_best)
    {
        if (forward_steps_to_best <= backward_steps_to_best)
            return RingNextPlanetType(initial_planet_type);
        else
            return RingPreviousPlanetType(initial_planet_type);
    });
}

uint32_t Species::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_gameplay_description);
    // opinions and homeworlds are per-game specific, so not included in checksum
    CheckSums::CheckSumCombine(retval, m_foci);
    CheckSums::CheckSumCombine(retval, m_default_focus);
    CheckSums::CheckSumCombine(retval, m_likes);
    CheckSums::CheckSumCombine(retval, m_dislikes);
    CheckSums::CheckSumCombine(retval, m_planet_environments);
    CheckSums::CheckSumCombine(retval, m_combat_targets);
    CheckSums::CheckSumCombine(retval, m_annexation_condition);
    CheckSums::CheckSumCombine(retval, m_annexation_cost);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_playable);
    CheckSums::CheckSumCombine(retval, m_native);
    CheckSums::CheckSumCombine(retval, m_can_colonize);
    CheckSums::CheckSumCombine(retval, m_can_produce_ships);
    CheckSums::CheckSumCombine(retval, m_spawn_rate);
    CheckSums::CheckSumCombine(retval, m_spawn_limit);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

/////////////////////////////////////////////////
// SpeciesManager                              //
/////////////////////////////////////////////////
SpeciesManager& SpeciesManager::operator=(SpeciesManager&& rhs) noexcept {
    // intentionally not moving m_pending_types, m_species, or m_census_order
    // as these are parsed once

    m_species_homeworlds = std::move(rhs.m_species_homeworlds);
    m_species_empire_opinions = std::move(rhs.m_species_empire_opinions);
    m_species_species_opinions = std::move(rhs.m_species_species_opinions);
    m_species_species_ships_destroyed = std::move(rhs.m_species_species_ships_destroyed);

    return *this;
}

const Species* SpeciesManager::GetSpecies(std::string_view name) const {
    CheckPendingSpeciesTypes();
    const auto it = m_species.find(name);
    return it != m_species.end() ? &(it->second) : nullptr;
}

const Species* SpeciesManager::GetSpeciesUnchecked(std::string_view name) const {
    const auto it = m_species.find(name);
    return it != m_species.end() ? &(it->second) : nullptr;
}

void SpeciesManager::SetSpeciesTypes(Pending::Pending<std::pair<std::map<std::string, Species>, CensusOrder>>&& future) {
    std::scoped_lock lock(m_species_mutex);
    m_pending_types = std::move(future);
}

void SpeciesManager::CheckPendingSpeciesTypes() const {
    std::scoped_lock lock(m_species_mutex);

    if (!m_pending_types) {
        if (m_species.empty())
            ErrorLogger() << "CheckPendingSpeciesTypes() has no pending species but also no already-parsed species";
        return;
    }

    decltype(m_pending_types)::value_type::result_type container;
    Pending::SwapPending(m_pending_types, container); 

    m_species.clear();
    m_species.insert(std::make_move_iterator(container.first.begin()),
                     std::make_move_iterator(container.first.end()));
    m_census_order = std::move(container.second);
}

SpeciesManager::iterator SpeciesManager::begin() const {
    CheckPendingSpeciesTypes();
    return m_species.begin();
}

SpeciesManager::iterator SpeciesManager::end() const {
    CheckPendingSpeciesTypes();
    return m_species.end();
}

SpeciesManager::playable_iterator SpeciesManager::playable_begin() const
{ return playable_iterator(PlayableSpecies(), begin(), end()); }

SpeciesManager::playable_iterator SpeciesManager::playable_end() const
{ return playable_iterator(PlayableSpecies(), end(), end()); }

SpeciesManager::native_iterator SpeciesManager::native_begin() const
{ return native_iterator(NativeSpecies(), begin(), end()); }

SpeciesManager::native_iterator SpeciesManager::native_end() const
{ return native_iterator(NativeSpecies(), end(), end()); }

const SpeciesManager::CensusOrder& SpeciesManager::census_order() const {
    CheckPendingSpeciesTypes();
    return m_census_order;
}

bool SpeciesManager::empty() const {
    CheckPendingSpeciesTypes();
    return m_species.empty();
}

int SpeciesManager::NumSpecies() const {
    CheckPendingSpeciesTypes();
    return m_species.size();
}

int SpeciesManager::NumPlayableSpecies() const
{ return std::distance(playable_begin(), playable_end()); }

int SpeciesManager::NumNativeSpecies() const
{ return std::distance(native_begin(), native_end()); }

namespace {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif
}

const std::string& SpeciesManager::RandomSpeciesName() const {
    CheckPendingSpeciesTypes();
    if (m_species.empty())
        return EMPTY_STRING;

    int species_idx = RandInt(0, static_cast<int>(m_species.size()) - 1);
    return std::next(begin(), species_idx)->first;
}

const std::string& SpeciesManager::RandomPlayableSpeciesName() const {
    if (NumPlayableSpecies() <= 0)
        return EMPTY_STRING;

    int species_idx = RandInt(0, NumPlayableSpecies() - 1);
    return std::next(playable_begin(), species_idx)->first;
}

const std::string& SpeciesManager::SequentialPlayableSpeciesName(int id) const {
    if (NumPlayableSpecies() <= 0)
        return EMPTY_STRING;

    int species_idx = id % NumPlayableSpecies();
    DebugLogger() << "SpeciesManager::SequentialPlayableSpeciesName has " << NumPlayableSpecies() << " and is given id " << id << " yielding index " << species_idx;
    return std::next(playable_begin(), species_idx)->first;
}

void SpeciesManager::SetSpeciesHomeworlds(std::map<std::string, std::set<int>>&& species_homeworld_ids) {
    m_species_homeworlds.clear();
    using homeworlds_value_t = decltype(m_species_homeworlds)::value_type;
    std::transform(species_homeworld_ids.begin(), species_homeworld_ids.end(),
                   std::inserter(m_species_homeworlds, m_species_homeworlds.end()),
                   [](auto& sp_ids) -> homeworlds_value_t
                   { return {sp_ids.first, {sp_ids.second.begin(), sp_ids.second.end()}}; });
}

void SpeciesManager::SetSpeciesSpeciesOpinion(const std::string& opinionated_species,
                                              const std::string& rated_species, float opinion, bool target)
{
    auto& [opinion_meter, target_meter] = m_species_species_opinions[opinionated_species][rated_species];
    if (target)
        target_meter.SetCurrent(opinion);
    else
        opinion_meter.SetCurrent(opinion);
}

void SpeciesManager::SetSpeciesEmpireOpinion(const std::string& opinionated_species,
                                             int empire_id, float opinion, bool target)
{
    auto& [opinion_meter, target_meter] = m_species_empire_opinions[opinionated_species][empire_id];
    if (target)
        target_meter.SetCurrent(opinion);
    else
        opinion_meter.SetCurrent(opinion);
}

void SpeciesManager::ResetSpeciesOpinions(bool active, bool target) {
    for (auto& [species, empire_ops] : m_species_empire_opinions) {
        for (auto& [empire_id, ops] : empire_ops) {
            if (active)
                ops.first.SetCurrent(ops.first.Initial());
            if (target)
                ops.second.ResetCurrent();
        }
    }

    for (auto& [opinionated_species, species_ops] : m_species_species_opinions) {
        for (auto& [about_species, ops] : species_ops) {
            if (active)
                ops.second.SetCurrent(ops.second.Initial());
            if (target)
                ops.second.ResetCurrent();
        }
    }
}

void SpeciesManager::BackPropagateOpinions() {
    for (auto& empire_ops : m_species_empire_opinions | range_values) {
        for (auto& [op, target_op] : empire_ops | range_values) {
            op.BackPropagate();
            target_op.BackPropagate();
        }
    }

    for (auto& species_ops : m_species_species_opinions | range_values) {
        for (auto& [op, target_op] : species_ops | range_values) {
            op.BackPropagate();
            target_op.BackPropagate();
        }
    }
}

float SpeciesManager::SpeciesEmpireOpinion(const std::string& species_name, int empire_id,
                                           bool target, bool current) const
{
    const auto sp_it = m_species_empire_opinions.find(species_name);
    if (sp_it == m_species_empire_opinions.end())
        return 0.0f;
    const auto& emp_map = sp_it->second;
    const auto emp_it = emp_map.find(empire_id);
    if (emp_it == emp_map.end())
        return 0.0f;
    TraceLogger() << "SpeciesEmpireOpinion " << species_name << ", " << empire_id
                  << ": " << emp_it->second.first.Dump().data() << " / " << emp_it->second.second.Dump().data();
    const auto& meter = target ? emp_it->second.second : emp_it->second.first;
    return current ? meter.Current() : meter.Initial();
}

float SpeciesManager::SpeciesSpeciesOpinion(const std::string& opinionated_species_name,
                                            const std::string& rated_species_name,
                                            bool target, bool current) const
{
    const auto sp_it = m_species_species_opinions.find(opinionated_species_name);
    if (sp_it == m_species_species_opinions.end())
        return 0.0f;
    const auto& ra_sp_map = sp_it->second;
    const auto ra_sp_it = ra_sp_map.find(rated_species_name);
    if (ra_sp_it == ra_sp_map.end())
        return 0.0f;
    const auto& meter = target ? ra_sp_it->second.second : ra_sp_it->second.first;
    return current ? meter.Current() : meter.Initial();
}

std::vector<std::string_view> SpeciesManager::SpeciesThatLike(std::string_view content_name) const {
    CheckPendingSpeciesTypes();
    std::vector<std::string_view> retval;
    retval.reserve(m_species.size());
    std::for_each(m_species.begin(), m_species.end(), [&retval, content_name](const auto& s) {
        const auto& likes = s.second.Likes();
        if (std::any_of(likes.begin(), likes.end(), [content_name](const auto& l) { return l == content_name; }))
            retval.emplace_back(s.first);
    });
    return retval;
}

std::vector<std::string_view> SpeciesManager::SpeciesThatDislike(std::string_view content_name) const {
    CheckPendingSpeciesTypes();
    std::vector<std::string_view> retval;
    retval.reserve(m_species.size());
    std::for_each(m_species.begin(), m_species.end(), [&retval, content_name](const auto& s) {
        const auto& dislikes = s.second.Dislikes();
        if (std::any_of(dislikes.begin(), dislikes.end(), [content_name](const auto& l) { return l == content_name; }))
            retval.emplace_back(s.first);
    });
    return retval;
}

void SpeciesManager::AddSpeciesHomeworld(std::string species, int homeworld_id) {
    if (homeworld_id == INVALID_OBJECT_ID)
        return;
    if (species.empty())
        return;
    m_species_homeworlds[std::move(species)].insert(homeworld_id);
}

void SpeciesManager::RemoveSpeciesHomeworld(const std::string& species, int homeworld_id) {
    if (homeworld_id == INVALID_OBJECT_ID)
        return;
    if (species.empty())
        return;
    auto it = m_species_homeworlds.find(species);
    if (it != m_species_homeworlds.end())
        it->second.erase(homeworld_id);
}

void SpeciesManager::ClearSpeciesHomeworlds()
{ m_species_homeworlds.clear(); }

void SpeciesManager::SetSpeciesShipsDestroyed(std::map<std::string, std::map<std::string, int>> ssd) {
    m_species_species_ships_destroyed.clear();
    using opinions_value_t = decltype(m_species_species_ships_destroyed)::value_type;
    std::transform(ssd.begin(), ssd.end(),
                   std::inserter(m_species_species_ships_destroyed, m_species_species_ships_destroyed.end()),
                   [](auto& sp_objs) -> opinions_value_t
                   { return {sp_objs.first, {sp_objs.second.begin(), sp_objs.second.end()}}; });
}

uint32_t SpeciesManager::GetCheckSum() const {
    CheckPendingSpeciesTypes();
    uint32_t retval{0};
    for (auto const& name_type_pair : m_species)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_species.size());

    DebugLogger() << "SpeciesManager checksum: " << retval;
    return retval;
}
