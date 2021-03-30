#ifndef _SourcePythonParser_h_
#define _SourcePythonParser_h_

#include "ValueRefPythonParser.h"

struct source_wrapper {
    value_ref_wrapper<int> owner() const;
};

struct target_wrapper {
    value_ref_wrapper<double> habitable_size() const;
};

#endif // _SourcePythonParser_h_
