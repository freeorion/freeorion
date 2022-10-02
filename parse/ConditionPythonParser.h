#ifndef _ConditionPythonParser_h_
#define _ConditionPythonParser_h_

#include <memory>

#include "../universe/Condition.h"

template<typename T>
struct value_ref_wrapper;

namespace boost::python {
    class dict;
}

struct condition_wrapper {
    condition_wrapper(std::shared_ptr<Condition::Condition>&& ref) : condition(std::move(ref)) {}
    condition_wrapper(const std::shared_ptr<Condition::Condition>& ref) : condition(ref) {}

    std::shared_ptr<Condition::Condition> condition;
};

condition_wrapper operator&(const condition_wrapper&, const condition_wrapper&);
condition_wrapper operator&(const condition_wrapper&, const value_ref_wrapper<double>&);
condition_wrapper operator|(const condition_wrapper&, const condition_wrapper&);
condition_wrapper operator~(const condition_wrapper&);

void RegisterGlobalsConditions(boost::python::dict& globals);

#endif // _ConditionPythonParser_h_

