#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>


namespace parse {
    template <typename T>
    using value_ref_rule = detail::rule<
        // TODO: Investigate refactoring ValueRef to use variant,
        // for increased locality of reference.
        ValueRef::ValueRefBase<T>* ()
    >;

    value_ref_rule<int>& int_value_ref();

    value_ref_rule<int>& flexible_int_value_ref();

    value_ref_rule<double>& double_value_ref();

    value_ref_rule<std::string>& string_value_ref();

    value_ref_rule<PlanetSize>& planet_size_value_ref();

    value_ref_rule<PlanetType>& planet_type_value_ref();

    value_ref_rule<PlanetEnvironment>& planet_environment_value_ref();

    value_ref_rule<UniverseObjectType>& universe_object_type_value_ref();

    value_ref_rule<StarType>& star_type_value_ref();
}

#endif
