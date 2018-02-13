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

    void insert_species(std::map<std::string, std::unique_ptr<Species>>& species,
                        const SpeciesStrings& strings,
                        const boost::optional<std::vector<FocusType>>& foci,
                        const boost::optional<std::string>& preferred_focus,
                        const boost::optional<std::map<PlanetType, PlanetEnvironment>>& planet_environments,
                        const boost::optional<parse::effects_group_payload>& effects,
                        const SpeciesParams& params,
                        const std::pair<std::set<std::string>, std::string>& tags_and_graphic,
                        bool& pass)
    {
        auto species_ptr = boost::make_unique<Species>(
            strings,
            (foci ? *foci : std::vector<FocusType>()),
            (preferred_focus ? *preferred_focus : std::string()),
            (planet_environments ? *planet_environments : std::map<PlanetType, PlanetEnvironment>()),
            (effects ? OpenEnvelopes(*effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>()),
            params,
            tags_and_graphic.first, tags_and_graphic.second);

        species.insert(std::make_pair(species_ptr->Name(), std::move(species_ptr)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_species_, insert_species, 9)


    using start_rule_payload = std::map<std::string, std::unique_ptr<Species>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok),
            condition_parser(tok, labeller),
            string_grammar(tok, labeller, condition_parser),
            tags_parser(tok, labeller),
            effects_group_grammar(tok, labeller, condition_parser, string_grammar),
            one_or_more_foci(focus_type),
            planet_type_rules(tok, labeller, condition_parser),
            planet_environment_rules(tok, labeller, condition_parser)
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
                >    labeller.rule(Name_token)        > tok.string
                >    labeller.rule(Description_token) > tok.string
                >    labeller.rule(Location_token)    > condition_parser
                >    labeller.rule(Graphic_token)     > tok.string
                ) [ _val = construct<FocusType>(_1, _2, deconstruct_movable_(_3, _pass), _4) ]
                ;

            foci
                =    labeller.rule(Foci_token)
                >    one_or_more_foci
                ;

            environment_map_element
                =  ( labeller.rule(Type_token)        > planet_type_rules.enum_expr
                >    labeller.rule(Environment_token) > planet_environment_rules.enum_expr
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
                =  ( labeller.rule(Name_token)                   > tok.string
                     [ _pass = is_unique_(_r1, Species_token, _1) ]
                >    labeller.rule(Description_token)            > tok.string
                >    labeller.rule(Gameplay_Description_token)   > tok.string
                   ) [ _val = construct<SpeciesStrings>(_1, _2, _3) ]
                ;

            species
                =  ( omit_[tok.Species_]
                >    species_strings(_r1)
                >    species_params
                >    tags_parser
                >   -foci
                >   -as_string_[(labeller.rule(PreferredFocus_token)        >> tok.string )]
                >   -(labeller.rule(EffectsGroups_token) > effects_group_grammar)
                >   -(labeller.rule(Environments_token)  > environment_map)
                >    labeller.rule(Graphic_token) > tok.string
                   ) [ insert_species_(_r1, _1, _4, _5, _7, _6, _2,
                                       construct<std::pair<std::set<std::string>, std::string>>(_3, _8), _pass) ]
                ;

            start
                =   +species(_r1)
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

        using species_strings_rule = parse::detail::rule<SpeciesStrings (const start_rule_payload&)>;

        using species_rule = parse::detail::rule<void (start_rule_payload&)>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller                                    labeller;
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
}

namespace parse {
    start_rule_payload species(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload species_;

        for (const boost::filesystem::path& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, species_);
        }

        return species_;
    }
}
