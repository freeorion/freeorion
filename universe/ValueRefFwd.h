// -*- C++ -*-
#ifndef _ValueRefFwd_h_
#define _ValueRefFwd_h_

/** This namespace contains ValueRefBase and its subclasses.  The ValueRefBase
  * subclasses represent expression trees that may be evaluated at various
  * times, and which refer to both constant and variable values. */
namespace ValueRef {
    template <class T> struct ValueRefBase;
    template <class T> struct Constant;
    template <class T> struct Variable;
    template <class T> struct Statistic;
    enum StatisticType {
        NUMBER, // returns the number of objects matching the condition
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
    template <class T> struct Operation;
    enum OpType {
        PLUS,
        MINUS,
        TIMES,
        DIVIDES,
        NEGATE
    };
    template <class T> bool ConstantExpr(const ValueRefBase<T>* expr);
}

#endif // _ValueRefFwd_h_
