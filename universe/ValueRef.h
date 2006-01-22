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
    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const = 0; ///< evaluates the expression tree and return the results
    virtual std::string Description() const = 0;
};

/** the constant value leaf ValueRef class. */
template <class T>
struct ValueRef::Constant : public ValueRef::ValueRefBase<T>
{
    Constant(T value); ///< basic ctor

    T Value() const;

    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const;
    virtual std::string Description() const;

private:
    T m_value;
};

/** the variable value leaf ValueRef class.  The value returned by this node is taken from either the \a source or \a target parameters to Eval. */
template <class T>
struct ValueRef::Variable : public ValueRef::ValueRefBase<T>
{
    /** basic ctor.  If \a source_ref is true, the field corresponding to \a property_name is read from the 
        \a source parameter of Eval; otherwise, the same field is read from Eval's \a target parameter. */
    Variable(bool source_ref, const std::string& property_name);

    bool SourceRef() const;
    const std::vector<std::string>& PropertyName() const;

    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const;
    virtual std::string Description() const;

private:
    bool                     m_source_ref;
    std::vector<std::string> m_property_name;
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

    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const;
    virtual std::string Description() const;

private:
    OpType                 m_op_type;
    const ValueRefBase<T>* m_operand1;
    const ValueRefBase<T>* m_operand2;
};


// Temlate Implementations

///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Constant<T>::Constant(T value) :
    m_value(value)
{
}

template <class T>
T ValueRef::Constant<T>::Value() const
{
    return m_value;
}

template <class T>
T ValueRef::Constant<T>::Eval(const UniverseObject* source, const UniverseObject* target) const
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
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Variable<T>::Variable(bool source_ref, const std::string& property_name) :
    m_source_ref(source_ref),
    m_property_name(::detail::TokenizeDottedReference(property_name))
{
}

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
    boost::format formatter(UserString("DESC_VALUE_REF_MULTIPART_VARIABLE" + boost::lexical_cast<std::string>(m_property_name.size())));
    formatter % UserString(m_source_ref ? "DESC_VAR_SOURCE" : "DESC_VAR_TARGET");
    for (unsigned int i = 0; i < m_property_name.size(); ++i) {
        formatter % UserString("DESC_VAR_" + boost::to_upper_copy(m_property_name[i]));
    }
    return boost::io::str(formatter);
}

namespace ValueRef {
template <>
PlanetSize Variable<PlanetSize>::Eval(const UniverseObject* source, const UniverseObject* target) const;

template <>
PlanetType Variable<PlanetType>::Eval(const UniverseObject* source, const UniverseObject* target) const;

template <>
PlanetEnvironment Variable<PlanetEnvironment>::Eval(const UniverseObject* source, const UniverseObject* target) const;

template <>
UniverseObjectType Variable<UniverseObjectType>::Eval(const UniverseObject* source, const UniverseObject* target) const;

template <>
StarType Variable<StarType>::Eval(const UniverseObject* source, const UniverseObject* target) const;

template <>
FocusType Variable<FocusType>::Eval(const UniverseObject* source, const UniverseObject* target) const;

template <>
double Variable<double>::Eval(const UniverseObject* source, const UniverseObject* target) const;

template <>
int Variable<int>::Eval(const UniverseObject* source, const UniverseObject* target) const;
}

///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Operation<T>::Operation(OpType op_type, const ValueRefBase<T>* operand1, const ValueRefBase<T>* operand2) :
    m_op_type(op_type),
    m_operand1(operand1),
    m_operand2(operand2)
{
}

template <class T>
ValueRef::Operation<T>::Operation(OpType op_type, const ValueRefBase<T>* operand) :
    m_op_type(op_type),
    m_operand1(operand),
    m_operand2(0)
{
}

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
T ValueRef::Operation<T>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    switch (m_op_type) {
        case PLUS:
            return T(m_operand1->Eval(source, target) + m_operand2->Eval(source, target));
            break;
        case MINUS:
            return T(m_operand1->Eval(source, target) - m_operand2->Eval(source, target));
            break;
        case TIMES:
            return T(m_operand1->Eval(source, target) * m_operand2->Eval(source, target));
            break;
        case DIVIDES:
            return T(m_operand1->Eval(source, target) / m_operand2->Eval(source, target));
            break;
        case NEGATE:
            return T(-m_operand1->Eval(source, target));
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

// template implementations
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


inline std::pair<std::string, std::string> ValueRefRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _ValueRef_h_
