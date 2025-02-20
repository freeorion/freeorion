#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParamsParser.h"

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

#include <boost/python/import.hpp>
#include <boost/python/raw_function.hpp>

namespace {
    DeclareThreadSafeLogger(parsing);
}

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<BuildingType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<BuildingType>>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_building(std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>& building_types,
                         std::string& name, std::string& description,
                         parse::detail::MovableEnvelope<CommonParams>& common_params,
                         CaptureResult& capture_result, std::string& icon,
                         bool& pass)
    {
        auto building_type = std::make_unique<BuildingType>(
            std::string(name), std::move(description),
            std::move(*common_params.OpenEnvelope(pass)),
            capture_result, std::move(icon));

        building_types.emplace(std::move(name), std::move(building_type));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_building_, insert_building, 7)

    using start_rule_payload = std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator first,
                const parse::text_iterator last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            tags_parser(tok, label),
            common_rules(tok, label, condition_parser, string_grammar, tags_parser),
            capture_result_enum(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::_r1_type _r1;
            qi::eps_type eps;

            capture %=
                (label(tok.captureresult_) >> capture_result_enum)
                | eps [ _val = CaptureResult::CR_CAPTURE ]
                ;

            building_type
                = ( tok.BuildingType_                       // _1
                >   label(tok.name_)        > tok.string    // _2
                >   label(tok.description_) > tok.string    // _3
                >   capture                                 // _4
                >   common_rules.common                     // _5
                >   label(tok.icon_)        > tok.string)   // _6
                [ _pass = is_unique_(_r1, _1, _2),
                  insert_building_(_r1, _2, _3, _5, _4, _6, _pass) ]
                ;

            start
                =   +building_type(_r1)
                ;

            building_type.name("BuildingType");

#if DEBUG_PARSERS
            debug(building_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using building_type_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>&)>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller                 label;
        const parse::conditions_parser_grammar  condition_parser;
        const parse::string_parser_grammar      string_grammar;
        parse::detail::tags_grammar             tags_parser;
        parse::detail::common_params_rules      common_rules;
        parse::capture_result_enum_grammar      capture_result_enum;
        parse::detail::rule<CaptureResult ()>   capture;
        building_type_rule                      building_type;
        start_rule                              start;
    };

    boost::python::object py_insert_buildings_(start_rule_payload& buildings_, const boost::python::tuple& args,
                                             const boost::python::dict& kw)
    {
        auto name = boost::python::extract<std::string>(kw["name"])();

        auto description = boost::python::extract<std::string>(kw["description"])();

        auto capture_result = CaptureResult::CR_CAPTURE;
        if (kw.has_key("captureresult")) {
            capture_result = boost::python::extract<enum_wrapper<CaptureResult>>(kw["captureresult"])().value;
        }

        auto icon = boost::python::extract<std::string>(kw["icon"])();

        std::unique_ptr<ValueRef::ValueRef<double>> production_cost;
        auto production_cost_arg = boost::python::extract<value_ref_wrapper<double>>(kw["buildcost"]);
        if (production_cost_arg.check()) {
            production_cost = ValueRef::CloneUnique(production_cost_arg().value_ref);
        } else {
            production_cost = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["buildcost"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> production_time;
        auto production_time_arg = boost::python::extract<value_ref_wrapper<int>>(kw["buildtime"]);
        if (production_time_arg.check()) {
            production_time = ValueRef::CloneUnique(production_time_arg().value_ref);
        } else {
            production_time = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["buildtime"])());
        }

        bool producible = true;
        if (kw.has_key("producible"))
            producible = boost::python::extract<bool>(kw["producible"])();

        std::set<std::string> tags;
        if (kw.has_key("tags")) {
            boost::python::stl_input_iterator<std::string> tags_begin(kw["tags"]), tags_end;
            tags = std::set<std::string>(tags_begin, tags_end);
        }

        std::unique_ptr<Condition::Condition> location;
        if (kw.has_key("location")) {
            location = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["location"])().condition);
        } else {
            location = std::make_unique<Condition::All>();
        }

        std::unique_ptr<Condition::Condition> enqueue_location;
        if (kw.has_key("enqueuelocation")) {
            enqueue_location = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["enqueuelocation"])().condition);
        } else {
            enqueue_location = std::make_unique<Condition::All>();
        }

        std::vector<std::unique_ptr<Effect::EffectsGroup>> effectsgroups;
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

        auto building_type = std::make_unique<BuildingType>(
            std::string{name},
            std::move(description),
            CommonParams{
                std::move(production_cost),
                std::move(production_time),
                producible,
                tags, // TODO: make this parameter by value and move?
                std::move(location),
                std::move(effectsgroups),
                {},
                {},
                std::move(enqueue_location)
            },
            capture_result,
            std::move(icon));

        buildings_.emplace(std::move(name), std::move(building_type));

        return boost::python::object();
    }

    struct py_grammar {
        boost::python::dict globals;

        py_grammar(const PythonParser& parser, start_rule_payload& buildings_) :
            globals(boost::python::import("builtins").attr("__dict__"))
        {
#if PY_VERSION_HEX < 0x03080000
            globals["__builtins__"] = boost::python::import("builtins");
#endif
            RegisterGlobalsEffects(globals);
            RegisterGlobalsConditions(globals);
            RegisterGlobalsValueRefs(globals, parser);
            RegisterGlobalsSources(globals);
            RegisterGlobalsEnums(globals);

            globals["BuildingType"] = boost::python::raw_function(
                [&buildings_](const boost::python::tuple& args, const boost::python::dict& kw)
                { return py_insert_buildings_(buildings_, args, kw); });
        }

        boost::python::dict operator()() const { return globals; }
    };
}

namespace parse {
    start_rule_payload buildings(const PythonParser& parser, const boost::filesystem::path& path, bool& success) {
        start_rule_payload building_types;

        ScopedTimer timer("Buildings Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, building_types);

        bool file_success = true;
        py_grammar p = py_grammar(parser, building_types);
        for (const auto& file : ListDir(path, IsFOCPyScript))
            file_success = py_parse::detail::parse_file<py_grammar>(parser, file, p) && file_success;

        TraceLogger(parsing) << "Start parsing FOCS for BuildingTypes: " << building_types.size();
        for (auto& [building_name, bt] : building_types)
            TraceLogger(parsing) << "BuildingType " << building_name << " : " << bt->GetCheckSum() << "\n" << bt->Dump();
        TraceLogger(parsing) << "End parsing FOCS for BuildingTypes" << building_types.size();

        success = file_success;
        return building_types;
    }
}
