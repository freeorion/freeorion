// -*- C++ -*-
#ifndef _ValueRef_h_
#define _ValueRef_h_

#include "Condition.h"
#include "../util/Export.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>

#include <map>
#include <set>

class UniverseObject;

const std::string& UserString(const std::string& str);
boost::format FlexibleFormat(const std::string& string_to_format);

struct ScriptingContext {
    /** Empty context.  Useful for evaluating ValueRef::Constant that don't
      * depend on their context. */
    ScriptingContext()
    {}

    /** Context with only a source object.  Useful for evaluating effectsgroup
      * scope and activation conditions that have no external candidates or
      * effect target to propegate. */
    explicit ScriptingContext(TemporaryPtr<const UniverseObject> source_) :
        source(source_)
    {}

    ScriptingContext(TemporaryPtr<const UniverseObject> source_, TemporaryPtr<UniverseObject> target_) :
        source(source_),
        effect_target(target_)
    {}

    ScriptingContext(TemporaryPtr<const UniverseObject> source_, TemporaryPtr<UniverseObject> target_,
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
        current_value(current_value_)
    {}

    /** For recusrive evaluation of Conditions.  Keeps source and effect_target
      * from input context, but sets local candidate with input object, and if
      * there is no root candidate in the parent context, then the input object
      * becomes the root candidate. */
    ScriptingContext(const ScriptingContext& parent_context,
                     TemporaryPtr<const UniverseObject> condition_local_candidate) :
        source(                     parent_context.source),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate ?
                                        parent_context.condition_root_candidate :
                                        condition_local_candidate),                 // if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(  condition_local_candidate),                     // new local candidate
        current_value(              parent_context.current_value)
    {}

    ScriptingContext(TemporaryPtr<const UniverseObject> source_, TemporaryPtr<UniverseObject> target_,
                     const boost::any& current_value_,
                     TemporaryPtr<const UniverseObject> condition_root_candidate_,
                     TemporaryPtr<const UniverseObject> condition_local_candidate_) :
        source(source_),
        condition_root_candidate(condition_root_candidate_),
        condition_local_candidate(condition_local_candidate_),
        current_value(current_value_)
    {}

    TemporaryPtr<const UniverseObject>  source;
    TemporaryPtr<UniverseObject>        effect_target;
    TemporaryPtr<const UniverseObject>  condition_root_candidate;
    TemporaryPtr<const UniverseObject>  condition_local_candidate;
    const boost::any                    current_value;
};

/** The base class for all ValueRef classes.  This class provides the public
  * interface for a ValueRef expression tree. */
template <class T>
struct ValueRef::ValueRefBase
{
    virtual ~ValueRefBase() {} ///< virtual dtor

    virtual bool        operator==(const ValueRef::ValueRefBase<T>& rhs) const;
    bool                operator!=(const ValueRef::ValueRefBase<T>& rhs) const { return !(*this == rhs); }

    /** Evaluates the expression tree and return the results; \a context
      * is used to fill in any instances of the "Value" variable or references
      * to objects such as the source, effect target, or condition candidates
      * that exist in the tree. */
    virtual T           Eval(const ScriptingContext& context) const = 0;

    /** Evaluates the expression tree with an empty context.  Useful for
      * evaluating expressions that do not depend on context. */
    T                   Eval() const { return Eval(::ScriptingContext()); }

    virtual bool        RootCandidateInvariant() const { return false; }
    virtual bool        LocalCandidateInvariant() const { return false; }
    virtual bool        TargetInvariant() const { return false; }
    virtual bool        SourceInvariant() const { return false; }

    virtual std::string Description() const = 0;
    virtual std::string Dump() const = 0; ///< returns a text description of this type of special

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** the constant value leaf ValueRef class. */
template <class T>
struct FO_COMMON_API ValueRef::Constant : public ValueRef::ValueRefBase<T>
{
    Constant(T value); ///< basic ctor

    virtual bool        operator==(const ValueRef::ValueRefBase<T>& rhs) const;
    T                   Value() const;
    virtual T           Eval(const ScriptingContext& context) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        LocalCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }

    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    T           m_value;
    std::string m_top_level_content;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The variable value ValueRef class.  The value returned by this node is
  * taken from the gamestate, most often from the Source or Target objects. */
template <class T>
struct FO_COMMON_API ValueRef::Variable : public ValueRef::ValueRefBase<T>
{
    Variable(ReferenceType ref_type, const std::vector<std::string>& property_name);
    Variable(ReferenceType ref_type, const std::string& property_name = "");

    virtual bool                    operator==(const ValueRef::ValueRefBase<T>& rhs) const;
    ReferenceType                   GetReferenceType() const;
    const std::vector<std::string>& PropertyName() const;
    virtual T                       Eval(const ScriptingContext& context) const;
    virtual bool                    RootCandidateInvariant() const;
    virtual bool                    LocalCandidateInvariant() const;
    virtual bool                    TargetInvariant() const;
    virtual bool                    SourceInvariant() const;
    virtual std::string             Description() const;
    virtual std::string             Dump() const;

protected:
    mutable ReferenceType       m_ref_type;
    std::vector<std::string>    m_property_name;

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
struct FO_COMMON_API ValueRef::Statistic : public ValueRef::Variable<T>
{
    Statistic(ValueRef::ValueRefBase<T>* value_ref,
              StatisticType stat_type,
              Condition::ConditionBase* sampling_condition);

    ~Statistic();

    virtual bool                    operator==(const ValueRef::ValueRefBase<T>& rhs) const;

    StatisticType                   GetStatisticType() const    { return m_stat_type; }
    const Condition::ConditionBase* GetSamplingCondition() const{ return m_sampling_condition; }
    const ValueRef::ValueRefBase<T>*GetValueRef() const         { return m_value_ref; }

    virtual T                       Eval(const ScriptingContext& context) const;

    virtual bool                    RootCandidateInvariant() const;
    virtual bool                    LocalCandidateInvariant() const;
    virtual bool                    TargetInvariant() const;
    virtual bool                    SourceInvariant() const;

    virtual std::string             Description() const;
    virtual std::string             Dump() const;

    virtual void                    SetTopLevelContent(const std::string& content_name);

protected:
    /** Gets the set of objects in the Universe that match the sampling condition. */
    void    GetConditionMatches(const ScriptingContext& context,
                                Condition::ObjectSet& condition_targets,
                                Condition::ConditionBase* condition) const;

    /** Evaluates the property for the specified objects. */
    void    GetObjectPropertyValues(const ScriptingContext& context,
                                    const Condition::ObjectSet& objects,
                                    std::map<TemporaryPtr<const UniverseObject>, T>& object_property_values) const;

    /** Computes the statistic from the specified set of property values. */
    T       ReduceData(const std::map<TemporaryPtr<const UniverseObject>, T>& object_property_values) const;

private:
    StatisticType               m_stat_type;
    Condition::ConditionBase*   m_sampling_condition;
    ValueRef::ValueRefBase<T>*  m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The complex variable ValueRef class. The value returned by this node
  * is taken from the gamestate. */
template <class T>
struct FO_COMMON_API ValueRef::ComplexVariable : public ValueRef::Variable<T>
{
    explicit ComplexVariable(const std::string& variable_name,
                             ValueRefBase<int>* int_ref1 = 0,
                             ValueRefBase<int>* int_ref2 = 0,
                             ValueRefBase<int>* int_ref3 = 0,
                             ValueRefBase<std::string>* string_ref1 = 0,
                             ValueRefBase<std::string>* string_ref2 = 0);

    ~ComplexVariable();

    virtual bool                    operator==(const ValueRef::ValueRefBase<T>& rhs) const;
    const ValueRefBase<int>*        IntRef1() const;
    const ValueRefBase<int>*        IntRef2() const;
    const ValueRefBase<int>*        IntRef3() const;
    const ValueRefBase<std::string>*StringRef1() const;
    const ValueRefBase<std::string>*StringRef2() const;
    virtual T                       Eval(const ScriptingContext& context) const;
    virtual bool                    RootCandidateInvariant() const;
    virtual bool                    LocalCandidateInvariant() const;
    virtual bool                    TargetInvariant() const;
    virtual bool                    SourceInvariant() const;
    virtual std::string             Description() const;
    virtual std::string             Dump() const;

    virtual void                    SetTopLevelContent(const std::string& content_name);

protected:
    ValueRefBase<int>*        m_int_ref1;
    ValueRefBase<int>*        m_int_ref2;
    ValueRefBase<int>*        m_int_ref3;
    ValueRefBase<std::string>*m_string_ref1;
    ValueRefBase<std::string>*m_string_ref2;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The variable static_cast class.  The value returned by this node is taken
  * from the ctor \a value_ref parameter's FromType value, static_cast to
  * ToType. */
template <class FromType, class ToType>
struct FO_COMMON_API ValueRef::StaticCast : public ValueRef::Variable<ToType>
{
    StaticCast(ValueRef::Variable<FromType>* value_ref);
    StaticCast(ValueRef::ValueRefBase<FromType>* value_ref);
    ~StaticCast();

    virtual bool        operator==(const ValueRef::ValueRefBase<ToType>& rhs) const;
    virtual ToType      Eval(const ScriptingContext& context) const;
    virtual bool        RootCandidateInvariant() const;
    virtual bool        LocalCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description() const;
    virtual std::string Dump() const;
    const ValueRefBase<FromType>*   GetValueRef() const { return m_value_ref; }

    virtual void                    SetTopLevelContent(const std::string& content_name);

private:
    ValueRefBase<FromType>* m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The variable lexical_cast to string class.  The value returned by this node
  * is taken from the ctor \a value_ref parameter's FromType value,
  * lexical_cast to std::string */
template <class FromType>
struct FO_COMMON_API ValueRef::StringCast : public ValueRef::Variable<std::string>
{
    StringCast(ValueRef::Variable<FromType>* value_ref);
    StringCast(ValueRef::ValueRefBase<FromType>* value_ref);
    ~StringCast();

    virtual bool        operator==(const ValueRef::ValueRefBase<std::string>& rhs) const;
    virtual std::string Eval(const ScriptingContext& context) const;
    virtual bool        RootCandidateInvariant() const;
    virtual bool        LocalCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description() const;
    virtual std::string Dump() const;
    const ValueRefBase<FromType>*   GetValueRef() const { return m_value_ref; }

    virtual void                    SetTopLevelContent(const std::string& content_name);

private:
    ValueRefBase<FromType>* m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Looks up a string ValueRef and returns the UserString equivalent. */
struct FO_COMMON_API ValueRef::UserStringLookup : public ValueRef::Variable<std::string> {
    UserStringLookup(ValueRef::Variable<std::string>* value_ref);
    UserStringLookup(ValueRef::ValueRefBase<std::string>* value_ref);
    ~UserStringLookup();

    virtual bool        operator==(const ValueRef::ValueRefBase<std::string>& rhs) const;
    virtual std::string Eval(const ScriptingContext& context) const;
    virtual bool        RootCandidateInvariant() const;
    virtual bool        LocalCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description() const;
    virtual std::string Dump() const;
    const ValueRefBase<std::string>*    GetValueRef() const { return m_value_ref; }

    virtual void                        SetTopLevelContent(const std::string& content_name);

private:
    ValueRefBase<std::string>* m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** An arithmetic operation node ValueRef class.  One of addition, subtraction,
  * mutiplication, division, or unary negation is performed on the child(ren)
  * of this node, and the result is returned. */
template <class T>
struct FO_COMMON_API ValueRef::Operation : public ValueRef::ValueRefBase<T>
{
    Operation(OpType op_type, ValueRefBase<T>* operand1,
              ValueRefBase<T>* operand2);               ///< binary operation ctor
    Operation(OpType op_type, ValueRefBase<T>* operand);///< unary operation ctor
    ~Operation();

    virtual bool            operator==(const ValueRef::ValueRefBase<T>& rhs) const;
    OpType                  GetOpType() const;
    const ValueRefBase<T>*  LHS() const;
    const ValueRefBase<T>*  RHS() const;
    virtual T               Eval(const ScriptingContext& context) const;
    virtual bool            RootCandidateInvariant() const;
    virtual bool            LocalCandidateInvariant() const;
    virtual bool            TargetInvariant() const;
    virtual bool            SourceInvariant() const;
    virtual std::string     Description() const;
    virtual std::string     Dump() const;

    virtual void            SetTopLevelContent(const std::string& content_name);

private:
    OpType              m_op_type;
    ValueRefBase<T>*    m_operand1;
    ValueRefBase<T>*    m_operand2;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

namespace ValueRef {
    FO_COMMON_API MeterType     NameToMeter(const std::string& name);
    FO_COMMON_API std::string   MeterToName(MeterType meter);
    FO_COMMON_API std::string   ReconstructName(const std::vector<std::string>& property_name,
                                                ReferenceType ref_type);
}

// Template Implementations
///////////////////////////////////////////////////////////
// ValueRefBase                                          //
///////////////////////////////////////////////////////////
template <class T>
bool ValueRef::ValueRefBase<T>::operator==(const ValueRef::ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    return true;
}

template <class T>
template <class Archive>
void ValueRef::ValueRefBase<T>::serialize(Archive& ar, const unsigned int version)
{}

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Constant<T>::Constant(T value) :
    m_value(value)
{}

template <class T>
bool ValueRef::Constant<T>::operator==(const ValueRef::ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::Constant<T>& rhs_ = static_cast<const ValueRef::Constant<T>&>(rhs);

    return m_value == rhs_.m_value && m_top_level_content == rhs_.m_top_level_content;
}

template <class T>
T ValueRef::Constant<T>::Value() const
{ return m_value; }

template <class T>
T ValueRef::Constant<T>::Eval(const ScriptingContext& context) const
{ return m_value; }

template <class T>
std::string ValueRef::Constant<T>::Description() const
{ return UserString(boost::lexical_cast<std::string>(m_value)); }

template <class T>
void ValueRef::Constant<T>::SetTopLevelContent(const std::string& content_name)
{ m_top_level_content = content_name; }

namespace ValueRef {
    template <>
    std::string Constant<int>::Description() const;

    template <>
    std::string Constant<double>::Description() const;

    template <>
    std::string Constant<std::string>::Description() const;

    template <>
    std::string Constant<PlanetSize>::Dump() const;

    template <>
    std::string Constant<PlanetType>::Dump() const;

    template <>
    std::string Constant<PlanetEnvironment>::Dump() const;

    template <>
    std::string Constant<UniverseObjectType>::Dump() const;

    template <>
    std::string Constant<StarType>::Dump() const;

    template <>
    std::string Constant<double>::Dump() const;

    template <>
    std::string Constant<int>::Dump() const;

    template <>
    std::string Constant<std::string>::Dump() const;

    template <>
    std::string Constant<std::string>::Eval(const ScriptingContext& context) const;
}

template <class T>
template <class Archive>
void ValueRef::Constant<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value)
        & BOOST_SERIALIZATION_NVP(m_top_level_content);
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Variable<T>::Variable(ReferenceType ref_type, const std::vector<std::string>& property_name) :
    m_ref_type(ref_type),
    m_property_name(property_name.begin(), property_name.end())
{}

template <class T>
ValueRef::Variable<T>::Variable(ReferenceType ref_type, const std::string& property_name) :
    m_ref_type(ref_type),
    m_property_name()
{
    m_property_name.push_back(property_name);
}

template <class T>
bool ValueRef::Variable<T>::operator==(const ValueRef::ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::Variable<T>& rhs_ = static_cast<const ValueRef::Variable<T>&>(rhs);
    return (m_ref_type == rhs_.m_ref_type) && (m_property_name == rhs_.m_property_name);
}

template <class T>
ValueRef::ReferenceType ValueRef::Variable<T>::GetReferenceType() const
{ return m_ref_type; }

template <class T>
const std::vector<std::string>& ValueRef::Variable<T>::PropertyName() const
{ return m_property_name; }

template <class T>
bool ValueRef::Variable<T>::RootCandidateInvariant() const
{ return m_ref_type != CONDITION_ROOT_CANDIDATE_REFERENCE; }

template <class T>
bool ValueRef::Variable<T>::LocalCandidateInvariant() const
{ return m_ref_type != CONDITION_LOCAL_CANDIDATE_REFERENCE; }

template <class T>
bool ValueRef::Variable<T>::TargetInvariant() const
{ return m_ref_type != EFFECT_TARGET_REFERENCE && m_ref_type != EFFECT_TARGET_VALUE_REFERENCE; }

template <class T>
bool ValueRef::Variable<T>::SourceInvariant() const
{ return m_ref_type != SOURCE_REFERENCE; }

FO_COMMON_API std::string FormatedDescriptionPropertyNames(ValueRef::ReferenceType ref_type,
                                                           const std::vector<std::string>& property_names);

template <class T>
std::string ValueRef::Variable<T>::Description() const
{
    return FormatedDescriptionPropertyNames(m_ref_type, m_property_name);
}

template <class T>
std::string ValueRef::Variable<T>::Dump() const
{ return ReconstructName(m_property_name, m_ref_type); }

namespace ValueRef {
    template <>
    PlanetSize Variable<PlanetSize>::Eval(const ScriptingContext& context) const;

    template <>
    PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const;

    template <>
    PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const;

    template <>
    UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const;

    template <>
    StarType Variable<StarType>::Eval(const ScriptingContext& context) const;

    template <>
    double Variable<double>::Eval(const ScriptingContext& context) const;

    template <>
    int Variable<int>::Eval(const ScriptingContext& context) const;
}

template <class T>
template <class Archive>
void ValueRef::Variable<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_ref_type)
        & BOOST_SERIALIZATION_NVP(m_property_name);
}

///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Statistic<T>::Statistic(ValueRef::ValueRefBase<T>* value_ref,
                                  StatisticType stat_type,
                                  Condition::ConditionBase* sampling_condition) :
    Variable<T>(ValueRef::NON_OBJECT_REFERENCE, ""),
    m_stat_type(stat_type),
    m_sampling_condition(sampling_condition),
    m_value_ref(value_ref)
{}

template <class T>
ValueRef::Statistic<T>::~Statistic()
{
    delete m_sampling_condition;
    delete m_value_ref;
}

template <class T>
bool ValueRef::Statistic<T>::operator==(const ValueRef::ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::Statistic<T>& rhs_ = static_cast<const ValueRef::Statistic<T>&>(rhs);

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
void ValueRef::Statistic<T>::GetConditionMatches(const ScriptingContext& context,
                                                 Condition::ObjectSet& condition_targets,
                                                 Condition::ConditionBase* condition) const
{
    condition_targets.clear();
    if (!condition)
        return;
    condition->Eval(context, condition_targets);
}

template <class T>
void ValueRef::Statistic<T>::GetObjectPropertyValues(const ScriptingContext& context,
                                                     const Condition::ObjectSet& objects,
                                                     std::map<TemporaryPtr<const UniverseObject>, T>& object_property_values) const
{
    object_property_values.clear();

    if (m_value_ref) {
        // evaluate ValueRef with each condition match as the LocalCandidate
        // TODO: Can / should this be paralleized?
        for (Condition::ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it) {
            T property_value = m_value_ref->Eval(ScriptingContext(context, *it));
            object_property_values[*it] = property_value;
        }
    }
}

template <class T>
bool ValueRef::Statistic<T>::RootCandidateInvariant() const
{
    return ValueRef::Variable<T>::RootCandidateInvariant() &&
           m_sampling_condition->RootCandidateInvariant() &&
           (!m_value_ref || m_value_ref->RootCandidateInvariant());
}

template <class T>
bool ValueRef::Statistic<T>::LocalCandidateInvariant() const
{
    // don't need to check if sampling condition is LocalCandidateInvariant, as
    // all conditions aren't, but that refers to their own local candidate.  no
    // condition is explicitly dependent on the parent context's local candidate.
    return ValueRef::Variable<T>::LocalCandidateInvariant() &&
           (!m_value_ref || m_value_ref->LocalCandidateInvariant());
}

template <class T>
bool ValueRef::Statistic<T>::TargetInvariant() const
{
    return ValueRef::Variable<T>::TargetInvariant() &&
           m_sampling_condition->TargetInvariant() &&
           (!m_value_ref || m_value_ref->TargetInvariant());
}

template <class T>
bool ValueRef::Statistic<T>::SourceInvariant() const
{
    return ValueRef::Variable<T>::SourceInvariant() &&
           m_sampling_condition->SourceInvariant() &&
           (!m_value_ref || m_value_ref->SourceInvariant());
}

template <class T>
std::string ValueRef::Statistic<T>::Description() const {
    std::string retval = UserString("DESC_STATISTIC") + ": [(" + UserString("DESC_STAT_TYPE") + ": " + boost::lexical_cast<std::string>(m_stat_type) + ")";
    switch (m_stat_type) {
        case ValueRef::COUNT:              retval += "(COUNT)";            break;
        case ValueRef::UNIQUE_COUNT:       retval += "(UNIQUE_COUNT)";     break;
        case ValueRef::IF:                 retval += "(IF ANY)";           break;
        case ValueRef::SUM:                retval += "(SUM)";              break;
        case ValueRef::MEAN:               retval += "(MEAN)";             break;
        case ValueRef::RMS:                retval += "(RMS)";              break;
        case ValueRef::MODE:               retval += "(MODE)";             break;
        case ValueRef::MAX:                retval += "(MAX)";              break;
        case ValueRef::MIN:                retval += "(MIN)";              break;
        case ValueRef::SPREAD:             retval += "(SPREAD)";           break;
        case ValueRef::STDEV:              retval += "(STDEV)";            break;
        case ValueRef::PRODUCT:            retval += "(PRODUCT)";          break;
        default:                           retval += "()";                 break;
    }
    if (m_value_ref) {
        retval += "(" + m_value_ref->Description() + ")";
    } else if (!ValueRef::Variable<T>::Description().empty()){
        retval += "(" + ValueRef::Variable<T>::Description() + ")";
    }
    retval += "(" + UserString("DESC_SAMPLING_CONDITION") + ": " + m_sampling_condition->Description() + ")]";
    return retval;
}

template <class T>
std::string ValueRef::Statistic<T>::Dump() const
{ return Description(); }

template <class T>
void ValueRef::Statistic<T>::SetTopLevelContent(const std::string& content_name)
{
    if (m_sampling_condition)
        m_sampling_condition->SetTopLevelContent(content_name);
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <class T>
T ValueRef::Statistic<T>::Eval(const ScriptingContext& context) const
{
    // the only statistic that can be computed on non-number property types
    // and that is itself of a non-number type is the most common value
    if (m_stat_type != MODE)
        throw std::runtime_error("ValueRef evaluated with an invalid StatisticType for the return type.");

    Condition::ObjectSet condition_matches;
    GetConditionMatches(context, condition_matches, m_sampling_condition);

    if (condition_matches.empty())
        return T(-1);   // should be INVALID_T of enum types

    // evaluate property for each condition-matched object
    std::map<TemporaryPtr<const UniverseObject>, T> object_property_values;
    GetObjectPropertyValues(context, condition_matches, object_property_values);

    // count number of each result, tracking which has the most occurances
    std::map<T, unsigned int> histogram;
    typename std::map<T, unsigned int>::const_iterator most_common_property_value_it = histogram.begin();
    unsigned int max_seen(0);

    for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
         it != object_property_values.end(); ++it)
    {
        const T& property_value = it->second;

        typename std::map<T, unsigned int>::iterator hist_it = histogram.find(property_value);
        if (hist_it == histogram.end())
            hist_it = histogram.insert(std::make_pair(property_value, 0)).first;
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

namespace ValueRef {
    template <>
    double Statistic<double>::Eval(const ScriptingContext& context) const;

    template <>
    int Statistic<int>::Eval(const ScriptingContext& context) const;

    template <>
    std::string Statistic<std::string>::Eval(const ScriptingContext& context) const;
}

template <class T>
T ValueRef::Statistic<T>::ReduceData(const std::map<TemporaryPtr<const UniverseObject>, T>& object_property_values) const
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
            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            { observed_values.insert(it->second); }
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
            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            { accumulator += it->second; }
            return accumulator;
            break;
        }

        case MEAN: {
            T accumulator(0);
            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            { accumulator += it->second; }
            return accumulator / static_cast<T>(object_property_values.size());
            break;
        }

        case RMS: {
            T accumulator(0);
            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            { accumulator += (it->second * it->second); }
            accumulator /= static_cast<T>(object_property_values.size());

            double retval = std::sqrt(static_cast<double>(accumulator));
            return static_cast<T>(retval);
            break;
        }

        case MODE: {
            // count number of each result, tracking which has the most occurances
            std::map<T, unsigned int> histogram;
            typename std::map<T, unsigned int>::const_iterator most_common_property_value_it = histogram.begin();
            unsigned int max_seen(0);

            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            {
                const T& property_value = it->second;

                typename std::map<T, unsigned int>::iterator hist_it = histogram.find(property_value);
                if (hist_it == histogram.end())
                    hist_it = histogram.insert(std::make_pair(property_value, 0)).first;
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
            typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator max_it = object_property_values.begin();

            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
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
            typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator min_it = object_property_values.begin();

            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
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
            typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator max_it = object_property_values.begin();
            typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator min_it = object_property_values.begin();

            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
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
            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            { accumulator += it->second; }
            const T MEAN(accumulator / static_cast<T>(object_property_values.size()));

            // find average of squared deviations from sample mean
            accumulator = T(0);
            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            { accumulator += (it->second - MEAN) * (it->second - MEAN); }
            const T MEAN_DEV2(accumulator / static_cast<T>(static_cast<int>(object_property_values.size()) - 1));
            double retval = std::sqrt(static_cast<double>(MEAN_DEV2));
            return static_cast<T>(retval);
            break;
        }

        case PRODUCT: {
            T accumulator(1);
            for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
                 it != object_property_values.end(); ++it)
            { accumulator *= it->second; }
            return accumulator;
            break;
        }

        default:
            throw std::runtime_error("ValueRef evaluated with an unknown or invalid StatisticType.");
            break;
    }

    return T(0);
}

template <class T>
template <class Archive>
void ValueRef::Statistic<T>::serialize(Archive& ar, const unsigned int version)
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
ValueRef::ComplexVariable<T>::ComplexVariable(const std::string& variable_name,
                                              ValueRefBase<int>* int_ref1,
                                              ValueRefBase<int>* int_ref2,
                                              ValueRefBase<int>* int_ref3,
                                              ValueRefBase<std::string>* string_ref1,
                                              ValueRefBase<std::string>* string_ref2) :
    Variable<T>(ValueRef::NON_OBJECT_REFERENCE, std::vector<std::string>(1, variable_name)),
    m_int_ref1(int_ref1),
    m_int_ref2(int_ref2),
    m_int_ref3(int_ref3),
    m_string_ref1(string_ref1),
    m_string_ref2(string_ref2)
{
    //std::cout << "ComplexVariable: " << variable_name << ", "
    //          << int_ref1 << ", " << int_ref2 << ", "
    //          << string_ref1 << ", " << string_ref2 << std::endl;
}

template <class T>
ValueRef::ComplexVariable<T>::~ComplexVariable()
{
    delete m_int_ref1;
    delete m_int_ref2;
    delete m_int_ref3;
    delete m_string_ref1;
    delete m_string_ref2;
}

template <class T>
bool ValueRef::ComplexVariable<T>::operator==(const ValueRef::ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::ComplexVariable<T>& rhs_ = static_cast<const ValueRef::ComplexVariable<T>&>(rhs);

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
const ValueRef::ValueRefBase<int>* ValueRef::ComplexVariable<T>::IntRef1() const
{ return m_int_ref1; }

template <class T>
const ValueRef::ValueRefBase<int>* ValueRef::ComplexVariable<T>::IntRef2() const
{ return m_int_ref2; }

template <class T>
const ValueRef::ValueRefBase<int>* ValueRef::ComplexVariable<T>::IntRef3() const
{ return m_int_ref3; }

template <class T>
const ValueRef::ValueRefBase<std::string>* ValueRef::ComplexVariable<T>::StringRef1() const
{ return m_string_ref1; }

template <class T>
const ValueRef::ValueRefBase<std::string>* ValueRef::ComplexVariable<T>::StringRef2() const
{ return m_string_ref2; }

template <class T>
bool ValueRef::ComplexVariable<T>::RootCandidateInvariant() const
{
    return ValueRef::Variable<T>::RootCandidateInvariant()
        && (!m_int_ref1 || m_int_ref1->RootCandidateInvariant())
        && (!m_int_ref2 || m_int_ref2->RootCandidateInvariant())
        && (!m_int_ref3 || m_int_ref3->RootCandidateInvariant())
        && (!m_string_ref1 || m_string_ref1->RootCandidateInvariant())
        && (!m_string_ref2 || m_string_ref2->RootCandidateInvariant());
}

template <class T>
bool ValueRef::ComplexVariable<T>::LocalCandidateInvariant() const
{
    return (!m_int_ref1 || m_int_ref1->LocalCandidateInvariant())
        && (!m_int_ref2 || m_int_ref2->LocalCandidateInvariant())
        && (!m_int_ref3 || m_int_ref3->LocalCandidateInvariant())
        && (!m_string_ref1 || m_string_ref1->LocalCandidateInvariant())
        && (!m_string_ref2 || m_string_ref2->LocalCandidateInvariant());
}

template <class T>
bool ValueRef::ComplexVariable<T>::TargetInvariant() const
{
    return (!m_int_ref1 || m_int_ref1->TargetInvariant())
        && (!m_int_ref2 || m_int_ref2->TargetInvariant())
        && (!m_int_ref3 || m_int_ref3->TargetInvariant())
        && (!m_string_ref1 || m_string_ref1->TargetInvariant())
        && (!m_string_ref2 || m_string_ref2->TargetInvariant());
}

template <class T>
bool ValueRef::ComplexVariable<T>::SourceInvariant() const
{
    return (!m_int_ref1 || m_int_ref1->SourceInvariant())
        && (!m_int_ref2 || m_int_ref2->SourceInvariant())
        && (!m_int_ref3 || m_int_ref3->SourceInvariant())
        && (!m_string_ref1 || m_string_ref1->SourceInvariant())
        && (!m_string_ref2 || m_string_ref2->SourceInvariant());
}

template <class T>
std::string ValueRef::ComplexVariable<T>::Description() const {
    std::string variable_name;
    if (!this->m_property_name.empty())
        variable_name = this->m_property_name.back();
    std::string retval = UserString("DESC_COMPLEX") + ": [(" + UserString("DESC_VARIABLE_NAME") + ": " + variable_name + ") (";
    if (variable_name == "PartCapacity") {
        //retval += m_string_ref1->Description();
    } else if (variable_name == "JumpsBetween") {
        if (m_int_ref1)
            retval += m_int_ref1->Description() + ", ";
        if (m_int_ref2)
            retval += m_int_ref2->Description() + ", ";
    } else if (false) {
        if (m_int_ref1)
            retval += m_int_ref1->Description() + ", ";
        if (m_int_ref2)
            retval += m_int_ref2->Description() + ", ";
        if (m_int_ref2)
            retval += m_int_ref2->Description() + ", ";
        if (m_string_ref1)
            retval += m_string_ref1->Description() + ", ";
        if (m_string_ref2)
            retval += m_string_ref2->Description();
    }
    retval += ")]";
    return retval;
}

template <class T>
std::string ValueRef::ComplexVariable<T>::Dump() const
{ return "ComplexVariable"; }

template <class T>
void ValueRef::ComplexVariable<T>::SetTopLevelContent(const std::string& content_name)
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

namespace ValueRef {
    template <>
    PlanetSize ComplexVariable<PlanetSize>::Eval(const ScriptingContext& context) const;

    template <>
    PlanetType ComplexVariable<PlanetType>::Eval(const ScriptingContext& context) const;

    template <>
    PlanetEnvironment ComplexVariable<PlanetEnvironment>::Eval(const ScriptingContext& context) const;

    template <>
    UniverseObjectType ComplexVariable<UniverseObjectType>::Eval(const ScriptingContext& context) const;

    template <>
    StarType ComplexVariable<StarType>::Eval(const ScriptingContext& context) const;

    template <>
    double ComplexVariable<double>::Eval(const ScriptingContext& context) const;

    template <>
    int ComplexVariable<int>::Eval(const ScriptingContext& context) const;
}

template <class T>
template <class Archive>
void ValueRef::ComplexVariable<T>::serialize(Archive& ar, const unsigned int version)
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
ValueRef::StaticCast<FromType, ToType>::StaticCast(ValueRef::Variable<FromType>* value_ref) :
    ValueRef::Variable<ToType>(value_ref->GetReferenceType(), value_ref->PropertyName()),
    m_value_ref(value_ref)
{}

template <class FromType, class ToType>
ValueRef::StaticCast<FromType, ToType>::StaticCast(ValueRef::ValueRefBase<FromType>* value_ref) :
    ValueRef::Variable<ToType>(ValueRef::NON_OBJECT_REFERENCE),
    m_value_ref(value_ref)
{}

template <class FromType, class ToType>
ValueRef::StaticCast<FromType, ToType>::~StaticCast()
{ delete m_value_ref; }

template <class FromType, class ToType>
bool ValueRef::StaticCast<FromType, ToType>::operator==(const ValueRef::ValueRefBase<ToType>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::StaticCast<FromType, ToType>& rhs_ =
        static_cast<const ValueRef::StaticCast<FromType, ToType>&>(rhs);

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
ToType ValueRef::StaticCast<FromType, ToType>::Eval(const ScriptingContext& context) const
{ return static_cast<ToType>(m_value_ref->Eval(context)); }

template <class FromType, class ToType>
bool ValueRef::StaticCast<FromType, ToType>::RootCandidateInvariant() const
{ return m_value_ref->RootCandidateInvariant(); }

template <class FromType, class ToType>
bool ValueRef::StaticCast<FromType, ToType>::LocalCandidateInvariant() const
{ return m_value_ref->LocalCandidateInvariant(); }

template <class FromType, class ToType>
bool ValueRef::StaticCast<FromType, ToType>::TargetInvariant() const
{ return m_value_ref->TargetInvariant(); }

template <class FromType, class ToType>
bool ValueRef::StaticCast<FromType, ToType>::SourceInvariant() const
{ return m_value_ref->SourceInvariant(); }

template <class FromType, class ToType>
std::string ValueRef::StaticCast<FromType, ToType>::Description() const
{ return m_value_ref->Description(); }

template <class FromType, class ToType>
std::string ValueRef::StaticCast<FromType, ToType>::Dump() const
{ return m_value_ref->Dump(); }

template <class FromType, class ToType>
void ValueRef::StaticCast<FromType, ToType>::SetTopLevelContent(const std::string& content_name)
{
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <class FromType, class ToType>
template <class Archive>
void ValueRef::StaticCast<FromType, ToType>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// StringCast                                            //
///////////////////////////////////////////////////////////
template <class FromType>
ValueRef::StringCast<FromType>::StringCast(ValueRef::Variable<FromType>* value_ref) :
    ValueRef::Variable<std::string>(value_ref->GetReferenceType(), value_ref->PropertyName()),
    m_value_ref(value_ref)
{}

template <class FromType>
ValueRef::StringCast<FromType>::StringCast(ValueRef::ValueRefBase<FromType>* value_ref) :
    ValueRef::Variable<std::string>(ValueRef::NON_OBJECT_REFERENCE),
    m_value_ref(value_ref)
{}

template <class FromType>
ValueRef::StringCast<FromType>::~StringCast()
{ delete m_value_ref; }

template <class FromType>
bool ValueRef::StringCast<FromType>::operator==(const ValueRef::ValueRefBase<std::string>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::StringCast<FromType>& rhs_ =
        static_cast<const ValueRef::StringCast<FromType>&>(rhs);

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
std::string ValueRef::StringCast<FromType>::Eval(const ScriptingContext& context) const
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

namespace ValueRef {
    template <>
    std::string StringCast<double>::Eval(const ScriptingContext& context) const;
}

template <class FromType>
bool ValueRef::StringCast<FromType>::RootCandidateInvariant() const
{ return m_value_ref->RootCandidateInvariant(); }

template <class FromType>
bool ValueRef::StringCast<FromType>::LocalCandidateInvariant() const
{ return m_value_ref->LocalCandidateInvariant(); }

template <class FromType>
bool ValueRef::StringCast<FromType>::TargetInvariant() const
{ return m_value_ref->TargetInvariant(); }

template <class FromType>
bool ValueRef::StringCast<FromType>::SourceInvariant() const
{ return m_value_ref->SourceInvariant(); }

template <class FromType>
std::string ValueRef::StringCast<FromType>::Description() const
{ return m_value_ref->Description(); }

template <class FromType>
std::string ValueRef::StringCast<FromType>::Dump() const
{ return m_value_ref->Dump(); }

template <class FromType>
void ValueRef::StringCast<FromType>::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref)
        m_value_ref->SetTopLevelContent(content_name);
}

template <class FromType>
template <class Archive>
void ValueRef::StringCast<FromType>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// UserStringLookup                                      //
///////////////////////////////////////////////////////////
template <class Archive>
void ValueRef::UserStringLookup::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase<std::string>)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Operation<T>::Operation(OpType op_type, ValueRefBase<T>* operand1, ValueRefBase<T>* operand2) :
    m_op_type(op_type),
    m_operand1(operand1),
    m_operand2(operand2)
{}

template <class T>
ValueRef::Operation<T>::Operation(OpType op_type, ValueRefBase<T>* operand) :
    m_op_type(op_type),
    m_operand1(operand),
    m_operand2(0)
{}

template <class T>
ValueRef::Operation<T>::~Operation()
{
    delete m_operand1;
    delete m_operand2;
}

template <class T>
bool ValueRef::Operation<T>::operator==(const ValueRef::ValueRefBase<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const ValueRef::Operation<T>& rhs_ = static_cast<const ValueRef::Operation<T>&>(rhs);

    if (m_operand1 == rhs_.m_operand1) {
        // check next member
    } else if (!m_operand1 || !rhs_.m_operand1) {
        return false;
    } else {
        if (*m_operand1 != *(rhs_.m_operand1))
            return false;
    }

    if (m_operand2 == rhs_.m_operand2) {
        // check next member
    } else if (!m_operand2 || !rhs_.m_operand2) {
        return false;
    } else {
        if (*m_operand2 != *(rhs_.m_operand2))
            return false;
    }

    return true;
}

template <class T>
ValueRef::OpType ValueRef::Operation<T>::GetOpType() const
{ return m_op_type; }

template <class T>
const ValueRef::ValueRefBase<T>* ValueRef::Operation<T>::LHS() const
{ return m_operand1; }

template <class T>
const ValueRef::ValueRefBase<T>* ValueRef::Operation<T>::RHS() const
{ return m_operand2; }

template <class T>
T ValueRef::Operation<T>::Eval(const ScriptingContext& context) const
{
    switch (m_op_type) {
        case PLUS:
            return T(m_operand1->Eval(context) +
                     m_operand2->Eval(context));
            break;
        case MINUS:
            return T(m_operand1->Eval(context) -
                     m_operand2->Eval(context));
            break;
        default:
            throw std::runtime_error("ValueRef evaluated with an unknown or invalid OpType.");
            break;
    }
}

namespace ValueRef {
    template <>
    std::string Operation<std::string>::Eval(const ScriptingContext& context) const;

    template <>
    double      Operation<double>::Eval(const ScriptingContext& context) const;

    template <>
    int         Operation<int>::Eval(const ScriptingContext& context) const;
}

template <class T>
bool ValueRef::Operation<T>::RootCandidateInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM)
        return false;
    if (m_operand1 && !m_operand1->RootCandidateInvariant())
        return false;
    if (m_operand2 && !m_operand2->RootCandidateInvariant())
        return false;
    return true;
}

template <class T>
bool ValueRef::Operation<T>::LocalCandidateInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM)
        return false;
    if (m_operand1 && !m_operand1->LocalCandidateInvariant())
        return false;
    if (m_operand2 && !m_operand2->LocalCandidateInvariant())
        return false;
    return true;
}

template <class T>
bool ValueRef::Operation<T>::TargetInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM)
        return false;
    if (m_operand1 && !m_operand1->TargetInvariant())
        return false;
    if (m_operand2 && !m_operand2->TargetInvariant())
        return false;
    return true;
}

template <class T>
bool ValueRef::Operation<T>::SourceInvariant() const
{
    if (m_op_type == RANDOM_UNIFORM)
        return false;
    if (m_operand1 && !m_operand1->SourceInvariant())
        return false;
    if (m_operand2 && !m_operand2->SourceInvariant())
        return false;
    return true;
}

template <class T>
std::string ValueRef::Operation<T>::Description() const
{
    if (m_op_type == NEGATE) {
        //DebugLogger() << "Operation is negation";
        if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
            OpType op_type = rhs->GetOpType();
            if (op_type == PLUS     || op_type == MINUS ||
                op_type == TIMES    || op_type == DIVIDE ||
                op_type == NEGATE   || op_type == EXPONENTIATE)
            return "-(" + m_operand1->Description() + ")";
        } else {
            return "-" + m_operand1->Description();
        }
    }

    if (m_op_type == ABS)
        return "abs(" + m_operand1->Description() + ")";
    if (m_op_type == LOGARITHM)
        return "log(" + m_operand1->Description() + ")";
    if (m_op_type == SINE)
        return "sin(" + m_operand1->Description() + ")";
    if (m_op_type == COSINE)
        return "cos(" + m_operand1->Description() + ")";
    if (m_op_type == MINIMUM)
        return "min(" + m_operand1->Description() + ", " + m_operand2->Description() + ")";
    if (m_op_type == MAXIMUM)
        return "max(" + m_operand1->Description() + ", " + m_operand2->Description() + ")";
    if (m_op_type == RANDOM_UNIFORM)
        return "randomnumber(" + m_operand1->Description() + ", " + m_operand2->Description() + ")";


    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (const ValueRef::Operation<T>* lhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
        OpType op_type = lhs->GetOpType();
        if (
            (m_op_type == EXPONENTIATE &&
             (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
              op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
            ) ||
            ((m_op_type == TIMES        || m_op_type == DIVIDE) &&
             (op_type == PLUS           || op_type == MINUS)    || op_type == NEGATE)
           )
            parenthesize_lhs = true;
    }
    if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand2)) {
        OpType op_type = rhs->GetOpType();
        if (
            (m_op_type == EXPONENTIATE &&
             (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
              op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
            ) ||
            ((m_op_type == TIMES        || m_op_type == DIVIDE) &&
             (op_type == PLUS           || op_type == MINUS)    || op_type == NEGATE)
           )
            parenthesize_rhs = true;
    }

    std::string retval;
    if (parenthesize_lhs)
        retval += '(' + m_operand1->Description() + ')';
    else
        retval += m_operand1->Description();

    switch (m_op_type) {
    case PLUS:          retval += " + "; break;
    case MINUS:         retval += " - "; break;
    case TIMES:         retval += " * "; break;
    case DIVIDE:        retval += " / "; break;
    case EXPONENTIATE:  retval += " ^ "; break;
    default:            retval += " ? "; break;
    }

    if (parenthesize_rhs)
        retval += '(' + m_operand2->Description() + ')';
    else
        retval += m_operand2->Description();

    return retval;
}

namespace ValueRef {
    template <>
    std::string Operation<double>::Description() const;
}

template <class T>
std::string ValueRef::Operation<T>::Dump() const
{
    if (m_op_type == NEGATE) {
        if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
            OpType op_type = rhs->GetOpType();
            if (op_type == PLUS     || op_type == MINUS ||
                op_type == TIMES    || op_type == DIVIDE ||
                op_type == NEGATE   || op_type == EXPONENTIATE)
            return "-(" + m_operand1->Dump() + ")";
        } else {
            return "-" + m_operand1->Dump();
        }
    }

    if (m_op_type == ABS)
        return "abs(" + m_operand1->Dump() + ")";
    if (m_op_type == LOGARITHM)
        return "log(" + m_operand1->Dump() + ")";
    if (m_op_type == SINE)
        return "sin(" + m_operand1->Dump() + ")";
    if (m_op_type == COSINE)
        return "cos(" + m_operand1->Dump() + ")";
    if (m_op_type == MINIMUM)
        return "min(" + m_operand1->Dump() + ", " + m_operand1->Dump() + ")";
    if (m_op_type == MAXIMUM)
        return "max(" + m_operand1->Dump() + ", " + m_operand1->Dump() + ")";
    if (m_op_type == RANDOM_UNIFORM)
        return "random(" + m_operand1->Dump() + ", " + m_operand1->Dump() + ")";


    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (const ValueRef::Operation<T>* lhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
        OpType op_type = lhs->GetOpType();
        if (
            (m_op_type == EXPONENTIATE &&
             (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
              op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
            ) ||
            ((m_op_type == TIMES        || m_op_type == DIVIDE) &&
             (op_type == PLUS           || op_type == MINUS)    || op_type == NEGATE)
           )
            parenthesize_lhs = true;
    }
    if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand2)) {
        OpType op_type = rhs->GetOpType();
        if (
            (m_op_type == EXPONENTIATE &&
             (op_type == EXPONENTIATE   || op_type == TIMES     || op_type == DIVIDE ||
              op_type == PLUS           || op_type == MINUS     || op_type == NEGATE)
            ) ||
            ((m_op_type == TIMES        || m_op_type == DIVIDE) &&
             (op_type == PLUS           || op_type == MINUS)    || op_type == NEGATE)
           )
            parenthesize_rhs = true;
    }

    std::string retval;
    if (parenthesize_lhs)
        retval += '(' + m_operand1->Dump() + ')';
    else
        retval += m_operand1->Dump();

    switch (m_op_type) {
    case PLUS:          retval += " + "; break;
    case MINUS:         retval += " - "; break;
    case TIMES:         retval += " * "; break;
    case DIVIDE:        retval += " / "; break;
    case EXPONENTIATE:  retval += " ^ "; break;
    default:            retval += " ? "; break;
    }

    if (parenthesize_rhs)
        retval += '(' + m_operand2->Dump() + ')';
    else
        retval += m_operand2->Dump();

    return retval;
}

template <class T>
void ValueRef::Operation<T>::SetTopLevelContent(const std::string& content_name) {
    if (m_operand1)
        m_operand1->SetTopLevelContent(content_name);
    if (m_operand2)
        m_operand2->SetTopLevelContent(content_name);
}

template <class T>
template <class Archive>
void ValueRef::Operation<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_op_type)
        & BOOST_SERIALIZATION_NVP(m_operand1)
        & BOOST_SERIALIZATION_NVP(m_operand2);
}

template <class T>
bool ValueRef::ConstantExpr(const ValueRefBase<T>* expr)
{
    assert(expr);
    if (dynamic_cast<const Constant<T>*>(expr))
        return true;
    else if (dynamic_cast<const Variable<T>*>(expr))
        return false;
    else if (const Operation<T>* op = dynamic_cast<const Operation<T>*>(expr))
        return ConstantExpr(op->LHS()) && ConstantExpr(op->RHS());
    return false;
}


#endif // _ValueRef_h_
