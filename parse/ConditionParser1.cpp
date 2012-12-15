#include "ConditionParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/home/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<const Condition::ConditionBase*>&) { return os; }
}
#endif

namespace {
    struct condition_parser_rules_1 {
        condition_parser_rules_1() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =
                parse::value_ref_parser<int>();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::lit_type lit;
            using phoenix::new_;
            using phoenix::push_back;

            all
                =    tok.All_ [ _val = new_<Condition::All>() ]
                ;

            source
                =    tok.Source_ [ _val = new_<Condition::Source>() ]
                ;

            root_candidate
                =    tok.RootCandidate_ [ _val = new_<Condition::RootCandidate>() ]
                ;

            target
                =    tok.Target_ [ _val = new_<Condition::Target>() ]
                ;

            stationary
                =    tok.Stationary_ [ _val = new_<Condition::Stationary>() ]
                ;

            can_colonize
                =    tok.CanColonize_ [ _val = new_<Condition::CanColonize>() ]
                ;

            can_produce_ships
                =    tok.CanProduceShips_ [ _val = new_<Condition::CanProduceShips>() ]
                ;

            capital
                =    tok.Capital_ [ _val = new_<Condition::Capital>() ]
                ;

            monster
                =    tok.Monster_ [ _val = new_<Condition::Monster>() ]
                ;

            armed
                =    tok.Armed_ [ _val = new_<Condition::Armed>() ]
                ;

            owned_by
                =    tok.OwnedBy_
                >>  -(
                            parse::label(Affiliation_name) >> parse::enum_parser<EmpireAffiliationType>() [ _a = _1 ]
                        |   eps [ _a = AFFIL_SELF ]
                     )
                >>   (
                            parse::label(Empire_name) >> int_value_ref [ _val = new_<Condition::EmpireAffiliation>(_1, _a) ]
                        |   eps [ _val = new_<Condition::EmpireAffiliation>(_a) ]
                     )
                ;

            and_
                =    tok.And_
                >>   '[' >> parse::detail::condition_parser [ push_back(_a, _1) ] >> +parse::detail::condition_parser [ push_back(_a, _1) ] >> lit(']')
                        [ _val = new_<Condition::And>(_a) ]
                ;

            or_
                =    tok.Or_
                >>   '[' >> parse::detail::condition_parser [ push_back(_a, _1) ] >> +parse::detail::condition_parser [ push_back(_a, _1) ] >> lit(']')
                        [ _val = new_<Condition::Or>(_a) ]
                ;

            not_
                =    tok.Not_
                >>   parse::detail::condition_parser [ _val = new_<Condition::Not>(_1) ]
                ;

            start
                %=   all
                |    source
                |    root_candidate
                |    target
                |    stationary
                |    can_colonize
                |    can_produce_ships
                |    capital
                |    monster
                |    armed
                |    owned_by
                |    and_
                |    or_
                |    not_
                ;

            all.name("All");
            source.name("Source");
            root_candidate.name("RootCandidate");
            target.name("Target");
            stationary.name("Stationary");
            can_colonize.name("CanColonize");
            can_produce_ships.name("CanProduceShips");
            capital.name("Capital");
            monster.name("Monster");
            armed.name("Armed");
            owned_by.name("OwnedBy");  // TODO: Should this be renamed Affilated or similar?
            and_.name("And");
            or_.name("Or");
            not_.name("Not");

#if DEBUG_CONDITION_PARSERS
            debug(all);
            debug(source);
            debug(root_candidate);
            debug(target);
            debug(stationary);
            debug(capital);
            debug(monster);
            debug(armed);
            debug(owned_by);
            debug(and_);
            debug(or_);
            debug(not_);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<EmpireAffiliationType>,
            parse::skipper_type
        > owned_by_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<const Condition::ConditionBase*> >,
            parse::skipper_type
        > and_or_rule;

        parse::condition_parser_rule    all;
        parse::condition_parser_rule    source;
        parse::condition_parser_rule    root_candidate;
        parse::condition_parser_rule    target;
        parse::condition_parser_rule    stationary;
        parse::condition_parser_rule    can_colonize;
        parse::condition_parser_rule    can_produce_ships;
        parse::condition_parser_rule    capital;
        parse::condition_parser_rule    monster;
        parse::condition_parser_rule    armed;
        owned_by_rule                   owned_by;
        and_or_rule                     and_;
        and_or_rule                     or_;
        parse::condition_parser_rule    not_;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_1() {
        static condition_parser_rules_1 retval;
        return retval.start;
    }
} }
