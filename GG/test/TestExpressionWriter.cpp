#include <GG/ExpressionParser.h>

#include <GG/ExpressionWriter.h>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/implementation/adam_parser_impl.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


const char* g_input_file = 0;
enum TestType
{
    AdamTest,
    EveTest
} g_test_type;

adobe::aggregate_name_t input_k      = { "input" };
adobe::aggregate_name_t output_k     = { "output" };
adobe::aggregate_name_t interface_k  = { "interface" };
adobe::aggregate_name_t logic_k      = { "logic" };
adobe::aggregate_name_t constant_k   = { "constant" };
adobe::aggregate_name_t invariant_k  = { "invariant" };
adobe::aggregate_name_t sheet_k      = { "sheet" };
adobe::aggregate_name_t unlink_k     = { "unlink" };
adobe::aggregate_name_t when_k       = { "when" };
adobe::aggregate_name_t relate_k     = { "relate" };
adobe::aggregate_name_t layout_k     = { "layout" };
adobe::aggregate_name_t view_k       = { "view" };

namespace adobe {

    bool eve_keyword_lookup(const name_t& name)
    {
        typedef boost::array<adobe::name_t, 4> keyword_table_t;

        keyword_table_t keyword_table =
        {{
            interface_k,
            constant_k,
            layout_k,
            view_k
        }};

        sort(keyword_table);

        keyword_table_t::const_iterator iter(lower_bound(keyword_table, name));

        return (iter != keyword_table.end() && *iter == name);
    }

    array_t parse_eve_expression(const std::string& str_expression)
    {
        std::stringstream expression_stream(str_expression);

        expression_parser parser(expression_stream, line_position_t("expression"));
        parser.set_keyword_extension_lookup(boost::bind(&eve_keyword_lookup, _1));

        array_t expression;
        parser.require_expression(expression);

        return expression;
    }

}

typedef adobe::array_t (*AdobeParserType)(const std::string&);

namespace GG {

    bool TestExpression(const lexer& lexer_,
                        const expression_parser_rules& parser_rules,
                        adobe::array_t& new_parsed_expression,
                        AdobeParserType adobe_parse,
                        const std::string& expression)
    {
        std::cout << "expression: \"" << expression << "\"\n";
        adobe::array_t original_parsed_expression;
        bool original_parse_failed = false;
        try {
            original_parsed_expression = adobe_parse(expression);
        } catch (const adobe::stream_error_t&) {
            original_parse_failed = true;
        }
        std::cout << "original: <parse " << (original_parse_failed ? "failure" : "success") << ">\n";
        using boost::spirit::qi::phrase_parse;
        text_iterator it(expression.begin());
        detail::s_text_it = &it;
        detail::s_begin = it;
        detail::s_end = text_iterator(expression.end());
        detail::s_filename = "test_expression";
        token_iterator iter = lexer_.begin(it, detail::s_end);
        token_iterator end = lexer_.end();
        bool new_parse_failed =
            !phrase_parse(iter,
                          end,
                          parser_rules.expression(boost::phoenix::ref(new_parsed_expression)),
                          boost::spirit::qi::in_state("WS")[lexer_.self]);
        std::cout << "new:      <parse " << (new_parse_failed ? "failure" : "success") << ">\n";
        bool pass =
            original_parse_failed && new_parse_failed ||
            new_parsed_expression == original_parsed_expression;

        if (pass) {
            std::string rewritten_expression = GG::WriteExpression(new_parsed_expression);
            std::cout << "Rewrite:    " << rewritten_expression << '\n';
            try {
                adobe::array_t round_trip_parsed_expression = adobe_parse(rewritten_expression);
                pass &= round_trip_parsed_expression == new_parsed_expression;
            } catch (const adobe::stream_error_t&) {
                pass = new_parsed_expression.empty();
            }
        }

        std::cout << "Round-trip parse: " << (pass ? "PASS" : "FAIL") << '\n';

        std::cout << "\n";
        new_parsed_expression.clear();

        return pass;
    }

}

BOOST_AUTO_TEST_CASE( adam_expression_parser )
{
    if (g_test_type != AdamTest)
        return;

    static const adobe::name_t s_keywords[] = {
        input_k,
        output_k,
        interface_k,
        logic_k,
        constant_k,
        invariant_k,
        sheet_k,
        unlink_k,
        when_k,
        relate_k
    };
    const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

    GG::lexer tok(s_keywords, s_keywords + s_num_keywords);

    using boost::spirit::qi::token;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    assert(tok.keywords.size() == 10u);
    const boost::spirit::lex::token_def<adobe::name_t>& input = tok.keywords[input_k];
    const boost::spirit::lex::token_def<adobe::name_t>& output = tok.keywords[output_k];
    const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
    const boost::spirit::lex::token_def<adobe::name_t>& logic = tok.keywords[logic_k];
    const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& invariant = tok.keywords[invariant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& sheet = tok.keywords[sheet_k];
    const boost::spirit::lex::token_def<adobe::name_t>& unlink = tok.keywords[unlink_k];
    const boost::spirit::lex::token_def<adobe::name_t>& when = tok.keywords[when_k];
    const boost::spirit::lex::token_def<adobe::name_t>& relate = tok.keywords[relate_k];
    assert(tok.keywords.size() == 10u);

    static GG::expression_parser_rules::keyword_rule adam_keywords =
        input[_val = _1]
      | output[_val = _1]
      | interface[_val = _1]
      | logic[_val = _1]
      | constant[_val = _1]
      | invariant[_val = _1]
      | sheet[_val = _1]
      | unlink[_val = _1]
      | when[_val = _1]
      | relate[_val = _1]
        ;
    adam_keywords.name("keyword");

    adobe::array_t stack;
    GG::expression_parser_rules parser_rules(tok, adam_keywords);

    std::string expressions_file_contents = read_file(g_input_file);
    std::vector<std::string> expressions;
    using boost::algorithm::split;
    using boost::algorithm::is_any_of;
    split(expressions, expressions_file_contents, is_any_of("\n"));

    std::size_t passes = 0;
    std::size_t failures = 0;
    for (std::size_t i = 0; i < expressions.size(); ++i) {
        if (!expressions[i].empty()) {
            bool success =
                TestExpression(tok, parser_rules, stack, &adobe::parse_adam_expression, expressions[i]);
            BOOST_CHECK(success);
            if (success)
                ++passes;
            else
                ++failures;
        }
    }

    std::cout << "Summary: " << passes << " passed, " << failures << " failed\n";
}

BOOST_AUTO_TEST_CASE( eve_expression_parser )
{
    if (g_test_type != EveTest)
        return;

    static const adobe::name_t s_keywords[] = {
        interface_k,
        constant_k,
        layout_k,
        view_k
    };
    const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

    GG::lexer tok(s_keywords, s_keywords + s_num_keywords);

    using boost::spirit::qi::token;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    assert(tok.keywords.size() == 4u);
    const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
    const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& layout = tok.keywords[layout_k];
    const boost::spirit::lex::token_def<adobe::name_t>& view = tok.keywords[view_k];
    assert(tok.keywords.size() == 4u);

    static GG::expression_parser_rules::keyword_rule eve_keywords =
        interface[_val = _1]
      | constant[_val = _1]
      | layout[_val = _1]
      | view[_val = _1]
        ;
    eve_keywords.name("keyword");

    adobe::array_t stack;
    GG::expression_parser_rules parser_rules(tok, eve_keywords);

    std::string expressions_file_contents = read_file(g_input_file);
    std::vector<std::string> expressions;
    using boost::algorithm::split;
    using boost::algorithm::is_any_of;
    split(expressions, expressions_file_contents, is_any_of("\n"));

    std::size_t passes = 0;
    std::size_t failures = 0;
    for (std::size_t i = 0; i < expressions.size(); ++i) {
        if (!expressions[i].empty()) {
            bool success =
                TestExpression(tok, parser_rules, stack, &adobe::parse_eve_expression, expressions[i]);
            BOOST_CHECK(success);
            if (success)
                ++passes;
            else
                ++failures;
        }
    }

    std::cout << "Summary: " << passes << " passed, " << failures << " failed\n";
}

// Most of this is boilerplate cut-and-pasted from Boost.Test.  We need to
// select which test(s) to do, so we can't use it here unmodified.

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
bool init_unit_test()                   {
#else
::boost::unit_test::test_suite*
init_unit_test_suite( int, char* [] )   {
#endif

#ifdef BOOST_TEST_MODULE
    using namespace ::boost::unit_test;
    assign_op( framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE( BOOST_TEST_MODULE ).trim( "\"" ), 0 );
    
#endif

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    return true;
}
#else
    return 0;
}
#endif

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    g_input_file = argv[1];
    g_test_type = (argv[2][0] == 'a') ? AdamTest : EveTest;
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
