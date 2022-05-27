#ifndef _SourcePythonParser_h_
#define _SourcePythonParser_h_

#include "ValueRefPythonParser.h"

struct variable_wrapper {
    variable_wrapper(ValueRef::ReferenceType reference_type)
        : m_reference_type(reference_type)
    {}

    value_ref_wrapper<int> owner() const;
    value_ref_wrapper<int> id() const;
    value_ref_wrapper<int> system_id() const;
    value_ref_wrapper<int> design_id() const;
    value_ref_wrapper<int> last_turn_attacked_by_ship() const;
    value_ref_wrapper<int> last_turn_conquered() const;
    value_ref_wrapper<int> last_turn_colonized() const;

    value_ref_wrapper<double> construction() const;
    value_ref_wrapper<double> habitable_size() const;
    value_ref_wrapper<double> max_shield() const;
    value_ref_wrapper<double> max_defense() const;
    value_ref_wrapper<double> max_troops() const;
    value_ref_wrapper<double> target_happiness() const;
    value_ref_wrapper<double> target_industry() const;
    value_ref_wrapper<double> target_research() const;
    value_ref_wrapper<double> target_construction() const;
    value_ref_wrapper<double> max_stockpile() const;
    value_ref_wrapper<double> population() const;

    value_ref_wrapper<double> industry() const;
    value_ref_wrapper<double> research() const;
    value_ref_wrapper<double> stockpile() const;

    operator condition_wrapper() const;

    const ValueRef::ReferenceType m_reference_type;
};

condition_wrapper operator&(const variable_wrapper&, const condition_wrapper&);

void RegisterGlobalsSources(boost::python::dict& globals);

#endif // _SourcePythonParser_h_
