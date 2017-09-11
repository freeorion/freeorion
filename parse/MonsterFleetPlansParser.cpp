#include "Parse.h"

#include "ParseImpl.h"
#include "ConditionParserImpl.h"

#include "../universe/Universe.h"
#include "../universe/UniverseGenerator.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<MonsterFleetPlan*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    struct new_monster_fleet_plan_ {
        typedef MonsterFleetPlan* result_type;

        MonsterFleetPlan* operator()(const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
                                     double spawn_rate, int spawn_limit, Condition::ConditionBase* location) const
        { return new MonsterFleetPlan(fleet_name, ship_design_names, spawn_rate, spawn_limit, location); }
    };
    const boost::phoenix::function<new_monster_fleet_plan_> new_monster_fleet_plan;

    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::clear;
            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            monster_fleet_plan_prefix
                =    tok.MonsterFleet_
                >    parse::detail::label(Name_token) > tok.string [ phoenix::ref(_a) = _1 ]
                ;

            ships
                =    parse::detail::label(Ships_token)
                >    eps [ clear(phoenix::ref(_b)) ]
                >    (
                            ('[' > +tok.string [ push_back(phoenix::ref(_b), _1) ] > ']')
                        |    tok.string [ push_back(phoenix::ref(_b), _1) ]
                     )
                ;

            spawns
                =    (
                            (parse::detail::label(SpawnRate_token) > parse::detail::double_ [ phoenix::ref(_c) = _1 ])
                        |    eps [ phoenix::ref(_c) = 1.0 ]
                     )
                >    (
                            (parse::detail::label(SpawnLimit_token) > parse::detail::int_ [ phoenix::ref(_d) = _1 ])
                        |    eps [ phoenix::ref(_d) = 9999 ]
                     )
                ;

            monster_fleet_plan
                =    (
                            monster_fleet_plan_prefix
                        >   ships
                        >   spawns
                        > -(parse::detail::label(Location_token) > parse::detail::condition_parser [ phoenix::ref(_e) = _1 ])
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

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<> generic_rule;

        typedef parse::detail::rule<
            MonsterFleetPlan* ()
        > monster_fleet_plan_rule;

        typedef parse::detail::rule<
            void (std::vector<MonsterFleetPlan*>&)
        > start_rule;

        generic_rule            monster_fleet_plan_prefix;
        generic_rule            ships;
        generic_rule            spawns;
        monster_fleet_plan_rule monster_fleet_plan;
        start_rule              start;

        // locals
        std::string                 _a;
        std::vector<std::string>    _b;
        double                      _c;
        int                         _d;
        Condition::ConditionBase*   _e;
    };

}

namespace parse {
    std::vector<MonsterFleetPlan*> monster_fleet_plans() {
        std::vector<MonsterFleetPlan*> monster_fleet_plans_;
        boost::filesystem::path path = GetResourceDir() / "scripting/monster_fleets.inf";
        /*auto success =*/ detail::parse_file<rules, std::vector<MonsterFleetPlan*>>(path, monster_fleet_plans_);
        return monster_fleet_plans_;
    }
}
