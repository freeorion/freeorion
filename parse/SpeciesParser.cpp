#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParserImpl.h"
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
                        const std::vector<FocusType>& foci,
                        const std::string& preferred_focus,
                        const std::map<PlanetType, PlanetEnvironment>& planet_environments,
                        const parse::effects_group_payload& effects,
                        const SpeciesParams& params,
                        std::pair<std::set<std::string>, std::string>& tags_and_graphic,
                        bool& pass)
    {
        auto species_ptr = boost::make_unique<Species>(
            strings, foci, preferred_focus, planet_environments,
            OpenEnvelopes(effects, pass), params,
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
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_g_type _g;
            qi::_h_type _h;
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            focus_type
                =    tok.Focus_
                >    labeller.rule(Name_token)        > tok.string [ _a = _1 ]
                >    labeller.rule(Description_token) > tok.string [ _b = _1 ]
                >    labeller.rule(Location_token)    > condition_parser [ _c = _1 ]
                >    labeller.rule(Graphic_token)     > tok.string
                [ _val = construct<FocusType>(_a, _b, deconstruct_movable_(_c, _pass), _1) ]
                ;

            foci
                =    labeller.rule(Foci_token)
                >    (
                            ('[' > +focus_type [ push_back(_r1, _1) ] > ']')
                        |    focus_type [ push_back(_r1, _1) ]
                     )
                ;

            effects
                =    labeller.rule(EffectsGroups_token) > effects_group_grammar [ _r1 = _1 ]
                ;

            environment_map_element
                =    labeller.rule(Type_token)        > planet_type_rules.enum_expr [ _a = _1 ]
                >    labeller.rule(Environment_token) > planet_environment_rules.enum_expr
                     [ _val = construct<std::pair<PlanetType, PlanetEnvironment>>(_a, _1) ]
                ;

            environment_map
                =    ('[' > +environment_map_element [ insert(_val, _1) ] > ']')
                |     environment_map_element [ insert(_val, _1) ]
                ;

            environments
                =    labeller.rule(Environments_token) > environment_map [ _r1 = _1 ]
                ;

            species_params
                =   ((tok.Playable_ [ _a = true ]) | eps)
                >   ((tok.Native_ [ _b = true ]) | eps)
                >   ((tok.CanProduceShips_ [ _c = true ]) | eps)
                >   ((tok.CanColonize_ [ _d = true ]) | eps)
                    [ _val = construct<SpeciesParams>(_a, _b, _d, _c) ]
                ;

            species_strings
                =    labeller.rule(Name_token)                   > tok.string
                     [ _pass = is_unique_(_r1, Species_token, _1), _a = _1 ]
                >    labeller.rule(Description_token)            > tok.string [ _b = _1 ]
                >    labeller.rule(Gameplay_Description_token)   > tok.string [ _c = _1 ]
                    [ _val = construct<SpeciesStrings>(_a, _b, _c) ]
                ;

            species
                =    tok.Species_
                >    species_strings(_r1) [ _a = _1 ]
                >    species_params [ _b = _1]
                >    tags_parser(_c)
                >   -foci(_d)
                >   -(labeller.rule(PreferredFocus_token)        >> tok.string [ _g = _1 ])
                >   -effects(_e)
                >   -environments(_f)
                >    labeller.rule(Graphic_token) > tok.string
                [ _h = construct<std::pair<std::set<std::string>, std::string>>(_c, _1),
                  insert_species_(_r1, _a, _d, _g, _f, _e, _b, _h, _pass) ]
                ;

            start
                =   +species(_r1)
                ;

            focus_type.name("Focus");
            foci.name("Foci");
            effects.name("EffectsGroups");
            environment_map_element.name("Type = <type> Environment = <env>");
            environment_map.name("Environments");
            environments.name("Environments");
            species_params.name("Species Flags");
            species_strings.name("Species Strings");
            species.name("Species");
            start.name("start");

#if DEBUG_PARSERS
            debug(focus_type);
            debug(foci);
            debug(effects);
            debug(environment_map_element);
            debug(environment_map);
            debug(environments);
            debug(species_params);
            debug(species_strings);
            debug(species);
            debug(start);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            FocusType (),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                parse::detail::condition_payload
            >
        > focus_type_rule;

        typedef parse::detail::rule<
            void (std::vector<FocusType>&)
        > foci_rule;

        typedef parse::detail::rule<
            void (parse::effects_group_payload&)
        > effects_rule;

        typedef parse::detail::rule<
            std::pair<PlanetType, PlanetEnvironment> (),
            boost::spirit::qi::locals<PlanetType>
        > environment_map_element_rule;

        typedef parse::detail::rule<
            std::map<PlanetType, PlanetEnvironment> ()
        > environment_map_rule;

        typedef parse::detail::rule<
            void (std::map<PlanetType, PlanetEnvironment>&)
        > environments_rule;

        typedef parse::detail::rule<
            SpeciesParams (),
            boost::spirit::qi::locals<
                bool,
                bool,
                bool,
                bool
            >
        > species_params_rule;

        typedef parse::detail::rule<
            SpeciesStrings (const start_rule_payload&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::string
            >
        > species_strings_rule;

        typedef parse::detail::rule<
            void (start_rule_payload&),
            boost::spirit::qi::locals<
                SpeciesStrings,
                SpeciesParams,
                std::set<std::string>,  // tags
                std::vector<FocusType>,
                parse::effects_group_payload,
                std::map<PlanetType, PlanetEnvironment>,
                std::string,             // graphic
                std::pair<std::set<std::string>, std::string>
                >
        > species_rule;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller labeller;
        const parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::effects_group_grammar effects_group_grammar;
        foci_rule                       foci;
        focus_type_rule                 focus_type;
        effects_rule                    effects;
        environment_map_element_rule    environment_map_element;
        environment_map_rule            environment_map;
        environments_rule               environments;
        species_params_rule             species_params;
        species_strings_rule            species_strings;
        species_rule                    species;
        start_rule                      start;
        parse::detail::planet_type_parser_rules planet_type_rules;
        parse::detail::planet_environment_parser_rules planet_environment_rules;
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
