#ifndef _SourcePythonParser_h_
#define _SourcePythonParser_h_

#include "ValueRefPythonParser.h"

struct variable_wrapper {
    variable_wrapper(ValueRef::ReferenceType reference_type)
        : m_reference_type(reference_type)
    {}

    value_ref_wrapper<int> get_int_property(const char *property) const;
    value_ref_wrapper<double> get_double_property(const char *property) const;

    operator condition_wrapper() const;

    const ValueRef::ReferenceType m_reference_type;
};

condition_wrapper operator&(const variable_wrapper&, const condition_wrapper&);

void RegisterGlobalsSources(boost::python::dict& globals);

#endif // _SourcePythonParser_h_
