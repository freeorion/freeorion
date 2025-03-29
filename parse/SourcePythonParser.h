#ifndef _SourcePythonParser_h_
#define _SourcePythonParser_h_

#include "ValueRefPythonParser.h"

struct variable_wrapper {
    explicit variable_wrapper(ValueRef::ReferenceType reference_type,
                              ValueRef::ContainerType container = ValueRef::ContainerType::NONE) :
        m_reference_type(reference_type),
        m_container(container)
    {}

    template<typename T>
    value_ref_wrapper<T> get_property(std::string property) const {
        return value_ref_wrapper<T>(std::make_shared<ValueRef::Variable<T>>(
            m_reference_type, std::move(property), m_container, ValueRef::ValueToReturn::Initial));
    }
    variable_wrapper get_variable_property(std::string_view container) const;

    static ValueRef::ContainerType ToContainer(std::string_view s) noexcept;

    const ValueRef::ReferenceType m_reference_type;
    const ValueRef::ContainerType m_container;
};

void RegisterGlobalsSources(boost::python::dict& globals);

#endif // _SourcePythonParser_h_
