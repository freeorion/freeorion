#ifndef _ValueRef_h_
#define _ValueRef_h_


#include "ScriptingContext.h"
#include "../util/Export.h"


namespace ValueRef {

//! The common base class for all ValueRef classes. This class provides
//! some the return-type-independent interface.
struct FO_COMMON_API ValueRefBase {
    ValueRefBase() = default;
    virtual ~ValueRefBase() = default;

    bool RootCandidateInvariant() const  { return m_root_candidate_invariant; }
    bool LocalCandidateInvariant() const { return m_local_candidate_invariant; }
    bool TargetInvariant() const         { return m_target_invariant; }
    bool SourceInvariant() const         { return m_source_invariant; }
    virtual bool SimpleIncrement() const { return false; }
    virtual bool ConstantExpr() const    { return false; }

    virtual std::string Description() const = 0;                    //! Returns a user-readable text description of this ValueRef
    virtual std::string Dump(unsigned short ntabs = 0) const = 0;   //! Returns a textual representation that should be parseable to recreate this ValueRef

    virtual void SetTopLevelContent(const std::string& content_name) {}

    virtual unsigned int GetCheckSum() const { return 0; }

protected:
    bool m_root_candidate_invariant = false;
    bool m_local_candidate_invariant = false;
    bool m_target_invariant = false;
    bool m_source_invariant = false;
};

//! The base class for all ValueRef classes returning type T. This class
//! provides the public interface for a ValueRef expression tree.
template <typename T>
struct FO_COMMON_API ValueRef : public ValueRefBase
{
    ValueRef() = default;
    virtual ~ValueRef() = default;

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
};

enum StatisticType : int {
    INVALID_STATISTIC_TYPE = -1,
    IF,     // returns T(1) if anything matches the condition, or T(0) otherwise

    COUNT,          // returns the number of objects matching the condition
    UNIQUE_COUNT,   // returns the number of distinct property values of objects matching the condition. eg. if 3 objects have the property value "small", and two have "big", then this value is 2, as there are 2 unique property values.
    HISTO_MAX,      // returns the maximum number of times a unique property value appears in the property values. eg. if 5 objects have values 1, 2, 3, 2, 2, then there are 3x2, 1x1, and 1x3, and the histo max is 3.
    HISTO_MIN,      // returns the maximum number of times a unique property value appears in the property values. eg. if 5 objects have values 1, 2, 3, 2, 2, then there are 3x2, 1x1, and 1x3, and the histo min is 1.
    HISTO_SPREAD,   // returns the (positive) difference between the maximum and minimum numbers of distinct property values. eg. if 5 objects have values, 1, 2, 3, 2, 2, then there are 3x2, 1x1, and 1x3, and the Histo Spread is 3-1 = 2.

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


#endif
