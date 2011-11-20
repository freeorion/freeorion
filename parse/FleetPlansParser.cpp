#include "ParseImpl.h"

#include "Label.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<FleetPlan*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    struct rules
    {
        rules()
        {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_r1_type _r1;
            using phoenix::new_;
            using phoenix::push_back;

            fleet_plan
                =    tok.Fleet_
                >    parse::label(Name_name) > tok.string [ _a = _1 ]
                >    parse::label(Ships_name)
                >    (
                            '[' > +tok.string [ push_back(_b, _1) ] > ']'
                        |   tok.string [ push_back(_b, _1) ]
                        )
                        [ push_back(_r1, new_<FleetPlan>(_a, _b)) ]
                ;

            start
                =   +fleet_plan(_r1)
                ;

            fleet_plan.name("Fleet");

#if DEBUG_PARSERS
            debug(fleet_plan);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<FleetPlan*>&),
            qi::locals<
                std::string,
                std::vector<std::string>
            >,
            parse::skipper_type
        > fleet_plan_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<FleetPlan*>&),
            parse::skipper_type
        > start_rule;

        fleet_plan_rule fleet_plan;
        start_rule start;
    };
}

namespace parse {
    bool fleet_plans(const boost::filesystem::path& path, std::vector<FleetPlan*>& fleet_plans_)
    { return detail::parse_file<rules, std::vector<FleetPlan*> >(path, fleet_plans_); }
}
