#include "SourcePythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Conditions.h"
#include "../universe/ValueRefs.h"

value_ref_wrapper<int> source_wrapper::owner() const {
    auto variable = std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner");
    return value_ref_wrapper<int>(variable);
}

source_wrapper::operator condition_wrapper() const {
    return condition_wrapper(std::make_shared<Condition::Source>());
}

value_ref_wrapper<double> target_wrapper::construction() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Construction"));
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

value_ref_wrapper<double> target_wrapper::target_happiness() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "TargetHappiness"));
}

value_ref_wrapper<double> target_wrapper::target_industry() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "TargetIndustry"));
}

value_ref_wrapper<double> target_wrapper::target_research() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "TargetResearch"));
}

value_ref_wrapper<double> target_wrapper::target_construction() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "TargetConstruction"));
}

value_ref_wrapper<double> target_wrapper::max_stockpile() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "MaxStockpile"));
}

value_ref_wrapper<double> target_wrapper::population() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Population"));
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

target_wrapper::operator condition_wrapper() const {
    return condition_wrapper(std::make_shared<Condition::Target>());
}

condition_wrapper operator&(const target_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::And>(
        std::make_unique<Condition::Target>(),
        rhs.condition->Clone()
    ));
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

value_ref_wrapper<double> local_candidate_wrapper::industry() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Industry"));
}

value_ref_wrapper<double> local_candidate_wrapper::target_industry() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "TargetIndustry"));
}

value_ref_wrapper<double> local_candidate_wrapper::research() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Research"));
}

value_ref_wrapper<double> local_candidate_wrapper::target_research() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "TargetResearch"));
}

value_ref_wrapper<double> local_candidate_wrapper::construction() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Construction"));
}

value_ref_wrapper<double> local_candidate_wrapper::target_construction() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "TargetConstruction"));
}

value_ref_wrapper<double> local_candidate_wrapper::stockpile() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Stockpile"));
}

value_ref_wrapper<double> local_candidate_wrapper::max_stockpile() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "MaxStockpile"));
}

void RegisterGlobalsSources(boost::python::dict& globals) {
    globals["Source"] = source_wrapper();
    globals["Target"] = target_wrapper();
    globals["LocalCandidate"] = local_candidate_wrapper();
}

