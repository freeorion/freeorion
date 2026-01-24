#include "Parse.h"
#include "NamedValueRefPythonParser.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/scope.hpp>

namespace {
    DeclareThreadSafeLogger(parsing);

    using start_rule_payload = std::map<std::string, std::unique_ptr<ValueRef::ValueRefBase>, std::less<>>;

    boost::python::object py_insert_named_value_definitions_(boost::python::object scope, const boost::python::tuple& args,
                                             const boost::python::dict& kw)
    {
        // actually there is no payload, calling NamedInteger, NamedReal... registers the named values
        // we could add the names for the vrefs named via python or similar
        // we should not introduce a different syntax, so moving expressions is frictionless
        return boost::python::object();
    }

    struct py_grammar {
        boost::python::dict globals;
        const PythonParser& parser;
        boost::python::object module;
        start_rule_payload& named_value_definitions;

        py_grammar(const PythonParser& parser_, start_rule_payload& named_value_definitions_) :
            globals(boost::python::import("builtins").attr("__dict__")),
            parser(parser_),
            module(parser_.LoadModule(&PyInit__named_values)),
          named_value_definitions(named_value_definitions_)
      {
            RegisterGlobalsEffects(globals);
            RegisterGlobalsConditions(globals);
            RegisterGlobalsValueRefs(globals, parser_);
            RegisterGlobalsSources(globals);
            RegisterGlobalsEnums(globals);

            parser.LoadValueRefsModule();
            parser.LoadEffectsModule();

            module.attr("__grammar") = boost::cref(*this);
        }

        ~py_grammar() {
            parser.UnloadModule(module);
        }

        boost::python::dict operator()() const { return globals; }
    };
}


BOOST_PYTHON_MODULE(_named_values) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::class_<py_grammar, boost::python::bases<>, py_grammar, boost::noncopyable>("__Grammar", boost::python::no_init);

    boost::python::object current_module = boost::python::scope();

    boost::python::def("NamedValuesPyFile", boost::python::raw_function(
        [current_module](const boost::python::tuple& args, const boost::python::dict& kw)
        { return py_insert_named_value_definitions_(current_module, args, kw); }));
}


namespace parse {
    start_rule_payload named_value_refs_py(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        start_rule_payload named_value_definitions;

        ScopedTimer timer("NamedValue Python Parsing");

        bool file_success = true;
        py_grammar p = py_grammar(parser, named_value_definitions);
        for (const auto& file : ListDir(path, IsFOCPyScript))
            file_success = py_parse::detail::parse_file<py_grammar>(parser, file, p) && file_success;

        TraceLogger(parsing) << "Start parsing FOCS for NamedValue definitions: " << named_value_definitions.size();
        for (auto& [name, def] : named_value_definitions)
            TraceLogger(parsing) << "ValueRef::ValueRefBase " << name << " : " << def->GetCheckSum() << "\n" << def->Dump();
        TraceLogger(parsing) << "End parsing FOCS.py for NamedValue definitions" << named_value_definitions.size();

        success = file_success;
        return named_value_definitions;
    }
}
