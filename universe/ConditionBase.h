#ifndef _ConditionBase_h_
#define _ConditionBase_h_

#include "../util/Export.h"

#include <boost/serialization/access.hpp>

#include <memory>
#include <string>
#include <vector>

class UniverseObject;
struct ScriptingContext;

namespace Condition {

typedef std::vector<std::shared_ptr<const UniverseObject>> ObjectSet;

enum Invariance {
    UNKNOWN_INVARIANCE, ///< This condition hasn't yet calculated this invariance type
    INVARIANT,          ///< This condition is invariant to a particular type of object change
    VARIANT             ///< This condition's result depends on the state of a particular object
};

enum SearchDomain {
    NON_MATCHES,    ///< The Condition will only examine items in the non matches set; those that match the Condition will be inserted into the matches set.
    MATCHES         ///< The Condition will only examine items in the matches set; those that do not match the Condition will be inserted into the nonmatches set.
};

/** The base class for all Conditions. */
struct FO_COMMON_API ConditionBase {
    ConditionBase() {}
    virtual ~ConditionBase();

    virtual bool operator==(const ConditionBase& rhs) const;
    bool operator!=(const ConditionBase& rhs) const
    { return !(*this == rhs); }

    virtual void Eval(const ScriptingContext& parent_context,
                      ObjectSet& matches,
                      ObjectSet& non_matches,
                      SearchDomain search_domain = NON_MATCHES) const;

    /** Tests all objects in universe as NON_MATCHES. */
    void Eval(const ScriptingContext& parent_context,
              ObjectSet& matches) const;

    /** Tests single candidate object, returning true iff it matches condition. */
    bool Eval(const ScriptingContext& parent_context,
              std::shared_ptr<const UniverseObject> candidate) const;

    /** Tests single candidate object, returning true iff it matches condition
      * with empty ScriptingContext. If this condition is not invariant to */
    bool Eval(std::shared_ptr<const UniverseObject> candidate) const;

    virtual void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                   ObjectSet& condition_non_targets) const;

    /** Returns true iff this condition's evaluation does not reference
      * the RootCandidate objects.  This requirement ensures that if this
      * condition is a subcondition to another Condition or a ValueRef, this
      * condition may be evaluated once and its result used to match all local
      * candidates to that condition. */
    virtual bool RootCandidateInvariant() const
    { return false; }

    /** (Almost) all conditions are varying with local candidates; this is the
      * point of evaluating a condition.  This funciton is provided for
      * consistency with ValueRef, which may not depend on the local candidiate
      * of an enclosing condition. */
    bool LocalCandidateInvariant() const
    { return false; }

    /** Returns true iff this condition's evaluation does not reference the
      * target object.*/
    virtual bool TargetInvariant() const
    { return false; }

    /** Returns true iff this condition's evaluation does not reference the
      * source object.*/
    virtual bool SourceInvariant() const
    { return false; }

    virtual std::string Description(bool negated = false) const = 0;
    virtual std::string Dump(unsigned short ntabs = 0) const = 0;
    virtual void SetTopLevelContent(const std::string& content_name) = 0;
    virtual unsigned int GetCheckSum() const
    { return 0; }

protected:
    mutable Invariance m_root_candidate_invariant = UNKNOWN_INVARIANCE;
    mutable Invariance m_target_invariant = UNKNOWN_INVARIANCE;
    mutable Invariance m_source_invariant = UNKNOWN_INVARIANCE;

private:
    struct MatchHelper;
    friend struct MatchHelper;

    virtual bool Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

}

#endif // _ConditionBase_h_
