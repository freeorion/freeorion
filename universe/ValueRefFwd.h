#ifndef _ValueRefFwd_h_
#define _ValueRefFwd_h_

/** This namespace contains ValueRefBase and its subclasses.  The ValueRefBase
  * subclasses represent expression trees that may be evaluated at various
  * times, and which refer to both constant and variable values. */
namespace ValueRef {
    enum ReferenceType {
        INVALID_REFERENCE_TYPE = -1,
        NON_OBJECT_REFERENCE,               // ValueRef::Variable is not evalulated on any specific object
        SOURCE_REFERENCE,                   // ValueRef::Variable is evaluated on the source object
        EFFECT_TARGET_REFERENCE,            // ValueRef::Variable is evaluated on the target object of an effect while it is being executed
        EFFECT_TARGET_VALUE_REFERENCE,      // ValueRef::Variable is evaluated on the target object value of an effect while it is being executed
        CONDITION_LOCAL_CANDIDATE_REFERENCE,// ValueRef::Variable is evaluated on an object that is a candidate to be matched by a condition.  In a subcondition, this will reference the local candidate, and not the candidate of an enclosing condition.
        CONDITION_ROOT_CANDIDATE_REFERENCE  // ValueRef::Variable is evaluated on an object that is a candidate to be matched by a condition.  In a subcondition, this will still reference the root candidate, and not the candidate of the local condition.
    };
    template <class T> struct ValueRefBase;
    template <class T> struct Constant;
    template <class T> struct Variable;
    template <class T> struct Statistic;
    template <class T> struct ComplexVariable;
    enum StatisticType {
        INVALID_STATISTIC_TYPE = -1,
        COUNT,  // returns the number of objects matching the condition
        UNIQUE_COUNT,   // returns the number of distinct property values of objects matching the condition. eg. if 3 objects have the property value "small", and two have "big", then this value is 2, as there are 2 unique property values.
        IF,     // returns T(1) if anything matches the condition, or T(0) otherwise
        SUM,    // returns the sum of the property values of all objects matching the condition
        MEAN,   // returns the mean of the property values of all objects matching the condition
        RMS,    // returns the sqrt of the mean of the squares of the property values of all objects matching the condition
        MODE,   // returns the most common property value of objects matching the condition.  supported for non-numeric types such as enums.
        MAX,    // returns the maximum value of the property amongst objects matching the condition
        MIN,    // returns the minimum value of the property amongst objects matching the condition
        SPREAD, // returns the (positive) difference between the maximum and minimum values of the property amongst objects matching the condition
        STDEV,  // returns the standard deviation of the property values of all objects matching the condition
        PRODUCT // returns the product of the property values of all objects matching the condition
    };
    template <class FromType, class ToType> struct StaticCast;
    template <class FromType> struct StringCast;
    template <class FromType> struct UserStringLookup;
    struct NameLookup;
    template <class T> struct Operation;
    enum OpType {
        PLUS,
        MINUS,
        TIMES,
        DIVIDE,
        NEGATE,
        EXPONENTIATE,
        ABS,
        LOGARITHM,
        SINE,
        COSINE,
        MINIMUM,
        MAXIMUM,
        RANDOM_UNIFORM,
        RANDOM_PICK,
        SUBSTITUTION,
        COMPARE_EQUAL,
        COMPARE_GREATER_THAN,
        COMPARE_GREATER_THAN_OR_EQUAL,
        COMPARE_LESS_THAN,
        COMPARE_LESS_THAN_OR_EQUAL,
        COMPARE_NOT_EQUAL,
        ROUND_NEAREST,
        ROUND_UP,
        ROUND_DOWN
    };
}

#endif // _ValueRefFwd_h_
