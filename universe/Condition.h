#ifndef _Condition_h_
#define _Condition_h_

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include "ScriptingContext.h"
#include "../util/Export.h"


class UniverseObject;

namespace Effect {
    using TargetSet = std::vector<UniverseObject*>;
}

namespace Condition {

using ObjectSet = std::vector<const UniverseObjectCXBase*>;

enum class SearchDomain : bool {
    NON_MATCHES,    ///< The Condition will only examine items in the non matches set; those that match the Condition will be inserted into the matches set.
    MATCHES         ///< The Condition will only examine items in the matches set; those that do not match the Condition will be inserted into the nonmatches set.
};

constexpr auto DoPartition(auto& candidates, const auto& pred)
    requires requires { pred(*candidates.begin()); candidates.end(); }
{
    if (!std::is_constant_evaluated()) {
        return std::stable_partition(candidates.begin(), candidates.end(), pred);
    } else {
#if defined(__cpp_lib_constexpr_algorithms)
#  if (__cpp_lib_constexpr_algorithms >= 202306L)
        return std::stable_partition(candidates.begin(), candidates.end(), pred);
#  elif (__cpp_lib_constexpr_algorithms >= 201806L)
        return std::partition(candidates.begin(), candidates.end(), pred);
#  endif
#else
        throw std::runtime_error("no constexpr EvalImpl possible");
#endif
    }
}

/** Used by 4-parameter Condition::Eval function, and some of its overrides,
  * to scan through \a matches or \a non_matches set and apply \a pred to
  * each object, to test if it should remain in its current set or be
  * transferred from the \a search_domain specified set into the other. */
constexpr void EvalImpl(auto& matches, auto& non_matches,
                        ::Condition::SearchDomain search_domain, const auto& pred)
    requires requires { non_matches.insert(non_matches.end(), DoPartition(matches, pred), matches.end()); }
{
    if (std::addressof(matches) == std::addressof(non_matches))
        return;
    const bool domain_matches = search_domain == ::Condition::SearchDomain::MATCHES;
    auto& from_set = domain_matches ? matches : non_matches;
    auto& to_set = domain_matches ? non_matches : matches;

    // checking for from_set.size() == 1 and/or to_set.empty() and early exiting didn't seem to speed up evaluation in general case
    const auto pred_for_search_domain = [&pred, domain_matches](const auto* o) { return pred(o) == domain_matches; };

    const auto part_it = DoPartition(from_set, pred_for_search_domain);
    to_set.insert(to_set.end(), part_it, from_set.end());
    from_set.erase(part_it, from_set.end());
}

namespace Impl {
    // flags for indicating possible objects or object types of initial candidates of conditions
    namespace MatchesType {
        constexpr uint16_t NOTHING =       0u;
        constexpr uint16_t SOURCE =        1u << 0u;
        constexpr uint16_t TARGET =        1u << 1u;
        constexpr uint16_t ROOTCANDIDATE = 1u << 2u;
        constexpr uint16_t PLANETS  =      1u << 3u;
        constexpr uint16_t BUILDINGS =     1u << 4u;
        constexpr uint16_t FLEETS =        1u << 5u;
        constexpr uint16_t SHIPS =         1u << 6u;
        constexpr uint16_t SYSTEMS =       1u << 7u;
        constexpr uint16_t FIELDS =        1u << 8u;

        constexpr uint16_t SINGLEOBJECT = SOURCE | TARGET | ROOTCANDIDATE;

        constexpr uint16_t PLANETS_BUILDINGS = PLANETS | BUILDINGS;
        constexpr uint16_t PLANETS_FLEETS =    PLANETS | FLEETS;
        constexpr uint16_t PLANETS_SHIPS =     PLANETS | SHIPS;
        constexpr uint16_t PLANETS_SYSTEMS =   PLANETS | SYSTEMS;
        constexpr uint16_t BUILDINGS_SHIPS =   BUILDINGS | SHIPS;
        constexpr uint16_t FLEETS_SHIPS =      FLEETS | SHIPS;
        constexpr uint16_t FLEETS_SYSTEMS =    FLEETS | SYSTEMS;

        constexpr uint16_t PLANETS_FLEETS_SYSTEMS =                PLANETS | FLEETS | SYSTEMS;
        constexpr uint16_t PLANETS_BUILDINGS_FLEETS_SHIPS =        PLANETS | BUILDINGS | FLEETS | SHIPS;
        constexpr uint16_t PLANETS_BUILDINGS_FLEETS_SHIPS_FIELDS = PLANETS | BUILDINGS | FLEETS | SHIPS | FIELDS;

        constexpr uint16_t ANYOBJECTTYPE = PLANETS | BUILDINGS | FLEETS | SHIPS | SYSTEMS | FIELDS;
    }
}

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
                      SearchDomain search_domain = SearchDomain::NON_MATCHES) const
    {
        const auto match_one_candidate = [cond{this}, &parent_context](const auto* candidate) -> bool {
            static constexpr ScriptingContext::LocalCandidate lc;
            const ScriptingContext candidate_context{parent_context, lc, candidate};
            return cond->Match(candidate_context); // this requies a derived Condition class to override either this Eval or Match
        };

        EvalImpl(matches, non_matches, search_domain, match_one_candidate);
    }

    /** Returns true iff at least one object in \a candidates matches this condition.
      * Returns false for an empty candiates list. */
    [[nodiscard]] virtual bool EvalAny(const ScriptingContext& parent_context,
                                       std::span<const UniverseObjectCXBase*> candidates) const;
    [[nodiscard]] virtual bool EvalAny(const ScriptingContext& parent_context,
                                       std::span<const int> candidate_ids) const
    {
        auto objs = parent_context.ContextObjects().findRaw<UniverseObjectCXBase>(candidate_ids);
        return EvalAny(parent_context, objs);
    }

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
                                       const UniverseObjectCXBase* candidate) const
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

    [[nodiscard]] constexpr virtual uint16_t GetDefaultInitialCandidateObjectTypes() const noexcept { return Impl::MatchesType::ANYOBJECTTYPE; }


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
    [[nodiscard]] constexpr virtual uint32_t GetCheckSum() const { return m_checksum_cache; }

    //! Makes a clone of this Condition in a new owning pointer. Required for
    //! Boost.Python, which doesn't support move semantics for returned values.
    [[nodiscard]] virtual std::unique_ptr<Condition> Clone() const = 0;

protected:
    constexpr Condition() = default;
    constexpr Condition(bool root_invariant, bool target_invariant, bool source_invariant, bool init_all_match,
                        uint32_t checksum = 0u) noexcept :
        m_checksum_cache(checksum),
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
    constexpr Condition(std::array<bool, 3> rts_invariants, uint32_t checksum = 0u) noexcept :
        Condition(rts_invariants[0], rts_invariants[1], rts_invariants[2], false, checksum)
    {}
    constexpr Condition(std::array<bool, 3> rts_invariants, bool init_all_match, uint32_t checksum = 0u) noexcept :
        Condition(rts_invariants[0], rts_invariants[1], rts_invariants[2], init_all_match, checksum)
    {}
    Condition& operator=(const Condition&) = delete;
    Condition& operator=(Condition&&) = delete;

    const uint32_t m_checksum_cache = 0u;

    const bool m_root_candidate_invariant = false;
    const bool m_target_invariant = false;
    const bool m_source_invariant = false;
    const bool m_initial_candidates_all_match = false;

private:
    virtual bool Match(const ScriptingContext& local_context) const { throw std::runtime_error("default Condition::Match called... Override missing?"); }
};

constexpr std::array<bool, 3> CondsRTSI(const auto& operands) {
    if constexpr (requires { *operands; operands->TargetInvariant(); }) {
        return {!operands || operands->RootCandidateInvariant(),
                !operands || operands->TargetInvariant(),
                !operands || operands->SourceInvariant()};

    } else if constexpr (requires { operands.TargetInvariant(); }) {
        return {operands.RootCandidateInvariant(), operands.TargetInvariant(), operands.SourceInvariant()};

    } else if constexpr (requires { operands.begin(); (*operands.begin()).TargetInvariant(); }) {
        return {std::all_of(operands.begin(), operands.end(), [](auto& e){ return e.RootCandidateInvariant(); }),
                std::all_of(operands.begin(), operands.end(), [](auto& e){ return e.TargetInvariant(); }),
                std::all_of(operands.begin(), operands.end(), [](auto& e){ return e.SourceInvariant(); })};

    } else if constexpr (requires { operands.begin(); (*operands.begin())->TargetInvariant(); }) {
        return {std::all_of(operands.begin(), operands.end(), [](auto& e){ return !e || e->RootCandidateInvariant(); }),
                std::all_of(operands.begin(), operands.end(), [](auto& e){ return !e || e->TargetInvariant(); }),
                std::all_of(operands.begin(), operands.end(), [](auto& e){ return !e || e->SourceInvariant(); })};

    } else if constexpr (std::is_same_v<std::decay_t<decltype(operands)>, std::nullptr_t>) {
        return {true, true, true};

    } else {
        throw std::invalid_argument("unrecognized type?");
    }
}

constexpr std::array<bool, 3> CondsRTSI(const auto&... operands) requires (sizeof...(operands) > 1) {
    std::array<bool, 3> retval{true, true, true};
    const auto get_and_rtsi = [&retval](const auto& op) {
        const auto op_rtsi = CondsRTSI(op);
        retval = {retval[0] && op_rtsi[0], retval[1] && op_rtsi[1], retval[2] && op_rtsi[2]};
    };
    (get_and_rtsi(operands), ...);
    return retval;
}

}


#endif
