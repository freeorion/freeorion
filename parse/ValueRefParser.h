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

    namespace detail {
        template <typename T>
        struct enum_value_ref_rules;

        enum_value_ref_rules<PlanetEnvironment>& planet_environment_rules();
        enum_value_ref_rules<PlanetSize>& planet_size_rules();
        enum_value_ref_rules<PlanetType>& planet_type_rules();
        enum_value_ref_rules<StarType>& star_type_rules();
        enum_value_ref_rules<Visibility>& visibility_rules();
        enum_value_ref_rules<UniverseObjectType>& universe_object_type_rules();
    }
}

#endif
