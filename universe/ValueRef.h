// -*- C++ -*-
#ifndef _ValueRef_h_
#define _ValueRef_h_

#include "Enums.h"
#include "../util/MultiplayerCommon.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>
#include <boost/mpl/if.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits/is_enum.hpp>

#include <string>
#include <vector>

class UniverseObject;

namespace detail {
    std::vector<std::string> TokenizeDottedReference(const std::string& str);
}

/** this namespace contains ValueRefBase and its subclasses.  The ValueRefBase subclasses represent expression trees that may be 
    evaluated at various times, and which refer to both constant and variable values. */
namespace ValueRef {
    template <class T> struct ValueRefBase;
    template <class T> struct Constant;
    template <class T> struct Variable;
    template <class FromType, class ToType> struct StaticCast;
    template <class T> struct Operation;
    enum OpType {
        PLUS,
        MINUS,
        TIMES,
        DIVIDES,
        NEGATE
    };
    template <class T> bool ConstantExpr(const ValueRefBase<T>* expr);
}

/** the base class for all ValueRef classes.  This class provides the public interface for a ValueRef expression tree. */
template <class T>
struct ValueRef::ValueRefBase
{
    virtual ~ValueRefBase() {} ///< virtual dtor

    /** Evaluates the expression tree and return the results; \a current_value
        is used to fill in any instances of the "CurrentValue" variable that
        exist in the tree. */
    virtual T Eval(const UniverseObject* source, const UniverseObject* target,
                   const boost::any& current_value) const = 0;

    virtual std::string Description() const = 0;
    virtual std::string Dump() const = 0; ///< returns a text description of this type of special

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** the constant value leaf ValueRef class. */
template <class T>
struct ValueRef::Constant : public ValueRef::ValueRefBase<T>
{
    Constant(T value); ///< basic ctor

    T Value() const;

    virtual T Eval(const UniverseObject* source, const UniverseObject* target,
                   const boost::any& current_value) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    T m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** the variable value ValueRef class.  The value returned by this node is taken from either the \a source or \a target parameters to Eval. */
template <class T>
struct ValueRef::Variable : public ValueRef::ValueRefBase<T>
{
    /** basic ctor.  If \a source_ref is true, the field corresponding to \a property_name is read from the 
        \a source parameter of Eval; otherwise, the same field is read from Eval's \a target parameter. */
    Variable(bool source_ref, const std::string& property_name);

    bool SourceRef() const;
    const std::vector<std::string>& PropertyName() const;

    virtual T Eval(const UniverseObject* source, const UniverseObject* target,
                   const boost::any& current_value) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

protected:
    Variable(bool source_ref, const std::vector<std::string>& property_name);

private:
    bool                     m_source_ref;
    std::vector<std::string> m_property_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** the variable static_cast class.  The value returned by this node is taken from the ctor \a value_ref parameter's FromType value,
    static_cast to ToType. */
template <class FromType, class ToType>
struct ValueRef::StaticCast : public ValueRef::Variable<ToType>
{
    StaticCast(const ValueRef::Variable<FromType>* value_ref);
    ~StaticCast();

    virtual double Eval(const UniverseObject* source, const UniverseObject* target,
                   const boost::any& current_value) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    const ValueRefBase<FromType>* m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** an arithmetic operation node ValueRef class.  One of addition, subtraction, mutiplication, division, or unary negation is 
    performed on the child(ren) of this node, and the result is returned. */
template <class T>
struct ValueRef::Operation : public ValueRef::ValueRefBase<T>
{
    Operation(OpType op_type, const ValueRefBase<T>* operand1, const ValueRefBase<T>* operand2); ///< binary operation ctor
    Operation(OpType op_type, const ValueRefBase<T>* operand); ///< unary operation ctor
    ~Operation(); ///< dtor

    OpType GetOpType() const;
    const ValueRefBase<T>* LHS() const;
    const ValueRefBase<T>* RHS() const;

    virtual T Eval(const UniverseObject* source, const UniverseObject* target,
                   const boost::any& current_value) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    OpType                 m_op_type;
    const ValueRefBase<T>* m_operand1;
    const ValueRefBase<T>* m_operand2;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** A function that returns the correct amount of spacing for the current indentation level during a dump.  Note that
    this function is used by several units (Condition.cpp, Effect.cpp, etc.), not just this one. */
std::string DumpIndent();


// Template Implementations
///////////////////////////////////////////////////////////
// ValueRefBase                                          //
///////////////////////////////////////////////////////////
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
T ValueRef::Constant<T>::Value() const
{
    return m_value;
}

template <class T>
T ValueRef::Constant<T>::Eval(const UniverseObject* source, const UniverseObject* target,
                              const boost::any& current_value) const
{
    return m_value;
}

template <class T>
std::string ValueRef::Constant<T>::Description() const
{
    return UserString(boost::lexical_cast<std::string>(m_value));
}

namespace ValueRef {
    template <>
    std::string Constant<int>::Description() const;

    template <>
    std::string Constant<double>::Description() const;

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
}

template <class T>
template <class Archive>
void ValueRef::Constant<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value);
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Variable<T>::Variable(bool source_ref, const std::string& property_name) :
    m_source_ref(source_ref),
    m_property_name(::detail::TokenizeDottedReference(property_name))
{}

template <class T>
ValueRef::Variable<T>::Variable(bool source_ref, const std::vector<std::string>& property_name) :
    m_source_ref(source_ref),
    m_property_name(property_name)
{}

template <class T>
bool ValueRef::Variable<T>::SourceRef() const
{
    return m_source_ref;
}

template <class T>
const std::vector<std::string>& ValueRef::Variable<T>::PropertyName() const
{
    return m_property_name;
}

template <class T>
std::string ValueRef::Variable<T>::Description() const
{
    boost::format formatter = FlexibleFormat(UserString("DESC_VALUE_REF_MULTIPART_VARIABLE" + boost::lexical_cast<std::string>(m_property_name.size())));
    formatter % UserString(m_source_ref ? "DESC_VAR_SOURCE" : "DESC_VAR_TARGET");
    for (unsigned int i = 0; i < m_property_name.size(); ++i) {
        formatter % UserString("DESC_VAR_" + boost::to_upper_copy(m_property_name[i]));
    }
    return boost::io::str(formatter);
}

template <class T>
std::string ValueRef::Variable<T>::Dump() const
{
    std::string str(m_source_ref ? "Source" : "Target");
    // now that we have some variables that apply globally, we need to special-case them:
    if (m_property_name.back() == "CurrentTurn")
        str = "";
    for (unsigned int i = 0; i < m_property_name.size(); ++i) {
        str += '.' + m_property_name[i];
    }
    return str;
}

namespace ValueRef {
    template <>
    PlanetSize Variable<PlanetSize>::Eval(const UniverseObject* source, const UniverseObject* target,
                                          const boost::any& current_value) const;

    template <>
    PlanetType Variable<PlanetType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                          const boost::any& current_value) const;

    template <>
    PlanetEnvironment Variable<PlanetEnvironment>::Eval(const UniverseObject* source, const UniverseObject* target,
                                                        const boost::any& current_value) const;

    template <>
    UniverseObjectType Variable<UniverseObjectType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                                          const boost::any& current_value) const;

    template <>
    StarType Variable<StarType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                      const boost::any& current_value) const;

    template <>
    double Variable<double>::Eval(const UniverseObject* source, const UniverseObject* target,
                                  const boost::any& current_value) const;

    template <>
    int Variable<int>::Eval(const UniverseObject* source, const UniverseObject* target,
                            const boost::any& current_value) const;
}

template <class T>
template <class Archive>
void ValueRef::Variable<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_source_ref)
        & BOOST_SERIALIZATION_NVP(m_property_name);
}

///////////////////////////////////////////////////////////
// StaticCast                                            //
///////////////////////////////////////////////////////////
template <class FromType, class ToType>
ValueRef::StaticCast<FromType, ToType>::StaticCast(const ValueRef::Variable<FromType>* value_ref) :
    ValueRef::Variable<ToType>(value_ref->SourceRef(), value_ref->PropertyName()),
    m_value_ref(value_ref)
{}

template <class FromType, class ToType>
ValueRef::StaticCast<FromType, ToType>::~StaticCast()
{ delete m_value_ref; }

template <class FromType, class ToType>
double ValueRef::StaticCast<FromType, ToType>::Eval(const UniverseObject* source, const UniverseObject* target,
                                                    const boost::any& current_value) const
{
    return static_cast<ToType>(m_value_ref->Eval(source, target, current_value));
}

template <class FromType, class ToType>
std::string ValueRef::StaticCast<FromType, ToType>::Description() const
{
    return m_value_ref->Description();
}

template <class FromType, class ToType>
std::string ValueRef::StaticCast<FromType, ToType>::Dump() const
{
    return m_value_ref->Dump();
}

template <class FromType, class ToType>
template <class Archive>
void ValueRef::StaticCast<FromType, ToType>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
        & BOOST_SERIALIZATION_NVP(m_value_ref);
}

///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Operation<T>::Operation(OpType op_type, const ValueRefBase<T>* operand1, const ValueRefBase<T>* operand2) :
    m_op_type(op_type),
    m_operand1(operand1),
    m_operand2(operand2)
{}

template <class T>
ValueRef::Operation<T>::Operation(OpType op_type, const ValueRefBase<T>* operand) :
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
ValueRef::OpType ValueRef::Operation<T>::GetOpType() const
{
    return m_op_type;
}

template <class T>
const ValueRef::ValueRefBase<T>* ValueRef::Operation<T>::LHS() const
{
    return m_operand1;
}

template <class T>
const ValueRef::ValueRefBase<T>* ValueRef::Operation<T>::RHS() const
{
    return m_operand2;
}

template <class T>
T ValueRef::Operation<T>::Eval(const UniverseObject* source, const UniverseObject* target,
                               const boost::any& current_value) const
{
    switch (m_op_type) {
        case PLUS:
            return T(m_operand1->Eval(source, target, current_value) +
                     m_operand2->Eval(source, target, current_value));
            break;
        case MINUS:
            return T(m_operand1->Eval(source, target, current_value) -
                     m_operand2->Eval(source, target, current_value));
            break;
        case TIMES:
            return T(m_operand1->Eval(source, target, current_value) *
                     m_operand2->Eval(source, target, current_value));
            break;
        case DIVIDES:
            return T(m_operand1->Eval(source, target, current_value) /
                     m_operand2->Eval(source, target, current_value));
            break;
        case NEGATE:
            return T(-m_operand1->Eval(source, target, current_value));
            break;
        default:
            throw std::runtime_error("ValueRef evaluated with an unknown OpType.");
            break;
    }
}

template <class T>
std::string ValueRef::Operation<T>::Description() const
{
    if (m_op_type == NEGATE)
        return ("-(" + m_operand1->Description() + ")");

    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (const ValueRef::Operation<T>* lhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
        if ((m_op_type == TIMES || m_op_type == DIVIDES) &&
            (lhs->GetOpType() == PLUS || lhs->GetOpType() == MINUS))
            parenthesize_lhs = true;
    }
    if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand2)) {
        if ((m_op_type == TIMES || m_op_type == DIVIDES) &&
            (rhs->GetOpType() == PLUS || rhs->GetOpType() == MINUS))
            parenthesize_rhs = true;
    }
    std::string retval;
    if (parenthesize_lhs)
        retval += '(' + m_operand1->Description() + ')';
    else
        retval += m_operand1->Description();
    switch (m_op_type) {
    case PLUS:    retval += " + "; break;
    case MINUS:   retval += " - "; break;
    case TIMES:   retval += " * "; break;
    case DIVIDES: retval += " / "; break;
    default:      retval += " ? "; break;
    }
    if (parenthesize_rhs)
        retval += '(' + m_operand2->Description() + ')';
    else
        retval += m_operand2->Description();
    return retval;
}

template <class T>
std::string ValueRef::Operation<T>::Dump() const
{
    if (m_op_type == NEGATE) {
        if (dynamic_cast<const ValueRef::Operation<T>*>(m_operand1))
            return "-(" + m_operand1->Dump() + ")";
        else
            return "-" + m_operand1->Dump();
    }

    bool parenthesize_lhs = false;
    bool parenthesize_rhs = false;
    if (const ValueRef::Operation<T>* lhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
        if ((m_op_type == TIMES || m_op_type == DIVIDES) &&
            (lhs->GetOpType() == PLUS || lhs->GetOpType() == MINUS))
            parenthesize_lhs = true;
    }
    if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand2)) {
        if ((m_op_type == TIMES || m_op_type == DIVIDES) &&
            (rhs->GetOpType() == PLUS || rhs->GetOpType() == MINUS))
            parenthesize_rhs = true;
    }

    std::string retval;
    if (parenthesize_lhs)
        retval += '(' + m_operand1->Dump() + ')';
    else
        retval += m_operand1->Dump();

    switch (m_op_type) {
    case PLUS:    retval += " + "; break;
    case MINUS:   retval += " - "; break;
    case TIMES:   retval += " * "; break;
    case DIVIDES: retval += " / "; break;
    default:      retval += " ? "; break;
    }

    if (parenthesize_rhs)
        retval += '(' + m_operand2->Dump() + ')';
    else
        retval += m_operand2->Dump();

    return retval;
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
