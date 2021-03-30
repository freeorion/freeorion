#include "SourcePythonParser.h"

#include "../universe/ValueRefs.h"

value_ref_wrapper<int> source_wrapper::owner() const {
    auto variable = std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner");
    return value_ref_wrapper<int>(variable);
}

value_ref_wrapper<double> target_wrapper::habitable_size() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "HabitableSize"));
}

