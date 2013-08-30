#include "ValueRefParserImpl.h"

namespace {
    struct variable_parser_rules {
        variable_parser_rules() {
            qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

            variable_scope
                =   tok.Source_         [ _val = ValueRef::SOURCE_REFERENCE ]
                |   tok.Target_         [ _val = ValueRef::EFFECT_TARGET_REFERENCE ]
                |   tok.LocalCandidate_ [ _val = ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE ]
                |   tok.RootCandidate_  [ _val = ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE ]
                ;

            container_type
                =   tok.Planet_
                |   tok.System_
                |   tok.Fleet_
                ;

            variable_scope.name("Source, Target, LocalCandidate, or RootCandidate");
            container_type.name("Planet, System, or Fleet");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_scope);
            debug(container_type);
#endif
        }

        reference_token_rule variable_scope;
        name_token_rule container_type;
    };

    variable_parser_rules& get_variable_parser_rules() {
        static variable_parser_rules retval;
        return retval;
    }
}

const reference_token_rule& variable_scope()
{ return get_variable_parser_rules().variable_scope; }

const name_token_rule& container_type()
{ return get_variable_parser_rules().container_type; }
