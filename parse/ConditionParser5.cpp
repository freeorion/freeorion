#include "ConditionParser5.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    condition_parser_rules_5::condition_parser_rules_5(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_5::base_type(start, "condition_parser_rules_5"),
        int_rules(tok, labeller, condition_parser, string_grammar)
    {
        qi::_1_type _1;
        qi::_val_type _val;
        qi::eps_type eps;
        const boost::phoenix::function<construct_movable> construct_movable_;
        using phoenix::new_;
        using phoenix::construct;

        has_special
            =   (   (tok.HasSpecial_
                     >>  labeller.rule(Name_token)
                    ) > string_grammar [ _val = construct_movable_(new_<Condition::HasSpecial>(
                            construct<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>(_1))) ]
                )
            |   tok.HasSpecial_ [ _val = construct_movable_(new_<Condition::HasSpecial>()) ]
            ;

        has_tag
            =   (   (tok.HasTag_
                     >>  labeller.rule(Name_token)
                    ) > string_grammar [ _val = construct_movable_(new_<Condition::HasTag>(
                            construct<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>(_1))) ]
                )
            |   tok.HasTag_ [ _val = construct_movable_(new_<Condition::HasTag>()) ]
            ;

        owner_has_tech
            =   tok.OwnerHasTech_
            >   labeller.rule(Name_token) > string_grammar [ _val = construct_movable_(new_<Condition::OwnerHasTech>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>(_1))) ]
            ;

        design_has_hull
            =   tok.DesignHasHull_
            >   labeller.rule(Name_token) > string_grammar [ _val = construct_movable_(new_<Condition::DesignHasHull>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>(_1))) ]
            ;

        predefined_design
            =   (tok.Design_
                 >>  labeller.rule(Name_token)
                ) > string_grammar [ _val = construct_movable_(new_<Condition::PredefinedShipDesign>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>(_1))) ]
            ;

        design_number
            =   (tok.Design_
                 >>  labeller.rule(Design_token)
                ) > int_rules.expr [ _val = construct_movable_(new_<Condition::NumberedShipDesign>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<int>>>(_1))) ]
            ;

        produced_by_empire // TODO: Lose "empire" part.
            =   tok.ProducedByEmpire_
            >   labeller.rule(Empire_token) > int_rules.expr [ _val = construct_movable_(new_<Condition::ProducedByEmpire>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<int>>>(_1))) ]
            ;

        visible_to_empire // TODO: Lose "empire" part.
            =   tok.VisibleToEmpire_
            >   labeller.rule(Empire_token) > int_rules.expr [ _val = construct_movable_(new_<Condition::VisibleToEmpire>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<int>>>(_1))) ]
            ;

        explored_by_empire // TODO: Lose "empire" part.
            =    tok.ExploredByEmpire_
            >    labeller.rule(Empire_token) > int_rules.expr [ _val = construct_movable_(new_<Condition::ExploredByEmpire>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<int>>>(_1))) ]
            ;

        resupplyable_by
            =   tok.ResupplyableBy_
            >   labeller.rule(Empire_token) > int_rules.expr [ _val = construct_movable_(new_<Condition::FleetSupplyableByEmpire>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<int>>>(_1))) ]
            ;

        object_id
            =   tok.Object_
            >   labeller.rule(ID_token) > int_rules.expr [ _val = construct_movable_(new_<Condition::ObjectID>(
                construct<std::unique_ptr<ValueRef::ValueRefBase<int>>>(_1))) ]
            ;

        start
            %=  has_special
            |   has_tag
            |   owner_has_tech
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

} }
