#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"
#include "ShipPartStatsParser.h"
#include "ValueRefParser.h"

#include "../universe/Condition.h"

#include <boost/spirit/home/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipSlotType>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, PartType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, PartType*>&) { return os; }
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

            const parse::value_ref_parser_rule<int>::type& int_value_ref =          parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =    parse::value_ref_parser<double>();

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
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;

            part_type_prefix
                =    tok.Part_
                >    parse::label(Name_name)        > tok.string [ _r1 = _1 ]
                >    parse::label(Description_name) > tok.string [ _r2 = _1 ]
                >    parse::label(Class_name)       > parse::enum_parser<ShipPartClass>() [ _r3 = _1 ]
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

            common_params
                =   parse::label(BuildCost_name)    > double_value_ref  [ _a = _1 ]
                >   parse::label(BuildTime_name)    > int_value_ref     [ _b = _1 ]
                >   producible                                          [ _c = _1 ]
                >   parse::detail::tags_parser()(_d)
                >   location(_e)
                >   -(
                        parse::label(EffectsGroups_name) >> parse::detail::effects_group_parser() [ _f = _1 ]
                     )
                >    parse::label(Icon_name)        > tok.string
                    [ _val = construct<PartHullCommonParams>(_a, _b, _c, _d, _e, _f, _1) ]
            ;

            part_type
                =    part_type_prefix(_a, _b, _c)
                >    parse::detail::part_stats_parser() [ _d = _1 ]
                >    slots(_f)
                >    common_params [ _e = _1 ]
                    [ insert(_r1, new_<PartType>(_a, _b, _c, _d, _e, _f)) ]
                ;

            start
                =   +part_type(_r1)
                ;

            part_type_prefix.name("Part");
            producible.name("Producible or Unproducible");
            slots.name("mountable slot types");
            location.name("Location");
            common_params.name("Part Hull Common Params");
            part_type.name("Part");

#if DEBUG_PARSERS
            debug(part_type_prefix);
            debug(producible);
            debug(slots);
            debug(location);
            debug(common_params);
            debug(part_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::string&, std::string&, ShipPartClass&),
            parse::skipper_type
        > part_type_prefix_rule;

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
            PartHullCommonParams (),
            qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<int>*,
                bool,
                std::set<std::string>,
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

        part_type_prefix_rule           part_type_prefix;
        producible_rule                 producible;
        location_rule                   location;
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
