#ifndef _GameRuleCategories_h_
#define _GameRuleCategories_h_

#include "Enum.h"

namespace GameRuleCategories {
    FO_ENUM(
        (GameRuleCategory),
        ((GENERAL, 0))
        ((CONTENT))
        ((BALANCE))
        ((TEST))
        ((BALANCE_STABILITY))
        ((PLANET_SIZE))
        ((MULTIPLAYER))
        ((UNDEFINED, std::numeric_limits<int8_t>::max()))
    )
}

#endif
