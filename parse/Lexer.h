#ifndef _Lexer_h_
#define _Lexer_h_

#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_lexertl_position_token.hpp>

#include "Tokens.h"

#include <unordered_map>

/** \namespace parse \brief The namespace that encloses the script file lexer
    and parser. */
namespace parse {

/** The type of iterator used by the script file lexer. */
typedef std::string::const_iterator text_iterator;

/** The type of token used by the script file lexer. */
typedef boost::spirit::lex::lexertl::position_token<
    text_iterator,
    boost::mpl::vector<
        bool,
        int,
        double,
        std::string
    >
> token_type;

typedef boost::spirit::lex::lexertl::actor_lexer<token_type> spirit_lexer_base_type;

/** The script file lexer. */
struct lexer :
    boost::spirit::lex::lexer<spirit_lexer_base_type>
{
    /** Ctor. */
    lexer();

    /** \name Comment tokens */ ///@{
    boost::spirit::lex::token_def<boost::spirit::lex::omit> inline_comment;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> end_of_line_comment;
    //@}

    using string_token_def = boost::spirit::lex::token_def<std::string>;

    /** \name Tokens for common C++ types and builtins. */ ///@{
    boost::spirit::lex::token_def<bool> bool_;
    boost::spirit::lex::token_def<int> int_;
    boost::spirit::lex::token_def<double> double_;
    boost::spirit::lex::token_def<std::string> string;
    //@}

    /** \name Keyword tokens.  These should be kept in lexicographically
        sorted order, so that finding, adding, and removing tokens is a bit
        easier.  See the note above the Enum tokens section. */ ///@{
#define DECLARE_TOKEN(r, _, name) string_token_def BOOST_PP_CAT(name, _);
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_1)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_2)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_3)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_4)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_5)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_6)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_7)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_8)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_9)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_10)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_11)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_12)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_13)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_14)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_15)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_16)
    BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_17)
#undef DECLARE_TOKEN
    //@}

    /** \name Error token. */ ///@{
    boost::spirit::lex::token_def<boost::spirit::lex::omit> error_token;
    //@}

    static const char* bool_regex;
    static const char* int_regex;
    static const char* double_regex;
    static const char* string_regex;

private:
};

/** The type of iterator passed to the script file parser by the script file
    lexer. */
typedef lexer::iterator_type token_iterator;

typedef lexer::lexer_def lexer_def;

/** The type of the skip-parser, defined in the script file lexer, used by the
    script file parser iterator. */
typedef boost::spirit::qi::in_state_skipper<lexer_def> skipper_type;

}

namespace boost { namespace spirit { namespace traits {

    // If you want to create a token with a custom value type, you must
    // declare the conversion handler here, and define it in the .cpp file.
} } }

#endif
