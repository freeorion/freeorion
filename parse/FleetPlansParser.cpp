#include "Parse.h"

#include "ParseImpl.h"
#include "MovableEnvelope.h"

#include "../universe/UniverseGenerator.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<parse::detail::MovableEnvelope<FleetPlan>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    void insert_fleet_plan(
        std::vector<std::unique_ptr<FleetPlan>>& plans,
        const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
        bool lookup_names)
    {
        plans.push_back(
            boost::make_unique<FleetPlan>(
                fleet_name, ship_design_names, lookup_names));
    }
    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_fleet_plan_, insert_fleet_plan, 4)

    using start_rule_payload = std::vector<std::unique_ptr<FleetPlan>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            one_or_more_string_tokens(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::new_;
            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_r1_type _r1;
            qi::omit_type omit_;

            fleet_plan
                =  ( omit_[tok.Fleet_]
                >    label(tok.Name_)   >   tok.string
                >    label(tok.Ships_)  >   one_or_more_string_tokens )
                [ insert_fleet_plan_(_r1, _1, _2, phoenix::val(true)) ]
                ;

            start
            =   (+fleet_plan(_r1));

            fleet_plan.name("Fleet");

#if DEBUG_PARSERS
            debug(fleet_plan);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        parse::detail::single_or_repeated_string<std::vector<std::string>> one_or_more_string_tokens;
        start_rule fleet_plan;
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
