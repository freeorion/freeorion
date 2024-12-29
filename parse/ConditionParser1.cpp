#include "ConditionParser1.h"

#include "../universe/Conditions.h"
#include "../universe/Enums.h"
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
    condition_parser_rules_1::condition_parser_rules_1(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_1::base_type(start, "condition_parser_rules_1"),
        int_rules(tok, label, condition_parser, string_grammar),
        empire_affiliation_type_enum(tok)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::lit_type lit;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        using phoenix::new_;
        using phoenix::construct;
        using phoenix::push_back;

        all
            =   tok.All_ [ _val = construct_movable_(new_<Condition::All>()) ]
            ;

        none
            =   tok.None_ [ _val = construct_movable_(new_<Condition::None>()) ]
            ;

        noop
            =   tok.NoOp_ [ _val = construct_movable_(new_<Condition::NoOp>()) ]
            ;

        source
            =   tok.Source_ [ _val = construct_movable_(new_<Condition::Source>()) ]
            ;

        root_candidate
            =   tok.RootCandidate_ [ _val = construct_movable_(new_<Condition::RootCandidate>()) ]
            ;

        target
            =   tok.Target_ [ _val = construct_movable_(new_<Condition::Target>()) ]
            ;

        stationary
            =   tok.Stationary_ [ _val = construct_movable_(new_<Condition::Stationary>()) ]
            ;

        aggressive
            = ((tok.Aggressive_ [ _val = construct_movable_(new_<Condition::Aggressive>(true)) ])
               |(tok.Passive_ [ _val = construct_movable_(new_<Condition::Aggressive>(false)) ])
              )
            ;

        can_colonize
            =   tok.CanColonize_ [ _val = construct_movable_(new_<Condition::CanColonize>()) ]
            ;

        can_produce_ships
            =   tok.CanProduceShips_ [ _val = construct_movable_(new_<Condition::CanProduceShips>()) ]
            ;

        capital1
            =   (tok.Capital_ >> label(tok.empire_)) > int_rules.expr
            [ _val = construct_movable_(new_<Condition::CapitalWithID>(deconstruct_movable_(_1, _pass))) ]
            ;

        capital2
            =   tok.Capital_ [ _val = construct_movable_(new_<Condition::Capital>()) ]
            ;

        monster
            =   tok.Monster_ [ _val = construct_movable_(new_<Condition::Monster>()) ]
            ;

        armed
            =   tok.Armed_ [ _val = construct_movable_(new_<Condition::Armed>()) ]
            ;

        owned_by_1
            =   (tok.OwnedBy_ >> label(tok.empire_)) > int_rules.expr
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>(deconstruct_movable_(_1, _pass))) ]
            ;

        owned_by_2
            =   tok.OwnedBy_
            >>  label(tok.affiliation_) >> tok.AnyEmpire_
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>( EmpireAffiliationType::AFFIL_ANY )) ]
            ;

        owned_by_3
            =   tok.Unowned_
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>( EmpireAffiliationType::AFFIL_NONE )) ]
            ;

        owned_by_4
            =   tok.Human_
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>( EmpireAffiliationType::AFFIL_HUMAN )) ]
            ;

        owned_by_5
            =  ((omit_[tok.OwnedBy_]
                 >>  label(tok.affiliation_) >> empire_affiliation_type_enum
                 >>  label(tok.empire_)    ) >  int_rules.expr)
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>(deconstruct_movable_(_2, _pass), _1)) ]
            ;

        owned_by
            %=  owned_by_1
            |   owned_by_2
            |   owned_by_3
            |   owned_by_4
            |   owned_by_5
            ;

        and_
            = ( omit_[tok.And_] > '[' > +condition_parser > lit(']'))
            [ _val = construct_movable_(new_<Condition::And>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        or_
            = ( omit_[tok.Or_] > '[' > +condition_parser > lit(']'))
            [ _val = construct_movable_(new_<Condition::Or>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        not_
            = tok.Not_ > condition_parser
            [ _val = construct_movable_(new_<Condition::Not>(deconstruct_movable_(_1, _pass))) ]
            ;

        ordered_alternatives_of
            = ( omit_[tok.OrderedAlternativesOf_] > '[' > +condition_parser > lit(']'))
            [ _val = construct_movable_(new_<Condition::OrderedAlternativesOf>(deconstruct_movable_vector_(_1, _pass))) ]
            ;

        described
            = ( omit_[tok.Described_]
                > label(tok.description_) > tok.string
                > label(tok.condition_) > condition_parser)
            [ _val = construct_movable_( new_<Condition::Described>(deconstruct_movable_(_2, _pass), _1)) ]
            ;

        start
            %=  all
            |   none
            |   noop
            |   source
            |   root_candidate
            |   target
            |   stationary
            |   aggressive
            |   can_colonize
            |   can_produce_ships
            |   capital1
            |   capital2
            |   monster
            |   armed
            |   owned_by
            |   and_
            |   or_
            |   not_
            |   ordered_alternatives_of
            |   described
            ;

        all.name("All");
        none.name("None");
        source.name("Source");
        root_candidate.name("RootCandidate");
        target.name("Target");
        stationary.name("Stationary");
        aggressive.name("Aggressive");
        can_colonize.name("CanColonize");
        can_produce_ships.name("CanProduceShips");
        capital1.name("Capital");
        capital2.name("Capital");
        monster.name("Monster");
        armed.name("Armed");
        owned_by.name("OwnedBy");  // TODO: Should this be renamed Affilated or similar?
        and_.name("And");
        or_.name("Or");
        not_.name("Not");
        ordered_alternatives_of.name("OrderedAlternativesOf");
        described.name("Described");

#if DEBUG_CONDITION_PARSERS
        debug(all);
        debug(none);
        debug(source);
        debug(root_candidate);
        debug(target);
        debug(stationary);
        debug(aggressive);
        debug(capital1);
        debug(capital2);
        debug(monster);
        debug(armed);
        debug(owned_by);
        debug(and_);
        debug(or_);
        debug(not_);
        debug(ordered_alternatives_of);
        debug(described);
#endif
    }

} }
