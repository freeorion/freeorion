#ifndef _ValueRef_h_
#define _ValueRef_h_

#include "ScriptingContext.h"
#include "../util/Export.h"

#include <string>

namespace ValueRef {

/** The base class for all ValueRef classes.  This class provides the public
  * interface for a ValueRef expression tree. */
template <typename T>
struct FO_COMMON_API ValueRef
{
    virtual ~ValueRef()
    {}

    virtual bool operator==(const ValueRef<T>& rhs) const;

    bool operator!=(const ValueRef<T>& rhs) const
    { return !(*this == rhs); }

    /** Evaluates the expression tree with an empty context.  Useful for
      * evaluating expressions that do not depend on context. */
    T Eval() const
    { return Eval(::ScriptingContext()); }

    /** Evaluates the expression tree and return the results; \a context
      * is used to fill in any instances of the "Value" variable or references
      * to objects such as the source, effect target, or condition candidates
      * that exist in the tree. */
    virtual T Eval(const ScriptingContext& context) const = 0;

    virtual bool RootCandidateInvariant() const
    { return false; }

    virtual bool LocalCandidateInvariant() const
    { return false; }

    virtual bool TargetInvariant() const
    { return false; }

    virtual bool SourceInvariant() const
    { return false; }

    virtual bool SimpleIncrement() const
    { return false; }

    virtual bool ConstantExpr() const
    { return false; }

    virtual std::string Description() const = 0;

    /** Returns a text description of this type of special. */
    virtual std::string Dump(unsigned short ntabs = 0) const = 0;

    virtual void SetTopLevelContent(const std::string& content_name)
    {}

    virtual unsigned int GetCheckSum() const
    { return 0; }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

enum StatisticType : int {
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

}

#endif // _ValueRef_h_
