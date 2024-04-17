#ifndef _ValueRefs_h_
#define _ValueRefs_h_


#include <concepts>
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
#include "ValueRef.h"
#include "../util/CheckSums.h"
#include "../util/Export.h"
#include "../util/i18n.h"
#include "../util/Random.h"

#if !defined(CONSTEXPR_STRING)
#  if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
#    define CONSTEXPR_STRING constexpr
#  else
#    define CONSTEXPR_STRING
#  endif
#endif

class UniverseObject;

namespace ValueRef {
/** the constant value leaf ValueRef class. */
template <typename T>
struct FO_COMMON_API Constant final : public ValueRef<T>
{
    template <typename TT> requires (std::is_convertible_v<TT, T>)
    [[nodiscard]] constexpr explicit Constant(TT&& value) noexcept(noexcept(std::string{}) && noexcept(T{std::declval<TT>()})) :
        ValueRef<T>(true, true, true, true, true),
        m_value(std::forward<TT>(value))
    {
        static_assert(std::is_nothrow_move_constructible_v<T>);
    }

    [[nodiscard]] constexpr explicit Constant(const Constant& value)
        noexcept(noexcept(std::string{}) && noexcept(T{std::declval<const T>()})) :
        ValueRef<T>(value),
        m_value(value.m_value)
    {}

#if defined(__GNUC__) && (__GNUC__ < 13)
    constexpr ~Constant() noexcept override {} // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
#endif

    [[nodiscard]] constexpr bool operator==(const ValueRef<T>& rhs) const noexcept override {
        if (&rhs == this)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *this == *rhs_p;
    }
    [[nodiscard]] constexpr bool operator==(const Constant& rhs) const noexcept {
        if (&rhs == this)
            return true;
        if (!this->ValueRefBase::operator==(static_cast<const ValueRefBase&>(rhs)))
            return false;
        return m_value == rhs.m_value;
    }

    [[nodiscard]] constexpr T Eval() const noexcept(noexcept(T{std::declval<const T>()})) override
    { return m_value; }
    [[nodiscard]] T Eval(const ScriptingContext&) const noexcept(noexcept(T{std::declval<const T>()})) override
    { return m_value; }

    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override
    { return DumpIndent(ntabs) + Description(); }

    [[nodiscard]] constexpr T Value() const noexcept(noexcept(T{std::declval<const T>()})) { return m_value; };

    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override
    { return std::make_unique<Constant>(m_value); }

private:
    const T m_value{};
};

template <>
struct FO_COMMON_API Constant<std::string> final : public ValueRef<std::string>
{
    template <typename S> requires (std::is_convertible_v<S, std::string>)
    constexpr explicit Constant(S&& value)
        noexcept(noexcept(std::string{}) && noexcept(std::string{std::declval<S>()})) :
        ValueRef<std::string>(true, true, true, true, true),
        m_value(std::forward<S>(value))
    {
        static_assert(std::is_nothrow_move_constructible_v<std::string>);
    }

#if defined(__GNUC__) && (__GNUC__ == 12)
    // must be __GNUC__ > 11 for ~basic_string to be constexpr (and thus also ~Constant<string>
    // only needed for __GNUC__ < 13 to avoid compiler bug, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
    constexpr ~Constant() noexcept override {}
#endif

    [[nodiscard]] constexpr bool operator==(const ValueRef<std::string>& rhs) const override {
        if (&rhs == this)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        if (!rhs_p)
            return false;
        const auto& rhs_ = *rhs_p;
        return m_top_level_content == rhs_.m_top_level_content && m_value == rhs_.m_value;
    }

    static constexpr std::string_view current_content = "CurrentContent";
    static constexpr std::string_view no_current_content = "THERE_IS_NO_TOP_LEVEL_CONTENT";

    [[nodiscard]] CONSTEXPR_STRING std::string Eval() const noexcept(noexcept(std::string{std::declval<const std::string>()})) override
    { return (m_value == current_content) ? m_top_level_content : m_value; }

    [[nodiscard]] CONSTEXPR_STRING std::string Eval(const ScriptingContext&) const override
    { return Eval(); }

    [[nodiscard]] CONSTEXPR_STRING std::string Description() const override
    { return (m_value == current_content) ? m_top_level_content : m_value; }
    [[nodiscard]] CONSTEXPR_STRING std::string Dump(uint8_t ntabs = 0) const override
    { return "\"" + Description() + "\""; }

    void CONSTEXPR_STRING SetTopLevelContent(const std::string& content_name) override {
#if defined(__cpp_lib_is_constant_evaluated) && (!defined(__clang_major__) || (__clang_major__ >= 14))
        if (m_value == current_content && content_name == no_current_content) {
            [&content_name]() {
                if (!std::is_constant_evaluated()) {
                    ErrorLogger() << "Constant<std::string>::SetTopLevelContent()  Scripted Content illegal.  Trying to set "
                                  << no_current_content << " for "
                                  << current_content << " (maybe you tried to use "
                                  << current_content << " in named_values.focs.txt)";
                } else {
                    throw std::invalid_argument("Constant<std::string>::SetTopLevelContent() passed " + content_name);
                }
            }();
        }
#endif
        if (!m_top_level_content.empty() && m_top_level_content != no_current_content) {
#if defined(__cpp_lib_is_constant_evaluated) && (!defined(__clang_major__) || (__clang_major__ >= 14))
            [this, &content_name]() {
                if (!std::is_constant_evaluated())
                    ErrorLogger() << "Constant<std::string>::SetTopLevelContent() tried to overwrite top level content from '" << m_top_level_content << "' to '" << content_name << "'";
                else
                    throw std::runtime_error("Constant<std::string>::SetTopLevelContent() tried to overwrite already-set top level content");
            }();
#endif
        } else {
            m_top_level_content = content_name;
        }
    }

    [[nodiscard]] CONSTEXPR_STRING const auto& Value() const noexcept { return m_value; };
    [[nodiscard]] CONSTEXPR_STRING uint32_t GetCheckSum() const override {
        uint32_t retval{0};

        CheckSums::CheckSumCombine(retval, "ValueRef::Constant<string>");
        CheckSums::CheckSumCombine(retval, m_value);
#if defined(__cpp_lib_is_constant_evaluated) && (!defined(__clang_major__) || (__clang_major__ >= 14))
        if (!std::is_constant_evaluated())
            [type_id_name{typeid(*this).name()}, this, retval]() {
                TraceLogger() << "GetCheckSum(Constant<std::string>): " << type_id_name
                              << " value: " << Description() << " retval: " << retval;
            }();
#endif
        return retval;
    }

    [[nodiscard]] std::unique_ptr<ValueRef<std::string>> Clone() const override {
        auto retval = std::make_unique<Constant>(m_value);
        retval->m_top_level_content = m_top_level_content;
        return retval;
    }

private:
    const std::string m_value;
    std::string m_top_level_content; // if m_value is "CurrentContent", return this instead
};

enum class ValueToReturn : bool { Initial = false, Immediate = true };

/** The variable value ValueRef class.  The value returned by this node is
  * taken from the gamestate, most often from the Source or Target objects. */
template <typename T>
struct FO_COMMON_API Variable : public ValueRef<T>
{
    CONSTEXPR_STRING Variable(ReferenceType ref_type, std::string property_name,
                              ContainerType container_type = ContainerType::NONE,
                              ValueToReturn retval_type = ValueToReturn::Initial) noexcept :
        ValueRef<T>(false,
                    ref_type != ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE,
                    ref_type != ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE,
                    ref_type != ReferenceType::EFFECT_TARGET_REFERENCE && ref_type != ReferenceType::EFFECT_TARGET_VALUE_REFERENCE, // not all effect target values are properties of the target object, eg. empire meters, but most are
                    ref_type != ReferenceType::SOURCE_REFERENCE,
                    static_cast<bool>(retval_type),
                    ref_type,
                    container_type),
        m_property_name(std::move(property_name))
    {}

    CONSTEXPR_STRING explicit Variable(ReferenceType ref_type, ValueToReturn retval_type = ValueToReturn::Initial) noexcept :
        ValueRef<T>(false,
                    ref_type != ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE,
                    ref_type != ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE,
                    ref_type != ReferenceType::EFFECT_TARGET_REFERENCE && ref_type != ReferenceType::EFFECT_TARGET_VALUE_REFERENCE, // not all effect target values are properties of the target object, eg. empire meters, but most are
                    ref_type != ReferenceType::SOURCE_REFERENCE,
                    static_cast<bool>(retval_type),
                    ref_type)
    {}

#if defined(__GNUC__) && (__GNUC__ == 12)
    // must be __GNUC__ > 11 for ~basic_string to be constexpr (and thus also ~Variable)
    // only needed for __GNUC__ < 13 to avoid compiler bug, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
    constexpr ~Variable() noexcept override {}
#endif

    [[nodiscard]] constexpr bool operator==(const ValueRef<T>& rhs) const override {
        if (&rhs == this)
            return true;
        const auto* rhs_p = dynamic_cast<decltype(this)>(&rhs);
        return rhs_p && *rhs_p == *this;
    }
    [[nodiscard]] constexpr bool operator==(const Variable<T>& rhs) const noexcept {
        if (&rhs == this)
            return true;
        if (!this->ValueRefBase::operator==(static_cast<const ValueRefBase&>(rhs)))
            return false;
        return this->m_container_type == rhs.m_container_type && m_property_name == rhs.m_property_name;
    }

    [[nodiscard]] T Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] constexpr auto& PropertyName() const noexcept { return m_property_name; }
    [[nodiscard]] constexpr auto GetContainerType() const noexcept { return this->m_container_type; }
    [[nodiscard]] constexpr bool ReturnImmediateValue() const noexcept { return this->m_return_immediate_value; }

    [[nodiscard]] CONSTEXPR_STRING uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override {
        return std::make_unique<Variable<T>>(this->m_ref_type, m_property_name, this->m_container_type,
                                             this->m_return_immediate_value ? ValueToReturn::Immediate : ValueToReturn::Initial);
    }

protected:
    constexpr Variable(bool root_inv, bool local_inv, bool target_inv, bool source_inv, StatisticType stat_type) :
        ValueRef<T>(false, root_inv, local_inv, target_inv, source_inv, stat_type)
    {}

    const std::string m_property_name;
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

    [[nodiscard]] StatisticType GetStatisticType() const noexcept { return this->m_stat_type; }

    [[nodiscard]] const auto* GetSamplingCondition() const noexcept { return m_sampling_condition.get(); }

    [[nodiscard]] const auto* GetValueRef() const noexcept { return m_value_ref.get(); }

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override {
        return std::make_unique<Statistic<T, V>>(CloneUnique(m_value_ref),
                                                 this->m_stat_type,
                                                 CloneUnique(m_sampling_condition));
    }

protected:
    /** Evaluates the property for the specified objects. */
    std::vector<V> GetObjectPropertyValues(const ScriptingContext& context,
                                           const Condition::ObjectSet& objects) const;

private:
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

    bool                        operator==(const ValueRef<int>& rhs) const override;
    [[nodiscard]] int           Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string   Description() const override;
    [[nodiscard]] std::string   Dump(uint8_t ntabs = 0) const override;
    void                        SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const auto*   GetSamplingCondition() const noexcept { return m_sampling_condition.get(); }

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
    template<typename S> requires requires(S s) { std::vector<std::string>{s}; }
    explicit ComplexVariable(S&& variable_name,
                             std::unique_ptr<ValueRef<int>>&& int_ref1 = nullptr,
                             std::unique_ptr<ValueRef<int>>&& int_ref2 = nullptr,
                             std::unique_ptr<ValueRef<int>>&& int_ref3 = nullptr,
                             std::unique_ptr<ValueRef<std::string>>&& string_ref1 = nullptr,
                             std::unique_ptr<ValueRef<std::string>>&& string_ref2 = nullptr,
                             bool return_immediate_value = false) :
        Variable<T>(ReferenceType::NON_OBJECT_REFERENCE, std::forward<S>(variable_name), ContainerType::NONE,
                    return_immediate_value ? ValueToReturn::Immediate : ValueToReturn::Initial),
        m_int_ref1(std::move(int_ref1)),
        m_int_ref2(std::move(int_ref2)),
        m_int_ref3(std::move(int_ref3)),
        m_string_ref1(std::move(string_ref1)),
        m_string_ref2(std::move(string_ref2))
    { InitInvariants(); } // TODO: remove InitInvariants, determine in Variable<T> constructor call

    explicit ComplexVariable(const ComplexVariable<T>& rhs) :
        Variable<T>(rhs),
        m_int_ref1(CloneUnique(rhs.m_int_ref1)),
        m_int_ref2(CloneUnique(rhs.m_int_ref2)),
        m_int_ref3(CloneUnique(rhs.m_int_ref3)),
        m_string_ref1(CloneUnique(rhs.m_string_ref1)),
        m_string_ref2(CloneUnique(rhs.m_string_ref2))
    {}

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
  * from the ctor \a value_ref parameter's FromType value, static_cast to ToType. */
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

/** An arithmetic operation node ValueRef class. Unary or binary operations such
  * as addition, mutiplication, negation, exponentiation, rounding,
  * value substitution, value comparisons, or random value selection or
  * random number generation are performed on the child(ren) of this node, and
  * the result is returned. */
template <typename T>
struct FO_COMMON_API Operation final : public ValueRef<T>
{
    /** N-ary operation ctor. */
    Operation(OpType op_type, std::unique_ptr<ValueRef<T>>&& operand1,
              std::unique_ptr<ValueRef<T>>&& operand2 = nullptr,
              std::unique_ptr<ValueRef<T>>&& operand3 = nullptr);

    /* N-ary operation ctor. */
    Operation(OpType op_type, std::vector<std::unique_ptr<ValueRef<T>>>&& operands);

    explicit Operation(const Operation<T>& rhs);

    [[nodiscard]] bool        operator==(const ValueRef<T>& rhs) const override;
    [[nodiscard]] T           Eval() const override
    { return this->m_constant_expr ? m_cached_const_value : Eval(IApp::GetApp()->GetContext()); };
    [[nodiscard]] T           Eval(const ScriptingContext& context) const override;
    [[nodiscard]] std::string Description() const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] OpType      GetOpType() const noexcept { return this->m_op_type; }

    [[nodiscard]] const auto* LHS() const { return m_operands.empty() ? nullptr : m_operands.front().get(); } // 1st operand (or nullptr if none exists)
    [[nodiscard]] const auto* RHS() const { return m_operands.size() < 2 ? nullptr : m_operands[1].get(); } // 2nd operand (or nullptr if no 2nd operand exists)
    [[nodiscard]] const std::vector<const ValueRef<T>*> Operands() const; // all operands

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override
    { return std::make_unique<Operation<T>>(*this); }

    void SetTopLevelContent(const std::string& content_name) override;

private:
    Operation(Operation<T>&& rhs) = delete;
    Operation& operator=(const Operation<T>& rhs) = delete;
    Operation& operator=(Operation<T>&& rhs) = delete;

    [[nodiscard]] T EvalConstantExpr() const;

    [[nodiscard]] static bool IsSimpleIncrement(OpType op_type, const std::vector<std::unique_ptr<ValueRef<T>>>& operands) {
        if (op_type == OpType::RANDOM_UNIFORM || op_type == OpType::RANDOM_PICK || op_type == OpType::NOOP)
            return false;

        // determine if this is a simple incrment operation, meaning it is a calculation
        // that depends only on:
        // 1) the effect target value (ie. a meter value or some other property that is
        //    being modified by an effect)
        // 2) a single target-invariant value (ie. a constant, something that depends only
        //    on the source object or a target-independent complex value ref)
        if (operands.size() != 2)
            return false;
        const auto& lhs{operands[0]};
        const auto& rhs{operands[1]};

        // LHS must be just the immediate value of what's being incremented
        // RHS must be the same value for all targets
        return lhs && rhs && lhs->GetReferenceType() == ReferenceType::EFFECT_TARGET_VALUE_REFERENCE && rhs->TargetInvariant();
    }

    const std::vector<std::unique_ptr<ValueRef<T>>> m_operands;
    const T                                         m_cached_const_value = T();
};

/* Convert between names and MeterType. Names are scripting token, like Population
 * and not the MeterType string representations like METER_POPULATION */
[[nodiscard]] FO_COMMON_API MeterType        NameToMeter(std::string_view name) noexcept;
[[nodiscard]] FO_COMMON_API std::string_view MeterToName(MeterType meter) noexcept;

[[nodiscard]] FO_COMMON_API std::string_view PlanetTypeToString(PlanetType type) noexcept;
[[nodiscard]] FO_COMMON_API std::string_view PlanetEnvironmentToString(PlanetEnvironment env) noexcept;
[[nodiscard]] FO_COMMON_API std::string      ReconstructName(std::string_view property_name, ContainerType container_type,
                                                             ReferenceType ref_type, bool return_immediate_value = false);

[[nodiscard]] FO_COMMON_API std::string FormatedDescriptionPropertyNames(
    ReferenceType ref_type, std::string_view property_name, ContainerType container_type,
    bool return_immediate_value = false);

[[nodiscard]] FO_COMMON_API std::string ComplexVariableDescription(
    std::string_view property_name,
    const ValueRef<int>* int_ref1,
    const ValueRef<int>* int_ref2,
    const ValueRef<int>* int_ref3,
    const ValueRef<std::string>* string_ref1,
    const ValueRef<std::string>* string_ref2);

[[nodiscard]] FO_COMMON_API std::string ComplexVariableDump(
    std::string_view property_name,
    const ValueRef<int>* int_ref1,
    const ValueRef<int>* int_ref2,
    const ValueRef<int>* int_ref3,
    const ValueRef<std::string>* string_ref1,
    const ValueRef<std::string>* string_ref2);

[[nodiscard]] FO_COMMON_API std::string StatisticDescription(
    StatisticType stat_type, std::string_view value_desc, std::string_view condition_desc);

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
template <>
FO_COMMON_API std::string Constant<int>::Description() const;

template <>
FO_COMMON_API std::string Constant<double>::Description() const;

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

template <typename T>
constexpr uint32_t Constant<T>::GetCheckSum() const noexcept {
    uint32_t retval{0};
    CheckSums::CheckSumCombine(retval, "ValueRef::Constant");
    CheckSums::CheckSumCombine(retval, m_value);
#if defined(__cpp_lib_is_constant_evaluated) && (!defined(__clang_major__) || (__clang_major__ >= 14))
    if (!std::is_constant_evaluated())
        [type_id_name{typeid(*this).name()}, this, retval]() {
        TraceLogger() << "GetCheckSum(Constant<T>): " << type_id_name
            << " value: " << Description() << " retval: " << retval;
    }();
#endif
    return retval;
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <typename T>
std::string Variable<T>::Description() const
{
    return FormatedDescriptionPropertyNames(this->m_ref_type, m_property_name,
                                            this->m_container_type, this->m_return_immediate_value);
}

template <typename T>
std::string Variable<T>::Dump(uint8_t ntabs) const
{ return ReconstructName(m_property_name, this->m_container_type, this->m_ref_type, this->m_return_immediate_value); }

template <typename T>
CONSTEXPR_STRING uint32_t Variable<T>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Variable");
    CheckSums::CheckSumCombine(retval, m_property_name);
    CheckSums::CheckSumCombine(retval, this->m_ref_type);
    CheckSums::CheckSumCombine(retval, this->m_container_type);
    CheckSums::CheckSumCombine(retval, this->m_return_immediate_value);
    //if (!std::is_constant_evaluated())
    //    TraceLogger() << "GetCheckSum(Variable<T>): " << typeid(*this).name() << " retval: " << retval;
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
    Variable<T>((!sampling_condition || sampling_condition->RootCandidateInvariant()) &&
                    (!value_ref || value_ref->RootCandidateInvariant()),
                true,
                (!sampling_condition || sampling_condition->TargetInvariant()) &&
                    (!value_ref || value_ref->TargetInvariant()),
                (!sampling_condition || sampling_condition->SourceInvariant()) &&
                    (!value_ref || value_ref->SourceInvariant()),
                stat_type),
    m_sampling_condition(std::move(sampling_condition)),
    m_value_ref(std::move(value_ref))
{
    // don't need to check if sampling condition is LocalCandidateInvariant, as
    // all conditions aren't, but that refers to their own local candidate.
    // no condition is explicitly dependent on the parent context's local candidate.
    // also don't need to check if sub-value-ref is local candidate invariant,
    // as it is applied to the subcondition matches, not the local candidate of
    // any containing condition
}

template <typename T, typename V>
bool Statistic<T, V>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Statistic<T, V>& rhs_ = static_cast<const Statistic<T, V>&>(rhs);

    if (this->m_stat_type != rhs_.m_stat_type)
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
        { return ref->Eval(ScriptingContext(context, ScriptingContext::LocalCandidate{}, obj)); });
    }

    return retval;
}

template <typename T, typename V>
std::string Statistic<T, V>::Description() const
{
    if (m_value_ref)
        return StatisticDescription(this->m_stat_type, m_value_ref->Description(),
                                    m_sampling_condition ? m_sampling_condition->Description() : "");

    auto temp = Variable<T>::Description();
    if (!temp.empty())
        return StatisticDescription(this->m_stat_type, temp, m_sampling_condition ? m_sampling_condition->Description() : "");

    return StatisticDescription(this->m_stat_type, "", m_sampling_condition ? m_sampling_condition->Description() : "");
}

template <typename T, typename V>
std::string Statistic<T, V>::Dump(uint8_t ntabs) const
{
    std::string retval = "Statistic ";

    switch (this->m_stat_type) {
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

namespace {
    template <typename T, typename V>
    concept decays_to = std::is_same_v<V, std::decay_t<T>>;

    template <typename C>
    using decayed_value_t = std::decay_t<decltype(*std::declval<C>().begin())>;

    template <typename T>
    constexpr T UniqueCount(auto&& opv)
    {
        // how many unique values appear
        constexpr auto sort_and_count = [](auto&& opv) {
            std::sort(opv.begin(), opv.end());
            auto unique_it = std::unique(opv.begin(), opv.end());
            return static_cast<T>(std::distance(opv.begin(), unique_it));
        };

        using opv_t = decltype(opv);
        using opv_decay_t = std::decay_t<opv_t>;
        using opv_val_t = std::decay_t<decltype(*opv.begin())>;

        if constexpr (requires { opv[0] = opv_val_t{}; })
            return sort_and_count(std::forward<decltype(opv)>(opv));
        else if constexpr (requires { std::declval<opv_decay_t>()[0] = opv_val_t{}; })
            return sort_and_count(opv_decay_t(std::forward<opv_t>(opv)));
        else
            return sort_and_count(std::vector<opv_val_t>(opv.begin(), opv.end()));
    };

    enum class MinMax : uint8_t { Min, Max, Spread, Mode };
    static constexpr auto second_less = [](const auto& p1, const auto& p2)
        noexcept(noexcept(p1.second < p2.second))
        { return p1.second < p2.second; };

    template <typename T, MinMax min_or_max = MinMax::Min, typename C, typename V = decayed_value_t<C>>
    constexpr T HistoMinMax(C&& opv)
    {
        // number of times the most common value appears
        std::vector<std::pair<V, unsigned int>> observed_values;
        observed_values.reserve(opv.size());
        const auto count = [&observed_values](auto&& v) {
            for (auto& [ov, c] : observed_values) {
                if (ov == v) {
                    ++c;
                    return;
                }
            }
            observed_values.emplace_back(std::forward<decltype(v)>(v), 1u);
        };
        for (const auto& entry : opv)
            count(entry);

        if constexpr (min_or_max == MinMax::Min) {
            // number of times least common element appears
            auto min = std::min_element(observed_values.begin(), observed_values.end(), second_less);
            return T(min->second);

        } else if constexpr (min_or_max == MinMax::Max) {
            // number of times most common element appears
            auto max = std::max_element(observed_values.begin(), observed_values.end(), second_less);
            return T(max->second);

        } else if constexpr (min_or_max == MinMax::Spread) {
            // positive difference between the number of times the most and least common values appear
            auto [min, max] = std::minmax_element(observed_values.begin(), observed_values.end(), second_less);
            return T(max->second - min->second);

        } else if constexpr (min_or_max == MinMax::Mode) {
            // value of most common element
            auto max = std::max_element(observed_values.begin(), observed_values.end(), second_less);
            return T(max->first);
        }
    }

    constexpr double constexprsqrt(double val2) {
        if (val2 >= 0 && val2 < std::numeric_limits<double>::infinity()) {
            auto recurse = [val2](double guess) { return 0.5*(guess + val2/guess); };

            double guess = val2, old_guess = 0;
            while (guess != old_guess)
                old_guess = std::exchange(guess, recurse(guess));
            return guess;
        } else {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    template <typename T>
    constexpr T CXRMS(const auto& c)
    {
        using V_t = decayed_value_t<decltype(c)>;
        V_t sum{0};
        for (const auto& v : c)
            sum += (v*v);
        return static_cast<T>(constexprsqrt(static_cast<double>(sum) / c.size()));
    }

    template <typename T>
    T RTRMS(const auto& c)
    {
        using V_t = decayed_value_t<decltype(c)>;
    #if (defined(__clang_major__)) || (defined(__GNUC__) && (__GNUC__ < 11))
        V_t sum{0};
        for (const auto& v : c)
            sum += (v*v);
    #else
        V_t sum = std::transform_reduce(c.begin(), c.end(), V_t{0}, std::plus{},
                                        [](const auto& a) noexcept { return a*a; });
    #endif
        const auto sz = std::max<std::size_t>(c.size(), 1u);
        return static_cast<T>(std::sqrt(sum/sz));
    }

    template <typename T>
    constexpr T RMS(const auto& c)
    { return std::is_constant_evaluated() ? CXRMS<T>(c) : RTRMS<T>(c); }

    constexpr auto Square(const auto vmm) noexcept { return vmm*vmm; };

    constexpr double CXMeanSqDiff(const auto& c) {
        if (c.empty())
            return 0.0;

        // find sample mean
        double sum{0.0};
        for (const auto& v : c)
            sum += static_cast<double>(v);

        const double MEAN = sum / c.size();

        // find average of squared deviations from sample mean
        double sum_sqdiff = 0.0;
        for (const auto& v : c)
            sum_sqdiff += Square(static_cast<double>(v) - MEAN);

        return sum_sqdiff / (c.size() - 1);
    }

    template <typename T>
    constexpr T CXSTD(const auto& c)
    {
        if (c.size() < 2)
            return T{0};
        if constexpr (std::is_floating_point_v<T>)
            return static_cast<T>(constexprsqrt(CXMeanSqDiff(c)));
        else
            return static_cast<T>(constexprsqrt(CXMeanSqDiff(c)) + 0.5); // + 0.5 to round rather then floor when truncating
    }

    template <typename T>
    T RTSTD(const auto& c)
    {
        if (c.size() < 2)
            return T{0};

#if (defined(__clang_major__)) || (defined(__GNUC__) && (__GNUC__ < 11))
        const double mean_sqdiff = CXMeanSqDiff(c);
#else
        const double sum = std::transform_reduce(c.begin(), c.end(), 0.0, std::plus{},
                                                 [](const auto& a) noexcept { return static_cast<double>(a); });
        const double MEAN = sum / c.size();
        const double sum_sqdiff = std::transform_reduce(c.begin(), c.end(), 0.0, std::plus{},
                                                        [MEAN](const auto& a) noexcept
                                                        { return Square(static_cast<double>(a - MEAN)); });
        const double mean_sqdiff = sum_sqdiff / static_cast<double>(c.size() - 1.0);
#endif
        if constexpr (std::is_floating_point_v<T>)
            return static_cast<T>(std::sqrt(mean_sqdiff));
        else
            return static_cast<T>(std::sqrt(mean_sqdiff) + 0.5); // + 0.5 to round rather then floor when truncating
    }

    template <typename T>
    constexpr T STD(const auto& c)
    { return std::is_constant_evaluated() ? CXSTD<T>(c) : RTSTD<T>(c); }
}

template<typename T, typename C, typename V = decayed_value_t<C>>
    requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<V> &&
              requires(C c) { c.size(); {*c.begin() } -> decays_to<V>; } )
constexpr T ReduceData(StatisticType stat_type, C&& object_property_values)
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
            return UniqueCount<T>(std::forward<C>(object_property_values));
            break;
        }

        case StatisticType::HISTO_MAX: {
            return HistoMinMax<T, MinMax::Max>(std::forward<C>(object_property_values));
            break;
        }

        case StatisticType::HISTO_MIN: {
            return HistoMinMax<T, MinMax::Min>(std::forward<C>(object_property_values));
            break;
        }

        case StatisticType::HISTO_SPREAD: {
            return HistoMinMax<T, MinMax::Spread>(std::forward<C>(object_property_values));
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
            return RMS<T>(object_property_values);
            break;
        }

        case StatisticType::MODE: {
            return HistoMinMax<T, MinMax::Mode>(std::forward<C>(object_property_values));
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
            return STD<T>(object_property_values);
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

template <typename T, typename C, typename V = decayed_value_t<C>>
    requires (std::is_enum_v<T> && std::is_same_v<T, V> &&
              requires(C c) { c.size(); { *c.begin() } -> decays_to<V>; })
constexpr T ReduceData(StatisticType stat_type, C&& object_property_values)
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
            return HistoMinMax<T, MinMax::Mode>(std::forward<C>(object_property_values));
            break;
        }

        default:
            throw std::runtime_error("ReduceData evaluated with an unknown or invalid StatisticType.");
            break;
    }
}

template <typename T, typename C, typename V = decayed_value_t<C>>
    requires (std::is_arithmetic_v<T> && !std::is_arithmetic_v<V>  &&
              requires(C c) { c.size(); { *c.begin() } -> decays_to<V>; })
constexpr T ReduceData(StatisticType stat_type, C&& object_property_values)
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
            return UniqueCount<T>(std::forward<C>(object_property_values));
            break;
        }

        case StatisticType::HISTO_MAX: {
            return HistoMinMax<T, MinMax::Max>(std::forward<C>(object_property_values));
            break;
        }

        case StatisticType::HISTO_MIN: {
            return HistoMinMax<T, MinMax::Min>(std::forward<C>(object_property_values));
            break;
        }

        case StatisticType::HISTO_SPREAD: {
            return HistoMinMax<T, MinMax::Spread>(std::forward<C>(object_property_values));
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
    if (this->m_stat_type == StatisticType::IF)
        return (m_sampling_condition && m_sampling_condition->EvalAny(context)) ? T{1} : T{0};

    const auto condition_matches = m_sampling_condition ?
        m_sampling_condition->Eval(context) : Condition::ObjectSet{};

    if (this->m_stat_type == StatisticType::COUNT)
        return static_cast<T>(condition_matches.size());

    // evaluate property for each condition-matched object
    return ReduceData<T>(this->m_stat_type, GetObjectPropertyValues(context, condition_matches));
}

template <typename T, typename V>
uint32_t Statistic<T, V>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Statistic");
    CheckSums::CheckSumCombine(retval, this->m_stat_type);
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
    if (!Variable<T>::operator==(static_cast<const Variable<T>&>(rhs)))
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
    } else if (*m_int_ref1 != *(rhs_.m_int_ref1)) {
        return false;
    }

    if (m_int_ref2 == rhs_.m_int_ref2) {
        // check next member
    } else if (!m_int_ref2 || !rhs_.m_int_ref2) {
        return false;
    } else if (*m_int_ref2 != *(rhs_.m_int_ref2)) {
        return false;
    }

    if (m_int_ref3 == rhs_.m_int_ref3) {
        // check next member
    } else if (!m_int_ref3 || !rhs_.m_int_ref3) {
        return false;
    } else if (*m_int_ref3 != *(rhs_.m_int_ref3)) {
        return false;
    }

    if (m_string_ref1 == rhs_.m_string_ref1) {
        // check next member
    } else if (!m_string_ref1 || !rhs_.m_string_ref1) {
        return false;
    } else if (*m_string_ref1 != *(rhs_.m_string_ref1)) {
        return false;
    }

    if (m_string_ref2 == rhs_.m_string_ref2) {
        // check next member
    } else if (!m_string_ref2 || !rhs_.m_string_ref2) {
        return false;
    } else if (*m_string_ref2 != *(rhs_.m_string_ref2)) {
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
        [ref{value_ref.get()}]() -> std::string {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->PropertyName();
            else
                return {};
        }(),
        [ref{value_ref.get()}]() -> ContainerType {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->GetContainerType();
            else
                return ContainerType::NONE;
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
    } else if constexpr (requires { std::to_string(value); }) {
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
        [ref{value_ref.get()}]() -> std::string {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->PropertyName();
            else
                return {};
        }(),
        [ref{value_ref.get()}]() -> ContainerType {
            if (auto var_ref = dynamic_cast<Variable<FromType>*>(ref))
                return var_ref->GetContainerType();
            else
                return ContainerType::NONE;
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
                        std::unique_ptr<ValueRef<T>>&& operand2,
                        std::unique_ptr<ValueRef<T>>&& operand3) :
    Operation(op_type, [&operand1, &operand2, &operand3]() {
                std::remove_const_t<decltype(m_operands)> retval{};
                retval.reserve((operand1 ? 1u : 0u) + (operand2 ? 1u : 0u) + (operand3 ? 1u : 0u));
                if (operand1)
                    retval.push_back(std::move(operand1));
                if (operand2)
                    retval.push_back(std::move(operand2));
                if (operand3)
                    retval.push_back(std::move(operand3));
                return retval;
              }())
{}

template <typename T>
Operation<T>::Operation(const Operation<T>& rhs) :
    ValueRef<T>::ValueRef(rhs),
    m_operands(CloneUnique(rhs.m_operands)),
    m_cached_const_value(rhs.m_cached_const_value)
{}

template <typename T>
bool Operation<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Operation<T>& rhs_ = static_cast<const Operation<T>&>(rhs);

    if (this->m_op_type != rhs_.m_op_type)
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
const std::vector<const ValueRef<T>*> Operation<T>::Operands() const
{
    std::vector<const ValueRef<T>*> retval;
    retval.reserve(m_operands.size());
    std::transform(m_operands.begin(), m_operands.end(), std::back_inserter(retval),
                   [](const auto& xx) noexcept { return xx.get(); });
    return retval;
}

namespace {
    auto NoOpLogAndReturn(auto&& val) { // TODO: handle vector<string> ?
        DebugLogger() << "NoOp<" << typeid(val).name() << ">: " << val;
        return val;
    }

    constexpr auto abs(auto val) {
#if defined(__cpp_lib_constexpr_cmath)
        return std::abs(val);
#else
        return (val >= 0) ? val : -val;
#endif
    }

    template <typename T>
    constexpr auto sign(T val) noexcept(noexcept(T()) && noexcept(T{1}))
    { return (val > T()) ? T{1} : (val < T()) ? T{-1} : T(); }

    template <typename T>
    constexpr T pow(T val) {
#if defined(__cpp_lib_constexpr_cmath)
        return static_cast<T>(std::exp(val));
#else
        if (std::is_constant_evaluated())
            return static_cast<T>(CheckSums::pow(val));
        else
            return static_cast<T>(std::exp(val));
#endif
    }

    template <typename T>
    constexpr auto pow(T base, T exp) {
        // std::pow not constexpr until C++26, so not bothering with it here
        return static_cast<T>(std::is_constant_evaluated() ? CheckSums::pow(base, exp) : std::pow(base, exp));
    }

    template <typename T>
    constexpr T log(T val) {
#if defined(__cpp_lib_constexpr_cmath)
        return static_cast<T>(std::log(val));
#else
        return static_cast<T>(std::is_constant_evaluated() ? CheckSums::log(val) : std::log(val));
#endif
    }

    template <typename T> constexpr T round(T val);

    constexpr auto log(auto base, auto val) {
        if (base <= 0)
            throw std::runtime_error("can't take log base <= 0");
        if (base == 1)
            throw std::runtime_error("can't take log base = 1");
        if (val <= 0)
            throw std::runtime_error("can't take log of <= 0");

        double log_ratio = log(static_cast<double>(val)) / log(static_cast<double>(base));

        if constexpr (std::is_integral_v<std::decay_t<decltype(val)>>)
            return static_cast<decltype(val)>(round(log_ratio));
        else
            return static_cast<decltype(val)>(log_ratio);
    }

    template <typename T>
    constexpr T sin(T val) {
#if defined(__cpp_lib_constexpr_cmath)
        return static_cast<T>(std::sin(val));
#else
        if (std::is_constant_evaluated())
            throw std::runtime_error("constexpr sin not implemented");
        else
            return static_cast<T>(std::sin(val));
#endif
    }

    template <typename T>
    constexpr T cos(T val) {
#if defined(__cpp_lib_constexpr_cmath)
        return static_cast<T>(std::cos(val));
#else
        if (std::is_constant_evaluated())
            throw std::runtime_error("constexpr cos not implemented");
        else
            return static_cast<T>(std::cos(val));
#endif
    }

    template <typename T>
    constexpr T round(T val) {
#if defined(__cpp_lib_constexpr_cmath)
        return static_cast<T>(std::round(val));
#else
        if constexpr (std::is_integral_v<std::decay_t<T>>) {
            return val;
        } else {
            const auto trunc_val_plus_half = static_cast<std::decay_t<T>>(static_cast<int64_t>(val + 0.5));
            return (val >= 0) ? trunc_val_plus_half : (trunc_val_plus_half - 1);
        }
#endif
    }

    template <typename T>
    constexpr T ceil(T val) {
#if defined(__cpp_lib_constexpr_cmath)
        return static_cast<T>(std::ceil(val));
#else
        if constexpr (std::is_integral_v<std::decay_t<T>>) {
            return val;
        } else {
            const auto trunc_val = static_cast<std::decay_t<T>>(static_cast<int64_t>(val));
            return (trunc_val == val) ? trunc_val : (val >= 0) ? (trunc_val + 1) : trunc_val;
        }
#endif
    }

    template <typename T>
    constexpr T floor(T val) {
#if defined(__cpp_lib_constexpr_cmath)
        return static_cast<T>(std::floor(val));
#else
        if constexpr (std::is_integral_v<std::decay_t<T>>) {
            return val;
        } else {
            const auto trunc_val = static_cast<std::decay_t<T>>(static_cast<int64_t>(val));
            return (trunc_val == val) ? trunc_val : (val >= 0) ? trunc_val : trunc_val - 1;
        }
#endif
    }

    constexpr bool ValCompareOp(const auto& lhs, OpType op_type, const auto& rhs) noexcept {
        switch (op_type) {
        case OpType::COMPARE_EQUAL:                 return lhs == rhs;  break;
        case OpType::COMPARE_GREATER_THAN:          return lhs > rhs;   break;
        case OpType::COMPARE_GREATER_THAN_OR_EQUAL: return lhs >= rhs;  break;
        case OpType::COMPARE_LESS_THAN:             return lhs < rhs;   break;
        case OpType::COMPARE_LESS_THAN_OR_EQUAL:    return lhs <= rhs;  break;
        case OpType::COMPARE_NOT_EQUAL:             return lhs != rhs;  break;
        default:                                      return false;       break;
        }
    }

    inline constexpr auto vr_rand_int = [](int low, int high)
    { return std::is_constant_evaluated() ? std::max(low, std::min(high, 42)) : RandInt(low, high); };

    inline constexpr auto vr_rand_double = [](double low, double high)
    { return std::is_constant_evaluated() ? std::max(low, std::min(high, 42.6)) : RandDouble(low, high); };

    auto DoFormat(const std::string& lhs, const std::string& rhs) { // TODO: remove this?
        // insert string into other string in place of %1% or similar placeholder
        boost::format formatter = FlexibleFormat(lhs);
        formatter % rhs;
        return formatter.str();
    }
}

template <typename T>
    requires std::is_enum_v<std::decay_t<T>> || std::is_arithmetic_v<std::decay_t<T>> ||
             std::is_same_v<std::decay_t<T>, std::string>
constexpr auto OperateData(OpType op_type, T&& val)
{
    using decayed_t = std::decay_t<T>;
    constexpr bool is_arith = std::is_arithmetic_v<decayed_t>;

    if constexpr (is_arith) {
        switch (op_type) {
        case OpType::NOOP:          return std::is_constant_evaluated() ?
                                        std::forward<T>(val) : NoOpLogAndReturn(std::forward<T>(val));  break;
        case OpType::NEGATE:        return -val;                                                        break;
        case OpType::EXPONENTIATE:  return pow(val);                                                    break;
        case OpType::ABS:           return abs(val);                                                    break;
        case OpType::LOGARITHM:     return log(val);                                                    break;
        case OpType::SINE:          return sin(val);                                                    break;
        case OpType::COSINE:        return cos(val);                                                    break;
        case OpType::ROUND_NEAREST: return round(val);                                                  break;
        case OpType::ROUND_UP:      return ceil(val);                                                   break;
        case OpType::ROUND_DOWN:    return floor(val);                                                  break;
        case OpType::SIGN:          return sign(val);                                                   break;
        default: break;    // not implemented
        }
    } else if (OpType::NOOP == op_type) {
        return std::is_constant_evaluated() ? std::forward<T>(val) : NoOpLogAndReturn(std::forward<T>(val));
    }

    throw std::runtime_error("OperateData evaluated with an unknown or invalid OpType for one operand.");
}

template <typename T>
constexpr auto OperateData(OpType op_type, T&& lhs, T&& rhs)
{
    using decayed_t = std::decay_t<T>;
    constexpr bool is_enum = std::is_enum_v<decayed_t>;
    constexpr bool is_arith = std::is_arithmetic_v<decayed_t>;
    constexpr bool is_string = std::is_same_v<decayed_t, std::string>;

    switch (op_type) {
    case OpType::PLUS: {
        if constexpr (requires { lhs + rhs; }) {
            return lhs + rhs;
        } else if constexpr (requires { lhs.insert(lhs.end(), rhs.begin(), rhs.end()); }) {
            lhs.insert(lhs.end(), rhs.begin(), rhs.end());
            return std::forward<T>(lhs);
        } else if constexpr (std::is_const_v<T> && requires(decayed_t t) { t.insert(lhs.end(), rhs.begin(), rhs.end()); }) {
            decayed_t retval{std::forward<T>(lhs)};
            retval.insert(lhs.end(), rhs.begin(), rhs.end());
            return retval;
        }
        break;
    }

    case OpType::MINUS: {
        if constexpr (is_arith)
            return lhs - rhs;
        break;
    }

    case OpType::TIMES: {
        if constexpr (is_arith) {
            return lhs * rhs;

        } else if constexpr (requires { lhs.empty(); }) {
            // useful for writing a "Statistic If" expression with strings.
            // An empty string indicates no matches, and non-empty string indicates matches, which is treated
            // like a multiplicative identity operation, so just returns rhs.
            return lhs.empty() ? std::forward<T>(lhs) : std::forward<T>(rhs);

        } else {
            // useful for writing a "Statistic If" expression with arbitrary types.
            // If returns T{0} for nothing or T{1} for something matching the
            // sampling condition. This can be checked here by returning T() if
            // the LHS operand is T() and just returning RHS() otherwise.
            return (lhs == T()) ? std::forward<T>(lhs) : std::forward<T>(rhs);
        }
        break;
    }

    case OpType::DIVIDE: {
        if constexpr (is_arith) {
            if (rhs == decayed_t{})
                return decayed_t{0};
            return lhs / rhs;
        }
        break;
    }

    case OpType::REMAINDER: {
        if constexpr (is_arith) {
            if (rhs == decayed_t{0})
                return decayed_t{0};
            const auto divisor = (rhs >= 0) ? rhs : -rhs;
            const auto quotient = floor(lhs / divisor);
            return lhs - (quotient*divisor);
        }
        break;
    }

    case OpType::EXPONENTIATE: {
        if constexpr (is_arith)
            return pow(lhs, rhs);
        break;
    }

    case OpType::LOGARITHM: {
        if constexpr (is_arith)
            return log(lhs, rhs);
        break;
    }

    case OpType::MAXIMUM: return std::max(std::forward<T>(lhs), std::forward<T>(rhs)); break;
    case OpType::MINIMUM: return std::min(std::forward<T>(lhs), std::forward<T>(rhs)); break;

    case OpType::SUBSTITUTION: {
        if constexpr (is_string) {
            if (lhs.empty())
                return std::forward<T>(lhs);
            else if (!std::is_constant_evaluated())
                return DoFormat(std::forward<T>(lhs), std::forward<T>(rhs));
        }
    }

    case OpType::COMPARE_EQUAL:
    case OpType::COMPARE_GREATER_THAN:
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL:
    case OpType::COMPARE_LESS_THAN:
    case OpType::COMPARE_LESS_THAN_OR_EQUAL:
    case OpType::COMPARE_NOT_EQUAL: {
        const bool result = ValCompareOp(std::forward<T>(lhs), op_type, std::forward<T>(rhs));
        if constexpr (is_arith || is_enum)
            return result ? decayed_t{1} : decayed_t();
        else if constexpr (is_string)
            return result ? std::string{"true"} : std::string{};
        break;
    }

    default: break;
    }

    throw std::runtime_error("OperateData evaluated with an unknown or invalid OpType for two operands.");
}

template <typename T>
constexpr auto OperateData(OpType op_type, T&& lhs, T&& rhs, auto rand_int)
    requires requires { {rand_int(0, 1)} -> std::same_as<int>; }
{
    using decayed_t = std::decay_t<T>;

    switch (op_type) {
    case OpType::RANDOM_UNIFORM: {
        if constexpr (std::is_same_v<decayed_t, int>)
            return rand_int(std::min(lhs, rhs), std::max(rhs, lhs));
        else if constexpr (std::is_enum_v<decayed_t>)
            return static_cast<decayed_t>(rand_int(static_cast<int>(std::min(lhs, rhs)), static_cast<int>(std::max(rhs, lhs))));
        break;
    }

    case OpType::RANDOM_PICK: return (rand_int(0, 1) == 0) ? std::forward<T>(lhs) : std::forward<T>(rhs); break;

    default: break;
    }
    return OperateData(op_type, std::forward<T>(lhs), std::forward<T>(rhs));
}

template <typename T>
constexpr auto OperateData(OpType op_type, T&& lhs, T&& rhs, auto rand_int, auto rand_double)
    requires requires { {rand_int(0, 1)} -> std::same_as<int>; {rand_double(0.1, 1.0)} -> std::same_as<double>; }
{
    using decayed_t = std::decay_t<T>;

    if constexpr (std::is_same_v<decayed_t, double>) {
        if (op_type == OpType::RANDOM_UNIFORM)
            return rand_double(std::min(lhs, rhs), std::max(rhs, lhs));
    }
    return OperateData(op_type, std::forward<T>(lhs), std::forward<T>(rhs), rand_int);
}

template <typename D>
    requires (requires(D d) { d.begin(); d.end(); d.front(); } && !std::is_same_v<std::decay_t<D>, std::string>)
constexpr auto OperateData(OpType op_type, D&& data)
{
    using T = std::decay_t<decltype(data.front())>;
    constexpr bool is_enum = std::is_enum_v<T>;
    constexpr bool is_arith = std::is_arithmetic_v<T>;
    constexpr bool is_string = std::is_same_v<T, std::string>;
    static_assert(is_arith || is_string || is_enum);

    if (data.empty())
        return T();

    switch (op_type) {
    case OpType::PLUS: {
        if constexpr (is_arith || is_string) {
#if defined(__cpp_lib_constexpr_numeric)
            return std::accumulate(data.begin(), data.end(), T());
#else
            T accum{};
            for (const auto& d : data)
                accum += d;
            return accum;
#endif
        }
        break;
    }
    case OpType::TIMES: {
        if constexpr (is_arith) {
#if defined(__cpp_lib_constexpr_numeric)
            return std::accumulate(data.begin(), data.end(), T(1), std::multiplies{});
#else
            T accum{};
            for (const auto& d : data)
                accum *= d;
            return accum;
#endif
        }
        break;
    }
    case OpType::MINIMUM: return *std::min_element(data.begin(), data.end()); break;
    case OpType::MAXIMUM: return *std::max_element(data.begin(), data.end()); break;
    default: break;
    }

    throw std::runtime_error("OperateData evaluated with an unknown or invalid OpType for data array.");
}

template <typename D>
    requires (requires(D d) { d.size(); d[std::size_t{0}]; d.begin(); d.end(); d.front(); } &&
              !std::is_same_v<std::decay_t<D>, std::string>)
constexpr auto OperateData(OpType op_type, D&& data, auto rand_int)
{
    using T = std::decay_t<decltype(data.front())>;
    constexpr bool is_enum = std::is_enum_v<T>;
    constexpr bool is_arith = std::is_arithmetic_v<T>;
    constexpr bool is_string = std::is_same_v<T, std::string>;
    static_assert(is_arith || is_string || is_enum);

    switch (op_type) {
    case OpType::RANDOM_PICK: {
        const auto data_sz = data.size();
        const int pick_max = std::max(0, static_cast<int>(data_sz) - 1);
        const std::size_t pick_idx = static_cast<std::size_t>(rand_int(0, pick_max));
        const std::size_t idx = std::max<std::size_t>(0, std::min<std::size_t>(pick_max, pick_idx));
        return std::forward<D>(data)[idx];
        break;
    }
    default: return OperateData(op_type, std::forward<D>(data)); break;
    }
}

template <typename T>
auto OperateValueRefs(OpType op_type, const std::unique_ptr<ValueRef<T>>& lhs,
                      const std::unique_ptr<ValueRef<T>>& rhs, const ScriptingContext& context)
{ return OperateData(op_type, lhs ? lhs->Eval(context) : T(), rhs ? rhs->Eval(context) : T()); }

template <typename T>
auto OperateValueRefs(OpType op_type, const ValueRef<T>* lhs, const ValueRef<T>* rhs, const ScriptingContext& context)
{ return OperateData(op_type, lhs ? lhs->Eval(context) : T(), rhs ? rhs->Eval(context) : T()); }

template <typename T>
auto OperateValueRefs(OpType op_type, const ValueRef<T>* lhs, const ValueRef<T>* rhs,
                      const ScriptingContext& context, auto rand_int, auto rand_double)
{
    switch (op_type) {
    case OpType::RANDOM_PICK: {
        const bool picker = static_cast<bool>(rand_int(0, 1));
        const auto& ref = picker ? rhs : lhs;
        return ref ? ref->Eval(context) : T();
        break;
    }

    default:
        return OperateData(op_type, lhs ? lhs->Eval(context) : T(),
                           rhs ? rhs->Eval(context) : T(), rand_int, rand_double);
        break;
    }
}

template <typename T>
auto OperateValueRefs(OpType op_type, const std::unique_ptr<ValueRef<T>>& lhs,
                      const std::unique_ptr<ValueRef<T>>& rhs,
                      const ScriptingContext& context, auto rand_int, auto rand_double)
{ return OperateValueRefs(op_type, lhs.get(), rhs.get(), context, rand_int, rand_double); }

namespace {
    // evaulates ref[i], unless ref[i] is null or \a refs is too small, in which case
    // returns a default value that is 1 or "true" if \a default_val is true, or otherwise 0 or ""
    constexpr auto EvalIdxConstantRef(const auto& refs, std::size_t i, bool default_val = false) {
        using T = std::decay_t<decltype(refs[i]->Eval())>;
        if (refs.size() > i) {
            const auto& ref{refs[i]};
            if (!ref)
                return T();
            if (!ref->ConstantExpr())
                throw std::runtime_error("can't evaluate non-constant Operation without a context");
            return ref->Eval();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::string{default_val ? "true" : ""};
        } else {
            return default_val ? T{1} : T();
        }
    };

    constexpr auto EvalIdxRef(const ScriptingContext& context, const auto& refs,
                              std::size_t i, bool default_val = false)
    {
        using T = std::decay_t<decltype(refs[i]->Eval())>;
        if (refs.size() > i) {
            const auto& ref{refs[i]};
            return ref ? ref->Eval(context) : T();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::string{default_val ? "true" : ""};
        } else {
            return default_val ? T{1} : T();
        }
    };
}

template <typename D>
    requires requires(D d) { d.begin(); d.end(); d.empty(); d.front()->Eval(); d[0]->ConstantExpr(); }
CONSTEXPR_VEC auto OperateConstantValueRefs(OpType op_type, D&& refs)
{
    using T = std::decay_t<decltype(refs.front()->Eval())>;
    if (refs.empty())
        return T();

    switch (op_type) {
    case OpType::COMPARE_EQUAL:
    case OpType::COMPARE_GREATER_THAN:
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL:
    case OpType::COMPARE_LESS_THAN:
    case OpType::COMPARE_LESS_THAN_OR_EQUAL:
    case OpType::COMPARE_NOT_EQUAL: {
        return ValCompareOp(EvalIdxConstantRef(refs, 0), op_type, EvalIdxConstantRef(refs, 1)) ?
            EvalIdxConstantRef(refs, 2, true) : EvalIdxConstantRef(refs, 3, false);
        break;
    }
    default: {
        std::vector<T> vals;
        vals.reserve(refs.size());
#if defined(_MSC_VER) && (_MSC_VER < 1939)
        // internal compiler error workaround
        for (auto& ref : refs)
            vals.push_back((ref && ref->ConstantExpr()) ? ref->Eval() : T());
#else
        std::transform(refs.begin(), refs.end(), std::back_inserter(vals),
                       [](const auto& ref) { return (ref && ref->ConstantExpr()) ? ref->Eval() : T(); });
#endif
        return OperateData(op_type, std::move(vals));
    }
    }
}

template <typename T>
auto OperateValueRefs(OpType op_type, const std::vector<std::unique_ptr<ValueRef<T>>>& refs,
                      const ScriptingContext& context, auto rand_int, auto rand_double)
{
    if (refs.empty())
        return T();
    const auto refs_sz = refs.size();

    switch (op_type) {
    case OpType::RANDOM_PICK: {
        const int pick_max = std::max(0, static_cast<int>(refs_sz) - 1);
        const std::size_t pick_idx = static_cast<std::size_t>(rand_int(0, pick_max));
        const std::size_t idx = std::max<std::size_t>(0, std::min<std::size_t>(pick_max, pick_idx));
        return EvalIdxRef(context, refs, idx);
        break;
    }
    case OpType::COMPARE_EQUAL:
    case OpType::COMPARE_GREATER_THAN:
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL:
    case OpType::COMPARE_LESS_THAN:
    case OpType::COMPARE_LESS_THAN_OR_EQUAL:
    case OpType::COMPARE_NOT_EQUAL: {
        return ValCompareOp(EvalIdxRef(context, refs, 0), op_type, EvalIdxRef(context, refs, 1)) ?
            EvalIdxRef(context, refs, 2, true) : EvalIdxRef(context, refs, 3, false);
        break;
    }
    default: {
        std::vector<T> vals;
        vals.reserve(refs_sz);
#if defined(_MSC_VER) && (_MSC_VER < 1939)
        // internal compiler error workaround
        for (auto& ref : refs)
            vals.push_back(ref ? ref->Eval(context) : T());
#else
        std::transform(refs.begin(), refs.end(), std::back_inserter(vals),
                       [&context](const auto& ref) { return ref ? ref->Eval(context) : T(); });
#endif
        return OperateData(op_type, std::move(vals), rand_int);
    }
    }
}

template <typename T>
T Operation<T>::EvalConstantExpr() const
{
    if (!this->m_constant_expr)
        throw std::runtime_error("can't evaluate non-constant Operation without a context");
    else if (m_operands.size() == 1)
        return OperateData<T>(this->m_op_type, m_operands.front() ? m_operands.front()->Eval() : T());
    else if (m_operands.size() == 2)
        return OperateData<T>(this->m_op_type, m_operands.front() ? m_operands.front()->Eval() : T(),
                              m_operands.back() ? m_operands.back()->Eval() : T());
    else
        return OperateConstantValueRefs(this->m_op_type, m_operands);
}

template <typename T>
T Operation<T>::Eval(const ScriptingContext& context) const
{
    if (this->m_constant_expr) {
        return m_cached_const_value;
    } else if (this->m_simple_increment) { // assumes simple increment test checks that refs aren't null
        return OperateData<T>(this->m_op_type, LHS()->Eval(context), RHS()->Eval(context));
    } else if (m_operands.size() == 1) {
        return OperateData<T>(this->m_op_type, LHS() ? LHS()->Eval(context) : T());
    } else if (m_operands.size() == 2) {
        return OperateValueRefs<T>(this->m_op_type, LHS(), RHS(), context, vr_rand_int, vr_rand_double);
    } else {
        return OperateValueRefs<T>(this->m_op_type, m_operands, context, vr_rand_int, vr_rand_double);
    }
}

template <typename T>
uint32_t Operation<T>::GetCheckSum() const
{
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Operation");
    CheckSums::CheckSumCombine(retval, this->m_op_type);
    CheckSums::CheckSumCombine(retval, m_operands);
    // derived member values should not be part of checksums
    // e.g. the invariants and m_cached_const_value
    TraceLogger() << "GetCheckSum(Operation<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <typename T>
Operation<T>::Operation(OpType op_type, std::vector<std::unique_ptr<ValueRef<T>>>&& operands) :
    ValueRef<T>(op_type != OpType::RANDOM_UNIFORM && op_type != OpType::RANDOM_PICK && op_type != OpType::NOOP &&
                std::all_of(operands.begin(), operands.end(),
                            [](const auto& operand) noexcept { return operand && operand->ConstantExpr(); }),
                op_type != OpType::RANDOM_UNIFORM && op_type != OpType::RANDOM_PICK && op_type != OpType::NOOP &&
                std::all_of(operands.begin(), operands.end(),
                            [](const auto& operand) noexcept { return operand && operand->RootCandidateInvariant(); }),
                op_type != OpType::RANDOM_UNIFORM && op_type != OpType::RANDOM_PICK && op_type != OpType::NOOP &&
                std::all_of(operands.begin(), operands.end(),
                            [](const auto& operand) noexcept { return operand && operand->LocalCandidateInvariant(); }),
                op_type != OpType::RANDOM_UNIFORM && op_type != OpType::RANDOM_PICK && op_type != OpType::NOOP &&
                std::all_of(operands.begin(), operands.end(),
                            [](const auto& operand) noexcept { return operand && operand->TargetInvariant(); }),
                op_type != OpType::RANDOM_UNIFORM && op_type != OpType::RANDOM_PICK && op_type != OpType::NOOP &&
                std::all_of(operands.begin(), operands.end(),
                            [](const auto& operand) noexcept { return operand && operand->SourceInvariant(); }),
                IsSimpleIncrement(op_type, operands),
                op_type
               ),
    m_operands(std::move(operands)),
    m_cached_const_value(this->m_constant_expr ? this->EvalConstantExpr() : T())
{
    if (std::any_of(m_operands.begin(), m_operands.end(), [](const auto& op) noexcept -> bool { return !op; }))
        throw std::invalid_argument("Operation passed null operand");
    if (this->m_op_type == OpType::INVALID_OP_TYPE)
        throw std::invalid_argument("Operation has invalid operation type");
}

template <typename T>
std::string Operation<T>::Description() const
{
    if (this->m_op_type == OpType::NEGATE) {
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

    if (this->m_op_type == OpType::NOOP)
        return LHS()->Description();
    if (this->m_op_type == OpType::ABS)
        return "abs(" + LHS()->Description() + ")";
    if (this->m_op_type == OpType::LOGARITHM)
        return "log(" + LHS()->Description() + ")";
    if (this->m_op_type == OpType::SINE)
        return "sin(" + LHS()->Description() + ")";
    if (this->m_op_type == OpType::COSINE)
        return "cos(" + LHS()->Description() + ")";

    if (this->m_op_type == OpType::MINIMUM) {
        std::string retval = "min(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }
    if (this->m_op_type == OpType::MAXIMUM) {
        std::string retval = "max(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }

    if (this->m_op_type == OpType::RANDOM_UNIFORM)
        return "RandomNumber(" + LHS()->Description() + ", " + RHS()->Description() + ")";

    if (this->m_op_type == OpType::RANDOM_PICK) {
        std::string retval = "OneOf(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }

    if (this->m_op_type == OpType::ROUND_NEAREST)
        return "round(" + LHS()->Description() + ")";
    if (this->m_op_type == OpType::ROUND_UP)
        return "ceil(" + LHS()->Description() + ")";
    if (this->m_op_type == OpType::ROUND_DOWN)
        return "floor(" + LHS()->Description() + ")";
    if (this->m_op_type == OpType::SIGN)
        return "sign(" + LHS()->Description() + ")";

    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (auto lhs = dynamic_cast<const Operation<T>*>(LHS())) {
        OpType op_type = lhs->GetOpType();
        if (
            (this->m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES   || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS   || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((this->m_op_type == OpType::TIMES || this->m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS          || op_type == OpType::MINUS)) ||
             op_type == OpType::NEGATE)
           )
            parenthesize_lhs = true;
    }
    if (auto rhs = dynamic_cast<const Operation<T>*>(RHS())) {
        OpType op_type = rhs->GetOpType();
        if (
            (this->m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES   || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS   || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((this->m_op_type == OpType::TIMES || this->m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS          || op_type == OpType::MINUS)) ||
             op_type == OpType::NEGATE)
           )
            parenthesize_rhs = true;
    }

    std::string retval = parenthesize_lhs ? ('(' + LHS()->Description() + ')') : LHS()->Description();

    switch (this->m_op_type) {
    case OpType::PLUS:         retval += " + "; break;
    case OpType::MINUS:        retval += " - "; break;
    case OpType::TIMES:        retval += " * "; break;
    case OpType::DIVIDE:       retval += " / "; break;
    case OpType::REMAINDER:    retval += " % "; break;
    case OpType::EXPONENTIATE: retval += " ^ "; break;
    default:                   retval += " ? "; break;
    }

    retval += parenthesize_rhs ? ('(' + RHS()->Description() + ')') : RHS()->Description();

    return retval;
}

template <typename T>
std::string Operation<T>::Dump(uint8_t ntabs) const
{
    if (this->m_op_type == OpType::NEGATE)
        return "-(" + LHS()->Dump(ntabs) + ")";
    if (this->m_op_type == OpType::NOOP)
        return "(" + LHS()->Dump() + ")";
    if (this->m_op_type == OpType::ABS)
        return "abs(" + LHS()->Dump(ntabs) + ")";
    if (this->m_op_type == OpType::LOGARITHM)
        return "log(" + LHS()->Dump(ntabs) + ")";
    if (this->m_op_type == OpType::SINE)
        return "sin(" + LHS()->Dump(ntabs) + ")";
    if (this->m_op_type == OpType::COSINE)
        return "cos(" + LHS()->Dump(ntabs) + ")";

    if (this->m_op_type == OpType::MINIMUM) {
        std::string retval = "min(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }
    if (this->m_op_type == OpType::MAXIMUM) {
        std::string retval = "max(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }

    if (this->m_op_type == OpType::RANDOM_UNIFORM)
        return "RandomNumber(" + LHS()->Dump(ntabs) + ", " + LHS()->Dump(ntabs) + ")";

    if (this->m_op_type == OpType::RANDOM_PICK) {
        std::string retval = "OneOf(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }

    if (this->m_op_type == OpType::ROUND_NEAREST)
        return "round(" + LHS()->Dump(ntabs) + ")";
    if (this->m_op_type == OpType::ROUND_UP)
        return "ceil(" + LHS()->Dump(ntabs) + ")";
    if (this->m_op_type == OpType::ROUND_DOWN)
        return "floor(" + LHS()->Dump(ntabs) + ")";
    if (this->m_op_type == OpType::SIGN)
        return "sign(" + LHS()->Dump(ntabs) + ")";

    bool parenthesize_whole = [this]() {
        switch (this->m_op_type) {
        case OpType::COMPARE_EQUAL: [[fallthrough]];
        case OpType::COMPARE_GREATER_THAN: [[fallthrough]];
        case OpType::COMPARE_GREATER_THAN_OR_EQUAL: [[fallthrough]];
        case OpType::COMPARE_LESS_THAN: [[fallthrough]];
        case OpType::COMPARE_LESS_THAN_OR_EQUAL: [[fallthrough]];
        case OpType::COMPARE_NOT_EQUAL: return true; break;
        default: return false;
        }
    }();
    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (auto lhs = dynamic_cast<const Operation<T>*>(LHS())) {
        OpType op_type = lhs->GetOpType();
        if (
            (this->m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES  || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS  || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((this->m_op_type == OpType::TIMES || this->m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS          || op_type == OpType::MINUS)) ||
             op_type == OpType::NEGATE)
           )
            parenthesize_lhs = true;
    }
    if (auto rhs = dynamic_cast<const Operation<T>*>(RHS())) {
        OpType op_type = rhs->GetOpType();
        if (
            (this->m_op_type == OpType::EXPONENTIATE &&
             (op_type == OpType::EXPONENTIATE || op_type == OpType::TIMES   || op_type == OpType::DIVIDE ||
              op_type == OpType::PLUS         || op_type == OpType::MINUS   || op_type == OpType::NEGATE ||
              op_type == OpType::REMAINDER)
            ) ||
            (((this->m_op_type == OpType::TIMES || this->m_op_type == OpType::DIVIDE || op_type == OpType::REMAINDER) &&
              (op_type == OpType::PLUS          || op_type == OpType::MINUS))
             || op_type == OpType::NEGATE)
           )
            parenthesize_rhs = true;
    }

    std::string retval = parenthesize_whole ? "(" : "";
    retval += parenthesize_lhs ? ('(' + LHS()->Dump(ntabs) + ')') : LHS()->Dump(ntabs);

    switch (this->m_op_type) {
    case OpType::PLUS:                          retval += " + "; break;
    case OpType::MINUS:                         retval += " - "; break;
    case OpType::TIMES:                         retval += " * "; break;
    case OpType::DIVIDE:                        retval += " / "; break;
    case OpType::REMAINDER:                     retval += " % "; break;
    case OpType::EXPONENTIATE:                  retval += " ^ "; break;
    case OpType::SUBSTITUTION:                  retval += " % "; break;
    case OpType::COMPARE_EQUAL:                 retval += " == "; break;
    case OpType::COMPARE_GREATER_THAN:          retval += " > "; break;
    case OpType::COMPARE_GREATER_THAN_OR_EQUAL: retval += " >= "; break;
    case OpType::COMPARE_LESS_THAN:             retval += " < "; break;
    case OpType::COMPARE_LESS_THAN_OR_EQUAL:    retval += " <= "; break;
    case OpType::COMPARE_NOT_EQUAL:             retval += " != "; break;
    default: break;
    }

    retval += parenthesize_rhs ? ('(' + RHS()->Dump(ntabs) + ')') : RHS()->Dump(ntabs);

    retval += parenthesize_whole ? ")" : "";

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
