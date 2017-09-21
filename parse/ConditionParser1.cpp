#include "ConditionParserImpl.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Condition::ConditionBase*>&) { return os; }
}
#endif

namespace {
    struct condition_parser_rules_1 {
        condition_parser_rules_1(const parse::lexer& tok, const parse::condition_parser_rule& condition_parser) :
            int_rules(tok)
        {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::lit_type lit;
            using phoenix::new_;
            using phoenix::push_back;

            all
                =   tok.All_ [ _val = new_<Condition::All>() ]
                ;

            none
                =   tok.None_ [ _val = new_<Condition::None>() ]
                ;

            source
                =   tok.Source_ [ _val = new_<Condition::Source>() ]
                ;

            root_candidate
                =   tok.RootCandidate_ [ _val = new_<Condition::RootCandidate>() ]
                ;

            target
                =   tok.Target_ [ _val = new_<Condition::Target>() ]
                ;

            stationary
                =   tok.Stationary_ [ _val = new_<Condition::Stationary>() ]
                ;

            aggressive
                = ((tok.Aggressive_ [ _val = new_<Condition::Aggressive>(true) ])
                  |(tok.Passive_ [ _val = new_<Condition::Aggressive>(false) ])
                  )
                ;

            can_colonize
                =   tok.CanColonize_ [ _val = new_<Condition::CanColonize>() ]
                ;

            can_produce_ships
                =   tok.CanProduceShips_ [ _val = new_<Condition::CanProduceShips>() ]
                ;

            capital
                =   tok.Capital_ [ _val = new_<Condition::Capital>() ]
                ;

            monster
                =   tok.Monster_ [ _val = new_<Condition::Monster>() ]
                ;

            armed
                =   tok.Armed_ [ _val = new_<Condition::Armed>() ]
                ;

            owned_by_1
                =   (tok.OwnedBy_
                     >>  parse::detail::label(Empire_token)
                    ) > int_rules.expr
                    [ _val = new_<Condition::EmpireAffiliation>(_1) ]
                ;

            owned_by_2
                =   tok.OwnedBy_
                >>  parse::detail::label(Affiliation_token) >> tok.AnyEmpire_
                [ _val = new_<Condition::EmpireAffiliation>( AFFIL_ANY ) ]
                ;

            owned_by_3
                =   tok.Unowned_
                    [ _val = new_<Condition::EmpireAffiliation>( AFFIL_NONE ) ]
                ;

            owned_by_4
                =   tok.Human_
                    [ _val = new_<Condition::EmpireAffiliation>( AFFIL_HUMAN ) ]
                ;

            owned_by_5
                =   (tok.OwnedBy_
                >>  parse::detail::label(Affiliation_token) >> parse::empire_affiliation_type_enum() [ _a = _1 ]
                >>  parse::detail::label(Empire_token)    ) >  int_rules.expr
                [ _val = new_<Condition::EmpireAffiliation>(_1, _a) ]
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
                [ _val = new_<Condition::And>(_a) ]
                ;

            or_
                =   tok.Or_
                >   '[' > +condition_parser [ push_back(_a, _1) ] > lit(']')
                [ _val = new_<Condition::Or>(_a) ]
                ;

            not_
                =   tok.Not_
                >   condition_parser [ _val = new_<Condition::Not>(_1) ]
                ;

            described
                =   tok.Described_
                >   parse::detail::label(Description_token) > tok.string [ _a = _1 ]
                >   parse::detail::label(Condition_token) > condition_parser
                [ _val = new_<Condition::Described>(_1, _a) ]
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

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<EmpireAffiliationType>
        > owned_by_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<std::vector<Condition::ConditionBase*>>
        > and_or_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<std::string>
        > described_rule;

        parse::int_arithmetic_rules     int_rules;
        parse::condition_parser_rule    all;
        parse::condition_parser_rule    none;
        parse::condition_parser_rule    source;
        parse::condition_parser_rule    root_candidate;
        parse::condition_parser_rule    target;
        parse::condition_parser_rule    stationary;
        parse::condition_parser_rule    aggressive;
        parse::condition_parser_rule    can_colonize;
        parse::condition_parser_rule    can_produce_ships;
        parse::condition_parser_rule    capital;
        parse::condition_parser_rule    monster;
        parse::condition_parser_rule    armed;
        parse::condition_parser_rule    owned_by_1;
        parse::condition_parser_rule    owned_by_2;
        parse::condition_parser_rule    owned_by_3;
        parse::condition_parser_rule    owned_by_4;
        owned_by_rule                   owned_by_5;
        parse::condition_parser_rule    owned_by;
        and_or_rule                     and_;
        and_or_rule                     or_;
        parse::condition_parser_rule    not_;
        described_rule                  described;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_1() {
        static condition_parser_rules_1 retval(parse::lexer::instance(), parse::condition_parser());
        return retval.start;
    }
} }
