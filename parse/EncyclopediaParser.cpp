#include "Parse.h"

#include "ParseImpl.h"

#include "../universe/Encyclopedia.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<EncyclopediaArticle>&) { return os; }
    inline ostream& operator<<(ostream& os, const EncyclopediaArticle&) { return os; }
}
#endif

namespace {
    using ArticleMap = std::map<std::string, std::vector<EncyclopediaArticle>>;

    struct insert_ {
        typedef void result_type;

        void operator()(ArticleMap& articles, const EncyclopediaArticle& article) const
        { articles[article.category].push_back(article); }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_r1_type _r1;

            const parse::lexer& tok = parse::lexer::instance();

            article
                =    tok.Article_
                >    parse::detail::label(Name_token)                > tok.string [ _a = _1 ]
                >    parse::detail::label(Category_token)            > tok.string [ _b = _1 ]
                >    parse::detail::label(Short_Description_token)   > tok.string [ _c = _1 ]
                >    parse::detail::label(Description_token)         > tok.string [ _d = _1 ]
                >    parse::detail::label(Icon_token)                > tok.string
                    [ insert(_r1, construct<EncyclopediaArticle>(_a, _b, _c, _d, _1)) ]
                ;

            start
                =   +article(_r1)
                ;

            article.name("EncyclopediaArticle");

#if DEBUG_PARSERS
            debug(article);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (ArticleMap&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::string,
                std::string
            >
        > strings_rule;

        typedef parse::detail::rule<
            void (ArticleMap&)
        > start_rule;


        strings_rule    article;
        start_rule      start;
    };
}

namespace parse {
    ArticleMap encyclopedia_articles() {
        std::vector<boost::filesystem::path> file_list = ListScripts("scripting/encyclopedia");

        ArticleMap articles;
        for (const boost::filesystem::path& file : file_list) {
            /*auto success =*/ detail::parse_file<rules, ArticleMap>(file, articles);
        }

        return articles;
    }
}
