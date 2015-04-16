#include "ConditionParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
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

            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =
                parse::value_ref_parser<std::string>();

            const parse::value_ref_parser_rule<PlanetType>::type& planet_type_value_ref =
                parse::value_ref_parser<PlanetType>();

            const parse::value_ref_parser_rule<PlanetSize>::type& planet_size_value_ref =
                parse::value_ref_parser<PlanetSize>();

            const parse::value_ref_parser_rule<PlanetEnvironment>::type& planet_environment_value_ref =
                parse::value_ref_parser<PlanetEnvironment>();

            const parse::value_ref_parser_rule<UniverseObjectType>::type& universe_object_type_value_ref =
                parse::value_ref_parser<UniverseObjectType>();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::push_back;

            string_ref_vec
                =    '[' > +string_value_ref [ push_back(_val, _1) ] > ']'
                |    string_value_ref [ push_back(_val, _1) ]
                ;

            homeworld
                =    tok.Homeworld_
                >   (
                            parse::label(Name_token) > string_ref_vec [ _val = new_<Condition::Homeworld>(_1) ]
                        |   eps [ _val = new_<Condition::Homeworld>() ]
                    )
                ;

            building
                =    (
                            tok.Building_
                        >  -(parse::label(Name_token) > string_ref_vec [ _a = _1 ])
                     )
                     [ _val = new_<Condition::Building>(_a) ]
                ;

            species
                =    tok.Species_
                >    (
                            parse::label(Name_token) > string_ref_vec [ _val = new_<Condition::Species>(_1) ]
                        |   eps [ _val = new_<Condition::Species>() ]
                     )
                ;

            focus_type
                =    tok.Focus_
                >    (
                            parse::label(Type_token) > string_ref_vec [ _val = new_<Condition::FocusType>(_1) ]
                        |   eps [ _val = new_<Condition::FocusType>(std::vector<ValueRef::ValueRefBase<std::string>*>()) ]
                     )
                ;

            planet_type
                =    tok.Planet_
                >>   parse::label(Type_token)
                >    (
                            '[' > +planet_type_value_ref [ push_back(_a, _1) ] > ']'
                        |   planet_type_value_ref [ push_back(_a, _1) ]
                     )
                     [ _val = new_<Condition::PlanetType>(_a) ]
                ;

            planet_size
                =    tok.Planet_
                >>   parse::label(Size_token)
                >    (
                            '[' > +planet_size_value_ref [ push_back(_a, _1) ] > ']'
                        |   planet_size_value_ref [ push_back(_a, _1) ]
                     )
                     [ _val = new_<Condition::PlanetSize>(_a) ]
                ;

            planet_environment
                =   (tok.Planet_
                >>   parse::label(Environment_token)
                >    (
                            '[' > +planet_environment_value_ref [ push_back(_a, _1) ] > ']'
                        |   planet_environment_value_ref [ push_back(_a, _1) ]
                     )
                >  -(parse::label(Species_token)        >  string_value_ref [_b = _1]))
                     [ _val = new_<Condition::PlanetEnvironment>(_a, _b) ]
                ;

            object_type
                =    parse::enum_parser<UniverseObjectType>() [ _val = new_<Condition::Type>(new_<ValueRef::Constant<UniverseObjectType> >(_1)) ]
                |    (
                            tok.ObjectType_
                        >  parse::label(Type_token) > universe_object_type_value_ref [ _val = new_<Condition::Type>(_1) ]
                     )
                ;


            start
                %=   homeworld
                |    building
                |    species
                |    focus_type
                |    planet_type
                |    planet_size
                |    planet_environment
                |    object_type
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            std::vector<ValueRef::ValueRefBase<std::string>*> (),
            parse::skipper_type
        > string_ref_vec_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<ValueRef::ValueRefBase<std::string>*> >,
            parse::skipper_type
        > building_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<ValueRef::ValueRefBase<PlanetType>*> >,
            parse::skipper_type
        > planet_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<ValueRef::ValueRefBase<PlanetSize>*> >,
            parse::skipper_type
        > planet_size_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                std::vector<ValueRef::ValueRefBase<PlanetEnvironment>*>,
                ValueRef::ValueRefBase<std::string>*
            >,
            parse::skipper_type
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
} }
