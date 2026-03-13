#include "Parse.h"

#include "../universe/Effect.h"
#include "../universe/FieldType.h"
#include "../util/Directories.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/scope.hpp>


extern "C" BOOST_SYMBOL_EXPORT PyObject* PyInit__fields();

namespace {
    using start_rule_payload = std::map<std::string, std::unique_ptr<FieldType>, std::less<>>;

    struct py_grammar {
        boost::python::dict globals;
        const PythonParser& parser;
        boost::python::object module;
        start_rule_payload& field_types;

        py_grammar(const PythonParser& parser_, start_rule_payload& field_types_) :
            globals(boost::python::import("builtins").attr("__dict__")),
            parser(parser_),
            module(parser_.LoadModule(&PyInit__fields)),
            field_types(field_types_)
        {
            RegisterGlobalsEffects(globals);
            RegisterGlobalsConditions(globals);
            RegisterGlobalsValueRefs(globals, parser);
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

    boost::python::object py_insert_field_type_scoped_(boost::python::object scope, const boost::python::tuple& args,
                                                       const boost::python::dict& kw)
    {
        auto name = boost::python::extract<std::string>(kw["name"])();

        auto description = boost::python::extract<std::string>(kw["description"])();

        auto stealth = boost::python::extract<float>(kw["stealth"])();

        std::set<std::string> tags;
        if (kw.has_key("tags")) {
            boost::python::stl_input_iterator<std::string> tags_begin(kw["tags"]), tags_end;
            tags = std::set<std::string>(tags_begin, tags_end);
        }

        std::vector<std::unique_ptr<Effect::EffectsGroup>> effectsgroups;
        if (kw.has_key("effectsgroups")) {
            boost::python::stl_input_iterator<effect_group_wrapper> effectsgroups_begin(kw["effectsgroups"]), effectsgroups_end;
            for (auto it = effectsgroups_begin; it != effectsgroups_end; ++it) {
                const auto& effects_group = *it->effects_group;
                effectsgroups.push_back(std::make_unique<Effect::EffectsGroup>(
                    ValueRef::CloneUnique(effects_group.Scope()),
                    ValueRef::CloneUnique(effects_group.Activation()),
                    ValueRef::CloneUnique(effects_group.Effects()),
                    effects_group.AccountingLabel(),
                    effects_group.StackingGroup(),
                    effects_group.Priority(),
                    effects_group.GetDescription(),
                    effects_group.TopLevelContent()
                ));
            }
        }

        auto graphic = boost::python::extract<std::string>(kw["graphic"])();

        py_grammar& p = boost::python::extract<py_grammar&>(scope.attr("__grammar"))();

        auto fieldtype_ptr = std::make_unique<FieldType>(
            std::string{name},
            std::move(description),
            stealth,
            std::move(tags),
            std::move(effectsgroups),
            std::move(graphic));

        p.field_types.emplace(std::move(name), std::move(fieldtype_ptr));

        return boost::python::object();
    }

}

BOOST_PYTHON_MODULE(_fields) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::class_<py_grammar, boost::python::bases<>, py_grammar, boost::noncopyable>("__Grammar", boost::python::no_init);

    boost::python::object current_module = boost::python::scope();

    boost::python::def("FieldType", boost::python::raw_function(
        [current_module](const boost::python::tuple& args, const boost::python::dict& kw)
        { return py_insert_field_type_scoped_(current_module, args, kw); }));
}

namespace parse {
    start_rule_payload fields(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        start_rule_payload field_types;

        ScopedTimer timer("Fields Parsing");

        bool file_success = true;
        py_grammar p = py_grammar(parser, field_types);
        for (const auto& file : ListDir(path, IsFOCPyScript))
            file_success = file_success && py_parse::detail::parse_file<py_grammar>(parser, file, p);

        success = file_success;
        return field_types;
    }
}
