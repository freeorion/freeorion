#include "Parse.h"

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

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>


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

    struct SpeciesStuff {
        SpeciesStuff(const boost::optional<std::vector<FocusType>>& foci_,
                     const boost::optional<std::string>& preferred_focus_,
                     const std::set<std::string>& tags_,
                     const std::string& graphic_) :
            foci(foci_),
            preferred_focus(preferred_focus_),
            tags(tags_),
            graphic(graphic_)
        {}

        boost::optional<std::vector<FocusType>> foci;
        boost::optional<std::string>            preferred_focus;
        std::set<std::string>                   tags;
        std::string                             graphic;
    };


    void insert_species(
        std::map<std::string, std::unique_ptr<Species>>& species,
        const SpeciesStrings& strings,
        const boost::optional<std::map<PlanetType, PlanetEnvironment>>& planet_environments,
        const boost::optional<parse::effects_group_payload>& effects,
        boost::optional<parse::detail::MovableEnvelope<Condition::ConditionBase>>& combat_targets,
        const SpeciesParams& params,
        const SpeciesStuff& foci_preferred_tags_graphic,
        bool& pass)
    {
        auto species_ptr = boost::make_unique<Species>(
            strings,
            (foci_preferred_tags_graphic.foci ? *foci_preferred_tags_graphic.foci : std::vector<FocusType>()),
            (foci_preferred_tags_graphic.preferred_focus ? *foci_preferred_tags_graphic.preferred_focus : std::string()),
            (planet_environments ? *planet_environments : std::map<PlanetType, PlanetEnvironment>()),
            (effects ? OpenEnvelopes(*effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>()),
            (combat_targets ? (*combat_targets).OpenEnvelope(pass) : nullptr),
            params,
            foci_preferred_tags_graphic.tags,
            foci_preferred_tags_graphic.graphic);

        species.insert(std::make_pair(species_ptr->Name(), std::move(species_ptr)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_species_, insert_species, 8)


    using start_rule_payload = std::pair<
        std::map<std::string, std::unique_ptr<Species>>, // species_by_name
        std::vector<std::string> // census ordering
    >;
    using start_rule_signature = void(start_rule_payload::first_type&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            tags_parser(tok, label),
            effects_group_grammar(tok, label, condition_parser, string_grammar),
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
                >    label(tok.Name_)        > tok.string
                >    label(tok.Description_) > tok.string
                >    label(tok.Location_)    > condition_parser
                >    label(tok.Graphic_)     > tok.string
                ) [ _val = construct<FocusType>(_1, _2, deconstruct_movable_(_3, _pass), _4) ]
                ;

            foci
                =    label(tok.Foci_)
                >    one_or_more_foci
                ;

            environment_map_element
                =  ( label(tok.Type_)        > planet_type_rules.enum_expr
                >    label(tok.Environment_) > planet_environment_rules.enum_expr
                ) [ _val = construct<std::pair<PlanetType, PlanetEnvironment>>(_1, _2) ]
                ;

            environment_map
                =    ('[' > +environment_map_element [ insert(_val, _1) ] > ']')
                |     environment_map_element [ insert(_val, _1) ]
                ;

            species_params
                =   (matches_[tok.Playable_]
                >    matches_[tok.Native_]
                >    matches_[tok.CanProduceShips_]
                >    matches_[tok.CanColonize_]
                    ) [ _val = construct<SpeciesParams>(_1, _2, _4, _3) ]
                ;

            species_strings
                =  ( tok.Species_
                >    label(tok.Name_)                   > tok.string
                >    label(tok.Description_)            > tok.string
                >    label(tok.Gameplay_Description_)   > tok.string
                   ) [ _pass = is_unique_(_r1, _1, _2),
                       _val = construct<SpeciesStrings>(_2, _3, _4) ]
                ;

            species
                = ( species_strings(_r1)// _1
                >   species_params      // _2
                >   tags_parser         // _3
                >  -foci                // _4
                >  -as_string_[(label(tok.PreferredFocus_)  >   tok.string )]           // _5
                > -(label(tok.EffectsGroups_)               >   effects_group_grammar)  // _6
                > -(label(tok.CombatTargets_)               >   condition_parser)       // _7
                > -(label(tok.Environments_)                >   environment_map)        // _8
                >   label(tok.Graphic_)                     >   tok.string              // _9
                  ) [ insert_species_(_r1, _1, _8, _6, _7, _2,
                                      construct<SpeciesStuff>(_4, _5, _3, _9),
                                      _pass) ]
                ;

            start
                = +species(_r1)
                ;

            focus_type.name("Focus");
            foci.name("Foci");
            environment_map_element.name("Type = <type> Environment = <env>");
            environment_map.name("Environments");
            species_params.name("Species Flags");
            species_strings.name("Species Strings");
            species.name("Species");
            start.name("start");

#if DEBUG_PARSERS
            debug(focus_type);
            debug(foci);
            debug(environment_map_element);
            debug(environment_map);
            debug(species_params);
            debug(species_strings);
            debug(species);
            debug(start);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using focus_type_rule = parse::detail::rule<FocusType ()>;
        using foci_rule = parse::detail::rule<std::vector<FocusType> ()>;
        using environment_map_element_rule = parse::detail::rule<std::pair<PlanetType, PlanetEnvironment> ()>;
        using environment_map_rule = parse::detail::rule<std::map<PlanetType, PlanetEnvironment> ()>;
        using species_params_rule = parse::detail::rule<SpeciesParams ()>;
        using species_strings_rule = parse::detail::rule<SpeciesStrings (const start_rule_payload::first_type&)>;
        using species_rule = parse::detail::rule<void (start_rule_payload::first_type&)>;
        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller                                    label;
        const parse::conditions_parser_grammar                     condition_parser;
        const parse::string_parser_grammar                         string_grammar;
        parse::detail::tags_grammar                                tags_parser;
        parse::effects_group_grammar                               effects_group_grammar;
        foci_rule                                                  foci;
        focus_type_rule                                            focus_type;
        parse::detail::single_or_bracketed_repeat<focus_type_rule> one_or_more_foci;
        environment_map_element_rule                               environment_map_element;
        environment_map_rule                                       environment_map;
        species_params_rule                                        species_params;
        species_strings_rule                                       species_strings;
        species_rule                                               species;
        start_rule                                                 start;
        parse::detail::planet_type_parser_rules                    planet_type_rules;
        parse::detail::planet_environment_parser_rules             planet_environment_rules;
    };

    using manifest_start_rule_signature = void (std::vector<std::string>&);

    struct manifest_grammar : public parse::detail::grammar<manifest_start_rule_signature> {
        manifest_grammar(const parse::lexer& tok,
                         const std::string& filename,
                         const parse::text_iterator& first, const parse::text_iterator& last) :
            manifest_grammar::base_type(start)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_r1_type _r1;
            qi::omit_type omit_;

            species_manifest
                =    omit_[tok.SpeciesCensusOrdering_]
                >    *(label(tok.Tag_) > tok.string [ push_back(_r1, _1) ])
                ;

            start
                =   +species_manifest(_r1)
                ;

            species_manifest.name("ParsedSpeciesCensusOrdering");

#if DEBUG_PARSERS
            debug(species_manifest);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using manifest_rule = parse::detail::rule<void (std::vector<std::string>&)>;
        using start_rule = parse::detail::rule<manifest_start_rule_signature>;

        parse::detail::Labeller label;
        manifest_rule species_manifest;
        start_rule start;
    };
}

namespace parse {
    start_rule_payload species(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload::first_type species_;
        start_rule_payload::second_type ordering;

        boost::filesystem::path manifest_file;

        for (const boost::filesystem::path& file : ListScripts(path)) {
            if (file.filename() == "SpeciesCensusOrdering.focs.txt" ) {
                manifest_file = file;
                continue;
            }

            /*auto success =*/ detail::parse_file<grammar, start_rule_payload::first_type>(lexer, file, species_);
        }

        if (!manifest_file.empty()) {
            try {
                /*auto success =*/ detail::parse_file<manifest_grammar, start_rule_payload::second_type>(
                    lexer, manifest_file, ordering);

            } catch (const std::runtime_error& e) {
                ErrorLogger() << "Failed to species census manifest in " << manifest_file << " from " << path
                              << " because " << e.what();
            }
        }

        return {std::move(species_), ordering};
    }
}
