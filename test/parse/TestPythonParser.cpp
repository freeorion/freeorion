#include <boost/test/unit_test.hpp>

#include "parse/PythonParser.h"
#include "util/Directories.h"
#include "util/PythonCommon.h"

#include "ParserAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestPythonParser, ParserAppFixture)

BOOST_AUTO_TEST_CASE(parse0) {
    PythonParser parser(m_python);
}

BOOST_AUTO_TEST_SUITE_END()

