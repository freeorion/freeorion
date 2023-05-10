#ifndef _ValueRefs_h_
#define _ValueRefs_h_


#include <iterator>
#include <map>
#include <numeric>
#include <unordered_set>
#include <unordered_map>
#include <type_traits>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "Condition.h"
#include "ScriptingContext.h"
#include "Universe.h"
#include "ValueRef.h"
#include "../util/CheckSums.h"
#include "../util/Export.h"
#include "../util/i18n.h"
#include "../util/Random.h"

namespace CheckSums {
    template <typename T>
    void CheckSumCombine(uint32_t& sum, const typename ValueRef::ValueRef<T>& c)
    {
        TraceLogger() << "CheckSumCombine(ValueRef::ValueRef<T>): " << typeid(c).name();
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }
}

class UniverseObject;

namespace ValueRef {
/** the constant value leaf ValueRef class. */
template <typename T>
struct FO_COMMON_API Constant final : public ValueRef<T>
{
    template <typename TT> requires (std::is_convertible_v<TT, T>)
    constexpr explicit Constant(TT&& value)
        noexcept(noexcept(std::string{}) && noexcept(T{std::declval<TT>()})) :
        m_value(std::forward<TT>(value))
    {
        static_assert(std::is_nothrow_move_constructible_v<T>);
        this->m_root_candidate_invariant = true;
        this->m_local_candidate_invariant = true;
        this->m_target_invariant = true;
        this->m_source_invariant = true;
    }

    [[nodiscard]] bool operator==(const ValueRef<T>& rhs) const override;

    static constexpr std::string_view current_content = "CurrentContent";
    static constexpr std::string_view no_current_content = "THERE_IS_NO_TOP_LEVEL_CONTENT";

    [[nodiscard]] T Eval(const ScriptingContext& context) const
        noexcept(noexcept(T{std::declval<const T>()})) override
    {
        if constexpr (std::is_same_v<T, std::string>) {
            if (m_value == current_content)
                return m_top_level_content;
        }
        return m_value;
    }

    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] T Value() const noexcept(noexcept(T{})) { return m_value; };
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override {
        auto retval = std::make_unique<Constant>(m_value);
        retval->m_top_level_content = m_top_level_content;
        return retval;
    }

private:
    const T m_value{};
    std::string m_top_level_content;    // in the special case that T is std::string and m_value is "CurrentContent", return this instead
};


/** The variable value ValueRef class.  The value returned by this node is
  * taken from the gamestate, most often from the Source or Target objects. */
template <typename T>
struct FO_COMMON_API Variable : public ValueRef<T>
{
    template <typename S>
    Variable(ReferenceType ref_type, S&& property_name, bool return_immediate_value = false);

    Variable(ReferenceType ref_type, const char* property_name, bool return_immediate_value = false);

    explicit Variable(ReferenceType ref_type, bool return_immediate_value = false);

    template <typename S>
    Variable(ReferenceType ref_type,  boost::optional<std::string>&& container_name,
             S&& property_name, bool return_immediate_value = false);

    Variable(ReferenceType ref_type, std::vector<std::string>&& property_name, bool return_immediate_value = false);

    Variable(ReferenceType ref_type, const std::vector<std::string>& property_name, bool return_immediate_value = false);

    [[nodiscard]] bool operator==(const ValueRef<T>& rhs) const override;
    [[nodiscard]] T Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] auto& PropertyName() const noexcept { return m_property_name; }
    [[nodiscard]] bool ReturnImmediateValue() const noexcept { return m_return_immediate_value; }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override
    { return std::make_unique<Variable<T>>(this->m_ref_type, m_property_name, m_return_immediate_value); }

protected:
    void InitInvariants();

    const std::vector<std::string> m_property_name;
    const bool                     m_return_immediate_value = false;
};

/** The variable statistic class.   The value returned by this node is
  * computed from the general gamestate; the value of the indicated
  * \a property_name is computed for each object that matches
  * \a sampling_condition and the statistic indicated by \a stat_type is
  * calculated from them and returned. */
template <typename T, typename V = T>
struct FO_COMMON_API Statistic final : public Variable<T>
{
    Statistic(std::unique_ptr<ValueRef<V>>&& value_ref,
              StatisticType stat_type,
              std::unique_ptr<Condition::Condition>&& sampling_condition);

    [[nodiscard]] bool        operator==(const ValueRef<T>& rhs) const override;
    [[nodiscard]] T           Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] StatisticType GetStatisticType() const noexcept { return m_stat_type; }

    [[nodiscard]] const auto* GetSamplingCondition() const noexcept { return m_sampling_condition.get(); }

    [[nodiscard]] const auto* GetValueRef() const noexcept { return m_value_ref.get(); }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override {
        return std::make_unique<Statistic<T, V>>(CloneUnique(m_value_ref),
                                                 m_stat_type,
                                                 CloneUnique(m_sampling_condition));
    }

protected:
    /** Evaluates the property for the specified objects. */
    std::vector<V> GetObjectPropertyValues(const ScriptingContext& context,
                                           const Condition::ObjectSet& objects) const;

private:
    const StatisticType                         m_stat_type;
    const std::unique_ptr<Condition::Condition> m_sampling_condition;
    const std::unique_ptr<ValueRef<V>>          m_value_ref;
};

/** The variable TotalFighterShots class. The value returned by this node is
  * computed from the gamestate; the number of shots of a launched fighters
  * of the given \a carrier_id is counted (and added up) for all combat bouts
  * in which the given \a sampling_condition matches. */
struct FO_COMMON_API TotalFighterShots final : public Variable<int>
{
    TotalFighterShots(std::unique_ptr<ValueRef<int>>&& carrier_id,
                      std::unique_ptr<Condition::Condition>&& sampling_condition = nullptr);

    bool                      operator==(const ValueRef<int>& rhs) const override;
    [[nodiscard]] int         Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const auto* GetSamplingCondition() const noexcept { return m_sampling_condition.get(); }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<int>> Clone() const override
    { return std::make_unique<TotalFighterShots>(CloneUnique(m_carrier_id), CloneUnique(m_sampling_condition)); }

private:
    const std::unique_ptr<ValueRef<int>>        m_carrier_id;
    const std::unique_ptr<Condition::Condition> m_sampling_condition;
};

/** The complex variable ValueRef class. The value returned by this node
  * is taken from the gamestate. */
template <typename T>
struct FO_COMMON_API ComplexVariable final : public Variable<T>
{
    template<typename S>
    explicit ComplexVariable(S&& variable_name,
                             std::unique_ptr<ValueRef<int>>&& int_ref1 = nullptr,
                             std::unique_ptr<ValueRef<int>>&& int_ref2 = nullptr,
                             std::unique_ptr<ValueRef<int>>&& int_ref3 = nullptr,
                             std::unique_ptr<ValueRef<std::string>>&& string_ref1 = nullptr,
                             std::unique_ptr<ValueRef<std::string>>&& string_ref2 = nullptr,
                             bool return_immediate_value = false);

    explicit ComplexVariable(const char* variable_name,
                             std::unique_ptr<ValueRef<int>>&& int_ref1 = nullptr,
                             std::unique_ptr<ValueRef<int>>&& int_ref2 = nullptr,
                             std::unique_ptr<ValueRef<int>>&& int_ref3 = nullptr,
                             std::unique_ptr<ValueRef<std::string>>&& string_ref1 = nullptr,
                             std::unique_ptr<ValueRef<std::string>>&& string_ref2 = nullptr,
                             bool return_immediate_value = false);

    explicit ComplexVariable(const ComplexVariable<T>& rhs);

    [[nodiscard]] bool        operator==(const ValueRef<T>& rhs) const override;
    [[nodiscard]] T           Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const auto* IntRef1() const noexcept { return m_int_ref1.get(); }
    [[nodiscard]] const auto* IntRef2() const noexcept { return m_int_ref2.get(); }
    [[nodiscard]] const auto* IntRef3() const noexcept { return m_int_ref3.get(); }
    [[nodiscard]] const auto* StringRef1() const noexcept { return m_string_ref1.get(); }
    [[nodiscard]] const auto* StringRef2() const noexcept { return m_string_ref2.get(); }
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override
    { return std::make_unique<ComplexVariable<T>>(*this); }

protected:
    void InitInvariants();

    const std::unique_ptr<ValueRef<int>> m_int_ref1;
    const std::unique_ptr<ValueRef<int>> m_int_ref2;
    const std::unique_ptr<ValueRef<int>> m_int_ref3;
    const std::unique_ptr<ValueRef<std::string>> m_string_ref1;
    const std::unique_ptr<ValueRef<std::string>> m_string_ref2;
};

/** The variable static_cast class.  The value returned by this node is taken
  * from the ctor \a value_ref parameter's FromType value, static_cast to
  * ToType. */
template <typename FromType, typename ToType>
struct FO_COMMON_API StaticCast final : public Variable<ToType>
{
    template <typename T> requires (std::is_convertible_v<T, std::unique_ptr<Variable<FromType>>>)
    explicit StaticCast(T&& value_ref);

    template <typename T> requires (std::is_convertible_v<T, std::unique_ptr<ValueRef<FromType>>> &&
                                    !std::is_convertible_v<T, std::unique_ptr<Variable<FromType>>>)
    explicit StaticCast(T&& value_ref);

    [[nodiscard]]             bool operator==(const ValueRef<ToType>& rhs) const override;
    [[nodiscard]] ToType      Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const auto* GetValueRef() const noexcept { return m_value_ref.get(); }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<ToType>> Clone() const override
    { return std::make_unique<StaticCast<FromType, ToType>>(CloneUnique(m_value_ref)); }

private:
    std::unique_ptr<ValueRef<FromType>> m_value_ref;
};

/** The variable lexical_cast to string class.  The value returned by this node
  * is taken from the ctor \a value_ref parameter's FromType value,
  * lexical_cast to std::string */
template <typename FromType>
struct FO_COMMON_API StringCast final : public Variable<std::string>
{
    explicit StringCast(std::unique_ptr<ValueRef<FromType>>&& value_ref);

    [[nodiscard]] bool        operator==(const ValueRef<std::string>& rhs) const override;
    [[nodiscard]] std::string Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const auto* GetValueRef() const noexcept { return m_value_ref.get(); }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<std::string>> Clone() const override
    { return std::make_unique<StringCast<FromType>>(CloneUnique(m_value_ref)); }

private:
    const std::unique_ptr<ValueRef<FromType>> m_value_ref;
};

/** Looks up a string ValueRef or vector of string ValueRefs, and returns
  * and returns the UserString equivalent(s). */
template <typename FromType>
struct FO_COMMON_API UserStringLookup final : public Variable<std::string> {
    explicit UserStringLookup(std::unique_ptr<ValueRef<FromType>>&& value_ref);

    [[nodiscard]] bool        operator==(const ValueRef<std::string>& rhs) const override;
    [[nodiscard]] std::string Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const auto* GetValueRef() const noexcept { return m_value_ref; }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<std::string>> Clone() const override
    { return std::make_unique<UserStringLookup<FromType>>(CloneUnique(m_value_ref)); }

private:
    const std::unique_ptr<ValueRef<FromType>> m_value_ref;
};

/** Returns the in-game name of the object / empire / etc. with a specified id. */
struct FO_COMMON_API NameLookup final : public Variable<std::string> {
    enum class LookupType : int8_t {
        INVALID_LOOKUP = -1,
        OBJECT_NAME,
        EMPIRE_NAME,
        SHIP_DESIGN_NAME
    };

    NameLookup(std::unique_ptr<ValueRef<int>>&& value_ref, LookupType lookup_type);

    [[nodiscard]] bool        operator==(const ValueRef<std::string>& rhs) const override;
    [[nodiscard]] std::string Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const auto* GetValueRef() const noexcept { return m_value_ref.get(); }

    [[nodiscard]] LookupType GetLookupType() const noexcept { return m_lookup_type; }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<std::string>> Clone() const override
    { return std::make_unique<NameLookup>(CloneUnique(m_value_ref), m_lookup_type); }

private:
    const std::unique_ptr<ValueRef<int>> m_value_ref;
    const LookupType m_lookup_type;
};

enum class OpType : uint8_t {
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

/** An arithmetic operation node ValueRef class. Unary or binary operations such
  * as addition, mutiplication, negation, exponentiation, rounding,
  * value substitution, value comparisons, or random value selection or
  * random number generation are performed on the child(ren) of this node, and
  * the result is returned. */
template <typename T>
struct FO_COMMON_API Operation final : public ValueRef<T>
{
    /** Binary operation ctor. */
    Operation(OpType op_type, std::unique_ptr<ValueRef<T>>&& operand1,
              std::unique_ptr<ValueRef<T>>&& operand2);

    /** Unary operation ctor. */
    Operation(OpType op_type, std::unique_ptr<ValueRef<T>>&& operand);

    /* N-ary operation ctor. */
    Operation(OpType op_type, std::vector<std::unique_ptr<ValueRef<T>>>&& operands);

    explicit Operation(const Operation<T>& rhs);

    [[nodiscard]] bool        operator==(const ValueRef<T>& rhs) const override;
    [[nodiscard]] T           Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] OpType      GetOpType() const noexcept { return m_op_type; }

    [[nodiscard]] static T    EvalImpl(OpType op_type, T lhs, T rhs);

    [[nodiscard]] const auto* LHS() const { return m_operands.empty() ? nullptr : m_operands.front().get(); } // 1st operand (or nullptr if none exists)
    [[nodiscard]] const auto* RHS() const { return m_operands.size() < 2 ? nullptr : m_operands[1].get(); } // 2nd operand (or nullptr if no 2nd operand exists)
    [[nodiscard]] const std::vector<ValueRef<T>*> Operands() const; // all operands

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override
    { return std::make_unique<Operation<T>>(*this); }

    void SetTopLevelContent(const std::string& content_name) override;

private:
    Operation(Operation<T>&& rhs) = delete;
    Operation& operator=(const Operation<T>& rhs) = delete;
    Operation& operator=(Operation<T>&& rhs) = delete;

    void InitInvariants();

    [[nodiscard]] T EvalImpl(const ScriptingContext& context) const;

    const OpType                                    m_op_type = OpType::TIMES;
    const std::vector<std::unique_ptr<ValueRef<T>>> m_operands;
    const T                                         m_cached_const_value = T();
};

/* Convert between names and MeterType. Names are scripting token, like Population
 * and not the MeterType string representations like METER_POPULATION */
[[nodiscard]] FO_COMMON_API MeterType        NameToMeter(std::string_view name) noexcept;
[[nodiscard]] FO_COMMON_API std::string_view MeterToName(MeterType meter) noexcept;

[[nodiscard]] FO_COMMON_API std::string_view PlanetTypeToString(PlanetType type) noexcept;
[[nodiscard]] FO_COMMON_API std::string_view PlanetEnvironmentToString(PlanetEnvironment env) noexcept;
[[nodiscard]] FO_COMMON_API std::string      ReconstructName(const std::vector<std::string>& property_name,
                                                             ReferenceType ref_type,
                                                             bool return_immediate_value = false);

[[nodiscard]] FO_COMMON_API std::string FormatedDescriptionPropertyNames(
    ReferenceType ref_type, const std::vector<std::string>& property_names,
    bool return_immediate_value = false);

[[nodiscard]] FO_COMMON_API std::string ComplexVariableDescription(
    const std::vector<std::string>& property_names,
    const ValueRef<int>* int_ref1,
    const ValueRef<int>* int_ref2,
    const ValueRef<int>* int_ref3,
    const ValueRef<std::string>* string_ref1,
    const ValueRef<std::string>* string_ref2);

[[nodiscard]] FO_COMMON_API std::string ComplexVariableDump(
    const std::vector<std::string>& property_names,
    const ValueRef<int>* int_ref1,
    const ValueRef<int>* int_ref2,
    const ValueRef<int>* int_ref3,
    const ValueRef<std::string>* string_ref1,
    const ValueRef<std::string>* string_ref2);

[[nodiscard]] FO_COMMON_API std::string StatisticDescription(
    StatisticType stat_type, std::string_view value_desc, std::string_view condition_desc);

// Template Implementations
///////////////////////////////////////////////////////////
// ValueRef                                          //
///////////////////////////////////////////////////////////
template <typename T>
bool ValueRef<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;

    return true;
}

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
template <typename T>
bool Constant<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Constant<T>& rhs_ = static_cast<const Constant<T>&>(rhs);

    if constexpr (std::is_same_v<T, std::string>) {
        if (m_top_level_content != rhs_.m_top_level_content)
            return false;
    }
    return m_value == rhs_.m_value;
}

template <typename T>
void Constant<T>::SetTopLevelContent(const std::string& content_name)
{
    if constexpr (std::is_same_v<T, std::string>) {
        if (m_value == current_content && content_name == no_current_content) {
            ErrorLogger() << "Constant<std::string>::SetTopLevelContent()  Scripted Content illegal.  Trying to set "
                          << no_current_content << " for "
                          << current_content << " (maybe you tried to use "
                          << current_content << " in named_values.focs.txt)";
        }
        if (!m_top_level_content.empty() && m_top_level_content != no_current_content)
            ErrorLogger() << "Constant<std::string>::SetTopLevelContent()  Tried to overwrite top level content from '" << m_top_level_content << "' to '" << content_name << "'";
        else
            m_top_level_content = content_name;
    }
}

template <typename T>
uint32_t Constant<T>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Constant");
    CheckSums::CheckSumCombine(retval, m_value);
    TraceLogger() << "GetCheckSum(Constant<T>): " << typeid(*this).name()
                  << " value: " << Description() << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API std::string Constant<int>::Description() const;

template <>
FO_COMMON_API std::string Constant<double>::Description() const;

template <>
FO_COMMON_API std::string Constant<std::string>::Description() const;

template <>
FO_COMMON_API std::string Constant<PlanetType>::Description() const;

template <>
FO_COMMON_API std::string Constant<PlanetSize>::Description() const;

template <>
FO_COMMON_API std::string Constant<PlanetEnvironment>::Description() const;

template <>
FO_COMMON_API std::string Constant<UniverseObjectType>::Description() const;

template <>
FO_COMMON_API std::string Constant<StarType>::Description() const;

template <>
FO_COMMON_API std::string Constant<Visibility>::Description() const;

template <>
FO_COMMON_API std::string Constant<PlanetSize>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<PlanetType>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<PlanetEnvironment>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<UniverseObjectType>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<StarType>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<Visibility>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<double>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<int>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string Constant<std::string>::Dump(uint8_t ntabs) const;

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <typename T>
Variable<T>::Variable(ReferenceType ref_type, std::vector<std::string>&& property_name,
                      bool return_immediate_value) :
    ValueRef<T>(ref_type),
    m_property_name(std::move(property_name)),
    m_return_immediate_value(return_immediate_value)
{ InitInvariants(); }

template <typename T>
Variable<T>::Variable(ReferenceType ref_type, const std::vector<std::string>& property_name,
                      bool return_immediate_value) :
    ValueRef<T>(ref_type),
    m_property_name(property_name),
    m_return_immediate_value(return_immediate_value)
{ InitInvariants(); }

template <typename T>
Variable<T>::Variable(ReferenceType ref_type, bool return_immediate_value) :
    Variable<T>(ref_type, std::vector<std::string>{}, return_immediate_value)
{}

template <typename T>
Variable<T>::Variable(ReferenceType ref_type, const char* property_name,
                      bool return_immediate_value) :
    Variable<T>(ref_type, std::vector<std::string>{1, property_name}, return_immediate_value)
{}

template <typename T>
template <typename S>
Variable<T>::Variable(ReferenceType ref_type, S&& property_name,
                      bool return_immediate_value) :
    ValueRef<T>(ref_type),
    m_property_name{std::forward<S>(property_name)},
    m_return_immediate_value(return_immediate_value)
{ InitInvariants(); }

template <typename T>
template <typename S>
Variable<T>::Variable(ReferenceType ref_type,
                      boost::optional<std::string>&& container_name,
                      S&& property_name,
                      bool return_immediate_value) :
    ValueRef<T>(ref_type),
    m_property_name([&container_name, &property_name]() -> std::vector<std::string> {
        std::vector<std::string> pn;
        const bool cnhs = container_name.has_value();
        pn.reserve(1 + cnhs);
        if (cnhs)
            pn.push_back(std::move(*container_name));
        pn.push_back(std::forward<S>(property_name));
        return pn;
    }()),
    m_return_immediate_value(return_immediate_value)
{
    InitInvariants();
}

template <typename T>
void Variable<T>::InitInvariants()
{
    const auto rt = this->m_ref_type;
    this->m_root_candidate_invariant = rt != ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE;
    this->m_local_candidate_invariant = rt != ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE;
    this->m_target_invariant = rt != ReferenceType::EFFECT_TARGET_REFERENCE &&
                               rt != ReferenceType::EFFECT_TARGET_VALUE_REFERENCE;
    this->m_source_invariant = rt != ReferenceType::SOURCE_REFERENCE;
}

template <typename T>
bool Variable<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Variable<T>& rhs_ = static_cast<const Variable<T>&>(rhs);
    return (this->m_ref_type == rhs_.m_ref_type) &&
           (m_property_name == rhs_.m_property_name) &&
           (m_return_immediate_value == rhs_.m_return_immediate_value);
}

template <typename T>
std::string Variable<T>::Description() const
{ return FormatedDescriptionPropertyNames(this->m_ref_type, m_property_name, m_return_immediate_value); }

template <typename T>
std::string Variable<T>::Dump(uint8_t ntabs) const
{ return ReconstructName(m_property_name, this->m_ref_type, m_return_immediate_value); }

template <typename T>
uint32_t Variable<T>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Variable");
    CheckSums::CheckSumCombine(retval, m_property_name);
    CheckSums::CheckSumCombine(retval, this->m_ref_type);
    CheckSums::CheckSumCombine(retval, m_return_immediate_value);
    TraceLogger() << "GetCheckSum(Variable<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API PlanetSize Variable<PlanetSize>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API StarType Variable<StarType>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API Visibility Variable<Visibility>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API double Variable<double>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API int Variable<int>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string Variable<std::string>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::vector<std::string> Variable<std::vector<std::string>>::Eval(const ScriptingContext& context) const;


///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////
template <typename T, typename V>
Statistic<T, V>::Statistic(std::unique_ptr<ValueRef<V>>&& value_ref, StatisticType stat_type,
                           std::unique_ptr<Condition::Condition>&& sampling_condition) :
    Variable<T>(ReferenceType::NON_OBJECT_REFERENCE),
    m_stat_type(stat_type),
    m_sampling_condition(std::move(sampling_condition)),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = (!m_sampling_condition || m_sampling_condition->RootCandidateInvariant()) &&
                                       (!m_value_ref || m_value_ref->RootCandidateInvariant());

    // don't need to check if sampling condition is LocalCandidateInvariant, as
    // all conditions aren't, but that refers to their own local candidate.  no
    // condition is explicitly dependent on the parent context's local candidate.
    // also don't need to check if sub-value-ref is local candidate invariant,
    // as it is applied to the subcondition matches, not the local candidate of
    // any containing condition
    this->m_local_candidate_invariant = true;

    this->m_target_invariant = (!m_sampling_condition || m_sampling_condition->TargetInvariant()) &&
                               (!m_value_ref || m_value_ref->TargetInvariant());

    this->m_source_invariant = (!m_sampling_condition || m_sampling_condition->SourceInvariant()) &&
                               (!m_value_ref || m_value_ref->SourceInvariant());
}

template <typename T, typename V>
bool Statistic<T, V>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Statistic<T, V>& rhs_ = static_cast<const Statistic<T, V>&>(rhs);

    if (m_stat_type != rhs_.m_stat_type)
        return false;

    if (m_value_ref == rhs_.m_value_ref) { // both unique_ptr could be nullptr
        // check next member
    } else if (!m_value_ref || !rhs_.m_value_ref) {
        return false;
    } else if (*m_value_ref != *(rhs_.m_value_ref)) {
        return false;
    }

    if (m_sampling_condition == rhs_.m_sampling_condition) {
        // check next member
    } else if (!m_sampling_condition || !rhs_.m_sampling_condition) {
        return false;
    } else if (*m_sampling_condition != *(rhs_.m_sampling_condition)) {
        return false;
    }

    return true;
}

template <typename T, typename V>
std::vector<V> Statistic<T, V>::GetObjectPropertyValues(const ScriptingContext& context,
                                                        const Condition::ObjectSet& objects) const
{
    std::vector<V> retval(objects.size());

    if (m_value_ref) {
        std::transform(objects.begin(), objects.end(), retval.begin(),
                       [&context, &ref{m_value_ref}](const auto* obj)
        { return ref->Eval(ScriptingContext(context, obj)); });
    }

    return retval;
}

template <typename T, typename V>
std::string Statistic<T, V>::Description() const
{
    if (m_value_ref)
        return StatisticDescription(m_stat_type, m_value_ref->Description(),
                                    m_sampling_condition ? m_sampling_condition->Description() : "");

    auto temp = Variable<T>::Description();
    if (!temp.empty())
        return StatisticDescription(m_stat_type, temp, m_sampling_condition ? m_sampling_condition->Description() : "");

    return StatisticDescription(m_stat_type, "", m_sampling_condition ? m_sampling_condition->Description() : "");
}

template <typename T, typename V>
std::string Statistic<T, V>::Dump(uint8_t ntabs) const
{
    std::string retval = "Statistic ";

    switch (m_stat_type) {
        case StatisticType::IF:             retval += "If";                break;
        case StatisticType::COUNT:          retval += "Count";             break;
        case StatisticType::UNIQUE_COUNT:   retval += "CountUnique";       break;
        case StatisticType::HISTO_MAX:      retval += "HistogramMax";      break;
        case StatisticType::HISTO_MIN:      retval += "HistogramMin";      break;
        case StatisticType::HISTO_SPREAD:   retval += "HistogramSpread";   break;
        case StatisticType::SUM:            retval += "Sum";               break;
        case StatisticType::MEAN:           retval += "Mean";              break;
        case StatisticType::RMS:            retval += "RMS";               break;
        case StatisticType::MODE:           retval += "Mode";              break;
        case StatisticType::MAX:            retval += "Max";               break;
        case StatisticType::MIN:            retval += "Min";               break;
        case StatisticType::SPREAD:         retval += "Spread";            break;
        case StatisticType::STDEV:          retval += "StDev";             break;
        case StatisticType::PRODUCT:        retval += "Product";           break;
        default:                            retval += "???";               break;
    }
    if (m_value_ref)
        retval += " value = " + m_value_ref->Dump();
    if (m_sampling_condition)
        retval += " condition = " + m_sampling_condition->Dump();
    return retval;
}

template <typename T, typename V>
void Statistic<T, V>::SetTopLevelContent(const std::string& content_name)
{
    if (m_sampling_condition)
        m_sampling_condition->SetTopLevelContent(content_name);
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template<typename T, typename V> requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<V>)
T ReduceData(StatisticType stat_type, std::vector<V> object_property_values)
{
    if (object_property_values.empty())
        return T{0};

    // should be able to convert between V and T types, so can do a bunch of
    // numerical statistics or histogram statistics

    switch (stat_type) {
        case StatisticType::IF: {
            // 1 if any objects have property values, else 0 above
            return T{1};
            break;
        }

        case StatisticType::COUNT: {
            // how many objects / values
            return T(object_property_values.size());
            break;
        }

        case StatisticType::UNIQUE_COUNT: {
            // how many unique values appear
            std::unordered_set<V> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto entry : object_property_values)
                observed_values.insert(entry);

            return T(observed_values.size());
            break;
        }

        case StatisticType::HISTO_MAX: {
            // number of times the most common value appears
            std::unordered_map<V, unsigned int> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto entry : object_property_values)
                observed_values[entry]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                        [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

            return T(max->second);
            break;
        }

        case StatisticType::HISTO_MIN: {
            // number of times the least common value appears
            std::unordered_map<V, unsigned int> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto entry : object_property_values)
                observed_values[entry]++;

            auto min = std::min_element(observed_values.begin(), observed_values.end(),
                                        [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

            return T(min->second);
            break;
        }

        case StatisticType::HISTO_SPREAD: {
            // positive difference between the number of times the most and least common values appear
            std::unordered_map<V, unsigned int> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto entry : object_property_values)
                observed_values[entry]++;

            auto [min, max] = std::minmax_element(observed_values.begin(), observed_values.end(),
                                                  [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

            return T(max->second - min->second);
            break;
        }

        case StatisticType::SUM: {
            V sum = std::accumulate(object_property_values.begin(), object_property_values.end(), V(0));
            return static_cast<T>(sum);
            break;
        }

        case StatisticType::MEAN: {
            V sum = std::accumulate(object_property_values.begin(), object_property_values.end(), V(0));
            return static_cast<T>(sum) / static_cast<T>(object_property_values.size());
            break;
        }

        case StatisticType::RMS: {
#if (defined(__clang_major__)) || (defined(__GNUC__) && (__GNUC__ < 11))
            V accumulator{0};
            for (auto entry : object_property_values)
                accumulator += (entry * entry);
#else
            V accumulator = std::transform_reduce(
                object_property_values.begin(), object_property_values.end(),
                V{0}, std::plus{}, [](const auto& a) { return a*a; });
#endif
            double tempval = static_cast<double>(accumulator) / object_property_values.size();
            return static_cast<T>(std::sqrt(tempval));
            break;
        }

        case StatisticType::MODE: {
            // value that appears the most often
            std::map<V, unsigned int> observed_values;
            for (auto& entry : object_property_values)
                observed_values[entry]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end());

            return T(max->first);
            break;
        }

        case StatisticType::MAX: {
            auto max = std::max_element(object_property_values.begin(), object_property_values.end());

            return static_cast<T>(*max);
            break;
        }

        case StatisticType::MIN: {
            auto min = std::min_element(object_property_values.begin(), object_property_values.end());

            return static_cast<T>(*min);
            break;
        }

        case StatisticType::SPREAD: {
            auto [min, max] = std::minmax_element(object_property_values.begin(), object_property_values.end());

            return static_cast<T>(*max - *min);
            break;
        }

        case StatisticType::STDEV: {
            if (object_property_values.size() < 2)
                return T{0};

            // find sample mean
            double accumulator = 0.0;
            for (auto entry : object_property_values)
                accumulator += static_cast<double>(entry);

            double MEAN = accumulator / object_property_values.size();

            // find average of squared deviations from sample mean
            accumulator = 0.0;
            for (auto entry : object_property_values)
                accumulator += (static_cast<double>(entry) - MEAN) * (static_cast<double>(entry) - MEAN);

            double retval = accumulator / static_cast<double>(object_property_values.size() - 1.0);
            return static_cast<T>(std::sqrt(retval));
            break;
        }

        case StatisticType::PRODUCT: {
            V accumulator(1);
            for (const auto& entry : object_property_values)
                accumulator *= entry;

            return static_cast<T>(accumulator);
            break;
        }

        default:
            throw std::runtime_error("ReduceData evaluated with an unknown or invalid StatisticType.");
            break;
    }
}

template <typename T, typename V> requires (std::is_enum_v<T> && std::is_same_v<T, V>)
T ReduceData(StatisticType stat_type, std::vector<T> object_property_values)
{
    if (object_property_values.empty())
        return T{0};

    // enum types T and V are the return value type and the property value type
    // so can calculate the most common value and return it

    switch (stat_type) {
        case StatisticType::IF: {
            // 1 if any objects have property values, else 0
            if (object_property_values.empty())
                return T{0};
            return T{1};
            break;
        }

        case StatisticType::MODE: {
            // value that appears the most often
            std::unordered_map<T, unsigned int> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto entry : object_property_values)
                observed_values[entry]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                        [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

            return max->first;
            break;
        }

        default:
            throw std::runtime_error("ReduceData evaluated with an unknown or invalid StatisticType.");
            break;
    }
}

template <typename T, typename V> requires (std::is_arithmetic_v<T> && !std::is_arithmetic_v<V>)
T ReduceData(StatisticType stat_type, std::vector<V> object_property_values)
{
    if (object_property_values.empty())
        return T{0};

    // return value type T is a number and the object property value type V is
    // not a numeric type, such as std::string or an enum type, so can calculate
    // various histogram properties that are not enum type valued.

    switch (stat_type) {
        case StatisticType::IF: {
            // 1 if any objects have property values, else 0
            if (object_property_values.empty())
                return T{0};
            return T{1};
            break;
        }

        case StatisticType::COUNT: {
            // how many objects / values
            return T(object_property_values.size());
            break;
        }

        case StatisticType::UNIQUE_COUNT: {
            // how many unique values appear
            std::unordered_set<V> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto& entry : object_property_values)
                observed_values.insert(std::move(entry));

            return T(observed_values.size());
            break;
        }

        case StatisticType::HISTO_MAX: {
            // number of times the most common value appears
            std::unordered_map<V, unsigned int> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto& entry : object_property_values)
                observed_values[std::move(entry)]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                        [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

            return T(max->second);
            break;
        }

        case StatisticType::HISTO_MIN: {
            // number of times the least common value appears
            std::unordered_map<V, unsigned int> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto& entry : object_property_values)
                observed_values[std::move(entry)]++;

            auto min = std::min_element(observed_values.begin(), observed_values.end(),
                                        [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

            return T(min->second);
            break;
        }

        case StatisticType::HISTO_SPREAD: {
            // positive difference between the number of times the most and least common values appear
            std::unordered_map<V, unsigned int> observed_values;
            observed_values.reserve(object_property_values.size());
            for (auto& entry : object_property_values)
                observed_values[std::move(entry)]++;

            auto [min, max] = std::minmax_element(
                observed_values.begin(), observed_values.end(),
                [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

            return T(max->second - min->second);
            break;
        }

        default:
            throw std::runtime_error("ReduceData evaluated with an unknown or invalid StatisticType.");
            break;
    }
}

template <typename T, typename V>
T Statistic<T, V>::Eval(const ScriptingContext& context) const
{
    // these two statistic types don't depend on the object property values,
    // so can be evaluated without getting those values.
    if (m_stat_type == StatisticType::IF)
        return (m_sampling_condition && m_sampling_condition->EvalAny(context)) ? T{1} : T{0};

    const auto condition_matches = m_sampling_condition ?
        m_sampling_condition->Eval(context) : Condition::ObjectSet{};

    if (m_stat_type == StatisticType::COUNT)
        return static_cast<T>(condition_matches.size());

    // evaluate property for each condition-matched object
    return ReduceData<T, V>(m_stat_type, GetObjectPropertyValues(context, condition_matches));
}

template <typename T, typename V>
uint32_t Statistic<T, V>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Statistic");
    CheckSums::CheckSumCombine(retval, m_stat_type);
    CheckSums::CheckSumCombine(retval, m_sampling_condition);
    CheckSums::CheckSumCombine(retval, m_value_ref);
    TraceLogger() << "GetCheckSum(Statisic<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API std::string Statistic<std::string, std::string>::Eval(const ScriptingContext& context) const;

///////////////////////////////////////////////////////////
// TotalFighterShots (of a carrier during one battle)    //
///////////////////////////////////////////////////////////

// Defining implementation here leads to ODR-hell

///////////////////////////////////////////////////////////
// ComplexVariable                                       //
///////////////////////////////////////////////////////////
template <typename T>
template <typename S>
ComplexVariable<T>::ComplexVariable(S&& variable_name,
                                    std::unique_ptr<ValueRef<int>>&& int_ref1,
                                    std::unique_ptr<ValueRef<int>>&& int_ref2,
                                    std::unique_ptr<ValueRef<int>>&& int_ref3,
                                    std::unique_ptr<ValueRef<std::string>>&& string_ref1,
                                    std::unique_ptr<ValueRef<std::string>>&& string_ref2,
                                    bool return_immediate_value) :
    Variable<T>(ReferenceType::NON_OBJECT_REFERENCE, std::forward<S>(variable_name), return_immediate_value),
    m_int_ref1(std::move(int_ref1)),
    m_int_ref2(std::move(int_ref2)),
    m_int_ref3(std::move(int_ref3)),
    m_string_ref1(std::move(string_ref1)),
    m_string_ref2(std::move(string_ref2))
{ InitInvariants(); }

template <typename T>
ComplexVariable<T>::ComplexVariable(const char* variable_name,
                                    std::unique_ptr<ValueRef<int>>&& int_ref1,
                                    std::unique_ptr<ValueRef<int>>&& int_ref2,
                                    std::unique_ptr<ValueRef<int>>&& int_ref3,
                                    std::unique_ptr<ValueRef<std::string>>&& string_ref1,
                                    std::unique_ptr<ValueRef<std::string>>&& string_ref2,
                                    bool return_immediate_value) :
    Variable<T>(ReferenceType::NON_OBJECT_REFERENCE, variable_name, return_immediate_value),
    m_int_ref1(std::move(int_ref1)),
    m_int_ref2(std::move(int_ref2)),
    m_int_ref3(std::move(int_ref3)),
    m_string_ref1(std::move(string_ref1)),
    m_string_ref2(std::move(string_ref2))
{ InitInvariants(); }

template <typename T>
ComplexVariable<T>::ComplexVariable(const ComplexVariable<T>& rhs) :
    Variable<T>(rhs.m_ref_type, rhs.m_property_name, rhs.m_return_immediate_value),
    m_int_ref1(CloneUnique(rhs.m_int_ref1)),
    m_int_ref2(CloneUnique(rhs.m_int_ref2)),
    m_int_ref3(CloneUnique(rhs.m_int_ref3)),
    m_string_ref1(CloneUnique(rhs.m_string_ref1)),
    m_string_ref2(CloneUnique(rhs.m_string_ref2))
{
    this->m_root_candidate_invariant = rhs.m_root_candidate_invariant;
    this->m_local_candidate_invariant = rhs.m_local_candidate_invariant;
    this->m_target_invariant = rhs.m_target_invariant;
    this->m_source_invariant = rhs.m_source_invariant;
    // this->m_constant_expr and this->m_simple_increment should always be false
}

template <typename T>
void ComplexVariable<T>::InitInvariants()
{
    std::initializer_list<const ValueRefBase*> refs =
        { m_int_ref1.get(), m_int_ref2.get(), m_int_ref3.get(), m_string_ref1.get(), m_string_ref2.get() };
    this->m_root_candidate_invariant = std::all_of(refs.begin(), refs.end(), [](const auto& e) { return !e || e->RootCandidateInvariant(); });
    this->m_local_candidate_invariant = std::all_of(refs.begin(), refs.end(), [](const auto& e) { return !e || e->LocalCandidateInvariant(); });
    this->m_target_invariant = std::all_of(refs.begin(), refs.end(), [](const auto& e) { return !e || e->TargetInvariant(); });
    this->m_source_invariant = std::all_of(refs.begin(), refs.end(), [](const auto& e) { return !e || e->SourceInvariant(); });
    // this->m_constant_expr and this->m_simple_increment should always be false
}

template <typename T>
bool ComplexVariable<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ComplexVariable<T>& rhs_ = static_cast<const ComplexVariable<T>&>(rhs);

    if (this->m_property_name != rhs_.m_property_name)
        return false;
    if (this->m_return_immediate_value != rhs_.m_return_immediate_value)
        return false;

    if (m_int_ref1 == rhs_.m_int_ref1) {
        // check next member
    } else if (!m_int_ref1 || !rhs_.m_int_ref1) {
        return false;
    } else {
        if (*m_int_ref1 != *(rhs_.m_int_ref1))
            return false;
    }

    if (m_int_ref2 == rhs_.m_int_ref2) {
        // check next member
    } else if (!m_int_ref2 || !rhs_.m_int_ref2) {
        return false;
    } else {
        if (*m_int_ref2 != *(rhs_.m_int_ref2))
            return false;
    }

    if (m_int_ref3 == rhs_.m_int_ref3) {
        // check next member
    } else if (!m_int_ref3 || !rhs_.m_int_ref3) {
        return false;
    } else {
        if (*m_int_ref3 != *(rhs_.m_int_ref3))
            return false;
    }

    if (m_string_ref1 == rhs_.m_string_ref1) {
        // check next member
    } else if (!m_string_ref1 || !rhs_.m_string_ref1) {
        return false;
    } else {
        if (*m_string_ref1 != *(rhs_.m_string_ref1))
            return false;
    }

    if (m_string_ref2 == rhs_.m_string_ref2) {
        // check next member
    } else if (!m_string_ref2 || !rhs_.m_string_ref2) {
        return false;
    } else {
        if (*m_string_ref2 != *(rhs_.m_string_ref2))
            return false;
    }

    return true;
}

template <typename T>
std::string ComplexVariable<T>::Description() const
{
    std::string retval = ComplexVariableDescription(
        this->m_property_name,
        m_int_ref1 ? m_int_ref1.get() : nullptr,
        m_int_ref2 ? m_int_ref2.get() : nullptr,
        m_int_ref3 ? m_int_ref3.get() : nullptr,
        m_string_ref1 ? m_string_ref1.get() : nullptr,
        m_string_ref2 ? m_string_ref2.get() : nullptr);
    if (retval.empty())
        return Dump();
    return retval;
}

template <typename T>
std::string ComplexVariable<T>::Dump(uint8_t ntabs) const
{
    return ComplexVariableDump(this->m_property_name,
                               m_int_ref1 ? m_int_ref1.get() : nullptr,
                               m_int_ref2 ? m_int_ref2.get() : nullptr,
                               m_int_ref3 ? m_int_ref3.get() : nullptr,
                               m_string_ref1 ? m_string_ref1.get() : nullptr,
                               m_string_ref2 ? m_string_ref2.get() : nullptr);
}

template <typename T>
void ComplexVariable<T>::SetTopLevelContent(const std::string& content_name)
{
    if (m_int_ref1)
        m_int_ref1->SetTopLevelContent(content_name);
    if (m_int_ref2)
        m_int_ref2->SetTopLevelContent(content_name);
    if (m_int_ref3)
        m_int_ref3->SetTopLevelContent(content_name);
    if (m_string_ref1)
        m_string_ref1->SetTopLevelContent(content_name);
    if (m_string_ref2)
        m_string_ref2->SetTopLevelContent(content_name);
}

template <typename T>
uint32_t ComplexVariable<T>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::ComplexVariable");
    CheckSums::CheckSumCombine(retval, m_int_ref1);
    CheckSums::CheckSumCombine(retval, m_int_ref2);
    CheckSums::CheckSumCombine(retval, m_int_ref3);
    CheckSums::CheckSumCombine(retval, m_string_ref1);
    CheckSums::CheckSumCombine(retval, m_string_ref2);
    TraceLogger() << "GetCheckSum(ComplexVariable<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API PlanetSize ComplexVariable<PlanetSize>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API PlanetType ComplexVariable<PlanetType>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API PlanetEnvironment ComplexVariable<PlanetEnvironment>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API UniverseObjectType ComplexVariable<UniverseObjectType>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API StarType ComplexVariable<StarType>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::vector<std::string> ComplexVariable<std::vector<std::string>>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API Visibility ComplexVariable<Visibility>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API double ComplexVariable<double>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API int ComplexVariable<int>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string ComplexVariable<std::string>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string ComplexVariable<std::vector<std::string>>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<Visibility>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<double>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<int>::Dump(uint8_t ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<std::string>::Dump(uint8_t ntabs) const;


///////////////////////////////////////////////////////////
// StaticCast                                            //
///////////////////////////////////////////////////////////
template <typename FromType, typename ToType>
template <typename T> requires (std::is_convertible_v<T, std::unique_ptr<Variable<FromType>>>)
StaticCast<FromType, ToType>::StaticCast(T&& value_ref) :
    Variable<ToType>(value_ref->GetReferenceType(), value_ref->PropertyName()),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
    this->m_constant_expr = !m_value_ref || m_value_ref->ConstantExpr();
    // this->m_simple_increment should always be false
}

template <typename FromType, typename ToType>
template <typename T> requires(std::is_convertible_v<T, std::unique_ptr<ValueRef<FromType>>> &&
                               !std::is_convertible_v<T, std::unique_ptr<Variable<FromType>>>)
StaticCast<FromType, ToType>::StaticCast(T&& value_ref) :
    Variable<ToType>(ReferenceType::NON_OBJECT_REFERENCE),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
    this->m_constant_expr = !m_value_ref || m_value_ref->ConstantExpr();
    // this->m_simple_increment should always be false
}

template <typename FromType, typename ToType>
bool StaticCast<FromType, ToType>::operator==(const ValueRef<ToType>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const StaticCast<FromType, ToType>& rhs_ =
        static_cast<const StaticCast<FromType, ToType>&>(rhs);

    if (m_value_ref == rhs_.m_value_ref) {
        // check next member
    } else if (!m_value_ref || !rhs_.m_value_ref) {
        return false;
    } else {
        if (*m_value_ref != *(rhs_.m_value_ref))
            return false;
    }

    return true;
}

template <typename FromType, typename ToType>
ToType StaticCast<FromType, ToType>::Eval(const ScriptingContext& context) const
{ return static_cast<ToType>(m_value_ref->Eval(context)); }

template <typename FromType, typename ToType>
std::string StaticCast<FromType, ToType>::Description() const
{ return m_value_ref->Description(); }

template <typename FromType, typename ToType>
std::string StaticCast<FromType, ToType>::Dump(uint8_t ntabs) const
{ return "(" + m_value_ref->Dump(ntabs) + ") // StaticCast{" + typeid(FromType).name() + "," + typeid(ToType).name() + "}\n" + DumpIndent(ntabs + 1); }

template <typename FromType, typename ToType>
void StaticCast<FromType, ToType>::SetTopLevelContent(const std::string& content_name)
{
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <typename FromType, typename ToType>
uint32_t StaticCast<FromType, ToType>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::StaticCast");
    CheckSums::CheckSumCombine(retval, m_value_ref);
    TraceLogger() << "GetCheckSum(StaticCast<FromType, ToType>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// StringCast                                            //
///////////////////////////////////////////////////////////
template <typename FromType>
StringCast<FromType>::StringCast(std::unique_ptr<ValueRef<FromType>>&& value_ref) :
    Variable<std::string>(
        [ref{value_ref.get()}]() -> ReferenceType {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->GetReferenceType();
            else
                return ReferenceType::NON_OBJECT_REFERENCE;
        }(),
        [ref{value_ref.get()}]() -> std::vector<std::string> {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->PropertyName();
            else
                return {};
        }()),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
    this->m_constant_expr = !m_value_ref || m_value_ref->ConstantExpr();
    // this->m_simple_increment should always be false
}

template <typename FromType>
bool StringCast<FromType>::operator==(const ValueRef<std::string>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    auto& rhs_ = static_cast<const StringCast<FromType>&>(rhs);

    if (m_value_ref == rhs_.m_value_ref) {
        // check next member
    } else if (!m_value_ref || !rhs_.m_value_ref) {
        return false;
    } else {
        if (*m_value_ref != *(rhs_.m_value_ref))
            return false;
    }

    return true;
}

template <typename FromType>
std::string StringCast<FromType>::Eval(const ScriptingContext& context) const
{
    if (!m_value_ref)
        return "";
    const auto value = m_value_ref->Eval(context);

    if constexpr (std::is_same_v<FromType, std::string>) {
        return value;
    } else if constexpr (std::is_enum_v<FromType>) {
        return std::string{to_string(value)};
    } else if constexpr (std::is_arithmetic_v<FromType>) {
        return std::to_string(value);
    } else if constexpr (std::is_same_v<FromType, std::vector<std::string>>) {
        std::string retval;
        retval.reserve(16*value.size()); // TODO: sum sizes of value to reserve
        std::for_each(value.begin(), value.end(),
                      [&retval](const auto& v) { retval.append(v).append(" "); });
        return retval;
    } else {
        try {
            return boost::lexical_cast<std::string>(value);
        } catch (...) {
            return "";
        }
    }
}

template <typename FromType>
uint32_t StringCast<FromType>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::StringCast");
    CheckSums::CheckSumCombine(retval, m_value_ref);
    TraceLogger() << "GetCheckSum(StringCast<FromType>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API std::string StringCast<double>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string StringCast<int>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string StringCast<std::vector<std::string>>::Eval(const ScriptingContext& context) const;

template <typename FromType>
std::string StringCast<FromType>::Description() const
{ return m_value_ref->Description(); }

template <typename FromType>
std::string StringCast<FromType>::Dump(uint8_t ntabs) const
{
    return "(" + m_value_ref->Dump(ntabs) + ") // StringCast{"
        + typeid(FromType).name() + "}\n" + DumpIndent(ntabs + 1);
}

template <typename FromType>
void StringCast<FromType>::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// UserStringLookup                                      //
///////////////////////////////////////////////////////////
template <typename FromType>
UserStringLookup<FromType>::UserStringLookup(std::unique_ptr<ValueRef<FromType>>&& value_ref) :
    Variable<std::string>(
        [ref{value_ref.get()}]() -> ReferenceType {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->GetReferenceType();
            else
                return ReferenceType::NON_OBJECT_REFERENCE;
        }(),
        [ref{value_ref.get()}]() -> std::vector<std::string> {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->PropertyName();
            else
                return {};
        }()),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
    this->m_constant_expr = !m_value_ref || m_value_ref->ConstantExpr();
    // this->m_simple_increment should always be false
}

template <typename FromType>
bool UserStringLookup<FromType>::operator==(const ValueRef<std::string>& rhs) const {
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const UserStringLookup& rhs_ = static_cast<const UserStringLookup&>(rhs);

    if (m_value_ref == rhs_.m_value_ref) {
        // check next member
    }
    else if (!m_value_ref || !rhs_.m_value_ref) {
        return false;
    }
    else {
        if (*m_value_ref != *(rhs_.m_value_ref))
            return false;
    }

    return true;
}

template <typename FromType>
std::string UserStringLookup<FromType>::Eval(const ScriptingContext& context) const {
    if (!m_value_ref)
        return "";
    auto ref_val = to_string(m_value_ref->Eval(context));
    if (ref_val.empty() || !UserStringExists(ref_val))
        return "";
    return UserString(ref_val);
}

template <>
FO_COMMON_API std::string UserStringLookup<std::string>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string UserStringLookup<std::vector<std::string>>::Eval(const ScriptingContext& context) const;

template <typename FromType>
std::string UserStringLookup<FromType>::Description() const
{ return m_value_ref->Description(); }

template <typename FromType>
std::string UserStringLookup<FromType>::Dump(uint8_t ntabs) const
{ return m_value_ref->Dump(ntabs); }

template <typename FromType>
void UserStringLookup<FromType>::SetTopLevelContent(const std::string& content_name)
{
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <typename FromType>
uint32_t UserStringLookup<FromType>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::UserStringLookup");
    CheckSums::CheckSumCombine(retval, m_value_ref);
    TraceLogger() << "GetCheckSum(UserStringLookup<FromType>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
template <typename T>
Operation<T>::Operation(OpType op_type,
                        std::unique_ptr<ValueRef<T>>&& operand1,
                        std::unique_ptr<ValueRef<T>>&& operand2) :
    Operation(op_type, [&operand1, &operand2]() {
                std::remove_const_t<decltype(m_operands)> retval{};
                retval.reserve(2);
                retval.push_back(std::move(operand1));
                retval.push_back(std::move(operand2));
                return retval;
              }())
{}

template <typename T>
Operation<T>::Operation(OpType op_type, std::unique_ptr<ValueRef<T>>&& operand) :
    Operation(op_type, [&operand]() {
                std::remove_const_t<decltype(m_operands)> retval{};
                retval.push_back(std::move(operand));
                return retval;
              }())
{}

template <typename T>
Operation<T>::Operation(const Operation<T>& rhs) :
    m_op_type(rhs.m_op_type),
    m_operands(CloneUnique(rhs.m_operands)),
    m_cached_const_value(rhs.m_cached_const_value)
{
    this->m_root_candidate_invariant = rhs.m_root_candidate_invariant;
    this->m_local_candidate_invariant = rhs.m_local_candidate_invariant;
    this->m_target_invariant = rhs.m_target_invariant;
    this->m_source_invariant = rhs.m_source_invariant;
    this->m_constant_expr = rhs.m_constant_expr;
    this->m_simple_increment = rhs.m_simple_increment; // this is the only type of ValueRef that can be a simple increment ref
}

template <typename T>
void Operation<T>::InitInvariants()
{
    if (m_op_type == OpType::RANDOM_UNIFORM || m_op_type == OpType::RANDOM_PICK || m_op_type == OpType::NOOP) {
        // all defaults for invariants = false should apply
        //this->m_root_candidate_invariant = false;
        //this->m_local_candidate_invariant = false;
        //this->m_target_invariant = false;
        //this->m_source_invariant = false;
        //this->m_simple_increment = false;
        return;
    }

    this->m_root_candidate_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->RootCandidateInvariant(); });
    this->m_local_candidate_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->LocalCandidateInvariant(); });
    this->m_target_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->TargetInvariant(); });
    this->m_source_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->SourceInvariant(); });


    // m_simple_increment = false by default
    // determine if this is a simple incrment operation, meaning it is a calculation
    // that depends only on:
    // 1) the effect target value (ie. a meter value or some other property that is
    //    being modified by an effect)
    // 2) a single target-invariant value (ie. a constant, something that depends only
    //    on the source object or a target-independent complex value ref)

    if (m_operands.size() != 2)
        return;
    auto& lhs{m_operands[0]};
    auto& rhs{m_operands[1]};
    if (!rhs || !lhs)
        return;
    // RHS must be the same value for all targets
    if (!rhs->TargetInvariant())
        return;
    // LHS must be just the immediate value of what's being incremented
    this->m_simple_increment = !this->m_constant_expr &&
        (lhs->GetReferenceType() == ReferenceType::EFFECT_TARGET_VALUE_REFERENCE);
}

template <typename T>
bool Operation<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Operation<T>& rhs_ = static_cast<const Operation<T>&>(rhs);

    if (m_op_type != rhs_.m_op_type)
        return false;
    if (m_operands.size() != rhs_.m_operands.size())
        return false;

    try {
        for (std::size_t idx = 0; idx < m_operands.size(); ++idx) {
            const auto& my_op = m_operands[idx];
            const auto& rhs_op = rhs_.m_operands[idx];

            if (my_op == rhs_op) // operands are unique_ptr so can only compare equal if nullptr
                continue;
            if (!my_op || !rhs_op)
                return false;
            if (*my_op != *rhs_op)
                return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

template <typename T>
const std::vector<ValueRef<T>*> Operation<T>::Operands() const
{
    std::vector<ValueRef<T>*> retval;
    retval.reserve(m_operands.size());
    std::transform(m_operands.begin(), m_operands.end(), std::back_inserter(retval),
                   [](const auto& xx){ return xx.get(); });
    return retval;
}

template <typename T>
T Operation<T>::EvalImpl(OpType op_type, const T lhs, const T rhs)
{
    switch (op_type) {
    case OpType::NOOP : {
        return lhs;
        break;
    }

    case OpType::TIMES: {
        // useful for writing a "Statistic If" expression with arbitrary types.
        // If returns T{0} or T{1} for nothing or something matching the
        // sampling condition. This can be checked here by returning T{0} if
        // the LHS operand is T{0} and just returning RHS() otherwise.
        return (lhs == T{0}) ? T{0} : rhs;
        break;
    }

    case OpType::MAXIMUM: {
        return std::max<T>(lhs, rhs);
        break;
    }
    case OpType::MINIMUM: {
        return std::min<T>(lhs, rhs);
        break;
    }

    case OpType::RANDOM_PICK: {
        return (RandInt(0, 1) == 0) ? lhs : rhs;
        break;
    }

    case OpType::COMPARE_EQUAL: {
        return (lhs == rhs) ? T{1} : T{0};
        break;
    }
    case OpType::COMPARE_GREATER_THAN: {
        return (lhs > rhs) ? T{1} : T{0};
        break;
    }
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL: {
        return (lhs >= rhs) ? T{1} : T{0};
        break;
    }
    case OpType::COMPARE_LESS_THAN: {
        return (lhs < rhs) ? T{1} : T{0};
        break;
    }
    case OpType::COMPARE_LESS_THAN_OR_EQUAL: {
        return (lhs <= rhs) ? T{1} : T{0};
        break;
    }
    case OpType::COMPARE_NOT_EQUAL:  {
        return (lhs != rhs) ? T{1} : T{0};
        break;
    }

    default:
        break;
    }
    throw std::runtime_error("ValueRef::Operation<T>::EvalImpl evaluated with an unknown or invalid OpType.");
}

template <>
FO_COMMON_API std::string Operation<std::string>::EvalImpl(OpType op_type, std::string lhs, std::string rhs);

template <>
FO_COMMON_API double Operation<double>::EvalImpl(OpType op_type, double lhs, double rhs);

template <>
FO_COMMON_API double Operation<double>::EvalImpl(const ScriptingContext& context) const;

template <typename T>
T Operation<T>::EvalImpl(const ScriptingContext& context) const
{
    if (this->m_simple_increment)
        return EvalImpl(m_op_type, LHS()->Eval(context), RHS()->Eval(context));

    switch (m_op_type) {
    case OpType::NOOP : {
        DebugLogger() << "ValueRef::Operation<T>::NoOp::EvalImpl";
        auto retval = LHS()->Eval(context);
        DebugLogger() << "ValueRef::Operation<T>::NoOp::EvalImpl. Sub-Expression returned: " << retval
                        << " from: " << LHS()->Dump();
        return retval;
    }

    case OpType::TIMES: {
        // useful for writing a "Statistic If" expression with arbitrary types.
        // If returns T{0} or T{1} for nothing or something matching the
        // sampling condition. This can be checked here by returning T{0} if
        // the LHS operand is T{0} and just returning RHS() otherwise.
        if (LHS()->Eval(context) == T{0})
            return T{0};
        return RHS()->Eval(context);
        break;
    }

    case OpType::MAXIMUM:
    case OpType::MINIMUM: {
        if (m_operands.empty())
            return T{-1};

        // evaluate all operands, return smallest or biggest
        std::vector<T> vals;
        vals.reserve(m_operands.size());
        for (auto& vr : m_operands) {
            if (vr)
                vals.push_back(vr->Eval(context));
        }
        if (m_op_type == OpType::MINIMUM)
            return *std::min_element(vals.begin(), vals.end());
        else
            return *std::max_element(vals.begin(), vals.end());
        break;
    }

    case OpType::RANDOM_PICK: {
        // select one operand, evaluate it, return result
        if (m_operands.empty())
            return T{-1};   // should be INVALID_T of enum types
        auto idx = RandInt(0, static_cast<int>(m_operands.size()) - 1);
        auto& vr = *std::next(m_operands.begin(), idx);
        if (!vr)
            return T{-1};   // should be INVALID_T of enum types
        return vr->Eval(context);
        break;
    }

    case OpType::COMPARE_EQUAL:
    case OpType::COMPARE_GREATER_THAN:
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL:
    case OpType::COMPARE_LESS_THAN:
    case OpType::COMPARE_LESS_THAN_OR_EQUAL:
    case OpType::COMPARE_NOT_EQUAL: {
        T lhs_val = LHS()->Eval(context);
        T rhs_val = RHS()->Eval(context);
        if (m_operands.size() == 2)
            return EvalImpl(m_op_type, lhs_val, rhs_val);

        bool test_result = false;
        switch (m_op_type) {
            case OpType::COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
            case OpType::COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
            case OpType::COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
            case OpType::COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
            case OpType::COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
            case OpType::COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
            default:    break;  // ??? do nothing, default to false
        }

        if (m_operands.size() == 3) {
            if (test_result)
                return m_operands[2]->Eval(context);
            else
                return T{0};
        } else {
            if (test_result)
                return m_operands[2]->Eval(context);
            else
                return m_operands[3]->Eval(context);
        }
        break;
    }

    default:
        break;
    }

    throw std::runtime_error("ValueRef::Operation<T>::EvalImpl evaluated with an unknown or invalid OpType.");
}

template <typename T>
T Operation<T>::Eval(const ScriptingContext& context) const
{
    if (this->m_constant_expr)
        return m_cached_const_value;
    return this->EvalImpl(context);
}

template <typename T>
uint32_t Operation<T>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Operation");
    CheckSums::CheckSumCombine(retval, m_op_type);
    CheckSums::CheckSumCombine(retval, m_operands);
    // derived member values should not be part of checksums
    // e.g. the invariants and m_cached_const_value
    TraceLogger() << "GetCheckSum(Operation<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API int Operation<int>::EvalImpl(OpType op_type, int lhs, int rhs);

template <>
FO_COMMON_API std::string Operation<std::string>::EvalImpl(const ScriptingContext& context) const;

template <>
FO_COMMON_API int Operation<int>::EvalImpl(const ScriptingContext& context) const;

template <typename T>
Operation<T>::Operation(OpType op_type, std::vector<std::unique_ptr<ValueRef<T>>>&& operands) :
    ValueRef<T>(op_type != OpType::RANDOM_UNIFORM &&
                op_type != OpType::RANDOM_PICK &&
                op_type != OpType::NOOP &&
                std::all_of(operands.begin(), operands.end(),
                            [](const auto& operand) { return operand && operand->ConstantExpr(); })),
    m_op_type(op_type),
    m_operands(std::move(operands)),
    m_cached_const_value(this->m_constant_expr ? this->EvalImpl(ScriptingContext{}) : T{})
{
    if (std::any_of(m_operands.begin(), m_operands.end(), [](const auto& op) -> bool { return !op; }))
        throw std::invalid_argument("Operation passed null operand");
    InitInvariants();
}

template <typename T>
std::string Operation<T>::Description() const
{
    if (m_op_type == OpType::NEGATE) {
        if (auto rhs = dynamic_cast<const Operation<T>*>(LHS())) {
            OpType op_type = rhs->GetOpType();
            if (op_type == OpType::PLUS   || op_type == OpType::MINUS ||
                op_type == OpType::TIMES  || op_type == OpType::DIVIDE ||
                op_type == OpType::NEGATE || op_type == OpType::EXPONENTIATE ||
                op_type == OpType::REMAINDER)
            { return "-(" + LHS()->Description() + ")"; }
        } else {
            return "-" + LHS()->Description();
        }
    }

    if (m_op_type == OpType::NOOP)
        return LHS()->Description();
    if (m_op_type == OpType::ABS)
        return "abs(" + LHS()->Description() + ")";
    if (m_op_type == OpType::LOGARITHM)
        return "log(" + LHS()->Description() + ")";
    if (m_op_type == OpType::SINE)
        return "sin(" + LHS()->Description() + ")";
    if (m_op_type == OpType::COSINE)
        return "cos(" + LHS()->Description() + ")";

    if (m_op_type == OpType::MINIMUM) {
        std::string retval = "min(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }
    if (m_op_type == OpType::MAXIMUM) {
        std::string retval = "max(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == OpType::RANDOM_UNIFORM)
        return "RandomNumber(" + LHS()->Description() + ", " + RHS()->Description() + ")";

    if (m_op_type == OpType::RANDOM_PICK) {
        std::string retval = "OneOf(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == OpType::ROUND_NEAREST)
        return "round(" + LHS()->Description() + ")";
    if (m_op_type == OpType::ROUND_UP)
        return "ceil(" + LHS()->Description() + ")";
    if (m_op_type == OpType::ROUND_DOWN)
        return "floor(" + LHS()->Description() + ")";
    if (m_op_type == OpType::SIGN)
        return "sign(" + LHS()->Description() + ")";

    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (auto lhs = dynamic_cast<const Operation<T>*>(LHS())) {
        OpType op_type = lhs->GetOpType();
        if (
            (m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES   || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS   || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((m_op_type == OpType::TIMES     || m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS        || op_type == OpType::MINUS))  || op_type == OpType::NEGATE)
           )
            parenthesize_lhs = true;
    }
    if (auto rhs = dynamic_cast<const Operation<T>*>(RHS())) {
        OpType op_type = rhs->GetOpType();
        if (
            (m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES   || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS   || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((m_op_type == OpType::TIMES     || m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS        || op_type == OpType::MINUS))  || op_type == OpType::NEGATE)
           )
            parenthesize_rhs = true;
    }

    std::string retval;
    if (parenthesize_lhs)
        retval += '(' + LHS()->Description() + ')';
    else
        retval += LHS()->Description();

    switch (m_op_type) {
    case OpType::PLUS:         retval += " + "; break;
    case OpType::MINUS:        retval += " - "; break;
    case OpType::TIMES:        retval += " * "; break;
    case OpType::DIVIDE:       retval += " / "; break;
    case OpType::REMAINDER:    retval += " % "; break;
    case OpType::EXPONENTIATE: retval += " ^ "; break;
    default:                   retval += " ? "; break;
    }

    if (parenthesize_rhs)
        retval += '(' + RHS()->Description() + ')';
    else
        retval += RHS()->Description();

    return retval;
}

template <typename T>
std::string Operation<T>::Dump(uint8_t ntabs) const
{
    if (m_op_type == OpType::NEGATE)
        return "-(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == OpType::NOOP)
        return "(" + LHS()->Dump() + ")";
    if (m_op_type == OpType::ABS)
        return "abs(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == OpType::LOGARITHM)
        return "log(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == OpType::SINE)
        return "sin(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == OpType::COSINE)
        return "cos(" + LHS()->Dump(ntabs) + ")";

    if (m_op_type == OpType::MINIMUM) {
        std::string retval = "min(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }
    if (m_op_type == OpType::MAXIMUM) {
        std::string retval = "max(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == OpType::RANDOM_UNIFORM)
        return "RandomNumber(" + LHS()->Dump(ntabs) + ", " + LHS()->Dump(ntabs) + ")";

    if (m_op_type == OpType::RANDOM_PICK) {
        std::string retval = "randompick(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == OpType::ROUND_NEAREST)
        return "round(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == OpType::ROUND_UP)
        return "ceil(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == OpType::ROUND_DOWN)
        return "floor(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == OpType::SIGN)
        return "sign(" + LHS()->Dump(ntabs) + ")";

    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (auto lhs = dynamic_cast<const Operation<T>*>(LHS())) {
        OpType op_type = lhs->GetOpType();
        if (
            (m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES  || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS  || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((m_op_type == OpType::TIMES     || m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS        || op_type == OpType::MINUS))  || op_type == OpType::NEGATE)
           )
            parenthesize_lhs = true;
    }
    if (auto rhs = dynamic_cast<const Operation<T>*>(RHS())) {
        OpType op_type = rhs->GetOpType();
        if (
            (m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES   || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS   || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((m_op_type == OpType::TIMES     || m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS        || op_type == OpType::MINUS))  || op_type == OpType::NEGATE)
           )
            parenthesize_rhs = true;
    }

    std::string retval;
    if (parenthesize_lhs)
        retval += '(' + LHS()->Dump(ntabs) + ')';
    else
        retval += LHS()->Dump(ntabs);

    switch (m_op_type) {
    case OpType::PLUS:         retval += " + "; break;
    case OpType::MINUS:        retval += " - "; break;
    case OpType::TIMES:        retval += " * "; break;
    case OpType::DIVIDE:       retval += " / "; break;
    case OpType::REMAINDER:    retval += " % "; break;
    case OpType::EXPONENTIATE: retval += " ^ "; break;
    default:                   retval += " ? "; break;
    }

    if (parenthesize_rhs)
        retval += '(' + RHS()->Dump(ntabs) + ')';
    else
        retval += RHS()->Dump(ntabs);

    return retval;
}

template <typename T>
void Operation<T>::SetTopLevelContent(const std::string& content_name) {
    for (auto& operand : m_operands) {
        if (operand)
            operand->SetTopLevelContent(content_name);
    }
}

}


#endif
