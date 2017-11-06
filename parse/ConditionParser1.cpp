#include "ConditionParser1.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

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
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_1::base_type(start, "condition_parser_rules_1"),
        int_rules(tok, labeller, condition_parser, string_grammar),
        empire_affiliation_type_enum(tok)
    {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::lit_type lit;
        qi::_pass_type _pass;
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

        capital
            =   tok.Capital_ [ _val = construct_movable_(new_<Condition::Capital>()) ]
            ;

        monster
            =   tok.Monster_ [ _val = construct_movable_(new_<Condition::Monster>()) ]
            ;

        armed
            =   tok.Armed_ [ _val = construct_movable_(new_<Condition::Armed>()) ]
            ;

        owned_by_1
            =   (tok.OwnedBy_
                 >>  labeller.rule(Empire_token)
                ) > int_rules.expr
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>(deconstruct_movable_(_1, _pass))) ]
            ;

        owned_by_2
            =   tok.OwnedBy_
            >>  labeller.rule(Affiliation_token) >> tok.AnyEmpire_
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>( AFFIL_ANY )) ]
            ;

        owned_by_3
            =   tok.Unowned_
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>( AFFIL_NONE )) ]
            ;

        owned_by_4
            =   tok.Human_
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>( AFFIL_HUMAN )) ]
            ;

        owned_by_5
            =   (tok.OwnedBy_
                 >>  labeller.rule(Affiliation_token) >> empire_affiliation_type_enum [ _a = _1 ]
                 >>  labeller.rule(Empire_token)    ) >  int_rules.expr
            [ _val = construct_movable_(new_<Condition::EmpireAffiliation>(deconstruct_movable_(_1, _pass), _a)) ]
            ;

        owned_by
            %=  owned_by_1
            |   owned_by_2
            |   owned_by_3
            |   owned_by_4
            |   owned_by_5
            ;

        and_
            =   tok.And_
            >   '[' > +condition_parser [ push_back(_a, _1) ] > lit(']')
            [ _val = construct_movable_(new_<Condition::And>(deconstruct_movable_vector_(_a, _pass))) ]
            ;

        or_
            =   tok.Or_
            >   '[' > +condition_parser [ push_back(_a, _1) ] > lit(']')
            [ _val = construct_movable_(new_<Condition::Or>(deconstruct_movable_vector_(_a, _pass))) ]
            ;

        not_
            =   tok.Not_
            >   condition_parser [ _val = construct_movable_(new_<Condition::Not>(deconstruct_movable_(_1, _pass))) ]
            ;

        described
            =   tok.Described_
            >   labeller.rule(Description_token) > tok.string [ _a = _1 ]
            >   labeller.rule(Condition_token) > condition_parser
            [ _val = construct_movable_( new_<Condition::Described>(deconstruct_movable_(_1, _pass), _a)) ]
            ;

        start
            %=  all
            |   none
            |   source
            |   root_candidate
            |   target
            |   stationary
            |   aggressive
            |   can_colonize
            |   can_produce_ships
            |   capital
            |   monster
            |   armed
            |   owned_by
            |   and_
            |   or_
            |   not_
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
        capital.name("Capital");
        monster.name("Monster");
        armed.name("Armed");
        owned_by.name("OwnedBy");  // TODO: Should this be renamed Affilated or similar?
        and_.name("And");
        or_.name("Or");
        not_.name("Not");
        described.name("Described");

#if DEBUG_CONDITION_PARSERS
        debug(all);
        debug(none);
        debug(source);
        debug(root_candidate);
        debug(target);
        debug(stationary);
        debug(aggressive);
        debug(capital);
        debug(monster);
        debug(armed);
        debug(owned_by);
        debug(and_);
        debug(or_);
        debug(not_);
        debug(described);
#endif
    }

} }
