#ifndef _parse_UnlockableItem_h_
#define _parse_UnlockableItem_h_

#include "Lexer.h"

#include "ParseImpl.h"
#include "../universe/UnlockableItem.h"

namespace parse { namespace detail {
    struct unlockable_item_type_grammar : public detail::enum_grammar<UnlockableItemType> {
        unlockable_item_type_grammar(const parse::lexer& tok);
        detail::enum_rule<UnlockableItemType> rule;
    };

    struct unlockable_item_grammar : public grammar<UnlockableItem ()> {
        unlockable_item_grammar(const parse::lexer& tok, Labeller& label);
        unlockable_item_type_grammar unlockable_item_type_enum;
        rule<UnlockableItem ()> start;
    };
} }

#endif // _parse_UnlockableItem_h_
