// -*- C++ -*-
#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "Lexer.h"

#include <boost/spirit/include/qi.hpp>


namespace parse {

    template <typename T>
    struct value_ref_parser_rule
    {
        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            // TODO: Investigate refactoring ValueRef to use variant, for increased locality of reference.
            ValueRef::ValueRefBase<T>* (), // TODO: Rename this ValueRef::Base in the FO code.
            parse::skipper_type
        > type;
    };

    /** Returns a const reference to the ValueRef parser for the type \a T. */
    template <typename T>
    typename value_ref_parser_rule<T>::type& value_ref_parser();

    template <>
    value_ref_parser_rule<int>::type& value_ref_parser<int>();

    template <>
    value_ref_parser_rule<double>::type& value_ref_parser<double>();

    template <>
    value_ref_parser_rule<std::string>::type& value_ref_parser<std::string>();

    template <>
    value_ref_parser_rule<PlanetSize>::type& value_ref_parser<PlanetSize>();

    template <>
    value_ref_parser_rule<PlanetType>::type& value_ref_parser<PlanetType>();

    template <>
    value_ref_parser_rule<PlanetEnvironment>::type& value_ref_parser<PlanetEnvironment>();

    template <>
    value_ref_parser_rule<UniverseObjectType>::type& value_ref_parser<UniverseObjectType>();

    template <>
    value_ref_parser_rule<StarType>::type& value_ref_parser<StarType>();

}

#endif
