#include "Parse.h"

#include "ParseImpl.h"

#include "PythonParserImpl.h"

#include "../universe/Encyclopedia.h"
#include "../util/Directories.h"

#include "PythonParserImpl.h"

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/scope.hpp>

#include <boost/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<EncyclopediaArticle>&) { return os; }
    inline ostream& operator<<(ostream& os, const EncyclopediaArticle&) { return os; }
}
#endif

extern "C" BOOST_SYMBOL_EXPORT PyObject* PyInit__encyclopedia_articles();

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
                const parse::text_iterator first, const parse::text_iterator last) :
            grammar::base_type(start)
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
                >    label(tok.name_)                > tok.string
                >    label(tok.category_)            > tok.string
                >    label(tok.short_description_)   > tok.string
                >    label(tok.description_)         > tok.string
                >    label(tok.icon_)                > tok.string )
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

        using strings_rule = parse::detail::rule<void (ArticleMap&)>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        strings_rule    article;
        start_rule      start;
    };

    struct py_grammar {
        boost::python::dict globals;
        const PythonParser& parser;
        boost::python::object module;
        ArticleMap& articles;

        py_grammar(const PythonParser& parser_, ArticleMap& articles_) :
            globals(boost::python::import("builtins").attr("__dict__")),
            parser(parser_),
            articles(articles_)
        {
            module = parser.LoadModule(&PyInit__encyclopedia_articles);

            module.attr("__grammar") = boost::cref(*this);
        }

        ~py_grammar() {
            parser.UnloadModule(module);
        }

        boost::python::dict operator()() const { return globals; }
    };

    boost::python::object py_insert_encyclopedia_article_scoped_(boost::python::object scope, const boost::python::tuple& args,
                                                       const boost::python::dict& kw)
    {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto category = boost::python::extract<std::string>(kw["category"])();
        auto short_description = boost::python::extract<std::string>(kw["short_description"])();
        auto description = boost::python::extract<std::string>(kw["description"])();
        auto icon = boost::python::extract<std::string>(kw["icon"])();

        py_grammar& p = boost::python::extract<py_grammar&>(scope.attr("__grammar"))();
        p.articles[category].emplace_back(name, category, short_description, description, icon);
        return boost::python::object();
    }
}

BOOST_PYTHON_MODULE(_encyclopedia_articles) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::class_<py_grammar, boost::python::bases<>, py_grammar, boost::noncopyable>("__Grammar", boost::python::no_init);

    boost::python::object current_module = boost::python::scope();

    boost::python::def("Article", boost::python::raw_function(
        [current_module](const boost::python::tuple& args, const boost::python::dict& kw)
        { return py_insert_encyclopedia_article_scoped_(current_module, args, kw); }));

}

namespace parse {
    ArticleMap encyclopedia_articles(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        ArticleMap articles;

        ScopedTimer timer("Encyclopedia Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, ArticleMap>(GetLexer(), file, articles);

        bool file_success = true;
        py_grammar p = py_grammar(parser, articles);
        for (const auto& file : ListDir(path, IsFOCPyScript)) {
            if (!py_parse::detail::parse_file<py_grammar>(parser, file, p)) {
                file_success = false;
            }
        }

        success = file_success;
        return articles;
    }
}
