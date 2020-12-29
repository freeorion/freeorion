#include <boost/test/unit_test.hpp>

#include "parse/Parse.h"
#include "parse/PythonParser.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/PythonCommon.h"

#include "ParserAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestPythonParser, ParserAppFixture)

BOOST_AUTO_TEST_CASE(parse0) {
    PythonParser parser(m_python);

    GameRules game_rules = parse::game_rules("default/scripting/game_rules.focs.txt");
    BOOST_REQUIRE(!game_rules.Empty());
    BOOST_REQUIRE(game_rules.RuleExists("RULE_HABITABLE_SIZE_MEDIUM"));
    BOOST_REQUIRE(GameRules::Type::TOGGLE == game_rules.GetType("RULE_ENABLE_ALLIED_REPAIR"));
}

BOOST_AUTO_TEST_SUITE_END()

