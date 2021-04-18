#include <boost/test/unit_test.hpp>

#include "parse/Parse.h"
#include "parse/PythonParser.h"
#include "universe/Conditions.h"
#include "universe/Effects.h"
#include "universe/Tech.h"
#include "universe/UnlockableItem.h"
#include "universe/ValueRefs.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/Pending.h"
#include "util/PythonCommon.h"

#include "ParserAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestPythonParser, ParserAppFixture)

BOOST_AUTO_TEST_CASE(parse_game_rules) {
    PythonParser parser(m_python, m_scripting_dir);

    Pending::Pending<GameRules> game_rules_p = Pending::ParseSynchronously(parse::game_rules, parser,  m_scripting_dir / "game_rules.focs.py");
    auto game_rules = *Pending::WaitForPendingUnlocked(std::move(game_rules_p));
    BOOST_REQUIRE(!game_rules.Empty());
    BOOST_REQUIRE(game_rules.RuleExists("RULE_HABITABLE_SIZE_MEDIUM"));
    BOOST_REQUIRE(GameRules::Type::TOGGLE == game_rules.GetType("RULE_ENABLE_ALLIED_REPAIR"));
}

BOOST_AUTO_TEST_CASE(parse_techs) {
    PythonParser parser(m_python, m_scripting_dir);

    Pending::Pending<TechManager::TechParseTuple> techs_p = Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, parser, m_scripting_dir / "techs");
    auto [techs, tech_categories, categories_seen] = *Pending::WaitForPendingUnlocked(std::move(techs_p));
    BOOST_REQUIRE(!tech_categories.empty());

    BOOST_REQUIRE_EQUAL(1, tech_categories.count("PRODUCTION_CATEGORY"));
    BOOST_REQUIRE_EQUAL(1, tech_categories.count("GROWTH_CATEGORY"));

    const auto cat_it = tech_categories.find("CONSTRUCTION_CATEGORY");
    BOOST_REQUIRE(tech_categories.end() != cat_it);

    BOOST_REQUIRE_EQUAL("CONSTRUCTION_CATEGORY", cat_it->second->name);
    BOOST_REQUIRE_EQUAL("construction.png", cat_it->second->graphic);
    const std::array<unsigned char, 4> test_colour{241, 233, 87, 255};
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

        BOOST_REQUIRE_CLOSE(0.0, (*tech_it)->ResearchCost(ALL_EMPIRES), 0.1);
        BOOST_REQUIRE_EQUAL(3, (*tech_it)->ResearchTime(ALL_EMPIRES));
        BOOST_REQUIRE_EQUAL(0, (*tech_it)->Effects().size());
        BOOST_REQUIRE_EQUAL(0, (*tech_it)->UnlockedTechs().size());
        BOOST_REQUIRE_EQUAL(0, (*tech_it)->Prerequisites().size());

        const auto& tech_tags = (*tech_it)->Tags();
        BOOST_REQUIRE_EQUAL(1, tech_tags.size());
        BOOST_REQUIRE_EQUAL(1, tech_tags.count("PEDIA_LEARNING_CATEGORY"));

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

        std::shared_ptr<Effect::EffectsGroup> effect_group = std::shared_ptr<Effect::EffectsGroup>(new Effect::EffectsGroup(
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

BOOST_AUTO_TEST_CASE(parse_techs_full) {
    auto scripting_dir = boost::filesystem::system_complete(GetBinDir() / "default/scripting");
    BOOST_REQUIRE(scripting_dir.is_absolute());
    BOOST_REQUIRE(boost::filesystem::exists(scripting_dir));
    BOOST_REQUIRE(boost::filesystem::is_directory(scripting_dir));

    PythonParser parser(m_python, scripting_dir);

    Pending::Pending<TechManager::TechParseTuple> techs_p = Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, parser, scripting_dir / "techs");
    auto [techs, tech_categories, categories_seen] = *Pending::WaitForPendingUnlocked(std::move(techs_p));

    BOOST_REQUIRE(!techs.empty());
    BOOST_REQUIRE(!tech_categories.empty());
    BOOST_REQUIRE(!categories_seen.empty());

    BOOST_REQUIRE_EQUAL(207, techs.size());
    BOOST_REQUIRE_EQUAL(9, tech_categories.size());
    BOOST_REQUIRE_EQUAL(9, categories_seen.size());
}

BOOST_AUTO_TEST_SUITE_END()

