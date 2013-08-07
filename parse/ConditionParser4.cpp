#include "ConditionParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/home/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<const Condition::ConditionBase*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<const ValueRef::ValueRefBase<std::string>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<const ValueRef::ValueRefBase<PlanetSize>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<const ValueRef::ValueRefBase<PlanetType>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>&) { return os; }
}
#endif

namespace {
    struct condition_parser_rules_4 {
        condition_parser_rules_4() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =
                parse::value_ref_parser<int>();

            const parse::value_ref_parser_rule<double>::type& double_value_ref =
                parse::value_ref_parser<double>();

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
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::push_back;

            string_ref_vec
                =    '[' > +string_value_ref [ push_back(_val, _1) ] > ']'
                |    string_value_ref [ push_back(_val, _1) ]
                ;

            homeworld
                =    (
                            tok.Homeworld_
                        >>  parse::label(Name_token) >> string_ref_vec [ _val = new_<Condition::Homeworld>(_1) ]
                     )
                |    tok.Homeworld_ [ _val = new_<Condition::Homeworld>() ]
                ;

            building
                =    (
                            tok.Building_
                        >> -(
                                parse::label(Name_token) >> string_ref_vec [ _a = _1 ]
                            )
                     )
                     [ _val = new_<Condition::Building>(_a) ]
                ;

            species
                =    tok.Species_
                >>   (
                            parse::label(Name_token) >> string_ref_vec [ _val = new_<Condition::Species>(_1) ]
                        |   eps [ _val = new_<Condition::Species>() ] // TODO: Is this as useless as it looks?
                     )
                ;

            focus_type
                =    tok.Focus_
                >>   (
                            parse::label(Type_token) >> string_ref_vec [ _val = new_<Condition::FocusType>(_1) ]
                        |   eps [ _val = new_<Condition::FocusType>(std::vector<const ValueRef::ValueRefBase<std::string>*>()) ]
                     )
                ;

            planet_type
                =    tok.Planet_
                >>   parse::label(Type_token)
                >>   (
                            '[' >> +planet_type_value_ref [ push_back(_a, _1) ] >> ']'
                        |   planet_type_value_ref [ push_back(_a, _1) ]
                     )
                     [ _val = new_<Condition::PlanetType>(_a) ]
                ;

            planet_size
                =    tok.Planet_
                >>   parse::label(Size_token)
                >>   (
                            '[' >> +planet_size_value_ref [ push_back(_a, _1) ] >> ']'
                        |   planet_size_value_ref [ push_back(_a, _1) ]
                     )
                     [ _val = new_<Condition::PlanetSize>(_a) ]
                ;

            planet_environment
                =    tok.Planet_
                >>   parse::label(Environment_token)
                >>   (
                            '[' >> +planet_environment_value_ref [ push_back(_a, _1) ] >> ']'
                        |   planet_environment_value_ref [ push_back(_a, _1) ]
                     )
                     [ _val = new_<Condition::PlanetEnvironment>(_a) ]
                ;

            object_type
                =    parse::enum_parser<UniverseObjectType>() [ _val = new_<Condition::Type>(new_<ValueRef::Constant<UniverseObjectType> >(_1)) ]
                |    (
                            tok.ObjectType_
                        >>  parse::label(Type_token) >> universe_object_type_value_ref [ _val = new_<Condition::Type>(_1) ]
                     )
                ;

            meter_value
                =    (
                            parse::non_ship_part_meter_type_enum() [ _a = _1 ]
                        >> -(
                                parse::label(Low_token) >> double_value_ref [ _b = _1 ]
                            )
                        >> -(
                                parse::label(High_token) >> double_value_ref [ _c = _1 ]
                            )
                     )
                     [ _val = new_<Condition::MeterValue>(_a, _b, _c) ]
                ;

            ship_part_meter_value
                =    (      tok.ShipPartMeter_
                        >>  parse::label(Part_token)      >>  tok.string [ _d = _1 ]
                        >>  parse::ship_part_meter_type_enum() [ _a = _1 ]
                        >> -(
                                parse::label(Low_token)  >>  double_value_ref [ _b = _1 ]
                            )
                        >> -(
                                parse::label(High_token) >>  double_value_ref [ _c = _1 ]
                            )
                     )
                     [ _val = new_<Condition::ShipPartMeterValue>(_d, _a, _b, _c) ]
                ;

            empire_meter_value
                =   tok.EmpireMeter_
                >>  (
                        parse::label(Empire_token)   >>  int_value_ref [ _b = _1 ]
                    >>  parse::label(Meter_token)    >>  tok.string [ _a = _1 ]
                    >> -(
                            parse::label(Low_token)  >>  double_value_ref [ _c = _1 ]
                        )
                    >> -(
                            parse::label(High_token) >>  double_value_ref [ _d = _1 ]
                        )
                        [ _val = new_<Condition::EmpireMeterValue>(_b, _a, _c, _d) ]
                    )
                |   (
                        parse::label(Meter_token)    >>  tok.string [ _a = _1 ]
                    >> -(
                            parse::label(Low_token)  >>  double_value_ref [ _c = _1 ]
                        )
                    >> -(
                            parse::label(High_token) >>  double_value_ref [ _d = _1 ]
                        )
                        [ _val = new_<Condition::EmpireMeterValue>(_a, _c, _d) ]
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
                |    meter_value
                |    ship_part_meter_value
                |    empire_meter_value
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
            meter_value.name("MeterValue");
            ship_part_meter_value.name("ShipPartMeterValue");
            empire_meter_value.name("EmpireMeterValue");

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
            debug(meter_value);
            debug(ship_part_meter_value);
            debug(empire_meter_value);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            std::vector<const ValueRef::ValueRefBase<std::string>*> (),
            parse::skipper_type
        > string_ref_vec_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                MeterType,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                std::string
            >,
            parse::skipper_type
        > meter_value_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > empire_meter_value_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<const ValueRef::ValueRefBase<std::string>*> >,
            parse::skipper_type
        > building_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<const ValueRef::ValueRefBase<PlanetType>*> >,
            parse::skipper_type
        > planet_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<const ValueRef::ValueRefBase<PlanetSize>*> >,
            parse::skipper_type
        > planet_size_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*> >,
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
        meter_value_rule                meter_value;
        meter_value_rule                ship_part_meter_value;
        empire_meter_value_rule         empire_meter_value;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_4() {
        static condition_parser_rules_4 retval;
        return retval.start;
    }
} }
