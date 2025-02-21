#ifndef _Conditions_h_
#define _Conditions_h_


#include <memory>
#include <string>
#include <vector>
#include "Building.h"
#include "ConditionAll.h"
#include "ConditionSource.h"
#include "Condition.h"
#include "EnumsFwd.h"
#include "Planet.h"
#include "ValueRefs.h"
#include "../Empire/ProductionQueue.h"
#include "../util/CheckSums.h"
#include "../util/Export.h"


namespace ValueRef {
    template <typename T>
    struct ValueRef;
}

/** this namespace holds Condition and its subclasses; these classes represent
  * predicates about UniverseObjects used by, for instance, the Effect system. */
namespace Condition {

enum class SortingMethod : uint8_t {
    SORT_MAX,       ///< Objects with the largest sort key will be selected
    SORT_MIN,       ///< Objects with the smallest sort key will be selected
    SORT_MODE,      ///< Objects with the most common sort key will be selected
    SORT_RANDOM,    ///< Objects will be selected randomly, without consideration of property values
    SORT_UNIQUE     ///< Objects will be sorted by the sort key and one object per unique sort key will be selected
};

enum class ComparisonType : int8_t {
    INVALID_COMPARISON = -1,
    EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    NOT_EQUAL
};

enum class ContentType : uint8_t {
    CONTENT_BUILDING,
    CONTENT_SPECIES,
    CONTENT_SHIP_HULL,
    CONTENT_SHIP_PART,
    CONTENT_SPECIAL,
    CONTENT_FOCUS,
    CONTENT_POLICY,
    CONTENT_TECH
};

/** Same as ConditionDescription, but returns a string only with conditions that have not been met. */
[[nodiscard]] FO_COMMON_API std::string ConditionFailedDescription(
    const std::vector<const Condition*>& conditions,
    const ScriptingContext& source_context,
    const UniverseObject* candidate_object);

/** Returns a single string which describes a vector of Conditions. If multiple
  * conditions are passed, they are treated as if they were contained by an And
  * condition. Subconditions within an And (or nested And) are listed as
  * lines in a list, with duplicates removed, titled something like "All of...".
  * Subconditions within an Or (or nested Ors) are similarly listed as lines in
  * a list, with duplicates removed, titled something like "One of...". If a
  * candidate object is provided, the returned string will indicate which
  * subconditions the candidate matches, and indicate if the overall combination
  * of conditions matches the object. */
[[nodiscard]] FO_COMMON_API std::string ConditionDescription(
    const std::vector<const Condition*>& conditions,
    const ScriptingContext& source_context,
    const UniverseObject* candidate);

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

    } else {
        throw "unrecognized type?";
    }
}

/** Used by 4-parameter Condition::Eval function, and some of its overrides,
  * to scan through \a matches or \a non_matches set and apply \a pred to
  * each object, to test if it should remain in its current set or be
  * transferred from the \a search_domain specified set into the other. */
inline void EvalImpl(::Condition::ObjectSet& matches, ::Condition::ObjectSet& non_matches,
                     ::Condition::SearchDomain search_domain, const auto& pred)
{
    const bool domain_matches = search_domain == ::Condition::SearchDomain::MATCHES;
    auto& from_set = domain_matches ? matches : non_matches;
    auto& to_set = domain_matches ? non_matches : matches;

    // checking for from_set.size() == 1 and/or to_set.empty() and early exiting didn't seem to speed up evaluation in general case

    const auto part_it = std::stable_partition(from_set.begin(), from_set.end(),
                                               [&pred, domain_matches](const auto* o) { return pred(o) == domain_matches; });
    to_set.insert(to_set.end(), part_it, from_set.end());
    from_set.erase(part_it, from_set.end());
}

FO_COMMON_API void AddAllPlanetsSet(const ObjectMap& objects, ObjectSet& in_out);
FO_COMMON_API void AddAllBuildingsSet(const ObjectMap& objects, ObjectSet& in_out);
FO_COMMON_API void AddAllShipsSet(const ObjectMap& objects, ObjectSet& in_out);
FO_COMMON_API void AddAllFleetsSet(const ObjectMap& objects, ObjectSet& in_out);
FO_COMMON_API void AddAllSystemsSet(const ObjectMap& objects, ObjectSet& in_out);


// gets a planet from \a obj considering obj as a planet or a building on a planet
constexpr const Planet* PlanetFromObject(const ::Planet* obj, const auto&) noexcept
{ return obj; }

constexpr const Planet* PlanetFromObject(const ::Planet* obj) noexcept
{ return obj; }

constexpr const Planet* PlanetFromObject(const ::Building* obj, const auto get_planet)
    requires requires { {get_planet(obj->PlanetID())} -> std::same_as<const Planet*>; }
{ return obj ? get_planet(obj->PlanetID()) : nullptr; }

inline const Planet* PlanetFromObject(const ::Building* obj, const ObjectMap& objects)
{ return obj ? objects.getRaw<Planet>(obj->PlanetID()) : nullptr; }

constexpr const Planet* PlanetFromObject(const UniverseObject* obj, const auto& objects) {
    if (!obj)
        return nullptr;
    switch (obj->ObjectType()) {
    case UniverseObjectType::OBJ_PLANET: return static_cast<const ::Planet*>(obj); break;
    case UniverseObjectType::OBJ_BUILDING: return PlanetFromObject(static_cast<const ::Building*>(obj), objects); break;
    default: return nullptr;
    }
}

constexpr const Planet* PlanetFromObject(const auto*, const auto&)
{ return nullptr; }


/** Matches all objects if the number of objects that match Condition
  * \a condition is is >= \a low and < \a high.  Matched objects may
  * or may not themselves match the condition. */
struct FO_COMMON_API Number final : public Condition {
    Number(std::unique_ptr<ValueRef::ValueRef<int>>&& low,
           std::unique_ptr<ValueRef::ValueRef<int>>&& high,
           std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Number& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
    std::unique_ptr<Condition> m_condition;
    const bool m_high_low_local_invariant;
    const bool m_high_low_root_invariant;
};

/** Matches all objects if the current game turn is >= \a low and < \a high. */
struct FO_COMMON_API Turn final : public Condition {
    explicit Turn(std::unique_ptr<ValueRef::ValueRef<int>>&& low,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Turn& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
};

/** Matches a specified \a number of objects that match Condition \a condition
  * or as many objects as match the condition if the number of objects is less
  * than the number requested.  If more objects match the condition than are
  * requested, the objects are sorted according to the value of the specified
  * \a property_name and objects are matched according to whether they have
  * the specified \a sorting_type of those property values.  For example,
  * objects with the largest, smallest or most common property value may be
  * selected preferentially. */
struct FO_COMMON_API SortedNumberOf final : public Condition {
    /** Sorts randomly, without considering a sort key. */
    SortedNumberOf(std::unique_ptr<ValueRef::ValueRef<int>>&& number,
                   std::unique_ptr<Condition>&& condition);

    /** Sorts according to the specified method, based on the key values
      * evaluated for each object. */
    SortedNumberOf(std::unique_ptr<ValueRef::ValueRef<int>>&& number,
                   std::unique_ptr<ValueRef::ValueRef<double>>&& sort_key_ref,
                   SortingMethod sorting_method,
                   std::unique_ptr<Condition>&& condition);
    SortedNumberOf(std::unique_ptr<ValueRef::ValueRef<int>>&& number,
                   std::unique_ptr<ValueRef::ValueRef<std::string>>&& sort_key_ref,
                   SortingMethod sorting_method,
                   std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override;
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<int>> m_number;
    std::unique_ptr<ValueRef::ValueRef<double>> m_sort_key;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_sort_key_string;
    SortingMethod m_sorting_method;
    std::unique_ptr<Condition> m_condition;
};

/** Matches no objects. Currently only has an experimental use for efficient
  * immediate rejection as the top-line condition. Essentially, the entire point
  * of this Condition is to provide the specialized GetDefaultInitialCandidateObjects() */
struct FO_COMMON_API None final : public Condition {
    constexpr None() noexcept : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~None() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] bool operator==(const None&) const noexcept { return true; }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet&) const noexcept override { return false; }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override
    { return {}; /* efficient rejection of everything. */ }
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}
    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::None"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;
};

/** Does not modify the input ObjectSets. */
struct FO_COMMON_API NoOp final : public Condition {
    constexpr NoOp() noexcept : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~NoOp() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] bool operator==(const NoOp&) const noexcept { return true; }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override; // no noexcept due to logging
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override;
    [[nodiscard]] std::string Description(bool negated) const override
    { return UserString("DESC_NOOP"); }
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override
    { return DumpIndent(ntabs) + "NoOp\n"; }
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::NoOp"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override
    { return std::make_unique<NoOp>(); }
};

/** Matches all objects that are owned (if \a exclusive == false) or only owned
  * (if \a exclusive == true) by an empire that has affilitation type
  * \a affilitation with Empire \a empire_id. */
struct FO_COMMON_API EmpireAffiliation final : public Condition {
    EmpireAffiliation(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                      EmpireAffiliationType affiliation);
    explicit EmpireAffiliation(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);
    explicit EmpireAffiliation(EmpireAffiliationType affiliation);
    EmpireAffiliation(EmpireAffiliation&&) noexcept = default;
    EmpireAffiliation(const EmpireAffiliation&) noexcept = delete;

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const EmpireAffiliation& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    const EmpireAffiliationType m_affiliation;
};

/** Matches the root candidate object in a condition tree.  This is useful
  * within a subcondition to match the object actually being matched by the
  * whole compound condition, rather than an object just being matched in a
  * subcondition in order to evaluate the outer condition. */
struct FO_COMMON_API RootCandidate final : public Condition {
    constexpr RootCandidate() noexcept : Condition(false, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~RootCandidate() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const RootCandidate&) const noexcept { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const noexcept override {
        // if parent_context has no root candidate, then the local candidate is the root candidate for this condition
        return candidate && (!parent_context.condition_root_candidate || parent_context.condition_root_candidate == candidate);
    }

    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::RootCandidate"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const noexcept override {
        return local_context.condition_root_candidate &&
            local_context.condition_root_candidate == local_context.condition_local_candidate;
    }
};

/** There is no LocalCandidate condition. To match any local candidate object,
  * use the All condition. */

/** Matches the target of an effect being executed. */
struct FO_COMMON_API Target final : public Condition {
    constexpr Target() noexcept : Condition(true, false, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Target() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Target&) const noexcept { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const noexcept override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::Target"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const noexcept override {
        return local_context.effect_target &&
            local_context.effect_target == local_context.condition_local_candidate;
    }
};

/** Matches planets that are a homeworld for any of the species specified in
  * \a names.  If \a names is empty, matches any planet that is a homeworld for
  * any species in the current game Universe. */
struct FO_COMMON_API Homeworld final : public Condition {
    using string_vref_ptr_vec = std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>;
    explicit Homeworld(string_vref_ptr_vec&& names);
    explicit Homeworld(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    Homeworld() noexcept(noexcept(string_vref_ptr_vec{}));

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Homeworld& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    string_vref_ptr_vec m_names;
    const bool          m_names_local_invariant;
};

/** Matches planets that are an empire's capital. */
struct FO_COMMON_API Capital final : public Condition {
    constexpr Capital() noexcept : Condition(true, true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Capital() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Capital&) const noexcept { return true; }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override; // no noexcept due to logging
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string&) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::Capital"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;
};

struct FO_COMMON_API CapitalWithID final : public Condition {
    explicit CapitalWithID(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id) noexcept;
    CapitalWithID() = delete; // use Capital with no parameter
    CapitalWithID(CapitalWithID&&) noexcept = default;

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const CapitalWithID& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override; // no noexcept due to logging
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string&) noexcept override {}
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
};

/** Matches space monsters. */
struct FO_COMMON_API Monster final : public Condition {
    constexpr Monster() noexcept : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Monster() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Monster&) const noexcept { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::Monster"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;
};

/** Matches armed ships and monsters. */
struct FO_COMMON_API Armed final : public Condition {
    constexpr Armed() noexcept : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Armed() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Armed&) const noexcept { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override
    { return (!negated) ? UserString("DESC_ARMED") : UserString("DESC_ARMED_NOT"); }
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override
    { return DumpIndent(ntabs) + "Armed\n"; }
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::Armed"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;
};

/** Matches all objects that are of UniverseObjectType \a type. */
struct FO_COMMON_API Type final : public Condition {
    explicit Type(std::unique_ptr<ValueRef::ValueRef<UniverseObjectType>>&& type);
    explicit Type(UniverseObjectType type);
    Type(Type&&) noexcept = default;

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Type& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override;
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<UniverseObjectType>> m_type;

    const bool m_type_const;
    const bool m_type_local_invariant;
};

/** Matches all Building objects that are one of the building types specified in \a names. */
struct FO_COMMON_API Building final : public Condition {
    using string_vref_ptr_vec = std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>;
    explicit Building(string_vref_ptr_vec&& names);
    explicit Building(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit Building(std::string name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Building& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    string_vref_ptr_vec m_names;
    const bool m_names_local_invariant;
};

/** Matches all Field objects that are one of the field types specified in \a names. */
struct FO_COMMON_API Field final : public Condition {
    using string_vref_ptr_vec = std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>;
    explicit Field(string_vref_ptr_vec&& names);
    explicit Field(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit Field(std::string name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Field& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    string_vref_ptr_vec m_names;
    const bool m_names_local_invariant;
};

/** Matches all objects that have an attached Special named \a name. */
struct FO_COMMON_API HasSpecial final : public Condition {
    HasSpecial();
    explicit HasSpecial(std::string name);
    explicit HasSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    HasSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
               std::unique_ptr<ValueRef::ValueRef<int>>&& since_turn_low,
               std::unique_ptr<ValueRef::ValueRef<int>>&& since_turn_high = nullptr);
    HasSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
               std::unique_ptr<ValueRef::ValueRef<double>>&& capacity_low,
               std::unique_ptr<ValueRef::ValueRef<double>>&& capacity_high = nullptr);
    explicit HasSpecial(const HasSpecial& rhs);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_capacity_low;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_capacity_high;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_since_turn_low;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_since_turn_high;
    const bool                                          m_refs_local_invariant;
};

/** Matches all objects that have the tag \a tag. */
struct FO_COMMON_API HasTag final : public Condition {
    constexpr HasTag() noexcept : Condition(true, true, true) {}
    explicit HasTag(std::string name);
    explicit HasTag(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    // no constexpr destructor GCC workaround as ~unique_ptr is not constexpr before C++23 anyway...

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const HasTag& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
};

/** Matches all objects that were created on turns within the specified range. */
struct FO_COMMON_API CreatedOnTurn final : public Condition {
    CreatedOnTurn(std::unique_ptr<ValueRef::ValueRef<int>>&& low,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& high);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
};

/** Matches all objects that contain an object that matches Condition \a condition.
  * Container objects are Systems, Planets (which contain Buildings),
  * and Fleets (which contain Ships). */
template <class ConditionT = std::unique_ptr<Condition>>
    requires ((std::is_base_of_v<Condition, ConditionT> &&
               !std::is_same_v<Condition, std::decay_t<ConditionT>>) ||
              std::is_same_v<std::unique_ptr<Condition>, ConditionT>)
struct FO_COMMON_API Contains final : public Condition {
private:
    static constexpr bool cond_is_ptr = requires(const ConditionT c) { c.get(); };
    static constexpr bool cond_equals_noexcept = noexcept(std::declval<ConditionT>() == std::declval<ConditionT>());
    static constexpr bool cond_deref_equals_noexcept = noexcept(std::declval<ConditionT>() == std::declval<ConditionT>());
    static constexpr const UniverseObject* no_object = nullptr;

public:
    template <typename... Args>
#if !defined(__GNUC__) || (__GNUC__ > 13) || (__GNUC__ == 13 && __GNUC_MINOR__ >= 3)
        requires requires { ConditionT(std::forward<Args>(std::declval<Args>())...); }
#endif
    constexpr explicit Contains(Args&&... args) :
        Contains(ConditionT(std::forward<Args>(args)...))
    {}

    constexpr explicit Contains(ConditionT&& operand) noexcept :
        Condition(CondsRTSI(operand)),
        m_condition(std::move(operand))
    {}

#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Contains() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] constexpr bool operator==(const Contains& rhs) const noexcept(cond_equals_noexcept && cond_deref_equals_noexcept) {
        if (this == &rhs || m_condition == rhs.m_condition)
            return true;
        if constexpr (cond_is_ptr)
            return m_condition && (*m_condition == *rhs.m_condition);
        else
            return false;
    }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override
    {
        const auto search_domain_size = (search_domain == SearchDomain::MATCHES ?
                                         matches.size() : non_matches.size());
        const bool simple_eval_safe = parent_context.condition_root_candidate ||
                                      RootCandidateInvariant() || search_domain_size < 2;
        if (!simple_eval_safe) [[unlikely]] {
            // re-evaluate contained objects for each candidate object
            Condition::Eval(parent_context, matches, non_matches, search_domain);
            return;

        } else if (search_domain_size == 1u) [[likely]] {
            const auto* candidate = search_domain == SearchDomain::MATCHES ? matches.front() : non_matches.front();
            const bool contained_match_exists = EvalOne(parent_context, candidate); // faster than m_condition->EvalAny(local_context, contained_objects) in my tests

            // move single local candidate as appropriate...
            if (search_domain == SearchDomain::MATCHES && !contained_match_exists) {
                // move to non_matches
                matches.clear();
                non_matches.push_back(candidate);
            } else if (search_domain == SearchDomain::NON_MATCHES && contained_match_exists) {
                // move to matches
                non_matches.clear();
                matches.push_back(candidate);
            }

        } else if (search_domain_size > 1u) {
            // evaluate contained objects once using default initial candidates
            // of subcondition to find all subcondition matches in the Universe
            const ScriptingContext local_context{parent_context, ScriptingContext::LocalCandidate{}, no_object};
            const ObjectSet subcondition_matches = [this, &local_context]() {
                if constexpr (cond_is_ptr)
                    return m_condition->Eval(local_context);
                else
                    return static_cast<const Condition&>(m_condition).Eval(local_context);
            }();

            // check all candidates to see if they contain any subcondition matches
            EvalImpl(matches, non_matches, search_domain, ContainsSimpleMatch(subcondition_matches));
        }
    }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override {
        // objects that can contain other objects: systems, fleets, planets
        ObjectSet retval;
        retval.reserve(parent_context.ContextObjects().size<System>() +
                       parent_context.ContextObjects().size<Fleet>() +
                       parent_context.ContextObjects().size<Planet>());
        AddAllSystemsSet(parent_context.ContextObjects(), retval);
        AddAllFleetsSet(parent_context.ContextObjects(), retval);
        AddAllPlanetsSet(parent_context.ContextObjects(), retval);
        return retval;
    }

    [[nodiscard]] std::string Description(bool negated = false) const override {
        auto fmt = FlexibleFormat((!negated) ? UserString("DESC_CONTAINS") : UserString("DESC_CONTAINS_NOT"));
        if constexpr (cond_is_ptr)
            return str(fmt % m_condition->Description());
        else
            return str(fmt % m_condition.Description());
    }

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override {
        std::string retval = DumpIndent(ntabs) + "Contains condition =\n";
        if constexpr (cond_is_ptr)
            return retval + m_condition->Dump(ntabs+1);
        else
            return retval + m_condition.Dump(ntabs+1);
    }

    void SetTopLevelContent(const std::string& content_name) override {
        if constexpr (cond_is_ptr) {
            if (m_condition)
                m_condition->SetTopLevelContent(content_name);
        } else {
            m_condition.SetTopLevelContent(content_name);
        }
    }

    [[nodiscard]] constexpr uint32_t GetCheckSum() const
#if !defined(__GNUC__) || (__GNUC__ > 13)
        noexcept(noexcept(CheckSums::GetCheckSum("")) &&
                 noexcept(CheckSums::CheckSumCombine(std::declval<uint32_t&>(), std::declval<ConditionT>())))
#endif
        override
    {
        uint32_t retval = CheckSums::GetCheckSum("Condition::Contains");
        CheckSums::CheckSumCombine(retval, m_condition);
        return retval;
    }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override {
        using this_t = std::decay_t<decltype(*this)>;
        if constexpr (cond_is_ptr) {
            return std::make_unique<this_t>(m_condition->Clone());
        } else {
            auto operand_clone = m_condition.Clone();
            return std::make_unique<this_t>(std::move(dynamic_cast<ConditionT&>(*operand_clone))); // throws if .Clone() doesn't return a pointer to a valid ConditionT
        }
    }

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override {
        const auto* candidate = local_context.condition_local_candidate;
        if (!candidate) [[unlikely]]
            return false;
        const auto subcondition_candidates = local_context.ContextObjects().findRaw(candidate->ContainedObjectIDs());
        if constexpr (cond_is_ptr)
            return m_condition->EvalAny(local_context, subcondition_candidates); // TODO: add EvalAny overload taking object ids, as getting the objects isn't really necessary just to check if something contains their ID
        else
            return m_condition.EvalAny(local_context, subcondition_candidates);
    }

    struct ContainsSimpleMatch {
        CONSTEXPR_VEC ContainsSimpleMatch(std::vector<int> subcondition_matches_ids) :
            m_subcondition_matches_ids([&subcondition_matches_ids]() {
                // We need a sorted container for efficiently intersecting
                // subcondition_matches with the set of objects contained in some
                // candidate object.
                if (!subcondition_matches_ids.empty())
                    CheckSums::InPlaceSort(subcondition_matches_ids);
                return subcondition_matches_ids;
            }())
        {}

        ContainsSimpleMatch(const ObjectSet& subcondition_matches) :
            ContainsSimpleMatch([&subcondition_matches]() {
                // We only need ids, not objects.
                std::vector<int> m;
                m.reserve(subcondition_matches.size());
                // gather the ids and sort them
                for (auto* obj : subcondition_matches)
                    if (obj)
                        m.push_back(obj->ID());
                return m;
            }())
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            bool match = false;
            const auto& candidate_elements = candidate->ContainedObjectIDs(); // guaranteed O(1)

            // We need to test whether candidate_elements and m_subcondition_matches_ids have a common element.
            // We choose the strategy that is more efficient by comparing the sizes of both sets.
            if (candidate_elements.size() < m_subcondition_matches_ids.size()) {
                // candidate_elements is smaller, so we iterate it and look up each candidate element in m_subcondition_matches_ids
                for (int id : candidate_elements) {
                    // std::lower_bound requires m_subcondition_matches_ids to be sorted
                    auto matching_it = std::lower_bound(m_subcondition_matches_ids.begin(), m_subcondition_matches_ids.end(), id);

                    if (matching_it != m_subcondition_matches_ids.end() && *matching_it == id) {
                        match = true;
                        break;
                    }
                }
            } else {
                // m_subcondition_matches_ids is smaller, so we iterate it and look up each subcondition match in the set of candidate's elements
                for (int id : m_subcondition_matches_ids) {
                    // candidate->Contains() may have a faster implementation than candidate_elements->find()
                    if (candidate->Contains(id)) {
                        match = true;
                        break;
                    }
                }
            }

            return match;
        }

        const std::vector<int> m_subcondition_matches_ids;
    };

    ConditionT m_condition;
};

template <typename T>
Contains(T) -> Contains<std::decay_t<T>>;

/** Matches all objects that are contained by an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API ContainedBy final : public Condition {
    ContainedBy(std::unique_ptr<Condition>&& condition);
    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_condition;
};

/** Matches all objects that are in the system with the indicated \a system_id
  * or that are that system. If \a system_id is INVALID_OBJECT_ID then matches
  * all objects in any system*/
struct FO_COMMON_API InOrIsSystem final : public Condition {
    explicit InOrIsSystem(std::unique_ptr<ValueRef::ValueRef<int>>&& system_id);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_system_id;
};

/** Matches all objects that are on the planet with the indicated \a planet_id
  * or that are that planet. If \a planet_id is INVALID_OBJECT_ID then matches
  * all objects on any planet */
struct FO_COMMON_API OnPlanet final : public Condition {
    OnPlanet(std::unique_ptr<ValueRef::ValueRef<int>>&& planet_id);
    constexpr OnPlanet() noexcept : Condition(true, true, true, false) {}
    OnPlanet(OnPlanet&&) noexcept = default;

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const OnPlanet& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_planet_id;
};

/** Matches the object with the id \a object_id */
struct FO_COMMON_API ObjectID final : public Condition {
    ObjectID(std::unique_ptr<ValueRef::ValueRef<int>>&& object_id);

    [[nodiscard]] bool operator==(const Condition& rhs) const noexcept override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const ObjectID& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_object_id;
};

/** Matches all Planet objects that have one of the PlanetTypes in \a types.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API PlanetTypeBase : public Condition {
    constexpr PlanetTypeBase(std::array<bool, 3> rtsi) noexcept : Condition(rtsi) {}
    constexpr PlanetTypeBase(bool r, bool t, bool s) noexcept : Condition(r, t, s) {}
};

template <typename PT_t = std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>, std::size_t N = 0>
    requires(
        std::is_same_v<PT_t, ::PlanetType> ||                                    // store 1 or more PlanetType by value
        std::is_same_v<PT_t, std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>> // store ValueRefs. maybe fixed-sized array or runtime-determined vector
    )
struct FO_COMMON_API PlanetType final : public PlanetTypeBase {
private:
    static constexpr bool have_pt_values = std::is_same_v<PT_t, ::PlanetType>;
    static constexpr bool have_fixed_count = (N >= 1);

public:
    using pt_ref_up = std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>;

    using PTs_t = std::conditional_t<have_fixed_count,
        std::conditional_t<N == 1, PT_t, std::array<PT_t, N>>,
        std::vector<PT_t>>;

private:
    PTs_t       m_types;
    const bool  m_types_local_invariant = have_pt_values;

public:
    explicit PlanetType(std::vector<pt_ref_up>&& types) requires ((N == 0) && !have_pt_values) :
        PlanetTypeBase(CondsRTSI(types)),
        m_types(std::move(types)), // TODO: remove any nullptr types? throw if then empty?
        m_types_local_invariant(std::all_of(m_types.begin(), m_types.end(),
                                            [](const auto& e) { return e->LocalCandidateInvariant(); }))
    {}
    explicit PlanetType(std::array<pt_ref_up, N>&& types) requires ((N > 0) && !have_pt_values) :
        PlanetTypeBase(CondsRTSI(types)),
        m_types(std::move(types)),  // TODO: check / throw if any are nullptr types?
        m_types_local_invariant(std::all_of(m_types.begin(), m_types.end(),
                                            [](const auto& e) { return e->LocalCandidateInvariant(); }))
    {}
    constexpr explicit PlanetType(::PlanetType type) requires ((N == 1) && have_pt_values) :
        PlanetTypeBase(true, true, true),
        m_types(type)
    {}
    CONSTEXPR_VEC explicit PlanetType(std::vector<::PlanetType> types) requires ((N == 0) && have_pt_values) :
        PlanetTypeBase(true, true, true),
        m_types(std::move(types))
    {}

#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~PlanetType() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] constexpr bool operator==(const PlanetType& rhs_) const {
        if constexpr (requires { m_types.size() == rhs_.m_types.size(); }) {
            if (m_types.size() != rhs_.m_types.size())
                return false;
        }
        if constexpr (have_pt_values && requires { m_types == rhs_.m_types; }) {
            if (m_types != rhs_.m_types)
                return false;
        }
        if constexpr (have_pt_values && requires { m_types.size(); m_types.at(1) != rhs_.m_types.at(1); }) {
            for (std::size_t idx = 0u; idx < m_types.size(); ++idx)
                if (m_types.at(idx) != rhs_.m_types.at(idx))
                    return false;
        }
        if constexpr (!have_pt_values && requires { m_types.size(); bool(m_types.at(1)); *m_types.at(1) != *rhs_.m_types.at(1); }) {
            for (std::size_t idx = 0u; idx < m_types.size(); ++idx)
                if (m_types.at(idx) && rhs_.m_types.at(idx) && (*m_types.at(idx) != *rhs_.m_types.at(idx)))
                    return false;
        } else if constexpr (!have_pt_values && requires { *m_types != *rhs_.m_types; }) {
            if (m_types && rhs_.m_types && (*m_types != *rhs_.m_types))
                return false;
        }

        return true;
    }

    template <std::size_t SN>
    struct PlanetTypeSimpleMatch {
        using SMPT_t = std::conditional_t<SN == 1, ::PlanetType, std::span<const ::PlanetType, SN>>;

        PlanetTypeSimpleMatch(SMPT_t types, const ObjectMap& objects) noexcept :
            m_types(types),
            m_objects(objects)
        {}

        bool operator()(const Planet* candidate) const noexcept(SN == 1) {
            if (!candidate)
                return false;
            const auto pt = candidate->Type();
            if constexpr (SN == 1)
                return pt == m_types;
            else
                return std::any_of(m_types.begin(), m_types.end(),
                                   [pt](const auto t) noexcept { return t == pt; });
        }

        bool operator()(const auto* candidate) const
        { return operator()(PlanetFromObject(candidate, m_objects)); }

        const SMPT_t m_types;
        const ObjectMap& m_objects;
    };

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override
    {
        if constexpr (have_pt_values) {
            static constexpr std::size_t SM_size = (N == 0 ? std::dynamic_extent : N);
            EvalImpl(matches, non_matches, search_domain,
                     PlanetTypeSimpleMatch<SM_size>(m_types, parent_context.ContextObjects()));
        } else {
            const bool simple_eval_safe = m_types_local_invariant &&
                (parent_context.condition_root_candidate || RootCandidateInvariant());
            if (!simple_eval_safe) {
                // re-evaluate contained objects for each candidate object
                Condition::Eval(parent_context, matches, non_matches, search_domain);

            } else if constexpr (requires { m_types.begin(); m_types.end(); }) {
                // evaluate refs
                std::vector< ::PlanetType> types; // TODO: array if m_types is array?
                types.reserve(m_types.size());
                // get all types from valuerefs
                for (auto& type : m_types)
                    types.push_back(type->Eval(parent_context));
                EvalImpl(matches, non_matches, search_domain,
                         PlanetTypeSimpleMatch<std::dynamic_extent>(types, parent_context.ContextObjects()));

            } else {
                const auto cur_pt = m_types->Eval(parent_context);
                EvalImpl(matches, non_matches, search_domain,
                         PlanetTypeSimpleMatch<1u>(cur_pt, parent_context.ContextObjects()));
            }
        }
    }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override {
        // objects that are or are on planets: Building, Planet
        ObjectSet retval;
        retval.reserve(parent_context.ContextObjects().size<Planet>() +
                       parent_context.ContextObjects().size<::Building>());
        AddAllPlanetsSet(parent_context.ContextObjects(), retval);
        AddAllBuildingsSet(parent_context.ContextObjects(), retval);
        return retval;
    }

    [[nodiscard]] std::string Description(bool negated = false) const override {
        std::string values_str;

        if constexpr (requires { m_types.size(); m_types[1]; }) {
            for (std::size_t i = 0; i < m_types.size(); ++i) {
                if constexpr (have_pt_values) {
                    values_str += UserString(to_string(m_types[i]));
                } else {
                    values_str += m_types[i]->ConstantExpr() ?
                        UserString(to_string(m_types[i]->Eval())) :
                        m_types[i]->Description();
                }
                if (2 <= m_types.size() && i < m_types.size() - 2) {
                    values_str += ", ";
                } else if (i == m_types.size() - 2) {
                    values_str += m_types.size() < 3 ? " " : ", ";
                    values_str += UserString("OR");
                    values_str += " ";
                }
            }
        } else {
            if constexpr (have_pt_values) {
                values_str = UserString(to_string(m_types));
            } else {
                values_str += m_types->ConstantExpr() ?
                    UserString(to_string(m_types->Eval())) :
                    m_types->Description();
            }
        }
        return str(FlexibleFormat((!negated)
                                  ? UserString("DESC_PLANET_TYPE")
                                  : UserString("DESC_PLANET_TYPE_NOT"))
                   % values_str);
    }

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override {
        const auto dump1 = [=](const auto& pt) -> decltype(auto) {
            if constexpr (requires { DumpEnum(pt); })
                return std::string{DumpEnum(pt)};
            else if constexpr (requires { bool(pt); pt->Dump(); })
                return pt ? pt->Dump(ntabs) : std::string{};
            else if constexpr (requires { to_string(pt); })
                return std::string{to_string(pt)};
            else
                return "???";
        };
        const auto dumpN = [=](const auto& pts) -> decltype(auto) {
            if constexpr (requires { *pts.begin(); pts.end(); pts.size(); }) {
                if (pts.size() == 1)
                    return std::string{dump1(*pts.begin())};
                std::string retval = "[ ";
                for (const auto& pt : pts)
                    retval.append(dump1(pt) + " ");
                retval.append("]\n");
                return retval;
            } else {
                return dump1(pts);
            }
        };

        return DumpIndent(ntabs) + "Planet type = " + dumpN(m_types);
    }

    void SetTopLevelContent(const std::string& content_name) override {
        if constexpr (!have_pt_values) {
            if constexpr (requires { m_types.begin(); m_types.end(); }) {
                for (auto& type : m_types) {
                    if (type)
                        type->SetTopLevelContent(content_name);
                }
            } else {
                if (m_types)
                    m_types->SetTopLevelContext(content_name);
            }
        }
    }

    [[nodiscard]] constexpr uint32_t GetCheckSum() const override {
        uint32_t retval = CheckSums::GetCheckSum("Condition::PlanetType");
        CheckSums::CheckSumCombine(retval, m_types);
        return retval;
    }

    std::unique_ptr<Condition> Clone() const {
        if constexpr (have_pt_values)
            return std::make_unique<std::decay_t<decltype(*this)>>(m_types);
            //return std::make_unique<PlanetType<PT_t, N>>(m_types);
        else
            return std::make_unique<std::decay_t<decltype(*this)>>(ValueRef::CloneUnique(m_types));
    }

private:
    bool Match(const ScriptingContext& local_context) const {
        const auto* planet = PlanetFromObject(local_context.condition_local_candidate, local_context.ContextObjects());
        if (!planet)
            return false;
        const auto planet_type = planet->Type();

        if constexpr (requires { m_types.begin(); m_types.end(); }) {
            if constexpr (have_pt_values)
                return std::any_of(m_types.begin(), m_types.end(),
                                   [planet_type](auto pt) noexcept { return pt == planet_type; });
            else
                return std::any_of(m_types.begin(), m_types.end(),
                                   [planet_type, &local_context](const auto& pt_ref)
                                   { return pt_ref && pt_ref->Eval(local_context) == planet_type; });
        } else {
            if constexpr (have_pt_values)
                return m_types == planet_type;
            else
                return m_types && (m_types->Eval(local_context) == planet_type);
        }
    }
};

PlanetType(::PlanetType) -> PlanetType<::PlanetType, 1>;
PlanetType(std::vector<::PlanetType>) -> PlanetType<::PlanetType, 0>;
PlanetType(std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>) -> PlanetType<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>, 1>;
PlanetType(std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>>) -> PlanetType<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>, 0>;


/** Matches all Planet objects that have one of the PlanetSizes in \a sizes.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API PlanetSize final : public Condition {
    PlanetSize(std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>>&& sizes);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<::PlanetSize>>> m_sizes;
};

/** Matches all Planet objects that have one of the PlanetEnvironments in
  * \a environments.  Note that all Building objects which are on matching
  * planets are also matched. */
struct FO_COMMON_API PlanetEnvironment final : public Condition {
    PlanetEnvironment(std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>>&& environments,
                      std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name_ref = nullptr);

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_ = dynamic_cast<decltype(this)>(&rhs);
        return rhs_ && (*rhs_ == *this);
    }
    [[nodiscard]] bool operator==(const PlanetEnvironment& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<::PlanetEnvironment>>> m_environments;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_species_name;
};

/** Matches all planets or ships that have one of the species in \a species.
  * Note that all Building object which are on matching planets are also
  * matched. */
struct FO_COMMON_API Species final : public Condition {
    explicit Species(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);
    Species();

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Species& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> m_names;
};

/** Matches objects if the specified species' opinion of the specified content
  * is within the specified bounds. */
struct FO_COMMON_API SpeciesOpinion final : public Condition {
    SpeciesOpinion(std::unique_ptr<ValueRef::ValueRef<std::string>>&& species,
                   std::unique_ptr<ValueRef::ValueRef<std::string>>&& content,
                   ComparisonType comp = ComparisonType::GREATER_THAN);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_species;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_content;
    ComparisonType m_comp = ComparisonType::GREATER_THAN;
};

/** Matches planets where the indicated number of the indicated building type
  * or ship design are enqueued on the production queue. */
struct FO_COMMON_API Enqueued final : public Condition {
    Enqueued(BuildType build_type,
             std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
             std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id = nullptr,
             std::unique_ptr<ValueRef::ValueRef<int>>&& low = nullptr,
             std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);
    explicit Enqueued(std::unique_ptr<ValueRef::ValueRef<int>>&& design_id,
                      std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id = nullptr,
                      std::unique_ptr<ValueRef::ValueRef<int>>&& low = nullptr,
                      std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);
    Enqueued();
    Enqueued(const Enqueued& rhs);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Enqueued& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    BuildType m_build_type;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_design_id;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_empire_id;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_low;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_high;
};

/** Matches all ProdCenter objects that have one of the FocusTypes in \a foci. */
struct FO_COMMON_API FocusType final : public Condition {
    FocusType(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const FocusType& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> m_names;
};

/** Matches all System objects that have one of the StarTypes in \a types.  Note that all objects
    in matching Systems are also matched (Ships, Fleets, Buildings, Planets, etc.). */
struct FO_COMMON_API StarType final : public Condition {
    explicit StarType(std::vector<std::unique_ptr<ValueRef::ValueRef< ::StarType>>>&& types);
    explicit StarType(std::unique_ptr<ValueRef::ValueRef<::StarType>>&& type);
    explicit StarType(::StarType type);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const StarType& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<::StarType>>> m_types;
};

/** Matches all ships whose ShipDesign has the hull specified by \a name. */
struct FO_COMMON_API DesignHasHull final : public Condition {
    explicit DesignHasHull(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
};

/** Matches all ships whose ShipDesign has >= \a low and < \a high of the ship
  * part specified by \a name. */
struct FO_COMMON_API DesignHasPart final : public Condition {
    DesignHasPart(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& low = nullptr,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
};

/** Matches ships whose ShipDesign has >= \a low and < \a high of ship parts of
  * the specified \a part_class */
struct FO_COMMON_API DesignHasPartClass final : public Condition {
    DesignHasPartClass(ShipPartClass part_class,
                       std::unique_ptr<ValueRef::ValueRef<int>>&& low,
                       std::unique_ptr<ValueRef::ValueRef<int>>&& high);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
    ShipPartClass m_class;
};

/** Matches ships who ShipDesign is a predefined shipdesign with the name
  * \a name */
struct FO_COMMON_API PredefinedShipDesign final : public Condition {
    explicit PredefinedShipDesign(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
};

/** Matches ships whose design id \a id. */
struct FO_COMMON_API NumberedShipDesign final : public Condition {
    explicit NumberedShipDesign(std::unique_ptr<ValueRef::ValueRef<int>>&& design_id);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_design_id;
};

/** Matches ships or buildings produced by the empire with id \a empire_id.*/
struct FO_COMMON_API ProducedByEmpire final : public Condition {
    explicit ProducedByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
};

/** Matches a given object with a linearly distributed probability of \a chance. */
struct FO_COMMON_API Chance final : public Condition {
    explicit Chance(std::unique_ptr<ValueRef::ValueRef<double>>&& chance);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<double>> m_chance;
};

/** Matches all objects that have a meter of type \a meter, and whose current
  * value is >= \a low and <= \a high. */
struct FO_COMMON_API MeterValue final : public Condition {
    MeterValue(MeterType meter,
               std::unique_ptr<ValueRef::ValueRef<double>>&& low,
               std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const MeterValue& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    const MeterType m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;
    const bool m_low_high_local_invariant;
};

/** Matches ships that have a ship part meter of type \a meter for part \a part
  * whose current value is >= low and <= high. */
struct FO_COMMON_API ShipPartMeterValue final : public Condition {
    ShipPartMeterValue(std::unique_ptr<ValueRef::ValueRef<std::string>>&& ship_part_name,
                       MeterType meter,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_part_name;
    MeterType m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;
};

/** Matches all objects if the empire with id \a empire_id has an empire meter
  * \a meter whose current value is >= \a low and <= \a high. */
struct FO_COMMON_API EmpireMeterValue final : public Condition {
    EmpireMeterValue(std::string meter,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& high);
    EmpireMeterValue(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                     std::string meter,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    const std::string m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;
};

/** Matches all objects whose owner's stockpile of \a stockpile is between
  * \a low and \a high, inclusive. */
struct FO_COMMON_API EmpireStockpileValue final : public Condition {
    EmpireStockpileValue(ResourceType stockpile,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& high);
    EmpireStockpileValue(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                         ResourceType stockpile,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>>    m_empire_id;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;
    const ResourceType                          m_stockpile;
    const bool                                  m_refs_local_invariant;
};

/** Matches all objects if the empire with id \a empire_id has adopted the
  * imperial policy with name \a name */
struct FO_COMMON_API EmpireHasAdoptedPolicy final : public Condition {
    EmpireHasAdoptedPolicy(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                           std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit EmpireHasAdoptedPolicy(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    virtual ~EmpireHasAdoptedPolicy();

    [[nodiscard]] bool operator==(const Condition& rhs) const override;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_empire_id;
};

/** Matches all objects whose owner who has tech \a name or all objects when the
  * specified empire has tech \a name. */
struct FO_COMMON_API OwnerHasTech final : public Condition {
    OwnerHasTech(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                 std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit OwnerHasTech(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }

    [[nodiscard]] bool operator==(const OwnerHasTech& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>         m_empire_id;
};

/** Matches objects if the species empire has the building type \a name available. */
struct FO_COMMON_API EmpireHasBuildingTypeAvailable final : public Condition {
    EmpireHasBuildingTypeAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                                   std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit EmpireHasBuildingTypeAvailable(const std::string& name);
    explicit EmpireHasBuildingTypeAvailable(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>         m_empire_id;
};

/** Matches object if the specified empire has the ship design \a id available. */
struct FO_COMMON_API EmpireHasShipDesignAvailable final : public Condition {
    EmpireHasShipDesignAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                                 std::unique_ptr<ValueRef::ValueRef<int>>&& design_id);
    explicit EmpireHasShipDesignAvailable(int design_id);
    explicit EmpireHasShipDesignAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& design_id);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_id;
    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
};

/** Matches objects if the specified empire has the ship part @a name available. */
struct FO_COMMON_API EmpireHasShipPartAvailable final : public Condition {
    EmpireHasShipPartAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                               std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit EmpireHasShipPartAvailable(const std::string& name);
    explicit EmpireHasShipPartAvailable(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>         m_empire_id;
};

/** Matches all objects that are visible to the Empire with id \a empire_id */
struct FO_COMMON_API VisibleToEmpire final : public Condition {
    explicit VisibleToEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);
    VisibleToEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                    std::unique_ptr<ValueRef::ValueRef<int>>&& since_turn,
                    std::unique_ptr<ValueRef::ValueRef<Visibility>>&& vis);

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const VisibleToEmpire& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    std::unique_ptr<ValueRef::ValueRef<int>> m_since_turn;
    std::unique_ptr<ValueRef::ValueRef<Visibility>> m_vis;
};

/** Matches all objects that are within \a distance units of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct FO_COMMON_API WithinDistance final : public Condition {
    WithinDistance(std::unique_ptr<ValueRef::ValueRef<double>>&& distance,
                   std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<double>> m_distance;
    std::unique_ptr<Condition> m_condition;
};

/** Matches all objects that are within \a jumps starlane jumps of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct FO_COMMON_API WithinStarlaneJumps final : public Condition {
    WithinStarlaneJumps(std::unique_ptr<ValueRef::ValueRef<int>>&& jumps,
                        std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_jumps;
    std::unique_ptr<Condition> m_condition;
};

/** Matches objects that have a starlane to at least one object that matches \a condition. */
struct FO_COMMON_API HasStarlaneTo : Condition {
    explicit HasStarlaneTo(std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_condition;
};

/** Matches objects if a starlane from the location of the candidate and the location
  * of any object that matches \a condition would cross any existing starlane.
  * If given multiple candidates, does not consider if lines to them from the objects
  * that match \a condition may or may not cross. */
struct FO_COMMON_API StarlaneToWouldCrossExistingStarlane : Condition {
    explicit StarlaneToWouldCrossExistingStarlane(std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_condition;
};

/** Matches objects if a starlane from the location of the candidate and the location
  * of any object that matches \a condition would be angularly too close to an existing
  * starlane on either end. Objects on both ends of the new starlane that are systems
  * or that are in systems have that system's lanes check. If just one of the candidate
  * and other object are or are in systems, the only the other end's existing lanes are
  * checked. If neither object at the ends of the new lane are systems, then no existing
  * lanes are checked. */
struct FO_COMMON_API StarlaneToWouldBeAngularlyCloseToExistingStarlane : Condition {
    // magic limit adjusted to allow no more than 12 starlanes from a system arccos(0.87) = 0.515594 rad = 29.5 degrees
    static constexpr double DEFAULT_MAX_LANE_DOT_PRODUCT = 0.87;

    explicit StarlaneToWouldBeAngularlyCloseToExistingStarlane(
        std::unique_ptr<Condition>&& condition, double max_dotprod = DEFAULT_MAX_LANE_DOT_PRODUCT);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_condition;
    double m_max_dotprod; // if normalized 
};

/** Matches objects if a starlane from the location of the candidate and the location
  * of any object that matches \a lane_end_condition would be too close to any object
  * that matches \a close_object_condition. */
struct FO_COMMON_API StarlaneToWouldBeCloseToObject : Condition {
    static constexpr double DEFAULT_MAX_DISTANCE = 20.0;

    explicit StarlaneToWouldBeCloseToObject(
        std::unique_ptr<Condition>&& lane_end_condition,
        std::unique_ptr<Condition>&& close_object_condition =
            std::make_unique<Type>(UniverseObjectType::OBJ_SYSTEM),
        double max_distance = DEFAULT_MAX_DISTANCE);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_lane_end_condition;
    std::unique_ptr<Condition> m_close_object_condition;
    double m_max_distance;
};

/** Matches systems that have been explored by at least one Empire in \a empire_ids. */
struct FO_COMMON_API ExploredByEmpire final : public Condition {
    explicit ExploredByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
};

/** Matches fleets and ships that are not moving. That is, a fleet, or the
  * fleet of a ship, has a next system that is a system and is not its
  * current system. That is, the fleet. */
struct FO_COMMON_API Stationary final : public Condition {
    constexpr Stationary() noexcept : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Stationary() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Stationary&) const noexcept
    { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;
};

/** Matches objects that are aggressive fleets or are in aggressive fleets. */
struct FO_COMMON_API Aggressive final : public Condition {
    constexpr Aggressive() noexcept = default;
    constexpr explicit Aggressive(bool aggressive) noexcept :
        Condition(true, true, true),
        m_aggressive(aggressive)
    {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Aggressive() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Aggressive&) const noexcept
    { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}
    [[nodiscard]] bool GetAggressive() const noexcept { return m_aggressive; }
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    const bool m_aggressive = true; // false to match passive ships/fleets
};

/** Matches objects that are in systems that can be fleet supplied by the
  * empire with id \a empire_id */
struct FO_COMMON_API FleetSupplyableByEmpire final : public Condition {
    explicit FleetSupplyableByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const FleetSupplyableByEmpire& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
};

/** Matches objects that are in systems that are connected by resource-sharing
  * to at least one object that meets \a condition using the resource-sharing
  * network of the empire with id \a empire_id */
struct FO_COMMON_API ResourceSupplyConnectedByEmpire final : public Condition {
    ResourceSupplyConnectedByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                                    std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const ResourceSupplyConnectedByEmpire& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    std::unique_ptr<Condition> m_condition;
};

/** Matches objects whose species has the ability to found new colonies. */
struct FO_COMMON_API CanColonize final : public Condition {
    constexpr CanColonize() : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~CanColonize() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const CanColonize&) const noexcept
    { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string&) noexcept override {}
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;
};

/** Matches objects whose species has the ability to produce ships. */
struct FO_COMMON_API CanProduceShips final : public Condition {
    constexpr CanProduceShips() noexcept : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~CanProduceShips() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const CanProduceShips&) const noexcept
    { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string&) noexcept override {}
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;
};

/** Matches the objects that have been targeted for bombardment by at least one
  * object that matches \a m_by_object_condition. */
struct FO_COMMON_API OrderedBombarded final : public Condition {
    explicit OrderedBombarded(std::unique_ptr<Condition>&& by_object_condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    virtual void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_by_object_condition;
};

/** Matches the objects that have been ordered annexed by an empire. */
struct FO_COMMON_API OrderedAnnexed final : public Condition {
    constexpr OrderedAnnexed() : Condition(true, true, true) {}
#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~OrderedAnnexed() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const OrderedAnnexed&) const noexcept
    { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;
};

/** Matches all objects if the comparisons between values of ValueRefs meet the specified comparison types. */
struct FO_COMMON_API ValueTest final : public Condition {
    ValueTest(std::unique_ptr<ValueRef::ValueRef<double>>&& value_ref1,
              ComparisonType comp1,
              std::unique_ptr<ValueRef::ValueRef<double>>&& value_ref2,
              ComparisonType comp2 = ComparisonType::INVALID_COMPARISON,
              std::unique_ptr<ValueRef::ValueRef<double>>&& value_ref3 = nullptr);

    ValueTest(std::unique_ptr<ValueRef::ValueRef<std::string>>&& value_ref1,
              ComparisonType comp1,
              std::unique_ptr<ValueRef::ValueRef<std::string>>&& value_ref2,
              ComparisonType comp2 = ComparisonType::INVALID_COMPARISON,
              std::unique_ptr<ValueRef::ValueRef<std::string>>&& value_ref3 = nullptr);

    ValueTest(std::unique_ptr<ValueRef::ValueRef<int>>&& value_ref1,
              ComparisonType comp1,
              std::unique_ptr<ValueRef::ValueRef<int>>&& value_ref2,
              ComparisonType comp2 = ComparisonType::INVALID_COMPARISON,
              std::unique_ptr<ValueRef::ValueRef<int>>&& value_ref3 = nullptr);
    ValueTest(const ValueTest& rhs);
    ValueTest(ValueTest&& rhs) = default;

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const ValueTest& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

    [[nodiscard]] std::array<const ValueRef::ValueRef<double>*, 3> ValuesDouble() const noexcept
    { return {m_value_ref1.get(), m_value_ref2.get(), m_value_ref3.get()}; }

    [[nodiscard]] std::array<const ValueRef::ValueRef<std::string>*, 3> ValuesString() const noexcept
    { return {m_string_value_ref1.get(), m_string_value_ref2.get(), m_string_value_ref3.get()}; }

    [[nodiscard]] std::array<const ValueRef::ValueRef<int>*, 3> ValuesInt() const noexcept
    { return {m_int_value_ref1.get(), m_int_value_ref2.get(), m_int_value_ref3.get()}; }

    [[nodiscard]] std::array<ComparisonType, 2> CompareTypes() const noexcept
    { return {m_compare_type1, m_compare_type2}; }

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<double>> m_value_ref1;
    std::unique_ptr<ValueRef::ValueRef<double>> m_value_ref2;
    std::unique_ptr<ValueRef::ValueRef<double>> m_value_ref3;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_string_value_ref1;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_string_value_ref2;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_string_value_ref3;
    std::unique_ptr<ValueRef::ValueRef<int>> m_int_value_ref1;
    std::unique_ptr<ValueRef::ValueRef<int>> m_int_value_ref2;
    std::unique_ptr<ValueRef::ValueRef<int>> m_int_value_ref3;

    ComparisonType m_compare_type1 = ComparisonType::INVALID_COMPARISON;
    ComparisonType m_compare_type2 = ComparisonType::INVALID_COMPARISON;

    const bool m_refs_local_invariant;
    const bool m_no_refs12_comparable;
};

/** Matches objects that match the location condition of the specified content.  */
struct FO_COMMON_API Location final : public Condition {
public:
    Location(ContentType content_type,
             std::unique_ptr<ValueRef::ValueRef<std::string>>&& name1,
             std::unique_ptr<ValueRef::ValueRef<std::string>>&& name2 = nullptr);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name1;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name2;
    const ContentType                                m_content_type;
};

/** Matches objects that match the combat targeting condition of the specified content.  */
struct FO_COMMON_API CombatTarget final : public Condition {
public:
    CombatTarget(ContentType content_type,
                 std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, ScriptingContext::LocalCandidate{}, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    const ContentType                                m_content_type;
};

/** Matches all objects that match every Condition in \a operands. */
struct FO_COMMON_API And final : public Condition {
    explicit And(std::vector<std::unique_ptr<Condition>>&& operands);
    And(std::unique_ptr<Condition>&& operand1,
        std::unique_ptr<Condition>&& operand2,
        std::unique_ptr<Condition>&& operand3 = nullptr,
        std::unique_ptr<Condition>&& operand4 = nullptr,
        std::unique_ptr<Condition>&& operand5 = nullptr,
        std::unique_ptr<Condition>&& operand6 = nullptr,
        std::unique_ptr<Condition>&& operand7 = nullptr,
        std::unique_ptr<Condition>&& operand8 = nullptr);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override;
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] std::vector<const Condition*> OperandsRaw() const;
    [[nodiscard]] auto& Operands() noexcept { return m_operands; }
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    std::vector<std::unique_ptr<Condition>> m_operands;
};

/** Matches all objects that match at least one Condition in \a operands. */
struct FO_COMMON_API Or final : public Condition {
    explicit Or(std::vector<std::unique_ptr<Condition>>&& operands);
    Or(std::unique_ptr<Condition>&& operand1,
       std::unique_ptr<Condition>&& operand2,
       std::unique_ptr<Condition>&& operand3 = nullptr,
       std::unique_ptr<Condition>&& operand4 = nullptr);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override;
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    virtual void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] std::vector<const Condition*> OperandsRaw() const;
    [[nodiscard]] auto& Operands() noexcept { return m_operands; }
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    std::vector<std::unique_ptr<Condition>> m_operands;
};

/** Matches all objects that do not match the Condition \a operand. */
struct FO_COMMON_API Not final : public Condition {
    explicit Not(std::unique_ptr<Condition>&& operand);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override;

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    std::unique_ptr<Condition> m_operand;
};

/** Tests conditions in \a operands in order, to find the first condition that
  * matches at least one candidate object. Matches all objects that match that
  * condition, ignoring any conditions listed later. If no candidate matches
  * any of the conditions, it matches nothing. */
struct FO_COMMON_API OrderedAlternativesOf final : public Condition {
    explicit OrderedAlternativesOf(std::vector<std::unique_ptr<Condition>>&& operands);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] std::vector<const Condition*> Operands() const;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    std::vector<std::unique_ptr<Condition>> m_operands;
};

/** Matches whatever its subcondition matches, but has a customized description
  * string that is returned by Description() by looking up in the stringtable. */
struct FO_COMMON_API Described final : public Condition {
    Described(std::unique_ptr<Condition>&& condition, std::string desc_stringtable_key);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override;
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override
    { return m_condition->GetDefaultInitialCandidateObjects(parent_context); }
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override
    { return m_condition ? m_condition->Dump(ntabs) : ""; }
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    std::unique_ptr<Condition> m_condition;
    std::string m_desc_stringtable_key;
};
}


#endif
