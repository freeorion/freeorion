#ifndef _ValueRef_h_
#define _ValueRef_h_

#include "ScriptingContext.h"
#include "../util/Enum.h"
#include "../util/Export.h"
#include "../util/i18n.h"
#include <type_traits>

namespace ValueRef {
enum class ReferenceType : int8_t {
    INVALID_REFERENCE_TYPE = -1,        // ValueRef::Variable doesn't evaluate a object or global gamestate value. Might be a Complex ref that takes additional parameters
    NON_OBJECT_REFERENCE,               // ValueRef::Variable is not evaluated on any specific object. Might be a non-object variable like the current game turn
    SOURCE_REFERENCE,                   // ValueRef::Variable is evaluated on the source object
    EFFECT_TARGET_REFERENCE,            // ValueRef::Variable is evaluated on the target object of an effect while it is being executed
    EFFECT_TARGET_VALUE_REFERENCE,      // ValueRef::Variable is evaluated on the value being set by an effect while it is being executed, eg. adding 5 to the current value of a meter
    CONDITION_LOCAL_CANDIDATE_REFERENCE,// ValueRef::Variable is evaluated on an object that is a candidate to be matched by a condition.  In a subcondition, this will reference the local candidate, and not the candidate of an enclosing condition.
    CONDITION_ROOT_CANDIDATE_REFERENCE  // ValueRef::Variable is evaluated on an object that is a candidate to be matched by a condition.  In a subcondition, this will still reference the root candidate, and not the candidate of the local condition.
};

enum class ContainerType : int8_t {
    NONE = 0,
    PLANET,
    SYSTEM,
    FLEET
};

enum class OpType : uint8_t {
    INVALID_OP_TYPE,
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    REMAINDER,
    NEGATE,
    EXPONENTIATE,
    ABS,
    LOGARITHM,
    SINE,
    COSINE,
    MINIMUM,
    MAXIMUM,
    RANDOM_UNIFORM,
    RANDOM_PICK,
    SUBSTITUTION,
    COMPARE_EQUAL,
    COMPARE_GREATER_THAN,
    COMPARE_GREATER_THAN_OR_EQUAL,
    COMPARE_LESS_THAN,
    COMPARE_LESS_THAN_OR_EQUAL,
    COMPARE_NOT_EQUAL,
    ROUND_NEAREST,
    ROUND_UP,
    ROUND_DOWN,
    SIGN,
    NOOP
};

FO_ENUM(
    (StatisticType),
    ((INVALID_STATISTIC_TYPE, -1))

    ((IF))          // returns T{1} if anything matches the condition, or T{0} otherwise

    ((COUNT))       // returns the number of objects matching the condition
    ((UNIQUE_COUNT))// returns the number of distinct property values of objects matching the condition. eg. if 3 objects have the property value "small", and five have "big", then this value is 2, as there are 2 unique property values.
    ((HISTO_MAX))   // returns the maximum number of times a unique property value appears in the property values. eg. if 5 objects have values 1, 2, 3, 2, 2, then there are 3x2, 1x1, and 1x3, and the histo max is 3.
    ((HISTO_MIN))   // returns the maximum number of times a unique property value appears in the property values. eg. if 5 objects have values 1, 2, 3, 2, 2, then there are 3x2, 1x1, and 1x3, and the histo min is 1.
    ((HISTO_SPREAD))// returns the (positive) difference between the maximum and minimum numbers of distinct property values. eg. if 5 objects have values, 1, 2, 3, 2, 2, then there are 3x2, 1x1, and 1x3, and the Histo Spread is 3-1 = 2.

    ((SUM))         // returns the sum of the property values of all objects matching the condition
    ((MEAN))        // returns the mean of the property values of all objects matching the condition
    ((RMS))         // returns the sqrt of the mean of the squares of the property values of all objects matching the condition
    ((MODE))        // returns the most common property value of objects matching the condition.  supported for non-numeric types such as enums.
    ((MAX))         // returns the maximum value of the property amongst objects matching the condition
    ((MIN))         // returns the minimum value of the property amongst objects matching the condition
    ((SPREAD))      // returns the (positive) difference between the maximum and minimum values of the property amongst objects matching the condition
    ((STDEV))       // returns the standard deviation of the property values of all objects matching the condition
    ((PRODUCT))     // returns the product of the property values of all objects matching the condition
)

//! The common base class for all ValueRef classes. This class provides
//! some the return-type-independent interface.
struct FO_COMMON_API ValueRefBase {
    // these getters can't be noexcept due to a derived class doing complicated stuff
    [[nodiscard]] constexpr virtual bool RootCandidateInvariant() const  { return m_root_candidate_invariant; }
    [[nodiscard]] constexpr virtual bool LocalCandidateInvariant() const { return m_local_candidate_invariant; }
    [[nodiscard]] constexpr virtual bool TargetInvariant() const         { return m_target_invariant; }
    [[nodiscard]] constexpr virtual bool SourceInvariant() const         { return m_source_invariant; }
    [[nodiscard]] constexpr virtual bool SimpleIncrement() const         { return m_simple_increment; }
    [[nodiscard]] constexpr virtual bool ConstantExpr() const            { return m_constant_expr; }

    [[nodiscard]] std::string         InvariancePattern() const;

    // does this ValueRef return the value of something that is being modified by an effect. eg. in an effect that sets a meter, does this ref return the value of that meter?
    [[nodiscard]] constexpr bool      IsEffectModifiedValueReference() const noexcept { return m_ref_type == ReferenceType::EFFECT_TARGET_VALUE_REFERENCE; }
    // does this ValueRef return the immediate or initial values of whatever it returns. ie. is the value it is immediately when this is evaluated, including any previous modifications, or is it the value that was set at the start of the current turn?
    [[nodiscard]] constexpr bool      ReturnImmediateValue() const noexcept { return m_return_immediate_value; }

    [[nodiscard]] virtual std::string Description() const = 0;              //! Returns a user-readable text description of this ValueRef
    [[nodiscard]] virtual std::string EvalAsString() const = 0;             //! Returns a textual representation of the evaluation result  with an empty/default context
    [[nodiscard]] virtual std::string Dump(uint8_t ntabs = 0) const = 0;    //! Returns a textual representation that should be parseable to recreate this ValueRef

    virtual void SetTopLevelContent(const std::string& content_name) {}

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept { return m_checksum_cache; }

    constexpr virtual ~ValueRefBase() noexcept
#if defined(__GNUC__) && (__GNUC__ < 13)
        {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#else
        = default;
#endif

protected:
    constexpr ValueRefBase() noexcept = default;
    constexpr ValueRefBase(bool constant_expr, ReferenceType ref_type, uint32_t checksum = 0u) noexcept :
        m_ref_type(ref_type),
        m_checksum_cache(checksum & 0xffffff),
        m_constant_expr(constant_expr)
    {}
    constexpr explicit ValueRefBase(ReferenceType ref_type, uint32_t checksum = 0u) noexcept :
        m_ref_type(ref_type),
        m_checksum_cache(checksum & 0xffffff)
    {}
    constexpr ValueRefBase(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                           uint32_t checksum) noexcept :
        m_checksum_cache(checksum & 0xffffff),
        m_root_candidate_invariant(root_inv),
        m_local_candidate_invariant(local_inv),
        m_target_invariant(target_inv),
        m_source_invariant(source_inv),
        m_constant_expr(constant_expr)
    {}
    constexpr ValueRefBase(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                           ReferenceType ref_type, ContainerType container, uint32_t checksum = 0u) noexcept :
        m_ref_type(ref_type),
        m_container_type(container),
        m_checksum_cache(checksum & 0xffffff),
        m_root_candidate_invariant(root_inv),
        m_local_candidate_invariant(local_inv),
        m_target_invariant(target_inv),
        m_source_invariant(source_inv),
        m_constant_expr(constant_expr)
    {}
    constexpr ValueRefBase(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                           bool return_immediate_value, ReferenceType ref_type, ContainerType container,
                           uint32_t checksum = 0u) noexcept :
        m_ref_type(ref_type),
        m_container_type(container),
        m_checksum_cache(checksum & 0xffffff),
        m_root_candidate_invariant(root_inv),
        m_local_candidate_invariant(local_inv),
        m_target_invariant(target_inv),
        m_source_invariant(source_inv),
        m_constant_expr(constant_expr),
        m_return_immediate_value(return_immediate_value)
    {}
    constexpr ValueRefBase(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                           bool simple_increment, OpType op_type, uint32_t checksum = 0u) noexcept :
        m_op_type(op_type),
        m_checksum_cache(checksum & 0xffffff),
        m_root_candidate_invariant(root_inv),
        m_local_candidate_invariant(local_inv),
        m_target_invariant(target_inv),
        m_source_invariant(source_inv),
        m_constant_expr(constant_expr),
        m_simple_increment(simple_increment)
    {}
    constexpr ValueRefBase(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                           StatisticType stat_type, uint32_t checksum = 0u) noexcept :
        m_stat_type(stat_type),
        m_checksum_cache(checksum & 0xffffff),
        m_root_candidate_invariant(root_inv),
        m_local_candidate_invariant(local_inv),
        m_target_invariant(target_inv),
        m_source_invariant(source_inv),
        m_constant_expr(constant_expr)
    {}

    [[nodiscard]] constexpr bool operator==(const ValueRefBase& rhs) const = default;

    const ReferenceType m_ref_type = ReferenceType::INVALID_REFERENCE_TYPE;
    const ContainerType m_container_type = ContainerType::NONE;
    const OpType m_op_type = OpType::INVALID_OP_TYPE;
    const StatisticType m_stat_type = StatisticType::INVALID_STATISTIC_TYPE;

    const uint32_t m_checksum_cache : 24 = 0u;    // derived classes may use this to store their checksum value

    uint32_t m_root_candidate_invariant : 1 = false; // does the value of this depend on the Condition root candidate object?
    uint32_t m_local_candidate_invariant : 1 = false;// does the value of this depend on the Condition local candidate object?
    uint32_t m_target_invariant : 1 = false;         // does the value of this depend on the Effect target object?
    uint32_t m_source_invariant : 1 = false;         // does the value of this depend on the Source object?
    uint32_t m_constant_expr : 1 = false;            // is this a constant expression, which can be evaluated without a ScriptingContext?
    // is this an expression that is a fixed modification of something that is being set in by effect,
    // eg. adding a constant to a meter, where the "Value" to set is calculated using a simple expressing like "Value + 5" 
    uint32_t m_simple_increment : 1 = false;
    // is the value of this expression the immediate value or the initial value of what's referenced?
    // eg. if the expression is "Target.Population", if m_return_immediate_value == false, then the
    // returned value is the initial population before the current effects are applied.
    // or if m_return_immediate_value == true, then as various effects are applied that modify the
    // target's populated, the value of this expression will change as each effect is applied, depending
    // on the order of application
    uint32_t m_return_immediate_value : 1 = false;
};


template <class N>
struct is_vector { static const int value = 0; };

template <class N, class A>
struct is_vector<std::vector<N, A> > { static const int value = 1; };

template<typename T>
decltype(auto) FlexibleToString(const T& t)
{
    static_assert(!std::is_enum_v<T>);
    static_assert(!is_vector<T>::value);

    if constexpr (std::is_floating_point_v<std::decay_t<T>>) {
        return DoubleToString(t, 3, false);

    } else if constexpr (std::is_convertible_v<std::decay_t<T>, std::string>) {
        return std::forward<T>(t);

    } else if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>) {
        return std::string{t};

    } else {
        return std::to_string(t);
    }
}

[[nodiscard]] FO_COMMON_API std::string FlexibleToString(const StarType& t);
[[nodiscard]] FO_COMMON_API std::string FlexibleToString(const PlanetEnvironment& t);
[[nodiscard]] FO_COMMON_API std::string FlexibleToString(const PlanetType& t);
[[nodiscard]] FO_COMMON_API std::string FlexibleToString(const PlanetSize& t);
[[nodiscard]] FO_COMMON_API std::string FlexibleToString(const ShipPartClass& t);
[[nodiscard]] FO_COMMON_API std::string FlexibleToString(const Visibility& t);
[[nodiscard]] FO_COMMON_API std::string FlexibleToString(const UniverseObjectType& t);

template <typename EnumT>
std::string EnumToString(EnumT t);

// after declarations of specialization of FlexibleToString for enums
template<typename T>
decltype(auto) FlexibleToString(const std::vector<T>& tv)
{
    if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
        std::string retval;
        for (auto& ts: tv)
            retval.append(std::to_string(ts));
        return retval;

    } else if constexpr (std::is_same_v<T, std::string>) {
        std::size_t total_size = 0;
        for (auto& ts : tv)
            total_size += ts.size();
        std::string retval;
        retval.reserve(total_size);
        for (auto& ts: tv)
            retval.append(ts);
        return retval;

    } else if constexpr (std::is_enum_v<T>) {
        std::string retval;
        for (auto& ts: tv)
            retval.append(EnumToString(ts));
        return retval;

    } else {
        std::string retval;
        for (auto& ts: tv)
          retval.append(FlexibleToString<T>(ts));
        return retval;
    }
}

//! The base class for all ValueRef classes returning type T. This class
//! provides the public interface for a ValueRef expression tree.
template <typename T>
struct FO_COMMON_API ValueRef : public ValueRefBase
{
    using ResultT = T;

    /** Evaluates the expression tree with no input context.  Useful for
      * evaluating expressions that do not depend on source, target, or
      * candidate objects. */
    [[nodiscard]] virtual T Eval() const { return Eval(IApp::GetApp()->GetContext()); }

    /** Evaluates the expression tree and return the results; \a context
      * is used to fill in any instances of the "Value" variable or references
      * to objects such as the source, effect target, or condition candidates
      * that exist in the tree. */
    [[nodiscard]] virtual T Eval(const ScriptingContext& context) const = 0;

    /** Evaluates the expression tree with an empty context and returns the
      * a string representation of the result value iff the result type is
      * supported. Otherwise returns and empty string. */
    [[nodiscard]] std::string EvalAsString() const override final {
        if constexpr (std::is_same_v<T, std::string>)
            return Eval();
        else
            return FlexibleToString(Eval());
    }

    [[nodiscard]] constexpr auto GetReferenceType() const noexcept { return m_ref_type; }

    /** Makes a clone of this ValueRef in a new owning pointer. Required for Boost.Python, which
      * doesn't supports move semantics for returned values. */
    [[nodiscard]] virtual std::unique_ptr<ValueRef<T>> Clone() const = 0;

    [[nodiscard]] virtual bool operator==(const ValueRef& rhs) const = 0;

protected:
    constexpr ValueRef() noexcept = default;
    constexpr explicit ValueRef(ReferenceType ref_type, uint32_t checksum = 0u) noexcept :
        ValueRefBase(ref_type, checksum)
    {}
    constexpr ValueRef(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                       uint32_t checksum) noexcept :
        ValueRefBase(constant_expr, root_inv, local_inv, target_inv, source_inv, checksum)
    {}

    constexpr ValueRef(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                       ReferenceType ref_type = ReferenceType::INVALID_REFERENCE_TYPE, uint32_t checksum = 0u) noexcept :
        ValueRefBase(constant_expr, root_inv, local_inv, target_inv, source_inv, ref_type, ContainerType::NONE, checksum)
    {}
    constexpr ValueRef(std::array<bool, 5> rtslice, ReferenceType ref_type = ReferenceType::INVALID_REFERENCE_TYPE,
                       uint32_t checksum = 0u) noexcept :
        ValueRef(rtslice[4], rtslice[0], rtslice[3], rtslice[1], rtslice[2], ref_type, checksum)
    {}          // constant,       root,      local,     target,     source

    constexpr ValueRef(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                       bool return_immediate_value, ReferenceType ref_type = ReferenceType::INVALID_REFERENCE_TYPE,
                       ContainerType container = ContainerType::NONE, uint32_t checksum = 0u) noexcept :
        ValueRefBase(constant_expr, root_inv, local_inv, target_inv, source_inv, return_immediate_value, ref_type, container, checksum)
    {}

    constexpr ValueRef(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                       bool simple_increment, OpType op_type, uint32_t checksum = 0u) noexcept :
        ValueRefBase(constant_expr, root_inv, local_inv, target_inv, source_inv, simple_increment, op_type, checksum)
    {}
    constexpr ValueRef(std::array<bool, 5> rtslice, bool simple_increment, OpType op_type, uint32_t checksum = 0u) noexcept :
        ValueRefBase(rtslice[4], rtslice[0], rtslice[3], rtslice[1], rtslice[2], simple_increment, op_type, checksum)
    {}              // constant,       root,      local,     target,     source

    constexpr ValueRef(bool constant_expr, bool root_inv, bool local_inv, bool target_inv, bool source_inv,
                       StatisticType stat_type, uint32_t checksum = 0u) noexcept :
        ValueRefBase(constant_expr, root_inv, local_inv, target_inv, source_inv, stat_type, checksum)
    {}
};


template<typename T>
[[nodiscard]] constexpr std::unique_ptr<std::remove_const_t<T>> CloneUnique(const T* ptr)
{ return ptr ? ptr->Clone() : nullptr; }

template<typename T>
[[nodiscard]] constexpr std::unique_ptr<std::remove_const_t<T>> CloneUnique(const std::unique_ptr<T>& ptr)
{ return ptr ? ptr->Clone() : nullptr; }

template<typename T>
[[nodiscard]] inline std::unique_ptr<std::remove_const_t<T>> CloneUnique(const std::shared_ptr<T>& ptr)
{ return ptr ? ptr->Clone() : nullptr; }

template<typename T>
[[nodiscard]] inline auto CloneUnique(const std::vector<std::unique_ptr<T>>& vec)
{
    std::vector<std::unique_ptr<T>> retval;
    retval.reserve(vec.size());
    for (const auto& val : vec)
        retval.push_back(CloneUnique(val));
    return retval;
}

template<typename T, std::size_t N>
[[nodiscard]] inline auto CloneUnique(const std::array<std::unique_ptr<T>, N>& arr)
{
    std::array<std::unique_ptr<T>, N> retval{};
    for (std::size_t i = 0; i < arr.size(); ++i)
        retval[i] = CloneUnique(arr[i]);
    return retval;
}

template<typename T>
[[nodiscard]] inline auto CloneUnique(const std::vector<std::pair<std::string, std::unique_ptr<T>>>& vec)
{
    std::vector<std::pair<std::string, std::unique_ptr<T>>> retval;
    retval.reserve(vec.size());
    for (const auto& val : vec)
        retval.emplace_back(val.first, CloneUnique(val.second));
    return retval;
}

}

#endif // _ValueRef_h_
