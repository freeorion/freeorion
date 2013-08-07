#include "test.h"

struct quote_
{
    template <typename Arg>
    struct result
    { typedef std::string type; };

    std::string operator()(const std::string& arg1) const
        { return '"' + arg1 + '"'; }
};
const boost::phoenix::function<quote_> quote;

struct remove_
{
    template <typename Arg>
    struct result
    { typedef void type; };

    void operator()(const adobe::name_t& arg1) const
        { lexer_test_rules::unchecked_tokens.erase(arg1); }
};
const boost::phoenix::function<remove_> do_erase;

std::set<adobe::name_t> lexer_test_rules::unchecked_tokens;

lexer_test_rules::lexer_test_rules()
{
    namespace qi = boost::spirit::qi;
    namespace phoenix = boost::phoenix;
    qi::_1_type _1;
    qi::char_type char_;
    qi::as_string_type as_string;
    using phoenix::val;

    const parse::lexer& tok = parse::lexer::instance();

    lexer = *(lexer_1 | lexer_2 | lexer_3 | lexer_4 | lexer_5);

    lexer_1
        =  as_string[ tok.bool_ ] [ std::cout << _1 << "\n" ]
        |  as_string[ tok.int_ ] [ std::cout << _1 << "\n" ]
        |  as_string[ tok.double_ ] [ std::cout << _1 << "\n" ]
        |  tok.string [ std::cout << quote(_1) << "\n" ]
        |  char_('=') [ std::cout << _1 << "\n" ]
        |  char_('+') [ std::cout << _1 << "\n" ]
        |  char_('-') [ std::cout << _1 << "\n" ]
        |  char_('*') [ std::cout << _1 << "\n" ]
        |  char_('/') [ std::cout << _1 << "\n" ]
        |  char_('.') [ std::cout << _1 << "\n" ]
        |  char_(',') [ std::cout << _1 << "\n" ]
        ;

#define ADD_TOKEN(r, _, name) \
    | tok.BOOST_PP_CAT(name, _) [ std::cout << _1 << "\n", do_erase(BOOST_PP_CAT(name, _token)) ]

    lexer_2
        =  char_('[') [ std::cout << _1 << "\n" ]
        BOOST_PP_SEQ_FOR_EACH(ADD_TOKEN, _, TOKEN_SEQ_1)
        ;

    lexer_3
        =  char_(']') [ std::cout << _1 << "\n" ]
        BOOST_PP_SEQ_FOR_EACH(ADD_TOKEN, _, TOKEN_SEQ_2)
        ;

    lexer_4
        =  char_('(') [ std::cout << _1 << "\n" ]
        BOOST_PP_SEQ_FOR_EACH(ADD_TOKEN, _, TOKEN_SEQ_3)
        ;

    lexer_5
        =  char_(')') [ std::cout << _1 << "\n" ]
        BOOST_PP_SEQ_FOR_EACH(ADD_TOKEN, _, TOKEN_SEQ_4)
        ;

#undef ADD_TOKEN

#define ADD_UNCHECKED_TOKENS(r, _, name) \
    unchecked_tokens.insert(BOOST_PP_CAT(name, _token));

        BOOST_PP_SEQ_FOR_EACH(ADD_UNCHECKED_TOKENS, _, TOKEN_SEQ_1)
        BOOST_PP_SEQ_FOR_EACH(ADD_UNCHECKED_TOKENS, _, TOKEN_SEQ_2)
        BOOST_PP_SEQ_FOR_EACH(ADD_UNCHECKED_TOKENS, _, TOKEN_SEQ_3)
        BOOST_PP_SEQ_FOR_EACH(ADD_UNCHECKED_TOKENS, _, TOKEN_SEQ_4)
#undef ADD_UNCHECKED_TOKENS

#define NAME(x)
// x.name(#x); debug(x)
    NAME(lexer);
#undef NAME
}
