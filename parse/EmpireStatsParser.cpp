#include "Parse.h"

#include "../util/Directories.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/scope.hpp>

extern "C" BOOST_SYMBOL_EXPORT PyObject* PyInit__empire_statistics();

namespace {
    using start_rule_payload = std::map<std::string, std::unique_ptr<ValueRef::ValueRef<double>>>;

    boost::python::object py_insert_empire_statistics_scoped_(boost::python::object scope, const boost::python::tuple& args,
                                                       const boost::python::dict& kw);

    struct py_grammar {
        boost::python::dict globals;
        const PythonParser& parser;
        boost::python::object module;
        start_rule_payload& stats;

        py_grammar(const PythonParser& parser_, start_rule_payload& stats_) :
            globals(boost::python::import("builtins").attr("__dict__")),
            parser(parser_),
            module(parser_.LoadModule(&PyInit__empire_statistics)),
            stats(stats_)
        {
            RegisterGlobalsConditions(globals);
            RegisterGlobalsValueRefs(globals, parser);
            RegisterGlobalsSources(globals);
            RegisterGlobalsEnums(globals);

            parser.LoadValueRefsModule();

            module.attr("__stats") = boost::cref(*this);
        }

        ~py_grammar() {
            parser.UnloadModule(module);
        }

        boost::python::dict operator()() const { return globals; }
    };

    boost::python::object py_insert_empire_statistics_scoped_(boost::python::object scope, const boost::python::tuple& args,
                                                       const boost::python::dict& kw)
    {
        auto name = boost::python::extract<std::string>(kw["name"])();

        auto value = pyobject_to_vref_or_cast<double, int>(kw["value"]);

        py_grammar& p = boost::python::extract<py_grammar&>(scope.attr("__stats"))();

        p.stats.emplace(std::move(name), std::move(value));

        return boost::python::object();
    }
}

BOOST_PYTHON_MODULE(_empire_statistics) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::class_<py_grammar, boost::python::bases<>, py_grammar, boost::noncopyable>("__Grammar", boost::python::no_init);

    boost::python::object current_module = boost::python::scope();

    boost::python::def("EmpireStatistic", boost::python::raw_function(
        [current_module](const boost::python::tuple& args, const boost::python::dict& kw)
        { return py_insert_empire_statistics_scoped_(current_module, args, kw); }));
}

namespace parse {
    start_rule_payload statistics(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        start_rule_payload all_stats;

        bool file_success = true;
        for (const auto& file : ListDir(path, IsFOCPyScript)) {
            start_rule_payload stats_;
            py_grammar p = py_grammar(parser, stats_);
            if (py_parse::detail::parse_file<py_grammar>(parser, file, p)) {
                for (auto& stat : stats_) {
                    auto maybe_inserted = all_stats.emplace(stat.first, std::move(stat.second));
                    if (!maybe_inserted.second) {
                        WarnLogger() << "Addition of second statistic with name " << maybe_inserted.first->first
                                     << " failed.  Keeping first statistic found.";
                    }
                }
            } else {
                file_success = false;
            }
        }

        success = file_success;
        return all_stats;
    }
}
