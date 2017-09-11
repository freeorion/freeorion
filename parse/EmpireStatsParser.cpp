#include "Parse.h"

#include "ParseImpl.h"
#include "ValueRefParser.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0
#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<std::string, ValueRef::ValueRefBase<double>*>&)
    { return os; }
}
#endif

namespace {
    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
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

            const parse::lexer& tok = parse::lexer::instance();

            stat
                =   tok.Statistic_
                >   parse::detail::label(Name_token)    > tok.string [ _a = _1 ]
                >   parse::detail::label(Value_token)   > parse::double_value_ref()
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

        typedef parse::detail::rule<
            void (std::map<std::string, ValueRef::ValueRefBase<double>*>&)
        > start_rule;

        stat_rule   stat;
        start_rule  start;
    };
}

namespace parse {
    std::map<std::string, ValueRef::ValueRefBase<double>*> statistics() {
        std::map<std::string, ValueRef::ValueRefBase<double>*> stats_;

        for (const boost::filesystem::path& file : ListScripts("scripting/empire_statistics")) {
            /*auto success =*/ detail::parse_file<rules, std::map<std::string, ValueRef::ValueRefBase<double>*>>(file, stats_);
        }

        return stats_;
    }
}
