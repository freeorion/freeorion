#ifndef _Condition_h_
#define _Condition_h_

#include "../util/Export.h"

#include <boost/serialization/access.hpp>

#include <memory>
#include <string>
#include <vector>

class UniverseObject;
struct ScriptingContext;

namespace Effect {
    typedef std::vector<std::shared_ptr<UniverseObject>> TargetSet;
}

namespace Condition {

typedef std::vector<std::shared_ptr<const UniverseObject>> ObjectSet;

enum SearchDomain : int {
    NON_MATCHES,    ///< The Condition will only examine items in the non matches set; those that match the Condition will be inserted into the matches set.
    MATCHES         ///< The Condition will only examine items in the matches set; those that do not match the Condition will be inserted into the nonmatches set.
};

/** The base class for all Conditions. */
struct FO_COMMON_API Condition {
    Condition() {}
    virtual ~Condition();

    virtual bool operator==(const Condition& rhs) const;
    bool operator!=(const Condition& rhs) const
    { return !(*this == rhs); }

    virtual void Eval(const ScriptingContext& parent_context,
                      ObjectSet& matches,
                      ObjectSet& non_matches,
                      SearchDomain search_domain = NON_MATCHES) const;

    void Eval(const ScriptingContext& parent_context,
              Effect::TargetSet& matches,
              Effect::TargetSet& non_matches,
              SearchDomain search_domain = NON_MATCHES) const;

    /** Tests all objects in universe as NON_MATCHES. */
    void Eval(const ScriptingContext& parent_context,
              ObjectSet& matches) const;

    /** Tests all objects in universe as NON_MATCHES. */
    void Eval(const ScriptingContext& parent_context,
              Effect::TargetSet& matches) const;

    /** Tests single candidate object, returning true iff it matches condition. */
    bool Eval(const ScriptingContext& parent_context,
              std::shared_ptr<const UniverseObject> candidate) const;

    /** Tests single candidate object, returning true iff it matches condition
      * with empty ScriptingContext. If this condition is not invariant to */
    bool Eval(std::shared_ptr<const UniverseObject> candidate) const;

    virtual void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                   ObjectSet& condition_non_targets) const;

    //! Returns true iff this condition's evaluation does not reference
    //! the RootCandidate objects.  This requirement ensures that if this
    //! condition is a subcondition to another Condition or a ValueRef, this
    //! condition may be evaluated once and its result used to match all local
    //! candidates to that condition.
    bool RootCandidateInvariant() const
    { return m_root_candidate_invariant; }

    //! (Almost) all conditions are varying with local candidates; this is the
    //! point of evaluating a condition.  This funciton is provided for
    //! consistency with ValueRef, which may not depend on the local candidiate
    //! of an enclosing condition.
    bool LocalCandidateInvariant() const
    { return false; }

    //! Returns true iff this condition's evaluation does not reference the
    //! target object.
    bool TargetInvariant() const
    { return m_target_invariant; }

    //! Returns true iff this condition's evaluation does not reference the
    //! source object.
    bool SourceInvariant() const
    { return m_source_invariant; }

    virtual std::string Description(bool negated = false) const = 0;
    virtual std::string Dump(unsigned short ntabs = 0) const = 0;
    virtual void SetTopLevelContent(const std::string& content_name) = 0;
    virtual unsigned int GetCheckSum() const
    { return 0; }

protected:
    bool m_root_candidate_invariant = false;
    bool m_target_invariant = false;
    bool m_source_invariant = false;

private:
    struct MatchHelper;
    friend struct MatchHelper;

    virtual bool Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

}

#endif // _Condition_h_
