#ifndef _EnumParser_h_
#define _EnumParser_h_

#include "Lexer.h"

#include "ParseImpl.h"
#include "../universe/ValueRef.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>

struct UnlockableItem;

namespace parse {
    struct empire_affiliation_enum_grammar : public detail::enum_grammar<EmpireAffiliationType> {
        empire_affiliation_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<EmpireAffiliationType> rule;
    };

    struct unlockable_item_enum_grammar : public detail::enum_grammar<UnlockableItemType> {
        unlockable_item_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<UnlockableItemType> rule;
    };

    struct ship_slot_enum_grammar : public detail::enum_grammar<ShipSlotType> {
        ship_slot_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<ShipSlotType> rule;
    };

    struct ship_part_class_enum_grammar : public detail::enum_grammar<ShipPartClass> {
        ship_part_class_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<ShipPartClass> rule;
    };

    struct capture_result_enum_grammar : public detail::enum_grammar<CaptureResult> {
        capture_result_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<CaptureResult> rule;
    };

    struct statistic_enum_grammar : public detail::enum_grammar<ValueRef::StatisticType> {
        statistic_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<ValueRef::StatisticType> rule;
    };

    struct non_ship_part_meter_enum_grammar : public detail::enum_grammar<MeterType> {
        non_ship_part_meter_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<MeterType> rule;
    };

    struct ship_part_meter_enum_grammar : public detail::enum_grammar<MeterType> {
        ship_part_meter_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<MeterType> rule;
    };

    struct set_non_ship_part_meter_enum_grammar : public detail::enum_grammar<MeterType> {
        set_non_ship_part_meter_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<MeterType> rule;
    };

    struct set_ship_part_meter_enum_grammar : public detail::enum_grammar<MeterType> {
        set_ship_part_meter_enum_grammar(const parse::lexer& tok);
        detail::enum_rule<MeterType> rule;
    };

    namespace detail {
    using unlockable_item_rule_type = rule<UnlockableItem ()>;
    using unlockable_item_grammar_type = grammar<UnlockableItem ()>;

    struct unlockable_item_grammar : public unlockable_item_grammar_type {
        unlockable_item_grammar(const parse::lexer& tok, Labeller& label);
        parse::unlockable_item_enum_grammar unlockable_item_type_enum;
        unlockable_item_rule_type start;
    };
    }
}

#endif
