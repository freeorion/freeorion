#include <boost/test/unit_test.hpp>

#include "parse/Parse.h"
#include "parse/PythonParser.h"
#include "universe/Conditions.h"
#include "universe/Effects.h"
#include "universe/Planet.h"
#include "universe/Tech.h"
#include "universe/UnlockableItem.h"
#include "universe/ValueRefs.h"
#include "universe/NamedValueRefManager.h"
#include "util/CheckSums.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/Pending.h"
#include "util/PythonCommon.h"

#include "ParserAppFixture.h"

namespace {
    template <typename T, size_t N>
    inline std::vector<std::unique_ptr<T>> array_to_vector(std::array<std::unique_ptr<T>, N>&& a)
    { return {std::make_move_iterator(a.begin()), std::make_move_iterator(a.end())}; }
}

BOOST_FIXTURE_TEST_SUITE(TestPythonParser, ParserAppFixture)

BOOST_AUTO_TEST_CASE(parse_game_rules) {
    PythonParser parser(m_python, m_scripting_dir, true);

    auto game_rules_p = Pending::ParseSynchronously(parse::game_rules, parser,  m_scripting_dir / "game_rules.focs.py");
    auto game_rules = *Pending::WaitForPendingUnlocked(std::move(game_rules_p));
    BOOST_REQUIRE(!game_rules.empty());
    BOOST_REQUIRE(game_rules.count("RULE_HABITABLE_SIZE_MEDIUM") > 0);
    BOOST_REQUIRE(GameRule::Type::TOGGLE == game_rules["RULE_ENABLE_ALLIED_REPAIR"].type);
}

BOOST_AUTO_TEST_CASE(parse_techs) {
    PythonParser parser(m_python, m_scripting_dir, true);

    auto techs_p = Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, parser, m_scripting_dir / "techs");
    auto [techs, tech_categories, categories_seen] = *Pending::WaitForPendingUnlocked(std::move(techs_p));
    BOOST_REQUIRE(!tech_categories.empty());

    BOOST_REQUIRE_EQUAL(1, tech_categories.count("PRODUCTION_CATEGORY"));
    BOOST_REQUIRE_EQUAL(1, tech_categories.count("GROWTH_CATEGORY"));

    const auto cat_it = tech_categories.find("CONSTRUCTION_CATEGORY");
    BOOST_REQUIRE(tech_categories.end() != cat_it);

    BOOST_REQUIRE_EQUAL("CONSTRUCTION_CATEGORY", cat_it->second->name);
    BOOST_REQUIRE_EQUAL("construction.png", cat_it->second->graphic);
    const std::array<uint8_t, 4> test_colour{241, 233, 87, 255};
    BOOST_REQUIRE(test_colour == cat_it->second->colour);

    BOOST_REQUIRE(!techs.empty());
    BOOST_REQUIRE(!categories_seen.empty());

    BOOST_REQUIRE_EQUAL(0, categories_seen.count("PRODUCTION_CATEGORY"));
    BOOST_REQUIRE_EQUAL(1, categories_seen.count("GROWTH_CATEGORY"));

    {
        const auto tech_it = techs.get<TechManager::NameIndex>().find("LRN_ALGO_ELEGANCE");
        BOOST_REQUIRE(techs.get<TechManager::NameIndex>().end() != tech_it);
        BOOST_REQUIRE_EQUAL("LRN_ALGO_ELEGANCE", (*tech_it)->Name());
        BOOST_REQUIRE_EQUAL("LRN_ALGO_ELEGANCE_DESC", (*tech_it)->Description());
        BOOST_REQUIRE_EQUAL("RESEARCH_SHORT_DESC", (*tech_it)->ShortDescription());
        BOOST_REQUIRE_EQUAL("LEARNING_CATEGORY", (*tech_it)->Category());
        BOOST_REQUIRE_EQUAL(true, (*tech_it)->Researchable());
        BOOST_REQUIRE_EQUAL("icons/tech/algorithmic_elegance.png", (*tech_it)->Graphic());

        const ScriptingContext context;

        BOOST_REQUIRE_CLOSE(0.0, (*tech_it)->ResearchCost(ALL_EMPIRES, context), 0.1);
        BOOST_REQUIRE_EQUAL(3, (*tech_it)->ResearchTime(ALL_EMPIRES, context));
        BOOST_REQUIRE_EQUAL(0, (*tech_it)->Effects().size());
        BOOST_REQUIRE_EQUAL(0, (*tech_it)->UnlockedTechs().size());
        BOOST_REQUIRE_EQUAL(0, (*tech_it)->Prerequisites().size());

        const auto& tech_tags = (*tech_it)->Tags();
        BOOST_REQUIRE_EQUAL(1, tech_tags.size());
        BOOST_REQUIRE((*tech_it)->HasTag("PEDIA_LEARNING_CATEGORY"));

        const auto& tech_items = (*tech_it)->UnlockedItems();
        BOOST_REQUIRE_EQUAL(1, tech_items.size());
        BOOST_REQUIRE_EQUAL(UnlockableItemType::UIT_POLICY, tech_items[0].type);
        BOOST_REQUIRE_EQUAL("PLC_ALGORITHMIC_RESEARCH", tech_items[0].name);

        Tech tech{
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
                    std::make_unique<ValueRef::Constant<std::string>>(std::string("RULE_TECH_COST_FACTOR")),
                    nullptr
                )),
            std::make_unique<ValueRef::Constant<int>>(3),
            true,
            {"PEDIA_LEARNING_CATEGORY"},
            {},
            {},
            {UnlockableItem{UnlockableItemType::UIT_POLICY, "PLC_ALGORITHMIC_RESEARCH"}},
            "icons/tech/algorithmic_elegance.png"
        };
        BOOST_REQUIRE(tech == (**tech_it));
    }

    {
        const auto tech_it = techs.get<TechManager::NameIndex>().find("CON_ORBITAL_HAB");
        BOOST_REQUIRE(techs.get<TechManager::NameIndex>().end() != tech_it);

        BOOST_REQUIRE_EQUAL(1, (*tech_it)->Effects().size());
        const auto& tech_effect_gr = (*tech_it)->Effects().front();
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

        const auto& prereqs = (*tech_it)->Prerequisites();
        BOOST_REQUIRE_EQUAL(1, prereqs.size());
        BOOST_REQUIRE_EQUAL(1, std::count(prereqs.begin(), prereqs.end(), "PRO_MICROGRAV_MAN"));

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        effects.push_back(std::make_unique<Effect::SetMeter>(MeterType::METER_TARGET_POPULATION,
                        std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
                            std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
                            std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
                                std::make_unique<ValueRef::Constant<double>>(1.0),
                                std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "HabitableSize"))
                        ),
                        std::string("ORBITAL_HAB_LABEL")));

        auto effect_group = std::shared_ptr<Effect::EffectsGroup>(new Effect::EffectsGroup(
                std::make_unique<Condition::And>(
                    std::make_unique<Condition::Species>(),
                    std::make_unique<Condition::EmpireAffiliation>(
                        std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner")
                    )
                ),
                nullptr,
                std::move(effects),
                "",
                "",
                17,
                "",
                ""
        ));

        Tech tech{
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
                    std::make_unique<ValueRef::Constant<std::string>>(std::string("RULE_TECH_COST_FACTOR")),
                    nullptr
                )),
            std::make_unique<ValueRef::Constant<int>>(7),
            true,
            {"PEDIA_GROWTH_CATEGORY"},
            {effect_group},
            {"PRO_MICROGRAV_MAN"},
            {},
            "icons/tech/orbital_gardens.png"
        };
        BOOST_REQUIRE(tech == (**tech_it));

    }

    // test it last
    BOOST_REQUIRE_EQUAL(2, techs.size());
    BOOST_REQUIRE_EQUAL(9, tech_categories.size());
    BOOST_REQUIRE_EQUAL(2, categories_seen.size());
}

BOOST_AUTO_TEST_CASE(parse_species) {
    PythonParser parser(m_python, m_scripting_dir, true);

    auto species_p = Pending::ParseSynchronously(parse::species, parser, m_scripting_dir / "species");
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
        BOOST_REQUIRE_EQUAL(true, effect_group.HasMeterEffects());
        BOOST_REQUIRE_EQUAL(false, effect_group.HasAppearanceEffects());
        BOOST_REQUIRE_EQUAL(false, effect_group.HasSitrepEffects());
        BOOST_REQUIRE_EQUAL(35781, effect_group.GetCheckSum());

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

        BOOST_REQUIRE_EQUAL(2460986, species.GetCheckSum());

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
                    std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_PLANET_CLOAK")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("DEF_PLANET_CLOAK"))
                    )
                ), "icons/focus/stealth.png"},
                {"FOCUS_BIOTERROR", "FOCUS_BIOTERROR_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_BIOTERROR_PROJECTOR")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("GRO_BIOTERROR"))
                    )
                ), "icons/focus/bioterror.png"},
                {"FOCUS_STARGATE_SEND", "FOCUS_STARGATE_SEND_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_STARGATE")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("CON_STARGATE"))
                    )
                ), "icons/focus/stargate_send.png"},
                {"FOCUS_STARGATE_RECEIVE", "FOCUS_STARGATE_RECEIVE_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_STARGATE")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("CON_STARGATE"))
                    )
                ), "icons/focus/stargate_receive.png"},
                {"FOCUS_PLANET_DRIVE", "FOCUS_PLANET_DRIVE_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_PLANET_DRIVE")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
                        std::make_unique<Condition::OwnerHasTech>(std::make_unique<ValueRef::Constant<std::string>>("CON_PLANET_DRIVE"))
                    )
                ), "icons/building/planetary_stardrive.png"},
                {"FOCUS_DISTORTION", "FOCUS_DISTORTION_DESC", std::make_unique<Condition::Or>(
                    std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_SPATIAL_DISTORT_GEN")}))),
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Contains>(std::make_unique<Condition::Building>(array_to_vector<ValueRef::ValueRef<std::string>, 1>({std::make_unique<ValueRef::Constant<std::string>>("BLD_TRANSFORMER")}))),
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

/**
 * Checks count of techs and tech categories in real scripts
 * FO_CHECKSUM_TECH_NAME determines tech name to be check for FO_CHECKSUM_TECH_VALUE checksum
 */

BOOST_AUTO_TEST_CASE(parse_techs_full) {
    auto scripting_dir = boost::filesystem::system_complete(GetBinDir() / "default/scripting");
    BOOST_REQUIRE(scripting_dir.is_absolute());
    BOOST_REQUIRE(boost::filesystem::exists(scripting_dir));
    BOOST_REQUIRE(boost::filesystem::is_directory(scripting_dir));

    PythonParser parser(m_python, scripting_dir, true);

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, scripting_dir / "common");

    auto techs_p = Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, parser, scripting_dir / "techs");
    auto [techs, tech_categories, categories_seen] = *Pending::WaitForPendingUnlocked(std::move(techs_p));

    BOOST_REQUIRE(!techs.empty());
    BOOST_REQUIRE(!tech_categories.empty());
    BOOST_REQUIRE(!categories_seen.empty());

    BOOST_REQUIRE_EQUAL(209, techs.size());
    BOOST_REQUIRE_EQUAL(9, tech_categories.size());
    BOOST_REQUIRE_EQUAL(9, categories_seen.size());

    if (const char *tech_name = std::getenv("FO_CHECKSUM_TECH_NAME")) {
        const auto tech_it = techs.get<TechManager::NameIndex>().find(tech_name);
        BOOST_REQUIRE(techs.get<TechManager::NameIndex>().end() != tech_it);
        BOOST_REQUIRE_EQUAL(tech_name, (*tech_it)->Name());

        BOOST_TEST_MESSAGE("Dump " << tech_name << ":");
        BOOST_TEST_MESSAGE((*tech_it)->Dump(0));

        if (const char *tech_checksum_str = std::getenv("FO_CHECKSUM_TECH_VALUE")) {
            unsigned int tech_checksum = boost::lexical_cast<unsigned int>(tech_checksum_str);
            unsigned int value{0};
            CheckSums::CheckSumCombine(value, *tech_it);
            BOOST_REQUIRE_EQUAL(tech_checksum, value);
        }
    }
}

/**
 * Checks count of species and species census ordering in real scripts
 * FO_CHECKSUM_SPECIES_NAME determines tech name to be check for FO_CHECKSUM_SPECIES_VALUE checksum
 */

BOOST_AUTO_TEST_CASE(parse_species_full) {
    auto scripting_dir = boost::filesystem::system_complete(GetBinDir() / "default/scripting");
    BOOST_REQUIRE(scripting_dir.is_absolute());
    BOOST_REQUIRE(boost::filesystem::exists(scripting_dir));
    BOOST_REQUIRE(boost::filesystem::is_directory(scripting_dir));

    PythonParser parser(m_python, scripting_dir, true);

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, scripting_dir / "common");

    auto species_p = Pending::ParseSynchronously(parse::species, parser, scripting_dir / "species");
    const auto [species, ordering] = *Pending::WaitForPendingUnlocked(std::move(species_p));

    BOOST_REQUIRE(!ordering.empty());
    BOOST_REQUIRE(!species.empty());

    BOOST_REQUIRE_EQUAL(7, ordering.size());
    BOOST_REQUIRE_EQUAL(50, species.size());

    if (const char *species_name = std::getenv("FO_CHECKSUM_SPECIES_NAME")) {
        const auto species_it = species.find(species_name);
        BOOST_REQUIRE(species.end() != species_it);
        BOOST_REQUIRE_EQUAL(species_name, species_it->second.Name());

        BOOST_TEST_MESSAGE("Dump " << species_name << ":");
        BOOST_TEST_MESSAGE(species_it->second.Dump(0));

        if (const char *species_checksum_str = std::getenv("FO_CHECKSUM_SPECIES_VALUE")) {
            uint32_t species_checksum = boost::lexical_cast<uint32_t>(species_checksum_str);
            uint32_t value{0};
            CheckSums::CheckSumCombine(value, species_it->second);
            BOOST_REQUIRE_EQUAL(species_checksum, value);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

