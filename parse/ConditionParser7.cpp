#include "ConditionParser7.h"

#include "ValueRefParser.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ValueRef::ValueRefBase<StarType>*>&) { return os; }
}
#endif

namespace parse { namespace detail {
    condition_parser_rules_7::condition_parser_rules_7(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_7::base_type(start, "condition_parser_rules_7"),
        star_type_rules(tok, labeller, condition_parser),
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
            >   -labeller.rule(Condition_token) > condition_parser
            [ _val = construct_movable_(new_<Condition::OrderedBombarded>(deconstruct_movable_(_1, _pass))) ]
            ;

        contains
            =    tok.Contains_
            >   -labeller.rule(Condition_token) > condition_parser
            [ _val = construct_movable_(new_<Condition::Contains>(deconstruct_movable_(_1, _pass))) ]
            ;

        contained_by
            =    tok.ContainedBy_
            >   -labeller.rule(Condition_token) > condition_parser
            [ _val = construct_movable_(new_<Condition::ContainedBy>(deconstruct_movable_(_1, _pass))) ]
            ;

        star_type
            =    tok.Star_
            >    labeller.rule(Type_token)
            >    one_or_more_star_types
            [ _val = construct_movable_(new_<Condition::StarType>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        building_type =
            tok.Building_   [ _val = Condition::CONTENT_BUILDING ]
            |   tok.Species_    [ _val = Condition::CONTENT_SPECIES ]
            |   tok.Hull_       [ _val = Condition::CONTENT_SHIP_HULL ]
            |   tok.Part_       [ _val = Condition::CONTENT_SHIP_PART ]
            |   tok.Special_    [ _val = Condition::CONTENT_SPECIAL ]
            |   tok.Focus_      [ _val = Condition::CONTENT_FOCUS ];


        location
            =   (omit_[tok.Location_]
                 >    labeller.rule(Type_token) > building_type
                 >    labeller.rule(Name_token)   > string_grammar
                 >  -(labeller.rule(Name_token)   > string_grammar))
            [ _val = construct_movable_(new_<Condition::Location>(
                    _1,
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass))) ]
            ;

        owner_has_shippart_available
            =   (tok.OwnerHasShipPartAvailable_
                 >>  (labeller.rule(Name_token)
                      > string_grammar [ _val = construct_movable_(new_<Condition::OwnerHasShipPartAvailable>(
                              deconstruct_movable_(_1, _pass))) ]
                     )
                )
            ;

        start
            %=   ordered_bombarded_by
            |    contains
            |    contained_by
            |    star_type
            |    location
            |    owner_has_shippart_available
            ;

        ordered_bombarded_by.name("OrderedBombardedBy");
        contains.name("Contains");
        contained_by.name("ContainedBy");
        star_type.name("StarType");
        location.name("Location");
        owner_has_shippart_available.name("OwnerHasShipPartAvailable");

#if DEBUG_CONDITION_PARSERS
        debug(ordered_bombarded_by);
        debug(contains);
        debug(contained_by);
        debug(star_type);
        debug(location);
        debug(owner_has_shippart_available);
#endif
    }

} }
