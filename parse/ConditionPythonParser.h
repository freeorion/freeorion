#ifndef _ConditionPythonParser_h_
#define _ConditionPythonParser_h_

#include <memory>

#include "../universe/Condition.h"

struct condition_wrapper {
    condition_wrapper(std::shared_ptr<Condition::Condition>&& ref)
        : condition(std::move(ref))
    { }

    condition_wrapper(const std::shared_ptr<Condition::Condition>& ref)
        : condition(ref)
    { }

    std::shared_ptr<Condition::Condition> condition;
};

condition_wrapper operator&(const condition_wrapper&, const condition_wrapper&);
condition_wrapper operator|(const condition_wrapper&, const condition_wrapper&);
condition_wrapper operator~(const condition_wrapper&);

#endif // _ConditionPythonParser_h_

