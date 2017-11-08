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
        const parse::detail::condition_payload& location, bool& pass)
    {
        plans.push_back(
            boost::make_unique<MonsterFleetPlan>(
                fleet_name, ship_design_names, spawn_rate, spawn_limit, location.OpenEnvelope(pass)));
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
            int_rule(tok)
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
            qi::eps_type eps;
            qi::_pass_type _pass;
            const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            monster_fleet_plan_prefix
                =    tok.MonsterFleet_
                >    labeller.rule(Name_token) > tok.string [ phoenix::ref(_a) = _1 ]
                ;

            ships
                =    labeller.rule(Ships_token)
                >    eps [ clear(phoenix::ref(_b)) ]
                >    (
                            ('[' > +tok.string [ push_back(phoenix::ref(_b), _1) ] > ']')
                        |    tok.string [ push_back(phoenix::ref(_b), _1) ]
                     )
                ;

            spawns
                =    (
                            (labeller.rule(SpawnRate_token) > double_rule [ phoenix::ref(_c) = _1 ])
                        |    eps [ phoenix::ref(_c) = 1.0 ]
                     )
                >    (
                            (labeller.rule(SpawnLimit_token) > int_rule [ phoenix::ref(_d) = _1 ])
                        |    eps [ phoenix::ref(_d) = 9999 ]
                     )
                ;

            monster_fleet_plan
                =    (
                            monster_fleet_plan_prefix
                        >   ships
                        >   spawns
                        > -(labeller.rule(Location_token) > condition_parser [ phoenix::ref(_e) = _1 ])
                     )
                [ insert_monster_fleet_plan_(
                        _r1,
                        phoenix::ref(_a),
                        phoenix::ref(_b),
                        phoenix::ref(_c),
                        phoenix::ref(_d),
                        phoenix::ref(_e),
                        _pass) ]
                ;

            start = (+monster_fleet_plan(_r1));

            monster_fleet_plan_prefix.name("MonsterFleet");
            ships.name("Ships");
            spawns.name("spawn rate and spawn limit");
            monster_fleet_plan.name("MonsterFleet");

#if DEBUG_PARSERS
            debug(monster_fleet_plan);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using generic_rule = parse::detail::rule<>;

        using monster_fleet_plan_rule = parse::detail::rule<start_rule_signature>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller labeller;
        parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::double_grammar double_rule;
        parse::detail::int_grammar int_rule;
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
        parse::detail::condition_payload   _e;
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
