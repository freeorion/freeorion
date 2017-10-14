#include "Parse.h"

#include "ParseImpl.h"

#include "../universe/UniverseGenerator.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<FleetPlan*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    using start_rule_payload = std::vector<FleetPlan*>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::new_;
            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_r1_type _r1;

            fleet_plan
                =    tok.Fleet_
                >    labeller.rule(Name_token) > tok.string [ _a = _1 ]
                >    labeller.rule(Ships_token)
                >    (
                            ('[' > +tok.string [ push_back(_b, _1) ] > ']')
                        |    tok.string [ push_back(_b, _1) ]
                     )
                [ push_back(_r1, new_<FleetPlan>(_a, _b, phoenix::val(true))) ]
                ;

            start
                =   +fleet_plan(_r1)
                ;

            fleet_plan.name("Fleet");

#if DEBUG_PARSERS
            debug(fleet_plan);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::vector<FleetPlan*>&),
            boost::spirit::qi::locals<
                std::string,
                std::vector<std::string>
            >
        > fleet_plan_rule;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller labeller;
        fleet_plan_rule fleet_plan;
        start_rule start;
    };
}

namespace parse {
    start_rule_payload fleet_plans(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload fleet_plans_;
        /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, path, fleet_plans_);
        return fleet_plans_;
    }
}
