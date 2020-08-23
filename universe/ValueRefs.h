#ifndef _ValueRefs_h_
#define _ValueRefs_h_


#include <iterator>
#include <map>
#include <set>
#include <boost/algorithm/cxx11/all_of.hpp>
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
    void CheckSumCombine(unsigned int& sum, const typename ValueRef::ValueRef<T>& c)
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
    explicit Constant(T value);

    bool operator==(const ValueRef<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;

    bool ConstantExpr() const override
    { return true; }

    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    T Value() const;
    unsigned int GetCheckSum() const override;

private:
    T           m_value;
    std::string m_top_level_content;    // in the special case that T is std::string and m_value is "CurrentContent", return this instead
};

enum ReferenceType : int {
    INVALID_REFERENCE_TYPE = -1,
    NON_OBJECT_REFERENCE,               // ValueRef::Variable is not evalulated on any specific object
    SOURCE_REFERENCE,                   // ValueRef::Variable is evaluated on the source object
    EFFECT_TARGET_REFERENCE,            // ValueRef::Variable is evaluated on the target object of an effect while it is being executed
    EFFECT_TARGET_VALUE_REFERENCE,      // ValueRef::Variable is evaluated on the target object value of an effect while it is being executed
    CONDITION_LOCAL_CANDIDATE_REFERENCE,// ValueRef::Variable is evaluated on an object that is a candidate to be matched by a condition.  In a subcondition, this will reference the local candidate, and not the candidate of an enclosing condition.
    CONDITION_ROOT_CANDIDATE_REFERENCE  // ValueRef::Variable is evaluated on an object that is a candidate to be matched by a condition.  In a subcondition, this will still reference the root candidate, and not the candidate of the local condition.
};

/** The variable value ValueRef class.  The value returned by this node is
  * taken from the gamestate, most often from the Source or Target objects. */
template <typename T>
struct FO_COMMON_API Variable : public ValueRef<T>
{
    template <typename S>
    Variable(ReferenceType ref_type, S&& property_name,
             bool return_immediate_value = false);

    Variable(ReferenceType ref_type, const char* property_name,
             bool return_immediate_value = false);

    explicit Variable(ReferenceType ref_type,
             bool return_immediate_value = false);

    Variable(ReferenceType ref_type,
             boost::optional<std::string>&& container_name,
             std::string&& property_name,
             bool return_immediate_value = false);

    Variable(ReferenceType ref_type, std::vector<std::string>&& property_name,
             bool return_immediate_value = false);

    Variable(ReferenceType ref_type, const std::vector<std::string>& property_name,
             bool return_immediate_value = false);

    bool operator==(const ValueRef<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    ReferenceType GetReferenceType() const;
    const std::vector<std::string>& PropertyName() const;
    bool ReturnImmediateValue() const;
    unsigned int GetCheckSum() const override;

protected:
    void InitInvariants();

    ReferenceType               m_ref_type = INVALID_REFERENCE_TYPE;
    std::vector<std::string>    m_property_name;
    bool                        m_return_immediate_value = false;
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

    bool        operator==(const ValueRef<T>& rhs) const override;
    T           Eval(const ScriptingContext& context) const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void        SetTopLevelContent(const std::string& content_name) override;

    StatisticType GetStatisticType() const
    { return m_stat_type; }

    const Condition::Condition* GetSamplingCondition() const
    { return m_sampling_condition.get(); }

    const ValueRef<V>* GetValueRef() const
    { return m_value_ref.get(); }

    unsigned int GetCheckSum() const override;

protected:
    /** Gets the set of objects in the Universe that match the sampling condition. */
    void GetConditionMatches(const ScriptingContext& context,
                             Condition::ObjectSet& condition_targets,
                             Condition::Condition* condition) const;

    /** Evaluates the property for the specified objects. */
    void GetObjectPropertyValues(const ScriptingContext& context,
                                 const Condition::ObjectSet& objects,
                                 std::map<std::shared_ptr<const UniverseObject>, V>& object_property_values) const;

private:
    StatisticType                         m_stat_type;
    std::unique_ptr<Condition::Condition> m_sampling_condition;
    std::unique_ptr<ValueRef<V>>          m_value_ref;
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

    bool operator==(const ValueRef<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    const ValueRef<int>* IntRef1() const;
    const ValueRef<int>* IntRef2() const;
    const ValueRef<int>* IntRef3() const;
    const ValueRef<std::string>* StringRef1() const;
    const ValueRef<std::string>* StringRef2() const;
    unsigned int GetCheckSum() const override;

protected:
    void InitInvariants();

    std::unique_ptr<ValueRef<int>> m_int_ref1;
    std::unique_ptr<ValueRef<int>> m_int_ref2;
    std::unique_ptr<ValueRef<int>> m_int_ref3;
    std::unique_ptr<ValueRef<std::string>> m_string_ref1;
    std::unique_ptr<ValueRef<std::string>> m_string_ref2;
};

/** The variable static_cast class.  The value returned by this node is taken
  * from the ctor \a value_ref parameter's FromType value, static_cast to
  * ToType. */
template <typename FromType, typename ToType>
struct FO_COMMON_API StaticCast final : public Variable<ToType>
{
    template <typename T>
    StaticCast(T&& value_ref,
               typename std::enable_if<std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type* = nullptr);

    template <typename T>
    StaticCast(T&& value_ref,
               typename std::enable_if<
               std::is_convertible<T, std::unique_ptr<ValueRef<FromType>>>::value
               && !std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type* = nullptr);

    bool operator==(const ValueRef<ToType>& rhs) const override;
    ToType Eval(const ScriptingContext& context) const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRef<FromType>* GetValueRef() const
    { return m_value_ref.get(); }

    unsigned int GetCheckSum() const override;

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

    bool operator==(const ValueRef<std::string>& rhs) const override;
    std::string Eval(const ScriptingContext& context) const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRef<FromType>* GetValueRef() const
    { return m_value_ref; }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef<FromType>> m_value_ref;
};

/** Looks up a string ValueRef or vector of string ValueRefs, and returns
  * and returns the UserString equivalent(s). */
template <typename FromType>
struct FO_COMMON_API UserStringLookup final : public Variable<std::string> {
    explicit UserStringLookup(std::unique_ptr<ValueRef<FromType>>&& value_ref);

    bool operator==(const ValueRef<std::string>& rhs) const override;
    std::string Eval(const ScriptingContext& context) const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRef<FromType>* GetValueRef() const
    { return m_value_ref; }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef<FromType>> m_value_ref;
};

/** Returns the in-game name of the object / empire / etc. with a specified id. */
struct FO_COMMON_API NameLookup final : public Variable<std::string> {
    enum LookupType : int {
        INVALID_LOOKUP = -1,
        OBJECT_NAME,
        EMPIRE_NAME,
        SHIP_DESIGN_NAME
    };

    NameLookup(std::unique_ptr<ValueRef<int>>&& value_ref, LookupType lookup_type);

    bool operator==(const ValueRef<std::string>& rhs) const override;
    std::string Eval(const ScriptingContext& context) const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRef<int>* GetValueRef() const
    { return m_value_ref.get(); }

    LookupType GetLookupType() const
    { return m_lookup_type; }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef<int>> m_value_ref;
    LookupType m_lookup_type;
};

enum OpType : int {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
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
    SIGN
};

/** An arithmetic operation node ValueRef class. Unary or binary operations such
  * as addition, mutiplication, negation, exponentiation, rounding,
  * value substitution, value comparisons, or random value selection or
  * random number generation are performed on the child(ren) of this node, and
  * the result is returned. */
template <typename T>
struct Operation final : public ValueRef<T>
{
    /** Binary operation ctor. */
    Operation(OpType op_type, std::unique_ptr<ValueRef<T>>&& operand1,
              std::unique_ptr<ValueRef<T>>&& operand2);

    /** Unary operation ctor. */
    Operation(OpType op_type, std::unique_ptr<ValueRef<T>>&& operand);

    /* N-ary operation ctor. */
    Operation(OpType op_type, std::vector<std::unique_ptr<ValueRef<T>>>&& operands);

    bool operator==(const ValueRef<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;
    bool SimpleIncrement() const override { return m_simple_increment; }
    bool ConstantExpr() const override { return m_constant_expr; }
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    OpType GetOpType() const;

    /** 1st operand (or 0 if none exists). */
    const ValueRef<T>* LHS() const;

    /** 2nd operand (or 0 if only one exists) */
    const ValueRef<T>* RHS() const;

    /** all operands */
    const std::vector<ValueRef<T>*> Operands() const;

    unsigned int GetCheckSum() const override;

private:
    void    InitConstInvariants();
    void    CacheConstValue();
    T       EvalImpl(const ScriptingContext& context) const;

    OpType                                      m_op_type = TIMES;
    std::vector<std::unique_ptr<ValueRef<T>>>   m_operands;
    bool                                        m_constant_expr = false;
    bool                                        m_simple_increment = false;
    T                                           m_cached_const_value = T();
};

template <> double Operation<double>::EvalImpl(const ScriptingContext& context) const;
template <> int Operation<int>::EvalImpl(const ScriptingContext& context) const;
template <> std::string Operation<std::string>::EvalImpl(const ScriptingContext& context) const;

extern template struct Operation<double>;
extern template struct Operation<int>;
extern template struct Operation<UniverseObjectType>;
extern template struct Operation<PlanetEnvironment>;
extern template struct Operation<PlanetSize>;
extern template struct Operation<PlanetType>;
extern template struct Operation<StarType>;
extern template struct Operation<Visibility>;
extern template struct Operation<std::string>;


FO_COMMON_API MeterType             NameToMeter(const std::string& name);
FO_COMMON_API const std::string&    MeterToName(MeterType meter);
FO_COMMON_API std::string           ReconstructName(const std::vector<std::string>& property_name,
                                                    ReferenceType ref_type,
                                                    bool return_immediate_value = false);

FO_COMMON_API std::string FormatedDescriptionPropertyNames(
    ReferenceType ref_type, const std::vector<std::string>& property_names,
    bool return_immediate_value = false);

FO_COMMON_API std::string ComplexVariableDescription(
    const std::vector<std::string>& property_names,
    const ValueRef<int>* int_ref1,
    const ValueRef<int>* int_ref2,
    const ValueRef<int>* int_ref3,
    const ValueRef<std::string>* string_ref1,
    const ValueRef<std::string>* string_ref2);

FO_COMMON_API std::string ComplexVariableDump(
    const std::vector<std::string>& property_names,
    const ValueRef<int>* int_ref1,
    const ValueRef<int>* int_ref2,
    const ValueRef<int>* int_ref3,
    const ValueRef<std::string>* string_ref1,
    const ValueRef<std::string>* string_ref2);

FO_COMMON_API std::string StatisticDescription(StatisticType stat_type,
                                               const std::string& value_desc,
                                               const std::string& condition_desc);

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
Constant<T>::Constant(T value) :
    ValueRef<T>(),
    m_value(std::move(value))
{
    this->m_root_candidate_invariant = true;
    this->m_local_candidate_invariant = true;
    this->m_target_invariant = true;
    this->m_source_invariant = true;
}

template <typename T>
bool Constant<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Constant<T>& rhs_ = static_cast<const Constant<T>&>(rhs);

    return m_value == rhs_.m_value && m_top_level_content == rhs_.m_top_level_content;
}

template <typename T>
T Constant<T>::Value() const
{ return m_value; }

template <typename T>
T Constant<T>::Eval(const ScriptingContext& context) const
{ return m_value; }

template <typename T>
std::string Constant<T>::Description() const
{ return UserString(boost::lexical_cast<std::string>(m_value)); }

template <typename T>
void Constant<T>::SetTopLevelContent(const std::string& content_name)
{ m_top_level_content = content_name; }

template <typename T>
unsigned int Constant<T>::GetCheckSum() const
{
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Constant");
    CheckSums::CheckSumCombine(retval, m_value);
    TraceLogger() << "GetCheckSum(Constant<T>): " << typeid(*this).name() << " value: " << m_value << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API std::string Constant<int>::Description() const;

template <>
FO_COMMON_API std::string Constant<double>::Description() const;

template <>
FO_COMMON_API std::string Constant<std::string>::Description() const;

template <>
FO_COMMON_API std::string Constant<PlanetSize>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<PlanetType>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<PlanetEnvironment>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<UniverseObjectType>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<StarType>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<Visibility>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<double>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<int>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<std::string>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string Constant<std::string>::Eval(const ScriptingContext& context) const;


///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <typename T>
Variable<T>::Variable(ReferenceType ref_type, std::vector<std::string>&& property_name,
                      bool return_immediate_value) :
    ValueRef<T>(),
    m_ref_type(ref_type),
    m_property_name(std::move(property_name)),
    m_return_immediate_value(return_immediate_value)
{ InitInvariants(); }

template <typename T>
Variable<T>::Variable(ReferenceType ref_type, const std::vector<std::string>& property_name,
                      bool return_immediate_value) :
    ValueRef<T>(),
    m_ref_type(ref_type),
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
    ValueRef<T>(),
    m_ref_type(ref_type),
    m_return_immediate_value(return_immediate_value)
{
    m_property_name.emplace_back(std::move(property_name));
    InitInvariants();
}

template <typename T>
Variable<T>::Variable(ReferenceType ref_type,
                      boost::optional<std::string>&& container_name,
                      std::string&& property_name,
                      bool return_immediate_value) :
    ValueRef<T>(),
    m_ref_type(ref_type),
    m_return_immediate_value(return_immediate_value)
{
    if (container_name)
        m_property_name.emplace_back(std::move(*container_name));
    m_property_name.emplace_back(std::move(property_name));

    InitInvariants();
}

template <typename T>
void Variable<T>::InitInvariants()
{
    this->m_root_candidate_invariant = m_ref_type != CONDITION_ROOT_CANDIDATE_REFERENCE;
    this->m_local_candidate_invariant = m_ref_type != CONDITION_LOCAL_CANDIDATE_REFERENCE;
    this->m_target_invariant = m_ref_type != EFFECT_TARGET_REFERENCE && m_ref_type != EFFECT_TARGET_VALUE_REFERENCE;
    this->m_source_invariant = m_ref_type != SOURCE_REFERENCE;
}

template <typename T>
bool Variable<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Variable<T>& rhs_ = static_cast<const Variable<T>&>(rhs);
    return (m_ref_type == rhs_.m_ref_type) &&
           (m_property_name == rhs_.m_property_name) &&
           (m_return_immediate_value == rhs_.m_return_immediate_value);
}

template <typename T>
ReferenceType Variable<T>::GetReferenceType() const
{ return m_ref_type; }

template <typename T>
const std::vector<std::string>& Variable<T>::PropertyName() const
{ return m_property_name; }

template <typename T>
bool Variable<T>::ReturnImmediateValue() const
{ return m_return_immediate_value; }

template <typename T>
std::string Variable<T>::Description() const
{ return FormatedDescriptionPropertyNames(m_ref_type, m_property_name, m_return_immediate_value); }

template <typename T>
std::string Variable<T>::Dump(unsigned short ntabs) const
{ return ReconstructName(m_property_name, m_ref_type, m_return_immediate_value); }

template <typename T>
unsigned int Variable<T>::GetCheckSum() const
{
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Variable");
    CheckSums::CheckSumCombine(retval, m_property_name);
    CheckSums::CheckSumCombine(retval, m_ref_type);
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
    Variable<T>(NON_OBJECT_REFERENCE),
    m_stat_type(stat_type),
    m_sampling_condition(std::move(sampling_condition)),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = (!m_sampling_condition || m_sampling_condition->RootCandidateInvariant()) &&
                                       (!m_value_ref || m_value_ref->RootCandidateInvariant());

    // don't need to check if sampling condition is LocalCandidateInvariant, as
    // all conditions aren't, but that refers to their own local candidate.  no
    // condition is explicitly dependent on the parent context's local candidate.
    this->m_local_candidate_invariant = (!m_value_ref || m_value_ref->LocalCandidateInvariant());

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
    if (this->m_value_ref != rhs_.m_value_ref)
        return false;

    if (m_sampling_condition == rhs_.m_sampling_condition) {
        // check next member
    } else if (!m_sampling_condition || !rhs_.m_sampling_condition) {
        return false;
    } else {
        if (*m_sampling_condition != *(rhs_.m_sampling_condition))
            return false;
    }

    return true;
}

template <typename T, typename V>
void Statistic<T, V>::GetConditionMatches(const ScriptingContext& context,
                                          Condition::ObjectSet& condition_targets,
                                          Condition::Condition* condition) const
{
    condition_targets.clear();
    if (!condition)
        return;
    condition->Eval(context, condition_targets);
}

template <typename T, typename V>
void Statistic<T, V>::GetObjectPropertyValues(const ScriptingContext& context,
                                              const Condition::ObjectSet& objects,
                                              std::map<std::shared_ptr<const UniverseObject>, V>& object_property_values) const
{
    object_property_values.clear();

    if (m_value_ref) {
        // evaluate ValueRef with each condition match as the LocalCandidate
        // TODO: Can / should this be paralleized?
        for (auto& object : objects)
            object_property_values.emplace(object, m_value_ref->Eval(ScriptingContext(context, object)));
    }
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
std::string Statistic<T, V>::Dump(unsigned short ntabs) const
{
    std::string retval = "Statistic ";

    switch (m_stat_type) {
        case IF:                 retval += "If";                break;
        case COUNT:              retval += "Count";             break;
        case UNIQUE_COUNT:       retval += "CountUnique";       break;
        case HISTO_MAX:          retval += "HistogramMax";      break;
        case HISTO_MIN:          retval += "HistogramMin";      break;
        case HISTO_SPREAD:       retval += "HistogramSpread";   break;
        case SUM:                retval += "Sum";               break;
        case MEAN:               retval += "Mean";              break;
        case RMS:                retval += "RMS";               break;
        case MODE:               retval += "Mode";              break;
        case MAX:                retval += "Max";               break;
        case MIN:                retval += "Min";               break;
        case SPREAD:             retval += "Spread";            break;
        case STDEV:              retval += "StDev";             break;
        case PRODUCT:            retval += "Product";           break;
        default:                 retval += "???";               break;
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

template <
    typename T,
    typename V,
    typename std::enable_if<std::is_arithmetic<T>::value && std::is_arithmetic<V>::value>::type* = nullptr
>
T ReduceData(StatisticType stat_type, std::map<std::shared_ptr<const UniverseObject>, V> object_property_values)
{
    if (object_property_values.empty())
        return T(0);

    // should be able to convert between V and T types, so can do a bunch of
    // numerical statistics or histogram statistics

    switch (stat_type) {
        case IF: {
            // 1 if any objects have property values, else 0 above
            return T(1);
            break;
        }

        case COUNT: {
            // how many objects / values
            return T(object_property_values.size());
            break;
        }

        case UNIQUE_COUNT: {
            // how many unique values appear
            std::set<V> observed_values;
            for (const auto& entry : object_property_values)
                observed_values.emplace(entry.second);

            return T(observed_values.size());
            break;
        }

        case HISTO_MAX: {
            // number of times the most common value appears
            std::map<V, unsigned int> observed_values;
            for (const auto& entry : object_property_values)
                observed_values[entry.second]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p2.second; });

            return T(max->second);
            break;
        }

        case HISTO_MIN: {
            // number of times the least common value appears
            std::map<V, unsigned int> observed_values;
            for (const auto& entry : object_property_values)
                observed_values[entry.second]++;

            auto min = std::min_element(observed_values.begin(), observed_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p2.second; });

            return T(min->second);
            break;
        }

        case HISTO_SPREAD: {
            // positive difference between the number of times the most and least common values appear
            std::map<V, unsigned int> observed_values;
            for (auto& entry : object_property_values)
                observed_values[std::move(entry.second)]++;

            auto minmax = std::minmax_element(observed_values.begin(), observed_values.end(),
                                              [](auto p1, auto p2) { return p1.second < p2.second; });

            return T(minmax.second->second - minmax.first->second);
            break;
        }

        case SUM: {
            V accumulator(0);
            for (const auto& entry : object_property_values)
                accumulator += entry.second;

            return static_cast<T>(accumulator);
            break;
        }

        case MEAN: {
            V accumulator(0);
            for (const auto& entry : object_property_values)
                accumulator += entry.second;

            return static_cast<T>(accumulator) / static_cast<T>(object_property_values.size());
            break;
        }

        case RMS: {
            V accumulator(0);
            for (const auto& entry : object_property_values)
                accumulator += (entry.second * entry.second);

            double tempval = static_cast<double>(accumulator) / object_property_values.size();
            return static_cast<T>(std::sqrt(tempval));
            break;
        }

        case MODE: {
            // value that appears the most often
            std::map<V, unsigned int> observed_values;
            for (auto& entry : object_property_values)
                observed_values[std::move(entry.second)]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p2.second; });

            return T(max->first);
            break;
        }

        case MAX: {
            auto max = std::max_element(object_property_values.begin(), object_property_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p1.second; });

            return static_cast<T>(max->second);
            break;
        }

        case MIN: {
            auto min = std::min_element(object_property_values.begin(), object_property_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p1.second; });

            return static_cast<T>(min->second);
            break;
        }

        case SPREAD: {
            auto minmax = std::minmax_element(object_property_values.begin(), object_property_values.end(),
                                              [](auto p1, auto p2) { return p1.second < p2.second; });

            return static_cast<T>(minmax.second->second - minmax.first->second);
            break;
        }

        case STDEV: {
            if (object_property_values.size() < 2)
                return T(0);

            // find sample mean
            double accumulator = 0.0;
            for (auto entry : object_property_values)
                accumulator += static_cast<double>(entry.second);

            double MEAN = accumulator / object_property_values.size();

            // find average of squared deviations from sample mean
            accumulator = 0;
            for (auto entry : object_property_values)
                accumulator += (static_cast<double>(entry.second) - MEAN) * (static_cast<double>(entry.second) - MEAN);

            double retval = accumulator / static_cast<double>(object_property_values.size() - 1.0);
            return static_cast<T>(std::sqrt(retval));
            break;
        }

        case PRODUCT: {
            V accumulator(1);
            for (const auto& entry : object_property_values)
                accumulator *= entry.second;

            return static_cast<T>(accumulator);
            break;
        }

        default:
            throw std::runtime_error("ReduceData evaluated with an unknown or invalid StatisticType.");
            break;
    }
}

template <
    typename T,
    typename V,
    typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr,
    typename std::enable_if<std::is_same<T, V>::value>::type* = nullptr
>
T ReduceData(StatisticType stat_type, std::map<std::shared_ptr<const UniverseObject>, T> object_property_values)
{
    if (object_property_values.empty())
        return T(0);

    // enum types T and V are the return value type and the property value type
    // so can calculate the most common value and return it

    switch (stat_type) {
        case IF: {
            // 1 if any objects have property values, else 0
            if (object_property_values.empty())
                return T(0);
            return T(1);
            break;
        }

        case MODE: {
            // value that appears the most often
            std::map<T, unsigned int> observed_values;
            for (auto& entry : object_property_values)
                observed_values[std::move(entry.second)]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p2.second; });

            return max->first;
            break;
        }

        default:
            throw std::runtime_error("ReduceData evaluated with an unknown or invalid StatisticType.");
            break;
    }
}

template <
    typename T,
    typename V,
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type* = nullptr,
    typename std::enable_if<!std::is_arithmetic<V>::value, V>::type* = nullptr>
T ReduceData(StatisticType stat_type, std::map<std::shared_ptr<const UniverseObject>, V> object_property_values)
{
    if (object_property_values.empty())
        return T(0);

    // return value type T is a number and the object property value type V is
    // not a numeric type, such as std::string or an enum type, so can calculate
    // various histogram properties that are not enum type valued.

    switch (stat_type) {
        case IF: {
            // 1 if any objects have property values, else 0
            if (object_property_values.empty())
                return T(0);
            return T(1);
            break;
        }

        case COUNT: {
            // how many objects / values
            return T(object_property_values.size());
            break;
        }

        case UNIQUE_COUNT: {
            // how many unique values appear
            std::set<V> observed_values;
            for (const auto& entry : object_property_values)
                observed_values.emplace(entry.second);

            return T(observed_values.size());
            break;
        }

        case HISTO_MAX: {
            // number of times the most common value appears
            std::map<V, unsigned int> observed_values;
            for (const auto& entry : object_property_values)
                observed_values[entry.second]++;

            auto max = std::max_element(observed_values.begin(), observed_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p2.second; });

            return T(max->second);
            break;
        }

        case HISTO_MIN: {
            // number of times the least common value appears
            std::map<V, unsigned int> observed_values;
            for (const auto& entry : object_property_values)
                observed_values[entry.second]++;

            auto min = std::min_element(observed_values.begin(), observed_values.end(),
                                        [](auto p1, auto p2) { return p1.second < p2.second; });

            return T(min->second);
            break;
        }

        case HISTO_SPREAD: {
            // positive difference between the number of times the most and least common values appear
            std::map<V, unsigned int> observed_values;
            for (auto& entry : object_property_values)
                observed_values[std::move(entry.second)]++;

            auto minmax = std::minmax_element(observed_values.begin(), observed_values.end(),
                                              [](auto p1, auto p2) { return p1.second < p2.second; });

            return T(minmax.second->second - minmax.first->second);
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
    Condition::ObjectSet condition_matches;
    GetConditionMatches(context, condition_matches, m_sampling_condition.get());

    // these two statistic types don't depend on the object property values,
    // so can be evaluated without getting those values.
    if (m_stat_type == COUNT)
        return static_cast<T>(condition_matches.size());
    if (m_stat_type == IF)
        return condition_matches.empty() ? T(0) : T(1);

    // evaluate property for each condition-matched object
    std::map<std::shared_ptr<const UniverseObject>, V> object_property_values;
    GetObjectPropertyValues(context, condition_matches, object_property_values);

    return ReduceData<T, V>(m_stat_type, std::move(object_property_values));
}

template <typename T, typename V>
unsigned int Statistic<T, V>::GetCheckSum() const
{
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Statistic");
    CheckSums::CheckSumCombine(retval, m_stat_type);
    CheckSums::CheckSumCombine(retval, m_sampling_condition);
    CheckSums::CheckSumCombine(retval, m_value_ref);
    TraceLogger() << "GetCheckSum(Statisic<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <>
FO_COMMON_API std::string Statistic<std::string, std::string>::Eval(const ScriptingContext& context) const;

template struct Statistic<double, double>;
template struct Statistic<double, std::string>;
template struct Statistic<int, int>;
template struct Statistic<int, std::string>;

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
    Variable<T>(NON_OBJECT_REFERENCE, std::forward<S>(variable_name), return_immediate_value),
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
    Variable<T>(NON_OBJECT_REFERENCE, variable_name, return_immediate_value),
    m_int_ref1(std::move(int_ref1)),
    m_int_ref2(std::move(int_ref2)),
    m_int_ref3(std::move(int_ref3)),
    m_string_ref1(std::move(string_ref1)),
    m_string_ref2(std::move(string_ref2))
{ InitInvariants(); }

template <typename T>
void ComplexVariable<T>::InitInvariants()
{
    std::initializer_list<ValueRefBase*> refs =
        { m_int_ref1.get(), m_int_ref2.get(), m_int_ref3.get(), m_string_ref1.get(), m_string_ref2.get() };
    this->m_root_candidate_invariant = boost::algorithm::all_of(refs, [](const auto& e) { return !e || e->RootCandidateInvariant(); });
    this->m_local_candidate_invariant = boost::algorithm::all_of(refs, [](const auto& e) { return !e || e->LocalCandidateInvariant(); });
    this->m_target_invariant = boost::algorithm::all_of(refs, [](const auto& e) { return !e || e->TargetInvariant(); });
    this->m_source_invariant = boost::algorithm::all_of(refs, [](const auto& e) { return !e || e->SourceInvariant(); });
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
const ValueRef<int>* ComplexVariable<T>::IntRef1() const
{ return m_int_ref1.get(); }

template <typename T>
const ValueRef<int>* ComplexVariable<T>::IntRef2() const
{ return m_int_ref2.get(); }

template <typename T>
const ValueRef<int>* ComplexVariable<T>::IntRef3() const
{ return m_int_ref3.get(); }

template <typename T>
const ValueRef<std::string>* ComplexVariable<T>::StringRef1() const
{ return m_string_ref1.get(); }

template <typename T>
const ValueRef<std::string>* ComplexVariable<T>::StringRef2() const
{ return m_string_ref2.get(); }

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
std::string ComplexVariable<T>::Dump(unsigned short ntabs) const
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
unsigned int ComplexVariable<T>::GetCheckSum() const
{
    unsigned int retval{0};

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
FO_COMMON_API std::string ComplexVariable<std::vector<std::string>>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<Visibility>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<double>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<int>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<std::string>::Dump(unsigned short ntabs) const;


///////////////////////////////////////////////////////////
// StaticCast                                            //
///////////////////////////////////////////////////////////
template <typename FromType, typename ToType>
template <typename T>
StaticCast<FromType, ToType>::StaticCast(
    T&& value_ref,
    typename std::enable_if<std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type*) :
    Variable<ToType>(value_ref->GetReferenceType(), value_ref->PropertyName()),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
}

template <typename FromType, typename ToType>
template <typename T>
StaticCast<FromType, ToType>::StaticCast(
    T&& value_ref,
    typename std::enable_if<
        std::is_convertible<T, std::unique_ptr<ValueRef<FromType>>>::value &&
        !std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type*) :
    Variable<ToType>(NON_OBJECT_REFERENCE),
    m_value_ref(std::move(value_ref))
{
    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
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
std::string StaticCast<FromType, ToType>::Dump(unsigned short ntabs) const
{ return m_value_ref->Dump(ntabs); }

template <typename FromType, typename ToType>
void StaticCast<FromType, ToType>::SetTopLevelContent(const std::string& content_name)
{
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <typename FromType, typename ToType>
unsigned int StaticCast<FromType, ToType>::GetCheckSum() const
{
    unsigned int retval{0};

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
    Variable<std::string>(NON_OBJECT_REFERENCE),
    m_value_ref(std::move(value_ref))
{
    auto raw_ref_ptr = m_value_ref.get();
    // if looking up a the results of ValueRef::Variable::Eval, can copy that
    // ValueRef's internals to expose the reference type and property name from
    // this ValueRef
    if (auto var_ref = dynamic_cast<Variable<FromType>*>(raw_ref_ptr)) {
        this->m_ref_type = var_ref->GetReferenceType();
        this->m_property_name = var_ref->PropertyName();
    }

    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
}

template <typename FromType>
bool StringCast<FromType>::operator==(const ValueRef<std::string>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const StringCast<FromType>& rhs_ =
        static_cast<const StringCast<FromType>&>(rhs);

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
    std::string retval;
    try {
        retval = boost::lexical_cast<std::string>(m_value_ref->Eval(context));
    } catch (...) {
    }
    return retval;
}

template <typename FromType>
unsigned int StringCast<FromType>::GetCheckSum() const
{
    unsigned int retval{0};

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
std::string StringCast<FromType>::Dump(unsigned short ntabs) const
{ return m_value_ref->Dump(ntabs); }

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
    Variable<std::string>(NON_OBJECT_REFERENCE),
    m_value_ref(std::move(value_ref))
{
    auto raw_ref_ptr = m_value_ref.get();
    // if looking up a the results of ValueRef::Variable::Eval, can copy that
    // ValueRef's internals to expose the reference type and property name from
    // this ValueRef
    if (auto var_ref = dynamic_cast<Variable<FromType>*>(raw_ref_ptr)) {
        this->m_ref_type = var_ref->GetReferenceType();
        this->m_property_name = var_ref->PropertyName();
    }

    this->m_root_candidate_invariant = !m_value_ref || m_value_ref->RootCandidateInvariant();
    this->m_local_candidate_invariant = !m_value_ref || m_value_ref->LocalCandidateInvariant();
    this->m_target_invariant = !m_value_ref || m_value_ref->TargetInvariant();
    this->m_source_invariant = !m_value_ref || m_value_ref->SourceInvariant();
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
    std::string ref_val = boost::lexical_cast<std::string>(m_value_ref->Eval(context));
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
{
    return m_value_ref->Description();
}

template <typename FromType>
std::string UserStringLookup<FromType>::Dump(unsigned short ntabs) const
{
    return m_value_ref->Dump(ntabs);
}

template <typename FromType>
void UserStringLookup<FromType>::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <typename FromType>
unsigned int UserStringLookup<FromType>::GetCheckSum() const
{
    unsigned int retval{0};

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
    ValueRef<T>(),
    m_op_type(op_type)
{
    if (operand1)
        m_operands.push_back(std::move(operand1));
    if (operand2)
        m_operands.push_back(std::move(operand2));
    InitConstInvariants();
    CacheConstValue();
}

template <typename T>
Operation<T>::Operation(OpType op_type, std::unique_ptr<ValueRef<T>>&& operand) :
    ValueRef<T>(),
    m_op_type(op_type)
{
    if (operand)
        m_operands.push_back(std::move(operand));
    InitConstInvariants();
    CacheConstValue();
}

template <typename T>
Operation<T>::Operation(OpType op_type, std::vector<std::unique_ptr<ValueRef<T>>>&& operands) :
    m_op_type(op_type),
    m_operands(std::move(operands))
{
    InitConstInvariants();
    CacheConstValue();
}

template <typename T>
void Operation<T>::InitConstInvariants()
{
    if (m_op_type == RANDOM_UNIFORM || m_op_type == RANDOM_PICK) {
        m_constant_expr = false;
        this->m_root_candidate_invariant = false;
        this->m_local_candidate_invariant = false;
        this->m_target_invariant = false;
        this->m_source_invariant = false;
        m_simple_increment = false;
        return;
    }

    m_constant_expr = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->ConstantExpr(); });

    this->m_root_candidate_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->RootCandidateInvariant(); });
    this->m_local_candidate_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->LocalCandidateInvariant(); });
    this->m_target_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->TargetInvariant(); });
    this->m_source_invariant = std::all_of(m_operands.begin(), m_operands.end(),
        [](const auto& operand) { return operand && operand->SourceInvariant(); });


    // determine if this is a simple incrment operation
    if (m_op_type != PLUS && m_op_type != MINUS) {
        m_simple_increment = false;
        return;
    }
    if (m_operands.size() < 2 || !m_operands[0] || !m_operands[1]) {
        m_simple_increment = false;
        return;
    }
    // RHS must be the same value for all targets
    if (!m_operands[1]->TargetInvariant()) {
        m_simple_increment = false;
        return;
    }
    // LHS must be just the immediate value of what's being incremented
    if (const auto lhs = dynamic_cast<const Variable<T>*>(m_operands[0].get()))
        m_simple_increment = (lhs->GetReferenceType() == EFFECT_TARGET_VALUE_REFERENCE);
    else
        m_simple_increment = false;
}

template <typename T>
void Operation<T>::CacheConstValue()
{
    if (!m_constant_expr)
        return;
    m_cached_const_value = this->EvalImpl(ScriptingContext());
}

template <typename T>
bool Operation<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Operation<T>& rhs_ = static_cast<const Operation<T>&>(rhs);

    if (m_operands == rhs_.m_operands)
        return true;

    if (m_operands.size() != rhs_.m_operands.size())
        return false;

    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        if (m_operands[i] != rhs_.m_operands[i])
            return false;
        if (m_operands[i] && *(m_operands[i]) != *(rhs_.m_operands[i]))
            return false;
    }

    // should be redundant...
    if (m_constant_expr != rhs_.m_constant_expr)
        return false;

    return true;
}

template <typename T>
OpType Operation<T>::GetOpType() const
{ return m_op_type; }

template <typename T>
const ValueRef<T>* Operation<T>::LHS() const
{
    if (m_operands.empty())
        return nullptr;
    return m_operands[0].get();
}

template <typename T>
const ValueRef<T>* Operation<T>::RHS() const
{
    if (m_operands.size() < 2)
        return nullptr;
    return m_operands[1].get();
}

template <typename T>
const std::vector<ValueRef<T>*> Operation<T>::Operands() const
{
    std::vector<ValueRef<T>*> retval(m_operands.size());
    std::transform(m_operands.begin(), m_operands.end(), retval.begin(),
                   [](const auto& xx){ return xx.get(); });
    return retval;
}

template <typename T>
T Operation<T>::Eval(const ScriptingContext& context) const
{
    if (m_constant_expr)
        return m_cached_const_value;
    return this->EvalImpl(context);
}

template <typename T>
unsigned int Operation<T>::GetCheckSum() const
{
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::Operation");
    CheckSums::CheckSumCombine(retval, m_op_type);
    CheckSums::CheckSumCombine(retval, m_operands);
    CheckSums::CheckSumCombine(retval, m_constant_expr);
    CheckSums::CheckSumCombine(retval, m_cached_const_value);
    TraceLogger() << "GetCheckSum(Operation<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}


template <typename T>
T Operation<T>::EvalImpl(const ScriptingContext& context) const
{
    switch (m_op_type) {
    case TIMES: {
        // useful for writing a "Statistic If" expression with arbitrary types.
        // If returns T(0) or T(1) for nothing or something matching the
        // sampling condition. This can be checked here by returning T(0) if
        // the LHS operand is T(0) and just returning RHS() otherwise.
        if (!LHS()->Eval(context))
            return T(0);
        return RHS()->Eval(context);
        break;
    }

    case MAXIMUM:
    case MINIMUM: {
        // evaluate all operands, return smallest or biggest
        std::set<T> vals;
        for (auto& vr : m_operands) {
            if (vr)
                vals.emplace(vr->Eval(context));
        }
        if (m_op_type == MINIMUM)
            return vals.empty() ? T(-1) : *vals.begin();
        else
            return vals.empty() ? T(-1) : *vals.rbegin();
        break;
    }

    case RANDOM_PICK: {
        // select one operand, evaluate it, return result
        if (m_operands.empty())
            return T(-1);   // should be INVALID_T of enum types
        auto idx = RandInt(0, m_operands.size() - 1);
        auto& vr = *std::next(m_operands.begin(), idx);
        if (!vr)
            return T(-1);   // should be INVALID_T of enum types
        return vr->Eval(context);
        break;
    }

    case COMPARE_EQUAL:
    case COMPARE_GREATER_THAN:
    case COMPARE_GREATER_THAN_OR_EQUAL:
    case COMPARE_LESS_THAN:
    case COMPARE_LESS_THAN_OR_EQUAL:
    case COMPARE_NOT_EQUAL: {
        const T&& lhs_val = LHS()->Eval(context);
        const T&& rhs_val = RHS()->Eval(context);
        bool test_result = false;
        switch (m_op_type) {
            case COMPARE_EQUAL:                 test_result = lhs_val == rhs_val;   break;
            case COMPARE_GREATER_THAN:          test_result = lhs_val > rhs_val;    break;
            case COMPARE_GREATER_THAN_OR_EQUAL: test_result = lhs_val >= rhs_val;   break;
            case COMPARE_LESS_THAN:             test_result = lhs_val < rhs_val;    break;
            case COMPARE_LESS_THAN_OR_EQUAL:    test_result = lhs_val <= rhs_val;   break;
            case COMPARE_NOT_EQUAL:             test_result = lhs_val != rhs_val;   break;
            default:    break;  // ??? do nothing, default to false
        }
        if (m_operands.size() < 3) {
            return T(1);
        } else if (m_operands.size() < 4) {
            if (test_result)
                return m_operands[2]->Eval(context);
            else
                return T(0);
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
std::string Operation<T>::Description() const
{
    if (m_op_type == NEGATE) {
        if (auto rhs = dynamic_cast<const Operation<T>*>(LHS())) {
            OpType op_type = rhs->GetOpType();
            if (op_type == PLUS     || op_type == MINUS ||
                op_type == TIMES    || op_type == DIVIDE ||
                op_type == NEGATE   || op_type == EXPONENTIATE)
            return "-(" + LHS()->Description() + ")";
        } else {
            return "-" + LHS()->Description();
        }
    }

    if (m_op_type == ABS)
        return "abs(" + LHS()->Description() + ")";
    if (m_op_type == LOGARITHM)
        return "log(" + LHS()->Description() + ")";
    if (m_op_type == SINE)
        return "sin(" + LHS()->Description() + ")";
    if (m_op_type == COSINE)
        return "cos(" + LHS()->Description() + ")";

    if (m_op_type == MINIMUM) {
        std::string retval = "min(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }
    if (m_op_type == MAXIMUM) {
        std::string retval = "max(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == RANDOM_UNIFORM)
        return "RandomNumber(" + LHS()->Description() + ", " + RHS()->Description() + ")";

    if (m_op_type == RANDOM_PICK) {
        std::string retval = "OneOf(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Description();
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == ROUND_NEAREST)
        return "round(" + LHS()->Description() + ")";
    if (m_op_type == ROUND_UP)
        return "ceil(" + LHS()->Description() + ")";
    if (m_op_type == ROUND_DOWN)
        return "floor(" + LHS()->Description() + ")";
    if (m_op_type == SIGN)
        return "sign(" + LHS()->Description() + ")";

    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (auto lhs = dynamic_cast<const Operation<T>*>(LHS())) {
        OpType op_type = lhs->GetOpType();
        if (
            (m_op_type == EXPONENTIATE &&
             (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
              op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
            ) ||
            (((m_op_type == TIMES        || m_op_type == DIVIDE) &&
              (op_type == PLUS           || op_type == MINUS))    || op_type == NEGATE)
           )
            parenthesize_lhs = true;
    }
    if (auto rhs = dynamic_cast<const Operation<T>*>(RHS())) {
        OpType op_type = rhs->GetOpType();
        if (
            (m_op_type == EXPONENTIATE &&
             (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
              op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
            ) ||
            (((m_op_type == TIMES        || m_op_type == DIVIDE) &&
              (op_type == PLUS           || op_type == MINUS))    || op_type == NEGATE)
           )
            parenthesize_rhs = true;
    }

    std::string retval;
    if (parenthesize_lhs)
        retval += '(' + LHS()->Description() + ')';
    else
        retval += LHS()->Description();

    switch (m_op_type) {
    case PLUS:          retval += " + "; break;
    case MINUS:         retval += " - "; break;
    case TIMES:         retval += " * "; break;
    case DIVIDE:        retval += " / "; break;
    case EXPONENTIATE:  retval += " ^ "; break;
    default:            retval += " ? "; break;
    }

    if (parenthesize_rhs)
        retval += '(' + RHS()->Description() + ')';
    else
        retval += RHS()->Description();

    return retval;
}

}


#endif
