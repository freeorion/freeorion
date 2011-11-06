#define FUSION_MAX_VECTOR_SIZE 20
#define PHOENIX_LIMIT 11

#include "Double.h"
#include "EnumParser.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"


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

    struct insert_
    {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, PartType*>& part_types, PartType* part_type) const
            {
                if (!part_types.insert(std::make_pair(part_type->Name(), part_type)).second) {
                    std::string error_str = "ERROR: More than one ship part in ship_parts.txt has the name " + part_type->Name();
                    throw std::runtime_error(error_str.c_str());
                }
            }
    };
    const boost::phoenix::function<insert_> insert;

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
                qi::_f_type _f;
                qi::_g_type _g;
                qi::_h_type _h;
                qi::_i_type _i;
                qi::_j_type _j;
                qi::_r1_type _r1;
                qi::_val_type _val;
                qi::eps_type eps;
                using phoenix::construct;
                using phoenix::new_;
                using phoenix::push_back;

                part_stats
                    =    (
                              parse::label(Type_name)               >> parse::enum_parser<CombatFighterType>() [ _a = _1 ]
                          >   parse::label(AntiShipDamage_name)     >  parse::double_ [ _b = _1 ]
                          >   parse::label(AntiFighterDamage_name)  >  parse::double_ [ _c = _1 ]
                          >   parse::label(LaunchRate_name)         >  parse::double_ [ _d = _1 ]
                          >   parse::label(FighterWeaponRange_name) >  parse::double_ [ _e = _1 ]
                          >   parse::label(Speed_name)              >  parse::double_ [ _f = _1 ]
                          >   parse::label(Stealth_name)            >  parse::double_ [ _g = _1 ]
                          >   parse::label(Structure_name)          >  parse::double_ [ _h = _1 ]
                          >   parse::label(Detection_name)          >  parse::double_ [ _i = _1 ]
                          >   parse::label(Capacity_name)           >  parse::int_ [ _val = construct<FighterStats>(_a, _b, _c, _d, _e, _f, _g, _h, _i, _1) ]
                         )
                    |    (
                              parse::label(Damage_name) >> parse::double_ [ _b = _1 ]
                          >   parse::label(ROF_name)    >  parse::double_ [ _c = _1 ]
                          >   parse::label(Range_name)  >  parse::double_ [ _d = _1 ]
                          >>  (
                                   parse::label(Speed_name)     >> parse::double_ [ _e = _1 ]
                               >   parse::label(Stealth_name)   >  parse::double_ [ _f = _1 ]
                               >   parse::label(Structure_name) >  parse::double_ [ _g = _1 ]
                               >   parse::label(Capacity_name)  >  parse::int_ [ _val = construct<LRStats>(_b, _c, _d, _e, _f, _g, _1) ]
                               |   eps [ _val = construct<DirectFireStats>(_b, _c, _d) ]
                              )
                         )
                    |    parse::label(Capacity_name) > parse::double_ [ _val = _1 ]
                    ;

                part_type
                    =    tok.Part_
                    >    parse::label(Name_name)        > tok.string [ _a = _1 ]
                    >    parse::label(Description_name) > tok.string [ _b = _1 ]
                    >    parse::label(Class_name)       > parse::enum_parser<ShipPartClass>() [ _c = _1 ]
                    >>   part_stats [ _d = _1 ]
                    >    parse::label(BuildCost_name)   > parse::double_ [ _e = _1 ]
                    >    parse::label(BuildTime_name)   > parse::int_ [ _f = _1 ]
                    >>   (
                              tok.Unproducible_ [ _g = false ]
                          |   tok.Producible_ [ _g = true ]
                          |   eps [ _g = true ]
                         )
                    >    parse::label(MountableSlotTypes_name)
                    >>   (
                              '[' > +parse::enum_parser<ShipSlotType>() [ push_back(_h, _1) ] > ']'
                          |   parse::enum_parser<ShipSlotType>() [ push_back(_h, _1) ]
                         )
                    >    parse::label(Location_name) > parse::detail::condition_parser [ _i = _1 ]
                    >>  -(
                              parse::label(EffectsGroups_name)
                          >>  parse::detail::effects_group_parser() [ _j = _1 ]
                         )
                    >    parse::label(Graphic_name) > tok.string [ insert(_r1, new_<PartType>(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _1)) ]
                    ;

                start
                    =   +part_type(_r1)
                    ;

                part_stats.name("Part stats");
                part_type.name("Part");

#if DEBUG_PARSERS
                debug(part_stats);
                debug(part_type);
#endif

                qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
            }

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
            void (std::map<std::string, PartType*>&),
            qi::locals<
                std::string,
                std::string,
                ShipPartClass,
                PartTypeStats,
                double,
                int,
                bool,
                std::vector<ShipSlotType>,
                Condition::ConditionBase*,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
            >,
            parse::skipper_type
        > part_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, PartType*>&),
            parse::skipper_type
        > start_rule;

        part_stats_rule part_stats;
        part_type_rule part_type;
        start_rule start;
    };

}

namespace parse {

    bool ship_parts(const boost::filesystem::path& path, std::map<std::string, PartType*>& parts)
    { return detail::parse_file<rules, std::map<std::string, PartType*> >(path, parts); }

}
