#include <boost/test/unit_test.hpp>

#include "parse/PythonParser.h"
#include "util/Directories.h"
#include "util/PythonCommon.h"

BOOST_FIXTURE_TEST_SUITE(TestPythonParser, PythonCommon)

BOOST_AUTO_TEST_CASE(parse0) {
    InitDirs("");

    BOOST_REQUIRE(Initialize());

    PythonParser parser(*this);
}

BOOST_AUTO_TEST_SUITE_END()

