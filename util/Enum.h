#ifndef _Enum_h_
#define _Enum_h_

#include <iostream>

#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/push_back.hpp>
#include <boost/preprocessor/tuple/size.hpp>


/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_VALUE(r, data, elem) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(elem), 3), \
        BOOST_PP_TUPLE_ELEM(0, elem) = BOOST_PP_TUPLE_ELEM(1, elem), \
        BOOST_PP_TUPLE_ELEM(0, elem)),

/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM(typeName, values) \
enum class \
BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
    BOOST_PP_TUPLE_ELEM(1, typeName), \
    BOOST_PP_TUPLE_ELEM(0, typeName)) \
: int { \
    BOOST_PP_SEQ_FOR_EACH(FO_DEF_ENUM_VALUE, _, values) \
};

/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_OSTREAM_CASE(r, data, elem) \
    case data::BOOST_PP_TUPLE_ELEM(0, elem): \
        stream << BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, elem)); \
        break;

/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_OSTREAM(typeName, values) \
inline \
BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
        friend, \
        BOOST_PP_EMPTY()) \
std::ostream& operator <<(std::ostream& stream, \
BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
    BOOST_PP_TUPLE_ELEM(1, typeName), \
    BOOST_PP_TUPLE_ELEM(0, typeName)) value) \
{ \
    switch(value) \
    { \
        BOOST_PP_SEQ_FOR_EACH(FO_DEF_ENUM_OSTREAM_CASE, \
            BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
                BOOST_PP_TUPLE_ELEM(1, typeName), \
                BOOST_PP_TUPLE_ELEM(0, typeName)), values) \
        default: \
            stream.setstate(std::ios::failbit); \
            break; \
    } \
 \
    return stream; \
}

/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_ISTREAM_CASE(r, data, elem) \
    else if( \
BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, elem)) == token) \
        value = data::BOOST_PP_TUPLE_ELEM(0, elem);

/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_ISTREAM(typeName, values) \
inline \
BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
        friend, \
        BOOST_PP_EMPTY()) \
std::istream& operator >>(std::istream& stream, \
BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
    BOOST_PP_TUPLE_ELEM(1, typeName), \
    BOOST_PP_TUPLE_ELEM(0, typeName))& value) \
{ \
    std::string token; \
 \
    stream >> token; \
 \
    if(false) \
        ; \
    BOOST_PP_SEQ_FOR_EACH(FO_DEF_ENUM_ISTREAM_CASE, \
        BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
            BOOST_PP_TUPLE_ELEM(1, typeName), \
            BOOST_PP_TUPLE_ELEM(0, typeName)), values) \
    else \
        stream.setstate(std::ios::failbit); \
 \
    return stream; \
}

/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_ADD_STRING_REPR(s, data, elem) \
    BOOST_PP_TUPLE_PUSH_BACK(elem, BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, elem)))

/** @brief Define an enumeration
 *
 * Defines an enumeration named @p typeName with the enumeration @p values in
 * the namespace where this macro is used. Also defines << and >> operators for
 * iostream usage.
 *
 * Use a tuple containing only the enumeration name when the enum is located
 * outside of a class as @p typeName.  Prepend the class name to the @p typeName
 * tuple when using it inside a class.
 *
 * Use a sequence of 1 to 2 element tuples as values.  The first tuple value
 * should contain the enum value symbol. The optional second tuple element should
 * contain the assigned enum element value.  If no second tuple value is given the
 * default enum element value assignment is used.
 *
 * When using the << operator the string representation of the enumeration is
 * written to the ostream.
 *
 * When using the >> operator and one string representation matches the input
 * value the output will be set to the associated enum value.  If the input
 * value doesn't match any string representation the istream failbit will be
 * set.
 *
 * Examples:
 *
 * Free enumeration:
 * @code
 * FO_ENUM(
 *     (Animal),
 *     ((CAT))
 *     ((DOG))
 *     ((COW, 5))
 * )
 * @endcode
 *
 * Class member enumeration:
 * @code
 * class AutomaticGearBox
 * {
 * public:
 *     FO_ENUM(
 *         (AutomaticGearBox, Gear),
 *         ((PARK,    -5))
 *         ((REVERSE, -1))
 *         ((NEUTRAL, 0))
 *         ((LOW))
 *         ((DRIVE))
 *         ((SPRINT))
 *     )
 * }
 * @endcode
 */
#define FO_ENUM(typeName, values) \
    FO_DEF_ENUM(typeName, BOOST_PP_SEQ_TRANSFORM(FO_DEF_ENUM_ADD_STRING_REPR, _, values)) \
    FO_DEF_ENUM_OSTREAM(typeName, BOOST_PP_SEQ_TRANSFORM(FO_DEF_ENUM_ADD_STRING_REPR, _, values)) \
    FO_DEF_ENUM_ISTREAM(typeName, BOOST_PP_SEQ_TRANSFORM(FO_DEF_ENUM_ADD_STRING_REPR, _, values))


#endif

