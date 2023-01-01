#ifndef _Condition_h_
#define _Condition_h_


#include <memory>
#include <string>
#include <vector>
#include "../util/Export.h"


class UniverseObject;
struct ScriptingContext;

namespace Effect {
    using TargetSet = std::vector<UniverseObject*>;
}

namespace Condition {

using ObjectSet = std::vector<const UniverseObject*>;

enum class SearchDomain : uint8_t {
    NON_MATCHES,    ///< The Condition will only examine items in the non matches set; those that match the Condition will be inserted into the matches set.
    MATCHES         ///< The Condition will only examine items in the matches set; those that do not match the Condition will be inserted into the nonmatches set.
};

/** The base class for all Conditions. */
struct FO_COMMON_API Condition {
    virtual ~Condition() = default;

    virtual bool operator==(const Condition& rhs) const;
    bool operator!=(const Condition& rhs) const
    { return !(*this == rhs); }

    virtual void Eval(const ScriptingContext& parent_context,
                      ObjectSet& matches, ObjectSet& non_matches,
                      SearchDomain search_domain = SearchDomain::NON_MATCHES) const;

    /** Returns true iff at least one object in \a candidates matches this condition.
      * Returns false for an empty candiates list. */
    virtual bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const;

    /** Overload for mutable TargetSet input/output. */
    void Eval(ScriptingContext& parent_context,
              Effect::TargetSet& matches, Effect::TargetSet& non_matches,
              SearchDomain search_domain = SearchDomain::NON_MATCHES) const;

    /** Tests all objects in \a parent_context ObjectMap and returns those that match. */
    [[nodiscard]] ObjectSet Eval(const ScriptingContext& parent_context) const;
    [[nodiscard]] Effect::TargetSet Eval(ScriptingContext& parent_context) const;

    /** Tests single candidate object, returning true iff it matches condition. */
    [[nodiscard]] bool Eval(const ScriptingContext& parent_context, const UniverseObject* candidate) const;

    /** Initializes \a condition_non_targets with a set of objects that could
      * match this condition, without checking if they all actually do. */
    virtual void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                   ObjectSet& condition_non_targets) const;

    /** Derived Condition classes can override this to true if all objects returned
      * by GetDefaultInitialCandidateObject() are guaranteed to also match this
      * condition. */
    [[nodiscard]] bool InitialCandidatesAllMatch() const noexcept { return m_initial_candidates_all_match; }

    //! Returns true iff this condition's evaluation does not reference
    //! the RootCandidate objects.  This requirement ensures that if this
    //! condition is a subcondition to another Condition or a ValueRef, this
    //! condition may be evaluated once and its result used to match all local
    //! candidates to that condition.
    [[nodiscard]] bool RootCandidateInvariant() const noexcept { return m_root_candidate_invariant; }

    //! (Almost) all conditions are varying with local candidates; this is the
    //! point of evaluating a condition.  This funciton is provided for
    //! consistency with ValueRef, which may not depend on the local candidiate
    //! of an enclosing condition.
    [[nodiscard]] bool LocalCandidateInvariant() const noexcept { return false; }

    //! Returns true iff this condition's evaluation does not reference the
    //! target object.
    [[nodiscard]] bool TargetInvariant() const noexcept { return m_target_invariant; }

    //! Returns true iff this condition's evaluation does not reference the
    //! source object.
    [[nodiscard]] bool SourceInvariant() const noexcept { return m_source_invariant; }

    [[nodiscard]] virtual std::string Description(bool negated = false) const { return ""; }
    [[nodiscard]] virtual std::string Dump(uint8_t ntabs = 0) const { return ""; }

    virtual void SetTopLevelContent(const std::string& content_name) {}
    [[nodiscard]] virtual uint32_t GetCheckSum() const { return 0; }

    //! Makes a clone of this Condition in a new owning pointer. Required for
    //! Boost.Python, which doesn't support move semantics for returned values.
    [[nodiscard]] virtual std::unique_ptr<Condition> Clone() const = 0;

protected:
    Condition() = default;
    Condition(bool root_invariant, bool target_invariant, bool source_invariant, bool init_all_match) noexcept :
        m_root_candidate_invariant(root_invariant),
        m_target_invariant(target_invariant),
        m_source_invariant(source_invariant),
        m_initial_candidates_all_match(init_all_match)
    {}
    Condition(bool root_invariant, bool target_invariant, bool source_invariant) noexcept :
        m_root_candidate_invariant(root_invariant),
        m_target_invariant(target_invariant),
        m_source_invariant(source_invariant)
    {}
    //! Copies invariants from other Condition
    Condition(const Condition& rhs) = default;
    Condition(Condition&& rhs) = delete;
    Condition& operator=(const Condition& rhs) = delete;
    Condition& operator=(Condition&& rhs) = delete;

    bool m_root_candidate_invariant = false;
    bool m_target_invariant = false;
    bool m_source_invariant = false;
    bool m_initial_candidates_all_match = false;

private:
    virtual bool Match(const ScriptingContext& local_context) const { return false; }
};

}


#endif
