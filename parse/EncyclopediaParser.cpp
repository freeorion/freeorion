#include "Parse.h"

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


extern "C" BOOST_SYMBOL_EXPORT PyObject* PyInit__encyclopedia_articles();

namespace {
    using ArticleMap = Encyclopedia::ArticleMap;

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
