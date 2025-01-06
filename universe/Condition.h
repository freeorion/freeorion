#ifndef _Condition_h_
#define _Condition_h_

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include "../util/Export.h"


class UniverseObject;
struct ScriptingContext;

namespace Effect {
    using TargetSet = std::vector<UniverseObject*>;
}

namespace Condition {

using ObjectSet = std::vector<const UniverseObject*>;

enum class SearchDomain : bool {
    NON_MATCHES,    ///< The Condition will only examine items in the non matches set; those that match the Condition will be inserted into the matches set.
    MATCHES         ///< The Condition will only examine items in the matches set; those that do not match the Condition will be inserted into the nonmatches set.
};

/** The base class for all Conditions. */
struct FO_COMMON_API Condition {
    constexpr Condition(const Condition&) noexcept = default;
    constexpr Condition(Condition&&) noexcept = default;
    constexpr virtual ~Condition()
#if defined(__GNUC__) && (__GNUC__ < 13)
    {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#else
        = default;
#endif

    [[nodiscard]] virtual bool operator==(const Condition& rhs) const;

    /** Moves object pointers from \a matches or \a non_matches (from whichever
     * is specified in \a search_domain) to the other, if each belongs in the
     * other, as determined by this condition. If searching in matches, then
     * all objects in matches that are not matched by this condition are moved
     * into non_matches. Initial contents of the not-searched container are not
     * modified, but objects from the searched container may be appended to the
     * not-searched container. */
    virtual void Eval(const ScriptingContext& parent_context,
                      ObjectSet& matches, ObjectSet& non_matches,
                      SearchDomain search_domain = SearchDomain::NON_MATCHES) const;

    /** Returns true iff at least one object in \a candidates matches this condition.
      * Returns false for an empty candiates list. */
    [[nodiscard]] virtual bool EvalAny(const ScriptingContext& parent_context,
                                       const ObjectSet& candidates) const;
    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context) const {
        ObjectSet candidates = GetDefaultInitialCandidateObjects(parent_context);

        if (InitialCandidatesAllMatch())
            return !candidates.empty(); // don't need to evaluate condition further

        return EvalAny(parent_context, candidates);
    }

    /** Overload for mutable TargetSet input/output. */
    void Eval(ScriptingContext& parent_context,
              Effect::TargetSet& matches, Effect::TargetSet& non_matches,
              SearchDomain search_domain = SearchDomain::NON_MATCHES) const;

    /** Tests all objects in \a parent_context ObjectMap and returns those that match. */
    [[nodiscard]] ObjectSet Eval(const ScriptingContext& parent_context) const;
    [[nodiscard]] Effect::TargetSet Eval(ScriptingContext& parent_context) const;

    /** Tests single candidate object, returning true iff it matches condition. */
    [[nodiscard]] virtual bool EvalOne(const ScriptingContext& parent_context,
                                       const UniverseObject* candidate) const
    {
        if (!candidate)
            return false;
        ObjectSet non_matches{candidate}, matches;
        Eval(parent_context, matches, non_matches);
        return non_matches.empty(); // if candidate has been matched, non_matches will now be empty
    }

    /** Initializes \a condition_non_targets with a set of objects that could
      * match this condition, without checking if they all actually do. */
    [[nodiscard]] virtual ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const;

    /** Derived Condition classes can override this to true if all objects returned
      * by GetDefaultInitialCandidateObject() are guaranteed to also match this
      * condition. */
    [[nodiscard]] constexpr bool InitialCandidatesAllMatch() const noexcept { return m_initial_candidates_all_match; }

    //! Returns true iff this condition's evaluation does not reference
    //! the RootCandidate objects.  This requirement ensures that if this
    //! condition is a subcondition to another Condition or a ValueRef, this
    //! condition may be evaluated once and its result used to match all local
    //! candidates to that condition.
    [[nodiscard]] constexpr bool RootCandidateInvariant() const noexcept { return m_root_candidate_invariant; }

    //! (Almost) all conditions are varying with local candidates; this is the
    //! point of evaluating a condition.  This funciton is provided for
    //! consistency with ValueRef, which may not depend on the local candidiate
    //! of an enclosing condition.
    [[nodiscard]] constexpr bool LocalCandidateInvariant() const noexcept { return false; }

    //! Returns true iff this condition's evaluation does not reference the
    //! target object.
    [[nodiscard]] constexpr bool TargetInvariant() const noexcept { return m_target_invariant; }

    //! Returns true iff this condition's evaluation does not reference the
    //! source object.
    [[nodiscard]] constexpr bool SourceInvariant() const noexcept { return m_source_invariant; }

    [[nodiscard]] virtual std::string Description(bool negated = false) const { return ""; } // TODO: pass in ScriptingContext
    [[nodiscard]] virtual std::string Dump(uint8_t ntabs = 0) const { return ""; }

    virtual void SetTopLevelContent(const std::string& content_name) {}
    [[nodiscard]] virtual uint32_t GetCheckSum() const { return 0; }

    //! Makes a clone of this Condition in a new owning pointer. Required for
    //! Boost.Python, which doesn't support move semantics for returned values.
    [[nodiscard]] virtual std::unique_ptr<Condition> Clone() const = 0;

protected:
    constexpr Condition() = default;
    constexpr Condition(bool root_invariant, bool target_invariant,
                        bool source_invariant, bool init_all_match) noexcept :
        m_root_candidate_invariant(root_invariant),
        m_target_invariant(target_invariant),
        m_source_invariant(source_invariant),
        m_initial_candidates_all_match(init_all_match)
    {}
    constexpr Condition(bool root_invariant, bool target_invariant, bool source_invariant) noexcept :
        m_root_candidate_invariant(root_invariant),
        m_target_invariant(target_invariant),
        m_source_invariant(source_invariant)
    {}
    constexpr Condition(std::array<bool, 3> rts_invariants) noexcept :
        Condition(rts_invariants[0], rts_invariants[1], rts_invariants[2])
    {}
    constexpr Condition(std::array<bool, 3> rts_invariants, bool init_all_match) noexcept :
        Condition(rts_invariants[0], rts_invariants[1], rts_invariants[2], init_all_match)
    {}
    Condition& operator=(const Condition&) = delete;
    Condition& operator=(Condition&&) = delete;

    bool m_root_candidate_invariant = false; // TODO: make these const once all derived classes initialize them in their constructors
    bool m_target_invariant = false;
    bool m_source_invariant = false;
    bool m_initial_candidates_all_match = false;

private:
    virtual bool Match(const ScriptingContext& local_context) const { throw std::runtime_error("default Condition::Match called... Override missing?"); }
};

}


#endif
