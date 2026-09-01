#include <boost/test/unit_test.hpp>

#include "parse/PythonParser.h"
#include "parse/EffectPythonParser.h"

#include "ParserAppFixture.h"

namespace py = boost::python;

class ExpressionParserAppFixture : public ParserAppFixture {
public:
    ExpressionParserAppFixture() :
        ParserAppFixture(true)
    { }
};

BOOST_FIXTURE_TEST_SUITE(TestExpressionsPythonParser, ExpressionParserAppFixture)

BOOST_AUTO_TEST_CASE(test_effect) {
    PythonParser parser(m_python);

    parser.LoadEffectsModule();
    parser.LoadConditionsModule();

    py::dict globals;
    const char* imports_code = R"(
from focs._effects import MoveTowards
from focs._conditions import IsSource
    )";
    try {
        PythonCommon::CompileEval(imports_code, "<imports>", globals);
        py::object obj_result = PythonCommon::CompileEvalExpression("MoveTowards(speed=5, target=IsSource)", globals);
        auto result = py::extract<effect_wrapper>(obj_result)();
        BOOST_CHECK_EQUAL("MoveTowards destination = Source\n\n", result.effect->Dump(0));
    } catch (const boost::python::error_already_set&) {
        m_python.HandleErrorAlreadySet();
        BOOST_FAIL("Expression evaluation failed");
    }
}

BOOST_AUTO_TEST_SUITE_END()
