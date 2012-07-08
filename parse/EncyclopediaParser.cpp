#include "ParseImpl.h"

#include "Label.h"
#include "../UI/Encyclopedia.h"

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<EncyclopediaArticle>&) { return os; }
    inline ostream& operator<<(ostream& os, const EncyclopediaArticle&) { return os; }
}
#endif

namespace {
    struct insert_ {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(Encyclopedia& enc, const EncyclopediaArticle& article) const
        { enc.articles[article.category].push_back(article); }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_a_type _c;
            qi::_b_type _d;
            qi::_r1_type _r1;
            using phoenix::construct;

            article
                =    tok.Article_
                >>   parse::label(Name_name)                >> tok.string [ _a = _1 ]
                >>   parse::label(Category_name)            >> tok.string [ _b = _1 ]
                >>   parse::label(Short_Description_name)   >> tok.string [ _c = _1 ]
                >>   parse::label(Description_name)         >> tok.string [ _d = _1 ]
                >>   parse::label(Icon_name)                >> tok.string
                    [ insert(_r1, construct<EncyclopediaArticle>(_a, _b, _c, _d, _1)) ]
                ;

            start
                =   +article(_r1)
                ;

            article.name("EncyclopediaArticle");

#if DEBUG_PARSERS
            debug(article);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (Encyclopedia&),
            qi::locals<
                std::string,
                std::string,
                std::string,
                std::string
            >,
            parse::skipper_type
        > strings_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (Encyclopedia&),
            parse::skipper_type
        > start_rule;


        strings_rule    article;
        start_rule      start;
    };
}

namespace parse {
    bool encyclopedia_articles(const boost::filesystem::path& path, Encyclopedia& enc)
    { return detail::parse_file<rules, Encyclopedia>(path, enc); }
}
