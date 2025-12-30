#include "Parse.h"

#include "ParseImpl.h"
#include "EffectParser.h"

#include "../universe/Effect.h"
#include "../universe/FieldType.h"
#include "../util/Directories.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include <boost/phoenix.hpp>

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/scope.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<FieldType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<FieldType>>&) { return os; }
}
#endif

extern "C" BOOST_SYMBOL_EXPORT PyObject* PyInit__fields();

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_fieldtype(std::map<std::string, std::unique_ptr<FieldType>, std::less<>>& fieldtypes,
                          std::string& name, std::string& description,
                          float stealth, std::set<std::string>& tags,
                          boost::optional<parse::effects_group_payload>& effects,
                          std::string& graphic,
                          bool& pass)
    {
        auto fieldtype_ptr = std::make_unique<FieldType>(
            std::string(name), std::move(description), stealth, std::move(tags),
            (effects ? OpenEnvelopes(*effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>{}),
            std::move(graphic));

        fieldtypes.emplace(std::move(name), std::move(fieldtype_ptr));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_fieldtype_, insert_fieldtype, 8)

    using start_rule_payload = std::map<std::string, std::unique_ptr<FieldType>, std::less<>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator first, const parse::text_iterator last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            effects_group_grammar(tok, label, condition_parser, string_grammar),
            tags_parser(tok, label),
            double_rule(tok)
        {
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_pass_type _pass;
            qi::_r1_type _r1;

            field
                = ( tok.FieldType_
                >   label(tok.name_)
                >   tok.string
                >   label(tok.description_)         > tok.string
                >   label(tok.stealth_)             > double_rule
                >   tags_parser
                > -(label(tok.effectsgroups_)       > effects_group_grammar )
                >   label(tok.graphic_)             > tok.string )
                [ ( _pass = is_unique_(_r1, _1, _2),
                    insert_fieldtype_(_r1, _2, _3, _4, _5, _6, _7, _pass) ) ];
                ;

            start
                =   +field(_r1)
                ;

            field.name("FieldType");

#if DEBUG_PARSERS
            debug(field);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using field_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<FieldType>, std::less<>>&)
            >;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        const parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::effects_group_grammar effects_group_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::detail::double_grammar double_rule;
        field_rule          field;
        start_rule          start;
    };

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

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, field_types);

        bool file_success = true;
        py_grammar p = py_grammar(parser, field_types);
        for (const auto& file : ListDir(path, IsFOCPyScript))
            file_success = file_success && py_parse::detail::parse_file<py_grammar>(parser, file, p);

        success = file_success;
        return field_types;
    }
}
