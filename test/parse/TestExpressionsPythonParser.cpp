#include <boost/test/unit_test.hpp>

#include "parse/PythonParser.h"

#include "ParserAppFixture.h"

namespace py = boost::python;

class ExpressionParserAppFixture : public ParserAppFixture {
public:
    ExpressionParserAppFixture() :
        ParserAppFixture(true)
    { }
};

BOOST_FIXTURE_TEST_SUITE(TestExpressionsPythonParser, ExpressionParserAppFixture)

BOOST_AUTO_TEST_CASE(test_1) {
    PythonParser parser(m_python);

    py::dict globals;
    try {
        py::object result = PythonCommon::CompileEvalExpression("2 + 2", globals);
        BOOST_CHECK_EQUAL(py::extract<int>(result)(), 4);
    } catch (const boost::python::error_already_set&) {
        m_python.HandleErrorAlreadySet();
        BOOST_FAIL("Expression evaluation failed");
    }
}

BOOST_AUTO_TEST_SUITE_END()
