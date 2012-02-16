#include "ParseImpl.h"

#include "Double.h"
#include "Int.h"
#include "Label.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<MonsterFleetPlan*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    struct new_monster_fleet_plan_ {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
        struct result
        { typedef MonsterFleetPlan* type; };

        MonsterFleetPlan* operator()(const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
                                     double spawn_rate, int spawn_limit, const Condition::ConditionBase* location) const
        { return new MonsterFleetPlan(fleet_name, ship_design_names, spawn_rate, spawn_limit, location); }
    };
    const boost::phoenix::function<new_monster_fleet_plan_> new_monster_fleet_plan;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::clear;
            using phoenix::new_;
            using phoenix::push_back;

            monster_fleet_plan_prefix
                =    tok.MonsterFleet_
                >    parse::label(Name_name) > tok.string [ phoenix::ref(_a) = _1 ]
                ;

            ships
                =    parse::label(Ships_name)
                >>   eps [ clear(phoenix::ref(_b)) ]
                >    (
                            '[' > +tok.string [ push_back(phoenix::ref(_b), _1) ] > ']'
                        |   tok.string [ push_back(phoenix::ref(_b), _1) ]
                     )
                ;

            spawns
                =    (
                            parse::label(SpawnRate_name) >> parse::double_ [ phoenix::ref(_c) = _1 ]
                        |   eps [ phoenix::ref(_c) = 1.0 ]
                     )
                >    (
                            parse::label(SpawnLimit_name) >> parse::int_ [ phoenix::ref(_d) = _1 ]
                        |   eps [ phoenix::ref(_d) = 9999 ]
                     )
                ;

            monster_fleet_plan
                =    (
                            monster_fleet_plan_prefix
                        >   ships
                        >   spawns
                        >  -(
                                parse::label(Location_name) >> parse::detail::condition_parser [ phoenix::ref(_e) = _1 ]
                            )
                     )
                     [ _val = new_monster_fleet_plan(phoenix::ref(_a), phoenix::ref(_b), phoenix::ref(_c), phoenix::ref(_d), phoenix::ref(_e)) ]
                ;

            start
                =   (+monster_fleet_plan) [ _r1 = _1 ]
                ;

            monster_fleet_plan_prefix.name("MonsterFleet");
            ships.name("Ships");
            spawns.name("spawn rate and spawn limit");
            monster_fleet_plan.name("MonsterFleet");

#if DEBUG_PARSERS
            debug(monster_fleet_plan);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            parse::skipper_type
        > monster_fleet_plan_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            parse::skipper_type
        > ships_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            parse::skipper_type
        > spawns_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            MonsterFleetPlan* (),
            parse::skipper_type
        > monster_fleet_plan_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<MonsterFleetPlan*>&),
            parse::skipper_type
        > start_rule;

        monster_fleet_plan_prefix_rule monster_fleet_plan_prefix;
        ships_rule ships;
        spawns_rule spawns;
        monster_fleet_plan_rule monster_fleet_plan;
        start_rule start;

        // locals
        std::string _a;
        std::vector<std::string> _b;
        double _c;
        int _d;
        Condition::ConditionBase* _e;
    };

}

namespace parse {
    bool monster_fleet_plans(const boost::filesystem::path& path, std::vector<MonsterFleetPlan*>& monster_fleet_plans_)
    { return detail::parse_file<rules, std::vector<MonsterFleetPlan*> >(path, monster_fleet_plans_); }
}
