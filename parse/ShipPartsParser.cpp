#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Double.h"
#include "EnumParser.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"
#include "../universe/Condition.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipSlotType>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, PartType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, PartType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const PartTypeStats&) { return os; }
}
#endif

namespace {
    struct insert_ {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, PartType*>& part_types, PartType* part_type) const {
            if (!part_types.insert(std::make_pair(part_type->Name(), part_type)).second) {
                std::string error_str = "ERROR: More than one ship part in ship_parts.txt has the name " + part_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules() {
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
            using phoenix::new_;
            using phoenix::push_back;

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

            part_stats
                =    fighter_stats [ _val = _1 ]
                |    lr_df_stats [ _val = _1 ]
                |    parse::label(Capacity_name) > parse::double_ [ _val = _1 ]
                ;

            part_type_prefix
                =    tok.Part_
                >    parse::label(Name_name)        > tok.string [ _r1 = _1 ]
                >    parse::label(Description_name) > tok.string [ _r2 = _1 ]
                >    parse::label(Class_name)       > parse::enum_parser<ShipPartClass>() [ _r3 = _1 ]
                ;

            cost
                =    parse::label(BuildCost_name)   > parse::double_ [ _r1 = _1 ]
                >    parse::label(BuildTime_name)   > parse::int_ [ _r2 = _1 ]
                ;

            producible
                =    tok.Unproducible_ [ _val = false ]
                |    tok.Producible_ [ _val = true ]
                |    eps [ _val = true ]
                ;

            slots
                =  -(
                        parse::label(MountableSlotTypes_name)
                    >>  (
                            '[' > +parse::enum_parser<ShipSlotType>() [ push_back(_r1, _1) ] > ']'
                        |   parse::enum_parser<ShipSlotType>() [ push_back(_r1, _1) ]
                        )
                     )
                ;

            location
                =    parse::label(Location_name) >> parse::detail::condition_parser [ _r1 = _1 ]
                |    eps [ _r1 = new_<Condition::All>() ]
                ;

            tags
                =  -(
                        parse::label(Tags_name)
                    >>  (
                            '[' > +tok.string [ push_back(_r1, _1) ] > ']'
                            |   tok.string [ push_back(_r1, _1) ]
                        )
                    )
                ;

            common_params
                =   parse::label(BuildCost_name)     > parse::double_ [ _a = _1 ]
                >   parse::label(BuildTime_name)     > parse::int_    [ _b = _1 ]
                >   producible                                        [ _c = _1 ]
                >   tags(_d)
                >   location(_e)
                >   -(
                        parse::label(EffectsGroups_name) >> parse::detail::effects_group_parser() [ _f = _1 ]
                     )
                >    parse::label(Icon_name) > tok.string
                    [ _val = construct<PartHullCommonParams>(_a, _b, _c, _d, _e, _f, _1) ]
            ;

            part_type
                =    part_type_prefix(_a, _b, _c)
                >>   part_stats [ _d = _1 ]
                >>   slots(_f)
                >>   common_params [ _e = _1 ]
                    [ insert(_r1, new_<PartType>(_a, _b, _c, _d, _e, _f)) ]
                ;

            start
                =   +part_type(_r1)
                ;

            fighter_stats_prefix.name("fighter stats");
            fighter_stats.name("fighter stats");
            lr_df_stats_prefix.name("LR or DF stats");
            lr_df_stats.name("LR or DF stats");
            part_stats.name("Part stats");
            part_type_prefix.name("Part");
            cost.name("cost");
            producible.name("Producible or Unproducible");
            slots.name("mountable slot types");
            location.name("Location");
            tags.name("Tags");
            common_params.name("Part Hull Common Params");
            part_type.name("Part");

#if DEBUG_PARSERS
            debug(fighter_stats_prefix);
            debug(fighter_stats);
            debug(lr_df_stats_prefix);
            debug(lr_df_stats);
            debug(part_stats);
            debug(part_type_prefix);
            debug(cost);
            debug(producible);
            debug(slots);
            debug(location);
            debug(tags);
            debug(common_params);
            debug(part_type);
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::string&, std::string&, ShipPartClass&),
            parse::skipper_type
        > part_type_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (double&, int&),
            parse::skipper_type
        > cost_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            bool (),
            parse::skipper_type
        > producible_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<ShipSlotType>&),
            parse::skipper_type
        > slots_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (Condition::ConditionBase*&),
            parse::skipper_type
        > location_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<std::string>&),
            parse::skipper_type
        > tags_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            PartHullCommonParams (),
            qi::locals<
                double,
                int,
                bool,
                std::vector<std::string>,
                Condition::ConditionBase*,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
            >,
            parse::skipper_type
        > part_hull_common_params_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, PartType*>&),
            qi::locals<
                std::string,
                std::string,
                ShipPartClass,
                PartTypeStats,
                PartHullCommonParams,
                std::vector<ShipSlotType>
            >,
            parse::skipper_type
        > part_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, PartType*>&),
            parse::skipper_type
        > start_rule;

        fighter_stats_prefix_rule       fighter_stats_prefix;
        part_stats_rule                 fighter_stats;
        lr_df_stats_prefix_rule         lr_df_stats_prefix;
        part_stats_rule                 lr_df_stats;
        part_stats_rule                 part_stats;
        part_type_prefix_rule           part_type_prefix;
        cost_rule                       cost;
        producible_rule                 producible;
        location_rule                   location;
        tags_rule                       tags;
        part_hull_common_params_rule    common_params;
        slots_rule                      slots;
        part_type_rule                  part_type;
        start_rule                      start;
    };

}

namespace parse {
    bool ship_parts(const boost::filesystem::path& path, std::map<std::string, PartType*>& parts)
    { return detail::parse_file<rules, std::map<std::string, PartType*> >(path, parts); }
}
