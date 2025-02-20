#include "Parse.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/Species.h"
#include "../util/Directories.h"

#include <boost/mpl/vector.hpp>

#include <boost/python/import.hpp>
#include <boost/python/make_function.hpp>
#include <boost/python/raw_function.hpp>

namespace {
    DeclareThreadSafeLogger(parsing);
}

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const FocusType&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<FocusType>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<PlanetType, PlanetEnvironment>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const PlanetType, PlanetEnvironment>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<PlanetType, PlanetEnvironment>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<Species>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<Species>>&) { return os; }
}
#endif

namespace {
    using start_rule_payload = std::pair<
        std::map<std::string, Species>, // species_by_name
        std::vector<std::string> // census ordering
    >;

    void insert_species_census_ordering_(const boost::python::list& tags, start_rule_payload::second_type& ordering) {
        boost::python::stl_input_iterator<std::string> tags_begin(tags), tags_end;
        for (auto it = tags_begin; it != tags_end; ++it)
            ordering.push_back(*it);
    }

    boost::python::object py_insert_species_(start_rule_payload::first_type& species_, const boost::python::tuple& args,
                                             const boost::python::dict& kw)
    {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto description = boost::python::extract<std::string>(kw["description"])();
        auto gameplay_description = boost::python::extract<std::string>(kw["gameplay_description"])();
        boost::python::stl_input_iterator<FocusType> foci_begin(kw["foci"]), foci_end;
        std::vector<FocusType> foci(foci_begin, foci_end);
        auto defaultfocus = boost::python::extract<std::string>(kw["defaultfocus"])();
        std::map<PlanetType, PlanetEnvironment> environments;
        auto environments_args = boost::python::extract<boost::python::dict>(kw["environments"])();
        boost::python::stl_input_iterator<enum_wrapper<PlanetType>> environments_begin(environments_args), environments_end;
        for (auto it = environments_begin; it != environments_end; ++it) {
            environments.emplace(it->value,
                boost::python::extract<enum_wrapper<PlanetEnvironment>>(environments_args[*it])().value);
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
        bool playable = false;
        if (kw.has_key("playable"))
            playable = boost::python::extract<bool>(kw["playable"])();

        bool native = false;
        if (kw.has_key("native"))
            native = boost::python::extract<bool>(kw["native"])();

        bool can_colonize = false;
        if (kw.has_key("can_colonize"))
            can_colonize = boost::python::extract<bool>(kw["can_colonize"])();

        bool can_produce_ships = false;
        if (kw.has_key("can_produce_ships"))
            can_produce_ships = boost::python::extract<bool>(kw["can_produce_ships"])();

        boost::python::stl_input_iterator<std::string> tags_begin(kw["tags"]), it_end;
        std::set<std::string> tags(tags_begin, it_end);
        std::set<std::string> likes;
        if (kw.has_key("likes")) {
            boost::python::stl_input_iterator<std::string> likes_begin(kw["likes"]);
            likes = std::set<std::string>(likes_begin, it_end);
        }
        std::set<std::string> dislikes;
        if (kw.has_key("dislikes")) {
            boost::python::stl_input_iterator<std::string> dislikes_begin(kw["dislikes"]);
            dislikes = std::set<std::string>(dislikes_begin, it_end);
        }
        auto graphic = boost::python::extract<std::string>(kw["graphic"])();
        double spawn_rate = 1.0;
        if (kw.has_key("spawnrate"))
            spawn_rate = boost::python::extract<double>(kw["spawnrate"])();

        int spawn_limit = 9999;
        if (kw.has_key("spawnlimit"))
            spawn_limit = boost::python::extract<int>(kw["spawnlimit"])();

        std::unique_ptr<Condition::Condition> combat_targets;
        if (kw.has_key("combat_targets"))
            combat_targets = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["combat_targets"])().condition);

        std::unique_ptr<Condition::Condition> annexation_condition;
        if (kw.has_key("annexation_condition"))
            annexation_condition = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["annexation_condition"])().condition);

        std::unique_ptr<ValueRef::ValueRef<double>> annexation_cost;
        if (kw.has_key("annexation_cost"))
            annexation_cost = ValueRef::CloneUnique(boost::python::extract<value_ref_wrapper<double>>(kw["annexation_cost"])().value_ref);


        auto species_ptr = std::make_unique<Species>(
            std::move(name), std::move(description), std::move(gameplay_description),
            std::move(foci),
            std::move(defaultfocus),
            std::move(environments),
            std::move(effectsgroups),
            std::move(combat_targets),
            playable,
            native,
            can_colonize,
            can_produce_ships,
            tags,    // intentionally not moved
            std::move(likes),
            std::move(dislikes),
            std::move(annexation_condition),
            std::move(annexation_cost),
            std::move(graphic),
            spawn_rate,
            spawn_limit);

        auto species_name{species_ptr->Name()};
        species_.emplace(std::move(species_name), std::move(*species_ptr));

        return boost::python::object();
    }

    struct py_grammar {
        boost::python::dict globals;

        py_grammar(const PythonParser& parser, start_rule_payload::first_type& species_) :
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

            globals["Species"] = boost::python::raw_function(
                [&species_](const boost::python::tuple& args, const boost::python::dict& kw)
                { return py_insert_species_(species_, args, kw); });
        }

        boost::python::dict operator()() const { return globals; }
    };

    struct py_manifest_grammar {
        boost::python::dict operator()(start_rule_payload::second_type& ordering) const {
            boost::python::dict globals(boost::python::import("builtins").attr("__dict__"));
            globals["SpeciesCensusOrdering"] = boost::python::make_function([&ordering](auto tags) { return insert_species_census_ordering_(tags, ordering); },
                boost::python::default_call_policies(),
                boost::mpl::vector<void, const boost::python::list&>());
            return globals;
        }
    };
}

namespace parse {
    start_rule_payload species(const PythonParser& parser, const boost::filesystem::path& path, bool& success) {
        start_rule_payload retval;
        auto& [species_, ordering] = retval;

        boost::filesystem::path manifest_file;

        ScopedTimer timer("Species Parsing");

        bool file_success = true;
        py_grammar p = py_grammar(parser, species_);
        for (const auto& file : ListDir(path, IsFOCPyScript)) {
            if (file.filename() == "SpeciesCensusOrdering.focs.py" ) {
                manifest_file = file;
                continue;
            }

            file_success = py_parse::detail::parse_file<py_grammar>(parser, file, p) && file_success;
        }

        TraceLogger(parsing) << "Start parsing FOCS for Species: " << species_.size();
        for (auto& [sp_name, sp] : species_)
            TraceLogger(parsing) << "Species " << sp_name << " : " << sp.GetCheckSum() << "\n" << sp.Dump();
        TraceLogger(parsing) << "End parsing FOCS for Soecies" << species_.size();

        if (!manifest_file.empty()) {
            try {
                file_success = py_parse::detail::parse_file<py_manifest_grammar, start_rule_payload::second_type>(
                    parser, manifest_file, py_manifest_grammar(), ordering) && file_success;

            } catch (const std::runtime_error& e) {
                ErrorLogger() << "Failed to species census manifest in " << manifest_file << " from " << path
                              << " because " << e.what();
            }
        }

        success = file_success;
        return retval;
    }
}
