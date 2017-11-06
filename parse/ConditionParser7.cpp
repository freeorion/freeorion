#include "ConditionParser7.h"

#include "ValueRefParserImpl.h"

#include "../universe/Condition.h"

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
        star_type_rules(tok, labeller, condition_parser)
    {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_val_type _val;
        qi::_pass_type _pass;
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
            >    (
                ('[' > +star_type_rules.expr [ push_back(_a, _1) ] > ']')
                |    star_type_rules.expr [ push_back(_a, _1) ]
            )
            [ _val = construct_movable_(new_<Condition::StarType>(deconstruct_movable_vector_(_a, _pass))) ]
            ;

        location
            =   (tok.Location_
                 >    labeller.rule(Type_token) >
                 (
                     tok.Building_   [ _a = Condition::CONTENT_BUILDING ]
                     |   tok.Species_    [ _a = Condition::CONTENT_SPECIES ]
                     |   tok.Hull_       [ _a = Condition::CONTENT_SHIP_HULL ]
                     |   tok.Part_       [ _a = Condition::CONTENT_SHIP_PART ]
                     |   tok.Special_    [ _a = Condition::CONTENT_SPECIAL ]
                     |   tok.Focus_      [ _a = Condition::CONTENT_FOCUS ]
                 )
                 >    labeller.rule(Name_token)   > string_grammar [ _b = _1 ]
                 >  -(labeller.rule(Name_token)   > string_grammar [ _c = _1 ]))
            [ _val = construct_movable_(new_<Condition::Location>(
                    _a,
                    deconstruct_movable_(_b, _pass),
                    deconstruct_movable_(_c, _pass))) ]
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
