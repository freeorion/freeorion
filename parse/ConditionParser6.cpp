#include "ConditionParser6.h"

#include "ValueRefParserImpl.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<condition_payload>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<std::string>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<PlanetSize>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<PlanetType>*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<PlanetEnvironment>*>&) { return os; }
}
#endif

namespace parse { namespace detail {
    condition_parser_rules_6::condition_parser_rules_6(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_6::base_type(start, "condition_parser_rules_6"),
        universe_object_type_rules(tok, labeller, condition_parser),
        planet_type_rules(tok, labeller, condition_parser),
        planet_size_rules(tok, labeller, condition_parser),
        planet_environment_rules(tok, labeller, condition_parser)
  {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_r1_type _r1;
        qi::_val_type _val;
        qi::eps_type eps;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<parse::detail::lazy_move> lazy_move_;

        using phoenix::new_;
        using phoenix::push_back;
        using phoenix::construct;

        string_ref_vec
            =   ('[' > +string_grammar [ emplace_back_1_(_r1, _1) ] > ']')
            |    string_grammar [ emplace_back_1_(_r1, _1) ]
            ;

        homeworld
            =   tok.Homeworld_
            >   (
                (labeller.rule(Name_token) > string_ref_vec(_a) [ _val = construct_movable_(new_<Condition::Homeworld>(lazy_move_(_a))) ])
                |    eps [ _val = construct_movable_(new_<Condition::Homeworld>()) ]
            )
            ;

        building
            =   (
                tok.Building_
                >  -(labeller.rule(Name_token) > string_ref_vec(_a))
            )
            [ _val = construct_movable_(new_<Condition::Building>(lazy_move_(_a))) ]
            ;

        species
            =   tok.Species_
            >   (
                (labeller.rule(Name_token) > string_ref_vec(_a) [ _val = construct_movable_(new_<Condition::Species>(lazy_move_(_a))) ])
                |    eps [ _val = construct_movable_(new_<Condition::Species>()) ]
            )
            ;

        focus_type
            =   tok.Focus_
            >   (
                (labeller.rule(Type_token) > string_ref_vec(_a) [ _val = construct_movable_(new_<Condition::FocusType>(lazy_move_(_a))) ])
                | eps [ _val = construct_movable_(new_<Condition::FocusType>(lazy_move_(_a))) ]
            )
            ;

        planet_type
            =   (tok.Planet_
                 >>  labeller.rule(Type_token)
                )
            >   (
                ('[' > +planet_type_rules.expr [ emplace_back_1_(_a, _1) ] > ']')
                |    planet_type_rules.expr [ emplace_back_1_(_a, _1) ]
            )
            [ _val = construct_movable_(new_<Condition::PlanetType>(lazy_move_(_a))) ]
            ;

        planet_size
            =   (tok.Planet_
                 >>  labeller.rule(Size_token)
                )
            >   (
                ('[' > +planet_size_rules.expr [ emplace_back_1_(_a, _1) ] > ']')
                |    planet_size_rules.expr [ emplace_back_1_(_a, _1) ]
            )
            [ _val = construct_movable_(new_<Condition::PlanetSize>(lazy_move_(_a))) ]
            ;

        planet_environment
            =   ((tok.Planet_
                  >>  labeller.rule(Environment_token)
                 )
                 >   (
                     ('[' > +planet_environment_rules.expr [ emplace_back_1_(_a, _1) ] > ']')
                     |    planet_environment_rules.expr [ emplace_back_1_(_a, _1) ]
                 )
                 >  -(labeller.rule(Species_token)        >  string_grammar [_b = _1]))
            [ _val = construct_movable_(new_<Condition::PlanetEnvironment>(
                    lazy_move_(_a),
                    construct<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>(_b))) ]
            ;

        object_type
            =   universe_object_type_rules.enum_expr [ _val = construct_movable_(new_<Condition::Type>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>>(
                    new_<ValueRef::Constant<UniverseObjectType>>(_1)))) ]
            |   (
                tok.ObjectType_
                >   labeller.rule(Type_token) > universe_object_type_rules.expr [ _val = construct_movable_(new_<Condition::Type>(
                        construct<std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>>(_1))) ]
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
