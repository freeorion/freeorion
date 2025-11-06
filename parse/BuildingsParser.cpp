#include "Parse.h"

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

    using start_rule_payload = std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>;

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
            auto production_time_arg_double = boost::python::extract<value_ref_wrapper<double>>(kw["buildtime"]);
            if (production_time_arg_double.check()) {
                production_time = std::make_unique<ValueRef::StaticCast<double, int>>(ValueRef::CloneUnique(production_time_arg_double().value_ref));
            } else {
                production_time = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["buildtime"])());
            }
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
    start_rule_payload buildings(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        start_rule_payload building_types;

        ScopedTimer timer("Buildings Parsing");

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
