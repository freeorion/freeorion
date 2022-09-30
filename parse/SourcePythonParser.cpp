#include "SourcePythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Conditions.h"
#include "../universe/ValueRefs.h"

value_ref_wrapper<int> variable_wrapper::get_int_property(const char *property) const {
    std::vector property_name = std::vector(m_container);
    property_name.emplace_back(property);
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, property_name));
}

value_ref_wrapper<double> variable_wrapper::get_double_property(const char *property) const {
    std::vector property_name = std::vector(m_container);
    property_name.emplace_back(property);
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, property_name));
}

value_ref_wrapper<std::string> variable_wrapper::get_string_property(const char *property) const {
    std::vector property_name = std::vector(m_container);
    property_name.emplace_back(property);
    return value_ref_wrapper<std::string>(std::make_shared<ValueRef::Variable<std::string>>(m_reference_type, property_name));
}

variable_wrapper variable_wrapper::get_variable_property(const char *property) const {
    return variable_wrapper(m_reference_type, std::vector{std::string(property)});
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

condition_wrapper operator&(const variable_wrapper& lhs, const value_ref_wrapper<double>& rhs) {
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
            throw std::runtime_error(std::string("Not implemented in ") + __func__ + " type " + std::to_string(static_cast<int>(lhs.m_reference_type)) + rhs.value_ref->Dump());
    }

    std::shared_ptr<ValueRef::Operation<double>> rhs_op = std::dynamic_pointer_cast<ValueRef::Operation<double>>(rhs.value_ref);

    if (rhs_op && rhs_op->LHS() && rhs_op->RHS()) {
        Condition::ComparisonType cmp_type;
        switch (rhs_op->GetOpType()) {
            case ValueRef::OpType::COMPARE_EQUAL:
                cmp_type = Condition::ComparisonType::EQUAL;
                break;
            case ValueRef::OpType::COMPARE_GREATER_THAN:
                cmp_type = Condition::ComparisonType::GREATER_THAN;
                break;
            case ValueRef::OpType::COMPARE_GREATER_THAN_OR_EQUAL:
                cmp_type = Condition::ComparisonType::GREATER_THAN_OR_EQUAL;
                break;
            case ValueRef::OpType::COMPARE_LESS_THAN:
                cmp_type = Condition::ComparisonType::LESS_THAN;
                break;
            case ValueRef::OpType::COMPARE_LESS_THAN_OR_EQUAL:
                cmp_type = Condition::ComparisonType::LESS_THAN_OR_EQUAL;
                break;
            case ValueRef::OpType::COMPARE_NOT_EQUAL:
                cmp_type = Condition::ComparisonType::NOT_EQUAL;
                break;
            default:
                throw std::runtime_error(std::string("Not implemented in ") + __func__ + " op type " + std::to_string(static_cast<int>(rhs_op->GetOpType())) + rhs.value_ref->Dump());
        }

        return condition_wrapper(std::make_shared<Condition::And>(
            std::move(variable),
            std::make_unique<Condition::ValueTest>(rhs_op->LHS()->Clone(),
                cmp_type,
                rhs_op->RHS()->Clone())
        ));
    }

    throw std::runtime_error(std::string("Not implemented in ") + __func__ + " type " + std::to_string(static_cast<int>(lhs.m_reference_type)) + rhs.value_ref->Dump());
}

condition_wrapper operator~(const variable_wrapper& lhs) {
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
            throw std::runtime_error(std::string("Not implemented in ") + __func__ + " type " + std::to_string(static_cast<int>(lhs.m_reference_type)));
    }

    return condition_wrapper(std::make_shared<Condition::Not>(std::move(variable)));
}

void RegisterGlobalsSources(boost::python::dict& globals) {
    globals["Source"] = variable_wrapper(ValueRef::ReferenceType::SOURCE_REFERENCE);
    globals["Target"] = variable_wrapper(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE);
    globals["LocalCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    globals["RootCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE);
}

