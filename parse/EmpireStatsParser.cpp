#include "ValueRefParser.h"
#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"

#include <boost/spirit/home/phoenix.hpp>

#define DEBUG_PARSERS 0
#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<std::string, ValueRef::ValueRefBase<double>*>&)
    { return os; }
}
#endif

namespace {
    struct rules {
        rules() {
            const parse::lexer& tok =                                               parse::lexer::instance();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =    parse::value_ref_parser<double>();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_r1_type _r1;
            qi::_val_type _val;
            using phoenix::insert;
            using phoenix::construct;


            stat
                =   tok.Statistic_
                >   parse::label(Name_token)    > tok.string [ _a = _1 ]
                >   parse::label(Value_token)   > double_value_ref
                [ _val = construct<std::pair<std::string, const ValueRef::ValueRefBase<double>*> >(_a, _1) ]
                ;

            start
                =   +stat [ insert(_r1, _1) ]
                ;

            stat.name("Double Statistic ValueRef");

#if DEBUG_PARSERS
            debug(stat);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            std::pair<std::string, const ValueRef::ValueRefBase<double>*> (),
            qi::locals<std::string>,
            parse::skipper_type
        > stat_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, const ValueRef::ValueRefBase<double>*>&),
            parse::skipper_type
        > start_rule;

        stat_rule   stat;
        start_rule  start;
    };
}

namespace parse {
    bool statistics(const boost::filesystem::path& path,
                    std::map<std::string, const ValueRef::ValueRefBase<double>*>& stats_)
    {
        return detail::parse_file<rules, std::map<std::string, const ValueRef::ValueRefBase<double>*> >(
            path, stats_);
    }
}
