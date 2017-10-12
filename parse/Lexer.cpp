#include "Lexer.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace {
    struct strip_quotes_ {
        typedef std::string result_type;

        std::string operator()(const parse::text_iterator& start, const parse::text_iterator& end) const {
            std::string::const_iterator start_ = start;
            std::string::const_iterator end_ = end;
            return std::string(++start_, --end_);
        }
    };
    const boost::phoenix::function<strip_quotes_> strip_quotes;
}

using namespace parse;

const char* lexer::bool_regex = "(?i:true|false)";
const char* lexer::int_regex = "\\d+";
const char* lexer::double_regex = "\\d+\\.\\d*|\\d*\\.\\d+";
const char* lexer::string_regex = "\\\"[^\\\"]*\\\"";

lexer::lexer() :
    inline_comment("\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/"),
    end_of_line_comment("\\/\\/.*$"),

    bool_(bool_regex),
    int_(int_regex),
    double_(double_regex),
    string(string_regex),

#define DEFINE_TOKEN(r, _, name) BOOST_PP_CAT(name, _)("(?i:" BOOST_PP_STRINGIZE(name) ")"),
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

#define REGISTER_TOKEN(r, _, name)                                    \
    {                                                                 \
        const char* n(BOOST_PP_CAT(name, _token));                    \
        self += BOOST_PP_CAT(name, _) [ _val = n ];                   \
        m_name_tokens[n] = &BOOST_PP_CAT(name, _);                    \
    }
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

const boost::spirit::lex::token_def<const char*>& lexer::name_token(const char* name) const {
    auto it = m_name_tokens.find(name);
    assert(it != m_name_tokens.end());
    return *it->second;
}

namespace boost { namespace spirit { namespace traits {

    // This template specialization is required by Spirit.Lex to automatically
    // convert an iterator pair to an const char* in the lexer.
    void assign_to_attribute_from_iterators<const char*, parse::text_iterator, void>::
    call(const parse::text_iterator& first, const parse::text_iterator& last, const char*& attr) {
        std::string str(first, last);
        boost::algorithm::to_lower(str);
        attr = str.c_str();
    }

    void assign_to_attribute_from_iterators<bool, parse::text_iterator, void>::
    call(const parse::text_iterator& first, const parse::text_iterator& last, bool& attr)
    { attr = *first == 't' || *first == 'T' ? true : false; }

} } }
