#include "SourcePythonParser.h"

#include "../universe/ValueRefs.h"

value_ref_wrapper<int> source_wrapper::owner() const {
    auto variable = std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner");
    return value_ref_wrapper<int>(variable);
}

value_ref_wrapper<double> target_wrapper::habitable_size() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "HabitableSize"));
}

value_ref_wrapper<double> target_wrapper::max_shield() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "MaxShield"));
}

value_ref_wrapper<double> target_wrapper::max_defense() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "MaxDefense"));
}

value_ref_wrapper<double> target_wrapper::max_troops() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "MaxTroops"));
}

value_ref_wrapper<int> target_wrapper::id() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "ID"));
}

value_ref_wrapper<int> target_wrapper::owner() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Owner"));
}

value_ref_wrapper<int> target_wrapper::system_id() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "SystemID"));
}

value_ref_wrapper<int> target_wrapper::design_id() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "DesignID"));
}

value_ref_wrapper<int> local_candidate_wrapper::last_turn_attacked_by_ship() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "LastTurnAttackedByShip"));
}

value_ref_wrapper<int> local_candidate_wrapper::last_turn_conquered() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "LastTurnConquered"));
}

value_ref_wrapper<int> local_candidate_wrapper::last_turn_colonized() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "LastTurnColonized"));
}

