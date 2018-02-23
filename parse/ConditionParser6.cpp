#include "ConditionParser6.h"

#include "ValueRefParser.h"

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
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_6::base_type(start, "condition_parser_rules_6"),
        one_or_more_string_values(string_grammar),
        universe_object_type_rules(tok, label, condition_parser),
        planet_type_rules(tok, label, condition_parser),
        planet_size_rules(tok, label, condition_parser),
        planet_environment_rules(tok, label, condition_parser),
        one_or_more_planet_types(planet_type_rules.expr),
        one_or_more_planet_sizes(planet_size_rules.expr),
        one_or_more_planet_environments(planet_environment_rules.expr)
  {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        using phoenix::new_;
        using phoenix::push_back;
        using phoenix::construct;

        homeworld
            =   tok.Homeworld_
            >   (
                (label(tok.Name_) > one_or_more_string_values
                 [ _val = construct_movable_(new_<Condition::Homeworld>(deconstruct_movable_vector_(_1, _pass))) ])
                |    eps [ _val = construct_movable_(new_<Condition::Homeworld>()) ]
            )
            ;

        building
            =   ( omit_[tok.Building_]
                > -(label(tok.Name_) > one_or_more_string_values)
                ) [ _val = construct_movable_(new_<Condition::Building>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        species
            = ( omit_[tok.Species_]
                > -(label(tok.Name_) > one_or_more_string_values)
              ) [ _val = construct_movable_(new_<Condition::Species>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        focus_type
            =   tok.Focus_
            > -(label(tok.Type_) > one_or_more_string_values)
            [ _val = construct_movable_(new_<Condition::FocusType>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        planet_type
            =   (tok.Planet_
                 >>  label(tok.Type_)
                )
            >   one_or_more_planet_types
            [ _val = construct_movable_(new_<Condition::PlanetType>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        planet_size
            =   (tok.Planet_
                 >>  label(tok.Size_)
                )
            >   one_or_more_planet_sizes
            [ _val = construct_movable_(new_<Condition::PlanetSize>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        planet_environment
            =   ((omit_[tok.Planet_]
                  >>  label(tok.Environment_)
                 )
                 >   one_or_more_planet_environments
                 >  -(label(tok.Species_)        >  string_grammar))
            [ _val = construct_movable_(new_<Condition::PlanetEnvironment>(
                    deconstruct_movable_vector_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        object_type
            =   universe_object_type_rules.enum_expr [
                _val = construct_movable_(
                    new_<Condition::Type>(
                        construct<std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>>(
                            new_<ValueRef::Constant<UniverseObjectType>>(_1)))) ]
            |   (
                tok.ObjectType_
                >   label(tok.Type_) > universe_object_type_rules.expr [
                    _val = construct_movable_(new_<Condition::Type>(deconstruct_movable_(_1, _pass))) ]
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

        one_or_more_string_values.name("sequence of string expressions");
        homeworld.name("Homeworld");
        building.name("Building");
        species.name("Species");
        focus_type.name("Focus");
        planet_type.name("PlanetType");
        planet_size.name("PlanetSize");
        planet_environment.name("PlanetEnvironment");
        object_type.name("ObjectType");

#if DEBUG_CONDITION_PARSERS
        debug(one_or_more_string_values);
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
