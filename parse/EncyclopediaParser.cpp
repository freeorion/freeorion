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
    using ArticleMap = Encyclopedia::ArticleMap;

    struct insert_ {
        typedef void result_type;

        void operator()(ArticleMap& articles, const EncyclopediaArticle& article) const
        { articles[article.category].push_back(article); }
    };
    const boost::phoenix::function<insert_> insert;

    using start_rule_payload = ArticleMap;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_r1_type _r1;
            qi::omit_type omit_;

            article
                =  ( omit_[tok.Article_]
                >    labeller.rule(Name_token)                > tok.string
                >    labeller.rule(Category_token)            > tok.string
                >    labeller.rule(Short_Description_token)   > tok.string
                >    labeller.rule(Description_token)         > tok.string
                >    labeller.rule(Icon_token)                > tok.string )
                    [ insert(_r1, construct<EncyclopediaArticle>(_1, _2, _3, _4, _5)) ]
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

        using  strings_rule = parse::detail::rule<void (ArticleMap&)>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller labeller;
        strings_rule    article;
        start_rule      start;
    };
}

namespace parse {
    ArticleMap encyclopedia_articles(const boost::filesystem::path& path) {
        const lexer lexer;
        std::vector<boost::filesystem::path> file_list = ListScripts(path);

        ArticleMap articles;
        for (const boost::filesystem::path& file : file_list) {
            /*auto success =*/ detail::parse_file<grammar, ArticleMap>(lexer, file, articles);
        }

        return articles;
    }
}
