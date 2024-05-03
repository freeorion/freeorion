#ifndef _Conditions_h_
#define _Conditions_h_


#include <memory>
#include <string>
#include <vector>
#include "ConditionAll.h"
#include "ConditionSource.h"
#include "Condition.h"
#include "EnumsFwd.h"
#include "ValueRefs.h"
#include "../Empire/ProductionQueue.h"
#include "../util/CheckSums.h"
#include "../util/Export.h"

const std::string& UserString(const std::string_view);

#if !defined(CONSTEXPR_STRING)
#  if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 11))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
#    define CONSTEXPR_STRING constexpr
#  else
#    define CONSTEXPR_STRING
#  endif
#endif

#if !defined(CONSTEXPR_VEC)
#  if defined(__cpp_lib_constexpr_vector)
#    define CONSTEXPR_VEC constexpr
#  else
#    define CONSTEXPR_VEC
#  endif
#endif

namespace ValueRef {
    template <typename T>
    struct ValueRef;
}

/** this namespace holds Condition and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
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


void AddAllPlanetsSet(const ObjectMap& objects, ObjectSet& in_out);
void AddAllBuildingsSet(const ObjectMap& objects, ObjectSet& in_out);


template <class Cond>
concept is_condition = std::is_base_of_v<Condition, Cond>;

using upc = std::unique_ptr<Condition>;

// flattens nested conditions by extracting contained operands of the same type.
// also removes null operands
template <is_condition ContainingCondition>
std::vector<upc> DenestOps(auto&& in) requires(requires { {*in.begin()} -> std::same_as<upc&>; })
{
    std::vector<upc> retval;
    retval.reserve(in.size()); // may be underestimate if in has nested and/or conditions

    static constexpr auto not_null = [](auto& op) noexcept -> bool { return !!op; };
    for (upc& op : in | range_filter(not_null)) {
        if (auto* op_ = dynamic_cast<ContainingCondition*>(op.get())) {
            ContainingCondition& op_and_ref = *op_;

            auto sub_ops = std::move(op_and_ref).Operands();
            auto denested_sub_ops = DenestOps<ContainingCondition>(sub_ops);

            retval.insert(retval.end(), std::make_move_iterator(denested_sub_ops.begin()),
                            std::make_move_iterator(denested_sub_ops.end()));
        } else {
            retval.push_back(std::move(op));
        }
    };
    return retval;
}

std::vector<upc> Vectorize(upc&& op1, upc&& op2, upc&& op3, upc&& op4, upc&& op5 = nullptr,
                           upc&& op6 = nullptr, upc&& op7 = nullptr, upc&& op8 = nullptr);

constexpr auto conds_rtsi = [](const auto&... conds) noexcept -> std::array<bool, 3> {
    return {(conds.RootCandidateInvariant() && ...),
            (conds.TargetInvariant() && ...),
            (conds.SourceInvariant() && ...)};
};

constexpr std::array<bool, 3> CondsRTSI(const auto& operands) {
    if constexpr (requires { *operands; operands->TargetInvariant(); }) {
        return {!operands || operands->RootCandidateInvariant(),
                !operands || operands->TargetInvariant(),
                !operands || operands->SourceInvariant()};

    } else if constexpr (requires { operands.TargetInvariant(); }) {
        return {operands.RootCandidateInvariant(), operands.TargetInvariant(), operands.SourceInvariant()};

    } else if constexpr (requires { operands.begin(); operands.begin()->TargetInvariant(); }) {
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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const None&) const noexcept { return true; }
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet&) const noexcept override { return false; }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override
    { return {}; /* efficient rejection of everything. */ }
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}
    [[nodiscard]] constexpr uint32_t GetCheckSum() const
        noexcept(noexcept(CheckSums::CheckSumCombine(std::declval<uint32_t&>(), ""))) override
    { return CheckSums::GetCheckSum("Condition::None"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;
};

/** Does not modify the input ObjectSets. */
struct FO_COMMON_API NoOp final : public Condition {
    constexpr NoOp() noexcept : Condition(true, true, true) {}
    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const NoOp&) const noexcept { return true; }
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override; // no noexcept due to logging
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::NoOp"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;
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
    { return Match(ScriptingContext{parent_context, candidate}); }

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

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const RootCandidate&) const noexcept { return true; }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const noexcept override {
        return candidate &&
            (!parent_context.condition_root_candidate || parent_context.condition_root_candidate == candidate);
    }

    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::RootCandidate"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const noexcept override;
};

/** There is no LocalCandidate condition. To match any local candidate object,
  * use the All condition. */

/** Matches the target of an effect being executed. */
struct FO_COMMON_API Target final : public Condition {
    constexpr Target() noexcept : Condition(true, false, true) {}
    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Target&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const noexcept override
    { return Match(ScriptingContext{parent_context, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) noexcept override {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::Target"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const noexcept override;
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
    { return Match(ScriptingContext{parent_context, candidate}); }
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

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Capital&) const noexcept
    { return true; }
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override; // no noexcept due to logging
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const override; // no noexcept due to logging
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Monster&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Armed&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
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
    { return Match(ScriptingContext{parent_context, candidate}); }

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

/** Matches all Building objects that are one of the building types specified
  * in \a names. */
struct FO_COMMON_API Building final : public Condition {
    explicit Building(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const Building& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> m_names;
    const bool m_names_local_invariant;
};

/** Matches all Field objects that are one of the field types specified
  * in \a names. */
struct FO_COMMON_API Field final : public Condition {
    explicit Field(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    HasTag() noexcept :
        Condition(true, true, true)
    {}
    explicit HasTag(std::string name);
    explicit HasTag(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const HasTag& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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

/** Matches all objects that contain an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API Contains final : public Condition {
    explicit Contains(std::unique_ptr<Condition>&& condition);

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const Contains& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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

/** Matches all objects that are contained by an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API ContainedBy final : public Condition {
    explicit ContainedBy(std::unique_ptr<Condition>&& condition);
    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] bool operator==(const ContainedBy& rhs) const;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    OnPlanet() noexcept : Condition(true, true, true, false) {}
    OnPlanet(OnPlanet&&) noexcept = default;

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const OnPlanet& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const ObjectID& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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
  * Note that all Building objects which are on matching planets are also matched. */
struct FO_COMMON_API PlanetType : public Condition {
    constexpr PlanetType(std::array<bool, 3> rts_invariants) noexcept : Condition(rts_invariants) {}

    // gets a planet from \a obj considering obj as a planet or a building on a planet
    static const ::Planet* PlanetFromObject(const UniverseObject* obj, const ObjectMap& objects);
    static ::PlanetType PlanetTypeFromPlanet(const Planet* planet) noexcept;

    template <std::size_t span_extent>
    struct SimpleMatch {
        using SpanT = std::conditional_t<span_extent == 1, ::PlanetType, std::span<const ::PlanetType, span_extent>>;

        SimpleMatch(SpanT types, const ObjectMap& objects) noexcept :
            m_types(types),
            m_objects(objects)
        {}

        SimpleMatch(std::span<::PlanetType, 1> types, const ObjectMap& objects) noexcept requires (span_extent == 1) :
            m_types(types.front()),
            m_objects(objects)
        {}

        bool operator()(::PlanetType pt) const {
            if constexpr (span_extent == 1)
                return pt == m_types;
            else
                return std::any_of(m_types.begin(), m_types.end(), [pt](const auto t) noexcept { return t == pt; });
        }

        bool operator()(const ::Planet* candidate) const
        { return candidate && operator()(PlanetType::PlanetTypeFromPlanet(candidate)); }

        bool operator()(const UniverseObject* candidate) const
        { return operator()(PlanetType::PlanetFromObject(candidate, m_objects)); }

        const SpanT m_types;
        const ObjectMap& m_objects;
    };

    SimpleMatch(::PlanetType, const ObjectMap&) -> SimpleMatch<1>;
    template <std::size_t span_extent>
    SimpleMatch(std::span<::PlanetType, span_extent>, const ObjectMap&) -> SimpleMatch<span_extent>;
    template <std::size_t span_extent>
    SimpleMatch(std::span<const ::PlanetType, span_extent>, const ObjectMap&) -> SimpleMatch<span_extent>;

    static std::string_view to_string(::PlanetType) noexcept;
};


using up_vref_pt = std::unique_ptr<ValueRef::ValueRef<::PlanetType>>;

template <std::size_t N = 0, typename ValuesT = ::PlanetType>
    requires (std::is_same_v<ValuesT, ::PlanetType> ||
              std::is_same_v<ValuesT, up_vref_pt>)
struct FO_COMMON_API PlanetTypes final : public PlanetType {
    static constexpr bool have_specific_pts = std::is_same_v<ValuesT, ::PlanetType>;
    static constexpr bool have_fixed_number = N != 0;

    using PlanetTypesT = std::conditional_t<have_fixed_number,
                                            std::conditional_t<N == 1, ValuesT, std::array<ValuesT, N>>,
                                            std::vector<ValuesT>>;

    PlanetTypes(PlanetTypesT types) :
        PlanetType([&types]() {
            if constexpr (have_specific_pts)
                return std::array{true, true, true};
            else
                return CondsRTSI(types);
        }()),
        m_types(std::move(types))
    {}

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_and = dynamic_cast<decltype(this)>(&rhs);
        return rhs_and && *this == *rhs_and;
    }
    [[nodiscard]] constexpr bool operator==(const PlanetTypes& rhs) const {
        if (this == &rhs || m_types == rhs.m_types)
            return true;
        // can be (array or vec) of (PlanetType or ValueRef<PlanetType>)

        if constexpr (have_specific_pts) {
            return false;

        } else {
            // have ValueRefs; need to inspect number and deep equality of each
            if (m_types.size() != rhs.m_types.size())
                return false;

            for (std::size_t i = 0; i < m_types.size(); ++i) {
                const auto& m_ptr = m_types[i];
                const auto& rhs_ptr = rhs.m_types[i];
                if (m_ptr == rhs_ptr)   // two unique_ptr both point to null
                    continue;
                if (!m_ptr || !rhs_ptr || *m_ptr != *(rhs_ptr))
                    return false;       // only one null, or deep inspection of ValueRefs found difference
            }
            return true;
        }
    };

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override
    {
        if constexpr (have_specific_pts) {
            if constexpr (N == 1) {
                const auto simple_match = PlanetType::SimpleMatch<1>(m_types, parent_context.ContextObjects());
                EvalImpl(matches, non_matches, search_domain, simple_match);
            } else {
                // make a span of the allowed PlanetTypes, test each candidate...
                const auto simple_match = PlanetType::SimpleMatch(std::span(m_types), parent_context.ContextObjects());
                EvalImpl(matches, non_matches, search_domain, simple_match);
            }

        } else {
            static constexpr auto is_lci = [](const auto& type) noexcept { return type->LocalCandidateInvariant(); };
            bool simple_eval_safe = (parent_context.condition_root_candidate || RootCandidateInvariant()) &&
                std::all_of(m_types.begin(), m_types.end(), is_lci);

            if (simple_eval_safe) {
                // Evaluate refs to determine matched PlanetTypes, test on each candidate
                const auto ref_to_type = [&parent_context](const auto& tref) { return tref->Eval(parent_context); };

                // evaluate types once, and use to check all candidate objects
                const auto simple_match = [&parent_context, this, &ref_to_type]() {
                    if constexpr (have_fixed_number) {
                        if constexpr (N == 1) {
                            return PlanetType::SimpleMatch(ref_to_type(m_types), parent_context.ContextObjects());
                        } else {
                            std::array< ::PlanetType, m_types.size()> types{};
                            std::transform(m_types.begin(), m_types.end(), types.begin(), ref_to_type);
                            std::span span(std::as_const(types));
                            return PlanetType::SimpleMatch{span, parent_context.ContextObjects()};
                        }
                    } else {
                        std::vector<::PlanetType> types;
                        types.reserve(m_types.size());
                        std::transform(m_types.begin(), m_types.end(), std::back_inserter(types), ref_to_type);
                        std::span span(std::as_const(types));
                        return PlanetType::SimpleMatch{span, parent_context.ContextObjects()};
                    }
                }();
                EvalImpl(matches, non_matches, search_domain, simple_match);

            } else {
                // re-evaluate contained objects for each candidate object
                Condition::Eval(parent_context, matches, non_matches, search_domain);
            }
        }
    }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override {
        // objects that are or are on planets: Building, Planet
        ObjectSet retval;
        retval.reserve(parent_context.ContextObjects().size<::Planet>() +
                       parent_context.ContextObjects().size<::Building>());
        AddAllPlanetsSet(parent_context.ContextObjects(), retval);
        AddAllBuildingsSet(parent_context.ContextObjects(), retval);
        return retval;
    }

    [[nodiscard]] std::string Description(bool negated = false) const override {
        std::string values_str;
        if constexpr (N == 1) {
            if constexpr (have_specific_pts) 
                values_str += UserString(to_string(m_types));
            else
                values_str += m_types->ConstantExpr() ? UserString(to_string(m_types->Eval())) : m_types->Description();

        } else {
            for (std::size_t i = 0; i < m_types.size(); ++i) {
                if constexpr (have_specific_pts) {
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
        }
        return str(FlexibleFormat((!negated)
                                  ? UserString("DESC_PLANET_TYPE")
                                  : UserString("DESC_PLANET_TYPE_NOT"))
                   % values_str);
    }
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override {
        std::string retval = DumpIndent(ntabs) + "Planet type = ";
        if constexpr (have_specific_pts) {
            // TODO: this
        } else {
            if (m_types.size() == 1) {
                retval += m_types[0]->Dump(ntabs) + "\n";
            } else {
                retval += "[ ";
                for (auto& type : m_types)
                    retval += type->Dump(ntabs) + " ";
                retval += "]\n";
            }
        }
        return retval;
    }

    void SetTopLevelContent(const std::string& content_name) noexcept(have_specific_pts) override {
        if constexpr (!have_specific_pts) {
            for (auto& type : m_types) {
                if (type)
                    type->SetTopLevelContent(content_name);
            }
        }
    }

    [[nodiscard]] uint32_t GetCheckSum() const
        noexcept(noexcept(CheckSums::GetCheckSum("")) &&
                 noexcept(CheckSums::CheckSumCombine(std::declval<uint32_t&>(), m_types))) override
    {
        uint32_t retval = CheckSums::GetCheckSum("Condition::PlanetType");
        CheckSums::CheckSumCombine(retval, m_types);
        return retval;
    }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override {
        if constexpr (have_specific_pts) {  // array or vector of ::PlanetType
            return std::make_unique<std::decay_t<decltype(*this)>>(m_types);

        } else {    // array or vector of unique_ptr<ValueRef>
            decltype(m_types) types{};
            if constexpr (!have_fixed_number) // vector
                types.reserve(m_types.size());
            for (auto idx = 0; idx < m_types.size(); ++idx)
                types[idx] = m_types[idx]->Clone();
            return std::make_unique<std::decay_t<decltype(*this)>>(std::move(types));
        }
    }

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const override {
        const auto* candidate = local_context.condition_local_candidate;
        if (!candidate)
            return false;
        const Planet* planet = PlanetType::PlanetFromObject(candidate, local_context.ContextObjects());
        if (!planet)
            return false;
        const ::PlanetType pt = PlanetType::PlanetTypeFromPlanet(planet);

        // test candidate's planet type against all matched types or results of refs for matched types
        if constexpr (have_specific_pts) {
            if constexpr (N == 1) {
                return pt == m_types;
            } else {
                const auto is_pt = [pt](const auto pt_test) noexcept { return pt_test == pt; };
                return std::any_of(m_types.begin(), m_types.end(), is_pt);
            }
        } else {
            if constexpr (N == 1) {
                return m_types && m_types->Eval(local_context) == pt;
            } else {
                const auto is_pt = [pt, &local_context](const auto& pt_ref)
                { return pt_ref && pt_ref->Eval(local_context) == pt; };
                return std::any_of(m_types.begin(), m_types.end(), is_pt);
            }
        }
    }

    PlanetTypesT m_types;
};

PlanetTypes(std::vector<::PlanetType>) -> PlanetTypes<0, ::PlanetType>;
template <std::size_t N>
PlanetTypes(std::array<::PlanetType, N>) -> PlanetTypes<N, ::PlanetType>;
PlanetTypes(std::vector<up_vref_pt>) -> PlanetTypes<0, up_vref_pt>;
template <std::size_t N>
PlanetTypes(std::array<up_vref_pt, N>) -> PlanetTypes<N, up_vref_pt>;
PlanetTypes(::PlanetType) -> PlanetTypes<1>;


/** Matches all Planet objects that have one of the PlanetSizes in \a sizes.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API PlanetSize final : public Condition {
    PlanetSize(std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>>&& sizes);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    StarType(std::vector<std::unique_ptr<ValueRef::ValueRef< ::StarType>>>&& types);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const StarType& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }
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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    [[nodiscard]] bool operator==(const OwnerHasTech& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Stationary&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Aggressive&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const CanColonize&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const CanProduceShips&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const OrderedAnnexed&) const noexcept
    { return true; }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    { return Match(ScriptingContext{parent_context, candidate}); }

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
    CombatTarget(ContentType content_type, std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    [[nodiscard]] bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override
    { return Match(ScriptingContext{parent_context, candidate}); }

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

struct FO_COMMON_API And : public Condition {
    [[nodiscard]] virtual std::vector<const Condition*> OperandsRaw() const = 0;
    [[nodiscard]] virtual std::vector<std::unique_ptr<Condition>> Operands() && = 0;
    [[nodiscard]] constexpr virtual std::size_t OperandCount() const noexcept = 0;
    constexpr And(std::array<bool, 3> rts_invariants) noexcept : Condition(rts_invariants) {}
};

/** Matches all objects that match every Condition in \a operands. */
template <class... ConditionTs>
    requires ((std::is_base_of_v<Condition, ConditionTs> && ...))
struct FO_COMMON_API AndTuple final : public And {
    static constexpr auto N = sizeof...(ConditionTs);
    static_assert(N > 1);
    using ConditionTupleT = std::tuple<ConditionTs...>;
    static_assert(std::tuple_size_v<ConditionTupleT> == N);

    constexpr AndTuple(ConditionTupleT&& operands):
        And(std::apply(conds_rtsi, operands)),
        m_operands(std::forward<decltype(operands)>(operands))
    {}

    constexpr AndTuple(ConditionTs&&... operands) :
        And(conds_rtsi(operands...)),
        m_operands(std::forward<decltype(operands)>(operands)...)
    {}

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }

    [[nodiscard]] constexpr bool operator==(const AndTuple& rhs) const {
        static_assert(std::is_same_v<decltype(this), decltype(&rhs)>);
        return (this == &rhs) || m_operands == rhs.m_operands;
    }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override
    {
        const auto eval_conds =
            [&parent_context, &matches, &non_matches, search_domain](const auto& cond0, const auto&... conds)
        {
            if (search_domain == SearchDomain::MATCHES) {
                const auto eval_cond_not_empty = [&parent_context, &matches, &non_matches](const Condition& cond) {
                    cond.Eval(parent_context, matches, non_matches, SearchDomain::MATCHES);
                    return !matches.empty();
                };
                eval_cond_not_empty(cond0) && (eval_cond_not_empty(conds) && ...);

            } else /* search_domain == SearchDomain::NON_MATCHES */{
                ObjectSet partly_checked_non_matches;
                partly_checked_non_matches.reserve(non_matches.size());

                cond0.Eval(parent_context, partly_checked_non_matches, non_matches, SearchDomain::NON_MATCHES);

                const auto eval_cond_not_empty =
                    [&parent_context, &partly_checked_non_matches, &non_matches](const Condition& cond)
                {
                    cond.Eval(parent_context, partly_checked_non_matches, non_matches, SearchDomain::MATCHES);
                    return !partly_checked_non_matches.empty();
                };

                !partly_checked_non_matches.empty() && (eval_cond_not_empty(conds) && ...);
                matches.insert(matches.end(), partly_checked_non_matches.begin(), partly_checked_non_matches.end());
            }
        };

        std::apply(eval_conds, m_operands);
    }

    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override {
        if (candidates.empty() || candidates.size() == 1 && !candidates.front())
            return false;
        ObjectSet matches{candidates};
        ObjectSet non_matches;
        non_matches.reserve(candidates.size());

        const auto eval_cond_on_candidates = [&parent_context, &matches, &non_matches](const auto& cond) {
            cond.Eval(parent_context, matches, non_matches, SearchDomain::MATCHES);
            return !matches.empty();
        };

        const auto eval_conds_on_candidates = [&eval_cond_on_candidates](const auto&... conds)
        { return (eval_cond_on_candidates(conds) && ...); };

        return std::apply(eval_conds_on_candidates, m_operands);
    }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override {
        if (!candidate)
            return false;

        const auto eval_cond_on_candidate = [&parent_context, candidate](const auto& cond)
        { return cond.EvalOne(parent_context, candidate); };

        const auto eval_conds_on_candidate = [&eval_cond_on_candidate](const auto&... conds)
        { return (eval_cond_on_candidate(conds) && ...); };

        return std::apply(eval_conds_on_candidate, m_operands);
    }

    [[nodiscard]] CONSTEXPR_VEC ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override
    { return std::get<0>(m_operands).GetDefaultInitialCandidateObjects(parent_context); }

    [[nodiscard]] CONSTEXPR_STRING std::string Description(bool negated = false) const override { return {}; }
    [[nodiscard]] CONSTEXPR_STRING std::string Dump(uint8_t ntabs = 0) const override { return {}; }

    CONSTEXPR_STRING void SetTopLevelContent(const std::string& content_name) override {
        const auto set_tlc = [&content_name](auto& op)
        { op.SetTopLevelContent(content_name); };

        const auto set_tlcs = [&set_tlc](auto&... ops)
        { (set_tlc(ops), ...); };

        std::apply(set_tlcs, m_operands);
    }

    [[nodiscard]] CONSTEXPR_VEC std::vector<const Condition*> OperandsRaw() const override {
        std::vector<const Condition*> retval;
        retval.reserve(N);
        const auto to_ptrs = [&retval](const auto&... ops) { ((retval.push_back(&ops)), ...); };
        std::apply(to_ptrs, m_operands);
        return retval;
    }

    [[nodiscard]] constexpr const auto& OperandsTuple() noexcept { return m_operands; }

    [[nodiscard]] std::vector<std::unique_ptr<Condition>> Operands() && override {
        std::vector<std::unique_ptr<Condition>> retval;
        retval.reserve(N);

        static constexpr auto to_unique_ptr = [](auto& cond)
        { return std::make_unique<std::decay_t<decltype(cond)>>(std::move(cond)); };

        const auto to_ptrs = [&retval](auto&... ops)
        { ((retval.push_back(to_unique_ptr(ops))), ...); };

        std::apply(to_ptrs, m_operands);
        return retval;
    }

    [[nodiscard]] constexpr std::size_t OperandCount() const noexcept override { return N; }

    [[nodiscard]] constexpr uint32_t GetCheckSum() const {
        uint32_t retval{0};

        CheckSums::CheckSumCombine(retval, "Condition::And");
        CheckSums::CheckSumCombine(retval, m_operands);

        return retval;
    }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override {
        using zeroth_operand_t = std::tuple_element_t<0, ConditionTupleT>;

        static constexpr auto clone_cast_op_type_rval = [](const auto& op) -> auto&& {
            static_assert(requires { op.Clone(); } );
            using OperandT = std::decay_t<decltype(op)>;
            auto cond_clone_unique = op.Clone();
            OperandT& ref = *static_cast<OperandT*>(cond_clone_unique.release());
            return std::move(ref);
        };
        using get0_operand_t = decltype(std::get<0>(m_operands));
        static_assert(std::is_same_v<get0_operand_t, const zeroth_operand_t&>);
        using clone_cast0_operand_t = decltype(clone_cast_op_type_rval(std::get<0>(m_operands)));
        static_assert(std::is_same_v<clone_cast0_operand_t, zeroth_operand_t&&>);

        static constexpr auto clone_operands = [](const auto&... ops)
        { return std::make_tuple(clone_cast_op_type_rval(ops)...); };

        ConditionTupleT cloned_operands = std::apply(clone_operands, m_operands);
        return std::make_unique<std::decay_t<decltype(*this)>>(std::move(cloned_operands));
    }

private:
    ConditionTupleT m_operands;
};

template <typename... Ts>
AndTuple(Ts...) -> AndTuple<std::decay_t<Ts>...>;
template <typename... Ts>
AndTuple(std::tuple<Ts...>&&) -> AndTuple<std::decay_t<Ts>...>;

template <std::size_t N = 0>
struct FO_COMMON_API AndPtrs final : public And {
    using ConditionsT = std::conditional_t<N == 0, std::vector<std::unique_ptr<Condition>>,
                                                   std::array<std::unique_ptr<Condition>, N>>;

    explicit AndPtrs(ConditionsT&& operands) requires(N == 0) :
        And(CondsRTSI(operands)),
        m_operands(DenestOps<And>(operands)) // if container is of flexible size, can edit input before storing
    {}

    explicit AndPtrs(ConditionsT&& operands) requires(N != 0) :
        And(CondsRTSI(operands)),
        m_operands(std::move(operands))
    {}

    [[nodiscard]] bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_and = dynamic_cast<decltype(this)>(&rhs);
        return rhs_and && *this == *rhs_and;
    }
    [[nodiscard]] bool operator==(const AndPtrs& rhs) const {
        if (this == &rhs)
            return true;
        if (m_operands.size() != rhs.m_operands.size())
            return false;
        for (std::size_t i = 0; i < m_operands.size(); ++i) {
            const auto& m_ptr = m_operands[i];
            const auto& rhs_ptr = rhs.m_operands[i];
            if (m_ptr == rhs_ptr)
                continue;
            if (!m_ptr || !rhs_ptr || *m_ptr != *(rhs_ptr))
                return false;
        }
        return true;
    }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override
    {
        if (m_operands.empty())
            return;

        if (search_domain == SearchDomain::NON_MATCHES) {
            ObjectSet partly_checked_non_matches;
            partly_checked_non_matches.reserve(non_matches.size());

            // move items in non_matches set that pass first operand condition into
            // partly_checked_non_matches set
            m_operands[0]->Eval(parent_context, partly_checked_non_matches, non_matches, SearchDomain::NON_MATCHES);

            // move items that don't pass one of the other conditions back to non_matches
            for (std::size_t i = 1; i < m_operands.size(); ++i) {
                if (partly_checked_non_matches.empty()) break;
                m_operands[i]->Eval(parent_context, partly_checked_non_matches, non_matches, SearchDomain::MATCHES);
            }

            // merge items that passed all operand conditions into matches
            matches.insert(matches.end(), partly_checked_non_matches.begin(),
                           partly_checked_non_matches.end());

            // items already in matches set are not checked, and remain in matches set even if
            // they don't match one of the operand conditions

        } else /*(search_domain == SearchDomain::MATCHES)*/ {
            // check all operand conditions on all objects in the matches set, moving those
            // that don't pass a condition to the non-matches set

            for (const auto& operand : m_operands) {
                if (matches.empty()) break;
                operand->Eval(parent_context, matches, non_matches, SearchDomain::MATCHES);
            }

            // items already in non_matches set are not checked, and remain in non_matches set
            // even if they pass all operand conditions
        }
    }

    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override {
        if (m_operands.empty() || candidates.empty())
            return false;
        if (candidates.size() == 1 && !candidates.front())
            return false;

        ObjectSet matches{candidates};
        ObjectSet non_matches;
        non_matches.reserve(candidates.size());
        // check candidates against all operands and return true if any candidate is matched by all operands
        for (const auto& op : m_operands) {
            op->Eval(parent_context, matches, non_matches, SearchDomain::MATCHES);
            if (matches.empty())
                return false;
        }
        return true;
    }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override {
        return candidate &&
            std::all_of(m_operands.begin(), m_operands.end(),
                        [candidate, &parent_context](const auto& op)
                        { return op->EvalOne(parent_context, candidate); });
    }

    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override {
        if (m_operands.empty()) [[unlikely]]
            return {};
        return m_operands.front()->GetDefaultInitialCandidateObjects(parent_context);
    }

    [[nodiscard]] std::string Description(bool negated = false) const override {
        std::string values_str;
        if (m_operands.size() == 1) {
            values_str += (!negated)
                ? UserString("DESC_AND_BEFORE_SINGLE_OPERAND")
                : UserString("DESC_NOT_AND_BEFORE_SINGLE_OPERAND");
            // Pushing the negation to the enclosed conditions
            values_str += m_operands[0]->Description(negated);
            values_str += (!negated)
                ? UserString("DESC_AND_AFTER_SINGLE_OPERAND")
                : UserString("DESC_NOT_AND_AFTER_SINGLE_OPERAND");
        } else {
            values_str += (!negated)
                ? UserString("DESC_AND_BEFORE_OPERANDS")
                : UserString("DESC_NOT_AND_BEFORE_OPERANDS");
            for (std::size_t i = 0; i < m_operands.size(); ++i) {
                // Pushing the negation to the enclosed conditions
                values_str += m_operands[i]->Description(negated);
                if (i != m_operands.size() - 1) {
                    values_str += (!negated)
                        ? UserString("DESC_AND_BETWEEN_OPERANDS")
                        : UserString("DESC_NOT_AND_BETWEEN_OPERANDS");
                }
            }
            values_str += (!negated)
                ? UserString("DESC_AND_AFTER_OPERANDS")
                : UserString("DESC_NOT_AND_AFTER_OPERANDS");
        }
        return values_str;
    }
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override {
        std::string retval = DumpIndent(ntabs) + "And [\n";
        for (auto& operand : m_operands)
            retval += operand->Dump(ntabs+1);
        retval += DumpIndent(ntabs) + "]\n";
        return retval;
    }

    void SetTopLevelContent(const std::string& content_name) override {
        for (auto& operand : m_operands)
            operand->SetTopLevelContent(content_name);
    }

    [[nodiscard]] std::vector<const Condition*> OperandsRaw() const {
        std::vector<const Condition*> retval;
        retval.reserve(m_operands.size());
        std::transform(m_operands.begin(), m_operands.end(), std::back_inserter(retval),
                       [](auto& xx) {return xx.get();});
        return retval;
    }

    [[nodiscard]] std::vector<std::unique_ptr<Condition>> Operands() && override {
        using RetvalT = std::vector<std::unique_ptr<Condition>>;
        if constexpr (std::is_same_v<ConditionsT, RetvalT>)
            return std::move(m_operands);
        else
            return RetvalT(std::make_move_iterator(m_operands.begin()), std::make_move_iterator(m_operands.end()));
    }

    [[nodiscard]] constexpr std::size_t OperandCount() const noexcept override {
        if constexpr (N == 0)
            return m_operands.size();
        else
            return N;
    }

    [[nodiscard]] uint32_t GetCheckSum() const override {
        uint32_t retval{0};

        CheckSums::CheckSumCombine(retval, "Condition::And");
        CheckSums::CheckSumCombine(retval, m_operands);

        return retval;
    }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override
    { return std::make_unique<std::decay_t<decltype(*this)>>(ValueRef::CloneUnique(m_operands)); }

private:
    ConditionsT m_operands;
};

template <typename... Ts> requires((std::is_base_of_v<Condition, Ts>&& ...))
AndPtrs(Ts...) -> AndPtrs<sizeof...(Ts)>;
template <typename... Ts>
AndPtrs(std::tuple<Ts...>&&) -> AndPtrs<sizeof...(Ts)>;

/** Matches all objects that match at least one Condition in \a operands. */
struct FO_COMMON_API OrBase : public Condition {
    [[nodiscard]] virtual std::vector<const Condition*> OperandsRaw() const = 0;
    [[nodiscard]] virtual std::vector<std::unique_ptr<Condition>> Operands() && = 0;
    [[nodiscard]] constexpr virtual std::size_t OperandCount() const noexcept = 0;
    constexpr OrBase(std::array<bool, 3> rts_invariants) noexcept : Condition(rts_invariants) {}
};

template <class... ConditionTs>
    requires ((std::is_base_of_v<Condition, ConditionTs> && ...))
struct FO_COMMON_API Or final : public OrBase {
    static constexpr auto N = sizeof...(ConditionTs);
    static_assert(N > 1);
    using ConditionTupleT = std::tuple<ConditionTs...>;

    constexpr explicit Or(ConditionTupleT&& operands) noexcept :
        OrBase(std::apply(conds_rtsi, operands)),
        m_operands(std::move(operands))
    {}

    constexpr explicit Or(ConditionTs&&... operands) :
        Or(ConditionTupleT(std::forward<decltype(operands)>(operands)...))
    {}

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_and = dynamic_cast<decltype(this)>(&rhs);
        return rhs_and && m_operands == rhs_and->m_operands;
    }

    [[nodiscard]] constexpr bool operator==(const Or& rhs) const {
        static_assert(std::is_same_v<decltype(this), decltype(&rhs)>);
        return (this == &rhs) || m_operands == rhs.m_operands;
    }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override
    {
        const auto eval_conds =
            [&parent_context, &matches, &non_matches, search_domain](const auto& cond0, const auto&... conds)
        {
            if (search_domain == SearchDomain::NON_MATCHES) {
                // apply operand conditions to NON_MATCHES until nothing is left to test
                const auto eval_cond_not_empty = [&parent_context, &matches, &non_matches](const Condition& cond) {
                    cond.Eval(parent_context, matches, non_matches, SearchDomain::NON_MATCHES);
                    return !non_matches.empty();
                };
                eval_cond_not_empty(cond0) && (eval_cond_not_empty(conds) && ...);

            } else /* search_domain == SearchDomain::MATCHES */{
                ObjectSet partly_checked_matches;
                partly_checked_matches.reserve(matches.size());

                // apply first condition to matches objects
                cond0.Eval(parent_context, matches, partly_checked_matches, SearchDomain::MATCHES);

                // apply additional conditions to partly checked matches until nothing is left to test
                const auto eval_cond_not_empty =
                    [&parent_context, &matches, &partly_checked_matches](const Condition& cond)
                {
                    cond.Eval(parent_context, matches, partly_checked_matches, SearchDomain::NON_MATCHES);
                    return !partly_checked_matches.empty();
                };

                !partly_checked_matches.empty() && (eval_cond_not_empty(conds) && ...);
                matches.insert(matches.end(), partly_checked_matches.begin(), partly_checked_matches.end());
            }
        };

        std::apply(eval_conds, m_operands);
    }

    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override {
        if (candidates.empty() || candidates.size() == 1 && !candidates.front())
            return false;

        const auto eval_cond_on_candidates = [&parent_context, &candidates](const auto& cond)
        { return cond.EvalAny(parent_context, candidates); };

        const auto eval_conds_on_candidates = [&eval_cond_on_candidates](const auto&... conds)
        { return (eval_cond_on_candidates(conds) || ...); };

        return std::apply(eval_conds_on_candidates, m_operands);
    }

    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override {
        if (!candidate)
            return false;

        const auto eval_cond_on_candidate = [&parent_context, candidate](const auto& cond)
        { return cond.EvalOne(parent_context, candidate); };

        const auto eval_conds_on_candidate = [&eval_cond_on_candidate](const auto&... conds)
        { return (eval_cond_on_candidate(conds) || ...); };

        return std::apply(eval_conds_on_candidate, m_operands);
    }

    [[nodiscard]] CONSTEXPR_STRING std::string Description(bool negated = false) const override { return {}; }

    [[nodiscard]] CONSTEXPR_STRING std::string Dump(uint8_t ntabs = 0) const override { return {}; }

    CONSTEXPR_STRING void SetTopLevelContent(const std::string& content_name) {
        const auto set_tlc = [&content_name](auto& op)
        { op.SetTopLevelContent(content_name); };

        const auto set_tlcs = [&set_tlc](auto&... ops)
        { (set_tlc(ops), ...); };

        std::apply(set_tlcs, m_operands);
    }

    [[nodiscard]] constexpr auto OperandsRaw() const noexcept {
        std::array<const Condition*, N> retval{};
        const auto to_ptrs = [&retval](const auto&... ops) {
            std::size_t idx = 0;
            ((retval[idx++] = &ops), ...);
        };
        std::apply(to_ptrs, m_operands);
        return retval;
    }

    [[nodiscard]] const auto& OperandsTuple() const { return m_operands; }


    [[nodiscard]] std::vector<std::unique_ptr<Condition>> Operands() && override {
        std::vector<std::unique_ptr<Condition>> retval;
        retval.reserve(N);

        static constexpr auto to_unique_ptr = [](auto& cond)
        { return std::make_unique<std::decay_t<decltype(cond)>>(std::move(cond)); };

        const auto to_ptrs = [&retval](auto&... ops)
        { ((retval.push_back(to_unique_ptr(ops))), ...); };

        std::apply(to_ptrs, m_operands);
        return retval;
    }

    [[nodiscard]] constexpr std::size_t OperandCount() const noexcept override { return N; }

    [[nodiscard]] constexpr uint32_t GetCheckSum() const {
        uint32_t retval{0};

        CheckSums::CheckSumCombine(retval, "Condition::Or");
        CheckSums::CheckSumCombine(retval, m_operands);

        return retval;
    }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override {
        using zeroth_operand_t = std::tuple_element_t<0, ConditionTupleT>;

        static constexpr auto clone_cast_op_type_rval = [](const auto& op) -> auto&& {
            static_assert(requires { op.Clone(); } );
            using OperandT = std::decay_t<decltype(op)>;
            auto cond_clone_unique = op.Clone();
            OperandT& ref = *static_cast<OperandT*>(cond_clone_unique.release());
            return std::move(ref);
        };
        using get0_operand_t = decltype(std::get<0>(m_operands));
        static_assert(std::is_same_v<get0_operand_t, const zeroth_operand_t&>);
        using clone_cast0_operand_t = decltype(clone_cast_op_type_rval(std::get<0>(m_operands)));
        static_assert(std::is_same_v<clone_cast0_operand_t, zeroth_operand_t&&>);

        static constexpr auto clone_operands = [](const auto&... ops)
        { return std::make_tuple(clone_cast_op_type_rval(ops)...); };

        ConditionTupleT cloned_operands = std::apply(clone_operands, m_operands);
        return std::make_unique<std::decay_t<decltype(*this)>>(std::move(cloned_operands));
    }

private:
    ConditionTupleT m_operands;
};

template <typename... Ts>
Or(Ts...) -> Or<std::decay_t<Ts>...>;
template <typename... Ts>
Or(std::tuple<Ts...>&&) -> Or<std::decay_t<Ts>...>;

template <>
struct FO_COMMON_API Or<> final : public OrBase {
    explicit Or(std::vector<std::unique_ptr<Condition>>&& operands);

    Or(std::unique_ptr<Condition>&& operand1,
       std::unique_ptr<Condition>&& operand2,
       std::unique_ptr<Condition>&& operand3 = nullptr,
       std::unique_ptr<Condition>&& operand4 = nullptr);

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const override {
        if (this == &rhs)
            return true;
        const auto* rhs_and = dynamic_cast<decltype(this)>(&rhs);
        return rhs_and && m_operands == rhs_and->m_operands;
    }
    [[nodiscard]] bool operator==(const Or& rhs) const;

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override;
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override;
    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    virtual void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] std::vector<const Condition*> OperandsRaw() const;
    [[nodiscard]] std::vector<std::unique_ptr<Condition>> Operands() && override { return std::move(m_operands); }
    [[nodiscard]] std::size_t OperandCount() const noexcept override { return m_operands.size(); }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    std::vector<std::unique_ptr<Condition>> m_operands;
};

/** Matches all objects that do not match the Condition \a operand. */
template <class ConditionT = std::unique_ptr<Condition>>
    requires ((std::is_base_of_v<Condition, ConditionT> && !std::is_same_v<Condition, std::decay_t<ConditionT>>) ||
              std::is_same_v<std::unique_ptr<Condition>, ConditionT>)
struct FO_COMMON_API Not final : public Condition {
private:
    static constexpr bool cond_is_ptr = requires(const ConditionT c) { c.get(); };
    static constexpr bool cond_equals_noexcept = noexcept(std::declval<ConditionT>() == std::declval<ConditionT>());
    static constexpr bool cond_deref_equals_noexcept = noexcept(std::declval<ConditionT>() == std::declval<ConditionT>());

    [[nodiscard]] constexpr std::array<bool, 3> GetRTSInvariants(const ConditionT& c) noexcept {
        if constexpr (cond_is_ptr)
            return conds_rtsi(*c);
        else
            return conds_rtsi(c);
    }

public:
    constexpr explicit Not(ConditionT&& operand) :
        Condition(GetRTSInvariants(operand)),
        m_operand(std::move(operand))
    {}

    constexpr bool operator==(const Not& rhs) const noexcept(cond_equals_noexcept && cond_deref_equals_noexcept) {
        if (this == &rhs || m_operand == rhs.m_operand)
            return true;
        if constexpr (cond_is_ptr)
            return *m_operand  == *rhs.m_operand;
        else
            return false;
    }

    constexpr bool operator==(const Condition& rhs) const
        noexcept(noexcept(std::declval<Not>() == std::declval<Not>())) override
    {
        if (this == &rhs)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override
    {
        // swapping order of matches and non_matches set parameters and MATCHES / NON_MATCHES search domain effects NOT on requested search domain
        const auto subcondition_domain = (search_domain == SearchDomain::NON_MATCHES) ?
            SearchDomain::MATCHES : SearchDomain::NON_MATCHES;
        if constexpr (cond_is_ptr)
            m_operand->Eval(parent_context, non_matches, matches, subcondition_domain);
        else
            m_operand.Eval(parent_context, non_matches, matches, subcondition_domain);
    }

    [[nodiscard]] bool EvalAny(const ScriptingContext& parent_context, const ObjectSet& candidates) const override {
        // need to determine if (anything does not match the subcondition). that's not the same
        // as (nothing matches the subcondition).
        //
        // eg. Capital::EvalAny would return true iff there is any candidate that is a capital
        // Not(Capital)::EvalAny would return true iff there is any candidate that is not a capital
        return std::any_of(candidates.begin(), candidates.end(),
                           [&parent_context, this](const auto* candidate) {
                               if constexpr (cond_is_ptr)
                                   return !m_operand->EvalOne(parent_context, candidate);
                               else
                                   return !m_operand.EvalOne(parent_context, candidate);
                           });
        // or:
        //ObjectSet potential_matches{candidates};
        //ObjectSet non_matches;
        //non_matches.reserve(candidates.size());
        //m_operand->Eval(parent_context, potential_matches, non_matches, SearchDomain::MATCHES);
        //return !non_matches.empty(); // if non_matches is not empty, than something initially in potential_matches was not matched by m_operand
    }
    [[nodiscard]] bool EvalOne(const ScriptingContext& parent_context, const UniverseObject* candidate) const override {
        if constexpr (cond_is_ptr)
            return m_operand->EvalOne(parent_context, candidate);
        else
            return m_operand.EvalOne(parent_context, candidate);
    }

    [[nodiscard]] std::string Description(bool negated = false) const override {
        if constexpr (cond_is_ptr)
            return m_operand->Description(!negated);
        else
            return m_operand.Description(!negated);
    }

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override {
        std::string retval = DumpIndent(ntabs) + "Not\n";
        if constexpr (cond_is_ptr)
            retval += m_operand->Dump(ntabs+1);
        else
            retval += m_operand.Dump(ntabs+1);
        return retval;
    }

    void SetTopLevelContent(const std::string& content_name) override {
        if constexpr (cond_is_ptr)
            m_operand->SetTopLevelContent(content_name);
        else
            m_operand.SetTopLevelContent(content_name);
    }

    [[nodiscard]] uint32_t GetCheckSum() const override {
        uint32_t retval{0};

        CheckSums::CheckSumCombine(retval, "Condition::Not");
        CheckSums::CheckSumCombine(retval, m_operand);

        return retval;
    }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override {
        if constexpr (cond_is_ptr) {
            return std::make_unique<Not>(m_operand->Clone());
        } else {
            auto operand_clone = m_operand.Clone();
            return std::make_unique<Not>(std::move(dynamic_cast<ConditionT&>(*operand_clone))); // throws if .Clone() doesn't return a pointer to a valid ConditionT
        }
    }

private:
    ConditionT m_operand;
};

template <typename T>
Not(T) -> Not<std::decay_t<T>>;

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
