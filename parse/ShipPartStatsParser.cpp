#include "ShipPartStatsParser.h"

#include "Double.h"
#include "EnumParser.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const PartTypeStats&) { return os; }
}
#endif

namespace {
    struct part_stats_parser_rules {
        part_stats_parser_rules() {
            //const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_g_type _g;
            qi::_h_type _h;
            qi::_i_type _i;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::_r4_type _r4;
            qi::_r5_type _r5;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::construct;

            fighter_stats_prefix
                =    parse::label(Type_name)               >> parse::enum_parser<CombatFighterType>() [ _r1 = _1 ]
                >    parse::label(AntiShipDamage_name)     >  parse::double_ [ _r2 = _1 ]
                >    parse::label(AntiFighterDamage_name)  >  parse::double_ [ _r3 = _1 ]
                >    parse::label(LaunchRate_name)         >  parse::double_ [ _r4 = _1 ]
                >    parse::label(FighterWeaponRange_name) >  parse::double_ [ _r5 = _1 ]
                ;

            fighter_stats
                =    fighter_stats_prefix(_a, _b, _c, _d, _e)
                >    parse::label(Speed_name)              >  parse::double_ [ _f = _1 ]
                >    parse::label(Stealth_name)            >  parse::double_ [ _g = _1 ]
                >    parse::label(Structure_name)          >  parse::double_ [ _h = _1 ]
                >    parse::label(Detection_name)          >  parse::double_ [ _i = _1 ]
                >    parse::label(Capacity_name)           >  parse::int_ [ _val = construct<FighterStats>(_a, _b, _c, _d, _e, _f, _g, _h, _i, _1) ]
                ;

            lr_df_stats_prefix
                =    parse::label(Damage_name) >> parse::double_ [ _r1 = _1 ]
                >    parse::label(ROF_name)    >  parse::double_ [ _r2 = _1 ]
                >    parse::label(Range_name)  >  parse::double_ [ _r3 = _1 ]
                ;

            lr_df_stats
                =    lr_df_stats_prefix(_b, _c, _d)
                >>   (
                            parse::label(Speed_name)     >> parse::double_ [ _e = _1 ]
                        >   parse::label(Stealth_name)   >  parse::double_ [ _f = _1 ]
                        >   parse::label(Structure_name) >  parse::double_ [ _g = _1 ]
                        >   parse::label(Capacity_name)  >  parse::int_ [ _val = construct<LRStats>(_b, _c, _d, _e, _f, _g, _1) ]
                        |   eps [ _val = construct<DirectFireStats>(_b, _c, _d) ]
                        )
                ;

            start
                =    fighter_stats [ _val = _1 ]
                |    lr_df_stats [ _val = _1 ]
                |    parse::label(Capacity_name) > parse::double_ [ _val = _1 ]
                ;

            fighter_stats_prefix.name("fighter stats");
            fighter_stats.name("fighter stats");
            lr_df_stats_prefix.name("LR or DF stats");
            lr_df_stats.name("LR or DF stats");
            start.name("Part Stats");

#if DEBUG_PARSERS
            debug(fighter_stats_prefix);
            debug(fighter_stats);
            debug(lr_df_stats_prefix);
            debug(lr_df_stats);
            debug(start);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (CombatFighterType&, double&, double&, double&, double&),
            parse::skipper_type
        > fighter_stats_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (double&, double&, double&),
            parse::skipper_type
        > lr_df_stats_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            PartTypeStats (),
            qi::locals<
                CombatFighterType,
                double,
                double,
                double,
                double,
                double,
                double,
                double,
                double
            >,
            parse::skipper_type
        > part_stats_rule;

        fighter_stats_prefix_rule               fighter_stats_prefix;
        part_stats_rule                         fighter_stats;
        lr_df_stats_prefix_rule                 lr_df_stats_prefix;
        part_stats_rule                         lr_df_stats;
        parse::detail::part_stats_parser_rule   start;
    };
}

namespace parse { namespace detail {
    part_stats_parser_rule& part_stats_parser() {
        static part_stats_parser_rules rules;
        return rules.start;
    }
} }
