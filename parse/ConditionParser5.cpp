#include "ConditionParser5.h"

#include "../universe/Conditions.h"
#include "../universe/ValueRef.h"

#include <boost/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<condition_payload>&) { return os; }
}
#endif

namespace parse { namespace detail {
    condition_parser_rules_5::condition_parser_rules_5(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_5::base_type(start, "condition_parser_rules_5"),
        int_rules(tok, label, condition_parser, string_grammar),
        visibility_rules(tok, label, condition_parser)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        using phoenix::new_;
        using phoenix::construct;

        has_special
            =   (   (tok.HasSpecial_
                     >>  label(tok.name_)) > string_grammar
                     [ _val = construct_movable_(new_<Condition::HasSpecial>(deconstruct_movable_(_1, _pass))) ]
                )
            |   tok.HasSpecial_ [ _val = construct_movable_(new_<Condition::HasSpecial>()) ]
            ;

        has_tag
            = (
                (tok.HasTag_
                 >>  label(tok.name_)) > string_grammar
                 [ _val = construct_movable_(new_<Condition::HasTag>(deconstruct_movable_(_1, _pass))) ]
                )
            |   tok.HasTag_ [ _val = construct_movable_(new_<Condition::HasTag>()) ]
            ;

        owner_has_tech
            =   tok.OwnerHasTech_
            >   label(tok.name_) > string_grammar
            [ _val = construct_movable_(new_<Condition::OwnerHasTech>(deconstruct_movable_(_1, _pass))) ]
            ;

        empire_adopted_policy1
            = (
                (omit_[tok.EmpireHasAdoptedPolicy_]
                >>  label(tok.empire_)) > int_rules.expr
                >   label(tok.name_)    > string_grammar
              ) [ _val = construct_movable_(new_<Condition::EmpireHasAdoptedPolicy>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        empire_adopted_policy2
            = (
                (omit_[tok.EmpireHasAdoptedPolicy_]
                >>  label(tok.name_)) > string_grammar
                ) [ _val = construct_movable_(new_<Condition::EmpireHasAdoptedPolicy>(deconstruct_movable_(_1, _pass))) ]
            ;

        empire_adopted_policy
            %=   empire_adopted_policy1
             |   empire_adopted_policy2
            ;

        design_has_hull
            =   tok.DesignHasHull_
            >   label(tok.name_) > string_grammar
            [ _val = construct_movable_(new_<Condition::DesignHasHull>(deconstruct_movable_(_1, _pass))) ]
            ;

        predefined_design
            =   (tok.Design_
                 >>  label(tok.name_)) > string_grammar
                 [ _val = construct_movable_(new_<Condition::PredefinedShipDesign>(deconstruct_movable_(_1, _pass))) ]
            ;

        design_number
            =   (tok.Design_
                 >>  label(tok.design_)) > int_rules.expr
                 [ _val = construct_movable_(new_<Condition::NumberedShipDesign>(deconstruct_movable_(_1, _pass))) ]
            ;

        produced_by_empire // TODO: Lose "empire" part.
            =   tok.ProducedByEmpire_
            >   label(tok.empire_) > int_rules.expr
            [ _val = construct_movable_(new_<Condition::ProducedByEmpire>(deconstruct_movable_(_1, _pass))) ]
            ;

        visible_to_empire // TODO: Lose "empire" part.
            =   (
                     omit_[tok.VisibleToEmpire_]
                >    label(tok.empire_)     > int_rules.expr
                >  -(label(tok.turn_)       > int_rules.expr)
                >  -(label(tok.visibility_) > visibility_rules.expr)
            ) [ _val = construct_movable_(new_<Condition::VisibleToEmpire>(
                deconstruct_movable_(_1, _pass),
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass))) ]
            ;

        explored_by_empire // TODO: Lose "empire" part.
            =    tok.ExploredByEmpire_
            >    label(tok.empire_) > int_rules.expr
            [ _val = construct_movable_(new_<Condition::ExploredByEmpire>(deconstruct_movable_(_1, _pass))) ]
            ;

        resupplyable_by
            =   tok.ResupplyableBy_
            >   label(tok.empire_) > int_rules.expr
            [ _val = construct_movable_(new_<Condition::FleetSupplyableByEmpire>(deconstruct_movable_(_1, _pass))) ]
            ;

        object_id
            =   tok.Object_
            >   label(tok.id_) > int_rules.expr
            [ _val = construct_movable_(new_<Condition::ObjectID>(deconstruct_movable_(_1, _pass))) ]
            ;

        start
            %=  has_special
            |   has_tag
            |   owner_has_tech
            |   empire_adopted_policy
            |   design_has_hull
            |   predefined_design
            |   design_number
            |   produced_by_empire
            |   visible_to_empire
            |   explored_by_empire
            |   resupplyable_by
            |   object_id
            ;

        has_special.name("HasSpecial");
        has_tag.name("HasTag");
        owner_has_tech.name("OwnerHasTech");
        empire_adopted_policy.name("EmpireHasAdoptedPolicy");
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
        debug(empire_adopted_policy);
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

} }
