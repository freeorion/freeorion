#include "ValueRefParser.h"

#include "ConditionParserImpl.h"
#include "MovableEnvelope.h"
#include "../universe/ValueRefs.h"

#include <boost/phoenix.hpp>


#define DEBUG_VALUEREF_PARSERS 0

// These are just here to satisfy the requirements of boost::spirit::qi::debug(<rule>).
#if DEBUG_VALUEREF_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<std::variant<ValueRef::OpType, value_ref_payload<int>>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::variant<ValueRef::OpType, value_ref_payload<double>>>&) { return os; }
}
#endif

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse::detail {

    const reference_token_rule variable_scope(const parse::lexer& tok) {
        qi::_val_type _val;

        reference_token_rule variable_scope;
        variable_scope
            =   tok.Source_         [ _val = ValueRef::ReferenceType::SOURCE_REFERENCE ]
            |   tok.Target_         [ _val = ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE ]
            |   tok.LocalCandidate_ [ _val = ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE ]
            |   tok.RootCandidate_  [ _val = ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE ]
            ;

        variable_scope.name("Source, Target, LocalCandidate, or RootCandidate");

        return variable_scope;
    }

    const container_token_rule container(const parse::lexer& tok) {
        qi::eps_type eps;
        qi::_val_type _val;

        container_token_rule container_type =
                tok.Planet_ [ _val = ValueRef::ContainerType::PLANET ]
            |   tok.System_ [ _val = ValueRef::ContainerType::SYSTEM ]
            |   tok.Fleet_  [ _val = ValueRef::ContainerType::FLEET ]
            |   eps         [ _val = ValueRef::ContainerType::NONE ]
            ;

        container_type.name("Planet, System, or Fleet");

        return container_type;
    }

    template <typename T>
    simple_variable_rules<T>::simple_variable_rules(
        const std::string& type_name, const parse::lexer& tok)
    {
        using phoenix::new_;

        qi::_1_type _1;
        qi::_val_type _val;
        qi::lit_type lit;
        const phoenix::function<construct_movable> construct_movable_;

        free_variable
            =  (tok.Value_ >> !lit('('))
                [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                    ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE)) ]
            |   free_variable_name
                [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                    ValueRef::ReferenceType::NON_OBJECT_REFERENCE, _1)) ]
            ;

        simple
            =   constant
            |   bound_variable
            |   free_variable
            ;

        variable_scope_rule = variable_scope(tok);
        container_type_rule = container(tok);
        initialize_bound_variable_parser<T>(
            bound_variable, unwrapped_bound_variable,
            value_wrapped_bound_variable, bound_variable_name,
            variable_scope_rule, container_type_rule, tok);

#if DEBUG_VALUEREF_PARSERS
        debug(bound_variable_name);
        debug(free_variable_name);
        debug(constant);
        debug(free_variable);
        debug(bound_variable);
        debug(simple);
#endif

        unwrapped_bound_variable.name(type_name + " unwrapped bound variable name");
        value_wrapped_bound_variable.name(type_name + " value-wrapped bound variable name");
        bound_variable_name.name(type_name + " bound variable name");
        free_variable_name.name(type_name + " free variable name");
        constant.name(type_name + " constant");
        free_variable.name(type_name + " free variable");
        bound_variable.name(type_name + " bound variable");
        simple.name(type_name + " simple variable expression");
    }

    // Explicit instantiation to prevent costly recompilation in multiple units
    template simple_variable_rules<int>::simple_variable_rules(
        const std::string& type_name, const parse::lexer& tok);
    template simple_variable_rules<double>::simple_variable_rules(
        const std::string& type_name, const parse::lexer& tok);
}
