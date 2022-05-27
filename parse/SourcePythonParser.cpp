#include "SourcePythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Conditions.h"
#include "../universe/ValueRefs.h"

value_ref_wrapper<int> variable_wrapper::owner() const {
    auto variable = std::make_shared<ValueRef::Variable<int>>(m_reference_type, "Owner");
    return value_ref_wrapper<int>(variable);
}

value_ref_wrapper<int> variable_wrapper::id() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, "ID"));
}

value_ref_wrapper<int> variable_wrapper::system_id() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, "SystemID"));
}

value_ref_wrapper<int> variable_wrapper::design_id() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, "DesignID"));
}

value_ref_wrapper<int> variable_wrapper::last_turn_attacked_by_ship() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, "LastTurnAttackedByShip"));
}

value_ref_wrapper<int> variable_wrapper::last_turn_conquered() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, "LastTurnConquered"));
}

value_ref_wrapper<int> variable_wrapper::last_turn_colonized() const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, "LastTurnColonized"));
}

value_ref_wrapper<double> variable_wrapper::construction() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "Construction"));
}

value_ref_wrapper<double> variable_wrapper::habitable_size() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "HabitableSize"));
}

value_ref_wrapper<double> variable_wrapper::max_shield() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "MaxShield"));
}

value_ref_wrapper<double> variable_wrapper::max_defense() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "MaxDefense"));
}

value_ref_wrapper<double> variable_wrapper::max_troops() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "MaxTroops"));
}

value_ref_wrapper<double> variable_wrapper::target_happiness() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "TargetHappiness"));
}

value_ref_wrapper<double> variable_wrapper::target_industry() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "TargetIndustry"));
}

value_ref_wrapper<double> variable_wrapper::target_research() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "TargetResearch"));
}

value_ref_wrapper<double> variable_wrapper::target_construction() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "TargetConstruction"));
}

value_ref_wrapper<double> variable_wrapper::max_stockpile() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "MaxStockpile"));
}

value_ref_wrapper<double> variable_wrapper::population() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "Population"));
}

value_ref_wrapper<double> variable_wrapper::industry() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "Industry"));
}

value_ref_wrapper<double> variable_wrapper::research() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "Research"));
}

value_ref_wrapper<double> variable_wrapper::stockpile() const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, "Stockpile"));
}

variable_wrapper::operator condition_wrapper() const {
    switch (m_reference_type) {
        case ValueRef::ReferenceType::SOURCE_REFERENCE:
            return condition_wrapper(std::make_shared<Condition::Source>());
        case ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE:
            return condition_wrapper(std::make_shared<Condition::Target>());
        case ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:
            return condition_wrapper(std::make_shared<Condition::RootCandidate>());
        default:
            throw std::runtime_error(std::string("Not implemented in ") + __func__ + " type " + std::to_string(static_cast<signed int>(m_reference_type)));
    }
}

condition_wrapper operator&(const variable_wrapper& lhs, const condition_wrapper& rhs) {
    std::unique_ptr<Condition::Condition> variable;
    switch (lhs.m_reference_type) {
        case ValueRef::ReferenceType::SOURCE_REFERENCE:
            variable = std::make_unique<Condition::Source>();
            break;
        case ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE:
            variable = std::make_unique<Condition::Target>();
            break;
        case ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE:
            variable = std::make_unique<Condition::RootCandidate>();
            break;
        default:
            throw std::runtime_error(std::string("Not implemented in ") + __func__ + " type " + std::to_string(static_cast<int>(lhs.m_reference_type)) + rhs.condition->Dump());
    }

    return condition_wrapper(std::make_shared<Condition::And>(
        std::move(variable),
        rhs.condition->Clone()
    ));
}

void RegisterGlobalsSources(boost::python::dict& globals) {
    globals["Source"] = variable_wrapper(ValueRef::ReferenceType::SOURCE_REFERENCE);
    globals["Target"] = variable_wrapper(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE);
    globals["LocalCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    globals["RootCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE);
}

