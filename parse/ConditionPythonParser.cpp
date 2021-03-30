#include "ConditionPythonParser.h"

#include "../universe/Conditions.h"

condition_wrapper operator&(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::And>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

