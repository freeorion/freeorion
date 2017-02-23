#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>


namespace parse {
    template <typename T>
    struct value_ref_parser_rule
    {
        typedef detail::rule<
            // TODO: Investigate refactoring ValueRef to use variant,
            // for increased locality of reference.
            ValueRef::ValueRefBase<T>* ()
        > type;
    };

    value_ref_parser_rule<int>::type& int_value_ref();

    value_ref_parser_rule<int>::type& flexible_int_value_ref();

    value_ref_parser_rule<double>::type& double_value_ref();

    value_ref_parser_rule<std::string>::type& string_value_ref();

    value_ref_parser_rule<PlanetSize>::type& planet_size_value_ref();

    value_ref_parser_rule<PlanetType>::type& planet_type_value_ref();

    value_ref_parser_rule<PlanetEnvironment>::type& planet_environment_value_ref();

    value_ref_parser_rule<UniverseObjectType>::type& universe_object_type_value_ref();

    value_ref_parser_rule<StarType>::type& star_type_value_ref();
}

#endif
