#include "ConditionParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"

//#include <GG/ReportParseError.h>

#include <boost/spirit/home/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


namespace {
    struct condition_parser_rules_2 {
        condition_parser_rules_2() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref = parse::value_ref_parser<int>();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_val_type _val;
            using phoenix::new_;

            has_special
                =    tok.HasSpecial_
                >    parse::label(Name_name) > tok.string [ _val = new_<Condition::HasSpecial>(_1) ]
                ;

            has_special_since_turn
                =    (
                            tok.HasSpecialSinceTurn_
                        >   parse::label(Name_name) > tok.string [ _a = _1 ]
                        >> -(
                                parse::label(Low_name) >> int_value_ref [ _b = _1 ]
                            )
                        >> -(
                                parse::label(High_name) >> int_value_ref [ _c = _1 ]
                            )
                     )
                     [ _val = new_<Condition::HasSpecial>(_a, _b, _c) ]
                ;

            has_tag
                =    tok.HasTag_
                >    parse::label(Name_name) > tok.string [ _val = new_<Condition::HasTag>(_1) ]
                ;

            owner_has_tech
                =    tok.OwnerHasTech_
                >    parse::label(Name_name) > tok.string [ _val = new_<Condition::OwnerHasTech>(_1) ]
                ;

            design_has_hull
                =    tok.DesignHasHull_
                >    parse::label(Name_name) > tok.string [ _val = new_<Condition::DesignHasHull>(_1) ]
                ;

            design_has_part
                =    tok.DesignHasPart_
                >    parse::label(Low_name)   > int_value_ref [ _a = _1 ]
                >    parse::label(High_name)  > int_value_ref [ _b = _1 ]
                >    parse::label(Class_name) > tok.string [ _val = new_<Condition::DesignHasPart>(_a, _b, _1) ]
                ;

            design_has_part_class
                =    tok.DesignHasPartClass_
                >    parse::label(Low_name)   > int_value_ref [ _a = _1 ]
                >    parse::label(High_name)  > int_value_ref [ _b = _1 ]
                >    parse::label(Class_name) > parse::enum_parser<ShipPartClass>() [ _val = new_<Condition::DesignHasPartClass>(_a, _b, _1) ]
                ;

            predefined_design
                =    tok.Design_
                >>   parse::label(Name_name) >> tok.string [ _val = new_<Condition::PredefinedShipDesign>(_1) ]
                ;

            design_number
                =    tok.Design_
                >    parse::label(Design_name) > int_value_ref [ _val = new_<Condition::NumberedShipDesign>(_1) ]
                ;

            produced_by_empire // TODO: Lose "empire" part.
                =    tok.ProducedByEmpire_
                >    parse::label(Empire_name) > int_value_ref [ _val = new_<Condition::ProducedByEmpire>(_1) ]
                ;

            visible_to_empire // TODO: Lose "empire" part.
                =    tok.VisibleToEmpire_
                >    parse::label(Empire_name) > int_value_ref [ _val = new_<Condition::VisibleToEmpire>(_1) ]
                ;

            explored_by_empire // TODO: Lose "empire" part.
                =    tok.ExploredByEmpire_
                >    parse::label(Empire_name) > int_value_ref [ _val = new_<Condition::ExploredByEmpire>(_1) ]
                ;

            resupplyable_by
                =    tok.ResupplyableBy_
                >    parse::label(Empire_name) > int_value_ref [ _val = new_<Condition::FleetSupplyableByEmpire>(_1) ]
                ;

            in_system
                =   (
                        tok.InSystem_
                    >> -(
                            parse::label(ID_name) >> int_value_ref [ _a = _1 ]
                        )
                    )
                    [ _val = new_<Condition::InSystem>(_a) ]
                ;

            object_id
                =    tok.Object_
                >    parse::label(ID_name) > int_value_ref [ _val = new_<Condition::ObjectID>(_1) ]
                ;

            start
                %=   has_special
                |    has_special_since_turn
                |    has_tag
                |    owner_has_tech
                |    design_has_hull
                |    design_has_part
                |    design_has_part_class
                |    predefined_design
                |    design_number
                |    produced_by_empire
                |    visible_to_empire
                |    explored_by_empire
                |    resupplyable_by
                |    in_system
                |    object_id
                ;

            has_special.name("HasSpecial");
            has_special_since_turn.name("HasSpecialSinceTurn");
            has_tag.name("HasTag");
            owner_has_tech.name("OwnerHasTech");
            design_has_hull.name("DesignHasHull");
            design_has_part.name("DesignHasPart");
            design_has_part_class.name("DesignHasPartClass");
            predefined_design.name("PredefinedDesign");
            design_number.name("DesignNumber");
            produced_by_empire.name("ProducedByEmpire");
            visible_to_empire.name("VisibleToEmpire");
            explored_by_empire.name("ExploredByEmpire");
            resupplyable_by.name("ResupplyableBy");
            in_system.name("InSystem");
            object_id.name("ID");

#if DEBUG_CONDITION_PARSERS
            debug(has_special);
            debug(has_special_since_turn);
            debug(has_tag);
            debug(owner_has_tech);
            debug(design_has_hull);
            debug(design_has_part);
            debug(design_has_part_class);
            debug(predefined_design);
            debug(design_number);
            debug(produced_by_empire);
            debug(visible_to_empire);
            debug(explored_by_empire);
            debug(resupplyable_by);
            debug(in_system);
            debug(object_id);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*
            >,
            parse::skipper_type
        > value_ref_int_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*
            >,
            parse::skipper_type
        > value_ref_ints_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*
            >,
            parse::skipper_type
        > has_special_since_turn_rule;

        parse::condition_parser_rule has_special;
        has_special_since_turn_rule has_special_since_turn;
        parse::condition_parser_rule has_tag;
        parse::condition_parser_rule owner_has_tech;
        parse::condition_parser_rule design_has_hull;
        value_ref_ints_rule design_has_part;
        value_ref_ints_rule design_has_part_class;
        parse::condition_parser_rule predefined_design;
        parse::condition_parser_rule design_number;
        parse::condition_parser_rule produced_by_empire;
        parse::condition_parser_rule visible_to_empire;
        parse::condition_parser_rule explored_by_empire;
        parse::condition_parser_rule resupplyable_by;
        value_ref_int_rule in_system;
        parse::condition_parser_rule object_id;
        parse::condition_parser_rule start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_2() {
        static condition_parser_rules_2 retval;
        return retval.start;
    }
} }
