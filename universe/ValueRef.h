#ifndef _ValueRef_h_
#define _ValueRef_h_

#include "ScriptingContext.h"
#include "../util/Export.h"

#include <string>

namespace ValueRef {

/** The base class for all ValueRef classes.  This class provides the public
  * interface for a ValueRef expression tree. */
template <class T>
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
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

}

#endif // _ValueRef_h_
