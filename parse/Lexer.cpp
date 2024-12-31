#include "Lexer.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/phoenix.hpp>


namespace {
    struct strip_quotes_ {
        typedef std::string result_type;

        std::string operator()(parse::text_iterator start, parse::text_iterator end) const
        { return std::string(++start, --end); }
    };
    const boost::phoenix::function<strip_quotes_> strip_quotes;
}

using namespace parse;

lexer::lexer() :
    inline_comment("\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/"),
    end_of_line_comment("\\/\\/.*$"),

    bool_(bool_regex),
    int_(int_regex),
    double_(double_regex),
    string(string_regex),

#define DEFINE_TOKEN(r, _, name) BOOST_PP_CAT(name, _)("(?-i:" BOOST_PP_STRINGIZE(name) ")"),
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_1)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_2)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_3)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_4)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_5)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_6)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_7)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_8)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_9)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_10)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_11)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_12)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_13)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_14)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_15)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_16)
    BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_17)
#undef DEFINE_TOKEN

    error_token("\\S+?")

    // TODO: Is Design used for ShipDesign? (if so, replace)
    // TODO: Get rid of underscore in Lookup_Strings.
    // TODO: Can we replace OwnedBy with Owner? (if so, replace)
    // TODO: Can PlanetEnvironment be replaced with Environment?
    // TODO: Get rid of underscore in Short_Description.
    // TODO: Size is used for PlanetSize (so replace).
{
    namespace lex = boost::spirit::lex;

    lex::_end_type _end;
    lex::_start_type _start;
    lex::_val_type _val;
    using boost::phoenix::construct;

    self
        +=    bool_
        |     int_
        |     double_
        |     string [ _val = strip_quotes(_start, _end) ]
        |     '='
        |     '+'
        |     '-'
        |     '*'
        |     '/'
        |     '^'
        |     '%'
        |     '.'
        |     ','
        |     '('
        |     ')'
        |     '['
        |     ']'
        |     '>'
        |     '<'
        |     '!'
        |     ':'
        |     '?'
        ;

#define REGISTER_TOKEN(r, _, name) self += BOOST_PP_CAT(name, _);
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_1)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_2)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_3)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_4)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_5)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_6)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_7)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_8)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_9)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_10)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_11)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_12)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_13)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_14)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_15)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_16)
    BOOST_PP_SEQ_FOR_EACH(REGISTER_TOKEN, _, TOKEN_SEQ_17)
#undef REGISTER_TOKEN

    self
        +=    error_token
        ;

    self("WS")
        =    lex::token_def<>("\\s+")
        |    inline_comment
        |    end_of_line_comment
        ;
}
