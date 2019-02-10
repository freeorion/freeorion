#ifndef _ValueRef_h_
#define _ValueRef_h_

#include "Condition.h"
#include "Universe.h"
#include "../util/Export.h"
#include "../util/i18n.h"
#include "../util/Random.h"
#include "../util/CheckSums.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>

#include <iterator>
#include <map>
#include <set>

namespace CheckSums {
    template <typename T>
    void CheckSumCombine(unsigned int& sum, const typename ValueRef::ValueRefBase<T>& c)
    {
        TraceLogger() << "CheckSumCombine(ValueRef::ValueRefBase<T>): " << typeid(c).name();
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }
}

class UniverseObject;

struct ScriptingContext {
    /** Empty context.  Useful for evaluating ValueRef::Constant that don't
      * depend on their context. */
    ScriptingContext()
    {}

    /** Context with only a source object.  Useful for evaluating effectsgroup
      * scope and activation conditions that have no external candidates or
      * effect target to propagate. */
    explicit ScriptingContext(std::shared_ptr<const UniverseObject> source_) :
        source(source_)
    {}

    /** Context with source and visibility map to use when evalulating Visiblity
      * conditions. Useful in combat resolution when the visibility of objects
      * may be different from the overall universe visibility. */
    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     const Universe::EmpireObjectVisibilityMap& visibility_map) :
        source(source_),
        empire_object_vis_map_override(visibility_map)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_) :
        source(source_),
        effect_target(target_)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const boost::any& current_value_) :
        source(source_),
        effect_target(target_),
        current_value(current_value_)
    {}

    /** For evaluating ValueRef in an Effect::Execute function.  Keeps input
      * context, but specifies the current value. */
    ScriptingContext(const ScriptingContext& parent_context,
                     const boost::any& current_value_) :
        source(parent_context.source),
        effect_target(parent_context.effect_target),
        condition_root_candidate(parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(current_value_),
        empire_object_vis_map_override(parent_context.empire_object_vis_map_override)
    {}

    /** For recusrive evaluation of Conditions.  Keeps source and effect_target
      * from input context, but sets local candidate with input object, and if
      * there is no root candidate in the parent context, then the input object
      * becomes the root candidate. */
    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_) :
        source(                         parent_context.source),
        effect_target(                  parent_context.effect_target),
        condition_root_candidate(       parent_context.condition_root_candidate ?
                                            parent_context.condition_root_candidate :
                                            condition_local_candidate_),    // if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(      condition_local_candidate_),        // new local candidate
        current_value(                  parent_context.current_value),
        empire_object_vis_map_override( parent_context.empire_object_vis_map_override)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const boost::any& current_value_,
                     std::shared_ptr<const UniverseObject> condition_root_candidate_,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_) :
        source(source_),
        condition_root_candidate(condition_root_candidate_),
        condition_local_candidate(condition_local_candidate_),
        current_value(current_value_)
    {}

    std::shared_ptr<const UniverseObject>   source;
    std::shared_ptr<UniverseObject>         effect_target;
    std::shared_ptr<const UniverseObject>   condition_root_candidate;
    std::shared_ptr<const UniverseObject>   condition_local_candidate;
    const boost::any                        current_value;
    Universe::EmpireObjectVisibilityMap     empire_object_vis_map_override;
};

namespace ValueRef {
/** The base class for all ValueRef classes.  This class provides the public
  * interface for a ValueRef expression tree. */
template <class T>
struct FO_COMMON_API ValueRefBase
{
    virtual ~ValueRefBase()
    {}

    virtual bool operator==(const ValueRefBase<T>& rhs) const;

    bool operator!=(const ValueRefBase<T>& rhs) const
    { return !(*this == rhs); }

    /** Evaluates the expression tree with an empty context.  Useful for
      * evaluating expressions that do not depend on context. */
    T Eval() const
    { return Eval(::ScriptingContext()); }

    /** Evaluates the expression tree and return the results; \a context
      * is used to fill in any instances of the "Value" variable or references
      * to objects such as the source, effect target, or condition candidates
      * that exist in the tree. */
    virtual T Eval(const ScriptingContext& context) const = 0;

    virtual bool RootCandidateInvariant() const
    { return false; }

    virtual bool LocalCandidateInvariant() const
    { return false; }

    virtual bool TargetInvariant() const
    { return false; }

    virtual bool SourceInvariant() const
    { return false; }

    virtual bool SimpleIncrement() const
    { return false; }

    virtual bool ConstantExpr() const
    { return false; }

    virtual std::string Description() const = 0;

    /** Returns a text description of this type of special. */
    virtual std::string Dump(unsigned short ntabs = 0) const = 0;

    virtual void SetTopLevelContent(const std::string& content_name)
    {}

    virtual unsigned int GetCheckSum() const
    { return 0; }

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** the constant value leaf ValueRef class. */
template <class T>
struct FO_COMMON_API Constant final : public ValueRefBase<T>
{
    explicit Constant(T value);

    bool operator==(const ValueRefBase<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;

    bool RootCandidateInvariant() const override
    { return true; }

    bool LocalCandidateInvariant() const override
    { return true; }

    bool TargetInvariant() const override
    { return true; }

    bool SourceInvariant() const override
    { return true; }

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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The variable value ValueRef class.  The value returned by this node is
  * taken from the gamestate, most often from the Source or Target objects. */
template <class T>
struct FO_COMMON_API Variable : public ValueRefBase<T>
{
    explicit Variable(ReferenceType ref_type, const std::string& property_name = "",
                      bool return_immediate_value = false);
    Variable(ReferenceType ref_type, const std::vector<std::string>& property_name,
             bool return_immediate_value = false);
    Variable(ReferenceType ref_type,
             const boost::optional<std::string>& container_name,
             const std::string& property_name,
             bool return_immediate_value = false);

    bool operator==(const ValueRefBase<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;
    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    ReferenceType GetReferenceType() const;
    const std::vector<std::string>& PropertyName() const;
    bool ReturnImmediateValue() const;
    unsigned int GetCheckSum() const override;

protected:
    mutable ReferenceType       m_ref_type = INVALID_REFERENCE_TYPE;
    std::vector<std::string>    m_property_name;
    bool                        m_return_immediate_value = false;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The variable statistic class.   The value returned by this node is
  * computed from the general gamestate; the value of the indicated
  * \a property_name is computed for each object that matches
  * \a sampling_condition and the statistic indicated by \a stat_type is
  * calculated from them and returned. */
template <class T>
struct FO_COMMON_API Statistic final : public Variable<T>
{
    Statistic(std::unique_ptr<ValueRefBase<T>>&& value_ref,
              StatisticType stat_type,
              std::unique_ptr<Condition::ConditionBase>&& sampling_condition);

    bool        operator==(const ValueRefBase<T>& rhs) const override;
    T           Eval(const ScriptingContext& context) const override;
    bool        RootCandidateInvariant() const override;
    bool        LocalCandidateInvariant() const override;
    bool        TargetInvariant() const override;
    bool        SourceInvariant() const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void        SetTopLevelContent(const std::string& content_name) override;

    StatisticType GetStatisticType() const
    { return m_stat_type; }

    const Condition::ConditionBase* GetSamplingCondition() const
    { return m_sampling_condition.get(); }

    const ValueRefBase<T>* GetValueRef() const
    { return m_value_ref.get(); }

    unsigned int GetCheckSum() const override;

protected:
    /** Gets the set of objects in the Universe that match the sampling condition. */
    void GetConditionMatches(const ScriptingContext& context,
                             Condition::ObjectSet& condition_targets,
                             Condition::ConditionBase* condition) const;

    /** Evaluates the property for the specified objects. */
    void  GetObjectPropertyValues(const ScriptingContext& context,
                                  const Condition::ObjectSet& objects,
                                  std::map<std::shared_ptr<const UniverseObject>, T>& object_property_values) const;

    /** Computes the statistic from the specified set of property values. */
    T ReduceData(const std::map<std::shared_ptr<const UniverseObject>, T>& object_property_values) const;

private:
    StatisticType                             m_stat_type;
    std::unique_ptr<Condition::ConditionBase> m_sampling_condition;
    std::unique_ptr<ValueRefBase<T>>          m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The complex variable ValueRef class. The value returned by this node
  * is taken from the gamestate. */
template <class T>
struct FO_COMMON_API ComplexVariable final : public Variable<T>
{
    explicit ComplexVariable(const std::string& variable_name,
                             std::unique_ptr<ValueRefBase<int>>&& int_ref1 = nullptr,
                             std::unique_ptr<ValueRefBase<int>>&& int_ref2 = nullptr,
                             std::unique_ptr<ValueRefBase<int>>&& int_ref3 = nullptr,
                             std::unique_ptr<ValueRefBase<std::string>>&& string_ref1 = nullptr,
                             std::unique_ptr<ValueRefBase<std::string>>&& string_ref2 = nullptr);

    bool operator==(const ValueRefBase<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;
    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    const ValueRefBase<int>* IntRef1() const;
    const ValueRefBase<int>* IntRef2() const;
    const ValueRefBase<int>* IntRef3() const;
    const ValueRefBase<std::string>* StringRef1() const;
    const ValueRefBase<std::string>* StringRef2() const;
    unsigned int GetCheckSum() const override;

protected:
    std::unique_ptr<ValueRefBase<int>> m_int_ref1;
    std::unique_ptr<ValueRefBase<int>> m_int_ref2;
    std::unique_ptr<ValueRefBase<int>> m_int_ref3;
    std::unique_ptr<ValueRefBase<std::string>> m_string_ref1;
    std::unique_ptr<ValueRefBase<std::string>> m_string_ref2;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The variable static_cast class.  The value returned by this node is taken
  * from the ctor \a value_ref parameter's FromType value, static_cast to
  * ToType. */
template <class FromType, class ToType>
struct FO_COMMON_API StaticCast final : public Variable<ToType>
{
    template <typename T>
    StaticCast(T&& value_ref,
               typename std::enable_if<std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type* = nullptr);

    template <typename T>
    StaticCast(T&& value_ref,
               typename std::enable_if<
               std::is_convertible<T, std::unique_ptr<ValueRefBase<FromType>>>::value
               && !std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type* = nullptr);

    bool operator==(const ValueRefBase<ToType>& rhs) const override;
    ToType Eval(const ScriptingContext& context) const override;
    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRefBase<FromType>* GetValueRef() const
    { return m_value_ref.get(); }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRefBase<FromType>> m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The variable lexical_cast to string class.  The value returned by this node
  * is taken from the ctor \a value_ref parameter's FromType value,
  * lexical_cast to std::string */
template <class FromType>
struct FO_COMMON_API StringCast final : public Variable<std::string>
{
    StringCast(std::unique_ptr<ValueRefBase<FromType>>&& value_ref);

    bool operator==(const ValueRefBase<std::string>& rhs) const override;
    std::string Eval(const ScriptingContext& context) const override;
    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRefBase<FromType>* GetValueRef() const
    { return m_value_ref; }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRefBase<FromType>> m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Looks up a string ValueRef or vector of string ValueRefs, and returns
  * and returns the UserString equivalent(s). */
template <class FromType>
struct FO_COMMON_API UserStringLookup final : public Variable<std::string> {
    explicit UserStringLookup(std::unique_ptr<ValueRefBase<FromType>>&& value_ref);

    bool operator==(const ValueRefBase<std::string>& rhs) const override;
    std::string Eval(const ScriptingContext& context) const override;
    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRefBase<FromType>* GetValueRef() const
    { return m_value_ref; }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRefBase<FromType>> m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns the in-game name of the object / empire / etc. with a specified id. */
struct FO_COMMON_API NameLookup final : public Variable<std::string> {
    enum LookupType {
        INVALID_LOOKUP = -1,
        OBJECT_NAME,
        EMPIRE_NAME,
        SHIP_DESIGN_NAME
    };

    NameLookup(std::unique_ptr<ValueRefBase<int>>&& value_ref, LookupType lookup_type);

    bool operator==(const ValueRefBase<std::string>& rhs) const override;
    std::string Eval(const ScriptingContext& context) const override;
    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRefBase<int>* GetValueRef() const
    { return m_value_ref.get(); }

    LookupType GetLookupType() const
    { return m_lookup_type; }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRefBase<int>> m_value_ref;
    LookupType m_lookup_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** An arithmetic operation node ValueRef class.  One of addition, subtraction,
  * mutiplication, division, or unary negation is performed on the child(ren)
  * of this node, and the result is returned. */
template <class T>
struct FO_COMMON_API Operation final : public ValueRefBase<T>
{
    /** Binary operation ctor. */
    Operation(OpType op_type, std::unique_ptr<ValueRefBase<T>>&& operand1,
              std::unique_ptr<ValueRefBase<T>>&& operand2);

    /** Unary operation ctor. */
    Operation(OpType op_type, std::unique_ptr<ValueRefBase<T>>&& operand);

    /* N-ary operation ctor. */
    Operation(OpType op_type, std::vector<std::unique_ptr<ValueRefBase<T>>>&& operands);

    bool operator==(const ValueRefBase<T>& rhs) const override;
    T Eval(const ScriptingContext& context) const override;
    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    bool SimpleIncrement() const override;
    bool ConstantExpr() const override { return m_constant_expr; }
    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    OpType GetOpType() const;

    /** 1st operand (or 0 if none exists). */
    const ValueRefBase<T>* LHS() const;

    /** 2nd operand (or 0 if only one exists) */
    const ValueRefBase<T>* RHS() const;

    /** all operands */
    const std::vector<ValueRefBase<T>*> Operands() const;

    unsigned int GetCheckSum() const override;

private:
    void    DetermineIfConstantExpr();
    void    CacheConstValue();
    T       EvalImpl(const ScriptingContext& context) const;

    OpType                          m_op_type;
    std::vector<std::unique_ptr<ValueRefBase<T>>>   m_operands;
    bool                            m_constant_expr = false;
    T                               m_cached_const_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

FO_COMMON_API MeterType     NameToMeter(const std::string& name);
FO_COMMON_API std::string   MeterToName(MeterType meter);
FO_COMMON_API std::string   ReconstructName(const std::vector<std::string>& property_name,
                                            ReferenceType ref_type,
                                            bool return_immediate_value = false);

FO_COMMON_API std::string FormatedDescriptionPropertyNames(
    ReferenceType ref_type, const std::vector<std::string>& property_names,
    bool return_immediate_value = false);

FO_COMMON_API std::string ComplexVariableDescription(
    const std::vector<std::string>& property_names,
    const ValueRef::ValueRefBase<int>* int_ref1,
    const ValueRef::ValueRefBase<int>* int_ref2,
    const ValueRef::ValueRefBase<int>* int_ref3,
    const ValueRef::ValueRefBase<std::string>* string_ref1,
    const ValueRef::ValueRefBase<std::string>* string_ref2);

FO_COMMON_API std::string ComplexVariableDump(
    const std::vector<std::string>& property_names,
    const ValueRef::ValueRefBase<int>* int_ref1,
    const ValueRef::ValueRefBase<int>* int_ref2,
    const ValueRef::ValueRefBase<int>* int_ref3,
    const ValueRef::ValueRefBase<std::string>* string_ref1,
    const ValueRef::ValueRefBase<std::string>* string_ref2);

FO_COMMON_API std::string StatisticDescription(StatisticType stat_type,
                                               const std::string& value_desc,
                                               const std::string& condition_desc);

// Template Implementations
///////////////////////////////////////////////////////////
// ValueRefBase                                          //
///////////////////////////////////////////////////////////
template <class T>
bool ValueRefBase<T>::operator==(const ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    return true;
}

template <class T>
template <class Archive>
void ValueRefBase<T>::serialize(Archive& ar, const unsigned int version)
{}

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
template <class T>
Constant<T>::Constant(T value) :
    m_value(value)
{}

template <class T>
bool Constant<T>::operator==(const ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Constant<T>& rhs_ = static_cast<const Constant<T>&>(rhs);

    return m_value == rhs_.m_value && m_top_level_content == rhs_.m_top_level_content;
}

template <class T>
T Constant<T>::Value() const
{ return m_value; }

template <class T>
T Constant<T>::Eval(const ScriptingContext& context) const
{ return m_value; }

template <class T>
std::string Constant<T>::Description() const
{ return UserString(boost::lexical_cast<std::string>(m_value)); }

template <class T>
void Constant<T>::SetTopLevelContent(const std::string& content_name)
{ m_top_level_content = content_name; }

template <class T>
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

template <class T>
template <class Archive>
void Constant<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value)
        & BOOST_SERIALIZATION_NVP(m_top_level_content);
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <class T>
Variable<T>::Variable(ReferenceType ref_type, const std::vector<std::string>& property_name,
                      bool return_immediate_value) :
    m_ref_type(ref_type),
    m_property_name(property_name.begin(), property_name.end()),
    m_return_immediate_value(return_immediate_value)
{}

template <class T>
Variable<T>::Variable(ReferenceType ref_type, const std::string& property_name,
                      bool return_immediate_value) :
    m_ref_type(ref_type),
    m_property_name(),
    m_return_immediate_value(return_immediate_value)
{
    m_property_name.push_back(property_name);
}

template <class T>
Variable<T>::Variable(ReferenceType ref_type,
                      const boost::optional<std::string>& container_name,
                      const std::string& property_name,
                      bool return_immediate_value) :
    m_ref_type(ref_type),
    m_property_name(),
    m_return_immediate_value(return_immediate_value)
{
    if (container_name)
        m_property_name.push_back(*container_name);

    m_property_name.push_back(property_name);
}

template <class T>
bool Variable<T>::operator==(const ValueRefBase<T>& rhs) const
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

template <class T>
ReferenceType Variable<T>::GetReferenceType() const
{ return m_ref_type; }

template <class T>
const std::vector<std::string>& Variable<T>::PropertyName() const
{ return m_property_name; }

template <class T>
bool Variable<T>::ReturnImmediateValue() const
{ return m_return_immediate_value; }

template <class T>
bool Variable<T>::RootCandidateInvariant() const
{ return m_ref_type != CONDITION_ROOT_CANDIDATE_REFERENCE; }

template <class T>
bool Variable<T>::LocalCandidateInvariant() const
{ return m_ref_type != CONDITION_LOCAL_CANDIDATE_REFERENCE; }

template <class T>
bool Variable<T>::TargetInvariant() const
{ return m_ref_type != EFFECT_TARGET_REFERENCE && m_ref_type != EFFECT_TARGET_VALUE_REFERENCE; }

template <class T>
bool Variable<T>::SourceInvariant() const
{ return m_ref_type != SOURCE_REFERENCE; }

template <class T>
std::string Variable<T>::Description() const
{ return FormatedDescriptionPropertyNames(m_ref_type, m_property_name, m_return_immediate_value); }

template <class T>
std::string Variable<T>::Dump(unsigned short ntabs) const
{ return ReconstructName(m_property_name, m_ref_type, m_return_immediate_value); }

template <class T>
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

template <class T>
template <class Archive>
void Variable<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_ref_type)
        & BOOST_SERIALIZATION_NVP(m_property_name)
        & BOOST_SERIALIZATION_NVP(m_return_immediate_value);
}

///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////
template <class T>
Statistic<T>::Statistic(std::unique_ptr<ValueRefBase<T>>&& value_ref, StatisticType stat_type,
                        std::unique_ptr<Condition::ConditionBase>&& sampling_condition) :
    Variable<T>(NON_OBJECT_REFERENCE, ""),
    m_stat_type(stat_type),
    m_sampling_condition(std::move(sampling_condition)),
    m_value_ref(std::move(value_ref))
{}

template <class T>
bool Statistic<T>::operator==(const ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const Statistic<T>& rhs_ = static_cast<const Statistic<T>&>(rhs);

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

template <class T>
void Statistic<T>::GetConditionMatches(const ScriptingContext& context,
                                       Condition::ObjectSet& condition_targets,
                                       Condition::ConditionBase* condition) const
{
    condition_targets.clear();
    if (!condition)
        return;
    condition->Eval(context, condition_targets);
}

template <class T>
void Statistic<T>::GetObjectPropertyValues(const ScriptingContext& context,
                                           const Condition::ObjectSet& objects,
                                           std::map<std::shared_ptr<const UniverseObject>, T>& object_property_values) const
{
    object_property_values.clear();

    if (m_value_ref) {
        // evaluate ValueRef with each condition match as the LocalCandidate
        // TODO: Can / should this be paralleized?
        for (auto& object : objects) {
            T property_value = m_value_ref->Eval(ScriptingContext(context, object));
            object_property_values[object] = property_value;
        }
    }
}

template <class T>
bool Statistic<T>::RootCandidateInvariant() const
{
    return Variable<T>::RootCandidateInvariant() &&
           m_sampling_condition->RootCandidateInvariant() &&
           (!m_value_ref || m_value_ref->RootCandidateInvariant());
}

template <class T>
bool Statistic<T>::LocalCandidateInvariant() const
{
    // don't need to check if sampling condition is LocalCandidateInvariant, as
    // all conditions aren't, but that refers to their own local candidate.  no
    // condition is explicitly dependent on the parent context's local candidate.
    return Variable<T>::LocalCandidateInvariant() &&
           (!m_value_ref || m_value_ref->LocalCandidateInvariant());
}

template <class T>
bool Statistic<T>::TargetInvariant() const
{
    return Variable<T>::TargetInvariant() &&
           m_sampling_condition->TargetInvariant() &&
           (!m_value_ref || m_value_ref->TargetInvariant());
}

template <class T>
bool Statistic<T>::SourceInvariant() const
{
    return Variable<T>::SourceInvariant() &&
           m_sampling_condition->SourceInvariant() &&
           (!m_value_ref || m_value_ref->SourceInvariant());
}

template <class T>
std::string Statistic<T>::Description() const
{
    if (m_value_ref)
        return StatisticDescription(m_stat_type, m_value_ref->Description(),
                                    m_sampling_condition ? m_sampling_condition->Description() : "");

    auto temp = Variable<T>::Description();
    if (!temp.empty())
        return StatisticDescription(m_stat_type, temp, m_sampling_condition ? m_sampling_condition->Description() : "");

    return StatisticDescription(m_stat_type, "", m_sampling_condition ? m_sampling_condition->Description() : "");
}

template <class T>
std::string Statistic<T>::Dump(unsigned short ntabs) const
{
    std::string retval = "Statistic ";

    switch (m_stat_type) {
        case COUNT:              retval += "Count";         break;
        case UNIQUE_COUNT:       retval += "CountUnique";   break;
        case IF:                 retval += "If";            break;
        case SUM:                retval += "Sum";           break;
        case MEAN:               retval += "Mean";          break;
        case RMS:                retval += "RMS";           break;
        case MODE:               retval += "Mode";          break;
        case MAX:                retval += "Max";           break;
        case MIN:                retval += "Min";           break;
        case SPREAD:             retval += "Spread";        break;
        case STDEV:              retval += "StDev";         break;
        case PRODUCT:            retval += "Product";       break;
        default:                 retval += "???";           break;
    }
    if (m_value_ref)
        retval += " value = " + m_value_ref->Dump();
    if (m_sampling_condition)
        retval += " condition = " + m_sampling_condition->Dump();
    return retval;
}

template <class T>
void Statistic<T>::SetTopLevelContent(const std::string& content_name)
{
    if (m_sampling_condition)
        m_sampling_condition->SetTopLevelContent(content_name);
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <class T>
T Statistic<T>::Eval(const ScriptingContext& context) const
{
    Condition::ObjectSet condition_matches;
    GetConditionMatches(context, condition_matches, m_sampling_condition.get());

    // special case for IF statistic... return a T(1) for true.
    if (m_stat_type == IF) {
        if (condition_matches.empty())
            return T(0);
        else
            return T(1);
    }

    // todo: consider allowing MAX and MIN using string sorting?

    // the only other statistic that can be computed on non-number property
    // types and that is itself of a non-number type is the most common value
    if (m_stat_type != MODE) {
        ErrorLogger() << "Statistic<std::string>::Eval has invalid statistic type: "
                      << m_stat_type;
        return T(-1);
    }

    // evaluate property for each condition-matched object
    std::map<std::shared_ptr<const UniverseObject>, T> object_property_values;
    GetObjectPropertyValues(context, condition_matches, object_property_values);

    // count number of each result, tracking which has the most occurances
    std::map<T, unsigned int> histogram;
    auto most_common_property_value_it = histogram.begin();
    unsigned int max_seen(0);

    for (const auto& entry : object_property_values) {
        const T& property_value = entry.second;

        auto hist_it = histogram.find(property_value);
        if (hist_it == histogram.end())
            hist_it = histogram.insert({property_value, 0}).first;
        unsigned int& num_seen = hist_it->second;

        num_seen++;

        if (num_seen > max_seen) {
            most_common_property_value_it = hist_it;
            max_seen = num_seen;
        }
    }

    // return result (property value) that occured most frequently
    return most_common_property_value_it->first;
}

template <class T>
unsigned int Statistic<T>::GetCheckSum() const
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
FO_COMMON_API double Statistic<double>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API int Statistic<int>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string Statistic<std::string>::Eval(const ScriptingContext& context) const;

template <class T>
T Statistic<T>::ReduceData(const std::map<std::shared_ptr<const UniverseObject>, T>& object_property_values) const
{
    if (object_property_values.empty())
        return T(0);

    switch (m_stat_type) {
        case COUNT: {
            return T(object_property_values.size());
            break;
        }
        case UNIQUE_COUNT: {
            std::set<T> observed_values;
            for (const auto& entry : object_property_values) {
                observed_values.insert(entry.second);
            }
            return T(observed_values.size());
            break;
        }
        case IF: {
            if (object_property_values.empty())
                return T(0);
            return T(1);
            break;
        }
        case SUM: {
            T accumulator(0);
            for (const auto& entry : object_property_values) {
                accumulator += entry.second;
            }
            return accumulator;
            break;
        }

        case MEAN: {
            T accumulator(0);
            for (const auto& entry : object_property_values) {
                accumulator += entry.second;
            }
            return accumulator / static_cast<T>(object_property_values.size());
            break;
        }

        case RMS: {
            T accumulator(0);
            for (const auto& entry : object_property_values) {
                accumulator += (entry.second * entry.second);
            }
            accumulator /= static_cast<T>(object_property_values.size());

            double retval = std::sqrt(static_cast<double>(accumulator));
            return static_cast<T>(retval);
            break;
        }

        case MODE: {
            // count number of each result, tracking which has the most occurances
            std::map<T, unsigned int> histogram;
            auto most_common_property_value_it = histogram.begin();
            unsigned int max_seen(0);

            for (const auto& entry : object_property_values) {
                const T& property_value = entry.second;

               auto hist_it = histogram.find(property_value);
                if (hist_it == histogram.end())
                    hist_it = histogram.insert({property_value, 0}).first;
                unsigned int& num_seen = hist_it->second;

                num_seen++;

                if (num_seen > max_seen) {
                    most_common_property_value_it = hist_it;
                    max_seen = num_seen;
                }
            }

            // return result (property value) that occured most frequently
            return most_common_property_value_it->first;
            break;
        }

        case MAX: {
            auto max_it = object_property_values.begin();

            for (auto it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            {
                const T& property_value = it->second;
                if (property_value > max_it->second)
                    max_it = it;
            }

            // return maximal observed propery value
            return max_it->second;
            break;
        }

        case MIN: {
            auto min_it = object_property_values.begin();

            for (auto it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            {
                const T& property_value = it->second;
                if (property_value < min_it->second)
                    min_it = it;
            }

            // return minimal observed propery value
            return min_it->second;
            break;
        }

        case SPREAD: {
            auto max_it = object_property_values.begin();
            auto min_it = object_property_values.begin();

            for (auto it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            {
                const T& property_value = it->second;
                if (property_value > max_it->second)
                    max_it = it;
                if (property_value < min_it->second)
                    min_it = it;
            }

            // return difference between maximal and minimal observed propery values
            return max_it->second - min_it->second;
            break;
        }

        case STDEV: {
            if (object_property_values.size() < 2)
                return T(0);

            // find sample mean
            T accumulator(0);
            for (const auto& entry : object_property_values) {
                accumulator += entry.second;
            }
            const T MEAN(accumulator / static_cast<T>(object_property_values.size()));

            // find average of squared deviations from sample mean
            accumulator = T(0);
            for (const auto& entry : object_property_values) {
                accumulator += (entry.second - MEAN) * (entry.second - MEAN);
            }
            const T MEAN_DEV2(accumulator / static_cast<T>(static_cast<int>(object_property_values.size()) - 1));
            double retval = std::sqrt(static_cast<double>(MEAN_DEV2));
            return static_cast<T>(retval);
            break;
        }

        case PRODUCT: {
            T accumulator(1);
            for (const auto& entry : object_property_values) {
                accumulator *= entry.second;
            }
            return accumulator;
            break;
        }

        default:
            throw std::runtime_error("ValueRef evaluated with an unknown or invalid StatisticType.");
            break;
    }
}

template <class T>
template <class Archive>
void Statistic<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Variable)
        & BOOST_SERIALIZATION_NVP(m_stat_type)
        & BOOST_SERIALIZATION_NVP(m_sampling_condition)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// ComplexVariable                                       //
///////////////////////////////////////////////////////////
template <class T>
ComplexVariable<T>::ComplexVariable(const std::string& variable_name,
                                    std::unique_ptr<ValueRefBase<int>>&& int_ref1,
                                    std::unique_ptr<ValueRefBase<int>>&& int_ref2,
                                    std::unique_ptr<ValueRefBase<int>>&& int_ref3,
                                    std::unique_ptr<ValueRefBase<std::string>>&& string_ref1,
                                    std::unique_ptr<ValueRefBase<std::string>>&& string_ref2) :
    Variable<T>(NON_OBJECT_REFERENCE, std::vector<std::string>(1, variable_name)),
    m_int_ref1(std::move(int_ref1)),
    m_int_ref2(std::move(int_ref2)),
    m_int_ref3(std::move(int_ref3)),
    m_string_ref1(std::move(string_ref1)),
    m_string_ref2(std::move(string_ref2))
{
    //std::cout << "ComplexVariable: " << variable_name << ", "
    //          << int_ref1 << ", " << int_ref2 << ", "
    //          << string_ref1 << ", " << string_ref2 << std::endl;
}

template <class T>
bool ComplexVariable<T>::operator==(const ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ComplexVariable<T>& rhs_ = static_cast<const ComplexVariable<T>&>(rhs);

    if (this->m_property_name != rhs_.m_property_name)
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

template <class T>
const ValueRefBase<int>* ComplexVariable<T>::IntRef1() const
{ return m_int_ref1.get(); }

template <class T>
const ValueRefBase<int>* ComplexVariable<T>::IntRef2() const
{ return m_int_ref2.get(); }

template <class T>
const ValueRefBase<int>* ComplexVariable<T>::IntRef3() const
{ return m_int_ref3.get(); }

template <class T>
const ValueRefBase<std::string>* ComplexVariable<T>::StringRef1() const
{ return m_string_ref1.get(); }

template <class T>
const ValueRefBase<std::string>* ComplexVariable<T>::StringRef2() const
{ return m_string_ref2.get(); }

template <class T>
bool ComplexVariable<T>::RootCandidateInvariant() const
{
    return Variable<T>::RootCandidateInvariant()
        && (!m_int_ref1 || m_int_ref1->RootCandidateInvariant())
        && (!m_int_ref2 || m_int_ref2->RootCandidateInvariant())
        && (!m_int_ref3 || m_int_ref3->RootCandidateInvariant())
        && (!m_string_ref1 || m_string_ref1->RootCandidateInvariant())
        && (!m_string_ref2 || m_string_ref2->RootCandidateInvariant());
}

template <class T>
bool ComplexVariable<T>::LocalCandidateInvariant() const
{
    return (!m_int_ref1 || m_int_ref1->LocalCandidateInvariant())
        && (!m_int_ref2 || m_int_ref2->LocalCandidateInvariant())
        && (!m_int_ref3 || m_int_ref3->LocalCandidateInvariant())
        && (!m_string_ref1 || m_string_ref1->LocalCandidateInvariant())
        && (!m_string_ref2 || m_string_ref2->LocalCandidateInvariant());
}

template <class T>
bool ComplexVariable<T>::TargetInvariant() const
{
    return (!m_int_ref1 || m_int_ref1->TargetInvariant())
        && (!m_int_ref2 || m_int_ref2->TargetInvariant())
        && (!m_int_ref3 || m_int_ref3->TargetInvariant())
        && (!m_string_ref1 || m_string_ref1->TargetInvariant())
        && (!m_string_ref2 || m_string_ref2->TargetInvariant());
}

template <class T>
bool ComplexVariable<T>::SourceInvariant() const
{
    return (!m_int_ref1 || m_int_ref1->SourceInvariant())
        && (!m_int_ref2 || m_int_ref2->SourceInvariant())
        && (!m_int_ref3 || m_int_ref3->SourceInvariant())
        && (!m_string_ref1 || m_string_ref1->SourceInvariant())
        && (!m_string_ref2 || m_string_ref2->SourceInvariant());
}

template <class T>
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

template <class T>
std::string ComplexVariable<T>::Dump(unsigned short ntabs) const
{
    return ComplexVariableDump(this->m_property_name,
                               m_int_ref1 ? m_int_ref1.get() : nullptr,
                               m_int_ref2 ? m_int_ref2.get() : nullptr,
                               m_int_ref3 ? m_int_ref3.get() : nullptr,
                               m_string_ref1 ? m_string_ref1.get() : nullptr,
                               m_string_ref2 ? m_string_ref2.get() : nullptr);
}

template <class T>
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

template <class T>
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
FO_COMMON_API Visibility ComplexVariable<Visibility>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API double ComplexVariable<double>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API int ComplexVariable<int>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string ComplexVariable<std::string>::Eval(const ScriptingContext& context) const;

template <>
FO_COMMON_API std::string ComplexVariable<Visibility>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<double>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<int>::Dump(unsigned short ntabs) const;

template <>
FO_COMMON_API std::string ComplexVariable<std::string>::Dump(unsigned short ntabs) const;

template <class T>
template <class Archive>
void ComplexVariable<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Variable)
        & BOOST_SERIALIZATION_NVP(m_int_ref1)
        & BOOST_SERIALIZATION_NVP(m_int_ref2)
        & BOOST_SERIALIZATION_NVP(m_int_ref3)
        & BOOST_SERIALIZATION_NVP(m_string_ref1)
        & BOOST_SERIALIZATION_NVP(m_string_ref2);
}

///////////////////////////////////////////////////////////
// StaticCast                                            //
///////////////////////////////////////////////////////////
template <class FromType, class ToType>
template <typename T>
StaticCast<FromType, ToType>::StaticCast(
    T&& value_ref,
    typename std::enable_if<std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type*) :
    Variable<ToType>(value_ref->GetReferenceType(), value_ref->PropertyName()),
    m_value_ref(std::move(value_ref))
{}

template <class FromType, class ToType>
template <typename T>
StaticCast<FromType, ToType>::StaticCast(
    T&& value_ref,
    typename std::enable_if<
    std::is_convertible<T, std::unique_ptr<ValueRefBase<FromType>>>::value
    && !std::is_convertible<T, std::unique_ptr<Variable<FromType>>>::value>::type*) :
    Variable<ToType>(NON_OBJECT_REFERENCE),
    m_value_ref(std::move(value_ref))
{}

template <class FromType, class ToType>
bool StaticCast<FromType, ToType>::operator==(const ValueRefBase<ToType>& rhs) const
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

template <class FromType, class ToType>
ToType StaticCast<FromType, ToType>::Eval(const ScriptingContext& context) const
{ return static_cast<ToType>(m_value_ref->Eval(context)); }

template <class FromType, class ToType>
bool StaticCast<FromType, ToType>::RootCandidateInvariant() const
{ return m_value_ref->RootCandidateInvariant(); }

template <class FromType, class ToType>
bool StaticCast<FromType, ToType>::LocalCandidateInvariant() const
{ return m_value_ref->LocalCandidateInvariant(); }

template <class FromType, class ToType>
bool StaticCast<FromType, ToType>::TargetInvariant() const
{ return m_value_ref->TargetInvariant(); }

template <class FromType, class ToType>
bool StaticCast<FromType, ToType>::SourceInvariant() const
{ return m_value_ref->SourceInvariant(); }

template <class FromType, class ToType>
std::string StaticCast<FromType, ToType>::Description() const
{ return m_value_ref->Description(); }

template <class FromType, class ToType>
std::string StaticCast<FromType, ToType>::Dump(unsigned short ntabs) const
{ return m_value_ref->Dump(ntabs); }

template <class FromType, class ToType>
void StaticCast<FromType, ToType>::SetTopLevelContent(const std::string& content_name)
{
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <class FromType, class ToType>
unsigned int StaticCast<FromType, ToType>::GetCheckSum() const
{
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::StaticCast");
    CheckSums::CheckSumCombine(retval, m_value_ref);
    TraceLogger() << "GetCheckSum(StaticCast<FromType, ToType>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <class FromType, class ToType>
template <class Archive>
void StaticCast<FromType, ToType>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// StringCast                                            //
///////////////////////////////////////////////////////////
template <class FromType>
StringCast<FromType>::StringCast(std::unique_ptr<ValueRefBase<FromType>>&& value_ref) :
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
}

template <class FromType>
bool StringCast<FromType>::operator==(const ValueRefBase<std::string>& rhs) const
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

template <class FromType>
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

template <class FromType>
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

template <class FromType>
bool StringCast<FromType>::RootCandidateInvariant() const
{ return m_value_ref->RootCandidateInvariant(); }

template <class FromType>
bool StringCast<FromType>::LocalCandidateInvariant() const
{ return m_value_ref->LocalCandidateInvariant(); }

template <class FromType>
bool StringCast<FromType>::TargetInvariant() const
{ return m_value_ref->TargetInvariant(); }

template <class FromType>
bool StringCast<FromType>::SourceInvariant() const
{ return m_value_ref->SourceInvariant(); }

template <class FromType>
std::string StringCast<FromType>::Description() const
{ return m_value_ref->Description(); }

template <class FromType>
std::string StringCast<FromType>::Dump(unsigned short ntabs) const
{ return m_value_ref->Dump(ntabs); }

template <class FromType>
void StringCast<FromType>::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <class FromType>
template <class Archive>
void StringCast<FromType>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// UserStringLookup                                      //
///////////////////////////////////////////////////////////
template <class FromType>
UserStringLookup<FromType>::UserStringLookup(std::unique_ptr<ValueRefBase<FromType>>&& value_ref) :
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
}

template <class FromType>
bool UserStringLookup<FromType>::operator==(const ValueRefBase<std::string>& rhs) const {
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

template <class FromType>
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

template <class FromType>
bool UserStringLookup<FromType>::RootCandidateInvariant() const
{
    return m_value_ref->RootCandidateInvariant();
}

template <class FromType>
bool UserStringLookup<FromType>::LocalCandidateInvariant() const
{
    return !m_value_ref || m_value_ref->LocalCandidateInvariant();
}

template <class FromType>
bool UserStringLookup<FromType>::TargetInvariant() const
{
    return !m_value_ref || m_value_ref->TargetInvariant();
}

template <class FromType>
bool UserStringLookup<FromType>::SourceInvariant() const
{
    return !m_value_ref || m_value_ref->SourceInvariant();
}

template <class FromType>
std::string UserStringLookup<FromType>::Description() const
{
    return m_value_ref->Description();
}

template <class FromType>
std::string UserStringLookup<FromType>::Dump(unsigned short ntabs) const
{
    return m_value_ref->Dump(ntabs);
}

template <class FromType>
void UserStringLookup<FromType>::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <class FromType>
unsigned int UserStringLookup<FromType>::GetCheckSum() const
{
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::UserStringLookup");
    CheckSums::CheckSumCombine(retval, m_value_ref);
    TraceLogger() << "GetCheckSum(UserStringLookup<FromType>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <class FromType>
template <class Archive>
void UserStringLookup<FromType>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase<std::string>)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// NameLookup                                            //
///////////////////////////////////////////////////////////
template <class Archive>
void NameLookup::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase<std::string>)
        & BOOST_SERIALIZATION_NVP(m_value_ref)
        & BOOST_SERIALIZATION_NVP(m_lookup_type);
}

///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
template <class T>
Operation<T>::Operation(OpType op_type,
                        std::unique_ptr<ValueRefBase<T>>&& operand1,
                        std::unique_ptr<ValueRefBase<T>>&& operand2) :
    m_op_type(op_type),
    m_operands()
{
    if (operand1)
        m_operands.push_back(std::move(operand1));
    if (operand2)
        m_operands.push_back(std::move(operand2));
    DetermineIfConstantExpr();
    CacheConstValue();
}

template <class T>
Operation<T>::Operation(OpType op_type, std::unique_ptr<ValueRefBase<T>>&& operand) :
    m_op_type(op_type),
    m_operands()
{
    if (operand)
        m_operands.push_back(std::move(operand));
    DetermineIfConstantExpr();
    CacheConstValue();
}

template <class T>
Operation<T>::Operation(OpType op_type, std::vector<std::unique_ptr<ValueRefBase<T>>>&& operands) :
    m_op_type(op_type),
    m_operands(std::move(operands))
{
    DetermineIfConstantExpr();
    CacheConstValue();
}

template <class T>
void Operation<T>::DetermineIfConstantExpr()
{
    if (m_op_type == RANDOM_UNIFORM || m_op_type == RANDOM_PICK) {
        m_constant_expr = false;
        return;
    }

    m_constant_expr = true; // may be overridden...

    for (auto& operand : m_operands) {
        if (operand && !operand->ConstantExpr()) {
            m_constant_expr = false;
            return;
        }
    }
}

template <class T>
void Operation<T>::CacheConstValue()
{
    if (!m_constant_expr)
        return;

    m_cached_const_value = this->EvalImpl(ScriptingContext());
}

template <class T>
bool Operation<T>::operator==(const ValueRefBase<T>& rhs) const
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

template <class T>
OpType Operation<T>::GetOpType() const
{ return m_op_type; }

template <class T>
const ValueRefBase<T>* Operation<T>::LHS() const
{
    if (m_operands.empty())
        return nullptr;
    return m_operands[0].get();
}

template <class T>
const ValueRefBase<T>* Operation<T>::RHS() const
{
    if (m_operands.size() < 2)
        return nullptr;
    return m_operands[1].get();
}

template <class T>
const std::vector<ValueRefBase<T>*> Operation<T>::Operands() const
{
    std::vector<ValueRefBase<T>*> retval(m_operands.size());
    std::transform(m_operands.begin(), m_operands.end(), retval.begin(),
                   [](const std::unique_ptr<ValueRefBase<T>>& p) { return *p; });
    return retval;
}

template <class T>
T Operation<T>::Eval(const ScriptingContext& context) const
{
    if (m_constant_expr)
        return m_cached_const_value;
    return this->EvalImpl(context);
}

template <class T>
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
                vals.insert(vr->Eval(context));
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
        unsigned int idx = RandSmallInt(0, m_operands.size() - 1);
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

template <class T>
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

template <>
FO_COMMON_API std::string Operation<std::string>::EvalImpl(const ScriptingContext& context) const;

template <>
FO_COMMON_API double Operation<double>::EvalImpl(const ScriptingContext& context) const;

template <>
FO_COMMON_API int Operation<int>::EvalImpl(const ScriptingContext& context) const;

template <class T>
bool Operation<T>::RootCandidateInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM || m_op_type == RANDOM_PICK)
        return false;
    for (auto& operand : m_operands) {
        if (operand && !operand->RootCandidateInvariant())
            return false;
    }
    return true;
}

template <class T>
bool Operation<T>::LocalCandidateInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM || m_op_type == RANDOM_PICK)
        return false;
    for (auto& operand : m_operands) {
        if (operand && !operand->LocalCandidateInvariant())
            return false;
    }
    return true;
}

template <class T>
bool Operation<T>::TargetInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM || m_op_type == RANDOM_PICK)
        return false;
    for (auto& operand : m_operands) {
        if (operand && !operand->TargetInvariant())
            return false;
    }
    return true;
}

template <class T>
bool Operation<T>::SourceInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM || m_op_type == RANDOM_PICK)
        return false;
    for (auto& operand : m_operands) {
        if (operand && !operand->SourceInvariant())
            return false;
    }
    return true;
}

template <class T>
bool Operation<T>::SimpleIncrement() const
{
    if (m_op_type != PLUS && m_op_type != MINUS)
        return false;
    if (m_operands.size() < 2 || !m_operands[0] || !m_operands[1])
        return false;
    if (!(m_operands[1]->ConstantExpr()))
        return false;
    const auto lhs = dynamic_cast<const Variable<T>*>(m_operands[0].get());
    if (!lhs)
        return false;
    return lhs->GetReferenceType() == EFFECT_TARGET_VALUE_REFERENCE;
}

template <class T>
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

template <class T>
std::string Operation<T>::Dump(unsigned short ntabs) const
{
    if (m_op_type == NEGATE) {
        if (auto rhs = dynamic_cast<const Operation<T>*>(LHS())) {
            OpType op_type = rhs->GetOpType();
            if (op_type == PLUS     || op_type == MINUS ||
                op_type == TIMES    || op_type == DIVIDE ||
                op_type == NEGATE   || op_type == EXPONENTIATE)
            return "-(" + LHS()->Dump(ntabs) + ")";
        } else {
            return "-" + LHS()->Dump(ntabs);
        }
    }

    if (m_op_type == ABS)
        return "abs(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == LOGARITHM)
        return "log(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == SINE)
        return "sin(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == COSINE)
        return "cos(" + LHS()->Dump(ntabs) + ")";

    if (m_op_type == MINIMUM) {
        std::string retval = "min(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }
    if (m_op_type == MAXIMUM) {
        std::string retval = "max(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == RANDOM_UNIFORM)
        return "random(" + LHS()->Dump(ntabs) + ", " + LHS()->Dump(ntabs) + ")";

    if (m_op_type == RANDOM_PICK) {
        std::string retval = "randompick(";
        for (auto it = m_operands.begin(); it != m_operands.end(); ++it) {
            if (it != m_operands.begin())
                retval += ", ";
            retval += (*it)->Dump(ntabs);
        }
        retval += ")";
        return retval;
    }

    if (m_op_type == ROUND_NEAREST)
        return "round(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == ROUND_UP)
        return "ceil(" + LHS()->Dump(ntabs) + ")";
    if (m_op_type == ROUND_DOWN)
        return "floor(" + LHS()->Dump(ntabs) + ")";

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
        retval += '(' + LHS()->Dump(ntabs) + ')';
    else
        retval += LHS()->Dump(ntabs);

    switch (m_op_type) {
    case PLUS:          retval += " + "; break;
    case MINUS:         retval += " - "; break;
    case TIMES:         retval += " * "; break;
    case DIVIDE:        retval += " / "; break;
    case EXPONENTIATE:  retval += " ^ "; break;
    default:            retval += " ? "; break;
    }

    if (parenthesize_rhs)
        retval += '(' + RHS()->Dump(ntabs) + ')';
    else
        retval += RHS()->Dump(ntabs);

    return retval;
}

template <class T>
void Operation<T>::SetTopLevelContent(const std::string& content_name) {
    for (auto& operand : m_operands) {
        if (operand)
            operand->SetTopLevelContent(content_name);
    }
}

template <class T>
template <class Archive>
void Operation<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_op_type)
        & BOOST_SERIALIZATION_NVP(m_operands)
        & BOOST_SERIALIZATION_NVP(m_constant_expr)
        & BOOST_SERIALIZATION_NVP(m_cached_const_value);
}

} // namespace ValueRef

#endif // _ValueRef_h_
