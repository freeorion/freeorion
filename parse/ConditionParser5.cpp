#include "ConditionParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


namespace {
    struct condition_parser_rules_5 {
        condition_parser_rules_5() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref = parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref = parse::value_ref_parser<std::string>();

            qi::_1_type _1;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;

            has_special
                =    tok.HasSpecial_
                >    parse::label(Name_token) > string_value_ref [ _val = new_<Condition::HasSpecial>(_1) ]
                ;

            has_tag
                =    tok.HasTag_
                >    parse::label(Name_token) > string_value_ref [ _val = new_<Condition::HasTag>(_1) ]
                ;

            owner_has_tech
                =    tok.OwnerHasTech_
                >    parse::label(Name_token) > string_value_ref [ _val = new_<Condition::OwnerHasTech>(_1) ]
                ;

            design_has_hull
                =    tok.DesignHasHull_
                >    parse::label(Name_token) > string_value_ref [ _val = new_<Condition::DesignHasHull>(_1) ]
                ;

            predefined_design
                =    tok.Design_
                >>   parse::label(Name_token) > string_value_ref [ _val = new_<Condition::PredefinedShipDesign>(_1) ]
                ;

            design_number
                =    tok.Design_
                >>   parse::label(Design_token) > int_value_ref [ _val = new_<Condition::NumberedShipDesign>(_1) ]
                ;

            produced_by_empire // TODO: Lose "empire" part.
                =    tok.ProducedByEmpire_
                >    parse::label(Empire_token) > int_value_ref [ _val = new_<Condition::ProducedByEmpire>(_1) ]
                ;

            visible_to_empire // TODO: Lose "empire" part.
                =    tok.VisibleToEmpire_
                >    parse::label(Empire_token) > int_value_ref [ _val = new_<Condition::VisibleToEmpire>(_1) ]
                ;

            explored_by_empire // TODO: Lose "empire" part.
                =    tok.ExploredByEmpire_
                >    parse::label(Empire_token) > int_value_ref [ _val = new_<Condition::ExploredByEmpire>(_1) ]
                ;

            resupplyable_by
                =    tok.ResupplyableBy_
                >    parse::label(Empire_token) > int_value_ref [ _val = new_<Condition::FleetSupplyableByEmpire>(_1) ]
                ;

            object_id
                =    tok.Object_
                >    parse::label(ID_token) > int_value_ref [ _val = new_<Condition::ObjectID>(_1) ]
                ;

            start
                %=   has_special
                |    has_tag
                |    owner_has_tech
                |    design_has_hull
                |    predefined_design
                |    design_number
                |    produced_by_empire
                |    visible_to_empire
                |    explored_by_empire
                |    resupplyable_by
                |    object_id
                ;

            has_special.name("HasSpecial");
            has_tag.name("HasTag");
            owner_has_tech.name("OwnerHasTech");
            design_has_hull.name("DesignHasHull");
            predefined_design.name("PredefinedDesign");
            design_number.name("DesignNumber");
            produced_by_empire.name("ProducedByEmpire");
            visible_to_empire.name("VisibleToEmpire");
            explored_by_empire.name("ExploredByEmpire");
            resupplyable_by.name("ResupplyableBy");
            object_id.name("ID");

#if DEBUG_CONDITION_PARSERS
            debug(has_special);
            debug(has_tag);
            debug(owner_has_tech);
            debug(design_has_hull);
            debug(predefined_design);
            debug(design_number);
            debug(produced_by_empire);
            debug(visible_to_empire);
            debug(explored_by_empire);
            debug(resupplyable_by);
            debug(object_id);
#endif
        }

        parse::condition_parser_rule    has_special;
        parse::condition_parser_rule    has_tag;
        parse::condition_parser_rule    owner_has_tech;
        parse::condition_parser_rule    design_has_hull;
        parse::condition_parser_rule    predefined_design;
        parse::condition_parser_rule    design_number;
        parse::condition_parser_rule    produced_by_empire;
        parse::condition_parser_rule    visible_to_empire;
        parse::condition_parser_rule    explored_by_empire;
        parse::condition_parser_rule    resupplyable_by;
        parse::condition_parser_rule    object_id;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_5() {
        static condition_parser_rules_5 retval;
        return retval.start;
    }
} }
