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
    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
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

            const parse::lexer& tok = parse::lexer::instance();

            fleet_plan
                =    tok.Fleet_
                >    parse::detail::label(Name_token) > tok.string [ _a = _1 ]
                >    parse::detail::label(Ships_token)
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

        typedef parse::detail::rule<
            void (std::vector<FleetPlan*>&)
        > start_rule;

        fleet_plan_rule fleet_plan;
        start_rule start;
    };
}

namespace parse {
    std::vector<FleetPlan*> fleet_plans() {
        std::vector<FleetPlan*> fleet_plans_;
        const boost::filesystem::path& path = GetResourceDir() / "scripting/starting_unlocks/fleets.inf";
        /*auto success =*/ detail::parse_file<rules, std::vector<FleetPlan*>>(path, fleet_plans_);
        return fleet_plans_;
    }
}
