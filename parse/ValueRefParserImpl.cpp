#include "ValueRefParserImpl.h"

namespace {
    struct variable_parser_rules {
        variable_parser_rules() {
            const parse::lexer& tok = parse::lexer::instance();

            variable_scope
                =   tok.Source_
                |   tok.Target_
                |   tok.LocalCandidate_
                |   tok.RootCandidate_
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

        name_token_rule variable_scope;
        name_token_rule container_type;
    };

    variable_parser_rules& get_variable_parser_rules() {
        static variable_parser_rules retval;
        return retval;
    }
}

const name_token_rule& variable_scope()
{ return get_variable_parser_rules().variable_scope; }

const name_token_rule& container_type()
{ return get_variable_parser_rules().container_type; }
