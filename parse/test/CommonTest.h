#ifndef COMMON_HPP
#define COMMON_HPP

#include <ostream>
#include <typeinfo>

#include "universe/ValueRefFwd.h"

namespace std {
    std::ostream& operator << (std::ostream& stream, const std::type_info& type);
    std::ostream& operator << (std::ostream& stream, const ValueRef::OpType& type);
}

#endif 
