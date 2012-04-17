#include "ParseImpl.h"
#include "Label.h"
#include "../universe/Species.h"

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const FocusType&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<FocusType>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<PlanetType, PlanetEnvironment>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const PlanetType, PlanetEnvironment>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<PlanetType, PlanetEnvironment>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, Species*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, Species*>&) { return os; }
}
#endif

namespace {
    struct insert_species_ {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, Species*>& species, Species* specie) const {
            if (!species.insert(std::make_pair(specie->Name(), specie)).second) {
                std::string error_str = "ERROR: More than one species in species.txt has the name " + specie->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_species_> insert_species;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

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
            qi::_r1_type _r1;
            qi::_val_type _val;
            using phoenix::construct;
            using phoenix::insert;
            using phoenix::new_;
            using phoenix::push_back;

            focus_type
                =    tok.Focus_
                >    parse::label(Name_name)        > tok.string [ _a = _1 ]
                >    parse::label(Description_name) > tok.string [ _b = _1 ]
                >    parse::label(Location_name)    > parse::detail::condition_parser [ _c = _1 ]
                >    parse::label(Graphic_name)     > tok.string [ _val = construct<FocusType>(_a, _b, _c, _1) ]
                ;

            foci
                =    parse::label(Foci_name)
                >>   (
                            '[' > +focus_type [ push_back(_r1, _1) ] > ']'
                        |   focus_type [ push_back(_r1, _1) ]
                        )
                ;

            effects
                =    parse::label(EffectsGroups_name) >> parse::detail::effects_group_parser() [ _r1 = _1 ]
                ;

            environment_map_element
                =    parse::label(Type_name)        >> parse::enum_parser<PlanetType>() [ _a = _1 ]
                >    parse::label(Environment_name) >  parse::enum_parser<PlanetEnvironment>()
                        [ _val = construct<std::pair<PlanetType, PlanetEnvironment> >(_a, _1) ]
                ;

            environment_map
                =    '[' > +environment_map_element [ insert(_val, _1) ] > ']'
                |    environment_map_element [ insert(_val, _1) ]
                ;

            environments
                =    parse::label(Environments_name) >> environment_map [ _r1 = _1 ]
                ;

            tags
                =  -(
                        parse::label(Tags_name)
                    >>  (
                            '[' > +tok.string [ push_back(_r1, _1) ] > ']'
                            |   tok.string [ push_back(_r1, _1) ]
                        )
                    )
                ;

            species_params
                =   -tok.Playable_ [ _a = true ]
                >   -tok.Native_ [ _b = true ]
                >   -tok.CanProduceShips_ [ _c = true ]
                >   -tok.CanColonize_ [ _d = true ]
                    [ _val = construct<SpeciesParams>(_a, _b, _c, _d) ]
                ;

            species
                =    tok.Species_
                >    parse::label(Name_name)        > tok.string [ _a = _1 ]
                >    parse::label(Description_name) > tok.string [ _b = _1 ]
                >    species_params [ _c = _1]
                >    tags(_d)
                >   -foci(_e)
                >   -effects(_f)
                >   -environments(_g)
                >    parse::label(Graphic_name) > tok.string
                     [ insert_species(_r1, new_<Species>(_a, _b, _e, _g, _f, _c, _d, _1)) ]
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
            tags.name("Tags");
            species_params.name("Species Flags");
            species.name("Species");
            start.name("start");

#if DEBUG_PARSERS
            debug(focus_type);
            debug(foci);
            debug(effects);
            debug(environment_map_element);
            debug(environment_map);
            debug(environments);
            debug(tags);
            debug(species_params);
            debug(species);
            debug(start);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            FocusType (),
            qi::locals<
                std::string,
                std::string,
                Condition::ConditionBase*
            >,
            parse::skipper_type
        > focus_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<FocusType>&),
            parse::skipper_type
        > foci_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&),
            parse::skipper_type
        > effects_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            std::pair<PlanetType, PlanetEnvironment> (),
            qi::locals<PlanetType>,
            parse::skipper_type
        > environment_map_element_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            std::map<PlanetType, PlanetEnvironment> (),
            parse::skipper_type
        > environment_map_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<PlanetType, PlanetEnvironment>&),
            parse::skipper_type
        > environments_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<std::string>&),
            parse::skipper_type
        > tags_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            SpeciesParams (),
            qi::locals<
                bool,
                bool,
                bool,
                bool
            >,
            parse::skipper_type
        > species_params_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, Species*>&),
            qi::locals<
                std::string,
                std::string,
                SpeciesParams,
                std::vector<std::string>,
                std::vector<FocusType>,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                std::map<PlanetType, PlanetEnvironment>
            >,
            parse::skipper_type
        > species_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, Species*>&),
            parse::skipper_type
        > start_rule;

        foci_rule                       foci;
        focus_type_rule                 focus_type;
        effects_rule                    effects;
        environment_map_element_rule    environment_map_element;
        environment_map_rule            environment_map;
        environments_rule               environments;
        tags_rule                       tags;
        species_params_rule             species_params;
        species_rule                    species;
        start_rule                      start;
    };
}

namespace parse {
    bool species(const boost::filesystem::path& path, std::map<std::string, Species*>& species_)
    { return detail::parse_file<rules, std::map<std::string, Species*> >(path, species_); }
}
