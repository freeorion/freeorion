#include "Parse.h"

#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "ConditionParserImpl.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0
#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<std::string, ValueRef::ValueRefBase<double>*>&)
    { return os; }
}
#endif

namespace {
    using start_rule_payload = std::map<std::string, ValueRef::ValueRefBase<double>*>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok),
            condition_parser(tok, labeller),
            string_grammar(tok, labeller, condition_parser),
            double_rules(tok, labeller, condition_parser, string_grammar)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::insert;
            using phoenix::construct;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_r1_type _r1;
            qi::_val_type _val;

            stat
                =   tok.Statistic_
                >   labeller.rule(Name_token)    > tok.string [ _a = _1 ]
                >   labeller.rule(Value_token)   > double_rules.expr
                [ _val = construct<std::pair<std::string, ValueRef::ValueRefBase<double>*>>(_a, _1) ]
                ;

            start
                =   +stat [ insert(_r1, _1) ]
                ;

            stat.name("Double Statistic ValueRef");

#if DEBUG_PARSERS
            debug(stat);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            std::pair<std::string, ValueRef::ValueRefBase<double>*> (),
            boost::spirit::qi::locals<std::string>
        > stat_rule;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller labeller;
        parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::double_parser_rules      double_rules;
        stat_rule   stat;
        start_rule  start;
    };
}

namespace parse {
    start_rule_payload statistics(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload stats_;

        for (const boost::filesystem::path& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, stats_);
        }

        return stats_;
    }
}
