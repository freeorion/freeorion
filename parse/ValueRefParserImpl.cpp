#include "ValueRefParserImpl.h"

const parse::detail::reference_token_rule variable_scope(const parse::lexer& tok) {
    boost::spirit::qi::_val_type _val;

    parse::detail::reference_token_rule variable_scope;
    variable_scope
        =   tok.Source_         [ _val = ValueRef::SOURCE_REFERENCE ]
        |   tok.Target_         [ _val = ValueRef::EFFECT_TARGET_REFERENCE ]
        |   tok.LocalCandidate_ [ _val = ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE ]
        |   tok.RootCandidate_  [ _val = ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE ]
        ;

    variable_scope.name("Source, Target, LocalCandidate, or RootCandidate");

    return variable_scope;
}

const parse::detail::name_token_rule container_type(const parse::lexer& tok) {
    parse::detail::name_token_rule container_type;
    container_type
        =   tok.Planet_
        |   tok.System_
        |   tok.Fleet_
        ;

    container_type.name("Planet, System, or Fleet");

    return container_type;
}
