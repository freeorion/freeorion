#ifndef _Enum_h_
#define _Enum_h_

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>

#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/push_back.hpp>
#include <boost/preprocessor/tuple/size.hpp>


/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_VALUE_IMPL(elemName, haveValue, maybeValue) \
    BOOST_PP_IF(haveValue, elemName = maybeValue, elemName) ,

#define FO_DEF_ENUM_VALUE(s, _, nameStringValue) \
    FO_DEF_ENUM_VALUE_IMPL( \
        BOOST_PP_TUPLE_ELEM(0, nameStringValue), \
        BOOST_PP_TUPLE_ELEM(2, nameStringValue), \
        BOOST_PP_TUPLE_ELEM(3, nameStringValue) \
    )

/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM(enumName, underlyingType, namesStringsValues) \
enum class enumName : underlyingType { \
    BOOST_PP_SEQ_FOR_EACH(FO_DEF_ENUM_VALUE, _, namesStringsValues) \
};


/** @brief Implementation detail for FO_ENUM */
#define FO_DEF_ENUM_TOSTRING_CASE_IMPL(enumName, elemName, elemNameString) \
    case enumName :: elemName : return elemNameString; break;

#define FO_DEF_ENUM_TOSTRING_CASE(s, enumName, nameStringValue) \
    FO_DEF_ENUM_TOSTRING_CASE_IMPL( \
        enumName, \
        BOOST_PP_TUPLE_ELEM(0, nameStringValue), \
        BOOST_PP_TUPLE_ELEM(1, nameStringValue) \
    )

/** Defines to_string(enumName) and ostream& operator<<(ostream&, enumName) */
#define FO_DEF_ENUM_TOSTRING(enumName, tupleSize, namesStringsValues) \
constexpr BOOST_PP_IF(BOOST_PP_EQUAL(tupleSize, 2), friend, BOOST_PP_EMPTY()) \
std::string_view to_string(enumName value) noexcept { \
    switch(value) \
    { \
        BOOST_PP_SEQ_FOR_EACH(FO_DEF_ENUM_TOSTRING_CASE, enumName, namesStringsValues) \
        default: return ""; break; \
    } \
} \
inline \
BOOST_PP_IF(BOOST_PP_EQUAL(tupleSize, 2), friend, BOOST_PP_EMPTY()) \
std::ostream& operator <<(std::ostream& stream, enumName value) \
{ stream << to_string(value); return stream; }


/** Assembles one initializer list pair of {enumName::elem, "elem"} */
#define FO_DEF_ENUM_ITERATE_ELEM(enumName, elemName, elemNameString) \
        { enumName :: elemName, elemNameString },

/** @brief Implementation detail for FO_ENUM.
  * elem may be a single enumeration value (enum_value) or may be
  * an enumeration value and underly type representation (enum_value = N).
  * This extracts just the enumeration value and passes that along. */
#define FO_DEF_ENUM_ITERATE_VALUE(_, enumName, nameStringValue) \
    FO_DEF_ENUM_ITERATE_ELEM( \
        enumName, \
        BOOST_PP_TUPLE_ELEM(0, nameStringValue), \
        BOOST_PP_TUPLE_ELEM(1, nameStringValue) \
    )

/** Assemables an array of pair<enumName, std::string_vew> where the
  * string_view are the text representations of the enumName.
  * Also defines a getter for that array as enumNameValues(). */
#define FO_DEF_ENUM_ITERATE(enumName, tupleSize, namesStringsValues) \
struct BOOST_PP_CAT(enumName, Impl) { \
    static constexpr std::size_t values_count = BOOST_PP_SEQ_SIZE(namesStringsValues); \
    using values_array_t = std::array<std::pair<enumName, std::string_view>, values_count>; \
    static constexpr values_array_t vals {{ \
    BOOST_PP_SEQ_FOR_EACH(FO_DEF_ENUM_ITERATE_VALUE, enumName, namesStringsValues) \
    }}; \
}; \
inline \
BOOST_PP_IF(BOOST_PP_EQUAL(tupleSize, 2), static, BOOST_PP_EMPTY()) \
constexpr auto& BOOST_PP_CAT(enumName, Values)() noexcept \
{ return BOOST_PP_CAT(enumName, Impl) :: vals; }


/** Defines enumName enumNameFromString(string_view sv, enumName not_found_result)
  * Return enumName::value that has string representation matching sv or,
  * if no such string representation exists, returns not_found_result. 
  * Also defines stream operator>> for enumName. */
#define FO_DEF_ENUM_FROM_STRING(enumName, tupleSize) \
constexpr auto BOOST_PP_CAT(enumName, FromString)( \
    std::string_view sv, enumName not_found_result = enumName(0)) noexcept { \
    const auto& vals = BOOST_PP_CAT(enumName, Values)(); \
    for (const auto& [val, val_sv] : vals) { if (val_sv == sv) return val; } \
    return not_found_result; \
} \
inline BOOST_PP_IF(BOOST_PP_EQUAL(tupleSize, 2), friend, BOOST_PP_EMPTY()) \
std::istream& operator >>(std::istream& stream, enumName & value) { \
    std::string token; \
    stream >> token; \
    const auto& vals = BOOST_PP_CAT(enumName, Values)(); \
    for (const auto& [val, val_sv] : vals) { if (val_sv == token) { value = val; return stream; } } \
    stream.setstate(std::ios::failbit); \
    return stream; \
}


/** @brief Implementation detail for FO_ENUM.
  * tupleSize is expected to be 2 for a non-class enum and 3 for an enum defined in a class. */
#define FO_DEF_ENUM_IMPL_IMPL(enumName, underlyingType, tupleSize, namesStringsValues) \
    FO_DEF_ENUM(enumName, underlyingType, namesStringsValues) \
    FO_DEF_ENUM_TOSTRING(enumName, tupleSize, namesStringsValues) \
    FO_DEF_ENUM_ITERATE(enumName, tupleSize, namesStringsValues) \
    FO_DEF_ENUM_FROM_STRING(enumName, tupleSize)


/** Combines into a 4-tuple: 
  * ( element name, string representation of element name, is there a value?, the value or a default value )*/
#define FO_DEF_ENUM_TX_VALUE_TUPLE(elemName, haveValue, maybeValue) \
    ( elemName, BOOST_PP_STRINGIZE(elemName), haveValue, maybeValue ) 

/** passes along element name and whether there is a value, and
  * passes either the value or () */
#define FO_DEF_ENUM_TX_VALUE_IMPL(elemName, haveValue, elem) \
    FO_DEF_ENUM_TX_VALUE_TUPLE( \
        elemName, \
        haveValue, \
        BOOST_PP_IF(haveValue, BOOST_PP_TUPLE_ELEM(1, elem), 0) \
    )

/** Extracts first sub-element (enum value name).
  * Checks whether there is more than one sub-element. If so, passes 1, and if not, passes 0, as a flag.
  * Also passes the input element. */
#define FO_DEF_ENUM_TX_VALUE(elem) \
    FO_DEF_ENUM_TX_VALUE_IMPL( \
        BOOST_PP_TUPLE_ELEM(0, elem), \
        BOOST_PP_IF(BOOST_PP_GREATER(BOOST_PP_TUPLE_SIZE(elem), 1), 1, 0), \
        elem \
    )

#define FO_DEF_ENUM_TX_VALUE_WRAP(s, _, elem) \
    FO_DEF_ENUM_TX_VALUE(elem)

#define FO_DEF_ENUM_TX_VALUES(values) \
    BOOST_PP_SEQ_TRANSFORM(FO_DEF_ENUM_TX_VALUE_WRAP, _, values)

/** @brief Extracts EnumName from (EnumName) or (ClassName, EnumName) */
#define FO_ENUM_NAME_FROM_TYPENAME(typeName) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(typeName), 2), \
                BOOST_PP_TUPLE_ELEM(1, typeName), \
                BOOST_PP_TUPLE_ELEM(0, typeName))

/** @brief Implementation detail for FO_ENUM. */
#define FO_DEF_ENUM_IMPL(typeName, underlyingType, values) \
    FO_DEF_ENUM_IMPL_IMPL( \
        FO_ENUM_NAME_FROM_TYPENAME(typeName), \
        underlyingType, \
        BOOST_PP_TUPLE_SIZE(typeName), \
        FO_DEF_ENUM_TX_VALUES(values) \
    )

/** @brief Define an enumeration
 *
 * Defines an enumeration named @p typeName with the enumeration @p values in
 * the namespace where this macro is used. Also defines << and >> operators for
 * iostream usage. Underlying type of the enum is int8_t.
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
 * Iterate over values:
 * @code
 * for (const auto& [val, string_view] : AnimalValues()) {
 *    ...
 * }
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
 *
 * Iterate over values:
 * @code
 * for (const auto& [val, string_view] : AutomaticGearBox::AnimalValues())) {
 *    ...
 * }
 * @endcode
 */
#define FO_ENUM(typeName, values) \
    FO_DEF_ENUM_IMPL(typeName, int8_t, values)

/* Defines an enum as above, except the underlying type is int16_t */
#define FO_ENUM_BIG(typeName, values) \
    FO_DEF_ENUM_IMPL(typeName, int16_t, values)

#endif