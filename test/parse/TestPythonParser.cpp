#include <boost/test/unit_test.hpp>

#include "parse/Parse.h"
#include "parse/PythonParser.h"
#include "universe/Conditions.h"
#include "universe/Effects.h"
#include "universe/Planet.h"
#include "universe/Tech.h"
#include "universe/UnlockableItem.h"
#include "universe/ValueRefs.h"
#include "util/CheckSums.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/Pending.h"
#include "util/PythonCommon.h"

#include "ParserAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestPythonParser, ParserAppFixture)

BOOST_AUTO_TEST_CASE(parse_game_rules) {
    PythonParser parser(m_python, m_scripting_dir);

    auto game_rules_p = Pending::ParseSynchronously(parse::game_rules, parser,  m_scripting_dir / "game_rules.focs.py");
    auto game_rules = *Pending::WaitForPendingUnlocked(std::move(game_rules_p));
    BOOST_REQUIRE(!game_rules.empty());
    BOOST_REQUIRE(game_rules.count("RULE_HABITABLE_SIZE_MEDIUM") > 0);
    BOOST_REQUIRE(GameRule::Type::TOGGLE == game_rules["RULE_ENABLE_ALLIED_REPAIR"].type);
}

BOOST_AUTO_TEST_CASE(parse_techs) {
    PythonParser parser(m_python, m_scripting_dir);

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
                std::make_unique<ValueRef::Constant<double>>(10),
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
        const auto& tech_effect_gr = (*tech_it)->Effects()[0];
        BOOST_REQUIRE_EQUAL("", tech_effect_gr->StackingGroup());
        BOOST_REQUIRE_EQUAL("", tech_effect_gr->GetDescription());
        BOOST_REQUIRE_EQUAL("", tech_effect_gr->AccountingLabel());
        BOOST_REQUIRE_EQUAL(17, tech_effect_gr->Priority());

        // scope
        const auto tech_effect_scope = tech_effect_gr->Scope();
        BOOST_REQUIRE_EQUAL(true, tech_effect_scope->RootCandidateInvariant());
        BOOST_REQUIRE_EQUAL(false, tech_effect_scope->LocalCandidateInvariant());
        BOOST_REQUIRE_EQUAL(true, tech_effect_scope->TargetInvariant());
        BOOST_REQUIRE_EQUAL(false, tech_effect_scope->SourceInvariant());

        Condition::And *tech_effect_scope_and = dynamic_cast<Condition::And*>(tech_effect_scope);
        BOOST_REQUIRE(tech_effect_scope_and != nullptr);
        BOOST_REQUIRE_EQUAL(2, tech_effect_scope_and->Operands().size());

        // effects list
        BOOST_REQUIRE_EQUAL(1, tech_effect_gr->EffectsList().size());
        const auto tech_effect = tech_effect_gr->EffectsList()[0];
        BOOST_REQUIRE_EQUAL(true, tech_effect->IsMeterEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsEmpireMeterEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsAppearanceEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsSitrepEffect());
        BOOST_REQUIRE_EQUAL(false, tech_effect->IsConditionalEffect());

        BOOST_REQUIRE_EQUAL(1, (*tech_it)->Prerequisites().size());
        BOOST_REQUIRE_EQUAL(1, (*tech_it)->Prerequisites().count("PRO_MICROGRAV_MAN"));

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        effects.push_back(std::make_unique<Effect::SetMeter>(MeterType::METER_TARGET_POPULATION,
                        std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
                            std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
                            std::make_unique<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
                                std::make_unique<ValueRef::Constant<double>>(1),
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
                std::make_unique<ValueRef::Constant<double>>(250),
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
    PythonParser parser(m_python, m_scripting_dir);

    auto species_p = Pending::StartAsyncParsing(parse::species, m_scripting_dir / "species");
    auto [species, ordering] = *Pending::WaitForPendingUnlocked(std::move(species_p));

    BOOST_REQUIRE(!ordering.empty());
    BOOST_REQUIRE(!species.empty());

    BOOST_REQUIRE_EQUAL("LITHIC", ordering[0]);
    BOOST_REQUIRE_EQUAL("ORGANIC", ordering[1]);
    BOOST_REQUIRE_EQUAL("GASEOUS", ordering[6]);

    {
        const auto species_it = species.find("SP_ABADDONI");
        BOOST_REQUIRE(species_it != species.end());

        const auto specie = species_it->second.get();
        BOOST_REQUIRE_EQUAL("SP_ABADDONI", specie->Name());
        BOOST_REQUIRE_EQUAL("SP_ABADDONI_DESC", specie->Description());
        // BOOST_REQUIRE_EQUAL("SP_ABADDONI_GAMEPLAY_DESC", specie->GameplayDescription()); // already resolved to user string

        BOOST_REQUIRE(specie->Location() != nullptr);
        BOOST_REQUIRE(specie->CombatTargets() == nullptr);

        BOOST_REQUIRE_EQUAL(14, specie->Foci().size());
        BOOST_REQUIRE_EQUAL("FOCUS_INDUSTRY", specie->Foci()[0].Name());
        BOOST_REQUIRE_EQUAL("FOCUS_DOMINATION", specie->Foci()[13].Name());
        BOOST_REQUIRE_EQUAL("FOCUS_INDUSTRY", specie->DefaultFocus());

        BOOST_REQUIRE_EQUAL(11, specie->PlanetEnvironments().size());
        BOOST_REQUIRE_EQUAL(PlanetEnvironment::PE_POOR, specie->GetPlanetEnvironment(PlanetType::PT_BARREN));
        BOOST_REQUIRE_EQUAL(PlanetEnvironment::PE_ADEQUATE, specie->GetPlanetEnvironment(PlanetType::PT_TOXIC));

        BOOST_REQUIRE_EQUAL(98, specie->Effects().size());
    }

    // test it last
    BOOST_REQUIRE_EQUAL(7, ordering.size());
    BOOST_REQUIRE_EQUAL(1, species.size());
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

    PythonParser parser(m_python, scripting_dir);

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, scripting_dir / "common");

    auto techs_p = Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, parser, scripting_dir / "techs");
    auto [techs, tech_categories, categories_seen] = *Pending::WaitForPendingUnlocked(std::move(techs_p));

    BOOST_REQUIRE(!techs.empty());
    BOOST_REQUIRE(!tech_categories.empty());
    BOOST_REQUIRE(!categories_seen.empty());

    BOOST_REQUIRE_EQUAL(208, techs.size());
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

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, scripting_dir / "common");

    auto species_p = Pending::StartAsyncParsing(parse::species, scripting_dir / "species");
    auto [species, ordering] = *Pending::WaitForPendingUnlocked(std::move(species_p));

    BOOST_REQUIRE(!ordering.empty());
    BOOST_REQUIRE(!species.empty());

    BOOST_REQUIRE_EQUAL(7, ordering.size());
    BOOST_REQUIRE_EQUAL(50, species.size());

    if (const char *species_name = std::getenv("FO_CHECKSUM_SPECIES_NAME")) {
        const auto species_it = species.find(species_name);
        BOOST_REQUIRE(species.end() != species_it);
        BOOST_REQUIRE_EQUAL(species_name, species_it->second->Name());

        BOOST_TEST_MESSAGE("Dump " << species_name << ":");
        BOOST_TEST_MESSAGE(species_it->second->Dump(0));

        if (const char *species_checksum_str = std::getenv("FO_CHECKSUM_SPECIES_VALUE")) {
            unsigned int species_checksum = boost::lexical_cast<unsigned int>(species_checksum_str);
            unsigned int value{0};
            CheckSums::CheckSumCombine(value, species_it->second);
            BOOST_REQUIRE_EQUAL(species_checksum, value);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

