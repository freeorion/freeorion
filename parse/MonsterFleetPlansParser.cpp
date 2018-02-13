#include "Parse.h"

#include "ParseImpl.h"
#include "MovableEnvelope.h"
#include "ConditionParserImpl.h"

#include "../universe/Universe.h"
#include "../universe/UniverseGenerator.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<parse::detail::MovableEnvelope<MonsterFleetPlan>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    void insert_monster_fleet_plan(
        std::vector<std::unique_ptr<MonsterFleetPlan>>& plans,
        const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
        double spawn_rate, int spawn_limit,
        const boost::optional<parse::detail::condition_payload>& location, bool& pass)
    {
        plans.push_back(
            boost::make_unique<MonsterFleetPlan>(
                fleet_name, ship_design_names, spawn_rate, spawn_limit,
                (location ? location->OpenEnvelope(pass) : nullptr)
            ));
    };
    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_monster_fleet_plan_, insert_monster_fleet_plan, 7)

    using start_rule_payload = std::vector<std::unique_ptr<MonsterFleetPlan>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok),
            condition_parser(tok, labeller),
            string_grammar(tok, labeller, condition_parser),
            double_rule(tok),
            int_rule(tok),
            one_or_more_string_tokens(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_r1_type _r1;
            qi::eps_type eps;
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::omit_type omit_;
            const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            ships
                =    labeller.rule(Ships_token) > one_or_more_string_tokens
                ;

            spawn_rate =
                (labeller.rule(SpawnRate_token) > double_rule [ _val = _1 ])
                |    eps [ _val = 1.0 ]
                ;

            spawn_limit =
                (labeller.rule(SpawnLimit_token) > int_rule [ _val = _1 ])
                |    eps [ _val = 9999 ]
                ;

            monster_fleet_plan
                = ( omit_[tok.MonsterFleet_]
                    > labeller.rule(Name_token) > tok.string
                    > ships
                    > spawn_rate
                    > spawn_limit
                    > -(labeller.rule(Location_token) > condition_parser)
                ) [ insert_monster_fleet_plan_(_r1, _1, _2, _3, _4, _5, _pass) ]
                ;

            start = (+monster_fleet_plan(_r1));

            ships.name("Ships");
            spawn_rate.name("spawn rate");
            spawn_limit.name("spawn limit");
            monster_fleet_plan.name("MonsterFleet");

#if DEBUG_PARSERS
            debug(monster_fleet_plan);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using monster_fleet_plan_rule = parse::detail::rule<start_rule_signature>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller            labeller;
        parse::conditions_parser_grammar   condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::double_grammar      double_rule;
        parse::detail::int_grammar         int_rule;
        parse::detail::single_or_repeated_string<std::vector<std::string>> one_or_more_string_tokens;
        parse::detail::rule<std::vector<std::string>()> ships;
        parse::detail::rule<double()>      spawn_rate;
        parse::detail::rule<int()>         spawn_limit;
        monster_fleet_plan_rule            monster_fleet_plan;
        start_rule                         start;
    };

}

namespace parse {
    start_rule_payload monster_fleet_plans(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload monster_fleet_plans_;
        /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, path, monster_fleet_plans_);
        return monster_fleet_plans_;
    }
}
