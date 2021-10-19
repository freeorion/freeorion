#ifndef _SourcePythonParser_h_
#define _SourcePythonParser_h_

#include "ValueRefPythonParser.h"

struct source_wrapper {
    value_ref_wrapper<int> owner() const;
};

struct target_wrapper {
    value_ref_wrapper<double> habitable_size() const;
    value_ref_wrapper<double> max_shield() const;
    value_ref_wrapper<double> max_defense() const;
    value_ref_wrapper<double> max_troops() const;

    value_ref_wrapper<int> id() const;
    value_ref_wrapper<int> owner() const;
    value_ref_wrapper<int> system_id() const;
    value_ref_wrapper<int> design_id() const;
};

struct local_candidate_wrapper {
    value_ref_wrapper<int> last_turn_attacked_by_ship() const;
    value_ref_wrapper<int> last_turn_conquered() const;
    value_ref_wrapper<int> last_turn_colonized() const;
};

void RegisterGlobalsSources(boost::python::dict& globals);

#endif // _SourcePythonParser_h_
