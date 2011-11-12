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
                qi::_c_type _c;
                qi::_d_type _d;
                qi::_e_type _e;
                qi::_r1_type _r1;
                qi::_r2_type _r2;
                qi::eps_type eps;
                using phoenix::new_;
                using phoenix::push_back;

                monster_fleet_plan_prefix
                    =    tok.MonsterFleet_
                    >    parse::label(Name_name) > tok.string [ _r1 = _1 ]
                    ;

                ships
                    =    parse::label(Ships_name)
                    >    (
                              '[' > +tok.string [ push_back(_r1, _1) ] > ']'
                          |   tok.string [ push_back(_r1, _1) ]
                         )
                    ;

                spawns
                    =    (
                              parse::label(SpawnRate_name) >> parse::double_ [ _r1 = _1 ]
                          |   eps [ _r1 = 1.0 ]
                         )
                    >    (
                              parse::label(SpawnLimit_name) >> parse::int_ [ _r2 = _1 ]
                          |   eps [ _r2 = 9999 ]
                         )
                    ;

                monster_fleet_plan
                    =    (
                              monster_fleet_plan_prefix(_a)
                          >   ships(_b)
                          >   spawns(_c, _d)
                          >  -(
                                   parse::label(Location_name) >> parse::detail::condition_parser [ _e = _1 ]
                              )
                         )
                         [ push_back(_r1, new_<MonsterFleetPlan>(_a, _b, _c, _d, _e)) ]
                    ;

                start
                    =   +monster_fleet_plan(_r1)
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
            void (std::string&),
            parse::skipper_type
        > monster_fleet_plan_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<std::string>&),
            parse::skipper_type
        > ships_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (double&, int&),
            parse::skipper_type
        > spawns_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<MonsterFleetPlan*>&),
            qi::locals<
                std::string,
                std::vector<std::string>,
                double,
                int,
                Condition::ConditionBase*
            >,
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
    };

}

namespace parse {

    bool monster_fleet_plans(const boost::filesystem::path& path, std::vector<MonsterFleetPlan*>& monster_fleet_plans_)
    { return detail::parse_file<rules, std::vector<MonsterFleetPlan*> >(path, monster_fleet_plans_); }

}
