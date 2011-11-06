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

lexer_test_rules::lexer_test_rules()
{
    namespace qi = boost::spirit::qi;
    namespace phoenix = boost::phoenix;
    qi::_1_type _1;
    qi::char_type char_;
    qi::as_string_type as_string;
    using phoenix::val;

    const parse::lexer& tok = parse::lexer::instance();

    lexer = *(lexer_1 | lexer_2 | lexer_3);

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
        |  char_('(') [ std::cout << _1 << "\n" ]
        |  char_(')') [ std::cout << _1 << "\n" ]
        ;

    lexer_2
        =  char_('[') [ std::cout << _1 << "\n" ]
#define NAME_TOKEN(r, _, name) | tok.BOOST_PP_CAT(name, _) [ std::cout << _1 << "\n" ]
        BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_1)
#undef NAME_TOKEN
        ;

    lexer_3
        =  char_(']') [ std::cout << _1 << "\n" ]
#define NAME_TOKEN(r, _, name) | tok.BOOST_PP_CAT(name, _) [ std::cout << _1 << "\n" ]
        BOOST_PP_SEQ_FOR_EACH(NAME_TOKEN, _, NAMES_SEQ_2)
#undef NAME_TOKEN
        ;

#define NAME(x)
// x.name(#x); debug(x)
    NAME(lexer);
#undef NAME
}
