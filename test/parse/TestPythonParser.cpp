#include <boost/test/unit_test.hpp>

#include "parse/Parse.h"
#include "parse/PythonParser.h"
#include "universe/BuildingType.h"
#include "universe/Conditions.h"
#include "universe/Effects.h"
#include "universe/Planet.h"
#include "universe/Tech.h"
#include "universe/UnlockableItem.h"
#include "universe/ValueRefs.h"
#include "universe/NamedValueRefManager.h"
#include "universe/System.h"
#include "util/i18n.h"
#include "util/CheckSums.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/Pending.h"
#include "util/PythonCommon.h"
#include "util/VarText.h"

#include "ParserAppFixture.h"

namespace {
    template <typename T, size_t N>
    inline std::vector<std::unique_ptr<T>> array_to_vector(std::array<std::unique_ptr<T>, N>&& a)
    { return {std::make_move_iterator(a.begin()), std::make_move_iterator(a.end())}; }

    template <typename T, size_t N>
    inline std::vector<std::pair<std::string, std::unique_ptr<T>>> pair_array_to_vector(
        std::array<std::pair<std::string, std::unique_ptr<T>>, N>&& a)
    { return {std::make_move_iterator(a.begin()), std::make_move_iterator(a.end())}; }

    template <typename T, size_t N>
    inline std::vector<std::pair<std::string, std::unique_ptr<T>>> pair_array_to_vector(
        std::array<std::pair<std::string_view, std::unique_ptr<T>>, N>&& a)
    {
        std::vector<std::pair<std::string, std::unique_ptr<T>>> retval;
        retval.reserve(a.size());
        std::transform(std::make_move_iterator(a.begin()), std::make_move_iterator(a.end()),
                       std::back_inserter(retval),
                       [](auto&& p) { return std::pair{std::string(p.first), std::move(p.second)}; });
        return retval;
    }
}

BOOST_FIXTURE_TEST_SUITE(TestPythonParser, ParserAppFixture)

BOOST_AUTO_TEST_CASE(parse_game_rules) {
    PythonParser parser(m_python, m_test_scripting_dir);

    auto game_rules_p = Pending::ParseSynchronously(parse::game_rules, parser,  m_test_scripting_dir / "game_rules.focs.py");
    auto game_rules = *Pending::WaitForPendingUnlocked(std::move(game_rules_p));
    BOOST_REQUIRE(!game_rules.empty());
    BOOST_REQUIRE(game_rules.contains("RULE_HABITABLE_SIZE_MEDIUM"));
    BOOST_REQUIRE(GameRule::Type::TOGGLE == game_rules.at("RULE_ENABLE_ALLIED_REPAIR").type);
}

BOOST_AUTO_TEST_CASE(parse_techs) {
    PythonParser parser(m_python, m_test_scripting_dir);

    auto techs_p = Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, parser, m_test_scripting_dir / "techs");
    auto [techs, tech_categories, categories_seen] = *Pending::WaitForPendingUnlocked(std::move(techs_p));
    BOOST_REQUIRE(!tech_categories.empty());

    BOOST_REQUIRE(tech_categories.contains("PRODUCTION_CATEGORY"));
    BOOST_REQUIRE(tech_categories.contains("GROWTH_CATEGORY"));

    const auto cat_it = tech_categories.find("CONSTRUCTION_CATEGORY");
    BOOST_REQUIRE(tech_categories.end() != cat_it);

    BOOST_REQUIRE_EQUAL("CONSTRUCTION_CATEGORY", cat_it->second.name);
    BOOST_REQUIRE_EQUAL("construction.png", cat_it->second.graphic);
    static constexpr std::array<uint8_t, 4> test_colour{241, 233, 87, 255};
    BOOST_REQUIRE(test_colour == cat_it->second.colour);

    BOOST_REQUIRE(!techs.empty());
    BOOST_REQUIRE(!categories_seen.empty());

    BOOST_REQUIRE(!categories_seen.contains("PRODUCTION_CATEGORY"));
    BOOST_REQUIRE(categories_seen.contains("GROWTH_CATEGORY"));

    {
        const auto tech_it = techs.find("LRN_ALGO_ELEGANCE");
        BOOST_REQUIRE(techs.end() != tech_it);
        BOOST_REQUIRE_EQUAL("LRN_ALGO_ELEGANCE", tech_it->second.Name());
        BOOST_REQUIRE_EQUAL("LRN_ALGO_ELEGANCE_DESC", tech_it->second.Description());
        BOOST_REQUIRE_EQUAL("RESEARCH_SHORT_DESC", tech_it->second.ShortDescription());
        BOOST_REQUIRE_EQUAL("LEARNING_CATEGORY", tech_it->second.Category());
        BOOST_REQUIRE_EQUAL(true, tech_it->second.Researchable());
        BOOST_REQUIRE_EQUAL("icons/tech/algorithmic_elegance.png", tech_it->second.Graphic());

        BOOST_REQUIRE_CLOSE(0.0, tech_it->second.ResearchCost(ALL_EMPIRES, m_context), 0.1);
        BOOST_REQUIRE_EQUAL(3, tech_it->second.ResearchTime(ALL_EMPIRES, m_context));
        BOOST_REQUIRE_EQUAL(0, tech_it->second.Effects().size());
        BOOST_REQUIRE_EQUAL(0, tech_it->second.UnlockedTechs().size());
        BOOST_REQUIRE_EQUAL(0, tech_it->second.Prerequisites().size());

        const auto& tech_tags = tech_it->second.Tags();
        BOOST_REQUIRE_EQUAL(1, tech_tags.size());
        BOOST_REQUIRE(tech_it->second.HasTag("PEDIA_LEARNING_CATEGORY"));

        const auto& tech_items = tech_it->second.UnlockedItems();
        BOOST_REQUIRE_EQUAL(1, tech_items.size());
        BOOST_REQUIRE_EQUAL(UnlockableItemType::UIT_POLICY, tech_items[0].type);
        BOOST_REQUIRE_EQUAL("PLC_ALGORITHMIC_RESEARCH", tech_items[0].name);

        const Tech tech{
            "LRN_ALGO_ELEGANCE",
            "LRN_ALGO_ELEGANCE_DESC",
            "RESEARCH_SHORT_DESC",
            "LEARNING_CATEGORY",
            std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
                std::make_unique<ValueRef::Constant<double>>(10.0),
                std::make_unique<ValueRef::ComplexVariable<double>>(
                    "GameRule",
                    nullptr,
                    nullptr,
                    nullptr,
                    std::make_unique<ValueRef::Constant<std::string>>("RULE_TECH_COST_FACTOR"),
                    nullptr
                )),
            std::make_unique<ValueRef::Constant<int>>(3),
            true,
            {"PEDIA_LEARNING_CATEGORY"},
            std::vector<Effect::EffectsGroup>{},
            {},
            {UnlockableItem{UnlockableItemType::UIT_POLICY, "PLC_ALGORITHMIC_RESEARCH"}},
            "icons/tech/algorithmic_elegance.png"
        };
        BOOST_REQUIRE(tech == tech_it->second);
    }

    {
        const auto tech_it = techs.find("CON_ORBITAL_HAB");
        BOOST_REQUIRE(techs.end() != tech_it);

        BOOST_REQUIRE_EQUAL(1, tech_it->second.Effects().size());
        const auto& tech_effect_gr = tech_it->second.Effects().front();
        BOOST_REQUIRE_EQUAL("", tech_effect_gr.StackingGroup());
        BOOST_REQUIRE_EQUAL("", tech_effect_gr.GetDescription());
        BOOST_REQUIRE_EQUAL("", tech_effect_gr.AccountingLabel());
        BOOST_REQUIRE_EQUAL(17, tech_effect_gr.Priority());

        // scope
        const auto tech_effect_scope = tech_effect_gr.Scope();
        BOOST_REQUIRE_EQUAL(true, tech_effect_scope->RootCandidateInvariant());
        BOOST_REQUIRE_EQUAL(false, tech_effect_scope->LocalCandidateInvariant());
        BOOST_REQUIRE_EQUAL(true, tech_effect_scope->TargetInvariant());
        BOOST_REQUIRE_EQUAL(false, tech_effect_scope->SourceInvariant());

        const auto tech_effect_scope_and = dynamic_cast<Condition::And*>(tech_effect_scope);
        BOOST_REQUIRE(tech_effect_scope_and != nullptr);
        BOOST_REQUIRE_EQUAL(2, tech_effect_scope_and->Operands().size());


        // effects list
        BOOST_REQUIRE_EQUAL(1, tech_effect_gr.Effects().size());
        const auto& tech_effect = tech_effect_gr.Effects().front();
        BOOST_REQUIRE_EQUAL(true, tech_effect->IsMeterEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsEmpireMeterEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsAppearanceEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsSitrepEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsConditionalEffect());

        const auto& prereqs = tech_it->second.Prerequisites();
        BOOST_REQUIRE_EQUAL(1, prereqs.size());
        BOOST_REQUIRE_EQUAL(1, std::count(prereqs.begin(), prereqs.end(), "PRO_MICROGRAV_MAN"));

        static constexpr auto create_effect = []() {
            return std::make_unique<Effect::SetMeter>(
                MeterType::METER_TARGET_POPULATION,
                std::make_unique<ValueRef::Operation<double>>(
                    ValueRef::OpType::PLUS,
                    std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
                    std::make_unique<ValueRef::Operation<double>>(
                        ValueRef::OpType::TIMES,
                        std::make_unique<ValueRef::Constant<double>>(1.0),
                        std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "HabitableSize"))
                ),
                "ORBITAL_HAB_LABEL");
        };

        static constexpr auto create_eg = []() {
            std::vector<std::unique_ptr<Effect::Effect>> effects;
            effects.push_back(create_effect());
            return std::make_shared<Effect::EffectsGroup>(
                std::make_unique<Condition::And>(
                    std::make_unique<Condition::Species>(),
                    std::make_unique<Condition::EmpireAffiliation>(
                        std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner")
                    )
                ),
                nullptr, std::move(effects), "", "", 17, "", ""
            );
        };

        const Tech tech{
            "CON_ORBITAL_HAB",
            "CON_ORBITAL_HAB_DESC",
            "POPULATION_SHORT_DESC",
            "GROWTH_CATEGORY",
            std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
                std::make_unique<ValueRef::Constant<double>>(250.0),
                std::make_unique<ValueRef::ComplexVariable<double>>(
                    "GameRule",
                    nullptr,
                    nullptr,
                    nullptr,
                    std::make_unique<ValueRef::Constant<std::string>>("RULE_TECH_COST_FACTOR"),
                    nullptr
                )),
            std::make_unique<ValueRef::Constant<int>>(7),
            true,
            {"PEDIA_GROWTH_CATEGORY"},
            {create_eg()},
            {"PRO_MICROGRAV_MAN"},
            {},
            "icons/tech/orbital_gardens.png"
        };
#if defined(FREEORION_MACOSX)
        BOOST_WARN(tech == tech_it->second);
#else
        BOOST_REQUIRE(tech.Name() == tech_it->second.Name());
        BOOST_REQUIRE(tech.Description() == tech_it->second.Description());
        BOOST_REQUIRE(tech.ShortDescription() == tech_it->second.ShortDescription());
        BOOST_REQUIRE(tech.Category() == tech_it->second.Category());
        BOOST_REQUIRE(tech.Researchable() == tech_it->second.Researchable());
        BOOST_REQUIRE(tech.Tags() == tech_it->second.Tags());
        BOOST_REQUIRE(tech.PediaTags() == tech_it->second.PediaTags());
        BOOST_REQUIRE(tech.HasTag("PEDIA_GROWTH_CATEGORY") == tech_it->second.HasTag("PEDIA_GROWTH_CATEGORY"));
        BOOST_REQUIRE(tech.HasTag("NOT_A_TAG") == tech_it->second.HasTag("NOT_A_TAG"));
        BOOST_REQUIRE(tech.Effects().size() == tech_it->second.Effects().size());
        BOOST_REQUIRE(tech.Prerequisites() == tech_it->second.Prerequisites());
        BOOST_REQUIRE(tech.Graphic() == tech_it->second.Graphic());
        BOOST_REQUIRE(tech.UnlockedItems() == tech_it->second.UnlockedItems());
        BOOST_REQUIRE(tech.ResearchCostRef() && tech_it->second.ResearchCostRef() && *tech.ResearchCostRef() == *tech_it->second.ResearchCostRef());
        BOOST_REQUIRE(tech.ResearchTurnsRef() && tech_it->second.ResearchTurnsRef() && *tech.ResearchTurnsRef() == *tech_it->second.ResearchTurnsRef());
        BOOST_REQUIRE(tech.UnlockedTechs() == tech_it->second.UnlockedTechs());
        BOOST_REQUIRE(tech.Dump() == tech_it->second.Dump());
        BOOST_REQUIRE(tech == tech_it->second);
#endif
    }

    // test it last
    BOOST_REQUIRE_EQUAL(2, techs.size());
    BOOST_REQUIRE_EQUAL(9, tech_categories.size());
    BOOST_REQUIRE_EQUAL(2, categories_seen.size());
}

BOOST_AUTO_TEST_CASE(parse_species) {
    PythonParser parser(m_python, m_test_scripting_dir);

    auto species_p = Pending::ParseSynchronously(parse::species, parser, m_test_scripting_dir / "species");
    const auto [species_map, ordering] = *Pending::WaitForPendingUnlocked(std::move(species_p));

    BOOST_REQUIRE(!ordering.empty());
    BOOST_CHECK(!species_map.empty());

    BOOST_REQUIRE_EQUAL("LITHIC", ordering[0]);
    BOOST_REQUIRE_EQUAL("ORGANIC", ordering[1]);
    BOOST_REQUIRE_EQUAL("GASEOUS", ordering[6]);

    {
        const auto species_it = species_map.find("SP_ABADDONI");
        BOOST_REQUIRE(species_it != species_map.end());

        auto& species = species_it->second;
        BOOST_REQUIRE_EQUAL("SP_ABADDONI", species.Name());
        BOOST_REQUIRE_EQUAL("SP_ABADDONI_DESC", species.Description());
        // BOOST_REQUIRE_EQUAL("SP_ABADDONI_GAMEPLAY_DESC", species.GameplayDescription()); // already resolved to user string

        BOOST_REQUIRE(species.Location() != nullptr);
        BOOST_REQUIRE(species.CombatTargets() == nullptr);

        BOOST_REQUIRE_EQUAL(14, species.Foci().size());
        BOOST_REQUIRE_EQUAL("FOCUS_INDUSTRY", species.Foci()[0].Name());
        BOOST_REQUIRE_EQUAL("FOCUS_DOMINATION", species.Foci()[13].Name());
        BOOST_CHECK_EQUAL("FOCUS_INDUSTRY", species.DefaultFocus());

        BOOST_CHECK_EQUAL(11, species.PlanetEnvironments().size());
        BOOST_CHECK_EQUAL(PlanetEnvironment::PE_POOR, species.GetPlanetEnvironment(PlanetType::PT_BARREN));
        BOOST_CHECK_EQUAL(PlanetEnvironment::PE_ADEQUATE, species.GetPlanetEnvironment(PlanetType::PT_TOXIC));

        BOOST_REQUIRE_EQUAL(1.0, species.SpawnRate());
        BOOST_REQUIRE_EQUAL(9999, species.SpawnLimit());
        BOOST_CHECK_EQUAL(true, species.Playable());
        BOOST_CHECK_EQUAL(false, species.Native());
        BOOST_CHECK_EQUAL(true, species.CanColonize());
        BOOST_CHECK_EQUAL(true, species.CanProduceShips());

        BOOST_REQUIRE_EQUAL(6, species.Tags().size());
        BOOST_CHECK_EQUAL("AVERAGE_SUPPLY", species.Tags()[0]);
        BOOST_CHECK_EQUAL("PEDIA_LITHIC_SPECIES_CLASS", species.Tags()[5]);

        BOOST_CHECK_EQUAL(12, species.Likes().size());

        BOOST_CHECK_EQUAL(13, species.Dislikes().size());

        BOOST_CHECK_EQUAL("icons/species/abaddonnian.png", species.Graphic());

        BOOST_REQUIRE_EQUAL(98, species.Effects().size());
        const auto& effect_group = species.Effects().front();
        BOOST_REQUIRE_EQUAL("", effect_group.StackingGroup());
        BOOST_REQUIRE_EQUAL("", effect_group.GetDescription());
        BOOST_REQUIRE_EQUAL("FOCUS_INDUSTRY_LABEL", effect_group.AccountingLabel());
        BOOST_REQUIRE_EQUAL(98, effect_group.Priority());

        BOOST_REQUIRE_EQUAL(1693, effect_group.Scope()->GetCheckSum());
        BOOST_REQUIRE_EQUAL(18072, effect_group.Activation()->GetCheckSum());
        BOOST_REQUIRE_EQUAL("", effect_group.GetDescription());

        BOOST_REQUIRE_EQUAL(true, effect_group.HasMeterEffects());
        BOOST_REQUIRE_EQUAL(false, effect_group.HasAppearanceEffects());
        BOOST_REQUIRE_EQUAL(false, effect_group.HasSitrepEffects());
        BOOST_REQUIRE_EQUAL(1, effect_group.Effects().size());
        BOOST_REQUIRE_EQUAL(13902, effect_group.Effects()[0]->GetCheckSum());
        BOOST_REQUIRE_EQUAL(36595, effect_group.GetCheckSum());

        BOOST_REQUIRE_NE(nullptr, effect_group.Scope());
        BOOST_REQUIRE_NE(nullptr, effect_group.Activation());

        BOOST_REQUIRE_EQUAL(1, effect_group.Effects().size());
        const auto& effect = effect_group.Effects().front();
        BOOST_REQUIRE_EQUAL(true, effect->IsMeterEffect());
        BOOST_REQUIRE_EQUAL(false, effect->IsEmpireMeterEffect());
        BOOST_REQUIRE_EQUAL(false, effect->IsAppearanceEffect());
        BOOST_REQUIRE_EQUAL(false, effect->IsSitrepEffect());
        BOOST_REQUIRE_EQUAL(false, effect->IsConditionalEffect());

        BOOST_TEST_MESSAGE("Dump " << species.Name() << ":");
        BOOST_TEST_MESSAGE(species.Dump(0));

        BOOST_REQUIRE_EQUAL(6764239, species.GetCheckSum());

        const Species test_species{"SP_ABADDONI",
            "SP_ABADDONI_DESC",
            "SP_ABADDONI_GAMEPLAY_DESC",
            {
                {"FOCUS_INDUSTRY", "FOCUS_INDUSTRY_DESC", std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET), "icons/focus/industry.png"},
                {"FOCUS_RESEARCH", "FOCUS_RESEARCH_DESC", std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET), "icons/focus/research.png"},
                {"FOCUS_INFLUENCE", "FOCUS_INFLUENCE_DESC", std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET), "icons/focus/influence.png"},
                {"FOCUS_GROWTH", "FOCUS_GROWTH_DESC", std::make_unique<Condition::Or>(array_to_vector<Condition::Condition, 10>({
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Homeworld>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Variable<std::string>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Species")})),
                        std::make_unique<Condition::Not>(std::make_unique<Condition::HasTag>("SELF_SUSTAINING"))
                    ),
                    std::make_unique<Condition::HasSpecial>("POSITRONIUM_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("SUPERCONDUCTOR_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("MONOPOLE_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("SPICE_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("FRUIT_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("PROBIOTIC_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("ELERIUM_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("CRYSTALS_SPECIAL"),
                    std::make_unique<Condition::HasSpecial>("MINERALS_SPECIAL")
                })), "icons/focus/growth.png"},
                {"FOCUS_PROTECTION", "FOCUS_PROTECTION_DESC", std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET), "icons/focus/protection.png"},
                {"FOCUS_LOGISTICS", "FOCUS_LOGISTICS_DESC", std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("SHP_INTSTEL_LOG")), "icons/focus/supply.png"},
                {"FOCUS_STOCKPILE", "FOCUS_STOCKPILE_DESC", std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("PRO_GENERIC_SUPPLIES")), "icons/focus/stockpile.png"},
                {"FOCUS_STEALTH", "FOCUS_STEALTH_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_PLANET_CLOAK")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("DEF_PLANET_CLOAK"))
                    )
                ), "icons/focus/stealth.png"},
                {"FOCUS_BIOTERROR", "FOCUS_BIOTERROR_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_BIOTERROR_PROJECTOR")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("GRO_BIOTERROR"))
                    )
                ), "icons/focus/bioterror.png"},
                {"FOCUS_STARGATE_SEND", "FOCUS_STARGATE_SEND_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_STARGATE")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("CON_STARGATE"))
                    )
                ), "icons/focus/stargate_send.png"},
                {"FOCUS_STARGATE_RECEIVE", "FOCUS_STARGATE_RECEIVE_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_STARGATE")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("CON_STARGATE"))
                    )
                ), "icons/focus/stargate_receive.png"},
                {"FOCUS_PLANET_DRIVE", "FOCUS_PLANET_DRIVE_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_PLANET_DRIVE")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("CON_PLANET_DRIVE"))
                    )
                ), "icons/building/planetary_stardrive.png"},
                {"FOCUS_DISTORTION", "FOCUS_DISTORTION_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_SPATIAL_DISTORT_GEN")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains<>>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("LRN_SPATIAL_DISTORT_GEN"))
                    )
                ), "icons/focus/distortion.png"},
                {"FOCUS_DOMINATION", "FOCUS_DOMINATION_DESC", std::make_unique<Condition::And>(
                    std::make_unique<Condition::HasTag>("TELEPATHIC"),
                    std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("LRN_PSY_DOM"))
                ), "icons/focus/psi_domination.png"}
            },
            "FOCUS_INDUSTRY",
            {
                {PlanetType::PT_SWAMP, PlanetEnvironment::PE_POOR},
                {PlanetType::PT_TOXIC, PlanetEnvironment::PE_ADEQUATE},
                {PlanetType::PT_INFERNO, PlanetEnvironment::PE_GOOD},
                {PlanetType::PT_RADIATED, PlanetEnvironment::PE_ADEQUATE},
                {PlanetType::PT_BARREN, PlanetEnvironment::PE_POOR},
                {PlanetType::PT_TUNDRA, PlanetEnvironment::PE_POOR},
                {PlanetType::PT_DESERT, PlanetEnvironment::PE_HOSTILE},
                {PlanetType::PT_TERRAN, PlanetEnvironment::PE_HOSTILE},
                {PlanetType::PT_OCEAN, PlanetEnvironment::PE_POOR},
                {PlanetType::PT_ASTEROIDS, PlanetEnvironment::PE_UNINHABITABLE},
                {PlanetType::PT_GASGIANT, PlanetEnvironment::PE_UNINHABITABLE},
            },
            array_to_vector<Effect::EffectsGroup, 1>({
                std::make_unique<Effect::EffectsGroup>(
                    std::make_unique<Condition::Source>(),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET),
                        std::make_unique<Condition::MeterValue>(MeterType::METER_TARGET_INDUSTRY,
                            std::make_unique<ValueRef::Constant<double>>(0.0),
                            nullptr),
                        std::make_unique<Condition::MeterValue>(MeterType::METER_HAPPINESS,
                            std::make_unique<ValueRef::Constant<double>>(0.0),
                            nullptr),
                        std::make_unique<Condition::FocusType>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("FOCUS_INDUSTRY")}))
                    ),
                    array_to_vector<Effect::Effect, 1>({std::make_unique<Effect::SetMeter>(MeterType::METER_TARGET_INDUSTRY,
                        std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
                            std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
                            std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
                                std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Population"),
                                std::make_unique<ValueRef::NamedRef<double>>("INDUSTRY_FOCUS_TARGET_INDUSTRY_PERPOP")
                            )
                        )
                    )}),
                    "FOCUS_INDUSTRY_LABEL",
                    "",
                    98
                )
            }),
            nullptr,
            true,
            false,
            true,
            true,
            {"LITHIC", "BAD_RESEARCH", "GREAT_INFLUENCE", "GOOD_HAPPINESS", "AVERAGE_SUPPLY", "PEDIA_LITHIC_SPECIES_CLASS"},
            {"FOCUS_INDUSTRY", "SHIMMER_SILK_SPECIAL", "FRACTAL_GEODES_SPECIAL", "SUPERCONDUCTOR_SPECIAL", "PROBIOTIC_SPECIAL", "MINERALS_SPECIAL", "CRYSTALS_SPECIAL", "PLC_DIVINE_AUTHORITY", "PLC_CONFORMANCE", "PLC_TERROR_SUPPRESSION", "PLC_INDOCTRINATION", "PLC_BUREAUCRACY"},
            {"BLD_SCRYING_SPHERE", "BLD_MEGALITH", "BLD_PLANET_DRIVE", "BLD_GATEWAY_VOID", "BLD_GAS_GIANT_GEN", "FORTRESS_SPECIAL", "HONEYCOMB_SPECIAL", "PHILOSOPHER_SPECIAL", "TIDAL_LOCK_SPECIAL", "PLC_DIVERSITY", "PLC_LIBERTY", "PLC_ARTISAN_WORKSHOPS", "PLC_CONFEDERATION"},
            nullptr,
            nullptr,
            "icons/species/abaddonnian.png",
            1.0,
            9999};
        BOOST_WARN(test_species == species);
        BOOST_WARN(test_species.Name() == species.Name());
        // TODO: test equality of Effects once they are stored by value not pointer
        BOOST_WARN(test_species.Likes() == species.Likes());
        BOOST_WARN(test_species.Dislikes() == species.Dislikes());
        BOOST_WARN(test_species.CombatTargets() == species.CombatTargets());
        BOOST_WARN(test_species.Tags() == species.Tags());
        BOOST_WARN(test_species.Foci() == species.Foci());
    }

    // test it last
    BOOST_CHECK_EQUAL(7, ordering.size());
    BOOST_CHECK_EQUAL(1, species_map.size());
}

BOOST_AUTO_TEST_CASE(parse_buildings) {
    PythonParser parser(m_python, m_test_scripting_dir);

    auto buildings_p = Pending::ParseSynchronously(parse::buildings, parser, m_test_scripting_dir / "buildings");
    const auto buildings = *Pending::WaitForPendingUnlocked(std::move(buildings_p));

    BOOST_REQUIRE(!buildings.empty());

    BOOST_CHECK_EQUAL(1, buildings.size());
    BOOST_CHECK_EQUAL(true, buildings.contains("BLD_ART_BLACK_HOLE"));

    const auto building_it = buildings.find("BLD_ART_BLACK_HOLE");
    BOOST_REQUIRE(building_it != buildings.end());

    const auto& building = building_it->second;

    BOOST_REQUIRE(building);

    std::set<std::string> test_tags{};

    const BuildingType test_building{
        "BLD_ART_BLACK_HOLE",
        "BLD_ART_BLACK_HOLE_DESC",
        CommonParams{
            std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
                std::make_unique<ValueRef::Constant<double>>(45.0),
                std::make_unique<ValueRef::ComplexVariable<double>>(
                    "GameRule",
                    nullptr,
                    nullptr,
                    nullptr,
                    std::make_unique<ValueRef::Constant<std::string>>("RULE_BUILDING_COST_FACTOR"),
                    nullptr
                )),
            std::make_unique<ValueRef::Constant<int>>(6),
            true,
            test_tags,
            std::make_unique<Condition::And>(
                std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET),
                std::make_unique<Condition::Not>(std::make_unique<Condition::Contains<>>(
                    std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({
                       std::make_unique<ValueRef::Constant<std::string>>("BLD_ART_BLACK_HOLE")
                    }))
                )),
                std::make_unique<Condition::EmpireAffiliation>(
                    std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner")
                ),
                std::make_unique<Condition::StarType>(array_to_vector<ValueRef::ValueRef< ::StarType>, 1>({
                    std::make_unique<ValueRef::Constant< ::StarType>>(::StarType::STAR_RED)
                }))
            ),
            array_to_vector<Effect::EffectsGroup, 2>({
                std::make_unique<Effect::EffectsGroup>(
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::ObjectID>(
                            std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "SystemID")
                        ),
                        std::make_unique<Condition::Type>(UniverseObjectType::OBJ_SYSTEM)
                    ),
                    std::make_unique<Condition::StarType>(array_to_vector<ValueRef::ValueRef< ::StarType>, 1>({
                        std::make_unique<ValueRef::Constant< ::StarType>>(::StarType::STAR_RED)
                    })),
                    array_to_vector<Effect::Effect, 2>({
                        std::make_unique<Effect::SetStarType>(
                            std::make_unique<ValueRef::Constant< ::StarType>>(::StarType::STAR_BLACK)
                        ),
                        std::make_unique<Effect::GenerateSitRepMessage>(
                            std::string{"EFFECT_BLACKHOLE"},
                            std::string{"icons/building/blackhole.png"},
                            pair_array_to_vector<ValueRef::ValueRef<std::string>, 1>({
                                std::pair(VarText::SYSTEM_ID_TAG, std::make_unique<ValueRef::StringCast<int>>(
                                    std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "SystemID")
                                ))
                            }),
                            std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner"),
                            EmpireAffiliationType::AFFIL_SELF,
                            std::string{"EFFECT_BLACKHOLE_LABEL"}
                        )
                    }),
                    "",
                    "ART_BLACK_HOLE",
                    100
                ),
                std::make_unique<Effect::EffectsGroup>(
                    std::make_unique<Condition::Source>(),
                    nullptr,
                    array_to_vector<Effect::Effect, 1>({
                        std::make_unique<Effect::Destroy>()
                    }),
                    "",
                    "",
                    100
                )
            }),
            {},
            {},
            std::make_unique<Condition::And>(
                std::make_unique<Condition::Not>(std::make_unique<Condition::Contains<>>(
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({
                            std::make_unique<ValueRef::Constant<std::string>>(std::string{ValueRef::Constant<std::string>::current_content})
                        })),
                        std::make_unique<Condition::EmpireAffiliation>(
                            std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner")
                        )
                    )
                )),
                std::make_unique<Condition::Not>(std::make_unique<Condition::Enqueued>(
                    BuildType::BT_BUILDING,
                    std::make_unique<ValueRef::Constant<std::string>>(std::string{ValueRef::Constant<std::string>::current_content})
                )),
                std::make_unique<Condition::EmpireAffiliation>(
                    std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner")
                )
            )
        },
        CaptureResult::CR_CAPTURE,
        "icons/building/blackhole.png" 
    };

    BOOST_CHECK_EQUAL(test_building.Name(), building->Name());
    BOOST_CHECK_EQUAL(test_building.Description(), building->Description());
    BOOST_CHECK_EQUAL(test_building.ProductionCostTimeLocationInvariant(), building->ProductionCostTimeLocationInvariant());
    BOOST_CHECK((*test_building.Cost()) == (*building->Cost()));
    BOOST_CHECK((*test_building.Time()) == (*building->Time()));
    BOOST_CHECK_EQUAL(test_building.Producible(), building->Producible());
    BOOST_CHECK(test_building.ProductionMeterConsumption() == building->ProductionMeterConsumption());
    BOOST_CHECK(test_building.ProductionSpecialConsumption() == building->ProductionSpecialConsumption());
    BOOST_CHECK(test_building.Tags() == building->Tags());

    const Condition::And* location_cond = dynamic_cast<const Condition::And*>(building->Location());
    BOOST_REQUIRE(location_cond != nullptr);
    std::vector<const Condition::Condition*> location_conds = location_cond->OperandsRaw();
    BOOST_REQUIRE_EQUAL(4, location_conds.size());
    BOOST_CHECK(dynamic_cast<const Condition::Type*>(location_conds[0]) != nullptr);
    BOOST_CHECK(dynamic_cast<const Condition::Not*>(location_conds[1]) != nullptr);
    BOOST_CHECK(dynamic_cast<const Condition::EmpireAffiliation*>(location_conds[2]) != nullptr);
    BOOST_CHECK(dynamic_cast<const Condition::StarType*>(location_conds[3]) != nullptr);
    BOOST_CHECK_EQUAL(22510, location_cond->GetCheckSum());
    BOOST_CHECK_EQUAL(3267, location_conds[0]->GetCheckSum());
    BOOST_CHECK_EQUAL(9108, location_conds[1]->GetCheckSum());
    BOOST_CHECK_EQUAL(5108, location_conds[2]->GetCheckSum());
    BOOST_CHECK_EQUAL(3683, location_conds[3]->GetCheckSum());

    const Condition::And* test_location_cond = dynamic_cast<const Condition::And*>(test_building.Location());
    BOOST_REQUIRE(test_location_cond != nullptr);
    std::vector<const Condition::Condition*> test_location_conds = test_location_cond->OperandsRaw();
    BOOST_REQUIRE_EQUAL(4, test_location_conds.size());
    BOOST_CHECK(dynamic_cast<const Condition::Type*>(test_location_conds[0]) != nullptr);
    BOOST_CHECK(dynamic_cast<const Condition::Not*>(test_location_conds[1]) != nullptr);
    BOOST_CHECK(dynamic_cast<const Condition::EmpireAffiliation*>(test_location_conds[2]) != nullptr);
    BOOST_CHECK(dynamic_cast<const Condition::StarType*>(test_location_conds[3]) != nullptr);
    BOOST_CHECK_EQUAL(22510, test_location_cond->GetCheckSum());
    BOOST_CHECK_EQUAL(3267, test_location_conds[0]->GetCheckSum());
    BOOST_CHECK_EQUAL(9108, test_location_conds[1]->GetCheckSum());
    BOOST_CHECK_EQUAL(5108, test_location_conds[2]->GetCheckSum());
    BOOST_CHECK_EQUAL(3683, test_location_conds[3]->GetCheckSum());

#if defined(FREEORION_MACOSX)
    // ToDo: fix broken test on MacOS
    BOOST_WARN((*test_building.Location()) == (*building->Location()));
#else
    BOOST_CHECK((*test_building.Location()) == (*building->Location()));
#endif
    BOOST_CHECK_EQUAL(test_building.Location()->GetCheckSum(), building->Location()->GetCheckSum());
    BOOST_CHECK((*test_building.EnqueueLocation()) == (*building->EnqueueLocation()));
    BOOST_REQUIRE_EQUAL(test_building.Effects().size(), building->Effects().size());
#if defined(FREEORION_MACOSX)
    // ToDo: fix broken test on MacOS
    BOOST_WARN(test_building.Effects() == building->Effects());
    BOOST_WARN(test_building.Effects()[0] == building->Effects()[0]);
#else
    BOOST_CHECK(test_building.Effects() == building->Effects());
    BOOST_CHECK(test_building.Effects()[0] == building->Effects()[0]);
#endif
    BOOST_CHECK_EQUAL(test_building.Effects()[0].GetCheckSum(), building->Effects()[0].GetCheckSum());
    BOOST_CHECK(test_building.Effects()[1] == building->Effects()[1]);
    BOOST_CHECK_EQUAL(test_building.Effects()[1].GetCheckSum(), building->Effects()[1].GetCheckSum());
    BOOST_CHECK_EQUAL(test_building.Icon(), building->Icon());
    BOOST_CHECK_EQUAL(test_building.GetCheckSum(), building->GetCheckSum());
}

BOOST_AUTO_TEST_CASE(parse_empire_statistics) {
    PythonParser parser(m_python, m_test_scripting_dir);

    auto empire_statistics_p = Pending::ParseSynchronously(parse::statistics, parser, m_test_scripting_dir / "empire_statistics");
    const auto empire_statistics = *Pending::WaitForPendingUnlocked(std::move(empire_statistics_p));
    BOOST_REQUIRE_EQUAL(1, empire_statistics.size());

    const auto statistic_it = empire_statistics.find("ARMED_MONSTER_COUNT");

    BOOST_REQUIRE(statistic_it != empire_statistics.end());

    BOOST_REQUIRE_EQUAL("ARMED_MONSTER_COUNT", statistic_it->first);

    const auto* statistic = dynamic_cast<const ValueRef::Statistic<double>*>(statistic_it->second.get());

    BOOST_REQUIRE(statistic != nullptr);

    BOOST_REQUIRE_EQUAL(ValueRef::StatisticType::COUNT, statistic->GetStatisticType());

    const auto* condition = dynamic_cast<const Condition::And*>(statistic->GetSamplingCondition());

    BOOST_REQUIRE(condition != nullptr);
    BOOST_REQUIRE_EQUAL(4, condition->OperandsRaw().size());
    BOOST_CHECK_EQUAL(3265, condition->OperandsRaw()[0]->GetCheckSum());
    BOOST_CHECK_EQUAL(1813, condition->OperandsRaw()[1]->GetCheckSum());
    BOOST_CHECK_EQUAL(1556, condition->OperandsRaw()[2]->GetCheckSum());
    BOOST_CHECK_EQUAL(2830, condition->OperandsRaw()[3]->GetCheckSum());
}

BOOST_AUTO_TEST_SUITE_END()

