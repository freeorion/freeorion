#ifndef _ConditionParser_h_
#define _ConditionParser_h_

#include "Lexer.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"

#include <boost/spirit/include/qi.hpp>

namespace Condition {
    enum SortingMethod : int;
    enum ComparisonType : int;
    enum ContentType : int;
    struct ConditionBase;
}

namespace parse {
    typedef detail::rule<
        Condition::ConditionBase* ()
    > condition_parser_rule;

    /** Returns a const reference to the Condition parser. */
    condition_parser_rule& condition_parser();
}

namespace parse { namespace detail {
    using condition_signature      = Condition::ConditionBase* ();
    using condition_parser_rule    = rule<condition_signature>;
    using condition_parser_grammar = grammar<condition_signature>;

    struct condition_parser_rules_1 : public condition_parser_grammar {
        condition_parser_rules_1(const parse::lexer& tok,
                                 const condition_parser_grammar& condition_parser);

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<EmpireAffiliationType>
        > owned_by_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<std::vector<Condition::ConditionBase*>>
        > and_or_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<std::string>
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

    struct condition_parser_rules_2 : public condition_parser_grammar {
        condition_parser_rules_2(const parse::lexer& tok,
                                 const condition_parser_grammar& condition_parser);

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*
            >
        > common_rule;

        parse::int_arithmetic_rules     int_rules;
        parse::castable_as_int_parser_rules    castable_int_rules;
        common_rule                     has_special_since_turn;
        common_rule                     enqueued;
        common_rule                     enqueued1;
        common_rule                     enqueued2;
        common_rule                     enqueued3;
        common_rule                     enqueued4;
        common_rule                     design_has_part;
        common_rule                     design_has_part_class;
        common_rule                     in_system;
        parse::condition_parser_rule    start;
    };

    struct condition_parser_rules_3 : public condition_parser_grammar {
        condition_parser_rules_3(const parse::lexer& tok,
                                 const condition_parser_grammar& condition_parser);

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*,
                Condition::ComparisonType,
                Condition::ComparisonType,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*
            >
        > double_ref_double_ref_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*
            >
        > int_ref_int_ref_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<int>*,
                Condition::SortingMethod,
                ValueRef::ValueRefBase<double>*
            >
        > int_ref_sorting_method_double_ref_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                ResourceType,
                ValueRef::ValueRefBase<double>*
            >
        > resource_type_double_ref_rule;

        parse::int_arithmetic_rules             int_rules;
        parse::castable_as_int_parser_rules     castable_int_rules;
        parse::double_parser_rules              double_rules;
        double_ref_double_ref_rule              has_special_capacity;
        double_ref_double_ref_rule              within_distance;
        int_ref_int_ref_rule                    within_starlane_jumps;
        int_ref_int_ref_rule                    number;
        double_ref_double_ref_rule              value_test_1;
        double_ref_double_ref_rule              value_test_2;
        double_ref_double_ref_rule              value_test_3;
        double_ref_double_ref_rule              value_test_4;
        double_ref_double_ref_rule              value_test_5;
        double_ref_double_ref_rule              value_test_6;
        int_ref_int_ref_rule                    turn;
        int_ref_int_ref_rule                    created_on_turn;
        int_ref_sorting_method_double_ref_rule  number_of;
        int_ref_sorting_method_double_ref_rule  number_of1;
        int_ref_sorting_method_double_ref_rule  number_of2;
        parse::condition_parser_rule            random;
        resource_type_double_ref_rule           owner_stockpile;
        int_ref_int_ref_rule                    resource_supply_connected;
        parse::condition_parser_rule            can_add_starlane;
        parse::condition_parser_rule            start;
    };

    struct condition_parser_rules_4 : public condition_parser_grammar {
        condition_parser_rules_4(const parse::lexer& tok,
                                 const condition_parser_grammar& condition_parser);

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                MeterType,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                std::string,
                ValueRef::ValueRefBase<std::string>*
            >
        > meter_value_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >
        > empire_meter_value_rule;

        parse::int_arithmetic_rules     int_rules;
        parse::double_parser_rules      double_rules;
        meter_value_rule                meter_value;
        meter_value_rule                ship_part_meter_value;
        empire_meter_value_rule         empire_meter_value;
        empire_meter_value_rule         empire_meter_value1;
        empire_meter_value_rule         empire_meter_value2;
        parse::condition_parser_rule    start;
    };


    struct condition_parser_rules_5 : public condition_parser_grammar {
        condition_parser_rules_5(const parse::lexer& tok,
                                 const condition_parser_grammar& condition_parser);

        parse::int_arithmetic_rules     int_rules;
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

    struct condition_parser_rules_6 : public condition_parser_grammar {
        condition_parser_rules_6();

        typedef parse::detail::rule<
            std::vector<ValueRef::ValueRefBase<std::string>*> ()
        > string_ref_vec_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<std::vector<ValueRef::ValueRefBase<std::string>*>>
        > building_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<std::vector<ValueRef::ValueRefBase<PlanetType>*>>
        > planet_type_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<std::vector<ValueRef::ValueRefBase<PlanetSize>*>>
        > planet_size_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                std::vector<ValueRef::ValueRefBase<PlanetEnvironment>*>,
                ValueRef::ValueRefBase<std::string>*
            >
        > planet_environment_rule;

        string_ref_vec_rule             string_ref_vec;
        parse::condition_parser_rule    homeworld;
        building_rule                   building;
        parse::condition_parser_rule    species;
        parse::condition_parser_rule    focus_type;
        planet_type_rule                planet_type;
        planet_size_rule                planet_size;
        planet_environment_rule         planet_environment;
        parse::condition_parser_rule    object_type;
        parse::condition_parser_rule    start;
    };

    struct condition_parser_rules_7 : public condition_parser_grammar {
        condition_parser_rules_7(const condition_parser_grammar& condition_parser);

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<std::vector<ValueRef::ValueRefBase<StarType>*>>
        > star_type_vec_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            boost::spirit::qi::locals<
                Condition::ContentType,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*
            >
        > string_ref_rule;

        parse::condition_parser_rule            ordered_bombarded_by;
        parse::condition_parser_rule            contains;
        parse::condition_parser_rule            contained_by;
        star_type_vec_rule                      star_type;
        string_ref_rule                         location;
        parse::condition_parser_rule            owner_has_shippart_available;
        parse::condition_parser_rule            start;
    };

}}

namespace parse {
    struct conditions_parser_grammar : public detail::condition_parser_grammar {
        conditions_parser_grammar(const parse::lexer& tok);

        detail::condition_parser_rules_1 condition_parser_1;
        detail::condition_parser_rules_2 condition_parser_2;
        detail::condition_parser_rules_3 condition_parser_3;
        detail::condition_parser_rules_4 condition_parser_4;
        detail::condition_parser_rules_5 condition_parser_5;
        detail::condition_parser_rules_6 condition_parser_6;
        detail::condition_parser_rules_7 condition_parser_7;
        condition_parser_rule start;
    };
}

#endif
