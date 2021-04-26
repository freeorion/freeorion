#include "ConditionPythonParser.h"

#include "../universe/Conditions.h"

condition_wrapper operator&(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::And>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

condition_wrapper operator|(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::Or>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

condition_wrapper operator~(const condition_wrapper& lhs) {
return condition_wrapper(std::make_shared<Condition::Not>(
        lhs.condition->Clone()
    ));
}

