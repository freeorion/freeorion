#include "ConditionParserImpl.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "ValueRefParserImpl.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Condition::ConditionBase*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<std::string>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<PlanetSize>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<PlanetType>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<PlanetEnvironment>*>&) { return os; }
}
#endif

namespace {
    struct condition_parser_rules_6 {
        condition_parser_rules_6() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::push_back;

            string_ref_vec
                =   ('[' > +parse::string_value_ref() [ push_back(_val, _1) ] > ']')
                |    parse::string_value_ref() [ push_back(_val, _1) ]
                ;

            homeworld
                =   tok.Homeworld_
                >   (
                        (parse::detail::label(Name_token) > string_ref_vec [ _val = new_<Condition::Homeworld>(_1) ])
                    |    eps [ _val = new_<Condition::Homeworld>() ]
                    )
                ;

            building
                =   (
                        tok.Building_
                    >  -(parse::detail::label(Name_token) > string_ref_vec [ _a = _1 ])
                    )
                    [ _val = new_<Condition::Building>(_a) ]
                ;

            species
                =   tok.Species_
                >   (
                        (parse::detail::label(Name_token) > string_ref_vec [ _val = new_<Condition::Species>(_1) ])
                    |    eps [ _val = new_<Condition::Species>() ]
                    )
                ;

            focus_type
                =   tok.Focus_
                >   (
                        (parse::detail::label(Type_token) > string_ref_vec [ _val = new_<Condition::FocusType>(_1) ])
                |        eps [ _val = new_<Condition::FocusType>(std::vector<ValueRef::ValueRefBase<std::string>*>()) ]
                    )
                ;

            planet_type
                =   (tok.Planet_
                    >>  parse::detail::label(Type_token)
                    )
                >   (
                        ('[' > +parse::detail::planet_type_rules().expr [ push_back(_a, _1) ] > ']')
                    |    parse::detail::planet_type_rules().expr [ push_back(_a, _1) ]
                    )
                    [ _val = new_<Condition::PlanetType>(_a) ]
                ;

            planet_size
                =   (tok.Planet_
                    >>  parse::detail::label(Size_token)
                    )
                >   (
                        ('[' > +parse::detail::planet_size_rules().expr [ push_back(_a, _1) ] > ']')
                    |    parse::detail::planet_size_rules().expr [ push_back(_a, _1) ]
                    )
                    [ _val = new_<Condition::PlanetSize>(_a) ]
                ;

            planet_environment
                =   ((tok.Planet_
                     >>  parse::detail::label(Environment_token)
                     )
                >   (
                        ('[' > +parse::detail::planet_environment_rules().expr [ push_back(_a, _1) ] > ']')
                    |    parse::detail::planet_environment_rules().expr [ push_back(_a, _1) ]
                    )
                >  -(parse::detail::label(Species_token)        >  parse::string_value_ref() [_b = _1]))
                    [ _val = new_<Condition::PlanetEnvironment>(_a, _b) ]
                ;

            object_type
                =   parse::detail::universe_object_type_rules().enum_expr [ _val = new_<Condition::Type>(new_<ValueRef::Constant<UniverseObjectType>>(_1)) ]
                |   (
                        tok.ObjectType_
                    >   parse::detail::label(Type_token) > parse::detail::universe_object_type_rules().expr [ _val = new_<Condition::Type>(_1) ]
                    )
                ;


            start
                %=  homeworld
                |   building
                |   species
                |   focus_type
                |   planet_type
                |   planet_size
                |   planet_environment
                |   object_type
                ;

            string_ref_vec.name("sequence of string expressions");
            homeworld.name("Homeworld");
            building.name("Building");
            species.name("Species");
            focus_type.name("Focus");
            planet_type.name("PlanetType");
            planet_size.name("PlanetSize");
            planet_environment.name("PlanetEnvironment");
            object_type.name("ObjectType");

#if DEBUG_CONDITION_PARSERS
            debug(string_ref_vec);
            debug(homeworld);
            debug(building);
            debug(species);
            debug(focus_type);
            debug(planet_type);
            debug(planet_size);
            debug(planet_environment);
            debug(object_type);
#endif
        }

        typedef parse::detail::rule<
            std::vector<ValueRef::ValueRefBase<std::string>*> ()
        > string_ref_vec_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<std::vector<ValueRef::ValueRefBase<std::string>*>>
        > building_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<std::vector<ValueRef::ValueRefBase<PlanetType>*>>
        > planet_type_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<std::vector<ValueRef::ValueRefBase<PlanetSize>*>>
        > planet_size_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                std::vector<ValueRef::ValueRefBase<PlanetEnvironment>*>,
                ValueRef::ValueRefBase<std::string>*
            >
        > planet_environment_rule;

        string_ref_vec_rule             string_ref_vec;
        parse::condition_parser_rule    homeworld;
        building_rule                   building;
        parse::condition_parser_rule    species;
        parse::condition_parser_rule    focus_type;
        planet_type_rule                planet_type;
        planet_size_rule                planet_size;
        planet_environment_rule         planet_environment;
        parse::condition_parser_rule    object_type;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_6() {
        static condition_parser_rules_6 retval;
        return retval.start;
    }

    condition_parser_rules_6::condition_parser_rules_6() :
        condition_parser_rules_6::base_type(start, "condition_parser_rules_6")
    {
        const parse::lexer& tok = parse::lexer::instance();

        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_val_type _val;
        qi::eps_type eps;
        using phoenix::new_;
        using phoenix::push_back;

        string_ref_vec
            =   ('[' > +parse::string_value_ref() [ push_back(_val, _1) ] > ']')
            |    parse::string_value_ref() [ push_back(_val, _1) ]
            ;

        homeworld
            =   tok.Homeworld_
            >   (
                (parse::detail::label(Name_token) > string_ref_vec [ _val = new_<Condition::Homeworld>(_1) ])
                |    eps [ _val = new_<Condition::Homeworld>() ]
            )
            ;

        building
            =   (
                tok.Building_
                >  -(parse::detail::label(Name_token) > string_ref_vec [ _a = _1 ])
            )
            [ _val = new_<Condition::Building>(_a) ]
            ;

        species
            =   tok.Species_
            >   (
                (parse::detail::label(Name_token) > string_ref_vec [ _val = new_<Condition::Species>(_1) ])
                |    eps [ _val = new_<Condition::Species>() ]
            )
            ;

        focus_type
            =   tok.Focus_
            >   (
                (parse::detail::label(Type_token) > string_ref_vec [ _val = new_<Condition::FocusType>(_1) ])
                |        eps [ _val = new_<Condition::FocusType>(std::vector<ValueRef::ValueRefBase<std::string>*>()) ]
            )
            ;

        planet_type
            =   (tok.Planet_
                 >>  parse::detail::label(Type_token)
                )
            >   (
                ('[' > +parse::detail::planet_type_rules().expr [ push_back(_a, _1) ] > ']')
                |    parse::detail::planet_type_rules().expr [ push_back(_a, _1) ]
            )
            [ _val = new_<Condition::PlanetType>(_a) ]
            ;

        planet_size
            =   (tok.Planet_
                 >>  parse::detail::label(Size_token)
                )
            >   (
                ('[' > +parse::detail::planet_size_rules().expr [ push_back(_a, _1) ] > ']')
                |    parse::detail::planet_size_rules().expr [ push_back(_a, _1) ]
            )
            [ _val = new_<Condition::PlanetSize>(_a) ]
            ;

        planet_environment
            =   ((tok.Planet_
                  >>  parse::detail::label(Environment_token)
                 )
                 >   (
                     ('[' > +parse::detail::planet_environment_rules().expr [ push_back(_a, _1) ] > ']')
                     |    parse::detail::planet_environment_rules().expr [ push_back(_a, _1) ]
                 )
                 >  -(parse::detail::label(Species_token)        >  parse::string_value_ref() [_b = _1]))
            [ _val = new_<Condition::PlanetEnvironment>(_a, _b) ]
            ;

        object_type
            =   parse::detail::universe_object_type_rules().enum_expr [ _val = new_<Condition::Type>(new_<ValueRef::Constant<UniverseObjectType>>(_1)) ]
            |   (
                tok.ObjectType_
                >   parse::detail::label(Type_token) > parse::detail::universe_object_type_rules().expr [ _val = new_<Condition::Type>(_1) ]
            )
            ;


        start
            %=  homeworld
            |   building
            |   species
            |   focus_type
            |   planet_type
            |   planet_size
            |   planet_environment
            |   object_type
            ;

        string_ref_vec.name("sequence of string expressions");
        homeworld.name("Homeworld");
        building.name("Building");
        species.name("Species");
        focus_type.name("Focus");
        planet_type.name("PlanetType");
        planet_size.name("PlanetSize");
        planet_environment.name("PlanetEnvironment");
        object_type.name("ObjectType");

#if DEBUG_CONDITION_PARSERS
        debug(string_ref_vec);
        debug(homeworld);
        debug(building);
        debug(species);
        debug(focus_type);
        debug(planet_type);
        debug(planet_size);
        debug(planet_environment);
        debug(object_type);
#endif
    }

} }
