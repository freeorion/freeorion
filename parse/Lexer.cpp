#include "Lexer.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/spirit/home/phoenix.hpp>


namespace {
    struct strip_quotes_ {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef std::string type; };

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

#define NAME_TOKEN(r, _, name) BOOST_PP_CAT(name, _)("(?i:" BOOST_PP_STRINGIZE(name) ")"),
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_1)
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_2)
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_3)
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_4)
#undef NAME_TOKEN

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
    using boost::phoenix::bind;
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
        |     '.'
        |     ','
        |     '('
        |     ')'
        |     '['
        |     ']'
        ;

#define NAME_TOKEN(r, _, name)                                          \
    {                                                                   \
        adobe::name_t n(BOOST_PP_CAT(name, _name));                     \
        self += BOOST_PP_CAT(name, _) [ _val = n ];                     \
        m_name_tokens[n] = &BOOST_PP_CAT(name, _);                      \
    }
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_1)
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_2)
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_3)
    BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_4)
#undef NAME_TOKEN

    self
        +=    error_token
        ;

    self("WS")
        =    lex::token_def<>("\\s+")
        |    inline_comment
        |    end_of_line_comment
        ;
}

const lexer& lexer::instance() {
    static const lexer retval;
    return retval;
}

const boost::spirit::lex::token_def<adobe::name_t>& lexer::name_token(adobe::name_t name) const {
    std::map<adobe::name_t, boost::spirit::lex::token_def<adobe::name_t>*>::const_iterator it = m_name_tokens.find(name);
    assert(it != m_name_tokens.end());
    return *it->second;
}

namespace boost { namespace spirit { namespace traits {

    // This template specialization is required by Spirit.Lex to automatically
    // convert an iterator pair to an adobe::name_t in the lexer.
    void assign_to_attribute_from_iterators<adobe::name_t, parse::text_iterator, void>::
    call(const parse::text_iterator& first, const parse::text_iterator& last, adobe::name_t& attr) {
        std::string str(first, last);
        boost::algorithm::to_lower(str);
        attr = adobe::name_t(str.c_str());
    }

    void assign_to_attribute_from_iterators<bool, parse::text_iterator, void>::
    call(const parse::text_iterator& first, const parse::text_iterator& last, bool& attr)
    { attr = *first == 't' || *first == 'T' ? true : false; }

} } }
