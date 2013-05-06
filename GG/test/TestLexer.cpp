#include <GG/Lexer.h>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


#define GENERATE_TEST_RESULTS 0

const char* g_input_file = 0;
const char* g_expected_output_file = 0;
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

namespace GG {

#define DUMP_TOK(x) tok.x[print(boost::phoenix::ref(stream), val(#x" -- "), _1, val('\n'))]
#define DUMP_LIT(x) lit(x)[print(boost::phoenix::ref(stream), val("'"), val(x), val("'\n"))]
#define DUMP_UNATTRIBUTED(x) tok.x[print(boost::phoenix::ref(stream), val(#x"\n"))]
#define DUMP_KEYWORD_TOK(x) x[print(boost::phoenix::ref(stream), val("keyword -- "), _1, val('\n'))]

    struct print_
    {
        template <typename Arg1, typename Arg2, typename Arg3 = void, typename Arg4 = void>
        struct result
        { typedef void type; };

        template < typename Arg2>
        void operator()(std::stringstream& stream, Arg2 arg2) const
            { stream << arg2; }

        template <typename Arg2, typename Arg3>
        void operator()(std::stringstream& stream, Arg2 arg2, Arg3  arg3) const
            { stream << arg2 << arg3; }

        template <typename Arg2, typename Arg3, typename Arg4>
        void operator()(std::stringstream& stream, Arg2 arg2, Arg3  arg3, Arg4  arg4) const
            { stream << arg2 << arg3 << arg4; }
    };

    const boost::phoenix::function<print_> print;

    struct adam_lexer_test_grammar :
        boost::spirit::qi::grammar<GG::token_iterator, GG::skipper_type>
    {
        adam_lexer_test_grammar(GG::lexer& tok, std::stringstream& stream_) :
            base_type(start),
            stream(stream_)
            {
                using boost::spirit::qi::_1;
                using boost::spirit::qi::lit;
                using boost::phoenix::val;

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

                start =
                    +(
                        DUMP_KEYWORD_TOK(input)
                      | DUMP_KEYWORD_TOK(output)
                      | DUMP_KEYWORD_TOK(interface)
                      | DUMP_KEYWORD_TOK(logic)
                      | DUMP_KEYWORD_TOK(constant)
                      | DUMP_KEYWORD_TOK(invariant)
                      | DUMP_KEYWORD_TOK(sheet)
                      | DUMP_KEYWORD_TOK(unlink)
                      | DUMP_KEYWORD_TOK(when)
                      | DUMP_KEYWORD_TOK(relate)
                      | DUMP_TOK(identifier)
                      | DUMP_TOK(lead_comment)
                      | DUMP_TOK(trail_comment)
                      | DUMP_TOK(quoted_string)
                      | DUMP_TOK(number)
                      | DUMP_TOK(eq_op)
                      | DUMP_TOK(rel_op)
                      | DUMP_TOK(mul_op)
                      | DUMP_TOK(keyword_true_false)
                      | DUMP_UNATTRIBUTED(keyword_empty)
                      | DUMP_UNATTRIBUTED(define)
                      | DUMP_UNATTRIBUTED(or_)
                      | DUMP_UNATTRIBUTED(and_)
                      | DUMP_LIT('-')
                      | DUMP_LIT('+')
                      | DUMP_LIT('!')
                      | DUMP_LIT('?')
                      | DUMP_LIT(':')
                      | DUMP_LIT('.')
                      | DUMP_LIT(',')
                      | DUMP_LIT('(')
                      | DUMP_LIT(')')
                      | DUMP_LIT('[')
                      | DUMP_LIT(']')
                      | DUMP_LIT('{')
                      | DUMP_LIT('}')
                      | DUMP_LIT('@')
                      | DUMP_LIT(';')
                    )
                    ;
            }

        boost::spirit::qi::rule<GG::token_iterator, GG::skipper_type> start;

        std::stringstream& stream;
    };

    struct eve_lexer_test_grammar :
        boost::spirit::qi::grammar<GG::token_iterator, GG::skipper_type>
    {
        eve_lexer_test_grammar(GG::lexer& tok, std::stringstream& stream_) :
            base_type(start),
            stream(stream_)
            {
                using boost::spirit::qi::_1;
                using boost::spirit::qi::lit;
                using boost::phoenix::val;

                assert(tok.keywords.size() == 4u);
                const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
                const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
                const boost::spirit::lex::token_def<adobe::name_t>& layout = tok.keywords[layout_k];
                const boost::spirit::lex::token_def<adobe::name_t>& view = tok.keywords[view_k];
                assert(tok.keywords.size() == 4u);

                start =
                    +(
                        DUMP_KEYWORD_TOK(interface)
                      | DUMP_KEYWORD_TOK(constant)
                      | DUMP_KEYWORD_TOK(layout)
                      | DUMP_KEYWORD_TOK(view)
                      | DUMP_TOK(identifier)
                      | DUMP_TOK(lead_comment)
                      | DUMP_TOK(trail_comment)
                      | DUMP_TOK(quoted_string)
                      | DUMP_TOK(number)
                      | DUMP_TOK(eq_op)
                      | DUMP_TOK(rel_op)
                      | DUMP_TOK(mul_op)
                      | DUMP_TOK(keyword_true_false)
                      | DUMP_UNATTRIBUTED(keyword_empty)
                      | DUMP_UNATTRIBUTED(define)
                      | DUMP_UNATTRIBUTED(or_)
                      | DUMP_UNATTRIBUTED(and_)
                      | DUMP_LIT('-')
                      | DUMP_LIT('+')
                      | DUMP_LIT('!')
                      | DUMP_LIT('?')
                      | DUMP_LIT(':')
                      | DUMP_LIT('.')
                      | DUMP_LIT(',')
                      | DUMP_LIT('(')
                      | DUMP_LIT(')')
                      | DUMP_LIT('[')
                      | DUMP_LIT(']')
                      | DUMP_LIT('{')
                      | DUMP_LIT('}')
                      | DUMP_LIT('@')
                      | DUMP_LIT(';')
                    )
                    ;
            }

        boost::spirit::qi::rule<GG::token_iterator, GG::skipper_type> start;

        std::stringstream& stream;
    };

#undef DUMP_TOK
#undef DUMP_LIT
#undef DUMP_UNATTRIBUTED
#undef DUMP_KEYWORD_TOK

}

BOOST_AUTO_TEST_CASE( adam_lexer )
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

    GG::lexer lexer(s_keywords, s_keywords + s_num_keywords);
    std::stringstream stream;
    GG::adam_lexer_test_grammar test_grammar(lexer, stream);

    const std::string str = read_file(g_input_file);

    GG::text_iterator it(str.begin());
    GG::token_iterator iter = lexer.begin(it, GG::text_iterator(str.end()));
    GG::token_iterator end = lexer.end();

    boost::spirit::qi::phrase_parse(iter,
                                    end,
                                    test_grammar,
                                    boost::spirit::qi::in_state("WS")[lexer.self]);

    std::cout << stream.str();

#if GENERATE_TEST_RESULTS
    std::ofstream ofs(g_expected_output_file);
    ofs << stream.str();
#else
    std::string expected_results = read_file(g_expected_output_file);
    BOOST_CHECK_EQUAL(stream.str(), expected_results);
#endif
}

BOOST_AUTO_TEST_CASE( eve_lexer )
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

    GG::lexer lexer(s_keywords, s_keywords + s_num_keywords);
    std::stringstream stream;
    GG::eve_lexer_test_grammar test_grammar(lexer, stream);

    const std::string str = read_file(g_input_file);

    GG::text_iterator it(str.begin());
    GG::token_iterator iter = lexer.begin(it, GG::text_iterator(str.end()));
    GG::token_iterator end = lexer.end();

    boost::spirit::qi::phrase_parse(iter,
                                    end,
                                    test_grammar,
                                    boost::spirit::qi::in_state("WS")[lexer.self]);

    std::cout << stream.str();

#if GENERATE_TEST_RESULTS
    std::ofstream ofs(g_expected_output_file);
    ofs << stream.str();
#else
    std::string expected_results = read_file(g_expected_output_file);
    BOOST_CHECK_EQUAL(stream.str(), expected_results);
#endif
}

// Most of this is boilerplate cut-and-pasted from Boost.Test.  We need to
// select which test(s) to do, so we can't use it here unmodified.

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
bool init_unit_test()                   {
#else
::boost::unit_test::test_suite*
init_unit_test_suite( int, char* argv[] )   {
#endif

#if !defined(BOOST_TEST_DYN_LINK)
    g_input_file = argv[1];
    g_expected_output_file = argv[2];
    g_test_type = (g_expected_output_file[0] == 'a') ? AdamTest : EveTest;
#endif

#ifdef BOOST_TEST_MODULE
    using namespace ::boost::unit_test;
    assign_op( framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE( BOOST_TEST_MODULE ).trim( "\"" ), 0 );
    
#endif

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    return true;
#else
    return 0;
#endif
}

#if defined(BOOST_TEST_DYN_LINK)
int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    g_input_file = argv[1];
    g_expected_output_file = argv[2];
    g_test_type = (g_expected_output_file[0] == 'a') ? AdamTest : EveTest;
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
#endif

