#include "ConditionParser7.h"

#include "ValueRefParser.h"

#include "../universe/Conditions.h"
#include "../universe/ValueRef.h"

#include <boost/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRef<StarType>*>&) { return os; }
}
#endif

namespace parse { namespace detail {
    condition_parser_rules_7::condition_parser_rules_7(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_7::base_type(start, "condition_parser_rules_7"),
        int_rules(tok, label, condition_parser, string_grammar),
        star_type_rules(tok, label, condition_parser),
        one_or_more_star_types(star_type_rules.expr)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_val_type _val;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        using phoenix::new_;
        using phoenix::push_back;
        using phoenix::construct;

        ordered_bombarded_by
            =    tok.OrderedBombardedBy_
            >   -label(tok.condition_) > condition_parser
            [ _val = construct_movable_(new_<Condition::OrderedBombarded>(deconstruct_movable_(_1, _pass))) ]
            ;

        contains
            =    tok.Contains_
            >   -label(tok.condition_) > condition_parser
            [ _val = construct_movable_(new_<Condition::Contains<>>(deconstruct_movable_(_1, _pass))) ]
            ;

        contained_by
            =    tok.ContainedBy_
            >   -label(tok.condition_) > condition_parser
            [ _val = construct_movable_(new_<Condition::ContainedBy>(deconstruct_movable_(_1, _pass))) ]
            ;

        star_type
            =    tok.Star_
            >    label(tok.type_)
            >    one_or_more_star_types
            [ _val = construct_movable_(new_<Condition::StarType>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        content_type =
                tok.Building_   [ _val = Condition::ContentType::CONTENT_BUILDING ]
            |   tok.Species_    [ _val = Condition::ContentType::CONTENT_SPECIES ]
            |   tok.Hull_       [ _val = Condition::ContentType::CONTENT_SHIP_HULL ]
            |   tok.Part_       [ _val = Condition::ContentType::CONTENT_SHIP_PART ]
            |   tok.Special_    [ _val = Condition::ContentType::CONTENT_SPECIAL ]
            |   tok.Focus_      [ _val = Condition::ContentType::CONTENT_FOCUS ];

        location
            =   (omit_[tok.Location_]
                 >    label(tok.type_)  >   content_type
                 >    label(tok.name_)  >   string_grammar
                 >  -(label(tok.name_)  >   string_grammar))
            [ _val = construct_movable_(new_<Condition::Location>(
                    _1,
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass))) ]
            ;

        combat_targets
            =   (omit_[tok.combatTargets_]
                 >    label(tok.type_)  >   content_type
                 >    label(tok.name_)  >   string_grammar)
            [ _val = construct_movable_(new_<Condition::CombatTarget>(
                    _1,
                    deconstruct_movable_(_2, _pass))) ]
            ;

        empire_has_buildingtype_available1
            = (
                   (omit_[tok.EmpireHasBuildingAvailable_]
                 >> label(tok.name_)) > string_grammar
              ) [ _val = construct_movable_(new_<Condition::EmpireHasBuildingTypeAvailable>(
                    deconstruct_movable_(_1, _pass))) ]
            ;

        empire_has_buildingtype_available2
            = (
                   (omit_[tok.EmpireHasBuildingAvailable_]
                 >> label(tok.empire_)) > int_rules.expr
                 >  label(tok.name_)    > string_grammar
              ) [ _val = construct_movable_(new_<Condition::EmpireHasBuildingTypeAvailable>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        empire_has_buildingtype_available
            %=  empire_has_buildingtype_available1
             |  empire_has_buildingtype_available2
            ;

        empire_has_shipdesign_available1
            = (
                   (omit_[tok.EmpireHasShipDesignAvailable_]
                 >> label(tok.designid_)) > int_rules.expr
              ) [ _val = construct_movable_(new_<Condition::EmpireHasShipDesignAvailable>(
                    deconstruct_movable_(_1, _pass))) ]
            ;

        empire_has_shipdesign_available2
            = (
                   (omit_[tok.EmpireHasShipDesignAvailable_]
                 >> label(tok.empire_))  > int_rules.expr
                 >  label(tok.designid_) > int_rules.expr
              ) [ _val = construct_movable_(new_<Condition::EmpireHasShipDesignAvailable>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        empire_has_shipdesign_available
            %=  empire_has_shipdesign_available1
             |  empire_has_shipdesign_available2
            ;

        empire_has_shippart_available1
            = (
                   (omit_[tok.OwnerHasShipPartAvailable_]
                 >> label(tok.name_)) > string_grammar
              ) [ _val = construct_movable_(new_<Condition::EmpireHasShipPartAvailable>(
                    deconstruct_movable_(_1, _pass))) ]
            ;

        empire_has_shippart_available2
            = (
                   (omit_[tok.EmpireHasShipPartAvailable_]
                 >> label(tok.empire_)) > int_rules.expr
                 >  label(tok.name_)    > string_grammar
              ) [ _val = construct_movable_(new_<Condition::EmpireHasShipPartAvailable>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        empire_has_shippart_available
            %=  empire_has_shippart_available1
             |  empire_has_shippart_available2
            ;

        start
            %=  ordered_bombarded_by
            |   contains
            |   contained_by
            |   star_type
            |   location
            |   combat_targets
            |   empire_has_buildingtype_available
            |   empire_has_shipdesign_available
            |   empire_has_shippart_available
            ;

        ordered_bombarded_by.name("OrderedBombardedBy");
        contains.name("Contains");
        contained_by.name("ContainedBy");
        star_type.name("StarType");
        location.name("Location");
        combat_targets.name("CombatTargets");
        empire_has_buildingtype_available.name("EmpireHasBuildingTypeAvailable");
        empire_has_shipdesign_available.name("EmpireHasShipDesignAvailable");
        empire_has_shippart_available.name("EmpireHasShipPartAvailable");

#if DEBUG_CONDITION_PARSERS
        debug(ordered_bombarded_by);
        debug(contains);
        debug(contained_by);
        debug(star_type);
        debug(location);
        debug(combat_targets);
        debug(owner_has_shippart_available);
#endif
    }

} }
