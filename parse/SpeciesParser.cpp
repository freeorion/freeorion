#include "Parse.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "EnumValueRefRules.h"
#include "EffectParser.h"
#include "ConditionParserImpl.h"
#include "MovableEnvelope.h"

#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/Species.h"
#include "../util/Directories.h"

#include <boost/phoenix.hpp>

#include <boost/python/import.hpp>
#include <boost/python/make_function.hpp>
#include <boost/python/raw_function.hpp>

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
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    struct SpeciesStrings {
        SpeciesStrings() = default;
        SpeciesStrings(std::string& name_, std::string& desc_,
                       std::string& gameplay_desc_) :
            name(std::move(name_)),
            desc(std::move(desc_)),
            gameplay_desc(std::move(gameplay_desc_))
        {}

        std::string name;
        std::string desc;
        std::string gameplay_desc;
    };

    struct SpeciesParamsAndStuff {
        SpeciesParamsAndStuff() = default;
        SpeciesParamsAndStuff(bool playable_, bool native_,
                              bool can_colonize_, bool can_produce_ships_,
                              boost::optional<std::vector<FocusType>>& foci_,
                              boost::optional<std::string>& default_focus_,
                              std::set<std::string>& tags_,
                              std::set<std::string>& likes_,
                              std::set<std::string>& dislikes_) :
            playable(playable_),
            native(native_),
            can_colonize(can_colonize_),
            can_produce_ships(can_produce_ships_),
            foci(std::move(foci_)),
            default_focus(std::move(default_focus_)),
            tags(std::move(tags_)),
            likes(std::move(likes_)),
            dislikes(std::move(dislikes_))
        {}
        bool                                    playable = false;
        bool                                    native = false;
        bool                                    can_colonize = false;
        bool                                    can_produce_ships = false;
        boost::optional<std::vector<FocusType>> foci;
        boost::optional<std::string>            default_focus;
        std::set<std::string>                   tags;
        std::set<std::string>                   likes;
        std::set<std::string>                   dislikes;
    };

    struct SpeciesData {
        SpeciesData() = default;
        SpeciesData(boost::optional<double>& spawn_rate_,
                    boost::optional<int>& spawn_limit_,
                    std::string& graphic_) :
            spawn_rate(std::move(spawn_rate_)),
            spawn_limit(std::move(spawn_limit_)),
            graphic(std::move(graphic_))
        {}

        boost::optional<double> spawn_rate;
        boost::optional<int> spawn_limit;
        std::string graphic;
    };

    void insert_species(
        std::map<std::string, Species>& species, // in/out
        SpeciesStrings& strings,
        boost::optional<std::map<PlanetType, PlanetEnvironment>>& planet_environments,
        boost::optional<parse::effects_group_payload>& effects,
        boost::optional<parse::detail::MovableEnvelope<Condition::Condition>>& combat_targets,
        SpeciesParamsAndStuff& params,
        SpeciesData& species_data,
        bool& pass)
    {
        auto species_name{strings.name};

        species.emplace(std::piecewise_construct,
                        std::forward_as_tuple(std::move(species_name)),
                        std::forward_as_tuple(
                            std::move(strings.name), std::move(strings.desc), std::move(strings.gameplay_desc),
                            (params.foci ? std::move(*params.foci) : std::vector<FocusType>{}),
                            (params.default_focus ? std::move(*params.default_focus) : std::string{}),
                            (planet_environments ? std::move(*planet_environments) : std::map<PlanetType, PlanetEnvironment>{}),
                            (effects ? OpenEnvelopes(*effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>{}),
                            (combat_targets ? (*combat_targets).OpenEnvelope(pass) : nullptr),
                            params.playable,
                            params.native,
                            params.can_colonize,
                            params.can_produce_ships,
                            params.tags,    // intentionally not moved
                            std::move(params.likes),
                            std::move(params.dislikes),
                            std::move(species_data.graphic),
                            (species_data.spawn_rate ? *species_data.spawn_rate : 1.0),
                            (species_data.spawn_limit ? *species_data.spawn_limit : 9999)
                        ));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_species_, insert_species, 8)

    using likes_rule_type    = parse::detail::rule<std::set<std::string> ()>;
    using likes_grammar_type = parse::detail::grammar<std::set<std::string> ()>;

    struct likes_grammar : public likes_grammar_type {
        likes_grammar(const parse::lexer& tok, parse::detail::Labeller& label) :
            likes_grammar::base_type(start, "likes_grammar"),
            one_or_more_string_tokens(tok)
        {
            start %= -(label(tok.likes_) >>  one_or_more_string_tokens);
            start.name("Likes");
#if DEBUG_PARSERS
            debug(start);
#endif
        }

        likes_rule_type start;
        parse::detail::single_or_repeated_string<std::set<std::string>> one_or_more_string_tokens;
    };

    struct dislikes_grammar : public likes_grammar_type {
        dislikes_grammar(const parse::lexer& tok, parse::detail::Labeller& label) :
            dislikes_grammar::base_type(start, "dislikes_grammar"),
            one_or_more_string_tokens(tok)
        {
            start %= -(label(tok.dislikes_) >>  one_or_more_string_tokens);
            start.name("Dislikes");
#if DEBUG_PARSERS
            debug(start);
#endif
        }

        likes_rule_type start;
        parse::detail::single_or_repeated_string<std::set<std::string>> one_or_more_string_tokens;
    };

    static_assert(std::is_same_v<SpeciesManager::SpeciesTypeMap, std::map<std::string, const Species, std::less<>>>);
    using start_rule_payload = std::pair<
        std::map<std::string, Species>, // species_by_name
        std::vector<std::string> // census ordering
    >;
    using start_rule_signature = void(start_rule_payload::first_type&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok, const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            tags_parser(tok, label),
            likes(tok, label),
            dislikes(tok, label),
            effects_group_grammar(tok, label, condition_parser, string_grammar),
            double_rule(tok),
            int_rule(tok),
            one_or_more_foci(focus_type),
            planet_type_rules(tok, label, condition_parser),
            planet_environment_rules(tok, label, condition_parser)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;
            using phoenix::insert;
            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_8_type _8;
            qi::_9_type _9;
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::matches_type matches_;
            qi::omit_type omit_;
            qi::as_string_type as_string_;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            focus_type
                =  ( omit_[tok.Focus_]
                >    label(tok.name_)        > tok.string
                >    label(tok.description_) > tok.string
                >    label(tok.location_)    > condition_parser
                >    label(tok.graphic_)     > tok.string
                ) [ _val = construct<FocusType>(_1, _2, deconstruct_movable_(_3, _pass), _4) ]
                ;

            foci
                =    label(tok.foci_)
                >    one_or_more_foci
                ;

            environment_map_element
                =  ( label(tok.type_)        > planet_type_rules.enum_expr
                >    label(tok.environment_) > planet_environment_rules.enum_expr
                ) [ _val = construct<std::pair<PlanetType, PlanetEnvironment>>(_1, _2) ]
                ;

            environment_map
                =    ('[' > +environment_map_element [ insert(_val, _1) ] > ']')
                |     environment_map_element [ insert(_val, _1) ]
                ;

            species_data
                = (-(label(tok.spawnrate_)      >   double_rule)// _1
                >  -(label(tok.spawnlimit_)     >   int_rule)   // _2
                >   label(tok.graphic_)         >   tok.string) // _3
                [ _val = construct<SpeciesData>(_1, _2, _3) ]
                ;

            species_params_and_stuff
                =   (matches_[tok.Playable_]        // _1
                >    matches_[tok.Native_]          // _2
                >    matches_[tok.CanProduceShips_] // _3
                >    matches_[tok.CanColonize_]     // _4
                >    tags_parser                    // _5
                >   -foci                           // _6
                >   -as_string_[(label(tok.defaultfocus_) > tok.string )] // _7
                >    likes                          // _8
                >    dislikes                       // _9
                    ) [ _val = construct<SpeciesParamsAndStuff>(_1, _2, _4, _3, _6, _7, _5, _8, _9) ]
                ;

            species_strings
                =  ( tok.Species_                                       // _1
                >    label(tok.name_)                   > tok.string    // _2
                >    label(tok.description_)            > tok.string    // _3
                >    label(tok.gameplay_description_)   > tok.string    // _4
                   ) [ _pass = is_unique_(_r1, _1, _2),
                       _val = construct<SpeciesStrings>(_2, _3, _4) ]
                ;

            species
                = ( species_strings(_r1)        // _1
                >   species_params_and_stuff    // _2
                > -(label(tok.effectsgroups_)   >   effects_group_grammar)  // _3
                > -(label(tok.combatTargets_)   >   condition_parser)       // _4
                > -(label(tok.environments_)    >   environment_map)        // _5
                >  species_data                                             // _6
                  ) [ insert_species_(_r1, _1, _5, _3, _4, _2, _6, _pass) ]
                ;

            start
                = +species(_r1)
                ;

            focus_type.name("Focus");
            foci.name("Foci");
            environment_map_element.name("Type = <type> Environment = <env>");
            environment_map.name("Environments");
            species_data.name("Species Data");
            species_params_and_stuff.name("Species Flags");
            species_strings.name("Species Strings");
            likes.name("Likes");
            dislikes.name("Dislikes");
            species.name("Species");
            start.name("start");

#if DEBUG_PARSERS
            debug(focus_type);
            debug(foci);
            debug(environment_map_element);
            debug(environment_map);
            debug(species_params);
            debug(species_strings);
            debug(likes);
            debug(dislikes);
            debug(species);
            debug(start);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using focus_type_rule = parse::detail::rule<FocusType ()>;
        using foci_rule = parse::detail::rule<std::vector<FocusType> ()>;
        using environment_map_element_rule = parse::detail::rule<std::pair<PlanetType, PlanetEnvironment> ()>;
        using environment_map_rule = parse::detail::rule<std::map<PlanetType, PlanetEnvironment> ()>;
        using specid_data_rule = parse::detail::rule<SpeciesData ()>;
        using species_params_rule = parse::detail::rule<SpeciesParamsAndStuff ()>;
        using species_strings_rule = parse::detail::rule<SpeciesStrings (const start_rule_payload::first_type&)>;
        using species_rule = parse::detail::rule<void (start_rule_payload::first_type&)>;
        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller                                    label;
        const parse::conditions_parser_grammar                     condition_parser;
        const parse::string_parser_grammar                         string_grammar;
        parse::detail::tags_grammar                                tags_parser;
        likes_grammar                                              likes;
        dislikes_grammar                                           dislikes;
        parse::effects_group_grammar                               effects_group_grammar;
        parse::detail::double_grammar                              double_rule;
        parse::detail::int_grammar                                 int_rule;
        foci_rule                                                  foci;
        focus_type_rule                                            focus_type;
        parse::detail::single_or_bracketed_repeat<focus_type_rule> one_or_more_foci;
        environment_map_element_rule                               environment_map_element;
        environment_map_rule                                       environment_map;
        specid_data_rule                                           species_data;
        species_params_rule                                        species_params_and_stuff;
        species_strings_rule                                       species_strings;
        species_rule                                               species;
        start_rule                                                 start;
        parse::detail::planet_type_parser_rules                    planet_type_rules;
        parse::detail::planet_environment_parser_rules             planet_environment_rules;
    };

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
        if (kw.has_key("playable")) {
            playable = boost::python::extract<bool>(kw["playable"])();
        }
        bool native = false;
        if (kw.has_key("native")) {
            native = boost::python::extract<bool>(kw["native"])();
        }
        bool can_colonize = false;
        if (kw.has_key("can_colonize")) {
            can_colonize = boost::python::extract<bool>(kw["can_colonize"])();
        }
        bool can_produce_ships = false;
        if (kw.has_key("can_produce_ships")) {
            can_produce_ships = boost::python::extract<bool>(kw["can_produce_ships"])();
        }
        boost::python::stl_input_iterator<std::string> tags_begin(kw["tags"]), it_end;
        std::set<std::string> tags(tags_begin, it_end);
        std::set<std::string> likes;
        if (kw.has_key("likes")) {
            boost::python::stl_input_iterator<std::string> likes_begin(kw["likes"]);
            likes = std::move(std::set<std::string>(likes_begin, it_end));
        }
        std::set<std::string> dislikes;
        if (kw.has_key("dislikes")) {
            boost::python::stl_input_iterator<std::string> dislikes_begin(kw["dislikes"]);
            dislikes = std::move(std::set<std::string>(dislikes_begin, it_end));
        }
        auto graphic = boost::python::extract<std::string>(kw["graphic"])();
        double spawn_rate = 1.0;
        if (kw.has_key("spawnrate")) {
            spawn_rate = boost::python::extract<double>(kw["spawnrate"])();
        }
        int spawn_limit = 9999;
        if (kw.has_key("spawnlimit")) {
            spawn_limit = boost::python::extract<int>(kw["spawnlimit"])();
        }
        std::unique_ptr<Condition::Condition> combat_targets;
        if (kw.has_key("combat_targets")) {
            combat_targets = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["combat_targets"])().condition);
        }

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
            std::move(graphic),
            spawn_rate,
            spawn_limit);

        auto& species_name{species_ptr->Name()};
        species_.emplace(species_name, std::move(*species_ptr));

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
    start_rule_payload species(const PythonParser& parser, const boost::filesystem::path& path) {
        start_rule_payload retval;
        auto& [species_, ordering] = retval;

        boost::filesystem::path manifest_file;

        ScopedTimer timer("Species Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload::first_type>(lexer::tok, file, species_);

        py_grammar p = py_grammar(parser, species_);
        for (const auto& file : ListDir(path, IsFOCPyScript)) {
            if (file.filename() == "SpeciesCensusOrdering.focs.py" ) {
                manifest_file = file;
                continue;
            }

            py_parse::detail::parse_file<py_grammar>(parser, file, p);
        }

        if (!manifest_file.empty()) {
            try {
                py_parse::detail::parse_file<py_manifest_grammar, start_rule_payload::second_type>(
                    parser, manifest_file, py_manifest_grammar(), ordering);

            } catch (const std::runtime_error& e) {
                ErrorLogger() << "Failed to species census manifest in " << manifest_file << " from " << path
                              << " because " << e.what();
            }
        }

        return retval;
    }
}
