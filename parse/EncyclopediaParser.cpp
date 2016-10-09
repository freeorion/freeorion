#include "Label.h"
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
    struct insert_ {
        typedef void result_type;

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
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_r1_type _r1;
            using phoenix::construct;

            article
                =    tok.Article_
                >    parse::label(Name_token)                > tok.string [ _a = _1 ]
                >    parse::label(Category_token)            > tok.string [ _b = _1 ]
                >    parse::label(Short_Description_token)   > tok.string [ _c = _1 ]
                >    parse::label(Description_token)         > tok.string [ _d = _1 ]
                >    parse::label(Icon_token)                > tok.string
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
    bool encyclopedia_articles(Encyclopedia& enc) {
        bool result = true;

        std::vector<boost::filesystem::path> file_list = ListScripts("scripting/encyclopedia");

        for (std::vector<boost::filesystem::path>::iterator file_it = file_list.begin();
             file_it != file_list.end(); ++file_it) 
        {
            result &= detail::parse_file<rules, Encyclopedia>(*file_it, enc);
        }

        return result;
    }
}
