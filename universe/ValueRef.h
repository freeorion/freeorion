// -*- C++ -*-
#ifndef _ValueRef_h_
#define _ValueRef_h_

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
}

/** the base class for all ValueRef classes.  This class provides the public interface for a ValueRef expression tree. */
template <class T>
struct ValueRef::ValueRefBase
{
    virtual ~ValueRefBase() {} ///< virtual dtor
    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const = 0; ///< evaluates the expression tree and return the results
};

/** the constant value leaf ValueRef class. */
template <class T>
struct ValueRef::Constant : public ValueRef::ValueRefBase<T>
{
    Constant(T value); ///< basic ctor
    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const;

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
    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const;

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
    virtual T Eval(const UniverseObject* source, const UniverseObject* target) const;

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
T ValueRef::Constant<T>::Eval(const UniverseObject* source, const UniverseObject* target) const
{
    return m_value;
}

///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////
template <class T>
ValueRef::Variable<T>::Variable(bool source_ref, const std::string& property_name) :
    m_source_ref(source_ref),
    m_property_name(detail::TokenizeDottedReference(property_name))
{
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

inline std::pair<std::string, std::string> ValueRefRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _ValueRef_h_
